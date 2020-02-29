/**
 * \file datafile.h
 * \brief Data file writing routines.
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
 */

#ifndef DATAFILE_H
#define DATAFILE_H

#include "object.h"
#include "parser.h"

struct file_parser {
	const char *name;
	struct parser *(*init)(void);
	errr (*run)(struct parser *p);
	errr (*finish)(struct parser *p);
	void (*cleanup)(void);
};

extern const char *parser_error_str[PARSE_ERROR_MAX];

errr run_parser(struct file_parser *fp);
errr parse_file_quit_not_found(struct parser *p, const char *filename);
errr parse_file(struct parser *p, const char *filename);
void cleanup_parser(struct file_parser *fp);
int lookup_flag(const char **flag_table, const char *flag_name);
int code_index_in_array(const char *code_name[], const char *code);
errr grab_rand_value(random_value *value, const char **value_type,
					 const char *name_and_value);
errr grab_int_value(int *value, const char **value_type,
					const char *name_and_value);
errr grab_index_and_int(int *value, int *index, const char **value_type,
						const char *prefix, const char *name_and_value);
errr grab_base_and_int(int *value, char **base, const char *name_and_value);
errr grab_name(const char *from, const char *what, const char *list[], int max,
			   int *num);
errr grab_flag(bitflag *flags, const size_t size, const char **flag_table,
			   const char *flag_name);
errr remove_flag(bitflag *flags, const size_t size, const char **flag_table,
				 const char *flag_name);

void write_flags(ang_file *fff, const char *intro_text, bitflag *flags,
				 int flag_size, const char *names[]);
void write_mods(ang_file *fff, const int values[]);
void write_elements(ang_file *fff, const struct element_info *el_info);
void file_archive(char *fname, char *append);
void activate_randart_file(void);
void deactivate_randart_file(void);

#endif /* !DATAFILE_H */
