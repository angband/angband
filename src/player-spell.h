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

extern void get_spell_info(int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int spell);
extern bool spell_is_identify(int spell);
extern expression_base_value_f spell_value_base_by_name(const char *name);

