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
#define SPELL_MAGIC_MISSILE             0
#define SPELL_DETECT_MONSTERS           1
#define SPELL_PHASE_DOOR                2
#define SPELL_LIGHT_AREA                3
#define SPELL_FIND_TRAPS_DOORS          4
#define SPELL_CURE_LIGHT_WOUNDS         5
#define SPELL_TREASURE_DETECTION        6
/* #define SPELL_OBJECT_DETECTION          7 */
#define SPELL_IDENTIFY                  8
#define SPELL_DETECT_INVISIBLE          9
#define SPELL_DETECT_ENCHANTMENT        10
#define SPELL_STINKING_CLOUD            11
#define SPELL_LIGHTNING_BOLT            12
#define SPELL_CONFUSE_MONSTER           13
#define SPELL_SLEEP_MONSTER             14
#define SPELL_WONDER                    15
#define SPELL_FROST_BOLT                16
#define SPELL_ACID_BOLT                 17
#define SPELL_FIRE_BOLT                 18
#define SPELL_TRAP_DOOR_DESTRUCTION     19
#define SPELL_SPEAR_OF_LIGHT            20
#define SPELL_TURN_STONE_TO_MUD         21
#define SPELL_DOOR_CREATION             22
#define SPELL_EARTHQUAKE                23
#define SPELL_STAIR_CREATION            24
#define SPELL_CURE_POISON               25
#define SPELL_SATISFY_HUNGER            26
#define SPELL_HEROISM                   27
#define SPELL_BERSERKER                 28
#define SPELL_HASTE_SELF                29
#define SPELL_TELEPORT_SELF             30
#define SPELL_SLOW_MONSTER              31
#define SPELL_TELEPORT_OTHER            32
#define SPELL_TELEPORT_LEVEL            33
#define SPELL_WORD_OF_RECALL            34
#define SPELL_POLYMORPH_OTHER           35
#define SPELL_SHOCK_WAVE                36
#define SPELL_EXPLOSION                 37
#define SPELL_CLOUD_KILL                38
#define SPELL_MASS_SLEEP                39
#define SPELL_BEDLAM                    40
#define SPELL_REND_SOUL                 41
#define SPELL_WORD_OF_DESTRUCTION       42
#define SPELL_CHAOS_STRIKE              43
#define SPELL_RESIST_COLD               44
#define SPELL_RESIST_FIRE               45
#define SPELL_RESIST_POISON             46
#define SPELL_RESISTANCE                47
#define SPELL_SHIELD                    48
#define SPELL_RUNE_OF_PROTECTION        49
#define SPELL_RECHARGE_ITEM_I           50
#define SPELL_ENCHANT_ARMOR             51
#define SPELL_ENCHANT_WEAPON            52
#define SPELL_RECHARGE_ITEM_II          53
#define SPELL_ELEMENTAL_BRAND           54
#define SPELL_FROST_BALL                55
#define SPELL_ACID_BALL                 56
#define SPELL_FIRE_BALL                 57
#define SPELL_ICE_STORM                 58
#define SPELL_BANISHMENT                59
#define SPELL_METEOR_SWARM              60
#define SPELL_MASS_BANISHMENT           61
#define SPELL_RIFT                      62
#define SPELL_MANA_STORM                63

/* Beginners Handbook */
#define PRAYER_DETECT_EVIL              0
#define PRAYER_CURE_LIGHT_WOUNDS        1
#define PRAYER_BLESS                    2
#define PRAYER_REMOVE_FEAR              3
#define PRAYER_CALL_LIGHT               4
#define PRAYER_FIND_TRAPS_DOORS         5
#define PRAYER_SLOW_POISON              7

/* Words of Wisdom */
#define PRAYER_SCARE_MONSTER            8
#define PRAYER_PORTAL                   9
#define PRAYER_CURE_SERIOUS_WOUNDS     10
#define PRAYER_CHANT                   11
#define PRAYER_SANCTUARY               12
#define PRAYER_SATISFY_HUNGER          13
#define PRAYER_REMOVE_CURSE            14
#define PRAYER_RESIST_HEAT_COLD        15

