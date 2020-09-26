#ifndef TXC_H
#define TXC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


#define TX_Magic        "\x04\x18TX"
#define TX_VERSION      0x10
#define TX_FORMAT       1
#define TX_DATA         "\x20\x01\r\n\x1a\n"

#define TX_VERSIONS     "1.0.1"

#define LUA_SIGNATURE     "\x1bLua"
#define LUAC_VERSION      0x53
#define LUAC_FORMAT       0
#define LUAC_DATA         "\x19\x93\r\n\x1a\n"
#define CINT_SIZE         4
#define CSIZET_SIZE       8
#define INSTRUCTION_SIZE  4
#define LUA_INTEGER_SIZE  8
#define LUA_NUMBER_SIZE   8
#define LUAC_INT          0x5678
#define LUAC_NUM          370.5



#define TAG_NIL        0x00
#define TAG_BOOLEAN    0x01
#define TAG_NUMBER     0x02
#define TAG_INTEGER    0x03
#define TAG_STRING     0x04

#define LUA_MINSTACK  20
#define LUAI_MAXSTACK  1000000
#define LUA_REGISTRYINDEX  (-LUAI_MAXSTACK - 1000)
#define LUA_RIDX_GLOBALS (2)

#if defined(TX_DLL)
#define TX_FUNC __declspec(dllexport)
#else
#define TX_FUNC 
#endif


typedef struct String{
    int len;
    char str[1];
} String;

typedef int (*lua_CFunction) (struct tx_State *L);

typedef union Value {
  void *p;          
  char b;            
  lua_CFunction f; 
  double n;           
  String *str;
  int64_t num;
  struct{
    struct hashtable **table;
    struct hashtable *hash_next;
  };
}Value;


typedef struct tx_TValue {
  Value value;
  int t;
}TValue;


typedef struct Prototype{
    char *funcname;     
    char NumParams;    
    char IsVararg;     
    char MaxStackSize; 

    uint64_t opLen;      
    uint32_t *Code;      
    
    uint32_t conNum;      
    TValue *Constants;   
} Prototype;



#define HASH_SIZE  16384 

typedef struct hashtable {
	TValue src;
  TValue dec;
  struct hashtable *next;
  struct hashtable *link;
}hashtable;
TValue *newtable(TValue *i);

typedef struct closure {
	  Prototype *proto;
}closure;



typedef struct tx_Stack{
	TValue *stack;
    uint32_t stackSize;
    int top;
    Prototype *closure;
    TValue *varargs;

    uint32_t eip;
    struct tx_Stack *prev;
}tx_Stack;

#define STACK_SIZE 50         
#define EIP(s) (s->stack->eip)
#define AddPC(s, n) (EIP(s) += (n))

typedef struct tx_State{
    tx_Stack *stack;
    uint32_t stackSize;
    Prototype **func;
    int funcn;

    const char *filename;
    
    
    
    TValue *varargs;
    int pc;

    TValue registry;
    uint32_t conNum;      
    TValue *Constants;   
} tx_State;

const char *vmname;




#define    LUA_TYPE      0xf

#define    LUA_TNONE     1
#define    LUA_TNIL      2
#define    LUA_TBOOLEAN  3
#define    LUA_TLIGHTUSERDATA   4
#define    LUA_TNUMBER   5
#define    LUA_TINTEGER  6
#define    LUA_TSTRING   7
#define    LUA_TTABLE    8
#define    LUA_TFUNCTION 9
#define    LUA_TUSERDATA 10
#define    LUA_TTHREAD   11

#define    C_FUNCTION   (1 << 4)
#define    TX_FUNCTION  (1 << 5)

#define typecmp(x, y) ((x).t == (y).t)
enum{
	LUA_OPADD,         
	LUA_OPSUB,         
	LUA_OPMUL,         
	LUA_OPMOD,         
	LUA_OPPOW,         
	LUA_OPDIV,         
	LUA_OPIDIV,        
	LUA_OPBAND,        
	LUA_OPBOR,         
	LUA_OPBXOR,        
	LUA_OPSHL,         
	LUA_OPSHR,         
	LUA_OPUNM,         
	LUA_OPBNOT,        
};


