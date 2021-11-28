/**
 * \file savefile.h
 * \brief Savefile loading and saving main routines
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
#ifndef INCLUDED_SAVEFILE_H
#define INCLUDED_SAVEFILE_H

#define FINISHED_CODE 255
#define ITEM_VERSION	5
#define EGO_ART_KNOWN 0xffffffff

/**
 * ------------------------------------------------------------------------
 * Savefile API
 * ------------------------------------------------------------------------ */

/**
 * Global "we've just saved" variable
 */
extern bool character_saved;

/**
 * Save to the given location.  Returns true on success, false otherwise.
 */
bool savefile_save(const char *path);

/**
 * Load the savefile given.  Returns true on succcess, false otherwise.
 */
bool savefile_load(const char *path, bool cheat_death);

/**
 * Try to get a description for this savefile.
 */
const char *savefile_get_description(const char *path);

/**
 * Fill the given buffer with the panic save equivalent for a savefile.
 */
void savefile_get_panic_name(char *buffer, size_t len, const char *path);


/**
 * ------------------------------------------------------------------------
 * Detailed saving and loading functions
 * ------------------------------------------------------------------------ */


/* Utility */
void note(const char *msg);

/* Writing bits */
void wr_byte(uint8_t v);
void wr_u16b(uint16_t v);
void wr_s16b(int16_t v);
void wr_u32b(uint32_t v);
void wr_s32b(int32_t v);
void wr_string(const char *str);
void pad_bytes(int n);

/* Reading bits */
void rd_byte(uint8_t *ip);
void rd_u16b(uint16_t *ip);
void rd_s16b(int16_t *ip);
void rd_u32b(uint32_t *ip);
void rd_s32b(int32_t *ip);
void rd_string(char *str, int max);
void strip_bytes(int n);



/* load.c */
int rd_randomizer(void);
int rd_options(void);
int rd_messages(void);
int rd_monster_memory(void);
int rd_object_memory(void);
int rd_quests(void);
int rd_artifacts(void);
int rd_player(void);
int rd_ignore(void);
int rd_misc(void);
int rd_player_hp(void);
int rd_player_spells(void);
int rd_gear(void);
int rd_stores(void);
int rd_dungeon(void);
int rd_chunks(void);
int rd_objects(void);
int rd_monsters(void);
int rd_monster_groups(void);
int rd_history(void);
int rd_traps(void);
int rd_null(void);

/* save.c */
void wr_description(void);
void wr_randomizer(void);
void wr_options(void);
void wr_messages(void);
void wr_monster_memory(void);
void wr_object_memory(void);
void wr_quests(void);
void wr_artifacts(void);
void wr_player(void);
void wr_ignore(void);
void wr_misc(void);
void wr_player_hp(void);
void wr_player_spells(void);
void wr_randarts(void);
void wr_gear(void);
void wr_stores(void);
void wr_dungeon(void);
void wr_chunks(void);
void wr_objects(void);
void wr_monsters(void);
void wr_monster_groups(void);
void wr_ghost(void);
void wr_history(void);
void wr_traps(void);


#endif /* INCLUDED_SAVEFILE_H */
