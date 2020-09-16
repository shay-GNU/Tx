#include "tx.h"
#include "txc.h"

int Type_size(TValue *i); 
ELF_buf *build_func(Prototype *f);

static void init_head(binheard *head){
    memset(head, 0, sizeof(binheard));

    memcpy(&head->signature, TX_Magic, strlen(TX_Magic));
    head->version=TX_VERSION;
    head->format=TX_FORMAT;
    memcpy(&head->luacData, TX_DATA, strlen(TX_DATA));
    head->cintSize=CINT_SIZE;
    head->sizetSize =CSIZET_SIZE;
    head->instructionSize = INSTRUCTION_SIZE;
    head->luaIntegerSize = LUA_INTEGER_SIZE;
    head->luaNumberSize= LUA_NUMBER_SIZE;
    head->luacInt = LUAC_INT;
    head->luacNum = LUAC_NUM;
}


TX_FUNC void build_bin(const char *filename){
    int ret, found;
    binheard head;
    FILE *fp;
    byte_stream t;

    if(!main_found)
        tx_error("symbol \'main\' not found\n");
    
    init_head(&head);
    fp = fopen(filename, "wb");
    if (!fp) 
        tx_error("'%s'�������ļ�����ʧ��", filename);
    fwrite(&head, sizeof(binheard), 1,fp);

    memset(&t, 0, sizeof(byte_stream));

    
    
    txgen_stream(&t, (&constTbale)->data, (&constTbale)->ind);

    fwrite(&constNum, 1, sizeof(int), fp);
    fwrite(constTbale.data, 1, constTbale.ind, fp);
    t.ind = 0;

    found = 0;
    fputc(funcn, fp);
    while(sec_text){
        funcinfo(&t, sec_text);
        fwrite(&t.ind, 1, 4, fp);
        fwrite(t.data, 1, t.ind, fp);

        sec_text = sec_text->next;
        t.ind = 0;
    }
    free(t.data);
    fclose(fp);
}

int Type_size(TValue *i){
    switch(i->t){
        case LUA_TNIL:      return 0;
        case LUA_TBOOLEAN:  return 1;
        case LUA_TNUMBER:   return 8;
        case LUA_TINTEGER:  return 8;
        case LUA_TSTRING:   return i->value.str->len;
    }
}

int data_prefix(TValue *i){
    switch(i->t){
        case LUA_TNIL:      return TAG_NIL;
        case LUA_TBOOLEAN:  return TAG_BOOLEAN;
        case LUA_TNUMBER:   return TAG_NUMBER;
        case LUA_TINTEGER:  return TAG_INTEGER;
        case LUA_TSTRING:   return TAG_STRING;
    }
}
void txgen_byte(byte_stream *text, uint8_t c){
    int ind1;
   
	ind1 = text->ind + 1;
    if (ind1 > text->data_allocated)
        section_realloc(text, ind1);
    text->data[text->ind] = c;
    text->ind = ind1;
}


void txgen_stream(byte_stream *text, void *ch,size_t len){
	uint8_t *c = (uint8_t *)ch;
	int i;
	for(i=0;i<len;i++){
		txgen_byte(text, c[i]);
	}
}



void funcinfo(byte_stream *text, funcInfo *f){
    if(!text)
        return;
    int64_t i = f->code->ind / 4;
    int len = strlen(f->funcname);
    txgen_byte(text, len + 1);
    txgen_stream(text, f->funcname, len);
    txgen_byte(text, f->argn);
    txgen_byte(text, f->isVararg);
    txgen_byte(text, f->maxRegs);
    txgen_stream(text, &i, 8);
    txgen_stream(text, f->code->data, i * 4);
}
