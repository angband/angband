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


/** Magic bits at beginning of savefile */
static const byte savefile_magic[4] = { 83, 97, 118, 101 };
static const byte savefile_name[4] = SAVEFILE_NAME;


/**
 * Big list of all savefile block types.
 */
static const struct
{
	char name[16];
	int (*loader)(u32b version);
	void (*saver)(void);
	u32b cur_ver;
	u32b oldest_ver;
} savefile_blocks[] =
{
	{ "rng", rd_randomizer, wr_randomizer, 1, 1 },
	{ "options", rd_options, wr_options, 1, 1 },
	{ "messages", rd_messages, wr_messages, 1, 1 },
	{ "monster memory", rd_monster_memory, wr_monster_memory, 1, 1 },
	{ "object memory", rd_object_memory, wr_object_memory, 1, 1 },
	{ "quests", rd_quests, wr_quests, 1, 1 },
	{ "artifacts", rd_artifacts, wr_artifacts, 2, 1 },
	{ "player", rd_player, wr_player, 2, 1 },
	{ "squelch", rd_squelch, wr_squelch, 1, 1 },
	{ "misc", rd_misc, wr_misc, 1, 1 },
	{ "player hp", rd_player_hp, wr_player_hp, 1, 1 },
	{ "player spells", rd_player_spells, wr_player_spells, 1, 1 },
	{ "randarts", rd_randarts, wr_randarts, 1, 1 },
	{ "inventory", rd_inventory, wr_inventory, 1, 1 },
	{ "stores", rd_stores, wr_stores, 1, 1 },
	{ "dungeon", rd_dungeon, wr_dungeon, 1, 1 },
	{ "objects", rd_objects, wr_objects, 1, 1 },
	{ "monsters", rd_monsters, wr_monsters, 1, 1 },
	{ "ghost", rd_ghost, wr_ghost, 1, 1 },
	{ "history", rd_history, wr_history, 1, 1 },
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
 * Avoid the top two lines, to avoid interference with "msg_print()".
 */
void note(cptr msg)
{
	static int y = 2;

	/* Draw the message */
	prt(msg, y, 0);

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

void wr_string(cptr str)
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




/*** ****/


static bool try_save(ang_file *file)
{
	byte savefile_head[SAVEFILE_HEAD_SIZE];
	size_t i, pos;

	/* Start off the buffer */
	buffer = mem_alloc(BUFFER_INITIAL_SIZE);
	buffer_size = BUFFER_INITIAL_SIZE;

	for (i = 0; i < N_ELEMENTS(savefile_blocks); i++)
	{
		buffer_pos = 0;
		buffer_check = 0;

		savefile_blocks[i].saver();

		/* 16-byte block name */
		pos = my_strcpy((char *)savefile_head,
				savefile_blocks[i].name,
				sizeof savefile_head);
		while (pos < 16)
			savefile_head[pos++] = 0;

		/* 4-byte block version */
		savefile_head[pos++] = (savefile_blocks[i].cur_ver & 0xFF);
		savefile_head[pos++] = ((savefile_blocks[i].cur_ver >> 8) & 0xFF);
		savefile_head[pos++] = ((savefile_blocks[i].cur_ver >> 16) & 0xFF);
		savefile_head[pos++] = ((savefile_blocks[i].cur_ver >> 24) & 0xFF);

		/* 4-byte block size */
		savefile_head[pos++] = (buffer_pos & 0xFF);
		savefile_head[pos++] = ((buffer_pos >> 8) & 0xFF);
		savefile_head[pos++] = ((buffer_pos >> 16) & 0xFF);
		savefile_head[pos++] = ((buffer_pos >> 24) & 0xFF);

		/* 4-byte block checksum */
		savefile_head[pos++] = (buffer_check & 0xFF);
		savefile_head[pos++] = ((buffer_check >> 8) & 0xFF);
		savefile_head[pos++] = ((buffer_check >> 16) & 0xFF);
		savefile_head[pos++] = ((buffer_check >> 24) & 0xFF);

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


static bool try_load(ang_file *file)
{
	byte savefile_head[SAVEFILE_HEAD_SIZE];
	u32b block_version, block_size, block_checksum;

	while (TRUE)
	{
		size_t i;
		size_t size;

		/* Load in the next header */
		size = file_read(file,
				(char *)savefile_head, SAVEFILE_HEAD_SIZE);

		/* If nothing was read, that's the end of the file */
		if (size == 0) break;

		assert(size == SAVEFILE_HEAD_SIZE);

		/* 16-byte block name, null-terminated */
		assert(savefile_head[15] == '\0');

		/* Determine the block ID */
		for (i = 0; i < N_ELEMENTS(savefile_blocks); i++)
		{
			if (strncmp((char *) savefile_head,
					savefile_blocks[i].name,
					sizeof savefile_blocks[i].name) == 0)
				break;
		}
		assert(i < N_ELEMENTS(savefile_blocks));


		/* 4-byte block version */
		block_version = ((u32b) savefile_head[16]) |
				((u32b) savefile_head[17] << 8) |
				((u32b) savefile_head[18] << 16) |
				((u32b) savefile_head[19] << 24);

		/* 4-byte block size */
		block_size = ((u32b) savefile_head[20]) |
				((u32b) savefile_head[21] << 8) |
				((u32b) savefile_head[22] << 16) |
				((u32b) savefile_head[23] << 24);

		/* 4-byte block checksum */
		block_checksum = ((u32b) savefile_head[24]) |
				((u32b) savefile_head[25] << 8) |
				((u32b) savefile_head[26] << 16) |
				((u32b) savefile_head[27] << 24);

		/* pad to 4 bytes */
		if (block_size % 4)
			block_size += 4 - (block_size % 4);

		/* Read stuff in */
		buffer = mem_alloc(block_size);
		buffer_size = block_size;
		buffer_pos = 0;
		buffer_check = 0;

		size = file_read(file, (char *) buffer, block_size);
		assert(size == block_size);

		/* Try loading */
		if (savefile_blocks[i].loader(block_version))
			return -1;

/*		assert(buffer_check == block_checksum); */
		mem_free(buffer);
	}

	return 0;
}




/*
 * Attempt to save the player in a savefile
 */
bool old_save(void)
{
	ang_file *file;

	char new_savefile[1024];
	char old_savefile[1024];

	/* New savefile */
	strnfmt(new_savefile, sizeof(new_savefile), "%s.new", savefile);
	strnfmt(old_savefile, sizeof(old_savefile), "%s.old", savefile);

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



/*
 * Attempt to Load a "savefile"
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool old_load(void)
{
	byte sf_major = 0;
	byte sf_minor = 0;
	byte sf_patch = 0;
	byte sf_extra = 0;
	ang_file *fh;
	byte head[8];

	cptr what = "generic";
	errr err = 0;

	/* Clear screen */
	Term_clear();


	fh = file_open(savefile, MODE_READ, -1);
	if (!fh)
	{
		err = -1;
		what = "Cannot open savefile";
	}

	/* Process file */
	if (!err)
	{

		/* Read version / ID marker */
		if (file_read(fh, (char *) &head, 8) != 8)
		{
			file_close(fh);

			what = "Cannot read savefile";
			err = -1;
		}

		sf_major = head[0];
		sf_minor = head[1];
		sf_patch = head[2];
		sf_extra = head[3];
	}

	/* Process file */
	if (!err)
	{
		if (sf_major == savefile_magic[0] && sf_minor == savefile_magic[1] &&
				sf_patch == savefile_magic[2] && sf_extra == savefile_magic[3])
		{
			if (strncmp((char *) &head[4], SAVEFILE_NAME, 4) != 0)
			{
				err = -1;
				what = "Savefile from different variant";
			}
			else
			{
				err = try_load(fh);
				file_close(fh);
				if (!err) what = "cannot read savefile";
			}
		}
		else if (sf_major == 3 && sf_minor == 0 &&
				sf_patch == 14 && sf_extra == 0)
		{
			file_close(fh);
			err = rd_savefile_old();
			if (err) what = "Cannot parse savefile";
		}
		else
		{
			what = "Unreadable savefile (too old?)";
			err = -1;
		}
	}

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}


	/* Okay */
	if (!err)
	{
		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			my_strcpy(p_ptr->died_from, "(alive and well)", sizeof(p_ptr->died_from));
		}

		/* Success */
		return (TRUE);
	}

	/* Message */
	msg_format("Error (%s) reading %d.%d.%d savefile.",
	           what, sf_major, sf_minor, sf_patch);
	message_flush();

	/* Oops */
	return (FALSE);
}
