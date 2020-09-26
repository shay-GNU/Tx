#include "txc.h"
#include "lex.h"


static Symbol *__sym_malloc(void){
    Symbol *sym_pool, *sym, *last_sym;
    int i;

    pools++;
    sym_pool = malloc(SYM_POOL_NB * sizeof(Symbol));
    

    last_sym = sym_free_first;
    sym = sym_pool;
    for(i = 0; i < SYM_POOL_NB; i++) {
        sym->next = last_sym;
        last_sym = sym;
        sym++;
    }
    sym_free_first = last_sym;
    return last_sym;
}

static inline Symbol *sym_malloc(void)
{
    Symbol *sym;
    sym = sym_free_first;
    if (!sym)
        sym = __sym_malloc();
    sym_free_first = sym->next;
    return sym;
}


Symbol *sym_search(int v){
    if (v >= tok_ident)
        return NULL;
	else
		return (table_ident[v- TOK_IDENT])->sym_identifier;
}


Symbol *func_sym_push(int v, Type *type){
    Symbol *s, **ps;
    s = sym_push2(&global_sym_stack, v, type->t , 0);
	s->type.ref = type->ref;
	s->isCFunc = 0;
	ps = &(table_ident[v - TOK_IDENT])->sym_identifier;
	
	while (*ps != NULL)
		ps = &(*ps)->prev_tok;
	s->prev_tok = NULL;
	*ps = s;
    return s;
}


Symbol *sym_push2(Symbol **ps, int v, int t, int c){
    Symbol *s;

    s = sym_malloc();
    memset(s, 0, sizeof *s);
    s->v = v;
    s->type.t = t;
    s->c = c;
    
    s->prev = *ps;
    *ps = s;
    return s;
}

Symbol *sym_push(int v, Type *type, int r, int c){
    Symbol *ps, **pps, **ss;
    TokenSym *ts;

    if (local_sym_stack)
		ss = &local_sym_stack;
    else
		ss = &global_sym_stack;
    ps = sym_push2(ss,v,type->t,c);
	ps->type.ref = type->ref;
	ps->r = r;
    
	if(v < SC_ANOM){
        
        ts = (TokenSym*)table_ident[(v- TOK_IDENT)];
        pps = &ts->sym_identifier;
        ps->prev_tok = *pps;
        *pps = ps;
		ps->sym_scope = syntax_level;
        if (ps->prev_tok && ps->prev_tok->sym_scope == ps->sym_scope)
            tx_cerror("redeclaration of ''");
    }
    return ps;
}

#define LABEL_DE 0x1
#define LABEL_UN 0x2
Symbol *label_push(int v, int r, int c){
    Symbol *ps, **pps, **ss;
    TokenSym *ts;
	Type type;
	
	ss = &global_label_stack;
    ps = sym_push2(ss,v,0,c);
	ps->type.ref =NULL;
	ps->r = r;    

	pps = &table_ident[v - TOK_IDENT]->sym_label;
    ps->prev_tok = *pps;
    *pps = ps;
    return ps;
}
void sym_free(Symbol *sym){
    sym->next = sym_free_first;
    sym_free_first = sym;
}
void sym_pop(Symbol **ptop, Symbol *b, int keep){
    Symbol *s, *ss, **ps;
    TokenSym *ts;
    int v;

    s = *ptop;
    while(s != b) {
        ss = s->prev;
        v = s->v;

        if (v < SC_ANOM) {
            ts = table_ident[v - TOK_IDENT];
            ps = &ts->sym_identifier;
            *ps = s->prev_tok;
        }
		if (!keep)
	    	sym_free(s);
        s = ss;
    }
    if (!keep)
		*ptop = b;
}

