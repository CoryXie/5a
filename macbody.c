#define VARMAC 0x80

long
getnsn(void)
{
	long n;
	int c;

	c = getnsc();
	if(c < '0' || c > '9')
		return -1;
	n = 0;
	while(c >= '0' && c <= '9') {
		n = n*10 + c-'0';
		c = lexgetc();
	}
	unget(c);
	return n;
}

Sym*
getsym(void)
{
	int c;
	char *cp;

	c = getnsc();
	if(!isalpha(c) && c != '_' && c < Runeself) {
		unget(c);
		return S;
	}
	for(cp = symb;;) {
		if(cp <= symb+NSYMB-4)
			*cp++ = c;
		c = lexgetc();
		if(isalnum(c) || c == '_' || c >= Runeself)
			continue;
		unget(c);
		break;
	}
	*cp = 0;
	if(cp > symb+NSYMB-4)
		yyerror("symbol too large: %s", symb);
	return lookup();
}

Sym*
getsymdots(int *dots)
{
	int c;
	Sym *s;

	s = getsym();
	if(s != S)
		return s;

	c = getnsc();
	if(c != '.'){
		unget(c);
		return S;
	}
	if(lexgetc() != '.' || lexgetc() != '.')
		yyerror("bad dots in macro");
	*dots = 1;
	return slookup("__VA_ARGS__");
}

int
lexgetcom(void)
{
	int c;

	for(;;) {
		c = getnsc();
		if(c != '/')
			break;
		c = lexgetc();
		if(c == '/') {
			while(c != '\n')
				c = lexgetc();
			break;
		}
		if(c != '*')
			break;
		c = lexgetc();
		for(;;) {
			if(c == '*') {
				c = lexgetc();
				if(c != '/')
					continue;
				c = lexgetc();
				break;
			}
			if(c == '\n') {
				yyerror("comment across newline");
				break;
			}
			c = lexgetc();
		}
		if(c == '\n')
			break;
	}
	return c;
}

void
dodefine(char *cp)
{
	Sym *s;
	char *p;
	long l;

	strcpy(symb, cp);
	p = strchr(symb, '=');
	if(p) {
		*p++ = 0;
		s = lookup();
		l = strlen(p) + 2;	/* +1 null, +1 nargs */
		while(l & 3)
			l++;
		while(nhunk < l)
			gethunk();
		*hunk = 0;
		strcpy(hunk+1, p);
		s->macro = hunk;
		hunk += l;
		nhunk -= l;
	} else {
		s = lookup();
		s->macro = "\0001";	/* \000 is nargs */
	}
	if(debug['m'])
		printf("#define (-D) %s %s\n", s->name, s->macro+1);
}

struct
{
	char	*macname;
	void	(*macf)(void);
} mactab[] =
{
	"ifdef",	0,	/* macif(0) */
	"ifndef",	0,	/* macif(1) */
	"else",		0,	/* macif(2) */

	"line",		maclin,
	"define",	macdef,
	"include",	macinc,
	"undef",	macund,

	"pragma",	macprag,
	"endif",	macend,
	0
};

void
domacro(void)
{
	int i;
	Sym *s;

	s = getsym();
	if(s == S)
		s = slookup("endif");
	for(i=0; mactab[i].macname; i++)
		if(strcmp(s->name, mactab[i].macname) == 0) {
			if(mactab[i].macf)
				(*mactab[i].macf)();
			else
				macif(i);
			return;
		}
	yyerror("unknown #: %s", s->name);
	macend();
}

void
macund(void)
{
	Sym *s;

	s = getsym();
	macend();
	if(s == S) {
		yyerror("syntax in #undef");
		return;
	}
	s->macro = 0;
}

