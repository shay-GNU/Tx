#include "lex.h"
#include "txc.h"


static inline int is_space(int ch) {
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f' || ch == '\r';
}
static inline int isid(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static inline int isnum(int c) {
    return c >= '0' && c <= '9';
}
static inline int isoct(int c) {
    return c >= '0' && c <= '7';
}
static inline int toup(int c) {
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}


int handle_eob(void)
{
    BufferedFile *bf = file;
    int len;
    if (bf->buf_ptr >= bf->buf_end) {
        if (bf->fd >= 0) {
            len = fread(bf->buffer, 1, IO_BUF_SIZE, bf->fd);
            if (len < 0)
                len = 0;
        } else {
            len = 0;
        }

        bf->buf_ptr = bf->buffer;
        bf->buf_end = bf->buffer + IO_BUF_SIZE;
        bf->buffer[len] = EOF;
    }
    if (bf->buf_ptr <= bf->buf_end) {
		return EOF;
    } 
}

#define CHECK_EOF(c, p)\
{\
    c = *p++;\
    if (c == EOF) {\
        file->buf_ptr = p;\
        c = handle_eob();\
        p = file->buf_ptr;\
    }\
}

int next(){
    if(last_tok != -1){
        tok = last_tok;
        last_tok = -1;
        return 1;
    }

    next_tok();
	if (tok == TOK_PPNUM) {
        parse_number((char *)tokc.str.data);
    } else if (tok == TOK_PPSTR) {
        parse_string((char *)tokc.str.data, tokc.str.size - 1);
    }
}

#define LONG_SIZE 4
static void parse_number(const char *p)
{
    int b, t, shift, frac_bits, s, exp_val, ch;
    char *q;
    unsigned int bn[2];
    double d;

    q = token_buf;
    ch = *p++;
    t = ch;
    ch = *p++;
    *q++ = t;
    b = 10;
	if (t == '0') {
        if (ch == 'x' || ch == 'X') {
            q--;
            ch = *p++;
            b = 16;
        } else if (ch == 'b' || ch == 'B') {
            q--;
            ch = *p++;
            b = 2;
        }
    }

    while (1) {
        if (ch >= 'a' && ch <= 'f')
            t = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F')
            t = ch - 'A' + 10;
        else if (isnum(ch))
            t = ch - '0';
        else
            break;
        if (t >= b)
            tx_cerror("���ָ�ʽ����");
        if (q >= token_buf + 1024) {
        num_too_long:
            tx_cerror("number too long");
        }
        *q++ = ch;
        ch = *p++;
    }

    if (ch == '.' ||
        ((ch == 'e' || ch == 'E') && b == 10) ) {
        if (ch == '.') {
            if (q >= token_buf + STRING_MAX_SIZE)
                goto num_too_long;
            *q++ = ch;
            ch = *p++;
            float_frac_parse:
            while (ch >= '0' && ch <= '9') {
                if (q >= token_buf + STRING_MAX_SIZE)
                    goto num_too_long;
                *q++ = ch;
                ch = *p++;
            }
        }
        if (ch == 'e' || ch == 'E') {
            if (q >= token_buf + STRING_MAX_SIZE)
                goto num_too_long;
            *q++ = ch;
            ch = *p++;
            if (ch == '-' || ch == '+') {
                if (q >= token_buf + STRING_MAX_SIZE)
                    goto num_too_long;
                *q++ = ch;
                ch = *p++;
            }
            if (ch < '0' || ch > '9')
                expect("exponent digits");
            while (ch >= '0' && ch <= '9') {
                if (q >= token_buf + STRING_MAX_SIZE)
                    goto num_too_long;
                *q++ = ch;
                ch = *p++;
            }
        }
        *q = '\0';
        t = toup(ch);
        errno = 0;
        if (t == 'F') {
            ch = *p++;
            tok = TOK_CDOUBLE;
                tokc.d = strtod(token_buf, NULL);
        } else if (t == 'L') {
            ch = *p++;
            tok = TOK_CDOUBLE;
            tokc.d = strtod(token_buf, NULL);
        } else {
            tok = TOK_CDOUBLE;
            tokc.d = strtod(token_buf, NULL);
        }
    } else {
        unsigned long long n, n1;
        int lcount, ucount, ov = 0;
        const char *p1;

        *q = '\0';
        q = token_buf;
        n = 0;
        while(1) {
            t = *q++;
            if (t == '\0')
                break;
            else if (t >= 'a')
                t = t - 'a' + 10;
            else if (t >= 'A')
                t = t - 'A' + 10;
            else
                t = t - '0';
            if (t >= b)
                tx_cerror("invalid digit");
            n1 = n;
            n = n * b + t;
        }
        lcount = ucount = 0;
        p1 = p;
        for(;;) {
            t = toup(ch);
            if (t == 'L') {
                if (lcount >= 2)
                    tx_cerror("three 'l',��֧����������������");
                if (lcount && *(p - 1) != ch)
                    tx_cerror("��������ݸ�ʽ %s", p1);
                lcount++;
                ch = *p++;
            } else if (t == 'U') {
                if (ucount >= 1)
                    tx_cerror("two 'u',��֧����������������");
                ucount++;
                ch = *p++;
            } else {
                break;
            }
        }
        tok = TOK_CINT;
	    if (lcount)
            tok = TOK_CINT;
	    if (ucount) 
            tok = TOK_CINT;
        tokc.i = n;
    }
    if (ch)
        tx_cerror("invalid number\n");
}




static void parse_escape_string(CString *outstr, const uint8_t *buf, int is_long)
{
    int c, n;
    const uint8_t *p;

    p = buf;
    for(;;) {
        c = *p;
        if (c == '\0')
            break;
        if (c == '\\') {
            p++;
            
            c = *p;
            switch(c) {
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                
                n = c - '0';
                p++;
                c = *p;
                if (isoct(c)) {
                    n = n * 8 + c - '0';
                    p++;
                    c = *p;
                    if (isoct(c)) {
                        n = n * 8 + c - '0';
                        p++;
                    }
                }
                c = n;
                goto add_char_nonext;
            case 'x':
            case 'u':
            case 'U':
                p++;
                n = 0;
                for(;;) {
                    c = *p;
                    if (c >= 'a' && c <= 'f')
                        c = c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F')
                        c = c - 'A' + 10;
                    else if (isnum(c))
                        c = c - '0';
                    else
                        break;
                    n = n * 16 + c;
                    p++;
                }
                c = n;
                goto add_char_nonext;
            case 'a':
                c = '\a';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            case '\'':
            case '\"':
            case '\\': 
            case '?':
                break;
            default:
            invalid_escape:
                if (c >= '!' && c <= '~')
                    tx_cwarning("unknown escape sequence: \'\\%c\'", c);
                else
                    tx_cwarning("unknown escape sequence: \'\\x%x\'", c);
                break;
            }
        } 
        p++;
    add_char_nonext:
        cstr_ccat(outstr, c);
    }
    cstr_ccat(outstr, '\0');
}



static void parse_string(const char *s, int len)
{
    uint8_t buf[1000], *p = buf;
    int is_long, sep;

    if ((is_long = *s == 'L'))
        ++s, --len;
    sep = *s++;
    len -= 2;
    if (len >= sizeof buf)
        p = tx_malloc(len + 1);
    memcpy(p, s, len);
    p[len] = 0;

    cstr_reset(&tokcstr);
    parse_escape_string(&tokcstr, p, is_long);
    if (p != buf)
        free(p);

    if (sep == '\'') {
        
        
        
        
        
        
        
        
        
        
        
        
    } else {
        tokc.str.size = tokcstr.size;
        tokc.str.data = tokcstr.data;
        tok = TOK_STR;
    }
}



static unsigned char *parser_comment(unsigned char *p){
	int ch;
	for(;;){
		for(;;p++){
			ch = *p;
			if (ch == '\n' || ch == '*' || ch == EOF)
			    break;
		}
		if(ch == '\n'){
			file->line++;
            p++;
		}else if(ch == '*'){
			p++;
            ch = *p;
            if(ch == '/'){
                p++;
                return p;
            }else if(ch == EOF){
                tx_cerror("bug:待处理(token:1)\n");
            }else{
                continue;
            }
		}else{
            tx_cerror("bug:待处理(token:1)\n");
		}
	}
}

static unsigned char *parser_line_comment(unsigned char *p){
	int ch;
	for(;;p++){
		ch = *p;
		if(ch == '\n'){
			file->line++;
			break;
		}else if(ch == EOF){

		}
	}
	return p;
}

static unsigned char *parser_string(CString *str, unsigned char *p, char sep){
	int ch;
	for(;;p++){
		ch = *p;
		if(ch == sep){
			break;
		}else if(ch == '\n'){
			file->line++;
		}else if(ch == '\r'){
			if(p[1] == '\n'){
				file->line++;
				p++;
			}else{
                cstr_ccat(str, ch);
            }
		}else if(ch == EOF){
            tx_error("�ַ���ȱ�ٽ�����־", sep);
        }else{
			cstr_ccat(str, ch);
		}
	}
	p++;
	return p;
}


int next_tok(){
	char *p, *ptr;
	TokenSym **t, *t1;

	int c, ch, is_long;
	unsigned int h;
	size_t len;
	p = file->buf_ptr;
redo:
	ch = *p++;
	switch(ch){
		 case '\f': case '\v': case '\t': case ' ':
			goto redo;

        case '\r':
            if(p[0] == '\n'){
                p++;
                goto _LINEFEED;
            }
            goto redo;

		case '\n':
		_LINEFEED:
            file->line++;
			goto redo;
			break;
			
	    case 'a': case 'b': case 'c': case 'd':
    	case 'e': case 'f': case 'g': case 'h':
    	case 'i': case 'j': case 'k': case 'l':
    	case 'm': case 'n': case 'o': case 'p':
    	case 'q': case 'r': case 's': case 't':
    	case 'u': case 'v': case 'w': case 'x':
    	case 'y': case 'z': 
    	case 'A': case 'B': case 'C': case 'D':
    	case 'E': case 'F': case 'G': case 'H':
    	case 'I': case 'J': case 'K': 
    	case 'M': case 'N': case 'O': case 'P':
    	case 'Q': case 'R': case 'S': case 'T':
    	case 'U': case 'V': case 'W': case 'X':
    	case 'Y': case 'Z': 
    	case '_':
    		ptr = p - 1;
            h = TOK_HASH_INIT;
        	h = TOK_HASH_FUNC(h, ch);
            
            CHECK_EOF(ch, p);
            while(isid(ch) || isnum(ch)){
                h = TOK_HASH_FUNC(h, ch);
                CHECK_EOF(ch, p);
            }
            p--;

			len = p - ptr;
			h &= (TOK_HASH_SIZE- 1);
			t = &hash_ident[h];
			while(1){
				t1 = *t;
				if (!t1)
					break;
				if ((t1->len == len) && !memcmp(t1->str, ptr, len))
					goto token_found;
				t = &(t1->hash_next);
			}
			t1 = tx_add_token(t, (char *) ptr, len);
			token_found:
			((void *)0);
            tok = t1->tok;
    		break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
		 	c = ch;
            CHECK_EOF(ch , p);
		 	cstr_reset(&tokcstr);
         for(;;) {
             cstr_ccat(&tokcstr, c);
             if (!(isnum(ch)|| ch == '.'|| isid(ch) ||(
                    (ch == '+' || ch == '-')
                    && (
                        (c == 'e' || c == 'E')
                        && (
                            !((char*)tokcstr.data)[0] == '0'
                            && toup(((char*)tokcstr.data)[1]) == 'X')
                    )
                ))
                )
                 break;
             c = ch;
             CHECK_EOF(ch , p);
         }
         p--;
		 cstr_ccat(&tokcstr, '\0');
		 tokc.str.size = tokcstr.size;
         tokc.str.data = tokcstr.data;
         tok = TOK_PPNUM;
		 break;

    	case '#':
			tok = '#';
			break;

		case '.':
            tok = ch;
			CHECK_EOF(ch, p);
			if(ch == '.'){
				CHECK_EOF(ch, p);
				if(ch == '.')
					tok = TOK_DOTS ;
			}else{
                p--;
			}
			break;

		case '/':
            tok = ch;
			CHECK_EOF(ch, p);
			if(ch == '*'){
				p = parser_comment(p);
				goto redo;
			}else if(ch == '/'){
				p = parser_line_comment(p);
				goto redo;
			}else if(ch == '='){
				tok = TOK_A_DIV;
			}else{
				tok = '/';
                p--;
			}
			break;

		 case '\"':
		 case '\'':
		 	cstr_reset(&tokcstr);
		 str_const:
		 	cstr_ccat(&tokcstr, p[-1]);
		 	p = parser_string(&tokcstr, p, p[-1]);
		 	cstr_ccat(&tokcstr, p[-1]);
		 	cstr_ccat(&tokcstr, '\0');
         	tokc.str.size = tokcstr.size;
         	tokc.str.data = tokcstr.data;
         	tok = TOK_PPSTR;
         	break;

		case '|':
			CHECK_EOF(ch, p);
			if(ch == '|'){
				tok = TOK_LOR;
			}else if(ch == '='){
				tok = TOK_A_OR;
			}else{
				tok = '|';
                p--;
			}
			break;

		case '&':
			CHECK_EOF(ch, p);
			if(ch == '&'){
				tok = TOK_LAND;
			}else if(ch == '='){
				tok = TOK_A_AND;
			}else{
				tok = '&';
                p--;
			}
			break;

		case '<':
            tok = ch;
			CHECK_EOF(ch, p);
			if(ch == '<'){
				CHECK_EOF(ch, p);
				if(ch == '=')
					tok = TOK_A_SHL;
				tok = TOK_SHL;
                p--;
			}else if(ch == '='){
				tok = TOK_LE;
			}else{
				tok = TOK_LT;
                p--;
			}
			break;

		case '>':
			CHECK_EOF(ch, p);
			if(ch == '>'){
				CHECK_EOF(ch, p);
				if(ch == '=')
					tok = TOK_A_SAR;
				tok = TOK_SAR;
                p--;
			}else if(ch == '='){
				tok = TOK_GE;
			}else{
				tok = TOK_GT;
                p--;
			}
			break;

		case '+':
			CHECK_EOF(ch, p);
			if(ch == '+'){
				tok = TOK_INC;
			}else if(ch == '='){
				tok = TOK_A_ADD;
			}else{
				tok = '+';
                p--;
			}
			break;

		case '-':
			CHECK_EOF(ch, p);
			if(ch == '-'){
				tok = TOK_DEC;
			}else if(ch == '='){
				tok = TOK_A_SUB;
			}else if(ch == '>'){
				tok = TOK_ARROW;
			}else{
				tok = '-';
                p--;
			}
			break;

		case '!':
			CHECK_EOF(ch, p);
			if(ch == '='){
                tok = TOK_NE;
            }
			else{
                tok = '!';
                p--;
            }
			break;

		case '=':
			CHECK_EOF(ch, p);
			if(ch == '='){
				tok = TOK_EQ ;
			}else{
				tok = '=';
                p--;
			}
			break;

		case '*':
			CHECK_EOF(ch, p);
			if(ch == '='){
				tok =  TOK_A_MUL;
			}else{
				tok = '*';
				p--;
			}
			break;

		case '%':
			CHECK_EOF(ch, p);
			if(ch == '='){
				tok = TOK_A_MOD;
			}else{
				tok = '%';
                p--;
			}
			break;

		case '^':
			CHECK_EOF(ch, p);
			if(ch == '='){
				tok = TOK_A_XOR;
			}else{
				tok = '^';
                p--;
			}
			break;
		
		case '(':
    	case ')':
   		case '[':
    	case ']':
    	case '{':
    	case '}':
    	case ',':
    	case ';':
    	case ':':
    	case '?':
    	case '~':
			tok = ch;
			break;
        
        case EOF:
            p--;
            if(p >= file->buf_end){
                tx_read(file);
                p = file->buf_ptr;
                goto redo;
            }else{
                tok = EOF;
            }	
        	break;

    	default:
			tx_cerror("δ��ʶ����ַ�");
	}
	if(p >= file->buf_end){
	}
	file->buf_ptr = p;
	return 0; 
}
