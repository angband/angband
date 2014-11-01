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
 * Given a certain width, split a textblock into wrapped lines of text.
 *
 * \returns Number of lines in output.
 */
size_t textblock_calculate_lines(textblock *tb,
		size_t **line_starts, size_t **line_lengths, size_t width)
{
	const wchar_t *text = tb->text;

	size_t cur_line = 0, n_lines = 0;

	size_t len = tb->strlen;
	size_t text_offset;

	size_t line_start = 0, line_length = 0;
	size_t word_start = 0, word_length = 0;

	assert(width > 0);

	for (text_offset = 0; text_offset < len; text_offset++) {
		if (text[text_offset] == L'\n') {
			new_line(line_starts, line_lengths, &n_lines, &cur_line,
					line_start, line_length);

			line_start = text_offset + 1;
			line_length = 0;
		} else if (text[text_offset] == L' ') {
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
			while (text[line_start + last_word_offset] != L' ')
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




/**
 * ------------------------------------------------------------------------
 * text_out()
 * ------------------------------------------------------------------------ */


/**
 * Function hook to output (colored) text to the screen or to a file.
 */
void (*text_out_hook)(byte a, const char *str);

/**
 * Hack -- Where to wrap the text when using text_out().  Use the default
 * value (for example the screen width) when 'text_out_wrap' is 0.
 */
int text_out_wrap = 0;

/**
 * Hack -- Indentation for the text when using text_out().
 */
int text_out_indent = 0;

/**
 * Hack -- Padding after wrapping
 */
int text_out_pad = 0;


/**
 * Hack - the destination file for text_out_to_file.
 */
ang_file *text_out_file = NULL;


/**
 * Write text to the given file and apply line-wrapping.
 *
 * Hook function for text_out(). Make sure that text_out_file points
 * to an open text-file.
 *
 * Long lines will be wrapped at text_out_wrap, or at column 75 if that
 * is not set; or at a newline character.  Note that punctuation can
 * sometimes be placed one column beyond the wrap limit.
 *
 * You must be careful to end all file output with a newline character
 * to "flush" the stored line position.
 */
void text_out_to_file(byte a, const char *str)
{
	const char *s;
	char buf[1024];

	/* Current position on the line */
	static int pos = 0;

	/* Wrap width */
	int wrap = (text_out_wrap ? text_out_wrap : 75);

	/* Unused parameter */
	(void)a;

	/* Copy to a rewriteable string */
 	my_strcpy(buf, str, 1024);

	/* Current location within "buf" */
 	s = buf;

	/* Process the string */
	while (*s) {
		int n = 0;
		int len = wrap - pos;
		int l_space = -1;

		/* In case we are already past the wrap point (which can happen with
		 * punctuation at the end of the line), make sure we don't overrun.
		 */
		if (len < 0)
			len = 0;

		/* If we are at the start of the line... */
		if (pos == 0) {
			int i;

			/* Output the indent */
			for (i = 0; i < text_out_indent; i++) {
				file_writec(text_out_file, ' ');
				pos++;
			}
		}

		/* Find length of line up to next newline or end-of-string */
		while ((n < len) && !((s[n] == '\n') || (s[n] == '\0'))) {
			/* Mark the most recent space in the string */
			if (s[n] == ' ') l_space = n;

			/* Increment */
			n++;
		}

		/* If we have encountered no spaces */
		if ((l_space == -1) && (n == len)) {
			/* If we are at the start of a new line */
			if (pos == text_out_indent) {
				len = n;
			} else if ((s[0] == ' ') || (s[0] == ',') || (s[0] == '.')) {
				/* HACK - Output punctuation at the end of the line */
				len = 1;
			} else {
				/* Begin a new line */
				file_writec(text_out_file, '\n');

				/* Reset */
				pos = 0;

				continue;
			}
		} else {
			/* Wrap at the newline */
			if ((s[n] == '\n') || (s[n] == '\0')) len = n;

			/* Wrap at the last space */
			else len = l_space;
		}

		/* Write that line to file */
		file_write(text_out_file, s, len);
		pos += len;

		/* Move 's' past the stuff we've written */
		s += len;

		/* If we are at the end of the string, end */
		if (*s == '\0') return;

		/* Skip newlines */
		if (*s == '\n') s++;

		/* Begin a new line */
		file_writec(text_out_file, '\n');

		/* Reset */
		pos = 0;

		/* Skip whitespace */
		while (*s == ' ') s++;
	}

	/* We are done */
	return;
}


/**
 * Output text to the screen or to a file depending on the selected
 * text_out hook.
 */
void text_out(const char *fmt, ...)
{
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Output now */
	text_out_hook(COLOUR_WHITE, buf);
}


/**
 * Output text to the screen (in color) or to a file depending on the
 * selected hook.
 */
void text_out_c(byte a, const char *fmt, ...)
{
	char buf[1024];
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Output now */
	text_out_hook(a, buf);
}

/**
 * Given a "formatted" chunk of text (i.e. one including tags like {red}{/})
 * in 'source', with starting point 'init', this finds the next section of
 * text and any tag that goes with it, return TRUE if it finds something to 
 * print.
 * 
 * If it returns TRUE, then it also fills 'text' with a pointer to the start
 * of the next printable section of text, and 'len' with the length of that 
 * text, and 'end' with a pointer to the start of the next section.  This
 * may differ from "text + len" because of the presence of tags.  If a tag
 * applies to the section of text, it returns a pointer to the start of that
 * tag in 'tag' and the length in 'taglen'.  Otherwise, 'tag' is filled with
 * NULL.
 *
 * See text_out_e for an example of its use.
 */
static bool next_section(const char *source, size_t init, const char **text,
						 size_t *len, const char **tag, size_t *taglen,
						 const char **end)
{
	const char *next;	

	*tag = NULL;
	*text = source + init;
	if (*text[0] == '\0') return FALSE;

	next = strchr(*text, '{');
	while (next)
	{
		const char *s = next + 1;

		while (*s && (isalpha((unsigned char) *s) ||
					  isspace((unsigned char) *s)))
			s++;

		/* Woo!  valid opening tag thing */
		if (*s == '}') {
			const char *close = strstr(s, "{/}");

			/* There's a closing thing, so it's valid. */
			if (close) {
				/* If this tag is at the start of the fragment */
				if (next == *text) {
					*tag = *text + 1;
					*taglen = s - *text - 1;
					*text = s + 1;
					*len = close - *text;
					*end = close + 3;
					return TRUE;
				} else {
					/* Otherwise return the chunk up to this */
					*len = next - *text;
					*end = *text + *len;
					return TRUE;
				}
			} else {
				/* No closing thing, therefore all one lump of text. */
				*len = strlen(*text);
				*end = *text + *len;
				return TRUE;
			}
		} else if (*s == '\0') {
			/* End of the string, that's fine. */
			*len = strlen(*text);
			*end = *text + *len;
			return TRUE;
		} else {
			/* An invalid tag, skip it. */
			next = next + 1;
		}

		next = strchr(next, '{');
	}

	/* Default to the rest of the string */
	*len = strlen(*text);
	*end = *text + *len;

	return TRUE;
}

/**
 * Output text to the screen or to a file depending on the
 * selected hook.  Takes strings with "embedded formatting",
 * such that something within {red}{/} will be printed in red.
 *
 * Note that such formatting will be treated as a "breakpoint"
 * for the printing, so if used within words may lead to part of the
 * word being moved to the next line.
 */
void text_out_e(const char *fmt, ...)
{
	char buf[1024];
	char smallbuf[1024];
	va_list vp;

	const char *start, *next, *text, *tag;
	size_t textlen, taglen = 0;

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Do the va_arg fmt to the buffer */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	start = buf;
	while (next_section(start, 0, &text, &textlen, &tag, &taglen, &next)) {
		int a = -1;

		memcpy(smallbuf, text, textlen);
		smallbuf[textlen] = 0;

		if (tag) {
			char tagbuffer[16];

			/* Colour names are less than 16 characters long. */
			assert(taglen < 16);

			memcpy(tagbuffer, tag, taglen);
			tagbuffer[taglen] = '\0';

			a = color_text_to_attr(tagbuffer);
		}
		
		if (a == -1) 
			a = COLOUR_WHITE;

		/* Output now */
		text_out_hook(a, smallbuf);

		start = next;
	}
}


/**
 * Write a text file from given input.
 *
 * \param path the path to write to
 * \param writer the text-writing function
 */
errr text_lines_to_file(const char *path, text_writer writer)
{
	char new_fname[1024];
	char old_fname[1024];

	ang_file *new_file;

	safe_setuid_grab();

	/* Format filenames */
	strnfmt(new_fname, sizeof(new_fname), "%s.new", path);
	strnfmt(old_fname, sizeof(old_fname), "%s.old", path);

	/* Write new file */
	new_file = file_open(new_fname, MODE_WRITE, FTYPE_TEXT);
	if (!new_file) {
		safe_setuid_drop();
		return -1;
	}

	text_out_file = new_file;
	writer(new_file);
	text_out_file = NULL;

	file_close(new_file);

	/* Move files around */
	strnfmt(old_fname, sizeof(old_fname), "%s.old", path);
	if (!file_exists(path)) {
		file_move(new_fname, path);
	} else if (file_move(path, old_fname)) {
		file_move(new_fname, path);
		file_delete(old_fname);
	} else {
		file_delete(new_fname);
	}

	safe_setuid_drop();

	return 0;
}