#define	NARG	25
void
macdef(void)
{
	Sym *s, *a;
	char *args[NARG], *np, *base;
	int n, i, c, len, dots;
	int ischr;

	s = getsym();
	if(s == S)
		goto bad;
	if(s->macro)
		yyerror("macro redefined: %s", s->name);
	c = lexgetc();
	n = -1;
	dots = 0;
	if(c == '(') {
		n++;
		c = getnsc();
		if(c != ')') {
			unget(c);
			for(;;) {
				a = getsymdots(&dots);
				if(a == S)
					goto bad;
				if(n >= NARG) {
					yyerror("too many arguments in #define: %s", s->name);
					goto bad;
				}
				args[n++] = a->name;
				c = getnsc();
				if(c == ')')
					break;
				if(c != ',' || dots)
					goto bad;
			}
		}
		c = lexgetc();
	}
	if(isspace(c))
		if(c != '\n')
			c = getnsc();
	base = hunk;
	len = 1;
	ischr = 0;
	for(;;) {
		if(isalpha(c) || c == '_') {
			np = symb;
			*np++ = c;
			c = lexgetc();
			while(isalnum(c) || c == '_') {
				*np++ = c;
				c = lexgetc();
			}
			*np = 0;
			for(i=0; i<n; i++)
				if(strcmp(symb, args[i]) == 0)
					break;
			if(i >= n) {
				i = strlen(symb);
				base = allocn(base, len, i);
				memcpy(base+len, symb, i);
				len += i;
				continue;
			}
			base = allocn(base, len, 2);
			base[len++] = '#';
			base[len++] = 'a' + i;
			continue;
		}
		if(ischr){
			if(c == '\\'){ 
				base = allocn(base, len, 1);
				base[len++] = c;
				c = lexgetc();
			}else if(c == ischr)
				ischr = 0;
		}else{
			if(c == '"' || c == '\''){
				base = allocn(base, len, 1);
				base[len++] = c;
				ischr = c;
				c = lexgetc();
				continue;
			}
			if(c == '/') {
				c = lexgetc();
				if(c == '/'){
					c = lexgetc();
					for(;;) {
						if(c == '\n')
							break;
						c = lexgetc();
					}
					continue;
				}
				if(c == '*'){
					c = lexgetc();
					for(;;) {
						if(c == '*') {
							c = lexgetc();
							if(c != '/')
								continue;
							c = lexgetc();
							break;
						}
						if(c == '\n') {
							yyerror("comment and newline in define: %s", s->name);
							break;
						}
						c = lexgetc();
					}
					continue;
				}
				base = allocn(base, len, 1);
				base[len++] = '/';
				continue;
			}
		}
		if(c == '\\') {
			c = lexgetc();
			if(c == '\n') {
				c = lexgetc();
				continue;
			}
			else if(c == '\r') {
				c = lexgetc();
				if(c == '\n') {
					c = lexgetc();
					continue;
				}
			}
			base = allocn(base, len, 1);
			base[len++] = '\\';
			continue;
		}
		if(c == '\n')
			break;
		if(c == '#')
		if(n > 0) {
			base = allocn(base, len, 1);
			base[len++] = c;
		}
		base = allocn(base, len, 1);
		base[len++] = c;
		c = ((--fi.c < 0)? filbuf(): (*fi.p++ & 0xff));
		if(c == '\n')
			lineno++;
		if(c == -1) {
			yyerror("eof in a macro: %s", s->name);
			break;
		}
	}
	do {
		base = allocn(base, len, 1);
		base[len++] = 0;
	} while(len & 3);

	*base = n+1;
	if(dots)
		*base |= VARMAC;
	s->macro = base;
	if(debug['m'])
		printf("#define %s %s\n", s->name, s->macro+1);
	return;

bad:
	if(s == S)
		yyerror("syntax in #define");
	else
		yyerror("syntax in #define: %s", s->name);
	macend();
}