/* Chants and Blessings */
#define PRAYER_NEUTRALIZE_POISON       16
#define PRAYER_ORB_OF_DRAINING         17
#define PRAYER_CURE_CRITICAL_WOUNDS    18
#define PRAYER_SENSE_INVISIBLE         19
#define PRAYER_PROTECTION_FROM_EVIL    20
#define PRAYER_EARTHQUAKE              21
#define PRAYER_SENSE_SURROUNDINGS      22
#define PRAYER_CURE_MORTAL_WOUNDS      23
#define PRAYER_TURN_UNDEAD             24

/* Exorcism and Dispelling */
#define PRAYER_PRAYER                  25
#define PRAYER_DISPEL_UNDEAD           26
#define PRAYER_HEAL                    27
#define PRAYER_DISPEL_EVIL             28
#define PRAYER_GLYPH_OF_WARDING        29
#define PRAYER_HOLY_WORD               30

/* Godly Insights */
#define PRAYER_DETECT_MONSTERS         31
#define PRAYER_DETECTION               32
#define PRAYER_PERCEPTION              33
#define PRAYER_PROBING                 34
#define PRAYER_CLAIRVOYANCE            35

/* Purifications and Healing */
#define PRAYER_CURE_SERIOUS_WOUNDS2    36
#define PRAYER_CURE_MORTAL_WOUNDS2     37
#define PRAYER_HEALING                 38
#define PRAYER_RESTORATION             39
#define PRAYER_REMEMBRANCE             40

/* Wrath of God */
#define PRAYER_DISPEL_UNDEAD2          41
#define PRAYER_DISPEL_EVIL2            42
#define PRAYER_BANISH_EVIL             43
#define PRAYER_WORD_OF_DESTRUCTION     44
#define PRAYER_ANNIHILATION            45

/* Holy Infusions */
#define PRAYER_UNBARRING_WAYS          46
#define PRAYER_RECHARGING              47
#define PRAYER_DISPEL_CURSE            48
#define PRAYER_ENCHANT_WEAPON          49
#define PRAYER_ENCHANT_ARMOUR          50
#define PRAYER_ELEMENTAL_BRAND         51

/* Ethereal openings */
#define PRAYER_BLINK                   52
#define PRAYER_TELEPORT_SELF           53
#define PRAYER_TELEPORT_OTHER          54
#define PRAYER_TELEPORT_LEVEL          55
#define PRAYER_WORD_OF_RECALL          56
#define PRAYER_ALTER_REALITY           57



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


bool spell_needs_aim(int tval, int spell)
{
	if (tval == TV_MAGIC_BOOK)
	{
		switch (spell)
		{
			case SPELL_MAGIC_MISSILE:
			case SPELL_STINKING_CLOUD:
			case SPELL_CONFUSE_MONSTER:
			case SPELL_LIGHTNING_BOLT:
			case SPELL_SLEEP_MONSTER:
			case SPELL_SPEAR_OF_LIGHT:
			case SPELL_FROST_BOLT:
			case SPELL_TURN_STONE_TO_MUD:
			case SPELL_WONDER:
			case SPELL_POLYMORPH_OTHER:
			case SPELL_FIRE_BOLT:
			case SPELL_SLOW_MONSTER:
			case SPELL_FROST_BALL:
			case SPELL_TELEPORT_OTHER:
			case SPELL_BEDLAM:
			case SPELL_FIRE_BALL:
			case SPELL_ACID_BOLT:
			case SPELL_CLOUD_KILL:
			case SPELL_ACID_BALL:
			case SPELL_ICE_STORM:
			case SPELL_METEOR_SWARM:
			case SPELL_MANA_STORM:
			case SPELL_SHOCK_WAVE:
			case SPELL_EXPLOSION:
			case SPELL_RIFT:
			case SPELL_REND_SOUL: 
			case SPELL_CHAOS_STRIKE: 
				return TRUE;
				
			default:
				return FALSE;
		}
	}
	else
	{
		switch (spell)
		{
			case PRAYER_SCARE_MONSTER:
			case PRAYER_ORB_OF_DRAINING:
			case PRAYER_ANNIHILATION:
			case PRAYER_TELEPORT_OTHER:
				return TRUE;

			default:
				return FALSE;
		}
	}
}


