/** Init file parser library
 *
 * The basic structure of the parser is as follows: there is a table of hooks
 * which are run when a directive matching their format is encountered. When the
 * hook is called, all the arguments it declares in its format have been parsed
 * out and can be accessed with parser_get*(). See the unit tests for examples.
 */
#ifndef PARSER_H
#define PARSER_H

#include "h-basic.h"
#include "z-rand.h"

struct parser;

enum parser_error {
	PARSE_ERROR_NONE = 0,
	PARSE_ERROR_GENERIC,
	PARSE_ERROR_INVALID_FLAG,
	PARSE_ERROR_INVALID_ITEM_NUMBER,
	PARSE_ERROR_INVALID_SPELL_FREQ,
	PARSE_ERROR_INVALID_VALUE,
	PARSE_ERROR_MISSING_FIELD,
	PARSE_ERROR_MISSING_COLON,
	PARSE_ERROR_MISSING_RECORD_HEADER,
	PARSE_ERROR_NON_SEQUENTIAL_RECORDS,
	PARSE_ERROR_NOT_NUMBER,
	PARSE_ERROR_NOT_RANDOM,
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

extern const char *parser_error_str[PARSE_ERROR_MAX];

/** Allocates a new parser. */
extern struct parser *parser_new(void);

/** Parses the provided line.
 *
 * This runs the first parser hook registered with `p` that matches `line`.
 */
extern enum parser_error parser_parse(struct parser *p, const char *line);

/** Destroys a parser. */
extern void parser_destroy(struct parser *p);

/** Gets parser's private data. */
extern void *parser_priv(struct parser *p);

/** Sets parser's private data.
 *
 * This is commonly used to store context for stateful parsing.
 */
extern void parser_setpriv(struct parser *p, void *v);

/** Registers a parser hook.
 *
 * Hooks have the following format:
 *   <fmt>  ::= <name> [<type> <name>]* [?<type> <name>]*
 *   <type> ::= int | str | sym | rand
 * The first <name> is called the directive for this hook. Any other hooks with
 * the same directive are superseded by this hook. It is an error for a
 * mandatory field to follow an optional field. It is an error for any field to
 * follow a field of type `str`, since `str` fields are not delimited and will
 * consume the entire rest of the line.
 */
extern errr parser_reg(struct parser *p, const char *fmt,
                       enum parser_error (*func)(struct parser *p));

/** Returns whether the parser has a value named `name`.
 *
 * Used to test for presence of optional values.
 */
extern bool parser_hasval(struct parser *p, const char *name);

/** Returns the symbol named `name`. This symbol must exist. */
extern const char *parser_getsym(struct parser *p, const char *name);

/** Returns the string named `name`. This symbol must exist. */
extern const char *parser_getstr(struct parser *p, const char *name);

/** Returns the integer named `name`. This symbol must exist. */
extern int parser_getint(struct parser *p, const char *name);

/** Returns the random value named `name`. This symbol must exist. */
extern struct random parser_getrand(struct parser *p, const char *name);

#endif /* !PARSER_H */
