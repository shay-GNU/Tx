#include "txc.h"
#include "tx.h"
#include "lex.h"
#include "sym.c"
#include "exp.c"
void genfunc(Symbol *sym, int argn);
void gen_prolog(Type *func_type);
void statement(int *bsym, int *csym);

void skip(int c){
    if (tok != c )
        tx_cerror("ȱ��%c", c);
    next();
}

void expect(const char *msg){
    tx_cerror("ȱ��%s", msg);
}

void decl(int l){
    decl0(l, 0, NULL);
}

int allocRegs(funcInfo *self, int n){
	if(n <= 0)
		tx_error("n <= 0 !");
    int i;
	for(i = 0; i < n; i++){
		allocReg(self);
	}
	return self->usedRegs - n;
}

int freeRegs(funcInfo *self, int n){
	if(n < 0)
		tx_error("n < 0 !");
	
    int i;
	for(i = 0; i < n; i++){
		freeReg(self);
	}
}

int enterScope(funcInfo *self){
    self->scopeLv++;
}

int allocReg(funcInfo *self){
	self->usedRegs++;
	if(self->usedRegs >= 255)
		tx_cerror("function or expression needs too many registers");
    
	if(self->usedRegs > self->maxRegs){
		self->maxRegs = self->usedRegs;
	}
	return self->usedRegs - 1;
}

int freeReg(funcInfo *self){
	if(self->usedRegs <= 0)
		tx_error("usedRegs <= 0 !");
	self->usedRegs--;
}



int parse_byte(Type *type){		
	int t,type_found;
	Type type1;
	t= 0;
	type_found = 0;
    switch(tok) {
		case TOK_VAR:
			t = T_LVAL;
			type_found = 1;
			next();
			break;
		case TOK_FUNCTION:
			t = T_FUNC;
			type_found = 1;
			next();
			break;
		default:		
			break;
	}
	type->t = t;
	return type_found;
}


