/**
 * \file parser.h
 * \brief Info file parser.
 *
 * Copyright (c) 2011 Elly <elly+angband@leptoquark.net>
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

/**
 * The basic structure of the parser is as follows: there is a table of hooks
 * which are run when a directive matching their format is encountered. When the
 * hook is called, all the arguments it declares in its format have been parsed
 * out and can be accessed with parser_get*(). See the unit tests for examples.
 */
#ifndef PARSER_H
#define PARSER_H

#include "h-basic.h"
#include "z-bitflag.h"
#include "z-dice.h"
#include "z-rand.h"

struct parser;

enum parser_error {
	#define PARSE_ERROR(a, b) PARSE_ERROR_##a,
	#include "list-parser-errors.h"
	#undef PARSE_ERROR

	PARSE_ERROR_MAX
};

struct parser_state {
	enum parser_error error;
	unsigned int line;
	unsigned int col;
	char *msg;
};

extern struct parser *parser_new(void);
extern enum parser_error parser_parse(struct parser *p, const char *line);
extern void parser_destroy(struct parser *p);
extern void *parser_priv(struct parser *p);
extern void parser_setpriv(struct parser *p, void *v);
extern errr parser_reg(struct parser *p, const char *fmt,
                       enum parser_error (*func)(struct parser *p));
extern enum parser_error ignored(struct parser *p);
extern bool parser_hasval(struct parser *p, const char *name);
extern const char *parser_getsym(struct parser *p, const char *name);
extern const char *parser_getstr(struct parser *p, const char *name);
extern int parser_getint(struct parser *p, const char *name);
extern unsigned int parser_getuint(struct parser *p, const char *name);
extern struct random parser_getrand(struct parser *p, const char *name);
extern wchar_t parser_getchar(struct parser *p, const char *name);
extern int parser_getstate(struct parser *p, struct parser_state *s);
extern void parser_setstate(struct parser *p, unsigned int col, const char *msg);

#endif /* !PARSER_H */
