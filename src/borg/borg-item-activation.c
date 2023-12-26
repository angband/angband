/**
 * \file borg-item-activation.c
 * \brief The code around activation of artifacts
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

#include "borg-item-activation.h"

#ifdef ALLOW_BORG

#include "borg-init.h"
#include "borg-io.h"

/* activations */
int act_dragon_power;
int act_dragon_shining;
int act_dragon_balance;
int act_dragon_law;
int act_dragon_chaos;
int act_dragon_gold;
int act_dragon_multihued;
int act_dragon_red;
int act_dragon_green;
int act_dragon_blue;
int act_ring_lightning;
int act_ring_ice;
int act_ring_flames;
int act_ring_acid;
int act_shroom_purging;
int act_shroom_sprinting;
int act_shroom_debility;
int act_shroom_stone;
int act_shroom_terror;
int act_shroom_emergency;
int act_food_waybread;
int act_drink_breath;
int act_staff_holy;
int act_staff_magi;
int act_wand_breath;
int act_wonder;
int act_berserker;
int act_starlight2;
int act_starlight;
int act_polymorph;
int act_door_dest;
int act_disable_traps;
int act_light_line;
int act_mon_scare;
int act_sleep_all;
int act_mon_confuse;
int act_mon_slow;
int act_confuse2;
int act_tele_other;
int act_stone_to_mud;
int act_stinking_cloud;
int act_arrow;
int act_bizarre;
int act_mana_bolt;
int act_missile;
int act_drain_life4;
int act_drain_life3;
int act_drain_life2;
int act_drain_life1;
int act_elec_ball2;
int act_elec_ball;
int act_elec_bolt;
int act_acid_ball;
int act_acid_bolt3;
int act_acid_bolt2;
int act_acid_bolt;
int act_cold_ball160;
int act_cold_ball100;
int act_cold_ball50;
int act_cold_ball2;
int act_cold_bolt2;
int act_cold_bolt;
int act_fire_ball200;
int act_fire_ball2;
int act_fire_ball;
int act_fire_bolt72;
int act_fire_bolt3;
int act_fire_bolt2;
int act_fire_bolt;
int act_firebrand;
int act_rem_fear_pois;
int act_restore_life;
int act_rage_bless_resist;
int act_star_ball;
int act_sleepii;
int act_dispel_all;
int act_dispel_undead;
int act_dispel_evil60;
int act_dispel_evil;
int act_haste2;
int act_haste1;
int act_haste;
int act_probing;
int act_clairvoyance;
int act_illumination;
int act_loskill;
int act_losconf;
int act_lossleep;
int act_losslow;
int act_destruction2;
int act_earthquakes;
int act_deep_descent;
int act_recall;
int act_blessing3;
int act_blessing2;
int act_blessing;
int act_satisfy;
int act_protevil;
int act_banishment;
int act_recharge;
int act_destroy_doors;
int act_glyph;
int act_mapping;
int act_confusing;
int act_tele_level;
int act_tele_long;
int act_tele_phase;
int act_light;
int act_remove_curse2;
int act_remove_curse;
int act_enchant_armor2;
int act_enchant_armor;
int act_enchant_weapon;
int act_enchant_todam;
int act_enchant_tohit;
int act_detect_objects;
int act_detect_all;
int act_detect_evil;
int act_detect_invis;
int act_detect_treasure;
int act_resist_all;
int act_resist_pois;
int act_resist_cold;
int act_resist_fire;
int act_resist_elec;
int act_resist_acid;
int act_shero;
int act_hero;
int act_enlightenment;
int act_tmd_esp;
int act_tmd_sinvis;
int act_tmd_infra;
int act_tmd_free_act;
int act_restore_st_lev;
int act_restore_all;
int act_restore_con;
int act_restore_dex;
int act_restore_wis;
int act_restore_int;
int act_restore_str;
int act_nimbleness;
int act_toughness;
int act_contemplation;
int act_intellect;
int act_brawn;
int act_restore_mana;
int act_restore_exp;
int act_heal3;
int act_heal2;
int act_heal1;
int act_cure_temp;
int act_cure_nonorlybig;
int act_cure_full2;
int act_cure_full;
int act_cure_critical;
int act_cure_serious;
int act_cure_light;
int act_cure_body;
int act_cure_mind;
int act_cure_confusion;
int act_cure_paranoia;

static int findact(const char *act_name)
{
    struct activation *act = &activations[1];
    while (act) {
        if (streq(act->name, act_name)) {
            return act->index;
        }
        act = act->next;
    }

    borg_note(format(
        "**STARTUP FAILURE** activation lookup failure - %s ", act_name));
    borg_init_failure = true;
    return 0;
}

