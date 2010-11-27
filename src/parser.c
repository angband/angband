/** Info file parser
 *
 * A parser has a list of hooks (which are run across new lines given to
 * parser_parse()) and a list of the set of named values for the current line.
 * Each hook has a list of specs, which are essentially named formal parameters;
 * when we run a particular hook across a line, each spec in the hook is
 * assigned a value.
 */

#include "parser.h"
#include "z-util.h"
#include "z-virt.h"


const char *parser_error_str[PARSE_ERROR_MAX] = {
	"(none)",
	"generic error",
	"invalid flag",
	"invalid item number",
	"invalid spell frequency",
	"invalid value",
	"invalid colour",
	"invalid effect",
	"invalid option",
	"missing field",
	"missing record header",
	"field too long",
	"non-sequential records",
	"not a number",
	"not random",
	"obsolete file",
	"out of bounds",
	"out of memory",
	"too few entries",
	"too many entries",
	"undefined directive",
	"unrecognized blow",
	"unrecognized tval",
	"unrecognized sval",
	"vault too big",
	"internal error",
};

enum {
	T_NONE = 0,
	T_INT = 2,
	T_SYM = 4,
	T_STR = 6,
	T_RAND = 8,
	T_UINT = 10,
	T_CHAR = 12,
	T_OPT = 0x00000001
};

struct parser_spec {
	struct parser_spec *next;
	int type;
	const char *name;
};

struct parser_value {
	struct parser_spec spec;
	union {
		char cval;
		int ival;
		unsigned int uval;
		char *sval;
		random_value rval;
	} u;
};

struct parser_hook {
	struct parser_hook *next;
	enum parser_error (*func)(struct parser *p);
	char *dir;
	struct parser_spec *fhead;
	struct parser_spec *ftail;
};

struct parser {
	enum parser_error error;
	unsigned int lineno;
	unsigned int colno;
	char errmsg[1024];
	struct parser_hook *hooks;
	struct parser_value *fhead;
	struct parser_value *ftail;
	void *priv;
};

struct parser *parser_new(void) {
	struct parser *p = mem_zalloc(sizeof *p);
	return p;
}

struct parser_hook *findhook(struct parser *p, const char *dir) {
	struct parser_hook *h = p->hooks;
	while (h)
	{
		if (!strcmp(h->dir, dir))
			break;
		h = h->next;
	}
	return h;
}

static void parser_freeold(struct parser *p) {
	struct parser_value *v;
	while (p->fhead)
	{
		int t = p->fhead->spec.type & ~T_OPT;
		v = (struct parser_value *)p->fhead->spec.next;
		if (t == T_SYM || t == T_STR)
			mem_free(p->fhead->u.sval);
		mem_free(p->fhead);
		p->fhead = v;
	}
}

static bool parse_random(const char *str, random_value *bonus) {
	bool negative = FALSE;

	char buffer[50];
	int i = 0, b, dn, ds, mb;
	
	const char end_chr = '|';
	char eov;

	/* Entire value may be negated */
	if (str[0] == '-')
	{
		negative = TRUE;
		i++;
	}

	/* Make a working copy of the string */
	my_strcpy(buffer, &str[i], N_ELEMENTS(buffer) - 2);

	/* Check for invalid negative numbers */
	if (NULL != strstr(buffer, "-"))
		return FALSE;

	/*
	 * Add a sentinal value at the end of the string.
	 * Used by scanf to make sure there's no text after the final conversion.
	 */
	buffer[strlen(buffer) + 1] = '\0';
	buffer[strlen(buffer)] = end_chr;

	/* Scan the value, apply defaults for unspecified components */
	if (5 == sscanf(buffer, "%d+%dd%dM%d%c", &b, &dn, &ds, &mb, &eov) && eov == end_chr)
	{
		/* No defaults */
	}
	else if (4 == sscanf(buffer, "%d+d%dM%d%c", &b, &ds, &mb, &eov) && eov == end_chr)
	{
		dn = 1;
	}
	else if (3 == sscanf(buffer, "%d+M%d%c", &b, &mb, &eov) && eov == end_chr)
	{
		dn = 0; ds = 0;
	}
	else if (4 == sscanf(buffer, "%d+%dd%d%c", &b, &dn, &ds, &eov) && eov == end_chr)
	{
		mb = 0;
	}
	else if (3 == sscanf(buffer, "%d+d%d%c", &b, &ds, &eov) && eov == end_chr)
	{
		dn = 1; mb = 0;
	}
	else if (4 == sscanf(buffer, "%dd%dM%d%c", &dn, &ds, &mb, &eov) && eov == end_chr)
	{
		b = 0;
	}
	else if (3 == sscanf(buffer, "d%dM%d%c", &ds, &mb, &eov) && eov == end_chr)
	{
		b = 0; dn = 1;
	}
	else if (2 == sscanf(buffer, "M%d%c", &mb, &eov) && eov == end_chr)
	{
		b = 0; dn = 0; ds = 0;
	}
	else if (3 == sscanf(buffer, "%dd%d%c", &dn, &ds, &eov) && eov == end_chr)
	{
		b = 0; mb = 0;
	}
	else if (2 == sscanf(buffer, "d%d%c", &ds, &eov) && eov == end_chr)
	{
		b = 0; dn = 1; mb = 0;
	}
	else if (2 == sscanf(buffer, "%d%c", &b, &eov) && eov == end_chr)
	{
		dn = 0; ds = 0; mb = 0;
	}
	else
	{
		return FALSE;
	}

	/* Assign the values */
	bonus->base = b;
	bonus->dice = dn;
	bonus->sides = ds;
	bonus->m_bonus = mb;

	/*
	 * Handle negation (the random components are always positive, so the base
	 * must be adjusted as necessary).
	 */
	if (negative)
	{
		bonus->base *= -1;
		bonus->base -= bonus->m_bonus;
		bonus->base -= bonus->dice * (bonus->sides + 1);
	}

	return TRUE;
}

