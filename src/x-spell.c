/*
 * File: x-spell.c
 * Purpose: Spell effect definitions and information about them
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

#include "angband.h"
#include "cave.h"
#include "effects.h"
#include "object/tvalsval.h"
#include "spells.h"

/*
 * The defines below must match the spell numbers in spell.txt
 * if they don't, "interesting" things will probably happen.
 *
 * It would be nice if we could get rid of this dependency.
 */

enum spell_index_arcane_e {
	SPELL_MAGIC_MISSILE = 0,
	SPELL_DETECT_MONSTERS = 1,
	SPELL_PHASE_DOOR = 2,
	SPELL_LIGHT_AREA = 3,
	SPELL_FIND_TRAPS_DOORS = 4,
	SPELL_CURE_LIGHT_WOUNDS = 5,
	SPELL_OBJECT_DETECTION = 6,
	SPELL_XXX = 7,
	SPELL_IDENTIFY = 8,
	SPELL_DETECT_INVISIBLE = 9,
	SPELL_TREASURE_DETECTION = 10,
	SPELL_STINKING_CLOUD = 11,
	SPELL_LIGHTNING_BOLT = 12,
	SPELL_CONFUSE_MONSTER = 13,
	SPELL_SLEEP_MONSTER = 14,
	SPELL_WONDER = 15,
	SPELL_FROST_BOLT = 16,
	SPELL_ACID_BOLT = 17,
	SPELL_FIRE_BOLT = 18,
	SPELL_TRAP_DOOR_DESTRUCTION = 19,
	SPELL_SPEAR_OF_LIGHT = 20,
	SPELL_TURN_STONE_TO_MUD = 21,
	SPELL_DOOR_CREATION = 22,
	SPELL_EARTHQUAKE = 23,
	SPELL_STAIR_CREATION = 24,
	SPELL_CURE_POISON = 25,
	SPELL_SATISFY_HUNGER = 26,
	SPELL_HEROISM = 27,
	SPELL_BERSERKER = 28,
	SPELL_HASTE_SELF = 29,
	SPELL_TELEPORT_SELF = 30,
	SPELL_SLOW_MONSTER = 31,
	SPELL_TELEPORT_OTHER = 32,
	SPELL_TELEPORT_LEVEL = 33,
	SPELL_WORD_OF_RECALL = 34,
	SPELL_POLYMORPH_OTHER = 35,
	SPELL_SHOCK_WAVE = 36,
	SPELL_EXPLOSION = 37,
	SPELL_CLOUD_KILL = 38,
	SPELL_MASS_SLEEP = 39,
	SPELL_BEDLAM = 40,
	SPELL_REND_SOUL = 41,
	SPELL_WORD_OF_DESTRUCTION = 42,
	SPELL_CHAOS_STRIKE = 43,
	SPELL_RESIST_COLD = 44,
	SPELL_RESIST_FIRE = 45,
	SPELL_RESIST_POISON = 46,
	SPELL_RESISTANCE = 47,
	SPELL_SHIELD = 48,
	SPELL_RUNE_OF_PROTECTION = 49,
	SPELL_RECHARGE_ITEM_I = 50,
	SPELL_ENCHANT_ARMOR = 51,
	SPELL_ENCHANT_WEAPON = 52,
	SPELL_RECHARGE_ITEM_II = 53,
	SPELL_ELEMENTAL_BRAND = 54,
	SPELL_FROST_BALL = 55,
	SPELL_ACID_BALL = 56,
	SPELL_FIRE_BALL = 57,
	SPELL_ICE_STORM = 58,
	SPELL_BANISHMENT = 59,
	SPELL_METEOR_SWARM = 60,
	SPELL_MASS_BANISHMENT = 61,
	SPELL_RIFT = 62,
	SPELL_MANA_STORM = 63,

	SPELL_MAX,
};

enum spell_index_prayer_e {
	/* Beginners Handbook */
	PRAYER_DETECT_EVIL = 0,
	PRAYER_CURE_LIGHT_WOUNDS = 1,
	PRAYER_BLESS = 2,
	PRAYER_REMOVE_FEAR = 3,
	PRAYER_CALL_LIGHT = 4,
	PRAYER_FIND_TRAPS_DOORS = 5,
	PRAYER_XXX = 6,
	PRAYER_SLOW_POISON = 7,

	/* Words of Wisdom */
	PRAYER_SCARE_MONSTER = 8,
	PRAYER_PORTAL = 9,
	PRAYER_CURE_SERIOUS_WOUNDS = 10,
	PRAYER_CHANT = 11,
	PRAYER_SANCTUARY = 12,
	PRAYER_SATISFY_HUNGER = 13,
	PRAYER_REMOVE_CURSE = 14,
	PRAYER_RESIST_HEAT_COLD = 15,

