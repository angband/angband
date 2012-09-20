/* File: x-spell.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */


#include "angband.h"
#include "script.h"

#ifndef USE_SCRIPT


/*
 * Maximum number of spells per realm
 */
#define BOOKS_PER_REALM 10


#define SPELL_MAGIC_MISSILE             0
#define SPELL_DETECT_MONSTERS           1
#define SPELL_PHASE_DOOR                2
#define SPELL_LIGHT_AREA                3
#define SPELL_FIND_TRAPS_DOORS          4
#define SPELL_CURE_LIGHT_WOUNDS         5
#define SPELL_TREASURE_DETECTION        6
#define SPELL_OBJECT_DETECTION          7
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
#define PRAYER_FIND_TRAPS               5
#define PRAYER_DETECT_DOORS_STAIRS      6
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


/*
 * Spells in each book (mage spells then priest spells)
 */
static const s16b spell_list[2][BOOKS_PER_REALM][SPELLS_PER_BOOK] =
{
	/* Mage spells */
	{
		/* Magic for Beginners */
		{
			SPELL_MAGIC_MISSILE,
			SPELL_DETECT_MONSTERS,
			SPELL_PHASE_DOOR,
			SPELL_LIGHT_AREA,
			SPELL_TREASURE_DETECTION,
			SPELL_CURE_LIGHT_WOUNDS,
			SPELL_OBJECT_DETECTION,
			SPELL_FIND_TRAPS_DOORS,
			SPELL_STINKING_CLOUD,
		},

		/* Conjurings and Tricks */
		{
			SPELL_CONFUSE_MONSTER,
			SPELL_LIGHTNING_BOLT,
			SPELL_TRAP_DOOR_DESTRUCTION,
			SPELL_CURE_POISON,
			SPELL_SLEEP_MONSTER,
			SPELL_TELEPORT_SELF,
			SPELL_SPEAR_OF_LIGHT,
			SPELL_FROST_BOLT,
			SPELL_WONDER,
		},

		/* Incantations and Illusions */
		{
			SPELL_SATISFY_HUNGER,
			SPELL_RECHARGE_ITEM_I,
			SPELL_TURN_STONE_TO_MUD,
			SPELL_FIRE_BOLT,
			SPELL_POLYMORPH_OTHER,
			SPELL_IDENTIFY,
			SPELL_DETECT_INVISIBLE,
			SPELL_ACID_BOLT,
			SPELL_SLOW_MONSTER,
		},

		/* Sorcery and Evocations */
		{
			SPELL_FROST_BALL,
			SPELL_TELEPORT_OTHER,
			SPELL_HASTE_SELF,
			SPELL_MASS_SLEEP,
			SPELL_FIRE_BALL,
			SPELL_DETECT_ENCHANTMENT,
			-1,
			-1,
			-1,
		},

		/* Resistances of Scarabtarices */
		{
			SPELL_RESIST_COLD,
			SPELL_RESIST_FIRE,
			SPELL_RESIST_POISON,
			SPELL_RESISTANCE,
			SPELL_SHIELD,
			-1,
			-1,
			-1,
			-1,
		},

		/* Raal's Tome of Destruction */
		{
			SPELL_SHOCK_WAVE,
			SPELL_EXPLOSION,
			SPELL_CLOUD_KILL,
			SPELL_ACID_BALL,
			SPELL_ICE_STORM,
			SPELL_METEOR_SWARM,
			SPELL_RIFT,
			-1,
			-1,
		},

		/* Mordenkainen's Escapes */
		{
			SPELL_DOOR_CREATION,
			SPELL_STAIR_CREATION,
			SPELL_TELEPORT_LEVEL,
			SPELL_WORD_OF_RECALL,
			SPELL_RUNE_OF_PROTECTION,
			-1,
			-1,
			-1,
			-1,
		},

		/* Tenser's transformations */
		{
			SPELL_HEROISM,
			SPELL_BERSERKER,
			SPELL_ENCHANT_ARMOR,
			SPELL_ENCHANT_WEAPON,
			SPELL_RECHARGE_ITEM_II,
			SPELL_ELEMENTAL_BRAND,
			-1,
			-1,
			-1,
		},

		/* Kelek's Grimoire of Power */
		{
			SPELL_EARTHQUAKE,
			SPELL_BEDLAM,
			SPELL_REND_SOUL,
			SPELL_BANISHMENT,
			SPELL_WORD_OF_DESTRUCTION,
			SPELL_MASS_BANISHMENT,
			SPELL_CHAOS_STRIKE,
			SPELL_MANA_STORM,
			-1,
		},
	},

	/* Priest spells */
	{
		/* Beginners Handbook */
		{
			PRAYER_DETECT_EVIL,
			PRAYER_CURE_LIGHT_WOUNDS,
			PRAYER_BLESS,
			PRAYER_REMOVE_FEAR,
			PRAYER_CALL_LIGHT,
			PRAYER_FIND_TRAPS,
			PRAYER_DETECT_DOORS_STAIRS,
			PRAYER_SLOW_POISON,
			-1,
		},

		/* Words of Wisdom */
		{
			PRAYER_SCARE_MONSTER,
			PRAYER_PORTAL,
			PRAYER_CURE_SERIOUS_WOUNDS,
			PRAYER_CHANT,
			PRAYER_SANCTUARY,
			PRAYER_SATISFY_HUNGER,
			PRAYER_REMOVE_CURSE,
			PRAYER_RESIST_HEAT_COLD,
			-1,
		},

		/* Chants and Blessings */
		{
			PRAYER_NEUTRALIZE_POISON,
			PRAYER_ORB_OF_DRAINING,
			PRAYER_CURE_CRITICAL_WOUNDS,
			PRAYER_SENSE_INVISIBLE,
			PRAYER_PROTECTION_FROM_EVIL,
			PRAYER_EARTHQUAKE,
			PRAYER_SENSE_SURROUNDINGS,
			PRAYER_CURE_MORTAL_WOUNDS,
			PRAYER_TURN_UNDEAD,
		},

		/* Exorcism and Dispelling */
		{
			PRAYER_PRAYER,
			PRAYER_DISPEL_UNDEAD,
			PRAYER_HEAL,
			PRAYER_DISPEL_EVIL,
			PRAYER_GLYPH_OF_WARDING,
			PRAYER_HOLY_WORD,
			-1,
			-1,
			-1,
		},

		/* Ethereal openings */
		{
			PRAYER_BLINK,
			PRAYER_TELEPORT_SELF,
			PRAYER_TELEPORT_OTHER,
			PRAYER_TELEPORT_LEVEL,
			PRAYER_WORD_OF_RECALL,
			PRAYER_ALTER_REALITY,
			-1,
			-1,
			-1,
		},

		/* Godly Insights */
		{
			PRAYER_DETECT_MONSTERS,
			PRAYER_DETECTION,
			PRAYER_PERCEPTION,
			PRAYER_PROBING,
			PRAYER_CLAIRVOYANCE,
			-1,
			-1,
			-1,
			-1,
		},

		/* Purifications and Healing */
		{
			PRAYER_CURE_SERIOUS_WOUNDS2,
			PRAYER_CURE_MORTAL_WOUNDS2,
			PRAYER_HEALING,
			PRAYER_RESTORATION,
			PRAYER_REMEMBRANCE,
			-1,
			-1,
			-1,
			-1,
		},

		/* Holy Infusions */
		{
			PRAYER_UNBARRING_WAYS,
			PRAYER_RECHARGING,
			PRAYER_DISPEL_CURSE,
			PRAYER_ENCHANT_WEAPON,
			PRAYER_ENCHANT_ARMOUR,
			PRAYER_ELEMENTAL_BRAND,
			-1,
			-1,
			-1,
		},

		/* Wrath of God */
		{
			PRAYER_DISPEL_UNDEAD2,
			PRAYER_DISPEL_EVIL2,
			PRAYER_BANISH_EVIL,
			PRAYER_WORD_OF_DESTRUCTION,
			PRAYER_ANNIHILATION,
			-1,
			-1,
			-1,
			-1,
		}
	}
};


