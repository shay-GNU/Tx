#include "tx.h"
#include "vm.h"
#include "txc.h"

#ifdef _WIN32

#include <windows.h>
#include <io.h>
#include <fcntl.h>

#endif



void vm_print_log(int level,int type,char *fmt, va_list ap){
    char buf[1024];
    vsprintf(buf,fmt,ap);

    if(level & VM_LOAD_ERROR){
        printf("%s(file load error):%s","vm",buf);
        exit(1);
    }else{
        printf("%s(error):%s","vm",buf);
        exit(1);
    }
}

TX_FUNC void vm_error(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	vm_print_log(0, 0,fmt,ap);
}

void load_error(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	vm_print_log(VM_LOAD_ERROR, 0,fmt,ap);
}


void check_head(binheard *h){
    if(!strcmp(h->signature, TX_Magic))
        load_error("Magic Number error");
    if(h->version != TX_VERSION)
        load_error("version error");
    if(h->format != TX_FORMAT)
        load_error("format error");
    if(! strcmp(h->luacData, TX_DATA))
        load_error("data damage");
    if(h->cintSize != CINT_SIZE)
        load_error("int type error");
    if(h->sizetSize != CSIZET_SIZE )
        load_error("size_t type error");
    if(h->instructionSize != INSTRUCTION_SIZE )
        load_error("bytecode error");
    if(h->luaIntegerSize != LUA_INTEGER_SIZE )
        load_error("Integer error");
    if(h->luaNumberSize != LUA_NUMBER_SIZE )
        load_error("Number error");
    if(!(h->luacInt == LUAC_INT))
        load_error("little-endian and big-endian error");
    if(!(h->luacNum == LUAC_NUM))
        load_error("float format error");
}

typedef struct{
    void *data;
    size_t ind;
    size_t data_all;
}data_alloc;

void *vm_mallocz(size_t size){
    void *ptr = malloc(size);
	if(!ptr)
		vm_error("memory full (malloc)");
	memset(ptr, 0, size);
    return ptr;
}

int read_func(tx_State *s1, char *data){
    char ch, num[8];
    char *str;
    int main_found = -1;

    Prototype *func = (Prototype *)tx_mallocz(sizeof(Prototype));
    s1->func[s1->funcn++] = func;
    ch = *data++;
    ch--;
    str = tx_mallocz((size_t)ch + 1);
    memcpy(str, data, ch);
    data += ch;
    func->funcname = str;
    if(!strcmp(str, "main")) main_found = s1->funcn - 1;

    ch = *data++;
    func->NumParams = ch;

    ch = *data++;
    func->IsVararg = ch;

    ch = *data++;
    func->MaxStackSize = ch;

    uint64_t da;
    da = (*(uint64_t *)data);data += 8;
    
    func->opLen = da;

    uint32_t *c = malloc(da * 4);
    memcpy(c, data, da * 4);
    data += da * 4;
    func->Code = c;

    return main_found;
}

TX_FUNC int load_bin(tx_State *s1, const char *filename){
    void *data, *ptr;
    binheard head;
    FILE *fp;
    data_alloc f;
    int funcn = 0, i, size, n, main_found;
    double num1;
    int64_t num2;

    fp = fopen(filename, "rb");
    if(!fp) 
        vm_error("not open file");
    fread(&head, 1, sizeof(binheard), fp);
    check_head(&head);
    
{
    uint32_t da;
    fread(&da, 1, sizeof(uint32_t), fp);
    s1->Constants = (TValue *)malloc(sizeof(TValue) * da);
    int i;
    int ch;

    for (i = 0; i < da;i++){
        ch = fgetc(fp);
        s1->Constants[i].t = ch;
        switch(ch){
            case TAG_NIL:
                s1->Constants[i].t = LUA_TNIL;
                break;
            case TAG_BOOLEAN:
                ch = fgetc(fp);
                s1->Constants[i].t = LUA_TBOOLEAN;
                s1->Constants[i].value.b = ch;
                break;
            case TAG_NUMBER:
                fread(&num1, 1, sizeof(double), fp);
                s1->Constants[i].t = LUA_TNUMBER;
                s1->Constants[i].value.n = num1;
                break;
            case TAG_INTEGER:
                fread(&num2, 1, sizeof(int64_t), fp);
                s1->Constants[i].t = LUA_TINTEGER;
                s1->Constants[i].value.num = num2;
                break;
            case TAG_STRING:
                ch = fgetc(fp);
                ch--;
                s1->Constants[i].value.str = tx_mallocz(sizeof(String) + ch);
                s1->Constants[i].t = LUA_TSTRING;
                fread(s1->Constants[i].value.str->str, 1, ch, fp);
                break;
        }
    }
}

    funcn = fgetc(fp);
    s1->func = malloc(funcn * sizeof(void *));

    for(i =0; i<funcn; i++){
        memset(&f, 0 ,sizeof(data_alloc));
        fread(&size, 1, 4, fp);
        if(f.data_all < size){
            if(f.data_all == 0) f.data_all = 1024;
            while(f.data_all < size)
                f.data_all *= 2;
            f.data = tx_realloc(f.data, f.data_all);
        }
        fread(f.data, 1, size, fp);
        n = read_func(s1, f.data);
        if(n != -1) main_found = n;
    }
    free(f.data);
    fclose(fp);
    
    return main_found;
}




