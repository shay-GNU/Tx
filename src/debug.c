#include "tx.h"


int printStack(tx_State *t) {
	int top = getTop(t);
    int i;
    printf("<-------stack start!!!------->\n");
	for (i = 1; i <= top; i++) {
		int t1 = type(t, i);
		switch(t1){
		case LUA_TBOOLEAN:
			printf("type:%s\t[%s]\n", typeName(get(t, i)->t), get(t, i)->value.b == 1 ? "true": "false");
            break;
		case LUA_TNUMBER:
			printf("type:%s\t[%d]\n", typeName(get(t, i)->t),get(t, i)->value.n);
            break;
        case LUA_TINTEGER:
			printf("type:%s\t[%d]\n", typeName(get(t, i)->t),get(t, i)->value.num);
            break;
		case LUA_TSTRING:
			printf("type:%s\t[\"%s\"]\n", typeName(get(t, i)->t),get(t, i)->value.str->str);
            break;
        case LUA_TNIL:
			printf("type:%s\t[\"%s\"]\n", typeName(get(t, i)->t),"nil");
            break;
        case LUA_TFUNCTION:
			printf("type:%s\t[\"%s\"]\n", typeName(get(t, i)->t),"function");
            break;
        case LUA_TTABLE:
            printf("type:%s\t[\"%s\"]\n", typeName(get(t, i)->t),"table");
            break;
		default: 
			printf("type:%s\t[%s]\n", typeName(get(t, i)->t),"other values");
            break;
		}
	}
	printf("<-------stack end!!!------->\n");
}

int opcodeMode(uint32_t opcode){
    int ind = getopcode(opcode);
    if(ind == OP_LOADK || ind == OP_LOADKX || ind == OP_CLOSURE){
        return IABx;
    }else if(ind == OP_JMP||ind ==OP_FORPREP||ind ==OP_FORLOOP||ind ==OP_TFORLOOP||ind ==OP_CMP){
        return IAsBx;
    }else if(ind == OP_EXTRAARG){
        return IAx;
    }else{
        return IABC;
    }
}
void print_line_code(uint32_t data){
    int mode;

    mode = opcodeMode(data);
    a=0;b=0;c=0;bx=0;
    printf("%s\t",modestr[mode]);
    printf("%*s\t",-10,codestr[getopcode(data)]);
    switch(mode){
        case IABC:
            abc(data);
            printf("%*u\t%*d\t%*d\t",-15,a,-20,b,-25,c);
            break;
        case IABx:
            abx(data);
            printf("%*d\t%*d\t",-15,a,-20,bx);
            break;
        case IAsBx:
            asbx(data);
            printf("%*d\t%*d\t",-15,a,-20,bx);
            break;
        case IAx:
            ax(data);
            printf("%*d\t",-15,a);
            break;
        default:
            vm_error("debug : print code error");
    }
}
void print_code(uint32_t *code,uint32_t i){
    uint32_t ch;
    uint32_t data;
    printf("%s\t%*s\t%*s\t%*s\t%*s","mode",-10,"opcode",-15,"a",-20,"b/Bx/sbx",-25,"c");
    putc('\n',stdout);
    for(ch=0;ch<i;ch++){
        print_line_code(code[ch]);
        putc('\n',stdout);
    }
}


