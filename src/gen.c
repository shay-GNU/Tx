#include "txc.h"
#include "tx.h"
#include "vm.h"
int gen_const(int i);
void gen_stream(void *ch,size_t len, int cmp);
#define TEXT_CMP 0x01
#define DATA_CMP 0x02

void section_realloc(byte_stream *sec, int new_size){
    int size;
    char *data;
    
    size = sec->data_allocated;
    if(size == 0)
    	size = 1024;
    while (size < new_size)
        size = size * 2;
    data = realloc(sec->data, size);
    if (!data)
        exit(1);
    memset(data + sec->data_allocated, 0, size - sec->data_allocated); 
    sec->data = data;
    sec->data_allocated = size;
}

void gen_byte(uint8_t c,int i){
    int ind1;
	byte_stream *text;
   
	if(i & 0x01)
		text = sec_text->code;
	else
		text = &constTbale;
	 ind1 = text->ind + 1;
    if (ind1 > text->data_allocated)
        section_realloc(text, ind1);
    text->data[text->ind] = c;
    text->ind = ind1;
}


void gen_stream(void *ch,size_t len, int cmp){
	uint8_t *c = (uint8_t *)ch;
	int i;
	for(i=0;i<len;i++){
		gen_byte(c[i], cmp);
	}
}

int gen_str_const(const char* str){
	gen_byte(TAG_STRING, DATA_CMP);
	gen_byte(strlen(str) + 1, DATA_CMP);
	gen_stream((void *)str, strlen(str), DATA_CMP);
	int n = constNum;
	constNum++;
	return n;
}


int gen_const(int i){
	uint32_t n;
	
	switch(i){
		case LUA_TNIL:
			gen_byte(TAG_NIL, DATA_CMP);
			break;
		case LUA_TBOOLEAN:
			gen_byte(TAG_BOOLEAN, DATA_CMP);
			gen_stream(&tokc.b, 1, DATA_CMP);
			break;
		case LUA_TNUMBER:
			gen_byte(TAG_NUMBER, DATA_CMP);
			gen_stream(&tokc.d, 8, DATA_CMP);
			break;
		case LUA_TINTEGER:
			gen_byte(TAG_INTEGER, DATA_CMP);
			gen_stream(&tokc.i, 8, DATA_CMP);
			break;
		case LUA_TSTRING:
			gen_byte(TAG_STRING, DATA_CMP);
			gen_byte(strlen((char*)tokc.str.data) + 1, DATA_CMP);
			gen_stream(tokc.str.data, strlen((char*)tokc.str.data), DATA_CMP);
			break;
	}
	n = constNum;
	constNum++;
	return n;
}

void emitABC(int opcode, int a, int b, int c) {
	uint32_t i = b<<23 | c<<14 | a<<6 | opcode;
	gen_stream(&i, sizeof(uint32_t), TEXT_CMP);
}
void emitABx(int opcode, int a, int bx) {
	
	uint32_t i = bx <<14 | a<<6 | opcode;
	gen_stream(&i, sizeof(uint32_t), TEXT_CMP);
}

void emitAsBx(int opcode, int a, int b) {
	uint32_t i = (b + MAXARG_sBx)<<14 | a<<6 | opcode;
	gen_stream(&i, sizeof(uint32_t), TEXT_CMP);
}

void emitAx(int opcode, int ax) {
	uint32_t i = ax<<6 | opcode;
	gen_stream(&i, sizeof(uint32_t), TEXT_CMP);
}



void emitMove(int a, int b) {
	emitABC(OP_MOVE, a, b, 0);
}
void  emitLoadNil(int a, int n) {
	emitABC(OP_LOADNIL, a, n-1, 0);
}
void  emitLoadBool(int a, int b, int c) {
	emitABC(OP_LOADBOOL, a, b, c);
}



void  emitLoadK(int a , int idx) {
	if(idx < (1 << 18)){
		emitABx(OP_LOADK, a, idx);
	} else {
		emitABx(OP_LOADKX, a, 0);
		emitAx(OP_EXTRAARG, idx);
	}
}


void emitVararg(int a, int n) {
	emitABC(OP_VARARG, a, n+1, 0);
}


void emitClosure(int a, int bx) {
	emitABx(OP_CLOSURE, a, bx);
}


void emitSetList(int a, int b, int c) {
	emitABC(OP_SETLIST, a, b, c);
}


void emitGetTable(int a, int b, int c) {
	emitABC(OP_GETTABLE, a, b, c);
}


void emitSetTable(int a, int b, int c) {
	emitABC(OP_SETTABLE, a, b, c);
}


void emitGetUpval(int a, int b) {
	emitABC(OP_GETUPVAL, a, b, 0);
}


void emitSetUpval(int a, int b) {
	emitABC(OP_SETUPVAL, a, b, 0);
}


void emitGetTabUp(int a, int b, int c){
	emitABC(OP_GETTABUP, a, b, c);
}


void emitSetTabUp(int a, int b, int c){
	emitABC(OP_SETTABUP, a, b, c);
}



void emitCall(int a, int nArgs, int nRet){
	emitABC(OP_CALL, a, nArgs, nRet);
}


void emitTailCall(int a, int nArgs) {
	emitABC(OP_TAILCALL, a, nArgs+1, 0);
}


void emitReturn(int a, int n) {
	emitABC(OP_RETURN, a, n+1, 0);
}


void emitSelf(int a, int b, int c) {
	emitABC(OP_SELF, a, b, c);
}


int emitJmp(int a, int sBx){
	int addr = sec_text->code->ind / 4;
	emitAsBx(OP_JMP, a, sBx);
	int32_t *ptr = (int32_t *)(sec_text->code->data + sec_text->code->ind);
	int32_t n = *ptr; 
	n = (n >> 6) << 6;
	return addr;
}


int emitCmp(int a, int sBx){
	int addr = sec_text->code->ind / 4;
	emitAsBx(OP_CMP, a, sBx);
	return addr;
}







void emitTestSet(int a, int b, int c) {
	emitABC(OP_TESTSET, a, b, c);
}














void emitTForCall(int a, int c) {
	emitABC(OP_TFORCALL, a, 0, c);
}

int emitTForLoop(int a, int sBx) {
	emitAsBx(OP_TFORLOOP, a, sBx);
}








