unsigned int getopcode(unsigned int code){
    return (code & 0x3F);
}

void abc(unsigned int code){
    a = (int)((code >> 6) & 0xFF);
    c = (int)((code >> 14) & 0x1FF);
    b = (int)((code >> 23) & 0x1FF);
}

void abx(unsigned int code){
    a = (int)((code >> 6) & 0xFF);
    bx = (int)((code >> 14));
}

void asbx(unsigned int code){
    abx(code);
    bx = bx - MAXARG_sBx;
}

void ax(unsigned int code){
    a = (code >> 6);
}




uint32_t Fetch(tx_State *s){
	uint32_t i = s->stack->closure->Code[EIP(s)];
	EIP(s)++;
	return i;
}


int GetConst(tx_State *s, int idx) {
	TValue c = s->Constants[idx];
	push(s, c);
}


int GetRK(tx_State *s, int rk) {
	if(rk > 0xFF){  
		GetConst(s, rk & 0xFF);
	} else {        
		pushValue(s, rk + 1);
	}
}


int move(tx_State *s, uint32_t i){
	abc(i);
	a += 1;
	b += 1;

	copy(s, b, a);
}

int jmp(tx_State *s, uint32_t i) {
	asbx(i);
	
    AddPC(s, bx - 1);
}

int cmp(tx_State *s, uint32_t i) {
	asbx(i);
	
    
    if(!getb(ToBoolean(s, a + 1))){
        
        AddPC(s, bx - 1);
    }
}


int loadNil(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;

	pushNil(s);
    int n;
	for(n = a; n <= a+b; n++){
		copy(s, -1, i);
	}
	pops(s,1);
}


int loadBool(tx_State *s, uint32_t i){
	abc(i);
	a += 1;

	pushBoolean(s, b != 0);
	Replace(s, a);

	if(c != 0)
		AddPC(s, 1);
}


int loadK(tx_State *s, uint32_t i){
	abx(i);
	a += 1;
	GetConst(s, bx);
	Replace(s, a);
}


int loadKx(tx_State *s, uint32_t i){
	abx(i);
	a += 1;
    int a1 = a;
    ax((uint32_t)Fetch(s));

	
	GetConst(s, a);
	Replace(s, a1);
}

void pushValue(tx_State *s, int idx) {
	TValue *val = get(s, idx);
	push(s, *val);
}

int _binaryArith(tx_State *s, uint32_t i, int op) {
	abc(i);
	a += 1;

	GetRK(s, b);
	GetRK(s, c);
	Arith(s, op);
	Replace(s, a);
}

int _unaryArith(tx_State *s, uint32_t i, int op) {
	abc(i);
	a += 1;
	b += 1;

	pushValue(s, b);
	Arith(s, op);
	Replace(s, a);
}


