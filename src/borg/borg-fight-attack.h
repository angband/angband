/**
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

#ifndef INCLUDED_BORG_FIGHT_ATTACK_H
#define INCLUDED_BORG_FIGHT_ATTACK_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-magic.h"

/*
 * All of the various attacks or projections the borg can do.
 !FIX !TODO !AJG probably want to externalize this somehow.
 */
enum {
    BORG_ATTACK_MISSILE,
    BORG_ATTACK_ARROW,
    BORG_ATTACK_MANA,
    BORG_ATTACK_METEOR,
    BORG_ATTACK_ACID,
    BORG_ATTACK_ELEC,
    BORG_ATTACK_FIRE,
    BORG_ATTACK_COLD,
    BORG_ATTACK_POIS,
    BORG_ATTACK_ICE,
    BORG_ATTACK_HOLY_ORB,
    BORG_ATTACK_DISP_UNDEAD,
    BORG_ATTACK_DISP_EVIL,
    BORG_ATTACK_DISP_SPIRITS,
    BORG_ATTACK_SLEEP_EVIL,
    BORG_ATTACK_HOLY_WORD,
    BORG_ATTACK_LIGHT_WEAK,
    BORG_ATTACK_OLD_DRAIN,
    BORG_ATTACK_KILL_WALL,
    BORG_ATTACK_NETHER,
    BORG_ATTACK_CHAOS,
    BORG_ATTACK_GRAVITY,
    BORG_ATTACK_SHARD,
    BORG_ATTACK_SOUND,
    BORG_ATTACK_PLASMA,
    BORG_ATTACK_CONFU,
    BORG_ATTACK_DISEN,
    BORG_ATTACK_NEXUS,
    BORG_ATTACK_FORCE,
    BORG_ATTACK_INERTIA,
    BORG_ATTACK_TIME,
    BORG_ATTACK_LIGHT,
    BORG_ATTACK_DARK,
    BORG_ATTACK_WATER,
    BORG_ATTACK_OLD_HEAL,
    BORG_ATTACK_OLD_CLONE,
    BORG_ATTACK_OLD_SPEED,
    BORG_ATTACK_DARK_WEAK,
    BORG_ATTACK_KILL_DOOR,
    BORG_ATTACK_KILL_TRAP,
    BORG_ATTACK_MAKE_WALL,
    BORG_ATTACK_MAKE_DOOR,
    BORG_ATTACK_MAKE_TRAP,
    BORG_ATTACK_AWAY_UNDEAD,
    BORG_ATTACK_TURN_EVIL,
    BORG_ATTACK_AWAY_ALL,
    BORG_ATTACK_AWAY_ALL_MORGOTH,
    BORG_ATTACK_DISP_ALL,
    BORG_ATTACK_OLD_CONF,
    BORG_ATTACK_TURN_ALL,
    BORG_ATTACK_OLD_SLOW,
    BORG_ATTACK_OLD_SLEEP,
    BORG_ATTACK_OLD_POLY,
    BORG_ATTACK_TURN_UNDEAD,
    BORG_ATTACK_AWAY_EVIL,
    BORG_ATTACK_TAP_UNLIFE,
    BORG_ATTACK_DRAIN_LIFE,
};

/*
 * Method for handling attacks, missiles, and spells
 *
 * Every turn, we evaluate every known method of causing damage
 * to monsters, and evaluate the "reward" inherent in each of
 * the known methods which is usable at that time, and then
 * we actually use whichever method, if any, scores highest.
 *
 * For each attack, we need a function which will determine the best
 * possible result of using that attack, and return its value.  Also,
 * if requested, the function should actually perform the action.
 *
 * Note that the functions should return zero if the action is not
 * usable, or if the action is not useful.
 *
 * These functions need to apply some form of "cost" evaluation, to
 * prevent the use of expensive spells with minimal reward.  Also,
 * we should always prefer attacking by hand to using spells if the
 * damage difference is "small", since there is no "cost" in making
 * a physical attack.
 *
 * We should take account of "spell failure", as well as "missile
 * missing" and "blow missing" probabilities.
 *
 * Note that the functions may store local state information when
 * doing a "simulation" and then they can use this information if
 * they are asked to implement their strategy.
 *
 * There are several types of damage inducers:
 *
 *   Attacking physically
 *   Launching missiles
 *   Throwing objects
 *   Casting spells
 *   Praying prayers
 *   Using wands
 *   Using rods
 *   Using staffs
 *   Using scrolls
 *   Activating Artifacts
 *   Activate Dragon Armour
 */
