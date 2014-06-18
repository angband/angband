/**
   \file player-spell.h
   \brief Spell and prayer casting/praying
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
/*
 * The defines below must match the spell numbers in spell.txt
 * if they don't, "interesting" things will probably happen.
 *
 * It would be nice if we could get rid of this dependency.
 */

/* Temp! NRM */
#define NEW_SPELLS 0

enum spell_index_arcane_e {
	#define SPELL(x, a, s, f) SPELL_##x,
	#include "list-spells-arcane.h"
	#undef SPELL
};

enum spell_index_prayer_e {
	#define PRAYER(x, a, s, f) PRAYER_##x,
	#include "list-spells-prayer.h"
	#undef PRAYER
};

typedef struct spell_handler_context_s {
	const int spell;
	const int dir;
	const int beam;
	const random_value value;
	const int p1, p2, p3;
} spell_handler_context_t;

typedef bool (*spell_handler_f)(spell_handler_context_t *);

typedef struct spell_info_s {
	u16b spell;
	bool aim;
	const char *info;
	spell_handler_f handler;
} spell_info_t;

#if NEW_SPELLS
int spell_collect_from_book(const object_type *o_ptr, struct player_spell **spells);
int spell_book_count_spells(const object_type *o_ptr,
							bool (*tester)(struct player_spell *spell));
bool spell_okay_list(bool (*spell_test)(struct player_spell *spell),
		const struct player_spell **spells, int n_spells);
bool spell_okay_to_cast(struct player_spell *spell);
bool spell_okay_to_study(struct player_spell *spell);
bool spell_okay_to_browse(struct player_spell *spell);
s16b spell_chance(struct player_spell *spell);
void spell_learn(class_spell cspell, int tval);
bool spell_cast(struct player_spell *spell, int dir);
#else
/* spell.c */
int spell_collect_from_book(const object_type *o_ptr, int *spells);
int spell_book_count_spells(const object_type *o_ptr, bool (*tester)(int spell));
bool spell_okay_list(bool (*spell_test)(int spell), const int spells[], int n_spells);
bool spell_okay_to_cast(int spell);
bool spell_okay_to_study(int spell);
bool spell_okay_to_browse(int spell);
s16b spell_chance(int spell);
void spell_learn(int spell);
bool spell_cast(int spell, int dir);
#endif
/* Start of old x-spell.c */
extern int get_spell_index(const object_type *o_ptr, int index);
extern const char *get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int tval, int spell);
extern bool spell_is_identify(int book, int spell);
extern int spell_lookup_by_name(int tval, const char *name);
extern expression_base_value_f spell_value_base_by_name(const char *name);

