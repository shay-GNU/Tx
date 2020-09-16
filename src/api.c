#include "tx.h"


tx_Stack *realloc_stack(tx_Stack *Stack, int newSize){
    char *stack = (char *)Stack->stack;
    stack = tx_realloc(stack, newSize * sizeof(TValue));
    memset(stack + (Stack->stackSize * sizeof(TValue)), 0, (newSize - Stack->stackSize)* sizeof(TValue));  
    Stack->stackSize = newSize;
    Stack->stack = (TValue *)stack;
    return Stack;
}


int check(tx_State *t, int size){
    int len = t->stack->stackSize - t->stack->top;
    if(size > len){
        t->stack= realloc_stack(t->stack, t->stack->stackSize + size);
        return 0;
    }
    else{
        return 1;
    }
}



void push(tx_State *t,TValue i){
    if(t->stack->top >= t->stack->stackSize)
        vm_error("stack overflow!");
    else
        t->stack->stack[t->stack->top] = i;
    t->stack->top++;
}


TValue pop(tx_State *t){
    if(t->stack->top < 1)
        vm_error("stack underflow!");
    t->stack->top--;
    TValue i = (t->stack->stack[t->stack->top]);
    t->stack->stack[t->stack->top].t = LUA_TNIL;
    return i;
}



__inline int absIndex(tx_State *t, int32_t ind) {
    if (ind <= LUA_REGISTRYINDEX)
        return ind;
	if (ind >= 0){
		return ind;
	}
	return ind + t->stack->top + 1;
}


int isValid(tx_State *t, int32_t ind){
    if (ind == LUA_REGISTRYINDEX);
        return 1;
    int i = absIndex(t, ind);
    return i>0 && i<=t->stack->top;
}


TValue *get(tx_State *t, int32_t ind){
    if (ind == LUA_REGISTRYINDEX){
        return &t->registry;
    }
    int i = absIndex(t, ind);
    if(i>0 && i<=t->stack->top)
        return &(t->stack->stack[i-1]);
    else
        return NULL;
}


int set(tx_State *t, TValue value, uint32_t ind){
    int i = absIndex(t, ind);
    if(i>0 && i<=t->stack->top){
        t->stack->stack[i-1] = value;
        return 1;
    }
    else{
        vm_error("invalid index!");
    }
}


__inline int getTop(tx_State *t){
    return t->stack->top;
}


int CheckStack(tx_State *t, int n){
    check(t, n);
}



void *pops(tx_State *t, int num){
    int i;
    for(i = 0; i<num; i++){
        pop(t);
    }
}



void copy(tx_State *t,int src,int dsc){
    TValue *i = get(t, src);
    set(t, *i, dsc);
}



void pushIndex(tx_State *t,int index){
    TValue *i = get(t, index);
    push(t, *i);
}



void Replace(tx_State *t,int index){
    TValue i = pop(t);
    set(t, i, index);
}




void insert(tx_State *t,int index){
    Rotate(t,index,1);
}




int Rotate(tx_State *t,int idx, int n){
    TValue **slots = &t->stack->stack;
	int t1 = t->stack->top - 1;         
	int p = idx - 1; 			
	int m;                         
	if (n >= 0) {
		m = t1 - n;
	} else {
		m = p - n - 1;
	}
	
	
	
	
	reverse(t, p, m);   
	reverse(t, m+1, t1); 
	reverse(t, p, t1) ;  
}



int reverse(tx_State *t,int from,int to){
    TValue *slots = t->stack->stack;
	TValue i;
	for(;from < to;to--,from++){
		i = slots[from];
		slots[from] = slots[to];
		slots[to] = i;
	}
}



void setTop(tx_State *t,int index){
    int newTop = absIndex(t, index);
	if (newTop < 0) 
		vm_error("stack underflow!");

	int n = t->stack->top - newTop;
	if (n > 0) {
        int i;
		for (i = 0; i < n; i++) 
			pop(t);
	} else if (n < 0) {
        int i;
		for (i = 0; i > n; i--) 
			pushNil(t);
	}
}



