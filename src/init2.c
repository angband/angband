/* File: init2.c */


/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

#include "init.h"
#include "cmds.h"
#include "option.h"

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
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(const char *path)
{
#ifdef PRIVATE_USER_PATH
	char buf[1024];
#endif /* PRIVATE_USER_PATH */

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
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);


	/*** Prepare the paths ***/

	/* Save the main directory */
	ANGBAND_DIR = string_make(path);

	/* Build path names */
	ANGBAND_DIR_EDIT = string_make(format("%sedit", path));
	ANGBAND_DIR_FILE = string_make(format("%sfile", path));
	ANGBAND_DIR_HELP = string_make(format("%shelp", path));
	ANGBAND_DIR_INFO = string_make(format("%sinfo", path));
	ANGBAND_DIR_PREF = string_make(format("%spref", path));
	ANGBAND_DIR_XTRA = string_make(format("%sxtra", path));


#ifdef PRIVATE_USER_PATH

	/* Build the path to the user specific directory */
	path_build(buf, sizeof(buf), PRIVATE_USER_PATH, VERSION_NAME);
	ANGBAND_DIR_USER = string_make(buf);

#else /* PRIVATE_USER_PATH */

        ANGBAND_DIR_USER = string_make(format("%suser", path));

#endif /* PRIVATE_USER_PATH */


#ifdef USE_PRIVATE_PATHS

	/* Build the path to the user specific sub-directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores");
	ANGBAND_DIR_APEX = string_make(buf);

	/* Build the path to the user specific sub-directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "bone");
	ANGBAND_DIR_BONE = string_make(buf);

	/* Build the path to the user specific sub-directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "data");
	ANGBAND_DIR_DATA = string_make(buf);

	/* Build the path to the user specific sub-directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "save");
	ANGBAND_DIR_SAVE = string_make(buf);

#else /* USE_PRIVATE_PATHS */

	/* Build pathnames */
	ANGBAND_DIR_APEX = string_make(format("%sapex", path));
	ANGBAND_DIR_BONE = string_make(format("%sbone", path));
	ANGBAND_DIR_DATA = string_make(format("%sdata", path));
	ANGBAND_DIR_SAVE = string_make(format("%ssave", path));

#endif /* USE_PRIVATE_PATHS */
}


#ifdef PRIVATE_USER_PATH

/*
 * Create an ".angband/" directory in the users home directory.
 *
 * ToDo: Add error handling.
 * ToDo: Only create the directories when actually writing files.
 */
void create_user_dirs(void)
{
	char dirpath[1024];
	char subdirpath[1024];


	/* Get an absolute path from the filename */
	path_build(dirpath, sizeof(dirpath), PRIVATE_USER_PATH, "");

	/* Create the ~/.angband/ directory */
	mkdir(dirpath, 0700);

	/* Build the path to the variant-specific sub-directory */
	path_build(subdirpath, sizeof(subdirpath), dirpath, VERSION_NAME);

	/* Create the directory */
	mkdir(subdirpath, 0700);

#ifdef USE_PRIVATE_PATHS
	/* Build the path to the scores sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "scores");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "bone");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "data");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "save");

	/* Create the directory */
	mkdir(dirpath, 0700);
#endif /* USE_PRIVATE_PATHS */
}

#endif /* PRIVATE_USER_PATH */



#ifdef ALLOW_TEMPLATES


/*
 * Hack -- help give useful error messages
 */
int error_idx;
int error_line;


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
	"too many allocation entries",
	"invalid spell frequency",
	"invalid number of items (0-99)",
	"too many entries",
	"vault too big",
};


#endif /* ALLOW_TEMPLATES */


/*
 * File headers
 */
header z_head;
header v_head;
header f_head;
header k_head;
header a_head;
header e_head;
header r_head;
header p_head;
header c_head;
header h_head;
header b_head;
header g_head;
header flavor_head;
header s_head;



