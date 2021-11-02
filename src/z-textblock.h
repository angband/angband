/**
 * \file z-textblock.h
 * \brief Text output bugger (?NRM) code
 *
 * Copyright (c) 2010 Andi Sidwell
 * Copyright (c) 2011 Peter Denison
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */

#ifndef INCLUDED_Z_TEXTBLOCK_H
#define INCLUDED_Z_TEXTBLOCK_H

#include "z-file.h"

/** Opaque text_block type */
typedef struct textblock textblock;


textblock *textblock_new(void);
void textblock_free(textblock *tb);


void textblock_append(textblock *tb, const char *fmt, ...);
void textblock_append_c(textblock *tb, uint8_t attr, const char *fmt, ...);
void textblock_append_pict(textblock *tb, uint8_t attr, int c);
void textblock_append_textblock(textblock *tb, const textblock *tba);

const wchar_t *textblock_text(textblock *tb);
const uint8_t *textblock_attrs(textblock *tb);

size_t textblock_calculate_lines(textblock *tb, size_t **line_starts,
								 size_t **line_lengths, size_t width);

void textblock_to_file(textblock *tb, ang_file *f, int indent, int wrap_at);

extern ang_file *text_out_file;
extern void (*text_out_hook)(uint8_t a, const char *str);
extern int text_out_wrap;
extern int text_out_indent;
extern int text_out_pad;

extern void text_out_to_file(uint8_t attr, const char *str);
extern void text_out(const char *fmt, ...);
extern void text_out_c(uint8_t a, const char *fmt, ...);
extern void text_out_e(const char *fmt, ...);

typedef void (*text_writer)(ang_file *f);
errr text_lines_to_file(const char *path, text_writer writer);

#endif /* INCLUDED_Z_TEXTBLOCK_H */
