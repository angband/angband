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

enum spell_effect_e {
	#define S_EF(x, a, s) SPELL_EFFECT_##x,
	#include "list-player-spells.h"
	#undef S_EF
};

typedef struct spell_handler_context_s {
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

/* spell.c */
void player_spells_init(struct player *p);
void player_spells_free(struct player *p);
const class_book *object_to_book(const struct object *obj);
const class_spell *spell_by_index(int index);
int spell_collect_from_book(const object_type *o_ptr, int **spells);
int spell_book_count_spells(const object_type *o_ptr, bool (*tester)(int spell));
bool spell_okay_list(bool (*spell_test)(int spell), const int spells[], int n_spells);
bool spell_okay_to_cast(int spell);
bool spell_okay_to_study(int spell);
bool spell_okay_to_browse(int spell);
s16b spell_chance(int spell);
void spell_learn(int spell);
bool spell_cast(int spell, int dir);

/* Start of old x-spell.c */
extern void get_spell_info(int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int spell);
extern bool spell_is_identify(int spell);
extern expression_base_value_f spell_value_base_by_name(const char *name);

