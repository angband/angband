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
#include "angband.h"
#include "savefile.h"


/**
 * Big list of all savefile block types.
 */
savefile_block_t savefile_blocks[N_SAVEFILE_BLOCKS] =
{
	{ "rng", rd_randomizer, wr_randomizer, 1, 1 },
	{ "options", rd_options, wr_options, 1, 1 },
	{ "messages", rd_messages, wr_messages, 1, 1 },
	{ "monster memory", rd_monster_memory, wr_monster_memory, 1, 1 },
	{ "object memory", rd_object_memory, wr_object_memory, 1, 1 },
	{ "quests", rd_quests, wr_quests, 1, 1 },
	{ "artifacts", rd_artifacts, wr_artifacts, 1, 1 },
	{ "player", rd_player, wr_player, 1, 1 },
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






static ang_file *fff;		/* Current save "file" */

static byte	xor_byte;	/* Simple encryption */

static u32b	v_stamp = 0L;	/* A simple "checksum" on the actual values */
static u32b	x_stamp = 0L;	/* A simple "checksum" on the encoded bytes */


/* Buffer bits */
static byte *buffer;
static u32b buffer_size;
static u32b buffer_pos;

#define BUFFER_INITIAL_SIZE		1024
#define BUFFER_BLOCK_INCREMENT	1024


void wr_block(void)
{
	file_write(fff, (char *)buffer, buffer_pos);
	buffer_pos = 0;
}


/*
 * These functions place information into a savefile a byte at a time
 */

static void sf_put(byte v)
{
	if (!buffer)
	{
		buffer = mem_alloc(BUFFER_INITIAL_SIZE);
		buffer_size = BUFFER_INITIAL_SIZE;
		buffer_pos = 0;
	}
	
	if (buffer_size == buffer_pos)
	{
		buffer = mem_realloc(buffer, buffer_size + BUFFER_BLOCK_INCREMENT);
		buffer_size += BUFFER_BLOCK_INCREMENT;
		printf("new size = %d\n", buffer_size);
	}
	
	/* Encode the value, write a character */
	xor_byte ^= v;
	
	assert(buffer_pos < buffer_size);
	buffer[buffer_pos++] = xor_byte;
	
	/* Maintain the checksum info */
	v_stamp += v;
	x_stamp += xor_byte;
}

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








/*
 * Actually write a save-file
 */
static void wr_savefile_new(void)
{
	size_t i;
	
	/* Dump the file header */
	xor_byte = 0;
	wr_byte(VERSION_MAJOR);
	xor_byte = 0;
	wr_byte(VERSION_MINOR);
	xor_byte = 0;
	wr_byte(VERSION_PATCH);
	xor_byte = 0;
	wr_byte(VERSION_EXTRA);
	
	/* Reset the checksum */
	v_stamp = 0L;
	x_stamp = 0L;
	
	wr_u32b(0L);
	wr_u32b(0L);
	wr_u32b(0L);
	wr_u32b(0L);
	wr_u32b(0L);
	
	/* Hack -- for now */
	wr_block();
	
	for (i = 0; i < N_SAVEFILE_BLOCKS; i++)
	{
		printf("in block %s\n", savefile_blocks[i].name);
		savefile_blocks[i].saver();
		wr_block();
	}
	
	
	/* Write the checksums */
	wr_u32b(v_stamp);
	wr_u32b(x_stamp);
	wr_block();				/* hack for now */
}


/*
 * Medium level player saver
 */
static bool save_player_aux(cptr name)
{
	bool ok = TRUE;
	
	/* No file yet */
	fff = NULL;
	
	/* Open the savefile */
	safe_setuid_grab();
	fff = file_open(name, MODE_WRITE, FTYPE_SAVE);
	safe_setuid_drop();
	
	/* Successful open */
	if (fff) wr_savefile_new();
	else ok = FALSE;
	
	/* Attempt to close it */
	if (ok && !file_close(fff)) ok = FALSE;
	
	
	if (ok)
		character_saved = TRUE;
	
	return ok;
}

#include <errno.h>

/*
 * Attempt to save the player in a savefile
 */
bool old_save(void)
{
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
	
	/* Attempt to save the player */
	if (save_player_aux(new_savefile))
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


