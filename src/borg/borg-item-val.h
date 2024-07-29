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
#ifndef INCLUDED_BORG_ITEM_VAL_H
#define INCLUDED_BORG_ITEM_VAL_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/* s_val's and k_val's now dynamically loaded */
extern int sv_food_apple;
extern int sv_food_ration;
extern int sv_food_slime_mold;
extern int sv_food_draught;
extern int sv_food_pint;
extern int sv_food_sip;
extern int sv_food_waybread;
extern int sv_food_honey_cake;
extern int sv_food_slice;
extern int sv_food_handful;

extern int sv_mush_second_sight;
extern int sv_mush_fast_recovery;
extern int sv_mush_restoring;
extern int sv_mush_mana;
extern int sv_mush_emergency;
extern int sv_mush_terror;
extern int sv_mush_stoneskin;
extern int kv_mush_stoneskin;
extern int sv_mush_debility;
extern int sv_mush_sprinting;
extern int sv_mush_cure_mind;
extern int sv_mush_purging;

extern int sv_light_lantern;
extern int sv_light_torch;

extern int sv_flask_oil;
extern int kv_flask_oil;

extern int sv_potion_cure_critical;
extern int sv_potion_cure_serious;
extern int sv_potion_cure_light;
extern int sv_potion_healing;
extern int kv_potion_healing;
extern int sv_potion_star_healing;
extern int sv_potion_life;
extern int sv_potion_restore_mana;
extern int kv_potion_restore_mana;
extern int sv_potion_cure_poison;
extern int sv_potion_resist_heat;
extern int sv_potion_resist_cold;
extern int sv_potion_resist_pois;
extern int sv_potion_inc_str;
extern int sv_potion_inc_int;
extern int sv_potion_inc_wis;
extern int sv_potion_inc_dex;
extern int sv_potion_inc_con;
extern int sv_potion_inc_str2;
extern int sv_potion_inc_int2;
extern int sv_potion_inc_wis2;
extern int sv_potion_inc_dex2;
extern int sv_potion_inc_con2;
extern int sv_potion_inc_all;
extern int sv_potion_restore_life;
extern int sv_potion_speed;
extern int sv_potion_berserk;
extern int sv_potion_sleep;
extern int sv_potion_slowness;
extern int sv_potion_poison;
extern int sv_potion_blindness;
extern int sv_potion_confusion;
extern int sv_potion_heroism;
extern int sv_potion_boldness;
extern int sv_potion_detect_invis;
extern int sv_potion_enlightenment;
extern int sv_potion_slime_mold;
extern int sv_potion_infravision;
extern int sv_potion_inc_exp;

extern int sv_scroll_identify;
extern int sv_scroll_phase_door;
extern int sv_scroll_teleport;
extern int sv_scroll_word_of_recall;
extern int sv_scroll_enchant_armor;
extern int sv_scroll_enchant_weapon_to_hit;
extern int sv_scroll_enchant_weapon_to_dam;
extern int sv_scroll_star_enchant_weapon;
extern int sv_scroll_star_enchant_armor;
extern int sv_scroll_protection_from_evil;
extern int sv_scroll_rune_of_protection;
extern int sv_scroll_teleport_level;
extern int sv_scroll_deep_descent;
extern int sv_scroll_recharging;
extern int sv_scroll_banishment;
extern int sv_scroll_mass_banishment;
extern int kv_scroll_mass_banishment;
extern int sv_scroll_blessing;
extern int sv_scroll_holy_chant;
extern int sv_scroll_holy_prayer;
extern int sv_scroll_detect_invis;
extern int sv_scroll_satisfy_hunger;
extern int sv_scroll_light;
extern int sv_scroll_mapping;
extern int sv_scroll_acquirement;
extern int sv_scroll_star_acquirement;
extern int sv_scroll_remove_curse;
extern int kv_scroll_remove_curse;
extern int sv_scroll_star_remove_curse;
extern int kv_scroll_star_remove_curse;
extern int sv_scroll_monster_confusion;
extern int sv_scroll_trap_door_destruction;
extern int sv_scroll_dispel_undead;

