/**
 * \file borg-item-val.c
 * \brief Load the sval and kval of the items
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

#include "borg-item-val.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../obj-tval.h"
#include "../obj-util.h"

#include "borg-init.h"
#include "borg-io.h"

int sv_food_apple;
int sv_food_ration;
int sv_food_slime_mold;
int sv_food_draught;
int sv_food_pint;
int sv_food_sip;
int sv_food_waybread;
int sv_food_honey_cake;
int sv_food_slice;
int sv_food_handful;

int sv_mush_second_sight;
int sv_mush_fast_recovery;
int sv_mush_restoring;
int sv_mush_mana;
int sv_mush_emergency;
int sv_mush_terror;
int sv_mush_stoneskin;
int kv_mush_stoneskin;
int sv_mush_debility;
int sv_mush_sprinting;
int sv_mush_cure_mind;
int sv_mush_purging;

int sv_light_lantern;
int sv_light_torch;

int sv_flask_oil;
int kv_flask_oil;

int sv_potion_cure_critical;
int sv_potion_cure_serious;
int sv_potion_cure_light;
int sv_potion_healing;
int kv_potion_healing;
int sv_potion_star_healing;
int sv_potion_life;
int sv_potion_restore_mana;
int kv_potion_restore_mana;
int sv_potion_cure_poison;
int sv_potion_resist_heat;
int sv_potion_resist_cold;
int sv_potion_resist_pois;
int sv_potion_inc_str;
int sv_potion_inc_int;
int sv_potion_inc_wis;
int sv_potion_inc_dex;
int sv_potion_inc_con;
int sv_potion_inc_str2;
int sv_potion_inc_int2;
int sv_potion_inc_wis2;
int sv_potion_inc_dex2;
int sv_potion_inc_con2;
int sv_potion_inc_all;
int sv_potion_restore_life;
int sv_potion_speed;
int sv_potion_berserk;
int sv_potion_sleep;
int sv_potion_slowness;
int sv_potion_poison;
int sv_potion_blindness;
int sv_potion_confusion;
int sv_potion_heroism;
int sv_potion_boldness;
int sv_potion_detect_invis;
int sv_potion_enlightenment;
int sv_potion_slime_mold;
int sv_potion_infravision;
int sv_potion_inc_exp;

int sv_scroll_identify;
int sv_scroll_phase_door;
int sv_scroll_teleport;
int sv_scroll_word_of_recall;
int sv_scroll_enchant_armor;
int sv_scroll_enchant_weapon_to_hit;
int sv_scroll_enchant_weapon_to_dam;
int sv_scroll_star_enchant_weapon;
int sv_scroll_star_enchant_armor;
int sv_scroll_protection_from_evil;
int sv_scroll_rune_of_protection;
int sv_scroll_teleport_level;
int sv_scroll_deep_descent;
int sv_scroll_recharging;
int sv_scroll_banishment;
int sv_scroll_mass_banishment;
int kv_scroll_mass_banishment;
int sv_scroll_blessing;
int sv_scroll_holy_chant;
int sv_scroll_holy_prayer;
int sv_scroll_detect_invis;
int sv_scroll_satisfy_hunger;
int sv_scroll_light;
int sv_scroll_mapping;
int sv_scroll_acquirement;
int sv_scroll_star_acquirement;
int sv_scroll_remove_curse;
int kv_scroll_remove_curse;
int sv_scroll_star_remove_curse;
int kv_scroll_star_remove_curse;
int sv_scroll_monster_confusion;
int sv_scroll_trap_door_destruction;
int sv_scroll_dispel_undead;

int sv_ring_flames;
int sv_ring_ice;
int sv_ring_acid;
int sv_ring_lightning;
int sv_ring_digging;
int sv_ring_speed;
int sv_ring_damage;
int sv_ring_dog;

int sv_amulet_teleportation;

int sv_rod_recall;
int kv_rod_recall;
int sv_rod_detection;
int sv_rod_illumination;
int sv_rod_speed;
int sv_rod_mapping;
int sv_rod_healing;
int kv_rod_healing;
int sv_rod_light;
int sv_rod_fire_bolt;
int sv_rod_elec_bolt;
int sv_rod_cold_bolt;
int sv_rod_acid_bolt;
int sv_rod_drain_life;
int sv_rod_fire_ball;
int sv_rod_elec_ball;
int sv_rod_cold_ball;
int sv_rod_acid_ball;
int sv_rod_teleport_other;
int sv_rod_slow_monster;
int sv_rod_sleep_monster;
int sv_rod_curing;

int sv_staff_teleportation;
int sv_staff_destruction;
int sv_staff_speed;
int sv_staff_healing;
int sv_staff_the_magi;
int sv_staff_power;
int sv_staff_curing;
int sv_staff_holiness;
int kv_staff_holiness;
int sv_staff_sleep_monsters;
int sv_staff_slow_monsters;
int sv_staff_detect_invis;
int sv_staff_detect_evil;
int sv_staff_dispel_evil;
int sv_staff_banishment;
int sv_staff_light;
int sv_staff_mapping;
int sv_staff_remove_curse;

int sv_wand_light;
int sv_wand_teleport_away;
int sv_wand_stinking_cloud;
int kv_wand_stinking_cloud;
int sv_wand_magic_missile;
int kv_wand_magic_missile;
int sv_wand_annihilation;
int kv_wand_annihilation;
int sv_wand_stone_to_mud;
int sv_wand_wonder;
int sv_wand_slow_monster;
int sv_wand_hold_monster;
int sv_wand_fear_monster;
int sv_wand_confuse_monster;
int sv_wand_fire_bolt;
int sv_wand_cold_bolt;
int sv_wand_acid_bolt;
int sv_wand_elec_bolt;
int sv_wand_fire_ball;
int sv_wand_cold_ball;
int sv_wand_acid_ball;
int sv_wand_elec_ball;
int sv_wand_dragon_cold;
int sv_wand_dragon_fire;
int sv_wand_drain_life;

int sv_dagger;

int sv_sling;
int sv_short_bow;
int sv_long_bow;
int sv_light_xbow;
int sv_heavy_xbow;

int sv_arrow_seeker;
int sv_arrow_mithril;

int sv_bolt_seeker;
int sv_bolt_mithril;

int sv_set_of_leather_gloves;

int sv_cloak;

int sv_robe;

int sv_iron_crown;

int sv_dragon_blue;
int sv_dragon_black;
int sv_dragon_white;
int sv_dragon_red;
int sv_dragon_green;
int sv_dragon_multihued;
int sv_dragon_shining;
int sv_dragon_law;
int sv_dragon_gold;
int sv_dragon_chaos;
int sv_dragon_balance;
int sv_dragon_power;

/* a helper to make sure our definitions are correct */
static int borg_lookup_sval_fail(int tval, const char *name)
{
    int sval = lookup_sval(tval, name);
    if (sval == -1) {
        borg_note(
            format("**STARTUP FAILURE** sval lookup failure - %s ", name));
        borg_init_failure = true;
    }
    return sval;
}