enum{
  LUA_OPEQ, 
  LUA_OPNE, 
	LUA_OPLT,        
	LUA_OPLE,        
  LUA_OPGE,        
  LUA_OPGT        
};



#define VM_LOAD_ERROR 0x1

void vm_print_log(int level,int type,char *fmt, va_list ap);
void vm_error(char *fmt, ...);
void load_error(char *fmt, ...);


void *tx_malloc(size_t size);
void *tx_mallocz(size_t size);
void *tx_realloc(void *ptr, unsigned long size);

void pushNil(tx_State *t);
void pushBoolean(tx_State *t,int32_t b);
void pushInteger(tx_State *t,uint64_t v);
void pushNmnber(tx_State *t,double v);
void pushString(tx_State *t,char *str); 



int IsNil(tx_State *s,int idx);
int IsBoolean(tx_State *s,int idx);
int IsString(tx_State *s,int idx);
int IsNumber(tx_State *s,int idx);
int IsInteger(tx_State *s,int idx);



TValue ToNumber(tx_State *s,int idx);
TValue ToNumberX(tx_State *s,int idx,int *b);
TValue convertToFloat(TValue val, int *b);
TValue ToInteger(tx_State *s,int idx);
TValue ToIntegerX(tx_State *s,int idx,int *b);
TValue convertToInteger(TValue val, int *b);
TValue ToBoolean(tx_State *s,int idx);
TValue convertToBoolean(TValue val);



TValue *get(tx_State *t, int32_t ind);
void *pops(tx_State *t, int num);
void push(tx_State *t,TValue i);
void copy(tx_State *t,int src,int dsc);
void Replace(tx_State *t,int index);
void pushValue(tx_State *s, int idx);



TValue pop(tx_State *t);
void push(tx_State *t,TValue i);


#define geti(o) ((o).value.num)
#define getn(o) ((o).value.n)
#define getb(o) ((o).value.b)
#define gets(o) ((o).value.str->str)


#define setivalue(obj,x) \
  { TValue *io=(obj);io->value.num = (x); io->t = LUA_TINTEGER;}

#define setnvalue(obj,x) \
  { TValue *io=(obj);io->value.n = (x); io->t = LUA_TNUMBER;}

#define setnil(obj) \
  { TValue *io=(obj);io->t = LUA_TNIL;}

#define setsvalue(obj,x) \
  { TValue *io=(obj);io->value.str = (x); io->t = LUA_TSTRING;}

#define setbvalue(obj,x) \
  { TValue *io=(obj);io->value.b = (x); io->t = LUA_TBOOLEAN;}

#define settvalue(obj) \
  { TValue *io=(obj); newtable(io); io->t = LUA_TTABLE;}


#define settxfvalue(obj, x) \
  { TValue *io=(obj); io->value.p = (x); io->t = LUA_TFUNCTION | TX_FUNCTION;}

#define setcfvalue(obj, x) \
  { TValue *io=(obj); io->value.f = (x); io->t = LUA_TFUNCTION | C_FUNCTION;}

#define isfunc(n) ((((n).t & 0xf)== LUA_TFUNCTION))
#define iscfunc(n) ((((n).t & 0xf)== LUA_TFUNCTION) && ((n).t & C_FUNCTION))
#define istxfunc(n) ((((n).t & 0xf)== LUA_TFUNCTION) && ((n).t & TX_FUNCTION))

typedef struct{
    void *data;
    void *ptr;
    size_t ind;
    size_t data_all;
    int num;
}
ELF_buf;

TX_FUNC int load_bin(tx_State *s1, const char *filename);
TX_FUNC int load_main(tx_State *s, int ind);
TX_FUNC tx_State *tx_newState();

#endif
