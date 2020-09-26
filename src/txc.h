#ifndef TXCC_H
#define TXCC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


#define TOK_IDENT 256

typedef struct BufferedFile {
    char *buf_ptr;
    char *buf_end;
    FILE *fd;
    struct BufferedFile *prev;
    int line;    
    char filename[1024];    
    char buffer[1]; 
} BufferedFile;

BufferedFile *file;
const char *filename;
#define IO_BUF_SIZE 8192    

typedef struct tx_Complie {
    const char *filename;
    const char *outName;
} tx_Complie;


typedef struct TokenSym {
    struct TokenSym *hash_next;
    struct Symbol *sym_label; 
    struct Symbol *sym_identifier; 
    int tok; 
    int len;
    char str[1];
} TokenSym;
int tok;
int tok_ident;          
TokenSym **table_ident; 

#define STRING_MAX_SIZE 1024
char token_buf[STRING_MAX_SIZE + 1];

#define TOK_HASH_SIZE       16384   
TokenSym *hash_ident[TOK_HASH_SIZE];


typedef struct CString {
    int size;
    void *data;
    int size_allocated;
} CString;

typedef union CValue {
    double d;
    char b;
    int64_t i;
    struct {
        int size;
        void *data;
    } str;
    int tab[8/4];
} CValue;
CValue tokc;
CString tokcstr;

typedef struct Type 
{
    int t;
    int isvars;
    int args;
    struct Symbol *ref;
    
} Type;


typedef struct Operand
{
    struct Type type;			    
    unsigned short r;       
    int value;              
    struct Symbol *sym;     
} Operand;
#define OPSTACK_SIZE 256
Operand opstack[OPSTACK_SIZE];  
Operand *optop;	                


#define TX_COMPLIE   0X1
#define TX_CWARING   0X1
#define TX_CERROR    0X2

#define TX_ERROR   0X2

void tx_cwaring(char *fmt, ...);
void tx_cerror(char *fmt, ...);
void tx_error(char *fmt, ...);
void vm_print_log(int level,int type,char *fmt, va_list ap);

inline void cstr_reset(CString *cstr);
void cstr_ccat(CString *cstr, int ch);

static void parse_number(const char *p);
static void parse_string(const char *s, int len);


typedef struct byte_stream 
{ 
	int data_offset;			
    uint8_t *data;					
    int data_allocated;			
    int ind;
} byte_stream;

typedef struct funcInfo{
    int ind;
    char *funcname;
    byte_stream *code;
    uint64_t codeNum;

    byte_stream *constTbale;
    uint32_t constNum;

    char argn;
    int usedRegs;
    int maxRegs;
    int scopeLv;
    char isVararg;

    struct funcInfo *next;
}funcInfo;

int ind;
int funcn;
int main_found;
funcInfo *sec_text;


byte_stream constTbale;
uint32_t constNum;

funcInfo *allocate_storage(Type *type, int r, int has_init, int v, int *addr);
int type_size(Type *t, int *a);
void initializer(Type *type, int c, funcInfo *sec);


typedef struct Symbol{
    union{
        int sym_scope;              
        int jnext;                  
        int isCFunc;                
    };
    int v;						
    int r;						
    int c;						
    Type type;					
    struct Symbol *next;
    struct Symbol *prev;
    struct Symbol *prev_tok;	
    struct funcInfo *func;
    
} Symbol;

Symbol *var_sym_put(Type *type, int r, int v, int addr);

#pragma push(1)
typedef struct binheard{
    char signature[4];
    char version;
    char format;
    char luacData[6];
    char cintSize; 
    char sizetSize;
    char instructionSize;
    char luaIntegerSize;
    char luaNumberSize;
    int64_t luacInt;
    double luacNum;
}binheard;
#pragma push()


Type int_type;
Type folat_type;
Type string_type;
int syntax_level; 
Type default_func_type;		
int last_tok;

void expect(const char *msg);
void skip(int c);
void decl(int l);
int decl0(int l, int is_for_loop_init, void *func_sym);
int parse_byte(Type *type);
void declarator(Type *type, int *v, int *force_align);
void operand_swap();
void store0_1();

Symbol  *global_sym_stack,		
	    *local_sym_stack,		
	    *global_label_stack;
Symbol *sym_free_first;
#define SYM_POOL_NB 1024 * 4
int pools;

Symbol *sym_search(int v);
Symbol *func_sym_push(int v, Type *type);
Symbol *sym_push2(Symbol **ps, int v, int t, int c);
Symbol *sym_push(int v, Type *type, int r, int c);
void sym_pop(Symbol **ptop, Symbol *b, int keep);


void emitABC(int opcode, int a, int b, int c) ;
void emitABx(int opcode, int a, int bx) ;
void emitAsBx(int opcode, int a, int b) ;
void emitAx(int opcode, int ax);
void emitMove(int a, int b) ;
void emitLoadNil(int a, int n);
void emitLoadBool(int a, int b, int c) ;
void emitLoadK(int a , int idx) ;
void emitVararg(int a, int n) ;
void emitClosure(int a, int bx) ;
void emitSetList(int a, int b, int c) ;
void emitGetTable(int a, int b, int c) ;
void emitSetTable(int a, int b, int c) ;
void emitGetUpval(int a, int b) ;
void emitSetUpval(int a, int b) ;
void emitGetTabUp(int a, int b, int c);
void emitSetTabUp(int a, int b, int c);
void emitCall(int a, int nArgs, int nRet);
void emitTailCall(int a, int nArgs) ;
void emitReturn(int a, int n) ;
void emitSelf(int a, int b, int c);
int emitJmp(int a, int sBx);
int emitCmp(int a, int sBx);


void txgen_stream(byte_stream *text, void *ch,size_t len);
void funcinfo(byte_stream *text, funcInfo *f);

TX_FUNC tx_Complie *tx_newComplie();
TX_FUNC int tx_complie(tx_Complie *s);
TX_FUNC void build_bin(const char *filename);
#endif
