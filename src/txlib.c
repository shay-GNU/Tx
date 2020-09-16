#include "tx.h"
#include "vm.h"
static inline uint16_t read16le(unsigned char *p) {
    return p[0] | (uint16_t)p[1] << 8;
}
static inline uint32_t read32le(unsigned char *p) {
  return read16le(p) | (uint32_t)read16le(p + 2) << 16;
}
static inline uint64_t read64le(unsigned char *p) {
  return read32le(p) | (uint64_t)read32le(p + 4) << 32;
}


void *tx_realloc(void *ptr, unsigned long size){
    void *ptr1;
    ptr1 = realloc(ptr, size);
    if (!ptr1 && size)
        vm_error("memory full (realloc)");
    return ptr1;
}

void *tx_malloc(size_t size){
	void *ptr = malloc(size);
	if(!ptr)
		vm_error("memory full (malloc)");
    return ptr;
}

void *tx_mallocz(size_t size){
    void *ptr = malloc(size);
	if(!ptr)
		vm_error("memory full (malloc)");
	memset(ptr, 0, size);
    return ptr;
}



tx_Stack *tx_newStack(uint32_t size){
    tx_Stack *s = (tx_Stack *)tx_mallocz(sizeof(tx_Stack));
    if(!size)   size = STACK_SIZE;
    s->stack = (TValue *)tx_mallocz(sizeof(TValue) * size);
    s->stackSize = size;
    
    return s;
}

int tx_freeStack(tx_Stack *i){
    free(i->stack);
    free(i);
}





#define TOK_HASH_FUNC(h, c) ((h) + ((h) << 5) + ((h) >> 27) + (c))

TValue *newtable(TValue *i){
    if(!i)
        vm_error(" null table");
    i->value.table = tx_mallocz(HASH_SIZE * sizeof(void *));
}

hashtable *hash_get(TValue *s,TValue key){
    int i = key.t;
    uint32_t h;
    hashtable **t, *t1;
    char *p;
    if(!s && s->value.table)
        vm_error("get table is null");
    switch(i){
        case LUA_TBOOLEAN: 
            h = TOK_HASH_FUNC(1, key.value.b);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.b == key.value.b)
                    goto token_found;
                t = &(t1->next);
            }
            break;
        case LUA_TNUMBER:
            h = TOK_HASH_FUNC(1, (int)key.value.n);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.n == key.value.n)
                    goto token_found;
                t = &(t1->next);
            }
            break;
        case LUA_TINTEGER:
            h = TOK_HASH_FUNC(1, key.value.num);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.num == key.value.num)
                    goto token_found;
                t = &(t1->next);
            }
            break;
        case LUA_TSTRING:
            p = key.value.str->str;
            int ch = *p++;
            h = TOK_HASH_FUNC(1, ch);
            while (ch = *p++, ch != '\0')
            	h = TOK_HASH_FUNC(h, ch);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && !strcmp(t1->dec.value.str->str,key.value.str->str))
                    goto token_found;
                t = &(t1->next);
            }
            break;
    }
    token_found:
    return t1;
} 

