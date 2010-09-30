/*
 * File: init2.c
 * Purpose: Various game initialistion routines
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "angband.h"

#include "init.h"
#include "cmds.h"
#include "option.h"
#include "game-event.h"
#include "game-cmd.h"
#include "parser.h"

/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/edit" directory.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 */

struct file_parser {
	struct parser *(*init)(void);
	errr (*run)(struct parser *p);
	errr (*finish)(struct parser *p);
};

static errr run_parser(struct file_parser *fp) {
	struct parser *p = fp->init();
	errr r;
	if (!p) {
		return PARSE_ERROR_GENERIC;
	}
	r = fp->run(p);
	if (r) {
		return r;
	}
	r = fp->finish(p);
	return r;
}

static const char *k_info_flags[] = {
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

static const char *effect_list[] = {
	#define EFFECT(x, y, r, z)    #x,
	#include "list-effects.h"
	#undef EFFECT
};

static int lookup_flag(const char **flag_table, const char *flag_name) {
	int i = FLAG_START;

	while (flag_table[i] && !streq(flag_table[i], flag_name))
		i++;

	/* End of table reached without match */
	if (!flag_table[i]) i = FLAG_END;

	return i;
}

static errr grab_flag(bitflag *flags, const size_t size, const char **flag_table, const char *flag_name) {
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_on(flags, size, flag);

	return 0;
}

static u32b grab_one_effect(const char *what) {
	size_t i;

	/* Scan activations */
	for (i = 0; i < N_ELEMENTS(effect_list); i++)
	{
		if (streq(what, effect_list[i]))
			return i;
	}

	/* Oops */
	msg_format("Unknown effect '%s'.", what);

	/* Error */
	return 0;
}

/*
 * Find the default paths to all of our important sub-directories.
 *
 * All of the sub-directories should, by default, be located inside
 * the main directory, whose location is very system dependant and is 
 * set by the ANGBAND_PATH environment variable, if it exists. (On multi-
 * user systems such as Linux this is not the default - see config.h for
 * more info.)
 *
 * This function takes a writable buffers, initially containing the
 * "path" to the "config", "lib" and "data" directories, for example, 
 * "/etc/angband/", "/usr/share/angband" and "/var/games/angband" -
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "apex" and "user" and "save" directories,
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
void init_file_paths(const char *configpath, const char *libpath, const char *datapath)
{
#ifdef PRIVATE_USER_PATH
	char buf[1024];
#endif /* PRIVATE_USER_PATH */

	/*** Free everything ***/

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);

	string_free(ANGBAND_DIR_XTRA_FONT);
	string_free(ANGBAND_DIR_XTRA_GRAF);
	string_free(ANGBAND_DIR_XTRA_SOUND);
	string_free(ANGBAND_DIR_XTRA_HELP);
	string_free(ANGBAND_DIR_XTRA_ICON);

	/*** Prepare the paths ***/

	/* Build path names */
	ANGBAND_DIR_EDIT = string_make(format("%sedit", configpath));
	ANGBAND_DIR_FILE = string_make(format("%sfile", libpath));
	ANGBAND_DIR_HELP = string_make(format("%shelp", libpath));
	ANGBAND_DIR_INFO = string_make(format("%sinfo", libpath));
	ANGBAND_DIR_PREF = string_make(format("%spref", configpath));
	ANGBAND_DIR_XTRA = string_make(format("%sxtra", libpath));

	/* Build xtra/ paths */
	ANGBAND_DIR_XTRA_FONT = string_make(format("%s" PATH_SEP "font", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_GRAF = string_make(format("%s" PATH_SEP "graf", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_SOUND = string_make(format("%s" PATH_SEP "sound", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_HELP = string_make(format("%s" PATH_SEP "help", ANGBAND_DIR_XTRA));
	ANGBAND_DIR_XTRA_ICON = string_make(format("%s" PATH_SEP "icon", ANGBAND_DIR_XTRA));

#ifdef PRIVATE_USER_PATH

	/* Build the path to the user specific directory */
	if (strncmp(ANGBAND_SYS, "test", 4) == 0)
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, "Test");
	else
		path_build(buf, sizeof(buf), PRIVATE_USER_PATH, VERSION_NAME);
	ANGBAND_DIR_USER = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores");
	ANGBAND_DIR_APEX = string_make(buf);

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "save");
	ANGBAND_DIR_SAVE = string_make(buf);

#else /* !PRIVATE_USER_PATH */

	/* Build pathnames */
    ANGBAND_DIR_USER = string_make(format("%suser", datapath));
	ANGBAND_DIR_APEX = string_make(format("%sapex", datapath));
	ANGBAND_DIR_SAVE = string_make(format("%ssave", datapath));

#endif /* PRIVATE_USER_PATH */
}


/*
 * Create any missing directories. We create only those dirs which may be
 * empty (user/, save/, apex/, info/, help/). The others are assumed 
 * to contain required files and therefore must exist at startup 
 * (edit/, pref/, file/, xtra/).
 *
 * ToDo: Only create the directories when actually writing files.
 */
void create_needed_dirs(void)
{
	char dirpath[512];

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_USER, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_SAVE, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_APEX, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_INFO, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);

	path_build(dirpath, sizeof(dirpath), ANGBAND_DIR_HELP, "");
	if (!dir_create(dirpath)) quit_fmt("Cannot create '%s'", dirpath);
}


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
	"invalid flag specification",
	"invalid number of items (0-99)",
	"invalid spell frequency",
	"invalid random value",
	"missing colon",
	"missing field",
	"missing record header",
	"non-sequential records",
	"value not a number",
	"obsolete file",
	"value out of bounds",
	"out of memory",
	"too few entries",
	"too many entries",
	"undefined directive",
	"unrecognised blow",
	"unrecognised tval name",
	"unrecognised sval name",
	"vault too big",
};


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


/*
 * Initialize the header of an *_info array.
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
       head->eval_info_post = NULL;

       /* Clear the template emission functions */
       head->emit_info_txt_index = NULL;
       head->emit_info_txt_always = NULL;
}


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

	char txt_file[1024];

	char buf[1024];

	void *fake_name;
	void *fake_text;

	/* Build the filename */
	path_build(txt_file, sizeof(txt_file), ANGBAND_DIR_EDIT, format("%s.txt", filename));

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
	if (head->eval_info_post) eval_info(head->eval_info_post, head);


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

	/* Copy the parsed data into the real array from the fakes */
	fake_name = head->name_ptr;
	head->name_ptr = C_ZNEW(head->name_size, char);
	memcpy(head->name_ptr, fake_name, head->name_size);

	fake_text = head->text_ptr;
	head->text_ptr = C_ZNEW(head->text_size, char);
	memcpy(head->text_ptr, fake_text, head->text_size);

	/* Free the fake arrays */
	if (z_info)
	{
		FREE(fake_name);
		FREE(fake_text);
	}

	/* Success */
	return (0);
}