/*
 * Names of the spells (mage spells then priest spells)
 */
static cptr spell_names[2][PY_MAX_SPELLS] =
{
	/*** Mage Spells ***/

	{
	  "Magic Missile",
	  "Detect Monsters",
	  "Phase Door",
	  "Light Area",
	  "Find Hidden Traps/Doors",
	  "Cure Light Wounds",
	  "Detect Treasure",
	  "Detect Objects",
	  "Identify",
	  "Detect Invisible",
	  "Detect Enchantment",
	  "Stinking Cloud",
	  "Lightning Bolt",
	  "Confuse Monster",
	  "Sleep Monster",
	  "Wonder",
	  "Frost Bolt",
	  "Acid Bolt",
	  "Fire Bolt",
	  "Trap/Door Destruction",
	  "Spear of Light",
	  "Turn Stone to Mud",
	  "Door Creation",
	  "Earthquake",
	  "Stair Creation",
	  "Cure Poison",
	  "Satisfy Hunger",
	  "Heroism",
	  "Berserker",
	  "Haste Self",
	  "Teleport Self",
	  "Slow Monster",
	  "Teleport Other",
	  "Teleport Level",
	  "Word of Recall",
	  "Polymorph Other",
	  "Shock Wave",
	  "Explosion",
	  "Cloudkill",
	  "Mass Sleep",
	  "Bedlam",
	  "Rend Soul",
	  "Word of Destruction",
	  "Chaos Strike",
	  "Resist Cold",
	  "Resist Fire",
	  "Resist Poison",
	  "Resistance",
	  "Shield",
	  "Rune of Protection",
	  "Lesser Recharging",
	  "Enchant Armor",
	  "Enchant Weapon",
	  "Greater Recharging",
	  "Elemental Brand",
	  "Frost Ball",
	  "Acid Ball",
	  "Fire Ball",
	  "Ice Storm",
	  "Banishment",
	  "Meteor Swarm",
	  "Mass Banishment",
	  "Rift",
	  "Mana Storm"
	},


	/*** Priest Spells ***/

	{
		/* Beginners Handbook (sval 0) */
		"Detect Evil",
		"Cure Light Wounds",
		"Bless",
		"Remove Fear",
		"Call Light",
		"Find Traps",
		"Detect Doors/Stairs",
		"Slow Poison",

		/* Words of Wisdom (sval 1) */
		"Scare Monster",
		"Portal",
		"Cure Serious Wounds",
		"Chant",
		"Sanctuary",
		"Satisfy Hunger",
		"Remove Curse",
		"Resist Heat and Cold",

		/* Chants and Blessings (sval 2) */
		"Neutralize Poison",
		"Orb of Draining",
		"Cure Critical Wounds",
		"Sense Invisible",
		"Protection from Evil",
		"Earthquake",
		"Sense Surroundings",
		"Cure Mortal Wounds",
		"Turn Undead",

		/* Exorcism and Dispelling (sval 3) */
		"Prayer",
		"Dispel Undead",
		"Heal",
		"Dispel Evil",
		"Glyph of Warding",
		"Holy Word",

		/* Godly Insights... (sval 5) */
		"Detect Monsters",
		"Detection",
		"Perception",
		"Probing",
		"Clairvoyance",

		/* Purifications and Healing (sval 6) */
		"Cure Serious Wounds",
		"Cure Mortal Wounds",
		"Healing",
		"Restoration",
		"Remembrance",

		/* Wrath of God (sval 8) */
		"Dispel Undead",
		"Dispel Evil",
		"Banish Evil",
		"Word of Destruction",
		"Annihilation",

		/* Holy Infusions (sval 7) */
		"Unbarring Ways",
		"Recharging",
		"Dispel Curse",
		"Enchant Weapon",
		"Enchant Armour",
		"Elemental Brand",

		/* Ethereal openings (sval 4) */
		"Blink",
		"Teleport Self",
		"Teleport Other",
		"Teleport Level",
		"Word of Recall",
		"Alter Reality",

		"(blank)",
		"(blank)",
		"(blank)",
		"(blank)",
		"(blank)",
		"(blank)"
	}
};