/*** Initialize from binary image files ***/


/*
 * Initialize a "*_info" array, by parsing a binary "image" file
 */
static bool init_info_raw(const char *fname, header *head)
{
	header test;
	ang_file *fh = file_open(fname, MODE_READ, -1);

	if (!fh) return FALSE;

	/* Read and verify the header */
	if (!file_read(fh, (char *)(&test), sizeof(header)) ||
	    (test.v_major != head->v_major) ||
	    (test.v_minor != head->v_minor) ||
	    (test.v_patch != head->v_patch) ||
	    (test.v_extra != head->v_extra) ||
	    (test.info_num != head->info_num) ||
	    (test.info_len != head->info_len) ||
	    (test.head_size != head->head_size) ||
	    (test.info_size != head->info_size))
	{
		file_close(fh);
		return FALSE;
	}


	/* Accept the header */
	COPY(head, &test, header);


	/* Allocate and read the "*_info" array */
	head->info_ptr = C_RNEW(head->info_size, char);
	file_read(fh, head->info_ptr, head->info_size);

	if (head->name_size)
	{
		/* Allocate and read the "*_name" array */
		head->name_ptr = C_RNEW(head->name_size, char);
		file_read(fh, head->name_ptr, head->name_size);
	}

	if (head->text_size)
	{
		/* Allocate and read the "*_text" array */
		head->text_ptr = C_RNEW(head->text_size, char);
		file_read(fh, head->text_ptr, head->text_size);
	}

	file_close(fh);
	return TRUE;
}


/*
 * Initialize the header of an *_info.raw file.
 */
static void init_header(header *head, int num, int len)
{
	/* Save the "version" */
	head->v_major = VERSION_MAJOR;
	head->v_minor = VERSION_MINOR;
	head->v_patch = VERSION_PATCH;
	head->v_extra = VERSION_EXTRA;

	/* Save the "record" information */
	head->info_num = num;
	head->info_len = len;

	/* Save the size of "*_head" and "*_info" */
	head->head_size = sizeof(header);
	head->info_size = head->info_num * head->info_len;

	/* Clear post-parsing evaluation function */
	head->eval_info_power = NULL;
	
	/* Clear the template emission functions */
	head->emit_info_txt_index = NULL;
	head->emit_info_txt_always = NULL;
}


#ifdef ALLOW_TEMPLATES

/*
 * Display a parser error message.
 */
static void display_parse_error(cptr filename, errr err, cptr buf)
{
	cptr oops;

	/* Error string */
	oops = (((err > 0) && (err < PARSE_ERROR_MAX)) ? err_str[err] : "unknown");

	/* Oops */
	msg_format("Error at line %d of '%s.txt'.", error_line, filename);
	msg_format("Record %d contains a '%s' error.", error_idx, oops);
	msg_format("Parsing '%s'.", buf);
	message_flush();

	/* Quit */
	quit_fmt("Error in '%s.txt' file.", filename);
}

#endif /* ALLOW_TEMPLATES */


/*
 * Initialize a "*_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_info(cptr filename, header *head)
{
	ang_file *fh;

	errr err = 1;

	char raw_file[1024];
	char txt_file[1024];

	char buf[1024];


	/* Build the filenames */
	path_build(raw_file, sizeof(raw_file), ANGBAND_DIR_DATA, format("%s.raw", filename));
	path_build(txt_file, sizeof(txt_file), ANGBAND_DIR_EDIT, format("%s.txt", filename));


