#ifndef LEX_H
#define LEX_H
#include <stdio.h>
char *keywords[] = {
    "break",
    "do",
    "else",
    "false",
    "for",
    "function",
    "goto",
    "if",
    "in",
    "var",
    "nil",
    "return",
    "true",
    "until",
    "while",
    "continue",
    "switch",
    "case",
    "default",
    NULL};

enum tx_keywords{
    TOK_BREAK = 256,
    TOK_DO,
    TOK_ELSE,
    TOK_FALSE,
    TOK_FOR,
    TOK_FUNCTION,
    TOK_GOTO,
    TOK_IF,
    TOK_IN,
    TOK_VAR,
    TOK_NIL,
    TOK_RETURN,
    TOK_TRUE,
    TOK_UNTIL,
    TOK_WHILE,
    TOK_CONTINUE,
    TOK_SWITCH,
    TOK_CASE,
    TOK_DEFAULT
};

#define TOK_EQ  0x94
#define TOK_NE  0x95
#define TOK_LT  0x9c
#define TOK_GE  0x9d
#define TOK_LE  0x9e
#define TOK_GT  0x9f

#define TOK_LAND  0xa0
#define TOK_LOR   0xa1
#define TOK_DEC   0xa2
#define TOK_INC   0xa4
#define TOK_UDIV  0xb0 
#define TOK_UMOD  0xb1 
#define TOK_PDIV  0xb2 

#define TOK_PPNUM   0xbe 
#define TOK_PPSTR   0xbf 


#define TOK_CINT    0xb5 
#define TOK_STR     0xb9 
#define TOK_LSTR    0xba
#define TOK_CDOUBLE  0xbb 

#define TOK_LINENUM 0xc0 


#define TOK_UMULL    0xc2 
#define TOK_ADDC1    0xc3 
#define TOK_ADDC2    0xc4 
#define TOK_SUBC1    0xc5 
#define TOK_SUBC2    0xc6 
#define TOK_ARROW    0xc7
#define TOK_DOTS     0xc8 
#define TOK_SHR      0xc9 
#define TOK_TWOSHARPS 0xca 
#define TOK_PLCHLDR  0xcb 
#define TOK_NOSUBST  0xcc 
#define TOK_PPJOIN   0xcd 
#define TOK_CLONG    0xce 
#define TOK_CULONG   0xcf 


#define TOK_A_MOD   0xa5
#define TOK_A_AND   0xa6
#define TOK_A_MUL   0xaa
#define TOK_A_ADD   0xab
#define TOK_A_SUB   0xad
#define TOK_A_DIV   0xaf
#define TOK_A_XOR   0xde
#define TOK_A_OR    0xfc
#define TOK_A_SHL   0x81
#define TOK_A_SAR   0x82

#define TOK_SHL     0x01  
#define TOK_SAR     0x02  


#define T_TYPE      0x000f  

#define T_INT                1       
#define T_FUNC               2       
#define T_STRING             3       
#define T_DOUBLE             4       

#define T_CFUNC              0x10       
#define T_TXFUNC             0x20       

#define    LUA_TNONE    1
#define    LUA_TNIL     2
#define    LUA_TBOOLEAN 3
#define    LUA_TLIGHTUSERDATA   4
#define    LUA_TNUMBER  5
#define    LUA_TINTEGER  6
#define    LUA_TSTRING  7
#define    LUA_TTABLE   8
#define    LUA_TFUNCTION 9
#define    LUA_TUSERDATA 10
#define    LUA_TTHREAD  11


#define T_VALMASK   0x003f  
#define T_CONST     0x0400  
#define T_LLOCAL    0x0031  
#define T_LOCAL     0x0032  
#define T_LVAL      0x0100  
#define T_SYM       0x0200  


#define	SC_ANOM	   0x1000     
#define	SC_PARAMS  0x2000     



#define TOK_HASH_INIT 1
#define TOK_HASH_FUNC(h, c) ((h) + ((h) << 5) + ((h) >> 27) + (c))

#define TOK_HASH_SIZE       16384


#endif