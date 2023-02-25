/**
 * \file datafile.c
 * \brief Angband data file reading and writing routines.
 *
 * Copyright (c) 2017 Nick McConnell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 *
 * This file deals with Angband specific functions for reading and writing
 * of data files; basic parser functions are in parser.c.
 */

#include "angband.h"
#include "datafile.h"
#include "game-world.h"
#include "init.h"
#include "parser.h"

/**
 * Hold a prefix to distinguish files from different users when the archive
 * directory is shared.
 */
static char *archive_user_pfx = NULL;

const char *parser_error_str[PARSE_ERROR_MAX] = {
	#define PARSE_ERROR(a, b) b,
	#include "list-parser-errors.h"
	#undef PARSE_ERROR
};

/**
 * ------------------------------------------------------------------------
 * Angband datafile parsing routines
 * ------------------------------------------------------------------------ */

static void print_error(struct file_parser *fp, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	msg("Parse error in %s line %d column %d: %s: %s", fp->name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	event_signal(EVENT_MESSAGE_FLUSH);
	quit_fmt("Parse error in %s line %d column %d.", fp->name, s.line, s.col);
}

errr run_parser(struct file_parser *fp) {
	struct parser *p = fp->init();
	errr r;
	if (!p) {
		return PARSE_ERROR_GENERIC;
	}
	r = fp->run(p);
	if (r) {
		print_error(fp, p);
		return r;
	}
	r = fp->finish(p);
	if (r) {
		msg("Parser finish error in %s: %s", fp->name,
			(r > 0 && r < PARSE_ERROR_MAX) ?
			parser_error_str[r] : "unspecified error");
		event_signal(EVENT_MESSAGE_FLUSH);
		quit_fmt("Parser finish error in %s.", fp->name);
	}
	return r;
}

/**
 * The basic file parsing function.  Attempt to load filename through
 * parser and perform a quit if the file is not found.
 */
errr parse_file_quit_not_found(struct parser *p, const char *filename) {
	errr parse_err = parse_file(p, filename);

	if (parse_err == PARSE_ERROR_NO_FILE_FOUND)
		quit(format("Cannot open '%s.txt'", filename));

	return parse_err;
}

/**
 * The basic file parsing function.
 */
errr parse_file(struct parser *p, const char *filename) {
	char path[1024];
	char buf[1024];
	ang_file *fh;
	errr r = 0;

	/* The player can put a customised file in the user directory */
	path_build(path, sizeof(path), ANGBAND_DIR_USER, format("%s.txt",
															filename));
	fh = file_open(path, MODE_READ, FTYPE_TEXT);

	/* If no custom file, just load the standard one */
	if (!fh) {
		path_build(path, sizeof(path), ANGBAND_DIR_GAMEDATA,
				   format("%s.txt", filename));
		fh = file_open(path, MODE_READ, FTYPE_TEXT);
	}

	/* File wasn't found, return the error */
	if (!fh)
		return PARSE_ERROR_NO_FILE_FOUND;

	/* Parse it */
	while (file_getl(fh, buf, sizeof(buf))) {
		r = parser_parse(p, buf);
		if (r)
			break;
	}
	file_close(fh);
	return r;
}

void cleanup_parser(struct file_parser *fp)
{
	fp->cleanup();
}

int lookup_flag(const char **flag_table, const char *flag_name) {
	int i = FLAG_START;

	while (flag_table[i] && !streq(flag_table[i], flag_name))
		i++;

	/* End of table reached without match */
	if (!flag_table[i]) i = FLAG_END;

	return i;
}

int code_index_in_array(const char *code_name[], const char *code)
{
	int i = 0;

	while (code_name[i]) {
		if (streq(code_name[i], code)) {
			return i;
		}
		i++;
	}

	return -1;
}

/**
 * Gets a name and argument for a value expression of the form NAME[arg]
 * \param name_and_value is the expression
 * \param string is the random value string to return (NULL if not required)
 * \param num is the integer to return (NULL if not required)
 */
static bool find_value_arg(char *value_name, char *string, int *num)
{
	char *t;

	/* Find the first bracket */
	for (t = value_name; *t && (*t != '['); ++t)
		;

	/* Choose random_value value or int or fail */
	if (string) {
		/* Get the dice */
		if (1 != sscanf(t + 1, "%s", string))
			return false;
	} else if (num) {
		/* Get the value */
		if (1 != sscanf(t + 1, "%d", num))
			return false;
	} else return false;

	/* Terminate the string */
	*t = '\0';

	/* Success */
	return true;
}

/**
 * Get the random value argument from a value expression and put it into the
 * appropriate place in an array
 * \param value the target array of values
 * \param value_type the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_rand_value(random_value *value, const char **value_type,
					 const char *name_and_value)
{
	int i = 0;
	char value_name[80];
	char dice_string[40];
	dice_t *dice;

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, dice_string, NULL))
		return PARSE_ERROR_INVALID_VALUE;

	dice = dice_new();

	while (value_type[i] && !streq(value_type[i], value_name))
		i++;

	if (value_type[i]) {
		if (!dice_parse_string(dice, dice_string)) {
			dice_free(dice);
			return PARSE_ERROR_NOT_RANDOM;
		}
		dice_random_value(dice, &value[i]);
	}

	dice_free(dice);

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a value expression and put it into the
 * appropriate place in an array
 * \param value the target array of integers
 * \param value_type the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_int_value(int *value, const char **value_type,
					const char *name_and_value)
{
	int val, i = 0;
	char value_name[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, &val))
		return PARSE_ERROR_INVALID_VALUE;

	while (value_type[i] && !streq(value_type[i], value_name))
		i++;

	if (value_type[i])
		value[i] = val;

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a value expression and the index in the
 * value_type array of the suffix used to build the value string
 * \param value the integer value
 * \param index the information on where to put it (eg array index)
 * \param value_type the variable suffix of the possible value strings
 * \param prefix the constant prefix of the possible value strings
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_index_and_int(int *value, int *index, const char **value_type,
						const char *prefix, const char *name_and_value)
{
	int i;
	char value_name[80];
	char value_string[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, value))
		return PARSE_ERROR_INVALID_VALUE;

	/* Compose the value string and look for it */
	for (i = 0; value_type[i]; i++) {
		my_strcpy(value_string, prefix, sizeof(value_string));
		my_strcat(value_string, value_type[i],
				  sizeof(value_string) - strlen(value_string));
		if (streq(value_string, value_name)) break;
	}

	if (value_type[i])
		*index = i;

	return value_type[i] ? PARSE_ERROR_NONE : PARSE_ERROR_INTERNAL;
}