void pushNil(tx_State *t){
    TValue i;
    i.t = LUA_TNIL;
    push(t, i);
}
void  pushBoolean(tx_State *t,int32_t b){
    TValue i;
    i.t = LUA_TBOOLEAN;
    i.value.b=b;
    push(t, i);
}
void pushInteger(tx_State *t,uint64_t v){
    TValue i;
    i.t = LUA_TINTEGER;
    i.value.num = v;
    push(t, i);
}
void pushNmnber(tx_State *t,double v){
    TValue i;
    i.t = LUA_TNUMBER;
    i.value.n = v;
    push(t, i);
}

void pushString(tx_State *t,char *str){
    TValue i;String *str1;int len;

    len = strlen(str);
    i.t = LUA_TSTRING;
    str1 = tx_mallocz(sizeof(String) + len);
    memcpy(str1->str, str, len);
    i.value.str = str1;
    push(t, i);
}



char* typeName(int t){
    switch(t){
        case LUA_TNONE:
            return "no value";
        case LUA_TNIL:
            return "nil";
        case LUA_TBOOLEAN:
            return "boolean";
        case LUA_TNUMBER:
            return "number";
        case LUA_TINTEGER:
            return "integer";
        case LUA_TSTRING:
            return "string";
        case LUA_TTABLE:
            return "table";
        case LUA_TFUNCTION:
            return "function";
        case LUA_TTHREAD:
            return "thread";
        default:
            return "userdata";
	}
}


#include <math.h>



int64_t IFloorDiv(int64_t a, int64_t b){
	if(a > 0 && b > 0 || a < 0 && b < 0 || (a % b == 0))
		return a / b;
	else
		return a/b - 1;
}


double FFloorDiv(double a, double b ){
	return floor(a / b);
}



int64_t IMod(int64_t a, int64_t b){
	return a - IFloorDiv(a, b) *b;
}


double FMod(double a, double b){
	
	
	
	
	
	
	return a - floor(a/b)*b;
}


int64_t ShiftLeft(int64_t a, int64_t n);
int64_t ShiftRight(int64_t a, int64_t n);

int64_t ShiftRight(int64_t a, int64_t n){
	if(n >= 0){
		return (int64_t)((uint64_t)a >> (uint64_t)n);
	} else {
		return ShiftLeft(a, -n);
	}
}

int64_t ShiftLeft(int64_t a, int64_t n){
	if(n >= 0){
		return a << (uint64_t)n;
	} else {
		return ShiftRight(a, -n);
	}
}


int64_t FloatToInteger(double f, int32_t *b){
	int64_t i = (int64_t)f;
    *b = (double)i == f;
	return i; 
}

int64_t convertTointeger(TValue val,int *b){
    int x = val.t;
    switch(x){
        case LUA_TNUMBER:
            *b = 1;
            return val.value.num;
        case LUA_TINTEGER: 
            return FloatToInteger(val.value.n,b);
        default:
            *b = 0;
            return 0;
    }
}




#define iadd(a, b) ((int64_t)((a) + (b)))
#define fadd(a, b) ((double)((a) + (b)))
#define isub(a, b) ((int64_t)((a) - (b)))
#define fsub(a, b) ((double)((a) - (b)))
#define imul(a, b) ((int64_t)((a) * (b)))
#define fmul(a, b) ((double)((a) * (b)))

#define imod IMod
#define fmod FMod




	
#define div(a, b) (a / b)
	
#define iidiv IFloorDiv
	
#define fidiv FFloorDiv


#define band(a, b) (a & b)
#define bor(a, b) (a | b)
#define bxor(a, b) (a ^ b)
	
#define shl ShiftLeft
	
#define shr ShiftRight

#define iunm(a, b) ((int64_t)(-a))
#define funm(a, b) ((double)(-a))
#define bnot(a, b) ((int64_t)(~a))


