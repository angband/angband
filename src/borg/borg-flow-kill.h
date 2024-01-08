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

#ifndef INCLUDED_BORG_FLOW_KILL_H
#define INCLUDED_BORG_FLOW_KILL_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "../mon-spell.h"

#include "borg-flow.h"

/*
 * Monster information
 */
typedef struct borg_kill borg_kill;

struct borg_kill {
    unsigned int r_idx; /* Race index */

    bool         known; /* Verified race */
    bool         awake; /* Probably awake */

    bool         confused; /* Probably confused */
    bool         afraid; /* Probably afraid */
    bool         quiver; /* Probably quivering */
    bool         stunned;
    bool         poisoned; /* Probably poisoned */

    bool         seen; /* Assigned motion */
    bool         used; /* Assigned message */

    struct loc   pos; /* Location */

    uint8_t      ox, oy; /* Old location */

    uint8_t      speed; /* Estimated speed */
    uint8_t      moves; /* Estimates moves */
    uint8_t      ranged_attack; /* qty of ranged attacks */
    uint8_t      spell[RSF_MAX]; /* spell flag for monster spells */
    int16_t      power; /* Estimated hit-points */
    int16_t      injury; /* Percent wounded */
    int16_t      other; /* Estimated something */
    int16_t      level; /* Monsters Level */
    uint32_t     spell_flags[RF_MAX]; /* Monster race spell flags preloaded */
    int16_t      when; /* When last seen */
    int16_t      m_idx; /* Game's index */
};

/*
 * The monster list.  This list is used to "track" monsters.
 */
extern int16_t    borg_kills_cnt;
extern int16_t    borg_kills_summoner; /* index of a summoning guy */
extern int16_t    borg_kills_nxt;
extern borg_kill *borg_kills;

/*
 * Hack -- count racial appearances per level
 */
extern int16_t *borg_race_count;

/*
 * Hack -- count racial kills (for uniques)
 */
extern int16_t *borg_race_death;

/*
 * Monsters or Uniques on this level
 */
extern unsigned int borg_morgoth_id;
extern unsigned int borg_sauron_id;
extern unsigned int borg_tarrasque_id;

extern unsigned int unique_on_level;
extern bool         scaryguy_on_level;
extern bool         morgoth_on_level;
extern bool         borg_morgoth_position;
extern bool         breeder_level;

extern uint8_t borg_nasties_num;
extern uint8_t borg_nasties_count[7];
extern char    borg_nasties[7];
extern uint8_t borg_nasties_limit[7];

extern int morgy_panel_y;
extern int morgy_panel_x;

/* am I fighting a unique? */
extern int borg_fighting_unique;
extern bool
    borg_fighting_evil_unique; /* Need to know if evil for Priest Banishment */

/* am I fighting a summoner? */
extern bool borg_fighting_summoner;

/*
 * Delete an old "kill" record
 */
extern void borg_delete_kill(int i);

/*
 * Force sleep onto a "kill" record
 */
extern void borg_sleep_kill(int i);

/*
 * Attempt to "follow" a missing monster
 */
extern void borg_follow_kill(int i);

/*
 * Attempt to notice a changing "kill"
 */
extern bool observe_kill_diff(int y, int x, uint8_t a, wchar_t c);

/*
 * Attempt to notice if a "kill" moved
 */
extern bool observe_kill_move(
    int y, int x, int d, uint8_t a, wchar_t c, bool flag);

/*
 * Attempt to locate a monster which could explain a message
 */
extern int borg_locate_kill(char *who, struct loc c, int r);

/*
 * Notice the "death" of a monster
 */
extern void borg_count_death(int i);

/*
 * Prepare to "flow" towards monsters to "kill"
 */
extern bool borg_flow_kill(bool viewable, int nearness);

/*
 * Take a couple of steps to line up a shot
 */
extern bool borg_flow_kill_aim(bool viewable);

/*
 * Dig an anti-summon corridor.
 */
extern bool borg_flow_kill_corridor(bool viewable);

/*
 * Dig a straight Tunnel to a close monster
 */
extern bool borg_flow_kill_direct(bool viewable, bool twitchy);

/*
 * Check if a dangerous monster is nearby
 */
extern void borg_near_monster_type(int dist);

/*
 * a bit of magic missile and phase
 */
extern bool borg_shoot_scoot_safe(int emergency, int turns, int b_p);

extern void borg_init_flow_kill(void);
extern void borg_free_flow_kill(void);

#endif
#endif