__inline int add(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPADD); }  
__inline int sub(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPSUB); }  
__inline int mul(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPMUL); }  
__inline int mod(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPMOD); }  
__inline int tpow(tx_State *s, uint32_t i) { _binaryArith(s, i, LUA_OPPOW); }  
__inline int tdiv(tx_State *s, uint32_t i) { _binaryArith(s, i, LUA_OPDIV); }  
__inline int idiv(tx_State *s, uint32_t i) { _binaryArith(s, i, LUA_OPIDIV); } 
__inline int band(tx_State *s, uint32_t i) { _binaryArith(s, i, LUA_OPBAND); } 
__inline int bor(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPBOR); }  
__inline int bxor(tx_State *s, uint32_t i) { _binaryArith(s, i, LUA_OPBXOR); } 
__inline int shl(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPSHL); }  
__inline int shr(tx_State *s, uint32_t i)  { _binaryArith(s, i, LUA_OPSHR); }  
__inline int unm(tx_State *s, uint32_t i)  { _unaryArith(s, i, LUA_OPUNM); }   
__inline int bnot(tx_State *s, uint32_t i) { _unaryArith(s, i, LUA_OPBNOT); }  

int length(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;
	b += 1;

	Len(s, b);
	Replace(s,a);
}


int concat(tx_State *s, uint32_t i){
	abc(i);
	a += 1;
	b += 1;
	c += 1;

	int n = c - b + 1;
	CheckStack(s, n);
    int n1;
	for(n1 = b; n1 <= c; n1++){
		pushValue(s, i);
	}
	Concat(s, n);
	Replace(s, a);
}



int _compare(tx_State *s, uint32_t i, int op) {
	abc(i);
	a += 1;

	GetRK(s, b);
	GetRK(s, c);
    pushBoolean(s, Compare(s, -2, -1, op));
    Replace(s, a);
	pops(s, 2);
}



__inline int ne(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPNE); } 
__inline int eq(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPEQ); } 
__inline int lt(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPLT); } 
__inline int le(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPLE); } 
__inline int gt(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPGT); } 
__inline int ge(tx_State *s, uint32_t i) { _compare(s, i, LUA_OPGE); } 
int not(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;
	b += 1;

	pushBoolean(s, !getb(ToBoolean(s, b)));
	Replace(s, a);
}


int testSet(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;
	b += 1;

	if(getb(ToBoolean(s,b)) == (c != 0) ){
		copy(s, b, a);
	} else {
		AddPC(s, 1);
	}
}



int test(tx_State *s, uint32_t i) {
	abc(i);

	a += 1;

	if(getb(ToBoolean(s, a)) != (c != 0)){
		AddPC(s, 1);
	}
}


int forPrep(tx_State *s, uint32_t i) {
	asbx(i);
	a += 1;

	if (type(s, a) == LUA_TSTRING) {
		push(s,ToNumber(s, a));
		Replace(s, a);
	}
	if(type(s, a+1) == LUA_TSTRING){
		push(s, ToNumber(s, a + 1));
		Replace(s, a + 1);
	}
	if(type(s, a+2) == LUA_TSTRING){
		push(s, ToNumber(s, a + 2));
		Replace(s, a + 2);
	}

	pushValue(s, a);
	pushValue(s, a + 2);
	Arith(s, LUA_OPSUB);
    Replace(s, a);
	AddPC(s, bx);
}






int forLoop(tx_State *s, uint32_t i) {
	asbx(i);
	a += 1;

	
	pushValue(s, a + 2);
	pushValue(s, a);
	Arith(s, LUA_OPADD);
	Replace(s, a);

	int isPositiveStep = getn(ToNumber(s, a+2)) >= 0;
	if(isPositiveStep && Compare(s, a, a+1, LUA_OPLE) ||
		!isPositiveStep && Compare(s, a+1, a, LUA_OPLE)){
		
		AddPC(s, bx);
		copy(s, a, a+3);
	}
}


