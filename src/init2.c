/* File: init2.c */


/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * The "init1.c" file is used only to parse the ascii template files,
 * to create the binary image files.  If you include the binary image
 * files instead of the ascii template files, then you can undefine
 * "ALLOW_TEMPLATES", saving about 20K by removing "init1.c".  Note
 * that the binary image files are extremely system dependant.
 */



/*
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variable.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Mega-Hack -- support fat raw files under NEXTSTEP, using special
 * "suffixed" directories for the "ANGBAND_DIR_DATA" directory, but
 * requiring the directories to be created by hand by the user.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(char *path)
{
	char *tail;


	/*** Free everything ***/

	/* Free the main path */
	string_free(ANGBAND_DIR);

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
	string_free(ANGBAND_DIR_DATA);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);


	/*** Prepare the "path" ***/

	/* Hack -- save the main directory */
	ANGBAND_DIR = string_make(path);

	/* Prepare to append to the Base Path */
	tail = path + strlen(path);


#ifdef VM


	/*** Use "flat" paths with VM/ESA ***/

	/* Use "blank" path names */
	ANGBAND_DIR_APEX = string_make("");
	ANGBAND_DIR_BONE = string_make("");
	ANGBAND_DIR_DATA = string_make("");
	ANGBAND_DIR_EDIT = string_make("");
	ANGBAND_DIR_FILE = string_make("");
	ANGBAND_DIR_HELP = string_make("");
	ANGBAND_DIR_INFO = string_make("");
	ANGBAND_DIR_SAVE = string_make("");
	ANGBAND_DIR_USER = string_make("");
	ANGBAND_DIR_XTRA = string_make("");


#else /* VM */


	/*** Build the sub-directory names ***/

	/* Build a path name */
	strcpy(tail, "apex");
	ANGBAND_DIR_APEX = string_make(path);

	/* Build a path name */
	strcpy(tail, "bone");
	ANGBAND_DIR_BONE = string_make(path);

	/* Build a path name */
	strcpy(tail, "data");
	ANGBAND_DIR_DATA = string_make(path);

	/* Build a path name */
	strcpy(tail, "edit");
	ANGBAND_DIR_EDIT = string_make(path);

	/* Build a path name */
	strcpy(tail, "file");
	ANGBAND_DIR_FILE = string_make(path);

	/* Build a path name */
	strcpy(tail, "help");
	ANGBAND_DIR_HELP = string_make(path);

	/* Build a path name */
	strcpy(tail, "info");
	ANGBAND_DIR_INFO = string_make(path);

	/* Build a path name */
	strcpy(tail, "save");
	ANGBAND_DIR_SAVE = string_make(path);

	/* Build a path name */
	strcpy(tail, "user");
	ANGBAND_DIR_USER = string_make(path);

	/* Build a path name */
	strcpy(tail, "xtra");
	ANGBAND_DIR_XTRA = string_make(path);

#endif /* VM */


#ifdef NeXT

	/* Allow "fat binary" usage with NeXT */
	if (TRUE)
	{
		cptr next = NULL;

# if defined(m68k)
		next = "m68k";
# endif

# if defined(i386)
		next = "i386";
# endif

# if defined(sparc)
		next = "sparc";
# endif

# if defined(hppa)
		next = "hppa";
# endif

		/* Use special directory */
		if (next)
		{
			/* Forget the old path name */
			string_free(ANGBAND_DIR_DATA);

			/* Build a new path name */
			sprintf(tail, "data-%s", next);
			ANGBAND_DIR_DATA = string_make(path);
		}
	}

#endif /* NeXT */

}



#ifdef ALLOW_TEMPLATES


/*
 * Hack -- help give useful error messages
 */
s16b error_idx;
s16b error_line;


/*
 * Standard error message text
 */
static cptr err_str[PARSE_ERROR_MAX] =
{
	NULL,
	"parse error",
	"obsolete file",
	"missing record header",
	"non-sequential records",
	"invalid flag specification",
	"undefined directive",
	"out of memory",
	"value out of bounds",
	"too few arguments",
	"too many arguments",
};


#endif /* ALLOW_TEMPLATES */



/*** Initialize from binary image files ***/


/*
 * Initialize the "z_info" array, by parsing a binary "image" file
 */
static errr init_z_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != z_head->v_major) ||
	    (test.v_minor != z_head->v_minor) ||
	    (test.v_patch != z_head->v_patch) ||
	    (test.v_extra != z_head->v_extra) ||
	    (test.info_num != z_head->info_num) ||
	    (test.info_len != z_head->info_len) ||
	    (test.head_size != z_head->head_size) ||
	    (test.info_size != z_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*z_head) = test;


	/* Allocate the "z_info" array */
	C_MAKE(z_info, z_head->info_num, maxima);

	/* Read the "z_info" array */
	fd_read(fd, (char*)(z_info), z_head->info_size);


	/* Success */
	return (0);
}



/*
 * Initialize the "z_info" array
 */
static errr init_z_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(z_head, header);

	/* Save the "version" */
	z_head->v_major = VERSION_MAJOR;
	z_head->v_minor = VERSION_MINOR;
	z_head->v_patch = VERSION_PATCH;
	z_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	z_head->info_num = 1;
	z_head->info_len = sizeof(maxima);

	/* Save the size of "z_head" and "z_info" */
	z_head->head_size = sizeof(header);
	z_head->info_size = z_head->info_num * z_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "z_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "z_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_z_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "z_info" array */
	C_MAKE(z_info, z_head->info_num, maxima);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "z_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'z_info.txt' file.");

	/* Parse the file */
	err = init_z_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'z_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'z_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "z_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(z_head), z_head->head_size);

		/* Dump the "z_info" array */
		fd_write(fd, (char*)(z_info), z_head->info_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "z_info" array */
	C_KILL(z_info, z_head->info_num, maxima);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "z_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'z_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_z_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'z_info.raw' file.");

	/* Success */
	return (0);
}


/*
 * Initialize the "f_info" array, by parsing a binary "image" file
 */
