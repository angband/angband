/**
 * \file z-textblock.c
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

/**
 * Text blocks now use the internal representation of display characters, i.e.
 * wchar_t, since they are expected to display straight to screen. Conversion
 * from the incoming locale-encoded format is done in textblock_vappend_c().
 */
#include "z-color.h"
#include "z-textblock.h"
#include "z-util.h"
#include "z-virt.h"
#include "z-form.h"

#define TEXTBLOCK_LEN_INITIAL		128
#define TEXTBLOCK_LEN_INCR(x)		((x) + 128)

struct textblock {
	wchar_t *text;
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
	tb->text = mem_zalloc(tb->size * sizeof *tb->text);
	tb->attrs = mem_zalloc(tb->size);

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

/**
 * Resize the internal textblock storage (if needed) to hold additional
 * characters.
 *
 * \param tb is the textblock we need to resize.
 * \param additional_size is how many characters we want to add.
 */
void textblock_resize_if_needed(textblock *tb, size_t additional_size)
{
	size_t remaining = tb->size - tb->strlen;

	/* If we need more room, reallocate it */
	if (remaining < additional_size) {
		tb->size = TEXTBLOCK_LEN_INCR(tb->strlen + additional_size);
		tb->text = mem_realloc(tb->text, tb->size * sizeof *tb->text);
		tb->attrs = mem_realloc(tb->attrs, tb->size);
	}
}

static void textblock_vappend_c(textblock *tb, byte attr, const char *fmt,
		va_list vp)
{
	size_t temp_len = TEXTBLOCK_LEN_INITIAL;
	char *temp_space = mem_zalloc(temp_len);
	int new_length;

	/* We have to format the incoming string in native (external) format
	 * re-allocating the temporary space as necessary. Once it's been
	 * successfully formatted, we can then do the conversion to wide chars
	 */
	while (1) {
		va_list args;
		size_t len;

		VA_COPY(args, vp);
		len = vstrnfmt(temp_space, temp_len, fmt, args);
		va_end(args);
		if (len < temp_len - 1) {
			/* Not using all space, therefore it completed */
			break;
		}

		temp_len = TEXTBLOCK_LEN_INCR(temp_len);
		temp_space = mem_realloc(temp_space, temp_len * sizeof *temp_space);
	}

	/* Get extent of addition in wide chars */
	new_length = text_mbstowcs(NULL, temp_space, 0);
	assert(new_length >= 0); /* If this fails, the string was badly formed */
	textblock_resize_if_needed(tb, new_length);

	/* Convert to wide chars, into the text block buffer */
	text_mbstowcs(tb->text + tb->strlen, temp_space, tb->size - tb->strlen);
	memset(tb->attrs + tb->strlen, attr, new_length);
	tb->strlen += new_length;
	mem_free(temp_space);
}

/**
 * Add a graphics tile to a text block.
 */
void textblock_append_pict(textblock *tb, byte attr, int c)
{
	textblock_resize_if_needed(tb, 1);
	tb->text[tb->strlen] = (wchar_t)c;
	tb->attrs[tb->strlen] = attr;
	tb->strlen += 1;
}

/**
 * Append a UTF-8 string to the textblock.
 *
 * This is needed in order for proper file writing. Normally, textblocks convert
 * to the system's encoding when a string is appended. However, there are still
 * some strings in the game that are imported from external files as UTF-8.
 * Instead of requiring each port to provide another converter back to UTF-8,
 * we'll just use the original strings as is.
 *
 * \param tb is the textblock we are appending to.
 * \param utf8_string is the C string that is encoded as UTF-8.
 */
void textblock_append_utf8(textblock *tb, const char *utf8_string)
{
	size_t i;
	size_t new_length = strlen(utf8_string);

	textblock_resize_if_needed(tb, new_length);

	/* Append each UTF-8 char one at a time, so we don't trigger any
	 * conversions (which would require multiple bytes). */
	for (i = 0; i < new_length; i++) {
		tb->text[tb->strlen + i] = (wchar_t)utf8_string[i];
	}

	memset(tb->attrs + tb->strlen, COLOUR_WHITE, new_length);
	tb->strlen += new_length;
}

/**
 * Add text to a text block, formatted.
 */
void textblock_append(textblock *tb, const char *fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);