enum {
    BF_REST,
    BF_THRUST,
    BF_OBJECT,
    BF_LAUNCH,
    BF_SPELL_MAGIC_MISSILE,
    BF_SPELL_MAGIC_MISSILE_RESERVE, /* 20 */
    BF_SPELL_STINK_CLOUD,
    BF_SPELL_LIGHT_BEAM,
    BF_SPELL_COLD_BOLT,
    BF_SPELL_STONE_TO_MUD,
    BF_SPELL_SLOW_MONSTER,
    BF_SPELL_SLEEP_III,
    BF_SPELL_FIRE_BALL,
    BF_SPELL_SHOCK_WAVE,
    BF_SPELL_EXPLOSION,
    BF_SPELL_CONFUSE_MONSTER,
    BF_SPELL_COLD_STORM,
    BF_SPELL_METEOR_SWARM,
    BF_SPELL_RIFT, /* 40 */
    BF_SPELL_MANA_STORM,
    BF_SPELL_BLIND_CREATURE,
    BF_SPELL_TRANCE,
    BF_PRAYER_HOLY_ORB_BALL,
    BF_PRAYER_DISP_UNDEAD,
    BF_PRAYER_DISP_EVIL,
    BF_PRAYER_DISP_SPIRITS,
    BF_PRAYER_HOLY_WORD, /* 50 */
    BF_SPELL_ANNIHILATE,

    BF_SPELL_ELECTRIC_ARC, /* new spells in 4.x */
    BF_SPELL_ACID_SPRAY,
    BF_SPELL_MANA_BOLT,
    BF_SPELL_THRUST_AWAY,
    BF_SPELL_LIGHTNING_STRIKE,
    BF_SPELL_EARTH_RISING,
    BF_SPELL_VOLCANIC_ERUPTION,
    BF_SPELL_RIVER_OF_LIGHTNING,
    BF_SPELL_SPEAR_OF_OROME,
    BF_SPELL_LIGHT_OF_MANWE,
    BF_SPELL_NETHER_BOLT,
    BF_SPELL_TAP_UNLIFE,
    BF_SPELL_CRUSH,
    BF_SPELL_SLEEP_EVIL,
    BF_SPELL_DISENCHANT,
    BF_SPELL_FRIGHTEN,
    BF_SPELL_VAMPIRE_STRIKE,
    BF_PRAYER_DISPEL_LIFE,
    BF_SPELL_DARK_SPEAR,
    BF_SPELL_UNLEASH_CHAOS,
    BF_SPELL_STORM_OF_DARKNESS,
    BF_SPELL_CURSE,
    BF_SPELL_WHIRLWIND_ATTACK,
    BF_SPELL_LEAP_INTO_BATTLE,
    BF_SPELL_MAIM_FOE,
    BF_SPELL_HOWL_OF_THE_DAMNED,

    BF_ROD_ELEC_BOLT,
    BF_ROD_COLD_BOLT,
    BF_ROD_ACID_BOLT,
    BF_ROD_FIRE_BOLT,
    BF_ROD_LIGHT_BEAM,
    BF_ROD_DRAIN_LIFE,
    BF_ROD_ELEC_BALL, /* 60 */
    BF_ROD_COLD_BALL,
    BF_ROD_ACID_BALL,
    BF_ROD_FIRE_BALL,
    BF_ROD_SLOW_MONSTER,
    BF_ROD_SLEEP_MONSTER,
    BF_ROD_UNKNOWN,
    BF_STAFF_SLEEP_MONSTERS,
    BF_STAFF_SLOW_MONSTERS,
    BF_STAFF_DISPEL_EVIL,
    BF_STAFF_POWER,
    BF_STAFF_HOLINESS,
    BF_WAND_UNKNOWN,
    BF_WAND_MAGIC_MISSILE,
    BF_WAND_ELEC_BOLT,
    BF_WAND_COLD_BOLT,
    BF_WAND_ACID_BOLT,
    BF_WAND_FIRE_BOLT,
    BF_WAND_SLOW_MONSTER,
    BF_WAND_HOLD_MONSTER,
    BF_WAND_CONFUSE_MONSTER,
    BF_WAND_FEAR_MONSTER,
    BF_WAND_ANNIHILATION,
    BF_WAND_DRAIN_LIFE,
    BF_WAND_LIGHT_BEAM,
    BF_WAND_STINKING_CLOUD,
    BF_WAND_ELEC_BALL,
    BF_WAND_COLD_BALL,
    BF_WAND_ACID_BALL,
    BF_WAND_FIRE_BALL,
    BF_WAND_WONDER,
    BF_WAND_DRAGON_COLD,
    BF_WAND_DRAGON_FIRE,

