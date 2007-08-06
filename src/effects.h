/*
 * File: effects.h
 * Purpose: List of effect types
 *
 * Copyright (c) 2007 Andrew Sidwell
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


/* Function */
#ifndef EFFECT
#ifndef INCLUDED_EFFECTS_H
#define INCLUDED_EFFECTS_H

bool do_effect(int effect, bool *ident, int dir, int beam);
bool effect_aim(int effect);
const char *effect_desc(int effect);

/* Types of effect */
typedef enum
{
    #define EFFECT(x, y, z)		EF_##x,
	#include "effects.h"

	EF_MAX
} effect_type;

#undef EFFECT

#endif /* INCLUDED_EFFECTS_H */

#else

	/*     name            aim?   short description */

	EFFECT(XXX,            FALSE, NULL)
	EFFECT(POISON,         FALSE, "poisons you for 2d7+10 turns")
	EFFECT(BLIND,          FALSE, "blinds you for 4d25+75 turns")
	EFFECT(SCARE,          FALSE, "induces fear in you for 1d10+10 turns")
	EFFECT(CONFUSE,        FALSE, "confuses you for 4d5+10 turns")
	EFFECT(HALLUC,         FALSE, "causes you to hallucinate")
	EFFECT(PARALYZE,       FALSE, "induces paralysis for 1d5+5 turns")
	EFFECT(SLOW,           FALSE, "slows you for 1d25+15 turns")

	EFFECT(CURE_POISON,    FALSE, "neutralizes poison")
	EFFECT(CURE_BLINDNESS, FALSE, "cures blindness")
	EFFECT(CURE_PARANOIA,  FALSE, "removes your fear")
	EFFECT(CURE_CONFUSION, FALSE, "cures confusion")
	EFFECT(CURE_MIND,      FALSE, "cures confusion and removes fear")
	EFFECT(CURE_BODY,      FALSE, "heals cut damage, and cures stunning, poison and blindness")

	EFFECT(CURE_LIGHT,     FALSE, "heals you a little (5% or 12HP), and heals some cut damage and cures blindness")
	EFFECT(CURE_SERIOUS,   FALSE, "heals you a little (15% or 21HP), heals some cut damage and cures blindness and confusion")
	EFFECT(CURE_CRITICAL,  FALSE, "heals you a little (30% or 30HP), heals cut damage, and cures poison, blindness, and confusion")
	EFFECT(CURE_FULL,      FALSE, "restores 300 hit points, heals cut damage, and cures stunning, poison, blindness, and confusion")
	EFFECT(CURE_FULL2,     FALSE, "restores 1200 hit points, heals cut damage, and cures stunning, poison, blindness, and confusion")
	EFFECT(CURE_NONORLYBIG,FALSE, "restores 5000 hit points, restores experience and stats, heals cut damage, and cures stunning, poison, blindness, and confusion")
	EFFECT(CURE_TEMP,      FALSE, "heals cut damage, and cures all stunning, poison, blindness and confusion")
	EFFECT(HEAL1,          FALSE, "heals 500 hit points")
	EFFECT(HEAL2,          FALSE, "heals 1000 hit points")
	EFFECT(HEAL3,          FALSE, "heals 500 hit points, heals cut damage, and cures stunning")

	EFFECT(GAIN_EXP,       FALSE, "grants either 100000 experience points or half the current experience point total plus 10, whichever is less")
	EFFECT(LOSE_EXP,       FALSE, "drains a quarter of your experience")
	EFFECT(RESTORE_EXP,    FALSE, "restores your experience")

	EFFECT(RESTORE_MANA,   FALSE, "restores your mana points to maximum")

	EFFECT(GAIN_STR,       FALSE, "restores and increases your strength")
	EFFECT(GAIN_INT,       FALSE, "restores and increases your intelligence")
	EFFECT(GAIN_WIS,       FALSE, "restores and increases your wisdom")
	EFFECT(GAIN_DEX,       FALSE, "restores and increases your dexterity")
	EFFECT(GAIN_CON,       FALSE, "restores and increases your constitution")
	EFFECT(GAIN_CHR,       FALSE, "restores and increases your charisma")
	EFFECT(GAIN_ALL,       FALSE, "restores and increases all your stats")
	EFFECT(BRAWN,          FALSE, "raises your strength at the expense of your intelligence")
	EFFECT(INTELLECT,      FALSE, "raises your intelligence at the expense of your constitution")
	EFFECT(CONTEMPLATION,  FALSE, "raises your wisdom at the expense of your dexterity")
	EFFECT(TOUGHNESS,      FALSE, "raises your constitution at the expense of your charisma")
	EFFECT(NIMBLENESS,     FALSE, "raises your dexterity at the expense of your strength")
	EFFECT(PLEASING,       FALSE, "raises your charisma at the expense of your wisdom")
	EFFECT(LOSE_STR,       FALSE, "reduces your strength with damage 5d5")
	EFFECT(LOSE_INT,       FALSE, "reduces your intelligence with damage 5d5")
	EFFECT(LOSE_WIS,       FALSE, "reduces your wisdom with damage 5d5")
	EFFECT(LOSE_DEX,       FALSE, "reduces your dexterity with damage 5d5")
	EFFECT(LOSE_CON,       FALSE, "reduces your constitution with damage 5d5")
	EFFECT(LOSE_CHR,       FALSE, "reduces your intelligence with damage 5d5")
	EFFECT(LOSE_CON2,      FALSE, "reduces your constitution with damage 10d10")
	EFFECT(RESTORE_STR,    FALSE, "restores your strength")
	EFFECT(RESTORE_INT,    FALSE, "restores your intelligence")
	EFFECT(RESTORE_WIS,    FALSE, "restores your wisdom")
	EFFECT(RESTORE_DEX,    FALSE, "restores your dexterity")
	EFFECT(RESTORE_CON,    FALSE, "restores your constitution")
	EFFECT(RESTORE_CHR,    FALSE, "restores your charisma")
	EFFECT(RESTORE_ALL,    FALSE, "restores all your stats")

	EFFECT(RESTORE_ST_LEV, FALSE, "restores all your stats and your experience points")

	EFFECT(TMD_INFRA,      FALSE, "extends your infravision by 50 feet for 4d25+100 turns")
	EFFECT(TMD_SINVIS,     FALSE, "cures blindness and allows you to see invisible things for 2d6+12 turns")
	EFFECT(TMD_ESP,        FALSE, "cures blindness and gives you telepathy for 6d6+12 turns")

	EFFECT(ENLIGHTENMENT,  FALSE, "completely lights up and magically maps the level")
	EFFECT(ENLIGHTENMENT2, FALSE, "increases your intelligence and wisdom, detects and maps everything in the surrounding area, and identifies all items in your pack")
	EFFECT(SELF_KNOW,      FALSE, "grants knowledge of all special powers that you currently possess")

	EFFECT(HERO,           FALSE, "restores 10 hit points, removes fear and grants you resistance to fear and +12 to-hit for 1d25+25 turns")
	EFFECT(SHERO,          FALSE, "restores 30 hit points, removes fear and grants you resistance to fear, +12 to-hit, and -10AC for 1d25+25 turns")

	EFFECT(RESIST_FIRE,    FALSE, "grants you temporary resistance to fire for 1d10+10 turns")
	EFFECT(RESIST_COLD,    FALSE, "grants you temporary resistance to cold for 1d10+10 turns")
	EFFECT(RESIST_ALL,     FALSE, "grants resistance to everything!!!")

	EFFECT(DET_GOLD,       FALSE, "detects all treasure in the surrounding area")
	EFFECT(DET_OBJ,        FALSE, "detects all objects in the surrounding area")
	EFFECT(DET_TRAP,       FALSE, "detects all traps in the surrounding area")
	EFFECT(DET_DOORSTAIR,  FALSE, "detects all doors and stairs in the surrounding area")
	EFFECT(DET_INVIS,      FALSE, "detects all invisible creatures in the surrounding area")
	EFFECT(DET_EVIL,       FALSE, "detects all evil creatures in the immidiate area")
	EFFECT(DET_ALL,        FALSE, "detects treasure, objects, traps, doors, stairs, and all creatures in the surrounding area")

	EFFECT(ENCHANT_TOHIT,  FALSE, "attempts to magically enhance a weapon's to-hit bonus")
	EFFECT(ENCHANT_TODAM,  FALSE, "attempts to magically enhance a weapon's to-dam bonus")
	EFFECT(ENCHANT_WEAPON, FALSE, "attempts to magically enhance a weapon both to-hit and to-dam")
	EFFECT(ENCHANT_ARMOR,  FALSE, "attempts to magically enhance a piece of armour")
	EFFECT(ENCHANT_ARMOR2, FALSE, "attempts to magically enhance a piece of armour with high chance of success")
	EFFECT(IDENTIFY,       FALSE, "reveals all unhidden powers of an object")
	EFFECT(IDENTIFY2,      FALSE, "reveals all powers of an object")
	EFFECT(REMOVE_CURSE,   FALSE, "removes all ordinary curses from all equipped items")
	EFFECT(REMOVE_CURSE2,  FALSE, "removes all curses from all equipped items")
	EFFECT(LIGHT,          FALSE, "lights up an area and inflicts 2d8 damage on light-sensitive creatures")
	EFFECT(SUMMON_MON,     FALSE, "summons monsters at the current dungeon level")
	EFFECT(SUMMON_UNDEAD,  FALSE, "summons undead monsters at the current dungeon level")
	EFFECT(TELE_PHASE,     FALSE, "teleports you randomly up to 10 squares away")
	EFFECT(TELE_LONG,      FALSE, "teleports you randomly up to 100 squares away")
	EFFECT(TELE_LEVEL,     FALSE, "teleports you one level up or down")
	EFFECT(CONFUSING,      FALSE, "causes your next attack upon a monster to confuse it")
	EFFECT(MAPPING,        FALSE, "maps the area around you in a 30-radius circle")
	EFFECT(RUNE,           FALSE, "inscribes a glyph of warding beneath you, which monsters cannot move onto")

	EFFECT(ACQUIRE,        FALSE, "creates a good object nearby")
	EFFECT(ACQUIRE2,       FALSE, "creates a few good items nearby")
	EFFECT(ANNOY_MON,      FALSE, "awakens all nearby sleeping monsters and hastens all monsters within line of sight")
	EFFECT(CREATE_TRAP,    FALSE, "creates traps surrounding you")
	EFFECT(DESTROY_TDOORS, FALSE, "destroys all traps and doors surrounding you")
	EFFECT(RECHARGE,       FALSE, "tries to recharge a wand or staff, destroying the wand or staff on failure")
	EFFECT(BANISHMENT,     FALSE, "removes all non-unique monsters represented by a chosen symbol from the level, dealing you damage in the process")
	EFFECT(DARKNESS,       FALSE, "darkens the nearby area and blinds you for 1d5+3 turns")
	EFFECT(PROTEVIL,       FALSE, "grants you protection from evil for 1d25 plus 3 times your character level turns")
	EFFECT(SATISFY,        FALSE, "magically renders you well-fed, curing any gastrointestinal problems")
	EFFECT(CURSE_WEAPON,   FALSE, "curses your currently wielded melee weapon")
	EFFECT(CURSE_ARMOR,    FALSE, "curses your currently worn body armor")
	EFFECT(BLESSING,       FALSE, "increases your AC and to-hit bonus for 1d12+6 turns")
	EFFECT(BLESSING2,      FALSE, "increases your AC and to-hit bonus for 1d24+12 turns")
	EFFECT(BLESSING3,      FALSE, "increases your AC and to-hit bonus for 1d48+24 turns")
	EFFECT(RECALL,         FALSE, "returns you from the dungeon or takes you to the dungeon after a short delay")

	EFFECT(EARTHQUAKES,    FALSE, "causes an earthquake around you")
	EFFECT(DESTRUCTION2,   FALSE, "destroys an area around you in the shape of a circle radius 15, and blinds you for 1d10+10 turns")

	EFFECT(LOSHASTE,       FALSE, "hastes all monsters within line of sight")
	EFFECT(LOSSLOW,        FALSE, "slows all non-unique monsters within line of sight")
	EFFECT(LOSSLEEP,       FALSE, "sleeps all non-unique creatures within line of sight")
	EFFECT(LOSKILL,        FALSE, "removes all non-unique monsters within 20 squares, dealing you damage in the process")

	EFFECT(ILLUMINATION,   FALSE, "!")
	EFFECT(CLAIRVOYANCE,   FALSE, "!")
	EFFECT(PROBING,        FALSE, "gives you information on the health and abilities of monsters you can see")

	EFFECT(HASTE,          FALSE, "hastens you for 2d10+20 turns")
	EFFECT(HASTE1,         FALSE, "hastens you for d20+20 turns")
	EFFECT(HASTE2,         FALSE, "hastens you for d75+75 turns")

	EFFECT(DISPEL_EVIL,    FALSE, "deals five times your level's damage to all evil creatures that you can see")
	EFFECT(DISPEL_EVIL60,  FALSE, "deals 60 damage to all evil creatures that you can see")
	EFFECT(DISPEL_UNDEAD,  FALSE, "deals 60 damge to all undead creatures that you can see")
	EFFECT(DISPEL_ALL,     FALSE, "deals 120 damage to all creatures that you can see")

	EFFECT(SLEEPII,        FALSE, "puts to sleep the monsters around you")
	EFFECT(STAR_BALL,      FALSE, "fires a ball of electricity in all directions, each one causing 150 damage")
	EFFECT(RAGE_BLESS_RESIST, FALSE, "beserk rage, bless, and resistance")
	EFFECT(RESTORE_LIFE,   FALSE, "restores your experience to full")
	EFFECT(REM_FEAR_POIS,  FALSE, "cures you of fear and poison")
	EFFECT(PROBE,          FALSE, "!probes, somewhat ominously")
	EFFECT(FIREBRAND,      FALSE, "brands bolts with fire, in an unbalanced fashion")

	EFFECT(FIRE_BOLT,      TRUE,  "creates a fire bolt with damage 9d8")
	EFFECT(FIRE_BOLT2,     TRUE,  "creates a fire bolt with damage 12d8")
	EFFECT(FIRE_BOLT3,     TRUE,  "creates a fire bolt with damage 16d8")
	EFFECT(FIRE_BOLT72,    TRUE,  "creates a fire bolt with damage 72")
	EFFECT(FIRE_BALL,      TRUE,  "creates a fire ball with damage 144")
	EFFECT(FIRE_BALL2,     TRUE,  "creates a large fire ball with damage 120")
	EFFECT(FIRE_BALL200,   TRUE,  "creates a large fire ball with damage 200")
	EFFECT(COLD_BOLT,      TRUE,  "creates a frost bolt with damage 6d8")
	EFFECT(COLD_BOLT2,     TRUE,  "creates a frost bolt with damage 12d8")
	EFFECT(COLD_BALL2,     TRUE,  "creates a large frost ball with damage 200")
	EFFECT(COLD_BALL50,    TRUE,  "creates a frost ball with damage 50")
	EFFECT(COLD_BALL100,   TRUE,  "creates a frost ball with damage 100")
	EFFECT(COLD_BALL160,   TRUE,  "creates a frost ball with damage 160")
	EFFECT(ACID_BOLT,      TRUE,  "creates an acid bolt with damage 5d8")
	EFFECT(ACID_BOLT2,     TRUE,  "creates an acid bolt with damage 10d8")
	EFFECT(ACID_BOLT3,     TRUE,  "creates an acid bolt with damage 12d8")
	EFFECT(ACID_BALL,      TRUE,  "creates an acid ball with damage 125")
	EFFECT(ELEC_BOLT,      TRUE,  "creates a lightning bolt with damage 6d6")
	EFFECT(ELEC_BALL,      TRUE,  "creates a lightning ball with damage 64")
	EFFECT(ELEC_BALL2,     TRUE,  "creates a large lightning ball with damage 250")

	EFFECT(DRAIN_LIFE1,    TRUE,  "drains up to 90 hit points of life from a target creature")
	EFFECT(DRAIN_LIFE2,    TRUE,  "drains up to 120 hit points of life from a target creature")
	EFFECT(DRAIN_LIFE3,    TRUE,  "drains up to 150 hit points of life from a target creature")
	EFFECT(DRAIN_LIFE4,    TRUE,  "drains up to 250 hit points of life from a target creature")
	EFFECT(MISSILE,        TRUE,  "fires a magic missile with damage 3d4")
	EFFECT(MANA_BOLT,      TRUE,  "fires a mana bolt with damage 12d8")
	EFFECT(BIZARRE,        TRUE,  "does bizarre things.")
	EFFECT(ARROW,          TRUE,  "fires a magical arrow with damage 150")
	EFFECT(STINKING_CLOUD, TRUE,  "fires a stinking cloud with damage 12")
	EFFECT(STONE_TO_MUD,   TRUE,  "turns rock into mud")
	EFFECT(TELE_OTHER,     TRUE,  "teleports a target monster away")
	EFFECT(CONFUSE2,       TRUE,  "confuses a target monster")

	EFFECT(MON_HEAL,       TRUE,  "heals a single monster 4d6 hit points")
	EFFECT(MON_HASTE,      TRUE,  "hastes a single monster")
	EFFECT(MON_SLOW,       TRUE,  "attempts to magically slow a single monster")
	EFFECT(MON_CONFUSE,    TRUE,  "attempts to magically confuse a single monster")
	EFFECT(MON_SLEEP,      TRUE,  "attempts to induce magical sleep in a single monster")
	EFFECT(MON_CLONE,      TRUE,  "hastes, heals, and magically duplicates a single monster")
	EFFECT(MON_SCARE,      TRUE,  "attempts to induce magical fear in a single monster")

	EFFECT(LIGHT_LINE,     TRUE,  "lights up part of the dungeon in a straight line")
	EFFECT(DISARMING,      TRUE,  "destroys traps, unlocks doors and reveals all secret doors in a given direction")
	EFFECT(TDOOR_DEST,     TRUE,  "destroys traps and doors")
	EFFECT(POLYMORPH,      TRUE,  "polymorphs a monster into another kind of creature")

	EFFECT(STARLIGHT,      FALSE, "fires a line of light in all directions, each one causing light-sensitive creatures 6d8 damage")
	EFFECT(STARLIGHT2,     FALSE, "fires a line of light in all directions, each one causing 10d8 damage")
	EFFECT(BERSERKER,      FALSE, "puts you in a berserker rage for d50+50 turns")

	EFFECT(WONDER,         TRUE,  "wonderous things")

	EFFECT(WAND_BREATH,    TRUE,  "shoots a large ball of one of the base elements for 120-200 damage")
	EFFECT(STAFF_MAGI,     FALSE, "restores both intelligence and manapoints to maximum")
	EFFECT(STAFF_HOLY,     FALSE, "inflicts damage on evil creatures you can see, cures 50 hit points, heals all temporary effects and grants you protection from evil")
	EFFECT(DRINK_GOOD,     FALSE, NULL)
	EFFECT(DRINK_SALT,     FALSE, "induces vomiting and paralysis for 4 turns, resulting in severe hunger but also curing poison")
	EFFECT(DRINK_DEATH,    FALSE, "inflicts 5000 points of damage")
	EFFECT(DRINK_RUIN,     FALSE, "inflicts 10d10 points of damage and decreases all your stats")
	EFFECT(DRINK_DETONATE, FALSE, "inflicts 50d20 points of damage, severe cuts, and stunning")
	EFFECT(FOOD_GOOD,      FALSE, NULL)
	EFFECT(FOOD_WAYBREAD,  FALSE, "restores 4d8 hit points and neutralizes poison")

	EFFECT(RING_ACID,      TRUE,  "grants acid resistance for d20+20 turns and creates an acid ball of damage 70")
	EFFECT(RING_FLAMES,    TRUE,  "grants fire resistance for d20+20 turns and creates an fire ball of damage 80")
	EFFECT(RING_ICE,       TRUE,  "grants cold resistance for d20+20 turns and creates an cold ball of damage 75")
	EFFECT(RING_LIGHTNING, TRUE,  "grants electricity resistance for d20+20 turns and creates an lightning ball of damage 85")

	EFFECT(DRAGON_BLUE,    TRUE, "allows you to breathe lightning for 100 damage")
	EFFECT(DRAGON_GREEN,   TRUE, "allows you to breathe poison gas for 150 damage")
	EFFECT(DRAGON_RED,     TRUE, "allows you to breathe fire for 200 damage")
	EFFECT(DRAGON_MULTIHUED, TRUE, "allows you to breathe the elements for 250 damage")
	EFFECT(DRAGON_BRONZE,  TRUE, "allows you to breathe confusion for 120 damage")
	EFFECT(DRAGON_GOLD,    TRUE, "allows you to breathe sound for 130 damage")
	EFFECT(DRAGON_CHAOS,   TRUE, "allows you to breathe chaos or disenchantment for 220 damage")
	EFFECT(DRAGON_LAW,     TRUE, "allows you to breathe sound/shards for 230 damage")
	EFFECT(DRAGON_BALANCE, TRUE, "allows you to breathe balance for 250 damage")
	EFFECT(DRAGON_SHINING, TRUE, "allows you to breathe light or darkness for 200 damage")
	EFFECT(DRAGON_POWER,   TRUE, "allows you to breathe for 300 damage")

#endif /* EFFECT */
