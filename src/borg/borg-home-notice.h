/**
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
#ifndef INCLUDED_BORG_HOME_NOTICE_H
#define INCLUDED_BORG_HOME_NOTICE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"

/*
 * Various "amounts" (for the home)
 */
extern int16_t num_food;
extern int16_t num_fuel;
extern int16_t num_mold;
extern int16_t num_ident;
extern int16_t num_recall;
extern int16_t num_phase;
extern int16_t num_escape;
extern int16_t num_tele_staves;
extern int16_t num_teleport;
extern int16_t num_berserk;
extern int16_t num_teleport_level;
extern int16_t num_recharge;

extern int16_t num_cure_critical;
extern int16_t num_cure_serious;

extern int16_t num_pot_rheat;
extern int16_t num_pot_rcold;

extern int16_t num_missile;

extern int16_t num_book[9];

extern int16_t num_fix_stat[STAT_MAX];
extern int16_t home_stat_add[STAT_MAX];

extern int16_t num_fix_exp;
extern int16_t num_mana;
extern int16_t num_heal;
extern int16_t num_heal_true;
extern int16_t num_ezheal;
extern int16_t num_ezheal_true;
extern int16_t num_life;
extern int16_t num_life_true;
extern int16_t num_pfe;
extern int16_t num_glyph;

extern int16_t num_enchant_to_a;
extern int16_t num_enchant_to_d;
extern int16_t num_enchant_to_h;
extern int16_t num_brand_weapon; /*  crubragol and bolts */
extern int16_t num_genocide;
extern int16_t num_mass_genocide;

extern int16_t num_artifact;
extern int16_t num_ego;

extern int16_t home_slot_free;
extern int16_t home_un_id;
extern int16_t home_damage;
extern int16_t num_duplicate_items;
extern int16_t num_slow_digest;
extern int16_t num_regenerate;
extern int16_t num_telepathy;
extern int16_t num_LIGHT;
extern int16_t num_see_inv;

extern int16_t num_invisible;

extern int16_t num_ffall;
extern int16_t num_free_act;
extern int16_t num_hold_life;
extern int16_t num_immune_acid;
extern int16_t num_immune_elec;
extern int16_t num_immune_fire;
extern int16_t num_immune_cold;
extern int16_t num_resist_acid;
extern int16_t num_resist_elec;
extern int16_t num_resist_fire;
extern int16_t num_resist_cold;
extern int16_t num_resist_pois;
extern int16_t num_resist_conf;
extern int16_t num_resist_sound;
extern int16_t num_resist_LIGHT;
extern int16_t num_resist_dark;
extern int16_t num_resist_chaos;
extern int16_t num_resist_disen;
extern int16_t num_resist_shard;
extern int16_t num_resist_nexus;
extern int16_t num_resist_blind;
extern int16_t num_resist_neth;
extern int16_t num_sustain_str;
extern int16_t num_sustain_int;
extern int16_t num_sustain_wis;
extern int16_t num_sustain_dex;
extern int16_t num_sustain_con;
extern int16_t num_sustain_all;

extern int16_t num_speed;
extern int16_t num_edged_weapon;
extern int16_t num_bad_gloves;
extern int16_t num_weapons;
extern int16_t num_bow;
extern int16_t num_rings;
extern int16_t num_neck;
extern int16_t num_armor;
extern int16_t num_cloaks;
extern int16_t num_shields;
extern int16_t num_hats;
extern int16_t num_gloves;
extern int16_t num_boots;

/*
 * Extract the bonuses for items in the home.
 */
extern void borg_notice_home(borg_item *in_item, bool no_items);

#endif
#endif