int Execute(tx_State *s, uint32_t i){
    switch(getopcode(i)){
        case (OP_MOVE):
            move(s, i);
            break;
        case (OP_LOADK):
            loadK(s, i);
            break;
        case (OP_LOADKX):
            loadKx(s, i);
            break;
        case (OP_LOADBOOL):
            loadBool(s, i);
            break;
        case (OP_LOADNIL):
            loadNil(s, i);
            break;
        case (OP_GETUPVAL):
            break;
        case (OP_GETTABUP):
            getTabUp(s, i);
            break;
        
        
        
        
        
        
        
    
        
        
        
        
        
        
        
        case (OP_ADD):
            add(s, i);
            break;
        case (OP_SUB):
            sub(s, i);
            break;
        case (OP_MUL):
            mul(s, i);
            break;
        case (OP_MOD):
            mod(s, i);
            break;
        case (OP_POW):
            tpow(s, i);
            break;
        case (OP_DIV):
            tdiv(s, i);
            break;
        case (OP_IDIV):
            idiv(s, i);
            break;
        case (OP_BAND):
            band(s, i);
            break;
        case (OP_BOR):
            bor(s, i);
            break;
        case (OP_BXOR):
            bxor(s, i);
            break;
        case (OP_SHL):
            shl(s, i);
            break;
        case (OP_SHR):
            shr(s, i);
            break;
        case (OP_UNM):
            break;
        case (OP_BNOT):
            bnot(s, i);
            break;
        case (OP_NOT):
            not(s, i);
            break;
        case (OP_LEN):
            Len(s, i);
            break;
        case (OP_CONCAT):
            concat(s, i);
            break;
        case (OP_JMP):
            jmp(s, i);
            break;
        case (OP_CMP):
            cmp(s, i);
            break;
        case (OP_EQ):
            eq(s, i);
            break;
        case (OP_NE):
            ne(s, i);
            break;
        case (OP_LT):
            lt(s, i);
            break;
        case (OP_LE):
            le(s, i);
            break;
         case (OP_GT):
            gt(s, i);
            break;
         case (OP_GE):
            ge(s, i);
            break;
        case (OP_TEST):
            test(s, i);
            break;
        case (OP_TESTSET):
            testSet(s, i);
            break;
        case (OP_CALL):
            call(s, i);
            break;
        
        
        case (OP_RETURN):
            break;
        case (OP_FORLOOP):
            forLoop(s, i);
            break;
        case (OP_FORPREP):
            forPrep(s, i);
            break;
        
        
        
        
        
        
        case (OP_CLOSURE):
            CLOSURE(s, i);
            break;
        
        
        
        
        default:
            vm_error("error bytecode\n");
    }
}


int IsNil(tx_State *s,int idx){
	return type(s, idx) == LUA_TNIL;
}






int IsBoolean(tx_State *s,int idx){
	return type(s, idx) == LUA_TBOOLEAN;
}

int IsTable(tx_State *s, int idx ){
	return type(s, idx) == LUA_TTABLE;
}


int IsFunction(tx_State *s, int idx ) {
	return type(s, idx) == LUA_TTABLE;
}


int IsThread(tx_State *s, int idx ){
	return type(s, idx) == LUA_TTHREAD;
}


int IsNumber(tx_State *s,int idx){
    int ok;
	ToNumberX(s, idx, &ok);
	return ok;
}


int IsInteger(tx_State *s,int idx){
	TValue *n = get(s, idx);;
	if(n->t == LUA_TINTEGER)
	    return 1;
    else
        return 0;
}


TValue convertToBoolean(TValue val){
    int x = val.t;
    TValue n;
	switch(x){
        case LUA_TNIL:
            setbvalue(&n, 0);
            return n;
        case LUA_TBOOLEAN:
            return val;
        default:
            setbvalue(&n, 1);
            return n;
	}
}
TValue ToBoolean(tx_State *s,int idx){
	TValue *n = get(s, idx);
	return convertToBoolean(*n);
}


TValue convertToInteger(TValue val, int *b){
    int x = val.t;
    *b = 1;
    TValue n;
	switch(x){
        case LUA_TINTEGER:
            return val;
        case LUA_TNUMBER:
            setivalue(&n, (int64_t)geti(val));
            return n;
        case LUA_TSTRING:
            vm_error("��ʱ����������ת�ַ���");
        default:
            setnil(&n);
            *b=0;
            return n;
	}
}
TValue ToIntegerX(tx_State *s,int idx,int *b){
	TValue *n = get(s, idx);
	return convertToInteger(*n, b);
}
TValue ToInteger(tx_State *s,int idx){
    int ok;
	TValue n = ToIntegerX(s, idx, &ok);
    if(ok)
	    return n;
    else
        vm_error("int type error,��֪������ô����");
}


TValue convertToFloat(TValue val, int *b){
    int x = val.t;
    *b = 1;
    TValue n;
	switch(x){
        case LUA_TINTEGER:
            setnvalue(&n, (double)geti(val));
            return n;
        case LUA_TNUMBER:
            return val;
        case LUA_TSTRING:
            vm_error("��ʱ������С��ת�ַ���");
        default:
            setnil(&n);
            *b=0;
            return n;
	}
}

