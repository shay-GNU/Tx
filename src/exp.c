#include "txc.h"
#include "lex.h"

Type char_pointer_type, int_type,default_func_type,str_type;
void assignment_expression();
void equality_expression();
void relational_expression();
void additive_expression();
void multiplicative_expression();
void unary_expression();
void postfix_expression();
void primary_expression();
void expression_statement();
void gen_invoke(int nb_args);
void argument_expression_list();
void expr_or(void);
void expr_xor(void);
void expr_and(void);
void gen_op(int op); 

void operand_pop(){
	optop--;
}

void expression(){
    while (1) {
        assignment_expression();
        if (tok != ',')
            break;
        operand_pop();
        next();
    }
}


void check_lvalue()
{
   if(!(optop->r & T_LVAL))
	   expect("��ֵ");
} 



void assignment_expression(){
	Symbol *s;

	int t = tok;
    expr_or();
    if (tok == '=') {
        check_lvalue();
        next();
		
		s = sym_search(t);
		optop[0].value = s->c;
        assignment_expression();
        store0_1();
    }
}








static void expr_cmpeq(void){
    int t;

    relational_expression();
    while (tok == TOK_EQ || tok == TOK_NE) {
        t = tok;
        next();
        relational_expression();
        gen_op(t);
    }
}

void expr_and(void){
    expr_cmpeq();;
    while (tok == '&') {
        next();
        expr_cmpeq();
        gen_op('&');
    }
}

void expr_xor(void){
    expr_and();
    while (tok == '^') {
        next();
        expr_and();
        gen_op('^');
    }
}

void expr_or(void){
    expr_xor();
    while (tok == '|') {
        next();
        expr_xor();
        gen_op('|');
    }
}


void equality_expression()
{
    int t;
    relational_expression();
    while (tok == TOK_EQ || tok == TOK_NE) 
	{
        t = tok;
        next();
        relational_expression();
        gen_op(t);
    }
}

void expr_shift(void){
    int t;

    additive_expression();
    while (tok == TOK_SHL || tok == TOK_SAR) {
        t = tok;
        next();
        additive_expression();
        gen_op(t);
    }
}


void relational_expression()
{
    int t;
    expr_shift();
    while ((tok == TOK_LT || tok == TOK_LE) ||
           tok == TOK_GT || tok == TOK_GE) 
	{
        t = tok;
        next();
        expr_shift();
        gen_op(t);
    }
}


void additive_expression()
{
    int t;
    multiplicative_expression();
    while (tok == '+' || tok == '-'){
        t = tok;
        next();
        multiplicative_expression();
        gen_op(t);
    }
}

#include "lex.h"