int get_spell_index(const object_type *o_ptr, int index)
{
	int spell;
	int num = 0;

	/* Get the item's sval */
	int sval = o_ptr->sval;

	int spell_type;

	/* Check bounds */
	if ((index < 0) || (index >= SPELLS_PER_BOOK)) return (-1);
	if ((sval < 0) || (sval >= BOOKS_PER_REALM)) return (-1);

	/* Mage or priest spells? */
	if (cp_ptr->spell_book == TV_MAGIC_BOOK)
		spell_type = 0;
	else
		spell_type = 1;

	return spell_list[spell_type][sval][index];
}


cptr get_spell_name(int tval, int spell)
{
	if (tval == TV_MAGIC_BOOK)
		return spell_names[0][spell];
	else
		return spell_names[1][spell];
}


cptr get_spell_info(int tval, int spell)
{
	static char p[80];

	/* Default */
	strcpy(p, "");

	/* Mage spells */
	if (tval == TV_MAGIC_BOOK)
	{
		int plev = p_ptr->lev;

		/* Analyze the spell */
		switch (spell)
		{
		case SPELL_MAGIC_MISSILE:
			sprintf(p, " dam %dd4", 3 + ((plev - 1) / 5));
			break;
		case SPELL_PHASE_DOOR:
			sprintf(p, " range 10");
			break;
		case SPELL_CURE_LIGHT_WOUNDS:
			sprintf(p, " heal 2d8");
			break;
		case SPELL_STINKING_CLOUD:
			sprintf(p, " dam %d", 10 + (plev / 2));
			break;
		case SPELL_LIGHTNING_BOLT:
			sprintf(p, " dam %dd6", (3 + ((plev - 5) / 6)));
			break;
		case SPELL_FROST_BOLT:
			sprintf(p, " dam %dd8", (5 + ((plev - 5) / 4)));
			break;
		case SPELL_ACID_BOLT:
			sprintf(p, " dam %dd8", (8 + ((plev - 5) / 4)));
			break;
		case SPELL_FIRE_BOLT:
			sprintf(p, " dam %dd8", (6 + ((plev - 5) / 4)));
			break;
		case SPELL_SPEAR_OF_LIGHT:
			sprintf(p, " dam 6d8");
			break;
		case SPELL_HEROISM:
			sprintf(p, " dur 25+d25");
			break;
		case SPELL_BERSERKER:
			sprintf(p, " dur 25+d25");
			break;
		case SPELL_HASTE_SELF:
			sprintf(p, " dur %d+d20", plev);
			break;
		case SPELL_TELEPORT_SELF:
			sprintf(p, " range %d", plev * 5);
			break;
		case SPELL_SHOCK_WAVE:
			sprintf(p, " dam %d", 10 + plev);
			break;
		case SPELL_EXPLOSION:
			sprintf(p, " dam %d", 20 + plev * 2);
			break;
		case SPELL_CLOUD_KILL:
			sprintf(p, " dam %d", 40 + (plev / 2));
			break;
		case SPELL_REND_SOUL:
			sprintf(p, " dam 11d%d", plev);
			break;
		case SPELL_CHAOS_STRIKE:
			sprintf(p, " dam 13d%d", plev);
			break;
		case SPELL_RESIST_COLD:
			sprintf(p, " dur 20+d20");
			break;
		case SPELL_RESIST_FIRE:
			sprintf(p, " dur 20+d20");
			break;
		case SPELL_RESIST_POISON:
			sprintf(p, " dur 20+d20");
			break;
		case SPELL_RESISTANCE:
			sprintf(p, " dur 20+d20");
			break;
		case SPELL_SHIELD:
			sprintf(p, " dur 30+d20");
			break;
		case SPELL_FROST_BALL:
			sprintf(p, " dam %d", 30 + plev);
			break;
		case SPELL_ACID_BALL:
			sprintf(p, " dam %d", 40 + plev);
			break;
		case SPELL_FIRE_BALL:
			sprintf(p, " dam %d", 55 + plev);
			break;
		case SPELL_ICE_STORM:
			sprintf(p, " dam %d", 50 + (plev * 2));
			break;
		case SPELL_METEOR_SWARM:
			sprintf(p, " dam %dx%d", 30 + plev / 2, 2 + plev / 20);
			break;
		case SPELL_RIFT:
			sprintf(p, " dam 40+%dd7", plev);
			break;
		case SPELL_MANA_STORM:
			sprintf(p, " dam %d", 300 + plev * 2);
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
				strcpy(p, " heal 2d10");
				break;
			case PRAYER_BLESS:
				strcpy(p, " dur 12+d12");
				break;
			case PRAYER_PORTAL:
				sprintf(p, " range %d", 3 * plev);
				break;
			case PRAYER_CURE_SERIOUS_WOUNDS:
				strcpy(p, " heal 4d10");
				break;
			case PRAYER_CHANT:
				strcpy(p, " dur 24+d24");
				break;
			case PRAYER_RESIST_HEAT_COLD:
				strcpy(p, " dur 10+d10");
				break;
			case PRAYER_ORB_OF_DRAINING:
				sprintf(p, " %d+3d6", plev +
				        (plev / ((cp_ptr->flags & CF_BLESS_WEAPON) ? 2 : 4)));
				break;
			case PRAYER_CURE_CRITICAL_WOUNDS:
				strcpy(p, " heal 6d10");
				break;
			case PRAYER_SENSE_INVISIBLE:
				strcpy(p, " dur 24+d24");
				break;
			case PRAYER_PROTECTION_FROM_EVIL:
				sprintf(p, " dur %d+d25", 3 * plev);
				break;
			case PRAYER_CURE_MORTAL_WOUNDS:
				strcpy(p, " heal 8d10");
				break;
			case PRAYER_PRAYER:
				strcpy(p, " dur 48+d48");
				break;
			case PRAYER_DISPEL_UNDEAD:
				sprintf(p, " dam d%d", 3 * plev);
				break;
			case PRAYER_HEAL:
				strcpy(p, " heal 300");
				break;
			case PRAYER_DISPEL_EVIL:
				sprintf(p, " dam d%d", 3 * plev);
				break;
			case PRAYER_HOLY_WORD:
				strcpy(p, " heal 1000");
				break;
			case PRAYER_CURE_SERIOUS_WOUNDS2:
				strcpy(p, " heal 4d10");
				break;
			case PRAYER_CURE_MORTAL_WOUNDS2:
				strcpy(p, " heal 8d10");
				break;
			case PRAYER_HEALING:
				strcpy(p, " heal 2000");
				break;
			case PRAYER_DISPEL_UNDEAD2:
				sprintf(p, " dam d%d", 4 * plev);
				break;
			case PRAYER_DISPEL_EVIL2:
				sprintf(p, " dam d%d", 4 * plev);
				break;
			case PRAYER_ANNIHILATION:
				strcpy(p, " dam 200");
				break;
			case PRAYER_BLINK:
				strcpy(p, " range 10");
				break;
			case PRAYER_TELEPORT_SELF:
				sprintf(p, " range %d", 8 * plev);
				break;
		}
	}

	return (p);
}


