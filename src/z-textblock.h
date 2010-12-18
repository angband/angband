#ifndef INCLUDED_Z_TEXTBLOCK_H
#define INCLUDED_Z_TEXTBLOCK_H

#include "z-file.h"
#include "defines.h"

/** Opaque text_block type */
typedef struct textblock textblock;


textblock *textblock_new(void);
void textblock_free(textblock *tb);

void textblock_append(textblock *tb, const char *fmt, ...);
void textblock_append_c(textblock *tb, byte attr, const char *fmt, ...);

const char *textblock_text(textblock *tb);
const byte *textblock_attrs(textblock *tb);

size_t textblock_calculate_lines(textblock *tb, size_t **line_starts, size_t **line_lengths, size_t width);

void textblock_to_file(textblock *tb, ang_file *f, int indent, int wrap_at);

#endif /* INCLUDED_Z_TEXTBLOCK_H */