void gen_opi(int op){
    switch(op) 
	{
		case '+':
			emitABC(OP_ADD, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '-':
			emitABC(OP_SUB, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '*':
			emitABC(OP_MUL, optop[-1].value , optop[-1].value, optop[0].value);
			break;
		case '/':
			emitABC(OP_DIV, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '%':
			emitABC(OP_MOD, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_LT:
			emitABC(OP_LT, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_LE:
			emitABC(OP_LE, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_GT:
			emitABC(OP_GT, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_GE:
			emitABC(OP_GE, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_EQ:
			emitABC(OP_EQ, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_NE:
			emitABC(OP_NE, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '|':
			emitABC(OP_BOR, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '&':
			emitABC(OP_BAND, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case '^':
			emitABC(OP_POW, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_SHL:
			emitABC(OP_SHL, optop[-1].value, optop[-1].value, optop[0].value);
			break;
		case TOK_SAR:
			emitABC(OP_SHR, optop[-1].value, optop[-1].value, optop[0].value);
			break;
	}
}


void gen_op(int op){
	int u, bt1, bt2;
    Type type1;
	
    bt1 = optop[-1].type.t & T_TYPE;
    bt2 = optop[0].type.t & T_TYPE;
	
	gen_opi(op);
	operand_pop();
	freeReg(sec_text);
}


void multiplicative_expression()
{
    int t;
    unary_expression();
    while (tok == '*' || tok == '/' || tok == '%') 
	{
        t = tok;
        next();
        unary_expression();
        gen_op(t);
    }
}



void unary_expression(){
    switch(tok) 
	{
		case '+':
			break;
		case '-':
			break;
		default:
			postfix_expression();
			break;
	}
}


void argument_expression_list()
{
	Operand ret;
	Symbol *s,*sa;
	int nb_args, t;
	s = optop->type.ref;
	t = optop->type.isvars;
	next();
	sa = s?s->next:NULL; 
	nb_args = 0;
	
	if (tok != '(') 
	{
		for(;;) 
		{
			assignment_expression();
			nb_args++;
			if (sa)
				sa = sa->next;
			if (tok == ')')
				break;
			skip(',');
		}
	}
	if(!t){
		if (sa)
			tx_cerror("ʵ�θ������ں����βθ���");
	}
	skip(')');
	gen_invoke(nb_args);
	
}




void gen_invoke(int nb_args){
    int size, r, args_size, i, reg;
    
    args_size = 0;
	
	
    for(i = 0;i < nb_args; i++) 
	{
		if((optop->type.t & T_CONST)){
			
			if(optop->type.t & T_LVAL){
			
			}else{
				
				
			}
		}else if((optop->type.t & T_VALMASK) == T_LOCAL){

		}else{
			//tx_cerror("�޷�ȷ��������");
		}
        operand_pop(); 
	}
	reg = allocReg(sec_text);
	if(optop->type.t & T_FUNC){
		if(optop->type.t & T_CFUNC){
			
			int reg1 = gen_str_const(table_ident[optop->sym->v - TOK_IDENT]->str);
			emitGetTabUp(reg, 0, reg1 | 0x100);
		}else if(optop->type.t & T_TXFUNC){
			emitClosure(reg ,optop->sym->func->ind);
		}
	}else{
		tx_cerror("function call error");
	}
	emitCall(reg, nb_args , 0);
	freeRegs(sec_text, nb_args + 1);
}



void postfix_expression()
{
	Symbol *s;
	primary_expression(); 
    while (1) 
	{
		if (tok == TOK_ARROW || tok == '.') 
		{		
			
            
            
            
            
            
            
            
            
			
            
            
            
            
            
            
            
            
            
            
            
            
            
			
            
            
            
		} else if (tok == '[') {
            
            
            
            
            
        } else if (tok == '(') {
			argument_expression_list();
		}else
			break;
	}
}


void operand_push(Type *type, int r, int value)
{

    if (optop >= opstack + (OPSTACK_SIZE - 1))
        tx_error("����ʽ���");
    optop++;
    optop->type = *type;
    optop->r = r;
    optop->value = value;
}

#include "vm.h"
#include "gen.c"


void primary_expression(){
	int t, r, addr, i, reg;
    Type type;
    Symbol *s;
	switch(tok) 
	{
    case TOK_CINT:
		i = gen_const(LUA_TINTEGER);
		reg = allocReg(sec_text);
		emitLoadK(reg, i);
        operand_push(&int_type, reg, reg);

		next();
        break;
	case TOK_STR:
		i = gen_const(LUA_TSTRING);
		reg = allocReg(sec_text);
		emitLoadK(reg, i);
        operand_push(&string_type, reg, reg);
		next();
        break;
	case TOK_CDOUBLE:
		i = gen_const(LUA_TNUMBER);
		reg = allocReg(sec_text);
		emitLoadK(reg, i);
        operand_push(&string_type, reg, reg);
		next();
        break;
	case '(':
		next();
		expression();
		skip(')');
		break;
	default:	
		t = tok;
		next();
		if(t < TOK_IDENT)
			expect("��ʶ������");
		s = sym_search(t);
		if (!s){
			if (tok != '(')
				tx_cerror("symbol not define\n");
			
			tx_cerror("symbol not define\n");
			if(strcmp("print",table_ident[t - TOK_IDENT]->str))
				tx_cerror("����ʽ����δ����,��ʶ������\n");
			s = func_sym_push(t, &default_func_type);
			s->r = T_CONST;
		}
		r = s->r;
		
		int rec;
		if(s->r & T_LVAL){
			rec = allocReg(sec_text);
			emitMove(rec, s->c);
			operand_push(&s->type, r, rec);
		}else{
			operand_push(&s->type, r, s->c);
		}
		
        		
        if ((optop->type.t & T_TYPE) == T_FUNC){ 
			optop->sym = s;      
            
        }
		break;
	}
}



void expression_statement(){	
	if (tok != ';') {
		expression();
		operand_pop();
	}
	skip(';');
}
