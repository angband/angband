/*
 * File: z-textblock.c
 * Purpose: Text output bugger code
 *
 * Copyright (c) 2010 Andi Sidwell
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
#include "z-term.h"
#include "z-textblock.h"
#include "z-virt.h"
#include "z-form.h"

#define TEXTBLOCK_LEN_INITIAL		128
#define TEXTBLOCK_LEN_INCR(x)		(x + 128)

struct textblock {
	char *text;
	byte *attrs;

	size_t strlen;
	size_t size;
};


/**
 * Create a new textblock object and return it.
 */
textblock *textblock_new(void)
{
	textblock *tb = mem_zalloc(sizeof *tb);

	tb->size = TEXTBLOCK_LEN_INITIAL;
	tb->text = mem_zalloc(tb->size);
	tb->attrs = mem_zalloc(tb->size * sizeof *tb->attrs);

	return tb;
}

/**
 * Free a textblock object.
 */
void textblock_free(textblock *tb)
{
	mem_free(tb->text);
	mem_free(tb->attrs);
	mem_free(tb);
}

static void textblock_vappend_c(textblock *tb, byte attr, const char *fmt,
		va_list vp)
{
	while (1)
	{
		size_t len;

		size_t remaining = tb->size - tb->strlen;
		char *fmt_start = tb->text + tb->strlen;

		len = vstrnfmt(fmt_start, remaining, fmt, vp);
		if (len < remaining - 1)
		{
			byte *attr_start = tb->attrs + (tb->strlen * sizeof *tb->attrs);
			memset(attr_start, attr, len * sizeof *tb->attrs);

			tb->strlen += len;
			break;
		}

		tb->size = TEXTBLOCK_LEN_INCR(tb->size);
		tb->text = mem_realloc(tb->text, tb->size);
		tb->attrs = mem_realloc(tb->attrs, tb->size * sizeof *tb->attrs);
	}
}

/**
 * Add text to a text block, formatted.
 */
void textblock_append(textblock *tb, const char *fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);

	textblock_vappend_c(tb, TERM_WHITE, fmt, vp);

	va_end(vp);
}

/**
 * Add coloured text to a text block, formatted.
 */
void textblock_append_c(textblock *tb, byte attr, const char *fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);

	textblock_vappend_c(tb, attr, fmt, vp);

	va_end(vp);

}

/**
 * Return a pointer to the text inputted thus far.
 */
const char *textblock_text(textblock *tb)
{
	return tb->text;
}

/**
 * Return a pointer to the text attrs.
 */
const byte *textblock_attrs(textblock *tb)
{
	return tb->attrs;
}
