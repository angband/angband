/* parser.c - info file parser */

#include "parser.h"
#include "z-virt.h"

enum {
	T_NONE = 0,
	T_INT,
	T_SYM,
	T_STR
};

struct parser_spec {
	struct parser_spec *next;
	int type;
	const char *name;
};

struct parser_value {
	struct parser_spec spec;
	union {
		int ival;
		char *sval;
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
	int lineno;
	struct parser_hook *hooks;
	struct parser_value *fhead;
	struct parser_value *ftail;
	void *priv;
};

struct parser *parser_new(void) {
	struct parser *p = mem_alloc(sizeof *p);
	p->lineno = 0;
	p->priv = NULL;
	return p;
}

struct parser_hook *findhook(struct parser *p, const char *dir) {
	struct parser_hook *h = p->hooks;
	while (h) {
		if (!strcmp(h->dir, dir))
			break;
		h = h->next;
	}
	return h;
}

static void parser_freeold(struct parser *p) {
	struct parser_value *v;
	while (p->fhead) {
		v = (struct parser_value *)p->fhead->spec.next;
		if (p->fhead->spec.type == T_SYM || p->fhead->spec.type == T_STR)
			mem_free(p->fhead->u.sval);
		mem_free(p->fhead);
		p->fhead = v;
	}
}

enum parser_error parser_parse(struct parser *p, const char *line) {
	char *cline;
	char *tok;
	struct parser_hook *h;
	struct parser_spec *s;
	struct parser_value *v;

	assert(p);
	assert(line);

	parser_freeold(p);

	p->lineno++;
	p->fhead = NULL;
	p->ftail = NULL;

	while (*line && (isspace(*line)))
		line++;
	if (!*line || *line == '#')
		return PARSE_ERROR_NONE;

	cline = string_make(line);

	tok = strtok(cline, ":");
	if (!tok)
		return PARSE_ERROR_MISSING_FIELD;

	h = findhook(p, tok);
	if (!h)
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	for (s = h->fhead; s; s = s->next) {
		if (s->type == T_INT || s->type == T_SYM)
			tok = strtok(NULL, ":");
		else
			tok = strtok(NULL, "");
		if (!tok)
			return PARSE_ERROR_MISSING_FIELD;
		v = mem_alloc(sizeof *v);
		v->spec.next = NULL;
		v->spec.type = s->type;
		v->spec.name = s->name;
		if (s->type == T_INT) {
			char *z = NULL;
			v->u.ival = strtol(tok, &z, 0);
			if (z == tok)
				return PARSE_ERROR_NOT_NUMBER;
		} else if (s->type == T_SYM || s->type == T_STR) {
			v->u.sval = string_make(tok);
		}
		if (!p->fhead)
			p->fhead = v;
		else
			p->ftail->spec.next = &v->spec;
		p->ftail = v;
	}

	mem_free(cline);
	return h->func(p);
}

void *parser_priv(struct parser *p) {
	return p->priv;
}

void parser_setpriv(struct parser *p, void *v) {
	p->priv = v;
}

static int parse_type(const char *s) {
	if (!strcmp(s, "int"))
		return T_INT;
	if (!strcmp(s, "sym"))
		return T_SYM;
	if (!strcmp(s, "str"))
		return T_STR;
	return T_NONE;
}

static void clean_specs(struct parser_hook *h) {
	struct parser_spec *s;
	while (h->fhead) {
		s = h->fhead;
		h->fhead = h->fhead->next;
		mem_free((void*)s->name);
		mem_free(s);
	}
}

void parser_destroy(struct parser *p) {
	struct parser_hook *h;
	parser_freeold(p);
	while (p->hooks) {
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
	while (name) {
		stype = strtok(NULL, " ");
		if (!stype)
			break;
		name = strtok(NULL, " ");
		if (!name) {
			clean_specs(h);
			return -EINVAL;
		}
		type = parse_type(stype);
		if (type == T_NONE) {
			clean_specs(h);
			return -EINVAL;
		}
		s = mem_alloc(sizeof *s);
		s->type = type;
		s->name = string_make(name);
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
	h->fhead = NULL;
	r = parse_specs(h, cfmt);
	if (r) {
		mem_free(h);
		mem_free(cfmt);
		return r;
	}

	p->hooks = h;
	mem_free(cfmt);
	return 0;
}

struct parser_value *parser_getval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next) {
		if (!strcmp(v->spec.name, name)) {
			return v;
		}
	}
	assert(0);
}

const char *parser_getsym(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert(v->spec.type == T_SYM);
	return v->u.sval;
}

int parser_getint(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert(v->spec.type == T_INT);
	return v->u.ival;
}

const char *parser_getstr(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert(v->spec.type == T_STR);
	return v->u.sval;
}