/**
 * Get the integer argument from a slay value expression and the monster base
 * name it is slaying
 * \param value the integer value
 * \param base the monster base name
 * \param name_and_value the value expression being matched
 * \return 0 if successful, otherwise an error value
 */
errr grab_base_and_int(int *value, char **base, const char *name_and_value)
{
	char value_name[80];

	/* Get a rewritable string */
	my_strcpy(value_name, name_and_value, strlen(name_and_value));

	/* Parse the value expression */
	if (!find_value_arg(value_name, NULL, value))
		return PARSE_ERROR_INVALID_VALUE;

	/* Must be a slay */
	if (strncmp(value_name, "SLAY_", 5))
		return PARSE_ERROR_INVALID_VALUE;
	else
		*base = string_make(value_name + 5);

	/* If we've got this far, assume it's a valid monster base name */
	return PARSE_ERROR_NONE;
}

errr grab_name(const char *from, const char *what, const char *list[], int max,
			   int *num)
{
	int i;

	/* Scan list */
	for (i = 0; i < max; i++) {
		if (streq(what, list[i])) {
			*num = i;
			return PARSE_ERROR_NONE;
		}
	}

	/* Oops */
	msg("Unknown %s '%s'.", from, what);

	/* Error */
	return PARSE_ERROR_GENERIC;
}

errr grab_flag(bitflag *flags, const size_t size, const char **flag_table, const char *flag_name) {
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_on(flags, size, flag);

	return 0;
}

errr remove_flag(bitflag *flags, const size_t size, const char **flag_table,
				 const char *flag_name) {
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_off(flags, size, flag);

	return 0;
}


/**
 * ------------------------------------------------------------------------
 * Angband datafile writing routines
 * ------------------------------------------------------------------------ */
/**
 * Write the flag lines for a set of flags.
 */
void write_flags(ang_file *fff, const char *intro_text, const bitflag *flags,
					   int flag_size, const char *names[])
{
	int flag;
	char buf[1024] = "";
	int pointer = 0;

	/* Write flag name list */
	for (flag = flag_next(flags, flag_size, FLAG_START); flag != FLAG_END;
		 flag = flag_next(flags, flag_size, flag + 1)) {

		/* Write the flags, keeping track of where we are */
		if (strlen(buf)) {
			my_strcat(buf, " | ", sizeof(buf));
			pointer += 3;
		}

		/* If no name, we're past the real flags */
		if (!names[flag]) break;
		my_strcat(buf, names[flag], sizeof(buf));
		pointer += strlen(names[flag]);

		/* Move to a new line if this one is long enough */
		if (pointer >= 60) {
			file_putf(fff, "%s%s\n", intro_text, buf);
			my_strcpy(buf, "", sizeof(buf));
			pointer = 0;
		}
	}

	/* Print remaining flags if any */
	if (pointer)
		file_putf(fff, "%s%s\n", intro_text, buf);
}

/**
 * Write value lines for a set of modifiers.
 */
