#ifndef TXLVM_H
#define TXLVM_H

#include <stdio.h>


enum
{
    OpArgN, 
    OpArgU, 
    OpArgR, 
    OpArgK, 
};


enum{
	IABC,         
	IABx,         
	IAsBx,        
	IAx,          
};


const char *modestr[] = {
    "IABC","IABx","IAsBx","IAx",NULL
};


enum{
    OP_MOVE,
    OP_LOADK,
    OP_LOADKX,
    OP_LOADBOOL,
    OP_LOADNIL,
    OP_GETUPVAL,
    OP_GETTABUP,
    OP_GETTABLE,
    OP_SETTABUP,
    OP_SETUPVAL,
    OP_SETTABLE,
    OP_NEWTABLE,
    OP_SELF,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_MOD,
    OP_POW,
    OP_DIV,
    OP_IDIV,
    OP_BAND,
    OP_BOR,
    OP_BXOR,
    OP_SHL,
    OP_SHR,
    OP_UNM,
    OP_BNOT,
    OP_NOT,
    OP_LEN,
    OP_CONCAT,
    OP_JMP,
    OP_CMP,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_TEST,
    OP_TESTSET,
    OP_CALL,
    OP_TAILCALL,
    OP_RETURN,
    OP_FORLOOP,
    OP_FORPREP,
    OP_TFORCALL,
    OP_TFORLOOP,
    OP_SETLIST,
    OP_CLOSURE,
    OP_VARARG,
    OP_EXTRAARG,
};

const char *codestr[] = {
    "MOVE",
    "LOADK",
    "LOADKX",
    "LOADBOOL",
    "LOADNIL",
    "GETUPVAL",
    "GETTABUP",
    "GETTABLE",
    "SETTABUP",
    "SETUPVAL",
    "SETTABLE",
    "NEWTABLE",
    "SELF",
    "ADD",
    "SUB",
    "MUL",
    "MOD",
    "POW",
    "DIV",
    "IDIV",
    "BAND",
    "BOR",
    "BXOR",
    "SHL",
    "SHR",
    "UNM",
    "BNOT",
    "NOT",
    "LEN",
    "CONCAT",
    "JMP",
    "CMP",
    "EQ",
    "NE",
    "LT",
    "LE",
    "TEST",
    "TESTSET",
    "CALL",
    "TAILCALL",
    "RETURN",
    "FORLOOP",
    "FORPREP",
    "TFORCALL",
    "TFORLOOP",
    "SETLIST",
    "CLOSURE",
    "VARARG",
    "EXTRAARG",
    NULL,
};


#define MAXARG_Bx ((1 << 18) - 1) 
#define MAXARG_sBx (MAXARG_Bx >> 1) 

int a;
int b;
int c;
int bx;

unsigned int getopcode(unsigned int code);
void abc(unsigned int code);
void abx(unsigned int code);
void asbx(unsigned int code);
void ax(unsigned int code);

void call(tx_State *s, uint32_t i);
#endif