	/* Chants and Blessings */
	PRAYER_NEUTRALIZE_POISON = 16,
	PRAYER_ORB_OF_DRAINING = 17,
	PRAYER_CURE_CRITICAL_WOUNDS = 18,
	PRAYER_SENSE_INVISIBLE = 19,
	PRAYER_PROTECTION_FROM_EVIL = 20,
	PRAYER_EARTHQUAKE = 21,
	PRAYER_SENSE_SURROUNDINGS = 22,
	PRAYER_CURE_MORTAL_WOUNDS = 23,
	PRAYER_TURN_UNDEAD = 24,

	/* Exorcism and Dispelling */
	PRAYER_PRAYER = 25,
	PRAYER_DISPEL_UNDEAD = 26,
	PRAYER_HEAL = 27,
	PRAYER_DISPEL_EVIL = 28,
	PRAYER_GLYPH_OF_WARDING = 29,
	PRAYER_HOLY_WORD = 30,

	/* Godly Insights */
	PRAYER_DETECT_MONSTERS = 31,
	PRAYER_DETECTION = 32,
	PRAYER_PERCEPTION = 33,
	PRAYER_PROBING = 34,
	PRAYER_CLAIRVOYANCE = 35,

	/* Purifications and Healing */
	PRAYER_CURE_SERIOUS_WOUNDS2 = 36,
	PRAYER_CURE_MORTAL_WOUNDS2 = 37,
	PRAYER_HEALING = 38,
	PRAYER_RESTORATION = 39,
	PRAYER_REMEMBRANCE = 40,

	/* Wrath of God */
	PRAYER_DISPEL_UNDEAD2 = 41,
	PRAYER_DISPEL_EVIL2 = 42,
	PRAYER_BANISH_EVIL = 43,
	PRAYER_WORD_OF_DESTRUCTION = 44,
	PRAYER_ANNIHILATION = 45,

	/* Holy Infusions */
	PRAYER_UNBARRING_WAYS = 46,
	PRAYER_RECHARGING = 47,
	PRAYER_DISPEL_CURSE = 48,
	PRAYER_ENCHANT_WEAPON = 49,
	PRAYER_ENCHANT_ARMOUR = 50,
	PRAYER_ELEMENTAL_BRAND = 51,

	/* Ethereal openings */
	PRAYER_BLINK = 52,
	PRAYER_TELEPORT_SELF = 53,
	PRAYER_TELEPORT_OTHER = 54,
	PRAYER_TELEPORT_LEVEL = 55,
	PRAYER_WORD_OF_RECALL = 56,
	PRAYER_ALTER_REALITY = 57,

	PRAYER_MAX,
};


int get_spell_index(const struct object *object, int index)
{
	struct spell *sp;

	for (sp = object->kind->spells; sp; sp = sp->next)
		if (sp->snum == index)
			return sp->spell_index;
	return -1;
}


const char *get_spell_name(int tval, int spell)
{
	if (tval == TV_MAGIC_BOOK)
		return s_info[spell].name;
	else
		return s_info[spell + PY_MAX_SPELLS].name;
}

typedef struct spell_handler_context_s {
	const int spell;
	const int dir;
	const int beam;
} spell_handler_context_t;

typedef bool (*spell_handler_f)(spell_handler_context_t *);

typedef struct spell_info_s {
	u16b spell;
	bool aim;
	spell_handler_f handler;
} spell_info_t;