void write_mods(ang_file *fff, const int values[])
{
	size_t i;
	char buf[1024] = "";
	int pointer = 0;

	static const char *obj_mods[] = {
		#define STAT(a) #a,
		#include "list-stats.h"
		#undef STAT
		#define OBJ_MOD(a) #a,
		#include "list-object-modifiers.h"
		#undef OBJ_MOD
		NULL
	};

	/* Write value list */
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		/* If no value, don't write */
		if (values[i] == 0) continue;

		/* If this line contains something, write a divider */
		if (strlen(buf)) {
			my_strcat(buf, " | ", sizeof(buf));
			pointer += 3;
		}

		/* Write the name and value */
		my_strcat(buf, obj_mods[i], sizeof(buf));
		pointer += strlen(obj_mods[i]);
		my_strcat(buf, format("[%d]", values[i]), sizeof(buf));
		pointer += 5;

		/* Move to a new line if this one is long enough */
		if (pointer >= 60) {
			file_putf(fff, "%s%s\n", "values:", buf);
			my_strcpy(buf, "", sizeof(buf));
			pointer = 0;
		}
	}

	/* Print remaining values if any */
	if (pointer)
		file_putf(fff, "%s%s\n", "values:", buf);
}

/**
 * Write value lines for a set of modifiers.
 */
void write_elements(ang_file *fff, const struct element_info *el_info)
{
	size_t i;
	char buf[1024] = "";
	int pointer = 0;

	static const char *element_names[] = {
		#define ELEM(a) #a,
		#include "list-elements.h"
		#undef ELEM
		NULL
	};

	/* Write value list */
	for (i = 0; i < ELEM_MAX; i++) {
		/* If no value, don't write */
		if (el_info[i].res_level == 0) continue;

		/* If this line contains something, write a divider */
		if (strlen(buf)) {
			my_strcat(buf, " | ", sizeof(buf));
			pointer += 3;
		}

		/* Write the name and value */
		my_strcat(buf, format("RES_%s", element_names[i]), sizeof(buf));
		pointer += strlen(element_names[i]) + 4;
		my_strcat(buf, format("[%d]", el_info[i].res_level), sizeof(buf));
		pointer += 5;

		/* Move to a new line if this one is long enough */
		if (pointer >= 60) {
			file_putf(fff, "%s%s\n", "values:", buf);
			my_strcpy(buf, "", sizeof(buf));
			pointer = 0;
		}
	}

	/* Print remaining values if any */
	if (pointer)
		file_putf(fff, "%s%s\n", "values:", buf);
}

/**
 * Set the prefix to use for the current user when archiving files.
 * \param pfx Is the new prefix to use.  May be NULL which is treated
 * like an empty string.
 */
void set_archive_user_prefix(const char *pfx)
{
	string_free(archive_user_pfx);
	archive_user_pfx = string_make(pfx);
}

/**
 * Archive a data file from ANGBAND_DIR_USER into ANGBAND_DIR_ARCHIVE
 */
void file_archive(const char *fname, const char *append)
{
	char arch[1024];
	char old[1024];
	int i, max_arch = 10000;

	/* Add a suffix to the filename, custom if requested */
	if (append) {
		path_build(arch, sizeof(arch), ANGBAND_DIR_ARCHIVE,
			format("%s%s_%s.txt",
			(archive_user_pfx) ? archive_user_pfx : "", fname,
			append));
	} else {
		/* Check the indices of existing archived files, get the next one */
		for (i = 1; i < max_arch; i++) {
			path_build(arch, sizeof(arch), ANGBAND_DIR_ARCHIVE,
				format("%s%s_%d.txt",
				(archive_user_pfx) ? archive_user_pfx : "",
				fname, i));
			if (!file_exists(arch)) break;
			my_strcpy(arch, "", sizeof(arch));
		} 
	}

	/* Move the file */
	path_build(old, sizeof(old), ANGBAND_DIR_USER, format("%s.txt", fname));
	safe_setuid_grab();
	file_move(old, arch);
	safe_setuid_drop();
}

/**
 * Check if an archived randart file for the current seed exists
 */
bool randart_file_exists(void)
{
	char path[1024];

	/* Get the randart filename and path */
	path_build(path, sizeof(path), ANGBAND_DIR_ARCHIVE,
			   format("%srandart_%08lx.txt", (archive_user_pfx) ?
					archive_user_pfx : "",
					(unsigned long)seed_randart));
	return file_exists(path);
}

/**
 * Prepare the randart file for the current seed to be loaded
 */
void activate_randart_file(void)
{
	char new[1024];
	char old[1024];

	/* Get the randart filename and path */
	path_build(old, sizeof(old), ANGBAND_DIR_ARCHIVE,
		format("%srandart_%08lx.txt",
		(archive_user_pfx) ? archive_user_pfx : "",
		(unsigned long)seed_randart));

	/* Move it into place */
	path_build(new, sizeof(new), ANGBAND_DIR_USER, "randart.txt");
	safe_setuid_grab();
	file_move(old, new);
	safe_setuid_drop();
}

/**
 * Move the randart file to the archive directory
 */
void deactivate_randart_file(void)
{
	char buf[10];
	strnfmt(buf, 9, "%08lx", (unsigned long)seed_randart);
	file_archive("randart", buf);
}