int parameter_type_list(Type *type){
	int n,i = 0;
    Symbol **plast, *s, *first;
    Type pt;

	next();
	first = NULL;
	plast = &first;    
	
	
	while(tok != ')'){
		if(tok == TOK_DOTS){
			if(i == 0){
				
				tx_cerror("���δʵ��?");
				next();
				break;
			}else{
				tx_cerror("��δ���?");
			}
		}
        i++;
		if (!parse_byte(&pt)) 
			tx_cerror("��Ч���ͱ�ʶ��");
        if(pt.t == T_FUNC)
            tx_cerror("�ݲ�֧�ֺ�����Ϊ����");

		pt.t = pt.t | T_LOCAL;
        n = tok;
        next();

		s = sym_push(n | SC_PARAMS, &pt, 0, 0);
		*plast = s;
		plast = &s->next;
		if (tok == ')')
			break;
		skip(',');
	}
	skip(')');
	
	
	s = sym_push(SC_ANOM, type, 1 , 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;
    return i;
}



int type_size(Type *t, int *a)
{
	int bt;

	bt = t->t & T_TYPE;
	switch (bt)
	{
	case LUA_TSTRING:
        return 0;
	case LUA_TNUMBER:
		*a = 8;
		return 8;

	case LUA_TINTEGER:
		*a = 8;
		return 8;
	case LUA_TNIL:
		*a = 0;
		return 0;
	case LUA_TBOOLEAN:
		*a = 1;
		return 1;
	default:
        tx_cerror("δʶ������ʹ��?");
	}
}





funcInfo *allocate_storage(Type *type, int r, int has_init, int v, int *addr){
	int size, align;
	funcInfo *sec = NULL;

    if(tok == ';'){
        if(has_init)
            tx_cerror("��Ӧ����;");
        type->t = LUA_TNIL;
    }else if(tok == '{'){
        type->t = LUA_TTABLE;
    }else if(tok == TOK_CINT){
        type->t = LUA_TINTEGER;
    }else if(tok == TOK_CDOUBLE){
        type->t = LUA_TNUMBER;
    }else if(tok == TOK_FALSE && tok == TOK_TRUE){
        type->t = LUA_TBOOLEAN;
    }else if(tok == TOK_NIL){
        type->t = LUA_TNIL;
    }else if(tok == TOK_STR){
        type->t = LUA_TSTRING;
    }
    
	if ((r & T_VALMASK) == T_LOCAL){
		*addr = allocReg(sec_text);
	}else{                  
		tx_cerror("��ʱ��֧��ȫ�ֱ�����");
	}
	return sec_text;
}

Symbol *var_sym_put(Type *type, int r, int v, int addr){
	Symbol *sym = NULL;
	if ((r & T_VALMASK) == T_LOCAL){			
        sym = sym_push(v, type, r, addr);
    } else if (v && (r & T_VALMASK) == T_CONST){ 
		sym = sym_search(v);
		if (sym)
			tx_cerror("%s�ض���\n",(table_ident[v- TOK_IDENT])->str);
		else
			sym = sym_push(v, type, r | T_SYM, 0);
	}
	
	return sym;
}


int direct_declarator_postfix(Type *type){
	int n;
    Symbol *s;

    if (tok == '('){
		return parameter_type_list(type);
	}else{
		expect("(");
	}
}

int parse_func(Type *type,int *v){
	if (tok >= TOK_IDENT) {		 
		*v = tok;
		next();
	} 
	else{
		expect("��ʶ��");
	}
	return direct_declarator_postfix(type);
}

void initializer(Type *type, int c, funcInfo *sec);
int decl0(int l, int is_for_loop_init, void *func_sym){
    int v, has_init, r, addr, argn;
    Type type, btype;
    Symbol *sym;
    funcInfo *sec;funcInfo * func;
    while(1){
        if (!parse_byte(&btype)) {
			if (is_for_loop_init)
                return 0;
            if (tok == ';'){
                next();
                continue;
            }
            if (l !=  T_CONST)
                break;
            if (tok != EOF)
                expect("declaration");
            break;
        }
        while (1) {
            type = btype;
            if (type.t == T_FUNC) {
                if (l == T_LOCAL)
                    tx_cerror("��֧�ֺ���Ƕ�׶���");
                argn = parse_func(&type, &v);
				if(tok != '{')
					tx_cerror("ȱ�ٺ�������");
                sym = sym_search(v);
				type.t = type.t | T_TXFUNC;
                if (sym){
                    tx_cerror("function redefine");
                }else {
                    sym = func_sym_push(v, &type);
					sym->isCFunc = 0;
                }
                func = tx_mallocz(sizeof(funcInfo));
				func->funcname = table_ident[v - TOK_IDENT]->str;
                sym->func = func;
				if(sec_text){
					func->next = sec_text;
					sec_text = func;
				}else{
					sec_text = func;
				}
				if(!strcmp(sec_text->funcname, "main")) main_found = 1;
				sec_text->code = tx_mallocz(sizeof(byte_stream));
				func->ind = funcn;
				func->argn = argn;
				funcn++;
                sym->r = T_SYM | T_CONST;
                genfunc(sym, argn);
                break;
            }else{
				
				v = tok;
				next();
				r = 0;
				r |= T_LVAL;
				r |= l;
				has_init = (tok == '=');

				if (has_init)
					next(); 

				
				sec = allocate_storage(&type, r, has_init, v, &addr);
				sym = var_sym_put(&type, r, v, addr);
				if (has_init)
					initializer(&type, addr, sec);
				if (tok != ',') 
                    if (is_for_loop_init)
                        return 1;
                if (tok == ','){
					next();
                }else{
                    skip(';');
                    break;
                }
            }
        }

    }
}


void init_variable(Type *type, funcInfo *sec, int c, int v){
    int bt;
    void *ptr;

    if (!sec) {                                                       
        if ((optop->r & (T_VALMASK | T_LVAL)) != T_CONST)                                             
            tx_cerror("ȫ�ֱ��������ó�����ʼ��");  
    } else {
		operand_push(type, T_LOCAL | T_LVAL, c);
		operand_swap();
		store0_1();
		operand_pop();
		freeReg(sec_text);
    }
}

void store0_1() {
	emitMove(optop[-1].value,optop[0].value);
    operand_swap();
    operand_pop(); 
}


void operand_swap(){
    Operand tmp;

    tmp = optop[0];
    optop[0] = optop[-1];
    optop[-1] = tmp;
}

void initializer(Type *type, int c, funcInfo *sec){
	assignment_expression();
	init_variable(type, sec, c, 0);
	operand_pop();
}





void gen_prolog(Type *func_type){
	Type *type;
	Symbol *sym =  func_type->ref->next;
	int i = 0, n;
    while(sym){
		type = &sym->type;
		sym_push(sym->v &(~SC_PARAMS) , type,
                 T_LOCAL | T_LVAL, allocReg(sec_text));
		n = sym->v;
		i++;
		sym = sym->next;
	}
}

int is_label(void){
    int n;

    
    if (tok < TOK_DEFAULT)
        return 0;
    
    n = tok;
    next();
    if (tok == ':') {
        return n;
    } else {
        last_tok = tok;
		tok = n;
        return 0;
    }
}


Symbol *label_find(int v){
    v -= TOK_IDENT;
    if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT))
        return NULL;
    return table_ident[v]->sym_label;
}

int gjmp(int t){
	return emitJmp(0 , t);
}

static inline uint16_t read16le(unsigned char *p) {
    return p[0] | (uint16_t)p[1] << 8;
}

static inline void write16le(unsigned char *p, uint16_t x) {
    p[0] = x & 255;  p[1] = x >> 8 & 255;
}

static inline uint32_t read32le(unsigned char *p) {
  return read16le(p) | (uint32_t)read16le(p + 2) << 16;
}

static inline void write32le(unsigned char *p, uint32_t x) {
    write16le(p, x);  write16le(p + 2, x >> 16);
}
static inline void add32le(unsigned char *p, int32_t x) {
    write32le(p, read32le(p) + x);
}