static int beam_chance(void)
{
	int plev = p_ptr->lev;
	return ((cp_ptr->flags & CF_BEAM) ? plev : (plev / 2));
}


static void spell_wonder(int dir)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */

	int py = p_ptr->py;
	int px = p_ptr->px;
	int plev = p_ptr->lev;
	int die = randint(100) + plev / 5;
	int beam = beam_chance();

	if (die > 100)
		msg_print("You feel a surge of power!");
	if (die < 8) clone_monster(dir);
	else if (die < 14) speed_monster(dir);
	else if (die < 26) heal_monster(dir);
	else if (die < 31) poly_monster(dir);
	else if (die < 36)
		fire_bolt_or_beam(beam - 10, GF_MISSILE, dir,
		                  damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) confuse_monster(dir, plev);
	else if (die < 46) fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) lite_line(dir);
	else if (die < 56)
		fire_beam(GF_ELEC, dir, damroll(3+((plev-5)/6), 6));
	else if (die < 61)
		fire_bolt_or_beam(beam-10, GF_COLD, dir,
		                  damroll(5+((plev-5)/4), 8));
	else if (die < 66)
		fire_bolt_or_beam(beam, GF_ACID, dir,
		                  damroll(6+((plev-5)/4), 8));
	else if (die < 71)
		fire_bolt_or_beam(beam, GF_FIRE, dir,
		                  damroll(8+((plev-5)/4), 8));
	else if (die < 76) drain_life(dir, 75);
	else if (die < 81) fire_ball(GF_ELEC, dir, 30 + plev / 2, 2);
	else if (die < 86) fire_ball(GF_ACID, dir, 40 + plev, 2);
	else if (die < 91) fire_ball(GF_ICE, dir, 70 + plev, 3);
	else if (die < 96) fire_ball(GF_FIRE, dir, 80 + plev, 3);
	else if (die < 101) drain_life(dir, 100 + plev);
	else if (die < 104) earthquake(py, px, 12);
	else if (die < 106) destroy_area(py, px, 15, TRUE);
	else if (die < 108) banishment();
	else if (die < 110) dispel_monsters(120);
	else /* RARE */
	{
		dispel_monsters(150);
		slow_monsters();
		sleep_monsters();
		hp_player(300);
	}
}