    BF_RING_ACID,
    BF_RING_FIRE,
    BF_RING_ICE,
    BF_RING_LIGHTNING,
    BF_DRAGON_BLUE,
    BF_DRAGON_WHITE,
    BF_DRAGON_BLACK,
    BF_DRAGON_GREEN,
    BF_DRAGON_RED,
    BF_DRAGON_MULTIHUED,
    BF_DRAGON_GOLD,
    BF_DRAGON_CHAOS,
    BF_DRAGON_LAW,
    BF_DRAGON_BALANCE,
    BF_DRAGON_SHINING,
    BF_DRAGON_POWER,

    BF_ACT_FIRE_BOLT,
    BF_ACT_FIRE_BOLT72,
    BF_ACT_FIRE_BALL,
    BF_ACT_COLD_BOLT,
    BF_ACT_COLD_BALL50,
    BF_ACT_COLD_BALL100,
    BF_ACT_COLD_BOLT2,
    BF_ACT_DRAIN_LIFE1,
    BF_ACT_DRAIN_LIFE2,
    BF_ACT_STINKING_CLOUD,
    BF_ACT_CONFUSE2,
    BF_ACT_ARROW,
    BF_ACT_MISSILE,
    BF_ACT_SLEEPII,
    BF_ACT_ELEC_BOLT,
    BF_ACT_ACID_BOLT,
    BF_ACT_DISPEL_EVIL,
    BF_ACT_MANA_BOLT,
    BF_ACT_STAR_BALL,
    BF_ACT_STARLIGHT2,

    BF_ACT_STARLIGHT,
    BF_ACT_MON_SLOW,
    BF_ACT_MON_CONFUSE,
    BF_ACT_SLEEP_ALL,
    BF_ACT_FEAR_MONSTER,
    BF_ACT_LIGHT_BEAM,
    BF_ACT_DRAIN_LIFE3,
    BF_ACT_DRAIN_LIFE4,
    BF_ACT_ELEC_BALL,
    BF_ACT_ELEC_BALL2,
    BF_ACT_ACID_BOLT2,
    BF_ACT_ACID_BOLT3,
    BF_ACT_ACID_BALL,
    BF_ACT_COLD_BALL160,
    BF_ACT_COLD_BALL2,
    BF_ACT_FIRE_BALL2,
    BF_ACT_FIRE_BALL200,
    BF_ACT_FIRE_BOLT2,
    BF_ACT_FIRE_BOLT3,
    BF_ACT_DISPEL_EVIL60,
    BF_ACT_DISPEL_UNDEAD,
    BF_ACT_DISPEL_ALL,
    BF_ACT_LOSSLOW,
    BF_ACT_LOSSLEEP,
    BF_ACT_LOSCONF,
    BF_ACT_WONDER,
    BF_ACT_STAFF_HOLY,
    BF_ACT_RING_ACID,
    BF_ACT_RING_FIRE,
    BF_ACT_RING_ICE,
    BF_ACT_RING_LIGHTNING,
    BF_ACT_DRAGON_BLUE,
    BF_ACT_DRAGON_GREEN,
    BF_ACT_DRAGON_RED,
    BF_ACT_DRAGON_MULTIHUED,
    BF_ACT_DRAGON_GOLD,
    BF_ACT_DRAGON_CHAOS,
    BF_ACT_DRAGON_LAW,
    BF_ACT_DRAGON_BALANCE,
    BF_ACT_DRAGON_SHINING,
    BF_ACT_DRAGON_POWER,

    BF_MAX
};

extern int successful_target;
extern int target_closest;

/*
 * Maintain a set of special grids used for Teleport Other
 */
extern int16_t borg_tp_other_n;
extern uint8_t borg_tp_other_x[255];
extern uint8_t borg_tp_other_y[255];
extern int     borg_tp_other_index[255];

/*
 * What effect does a blow from a monster have?
 *
 */
extern int borg_mon_blow_effect(const char *name);

/*
 * Simulate/Apply the optimal result of launching a beam/bolt/ball
 */
extern int borg_launch_bolt(
    int rad, int dam, int typ, int max, int ammo_location);

/*
 * Simulate/Apply the optimal result of launching a missile
 */
extern int borg_attack_aux_launch(void);

/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 */
extern int borg_attack_aux_spell_bolt(
    const enum borg_spells spell, int rad, int dam, int typ, int max_range, bool is_arc);

/*
 * Simulate/Apply the optimal result of using the given "type" of attack
 */
extern int borg_calculate_attack_effectiveness(int attack_type);

/*
 * Attack nearby monsters, in the best possible way, if any.
 */
extern bool borg_attack(bool boosted_bravery);

#endif
#endif
