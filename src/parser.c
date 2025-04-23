/**
 * \file parser.c
 * \brief Info file parser.
 *
 * Copyright (c) 2011 elly+angband@leptoquark.net
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "init.h"
#include "game-event.h"
#include "message.h"
#include "mon-util.h"
#include "parser.h"
#include "z-file.h"
#include "z-form.h"
#include "z-util.h"
#include "z-virt.h"


/**
 * A parser has a list of hooks (which are run across new lines given to
 * parser_parse()) and a list of the set of named values for the current line.
 * Each hook has a list of specs, which are essentially named formal parameters;
 * when we run a particular hook across a line, each spec in the hook is
 * assigned a value.
 */

enum {
	PARSE_T_NONE = 0,
	PARSE_T_INT = 2,
	PARSE_T_SYM = 4,
	PARSE_T_STR = 6,
	PARSE_T_RAND = 8,
	PARSE_T_UINT = 10,
	PARSE_T_CHAR = 12,
	PARSE_T_OPT = 0x00000001
};

struct parser_spec {
	struct parser_spec *next;
	int type;
	const char *name;
};

struct parser_value {
	struct parser_spec spec;
	union {
		wchar_t cval;
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

/**
 * Allocates a new parser.
 */
struct parser *parser_new(void) {
	struct parser *p = mem_zalloc(sizeof *p);
	return p;
}

static struct parser_hook *findhook(struct parser *p, const char *dir) {
	struct parser_hook *h = p->hooks;
	while (h) {
		if (streq(h->dir, dir))
			break;
		h = h->next;
	}
	return h;
}

static void parser_freeold(struct parser *p) {
	struct parser_value *v;
	while (p->fhead) {
		int t = p->fhead->spec.type & ~PARSE_T_OPT;
		v = (struct parser_value *)p->fhead->spec.next;
		if (t == PARSE_T_SYM || t == PARSE_T_STR)
			mem_free(p->fhead->u.sval);
		mem_free(p->fhead);
		p->fhead = v;
	}
}

static bool parse_random(const char *str, random_value *bonus) {
	bool negative = false;
	/* base, number of dice, sides, and bonus */
	int values[4] = { 0, 0, 0, 0 };
	int i = 0, min_i = 1;

	/* Entire value may be negated */
	if (str[0] == '-') {
		negative = true;
		++str;
	}

	while (1) {
		if (*str == 'd') {
			if (i > 2) {
				return false;
			}
			if (i < 2) {
				i = 2;
				/*
				 * 'd' with no preceding number implies one die.
				 */
				values[1] = 1;
			}
			min_i = 3;
			++str;
		} else if (*str == 'M') {
			if (i == 2) {
				return false;
			}
			i = 3;
			min_i = 4;
			++str;
		} else {
			char *pe;
			unsigned long uv = strtoul(str, &pe, 10);

			if (pe == str) {
				/*
				 * Trailing garbage or not enough values are
				 * not accepted.
				 */
				if (!contains_only_spaces(str) || i < min_i) {
					return false;
				}
				break;
			} else if (uv > INT_MAX || *str == '+') {
				return false;
			}
			str = pe;
			if (i == 0) {
				if (*str == 'd') {
					i = 1;
				} else if (*str == '+') {
					++str;
					min_i = 3;
				} else {
					if (!contains_only_spaces(pe)) {
						return false;
					}
					values[0] = (int)uv;
					break;
				}
			} else if (i == 4) {
				return false;
			}
			values[i] = (int)uv;
			++i;
		}
	}

	/* Assign the values */
	bonus->base = values[0];
	bonus->dice = values[1];
	bonus->sides = values[2];
	bonus->m_bonus = values[3];

	/*
	 * Handle negation (the random components are always positive, so the
	 * base must be adjusted as necessary).
	 */
	if (negative) {
		bonus->base *= -1;
		bonus->base -= bonus->m_bonus;
		bonus->base -= bonus->dice * (bonus->sides + 1);
	}

	return true;
}

/**
 * Parses the provided line.
 *
 * This runs the first parser hook registered with `p` that matches `line`.
 */
enum parser_error parser_parse(struct parser *p, const char *line) {
	char *cline;
	char *tok;
	struct parser_hook *h;
	struct parser_spec *s;
	struct parser_value *v;
	char *sp = NULL;

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
	for (s = h->fhead; s; s = s->next) {
		int t = s->type & ~PARSE_T_OPT;
		p->colno++;

		/* These types are tokenized on ':'; strings are not tokenized
		 * at all (i.e., they consume the remainder of the line) */
		if (t == PARSE_T_INT || t == PARSE_T_SYM || t == PARSE_T_RAND ||
			t == PARSE_T_UINT) {
			tok = strtok(sp, ":");
			sp = NULL;
		} else if (t == PARSE_T_CHAR) {
			tok = strtok(sp, "");
			if (tok) {
				sp = utf8_fskip(tok, 1, NULL);
				if (sp) {
					if (*sp == ':') {
						++sp;
					} else if (*sp) {
						my_strcpy(p->errmsg, s->name,
							sizeof(p->errmsg));
						p->error = PARSE_ERROR_FIELD_TOO_LONG;
						mem_free(cline);
						return PARSE_ERROR_FIELD_TOO_LONG;
					}
				}
			}
		} else {
			tok = strtok(sp, "");
			sp = NULL;
		}
		if (!tok) {
			if (!(s->type & PARSE_T_OPT)) {
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_MISSING_FIELD;
				mem_free(cline);
				return PARSE_ERROR_MISSING_FIELD;
			}
			break;
		}

		/* Allocate a value node. */
		v = mem_alloc(sizeof *v);
		v->spec.next = NULL;
		v->spec.type = s->type;
		v->spec.name = s->name;

		/* Parse out its value. */
		if (t == PARSE_T_INT) {
			char *z = NULL;
			v->u.ival = strtol(tok, &z, 0);
			if (z == tok) {
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_NUMBER;
				return PARSE_ERROR_NOT_NUMBER;
			}
		} else if (t == PARSE_T_UINT) {
			char *z = NULL;
			v->u.uval = strtoul(tok, &z, 0);
			if (z == tok || *tok == '-') {
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_NUMBER;
				return PARSE_ERROR_NOT_NUMBER;
			}
		} else if (t == PARSE_T_CHAR) {
			text_mbstowcs(&v->u.cval, tok, 1);
		} else if (t == PARSE_T_SYM || t == PARSE_T_STR) {
			v->u.sval = string_make(tok);
		} else if (t == PARSE_T_RAND) {
			if (!parse_random(tok, &v->u.rval)) {
				mem_free(v);
				mem_free(cline);
				my_strcpy(p->errmsg, s->name, sizeof(p->errmsg));
				p->error = PARSE_ERROR_NOT_RANDOM;
				return PARSE_ERROR_NOT_RANDOM;
			}
		}

		/* Link it into the value list. */
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

/**
 * Gets parser's private data.
 */
void *parser_priv(struct parser *p) {
	return p->priv;
}

/**
 * Sets parser's private data.
 *
 * This is commonly used to store context for stateful parsing.
 */
void parser_setpriv(struct parser *p, void *v) {
	p->priv = v;
}

static int parse_type(const char *s) {
	int rv = 0;
	if (s[0] == '?') {
		rv |= PARSE_T_OPT;
		s++;
	}
	if (streq(s, "int"))
		return PARSE_T_INT | rv;
	if (streq(s, "sym"))
		return PARSE_T_SYM | rv;
	if (streq(s, "str"))
		return PARSE_T_STR | rv;
	if (streq(s, "rand"))
		return PARSE_T_RAND | rv;
	if (streq(s, "uint"))
		return PARSE_T_UINT | rv;
	if (streq(s, "char"))
		return PARSE_T_CHAR | rv;
	return PARSE_T_NONE;
}

static void clean_specs(struct parser_hook *h) {
	struct parser_spec *s;
	mem_free(h->dir);
	while (h->fhead) {
		s = h->fhead;
		h->fhead = h->fhead->next;
		mem_free((void*)s->name);
		mem_free(s);
	}
}

/**
 * Destroys a parser.
 */
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
	h->fhead = NULL;
	h->ftail = NULL;
	while (name) {
		/* Lack of a type is legal; that means we're at the end of the line. */
		stype = strtok(NULL, " ");
		if (!stype)
			break;

		/* Lack of a name, on the other hand... */
		name = strtok(NULL, " ");
		if (!name) {
			clean_specs(h);
			return -EINVAL;
		}

		/* Grab a type, check to see if we have a mandatory type
		 * following an optional type. */
		type = parse_type(stype);
		if (type == PARSE_T_NONE) {
			clean_specs(h);
			return -EINVAL;
		}
		if (!(type & PARSE_T_OPT) && h->ftail &&
			(h->ftail->type & PARSE_T_OPT)) {
			clean_specs(h);
			return -EINVAL;
		}
		if (h->ftail && ((h->ftail->type & ~PARSE_T_OPT) == PARSE_T_STR)) {
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

/**
 * Registers a parser hook.
 *
 * Hooks have the following format:
 *   <fmt>  ::= <name> [<type> <name>]* [?<type> <name>]*
 *   <type> ::= int | str | sym | rand | char
 * The first <name> is called the directive for this hook. Any other hooks with
 * the same directive are superseded by this hook. It is an error for a
 * mandatory field to follow an optional field. It is an error for any field to
 * follow a field of type `str`, since `str` fields are not delimited and will
 * consume the entire rest of the line.
 */
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

/**
 * A placeholder parse hook indicating a value is ignored
 */
enum parser_error ignored(struct parser *p) {
	return PARSE_ERROR_NONE;
}

/**
 * Returns whether the parser has a value named `name`.
 *
 * Used to test for presence of optional values.
 */
bool parser_hasval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next) {
		if (streq(v->spec.name, name))
			return true;
	}
	return false;
}

static struct parser_value *parser_getval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next) {
		if (streq(v->spec.name, name)) {
			return v;
		}
	}
	quit_fmt("parser_getval error: name is %s\n", name);
	return 0; /* Needed to avoid Windows compiler warning */
}

/**
 * Returns the symbol named `name`. This symbol must exist.
 */
const char *parser_getsym(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_SYM);
	return v->u.sval;
}

/**
 * Returns the integer named `name`. This symbol must exist.
 */
int parser_getint(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_INT);
	return v->u.ival;
}