#ifdef ALLOW_TEMPLATES

	/* If the raw file's more recent than the text file, load it */
	if (file_newer(raw_file, txt_file) &&
	    init_info_raw(raw_file, head))
	{
		return 0;
	}


	/*** Make the fake arrays ***/

	/* Allocate the "*_info" array */
	head->info_ptr = C_ZNEW(head->info_size, char);

	/* MegaHack -- make "fake" arrays */
	if (z_info)
	{
		head->name_ptr = C_ZNEW(z_info->fake_name_size, char);
		head->text_ptr = C_ZNEW(z_info->fake_text_size, char);
	}


	/*** Load the ascii template file ***/

	/* Open the file */
	fh = file_open(txt_file, MODE_READ, -1);
	if (!fh) quit(format("Cannot open '%s.txt' file.", filename));

	/* Parse the file */
	err = init_info_txt(fh, buf, head, head->parse_info_txt);

	file_close(fh);

	/* Errors */
	if (err) display_parse_error(filename, err, buf);

	/* Post processing the data */
	if (head->eval_info_power) eval_info(head->eval_info_power, head);

#ifdef ALLOW_TEMPLATES_OUTPUT

	/*** Output a 'parsable' ascii template file ***/
	if ((head->emit_info_txt_index) || (head->emit_info_txt_always))
	{
		char user_file[1024];
		ang_file *fout;

		/* Open the original */
		fh = file_open(txt_file, MODE_READ, -1);
		if (!fh) quit(format("Cannot open '%s.txt' file for re-parsing.", filename));

		/* Open for output */
		path_build(user_file, 1024, ANGBAND_DIR_USER, format("%s.txt", filename));
		fout = file_open(user_file, MODE_WRITE, FTYPE_TEXT);
		if (!fout) quit(format("Cannot open '%s.txt' file for output.", filename));

		/* Parse and output the files */
		err = emit_info_txt(fout, fh, user_file, head, head->emit_info_txt_index, head->emit_info_txt_always);

		/* Close both files */
		file_close(fh);
		file_close(fout);
	}

#endif


	/*** Dump the binary image file ***/

	safe_setuid_grab();
	fh = file_open(raw_file, MODE_WRITE, FTYPE_RAW);
	safe_setuid_drop();

	/* Failure */
	if (!fh)
	{
		plog_fmt("Cannot write the '%s' file!", raw_file);
		return (0);
	}

	/* Dump it */
	file_write(fh, (const char *) head, head->head_size);

	/* Dump the "*_info" array */
	if (head->info_size > 0)
		file_write(fh, head->info_ptr, head->info_size);

	/* Dump the "*_name" array */
	if (head->name_size > 0)
		file_write(fh, head->name_ptr, head->name_size);

	/* Dump the "*_text" array */
	if (head->text_size > 0)
		file_write(fh, head->text_ptr, head->text_size);

	/* Close */
	file_close(fh);


	/*** Kill the fake arrays ***/

	/* Free the "*_info" array */
	FREE(head->info_ptr);

	/* MegaHack -- Free the "fake" arrays */
	if (z_info)
	{
		FREE(head->name_ptr);
		FREE(head->text_ptr);
	}


#endif /* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	if (!init_info_raw(raw_file, head))
		quit(format("Cannot load '%s.raw' file.", filename));


	/* Success */
	return (0);
}


/*
 * Free the allocated memory for the info-, name-, and text- arrays.
 */
static errr free_info(header *head)
{
	if (head->info_size)
		FREE(head->info_ptr);

	if (head->name_size)
		FREE(head->name_ptr);

	if (head->text_size)
		FREE(head->text_ptr);

	/* Success */
	return (0);
}


/*
 * Initialize the "z_info" array
 */
static errr init_z_info(void)
{
	errr err;

	/* Init the header */
	init_header(&z_head, 1, sizeof(maxima));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	z_head.parse_info_txt = parse_z_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("limits", &z_head);

	/* Set the global variables */
	z_info = z_head.info_ptr;

	return (err);
}


/*
 * Initialize the "f_info" array
 */