int hash_put(TValue *s,TValue key,TValue val){
    int i = key.t;
    uint32_t h;
    hashtable **t, *t1, *t2;
    char *p;
    if(!s && !s->value.table)
        vm_error("get table is null");
    switch(i){
        case LUA_TBOOLEAN:
            h = TOK_HASH_FUNC(1, key.value.b);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.b == key.value.b)
                    goto token_found;
                t = &(t1->next);
            }
            *t = tx_mallocz(sizeof(hashtable));
            (*t)->dec = key;
            break;
        case LUA_TNUMBER:
            h = TOK_HASH_FUNC(1, (int)key.value.n);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.n == key.value.n)
                    goto token_found;
                t = &(t1->next);
            }
            *t = tx_mallocz(sizeof(hashtable));
            (*t)->dec = key;
            break;
        case LUA_TINTEGER:
            h = TOK_HASH_FUNC(1, key.value.num);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && t1->dec.value.num == key.value.num)
                    goto token_found;
                t = &(t1->next);
            }
            *t = tx_mallocz(sizeof(hashtable));
            (*t)->dec = key;
            break;
        case LUA_TSTRING:
            p = key.value.str->str;
            int ch = *p++;
            h = TOK_HASH_FUNC(1, ch);
            while (ch = *p++, ch != '\0')
            	h = TOK_HASH_FUNC(h, ch);
            h &= (HASH_SIZE- 1);
            t = &s->value.table[h];
            while(1){
                t1 = *t;
                if (!t1)
                    break;
                if (typecmp(t1->dec, key) && !strcmp(t1->dec.value.str->str,key.value.str->str))
                    goto token_found;
                t = &(t1->next);
            }
            *t = tx_mallocz(sizeof(hashtable));
            (*t)->dec = key;
            break;
    }
    token_found:
    if(!s->value.hash_next)
        s->value.hash_next = *t;
    else{
        (*t)->link = s->value.hash_next;
        s->value.hash_next = (*t);
    }
    (*t)->src = val;
}

void createTable(tx_State *s){
    TValue t1,t2,table;
    settvalue(&table);
    push(s, table);
}



int GetTable(tx_State *s, int idx){
    TValue *t = get(s, idx);
    TValue t1 = hash_get(t, pop(s))->src;
    push(s, t1);
    return t1.t;
}


int SetTable(tx_State *s, int idx){
    TValue *t = get(s, idx);
    TValue t1 = pop(s);
    TValue t2 = pop(s);
    hash_put(t,t1 ,t2);
}



int type(tx_State *t, int ind){
    int i =  isValid (t, ind);
    if(isValid (t, ind)){
        return get(t,ind)->t;
    }else{
        return LUA_TNONE;
    }
}

int Call(tx_State *s, int nArgs, int nResults) {
    TValue f = pop(s);
    if(isfunc(f)){
        if(iscfunc(f)){
            pushInteger(s, nResults);
            pushInteger(s, nArgs);
            (lua_CFunction)f.value.f(s);
        }else if(istxfunc(f)){
            callTxClosure(s, nArgs, nResults, f);
        }
    }else{
        vm_error("not function");
    }
}
int exe_main(tx_State *t){
    int i;

    
    check(t, t->stack->closure->MaxStackSize);
    for(i=0;i < t->stack->closure->MaxStackSize;i++){
        pushNil(t);
    }

    for(i=0;EIP(t) < t->func[t->funcn]->opLen;EIP(t)++){
        if(getopcode(t->stack->closure->Code[EIP(t)]) == OP_RETURN)
            break;
        Execute(t, t->func[t->funcn]->Code[EIP(t)]);
    }
}
int exe(tx_State *t){
    int i;
    for(i=0;EIP(t) < t->stack->closure->opLen;EIP(t)++){
        if(getopcode(t->stack->closure->Code[EIP(t)]) == OP_RETURN)
            break;
        Execute(t, t->stack->closure->Code[EIP(t)]);
    }
}
TX_FUNC int load_main(tx_State *s, int ind){
    int i;

    s->stack->closure = s->func[ind];
    s->stack->prev = NULL;
    check(s, s->stack->closure->MaxStackSize);
    for(i=0;i < s->stack->closure->MaxStackSize;i++){
        pushNil(s);
    }
    exe(s);
}
#include "vm.c"
#include "api.c"
#include "debug.c"

TX_FUNC tx_State *tx_newState(){
    tx_State *s = (tx_State *)tx_mallocz(sizeof(tx_State));
    s->stack = tx_newStack(0);
    newtable(&s->registry);
    
    PushcFunction(s, (lua_CFunction)tx_print);
    SetGlobal(s, registerfunc[0].str);

    PushcFunction(s, (lua_CFunction)tx_exit);
    SetGlobal(s, registerfunc[2].str);
    return s;
}