errr parse_file(struct parser *p, const char *filename) {
	char path[1024];
	char buf[1024];
	ang_file *fh;
	errr r;

	path_build(path, sizeof(path), ANGBAND_DIR_EDIT, format("%s.txt", filename));
	fh = file_open(path, MODE_READ, -1);
	if (!fh)
		quit(format("Cannot open '%s.txt'", filename));
	while (file_getl(fh, buf, sizeof(buf))) {
		r = parser_parse(p, buf);
		if (r)
			break;
	}
	file_close(fh);
	return r;
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

static enum parser_error ignored(struct parser *p) {
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_z(struct parser *p) {
	maxima *z;
	const char *label;
	int value;

	z = parser_priv(p);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");

	if (value < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (streq(label, "F"))
		z->f_max = value;
	else if (streq(label, "K"))
		z->k_max = value;
	else if (streq(label, "A"))
		z->a_max = value;
	else if (streq(label, "E"))
		z->e_max = value;
	else if (streq(label, "R"))
		z->r_max = value;
	else if (streq(label, "V"))
		z->v_max = value;
	else if (streq(label, "P"))
		z->p_max = value;
	else if (streq(label, "C"))
		z->c_max = value;
	else if (streq(label, "H"))
		z->h_max = value;
	else if (streq(label, "B"))
		z->b_max = value;
	else if (streq(label, "S"))
		z->s_max = value;
	else if (streq(label, "O"))
		z->o_max = value;
	else if (streq(label, "M"))
		z->m_max = value;
	else if (streq(label, "L"))
		z->flavor_max = value;
	else if (streq(label, "N"))
		z->fake_name_size = value;
	else if (streq(label, "T"))
		z->fake_text_size = value;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return 0;
}

struct parser *init_parse_z(void) {
	struct maxima *z = mem_alloc(sizeof *z);
	struct parser *p = parser_new();

	parser_setpriv(p, z);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "M sym label int value", parse_z);
	return p;
}

static errr run_parse_z(struct parser *p) {
	return parse_file(p, "limits");
}

static errr finish_parse_z(struct parser *p) {
	z_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static struct file_parser z_parser = {
	init_parse_z,
	run_parse_z,
	finish_parse_z
};

static enum parser_error parse_k_n(struct parser *p) {
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct object_kind *h = parser_priv(p);

	struct object_kind *k = mem_alloc(sizeof *k);
	memset(k, 0, sizeof(*k));
	k->next = h;
	parser_setpriv(p, k);
	k->kidx = idx;
	k->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_g(struct parser *p) {
	const char *sym = parser_getsym(p, "char");
	const char *color = parser_getsym(p, "color");
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->d_char = sym[0];
	if (strlen(color) > 1)
		k->d_attr = color_text_to_attr(color);
	else
		k->d_attr = color_char_to_attr(color[0]);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_i(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->tval = parser_getint(p, "tval");
	k->sval = parser_getint(p, "sval");
	k->pval = parser_getrand(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_w(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->level = parser_getint(p, "level");
	k->weight = parser_getint(p, "weight");
	k->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_a(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;
	assert(k);

	k->alloc_prob = parser_getint(p, "common");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_GENERIC;

	k->alloc_min = amin;
	k->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_p(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(k);

	k->ac = parser_getint(p, "ac");
	k->dd = hd.dice;
	k->ds = hd.sides;
	k->to_h = parser_getrand(p, "to-h");
	k->to_d = parser_getrand(p, "to-d");
	k->to_a = parser_getrand(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_c(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->charge = parser_getrand(p, "charges");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_m(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->gen_mult_prob = parser_getint(p, "prob");
	k->stack_size = parser_getrand(p, "stack");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_f(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	char *s = string_make(parser_getstr(p, "flags"));
	char *t;
	assert(k);

	t = strtok(s, " |");
	while (t) {
		if (grab_flag(k->flags, OF_SIZE, k_info_flags, t))
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;

}

static enum parser_error parse_k_e(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);

	k->effect = grab_one_effect(parser_getsym(p, "name"));
	if (parser_hasval(p, "time"))
		k->time = parser_getrand(p, "time");
	if (!k->effect)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_k_d(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	assert(k);
	k->text = string_append(k->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_k(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_k_n);
	parser_reg(p, "G sym char sym color", parse_k_g);
	parser_reg(p, "I int tval int sval rand pval", parse_k_i);
	parser_reg(p, "W int level int extra int weight int cost", parse_k_w);
	parser_reg(p, "A int common str minmax", parse_k_a);
	parser_reg(p, "P int ac rand hd rand to-h rand to-d rand to-a", parse_k_p);
	parser_reg(p, "C rand charges", parse_k_c);
	parser_reg(p, "M int prob rand stack", parse_k_m);
	parser_reg(p, "F str flags", parse_k_f);
	parser_reg(p, "E sym name ?rand time", parse_k_e);
	parser_reg(p, "D str text", parse_k_d);
	return p;
}

static errr run_parse_k(struct parser *p) {
	return parse_file(p, "object");
}

static errr finish_parse_k(struct parser *p) {
	struct object_kind *k;

	k_info = mem_alloc(z_info->k_max * sizeof(*k));
	for (k = parser_priv(p); k; k = k->next) {
		if (k->kidx >= z_info->k_max)
			continue;
		memcpy(&k_info[k->kidx], k, sizeof(*k));
	}

	return 0;
}

struct file_parser k_parser = {
	init_parse_k,
	run_parse_k,
	finish_parse_k
};

static enum parser_error parse_a_n(struct parser *p) {
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct artifact *h = parser_priv(p);

	struct artifact *a = mem_alloc(sizeof *a);
	memset(a, 0, sizeof(*a));
	a->next = h;
	parser_setpriv(p, a);
	a->aidx = idx;
	a->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_i(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *tv, *sv;
	int tvi, svi;
	assert(a);

	tv = parser_getsym(p, "tval");
	sv = parser_getsym(p, "sval");

	if (sscanf(tv, "%d", &tvi) != 1) {
		tvi = tval_find_idx(tv);
		if (tvi == -1)
			return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	if (tvi < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	a->tval = tvi;

	if (sscanf(sv, "%d", &svi) != 1) {
		svi = lookup_sval(a->tval, sv);
		if (svi == -1)
			return PARSE_ERROR_UNRECOGNISED_SVAL;
	}
	if (svi < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	a->sval = svi;

	a->pval = parser_getint(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_w(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->level = parser_getint(p, "level");
	a->rarity = parser_getint(p, "rarity");
	a->weight = parser_getint(p, "weight");
	a->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_a(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;
	assert(a);

	a->alloc_prob = parser_getint(p, "common");
	if (sscanf(tmp, "%d to %d", &amin, &amax) != 2)
		return PARSE_ERROR_GENERIC;

	if (amin > 255 || amax > 255 || amin < 0 || amax < 0)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	a->alloc_min = amin;
	a->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_p(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");
	assert(a);

	a->ac = parser_getint(p, "ac");
	a->dd = hd.dice;
	a->ds = hd.sides;
	a->to_h = parser_getint(p, "to-h");
	a->to_d = parser_getint(p, "to-d");
	a->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_f(struct parser *p) {
	struct artifact *a = parser_priv(p);
	char *s;
	char *t;
	assert(a);

	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));

	t = strtok(s, " |");
	while (t) {
		if (grab_flag(a->flags, OF_SIZE, k_info_flags, t))
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_a_e(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->effect = grab_one_effect(parser_getsym(p, "name"));
	a->time = parser_getrand(p, "time");
	if (!a->effect)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_m(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->effect_msg = string_append(a->effect_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_a_d(struct parser *p) {
	struct artifact *a = parser_priv(p);
	assert(a);

	a->text = string_append(a->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_a(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_a_n);
	parser_reg(p, "I sym tval sym sval int pval", parse_a_i);
	parser_reg(p, "W int level int rarity int weight int cost", parse_a_w);
	parser_reg(p, "A int common str minmax", parse_a_a);
	parser_reg(p, "P int ac rand hd int to-h int to-d int to-a", parse_a_p);
	parser_reg(p, "F ?str flags", parse_a_f);
	parser_reg(p, "E sym name rand time", parse_a_e);
	parser_reg(p, "M str text", parse_a_m);
	parser_reg(p, "D str text", parse_a_d);
	return p;
}

static errr run_parse_a(struct parser *p) {
	return parse_file(p, "artifact");
}

static errr finish_parse_a(struct parser *p) {
	struct artifact *a;

	a_info = mem_alloc(z_info->a_max * sizeof(*a));
	for (a = parser_priv(p); a; a = a->next) {
		if (a->aidx >= z_info->a_max)
			continue;
		memcpy(&a_info[a->aidx], a, sizeof(*a));
	}

	return 0;
}

struct file_parser a_parser = {
	init_parse_a,
	run_parse_a,
	finish_parse_a
};

struct name {
	struct name *next;
	char *str;
};

struct names_parse {
	unsigned int section;
	unsigned int nnames[RANDNAME_NUM_TYPES];
	struct name *names[RANDNAME_NUM_TYPES];
};

static enum parser_error parse_names_n(struct parser *p) {
	unsigned int section = parser_getint(p, "section");
	struct names_parse *s = parser_priv(p);
	if (s->section >= RANDNAME_NUM_TYPES)
		return PARSE_ERROR_GENERIC;
	s->section = section;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_names_d(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct names_parse *s = parser_priv(p);
	struct name *ns = mem_alloc(sizeof *ns);

	s->nnames[s->section]++;
	ns->next = s->names[s->section];
	ns->str = string_make(name);
	s->names[s->section] = ns;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_names(void) {
	struct parser *p = parser_new();
	struct names_parse *n = mem_alloc(sizeof *n);
	memset(n, 0, sizeof(*n));
	n->section = 0;
	parser_setpriv(p, n);
	parser_reg(p, "N int section", parse_names_n);
	parser_reg(p, "D str name", parse_names_d);
	return p;
}

static errr run_parse_names(struct parser *p) {
	return parse_file(p, "names");
}

static errr finish_parse_names(struct parser *p) {
	int i;
	unsigned int j;
	struct names_parse *n = parser_priv(p);
	struct name *nm;
	name_sections = mem_alloc(sizeof(char**) * RANDNAME_NUM_TYPES);
	for (i = 0; i < RANDNAME_NUM_TYPES; i++) {
		name_sections[i] = mem_alloc(sizeof(char*) * (n->nnames[i] + 1));
		for (nm = n->names[i], j = 0; nm && j < n->nnames[i]; nm = nm->next, j++) {
			name_sections[i][j] = nm->str;
		}
		name_sections[i][n->nnames[i]] = NULL;
		while (n->names[i]) {
			nm = n->names[i]->next;
			mem_free(n->names[i]);
			n->names[i] = nm;
		}
	}
	mem_free(n);
	return 0;
}

struct file_parser names_parser = {
	init_parse_names,
	run_parse_names,
	finish_parse_names
};

/*
static enum parser_error parse_f_n(struct parser *p) {
}

struct parser *init_parse_f(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "N int idx string name", parse_f_n);
	parser_reg(p, "G char glyph sym color", parse_f_g);
	parser_reg(p, "M int idx", parse_f_m);
	parser_reg(p, "P int priority", parse_f_p);
	parser_reg(p, "F str flags", parse_f_f);
	parser_reg(p, "X int locked int jammed int shopnum int dig", parse_f_x);
	parser_reg(p, "E str effect", parse_f_e);
	return p;
}
*/

static enum parser_error parse_e_n(struct parser *p) {
	int idx = parser_getint(p, "index");
	const char *name = parser_getstr(p, "name");
	struct ego_item *h = parser_priv(p);

	struct ego_item *e = mem_alloc(sizeof *e);
	memset(e, 0, sizeof(*e));
	e->next = h;
	parser_setpriv(p, e);
	e->eidx = idx;
	e->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_w(struct parser *p) {
	int level = parser_getint(p, "level");
	int rarity = parser_getint(p, "rarity");
	int cost = parser_getint(p, "cost");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->level = level;
	e->rarity = rarity;
	e->cost = cost;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_x(struct parser *p) {
	int rating = parser_getint(p, "rating");
	int xtra = parser_getint(p, "xtra");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->rating = rating;
	e->xtra = xtra;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_t(struct parser *p) {
	int tval = parser_getint(p, "tval");
	int min_sval = parser_getint(p, "min-sval");
	int max_sval = parser_getint(p, "max-sval");
	struct ego_item *e = parser_priv(p);
	int i;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < EGO_TVALS_MAX; i++) {
		if (!e->tval[i]) {
			e->tval[i] = tval;
			e->min_sval[i] = min_sval;
			e->max_sval[i] = max_sval;
			break;
		}
	}

	if (i == EGO_TVALS_MAX)
		return PARSE_ERROR_GENERIC;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_c(struct parser *p) {
	struct random th = parser_getrand(p, "th");
	struct random td = parser_getrand(p, "td");
	struct random ta = parser_getrand(p, "ta");
	struct random pval = parser_getrand(p, "pval");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	e->to_h = th;
	e->to_d = td;
	e->to_a = ta;
	e->pval = pval;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_m(struct parser *p) {
	int th = parser_getint(p, "th");
	int td = parser_getint(p, "td");
	int ta = parser_getint(p, "ta");
	int pval = parser_getint(p, "pval");
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	e->min_to_h = th;
	e->min_to_d = td;
	e->min_to_a = ta;
	e->min_pval = pval;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_e_f(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *s;
	char *t;

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		if (grab_flag(e->flags, OF_SIZE, k_info_flags,t))
			break;
		t = strtok(NULL, " |");
	}
	mem_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_e_d(struct parser *p) {
	struct ego_item *e = parser_priv(p);

	if (!e)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	e->text = string_append(e->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_e(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N int index str name", parse_e_n);
	parser_reg(p, "W int level int rarity int pad int cost", parse_e_w);
	parser_reg(p, "X int rating int xtra", parse_e_x);
	parser_reg(p, "T int tval int min-sval int max-sval", parse_e_t);
	parser_reg(p, "C rand th rand td rand ta rand pval", parse_e_c);
	parser_reg(p, "M int th int td int ta int pval", parse_e_m);
	parser_reg(p, "F ?str flags", parse_e_f);
	parser_reg(p, "D str text", parse_e_d);
	return p;
}

static errr run_parse_e(struct parser *p) {
	return parse_file(p, "ego_item");
}

static errr eval_e_slays(struct ego_item *items)
{
	int i;
	int j;
	int count = 0;
	bitflag cacheme[OF_SIZE];
	bitflag slay_mask[OF_SIZE];
	bitflag **dupcheck;
	ego_item_type *e_ptr;

	/* Build the slay mask */
	flags_init(slay_mask, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END);

	/* Calculate necessary size of slay_cache */
	dupcheck = C_ZNEW(z_info->e_max, bitflag *);
	
	for (i = 0; i < z_info->e_max; i++)
	{
		dupcheck[i] = C_ZNEW(OF_SIZE, bitflag);
		e_ptr = items + i;

		/* Find the slay flags on this ego */		
		of_copy(cacheme, e_ptr->flags);
		of_inter(cacheme, slay_mask);

		/* Only consider non-empty combinations of slay flags */
		if (!of_is_empty(cacheme))
		{
			/* Skip previously scanned combinations */
			for (j = 0; j < i; j++)
			{
				if (of_is_equal(cacheme, dupcheck[j]))
					continue;
			}

			/* msg_print("Found a new slay combo on an ego item"); */
			count++;
			of_copy(dupcheck[i], cacheme);
		}
	}

	/* Allocate slay_cache with an extra empty element for an iteration stop */
	slay_cache = C_ZNEW((count + 1), flag_cache);
	count = 0;

	/* Populate the slay_cache */
	for (i = 0; i < z_info->e_max; i++)
	{
		if (!of_is_empty(dupcheck[i]))
		{
			of_copy(slay_cache[count].flags, dupcheck[i]);
			slay_cache[count].value = 0;
			count++;
			/*msg_print("Cached a slay combination");*/
		}
	}

	for (i = 0; i < z_info->e_max; i++)
		FREE(dupcheck[i]);
	FREE(dupcheck);

	/* Success */
	return 0;
}

static errr finish_parse_e(struct parser *p) {
	struct ego_item *e;

	e_info = mem_alloc(z_info->e_max * sizeof(*e));
	for (e = parser_priv(p); e; e = e->next) {
		if (e->eidx >= z_info->e_max)
			continue;
		memcpy(&e_info[e->eidx], e, sizeof(*e));
	}

	eval_e_slays(e_info);

	return 0;
}

struct file_parser e_parser = {
	init_parse_e,
	run_parse_e,
	finish_parse_e
};

/*
 * Initialize the "f_info" array
 */
static errr init_f_info(void)
{
	errr err;

	/* Init the header */
	init_header(&f_head, z_info->f_max, sizeof(feature_type));

	/* Save a pointer to the parsing function */
	f_head.parse_info_txt = parse_f_info;

	err = init_info("terrain", &f_head);

	/* Set the global variables */
	f_info = f_head.info_ptr;

	return (err);
}

/*
 * Initialize the "r_info" array
 */
static errr init_r_info(void)
{
	int i;
	errr err;

	/* Init the header */
	init_header(&r_head, z_info->r_max, sizeof(monster_race));

	/* Save a pointer to the parsing function */
	r_head.parse_info_txt = parse_r_info;

	/* Save a pointer to the evaluate power function*/
	r_head.eval_info_post = eval_r_power;

	/* Save a pointer to the text file output function*/
	if (arg_rebalance) r_head.emit_info_txt_index = emit_r_info_index;

	err = init_info("monster", &r_head);

	/* Set the global variables */
	r_info = r_head.info_ptr;
	r_name = r_head.name_ptr;
	r_text = r_head.text_ptr;
	tot_mon_power = 0;
	for (i = 0; i < z_info->r_max; i++) 
	{
		tot_mon_power += r_info[i].power;
	} 

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

	/* Save a pointer to the parsing function */
	v_head.parse_info_txt = parse_v_info;

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

	/* Save a pointer to the parsing function */
	p_head.parse_info_txt = parse_p_info;

	err = init_info("p_race", &p_head);

	/* Set the global variables */
	p_info = p_head.info_ptr;

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

	/* Save a pointer to the parsing function */
	c_head.parse_info_txt = parse_c_info;

	err = init_info("p_class", &c_head);

	/* Set the global variables */
	c_info = c_head.info_ptr;

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

	/* Save a pointer to the parsing function */
	h_head.parse_info_txt = parse_h_info;

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

	/* Save a pointer to the parsing function */
	b_head.parse_info_txt = parse_b_info;

	err = init_info("shop_own", &b_head);

	/* Set the global variables */
	b_info = b_head.info_ptr;
	b_name = b_head.name_ptr;
	b_text = b_head.text_ptr;

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

	/* Save a pointer to the parsing function */
	flavor_head.parse_info_txt = parse_flavor_info;

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

	/* Save a pointer to the parsing function */
	s_head.parse_info_txt = parse_s_info;

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


/*
 * Initialise stores, from the edit file.
 */
static void init_stores(void)
{
	errr err;
	char filename[1024];
	char buf[1024];
	ang_file *fh;

	path_build(filename, sizeof(filename), ANGBAND_DIR_EDIT, "store.txt");

	/* Open the file */
	fh = file_open(filename, MODE_READ, -1);
	if (!fh) quit("Cannot open 'store.txt' file.");

	/* Parse the file */
	err = init_store_txt(fh, buf);
	file_close(fh);

	/* Errors */
	if (err) display_parse_error("store", err, buf);

	return;
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
	squelch_init();
	init_cmd_know();

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

	/* Flow arrays */
	cave_cost = C_ZNEW(DUNGEON_HGT, byte_wid);
	cave_when = C_ZNEW(DUNGEON_HGT, byte_wid);


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


	/*** Prepare mouse buttons ***/

	button_init(button_add_text, button_kill_text);


	/*** Prepare quest array ***/

	/* Quests */
	q_list = C_ZNEW(MAX_Q_IDX, quest);


	/*** Prepare the inventory ***/

	/* Allocate it */
	p_ptr->inventory = C_ZNEW(ALL_INVEN_TOTAL, object_type);



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
bool init_angband(void)
{
	event_signal(EVENT_ENTER_INIT);

	/* Initialize the menus */
	/* This must occur before preference files are read(?) */
	init_cmd4_c();


	/*** Initialize some arrays ***/

	/* Initialize size info */
	event_signal_string(EVENT_INITSTATUS, "Initializing array sizes...");
	if (run_parser(&z_parser)) quit("Cannot initialize sizes");

	/* Initialize feature info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (features)");
	if (init_f_info()) quit("Cannot initialize features");

	/* Initialize object info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (objects)");
	if (run_parser(&k_parser)) quit("Cannot initialize objects");

	/* Initialize ego-item info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (ego-items)");
	if (run_parser(&e_parser)) quit("Cannot initialize ego-items");

	/* Initialize monster info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (monsters)");
	if (init_r_info()) quit("Cannot initialize monsters");

	/* Initialize artifact info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (artifacts)");
	if (run_parser(&a_parser)) quit("Cannot initialize artifacts");

	/* Initialize feature info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (vaults)");
	if (init_v_info()) quit("Cannot initialize vaults");

	/* Initialize history info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (histories)");
	if (init_h_info()) quit("Cannot initialize histories");

	/* Initialize race info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (races)");
	if (init_p_info()) quit("Cannot initialize races");

	/* Initialize class info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (classes)");
	if (init_c_info()) quit("Cannot initialize classes");

	/* Initialize owner info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (owners)");
	if (init_b_info()) quit("Cannot initialize owners");

	/* Initialize flavor info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (flavors)");
	if (init_flavor_info()) quit("Cannot initialize flavors");

	/* Initialize spell info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (spells)");
	if (init_s_info()) quit("Cannot initialize spells");

	/* Initialize spellbook info */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (spellbooks)");
	init_books();

	/* Initialise store stocking data */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (store stocks)");
	init_stores();

	/* Initialise random name data */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (random names)");
	if (run_parser(&names_parser)) quit("Can't parse names");

	/* Initialize some other arrays */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (other)");
	if (init_other()) quit("Cannot initialize other stuff");

	/* Initialize some other arrays */
	event_signal_string(EVENT_INITSTATUS, "Initializing arrays... (alloc)");
	if (init_alloc()) quit("Cannot initialize alloc stuff");

	/*** Load default user pref files ***/

	/* Initialize feature info */
	event_signal_string(EVENT_INITSTATUS, "Loading basic user pref file...");

	/* Process that file */
	(void)process_pref_file("pref.prf");

	/* Done */
	event_signal_string(EVENT_INITSTATUS, "Initialization complete");

	/* Sneakily init command list */
	cmd_init();

	/* Ask for a "command" until we get one we like. */
	while (1)
	{
		game_command command_req;

		cmd_get(CMD_INIT, &command_req, TRUE);

		if (command_req.command == CMD_QUIT)
		{
			quit(NULL);
		}
		else if (command_req.command == CMD_NEWGAME)
		{
			event_signal(EVENT_LEAVE_INIT);
			return TRUE;
		}
		else if (command_req.command == CMD_LOADFILE)
		{
			event_signal(EVENT_LEAVE_INIT);
			/* In future we might want to pass back or set the savefile
			   path here. */
			return FALSE;
		}
	}
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

	/* Free the quest list */
	FREE(q_list);

	FREE(p_ptr->inventory);

	/* Free the lore, monster, and object lists */
	FREE(l_list);
	FREE(mon_list);
	FREE(o_list);

	/* Flow arrays */
	FREE(cave_when);
	FREE(cave_cost);

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
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);
}
