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

const char *parser_error_str[PARSE_ERROR_MAX] = {
	#define PARSE_ERROR(a, b) b,
	#include "list-parser-errors.h"
	#undef PARSE_ERROR
};

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
		if (!strcmp(h->dir, dir))
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
	bool negative = FALSE;

	char buffer[50];
	int i = 0, b, dn, ds, mb;
	
	const char end_chr = '|';
	char eov;

	/* Entire value may be negated */
	if (str[0] == '-') {
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
	if (5 == sscanf(buffer, "%d+%dd%dM%d%c", &b, &dn, &ds, &mb, &eov) &&
		eov == end_chr) {
	} else if (4 == sscanf(buffer, "%d+d%dM%d%c", &b, &ds, &mb, &eov) &&
			   eov == end_chr) {
		dn = 1;
	} else if (3 == sscanf(buffer, "%d+M%d%c", &b, &mb, &eov) &&
			   eov == end_chr) {
		dn = 0; ds = 0;
	} else if (4 == sscanf(buffer, "%d+%dd%d%c", &b, &dn, &ds, &eov) &&
			   eov == end_chr) {
		mb = 0;
	} else if (3 == sscanf(buffer, "%d+d%d%c", &b, &ds, &eov) &&
			   eov == end_chr) {
		dn = 1; mb = 0;
	} else if (4 == sscanf(buffer, "%dd%dM%d%c", &dn, &ds, &mb, &eov) &&
			   eov == end_chr) {
		b = 0;
	} else if (3 == sscanf(buffer, "d%dM%d%c", &ds, &mb, &eov) &&
			   eov == end_chr) {
		b = 0; dn = 1;
	} else if (2 == sscanf(buffer, "M%d%c", &mb, &eov) &&
			   eov == end_chr) {
		b = 0; dn = 0; ds = 0;
	} else if (3 == sscanf(buffer, "%dd%d%c", &dn, &ds, &eov) &&
			   eov == end_chr) {
		b = 0; mb = 0;
	} else if (2 == sscanf(buffer, "d%d%c", &ds, &eov) &&
			   eov == end_chr) {
		b = 0; dn = 1; mb = 0;
	} else if (2 == sscanf(buffer, "%d%c", &b, &eov) &&
			   eov == end_chr) {
		dn = 0; ds = 0; mb = 0;
	} else {
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
	if (negative) {
		bonus->base *= -1;
		bonus->base -= bonus->m_bonus;
		bonus->base -= bonus->dice * (bonus->sides + 1);
	}

	return TRUE;
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
			if (tok)
				sp = tok + 2;
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
	if (!strcmp(s, "int"))
		return PARSE_T_INT | rv;
	if (!strcmp(s, "sym"))
		return PARSE_T_SYM | rv;
	if (!strcmp(s, "str"))
		return PARSE_T_STR | rv;
	if (!strcmp(s, "rand"))
		return PARSE_T_RAND | rv;
	if (!strcmp(s, "uint"))
		return PARSE_T_UINT | rv;
	if (!strcmp(s, "char"))
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
		if (!strcmp(v->spec.name, name))
			return TRUE;
	}
	return FALSE;
}