extern int sv_ring_flames;
extern int sv_ring_ice;
extern int sv_ring_acid;
extern int sv_ring_lightning;
extern int sv_ring_digging;
extern int sv_ring_speed;
extern int sv_ring_damage;
extern int sv_ring_dog;

extern int sv_amulet_teleportation;

extern int sv_rod_recall;
extern int kv_rod_recall;
extern int sv_rod_detection;
extern int sv_rod_illumination;
extern int sv_rod_speed;
extern int sv_rod_mapping;
extern int sv_rod_healing;
extern int kv_rod_healing;
extern int sv_rod_light;
extern int sv_rod_fire_bolt;
extern int sv_rod_elec_bolt;
extern int sv_rod_cold_bolt;
extern int sv_rod_acid_bolt;
extern int sv_rod_drain_life;
extern int sv_rod_fire_ball;
extern int sv_rod_elec_ball;
extern int sv_rod_cold_ball;
extern int sv_rod_acid_ball;
extern int sv_rod_teleport_other;
extern int sv_rod_slow_monster;
extern int sv_rod_sleep_monster;
extern int sv_rod_curing;

extern int sv_staff_teleportation;
extern int sv_staff_destruction;
extern int sv_staff_speed;
extern int sv_staff_healing;
extern int sv_staff_the_magi;
extern int sv_staff_power;
extern int sv_staff_curing;
extern int sv_staff_holiness;
extern int kv_staff_holiness;
extern int sv_staff_sleep_monsters;
extern int sv_staff_slow_monsters;
extern int sv_staff_detect_invis;
extern int sv_staff_detect_evil;
extern int sv_staff_dispel_evil;
extern int sv_staff_banishment;
extern int sv_staff_light;
extern int sv_staff_mapping;
extern int sv_staff_remove_curse;

extern int sv_wand_light;
extern int sv_wand_teleport_away;
extern int sv_wand_stinking_cloud;
extern int kv_wand_stinking_cloud;
extern int sv_wand_magic_missile;
extern int kv_wand_magic_missile;
extern int sv_wand_annihilation;
extern int kv_wand_annihilation;
extern int sv_wand_stone_to_mud;
extern int sv_wand_wonder;
extern int sv_wand_hold_monster;
extern int sv_wand_slow_monster;
extern int sv_wand_fear_monster;
extern int sv_wand_confuse_monster;
extern int sv_wand_fire_bolt;
extern int sv_wand_cold_bolt;
extern int sv_wand_acid_bolt;
extern int sv_wand_elec_bolt;
extern int sv_wand_fire_ball;
extern int sv_wand_cold_ball;
extern int sv_wand_acid_ball;
extern int sv_wand_elec_ball;
extern int sv_wand_dragon_cold;
extern int sv_wand_dragon_fire;
extern int sv_wand_drain_life;

extern int sv_dagger;

extern int sv_sling;
extern int sv_short_bow;
extern int sv_long_bow;
extern int sv_light_xbow;
extern int sv_heavy_xbow;

extern int sv_arrow_seeker;
extern int sv_arrow_mithril;

extern int sv_bolt_seeker;
extern int sv_bolt_mithril;

extern int sv_set_of_leather_gloves;

extern int sv_cloak;

extern int sv_robe;

extern int sv_iron_crown;

extern int sv_dragon_blue;
extern int sv_dragon_black;
extern int sv_dragon_white;
extern int sv_dragon_red;
extern int sv_dragon_green;
extern int sv_dragon_multihued;
extern int sv_dragon_shining;
extern int sv_dragon_law;
extern int sv_dragon_gold;
extern int sv_dragon_chaos;
extern int sv_dragon_balance;
extern int sv_dragon_power;

extern void borg_init_item_val(void);

extern int borg_lookup_kind(int tval, int sval);

#endif
#endif