TValue ToNumberX(tx_State *s,int idx,int *b){
	TValue *n = get(s, idx);
	return convertToFloat(*n, b);
}

TValue ToNumber(tx_State *s,int idx){
    int ok;
	TValue n = ToNumberX(s, idx,&ok);
    if(ok)
	    return n;
    else
        vm_error("double type error,��֪������ô����");
}




int pushTxStack(tx_State *s, tx_Stack *i) {
    i->prev = s->stack;
    s->stack = i;
}

int popTxStack(tx_State *s) {
    tx_Stack *i;

    if(s->stack->prev){
        i = s->stack->prev;
        s->stack = i;
    }else{
        vm_error("function call stack��ֵ\n");
    }
}










void newTable(tx_State *s, uint32_t i){
    abc(i);
    a += 1;
    createTable(s);
    Replace(s, a);
}


void getTable(tx_State *s, uint32_t i){
    abc(i);
    a += 1;b += 1;
    GetRK(s, c);
    GetTable(s, b);
    Replace(s, a);
}


TValue *popN(tx_State *s, int n){
    int i;
    TValue *vals = tx_mallocz(n * sizeof(TValue));
    
	for(i = n - 1; i >= 0; i--){
		vals[i] = pop(s);
	}
    return vals;
}

int pushN(tx_State *s, TValue *vals, int valn, int n) {
	int nVals = valn;
	if(n < 0)
		n = nVals;

    int i;TValue n1;
    setnil(&n1);
	for(i = 0; i < n; i++){
		if(i < nVals){
			push(s, vals[i]);
		} else {
			push(s, n1);
		}
	}
}

int CLOSURE(tx_State *s, uint32_t i){
    abx(i);
    a += 1;
    PushtxFunction(s, s->func[s->funcn-1-bx]);
    Replace(s, a);
}



int callTxClosure(tx_State *s, int nArgs, int nResults, TValue c) {
    Prototype *func = (Prototype *)(c.value.p);
	uint32_t nRegs = func->MaxStackSize;
	uint32_t nParams = func->NumParams;
	uint32_t isVararg = func->IsVararg;

	
	tx_Stack *stack = tx_newStack(0);
	stack->closure = func;

    tx_State s1;
    memset(&s1, 0, sizeof(tx_State));
    s1.stack = stack;

	
	TValue *funcAndArgs = popN(s, nArgs + 1);
	pushN(&s1, &funcAndArgs[1], nArgs + 1, nParams);
    
    if(stack->top < nRegs)
	    stack->top = nRegs;
	if(nArgs > nParams && isVararg){
        stack->varargs = &funcAndArgs[nParams+1];
	}

	pushTxStack(s, stack);
	exe(s);
	popTxStack(s);
	
	if(nResults != 0){
		TValue *results = popN(&s1, stack->top - nRegs);
		check(s, stack->top - nRegs);
		pushN(s, results, stack->top - nRegs,  nResults);
        free(results);
	}
    
    free(funcAndArgs);
    tx_freeStack(stack);
}


int callCClosure(tx_State *s, int nArgs, int nResults , TValue c) {
	
	
    
	

	
    
    
	
	
	
	
	

	
	
	
	

	
	
	
	
	
	
}


void _fixStack(tx_State *s, int a) {
	int64_t x = ToInteger(s, -1).value.num;
	pops(s, 1);

	CheckStack(s, x - a);
    int i;
	for(i = a; i < x; i++){
		pushValue(s, i);
	}
	Rotate(s, s->stack->closure->MaxStackSize+1, x-a);
}



void _popResults(tx_State *s, int a, int c) {
	if(c == 1){
		
	} else if(c > 1){
        int i;
		for(i = a + c - 2; i >= a; i--){
			Replace(s, i);
		}
	} else {
		
		CheckStack(s, 1);
		pushInteger(s, a);
	}
}



int _pushFuncAndArgs(tx_State *s, int a, int b){
    int i, n;
	if(b >= 1){
		CheckStack(s, b);
		for(i = a - b; i <= a; i++)
			pushValue(s, i);

		return b;
	} else {
		_fixStack(s, a);
		return getTop(s) - s->stack->closure->MaxStackSize - 1;
	}
}

