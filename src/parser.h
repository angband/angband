/* parser.h - info file parser */
#ifndef PARSER_H
#define PARSER_H

#include "h-basic.h"

struct parser;

enum parser_error {
	PARSE_ERROR_NONE = 0,
	PARSE_ERROR_GENERIC,
	PARSE_ERROR_INVALID_FLAG,
	PARSE_ERROR_INVALID_ITEM_NUMBER,
	PARSE_ERROR_INVALID_SPELL_FREQ,
	PARSE_ERROR_INVALID_VALUE,
	PARSE_ERROR_MISSING_FIELD,
	PARSE_ERROR_MISSING_RECORD_HEADER,
	PARSE_ERROR_NON_SEQUENTIAL_RECORDS,
	PARSE_ERROR_NOT_NUMBER,
	PARSE_ERROR_OBSOLETE_FILE,
	PARSE_ERROR_OUT_OF_BOUNDS,
	PARSE_ERROR_OUT_OF_MEMORY,
	PARSE_ERROR_TOO_FEW_ENTRIES,
	PARSE_ERROR_TOO_MANY_ENTRIES,
	PARSE_ERROR_UNDEFINED_DIRECTIVE,
	PARSE_ERROR_UNRECOGNISED_BLOW,
	PARSE_ERROR_UNRECOGNISED_TVAL,
	PARSE_ERROR_UNRECOGNISED_SVAL,
	PARSE_ERROR_VAULT_TOO_BIG,
	PARSE_ERROR_INTERNAL,

	PARSE_ERROR_MAX
};

extern struct parser *parser_new(void);
extern enum parser_error parser_parse(struct parser *p, const char *line);
extern void parser_destroy(struct parser *p);

extern void *parser_priv(struct parser *p);
extern void parser_setpriv(struct parser *p, void *v);

extern errr parser_reg(struct parser *p, const char *fmt,
                       enum parser_error (*func)(struct parser *p));

extern const char *parser_getsym(struct parser *p, const char *name);
extern const char *parser_getstr(struct parser *p, const char *name);
extern int parser_getint(struct parser *p, const char *name);

#endif /* !PARSER_H */
