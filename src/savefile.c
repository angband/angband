/*
 * File: savefile.c
 * Purpose: Savefile loading and saving main routines
 *
 * Copyright (c) 2009 Andi Sidwell <andi@takkaria.org>
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
#include <errno.h>
#include "angband.h"
#include "savefile.h"

/**
 * The savefile code.
 *
 * Savefiles since ~3.1 have used a block-based system.  Each savefile
 * consists of an 8-byte header, the first four bytes of which mark this
 * as a savefile, the second four bytes provide a variant ID.
 *
 * After that, each block has the format:
 * - 16-byte string giving the type of block
 * - 4-byte block version
 * - 4-byte block size
 * - 4-byte block checksum
 * ... data ...
 * padding so that block is a multiple of 4 bytes
 *
 * The savefile deosn't contain the version number of that game that saved it;
 * versioning is left at the individual block level.  The current code
 * keeps a list of savefile blocks to save in savers[] below, along with
 * their current versions.
 *
 * For each block type and version, there is a loading function to load that
 * type/version combination.  For example, there may be a loader for v1
 * and v2 of the RNG block; these must be different functions.  It has been
 * done this way since it allows easier maintenance; after each release, you
 * need simply remove old loaders and you will not have to disentangle
 * lots of code with "if (version > 3)" and its like everywhere.
 *
 * Savefile loading and saving is done by keeping the current block in
 * memory, which is accessed using the wr_* and rd_* functions.  This is
 * then written out, whole, to disk, with the appropriate header.
 *
 *
 * So, if you want to make a savefile compat-breaking change, then there are
 * a few things you should do:
 *
 * - increment the version in 'savers' below
 * - add a loading function that accepts the new version (in addition to
 *   the previous loading function) to 'loaders'
 * - and watch the magic happen.
 *
 *
 * TODO:
 * - wr_ and rd_ should be passed a buffer to work with, rather than using
 *   the rd_ and wr_ functions with a universal buffer
 * - 
 */


/** Magic bits at beginning of savefile */
static const byte savefile_magic[4] = { 83, 97, 118, 101 };
static const byte savefile_name[4] = "VNLA";

/* Some useful types */
typedef int (*loader_t)(void);

struct blockheader {
	char name[16];
	u32b version;
	u32b size;
};

struct blockinfo {
	char name[16];
	loader_t loader;
	u32b version;
};

/** Savefile saving functions */
static const struct {
	char name[16];
	void (*save)(void);
	u32b version;	
} savers[] = {
	{ "description", wr_description, 1 },
	{ "rng", wr_randomizer, 1 },
	{ "options", wr_options, 2 },
	{ "messages", wr_messages, 1 },
	{ "monster memory", wr_monster_memory, 3 },
	{ "object memory", wr_object_memory, 2 },
	{ "quests", wr_quests, 1 },
	{ "artifacts", wr_artifacts, 2 },
	{ "player", wr_player, 3 },
	{ "squelch", wr_squelch, 1 },
	{ "misc", wr_misc, 2 },
	{ "player hp", wr_player_hp, 1 },
	{ "player spells", wr_player_spells, 1 },
	{ "inventory", wr_inventory, 6 },
	{ "stores", wr_stores, 6 },
	{ "dungeon", wr_dungeon, 1 },
	{ "objects", wr_objects, 6 },
	{ "monsters", wr_monsters, 7 },
	{ "ghost", wr_ghost, 1 },
	{ "history", wr_history, 1 },
};