static errr init_f_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != f_head->v_major) ||
	    (test.v_minor != f_head->v_minor) ||
	    (test.v_patch != f_head->v_patch) ||
	    (test.v_extra != f_head->v_extra) ||
	    (test.info_num != f_head->info_num) ||
	    (test.info_len != f_head->info_len) ||
	    (test.head_size != f_head->head_size) ||
	    (test.info_size != f_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*f_head) = test;


	/* Allocate the "f_info" array */
	C_MAKE(f_info, f_head->info_num, feature_type);

	/* Read the "f_info" array */
	fd_read(fd, (char*)(f_info), f_head->info_size);


	/* Allocate the "f_name" array */
	C_MAKE(f_name, f_head->name_size, char);

	/* Read the "f_name" array */
	fd_read(fd, (char*)(f_name), f_head->name_size);


#ifndef DELAY_LOAD_F_TEXT

	/* Allocate the "f_text" array */
	C_MAKE(f_text, f_head->text_size, char);

	/* Read the "f_text" array */
	fd_read(fd, (char*)(f_text), f_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "f_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_f_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(f_head, header);

	/* Save the "version" */
	f_head->v_major = VERSION_MAJOR;
	f_head->v_minor = VERSION_MINOR;
	f_head->v_patch = VERSION_PATCH;
	f_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	f_head->info_num = z_info->f_max;
	f_head->info_len = sizeof(feature_type);

	/* Save the size of "f_head" and "f_info" */
	f_head->head_size = sizeof(header);
	f_head->info_size = f_head->info_num * f_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "f_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_f_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "f_info" array */
	C_MAKE(f_info, f_head->info_num, feature_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(f_name, z_info->fake_name_size, char);
	C_MAKE(f_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "f_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'f_info.txt' file.");

	/* Parse the file */
	err = init_f_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'f_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'f_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(f_head), f_head->head_size);

		/* Dump the "f_info" array */
		fd_write(fd, (char*)(f_info), f_head->info_size);

		/* Dump the "f_name" array */
		fd_write(fd, (char*)(f_name), f_head->name_size);

		/* Dump the "f_text" array */
		fd_write(fd, (char*)(f_text), f_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "f_info" array */
	C_KILL(f_info, f_head->info_num, feature_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(f_name, z_info->fake_name_size, char);
	C_KILL(f_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'f_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_f_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'f_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "k_info" array, by parsing a binary "image" file
 */
static errr init_k_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != k_head->v_major) ||
	    (test.v_minor != k_head->v_minor) ||
	    (test.v_patch != k_head->v_patch) ||
	    (test.v_extra != k_head->v_extra) ||
	    (test.info_num != k_head->info_num) ||
	    (test.info_len != k_head->info_len) ||
	    (test.head_size != k_head->head_size) ||
	    (test.info_size != k_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*k_head) = test;


	/* Allocate the "k_info" array */
	C_MAKE(k_info, k_head->info_num, object_kind);

	/* Read the "k_info" array */
	fd_read(fd, (char*)(k_info), k_head->info_size);


	/* Allocate the "k_name" array */
	C_MAKE(k_name, k_head->name_size, char);

	/* Read the "k_name" array */
	fd_read(fd, (char*)(k_name), k_head->name_size);


#ifndef DELAY_LOAD_K_TEXT

	/* Allocate the "k_text" array */
	C_MAKE(k_text, k_head->text_size, char);

	/* Read the "k_text" array */
	fd_read(fd, (char*)(k_text), k_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "k_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_k_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(k_head, header);

	/* Save the "version" */
	k_head->v_major = VERSION_MAJOR;
	k_head->v_minor = VERSION_MINOR;
	k_head->v_patch = VERSION_PATCH;
	k_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	k_head->info_num = z_info->k_max;
	k_head->info_len = sizeof(object_kind);

	/* Save the size of "k_head" and "k_info" */
	k_head->head_size = sizeof(header);
	k_head->info_size = k_head->info_num * k_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "k_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_k_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "k_info" array */
	C_MAKE(k_info, k_head->info_num, object_kind);

	/* Hack -- make "fake" arrays */
	C_MAKE(k_name, z_info->fake_name_size, char);
	C_MAKE(k_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "k_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'k_info.txt' file.");

	/* Parse the file */
	err = init_k_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'k_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'k_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(k_head), k_head->head_size);

		/* Dump the "k_info" array */
		fd_write(fd, (char*)(k_info), k_head->info_size);

		/* Dump the "k_name" array */
		fd_write(fd, (char*)(k_name), k_head->name_size);

		/* Dump the "k_text" array */
		fd_write(fd, (char*)(k_text), k_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "k_info" array */
	C_KILL(k_info, k_head->info_num, object_kind);

	/* Hack -- Free the "fake" arrays */
	C_KILL(k_name, z_info->fake_name_size, char);
	C_KILL(k_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'k_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_k_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'k_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "a_info" array, by parsing a binary "image" file
 */
static errr init_a_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != a_head->v_major) ||
	    (test.v_minor != a_head->v_minor) ||
	    (test.v_patch != a_head->v_patch) ||
	    (test.v_extra != a_head->v_extra) ||
	    (test.info_num != a_head->info_num) ||
	    (test.info_len != a_head->info_len) ||
	    (test.head_size != a_head->head_size) ||
	    (test.info_size != a_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*a_head) = test;


	/* Allocate the "a_info" array */
	C_MAKE(a_info, a_head->info_num, artifact_type);

	/* Read the "a_info" array */
	fd_read(fd, (char*)(a_info), a_head->info_size);


	/* Allocate the "a_name" array */
	C_MAKE(a_name, a_head->name_size, char);

	/* Read the "a_name" array */
	fd_read(fd, (char*)(a_name), a_head->name_size);


#ifndef DELAY_LOAD_A_TEXT

	/* Allocate the "a_text" array */
	C_MAKE(a_text, a_head->text_size, char);

	/* Read the "a_text" array */
	fd_read(fd, (char*)(a_text), a_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "a_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_a_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(a_head, header);

	/* Save the "version" */
	a_head->v_major = VERSION_MAJOR;
	a_head->v_minor = VERSION_MINOR;
	a_head->v_patch = VERSION_PATCH;
	a_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	a_head->info_num = z_info->a_max;
	a_head->info_len = sizeof(artifact_type);

	/* Save the size of "a_head" and "a_info" */
	a_head->head_size = sizeof(header);
	a_head->info_size = a_head->info_num * a_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "a_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_a_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "a_info" array */
	C_MAKE(a_info, a_head->info_num, artifact_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(a_name, z_info->fake_name_size, char);
	C_MAKE(a_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "a_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'a_info.txt' file.");

	/* Parse the file */
	err = init_a_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'a_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'a_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(a_head), a_head->head_size);

		/* Dump the "a_info" array */
		fd_write(fd, (char*)(a_info), a_head->info_size);

		/* Dump the "a_name" array */
		fd_write(fd, (char*)(a_name), a_head->name_size);

		/* Dump the "a_text" array */
		fd_write(fd, (char*)(a_text), a_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "a_info" array */
	C_KILL(a_info, a_head->info_num, artifact_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(a_name, z_info->fake_name_size, char);
	C_KILL(a_text, z_info->fake_text_size, char);


#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 'a_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_a_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'a_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "e_info" array, by parsing a binary "image" file
 */
static errr init_e_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != e_head->v_major) ||
	    (test.v_minor != e_head->v_minor) ||
	    (test.v_patch != e_head->v_patch) ||
	    (test.v_extra != e_head->v_extra) ||
	    (test.info_num != e_head->info_num) ||
	    (test.info_len != e_head->info_len) ||
	    (test.head_size != e_head->head_size) ||
	    (test.info_size != e_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*e_head) = test;


	/* Allocate the "e_info" array */
	C_MAKE(e_info, e_head->info_num, ego_item_type);

	/* Read the "e_info" array */
	fd_read(fd, (char*)(e_info), e_head->info_size);


	/* Allocate the "e_name" array */
	C_MAKE(e_name, e_head->name_size, char);

	/* Read the "e_name" array */
	fd_read(fd, (char*)(e_name), e_head->name_size);


#ifndef DELAY_LOAD_E_TEXT

	/* Allocate the "e_text" array */
	C_MAKE(e_text, e_head->text_size, char);

	/* Read the "e_text" array */
	fd_read(fd, (char*)(e_text), e_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "e_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_e_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(e_head, header);

	/* Save the "version" */
	e_head->v_major = VERSION_MAJOR;
	e_head->v_minor = VERSION_MINOR;
	e_head->v_patch = VERSION_PATCH;
	e_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	e_head->info_num = z_info->e_max;
	e_head->info_len = sizeof(ego_item_type);

	/* Save the size of "e_head" and "e_info" */
	e_head->head_size = sizeof(header);
	e_head->info_size = e_head->info_num * e_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "e_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_e_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "e_info" array */
	C_MAKE(e_info, e_head->info_num, ego_item_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(e_name, z_info->fake_name_size, char);
	C_MAKE(e_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "e_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'e_info.txt' file.");

	/* Parse the file */
	err = init_e_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'e_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'e_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(e_head), e_head->head_size);

		/* Dump the "e_info" array */
		fd_write(fd, (char*)(e_info), e_head->info_size);

		/* Dump the "e_name" array */
		fd_write(fd, (char*)(e_name), e_head->name_size);

		/* Dump the "e_text" array */
		fd_write(fd, (char*)(e_text), e_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "e_info" array */
	C_KILL(e_info, e_head->info_num, ego_item_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(e_name, z_info->fake_name_size, char);
	C_KILL(e_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'e_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_e_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'e_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "r_info" array, by parsing a binary "image" file
 */
static errr init_r_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != r_head->v_major) ||
	    (test.v_minor != r_head->v_minor) ||
	    (test.v_patch != r_head->v_patch) ||
	    (test.v_extra != r_head->v_extra) ||
	    (test.info_num != r_head->info_num) ||
	    (test.info_len != r_head->info_len) ||
	    (test.head_size != r_head->head_size) ||
	    (test.info_size != r_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*r_head) = test;


	/* Allocate the "r_info" array */
	C_MAKE(r_info, r_head->info_num, monster_race);

	/* Read the "r_info" array */
	fd_read(fd, (char*)(r_info), r_head->info_size);


	/* Allocate the "r_name" array */
	C_MAKE(r_name, r_head->name_size, char);

	/* Read the "r_name" array */
	fd_read(fd, (char*)(r_name), r_head->name_size);


#ifndef DELAY_LOAD_R_TEXT

	/* Allocate the "r_text" array */
	C_MAKE(r_text, r_head->text_size, char);

	/* Read the "r_text" array */
	fd_read(fd, (char*)(r_text), r_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "r_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_r_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(r_head, header);

	/* Save the "version" */
	r_head->v_major = VERSION_MAJOR;
	r_head->v_minor = VERSION_MINOR;
	r_head->v_patch = VERSION_PATCH;
	r_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	r_head->info_num = z_info->r_max;
	r_head->info_len = sizeof(monster_race);

	/* Save the size of "r_head" and "r_info" */
	r_head->head_size = sizeof(header);
	r_head->info_size = r_head->info_num * r_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "r_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_r_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "r_info" array */
	C_MAKE(r_info, r_head->info_num, monster_race);

	/* Hack -- make "fake" arrays */
	C_MAKE(r_name, z_info->fake_name_size, char);
	C_MAKE(r_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "r_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'r_info.txt' file.");

	/* Parse the file */
	err = init_r_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'r_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'r_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(r_head), r_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(r_info), r_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(r_name), r_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(r_text), r_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "r_info" array */
	C_KILL(r_info, r_head->info_num, monster_race);

	/* Hack -- Free the "fake" arrays */
	C_KILL(r_name, z_info->fake_name_size, char);
	C_KILL(r_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'r_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_r_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'r_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "v_info" array, by parsing a binary "image" file
 */
static errr init_v_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != v_head->v_major) ||
	    (test.v_minor != v_head->v_minor) ||
	    (test.v_patch != v_head->v_patch) ||
	    (test.v_extra != v_head->v_extra) ||
	    (test.info_num != v_head->info_num) ||
	    (test.info_len != v_head->info_len) ||
	    (test.head_size != v_head->head_size) ||
	    (test.info_size != v_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*v_head) = test;


	/* Allocate the "v_info" array */
	C_MAKE(v_info, v_head->info_num, vault_type);

	/* Read the "v_info" array */
	fd_read(fd, (char*)(v_info), v_head->info_size);


	/* Allocate the "v_name" array */
	C_MAKE(v_name, v_head->name_size, char);

	/* Read the "v_name" array */
	fd_read(fd, (char*)(v_name), v_head->name_size);


#ifndef DELAY_LOAD_V_TEXT

	/* Allocate the "v_text" array */
	C_MAKE(v_text, v_head->text_size, char);

	/* Read the "v_text" array */
	fd_read(fd, (char*)(v_text), v_head->text_size);

#endif

	/* Success */
	return (0);
}



/*
 * Initialize the "v_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_v_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(v_head, header);

	/* Save the "version" */
	v_head->v_major = VERSION_MAJOR;
	v_head->v_minor = VERSION_MINOR;
	v_head->v_patch = VERSION_PATCH;
	v_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	v_head->info_num = z_info->v_max;
	v_head->info_len = sizeof(vault_type);

	/* Save the size of "v_head" and "v_info" */
	v_head->head_size = sizeof(header);
	v_head->info_size = v_head->info_num * v_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "v_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_v_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "k_info" array */
	C_MAKE(v_info, v_head->info_num, vault_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(v_name, z_info->fake_name_size, char);
	C_MAKE(v_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "v_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'v_info.txt' file.");

	/* Parse the file */
	err = init_v_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'v_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'v_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(v_head), v_head->head_size);

		/* Dump the "v_info" array */
		fd_write(fd, (char*)(v_info), v_head->info_size);

		/* Dump the "v_name" array */
		fd_write(fd, (char*)(v_name), v_head->name_size);

		/* Dump the "v_text" array */
		fd_write(fd, (char*)(v_text), v_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "v_info" array */
	C_KILL(v_info, v_head->info_num, vault_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(v_name, z_info->fake_name_size, char);
	C_KILL(v_text, z_info->fake_text_size, char);


#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'v_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_v_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'v_info.raw' file.");

	/* Success */
	return (0);
}


/*
 * Initialize the "p_info" array, by parsing a binary "image" file
 */
static errr init_p_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != p_head->v_major) ||
	    (test.v_minor != p_head->v_minor) ||
	    (test.v_patch != p_head->v_patch) ||
	    (test.v_extra != p_head->v_extra) ||
	    (test.info_num != p_head->info_num) ||
	    (test.info_len != p_head->info_len) ||
	    (test.head_size != p_head->head_size) ||
	    (test.info_size != p_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*p_head) = test;


	/* Allocate the "p_info" array */
	C_MAKE(p_info, p_head->info_num, player_race);

	/* Read the "p_info" array */
	fd_read(fd, (char*)(p_info), p_head->info_size);


	/* Allocate the "p_name" array */
	C_MAKE(p_name, p_head->name_size, char);

	/* Read the "p_name" array */
	fd_read(fd, (char*)(p_name), p_head->name_size);


#ifndef DELAY_LOAD_P_TEXT

	/* Allocate the "p_text" array */
	C_MAKE(p_text, p_head->text_size, char);

	/* Read the "p_text" array */
	fd_read(fd, (char*)(p_text), p_head->text_size);

#endif


	/* Success */
	return (0);
}



/*
 * Initialize the "p_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_p_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(p_head, header);

	/* Save the "version" */
	p_head->v_major = VERSION_MAJOR;
	p_head->v_minor = VERSION_MINOR;
	p_head->v_patch = VERSION_PATCH;
	p_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	p_head->info_num = z_info->p_max;
	p_head->info_len = sizeof(player_race);

	/* Save the size of "p_head" and "p_info" */
	p_head->head_size = sizeof(header);
	p_head->info_size = p_head->info_num * p_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "p_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_p_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "p_info" array */
	C_MAKE(p_info, p_head->info_num, player_race);

	/* Hack -- make "fake" arrays */
	C_MAKE(p_name, z_info->fake_name_size, char);
	C_MAKE(p_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "p_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'p_info.txt' file.");

	/* Parse the file */
	err = init_p_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'p_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'p_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(p_head), p_head->head_size);

		/* Dump the "p_info" array */
		fd_write(fd, (char*)(p_info), p_head->info_size);

		/* Dump the "p_name" array */
		fd_write(fd, (char*)(p_name), p_head->name_size);

		/* Dump the "p_text" array */
		fd_write(fd, (char*)(p_text), p_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "p_info" array */
	C_KILL(p_info, p_head->info_num, player_race);

	/* Hack -- Free the "fake" arrays */
	C_KILL(p_name, z_info->fake_name_size, char);
	C_KILL(p_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 'p_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_p_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'p_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "h_info" array, by parsing a binary "image" file
 */
static errr init_h_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != h_head->v_major) ||
	    (test.v_minor != h_head->v_minor) ||
	    (test.v_patch != h_head->v_patch) ||
	    (test.v_extra != h_head->v_extra) ||
	    (test.info_num != h_head->info_num) ||
	    (test.info_len != h_head->info_len) ||
	    (test.head_size != h_head->head_size) ||
	    (test.info_size != h_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*h_head) = test;


	/* Allocate the "h_info" array */
	C_MAKE(h_info, h_head->info_num, hist_type);

	/* Read the "h_info" array */
	fd_read(fd, (char*)(h_info), h_head->info_size);


	/* Allocate the "h_text" array */
	C_MAKE(h_text, h_head->text_size, char);

	/* Read the "h_text" array */
	fd_read(fd, (char*)(h_text), h_head->text_size);


	/* Success */
	return (0);
}



/*
 * Initialize the "h_info" array
 */
static errr init_h_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(h_head, header);

	/* Save the "version" */
	h_head->v_major = VERSION_MAJOR;
	h_head->v_minor = VERSION_MINOR;
	h_head->v_patch = VERSION_PATCH;
	h_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	h_head->info_num = z_info->h_max;
	h_head->info_len = sizeof(hist_type);

	/* Save the size of "h_head" and "h_info" */
	h_head->head_size = sizeof(header);
	h_head->info_size = h_head->info_num * h_head->info_len;

#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "h_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "h_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_h_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "h_info" array */
	C_MAKE(h_info, h_head->info_num, hist_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(h_text, z_info->fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "h_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'h_info.txt' file.");

	/* Parse the file */
	err = init_h_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'h_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'h_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "h_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(h_head), h_head->head_size);

		/* Dump the "h_info" array */
		fd_write(fd, (char*)(h_info), h_head->info_size);

		/* Dump the "h_text" array */
		fd_write(fd, (char*)(h_text), h_head->text_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "h_info" array */
	C_KILL(h_info, h_head->info_num, hist_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(h_text, z_info->fake_text_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "h_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'h_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_h_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'h_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "b_info" array, by parsing a binary "image" file
 */
static errr init_b_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != b_head->v_major) ||
	    (test.v_minor != b_head->v_minor) ||
	    (test.v_patch != b_head->v_patch) ||
	    (test.v_extra != b_head->v_extra) ||
	    (test.info_num != b_head->info_num) ||
	    (test.info_len != b_head->info_len) ||
	    (test.head_size != b_head->head_size) ||
	    (test.info_size != b_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*b_head) = test;


	/* Allocate the "b_info" array */
	C_MAKE(b_info, b_head->info_num, owner_type);

	/* Read the "b_info" array */
	fd_read(fd, (char*)(b_info), b_head->info_size);


	/* Allocate the "b_name" array */
	C_MAKE(b_name, b_head->name_size, char);

	/* Read the "b_name" array */
	fd_read(fd, (char*)(b_name), b_head->name_size);


	/* Success */
	return (0);
}



/*
 * Initialize the "b_info" array
 */
static errr init_b_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(b_head, header);

	/* Save the "version" */
	b_head->v_major = VERSION_MAJOR;
	b_head->v_minor = VERSION_MINOR;
	b_head->v_patch = VERSION_PATCH;
	b_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	b_head->info_num = MAX_STORES * z_info->b_max;
	b_head->info_len = sizeof(owner_type);

	/* Save the size of "b_head" and "b_info" */
	b_head->head_size = sizeof(header);
	b_head->info_size = b_head->info_num * b_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "b_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "b_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_b_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/*** Make the fake arrays ***/

	/* Allocate the "b_info" array */
	C_MAKE(b_info, b_head->info_num, owner_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(b_name, z_info->fake_name_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "b_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'b_info.txt' file.");

	/* Parse the file */
	err = init_b_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'b_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'b_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "b_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(b_head), b_head->head_size);

		/* Dump the "b_info" array */
		fd_write(fd, (char*)(b_info), b_head->info_size);

		/* Dump the "b_name" array */
		fd_write(fd, (char*)(b_name), b_head->name_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "b_info" array */
	C_KILL(b_info, b_head->info_num, owner_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(b_name, z_info->fake_name_size, char);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "b_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'b_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_b_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'b_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialize the "g_info" array, by parsing a binary "image" file
 */
static errr init_g_info_raw(int fd)
{
	header test;


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	    (test.v_major != g_head->v_major) ||
	    (test.v_minor != g_head->v_minor) ||
	    (test.v_patch != g_head->v_patch) ||
	    (test.v_extra != g_head->v_extra) ||
	    (test.info_num != g_head->info_num) ||
	    (test.info_len != g_head->info_len) ||
	    (test.head_size != g_head->head_size) ||
	    (test.info_size != g_head->info_size))
	{
		/* Error */
		return (-1);
	}


	/* Accept the header */
	(*g_head) = test;


	/* Allocate the "g_info" array */
	C_MAKE(g_info, g_head->info_num, byte);

	/* Read the "g_info" array */
	fd_read(fd, (char*)(g_info), g_head->info_size);


	/* Success */
	return (0);
}



/*
 * Initialize the "g_info" array
 */
static errr init_g_info(void)
{
	int fd;

	int mode = 0644;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(g_head, header);

	/* Save the "version" */
	g_head->v_major = VERSION_MAJOR;
	g_head->v_minor = VERSION_MINOR;
	g_head->v_patch = VERSION_PATCH;
	g_head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	g_head->info_num = (z_info->p_max * z_info->p_max);
	g_head->info_len = sizeof(byte);

	/* Save the size of "g_head" and "g_info" */
	g_head->head_size = sizeof(header);
	g_head->info_size = g_head->info_num * g_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "g_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "g_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_g_info_raw(fd);

		/* Close it */
		fd_close(fd);

		/* Success */
		if (!err) return (0);
	}


	/* Allocate the "g_info" array */
	C_MAKE(g_info, g_head->info_num, byte);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "g_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'g_info.txt' file.");

	/* Parse the file */
	err = init_g_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'g_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'g_info.txt' file.");
	}


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "g_info.raw");

	/* Kill the old file */
	fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(g_head), g_head->head_size);

		/* Dump the "g_info" array */
		fd_write(fd, (char*)(g_info), g_head->info_size);

		/* Close */
		fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "g_info" array */
	C_KILL(g_info, g_head->info_num, byte);

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "g_info.raw");

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'g_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_g_info_raw(fd);

	/* Close it */
	fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'g_info.raw' file.");

	/* Success */
	return (0);
}




/*** Initialize others ***/



/*
 * Hack -- Objects sold in the stores -- by tval/sval pair.
 */
static byte store_table[MAX_STORES-2][STORE_CHOICES][2] =
{
	{
		/* General Store */

		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_RATION },
		{ TV_FOOD, SV_FOOD_BISCUIT },
		{ TV_FOOD, SV_FOOD_JERKY },
		{ TV_FOOD, SV_FOOD_JERKY },

		{ TV_FOOD, SV_FOOD_PINT_OF_WINE },
		{ TV_FOOD, SV_FOOD_PINT_OF_ALE },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_TORCH },
		{ TV_LITE, SV_LITE_LANTERN },
		{ TV_LITE, SV_LITE_LANTERN },

		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_FLASK, 0 },
		{ TV_SPIKE, 0 },
		{ TV_SPIKE, 0 },

		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
		{ TV_DIGGING, SV_SHOVEL },
		{ TV_DIGGING, SV_PICK },
		{ TV_CLOAK, SV_CLOAK },
		{ TV_CLOAK, SV_CLOAK },
		{ TV_CLOAK, SV_CLOAK }
	},

	{
		/* Armoury */

		{ TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_SOFT_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
		{ TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS },
		{ TV_HELM, SV_HARD_LEATHER_CAP },
		{ TV_HELM, SV_HARD_LEATHER_CAP },
		{ TV_HELM, SV_METAL_CAP },
		{ TV_HELM, SV_IRON_HELM },

		{ TV_SOFT_ARMOR, SV_ROBE },
		{ TV_SOFT_ARMOR, SV_ROBE },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR },
		{ TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },
		{ TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER },

		{ TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
		{ TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL },
		{ TV_HARD_ARMOR, SV_METAL_SCALE_MAIL },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_AUGMENTED_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_BAR_CHAIN_MAIL },
		{ TV_HARD_ARMOR, SV_DOUBLE_CHAIN_MAIL },

		{ TV_HARD_ARMOR, SV_METAL_BRIGANDINE_ARMOUR },
		{ TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
		{ TV_GLOVES, SV_SET_OF_LEATHER_GLOVES },
		{ TV_GLOVES, SV_SET_OF_GAUNTLETS },
		{ TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
		{ TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
		{ TV_SHIELD, SV_LARGE_LEATHER_SHIELD },
		{ TV_SHIELD, SV_SMALL_METAL_SHIELD }
	},

	{
		/* Weaponsmith */

		{ TV_SWORD, SV_DAGGER },
		{ TV_SWORD, SV_MAIN_GAUCHE },
		{ TV_SWORD, SV_RAPIER },
		{ TV_SWORD, SV_SMALL_SWORD },
		{ TV_SWORD, SV_SHORT_SWORD },
		{ TV_SWORD, SV_SABRE },
		{ TV_SWORD, SV_CUTLASS },
		{ TV_SWORD, SV_TULWAR },

		{ TV_SWORD, SV_BROAD_SWORD },
		{ TV_SWORD, SV_LONG_SWORD },
		{ TV_SWORD, SV_SCIMITAR },
		{ TV_SWORD, SV_KATANA },
		{ TV_SWORD, SV_BASTARD_SWORD },
		{ TV_POLEARM, SV_SPEAR },
		{ TV_POLEARM, SV_AWL_PIKE },
		{ TV_POLEARM, SV_TRIDENT },

		{ TV_POLEARM, SV_PIKE },
		{ TV_POLEARM, SV_BEAKED_AXE },
		{ TV_POLEARM, SV_BROAD_AXE },
		{ TV_POLEARM, SV_LANCE },
		{ TV_POLEARM, SV_BATTLE_AXE },
		{ TV_HAFTED, SV_WHIP },
		{ TV_BOW, SV_SLING },
		{ TV_BOW, SV_SHORT_BOW },

		{ TV_BOW, SV_LONG_BOW },
		{ TV_BOW, SV_LIGHT_XBOW },
		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_SHOT, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_ARROW, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
		{ TV_BOLT, SV_AMMO_NORMAL },
	},

	{
		/* Temple */

		{ TV_HAFTED, SV_WHIP },
		{ TV_HAFTED, SV_QUARTERSTAFF },
		{ TV_HAFTED, SV_MACE },
		{ TV_HAFTED, SV_MACE },
		{ TV_HAFTED, SV_BALL_AND_CHAIN },
		{ TV_HAFTED, SV_WAR_HAMMER },
		{ TV_HAFTED, SV_LUCERN_HAMMER },
		{ TV_HAFTED, SV_MORNING_STAR },

		{ TV_HAFTED, SV_FLAIL },
		{ TV_HAFTED, SV_FLAIL },
		{ TV_HAFTED, SV_LEAD_FILLED_MACE },
		{ TV_SCROLL, SV_SCROLL_REMOVE_CURSE },
		{ TV_SCROLL, SV_SCROLL_BLESSING },
		{ TV_SCROLL, SV_SCROLL_HOLY_CHANT },
		{ TV_POTION, SV_POTION_BOLDNESS },
		{ TV_POTION, SV_POTION_HEROISM },

		{ TV_POTION, SV_POTION_CURE_LIGHT },
		{ TV_POTION, SV_POTION_CURE_SERIOUS },
		{ TV_POTION, SV_POTION_CURE_SERIOUS },
		{ TV_POTION, SV_POTION_CURE_CRITICAL },
		{ TV_POTION, SV_POTION_CURE_CRITICAL },
		{ TV_POTION, SV_POTION_RESTORE_EXP },
		{ TV_POTION, SV_POTION_RESTORE_EXP },
		{ TV_POTION, SV_POTION_RESTORE_EXP },

		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 0 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_PRAYER_BOOK, 1 },
		{ TV_PRAYER_BOOK, 2 },
		{ TV_PRAYER_BOOK, 2 },
		{ TV_PRAYER_BOOK, 3 }
	},

	{
		/* Alchemy shop */

		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_HIT },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_WEAPON_TO_DAM },
		{ TV_SCROLL, SV_SCROLL_ENCHANT_ARMOR },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_IDENTIFY },
		{ TV_SCROLL, SV_SCROLL_LIGHT },

		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_PHASE_DOOR },
		{ TV_SCROLL, SV_SCROLL_MONSTER_CONFUSION },
		{ TV_SCROLL, SV_SCROLL_MAPPING },
		{ TV_SCROLL, SV_SCROLL_DETECT_GOLD },
		{ TV_SCROLL, SV_SCROLL_DETECT_ITEM },
		{ TV_SCROLL, SV_SCROLL_DETECT_TRAP },

		{ TV_SCROLL, SV_SCROLL_DETECT_DOOR },
		{ TV_SCROLL, SV_SCROLL_DETECT_INVIS },
		{ TV_SCROLL, SV_SCROLL_RECHARGING },
		{ TV_SCROLL, SV_SCROLL_SATISFY_HUNGER },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },
		{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL },

		{ TV_POTION, SV_POTION_RESIST_HEAT },
		{ TV_POTION, SV_POTION_RESIST_COLD },
		{ TV_POTION, SV_POTION_RES_STR },
		{ TV_POTION, SV_POTION_RES_INT },
		{ TV_POTION, SV_POTION_RES_WIS },
		{ TV_POTION, SV_POTION_RES_DEX },
		{ TV_POTION, SV_POTION_RES_CON },
		{ TV_POTION, SV_POTION_RES_CHR }
	},

	{
		/* Magic-User store */

		{ TV_RING, SV_RING_SEARCHING },
		{ TV_RING, SV_RING_FEATHER_FALL },
		{ TV_RING, SV_RING_PROTECTION },
		{ TV_AMULET, SV_AMULET_CHARISMA },
		{ TV_AMULET, SV_AMULET_SLOW_DIGEST },
		{ TV_AMULET, SV_AMULET_RESIST_ACID },
		{ TV_WAND, SV_WAND_SLOW_MONSTER },
		{ TV_WAND, SV_WAND_CONFUSE_MONSTER },

		{ TV_WAND, SV_WAND_SLEEP_MONSTER },
		{ TV_WAND, SV_WAND_MAGIC_MISSILE },
		{ TV_WAND, SV_WAND_STINKING_CLOUD },
		{ TV_WAND, SV_WAND_WONDER },
		{ TV_STAFF, SV_STAFF_LITE },
		{ TV_STAFF, SV_STAFF_MAPPING },
		{ TV_STAFF, SV_STAFF_DETECT_TRAP },
		{ TV_STAFF, SV_STAFF_DETECT_DOOR },

		{ TV_STAFF, SV_STAFF_DETECT_GOLD },
		{ TV_STAFF, SV_STAFF_DETECT_ITEM },
		{ TV_STAFF, SV_STAFF_DETECT_INVIS },
		{ TV_STAFF, SV_STAFF_DETECT_EVIL },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_TELEPORTATION },
		{ TV_STAFF, SV_STAFF_IDENTIFY },
		{ TV_STAFF, SV_STAFF_IDENTIFY },

		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 0 },
		{ TV_MAGIC_BOOK, 1 },
		{ TV_MAGIC_BOOK, 1 },
		{ TV_MAGIC_BOOK, 2 },
		{ TV_MAGIC_BOOK, 2 },
		{ TV_MAGIC_BOOK, 3 }
	}
};