void borg_init_item_activation(void)
{
    act_dragon_power      = findact("DRAGON_POWER");
    act_dragon_shining    = findact("DRAGON_SHINING");
    act_dragon_balance    = findact("DRAGON_BALANCE");
    act_dragon_law        = findact("DRAGON_LAW");
    act_dragon_chaos      = findact("DRAGON_CHAOS");
    act_dragon_gold       = findact("DRAGON_GOLD");
    act_dragon_multihued  = findact("DRAGON_MULTIHUED");
    act_dragon_red        = findact("DRAGON_RED");
    act_dragon_green      = findact("DRAGON_GREEN");
    act_dragon_blue       = findact("DRAGON_BLUE");
    act_ring_lightning    = findact("RING_LIGHTNING");
    act_ring_ice          = findact("RING_ICE");
    act_ring_flames       = findact("RING_FLAMES");
    act_ring_acid         = findact("RING_ACID");
    act_shroom_purging    = findact("SHROOM_PURGING");
    act_shroom_sprinting  = findact("SHROOM_SPRINTING");
    act_shroom_debility   = findact("SHROOM_DEBILITY");
    act_shroom_stone      = findact("SHROOM_STONE");
    act_shroom_terror     = findact("SHROOM_TERROR");
    act_shroom_emergency  = findact("SHROOM_EMERGENCY");
    act_food_waybread     = findact("FOOD_WAYBREAD");
    act_drink_breath      = findact("DRINK_BREATH");
    act_staff_holy        = findact("STAFF_HOLY");
    act_staff_magi        = findact("STAFF_MAGI");
    act_wand_breath       = findact("WAND_BREATH");
    act_wonder            = findact("WONDER");
    act_berserker         = findact("BERSERKER");
    act_starlight2        = findact("STARLIGHT2");
    act_starlight         = findact("STARLIGHT");
    act_polymorph         = findact("POLYMORPH");
    act_door_dest         = findact("DOOR_DEST");
    act_disable_traps     = findact("DISABLE_TRAPS");
    act_light_line        = findact("LIGHT_LINE");
    act_mon_scare         = findact("MON_SCARE");
    act_sleep_all         = findact("SLEEP_ALL");
    act_mon_confuse       = findact("MON_CONFUSE");
    act_mon_slow          = findact("MON_SLOW");
    act_confuse2          = findact("CONFUSE2");
    act_tele_other        = findact("TELE_OTHER");
    act_stone_to_mud      = findact("STONE_TO_MUD");
    act_stinking_cloud    = findact("STINKING_CLOUD");
    act_arrow             = findact("ARROW");
    act_bizarre           = findact("BIZARRE");
    act_mana_bolt         = findact("MANA_BOLT");
    act_missile           = findact("MISSILE");
    act_drain_life4       = findact("DRAIN_LIFE4");
    act_drain_life3       = findact("DRAIN_LIFE3");
    act_drain_life2       = findact("DRAIN_LIFE2");
    act_drain_life1       = findact("DRAIN_LIFE1");
    act_elec_ball2        = findact("ELEC_BALL2");
    act_elec_ball         = findact("ELEC_BALL");
    act_elec_bolt         = findact("ELEC_BOLT");
    act_acid_ball         = findact("ACID_BALL");
    act_acid_bolt3        = findact("ACID_BOLT3");
    act_acid_bolt2        = findact("ACID_BOLT2");
    act_acid_bolt         = findact("ACID_BOLT");
    act_cold_ball160      = findact("COLD_BALL160");
    act_cold_ball100      = findact("COLD_BALL100");
    act_cold_ball50       = findact("COLD_BALL50");
    act_cold_ball2        = findact("COLD_BALL2");
    act_cold_bolt2        = findact("COLD_BOLT2");
    act_cold_bolt         = findact("COLD_BOLT");
    act_fire_ball200      = findact("FIRE_BALL200");
    act_fire_ball2        = findact("FIRE_BALL2");
    act_fire_ball         = findact("FIRE_BALL");
    act_fire_bolt72       = findact("FIRE_BOLT72");
    act_fire_bolt3        = findact("FIRE_BOLT3");
    act_fire_bolt2        = findact("FIRE_BOLT2");
    act_fire_bolt         = findact("FIRE_BOLT");
    act_firebrand         = findact("FIREBRAND");
    act_rem_fear_pois     = findact("REM_FEAR_POIS");
    act_restore_life      = findact("RESTORE_LIFE");
    act_rage_bless_resist = findact("RAGE_BLESS_RESIST");
    act_star_ball         = findact("STAR_BALL");
    act_sleepii           = findact("SLEEPII");
    act_dispel_all        = findact("DISPEL_ALL");
    act_dispel_undead     = findact("DISPEL_UNDEAD");
    act_dispel_evil60     = findact("DISPEL_EVIL60");
    act_dispel_evil       = findact("DISPEL_EVIL");
    act_haste2            = findact("HASTE2");
    act_haste1            = findact("HASTE1");
    act_haste             = findact("HASTE");
    act_probing           = findact("PROBING");
    act_clairvoyance      = findact("CLAIRVOYANCE");
    act_illumination      = findact("ILLUMINATION");
    act_loskill           = findact("LOSKILL");
    act_losconf           = findact("LOSCONF");
    act_lossleep          = findact("LOSSLEEP");
    act_losslow           = findact("LOSSLOW");
    act_destruction2      = findact("DESTRUCTION2");
    act_earthquakes       = findact("EARTHQUAKES");
    act_deep_descent      = findact("DEEP_DESCENT");
    act_recall            = findact("RECALL");
    act_blessing3         = findact("BLESSING3");
    act_blessing2         = findact("BLESSING2");
    act_blessing          = findact("BLESSING");
    act_satisfy           = findact("SATISFY");
    act_protevil          = findact("PROTEVIL");
    act_banishment        = findact("BANISHMENT");
    act_recharge          = findact("RECHARGE");
    act_destroy_doors     = findact("DESTROY_DOORS");
    act_glyph             = findact("GLYPH");
    act_mapping           = findact("MAPPING");
    act_confusing         = findact("CONFUSING");
    act_tele_level        = findact("TELE_LEVEL");
    act_tele_long         = findact("TELE_LONG");
    act_tele_phase        = findact("TELE_PHASE");
    act_light             = findact("LIGHT");
    act_remove_curse2     = findact("REMOVE_CURSE2");
    act_remove_curse      = findact("REMOVE_CURSE");
    act_enchant_armor2    = findact("ENCHANT_ARMOR2");
    act_enchant_armor     = findact("ENCHANT_ARMOR");
    act_enchant_weapon    = findact("ENCHANT_WEAPON");
    act_enchant_todam     = findact("ENCHANT_TODAM");
    act_enchant_tohit     = findact("ENCHANT_TOHIT");
    act_detect_objects    = findact("DETECT_OBJECTS");
    act_detect_all        = findact("DETECT_ALL");
    act_detect_evil       = findact("DETECT_EVIL");
    act_detect_invis      = findact("DETECT_INVIS");
    act_detect_treasure   = findact("DETECT_TREASURE");
    act_resist_all        = findact("RESIST_ALL");
    act_resist_pois       = findact("RESIST_POIS");
    act_resist_cold       = findact("RESIST_COLD");
    act_resist_fire       = findact("RESIST_FIRE");
    act_resist_elec       = findact("RESIST_ELEC");
    act_resist_acid       = findact("RESIST_ACID");
    act_shero             = findact("SHERO");
    act_hero              = findact("HERO");
    act_enlightenment     = findact("ENLIGHTENMENT");
    act_tmd_esp           = findact("TMD_ESP");
    act_tmd_sinvis        = findact("TMD_SINVIS");
    act_tmd_infra         = findact("TMD_INFRA");
    act_tmd_free_act      = findact("TMD_FREE_ACT");
    act_restore_st_lev    = findact("RESTORE_ST_LEV");
    act_restore_all       = findact("RESTORE_ALL");
    act_restore_con       = findact("RESTORE_CON");
    act_restore_dex       = findact("RESTORE_DEX");
    act_restore_wis       = findact("RESTORE_WIS");
    act_restore_int       = findact("RESTORE_INT");
    act_restore_str       = findact("RESTORE_STR");
    act_nimbleness        = findact("NIMBLENESS");
    act_toughness         = findact("TOUGHNESS");
    act_contemplation     = findact("CONTEMPLATION");
    act_intellect         = findact("INTELLECT");
    act_brawn             = findact("BRAWN");
    act_restore_mana      = findact("RESTORE_MANA");
    act_restore_exp       = findact("RESTORE_EXP");
    act_heal3             = findact("HEAL3");
    act_heal2             = findact("HEAL2");
    act_heal1             = findact("HEAL1");
    act_cure_temp         = findact("CURE_TEMP");
    act_cure_nonorlybig   = findact("CURE_NONORLYBIG");
    act_cure_full2        = findact("CURE_FULL2");
    act_cure_full         = findact("CURE_FULL");
    act_cure_critical     = findact("CURE_CRITICAL");
    act_cure_serious      = findact("CURE_SERIOUS");
    act_cure_light        = findact("CURE_LIGHT");
    act_cure_body         = findact("CURE_BODY");
    act_cure_mind         = findact("CURE_MIND");
    act_cure_confusion    = findact("CURE_CONFUSION");
    act_cure_paranoia     = findact("CURE_PARANOIA");
}

#endif
