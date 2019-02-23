/**
 * \file mon-spell.h
 * \brief structures and functions for monster spells
 *
 * Copyright (c) 2011 Chris Carr
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

#ifndef MONSTER_SPELL_H
#define MONSTER_SPELL_H

#include "monster.h"

/** Variables **/
/* none so far */

/** Constants **/

/* Spell type bitflags */
enum mon_spell_type {
	RST_NONE		= 0x0000,
	RST_BOLT		= 0x0001,
	RST_BALL		= 0x0002,	/* Ball spells, but also beams */
	RST_BREATH		= 0x0004,
	RST_DIRECT		= 0x0008,	/* Direct (non-projectable) attacks */
	RST_ANNOY		= 0x0010,	/* Irritant spells, usually non-fatal */
	RST_HASTE		= 0x0020,	/* Relative speed advantage */
	RST_HEAL		= 0x0040,
	RST_HEAL_OTHER	= 0x0080,
	RST_TACTIC		= 0x0100,	/* Get a better position */
	RST_ESCAPE		= 0x0200,
	RST_SUMMON		= 0x0400,
	RST_INNATE		= 0x0800,
	RST_ARCHERY		= 0x1000
};

#define RST_DAMAGE (RST_BOLT | RST_BALL | RST_BREATH | RST_DIRECT)

/** Macros **/
#define rsf_has(f, flag)       flag_has_dbg(f, RSF_SIZE, flag, #f, #flag)
#define rsf_next(f, flag)      flag_next(f, RSF_SIZE, flag)
#define rsf_count(f)           flag_count(f, RSF_SIZE)
#define rsf_is_empty(f)        flag_is_empty(f, RSF_SIZE)
#define rsf_is_full(f)         flag_is_full(f, RSF_SIZE)
#define rsf_is_inter(f1, f2)   flag_is_inter(f1, f2, RSF_SIZE)
#define rsf_is_subset(f1, f2)  flag_is_subset(f1, f2, RSF_SIZE)
#define rsf_is_equal(f1, f2)   flag_is_equal(f1, f2, RSF_SIZE)
#define rsf_on(f, flag)        flag_on_dbg(f, RSF_SIZE, flag, #f, #flag)
#define rsf_off(f, flag)       flag_off(f, RSF_SIZE, flag)
#define rsf_wipe(f)            flag_wipe(f, RSF_SIZE)
#define rsf_setall(f)          flag_setall(f, RSF_SIZE)
#define rsf_negate(f)          flag_negate(f, RSF_SIZE)
#define rsf_copy(f1, f2)       flag_copy(f1, f2, RSF_SIZE)
#define rsf_union(f1, f2)      flag_union(f1, f2, RSF_SIZE)
#define rsf_inter(f1, f2)      flag_inter(f1, f2, RSF_SIZE)
#define rsf_diff(f1, f2)       flag_diff(f1, f2, RSF_SIZE)


/** Functions **/
int breath_dam(int element, int hp);
const struct monster_spell *monster_spell_by_index(int index);
void do_mon_spell(int index, struct monster *mon, bool seen);
bool test_spells(bitflag *f, int types);
void ignore_spells(bitflag *f, int types);
void unset_spells(bitflag *spells, bitflag *flags, bitflag *pflags,
				  struct element_info *el, const struct monster *mon);
bool mon_spell_is_innate(int index);
void create_mon_spell_mask(bitflag *f, ...);
const char *mon_spell_lore_description(int index,
									   const struct monster_race *race);
int mon_spell_lore_damage(int index, const struct monster_race *race,
						  bool know_hp);

#endif /* MONSTER_SPELL_H */
