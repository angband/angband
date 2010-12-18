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
		va_list args;
		size_t len;

		size_t remaining = tb->size - tb->strlen;
		char *fmt_start = tb->text + tb->strlen;

		VA_COPY(args, vp);
		len = vstrnfmt(fmt_start, remaining, fmt, args);
		va_end(args);
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

static void new_line(size_t **line_starts, size_t **line_lengths,
		size_t *n_lines, size_t *cur_line,
		size_t start, size_t len)
{
	if (*cur_line == *n_lines) {
		/* this number is not arbitrary: it's the height of a "standard" term */
		(*n_lines) += 24;

		*line_starts = mem_realloc(*line_starts,
				*n_lines * sizeof **line_starts);
		*line_lengths = mem_realloc(*line_lengths,
				*n_lines * sizeof **line_lengths);
	}

	(*line_starts)[*cur_line] = start;
	(*line_lengths)[*cur_line] = len;

	(*cur_line)++;
}

/**
 * Given a certain width, split a textblock into wrapped lines of text.
 *
 * \returns Number of lines in output.
 */
size_t textblock_calculate_lines(textblock *tb,
		size_t **line_starts, size_t **line_lengths, size_t width)
{
	const char *text = tb->text;

	size_t cur_line = 0, n_lines = 0;

	size_t len = strlen(text);
	size_t text_offset;

	size_t line_start = 0, line_length = 0;
	size_t word_start = 0, word_length = 0;

	assert(width > 0);

	for (text_offset = 0; text_offset < len; text_offset++) {
		if (text[text_offset] == '\n') {
			new_line(line_starts, line_lengths, &n_lines, &cur_line,
					line_start, line_length);

			line_start = text_offset + 1;
			line_length = 0;
		} else if (text[text_offset] == ' ') {
			line_length++;

			word_start = line_length;
			word_length = 0;
		} else {
			line_length++;
			word_length++;
		}

		/* special case: if we have a very long word, just slice it */
		if (word_length == width) {
			new_line(line_starts, line_lengths, &n_lines, &cur_line,
					line_start, line_length);

			line_start += line_length;
			line_length = 0;
		}

		/* normal wrapping: wrap text at last word */
		if (line_length == width) {
			size_t last_word_offset = word_start;
			while (text[line_start + last_word_offset] != ' ')
				last_word_offset--;

			new_line(line_starts, line_lengths, &n_lines, &cur_line,
					line_start, last_word_offset);

			line_start += word_start;
			line_length = word_length;
		}
	}

	return cur_line;
}

/**
 * Output a textblock to file.
 */
void textblock_to_file(textblock *tb, ang_file *f, int indent, int wrap_at)
{
	size_t *line_starts = NULL;
	size_t *line_lengths = NULL;

	size_t n_lines, i;

	int width = wrap_at - indent;
	assert(width > 0);

	n_lines = textblock_calculate_lines(tb, &line_starts, &line_lengths, width);

	for (i = 0; i < n_lines; i++) {
		file_putf(f, "%*c%.*s\n",
				indent, ' ',
				line_lengths[i], tb->text + line_starts[i]);
	}
}
