/* parser.c - info file parser */

#include "parser.h"
#include "z-virt.h"

struct parser_value {
	int type;
	union {
		int ival;
		char *sval;
	} u;
};

struct parser {
	int lineno;
	char *line;
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
	if (!line)
		return PARSE_ERROR_INTERNAL;
	while (*line && (isspace(*line)))
		line++;
	if (!*line || *line == '#')
		return PARSE_ERROR_NONE;
	return PARSE_ERROR_UNDEFINED_DIRECTIVE;
}

void parser_destroy(struct parser *p) {
	if (p->line)
		mem_free(p->line);
	mem_free(p);
}