/** Savefile loading functions */
static const struct blockinfo loaders[] = {
	{ "description", rd_null, 1 },
	{ "ghost", rd_null, 1 },
	{ "randarts", rd_null, 3 },
	{ "rng", rd_randomizer, 1 },
	{ "options", rd_options_2, 2 },
	{ "messages", rd_messages, 1 },
	{ "monster memory", rd_monster_memory_2, 2 },
	{ "monster memory", rd_monster_memory_3, 3 },
	{ "object memory", rd_object_memory_1, 1 },
	{ "object memory", rd_object_memory_2, 2 },
	{ "quests", rd_quests, 1 },
	{ "artifacts", rd_artifacts, 2 },
	{ "player", rd_player_2, 2 },
	{ "player", rd_player_3, 3 },
	{ "squelch", rd_squelch, 1 },
	{ "misc", rd_misc, 1 },
	{ "misc", rd_misc_2, 2},
	{ "player hp", rd_player_hp, 1 },
	{ "player spells", rd_player_spells, 1 },
	{ "inventory", rd_inventory_1, 1 },
	{ "inventory", rd_inventory_2, 2 },
	{ "inventory", rd_inventory_3, 3 },
	{ "inventory", rd_inventory_4, 4 },	
	{ "inventory", rd_inventory_5, 5 },	
	{ "inventory", rd_inventory_6, 6 },	
	{ "stores", rd_stores_1, 1 },
	{ "stores", rd_stores_2, 2 },
	{ "stores", rd_stores_3, 3 },
	{ "stores", rd_stores_4, 4 },	
	{ "stores", rd_stores_5, 5 },	
	{ "stores", rd_stores_6, 6 },	
	{ "dungeon", rd_dungeon, 1 },
	{ "objects", rd_objects_1, 1 },
	{ "objects", rd_objects_2, 2 },
	{ "objects", rd_objects_3, 3 },
	{ "objects", rd_objects_4, 4 },
	{ "objects", rd_objects_5, 5 },	
	{ "objects", rd_objects_6, 6 },	
	{ "monsters", rd_monsters_6, 6 },
	{ "monsters", rd_monsters_7, 7 },
	{ "history", rd_history, 1 },
};


/* Buffer bits */
static byte *buffer;
static u32b buffer_size;
static u32b buffer_pos;
static u32b buffer_check;

#define BUFFER_INITIAL_SIZE		1024
#define BUFFER_BLOCK_INCREMENT	1024

#define SAVEFILE_HEAD_SIZE		28


/** Utility **/


/*
 * Hack -- Show information on the screen, one line at a time.
 *
 * Avoid the top two lines, to avoid interference with "note()".
 */
void note(const char *message)
{
	static int y = 2;

	/* Draw the message */
	prt(message, y, 0);
	pause_line(Term);

	/* Advance one line (wrap if needed) */
	if (++y >= 24) y = 2;

	/* Flush it */
	Term_fresh();
}




/** Base put/get **/

static void sf_put(byte v)
{
	assert(buffer != NULL);
	assert(buffer_size > 0);

	if (buffer_size == buffer_pos)
	{
		buffer_size += BUFFER_BLOCK_INCREMENT;
		buffer = mem_realloc(buffer, buffer_size);
	}

	assert(buffer_pos < buffer_size);

	buffer[buffer_pos++] = v;
	buffer_check += v;
}

static byte sf_get(void)
{
	assert(buffer != NULL);
	assert(buffer_size > 0);
	assert(buffer_pos < buffer_size);

	buffer_check += buffer[buffer_pos];

	return buffer[buffer_pos++];
}


/* accessor */

void wr_byte(byte v)
{
	sf_put(v);
}

void wr_u16b(u16b v)
{
	sf_put((byte)(v & 0xFF));
	sf_put((byte)((v >> 8) & 0xFF));
}

void wr_s16b(s16b v)
{
	wr_u16b((u16b)v);
}

void wr_u32b(u32b v)
{
	sf_put((byte)(v & 0xFF));
	sf_put((byte)((v >> 8) & 0xFF));
	sf_put((byte)((v >> 16) & 0xFF));
	sf_put((byte)((v >> 24) & 0xFF));
}

void wr_s32b(s32b v)
{
	wr_u32b((u32b)v);
}

void wr_string(const char *str)
{
	while (*str)
	{
		wr_byte(*str);
		str++;
	}
	wr_byte(*str);
}


void rd_byte(byte *ip)
{
	*ip = sf_get();
}

void rd_u16b(u16b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u16b)(sf_get()) << 8);
}

void rd_s16b(s16b *ip)
{
	rd_u16b((u16b*)ip);
}

void rd_u32b(u32b *ip)
{
	(*ip) = sf_get();
	(*ip) |= ((u32b)(sf_get()) << 8);
	(*ip) |= ((u32b)(sf_get()) << 16);
	(*ip) |= ((u32b)(sf_get()) << 24);
}

void rd_s32b(s32b *ip)
{
	rd_u32b((u32b*)ip);
}

void rd_string(char *str, int max)
{
	byte tmp8u;
	int i = 0;

	do
	{
		rd_byte(&tmp8u);

		if (i < max) str[i] = tmp8u;
		if (!tmp8u) break;
	} while (++i);

	str[max - 1] = '\0';
}

void strip_bytes(int n)
{
	byte tmp8u;
	while (n--) rd_byte(&tmp8u);
}

void pad_bytes(int n)
{
	while (n--) wr_byte(0);
}


/*** Savefile saving functions ***/