static bool cast_mage_spell(int spell)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int dir;

	int plev = p_ptr->lev;

	/* Hack -- chance of "beam" instead of "bolt" */
	int beam = beam_chance();

	/* Spells. */
	switch (spell)
	{
		case SPELL_MAGIC_MISSILE:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam-10, GF_MISSILE, dir,
			                  damroll(3 + ((plev - 1) / 5), 4));
			break;
		}

		case SPELL_DETECT_MONSTERS:
		{
			(void)detect_monsters_normal();
			break;
		}

		case SPELL_PHASE_DOOR:
		{
			teleport_player(10);
			break;
		}

		case SPELL_LIGHT_AREA:
		{
			(void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
			break;
		}

		case SPELL_TREASURE_DETECTION:
		{
			(void)detect_treasure();
			(void)detect_objects_gold();
			break;
		}

		case SPELL_CURE_LIGHT_WOUNDS:
		{

			(void)hp_player(damroll(2, 8));
			(void)set_cut(p_ptr->cut - 15);
			break;
		}

		case SPELL_OBJECT_DETECTION:
		{
			(void)detect_objects_normal();
			break;
		}

		case SPELL_FIND_TRAPS_DOORS:
		{
			(void)detect_traps();
			(void)detect_doors();
			(void)detect_stairs();
			break;
		}

		case SPELL_STINKING_CLOUD:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_POIS, dir, 10 + (plev / 2), 2);
			break;
		}

		case SPELL_CONFUSE_MONSTER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)confuse_monster(dir, plev);
			break;
		}

		case SPELL_LIGHTNING_BOLT:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_beam(GF_ELEC, dir,
			          damroll(3+((plev-5)/6), 6));
			break;
		}

		case SPELL_TRAP_DOOR_DESTRUCTION:
		{
			(void)destroy_doors_touch();
			break;
		}

		case SPELL_SLEEP_MONSTER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)sleep_monster(dir);
			break;
		}

		case SPELL_CURE_POISON:
		{
			(void)set_poisoned(0);
			break;
		}

		case SPELL_TELEPORT_SELF:
		{
			teleport_player(plev * 5);
			break;
		}

		case SPELL_SPEAR_OF_LIGHT: /* spear of light */
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			msg_print("A line of blue shimmering light appears.");
			lite_line(dir);
			break;
		}

		case SPELL_FROST_BOLT:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam-10, GF_COLD, dir,
			                  damroll(5+((plev-5)/4), 8));
			break;
		}

		case SPELL_TURN_STONE_TO_MUD:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)wall_to_mud(dir);
			break;
		}

		case SPELL_SATISFY_HUNGER:
		{
			(void)set_food(PY_FOOD_MAX - 1);
			break;
		}

		case SPELL_RECHARGE_ITEM_I:
		{
			return recharge(2 + plev / 5);
		}

		case SPELL_WONDER: /* wonder */
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)spell_wonder(dir);
			break;
		}

		case SPELL_POLYMORPH_OTHER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)poly_monster(dir);
			break;
		}

		case SPELL_IDENTIFY:
		{
			return ident_spell();
		}

		case SPELL_MASS_SLEEP:
		{
			(void)sleep_monsters();
			break;
		}

		case SPELL_FIRE_BOLT:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam, GF_FIRE, dir,
			                  damroll(6+((plev-5)/4), 8));
			break;
		}

		case SPELL_SLOW_MONSTER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)slow_monster(dir);
			break;
		}

		case SPELL_FROST_BALL:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_COLD, dir, 30 + (plev), 2);
			break;
		}

		case SPELL_RECHARGE_ITEM_II: /* greater recharging */
		{
			return recharge(50 + plev);
		}

		case SPELL_TELEPORT_OTHER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)teleport_monster(dir);
			break;
		}

		case SPELL_BEDLAM:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_OLD_CONF, dir, plev, 4);
			break;
		}

		case SPELL_FIRE_BALL:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
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
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam, GF_ACID, dir, damroll(8+((plev-5)/4), 8));
			break;
		}

		case SPELL_CLOUD_KILL:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_POIS, dir, 40 + (plev / 2), 3);
			break;
		}

		case SPELL_ACID_BALL:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_ACID, dir, 40 + (plev), 2);
			break;
		}

		case SPELL_ICE_STORM:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_ICE, dir, 50 + (plev * 2), 3);
			break;
		}

		case SPELL_METEOR_SWARM:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_swarm(2 + plev / 20, GF_METEOR, dir, 30 + plev / 2, 1);
			break;
		}

		case SPELL_MANA_STORM:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_MANA, dir, 300 + (plev * 2), 3);
			break;
		}
		case SPELL_DETECT_INVISIBLE:
		{
			(void)detect_monsters_invis();
			break;
		}

		case SPELL_DETECT_ENCHANTMENT:
		{
			(void)detect_objects_magic();
			break;
		}

		case SPELL_SHOCK_WAVE:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_SOUND, dir, 10 + plev, 2);
			break;
		}

		case SPELL_EXPLOSION:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
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
			(void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);
			break;
		}

		case SPELL_RESIST_COLD:
		{
			(void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);
			break;
		}

		case SPELL_ELEMENTAL_BRAND: /* elemental brand */
		{
			return brand_ammo();
		}

		case SPELL_RESIST_POISON:
		{
			(void)set_oppose_pois(p_ptr->oppose_pois + randint(20) + 20);
			break;
		}

		case SPELL_RESISTANCE:
		{
			int time = randint(20) + 20;
			(void)set_oppose_acid(p_ptr->oppose_acid + time);
			(void)set_oppose_elec(p_ptr->oppose_elec + time);
			(void)set_oppose_fire(p_ptr->oppose_fire + time);
			(void)set_oppose_cold(p_ptr->oppose_cold + time);
			(void)set_oppose_pois(p_ptr->oppose_pois + time);
			break;
		}

		case SPELL_HEROISM:
		{
			(void)hp_player(10);
			(void)set_hero(p_ptr->hero + randint(25) + 25);
			(void)set_afraid(0);
			break;
		}

		case SPELL_SHIELD:
		{
			(void)set_shield(p_ptr->shield + randint(20) + 30);
			break;
		}

		case SPELL_BERSERKER:
		{
			(void)hp_player(30);
			(void)set_shero(p_ptr->shero + randint(25) + 25);
			(void)set_afraid(0);
			break;
		}

		case SPELL_HASTE_SELF:
		{
			if (!p_ptr->fast)
			{
				(void)set_fast(randint(20) + plev);
			}
			else
			{
				(void)set_fast(p_ptr->fast + randint(5));
			}
			break;
		}

		case SPELL_RIFT:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_beam(GF_GRAVITY, dir,	40 + damroll(plev, 7));
			break;
		}

		case SPELL_REND_SOUL: /* rend soul */
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam / 4, GF_NETHER, dir, damroll(11, plev));
			break;
		}

		case SPELL_CHAOS_STRIKE: /* chaos strike */
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_bolt_or_beam(beam, GF_CHAOS, dir, damroll(13, plev));
			break;
		}

		case SPELL_RUNE_OF_PROTECTION: /* rune of protection */
		{
			(void)warding_glyph();
			break;
		}

		case SPELL_ENCHANT_ARMOR: /* enchant armor */
		{
			return enchant_spell(0, 0, rand_int(3) + plev / 20);
		}

		case SPELL_ENCHANT_WEAPON: /* enchant weapon */
		{
			return enchant_spell(rand_int(4) + plev / 20,
			                     rand_int(4) + plev / 20, 0);
		}
	}

	/* Success */
	return (TRUE);
}