static bool cast_mage_spell(int spell, int dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int plev = p_ptr->lev;

	/* Hack -- chance of "beam" instead of "bolt" */
	int beam = beam_chance();

	/* Spells. */
	switch (spell)
	{
		case SPELL_MAGIC_MISSILE:
		{
			fire_bolt_or_beam(beam-10, GF_MISSILE, dir,
			                  damroll(3 + ((plev - 1) / 5), 4));
			break;
		}

		case SPELL_DETECT_MONSTERS:
		{
			(void)detect_monsters_normal(TRUE);
			break;
		}

		case SPELL_PHASE_DOOR:
		{
			teleport_player(10);
			break;
		}

		case SPELL_LIGHT_AREA:
		{
			(void)light_area(damroll(2, (plev / 2)), (plev / 10) + 1);
			break;
		}

		case SPELL_TREASURE_DETECTION:
		{
			(void)detect_treasure(TRUE);
			break;
		}

		case SPELL_CURE_LIGHT_WOUNDS:
		{

			heal_player(15, 15);
			player_dec_timed(p_ptr, TMD_CUT, 20, TRUE);
			player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE);
			player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			break;
		}

		case SPELL_FIND_TRAPS_DOORS:
		{
			(void)detect_traps(TRUE);
			(void)detect_doorstairs(TRUE);
			break;
		}

		case SPELL_STINKING_CLOUD:
		{
			fire_ball(GF_POIS, dir, 10 + (plev / 2), 2);
			break;
		}

		case SPELL_CONFUSE_MONSTER:
		{
			(void)confuse_monster(dir, plev, TRUE);
			break;
		}

		case SPELL_LIGHTNING_BOLT:
		{
			fire_beam(GF_ELEC, dir, damroll(3+((plev-5)/6), 6));
			break;
		}

		case SPELL_TRAP_DOOR_DESTRUCTION:
		{
			(void)destroy_doors_touch();
			break;
		}

		case SPELL_SLEEP_MONSTER:
		{
			(void)sleep_monster(dir, TRUE);
			break;
		}

		case SPELL_CURE_POISON:
		{
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			break;
		}

		case SPELL_TELEPORT_SELF:
		{
			teleport_player(plev * 5);
			break;
		}

		case SPELL_SPEAR_OF_LIGHT:
		{
			msg("A line of blue shimmering light appears.");
			light_line(dir);
			break;
		}

		case SPELL_FROST_BOLT:
		{
			fire_bolt_or_beam(beam-10, GF_COLD, dir,
			                  damroll(5+((plev-5)/4), 8));
			break;
		}

		case SPELL_TURN_STONE_TO_MUD:
		{
			(void)wall_to_mud(dir);
			break;
		}

		case SPELL_SATISFY_HUNGER:
		{
			player_set_food(p_ptr, PY_FOOD_MAX - 1);
			break;
		}

		case SPELL_RECHARGE_ITEM_I:
		{
			return recharge(2 + plev / 5);
		}

		case SPELL_WONDER:
		{
			(void)spell_wonder(dir);
			break;
		}

		case SPELL_POLYMORPH_OTHER:
		{
			(void)poly_monster(dir);
			break;
		}

		case SPELL_IDENTIFY:
		{
			return ident_spell();
		}

		case SPELL_MASS_SLEEP:
		{
			(void)sleep_monsters(TRUE);
			break;
		}

		case SPELL_FIRE_BOLT:
		{
			fire_bolt_or_beam(beam, GF_FIRE, dir,
			                  damroll(6+((plev-5)/4), 8));
			break;
		}

		case SPELL_SLOW_MONSTER:
		{
			(void)slow_monster(dir);
			break;
		}

		case SPELL_FROST_BALL:
		{
			fire_ball(GF_COLD, dir, 30 + (plev), 2);
			break;
		}

		case SPELL_RECHARGE_ITEM_II: /* greater recharging */
		{
			return recharge(50 + plev);
		}

		case SPELL_TELEPORT_OTHER:
		{
			(void)teleport_monster(dir);
			break;
		}

		case SPELL_BEDLAM:
		{
			fire_ball(GF_OLD_CONF, dir, plev, 4);
			break;
		}

		case SPELL_FIRE_BALL:
		{
			fire_ball(GF_FIRE, dir, 55 + (plev), 2);
			break;
		}

		case SPELL_WORD_OF_DESTRUCTION:
		{
			destroy_area(py, px, 15, TRUE);
			break;
		}

		case SPELL_BANISHMENT:
		{
			return banishment();
			break;
		}

		case SPELL_DOOR_CREATION:
		{
			(void)door_creation();
			break;
		}

		case SPELL_STAIR_CREATION:
		{
			(void)stair_creation();
			break;
		}

		case SPELL_TELEPORT_LEVEL:
		{
			(void)teleport_player_level();
			break;
		}

		case SPELL_EARTHQUAKE:
		{
			earthquake(py, px, 10);
			break;
		}

		case SPELL_WORD_OF_RECALL:
		{
			set_recall();
			break;
		}

		case SPELL_ACID_BOLT:
		{
			fire_bolt_or_beam(beam, GF_ACID, dir, damroll(8+((plev-5)/4), 8));
			break;
		}

		case SPELL_CLOUD_KILL:
		{
			fire_ball(GF_POIS, dir, 40 + (plev / 2), 3);
			break;
		}

		case SPELL_ACID_BALL:
		{
			fire_ball(GF_ACID, dir, 40 + (plev), 2);
			break;
		}

		case SPELL_ICE_STORM:
		{
			fire_ball(GF_ICE, dir, 50 + (plev * 2), 3);
			break;
		}

		case SPELL_METEOR_SWARM:
		{
			fire_swarm(2 + plev / 20, GF_METEOR, dir, 30 + plev / 2, 1);
			break;
		}

		case SPELL_MANA_STORM:
		{
			fire_ball(GF_MANA, dir, 300 + (plev * 2), 3);
			break;
		}
		case SPELL_DETECT_INVISIBLE:
		{
			(void)detect_monsters_normal(TRUE);
			(void)detect_monsters_invis(TRUE);
			break;
		}

		case SPELL_DETECT_ENCHANTMENT:
		{
			(void)detect_objects_magic(TRUE);
			break;
		}

		case SPELL_SHOCK_WAVE:
		{
			fire_ball(GF_SOUND, dir, 10 + plev, 2);
			break;
		}

		case SPELL_EXPLOSION:
		{
			fire_ball(GF_SHARD, dir, 20 + (plev * 2), 2);
			break;
		}

		case SPELL_MASS_BANISHMENT:
		{
			(void)mass_banishment();
			break;
		}

		case SPELL_RESIST_FIRE:
		{
			(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(20) + 20, TRUE, TRUE);
			break;
		}

		case SPELL_RESIST_COLD:
		{
			(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(20) + 20, TRUE, TRUE);
			break;
		}

		case SPELL_ELEMENTAL_BRAND: /* elemental brand */
		{
			return brand_ammo();
		}

		case SPELL_RESIST_POISON:
		{
			(void)player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(20) + 20, TRUE, TRUE);
			break;
		}

		case SPELL_RESISTANCE:
		{
			int time = randint1(20) + 20;
			(void)player_inc_timed(p_ptr, TMD_OPP_ACID, time, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_ELEC, time, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, time, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_COLD, time, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_POIS, time, TRUE, TRUE);
			break;
		}

		case SPELL_HEROISM:
		{
			(void)hp_player(10);
			(void)player_inc_timed(p_ptr, TMD_HERO, randint1(25) + 25, TRUE, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			break;
		}

		case SPELL_SHIELD:
		{
			(void)player_inc_timed(p_ptr, TMD_SHIELD, randint1(20) + 30, TRUE, TRUE);
			break;
		}

		case SPELL_BERSERKER:
		{
			(void)hp_player(30);
			(void)player_inc_timed(p_ptr, TMD_SHERO, randint1(25) + 25, TRUE, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			break;
		}

		case SPELL_HASTE_SELF:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				(void)player_set_timed(p_ptr, TMD_FAST, randint1(20) + plev, TRUE);
			}
			else
			{
				(void)player_inc_timed(p_ptr, TMD_FAST, randint1(5), TRUE, TRUE);
			}
			break;
		}

		case SPELL_RIFT:
		{
			fire_beam(GF_GRAVITY, dir,	40 + damroll(plev, 7));
			break;
		}

		case SPELL_REND_SOUL: /* rend soul */
		{
			fire_bolt_or_beam(beam / 4, GF_NETHER, dir, damroll(11, plev));
			break;
		}

		case SPELL_CHAOS_STRIKE: /* chaos strike */
		{
			fire_bolt_or_beam(beam, GF_CHAOS, dir, damroll(13, plev));
			break;
		}

		case SPELL_RUNE_OF_PROTECTION: /* rune of protection */
		{
			warding_glyph_spell();
			break;
		}

		case SPELL_ENCHANT_ARMOR: /* enchant armor */
		{
			return enchant_spell(0, 0, randint0(3) + plev / 20);
		}

		case SPELL_ENCHANT_WEAPON: /* enchant weapon */
		{
			return enchant_spell(randint0(4) + plev / 20,
			                     randint0(4) + plev / 20, 0);
		}
	}

	/* Success */
	return (TRUE);
}


