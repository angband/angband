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


/*
 * Parse errors
 */
#define PARSE_ERROR_GENERIC                  1
#define PARSE_ERROR_OBSOLETE_FILE            2
#define PARSE_ERROR_MISSING_RECORD_HEADER    3
#define PARSE_ERROR_NON_SEQUENTIAL_RECORDS   4
#define PARSE_ERROR_INVALID_FLAG             5
#define PARSE_ERROR_UNDEFINED_DIRECTIVE      6
#define PARSE_ERROR_OUT_OF_MEMORY            7
#define PARSE_ERROR_OUT_OF_BOUNDS            8
#define PARSE_ERROR_TOO_FEW_ARGUMENTS        9
#define PARSE_ERROR_TOO_MANY_ARGUMENTS      10
#define PARSE_ERROR_TOO_MANY_ALLOCATIONS    11
#define PARSE_ERROR_INVALID_SPELL_FREQ      12
#define PARSE_ERROR_INVALID_ITEM_NUMBER     13
#define PARSE_ERROR_TOO_MANY_ENTRIES        14
#define PARSE_ERROR_VAULT_TOO_BIG           15

#define PARSE_ERROR_MAX                     16


typedef struct header header;

typedef errr (*parse_info_txt_func)(char *buf, header *head);

/*
 * Template file header information (see "init.c").  16 bytes.
 *
 * Note that the sizes of many of the "arrays" are between 32768 and
 * 65535, and so we must use "unsigned" values to hold the "sizes" of
 * these arrays below.  Normally, I try to avoid using unsigned values,
 * since they can cause all sorts of bizarre problems, but I have no
 * choice here, at least, until the "race" array is split into "normal"
 * and "unique" monsters, which may or may not actually help.
 *
 * Note that, on some machines, for example, the Macintosh, the standard
 * "read()" and "write()" functions cannot handle more than 32767 bytes
 * at one time, so we need replacement functions, see "util.c" for details.
 *
 * Note that, on some machines, for example, the Macintosh, the standard
 * "malloc()" function cannot handle more than 32767 bytes at one time,
 * but we may assume that the "ralloc()" function can handle up to 65535
 * butes at one time.  We should not, however, assume that the "ralloc()"
 * function can handle more than 65536 bytes at a time, since this might
 * result in segmentation problems on certain older machines, and in fact,
 * we should not assume that it can handle exactly 65536 bytes at a time,
 * since the internal functions may use an unsigned short to specify size.
 *
 * In general, these problems occur only on machines (such as most personal
 * computers) which use 2 byte "int" values, and which use "int" for the
 * arguments to the relevent functions.
 */
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
};

extern errr init_info_txt(FILE *fp, char *buf, header *head,
                          parse_info_txt_func parse_info_txt_line);

#ifdef ALLOW_TEMPLATES
extern errr parse_z_info(char *buf, header *head);
extern errr parse_v_info(char *buf, header *head);
extern errr parse_f_info(char *buf, header *head);
extern errr parse_k_info(char *buf, header *head);
extern errr parse_a_info(char *buf, header *head);
extern errr parse_e_info(char *buf, header *head);
extern errr parse_r_info(char *buf, header *head);
extern errr parse_p_info(char *buf, header *head);
extern errr parse_c_info(char *buf, header *head);
extern errr parse_h_info(char *buf, header *head);
extern errr parse_b_info(char *buf, header *head);
extern errr parse_g_info(char *buf, header *head);
extern errr parse_flavor_info(char *buf, header *head);

/*
 * Error tracking
 */
extern int error_idx;
extern int error_line;

#endif /* ALLOW_TEMPLATES */


/*
 * File headers
 */
extern header z_head;
extern header v_head;
extern header f_head;
extern header k_head;
extern header a_head;
extern header e_head;
extern header r_head;
extern header p_head;
extern header h_head;
extern header b_head;
extern header g_head;
extern header flavor_head;

#endif /* INCLUDED_INIT_H */