void
macexpand(Sym *s, char *b)
{
	char buf[2000];
	int n, l, c, nargs;
	char *arg[NARG], *cp, *ob, *ecp, dots;

	ob = b;
	if(*s->macro == 0) {
		strcpy(b, s->macro+1);
		if(debug['m'])
			printf("#expand %s %s\n", s->name, ob);
		return;
	}
	
	nargs = (char)(*s->macro & ~VARMAC) - 1;
	dots = *s->macro & VARMAC;

	c = getnsc();
	if(c != '(')
		goto bad;
	n = 0;
	c = lexgetc();
	if(c != ')') {
		unget(c);
		l = 0;
		cp = buf;
		ecp = cp + sizeof(buf)-4;
		arg[n++] = cp;
		for(;;) {
			if(cp >= ecp)
				goto toobig;
			c = lexgetc();
			if(c == '"')
				for(;;) {
					if(cp >= ecp)
						goto toobig;
					*cp++ = c;
					c = lexgetc();
					if(c == '\\') {
						*cp++ = c;
						c = lexgetc();
						continue;
					}
					if(c == '\n')
						goto bad;
					if(c == '"')
						break;
				}
			if(c == '\'')
				for(;;) {
					if(cp >= ecp)
						goto toobig;
					*cp++ = c;
					c = lexgetc();
					if(c == '\\') {
						*cp++ = c;
						c = lexgetc();
						continue;
					}
					if(c == '\n')
						goto bad;
					if(c == '\'')
						break;
				}
			if(c == '/') {
				c = lexgetc();
				switch(c) {
				case '*':
					for(;;) {
						c = lexgetc();
						if(c == '*') {
							c = lexgetc();
							if(c == '/')
								break;
						}
					}
					*cp++ = ' ';
					continue;
				case '/':
					while((c = lexgetc()) != '\n')
						;
					break;
				default:
					unget(c);
					c = '/';
				}
			}
			if(l == 0) {
				if(c == ',') {
					if(n == nargs && dots) {
						*cp++ = ',';
						continue;
					}
					*cp++ = 0;
					arg[n++] = cp;
					if(n > nargs)
						break;
					continue;
				}
				if(c == ')')
					break;
			}
			if(c == '\n')
				c = ' ';
			*cp++ = c;
			if(c == '(')
				l++;
			if(c == ')')
				l--;
		}
		*cp = 0;
	}
	if(n != nargs) {
		yyerror("argument mismatch expanding: %s", s->name);
		*b = 0;
		return;
	}
	cp = s->macro+1;
	for(;;) {
		c = *cp++;
		if(c == '\n')
			c = ' ';
		if(c != '#') {
			*b++ = c;
			if(c == 0)
				break;
			continue;
		}
		c = *cp++;
		if(c == 0)
			goto bad;
		if(c == '#') {
			*b++ = c;
			continue;
		}
		c -= 'a';
		if(c < 0 || c >= n)
			continue;
		strcpy(b, arg[c]);
		b += strlen(arg[c]);
	}
	*b = 0;
	if(debug['m'])
		printf("#expand %s %s\n", s->name, ob);
	return;

bad:
	yyerror("syntax in macro expansion: %s", s->name);
	*b = 0;
	return;

toobig:
	yyerror("too much text in macro expansion: %s", s->name);
	*b = 0;
}

void
macinc(void)
{
	int c0, c, i, f;
	char str[STRINGSZ], *hp;

	c0 = getnsc();
	if(c0 != '"') {
		c = c0;
		if(c0 != '<')
			goto bad;
		c0 = '>';
	}
	for(hp = str;;) {
		c = lexgetc();
		if(c == c0)
			break;
		if(c == '\n')
			goto bad;
		*hp++ = c;
	}
	*hp = 0;

	c = lexgetcom();
	if(c != '\n')
		goto bad;

	f = -1;
	for(i=0; i<ninclude; i++) {
		if(i == 0 && c0 == '>')
			continue;
		strcpy(symb, include[i]);
		strcat(symb, "/");
		if(strcmp(symb, "./") == 0)
			symb[0] = 0;
		strcat(symb, str);
		f = open(symb, 0);
		if(f >= 0)
			break;
	}
	if(f < 0)
		strcpy(symb, str);
	c = strlen(symb) + 1;
	while(c & 3)
		c++;
	while(nhunk < c)
		gethunk();
	hp = hunk;
	memcpy(hunk, symb, c);
	nhunk -= c;
	hunk += c;
	newio();
	pushio();
	newfile(hp, f);
	return;

bad:
	unget(c);
	yyerror("syntax in #include");
	macend();
}

void
maclin(void)
{
	char *cp;
	int c;
	long n;

	n = getnsn();
	c = lexgetc();
	if(n < 0)
		goto bad;

	for(;;) {
		if(c == ' ' || c == '\t') {
			c = lexgetc();
			continue;
		}
		if(c == '"')
			break;
		if(c == '\n') {
			strcpy(symb, "<noname>");
			goto nn;
		}
		goto bad;
	}
	cp = symb;
	for(;;) {
		c = lexgetc();
		if(c == '"')
			break;
		*cp++ = c;
	}
	*cp = 0;
	c = lexgetcom();
	if(c != '\n')
		goto bad;

nn:
	c = strlen(symb) + 1;
	while(c & 3)
		c++;
	while(nhunk < c)
		gethunk();
	cp = hunk;
	memcpy(hunk, symb, c);
	nhunk -= c;
	hunk += c;
	linehist(cp, n);
	return;

bad:
	unget(c);
	yyerror("syntax in #line");
	macend();
}