static bool try_save(ang_file *file)
{
	byte savefile_head[SAVEFILE_HEAD_SIZE];
	size_t i, pos;

	/* Start off the buffer */
	buffer = mem_alloc(BUFFER_INITIAL_SIZE);
	buffer_size = BUFFER_INITIAL_SIZE;

	for (i = 0; i < N_ELEMENTS(savers); i++)
	{
		buffer_pos = 0;
		buffer_check = 0;

		savers[i].save();

		/* 16-byte block name */
		pos = my_strcpy((char *)savefile_head,
				savers[i].name,
				sizeof savefile_head);
		while (pos < 16)
			savefile_head[pos++] = 0;

#define SAVE_U32B(v)	\
		savefile_head[pos++] = (v & 0xFF); \
		savefile_head[pos++] = ((v >> 8) & 0xFF); \
		savefile_head[pos++] = ((v >> 16) & 0xFF); \
		savefile_head[pos++] = ((v >> 24) & 0xFF);

		SAVE_U32B(savers[i].version);
		SAVE_U32B(buffer_pos);
		SAVE_U32B(buffer_check);

		assert(pos == SAVEFILE_HEAD_SIZE);

		file_write(file, (char *)savefile_head, SAVEFILE_HEAD_SIZE);
		file_write(file, (char *)buffer, buffer_pos);

		/* pad to 4 byte multiples */
		if (buffer_pos % 4)
			file_write(file, "xxx", 4 - (buffer_pos % 4));
	}

	mem_free(buffer);

	return TRUE;
}

/*
 * Set the savefile name.
 */
void savefile_set_name(const char *fname)
{
	char path[128];

#if defined(SETGID)
	/* Rename the savefile, using the player_uid and base_name */
	strnfmt(path, sizeof(path), "%d.%s", player_uid, fname);
#else
	/* Rename the savefile, using the base name */
	strnfmt(path, sizeof(path), "%s", fname);
#endif

	/* Save the path */
	path_build(savefile, sizeof(savefile), ANGBAND_DIR_SAVE, path);
}




/*
 * Attempt to save the player in a savefile
 */
bool savefile_save(const char *path)
{
	ang_file *file;
	int count = 0;
	char new_savefile[1024];
	char old_savefile[1024];

	/* New savefile */
	strnfmt(old_savefile, sizeof(old_savefile), "%s%u.old", path,Rand_simple(1000000));
	while (file_exists(old_savefile) && (count++ < 100)) {
		strnfmt(old_savefile, sizeof(old_savefile), "%s%u%u.old", path,Rand_simple(1000000),count);
	}
	count = 0;

	/* Make sure that the savefile doesn't already exist */
	/*safe_setuid_grab();
	file_delete(new_savefile);
	file_delete(old_savefile);
	safe_setuid_drop();*/

	/* Open the savefile */
	safe_setuid_grab();
	strnfmt(new_savefile, sizeof(new_savefile), "%s%u.new", path,Rand_simple(1000000));
	while (file_exists(new_savefile) && (count++ < 100)) {
		strnfmt(new_savefile, sizeof(new_savefile), "%s%u%u.new", path,Rand_simple(1000000),count);
	}
	file = file_open(new_savefile, MODE_WRITE, FTYPE_SAVE);
	safe_setuid_drop();

	if (file)
	{
		file_write(file, (char *) &savefile_magic, 4);
		file_write(file, (char *) &savefile_name, 4);

		character_saved = try_save(file);
		file_close(file);
	}

	if (character_saved)
	{
		bool err = FALSE;

		safe_setuid_grab();

		if (file_exists(savefile) && !file_move(savefile, old_savefile))
			err = TRUE;

		if (!err)
		{
			if (!file_move(new_savefile, savefile))
				err = TRUE;

			if (err)
				file_move(old_savefile, savefile);
			else
				file_delete(old_savefile);
		} 

		safe_setuid_drop();

		return err ? FALSE : TRUE;
	}

	/* Delete temp file if the save failed */
	if (file)
	{
		/* file is no longer valid, but it still points to a non zero
		 * value if the file was created above */
		safe_setuid_grab();
		file_delete(new_savefile);
		safe_setuid_drop();
	}
	return FALSE;
}



/*** Savefile loading functions ***/

/* Check the savefile header file clearly inicates that it's a savefile */
static bool check_header(ang_file *f) {
	byte head[8];

	if (file_read(f, (char *) &head, 8) == 8 &&
			memcmp(&head[0], savefile_magic, 4) == 0 &&
			memcmp(&head[4], savefile_name, 4) == 0)
		return TRUE;

	return FALSE;
}