/*
 * Initialize some other arrays
 */
static errr init_other(void)
{
	int i, k, n;


	/*** Prepare the various "bizarre" arrays ***/

	/* Initialize the "macro" package */
	(void)macro_init();

	/* Initialize the "quark" package */
	(void)quark_init();

	/* Initialize the "message" package */
	(void)message_init();

	/*** Prepare grid arrays ***/

	/* Array of grids */
	C_MAKE(view_g, VIEW_MAX, u16b);

	/* Array of grids */
	C_MAKE(temp_g, TEMP_MAX, u16b);

	/* Hack -- use some memory twice */
	temp_y = ((byte*)(temp_g)) + 0;
	temp_x = ((byte*)(temp_g)) + TEMP_MAX;


	/*** Prepare dungeon arrays ***/

	/* Padded into array */
	C_MAKE(cave_info, DUNGEON_HGT, byte_256);

	/* Feature array */
	C_MAKE(cave_feat, DUNGEON_HGT, byte_wid);

	/* Entity arrays */
	C_MAKE(cave_o_idx, DUNGEON_HGT, s16b_wid);
	C_MAKE(cave_m_idx, DUNGEON_HGT, s16b_wid);

#ifdef MONSTER_FLOW

	/* Flow arrays */
	C_MAKE(cave_cost, DUNGEON_HGT, byte_wid);
	C_MAKE(cave_when, DUNGEON_HGT, byte_wid);

#endif /* MONSTER_FLOW */

	/*** Prepare "vinfo" array ***/

	/* Used by "update_view()" */
	(void)vinfo_init();


	/*** Prepare entity arrays ***/

	/* Objects */
	C_MAKE(o_list, z_info->o_max, object_type);

	/* Monsters */
	C_MAKE(m_list, z_info->m_max, monster_type);


	/*** Prepare lore array ***/

	/* Lore */
	C_MAKE(l_list, z_info->r_max, monster_lore);


	/*** Prepare quest array ***/

	/* Quests */
	C_MAKE(q_list, MAX_Q_IDX, quest);


	/*** Prepare the inventory ***/

	/* Allocate it */
	C_MAKE(inventory, INVEN_TOTAL, object_type);


	/*** Prepare the stores ***/

	/* Allocate the stores */
	C_MAKE(store, MAX_STORES, store_type);

	/* Fill in each store */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Get the store */
		store_type *st_ptr = &store[i];

		/* Assume full stock */
		st_ptr->stock_size = STORE_INVEN_MAX;

		/* Allocate the stock */
		C_MAKE(st_ptr->stock, st_ptr->stock_size, object_type);

		/* No table for the black market or home */
		if ((i == STORE_B_MARKET) || (i == STORE_HOME)) continue;

		/* Assume full table */
		st_ptr->table_size = STORE_CHOICES;

		/* Allocate the stock */
		C_MAKE(st_ptr->table, st_ptr->table_size, s16b);

		/* Scan the choices */
		for (k = 0; k < STORE_CHOICES; k++)
		{
			int k_idx;

			/* Extract the tval/sval codes */
			int tv = store_table[i][k][0];
			int sv = store_table[i][k][1];

			/* Look for it */
			for (k_idx = 1; k_idx < z_info->k_max; k_idx++)
			{
				object_kind *k_ptr = &k_info[k_idx];

				/* Found a match */
				if ((k_ptr->tval == tv) && (k_ptr->sval == sv)) break;
			}

			/* Catch errors */
			if (k_idx == z_info->k_max) continue;

			/* Add that item index to the table */
			st_ptr->table[st_ptr->table_num++] = k_idx;
		}
	}



	/*** Prepare the options ***/

	/* Initialize the options */
	for (i = 0; i < OPT_MAX; i++)
	{
		/* Default value */
		op_ptr->opt[i] = option_norm[i];
	}

	/* Initialize the window flags */
	for (n = 0; n < 8; n++)
	{
		/* Assume no flags */
		op_ptr->window_flag[n] = 0L;
	}


	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void)format("%s (%s).", "Robert Ruehlmann", MAINTAINER);


	/* Success */
	return (0);
}



