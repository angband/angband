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

/** Savefile saving functions */
static const struct {
	char name[16];
	void (*save)(void);
	u32b version;	
} savers[] = {
	{ "rng", wr_randomizer, 1 },
	{ "options", wr_options, 2 },
	{ "messages", wr_messages, 1 },
	{ "monster memory", wr_monster_memory, 2 },
	{ "object memory", wr_object_memory, 1 },
	{ "quests", wr_quests, 1 },
	{ "artifacts", wr_artifacts, 2 },
	{ "player", wr_player, 2 },
	{ "squelch", wr_squelch, 1 },
	{ "misc", wr_misc, 1 },
	{ "player hp", wr_player_hp, 1 },
	{ "player spells", wr_player_spells, 1 },
	{ "randarts", wr_randarts, 2 },
	{ "inventory", wr_inventory, 3 },
	{ "stores", wr_stores, 3 },
	{ "dungeon", wr_dungeon, 1 },
	{ "objects", wr_objects, 3 },
	{ "monsters", wr_monsters, 5 },
	{ "ghost", wr_ghost, 1 },
	{ "history", wr_history, 1 },
};

/** Savefile loading functions */
static const struct {
	char name[16];
	int (*load)(void);
	u32b version;
} loaders[] = {
	{ "rng", rd_randomizer, 1 },
	{ "options", rd_options_1, 1 },
	{ "options", rd_options_2, 2 },
	{ "messages", rd_messages, 1 },
	{ "monster memory", rd_monster_memory_1, 1 },
	{ "monster memory", rd_monster_memory_2, 2 },
	{ "object memory", rd_object_memory, 1 },
	{ "quests", rd_quests, 1 },
	{ "artifacts", rd_artifacts, 2 },
	{ "player", rd_player, 2 },
	{ "squelch", rd_squelch, 1 },
	{ "misc", rd_misc, 1 },
	{ "player hp", rd_player_hp, 1 },
	{ "player spells", rd_player_spells, 1 },
	{ "randarts", rd_randarts_1, 1 },
	{ "randarts", rd_randarts_2, 2 },
	{ "inventory", rd_inventory_1, 1 },
	{ "inventory", rd_inventory_2, 2 },
	{ "inventory", rd_inventory_3, 3 },
	{ "stores", rd_stores_1, 1 },
	{ "stores", rd_stores_2, 2 },
	{ "stores", rd_stores_3, 3 },
	{ "dungeon", rd_dungeon, 1 },
	{ "objects", rd_objects_1, 1 },
	{ "objects", rd_objects_2, 2 },
	{ "objects", rd_objects_3, 3 },
	{ "monsters", rd_monsters_1, 1 },
	{ "monsters", rd_monsters_2, 2 },
	{ "monsters", rd_monsters_3, 3 },
	{ "monsters", rd_monsters_4, 4 },
	{ "monsters", rd_monsters_5, 5 },
	{ "ghost", rd_ghost, 1 },
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
 * Attempt to save the player in a savefile
 */
bool savefile_save(const char *path)
{
	ang_file *file;

	char new_savefile[1024];
	char old_savefile[1024];

	/* New savefile */
	strnfmt(new_savefile, sizeof(new_savefile), "%s.new", path);
	strnfmt(old_savefile, sizeof(old_savefile), "%s.old", path);

	/* Make sure that the savefile doesn't already exist */
	safe_setuid_grab();
	file_delete(new_savefile);
	file_delete(old_savefile);
	safe_setuid_drop();

	/* Open the savefile */
	safe_setuid_grab();
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

	/* Delete temp file */
	safe_setuid_grab();
	file_delete(new_savefile);
	safe_setuid_drop();

	return FALSE;
}



/*** Savefiel loading functions ***/

static bool try_load(ang_file *f)
{
	byte savefile_head[SAVEFILE_HEAD_SIZE];
	u32b block_version, block_size;
	char *block_name;

	while (TRUE)
	{
		size_t i;
		int (*loader)(void) = NULL;

		/* Load in the next header */
		size_t size = file_read(f, (char *)savefile_head, SAVEFILE_HEAD_SIZE);
		if (!size)
			break;

		if (size != SAVEFILE_HEAD_SIZE || savefile_head[15] != 0) {
			note("Savefile is corrupted -- block header mangled.");
			return FALSE;
		}

#define RECONSTRUCT_U32B(from) \
		((u32b) savefile_head[from]) | \
		((u32b) savefile_head[from+1] << 8) | \
		((u32b) savefile_head[from+2] << 16) | \
		((u32b) savefile_head[from+3] << 24);

		block_name = (char *) savefile_head;
		block_version = RECONSTRUCT_U32B(16);
		block_size = RECONSTRUCT_U32B(20);

		/* pad to 4 bytes */
		if (block_size % 4)
			block_size += 4 - (block_size % 4);

		/* Find the right loader */
		for (i = 0; i < N_ELEMENTS(loaders); i++) {
			if (streq(block_name, loaders[i].name) &&
					block_version == loaders[i].version) {
				loader = loaders[i].load;
			}
		}

		if (!loader) {
			/* No loader found */
			note("Savefile too old.  Try importing it into an older Angband first.");
			return FALSE;
		}

		/* Allocate space for the buffer */
		buffer = mem_alloc(block_size);
		buffer_pos = 0;
		buffer_check = 0;

		buffer_size = file_read(f, (char *) buffer, block_size);
		if (buffer_size != block_size) {
			note("Savefile is corrupted -- not enough bytes.");
			mem_free(buffer);
			return FALSE;
		}

		/* Try loading */
		if (loader() != 0) {
			note("Savefile is corrupted.");
			mem_free(buffer);
			return FALSE;
		}

		mem_free(buffer);
	}

	/* Still alive */
	if (p_ptr->chp >= 0)
	{
		/* Reset cause of death */
		my_strcpy(p_ptr->died_from, "(alive and well)", sizeof(p_ptr->died_from));
	}

	return TRUE;
}


/**
 * Load a savefile.
 */
bool savefile_load(const char *path)
{
	byte head[8];
	bool ok = TRUE;

	ang_file *f = file_open(path, MODE_READ, -1);
	if (f) {
		if (file_read(f, (char *) &head, 8) == 8 &&
				memcmp(&head[0], savefile_magic, 4) == 0 &&
				memcmp(&head[4], savefile_name, 4) == 0) {
			if (!try_load(f)) {
				ok = FALSE;
				note("Failed loading savefile.");
			}
		} else {
			ok = FALSE;
			note("Savefile is corrupted -- incorrect file header.");
		}

		file_close(f);
	} else {
		ok = FALSE;
		note("Couldn't open savefile.");
	}

	return ok;
}