void get_spell_info(int tval, int spell, char *p, size_t len)
{
	/* Blank 'p' first */
	p[0] = '\0';

	/* Mage spells */
	if (tval == TV_MAGIC_BOOK)
	{
		int plev = p_ptr->lev;

		/* Analyze the spell */
		switch (spell)
		{
		case SPELL_MAGIC_MISSILE:
			strnfmt(p, len, " dam %dd4", 3 + ((plev - 1) / 5));
			break;
		case SPELL_PHASE_DOOR:
			strnfmt(p, len, " range 10");
			break;
		case SPELL_LIGHT_AREA:
			strnfmt(p, len, " dam 2d%d", (plev / 2));
			break; 
		case SPELL_CURE_LIGHT_WOUNDS:
			strnfmt(p, len, " heal 15%%");
			break;
		case SPELL_STINKING_CLOUD:
			strnfmt(p, len, " dam %d", 10 + (plev / 2));
			break;
		case SPELL_LIGHTNING_BOLT:
			strnfmt(p, len, " dam %dd6", (3 + ((plev - 5) / 6)));
			break;
		case SPELL_FROST_BOLT:
			strnfmt(p, len, " dam %dd8", (5 + ((plev - 5) / 4)));
			break;
		case SPELL_ACID_BOLT:
			strnfmt(p, len, " dam %dd8", (8 + ((plev - 5) / 4)));
			break;
		case SPELL_FIRE_BOLT:
			strnfmt(p, len, " dam %dd8", (6 + ((plev - 5) / 4)));
			break;
		case SPELL_SPEAR_OF_LIGHT:
			strnfmt(p, len, " dam 6d8");
			break;
		case SPELL_HEROISM:
			strnfmt(p, len, " dur 25+d25");
			break;
		case SPELL_BERSERKER:
			strnfmt(p, len, " dur 25+d25");
			break;
		case SPELL_HASTE_SELF:
			strnfmt(p, len, " dur %d+d20", plev);
			break;
		case SPELL_TELEPORT_SELF:
			strnfmt(p, len, " range %d", plev * 5);
			break;
		case SPELL_SHOCK_WAVE:
			strnfmt(p, len, " dam %d", 10 + plev);
			break;
		case SPELL_EXPLOSION:
			strnfmt(p, len, " dam %d", 20 + plev * 2);
			break;
		case SPELL_CLOUD_KILL:
			strnfmt(p, len, " dam %d", 40 + (plev / 2));
			break;
		case SPELL_REND_SOUL:
			strnfmt(p, len, " dam 11d%d", plev);
			break;
		case SPELL_CHAOS_STRIKE:
			strnfmt(p, len, " dam 13d%d", plev);
			break;
		case SPELL_RESIST_COLD:
			strnfmt(p, len, " dur 20+d20");
			break;
		case SPELL_RESIST_FIRE:
			strnfmt(p, len, " dur 20+d20");
			break;
		case SPELL_RESIST_POISON:
			strnfmt(p, len, " dur 20+d20");
			break;
		case SPELL_RESISTANCE:
			strnfmt(p, len, " dur 20+d20");
			break;
		case SPELL_SHIELD:
			strnfmt(p, len, " dur 30+d20");
			break;
		case SPELL_FROST_BALL:
			strnfmt(p, len, " dam %d", 30 + plev);
			break;
		case SPELL_ACID_BALL:
			strnfmt(p, len, " dam %d", 40 + plev);
			break;
		case SPELL_FIRE_BALL:
			strnfmt(p, len, " dam %d", 55 + plev);
			break;
		case SPELL_ICE_STORM:
			strnfmt(p, len, " dam %d", 50 + (plev * 2));
			break;
		case SPELL_METEOR_SWARM:
			strnfmt(p, len, " dam %dx%d", 30 + plev / 2, 2 + plev / 20);
			break;
		case SPELL_RIFT:
			strnfmt(p, len, " dam 40+%dd7", plev);
			break;
		case SPELL_MANA_STORM:
			strnfmt(p, len, " dam %d", 300 + plev * 2);
			break;
		}
	}

	/* Priest spells */
	if (tval == TV_PRAYER_BOOK)
	{
		int plev = p_ptr->lev;

		/* Analyze the spell */
		switch (spell)
		{
			case PRAYER_CURE_LIGHT_WOUNDS:
				my_strcpy(p, " heal 15%", len);
				break;
			case PRAYER_BLESS:
				my_strcpy(p, " dur 12+d12", len);
				break;
			case PRAYER_CALL_LIGHT:
				strnfmt(p, len, " dam 2d%d", (plev / 2));
				break; 
			case PRAYER_PORTAL:
				strnfmt(p, len, " range %d", 3 * plev);
				break;
			case PRAYER_CURE_SERIOUS_WOUNDS:
				my_strcpy(p, " heal 20%", len);
				break;
			case PRAYER_CHANT:
				my_strcpy(p, " dur 24+d24", len);
				break;
			case PRAYER_RESIST_HEAT_COLD:
				my_strcpy(p, " dur 10+d10", len);
				break;
			case PRAYER_ORB_OF_DRAINING:
				strnfmt(p, len, " %d+3d6", plev +
				        (player_has(PF_ZERO_FAIL) 
						? (plev / 2)
						: (plev / 4)));
				break;
			case PRAYER_CURE_CRITICAL_WOUNDS:
				my_strcpy(p, " heal 25%", len);
				break;
			case PRAYER_SENSE_INVISIBLE:
				my_strcpy(p, " dur 24+d24", len);
				break;
			case PRAYER_PROTECTION_FROM_EVIL:
				strnfmt(p, len, " dur %d+d25", 3 * plev);
				break;
			case PRAYER_CURE_MORTAL_WOUNDS:
				my_strcpy(p, " heal 30%", len);
				break;
			case PRAYER_PRAYER:
				my_strcpy(p, " dur 48+d48", len);
				break;
			case PRAYER_DISPEL_UNDEAD:
				strnfmt(p, len, " dam d%d", 3 * plev);
				break;
			case PRAYER_HEAL:
				my_strcpy(p, " heal 35%", len);
				break;
			case PRAYER_DISPEL_EVIL:
				strnfmt(p, len, " dam d%d", 3 * plev);
				break;
			case PRAYER_HOLY_WORD:
				my_strcpy(p, " heal 1000", len);
				break;
			case PRAYER_CURE_SERIOUS_WOUNDS2:
				my_strcpy(p, " heal 20%", len);
				break;
			case PRAYER_CURE_MORTAL_WOUNDS2:
				my_strcpy(p, " heal 30%", len);
				break;
			case PRAYER_HEALING:
				my_strcpy(p, " heal 2000", len);
				break;
			case PRAYER_DISPEL_UNDEAD2:
				strnfmt(p, len, " dam d%d", 4 * plev);
				break;
			case PRAYER_DISPEL_EVIL2:
				strnfmt(p, len, " dam d%d", 4 * plev);
				break;
			case PRAYER_ANNIHILATION:
				my_strcpy(p, " dam 200", len);
				break;
			case PRAYER_BLINK:
				my_strcpy(p, " range 10", len);
				break;
			case PRAYER_TELEPORT_SELF:
				strnfmt(p, len, " range %d", 8 * plev);
				break;
		}
	}

	return;
}