/*
 * Initialize some other arrays
 */
static errr init_alloc(void)
{
	int i, j;

	object_kind *k_ptr;

	monster_race *r_ptr;

	ego_item_type *e_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH];

	s16b aux[MAX_DEPTH];


	/*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	(void)C_WIPE(&aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(&num, MAX_DEPTH, s16b);

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (i = 1; i < z_info->k_max; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town objects!");


	/*** Initialize object allocation info ***/

	/* Allocate the alloc_kind_table */
	C_MAKE(alloc_kind_table, alloc_kind_size, alloc_entry);

	/* Get the table entry */
	table = alloc_kind_table;

	/* Scan the objects */
	for (i = 1; i < z_info->k_max; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				int p, x, y, z;

				/* Extract the base level */
				x = k_ptr->locale[j];

				/* Extract the base probability */
				p = (100 / k_ptr->chance[j]);

				/* Skip entries preceding our locale */
				y = (x > 0) ? num[x-1] : 0;

				/* Skip previous entries at this locale */
				z = y + aux[x];

				/* Load the entry */
				table[z].index = i;
				table[z].level = x;
				table[z].prob1 = p;
				table[z].prob2 = p;
				table[z].prob3 = p;

				/* Another entry complete for this locale */
				aux[x]++;
			}
		}
	}


	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	(void)C_WIPE(&aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(&num, MAX_DEPTH, s16b);

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity)
		{
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");


	/*** Initialize monster allocation info ***/

	/* Allocate the alloc_race_table */
	C_MAKE(alloc_race_table, alloc_race_size, alloc_entry);

	/* Get the table entry */
	table = alloc_race_table;

	/* Scan the monsters (not the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x-1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}

	/*** Analyze ego_item allocation info ***/

	/* Clear the "aux" array */
	(void)C_WIPE(&aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(&num, MAX_DEPTH, s16b);

	/* Size of "alloc_ego_table" */
	alloc_ego_size = 0;

	/* Scan the ego items */
	for (i = 1; i < z_info->e_max; i++)
	{
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Legal items */
		if (e_ptr->rarity)
		{
			/* Count the entries */
			alloc_ego_size++;

			/* Group by level */
			num[e_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH; i++)
	{
		/* Group by level */
		num[i] += num[i-1];
	}

	/*** Initialize ego-item allocation info ***/

	/* Allocate the alloc_ego_table */
	C_MAKE(alloc_ego_table, alloc_ego_size, alloc_entry);

	/* Get the table entry */
	table = alloc_ego_table;

	/* Scan the ego-items */
	for (i = 1; i < z_info->e_max; i++)
	{
		/* Get the i'th ego item */
		e_ptr = &e_info[i];

		/* Count valid pairs */
		if (e_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = e_ptr->level;

			/* Extract the base probability */
			p = (100 / e_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x-1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}


	/* Success */
	return (0);
}


/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, str);
	Term_fresh();
}



/*
 * Hack -- Explain a broken "lib" folder and quit (see below).
 *
 * XXX XXX XXX This function is "messy" because various things
 * may or may not be initialized, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 */
static void init_angband_aux(cptr why)
{
	/* Why */
	plog(why);

	/* Explain */
	plog("The 'lib' directory is probably missing or broken.");

	/* More details */
	plog("Perhaps the archive was not extracted correctly.");

	/* Explain */
	plog("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
}


/*
 * Hack -- main Angband initialization entry point
 *
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialize all internal arrays, and
 * load the basic "user pref files".
 *
 * Be very careful to keep track of the order in which things
 * are initialized, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialized until the
 * end of this function, when the default "user pref files"
 * are loaded and "Term_xtra(TERM_XTRA_REACT,0)" is called.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 *
 * Note that various things are initialized by this function,
 * including everything that was once done by "init_some_arrays".
 *
 * This initialization involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialized first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 *
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 *
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
void init_angband(void)
{
	int fd;

	int mode = 0644;

	FILE *fp;

	char buf[1024];


	/*** Verify the "news" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "news.txt");

	/* Attempt to open the file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		char why[1024];

		/* Message */
		sprintf(why, "Cannot access the '%s' file!", buf);

		/* Crash and burn */
		init_angband_aux(why);
	}

	/* Close it */
	fd_close(fd);


	/*** Display the "news" file ***/

	/* Clear screen */
	Term_clear();

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "news.txt");

	/* Open the News file */
	fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, 1024))
		{
			/* Display and advance */
			Term_putstr(0, i++, -1, TERM_WHITE, buf);
		}

		/* Close */
		my_fclose(fp);
	}

	/* Flush it */
	Term_fresh();


	/*** Verify (or create) the "high score" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		/* File type is "DATA" */
		FILE_TYPE(FILE_TYPE_DATA);

		/* Create a new high score file */
		fd = fd_make(buf, mode);

		/* Failure */
		if (fd < 0)
		{
			char why[1024];

			/* Message */
			sprintf(why, "Cannot create the '%s' file!", buf);

			/* Crash and burn */
			init_angband_aux(why);
		}
	}

	/* Close it */
	fd_close(fd);


	/*** Initialize some arrays ***/

	/* Initialize size info */
	note("[Initializing array sizes...]");
	if (init_z_info()) quit("Cannot initialize sizes");

	/* Initialize feature info */
	note("[Initializing arrays... (features)]");
	if (init_f_info()) quit("Cannot initialize features");

	/* Initialize object info */
	note("[Initializing arrays... (objects)]");
	if (init_k_info()) quit("Cannot initialize objects");

	/* Initialize artifact info */
	note("[Initializing arrays... (artifacts)]");
	if (init_a_info()) quit("Cannot initialize artifacts");

	/* Initialize ego-item info */
	note("[Initializing arrays... (ego-items)]");
	if (init_e_info()) quit("Cannot initialize ego-items");

	/* Initialize monster info */
	note("[Initializing arrays... (monsters)]");
	if (init_r_info()) quit("Cannot initialize monsters");

	/* Initialize feature info */
	note("[Initializing arrays... (vaults)]");
	if (init_v_info()) quit("Cannot initialize vaults");

	/* Initialize history info */
	note("[Initializing arrays... (histories)]");
	if (init_h_info()) quit("Cannot initialize histories");

	/* Initialize race info */
	note("[Initializing arrays... (races)]");
	if (init_p_info()) quit("Cannot initialize races");

	/* Initialize owner info */
	note("[Initializing arrays... (owners)]");
	if (init_b_info()) quit("Cannot initialize owners");

	/* Initialize price info */
	note("[Initializing arrays... (prices)]");
	if (init_g_info()) quit("Cannot initialize prices");

	/* Initialize some other arrays */
	note("[Initializing arrays... (other)]");
	if (init_other()) quit("Cannot initialize other stuff");

	/* Initialize some other arrays */
	note("[Initializing arrays... (alloc)]");
	if (init_alloc()) quit("Cannot initialize alloc stuff");


	/*** Load default user pref files ***/

	/* Initialize feature info */
	note("[Loading basic user pref file...]");

	/* Process that file */
	(void)process_pref_file("pref.prf");

	/* Done */
	note("[Initialization complete]");
}
