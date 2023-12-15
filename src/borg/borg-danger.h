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

#ifndef INCLUDED_BORG_DANGER_H
#define INCLUDED_BORG_DANGER_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-cave.h"

enum BORG_MONBLOW {
    MONBLOW_NONE,
    MONBLOW_HURT,
    MONBLOW_POISON,
    MONBLOW_DISENCHANT,
    MONBLOW_DRAIN_CHARGES,
    MONBLOW_EAT_GOLD,
    MONBLOW_EAT_ITEM,
    MONBLOW_EAT_FOOD,
    MONBLOW_EAT_LIGHT,
    MONBLOW_ACID,
    MONBLOW_ELEC,
    MONBLOW_FIRE,
    MONBLOW_COLD,
    MONBLOW_BLIND,
    MONBLOW_CONFUSE,
    MONBLOW_TERRIFY,
    MONBLOW_PARALYZE,
    MONBLOW_LOSE_STR,
    MONBLOW_LOSE_INT,
    MONBLOW_LOSE_WIS,
    MONBLOW_LOSE_DEX,
    MONBLOW_LOSE_CON,
    MONBLOW_LOSE_ALL,
    MONBLOW_SHATTER,
    MONBLOW_EXP_10,
    MONBLOW_EXP_20,
    MONBLOW_EXP_40,
    MONBLOW_EXP_80,
    MONBLOW_HALLU,
    MONBLOW_BLACK_BREATH,
    MONBLOW_UNDEFINED
};

/*
 * Hack -- extra fear per "region"
 */
extern uint16_t borg_fear_region[(AUTO_MAX_Y / 11) + 1][(AUTO_MAX_X / 11) + 1];

/*
 * Hack -- extra fear per "region" induced from extra monsters.
 */
extern uint16_t borg_fear_monsters[AUTO_MAX_Y + 1][AUTO_MAX_X + 1];

/*
 * Recalculate danger
 */
extern bool borg_danger_wipe;

/*
 * Calculate danger to a grid from a monster
 */
extern int borg_danger_one_kill(
    int y, int x, int c, int i, bool average, bool full_damage);

/*
 * Hack -- Calculate the "danger" of the given grid.
 */
extern int borg_danger(int y, int x, int c, bool average, bool full_damage);

#endif
#endif