/* This is a bit long and should probably be refactored a bit. */
enum parser_error parser_parse(struct parser *p, const char *line) {
	char *cline;
	char *tok;
	struct parser_hook *h;
	struct parser_spec *s;
	struct parser_value *v;
	char *sp = NULL;
	char *iline;

	assert(p);
	assert(line);

	parser_freeold(p);

	p->lineno++;
	p->colno = 1;
	p->fhead = NULL;
	p->ftail = NULL;

	/* Ignore empty lines and comments. */
	while (*line && (isspace(*line)))
		line++;
	if (!*line || *line == '#')
		return PARSE_ERROR_NONE;

	cline = string_make(line);
	iline = cline;

	tok = strtok(cline, ":");
	if (!tok) {
		mem_free(cline);
		p->error = PARSE_ERROR_MISSING_FIELD;
		return PARSE_ERROR_MISSING_FIELD;
	}

	h = findhook(p, tok);
	if (!h) {
		my_strcpy(p->errmsg, tok, sizeof(p->errmsg));
		p->error = PARSE_ERROR_UNDEFINED_DIRECTIVE;
		mem_free(cline);
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;
	}

	/* There's a little bit of trickiness here to account for optional
	 * types. The optional flag has a bit assigned to it in the spec's type
	 * tag; we compute a temporary type for the spec with that flag removed
	 * and use that instead. */
	for (s = h->fhead; s; s = s->next)
	{
		int t = s->type & ~T_OPT;
		p->colno++;
		/* These types are tokenized on ':'; strings are not tokenized
		 * at all (i.e., they consume the remainder of the line) */
		if (t == T_INT || t == T_SYM || t == T_RAND || t == T_UINT) {
			tok = strtok(sp, ":");
			sp = NULL;
		} else if (t == T_CHAR) {
			tok = strtok(sp, "");
			if (tok)
				sp = tok + 1;
		} else {
			tok = strtok(sp, "");
			sp = NULL;
		}
		if (!tok)
		{
			if (!(s->type & T_OPT)) {
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_MISSING_FIELD;
				mem_free(cline);
				return PARSE_ERROR_MISSING_FIELD;
			}
			break;
		}

		/* Allocate a value node, parse out its value, and link it into
		 * the value list. */
		v = mem_alloc(sizeof *v);
		v->spec.next = NULL;
		v->spec.type = s->type;
		v->spec.name = s->name;
		if (t == T_INT)
		{
			char *z = NULL;
			v->u.ival = strtol(tok, &z, 0);
			if (z == tok)
			{
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_NUMBER;
				return PARSE_ERROR_NOT_NUMBER;
			}
		}
		else if (t == T_UINT)
		{
			char *z = NULL;
			v->u.uval = strtoul(tok, &z, 0);
			if (z == tok || *tok == '-')
			{
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_NUMBER;
				return PARSE_ERROR_NOT_NUMBER;
			}
		}
		else if (t == T_CHAR)
		{
			v->u.cval = *tok;
		}
		else if (t == T_SYM || t == T_STR)
		{
			v->u.sval = string_make(tok);
		}
		else if (t == T_RAND)
		{
			if (!parse_random(tok, &v->u.rval))
			{
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_RANDOM;
				return PARSE_ERROR_NOT_RANDOM;
			}
		}
		if (!p->fhead)
			p->fhead = v;
		else
			p->ftail->spec.next = &v->spec;
		p->ftail = v;
	}