static struct parser_value *parser_getval(struct parser *p, const char *name) {
	struct parser_value *v;
	for (v = p->fhead; v; v = (struct parser_value *)v->spec.next) {
		if (!strcmp(v->spec.name, name)) {
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

/**
 * ------------------------------------------------------------------------
 * More angband-specific bits of the parser
 * These require hooks into other parts of the code, and are a candidate for
 * moving elsewhere.
 * ------------------------------------------------------------------------ */

static void print_error(struct file_parser *fp, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	msg("Parse error in %s line %d column %d: %s: %s", fp->name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	event_signal(EVENT_MESSAGE_FLUSH);
	quit_fmt("Parse error in %s line %d column %d.", fp->name, s.line, s.col);
}

errr run_parser(struct file_parser *fp) {
	struct parser *p = fp->init();
	errr r;
	if (!p) {
		return PARSE_ERROR_GENERIC;
	}
	r = fp->run(p);
	if (r) {
		print_error(fp, p);
		return r;
	}
	r = fp->finish(p);
	if (r)
		print_error(fp, p);
	return r;
}

/**
 * The basic file parsing function
 */
errr parse_file(struct parser *p, const char *filename) {
	char path[1024];
	char buf[1024];
	ang_file *fh;
	errr r = 0;

	/* The player can put a customised file in the user directory */
	path_build(path, sizeof(path), ANGBAND_DIR_USER, format("%s.txt",
															filename));
	fh = file_open(path, MODE_READ, FTYPE_TEXT);

	/* If no custom file, just load the standard one */
	if (!fh) {
		path_build(path, sizeof(path), ANGBAND_DIR_GAMEDATA,
				   format("%s.txt", filename));
		fh = file_open(path, MODE_READ, FTYPE_TEXT);
	}

	/* The lore file is optional, lack of others is terminal */
	if (!fh) {
		if (streq(filename, "lore"))
			return PARSE_ERROR_NO_FILE_FOUND;
		else
			quit(format("Cannot open '%s.txt'", filename));
	}

	/* Parse it */
	while (file_getl(fh, buf, sizeof(buf))) {
		r = parser_parse(p, buf);
		if (r)
			break;
	}
	file_close(fh);
	return r;
}

void cleanup_parser(struct file_parser *fp)
{
	fp->cleanup();
}

int lookup_flag(const char **flag_table, const char *flag_name) {
	int i = FLAG_START;

	while (flag_table[i] && !streq(flag_table[i], flag_name))
		i++;

	/* End of table reached without match */
	if (!flag_table[i]) i = FLAG_END;

	return i;
}

/**
 * Gets a name and argument for a value expression of the form NAME[arg]
 * \param name_and_value is the expression
 * \param string is the random value string to return (NULL if not required)
 * \param num is the integer to return (NULL if not required)
 */
bool find_value_arg(char *value_name, char *string, int *num)
{
	char *t;

	/* Find the first bracket */
	for (t = value_name; *t && (*t != '['); ++t)
		;

	/* Choose random_value value or int or fail */
	if (string) {
		/* Get the dice */
		if (1 != sscanf(t + 1, "%s", string))
			return FALSE;
	} else if (num) {
		/* Get the value */
		if (1 != sscanf(t + 1, "%d", num))
			return FALSE;
	} else return FALSE;

	/* Terminate the string */
	*t = '\0';

	/* Success */
	return TRUE;
}

/**
 * Get the random value argument from a value expression and put it into the
 * appropriate place in an array
 * \param value the target array of values
 * \param value_type the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_rand_value(random_value *value, const char **value_type, const char *name_and_value)
{
	int i = 0;
	char value_name[80];
	char dice_string[40];
	dice_t *dice;

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, dice_string, NULL))
		return PARSE_ERROR_INVALID_VALUE;

	dice = dice_new();

	while (value_type[i] && !streq(value_type[i], value_name))
		i++;

	if (value_type[i]) {
		if (!dice_parse_string(dice, dice_string)) {
			dice_free(dice);
			return PARSE_ERROR_NOT_RANDOM;
		}
		dice_random_value(dice, &value[i]);
	}

	dice_free(dice);

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a value expression and put it into the
 * appropriate place in an array
 * \param value the target array of integers
 * \param value_type the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_int_value(int *value, const char **value_type, const char *name_and_value)
{
	int val, i = 0;
	char value_name[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, &val))
		return PARSE_ERROR_INVALID_VALUE;

	while (value_type[i] && !streq(value_type[i], value_name))
		i++;

	if (value_type[i])
		value[i] = val;

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a value expression and the index in the
 * value_type array of the suffix used to build the value string
 * \param value the integer value
 * \param index the information on where to put it (eg array index)
 * \param value_type the variable suffix of the possible value strings
 * \param prefix the constant prefix of the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_index_and_int(int *value, int *index, const char **value_type,
						const char *prefix, const char *name_and_value)
{
	int i;
	char value_name[80];
	char value_string[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, value))
		return PARSE_ERROR_INVALID_VALUE;

	/* Compose the value string and look for it */
	for (i = 0; value_type[i]; i++) {
		my_strcpy(value_string, prefix, sizeof(value_string));
		my_strcat(value_string, value_type[i],
				  sizeof(value_string) - strlen(value_string));
		if (streq(value_string, value_name)) break;
	}

	if (value_type[i])
		*index = i;

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a slay value expression and the monster base
 * name it is slaying
 * \param value the integer value
 * \param base the monster base name
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_base_and_int(int *value, char **base, const char *name_and_value)
{
	char value_name[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, value))
		return PARSE_ERROR_INVALID_VALUE;

	/* Must be a slay */
	if (strncmp(value_name, "SLAY_", 5))
		return PARSE_ERROR_INVALID_VALUE;
	else
		*base = string_make(value_name + 5);

	/* If we've got this far, assume it's a valid monster base name */
	return PARSE_ERROR_NONE;
}

errr grab_name(const char *from, const char *what, const char *list[], int max, int *num)
{
	int i;

	/* Scan list */
	for (i = 0; i < max; i++) {
		if (streq(what, list[i])) {
			*num = i;
			return PARSE_ERROR_NONE;
		}
	}

	/* Oops */
	msg("Unknown %s '%s'.", from, what);

	/* Error */
	return PARSE_ERROR_GENERIC;
}

errr grab_flag(bitflag *flags, const size_t size, const char **flag_table, const char *flag_name) {
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_on(flags, size, flag);

	return 0;
}

errr remove_flag(bitflag *flags, const size_t size, const char **flag_table, const char *flag_name) {
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_off(flags, size, flag);

	return 0;
}
