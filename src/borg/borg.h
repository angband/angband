/**
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_H
#define INCLUDED_BORG_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-trait.h"

extern bool borg_cheat_death;

/*
 * Use a simple internal random number generator
 */
extern uint32_t borg_rand_local; /* Save personal setting */

/*
 * Date of the last change
 */
extern char borg_engine_date[];

/* options from the borg.txt file */
/* IMPORTANT keep these in sync with borg_settings in borg-init.c */
enum {
    BORG_VERBOSE,
    BORG_MUNCHKIN_START,
    BORG_MUNCHKIN_LEVEL,
    BORG_MUNCHKIN_DEPTH,
    BORG_WORSHIPS_DAMAGE,
    BORG_WORSHIPS_SPEED,
    BORG_WORSHIPS_HP,
    BORG_WORSHIPS_MANA,
    BORG_WORSHIPS_AC,
    BORG_WORSHIPS_GOLD,
    BORG_PLAYS_RISKY,
    BORG_KILLS_UNIQUES,
    BORG_USES_SWAPS,
    BORG_USES_DYNAMIC_CALCS,
    BORG_STOP_DLEVEL,
    BORG_STOP_CLEVEL,
    BORG_NO_DEEPER,
    BORG_STOP_KING,
    BORG_CHEAT_DEATH,
    BORG_RESPAWN_WINNERS,
    BORG_RESPAWN_CLASS,
    BORG_RESPAWN_RACE,
    BORG_CHEST_FAIL_TOLERANCE,
    BORG_DELAY_FACTOR,
    BORG_MONEY_SCUM_AMOUNT,
    BORG_SELF_SCUM,
    BORG_LUNAL_MODE,
    BORG_SELF_LUNAL,
    BORG_ENCHANT_LIMIT,
    BORG_DUMP_LEVEL,
    BORG_SAVE_DEATH,
    BORG_STOP_ON_BELL,
    BORG_ALLOW_STRANGE_OPTS,
    BORG_AUTOSAVE,
    BORG_MAX_SETTINGS
};
extern int *borg_cfg;

/*
 * Status variables
 */
extern bool borg_active; /* Actually active */
extern bool borg_cancel; /* Being cancelled */
extern bool borg_save; /* do a save next time we get to press a key! */
extern bool borg_graphics; /* rr9's graphics */

extern int16_t old_depth;
extern int16_t borg_respawning;

/*
 * Hack -- Time variables
 */
extern int16_t borg_t; /* Current "time" */
extern int32_t borg_began; /* When this level began */
extern int32_t borg_time_town; /* how long it has been since I was in town */
extern int16_t borg_t_morgoth; /* Last time I saw Morgoth */

/*
 * Number of turns to (manually) step for (zero means forever)
 */
extern uint16_t borg_step;

extern int w_x; /* Current panel offset (X) */
extern int w_y; /* Current panel offset (Y) */

/*
 * KEYMAP_MODE_ROGUE or KEYMAP_MODE_ORIG
 */
extern int key_mode;

/*
 * Special "inkey_hack" hook.
 */
extern struct keypress (*inkey_hack)(int flush_first);

/*
 * Entry point for borg commands.
 */
extern void do_cmd_borg(void);

#endif
#endif