	mem_free(cline);

	p->error = h->func(p);
	return p->error;
}

void *parser_priv(struct parser *p) {
	return p->priv;
}

void parser_setpriv(struct parser *p, void *v) {
	p->priv = v;
}

static int parse_type(const char *s) {
	int rv = 0;
	if (s[0] == '?')
	{
		rv |= T_OPT;
		s++;
	}
	if (!strcmp(s, "int"))
		return T_INT | rv;
	if (!strcmp(s, "sym"))
		return T_SYM | rv;
	if (!strcmp(s, "str"))
		return T_STR | rv;
	if (!strcmp(s, "rand"))
		return T_RAND | rv;
	if (!strcmp(s, "uint"))
		return T_UINT | rv;
	if (!strcmp(s, "char"))
		return T_CHAR | rv;
	return T_NONE;
}

static void clean_specs(struct parser_hook *h) {
	struct parser_spec *s;
	mem_free(h->dir);
	while (h->fhead)
	{
		s = h->fhead;
		h->fhead = h->fhead->next;
		mem_free((void*)s->name);
		mem_free(s);
	}
}

void parser_destroy(struct parser *p) {
	struct parser_hook *h;
	parser_freeold(p);
	while (p->hooks)
	{
		h = p->hooks->next;
		clean_specs(p->hooks);
		mem_free(p->hooks);
		p->hooks = h;
	}
	mem_free(p);
}

static errr parse_specs(struct parser_hook *h, char *fmt) {
	char *name ;
	char *stype = NULL;
	int type;
	struct parser_spec *s;

	assert(h);
	assert(fmt);

	name = strtok(fmt, " ");
	if (!name)
		return -EINVAL;
	h->dir = string_make(name);
	h->fhead = NULL;
	h->ftail = NULL;
	while (name)
	{
		/* Lack of a type is legal; that means we're at the end of the
		 * line. */
		stype = strtok(NULL, " ");
		if (!stype)
			break;

		/* Lack of a name, on the other hand... */
		name = strtok(NULL, " ");
		if (!name)
		{
			clean_specs(h);
			return -EINVAL;
		}

		/* Grab a type, check to see if we have a mandatory type
		 * following an optional type. */
		type = parse_type(stype);
		if (type == T_NONE)
		{
			clean_specs(h);
			return -EINVAL;
		}
		if (!(type & T_OPT) && h->ftail && (h->ftail->type & T_OPT))
		{
			clean_specs(h);
			return -EINVAL;
		}
		if (h->ftail && ((h->ftail->type & ~T_OPT) == T_STR))
		{
			clean_specs(h);
			return -EINVAL;
		}

		/* Save this spec. */
		s = mem_alloc(sizeof *s);
		s->type = type;
		s->name = string_make(name);
		s->next = NULL;
		if (h->fhead)
			h->ftail->next = s;
		else
			h->fhead = s;
		h->ftail = s;
	}

	return 0;
}

errr parser_reg(struct parser *p, const char *fmt,
                enum parser_error (*func)(struct parser *p)) {
	errr r;
	char *cfmt;
	struct parser_hook *h;

	assert(p);
	assert(fmt);
	assert(func);

	h = mem_alloc(sizeof *h);
	cfmt = string_make(fmt);
	h->next = p->hooks;
	h->func = func;
	r = parse_specs(h, cfmt);
	if (r)
	{
		mem_free(h);
		mem_free(cfmt);
		return r;
	}

	p->hooks = h;
	mem_free(cfmt);
	return 0;
}

bool parser_hasval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next)
	{
		if (!strcmp(v->spec.name, name))
			return TRUE;
	}
	return FALSE;
}

struct parser_value *parser_getval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next)
	{
		if (!strcmp(v->spec.name, name))
		{
			return v;
		}
	}
	assert(0);
}

const char *parser_getsym(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_SYM);
	return v->u.sval;
}

int parser_getint(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_INT);
	return v->u.ival;
}

unsigned int parser_getuint(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_UINT);
	return v->u.uval;
}

const char *parser_getstr(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_STR);
	return v->u.sval;
}

struct random parser_getrand(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_RAND);
	return v->u.rval;
}

char parser_getchar(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~T_OPT) == T_CHAR);
	return v->u.cval;
}

int parser_getstate(struct parser *p, struct parser_state *s) {
	s->error = p->error;
	s->line = p->lineno;
	s->col = p->colno;
	s->msg = p->errmsg;
	return s->error != PARSE_ERROR_NONE;
}

void parser_setstate(struct parser *p, unsigned int col, const char *msg) {
	p->colno = col;
	my_strcpy(p->errmsg, msg, sizeof(p->errmsg));
}