TValue _arith(TValue a, TValue b, int op,int *ok){
    *ok = 1;
    
    switch(op){
        case LUA_OPADD:{
        	int64_t i = iadd(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
        case LUA_OPSUB:{
        	int64_t i = isub(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPMUL:{
        	int64_t i = imul(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPMOD:
        {
        	int64_t i = imod(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
		}
        case LUA_OPPOW:{
        	int64_t i = pow(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPDIV: {
        	int64_t i = div(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}   
            
        case LUA_OPIDIV:{
        	int64_t i = iidiv(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPBAND:{
        	int64_t i = band(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPBOR:{
        	int64_t i = bor(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPBXOR:{
        	int64_t i = bxor(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPSHL:{
        	int64_t i = shl(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPSHR:{
        	int64_t i = shr(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPUNM:{
        	int64_t i = iunm(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
        case LUA_OPBNOT:{
        	int64_t i = bnot(geti(a),geti(b));
            TValue n;
            setivalue(&n,i);
            return n;
			break;
		}
            
    }
    *ok = 0;
    TValue n;
    setnil(&n);
    return n;
}

int Arith(tx_State *t, int op) {
    TValue a, b; 
    int ok;

    b = pop(t);
    if (op != LUA_OPUNM && op != LUA_OPBNOT){
        a = pop(t);
    }else{
        a = b;
    }
    TValue result = _arith(a, b, op, &ok);
    if(ok){
        push(t, result);
    }else{
        vm_error("arithmetic error!");
    }
}

int _ge(TValue a, TValue b){
    int x = a.t;
	switch(x){
	
    
	
	
	
	case LUA_TINTEGER:{
        int y = b.t;
		switch(y){
            case LUA_TINTEGER:
                return geti(a) >= geti(b);
            case LUA_TNUMBER:
                return (double)geti(a) >= getn(b);
		}
        vm_error("error1");
    };
	case LUA_TNUMBER:{
        int y = b.t;
		switch(y){
            case LUA_TNUMBER:
                return getn(a) >= getn(b);
            case LUA_TINTEGER:
                return getn(a) >= (double)geti(b);
		}
        vm_error("error1");
    }

	}
	vm_error("comparison error!");
}


int Compare(tx_State *t,int idxl, int idx2, int op){
    int i;

    TValue *a = get(t, idxl);
    TValue *b = get(t, idx2);
    switch(op){
        case LUA_OPEQ: return _eq(*a, *b);break;
        case LUA_OPNE: i = !_eq(*a, *b); return i;break;
        case LUA_OPLT: return _lt(*a, *b);break;
        case LUA_OPLE: return _le(*a, *b);break;
        case LUA_OPGT: return !(_lt(*a, *b));break;
        case LUA_OPGE: return _ge(*a, *b);break;
        default:
            vm_error("invalid compare op !");
    }
}


int _eq(TValue a, TValue b){
    TValue n;

    int x = a.t;
    switch(x){
        case LUA_TNIL: 
            return (b.t == LUA_TNIL);
        case LUA_TBOOLEAN:
            if(a.t == LUA_TBOOLEAN && b.t == LUA_TBOOLEAN)
                return a.value.b == b.value.b;
            else{
                n = convertToBoolean(b);
                return a.value.b == n.value.b;
            }
                vm_error("_eq type error1");
            
        case LUA_TSTRING: 
        case LUA_TINTEGER:{
        	int y = b.t;
            switch(y){ 
                case LUA_TINTEGER: return a.value.num == b.value.num;
                case LUA_TNUMBER: return (double)a.value.num == b.value.n;
                default: vm_error("_eq type error2");
            }
			break;
		} 
            
        case LUA_TNUMBER:{
        	int y = b.t;
            switch(y){
                case LUA_TNUMBER: return a.value.num == b.value.num;
                case LUA_TINTEGER: return a.value.num == (double)b.value.num ;
                default: vm_error("_eq type error3");
            }
			break;
		}
            
        default: 
            vm_error("_eq type error4");
    }
}

int _lt(TValue a, TValue b){
    int x = a.t;
	switch(x){
	case LUA_TSTRING:
		
		
		
	case LUA_TINTEGER:{
		int y = b.t;
		switch(y){
		    case LUA_TINTEGER:
			    return a.value.num < b.value.num;
		    case LUA_TNUMBER:
			    return (double)a.value.num < a.value.n;
		}
		break;
	}
        
	case LUA_TNUMBER:{
		int y = b.t;
		switch(y){
		case LUA_TNUMBER:
			return a.value.n < a.value.n;
		case LUA_TINTEGER:
			return a.value.n < (double)a.value.num;
		}
		break;
	}
        
	}
	vm_error("comparison error!");
}


int _le(TValue a, TValue b){
    int x = a.t;
	switch(x){
	
    
	
	
	
	case LUA_TINTEGER:{
        int y = b.t;
		switch(y){
            case LUA_TINTEGER:
                return geti(a) <= geti(b);
            case LUA_TNUMBER:
                return (double)geti(a) <= getn(b);
		}
        vm_error("error1");
    };
	case LUA_TNUMBER:{
        int y = b.t;
		switch(y){
            case LUA_TNUMBER:
                return getn(a) <= getn(b);
            case LUA_TINTEGER:
                return getn(a) <= (double)geti(b);
		}
        vm_error("error1");
    }

	}
	vm_error("comparison error!");
}



int Len(tx_State *t, int idx) {
	TValue *val = get(t, idx);
	if (val->t == LUA_TSTRING) {
        TValue i;
        setivalue(&i ,strlen(val->value.str->str));
		push(t, i);
	} else {
		vm_error("length error!");
	}
}





TValue ToStringX(tx_State *t,int idx,int *b) {
	TValue *val = get(t, idx);
    int x = val->t;
    *b = 1 ;
	switch(x){
	case LUA_TSTRING:
		return *val;
	case LUA_TINTEGER:{
		String *str = tx_mallocz(sizeof(String) + 15);
        itoa(geti(*val),str->str,10);
        TValue s;
        setsvalue(&s, str);
		set(t,s,idx);
		return s;
		break;
	}
        
    case LUA_TNUMBER:{
    	String *str = tx_mallocz(sizeof(String) + 15);
        gcvt(geti(*val),8,str->str);
        TValue s;
        setsvalue(&s, str);
		set(t,s,idx);
		return s;
		break;
	}
        
	default:
        *b = 0;
        TValue s;
        setnil(&s);
		return s;
	}
}

TValue ToString(tx_State *t,int idx){
    TValue n;int ok;
	n = ToStringX(t, idx, &ok);
    if(ok)
	    return n;
    else
        vm_error("string type error");
}








int IsString(tx_State *s,int idx){
	int t = type(s, idx);
	return t == LUA_TSTRING || t == LUA_TNUMBER || t == LUA_TINTEGER;
}

int Concat(tx_State *t, int n) {
	if(n == 0){
		pushString(t, "");
	} else if(n >= 2){
        int i;
		for(i = 1; i < n; i++){
			if(IsString(t, -1) && IsString(t, -2)){
				TValue s1= ToString(t,-1);
				TValue s2= ToString(t,-2);
				pop(t);
				pop(t);
                char *str = tx_mallocz(strlen(gets(s1)) + strlen(gets(s1)) + 1);
                memcpy(str, gets(s1), strlen(gets(s1)));
                memcpy(str + strlen(gets(s1)), gets(s2), strlen(gets(s2)));
				pushString(t, str);
                free(str);
				continue;
			}
			vm_error("concatenation error!");
		}
	}
	
}































































































int PushcFunction(tx_State *t,lua_CFunction f) {
    TValue n;
    setcfvalue(&n, f);
	push(t, n);
}


int PushtxFunction(tx_State *t,void* f) {
    TValue n;
    settxfvalue(&n, f);
	push(t, n);
}
