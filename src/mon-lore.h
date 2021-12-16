/**
 * \file mon-lore.h
 * \brief Structures and functions for monster recall.
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

#ifndef MONSTER_LORE_H
#define MONSTER_LORE_H

#include "z-textblock.h"
#include "monster.h"

/**
 * Monster "lore" information
 */
typedef struct monster_lore
{
	int ridx;			/* Index of monster race */

	uint16_t sights;		/* Count sightings of this monster */
	uint16_t deaths;		/* Count deaths from this monster */

	uint16_t pkills;		/* Count monsters killed in this life */
	uint16_t thefts;		/* Count objects stolen in this life */
	uint16_t tkills;		/* Count monsters killed in all lives */

	uint8_t wake;			/* Number of times woken up (?) */
	uint8_t ignore;			/* Number of times ignored (?) */

	uint8_t drop_gold;		/* Max number of gold dropped at once */
	uint8_t drop_item;		/* Max number of item dropped at once */

	uint8_t cast_innate;		/* Max number of innate spells seen */
	uint8_t cast_spell;		/* Max number of other spells seen */

	struct monster_blow *blows; /* Knowledge of blows */

	bitflag flags[RF_SIZE]; /* Observed racial flags - a 1 indicates
	                         * the flag (or lack thereof) is known to
	                         * the player */
	bitflag spell_flags[RSF_SIZE];  /* Observed racial spell flags */

	struct monster_drop *drops;
    struct monster_friends *friends;
	struct monster_friends_base *friends_base;
	struct monster_mimic *mimic_kinds;

	/* Derived known fields, put here for simplicity */
	bool all_known;
	bool *blow_known;
	bool armour_known;
	bool drop_known;
	bool sleep_known;
	bool spell_freq_known;
	bool innate_freq_known;
} monster_lore;

/**
 * Array[z_info->r_max] of monster lore
 */
extern struct monster_lore *l_list;

void get_attack_colors(int *melee_colors);
void lore_append_kills(textblock *tb, const struct monster_race *race,
					   const struct monster_lore *lore,
					   const bitflag known_flags[RF_SIZE]);
void lore_append_flavor(textblock *tb, const struct monster_race *race);
void lore_append_movement(textblock *tb, const struct monster_race *race,
						  const struct monster_lore *lore,
						  bitflag known_flags[RF_SIZE]);
void lore_append_toughness(textblock *tb, const struct monster_race *race,
						   const struct monster_lore *lore,
						   bitflag known_flags[RF_SIZE]);
void lore_append_exp(textblock *tb, const struct monster_race *race,
					 const struct monster_lore *lore,
					 bitflag known_flags[RF_SIZE]);
void lore_append_drop(textblock *tb, const struct monster_race *race,
					  const struct monster_lore *lore,
					  bitflag known_flags[RF_SIZE]);
void lore_append_abilities(textblock *tb, const struct monster_race *race,
						   const struct monster_lore *lore,
						   bitflag known_flags[RF_SIZE]);
void lore_append_awareness(textblock *tb, const struct monster_race *race,
						   const struct monster_lore *lore,
						   bitflag known_flags[RF_SIZE]);
void lore_append_friends(textblock *tb, const struct monster_race *race,
						 const struct monster_lore *lore,
						 bitflag known_flags[RF_SIZE]);
void lore_append_spells(textblock *tb, const struct monster_race *race,
						const struct monster_lore *lore,
						bitflag known_flags[RF_SIZE]);
void lore_append_attack(textblock *tb, const struct monster_race *race,
						const struct monster_lore *lore,
						bitflag known_flags[RF_SIZE]);

void lore_learn_spell_if_has(struct monster_lore *lore, const struct monster_race *race, int flag);
void lore_learn_spell_if_visible(struct monster_lore *lore, const struct monster *mon, int flag);
void lore_learn_flag_if_visible(struct monster_lore *lore, const struct monster *mon, int flag);

void lore_update(const struct monster_race *race, struct monster_lore *lore);
void cheat_monster_lore(const struct monster_race *race,
						struct monster_lore *lore);
void wipe_monster_lore(const struct monster_race *race,
					   struct monster_lore *lore);
void lore_do_probe(struct monster *m);
bool lore_is_fully_known(const struct monster_race *race);
void monster_flags_known(const struct monster_race *race,
						 const struct monster_lore *lore,
						 bitflag flags[RF_SIZE]);
void lore_treasure(struct monster *mon, int num_item, int num_gold);
struct monster_lore *get_lore(const struct monster_race *race);
bool lore_save(const char *name);

#endif /* MONSTER_LORE_H */