static bool cast_priest_spell(int spell, int dir)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int plev = p_ptr->lev;

	int amt;

	switch (spell)
	{
		case PRAYER_DETECT_EVIL:
		{
			(void)detect_monsters_evil(TRUE);
			break;
		}

		case PRAYER_CURE_LIGHT_WOUNDS:
		{
			(void)heal_player(15, 15);
			(void)player_dec_timed(p_ptr, TMD_CUT, 20, TRUE);
			(void)player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			break;
		}

		case PRAYER_BLESS:
		{
			(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(12) + 12, TRUE, TRUE);
			break;
		}

		case PRAYER_REMOVE_FEAR:
		{
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			break;
		}

		case PRAYER_CALL_LIGHT:
		{
			(void)light_area(damroll(2, (plev / 2)), (plev / 10) + 1);
			break;
		}

		case PRAYER_FIND_TRAPS_DOORS:
		{
			(void)detect_traps(TRUE);
			(void)detect_doorstairs(TRUE);
			break;
		}

		case PRAYER_SLOW_POISON:
		{
			(void)player_set_timed(p_ptr, TMD_POISONED, p_ptr->timed[TMD_POISONED] / 2, TRUE);
			break;
		}

		case PRAYER_SCARE_MONSTER:
		{
			(void)fear_monster(dir, plev, TRUE);
			break;
		}

		case PRAYER_PORTAL:
		{
			teleport_player(plev * 3);
			break;
		}

		case PRAYER_CURE_SERIOUS_WOUNDS:
		{
			(void)heal_player(20, 25);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			break;
		}

		case PRAYER_CHANT:
		{
			(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(24) + 24, TRUE, TRUE);
			break;
		}

		case PRAYER_SANCTUARY:
		{
			(void)sleep_monsters_touch(TRUE);
			break;
		}

		case PRAYER_SATISFY_HUNGER:
		{
			player_set_food(p_ptr, PY_FOOD_MAX - 1);
			break;
		}

		case PRAYER_REMOVE_CURSE:
		{
			remove_curse();
			break;
		}

		case PRAYER_RESIST_HEAT_COLD:
		{
			(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(10) + 10, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(10) + 10, TRUE, TRUE);
			break;
		}

		case PRAYER_NEUTRALIZE_POISON:
		{
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			break;
		}

		case PRAYER_ORB_OF_DRAINING:
		{
			fire_ball(GF_HOLY_ORB, dir,
				(damroll(3, 6) + plev +
				 (player_has(PF_ZERO_FAIL)
					? (plev / 2)
					: (plev / 4))),	
				((plev < 30) ? 2 : 3));
			break;
		}

		case PRAYER_CURE_CRITICAL_WOUNDS:
		{
			(void)heal_player(25, 30);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			break;
		}

		case PRAYER_SENSE_INVISIBLE:
		{
			(void)player_inc_timed(p_ptr, TMD_SINVIS, randint1(24) + 24, TRUE, TRUE);
			break;
		}

		case PRAYER_PROTECTION_FROM_EVIL:
		{
			(void)player_inc_timed(p_ptr, TMD_PROTEVIL, randint1(25) + 3 * p_ptr->lev, TRUE,
				TRUE);
			break;
		}

		case PRAYER_EARTHQUAKE:
		{
			earthquake(py, px, 10);
			break;
		}

		case PRAYER_SENSE_SURROUNDINGS:
		{
			map_area();
			break;
		}

		case PRAYER_CURE_MORTAL_WOUNDS:
		{
			(void)heal_player(30, 50);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			break;
		}

		case PRAYER_TURN_UNDEAD:
		{
			(void)turn_undead(TRUE);
			break;
		}

		case PRAYER_PRAYER:
		{
			(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(48) + 48, TRUE, TRUE);
			break;
		}

		case PRAYER_DISPEL_UNDEAD:
		{
			(void)dispel_undead(randint1(plev * 3));
			break;
		}

		case PRAYER_HEAL:
		{
			amt = (p_ptr->mhp * 35) / 100;
                        if (amt < 300) amt = 300;
			
			(void)hp_player(amt);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			break;
		}

		case PRAYER_DISPEL_EVIL:
		{
			(void)dispel_evil(randint1(plev * 3));
			break;
		}

		case PRAYER_GLYPH_OF_WARDING:
		{
			warding_glyph_spell();
			break;
		}

		case PRAYER_HOLY_WORD:
		{
			(void)dispel_evil(randint1(plev * 4));
			(void)hp_player(1000);
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			break;
		}

		case PRAYER_DETECT_MONSTERS:
		{
			(void)detect_monsters_normal(TRUE);
			break;
		}

		case PRAYER_DETECTION:
		{
			(void)detect_all(TRUE);
			break;
		}

		case PRAYER_PERCEPTION:
		{
			return ident_spell();
		}

		case PRAYER_PROBING:
		{
			(void)probing();
			break;
		}

		case PRAYER_CLAIRVOYANCE:
		{
			wiz_light();
			break;
		}

		case PRAYER_CURE_SERIOUS_WOUNDS2:
		{
			(void)heal_player(20, 25);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			break;
		}

		case PRAYER_CURE_MORTAL_WOUNDS2:
		{
			(void)heal_player(30, 50);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			break;
		}

		case PRAYER_HEALING:
		{
			(void)hp_player(2000);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			break;
		}

		case PRAYER_RESTORATION:
		{
			(void)do_res_stat(A_STR);
			(void)do_res_stat(A_INT);
			(void)do_res_stat(A_WIS);
			(void)do_res_stat(A_DEX);
			(void)do_res_stat(A_CON);
			(void)do_res_stat(A_CHR);
			break;
		}

		case PRAYER_REMEMBRANCE:
		{
			(void)restore_level();
			break;
		}

		case PRAYER_DISPEL_UNDEAD2:
		{
			(void)dispel_undead(randint1(plev * 4));
			break;
		}

		case PRAYER_DISPEL_EVIL2:
		{
			(void)dispel_evil(randint1(plev * 4));
			break;
		}

		case PRAYER_BANISH_EVIL:
		{
			if (banish_evil(100))
			{
				msg("The power of your god banishes evil!");
			}
			break;
		}

		case PRAYER_WORD_OF_DESTRUCTION:
		{
			destroy_area(py, px, 15, TRUE);
			break;
		}

		case PRAYER_ANNIHILATION:
		{
			drain_life(dir, 200);
			break;
		}

		case PRAYER_UNBARRING_WAYS:
		{
			(void)destroy_doors_touch();
			break;
		}

		case PRAYER_RECHARGING:
		{
			return recharge(15);
		}

		case PRAYER_DISPEL_CURSE:
		{
			(void)remove_all_curse();
			break;
		}

		case PRAYER_ENCHANT_WEAPON:
		{
			return enchant_spell(randint0(4) + 1, randint0(4) + 1, 0);
		}

		case PRAYER_ENCHANT_ARMOUR:
		{
			return enchant_spell(0, 0, randint0(3) + 2);
		}

		case PRAYER_ELEMENTAL_BRAND:
		{
			brand_weapon();
			break;
		}

		case PRAYER_BLINK:
		{
			teleport_player(10);
			break;
		}

		case PRAYER_TELEPORT_SELF:
		{
			teleport_player(plev * 8);
			break;
		}

		case PRAYER_TELEPORT_OTHER:
		{
			(void)teleport_monster(dir);
			break;
		}

		case PRAYER_TELEPORT_LEVEL:
		{
			(void)teleport_player_level();
			break;
		}

		case PRAYER_WORD_OF_RECALL:
		{
			set_recall();
			break;
		}

		case PRAYER_ALTER_REALITY:
		{
			msg("The world changes!");

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}
	}

	/* Success */
	return (TRUE);
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