/**
 * Returns the unsigned integer named `name`. This symbol must exist.
 */
unsigned int parser_getuint(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_UINT);
	return v->u.uval;
}

/**
 * Returns the string named `name`. This symbol must exist.
 */
const char *parser_getstr(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_STR);
	return v->u.sval;
}

/**
 * Returns the random value named `name`. This symbol must exist.
 */
struct random parser_getrand(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_RAND);
	return v->u.rval;
}

/**
 * Returns the character named `name`. This symbol must exist.
 */
wchar_t parser_getchar(struct parser *p, const char *name) {
	struct parser_value *v = parser_getval(p, name);
	assert((v->spec.type & ~PARSE_T_OPT) == PARSE_T_CHAR);
	return v->u.cval;
}

/**
 * Fills the provided struct with the parser's state, if any. Returns true if
 * the parser is in an error state, and false otherwise.
 */
int parser_getstate(struct parser *p, struct parser_state *s) {
	s->error = p->error;
	s->line = p->lineno;
	s->col = p->colno;
	s->msg = p->errmsg;
	return s->error != PARSE_ERROR_NONE;
}

/**
 * Sets the parser's detailed error description and field number.
 */
void parser_setstate(struct parser *p, unsigned int col, const char *msg) {
	p->colno = col;
	my_strcpy(p->errmsg, msg, sizeof(p->errmsg));
}