/* Get the next block header from the savefile */
static errr next_blockheader(ang_file *f, struct blockheader *b) {
	byte savefile_head[SAVEFILE_HEAD_SIZE];
	size_t len;

	len = file_read(f, (char *)savefile_head, SAVEFILE_HEAD_SIZE);
	if (len == 0) /* no more blocks */
		return 1;

	if (len != SAVEFILE_HEAD_SIZE || savefile_head[15] != 0) {
		return -1;
	}

#define RECONSTRUCT_U32B(from) \
	((u32b) savefile_head[from]) | \
	((u32b) savefile_head[from+1] << 8) | \
	((u32b) savefile_head[from+2] << 16) | \
	((u32b) savefile_head[from+3] << 24);

	my_strcpy(b->name, (char *)&savefile_head, sizeof b->name);
	b->version = RECONSTRUCT_U32B(16);
	b->size = RECONSTRUCT_U32B(20);

	/* pad to 4 bytes */
	if (b->size % 4)
		b->size += 4 - (b->size % 4);

	return 0;
}

/* Find the right loader for this block, return it */
static loader_t find_loader(struct blockheader *b, const struct blockinfo *loaders) {
	size_t i = 0;

	/* Find the right loader */
	for (i = 0; loaders[i].name[0]; i++) {
		if (!streq(b->name, loaders[i].name)) continue;
		if (b->version != loaders[i].version) continue;

		return loaders[i].loader;
	} 

	return NULL;
}

/* Load a given block with the given loader */
static bool load_block(ang_file *f, struct blockheader *b, loader_t loader) {
	/* Allocate space for the buffer */
	buffer = mem_alloc(b->size);
	buffer_pos = 0;
	buffer_check = 0;

	buffer_size = file_read(f, (char *) buffer, b->size);
	if (buffer_size != b->size ||
			loader() != 0) {
		mem_free(buffer);
		return FALSE;
	}

	mem_free(buffer);
	return TRUE;
}

/* Skip a block */
static void skip_block(ang_file *f, struct blockheader *b) {
	file_skip(f, b->size);
}

/* Try to load a savefile */
static bool try_load(ang_file *f, const struct blockinfo *loaders) {
	struct blockheader b;
	errr err;

	if (!check_header(f)) {
		note("Savefile is corrupted -- incorrect file header.");
		return FALSE;
	}

	/* Get the next block header */
	while ((err = next_blockheader(f, &b)) == 0) {
		loader_t loader = find_loader(&b, loaders);
		if (!loader) {
			note("Savefile block can't be read.");
			note("Maybe try and load the savefile in an earlier version of Angband.");
			return FALSE;
		}

		if (!load_block(f, &b, loader)) {
			note(format("Savefile corrupted - Couldn't load block %s", b.name));
			return FALSE;
		}
	}

	if (err == -1) {
		note("Savefile is corrupted -- block header mangled.");
		return FALSE;
	}

	/* XXX Reset cause of death */
	if (p_ptr->chp >= 0)
		my_strcpy(p_ptr->died_from, "(alive and well)", sizeof(p_ptr->died_from));

	return TRUE;
}

/* XXX this isn't nice but it'll have to do */
static char savefile_desc[120];

static int get_desc(void) {
	rd_string(savefile_desc, sizeof savefile_desc);
	return 0;
}

/**
 * Try to get the 'description' block from a savefile.  Fail gracefully.
 */
const char *savefile_get_description(const char *path) {
	errr err;
	struct blockheader b;

	ang_file *f = file_open(path, MODE_READ, FTYPE_TEXT);
	if (!f) return NULL;

	/* Blank the description */
	savefile_desc[0] = 0;

	if (!check_header(f)) {
		my_strcpy(savefile_desc, "Invalid savefile", sizeof savefile_desc);
	} else {
		while ((err = next_blockheader(f, &b)) == 0) {
			if (!streq(b.name, "description")) {
				skip_block(f, &b);
				continue;
			}
			load_block(f, &b, get_desc);
			break;
		}
	}

	file_close(f);
	return savefile_desc;
}


/**
 * Load a savefile.
 */
bool savefile_load(const char *path)
{
	bool ok;
	ang_file *f = file_open(path, MODE_READ, FTYPE_TEXT);
	if (!f) {
		note("Couldn't open savefile.");
		return FALSE;
	}

	ok = try_load(f, loaders);
	file_close(f);

	return ok;
}