void gsym_addr(int t, int a){
	int num, addr, n, reg, c;
	addr = t * 4;

    while (addr) {
        int32_t *ptr = (int32_t *)(sec_text->code->data + addr);
        n = (int32_t)read32le((uint8_t *)ptr); 
		c = (int)(n & 0x3F);
		reg = (int)((n >> 6) & 0xFF);
		n = (int)(n >>14) ;
		n -= MAXARG_sBx;
		write32le((uint8_t *)ptr, ((a - addr/ 4) + MAXARG_sBx)<<14 | reg<<6 | c);

		
		num = (a + MAXARG_sBx)<<14 | 0<<6 | OP_JMP;
        addr = n * 4;
    }
}

void gsym(int t){
	int a = sec_text->code->ind / 4;
	gsym_addr(t, a);
}


int gen_cmp(){
	int a = emitCmp(optop->value, 0);
	operand_pop();
	return a;
}


static void block(int *bsym, int *csym){
    int a, b, c, d, i, cond, argn;
	Symbol *s;

    if (tok == TOK_IF) {
		next();
        skip('(');
        expression();
        skip(')');
		a = gen_cmp();
		block(bsym, csym);
		if (tok == TOK_ELSE){
            next();
            d = gjmp(0);
            gsym(a);
            block(bsym, csym);
            gsym(d); 
        } else
            gsym(a);
    } else if (tok == TOK_WHILE) {
	    next();
        d = sec_text->code->ind / 4;
        skip('(');
        expression();
        skip(')');
		a = gen_cmp();
		b = 0;
		syntax_level++;
		block(&a, &b);
		syntax_level--;
		gjmp(d - sec_text->code->ind / 4);
		gsym(a);
		
    }else if (tok == '{'){
		Symbol *s;
		s = local_sym_stack;
		syntax_level++;
        next();
        while (tok != '}') {
			
			if ((a = is_label())){
				last_tok = tok;
				tok = a;
			}else
				decl(T_LOCAL);
			if (tok != '}') {
				block(bsym, csym);
			}
        }
        syntax_level--;	
		sym_pop(&local_sym_stack, s, 0);
        next();
    }else if (tok == TOK_RETURN){
		i = 0;
		argn = 0;
		next();
        if (tok != ';') {
			while(tok != ';'){
				assignment_expression();
				argn++;
				if(tok == ';')
					break;
				skip(',');
			}
        }else{
			emitReturn(0, 0);
			goto funcend;
		}
        skip(';');
		i = argn;
		while(argn == 1){
			operand_pop();
			argn--;
		}
		
		emitReturn(optop[0].value, i);
		operand_pop();
		funcend:
		(void *)0;
		exit(0);
    } else if (tok == TOK_BREAK) {
		
        if (!bsym)
            tx_cerror("cannot break");
        *bsym = gjmp(*bsym);
        next();
        skip(';');
    } else if (tok == TOK_CONTINUE) {
		
        if (!csym)
            tx_cerror("cannot continue");
        *csym = gjmp(*csym);
        next();
        skip(';');
    } else if (tok == TOK_FOR) {
		int e;

		next();
        skip('(');
        Symbol *s;
		s = local_sym_stack;
        syntax_level++;
        if (tok != ';') {
            if (!decl0(T_LOCAL, 1, NULL)) {
                expression();
				operand_pop();
            }
        }
		skip(';');
        d = sec_text->code->ind / 4;
        c = sec_text->code->ind / 4;
		a = 0;
		b = 0;
		if (tok != ';') {
            expression();
            a = gen_cmp();
        }
		skip(';');
        if (tok != ')') {
            e = gjmp(0);
            c = sec_text->code->ind / 4;
            expression();
            operand_pop();
            gjmp(d - sec_text->code->ind / 4);
            gsym(e);
        }
        skip(')');
        block(&a, &b);
        gjmp(c - sec_text->code->ind / 4);
        gsym(a);
        gsym_addr(b, c);
        syntax_level--;
        sym_pop(&local_sym_stack, s, 0);
    } else if (tok == TOK_SWITCH) {
    } else if (tok == TOK_CASE) {
    } else if (tok == TOK_DEFAULT) {
    } else if (tok == TOK_GOTO) {
	    next();
		if (tok >= TOK_DEFAULT) {
            s = label_find(tok);
 
            if (!s) {
                s = label_push(tok, LABEL_UN, 0);
            }
            
			
	    	if (s->r & LABEL_UN){
				s->jnext = gjmp(s->jnext);
			}
            else
                gjmp((s->jnext)/4-(sec_text->code->ind)/4);
            next();
        } else {
            expect("label identifier");
        }
        skip(';');
    }else {
		
        b = is_label();
        if(b){
			next();
            s = label_find(b);
            if (s) {
				if (s->r & LABEL_DE)
                	tx_cerror("��ǩ�ض���");
				gsym(s->jnext);
                s->r = LABEL_DE;
            } else {
                s = label_push(b, LABEL_DE, 0);
            }
			s->jnext = sec_text->code->ind;
        } else {
			expression_statement();
		}
    }
}




void genfunc(Symbol *sym, int argn){
    sym_push2(&local_sym_stack, SC_ANOM, int_type.t, 0);
	gen_prolog(&sym->type);
    block(NULL,NULL); 
	sym_pop(&local_sym_stack, NULL, 0); 
}





