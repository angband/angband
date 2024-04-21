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
#ifndef INCLUDED_BORG_ITEM_ACTIVATION_H
#define INCLUDED_BORG_ITEM_ACTIVATION_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/* dynamically loaded activations */
extern int act_dragon_power;
extern int act_dragon_shining;
extern int act_dragon_balance;
extern int act_dragon_law;
extern int act_dragon_chaos;
extern int act_dragon_gold;
extern int act_dragon_multihued;
extern int act_dragon_red;
extern int act_dragon_green;
extern int act_dragon_blue;
extern int act_ring_lightning;
extern int act_ring_ice;
extern int act_ring_flames;
extern int act_ring_acid;
extern int act_shroom_purging;
extern int act_shroom_sprinting;
extern int act_shroom_debility;
extern int act_shroom_stone;
extern int act_shroom_terror;
extern int act_shroom_emergency;
extern int act_food_waybread;
extern int act_drink_breath;
extern int act_staff_holy;
extern int act_staff_magi;
extern int act_wand_breath;
extern int act_wonder;
extern int act_berserker;
extern int act_starlight2;
extern int act_starlight;
extern int act_polymorph;
extern int act_door_dest;
extern int act_disable_traps;
extern int act_light_line;
extern int act_mon_scare;
extern int act_sleep_all;
extern int act_mon_confuse;
extern int act_mon_slow;
extern int act_confuse2;
extern int act_tele_other;
extern int act_stone_to_mud;
extern int act_stinking_cloud;
extern int act_arrow;
extern int act_bizarre;
extern int act_mana_bolt;
extern int act_missile;
extern int act_drain_life4;
extern int act_drain_life3;
extern int act_drain_life2;
extern int act_drain_life1;
extern int act_elec_ball2;
extern int act_elec_ball;
extern int act_elec_bolt;
extern int act_acid_ball;
extern int act_acid_bolt3;
extern int act_acid_bolt2;
extern int act_acid_bolt;
extern int act_cold_ball160;
extern int act_cold_ball100;
extern int act_cold_ball50;
extern int act_cold_ball2;
extern int act_cold_bolt2;
extern int act_cold_bolt;
extern int act_fire_ball200;
extern int act_fire_ball2;
extern int act_fire_ball;
extern int act_fire_bolt72;
extern int act_fire_bolt3;
extern int act_fire_bolt2;
extern int act_fire_bolt;
extern int act_firebrand;
extern int act_rem_fear_pois;
extern int act_restore_life;
extern int act_rage_bless_resist;
extern int act_star_ball;
extern int act_sleepii;
extern int act_dispel_all;
extern int act_dispel_undead;
extern int act_dispel_evil60;
extern int act_dispel_evil;
extern int act_haste2;
extern int act_haste1;
extern int act_haste;
extern int act_probing;
extern int act_clairvoyance;
extern int act_illumination;
extern int act_loskill;
extern int act_losconf;
extern int act_lossleep;
extern int act_losslow;
extern int act_destruction2;
extern int act_earthquakes;
extern int act_deep_descent;
extern int act_recall;
extern int act_blessing3;
extern int act_blessing2;
extern int act_blessing;
extern int act_satisfy;
extern int act_protevil;
extern int act_banishment;
extern int act_recharge;
extern int act_destroy_doors;
extern int act_glyph;
extern int act_mapping;
extern int act_confusing;
extern int act_tele_level;
extern int act_tele_long;
extern int act_tele_phase;
extern int act_light;
extern int act_remove_curse2;
extern int act_remove_curse;
extern int act_enchant_armor2;
extern int act_enchant_armor;
extern int act_enchant_weapon;
extern int act_enchant_todam;
extern int act_enchant_tohit;
extern int act_detect_objects;
extern int act_detect_all;
extern int act_detect_evil;
extern int act_detect_invis;
extern int act_detect_treasure;
extern int act_resist_all;
extern int act_resist_pois;
extern int act_resist_cold;
extern int act_resist_fire;
extern int act_resist_elec;
extern int act_resist_acid;
extern int act_shero;
extern int act_hero;
extern int act_enlightenment;
extern int act_tmd_esp;
extern int act_tmd_sinvis;
extern int act_tmd_infra;
extern int act_tmd_free_act;
extern int act_restore_st_lev;
extern int act_restore_all;
extern int act_restore_con;
extern int act_restore_dex;
extern int act_restore_wis;
extern int act_restore_int;
extern int act_restore_str;
extern int act_nimbleness;
extern int act_toughness;
extern int act_contemplation;
extern int act_intellect;
extern int act_brawn;
extern int act_restore_mana;
extern int act_restore_exp;
extern int act_heal3;
extern int act_heal2;
extern int act_heal1;
extern int act_cure_temp;
extern int act_cure_nonorlybig;
extern int act_cure_full2;
extern int act_cure_full;
extern int act_cure_critical;
extern int act_cure_serious;
extern int act_cure_light;
extern int act_cure_body;
extern int act_cure_mind;
extern int act_cure_confusion;
extern int act_cure_paranoia;

extern int borg_findact(const char *act_name);

extern void borg_init_item_activation(void);

#endif
#endif
