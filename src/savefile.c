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
savefile_block_t savefile_blocks[] =
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
	{ "player_hp", rd_player_hp, wr_player_hp, 1, 1 },
	{ "player_spells", rd_player_spells, wr_player_spells, 1, 1 },
	{ "randarts", rd_randarts, wr_randarts, 1, 1 },
	{ "inventory", rd_inventory, wr_inventory, 1, 1 },
	{ "stores", rd_stores, wr_stores, 1, 1 },
	{ "dungeon", rd_dungeon, wr_dungeon, 1, 1 },
	{ "objects", rd_objects, wr_objects, 1, 1 },
	{ "monsters", rd_monsters, wr_monsters, 1, 1 },
	{ "ghost", rd_ghost, wr_ghost, 1, 1 },
	{ "history", rd_history, wr_history, 1, 1 },
};

