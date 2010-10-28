/* File: init.h */

/*
 * Copyright (c) 2000 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_INIT_H
#define INCLUDED_INIT_H

#include "h-basic.h"
#include "z-bitflag.h"
#include "z-file.h"
#include "z-rand.h"
#include "parser.h"

typedef struct header header;

typedef errr (*parse_info_txt_func)(char *buf, header *head);
typedef errr (*eval_info_post_func)(header *head);
typedef errr (*emit_info_txt_index_func)(ang_file *fp, header *head, int i);
typedef errr (*emit_info_txt_always_func)(ang_file *fp, header *head);

struct header
{
	byte v_major;		/* Version -- major */
	byte v_minor;		/* Version -- minor */
	byte v_patch;		/* Version -- patch */
	byte v_extra;		/* Version -- extra */


	u16b info_num;		/* Number of "info" records */

	u16b info_len;		/* Size of each "info" record */


	u32b head_size;		/* Size of the "header" in bytes */

	u32b info_size;		/* Size of the "info" array in bytes */

	u32b name_size;		/* Size of the "name" array in bytes */

	u32b text_size;		/* Size of the "text" array in bytes */

	void *info_ptr;
	char *name_ptr;
	char *text_ptr;

	parse_info_txt_func parse_info_txt;
	eval_info_post_func eval_info_post;	/* Evaluate after parsing */
	emit_info_txt_index_func emit_info_txt_index;
	emit_info_txt_always_func emit_info_txt_always;
	
};

extern errr init_info_txt(ang_file *fp, char *buf, header *head,
                          parse_info_txt_func parse_info_txt_line);
extern errr eval_info(eval_info_post_func eval_info_process, header *head);

extern errr emit_info_txt(ang_file *fp, ang_file *template, char *buf, header *head,
   emit_info_txt_index_func emit_info_txt_index, emit_info_txt_always_func emit_info_txt_always);

#ifdef TEST
extern struct parser *init_parse_a(void);
extern struct parser *init_parse_c(void);
extern struct parser *init_parse_e(void);
extern struct parser *init_parse_f(void);
extern struct parser *init_parse_h(void);
extern struct parser *init_parse_k(void);
extern struct parser *init_parse_p(void);
extern struct parser *init_parse_r(void);
extern struct parser *init_parse_v(void);
extern struct parser *init_parse_z(void);
extern struct parser *init_parse_flavor(void);
extern struct parser *init_parse_names(void);
#endif

extern errr parse_file(struct parser *p, const char *filename);

extern errr parse_s_info(char *buf, header *head);

extern errr emit_r_info_index(ang_file *fp, header *head, int i);

extern void init_file_paths(const char *config, const char *lib, const char *data);
extern void create_needed_dirs(void);
extern bool init_angband(void);
extern void cleanup_angband(void);


/*
 * Error tracking
 */
extern int error_idx;
extern int error_line;

extern header flavor_head;

#endif /* INCLUDED_INIT_H */
