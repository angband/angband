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
	char *line;
	struct parser_hook *hooks;
	struct parser_value *fhead;
	struct parser_value *ftail;
	void *priv;
};

struct parser *parser_new(void) {
	struct parser *p = mem_alloc(sizeof *p);
	p->lineno = 0;
	p->line = NULL;
	p->priv = NULL;
	return p;
}

enum parser_error parser_parse(struct parser *p, const char *line) {
	char *cline;
	char *tok;

	assert(p);
	assert(line);

	while (*line && (isspace(*line)))
		line++;
	if (!*line || *line == '#')
		return PARSE_ERROR_NONE;

	cline = string_make(line);

	tok = strtok(cline, ":");
	if (!tok)
		return PARSE_ERROR_MISSING_FIELD;

	return PARSE_ERROR_UNDEFINED_DIRECTIVE;
}

void parser_destroy(struct parser *p) {
	if (p->line)
		mem_free(p->line);
	mem_free(p);
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

static errr parse_specs(struct parser_hook *h, char *fmt) {
	char *name = strtok(fmt, " ");
	char *stype = NULL;
	int type;
	struct parser_spec *s;

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