	textblock_vappend_c(tb, COLOUR_WHITE, fmt, vp);

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
const wchar_t *textblock_text(textblock *tb)
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
 * Given a certain width, split a textblock into wrapped lines of text. Trailing
 * empty lines are trimmed.
 *
 * \param tb The textblock to wrap.
 * \param line_starts On return, an array (indexed by line number) of character
 *		  indexes to the text of \c tb where each line begins.
 * \param line_lengths On return, an array (indexed by line number) of line
 *		  lengths.
 * \param width The maximum permitted width of each line.
 * \return Number of lines in output.
 */
size_t textblock_calculate_lines(textblock *tb, size_t **line_starts, size_t **line_lengths, size_t width)
{
	const wchar_t *text = NULL;
	size_t text_offset = 0;
	size_t alloc_lines = 0;
	size_t total_lines = 0;
	size_t current_line_index = 0;
	size_t current_line_length = 0;
	size_t breaking_char_offset = 0;

	if (tb == NULL || line_starts == NULL || line_lengths == NULL || width == 0)
		return 0;

	text = textblock_text(tb);

	if (text == NULL || tb->strlen == 0)
		return 0;

	/* Start a line, since we have at least one. */
	new_line(line_starts, line_lengths, &alloc_lines, &total_lines, 0, 0);

	while (text_offset < tb->strlen) {
		if (text[text_offset] == L'\n') {
			(*line_lengths)[current_line_index] = current_line_length;
			new_line(line_starts, line_lengths, &alloc_lines, &total_lines, text_offset + 1, 0);
			current_line_index++;
			current_line_length = 0;
		}
		else if (text[text_offset] == L' ') {
			breaking_char_offset = text_offset;
			current_line_length++;
		}
		else {
			current_line_length++;
		}

		if (current_line_length == width) {
			/* We're out of space on the line and need to break it. */

			size_t const current_line_start = (*line_starts)[current_line_index];
			size_t next_line_start_offset = 0;
			size_t adjusted_line_length = 0;

			if (breaking_char_offset > current_line_start) {
				/* If we found a breaking character on the current line, break
				 * there and start the next line on the next character. The loop
				 * then backtracks to add the already-processed characters to
				 * the next line. */
				adjusted_line_length = breaking_char_offset - current_line_start;
				next_line_start_offset = breaking_char_offset + 1;
				text_offset = breaking_char_offset + 1;
			}
			else {
				/* There was no breaking character on the current line, so we
				 * just break at the current character. This can happen with a 
				 * word that takes up the whole line, for example. */
				adjusted_line_length = width;
				next_line_start_offset = text_offset + 1;
				text_offset++;
			}

			(*line_lengths)[current_line_index] = adjusted_line_length;
			new_line(line_starts, line_lengths, &alloc_lines, &total_lines, next_line_start_offset, 0);
			current_line_index++;
			current_line_length = 0;
		}
		else {
			/* There is still space on the line, so just add the character. */
			(*line_lengths)[current_line_index] = current_line_length;
			text_offset++;
		}
	}

	/* Trim the last line if it doesn't contain any characters. */
	if ((*line_lengths)[total_lines - 1] == 0)
		total_lines--;

	return total_lines;
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
		/* For some reason, the %*c part of the format string was still
		 * indenting, even when indent was zero */
		if (indent == 0)
			file_putf(f, "%.*ls\n", line_lengths[i], tb->text + line_starts[i]);
		else
			file_putf(f, "%*c%.*ls\n", indent, ' ', line_lengths[i],
					  tb->text + line_starts[i]);
	}

	mem_free(line_starts);
	mem_free(line_lengths);
}
