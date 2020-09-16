#include "tx.h"
#define funcpro(L)     TValue args = pop(L);\
    TValue res = pop(L);
int tx_print(tx_State *L){
    int i;
    int64_t argn;
    TValue args = pop(L);
    TValue res = pop(L);

    argn = args.value.num;
    for(i = 0; i< argn; i++){
        args = pop(L);
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
                
                printf("暂时不支持表table,或者直接打印出所有表?");
                break;
		}
    }
    putc('\n',stdin);
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