static errr init_f_info(void)
{
	errr err;

	/* Init the header */
	init_header(&f_head, z_info->f_max, sizeof(feature_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	f_head.parse_info_txt = parse_f_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("terrain", &f_head);

	/* Set the global variables */
	f_info = f_head.info_ptr;
	f_name = f_head.name_ptr;
	f_text = f_head.text_ptr;

	return (err);
}



/*
 * Initialize the "k_info" array
 */
static errr init_k_info(void)
{
	errr err;

	/* Init the header */
	init_header(&k_head, z_info->k_max, sizeof(object_kind));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	k_head.parse_info_txt = parse_k_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("object", &k_head);

	/* Set the global variables */
	k_info = k_head.info_ptr;
	k_name = k_head.name_ptr;
	k_text = k_head.text_ptr;

	return (err);
}



/*
 * Initialize the "a_info" array
 */
static errr init_a_info(void)
{
	errr err;

	/* Init the header */
	init_header(&a_head, z_info->a_max, sizeof(artifact_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	a_head.parse_info_txt = parse_a_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("artifact", &a_head);

	/* Set the global variables */
	a_info = a_head.info_ptr;
	a_name = a_head.name_ptr;
	a_text = a_head.text_ptr;

	return (err);
}



/*
 * Initialize the "e_info" array
 */
static errr init_e_info(void)
{
	errr err;

	/* Init the header */
	init_header(&e_head, z_info->e_max, sizeof(ego_item_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	e_head.parse_info_txt = parse_e_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("ego_item", &e_head);

	/* Set the global variables */
	e_info = e_head.info_ptr;
	e_name = e_head.name_ptr;
	e_text = e_head.text_ptr;

	return (err);
}



/*
 * Initialize the "r_info" array
 */
static errr init_r_info(void)
{
	errr err;

	/* Init the header */
	init_header(&r_head, z_info->r_max, sizeof(monster_race));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	r_head.parse_info_txt = parse_r_info;

#ifdef ALLOW_TEMPLATES_PROCESS
	/* Save a pointer to the evaluate power function*/
	r_head.eval_info_power = eval_r_power;
#endif

#ifdef ALLOW_TEMPLATES_OUTPUT

	/* Save a pointer to the evaluate power function*/
	r_head.emit_info_txt_index = emit_r_info_index;
#endif /* ALLOW_TEMPLATES_OUTPUT */

#endif /* ALLOW_TEMPLATES */

	err = init_info("monster", &r_head);

	/* Set the global variables */
	r_info = r_head.info_ptr;
	r_name = r_head.name_ptr;
	r_text = r_head.text_ptr;

	return (err);
}



/*
 * Initialize the "v_info" array
 */
static errr init_v_info(void)
{
	errr err;

	/* Init the header */
	init_header(&v_head, z_info->v_max, sizeof(vault_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	v_head.parse_info_txt = parse_v_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("vault", &v_head);

	/* Set the global variables */
	v_info = v_head.info_ptr;
	v_name = v_head.name_ptr;
	v_text = v_head.text_ptr;

	return (err);
}


/*
 * Initialize the "p_info" array
 */
static errr init_p_info(void)
{
	errr err;

	/* Init the header */
	init_header(&p_head, z_info->p_max, sizeof(player_race));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	p_head.parse_info_txt = parse_p_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_race", &p_head);

	/* Set the global variables */
	p_info = p_head.info_ptr;
	p_name = p_head.name_ptr;
	p_text = p_head.text_ptr;

	return (err);
}


/*
 * Initialize the "c_info" array
 */
static errr init_c_info(void)
{
	errr err;

	/* Init the header */
	init_header(&c_head, z_info->c_max, sizeof(player_class));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	c_head.parse_info_txt = parse_c_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_class", &c_head);

	/* Set the global variables */
	c_info = c_head.info_ptr;
	c_name = c_head.name_ptr;
	c_text = c_head.text_ptr;

	return (err);
}



/*
 * Initialize the "h_info" array
 */
static errr init_h_info(void)
{
	errr err;

	/* Init the header */
	init_header(&h_head, z_info->h_max, sizeof(hist_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	h_head.parse_info_txt = parse_h_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("p_hist", &h_head);

	/* Set the global variables */
	h_info = h_head.info_ptr;
	h_text = h_head.text_ptr;

	return (err);
}



/*
 * Initialize the "b_info" array
 */
static errr init_b_info(void)
{
	errr err;

	/* Init the header */
	init_header(&b_head, (u16b)(MAX_STORES * z_info->b_max), sizeof(owner_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	b_head.parse_info_txt = parse_b_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("shop_own", &b_head);

	/* Set the global variables */
	b_info = b_head.info_ptr;
	b_name = b_head.name_ptr;
	b_text = b_head.text_ptr;

	return (err);
}



/*
 * Initialize the "g_info" array
 */
static errr init_g_info(void)
{
	errr err;

	/* Init the header */
	init_header(&g_head, (u16b)(z_info->p_max * z_info->p_max), sizeof(byte));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	g_head.parse_info_txt = parse_g_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("cost_adj", &g_head);

	/* Set the global variables */
	g_info = g_head.info_ptr;
	g_name = g_head.name_ptr;
	g_text = g_head.text_ptr;

	return (err);
}


/*
 * Initialize the "flavor_info" array
 */
static errr init_flavor_info(void)
{
	errr err;

	/* Init the header */
	init_header(&flavor_head, z_info->flavor_max, sizeof(flavor_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	flavor_head.parse_info_txt = parse_flavor_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("flavor", &flavor_head);

	/* Set the global variables */
	flavor_info = flavor_head.info_ptr;
	flavor_name = flavor_head.name_ptr;
	flavor_text = flavor_head.text_ptr;

	return (err);
}



/*
 * Initialize the "s_info" array
 */
static errr init_s_info(void)
{
	errr err;

	/* Init the header */
	init_header(&s_head, z_info->s_max, sizeof(spell_type));

#ifdef ALLOW_TEMPLATES

	/* Save a pointer to the parsing function */
	s_head.parse_info_txt = parse_s_info;

#endif /* ALLOW_TEMPLATES */

	err = init_info("spell", &s_head);

	/* Set the global variables */
	s_info = s_head.info_ptr;
	s_name = s_head.name_ptr;
	s_text = s_head.text_ptr;

	return (err);
}

/*
 * Initialize the "spell_list" array
 */
static void init_books(void)
{
	byte realm, sval, snum;
	u16b spell;

	/* Since not all slots in all books are used, initialize to -1 first */
	for (realm = 0; realm < MAX_REALMS; realm++)
	{
		for (sval = 0; sval < BOOKS_PER_REALM; sval++)
		{
			for (snum = 0; snum < SPELLS_PER_BOOK; snum++)
			{
				spell_list[realm][sval][snum] = -1;
			}
		}
	}

	/* Place each spell in its own book */
	for (spell = 0; spell < z_info->s_max; spell++)
	{
		/* Get the spell */
		spell_type *s_ptr = &s_info[spell];

		/* Put it in the book */
		spell_list[s_ptr->realm][s_ptr->sval][s_ptr->snum] = spell;
	}
}

/*** Initialize others ***/

static void autoinscribe_init(void)
{
	if (inscriptions)
		FREE(inscriptions);
 
	inscriptions = 0;
	inscriptions_count = 0;

	inscriptions = C_ZNEW(AUTOINSCRIPTIONS_MAX, autoinscription);
}


/*
 * Initialize some other arrays
 */
static errr init_other(void)
{
	int i;


	/*** Prepare the various "bizarre" arrays ***/

	/* Initialize the "macro" package */
	(void)macro_init();

	/* Initialize the "quark" package */
	(void)quarks_init();

	/* Initialize squelch things */
	autoinscribe_init();

	/* Initialize the "message" package */
	(void)messages_init();

	/*** Prepare grid arrays ***/

	/* Array of grids */
	view_g = C_ZNEW(VIEW_MAX, u16b);

	/* Array of grids */
	temp_g = C_ZNEW(TEMP_MAX, u16b);

	/* Hack -- use some memory twice */
	temp_y = ((byte*)(temp_g)) + 0;
	temp_x = ((byte*)(temp_g)) + TEMP_MAX;


	/*** Prepare dungeon arrays ***/

	/* Padded into array */
	cave_info = C_ZNEW(DUNGEON_HGT, byte_256);
	cave_info2 = C_ZNEW(DUNGEON_HGT, byte_256);

	/* Feature array */
	cave_feat = C_ZNEW(DUNGEON_HGT, byte_wid);

	/* Entity arrays */
	cave_o_idx = C_ZNEW(DUNGEON_HGT, s16b_wid);
	cave_m_idx = C_ZNEW(DUNGEON_HGT, s16b_wid);

#ifdef MONSTER_FLOW

	/* Flow arrays */
	cave_cost = C_ZNEW(DUNGEON_HGT, byte_wid);
	cave_when = C_ZNEW(DUNGEON_HGT, byte_wid);

#endif /* MONSTER_FLOW */

	/*** Prepare "vinfo" array ***/

	/* Used by "update_view()" */
	(void)vinfo_init();


	/*** Prepare entity arrays ***/

	/* Objects */
	o_list = C_ZNEW(z_info->o_max, object_type);

	/* Monsters */
	mon_list = C_ZNEW(z_info->m_max, monster_type);


	/*** Prepare lore array ***/

	/* Lore */
	l_list = C_ZNEW(z_info->r_max, monster_lore);


	/*** Prepare quest array ***/

	/* Quests */
	q_list = C_ZNEW(MAX_Q_IDX, quest);


	/*** Prepare the inventory ***/

	/* Allocate it */
	inventory = C_ZNEW(INVEN_TOTAL, object_type);


	/*** Prepare the stores ***/

	/* Allocate the stores */
	store = C_ZNEW(MAX_STORES, store_type);

	/* Fill in each store */
	for (i = 0; i < MAX_STORES; i++)
	{
		int k;

		/* Get the store */
		store_type *st_ptr = &store[i];

		/* Assume full stock */
		st_ptr->stock_size = STORE_INVEN_MAX;

		/* Allocate the stock */
		st_ptr->stock = C_ZNEW(st_ptr->stock_size, object_type);

		/* No table for the black market or home */
		if ((i == STORE_B_MARKET) || (i == STORE_HOME)) continue;

		/* Assume full table */
		st_ptr->table_size = STORE_CHOICES;

		/* Allocate the stock */
		st_ptr->table = C_ZNEW(st_ptr->table_size, s16b);

		/* Scan the choices */
		for (k = 0; k < STORE_CHOICES; k++)
		{
			int k_idx;

			/* Extract the tval/sval codes */
			int tv = store_choices[i][k][0];
			int sv = store_choices[i][k][1];

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
	option_set_defaults();

	/* Initialize the window flags */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		/* Assume no flags */
		op_ptr->window_flag[i] = 0L;
	}


	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void)format("I wish you could swim, like dolphins can swim...");


	/* Success */
	return (0);
}



/*
 * Initialize some other arrays
 */
static errr init_alloc(void)
{
	int i;

	monster_race *r_ptr;

	ego_item_type *e_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH];

	s16b aux[MAX_DEPTH];


	/*** Initialize object allocation info ***/

	init_obj_alloc();



	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	(void)C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(num, MAX_DEPTH, s16b);

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
	alloc_race_table = C_ZNEW(alloc_race_size, alloc_entry);

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
	(void)C_WIPE(aux, MAX_DEPTH, s16b);

	/* Clear the "num" array */
	(void)C_WIPE(num, MAX_DEPTH, s16b);

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
	alloc_ego_table = C_ZNEW(alloc_ego_size, alloc_entry);

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
 */
static void init_angband_aux(cptr why)
{
	quit_fmt("%s\n\n%s", why,
	         "The 'lib' directory is probably missing or broken.\n"
	         "Perhaps the archive was not extracted correctly.\n"
	         "See the 'readme.txt' file for more information.");
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
	ang_file *fp;

	char buf[1024];


	/*** Verify the "news" file ***/

	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "news.txt");
	if (!file_exists(buf))
	{
		char why[1024];

		/* Crash and burn */
		strnfmt(why, sizeof(why), "Cannot access the '%s' file!", buf);
		init_angband_aux(why);
	}


	/*** Display the "news" file ***/

	Term_clear();

	/* Open the News file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "news.txt");
	fp = file_open(buf, MODE_READ, -1);

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
			Term_putstr(0, i++, -1, TERM_WHITE, buf);

		file_close(fp);
	}

	/* Flush it */
	Term_fresh();



	/* Initialize the menus */
	/* This must occur before preference files are read(?) */
	init_cmd4_c();
	
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

	/* Initialize class info */
	note("[Initializing arrays... (classes)]");
	if (init_c_info()) quit("Cannot initialize classes");

	/* Initialize owner info */
	note("[Initializing arrays... (owners)]");
	if (init_b_info()) quit("Cannot initialize owners");

	/* Initialize price info */
	note("[Initializing arrays... (prices)]");
	if (init_g_info()) quit("Cannot initialize prices");

	/* Initialize flavor info */
	note("[Initializing arrays... (flavors)]");
	if (init_flavor_info()) quit("Cannot initialize flavors");
	
	/* Initialize spell info */
	note("[Initializing arrays... (spells)]");
	if (init_s_info()) quit("Cannot initialize spells");

	/* Initialize spellbook info */
	note("[Initializing arrays... (spellbooks)]");
	init_books();

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

	/* Sneakily init command list */
	cmd_init();
}


void cleanup_angband(void)
{
	int i;


	/* Free the macros */
	macro_free();

	/* Free the macro triggers */
	macro_trigger_free();

	/* Free the allocation tables */
	free_obj_alloc();
	FREE(alloc_ego_table);
	FREE(alloc_race_table);

	if (store)
	{
		/* Free the store inventories */
		for (i = 0; i < MAX_STORES; i++)
		{
			/* Get the store */
			store_type *st_ptr = &store[i];

			/* Free the store inventory */
			FREE(st_ptr->stock);
			FREE(st_ptr->table);
		}
	}


	/* Free the stores */
	FREE(store);

	/* Free the player inventory */
	FREE(inventory);

	/* Free the quest list */
	FREE(q_list);

	/* Free the lore, monster, and object lists */
	FREE(l_list);
	FREE(mon_list);
	FREE(o_list);

#ifdef MONSTER_FLOW

	/* Flow arrays */
	FREE(cave_when);
	FREE(cave_cost);

#endif /* MONSTER_FLOW */

	/* Free the cave */
	FREE(cave_o_idx);
	FREE(cave_m_idx);
	FREE(cave_feat);
	FREE(cave_info2);
	FREE(cave_info);

	/* Free the "update_view()" array */
	FREE(view_g);

	/* Free the temp array */
	FREE(temp_g);

	/* Free the messages */
	messages_free();

	/* Free the "quarks" */
	quarks_free();

	/* Free the info, name, and text arrays */
	free_info(&flavor_head);
	free_info(&g_head);
	free_info(&b_head);
	free_info(&c_head);
	free_info(&p_head);
	free_info(&h_head);
	free_info(&v_head);
	free_info(&r_head);
	free_info(&e_head);
	free_info(&a_head);
	free_info(&k_head);
	free_info(&f_head);
	free_info(&z_head);
	free_info(&s_head);

	/* Free the format() buffer */
	vformat_kill();

	/* Free the directories */
	string_free(ANGBAND_DIR);
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
	string_free(ANGBAND_DIR_DATA);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);
}