void call(tx_State *s, uint32_t i){
    abc(i);
    a += 1 ;

    uint32_t nArgs = _pushFuncAndArgs(s, a, b);
    
    Call(s, b, c);
    _popResults(s, a, c);
}


void _return(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;

	if(b == 1){
		
	} else if(b > 1){
		
		CheckStack(s, b - 1);
        int i;
		for(i = a; i <= a+b-2; i++)
			pushValue(s, i);
	} else {
		_fixStack(s, a);
	}
}


void LoadVararg(tx_State *s, int varargn, int n) {
	if( n < 0 ){
		n = varargn;
	}

	check(s, n);
	pushN(s, s->stack->varargs, varargn, n);
}

void vararg(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;

	if(b != 1){ 
		
		
	}
}


void tailCall(tx_State *s, uint32_t i) {
	abc(i);
	a += 1;

	
	c = 0;
	int nArgs = _pushFuncAndArgs(s, a, b);
	Call(s, nArgs, c-1);
	_popResults(s, a, c);
}


void self(tx_State *s, uint32_t i) {
	
	
	

	
	
	
	
}


int PushGlobalTable(tx_State *s) {
	TValue *global = get(s, LUA_REGISTRYINDEX);
	push(s, *global);
}


int GetGlobal(tx_State *s, const char *name){
	TValue *t = get(s, LUA_REGISTRYINDEX);

    TValue str;
    String *str1 = tx_mallocz(sizeof(String) + strlen(name));
    memcpy(str1->str, name, strlen(name));
    setsvalue(&str, str1);
    TValue n = hash_get(t,str)->src;
	push(s, n);
}


int SetGlobal(tx_State *s, const char *name) {

	TValue *t = get(s, LUA_REGISTRYINDEX);

	TValue v = pop(s);
    TValue str;
    String *str1 = tx_mallocz(sizeof(String) + strlen(name));
    memcpy(str1->str, name, strlen(name));
    setsvalue(&str, str1);
    hash_put(t, str ,v);
}

#define funcpro(L)     TValue args = pop(L);\
    TValue res = pop(L);

void print_value(TValue args){
    switch(args.t){
        case LUA_TBOOLEAN:
            printf("%s\t", args.value.b ? "true": "false");
            break;
        case LUA_TNUMBER:
            printf("%lf\t", args.value.n);
            break;
        case LUA_TINTEGER:
            printf("%lld\t", args.value.num);
            break;
        case LUA_TSTRING:
            printf("%s\t", args.value.str->str);
            break;
        case LUA_TNIL:
            printf("nil\t");
            break;
        case LUA_TTABLE:
            
            printf("��ʱ��֧�ֱ�table,����ֱ�Ӵ�ӡ�����б�?");
            break;
    }
}
int tx_print(tx_State *L){
    int i;
    int64_t argn;
    TValue args = pop(L);
    TValue res = pop(L);

    argn = args.value.num;
    for(i = 0; i< argn; i++){
        args = pop(L);
        print_value(args);
    }
    putc('\n',stdout);
};

int tx_getin(tx_State *L){
    char buf[1024];
    TValue i;

    memset(buf,0,1024);
    uint32_t n = scanf("%s",buf);
    if(n > 1024){
        setnil(&i);
        push(L, i);
        return 1;
    }else{
        
        push(L, i);
    }
    
};

int tx_type(tx_State *t){
    funcpro(t);
    
}

int tx_exit(tx_State *t){
    exit(0);
}

int tx_print(tx_State *L);
int tx_getin(tx_State *L);
int tx_type(tx_State *t);
int tx_exit(tx_State *t);
typedef struct reg_func{
    lua_CFunction f;
    char *str;
}reg_func;
static const reg_func registerfunc[]={
    {(lua_CFunction)tx_print, "print"},
    {(lua_CFunction)tx_type, "type"},
    {(lua_CFunction)tx_exit, "exit"},
    
    NULL
};


int Register(tx_State *s, const char *name, lua_CFunction f) {
	PushcFunction(s, f);
	SetGlobal(s, name);
}



int getTabUp(tx_State *s, uint32_t i){
	abc(i);
	a += 1;
    b += 1;
	GetRK(s, c);
	GetTable(s, LUA_REGISTRYINDEX);
	Replace(s, a);
	
}



void Closure(tx_State *s, uint32_t i){
    
    
    
    
}
