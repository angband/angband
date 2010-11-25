#ifndef INCLUDED_Z_TEXTBLOCK_H
#define INCLUDED_Z_TEXTBLOCK_H

#include "h-basic.h"

/** Opaque text_block type */
typedef struct textblock textblock;


textblock *textblock_new(void);
void textblock_free(textblock *tb);

void textblock_append(textblock *tb, const char *fmt, ...);
void textblock_append_c(textblock *tb, byte attr, const char *fmt, ...);

const char *textblock_text(textblock *tb);
const byte *textblock_attrs(textblock *tb);

#endif /* INCLUDED_Z_TEXTBLOCK_H */