void
macif(int f)
{
	int c, l, bol;
	Sym *s;

	if(f == 2)
		goto skip;
	s = getsym();
	if(s == S)
		goto bad;
	if(lexgetcom() != '\n')
		goto bad;
	if((s->macro != 0) ^ f)
		return;

skip:
	bol = 1;
	l = 0;
	for(;;) {
		c = lexgetc();
		if(c != '#') {
			if(!isspace(c))
				bol = 0;
			if(c == '\n')
				bol = 1;
			continue;
		}
		if(!bol)
			continue;
		s = getsym();
		if(s == S)
			continue;
		if(strcmp(s->name, "endif") == 0) {
			if(l) {
				l--;
				continue;
			}
			macend();
			return;
		}
		if(strcmp(s->name, "ifdef") == 0 || strcmp(s->name, "ifndef") == 0) {
			l++;
			continue;
		}
		if(l == 0 && f != 2 && strcmp(s->name, "else") == 0) {
			macend();
			return;
		}
	}

bad:
	yyerror("syntax in #if(n)def");
	macend();
}

void
macprag(void)
{
	Sym *s;
	int c0, c;
	char *hp;
	Hist *h;

	s = getsym();

	if(s && strcmp(s->name, "lib") == 0)
		goto praglib;
	if(s && strcmp(s->name, "pack") == 0) {
		pragpack();
		return;
	}
	if(s && strcmp(s->name, "fpround") == 0) {
		pragfpround();
		return;
	}
	if(s && strcmp(s->name, "profile") == 0) {
		pragprofile();
		return;
	}
	if(s && strcmp(s->name, "varargck") == 0) {
		pragvararg();
		return;
	}
	if(s && strcmp(s->name, "incomplete") == 0) {
		pragincomplete();
		return;
	}
	while(getnsc() != '\n')
		;
	return;

praglib:
	c0 = getnsc();
	if(c0 != '"') {
		c = c0;
		if(c0 != '<')
			goto bad;
		c0 = '>';
	}
	for(hp = symb;;) {
		c = lexgetc();
		if(c == c0)
			break;
		if(c == '\n')
			goto bad;
		*hp++ = c;
	}
	*hp = 0;
	c = lexgetcom();
	if(c != '\n')
		goto bad;

	/*
	 * put pragma-line in as a funny history 
	 */
	c = strlen(symb) + 1;
	while(c & 3)
		c++;
	while(nhunk < c)
		gethunk();
	hp = hunk;
	memcpy(hunk, symb, c);
	nhunk -= c;
	hunk += c;

	h = alloc(sizeof(Hist));
	h->name = hp;
	h->line = lineno;
	h->offset = -1;
	h->link = H;
	if(ehist == H) {
		hist = h;
		ehist = h;
		return;
	}
	ehist->link = h;
	ehist = h;
	return;

bad:
	unget(c);
	yyerror("syntax in #pragma lib");
	macend();
}

void
macend(void)
{
	int c;

	for(;;) {
		c = getnsc();
		if(c < 0 || c == '\n')
			return;
	}
}

void
linehist(char *f, int offset)
{
	Hist *h;

	/*
	 * overwrite the last #line directive if
	 * no alloc has happened since the last one
	 */
	if(newflag == 0 && ehist != H && offset != 0 && ehist->offset != 0)
		if(f && ehist->name && strcmp(f, ehist->name) == 0) {
			ehist->line = lineno;
			ehist->offset = offset;
			return;
		}

	if(debug['f'])
		if(f) {
			if(offset)
				printf("%4ld: %s (#line %d)\n", lineno, f, offset);
			else
				printf("%4ld: %s\n", lineno, f);
		} else
			printf("%4ld: <pop>\n", lineno);
	newflag = 0;

	h = alloc(sizeof(Hist));
	h->name = f;
	h->line = lineno;
	h->offset = offset;
	h->link = H;
	if(ehist == H) {
		hist = h;
		ehist = h;
		return;
	}
	ehist->link = h;
	ehist = h;
}

void
gethunk(void)
{
	char *h;
	long nh;

	nh = NHUNK;
	if(thunk >= 10L*NHUNK)
		nh = 10L*NHUNK;
	h = (char*)mysbrk(nh);
	if(h == (char*)-1) {
		yyerror("out of memory");
		errorexit();
	}
	hunk = h;
	nhunk = nh;
	thunk += nh;
}
