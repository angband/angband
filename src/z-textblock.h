#ifndef INCLUDED_Z_TEXTBLOCK_H
#define INCLUDED_Z_TEXTBLOCK_H

#include "z-file.h"

/** Opaque text_block type */
typedef struct textblock textblock;


textblock *textblock_new(void);
void textblock_free(textblock *tb);


void textblock_append(textblock *tb, const char *fmt, ...);
void textblock_append_c(textblock *tb, byte attr, const char *fmt, ...);
void textblock_append_pict(textblock *tb, byte attr, int c);
void textblock_append_utf8(textblock *tb, const char *utf8_string);

const wchar_t *textblock_text(textblock *tb);
const byte *textblock_attrs(textblock *tb);

size_t textblock_calculate_lines(textblock *tb, size_t **line_starts, size_t **line_lengths, size_t width);

void textblock_to_file(textblock *tb, ang_file *f, int indent, int wrap_at);



extern ang_file *text_out_file;
extern void (*text_out_hook)(byte a, const char *str);
extern int text_out_wrap;
extern int text_out_indent;
extern int text_out_pad;

extern void text_out_to_file(byte attr, const char *str);
extern void text_out(const char *fmt, ...);
extern void text_out_c(byte a, const char *fmt, ...);
extern void text_out_e(const char *fmt, ...);


#endif /* INCLUDED_Z_TEXTBLOCK_H */