static int beam_chance(void)
{
	int plev = p_ptr->lev;
	return (player_has(PF_BEAM) ? plev : (plev / 2));
}


static void spell_wonder(int dir)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */
	effect_wonder(dir, randint1(100) + p_ptr->lev / 5, beam_chance());
}

#pragma mark arcane spell handlers

static bool spell_handler_arcane_MAGIC_MISSILE(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam-10, GF_MISSILE, context->dir,
					  damroll(3 + ((p_ptr->lev - 1) / 5), 4));
	return TRUE;
}

static bool spell_handler_arcane_DETECT_MONSTERS(spell_handler_context_t *context)
{
	(void)detect_monsters_normal(TRUE);
	return TRUE;
}

static bool spell_handler_arcane_PHASE_DOOR(spell_handler_context_t *context)
{
	teleport_player(10);
	return TRUE;
}

static bool spell_handler_arcane_LIGHT_AREA(spell_handler_context_t *context)
{
	(void)light_area(damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
	return TRUE;
}

static bool spell_handler_arcane_OBJECT_DETECTION(spell_handler_context_t *context)
{
	(void)detect_treasure(TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_CURE_LIGHT_WOUNDS(spell_handler_context_t *context)
{

	heal_player(15, 15);
	player_dec_timed(p_ptr, TMD_CUT, 20, TRUE);
	player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE);
	player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_FIND_TRAPS_DOORS(spell_handler_context_t *context)
{
	(void)detect_traps(TRUE);
	(void)detect_doorstairs(TRUE);
	return TRUE;
}

static bool spell_handler_arcane_STINKING_CLOUD(spell_handler_context_t *context)
{
	fire_ball(GF_POIS, context->dir, 10 + (p_ptr->lev / 2), 2);
	return TRUE;
}

static bool spell_handler_arcane_CONFUSE_MONSTER(spell_handler_context_t *context)
{
	(void)confuse_monster(context->dir, p_ptr->lev, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_LIGHTNING_BOLT(spell_handler_context_t *context)
{
	fire_beam(GF_ELEC, context->dir, damroll(3+((p_ptr->lev-5)/6), 6));
	return TRUE;
}

static bool spell_handler_arcane_TRAP_DOOR_DESTRUCTION(spell_handler_context_t *context)
{
	(void)destroy_doors_touch();
	return TRUE;
}

static bool spell_handler_arcane_SLEEP_MONSTER(spell_handler_context_t *context)
{
	(void)sleep_monster(context->dir, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_CURE_POISON(spell_handler_context_t *context)
{
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_TELEPORT_SELF(spell_handler_context_t *context)
{
	teleport_player(p_ptr->lev * 5);
	return TRUE;
}

static bool spell_handler_arcane_SPEAR_OF_LIGHT(spell_handler_context_t *context)
{
	msg("A line of blue shimmering light appears.");
	light_line(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_FROST_BOLT(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam-10, GF_COLD, context->dir,
					  damroll(5+((p_ptr->lev-5)/4), 8));
	return TRUE;
}

static bool spell_handler_arcane_TURN_STONE_TO_MUD(spell_handler_context_t *context)
{
	(void)wall_to_mud(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_SATISFY_HUNGER(spell_handler_context_t *context)
{
	player_set_food(p_ptr, PY_FOOD_MAX - 1);
	return TRUE;
}

static bool spell_handler_arcane_RECHARGE_ITEM_I(spell_handler_context_t *context)
{
	return recharge(2 + p_ptr->lev / 5);
}

static bool spell_handler_arcane_WONDER(spell_handler_context_t *context)
{
	(void)spell_wonder(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_POLYMORPH_OTHER(spell_handler_context_t *context)
{
	(void)poly_monster(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_IDENTIFY(spell_handler_context_t *context)
{
	return ident_spell();
}

static bool spell_handler_arcane_MASS_SLEEP(spell_handler_context_t *context)
{
	(void)sleep_monsters(TRUE);
	return TRUE;
}

static bool spell_handler_arcane_FIRE_BOLT(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam, GF_FIRE, context->dir,
					  damroll(6+((p_ptr->lev-5)/4), 8));
	return TRUE;
}

static bool spell_handler_arcane_SLOW_MONSTER(spell_handler_context_t *context)
{
	(void)slow_monster(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_FROST_BALL(spell_handler_context_t *context)
{
	fire_ball(GF_COLD, context->dir, 30 + (p_ptr->lev), 2);
	return TRUE;
}

static bool spell_handler_arcane_RECHARGE_ITEM_II(spell_handler_context_t *context) /* greater recharging */
{
	return recharge(50 + p_ptr->lev);
}

static bool spell_handler_arcane_TELEPORT_OTHER(spell_handler_context_t *context)
{
	(void)teleport_monster(context->dir);
	return TRUE;
}

static bool spell_handler_arcane_BEDLAM(spell_handler_context_t *context)
{
	fire_ball(GF_OLD_CONF, context->dir, p_ptr->lev, 4);
	return TRUE;
}

static bool spell_handler_arcane_FIRE_BALL(spell_handler_context_t *context)
{
	fire_ball(GF_FIRE, context->dir, 55 + (p_ptr->lev), 2);
	return TRUE;
}

static bool spell_handler_arcane_WORD_OF_DESTRUCTION(spell_handler_context_t *context)
{
	destroy_area(p_ptr->py, p_ptr->px, 15, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_BANISHMENT(spell_handler_context_t *context)
{
	return banishment();
}

static bool spell_handler_arcane_DOOR_CREATION(spell_handler_context_t *context)
{
	(void)door_creation();
	return TRUE;
}

static bool spell_handler_arcane_STAIR_CREATION(spell_handler_context_t *context)
{
	(void)stair_creation();
	return TRUE;
}

static bool spell_handler_arcane_TELEPORT_LEVEL(spell_handler_context_t *context)
{
	(void)teleport_player_level();
	return TRUE;
}

static bool spell_handler_arcane_EARTHQUAKE(spell_handler_context_t *context)
{
	earthquake(p_ptr->py, p_ptr->px, 10);
	return TRUE;
}

static bool spell_handler_arcane_WORD_OF_RECALL(spell_handler_context_t *context)
{
	return set_recall();
}

static bool spell_handler_arcane_ACID_BOLT(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam, GF_ACID, context->dir, damroll(8+((p_ptr->lev-5)/4), 8));
	return TRUE;
}

static bool spell_handler_arcane_CLOUD_KILL(spell_handler_context_t *context)
{
	fire_ball(GF_POIS, context->dir, 40 + (p_ptr->lev / 2), 3);
	return TRUE;
}

static bool spell_handler_arcane_ACID_BALL(spell_handler_context_t *context)
{
	fire_ball(GF_ACID, context->dir, 40 + (p_ptr->lev), 2);
	return TRUE;
}

static bool spell_handler_arcane_ICE_STORM(spell_handler_context_t *context)
{
	fire_ball(GF_ICE, context->dir, 50 + (p_ptr->lev * 2), 3);
	return TRUE;
}

static bool spell_handler_arcane_METEOR_SWARM(spell_handler_context_t *context)
{
	fire_swarm(2 + p_ptr->lev / 20, GF_METEOR, context->dir, 30 + p_ptr->lev / 2, 1);
	return TRUE;
}

static bool spell_handler_arcane_MANA_STORM(spell_handler_context_t *context)
{
	fire_ball(GF_MANA, context->dir, 300 + (p_ptr->lev * 2), 3);
	return TRUE;
}

static bool spell_handler_arcane_DETECT_INVISIBLE(spell_handler_context_t *context)
{
	(void)detect_monsters_normal(TRUE);
	(void)detect_monsters_invis(TRUE);
	return TRUE;
}

static bool spell_handler_arcane_TREASURE_DETECTION(spell_handler_context_t *context)
{
	(void)detect_treasure(TRUE, FALSE);
	return TRUE;
}

static bool spell_handler_arcane_SHOCK_WAVE(spell_handler_context_t *context)
{
	fire_ball(GF_SOUND, context->dir, 10 + p_ptr->lev, 2);
	return TRUE;
}

static bool spell_handler_arcane_EXPLOSION(spell_handler_context_t *context)
{
	fire_ball(GF_SHARD, context->dir, 20 + (p_ptr->lev * 2), 2);
	return TRUE;
}

static bool spell_handler_arcane_MASS_BANISHMENT(spell_handler_context_t *context)
{
	(void)mass_banishment();
	return TRUE;
}

static bool spell_handler_arcane_RESIST_FIRE(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_RESIST_COLD(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_ELEMENTAL_BRAND(spell_handler_context_t *context)
{
	return brand_ammo();
}

static bool spell_handler_arcane_RESIST_POISON(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(20) + 20, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_RESISTANCE(spell_handler_context_t *context)
{
	int time = randint1(20) + 20;
	(void)player_inc_timed(p_ptr, TMD_OPP_ACID, time, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_ELEC, time, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, time, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_COLD, time, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_POIS, time, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_HEROISM(spell_handler_context_t *context)
{
	int dur = randint1(25) + 25;
	(void)hp_player(10);
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_HERO, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_SHIELD(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_SHIELD, randint1(20) + 30, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_BERSERKER(spell_handler_context_t *context)
{
	int dur = randint1(25) + 25;
	(void)hp_player(30);
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_SHERO, dur, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_arcane_HASTE_SELF(spell_handler_context_t *context)
{
	if (!p_ptr->timed[TMD_FAST])
	{
		(void)player_set_timed(p_ptr, TMD_FAST, randint1(20) + p_ptr->lev, TRUE);
	}
	else
	{
		(void)player_inc_timed(p_ptr, TMD_FAST, randint1(5), TRUE, TRUE);
	}
	return TRUE;
}

static bool spell_handler_arcane_RIFT(spell_handler_context_t *context)
{
	fire_beam(GF_GRAVITY, context->dir,	40 + damroll(p_ptr->lev, 7));
	return TRUE;
}

static bool spell_handler_arcane_REND_SOUL(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam / 4, GF_NETHER, context->dir, damroll(11, p_ptr->lev));
	return TRUE;
}

static bool spell_handler_arcane_CHAOS_STRIKE(spell_handler_context_t *context)
{
	fire_bolt_or_beam(context->beam, GF_CHAOS, context->dir, damroll(13, p_ptr->lev));
	return TRUE;
}

static bool spell_handler_arcane_RUNE_OF_PROTECTION(spell_handler_context_t *context)
{
	warding_glyph_spell();
	return TRUE;
}

static bool spell_handler_arcane_ENCHANT_ARMOR(spell_handler_context_t *context)
{
	return enchant_spell(0, 0, randint0(3) + p_ptr->lev / 20);
}

static bool spell_handler_arcane_ENCHANT_WEAPON(spell_handler_context_t *context)
{
	return enchant_spell(randint0(4) + p_ptr->lev / 20,
						 randint0(4) + p_ptr->lev / 20, 0);
}

#pragma mark prayer handlers

static bool spell_handler_prayer_DETECT_EVIL(spell_handler_context_t *context)
{
	(void)detect_monsters_evil(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_CURE_LIGHT_WOUNDS(spell_handler_context_t *context)
{
	(void)heal_player(15, 15);
	(void)player_dec_timed(p_ptr, TMD_CUT, 20, TRUE);
	(void)player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_BLESS(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(12) + 12, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_REMOVE_FEAR(spell_handler_context_t *context)
{
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_CALL_LIGHT(spell_handler_context_t *context)
{
	(void)light_area(damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
	return TRUE;
}

static bool spell_handler_prayer_FIND_TRAPS_DOORS(spell_handler_context_t *context)
{
	(void)detect_traps(TRUE);
	(void)detect_doorstairs(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_SLOW_POISON(spell_handler_context_t *context)
{
	(void)player_set_timed(p_ptr, TMD_POISONED, p_ptr->timed[TMD_POISONED] / 2, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_SCARE_MONSTER(spell_handler_context_t *context)
{
	(void)fear_monster(context->dir, p_ptr->lev, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_PORTAL(spell_handler_context_t *context)
{
	teleport_player(p_ptr->lev * 3);
	return TRUE;
}

static bool spell_handler_prayer_CURE_SERIOUS_WOUNDS(spell_handler_context_t *context)
{
	(void)heal_player(20, 25);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_CHANT(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(24) + 24, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_SANCTUARY(spell_handler_context_t *context)
{
	(void)sleep_monsters_touch(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_SATISFY_HUNGER(spell_handler_context_t *context)
{
	player_set_food(p_ptr, PY_FOOD_MAX - 1);
	return TRUE;
}

static bool spell_handler_prayer_REMOVE_CURSE(spell_handler_context_t *context)
{
	/* Remove curse has been removed in 3.4 until curses are redone */
	/* remove_curse(); */
	return FALSE;
}

static bool spell_handler_prayer_RESIST_HEAT_COLD(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(10) + 10, TRUE, TRUE);
	(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(10) + 10, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_NEUTRALIZE_POISON(spell_handler_context_t *context)
{
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_ORB_OF_DRAINING(spell_handler_context_t *context)
{
	fire_ball(GF_HOLY_ORB, context->dir,
			  (damroll(3, 6) + p_ptr->lev +
			   (player_has(PF_ZERO_FAIL)
				? (p_ptr->lev / 2)
				: (p_ptr->lev / 4))),
			  ((p_ptr->lev < 30) ? 2 : 3));
	return TRUE;
}

static bool spell_handler_prayer_CURE_CRITICAL_WOUNDS(spell_handler_context_t *context)
{
	(void)heal_player(25, 30);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_SENSE_INVISIBLE(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_SINVIS, randint1(24) + 24, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_PROTECTION_FROM_EVIL(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_PROTEVIL, randint1(25) + 3 * p_ptr->lev, TRUE,
						   TRUE);
	return TRUE;
}

static bool spell_handler_prayer_EARTHQUAKE(spell_handler_context_t *context)
{
	earthquake(p_ptr->py, p_ptr->px, 10);
	return TRUE;
}

static bool spell_handler_prayer_SENSE_SURROUNDINGS(spell_handler_context_t *context)
{
	map_area();
	return TRUE;
}

static bool spell_handler_prayer_CURE_MORTAL_WOUNDS(spell_handler_context_t *context)
{
	(void)heal_player(30, 50);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_TURN_UNDEAD(spell_handler_context_t *context)
{
	(void)turn_undead(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_PRAYER(spell_handler_context_t *context)
{
	(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(48) + 48, TRUE, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_DISPEL_UNDEAD(spell_handler_context_t *context)
{
	(void)dispel_undead(randint1(p_ptr->lev * 3));
	return TRUE;
}

static bool spell_handler_prayer_HEAL(spell_handler_context_t *context)
{
	int amt = (p_ptr->mhp * 35) / 100;
	if (amt < 300) amt = 300;

	(void)hp_player(amt);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_DISPEL_EVIL(spell_handler_context_t *context)
{
	(void)dispel_evil(randint1(p_ptr->lev * 3));
	return TRUE;
}

static bool spell_handler_prayer_GLYPH_OF_WARDING(spell_handler_context_t *context)
{
	warding_glyph_spell();
	return TRUE;
}

static bool spell_handler_prayer_HOLY_WORD(spell_handler_context_t *context)
{
	(void)dispel_evil(randint1(p_ptr->lev * 4));
	(void)hp_player(1000);
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_DETECT_MONSTERS(spell_handler_context_t *context)
{
	(void)detect_monsters_normal(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_DETECTION(spell_handler_context_t *context)
{
	(void)detect_all(TRUE);
	return TRUE;
}

static bool spell_handler_prayer_PERCEPTION(spell_handler_context_t *context)
{
	return ident_spell();
}

static bool spell_handler_prayer_PROBING(spell_handler_context_t *context)
{
	(void)probing();
	return TRUE;
}

static bool spell_handler_prayer_CLAIRVOYANCE(spell_handler_context_t *context)
{
	wiz_light(FALSE);
	return TRUE;
}

static bool spell_handler_prayer_CURE_SERIOUS_WOUNDS2(spell_handler_context_t *context)
{
	(void)heal_player(20, 25);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_CURE_MORTAL_WOUNDS2(spell_handler_context_t *context)
{
	(void)heal_player(30, 50);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_HEALING(spell_handler_context_t *context)
{
	(void)hp_player(2000);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_RESTORATION(spell_handler_context_t *context)
{
	(void)do_res_stat(A_STR);
	(void)do_res_stat(A_INT);
	(void)do_res_stat(A_WIS);
	(void)do_res_stat(A_DEX);
	(void)do_res_stat(A_CON);
	return TRUE;
}

static bool spell_handler_prayer_REMEMBRANCE(spell_handler_context_t *context)
{
	(void)restore_level();
	return TRUE;
}

static bool spell_handler_prayer_DISPEL_UNDEAD2(spell_handler_context_t *context)
{
	(void)dispel_undead(randint1(p_ptr->lev * 4));
	return TRUE;
}

static bool spell_handler_prayer_DISPEL_EVIL2(spell_handler_context_t *context)
{
	(void)dispel_evil(randint1(p_ptr->lev * 4));
	return TRUE;
}

static bool spell_handler_prayer_BANISH_EVIL(spell_handler_context_t *context)
{
	if (banish_evil(100))
	{
		msg("The power of your god banishes evil!");
	}
	return TRUE;
}

static bool spell_handler_prayer_WORD_OF_DESTRUCTION(spell_handler_context_t *context)
{
	destroy_area(p_ptr->py, p_ptr->px, 15, TRUE);
	return TRUE;
}

static bool spell_handler_prayer_ANNIHILATION(spell_handler_context_t *context)
{
	drain_life(context->dir, 200);
	return TRUE;
}

static bool spell_handler_prayer_UNBARRING_WAYS(spell_handler_context_t *context)
{
	(void)destroy_doors_touch();
	return TRUE;
}

static bool spell_handler_prayer_RECHARGING(spell_handler_context_t *context)
{
	return recharge(20 + p_ptr->lev);
}

static bool spell_handler_prayer_DISPEL_CURSE(spell_handler_context_t *context)
{
	/* Dispel Curse has been removed in 3.4 until curses are redone */
	/* (void)remove_all_curse(); */
	return FALSE;
}

static bool spell_handler_prayer_ENCHANT_WEAPON(spell_handler_context_t *context)
{
	return enchant_spell(randint0(4) + 1, randint0(4) + 1, 0);
}

static bool spell_handler_prayer_ENCHANT_ARMOUR(spell_handler_context_t *context)
{
	return enchant_spell(0, 0, randint0(3) + 2);
}

static bool spell_handler_prayer_ELEMENTAL_BRAND(spell_handler_context_t *context)
{
	brand_weapon();
	return TRUE;
}

static bool spell_handler_prayer_BLINK(spell_handler_context_t *context)
{
	teleport_player(10);
	return TRUE;
}

static bool spell_handler_prayer_TELEPORT_SELF(spell_handler_context_t *context)
{
	teleport_player(p_ptr->lev * 8);
	return TRUE;
}

static bool spell_handler_prayer_TELEPORT_OTHER(spell_handler_context_t *context)
{
	(void)teleport_monster(context->dir);
	return TRUE;
}

static bool spell_handler_prayer_TELEPORT_LEVEL(spell_handler_context_t *context)
{
	(void)teleport_player_level();
	return TRUE;
}

static bool spell_handler_prayer_WORD_OF_RECALL(spell_handler_context_t *context)
{
	return set_recall();
}

static bool spell_handler_prayer_ALTER_REALITY(spell_handler_context_t *context)
{
	msg("The world changes!");

	/* Leaving */
	p_ptr->leaving = TRUE;

	return TRUE;
}

static const spell_info_t arcane_spells[] = {
	#define F(x) spell_handler_arcane_##x
	#define SPELL(x, a, f) {x, a, f},
	#include "list-spells-arcane.h"
	#undef SPELL
	#undef F
};

static const spell_info_t prayer_spells[] = {
	#define F(x) spell_handler_prayer_##x
	#define SPELL(x, a, f) {x, a, f},
	#include "list-spells-prayer.h"
	#undef SPELL
	#undef F
};

static const spell_info_t *spell_info_for_index(const spell_info_t *list, size_t list_size, int spell_max, int spell)
{
	size_t i;

	if (spell < 0 || spell >= spell_max)
		return NULL;

	for (i = 0; i < list_size; i++) {
		if (list[i].spell == spell)
			return &list[i];
	}

	return NULL;
}

static bool cast_mage_spell(int spell, int dir)
{
	spell_handler_f spell_handler = NULL;
	const spell_info_t *spell_info = spell_info_for_index(arcane_spells, N_ELEMENTS(arcane_spells), SPELL_MAX, spell);

	if (spell_info == NULL)
		return FALSE;

	spell_handler = spell_info->handler;

	if (spell_handler == NULL)
		return FALSE;

	spell_handler_context_t context = {
		spell,
		dir,
		beam_chance(),
	};

	return spell_handler(&context);
}

static bool cast_priest_spell(int spell, int dir)
{
	spell_handler_f spell_handler = NULL;
	const spell_info_t *spell_info = spell_info_for_index(prayer_spells, N_ELEMENTS(prayer_spells), PRAYER_MAX, spell);

	if (spell_info == NULL)
		return FALSE;

	spell_handler = spell_info->handler;

	if (spell_handler == NULL)
		return FALSE;

	spell_handler_context_t context = {
		spell,
		dir,
		0,
	};

	return spell_handler(&context);
}

bool cast_spell(int tval, int index, int dir)
{
	if (tval == TV_MAGIC_BOOK)
	{
		return cast_mage_spell(index, dir);
	}
	else
	{
		return cast_priest_spell(index, dir);
	}
}

bool spell_is_identify(int book, int spell)
{
	return (book == TV_MAGIC_BOOK && spell == SPELL_IDENTIFY) || (book == TV_PRAYER_BOOK && spell == PRAYER_PERCEPTION);
}

bool spell_needs_aim(int tval, int spell)
{
	const spell_info_t *spell_info = NULL;

	if (tval == TV_MAGIC_BOOK) {
		spell_info = spell_info_for_index(arcane_spells, N_ELEMENTS(arcane_spells), SPELL_MAX, spell);
	}
	else if (tval == TV_PRAYER_BOOK) {
		spell_info = spell_info_for_index(prayer_spells, N_ELEMENTS(prayer_spells), PRAYER_MAX, spell);
	}
	else {
		/* Unknown book */
		return FALSE;
	}

	if (spell_info == NULL)
		return FALSE;

	return spell_info->aim;
}

static int spell_lookup_by_name_arcane(const char *name)
{
	static const char *spell_names[] = {
		#define SPELL(x, a, f) #x,
		#include "list-spells-arcane.h"
		#undef SPELL
	};
	size_t i;
	unsigned int number;

	if (sscanf(name, "%u", &number) == 1)
		return number;

	for (i = 0; i < N_ELEMENTS(spell_names); i++) {
		if (my_stricmp(name, spell_names[i]) == 0)
			return (int)i;
	}

	return -1;
}

static int spell_lookup_by_name_prayer(const char *name)
{
	static const char *spell_names[] = {
		#define SPELL(x, a, f) #x,
		#include "list-spells-prayer.h"
		#undef SPELL
	};
	size_t i;
	unsigned int number;

	if (sscanf(name, "%u", &number) == 1)
		return number;

	for (i = 0; i < N_ELEMENTS(spell_names); i++) {
		if (my_stricmp(name, spell_names[i]) == 0)
			return (int)i;
	}

	return -1;
}

int spell_lookup_by_name(int tval, const char *name)
{
	if (name == NULL)
		return -1;

	if (tval == TV_MAGIC_BOOK)
		return spell_lookup_by_name_arcane(name);
	else if (tval == TV_PRAYER_BOOK)
		return spell_lookup_by_name_prayer(name);
	else
		return -1;
}
