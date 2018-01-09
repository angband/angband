/**
 * \file mon-timed.h
 * \brief Structures and functions for monster timed effects.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef MONSTER_TIMED_H
#define MONSTER_TIMED_H

/**
 * Monster timed effect constants
 */
#define STUN_MISS_CHANCE		10  /* 1 in __ chance of missing turn when stunned */
#define STUN_HIT_REDUCTION		25  /* Percentage reduction in accuracy for combat */
#define STUN_DAM_REDUCTION		25  /* Percentage reduction in damage */

#define CONF_ERRATIC_CHANCE		30  /* Percentage chance of erratic movement when confused */
#define CONF_HIT_REDUCTION		20  /* Percentage reduction in accuracy for spells */
#define CONF_RANDOM_CHANCE		40  /* Percentage chance of an aimed spell going in random direction */

#define DEBUFF_CRITICAL_HIT		10  /* Effective increase in to-hit for critical hit calcs */

/**
 * Monster Timed Effects
 */
enum {
	#define MON_TMD(a, b, c, d, e, f, g, h) MON_TMD_##a,
	#include "list-mon-timed.h"
	#undef MON_TMD
};

/**
 * Flags for the monster timed functions
 */
#define MON_TMD_FLG_NOTIFY		0x01 /* Give notification */
/* 0x02 */
#define MON_TMD_FLG_NOMESSAGE	0x04 /* Never show a message */
#define MON_TMD_FLG_NOFAIL		0x08 /* Never fail */

/** Functions **/
int mon_timed_name_to_idx(const char *name);
bool mon_inc_timed(struct monster *mon, int effect_type, int timer, int flag, bool id);
bool mon_dec_timed(struct monster *mon, int effect_type, int timer, int flag, bool id);
bool mon_clear_timed(struct monster *mon, int effect_type, int flag, bool id);
int monster_effect_level(struct monster *mon, int effect_type);

#endif /* MONSTER_TIMED_H */