static bool cast_priest_spell(int spell)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int dir;

	int plev = p_ptr->lev;

	switch (spell)
	{
		case PRAYER_DETECT_EVIL:
		{
			(void)detect_monsters_evil();
			break;
		}

		case PRAYER_CURE_LIGHT_WOUNDS:
		{
			(void)hp_player(damroll(2, 10));
			(void)set_cut(p_ptr->cut - 10);
			break;
		}

		case PRAYER_BLESS:
		{
			(void)set_blessed(p_ptr->blessed + randint(12) + 12);
			break;
		}

		case PRAYER_REMOVE_FEAR:
		{
			(void)set_afraid(0);
			break;
		}

		case PRAYER_CALL_LIGHT:
		{
			(void)lite_area(damroll(2, (plev / 2)), (plev / 10) + 1);
			break;
		}

		case PRAYER_FIND_TRAPS:
		{
			(void)detect_traps();
			break;
		}

		case PRAYER_DETECT_DOORS_STAIRS:
		{
			(void)detect_doors();
			(void)detect_stairs();
			break;
		}

		case PRAYER_SLOW_POISON:
		{
			(void)set_poisoned(p_ptr->poisoned / 2);
			break;
		}

		case PRAYER_SCARE_MONSTER:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			(void)fear_monster(dir, plev);
			break;
		}

		case PRAYER_PORTAL:
		{
			teleport_player(plev * 3);
			break;
		}

		case PRAYER_CURE_SERIOUS_WOUNDS:
		{
			(void)hp_player(damroll(4, 10));
			(void)set_cut((p_ptr->cut / 2) - 20);
			break;
		}

		case PRAYER_CHANT:
		{
			(void)set_blessed(p_ptr->blessed + randint(24) + 24);
			break;
		}

		case PRAYER_SANCTUARY:
		{
			(void)sleep_monsters_touch();
			break;
		}

		case PRAYER_SATISFY_HUNGER:
		{
			(void)set_food(PY_FOOD_MAX - 1);
			break;
		}

		case PRAYER_REMOVE_CURSE:
		{
			remove_curse();
			break;
		}

		case PRAYER_RESIST_HEAT_COLD:
		{
			(void)set_oppose_fire(p_ptr->oppose_fire + randint(10) + 10);
			(void)set_oppose_cold(p_ptr->oppose_cold + randint(10) + 10);
			break;
		}

		case PRAYER_NEUTRALIZE_POISON:
		{
			(void)set_poisoned(0);
			break;
		}

		case PRAYER_ORB_OF_DRAINING:
		{
			if (!get_aim_dir(&dir)) return (FALSE);
			fire_ball(GF_HOLY_ORB, dir,
			          (damroll(3, 6) + plev +
			           (plev / ((cp_ptr->flags & CF_BLESS_WEAPON) ? 2 : 4))),
			          ((plev < 30) ? 2 : 3));
			break;
		}

		case PRAYER_CURE_CRITICAL_WOUNDS:
		{
			(void)hp_player(damroll(6, 10));
			(void)set_cut(0);
			break;
		}

		case PRAYER_SENSE_INVISIBLE:
		{
			(void)set_tim_invis(p_ptr->tim_invis + randint(24) + 24);
			break;
		}

		case PRAYER_PROTECTION_FROM_EVIL:
		{
			(void)set_protevil(p_ptr->protevil + randint(25) + 3 * p_ptr->lev);
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
			(void)hp_player(damroll(8, 10));
			(void)set_stun(0);
			(void)set_cut(0);
			break;
		}

		case PRAYER_TURN_UNDEAD:
		{
			(void)turn_undead();
			break;
		}

		case PRAYER_PRAYER:
		{
			(void)set_blessed(p_ptr->blessed + randint(48) + 48);
			break;
		}

		case PRAYER_DISPEL_UNDEAD:
		{
			(void)dispel_undead(randint(plev * 3));
			break;
		}

		case PRAYER_HEAL:
		{
			(void)hp_player(300);
			(void)set_stun(0);
			(void)set_cut(0);
			break;
		}

		case PRAYER_DISPEL_EVIL:
		{
			(void)dispel_evil(randint(plev * 3));
			break;
		}

		case PRAYER_GLYPH_OF_WARDING:
		{
			warding_glyph();
			break;
		}

		case PRAYER_HOLY_WORD:
		{
			(void)dispel_evil(randint(plev * 4));
			(void)hp_player(1000);
			(void)set_afraid(0);
			(void)set_poisoned(0);
			(void)set_stun(0);
			(void)set_cut(0);
			break;
		}

		case PRAYER_DETECT_MONSTERS:
		{
			(void)detect_monsters_normal();
			break;
		}

		case PRAYER_DETECTION:
		{
			(void)detect_all();
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
			wiz_lite();
			break;
		}

		case PRAYER_CURE_SERIOUS_WOUNDS2:
		{
			(void)hp_player(damroll(4, 10));
			(void)set_cut(0);
			break;
		}

		case PRAYER_CURE_MORTAL_WOUNDS2:
		{
			(void)hp_player(damroll(8, 10));
			(void)set_stun(0);
			(void)set_cut(0);
			break;
		}

		case PRAYER_HEALING:
		{
			(void)hp_player(2000);
			(void)set_stun(0);
			(void)set_cut(0);
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
			(void)dispel_undead(randint(plev * 4));
			break;
		}

		case PRAYER_DISPEL_EVIL2:
		{
			(void)dispel_evil(randint(plev * 4));
			break;
		}

		case PRAYER_BANISH_EVIL:
		{
			if (banish_evil(100))
			{
				msg_print("The power of your god banishes evil!");
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
			if (!get_aim_dir(&dir)) return (FALSE);
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
			return enchant_spell(rand_int(4) + 1, rand_int(4) + 1, 0);
		}

		case PRAYER_ENCHANT_ARMOUR:
		{
			return enchant_spell(0, 0, rand_int(3) + 2);
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
			if (!get_aim_dir(&dir)) return (FALSE);
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
			msg_print("The world changes!");

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}
	}

	/* Success */
	return (TRUE);
}


bool cast_spell(int tval, int index)
{
	if (tval == TV_MAGIC_BOOK)
	{
		return cast_mage_spell(index);
	}
	else
	{
		return cast_priest_spell(index);
	}
}

#endif /* USE_SCRIPT */
