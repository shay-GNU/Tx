#include "txc.h"
#include "lex.h"

	

void stack_init(Stack *stack,int initsize){
	stack->base = (void **) tx_mallocz(sizeof(void **) * initsize);
	if (!stack->base) {
		tx_error("内存分配失败");;
	} else {
		stack->top = stack->base;
		stack->stacksize = initsize;
	}
}


void* stack_push(Stack *stack, void *element, int size)
{
	int newsize;
	if (stack->top >= stack->base + stack->stacksize) {	
		newsize = stack->stacksize * 2;
		stack->base = (void **) tx_realloc(stack->base,
				   (sizeof(void **) * newsize));
		if (!stack->base) {
			return NULL;
		}
		stack->top = stack->base + stack->stacksize;
		stack->stacksize = newsize;
	}
	*stack->top = (void **) tx_mallocz(size);
	memcpy(*stack->top, element, size);
    stack->top++;
	return *(stack->top-1);
}
Symbol *sym_direct_push(Stack *ss, int v, Type *type, int c){
	Symbol s,*p;
	memset(&s, 0, sizeof(Symbol));
    s.v = v;
	s.type.t = type->t;
	s.type.ref = type->ref;
    s.c = c;
    s.next = NULL;
	p = (Symbol*)stack_push(ss,&s,sizeof(Symbol));
    return p;
}

void stack_pop(Stack *stack)
{
	if (stack->top > stack->base) 
	{
		free(*(--stack->top));
	}
}


void* stack_get_top(Stack *stack){
	void **element;

    if (stack->top > stack->base) {
		element = stack->top - 1;
		return *element;
	} else {
		return NULL;
	}
}


int stack_is_empty(Stack *stack)
{
	if (stack->top == stack->base) {
		return 1;
	} 
	else 
	{
		return 0;
	}
}


void stack_destroy(Stack *stack)
{
	void **element;

	for (element = stack->base; element < stack->top; element++) {
		free(*element);
	}
	if (stack->base) {
		free(stack->base);
	}
	stack->base=NULL;
	stack->top=NULL;
	stack->stacksize=0;
}

Symbol *sym_search(int v){
    if (v >= tok_ident)
        return NULL;
	else
		return (table_ident[v- TOK_IDENT])->sym_identifier;
}


Symbol *func_sym_push(int v, Type *type){
    Symbol *s, **ps;
    s = sym_direct_push(&global_sym_stack, v, type, 0);
	s->isCFunc = 0;
	ps = &( table_ident[v - TOK_IDENT])->sym_identifier;
	
	while (*ps != NULL)
		ps = &(*ps)->prev_tok;
	s->prev_tok = NULL;
	*ps = s;
    return s;
}




Symbol *sym_push(int v, Type *type, int r, int c)
{
    Symbol *ps, **pps;
    TokenSym *ts;
	Stack *ss;

    if (stack_is_empty(&local_sym_stack) == 0)
		ss = &local_sym_stack;
    else
		ss = &global_sym_stack;
    ps = sym_direct_push(ss,v,type,c);
	ps->r = r;    

    
	if(v < SC_ANOM){
        
        ts = (TokenSym*)table_ident[(v- TOK_IDENT)];
        pps = &ts->sym_identifier;
        ps->prev_tok = *pps;
        *pps = ps;
		ps->sym_scope = syntax_level;
        if (ps->prev_tok && ps->prev_tok->sym_scope == ps->sym_scope)
            tx_cerror("重定义");
    }
    return ps;
}

#define LABEL_DE 0x1
#define LABEL_UN 0x2
Symbol *label_push(int v, int r, int c){
    Symbol *ps, **pps;
    TokenSym *ts;
	Stack *ss;
	Type type;
	
	type.t=0;
	type.ref=0;
	ss = &global_label_stack;
    ps = sym_direct_push(ss,v,&type,c);
	ps->r = r;    

	pps = &table_ident[v - TOK_IDENT]->sym_label;
    
    
    
    
    
    
    ps->prev_tok = *pps;
    *pps = ps;
    return ps;
}


void sym_pop(Stack *ptop, Symbol *b)
{
    Symbol *s, **ps;
    TokenSym *ts;
    int v;

    s = (Symbol*)stack_get_top(ptop);
    while(s != b) 
	{
        v = s->v;
        
		if(v < SC_ANOM)
		{
            ts = table_ident[(v- TOK_IDENT)];
                ps = &ts->sym_identifier;
            *ps = s->prev_tok;
        }
		stack_pop(ptop);
        s = (Symbol*)stack_get_top(ptop);  	
    }
}

