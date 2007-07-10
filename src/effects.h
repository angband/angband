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
bool do_effect(object_type *o_ptr, bool *ident, int dir);
bool effect_aim(int effect);
const char *effect_desc(int effect);


/*
 * If we're being included in the init1.c file, change definitions to avoid
 * needless duplication of lists...
 */
#if defined(LIST_STRINGS)
# define START				static const char *effect_list[] = {
# define EFFECT(x, y, z)	#x,
# define END				};
#elif defined(MAKE_TABLE)
# define START				static const info_entry effects[] = {
# define EFFECT(x, y, z)	{ EF_##x, y, z },
# define END				};
#else
# define START				typedef enum {
# define EFFECT(x, y, z)	EF_##x,
# define END				EFFECT(MAX, FALSE, NULL) } effect_type;
#endif

START
	EFFECT(XXX,            FALSE, NULL)
	EFFECT(POISON,         FALSE, "poisons you for 1d10+10 turns")
	EFFECT(BLIND,          FALSE, "blinds you for 1d200+200 turns")
	EFFECT(SCARE,          FALSE, "induces fear in you for 1d10+10 turns")
	EFFECT(CONFUSE,        FALSE, "confuses you for ")
	EFFECT(HALLUC,         FALSE, "causes you to hallucinate")
	EFFECT(PARALYZE,       FALSE, "induces paralysis for 1d10+10 turns")
	EFFECT(LOSE_STR,       FALSE, "reduces your strength with damage 6d6")
	EFFECT(LOSE_STR2,      FALSE, "reduces your strength with damage 10d10")
	EFFECT(LOSE_CON,       FALSE, "reduces your constitution with damage 6d6")
	EFFECT(LOSE_CON2,      FALSE, "reduces your constitution with damage 10d10")
	EFFECT(LOSE_INT,       FALSE, "reduces your intelligence with damage 8d8")
	EFFECT(LOSE_WIS,       FALSE, "reduces your wisdom with damage 8d8")
	EFFECT(CURE_POISON,    FALSE, "neutralizes poison")
	EFFECT(CURE_BLINDNESS, FALSE, "cures blindness")
	EFFECT(CURE_PARANOIA,  FALSE, "removes your fear")
	EFFECT(CURE_CONFUSION, FALSE, "cures confusion")
	EFFECT(CW_SERIOUS,     FALSE, "restores 4d8 hit points")
	EFFECT(RESTORE_STR,    FALSE, "restores your strength")
	EFFECT(RESTORE_CON,    FALSE, "restores your constitution")
	EFFECT(RESTORE_ALL,    FALSE, "restores all your stats")

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
	EFFECT(DET_GOLD,       FALSE, "detects all treasure in the surrounding area")
	EFFECT(DET_OBJ,        FALSE, "detects all objects in the surrounding area")
	EFFECT(DET_TRAP,       FALSE, "detects all traps in the surrounding area")
	EFFECT(DET_DOORSTAIR,  FALSE, "detects all doors and stairs in the surrounding area")
	EFFECT(DET_INVIS,      FALSE, "detects all invisible monsters in the surrounding area")
	EFFECT(ACQUIRE,        FALSE, "creates a good object nearby")
	EFFECT(ACQUIRE2,       FALSE, "creates a few good items nearby")
	EFFECT(LOSKILL,        FALSE, "removes all non-unique monsters within 20 squares, dealing you damage in the process")
	EFFECT(ANNOY_MON,      FALSE, "awakens all nearby sleeping monsters and hastens all monsters within line of sight")
	EFFECT(CREATE_TRAP,    FALSE, "creates traps surrounding you")
	EFFECT(DESTROY_TDOORS, FALSE, "destroys all traps and doors surrounding you")
	EFFECT(RECHARGE,       FALSE, "tries to recharge a wand or staff, destroying the wand or staff on failure")
	EFFECT(BANISHMENT,     FALSE, "removes all non-unique monsters represented by a chosen symbol from the level, dealing you damage in the process")
	EFFECT(DARKNESS,       FALSE, "darkens the nearby area and blinds you for 1d5+3 turns")
	EFFECT(PROTEVIL,       FALSE, "grants you protection from evil for 1d25 plus 3 times your character level turns")
	EFFECT(SATISFY,        FALSE, "magically renders you well-fed, curing any gastrointestinal problems")
	EFFECT(DISPEL_UNDEAD,  FALSE, "deals 60 damge to all undead creatures that you can see")
	EFFECT(CURSE_WEAPON,   FALSE, "curses your currently wielded melee weapon")
	EFFECT(CURSE_ARMOR,    FALSE, "curses your currently worn body armor")
	EFFECT(BLESSING,       FALSE, "increases your AC and to-hit bonus for 1d12+6 turns")
	EFFECT(BLESSING2,      FALSE, "increases your AC and to-hit bonus for 1d24+12 turns")
	EFFECT(BLESSING3,      FALSE, "increases your AC and to-hit bonus for 1d48+24 turns")
	EFFECT(RECALL,         FALSE, "returns you from the dungeon or takes you to the dungeon after a short delay")
	EFFECT(DESTRUCTION2,   FALSE, "destroys an area around you in the shape of a circle radius 15, and blinds you for 1d10+10 turns")

	EFFECT(FOOD_GOOD,      FALSE, NULL)
	EFFECT(FOOD_WAYBREAD,  FALSE, "restores 4d8 hit points and neutralizes poison")	
	EFFECT(RING_ACID,      TRUE,  "grants acid resistance for d20+20 turns and creates an acid ball of damage 70")
	EFFECT(RING_FLAMES,    TRUE,  "grants fire resistance for d20+20 turns and creates an fire ball of damage 80")
	EFFECT(RING_ICE,       TRUE,  "grants cold resistance for d20+20 turns and creates an cold ball of damage 75")
	EFFECT(RING_LIGHTNING, TRUE,  "grants electricity resistance for d20+20 turns and creates an lightning ball of damage 85")
END

#undef START
#undef EFFECT
#undef END