void borg_init_item_val(void)
{
    int tval           = tval_find_idx("food");
    sv_food_apple      = borg_lookup_sval_fail(tval, "Apple");
    sv_food_ration     = borg_lookup_sval_fail(tval, "Ration of Food");
    sv_food_slime_mold = borg_lookup_sval_fail(tval, "Slime Mold");
    sv_food_draught    = borg_lookup_sval_fail(tval, "Draught of the Ents");
    sv_food_pint       = borg_lookup_sval_fail(tval, "Pint of Fine Wine");
    sv_food_sip        = borg_lookup_sval_fail(tval, "Sip of Miruvor");
    sv_food_waybread = borg_lookup_sval_fail(tval, "Piece of Elvish Waybread");
    sv_food_honey_cake = borg_lookup_sval_fail(tval, "Honey-cake");
    sv_food_slice      = borg_lookup_sval_fail(tval, "Slice of Meat");
    sv_food_handful    = borg_lookup_sval_fail(tval, "Handful of Dried Fruits");

    tval               = tval_find_idx("mushroom");
    sv_mush_second_sight  = borg_lookup_sval_fail(tval, "Second Sight");
    sv_mush_fast_recovery = borg_lookup_sval_fail(tval, "Fast Recovery");
    sv_mush_restoring     = borg_lookup_sval_fail(tval, "Vigor");
    sv_mush_mana          = borg_lookup_sval_fail(tval, "Clear Mind");
    sv_mush_emergency     = borg_lookup_sval_fail(tval, "Emergency");
    sv_mush_terror        = borg_lookup_sval_fail(tval, "Terror");
    sv_mush_stoneskin     = borg_lookup_sval_fail(tval, "Stoneskin");
    kv_mush_stoneskin     = borg_lookup_kind(tval, sv_mush_stoneskin);
    sv_mush_debility      = borg_lookup_sval_fail(tval, "Debility");
    sv_mush_sprinting     = borg_lookup_sval_fail(tval, "Sprinting");
    sv_mush_cure_mind     = borg_lookup_sval_fail(tval, "Clear Mind");
    sv_mush_purging       = borg_lookup_sval_fail(tval, "Purging");

    tval                  = tval_find_idx("light");
    sv_light_lantern      = borg_lookup_sval_fail(tval, "Lantern");
    sv_light_torch        = borg_lookup_sval_fail(tval, "Wooden Torch");

    tval                  = tval_find_idx("flask");
    sv_flask_oil          = borg_lookup_sval_fail(tval, "Flask of Oil");
    kv_flask_oil          = borg_lookup_kind(tval, sv_flask_oil);

    tval                  = tval_find_idx("potion");
    sv_potion_cure_critical
        = borg_lookup_sval_fail(tval, "Cure Critical Wounds");
    sv_potion_cure_serious = borg_lookup_sval_fail(tval, "Cure Serious Wounds");
    sv_potion_cure_light   = borg_lookup_sval_fail(tval, "Cure Light Wounds");
    sv_potion_healing      = borg_lookup_sval_fail(tval, "Healing");
    kv_potion_healing      = borg_lookup_kind(tval, sv_potion_healing);
    sv_potion_star_healing = borg_lookup_sval_fail(tval, "*Healing*");
    sv_potion_life         = borg_lookup_sval_fail(tval, "Life");
    sv_potion_restore_mana = borg_lookup_sval_fail(tval, "Restore Mana");
    kv_potion_restore_mana = borg_lookup_kind(tval, sv_potion_restore_mana);
    sv_potion_cure_poison  = borg_lookup_sval_fail(tval, "Neutralize Poison");
    sv_potion_resist_heat  = borg_lookup_sval_fail(tval, "Resist Heat");
    sv_potion_resist_cold  = borg_lookup_sval_fail(tval, "Resist Cold");
    sv_potion_resist_pois  = borg_lookup_sval_fail(tval, "Resist Poison");
    sv_potion_inc_str      = borg_lookup_sval_fail(tval, "Strength");
    sv_potion_inc_int      = borg_lookup_sval_fail(tval, "Intelligence");
    sv_potion_inc_wis      = borg_lookup_sval_fail(tval, "Wisdom");
    sv_potion_inc_dex      = borg_lookup_sval_fail(tval, "Dexterity");
    sv_potion_inc_con      = borg_lookup_sval_fail(tval, "Constitution");
    sv_potion_inc_all      = borg_lookup_sval_fail(tval, "Augmentation");
    sv_potion_inc_str2     = borg_lookup_sval_fail(tval, "Brawn");
    sv_potion_inc_int2     = borg_lookup_sval_fail(tval, "Intellect");
    sv_potion_inc_wis2     = borg_lookup_sval_fail(tval, "Contemplation");
    sv_potion_inc_dex2     = borg_lookup_sval_fail(tval, "Nimbleness");
    sv_potion_inc_con2     = borg_lookup_sval_fail(tval, "Toughness");
    sv_potion_restore_life = borg_lookup_sval_fail(tval, "Restore Life Levels");
    sv_potion_speed        = borg_lookup_sval_fail(tval, "Speed");
    sv_potion_berserk      = borg_lookup_sval_fail(tval, "Berserk Strength");
    sv_potion_sleep        = borg_lookup_sval_fail(tval, "Sleep");
    sv_potion_slowness     = borg_lookup_sval_fail(tval, "Slowness");
    sv_potion_poison       = borg_lookup_sval_fail(tval, "Poison");
    sv_potion_blindness    = borg_lookup_sval_fail(tval, "Blindness");
    sv_potion_confusion    = borg_lookup_sval_fail(tval, "Confusion");
    sv_potion_heroism      = borg_lookup_sval_fail(tval, "Heroism");
    sv_potion_boldness     = borg_lookup_sval_fail(tval, "Boldness");
    sv_potion_detect_invis = borg_lookup_sval_fail(tval, "True Seeing");
    sv_potion_enlightenment  = borg_lookup_sval_fail(tval, "Enlightenment");
    sv_potion_slime_mold     = borg_lookup_sval_fail(tval, "Slime Mold Juice");
    sv_potion_berserk        = borg_lookup_sval_fail(tval, "Berserk Strength");
    sv_potion_infravision    = borg_lookup_sval_fail(tval, "Infravision");
    sv_potion_inc_exp        = borg_lookup_sval_fail(tval, "Experience");

    tval                     = tval_find_idx("scroll");
    sv_scroll_identify       = borg_lookup_sval_fail(tval, "Identify Rune");
    sv_scroll_phase_door     = borg_lookup_sval_fail(tval, "Phase Door");
    sv_scroll_teleport       = borg_lookup_sval_fail(tval, "Teleportation");
    sv_scroll_word_of_recall = borg_lookup_sval_fail(tval, "Word of Recall");
    sv_scroll_enchant_armor  = borg_lookup_sval_fail(tval, "Enchant Armour");
    sv_scroll_enchant_weapon_to_hit
        = borg_lookup_sval_fail(tval, "Enchant Weapon To-Hit");
    sv_scroll_enchant_weapon_to_dam
        = borg_lookup_sval_fail(tval, "Enchant Weapon To-Dam");
    sv_scroll_star_enchant_armor
        = borg_lookup_sval_fail(tval, "*Enchant Armour*");
    sv_scroll_star_enchant_weapon
        = borg_lookup_sval_fail(tval, "*Enchant Weapon*");
    sv_scroll_protection_from_evil
        = borg_lookup_sval_fail(tval, "Protection from Evil");
    sv_scroll_rune_of_protection
        = borg_lookup_sval_fail(tval, "Rune of Protection");
    sv_scroll_teleport_level  = borg_lookup_sval_fail(tval, "Teleport Level");
    sv_scroll_deep_descent    = borg_lookup_sval_fail(tval, "Deep Descent");
    sv_scroll_recharging      = borg_lookup_sval_fail(tval, "Recharging");
    sv_scroll_banishment      = borg_lookup_sval_fail(tval, "Banishment");
    sv_scroll_mass_banishment = borg_lookup_sval_fail(tval, "Mass Banishment");
    kv_scroll_mass_banishment
        = borg_lookup_kind(tval, sv_scroll_mass_banishment);
    sv_scroll_blessing       = borg_lookup_sval_fail(tval, "Blessing");
    sv_scroll_holy_chant     = borg_lookup_sval_fail(tval, "Holy Chant");
    sv_scroll_holy_prayer    = borg_lookup_sval_fail(tval, "Holy Prayer");
    sv_scroll_detect_invis   = borg_lookup_sval_fail(tval, "Detect Invisible");
    sv_scroll_satisfy_hunger = borg_lookup_sval_fail(tval, "Remove Hunger");
    sv_scroll_light          = borg_lookup_sval_fail(tval, "Light");
    sv_scroll_mapping        = borg_lookup_sval_fail(tval, "Magic Mapping");
    sv_scroll_acquirement    = borg_lookup_sval_fail(tval, "Acquirement");
    sv_scroll_star_acquirement = borg_lookup_sval_fail(tval, "*Acquirement*");
    sv_scroll_remove_curse     = borg_lookup_sval_fail(tval, "Remove Curse");
    kv_scroll_remove_curse     = borg_lookup_kind(tval, sv_scroll_remove_curse);
    sv_scroll_star_remove_curse = borg_lookup_sval_fail(tval, "*Remove Curse*");
    kv_scroll_star_remove_curse
        = borg_lookup_kind(tval, sv_scroll_star_remove_curse);
    sv_scroll_monster_confusion
        = borg_lookup_sval_fail(tval, "Monster Confusion");
    sv_scroll_trap_door_destruction
        = borg_lookup_sval_fail(tval, "Door Destruction");
    sv_scroll_dispel_undead = borg_lookup_sval_fail(tval, "Dispel Undead");

    tval                    = tval_find_idx("ring");
    sv_ring_flames          = borg_lookup_sval_fail(tval, "Flames");
    sv_ring_ice             = borg_lookup_sval_fail(tval, "Ice");
    sv_ring_acid            = borg_lookup_sval_fail(tval, "Acid");
    sv_ring_lightning       = borg_lookup_sval_fail(tval, "Lightning");
    sv_ring_digging         = borg_lookup_sval_fail(tval, "Digging");
    sv_ring_speed           = borg_lookup_sval_fail(tval, "Speed");
    sv_ring_damage          = borg_lookup_sval_fail(tval, "Damage");
    sv_ring_dog             = borg_lookup_sval_fail(tval, "the Dog");

    tval                    = tval_find_idx("amulet");
    sv_amulet_teleportation = borg_lookup_sval_fail(tval, "Teleportation");

    tval                    = tval_find_idx("rod");
    sv_rod_recall           = borg_lookup_sval_fail(tval, "Recall");
    kv_rod_recall           = borg_lookup_kind(tval, sv_rod_recall);
    sv_rod_detection        = borg_lookup_sval_fail(tval, "Detection");
    sv_rod_illumination     = borg_lookup_sval_fail(tval, "Illumination");
    sv_rod_speed            = borg_lookup_sval_fail(tval, "Speed");
    sv_rod_mapping          = borg_lookup_sval_fail(tval, "Magic Mapping");
    sv_rod_healing          = borg_lookup_sval_fail(tval, "Healing");
    kv_rod_healing          = borg_lookup_kind(tval, sv_rod_healing);
    sv_rod_light            = borg_lookup_sval_fail(tval, "Light");
    sv_rod_fire_bolt        = borg_lookup_sval_fail(tval, "Fire Bolts");
    sv_rod_elec_bolt        = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_rod_cold_bolt        = borg_lookup_sval_fail(tval, "Frost Bolts");
    sv_rod_acid_bolt        = borg_lookup_sval_fail(tval, "Acid Bolts");
    sv_rod_drain_life       = borg_lookup_sval_fail(tval, "Drain Life");
    sv_rod_fire_ball        = borg_lookup_sval_fail(tval, "Fire Balls");
    sv_rod_elec_ball        = borg_lookup_sval_fail(tval, "Lightning Balls");
    sv_rod_cold_ball        = borg_lookup_sval_fail(tval, "Cold Balls");
    sv_rod_acid_ball        = borg_lookup_sval_fail(tval, "Acid Balls");
    sv_rod_teleport_other   = borg_lookup_sval_fail(tval, "Teleport Other");
    sv_rod_slow_monster     = borg_lookup_sval_fail(tval, "Slow Monster");
    sv_rod_sleep_monster    = borg_lookup_sval_fail(tval, "Hold Monster");
    sv_rod_curing           = borg_lookup_sval_fail(tval, "Curing");

    tval                    = tval_find_idx("staff");
    sv_staff_teleportation  = borg_lookup_sval_fail(tval, "Teleportation");
    sv_staff_destruction    = borg_lookup_sval_fail(tval, "*Destruction*");
    sv_staff_speed          = borg_lookup_sval_fail(tval, "Speed");
    sv_staff_healing        = borg_lookup_sval_fail(tval, "Healing");
    sv_staff_the_magi       = borg_lookup_sval_fail(tval, "the Magi");
    sv_staff_power          = borg_lookup_sval_fail(tval, "Power");
    sv_staff_holiness       = borg_lookup_sval_fail(tval, "Holiness");
    kv_staff_holiness       = borg_lookup_kind(tval, sv_staff_holiness);
    sv_staff_curing         = borg_lookup_sval_fail(tval, "Curing");
    sv_staff_sleep_monsters = borg_lookup_sval_fail(tval, "Sleep Monsters");
    sv_staff_slow_monsters  = borg_lookup_sval_fail(tval, "Slow Monsters");
    sv_staff_detect_invis   = borg_lookup_sval_fail(tval, "Detect Invisible");
    sv_staff_detect_evil    = borg_lookup_sval_fail(tval, "Detect Evil");
    sv_staff_dispel_evil    = borg_lookup_sval_fail(tval, "Dispel Evil");
    sv_staff_banishment     = borg_lookup_sval_fail(tval, "Banishment");
    sv_staff_light          = borg_lookup_sval_fail(tval, "Light");
    sv_staff_mapping        = borg_lookup_sval_fail(tval, "Mapping");
    sv_staff_remove_curse   = borg_lookup_sval_fail(tval, "Remove Curse");

    tval                    = tval_find_idx("wand");
    sv_wand_light           = borg_lookup_sval_fail(tval, "Light");
    sv_wand_teleport_away   = borg_lookup_sval_fail(tval, "Teleport Other");
    sv_wand_stinking_cloud  = borg_lookup_sval_fail(tval, "Stinking Cloud");
    kv_wand_stinking_cloud  = borg_lookup_kind(tval, sv_wand_stinking_cloud);
    sv_wand_magic_missile   = borg_lookup_sval_fail(tval, "Magic Missile");
    kv_wand_magic_missile   = borg_lookup_kind(tval, sv_wand_magic_missile);
    sv_wand_annihilation    = borg_lookup_sval_fail(tval, "Annihilation");
    kv_wand_annihilation    = borg_lookup_kind(tval, sv_wand_annihilation);
    sv_wand_stone_to_mud    = borg_lookup_sval_fail(tval, "Stone to Mud");
    sv_wand_wonder          = borg_lookup_sval_fail(tval, "Wonder");
    sv_wand_hold_monster    = borg_lookup_sval_fail(tval, "Hold Monster");
    sv_wand_slow_monster    = borg_lookup_sval_fail(tval, "Slow Monster");
    sv_wand_fear_monster    = borg_lookup_sval_fail(tval, "Scare Monster");
    sv_wand_confuse_monster = borg_lookup_sval_fail(tval, "Scare Monster");
    sv_wand_fire_bolt       = borg_lookup_sval_fail(tval, "Fire Bolts");
    sv_wand_cold_bolt       = borg_lookup_sval_fail(tval, "Frost Bolts");
    sv_wand_acid_bolt       = borg_lookup_sval_fail(tval, "Acid Bolts");
    sv_wand_elec_bolt       = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_wand_fire_ball       = borg_lookup_sval_fail(tval, "Fire Balls");
    sv_wand_cold_ball       = borg_lookup_sval_fail(tval, "Cold Balls");
    sv_wand_acid_ball       = borg_lookup_sval_fail(tval, "Acid Balls");
    sv_wand_elec_ball       = borg_lookup_sval_fail(tval, "Lightning Bolts");
    sv_wand_dragon_cold     = borg_lookup_sval_fail(tval, "Dragon's Frost");
    sv_wand_dragon_fire     = borg_lookup_sval_fail(tval, "Dragon's Flame");
    sv_wand_drain_life      = borg_lookup_sval_fail(tval, "Drain Life");

    tval                    = tval_find_idx("sword");
    sv_dagger               = borg_lookup_sval_fail(tval, "Dagger");

    tval                    = tval_find_idx("bow");
    sv_sling                = borg_lookup_sval_fail(tval, "Sling");
    sv_short_bow            = borg_lookup_sval_fail(tval, "Short Bow");
    sv_long_bow             = borg_lookup_sval_fail(tval, "Long Bow");
    sv_light_xbow           = borg_lookup_sval_fail(tval, "Light Crossbow");
    sv_heavy_xbow           = borg_lookup_sval_fail(tval, "Heavy Crossbow");

    tval                    = tval_find_idx("arrow");
    sv_arrow_seeker         = borg_lookup_sval_fail(tval, "Seeker Arrow");
    sv_arrow_mithril        = borg_lookup_sval_fail(tval, "Mithril Arrow");

    tval                    = tval_find_idx("bolt");
    sv_bolt_seeker          = borg_lookup_sval_fail(tval, "Seeker Bolt");
    sv_bolt_mithril         = borg_lookup_sval_fail(tval, "Mithril Bolt");

    tval                    = tval_find_idx("gloves");
    sv_set_of_leather_gloves
        = borg_lookup_sval_fail(tval, "Set of Leather Gloves");

    tval            = tval_find_idx("cloak");
    sv_cloak        = borg_lookup_sval_fail(tval, "Cloak");

    tval            = tval_find_idx("soft armor");
    sv_robe         = borg_lookup_sval_fail(tval, "Robe");

    tval            = tval_find_idx("crown");
    sv_iron_crown   = borg_lookup_sval_fail(tval, "Iron Crown");

    tval            = tval_find_idx("dragon armor");
    sv_dragon_blue  = borg_lookup_sval_fail(tval, "Blue Dragon Scale Mail");
    sv_dragon_black = borg_lookup_sval_fail(tval, "Black Dragon Scale Mail");
    sv_dragon_white = borg_lookup_sval_fail(tval, "White Dragon Scale Mail");
    sv_dragon_red   = borg_lookup_sval_fail(tval, "Red Dragon Scale Mail");
    sv_dragon_green = borg_lookup_sval_fail(tval, "Green Dragon Scale Mail");
    sv_dragon_multihued
        = borg_lookup_sval_fail(tval, "Multi-Hued Dragon Scale Mail");
    sv_dragon_shining
        = borg_lookup_sval_fail(tval, "Shining Dragon Scale Mail");
    sv_dragon_law   = borg_lookup_sval_fail(tval, "Law Dragon Scale Mail");
    sv_dragon_gold  = borg_lookup_sval_fail(tval, "Gold Dragon Scale Mail");
    sv_dragon_chaos = borg_lookup_sval_fail(tval, "Chaos Dragon Scale Mail");
    sv_dragon_balance
        = borg_lookup_sval_fail(tval, "Balance Dragon Scale Mail");
    sv_dragon_power = borg_lookup_sval_fail(tval, "Power Dragon Scale Mail");
}

/**
 * Return the k_idx of the object kind with the given `tval` and `sval`, or 0.
 */
int borg_lookup_kind(int tval, int sval)
{
    int k;

    /* Look for it */
    for (k = 1; k < z_info->k_max; k++) {
        struct object_kind *k_ptr = &k_info[k];

        /* Found a match */
        if ((k_ptr->tval == tval) && (k_ptr->sval == sval))
            return (k);
    }

    /* Failure */
    msg("No object (%s,%d,%d)", tval_find_name(tval), tval, sval);
    return 0;
}

#endif
