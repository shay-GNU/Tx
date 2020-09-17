#include "tx.h"
#include "txc.h"
#include "lex.h"

TX_FUNC void *tx_realloc(void *ptr, unsigned long size){
    void *ptr1;
    ptr1 = realloc(ptr, size);
    if (!ptr1 && size)
        tx_error("memory full (realloc)");
    return ptr1;
}

TX_FUNC void *tx_malloc(size_t size){
	void *ptr = malloc(size);
	if(!ptr)
		tx_error("memory full (malloc)");
    return ptr;
}

TX_FUNC void *tx_mallocz(size_t size){
    void *ptr = malloc(size);
	if(!ptr)
		tx_error("memory full (malloc)");
	memset(ptr, 0, size);
    return ptr;
}


void vm_print_log(int level,int type,char *fmt, va_list ap){
    char buf[1024];
    vsprintf(buf,fmt,ap);

    if(level == TX_COMPLIE){
        if(type == TX_CWARING){
            printf("%s:(line:%d):%s\n",file->filename,file->line,buf);
        }else if(type == TX_CERROR){
            printf("%s:(line:%d):%s\n",file->filename,file->line,buf);
            exit(1);
        }
    }else if(level == TX_ERROR){
            printf("tx:error:%s\n",buf);
            exit(1);
    }
}

TX_FUNC void tx_error(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	vm_print_log(TX_ERROR, 0,fmt,ap);
}

void tx_cerror(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	vm_print_log(TX_COMPLIE, TX_CERROR,fmt,ap);
}

void tx_cwarning(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	vm_print_log(TX_COMPLIE, TX_CWARING,fmt,ap);
}





TokenSym *tx_add_token(TokenSym **p, const char *str, int size){
	TokenSym *ptr;
    int i = tok_ident - TOK_IDENT;
	if((i % 1024)==0){
		table_ident = tx_realloc(table_ident, (tok_ident + 1024) * sizeof(void *));
	}
	ptr = tx_mallocz(sizeof(TokenSym) + size);
	table_ident[i] = ptr;
    
	ptr->tok = tok_ident++;
	ptr->hash_next = NULL;
    ptr->sym_label = NULL;
    ptr->sym_identifier = NULL;
    ptr->len = size;
	memcpy(ptr->str, str, size);
	ptr->str[size] = '\0';
	*p = ptr;
	return ptr;
}

TokenSym *tok_alloc(const char *str, int len)
{
    TokenSym *ts, **pts;
    int i;
    unsigned int h;
    
    h = TOK_HASH_INIT;
    for(i=0;i<len;i++)
        h = TOK_HASH_FUNC(h, ((unsigned char *)str)[i]);
    h &= (TOK_HASH_SIZE - 1);

    pts = &hash_ident[h];
    for(;;) {
        ts = *pts;
        if (!ts)    
            break;
        if (ts->len == len && !memcmp(ts->str, str, len))
            return ts;
        pts = &(ts->hash_next);
    }
    return tx_add_token(pts, str, len);
} 
#include "vm.h"

typedef struct str_func{
    char *str;
    int isVars;
    int args;
}str_func;
static const str_func functionstr[]={
    {"print", 1, 0},
    {"type", 0, 1},
    {"exit", 0, 0},
    
};

TX_FUNC tx_Complie *tx_newComplie(){
    Symbol *sym;
    tx_Complie *s = tx_mallocz(sizeof(tx_Complie));
    
    last_tok = -1;
    stack_init(&global_sym_stack,8);
    stack_init(&local_sym_stack,8);
    stack_init(&global_label_stack,8);
    int_type.t = T_INT | T_CONST;
    folat_type.t = T_DOUBLE | T_CONST;
    string_type.t = T_STRING | T_CONST;
	default_func_type.t = T_FUNC;
	default_func_type.ref = sym_push(SC_ANOM, &int_type, 0, 0);
    optop = opstack;

    int num=0,c;
    const char *r;
    tok_ident = TOK_IDENT;
    const char *p = keywords[num];
    while (p) {
        r = p;
        for(;;) {
            c = *r++;
            if (c == '\0')
                break;
        }
        tok_alloc(p, r - p - 1);
        p = r;
        p = keywords[++num];
    }
    Type type;memset(&type, 0, sizeof(Type));
    type.t = T_CFUNC | T_FUNC;
    for(num=0; num < sizeof(functionstr)/sizeof(str_func); num++){
        tok_alloc(functionstr[num].str, strlen(functionstr[num].str));
        type.isvars = functionstr[num].isVars;
        type.args = functionstr[num].args;
        sym = func_sym_push(tok_ident -1, &type);
        sym->isCFunc = 1;
    }

    return s;
}




char *pstrcpy(char *buf, int buf_size, const char *s){
    char *q, *q_end;
    int c;

    if (buf_size > 0) {
        q = buf;
        q_end = buf + buf_size - 1;
        while (q < q_end) {
            c = *s++;
            if (c == '\0')
                break;
            *q++ = c;
        }
        *q = '\0';
    }
    return buf;
}

__inline void normalize_slashes(char *path){
    char *p;
    for (p = path; *p; ++p)
        if (*p == '\\')
            *p = '/';
}

void cstr_realloc(CString *cstr, int new_size)
{
    int size;

    size = cstr->size_allocated;
    if (size < 8)
        size = 8; 
    while (size < new_size)
        size = size * 2;
    cstr->data =  tx_realloc( cstr->data, size);
    cstr->size_allocated = size;
}


inline void cstr_reset(CString *cstr)
{
    cstr->size = 0;
}


void cstr_ccat(CString *cstr, int ch)
{
    int size;
    size = cstr->size + 1;
    if (size > cstr->size_allocated)
        cstr_realloc(cstr, size);
    ((unsigned char *)cstr->data)[size - 1] = ch;
    cstr->size = size;
}


#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif


void tx_open_bf(tx_Complie *s1, const char *filename, int initlen){
    BufferedFile *bf;
    int buflen = initlen ? initlen : IO_BUF_SIZE;

    bf = tx_mallocz(sizeof(BufferedFile) + buflen);
    bf->buf_ptr = bf->buffer;
    bf->buf_end = bf->buffer + initlen;
    bf->buf_end[0] = EOF;
    pstrcpy(bf->filename, sizeof(bf->filename), filename);
    bf->line = 1;
    bf->fd = NULL;
    bf->prev = file;
    file = bf;
}

FILE *tx_open(tx_Complie *s, const char *filename){
	FILE *fd = fopen(filename, "rb");
	
    tx_open_bf(s, filename, 0);

#ifdef _WIN32
    normalize_slashes(file->filename);
#endif

	file->fd = fd;
    return fd;
}

void tx_read(BufferedFile *bf){
    int len;
	len = fread(bf->buffer, 1, IO_BUF_SIZE, bf->fd);
	bf->buf_ptr = bf->buffer;
	bf->buf_end = bf->buffer + IO_BUF_SIZE;
	bf->buf_end[0] = (char)EOF;
    bf->buffer[len] = (char)EOF;
}

#include "lex.c"
#include "parse.c"
#include "out.c"

TX_FUNC int tx_complie(tx_Complie *s){
	
    if(!tx_open(s, s->filename))
        tx_error("file not open");
    tx_read(file);
    next();

    decl(T_CONST);
}