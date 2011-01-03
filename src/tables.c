/*
 * File: tables.c
 * Purpose: Finely-tuned constants for the game Angband
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
#include "object/tvalsval.h"
#include "slays.h"


/*
 * Global array for looping through the "keypad directions".
 */
const s16b ddd[9] =
{ 2, 8, 6, 4, 3, 1, 9, 7, 5 };

/*
 * Global arrays for converting "keypad direction" into "offsets".
 */
const s16b ddx[10] =
{ 0, -1, 0, 1, -1, 0, 1, -1, 0, 1 };

const s16b ddy[10] =
{ 0, 1, 1, 1, 0, 0, 0, -1, -1, -1 };

/*
 * Global arrays for optimizing "ddx[ddd[i]]" and "ddy[ddd[i]]".
 */
const s16b ddx_ddd[9] =
{ 0, 0, 1, -1, 1, -1, 1, -1, 0 };

const s16b ddy_ddd[9] =
{ 1, -1, 0, 0, 1, 1, -1, -1, 0 };


/*
 * Global array for converting numbers to uppercase hecidecimal digit
 * This array can also be used to convert a number to an octal digit
 */
const char hexsym[16] =
{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


/*
 * Stat Table (INT/WIS) -- Number of 1/100 spells per level
 */
const int adj_mag_study[STAT_RANGE] =
{
	  0	/* 3 */,
	  0	/* 4 */,
	 10	/* 5 */,
	 20	/* 6 */,
	 30	/* 7 */,
	 40	/* 8 */,
	 50	/* 9 */,
	 60	/* 10 */,
	 70	/* 11 */,
	 80	/* 12 */,
	 85	/* 13 */,
	 90	/* 14 */,
	 95	/* 15 */,
	100	/* 16 */,
	105	/* 17 */,
	110	/* 18/00-18/09 */,
	115	/* 18/10-18/19 */,
	120	/* 18/20-18/29 */,
	130	/* 18/30-18/39 */,
	140	/* 18/40-18/49 */,
	150	/* 18/50-18/59 */,
	160	/* 18/60-18/69 */,
	170	/* 18/70-18/79 */,
	180	/* 18/80-18/89 */,
	190	/* 18/90-18/99 */,
	200	/* 18/100-18/109 */,
	210	/* 18/110-18/119 */,
	220	/* 18/120-18/129 */,
	230	/* 18/130-18/139 */,
	240	/* 18/140-18/149 */,
	250	/* 18/150-18/159 */,
	250	/* 18/160-18/169 */,
	250	/* 18/170-18/179 */,
	250	/* 18/180-18/189 */,
	250	/* 18/190-18/199 */,
	250	/* 18/200-18/209 */,
	250	/* 18/210-18/219 */,
	250	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- extra 1/100 mana-points per level
 */
const int adj_mag_mana[STAT_RANGE] =
{
	  0	/* 3 */,
	 10	/* 4 */,
	 20	/* 5 */,
	 30	/* 6 */,
	 40	/* 7 */,
	 50	/* 8 */,
	 60	/* 9 */,
	 70	/* 10 */,
	 80	/* 11 */,
	 90	/* 12 */,
	100	/* 13 */,
	110	/* 14 */,
	120	/* 15 */,
	130	/* 16 */,
	140	/* 17 */,
	150	/* 18/00-18/09 */,
	160	/* 18/10-18/19 */,
	170	/* 18/20-18/29 */,
	180	/* 18/30-18/39 */,
	190	/* 18/40-18/49 */,
	200	/* 18/50-18/59 */,
	225	/* 18/60-18/69 */,
	250	/* 18/70-18/79 */,
	300	/* 18/80-18/89 */,
	350	/* 18/90-18/99 */,
	400	/* 18/100-18/109 */,
	450	/* 18/110-18/119 */,
	500	/* 18/120-18/129 */,
	550	/* 18/130-18/139 */,
	600	/* 18/140-18/149 */,
	650	/* 18/150-18/159 */,
	700	/* 18/160-18/169 */,
	750	/* 18/170-18/179 */,
	800	/* 18/180-18/189 */,
	800	/* 18/190-18/199 */,
	800	/* 18/200-18/209 */,
	800	/* 18/210-18/219 */,
	800	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- Minimum failure rate (percentage)
 */
const byte adj_mag_fail[STAT_RANGE] =
{
	99	/* 3 */,
	99	/* 4 */,
	99	/* 5 */,
	99	/* 6 */,
	99	/* 7 */,
	50	/* 8 */,
	30	/* 9 */,
	20	/* 10 */,
	15	/* 11 */,
	12	/* 12 */,
	11	/* 13 */,
	10	/* 14 */,
	9	/* 15 */,
	8	/* 16 */,
	7	/* 17 */,
	6	/* 18/00-18/09 */,
	6	/* 18/10-18/19 */,
	5	/* 18/20-18/29 */,
	5	/* 18/30-18/39 */,
	5	/* 18/40-18/49 */,
	4	/* 18/50-18/59 */,
	4	/* 18/60-18/69 */,
	4	/* 18/70-18/79 */,
	4	/* 18/80-18/89 */,
	3	/* 18/90-18/99 */,
	3	/* 18/100-18/109 */,
	2	/* 18/110-18/119 */,
	2	/* 18/120-18/129 */,
	2	/* 18/130-18/139 */,
	2	/* 18/140-18/149 */,
	1	/* 18/150-18/159 */,
	1	/* 18/160-18/169 */,
	1	/* 18/170-18/179 */,
	1	/* 18/180-18/189 */,
	1	/* 18/190-18/199 */,
	0	/* 18/200-18/209 */,
	0	/* 18/210-18/219 */,
	0	/* 18/220+ */
};


/*
 * Stat Table (INT/WIS) -- failure rate adjustment
 */
const int adj_mag_stat[STAT_RANGE] =
{
	-5	/* 3 */,
	-4	/* 4 */,
	-3	/* 5 */,
	-3	/* 6 */,
	-2	/* 7 */,
	-1	/* 8 */,
	 0	/* 9 */,
	 0	/* 10 */,
	 0	/* 11 */,
	 0	/* 12 */,
	 0	/* 13 */,
	 1	/* 14 */,
	 2	/* 15 */,
	 3	/* 16 */,
	 4	/* 17 */,
	 5	/* 18/00-18/09 */,
	 6	/* 18/10-18/19 */,
	 7	/* 18/20-18/29 */,
	 8	/* 18/30-18/39 */,
	 9	/* 18/40-18/49 */,
	10	/* 18/50-18/59 */,
	11	/* 18/60-18/69 */,
	12	/* 18/70-18/79 */,
	15	/* 18/80-18/89 */,
	18	/* 18/90-18/99 */,
	21	/* 18/100-18/109 */,
	24	/* 18/110-18/119 */,
	27	/* 18/120-18/129 */,
	30	/* 18/130-18/139 */,
	33	/* 18/140-18/149 */,
	36	/* 18/150-18/159 */,
	39	/* 18/160-18/169 */,
	42	/* 18/170-18/179 */,
	45	/* 18/180-18/189 */,
	48	/* 18/190-18/199 */,
	51	/* 18/200-18/209 */,
	54	/* 18/210-18/219 */,
	57	/* 18/220+ */
};

/*
 * This table is used to help calculate the number of blows the player can
 * make in a single round of attacks (one player turn) with a normal weapon.
 *
 * This number ranges from a single blow/round for weak players to up to six
 * blows/round for powerful warriors.
 *
 * Note that certain artifacts and ego-items give "bonus" blows/round.
 *
 * First, from the player class, we extract some values:
 *
 *    Warrior --> num = 6; mul = 5; div = MAX(30, weapon_weight);
 *    Mage    --> num = 4; mul = 2; div = MAX(40, weapon_weight);
 *    Priest  --> num = 4; mul = 3; div = MAX(35, weapon_weight);
 *    Rogue   --> num = 5; mul = 4; div = MAX(30, weapon_weight);
 *    Ranger  --> num = 5; mul = 4; div = MAX(35, weapon_weight);
 *    Paladin --> num = 5; mul = 5; div = MAX(30, weapon_weight);
 * (all specified in p_class.txt now)
 *
 * To get "P", we look up the relevant "adj_str_blow[]" (see above),
 * multiply it by "mul", and then divide it by "div", rounding down.
 *
 * To get "D", we look up the relevant "adj_dex_blow[]" (see above).
 *
 * Then we look up the energy cost of each blow using "blows_table[P][D]".
 * The player gets blows/round equal to 100/this number, up to a maximum of
 * "num" blows/round, plus any "bonus" blows/round.
 */
const byte blows_table[12][12] =
{
	/* P */
   /* D:   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11+ */
   /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */

	/* 0  */
	{  100, 100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23 },

	/* 1  */
	{  100, 95,  85,  75,  60,  50,  42,  35,  30,  25,  23,  21 },

	/* 2  */
	{  95,  85,  75,  60,  50,  42,  35,  30,  26,  23,  21,  20 },

	/* 3  */
	{  85,  75,  60,  50,  42,  36,  32,  28,  25,  22,  20,  19 },

	/* 4  */
	{  75,  60,  50,  42,  36,  33,  28,  25,  23,  21,  19,  18 },

	/* 5  */
	{  60,  50,  42,  36,  33,  30,  27,  24,  22,  21,  19,  17 },

	/* 6  */
	{  50,  42,  36,  33,  30,  27,  25,  23,  21,  20,  18,  17 },

	/* 7  */
	{  42,  36,  33,  30,  28,  26,  24,  22,  20,  19,  18,  17 },

	/* 8  */
	{  36,  33,  30,  28,  26,  24,  22,  21,  20,  19,  17,  16 },

	/* 9  */
	{  35,  32,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16 },

	/* 10 */
	{  34,  30,  27,  25,  23,  22,  21,  20,  19,  18,  17,  16 },

	/* 11+ */
	{  33,  29,  26,  24,  22,  21,  20,  19,  18,  17,  16,  15 },
   /* DEX: 3,   10,  17,  /20, /40, /60, /80, /100,/120,/150,/180,/200 */
};


/*
 * This table allows quick conversion from "speed" to "energy"
 * The basic function WAS ((S>=110) ? (S-110) : (100 / (120-S)))
 * Note that table access is *much* quicker than computation.
 *
 * Note that the table has been changed at high speeds.  From
 * "Slow (-40)" to "Fast (+30)" is pretty much unchanged, but
 * at speeds above "Fast (+30)", one approaches an asymptotic
 * effective limit of 50 energy per turn.  This means that it
 * is relatively easy to reach "Fast (+30)" and get about 40
 * energy per turn, but then speed becomes very "expensive",
 * and you must get all the way to "Fast (+50)" to reach the
 * point of getting 45 energy per turn.  After that point,
 * furthur increases in speed are more or less pointless,
 * except to balance out heavy inventory.
 *
 * Note that currently the fastest monster is "Fast (+30)".
 *
 * It should be possible to lower the energy threshhold from
 * 100 units to 50 units, though this may interact badly with
 * the (compiled out) small random energy boost code.  It may
 * also tend to cause more "clumping" at high speeds.
 */
const byte extract_energy[200] =
{
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* Slow */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* S-50 */     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	/* S-40 */     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	/* S-30 */     2,  2,  2,  2,  2,  2,  2,  3,  3,  3,
	/* S-20 */     3,  3,  3,  3,  3,  4,  4,  4,  4,  4,
	/* S-10 */     5,  5,  5,  5,  6,  6,  7,  7,  8,  9,
	/* Norm */    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	/* F+10 */    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	/* F+20 */    30, 31, 32, 33, 34, 35, 36, 36, 37, 37,
	/* F+30 */    38, 38, 39, 39, 40, 40, 40, 41, 41, 41,
	/* F+40 */    42, 42, 42, 43, 43, 43, 44, 44, 44, 44,
	/* F+50 */    45, 45, 45, 45, 45, 46, 46, 46, 46, 46,
	/* F+60 */    47, 47, 47, 47, 47, 48, 48, 48, 48, 48,
	/* F+70 */    49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
	/* Fast */    49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
};







/*
 * Base experience levels, may be adjusted up for race and/or class
 */
const s32b player_exp[PY_MAX_LEVEL] =
{
	10,
	25,
	45,
	70,
	100,
	140,
	200,
	280,
	380,
	500,
	650,
	850,
	1100,
	1400,
	1800,
	2300,
	2900,
	3600,
	4400,
	5400,
	6800,
	8400,
	10200,
	12500,
	17500,
	25000,
	35000L,
	50000L,
	75000L,
	100000L,
	150000L,
	200000L,
	275000L,
	350000L,
	450000L,
	550000L,
	700000L,
	850000L,
	1000000L,
	1250000L,
	1500000L,
	1800000L,
	2100000L,
	2400000L,
	2700000L,
	3000000L,
	3500000L,
	4000000L,
	4500000L,
	5000000L
};


/*
 * Player Sexes
 *
 *	Title,
 *	Winner
 */
const player_sex sex_info[MAX_SEXES] =
{
	{
		"Female",
		"Queen"
	},

	{
		"Male",
		"King"
	},

	{
		"Neuter",
		"Regent"
	}
};


/*
 * Each chest has a certain set of traps, determined by pval
 * Each chest has a "pval" from 1 to the chest level (max 55)
 * If the "pval" is negative then the trap has been disarmed
 * The "pval" of a chest determines the quality of its treasure
 * Note that disarming a trap on a chest also removes the lock.
 */
const byte chest_traps[64] =
{
	0,					/* 0 == empty */
	(CHEST_POISON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),			/* 5 == best small wooden */
	0,
	(CHEST_POISON),
	(CHEST_POISON),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_POISON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_SUMMON),			/* 15 == best large wooden */
	0,
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_PARALYZE),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_SUMMON),
	(CHEST_PARALYZE),
	(CHEST_LOSE_STR),
	(CHEST_LOSE_CON),
	(CHEST_EXPLODE),			/* 25 == best small iron */
	0,
	(CHEST_POISON | CHEST_LOSE_STR),
	(CHEST_POISON | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_PARALYZE),
	(CHEST_POISON | CHEST_SUMMON),
	(CHEST_SUMMON),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),	/* 35 == best large iron */
	0,
	(CHEST_SUMMON),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_POISON | CHEST_PARALYZE),
	(CHEST_EXPLODE),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_POISON | CHEST_PARALYZE),	/* 45 == best small steel */
	0,
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_STR),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE | CHEST_LOSE_STR | CHEST_LOSE_CON),
	(CHEST_POISON | CHEST_PARALYZE),
	(CHEST_POISON | CHEST_PARALYZE),	/* 55 == best large steel */
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
	(CHEST_EXPLODE | CHEST_SUMMON),
};



/*
 * Abbreviations of healthy stats
 */
cptr stat_names[A_MAX] =
{
	"STR: ", "INT: ", "WIS: ", "DEX: ", "CON: ", "CHR: "
};

/*
 * Abbreviations of damaged stats
 */
cptr stat_names_reduced[A_MAX] =
{
	"Str: ", "Int: ", "Wis: ", "Dex: ", "Con: ", "Chr: "
};

/*
 * Full stat names
 */
cptr stat_names_full[A_MAX] =
{
	"strength",
	"intelligence",
	"wisdom",
	"dexterity",
	"constitution",
	"charisma"
};


/*
 * Certain "screens" always use the main screen, including News, Birth,
 * Dungeon, Tomb-stone, High-scores, Macros, Colors, Visuals, Options.
 *
 * Later, special flags may allow sub-windows to "steal" stuff from the
 * main window, including File dump (help), File dump (artifacts, uniques),
 * Character screen, Small scale map, Previous Messages, Store screen, etc.
 */
const char *window_flag_desc[32] =
{
	"Display inven/equip",
	"Display equip/inven",
	"Display player (basic)",
	"Display player (extra)",
	"Display player (compact)",
	"Display map view",
	"Display messages",
	"Display overhead view",
	"Display monster recall",
	"Display object recall",
	"Display monster list",
	"Display status",
	"Display item list",
	NULL,
	"Display borg messages",
	"Display borg status",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


const char *inscrip_text[] =
{
	NULL,
	"strange",
	"average",
	"magical",
	"splendid",
	"excellent",
	"special",
	"unknown"
};

const grouper object_text_order[] =
{
	{TV_RING,			"Ring"			},
	{TV_AMULET,			"Amulet"		},
	{TV_POTION,			"Potion"		},
	{TV_SCROLL,			"Scroll"		},
	{TV_WAND,			"Wand"			},
	{TV_STAFF,			"Staff"			},
	{TV_ROD,			"Rod"			},
	{TV_FOOD,			"Food"			},
	{TV_PRAYER_BOOK,	"Priest Book"	},
	{TV_MAGIC_BOOK,		"Magic Book"	},
	{TV_LIGHT,			"Light"			},
	{TV_FLASK,			"Flask"			},
	{TV_SWORD,			"Sword"			},
	{TV_POLEARM,		"Polearm"		},
	{TV_HAFTED,			"Hafted Weapon" },
	{TV_BOW,			"Bow"			},
	{TV_ARROW,			"Ammunition"	},
	{TV_BOLT,			NULL			},
	{TV_SHOT,			NULL			},
	{TV_SHIELD,			"Shield"		},
	{TV_CROWN,			"Crown"			},
	{TV_HELM,			"Helm"			},
	{TV_GLOVES,			"Gloves"		},
	{TV_BOOTS,			"Boots"			},
	{TV_CLOAK,			"Cloak"			},
	{TV_DRAG_ARMOR,		"Dragon Scale Mail" },
	{TV_HARD_ARMOR,		"Hard Armor"	},
	{TV_SOFT_ARMOR,		"Soft Armor"	},
	{TV_SPIKE,			"Spike"			},
	{TV_DIGGING,		"Digger"		},
	{TV_JUNK,			"Junk"			},
	{0,					NULL			}
};


/*
 * Character translations and definitions.  -JG-
 *
 * Upper case and lower case equivalents of a given ISO Latin-1 character.
 * A character's general "type"; types may be combined.
 *
 * Note that this table assumes use of the standard Angband extended fonts.
 *
 * Notice the accented characters in positions 191+.  If they don't appear
 * correct to you, then you are viewing this table in a non-Latin-1 font.
 * Individual ports are responsible for translations from the game's
 * internal Latin-1 to their system's character encoding(s) (unless ASCII).
 */
const byte char_tables[256][CHAR_TABLE_SLOTS] =
{
/* TO_UPPER, TO_LOWER, CHAR TYPE */
    {   0,     0,           0L },               /*        Empty      */
    {   1,     1,  CHAR_SYMBOL },               /*        Solid      */
    {   2,     2,  CHAR_SYMBOL },               /* Mostly solid      */
    {   3,     3,  CHAR_SYMBOL },               /* Wall pattern      */
    {   4,     4,  CHAR_SYMBOL },               /*    Many dots      */
    {   5,     5,  CHAR_SYMBOL },               /*  Medium dots      */
    {   6,     6,  CHAR_SYMBOL },               /*     Few dots      */
    {   7,     7,  CHAR_SYMBOL },               /*     Tiny dot      */
    {   8,     8,  CHAR_SYMBOL },               /*    Small dot      */
    {   9,     9,  CHAR_SYMBOL },               /*   Medium dot      */
    {  10,    10,  CHAR_SYMBOL },               /*    Large dot      */
    {  11,    11,  CHAR_SYMBOL },               /*       Rubble      */
    {  12,    12,  CHAR_SYMBOL },               /*     Treasure      */
    {  13,    13,  CHAR_SYMBOL },               /*  Closed door      */
    {  14,    14,  CHAR_SYMBOL },               /*    Open Door      */
    {  15,    15,  CHAR_SYMBOL },               /*  Broken door      */
    {  16,    16,  CHAR_SYMBOL },               /*       Pillar      */
    {  17,    17,  CHAR_SYMBOL },               /*        Water      */
    {  18,    18,  CHAR_SYMBOL },               /*         Tree      */
    {  19,    19,  CHAR_SYMBOL },               /*    Fire/lava      */
    {  20,    20,  CHAR_SYMBOL },               /*   Pit/portal      */
    {  22,    22,           0L },               /*       Unused      */
    {  22,    22,           0L },               /*       Unused      */
    {  23,    23,           0L },               /*       Unused      */
    {  24,    24,           0L },               /*       Unused      */
    {  25,    25,           0L },               /*       Unused      */
    {  26,    26,           0L },               /*       Unused      */
    {  27,    27,           0L },               /*       Unused      */
    {  28,    28,           0L },               /*       Unused      */
    {  29,    29,           0L },               /*       Unused      */
    {  30,    30,           0L },               /*       Unused      */
    {  31,    31,           0L },               /*       Unused      */

    {  32,    32,   CHAR_BLANK },               /*        Space      */
    {  33,    33,   CHAR_PUNCT },               /*            !      */
    {  34,    34,   CHAR_PUNCT },               /*            "      */
    {  35,    35,   CHAR_PUNCT },               /*            #      */
    {  36,    36,   CHAR_PUNCT },               /*            $      */
    {  37,    37,   CHAR_PUNCT },               /*            %      */
    {  38,    38,   CHAR_PUNCT },               /*            &      */
    {  39,    39,   CHAR_PUNCT },               /*            '      */
    {  40,    40,   CHAR_PUNCT },               /*            (      */
    {  41,    41,   CHAR_PUNCT },               /*            )      */
    {  42,    42,   CHAR_PUNCT },               /*            *      */
    {  43,    43,   CHAR_PUNCT },               /*            +      */
    {  44,    44,   CHAR_PUNCT },               /*            ,      */
    {  45,    45,   CHAR_PUNCT },               /*            -      */
    {  46,    46,   CHAR_PUNCT },               /*            .      */
    {  47,    47,   CHAR_PUNCT },               /*            /      */

    {  48,    48,   CHAR_DIGIT },               /*            0      */
    {  49,    49,   CHAR_DIGIT },               /*            1      */
    {  50,    50,   CHAR_DIGIT },               /*            2      */
    {  51,    51,   CHAR_DIGIT },               /*            3      */
    {  52,    52,   CHAR_DIGIT },               /*            4      */
    {  53,    53,   CHAR_DIGIT },               /*            5      */
    {  54,    54,   CHAR_DIGIT },               /*            6      */
    {  55,    55,   CHAR_DIGIT },               /*            7      */
    {  56,    56,   CHAR_DIGIT },               /*            8      */
    {  57,    57,   CHAR_DIGIT },               /*            9      */
    {  58,    58,   CHAR_DIGIT },               /*            :      */
    {  59,    59,   CHAR_DIGIT },               /*            ;      */

    {  60,    60,   CHAR_PUNCT },               /*            <      */
    {  61,    61,   CHAR_PUNCT },               /*            =      */
    {  62,    62,   CHAR_PUNCT },               /*            >      */
    {  63,    63,   CHAR_PUNCT },               /*            ?      */
    {  64,    64,   CHAR_PUNCT },               /*            @      */
    {  65,    97,   CHAR_UPPER | CHAR_VOWEL },  /*            A      */
    {  66,    98,   CHAR_UPPER },               /*            B      */
    {  67,    99,   CHAR_UPPER },               /*            C      */
    {  68,   100,   CHAR_UPPER },               /*            D      */
    {  69,   101,   CHAR_UPPER | CHAR_VOWEL },  /*            E      */
    {  70,   102,   CHAR_UPPER },               /*            F      */
    {  71,   103,   CHAR_UPPER },               /*            G      */
    {  72,   104,   CHAR_UPPER },               /*            H      */
    {  73,   105,   CHAR_UPPER | CHAR_VOWEL },  /*            I      */
    {  74,   106,   CHAR_UPPER },               /*            J      */
    {  75,   107,   CHAR_UPPER },               /*            K      */
    {  76,   108,   CHAR_UPPER },               /*            L      */
    {  77,   109,   CHAR_UPPER },               /*            M      */
    {  78,   110,   CHAR_UPPER },               /*            N      */
    {  79,   111,   CHAR_UPPER | CHAR_VOWEL },  /*            O      */
    {  80,   112,   CHAR_UPPER },               /*            P      */
    {  81,   113,   CHAR_UPPER },               /*            Q      */
    {  82,   114,   CHAR_UPPER },               /*            R      */
    {  83,   115,   CHAR_UPPER },               /*            S      */
    {  84,   116,   CHAR_UPPER },               /*            T      */
    {  85,   117,   CHAR_UPPER | CHAR_VOWEL },  /*            U      */
    {  86,   118,   CHAR_UPPER },               /*            V      */
    {  87,   119,   CHAR_UPPER },               /*            W      */
    {  88,   120,   CHAR_UPPER },               /*            X      */
    {  89,   121,   CHAR_UPPER },               /*            Y      */
    {  90,   122,   CHAR_UPPER },               /*            Z      */

    {  91,    91,   CHAR_PUNCT },               /*            [      */
    {  92,    92,   CHAR_PUNCT },               /*            \      */
    {  93,    93,   CHAR_PUNCT },               /*            ]      */
    {  94,    94,   CHAR_PUNCT },               /*            ^      */
    {  95,    95,   CHAR_PUNCT },               /*            _      */
    {  96,    96,   CHAR_PUNCT },               /*            `      */
    {  65,    97,   CHAR_LOWER | CHAR_VOWEL },  /*            a      */
    {  66,    98,   CHAR_LOWER },               /*            b      */
    {  67,    99,   CHAR_LOWER },               /*            c      */
    {  68,   100,   CHAR_LOWER },               /*            d      */
    {  69,   101,   CHAR_LOWER | CHAR_VOWEL },  /*            e      */
    {  70,   102,   CHAR_LOWER },               /*            f      */
    {  71,   103,   CHAR_LOWER },               /*            g      */
    {  72,   104,   CHAR_LOWER },               /*            h      */
    {  73,   105,   CHAR_LOWER | CHAR_VOWEL },  /*            i      */
    {  74,   106,   CHAR_LOWER },               /*            j      */
    {  75,   107,   CHAR_LOWER },               /*            k      */
    {  76,   108,   CHAR_LOWER },               /*            l      */
    {  77,   109,   CHAR_LOWER },               /*            m      */
    {  78,   110,   CHAR_LOWER },               /*            n      */
    {  79,   111,   CHAR_LOWER | CHAR_VOWEL },  /*            o      */
    {  80,   112,   CHAR_LOWER },               /*            p      */
    {  81,   113,   CHAR_LOWER },               /*            q      */
    {  82,   114,   CHAR_LOWER },               /*            r      */
    {  83,   115,   CHAR_LOWER },               /*            s      */
    {  84,   116,   CHAR_LOWER },               /*            t      */
    {  85,   117,   CHAR_LOWER | CHAR_VOWEL },  /*            u      */
    {  86,   118,   CHAR_LOWER },               /*            v      */
    {  87,   119,   CHAR_LOWER },               /*            w      */
    {  88,   120,   CHAR_LOWER },               /*            x      */
    {  89,   121,   CHAR_LOWER },               /*            y      */
    {  90,   122,   CHAR_LOWER },               /*            z      */
    { 123,   123,   CHAR_PUNCT },               /*            {    */
    { 124,   124,   CHAR_PUNCT },               /*            |      */
    { 125,   125,   CHAR_PUNCT },               /*            }      */
    { 126,   126,   CHAR_PUNCT },               /*            ~      */
    { 127,   127,  CHAR_SYMBOL },               /* Wall pattern      */

    { 128,   128,           0L },               /*       Unused      */
    { 129,   129,           0L },               /*       Unused      */
    { 130,   130,           0L },               /*       Unused      */
    { 131,   131,           0L },               /*       Unused      */
    { 132,   132,           0L },               /*       Unused      */
    { 133,   133,           0L },               /*       Unused      */
    { 134,   134,           0L },               /*       Unused      */
    { 135,   135,           0L },               /*       Unused      */
    { 136,   136,           0L },               /*       Unused      */
    { 137,   137,           0L },               /*       Unused      */
    { 138,   138,           0L },               /*       Unused      */
    { 139,   139,           0L },               /*       Unused      */
    { 140,   140,           0L },               /*       Unused      */
    { 141,   141,           0L },               /*       Unused      */
    { 142,   142,           0L },               /*       Unused      */
    { 143,   143,           0L },               /*       Unused      */
    { 144,   144,           0L },               /*       Unused      */
    { 145,   145,           0L },               /*       Unused      */
    { 146,   146,           0L },               /*       Unused      */
    { 147,   147,           0L },               /*       Unused      */
    { 148,   148,           0L },               /*       Unused      */
    { 149,   149,           0L },               /*       Unused      */
    { 150,   150,           0L },               /*       Unused      */
    { 151,   151,           0L },               /*       Unused      */
    { 152,   152,           0L },               /*       Unused      */
    { 153,   153,           0L },               /*       Unused      */
    { 154,   154,           0L },               /*       Unused      */
    { 155,   155,           0L },               /*       Unused      */
    { 156,   156,           0L },               /*       Unused      */
    { 157,   157,           0L },               /*       Unused      */
    { 158,   158,           0L },               /*       Unused      */
    { 159,   159,           0L },               /*       Unused      */
    { 160,   160,           0L },               /*       Unused      */

    { 161,   161,   CHAR_PUNCT },               /*       iexcl   ¡   */
    { 162,   162,   CHAR_PUNCT },               /*        euro   ¢   */
    { 163,   163,   CHAR_PUNCT },               /*       pound   £   */
    { 164,   164,   CHAR_PUNCT },               /*      curren   ¤   */
    { 165,   165,   CHAR_PUNCT },               /*         yen   ¥   */
    { 166,   166,   CHAR_PUNCT },               /*      brvbar   ¦   */
    { 167,   167,   CHAR_PUNCT },               /*        sect   §   */
    { 168,   168,  CHAR_SYMBOL },               /*  Bolt - vert      */
    { 169,   169,  CHAR_SYMBOL },               /*  Bolt - horz      */
    { 170,   170,  CHAR_SYMBOL },               /*  Bolt -rdiag      */
    { 171,   171,  CHAR_SYMBOL },               /*  Bolt -ldiag      */
    { 172,   172,  CHAR_SYMBOL },               /*  Spell-cloud      */
    { 173,   173,  CHAR_SYMBOL },               /*   Spell-elec      */
    { 174,   174,  CHAR_SYMBOL },               /*  Spell-explo      */

    { 175,   175,           0L },               /*       Unused      */
    { 176,   176,           0L },               /*       Unused      */
    { 177,   177,           0L },               /*       Unused      */
    { 178,   178,           0L },               /*       Unused      */
    { 179,   179,           0L },               /*       Unused      */
    { 180,   180,           0L },               /*       Unused      */
    { 181,   181,           0L },               /*       Unused      */
    { 182,   182,           0L },               /*       Unused      */
    { 183,   183,           0L },               /*       Unused      */
    { 184,   184,           0L },               /*       Unused      */
    { 185,   185,           0L },               /*       Unused      */
    { 186,   186,           0L },               /*       Unused      */
    { 187,   187,           0L },               /*       Unused      */
    { 188,   188,           0L },               /*       Unused      */
    { 189,   189,           0L },               /*       Unused      */
    { 190,   190,           0L },               /*       Unused      */

    { 191,   191,   CHAR_PUNCT },               /*      iquest   ¿   */
    { 192,   224,   CHAR_UPPER | CHAR_VOWEL },  /*      Agrave   À   */
    { 193,   225,   CHAR_UPPER | CHAR_VOWEL },  /*      Aacute   Á   */
    { 194,   226,   CHAR_UPPER | CHAR_VOWEL },  /*       Acirc   Â   */
    { 195,   227,   CHAR_UPPER | CHAR_VOWEL },  /*      Atilde   Ã   */
    { 196,   228,   CHAR_UPPER | CHAR_VOWEL },  /*        Auml   Ä   */
    { 197,   229,   CHAR_UPPER | CHAR_VOWEL },  /*       Aring   Å   */
    { 198,   230,   CHAR_UPPER | CHAR_VOWEL },  /*       Aelig   Æ   */
    { 199,   231,   CHAR_UPPER },               /*      Ccedil   Ç   */
    { 200,   232,   CHAR_UPPER | CHAR_VOWEL },  /*      Egrave   È   */
    { 201,   233,   CHAR_UPPER | CHAR_VOWEL },  /*      Eacute   É   */
    { 202,   234,   CHAR_UPPER | CHAR_VOWEL },  /*       Ecirc   Ê   */
    { 203,   235,   CHAR_UPPER | CHAR_VOWEL },  /*        Euml   Ë   */
    { 204,   236,   CHAR_UPPER | CHAR_VOWEL },  /*      Igrave   Ì   */
    { 205,   237,   CHAR_UPPER | CHAR_VOWEL },  /*      Iacute   Í   */
    { 206,   238,   CHAR_UPPER | CHAR_VOWEL },  /*       Icirc   Î   */
    { 207,   239,   CHAR_UPPER | CHAR_VOWEL },  /*        Iuml   Ï   */
    { 208,   240,   CHAR_UPPER },               /*         ETH   Ð   */
    { 209,   241,   CHAR_UPPER },               /*      Ntilde   Ñ   */
    { 210,   242,   CHAR_UPPER | CHAR_VOWEL },  /*      Ograve   Ò   */
    { 211,   243,   CHAR_UPPER | CHAR_VOWEL },  /*      Oacute   Ó   */
    { 212,   244,   CHAR_UPPER | CHAR_VOWEL },  /*       Ocirc   Ô   */
    { 213,   245,   CHAR_UPPER | CHAR_VOWEL },  /*      Otilde   Õ   */
    { 214,   246,   CHAR_UPPER | CHAR_VOWEL },  /*        Ouml   Ö   */
    { 215,   215,           0L },               /*       Unused      */
    { 216,   248,   CHAR_UPPER | CHAR_VOWEL },  /*      Oslash   Ø   */
    { 217,   249,   CHAR_UPPER | CHAR_VOWEL },  /*      Ugrave   Ù   */
    { 218,   250,   CHAR_UPPER | CHAR_VOWEL },  /*      Uacute   Ú   */
    { 219,   251,   CHAR_UPPER | CHAR_VOWEL },  /*       Ucirc   Û   */
    { 220,   252,   CHAR_UPPER | CHAR_VOWEL },  /*        Uuml   Ü   */
    { 221,   253,   CHAR_UPPER },               /*      Yacute   Ý   */
    { 222,   254,   CHAR_UPPER },               /*       THORN   Þ   */
    { 223,   223,   CHAR_LOWER },               /*       szlig   ß   */

    { 192,   224,   CHAR_LOWER | CHAR_VOWEL },  /*      agrave   à   */
    { 193,   225,   CHAR_LOWER | CHAR_VOWEL },  /*      aacute   á   */
    { 194,   226,   CHAR_LOWER | CHAR_VOWEL },  /*       acirc   â   */
    { 195,   227,   CHAR_LOWER | CHAR_VOWEL },  /*      atilde   ã   */
    { 196,   228,   CHAR_LOWER | CHAR_VOWEL },  /*        auml   ä   */
    { 197,   229,   CHAR_LOWER | CHAR_VOWEL },  /*       aring   å   */
    { 198,   230,   CHAR_LOWER | CHAR_VOWEL },  /*       aelig   æ   */
    { 199,   231,   CHAR_LOWER },               /*      ccedil   ç   */
    { 200,   232,   CHAR_LOWER | CHAR_VOWEL },  /*      egrave   è   */
    { 201,   233,   CHAR_LOWER | CHAR_VOWEL },  /*      eacute   é   */
    { 202,   234,   CHAR_LOWER | CHAR_VOWEL },  /*       ecirc   ê   */
    { 203,   235,   CHAR_LOWER | CHAR_VOWEL },  /*        euml   ë   */
    { 204,   236,   CHAR_LOWER | CHAR_VOWEL },  /*      igrave   ì   */
    { 205,   237,   CHAR_LOWER | CHAR_VOWEL },  /*      iacute   í   */
    { 206,   238,   CHAR_LOWER | CHAR_VOWEL },  /*       icirc   î   */
    { 207,   239,   CHAR_LOWER | CHAR_VOWEL },  /*        iuml   ï   */
    { 208,   240,   CHAR_LOWER },               /*         eth   ð   */
    { 209,   241,   CHAR_LOWER },               /*      ntilde   ñ   */
    { 210,   242,   CHAR_LOWER | CHAR_VOWEL },  /*      ograve   ò   */
    { 211,   243,   CHAR_LOWER | CHAR_VOWEL },  /*      oacute   ó   */
    { 212,   244,   CHAR_LOWER | CHAR_VOWEL },  /*       ocirc   ô   */
    { 213,   245,   CHAR_LOWER | CHAR_VOWEL },  /*      otilde   õ   */
    { 214,   246,   CHAR_LOWER | CHAR_VOWEL },  /*        ouml   ö   */
    { 247,   247,           0L },               /*       Unused      */
    { 216,   248,   CHAR_LOWER | CHAR_VOWEL },  /*      oslash   ø   */
    { 217,   249,   CHAR_LOWER | CHAR_VOWEL },  /*      ugrave   ù   */
    { 218,   250,   CHAR_LOWER | CHAR_VOWEL },  /*      uacute   ú   */
    { 219,   251,   CHAR_LOWER | CHAR_VOWEL },  /*       ucirc   û   */
    { 220,   252,   CHAR_LOWER | CHAR_VOWEL },  /*        uuml   ü   */
    { 221,   253,   CHAR_LOWER },               /*      yacute   ý   */
    { 222,   254,   CHAR_LOWER },               /*       thorn   þ   */
    { 121,   255,   CHAR_LOWER },               /*        yuml   ÿ   */
};





/*
 * Translate from encodes to extended 8-bit characters and back again.
 */
const xchar_type latin1_encode[] =
{
    { "`A", 192 },  { "'A", 193 },  { "^A", 194 },  { "~A", 195 },
    { "\"A", 196 },  { "*A", 197 },  { ",C", 199 },  { "`E", 200 },
    { "'E", 201 },  { "^E", 202 }, { "\"E", 203 },  { "`I", 204 },
    { "'I", 205 },  { "^I", 206 }, { "\"I", 207 },  { "~N", 209 },
    { "`O", 210 },  { "'O", 211 },  { "^O", 212 },  { "~O", 213 },
	{ "\"O", 214 },  { "/O", 216 },  { "`U", 217 },  { "'U", 218 },
    { "^U", 219 }, { "\"U", 220 },  { "'Y", 221 },  { "`a", 224 },
    { "'a", 225 },  { "^a", 226 },  { "~a", 227 }, { "\"a", 228 },
    { "*a", 229 },  { ",c", 231 },  { "`e", 232 },  { "'e", 233 },
    { "^e", 234 }, { "\"e", 235 },  { "`i", 236 },  { "'i", 237 },
    { "^i", 238 }, { "\"i", 239 },  { "~n", 241 },  { "`o", 242 },
    { "'o", 243 },  { "^o", 244 },  { "~o", 245 }, { "\"o", 246 },
    { "/o", 248 },  { "`u", 249 },  { "'u", 250 },  { "^u", 251 },
    { "\"u", 252 },  { "'y", 253 }, { "\"y", 255 },

    { "iexcl", 161 }, { "euro", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 },   { "brvbar", 166 }, { "sect", 167 }, { "Agrave", 192 },
    { "Aacute", 193 }, { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 },
    { "Aring", 197 }, { "Aelig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 },
    { "Eacute", 201 }, { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 },
    { "Iacute", 205 }, { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "Oslash", 216 }, { "Ugrave", 217 },
    { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 }, { "Yacute", 221 },
    { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 }, { "aacute", 225 },
    { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 }, { "aring", 229 },
    { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 }, { "eacute", 233 },
    { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 }, { "iacute", 237 },
    { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },   { "ntilde", 241 },
    { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 }, { "otilde", 245 },
    { "ouml", 246 }, { "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 },
    { "ucirc", 251 }, { "uuml", 252 }, { "yacute", 253 }, { "thorn", 254 },
    { "yuml", 255 },   { "\0", 0 }
};


/*
 * Info about slays (see src/object/types.h for structure)
 */
const slays slay_table[] =
{
        #define SLAY(a, b, c, d, e, f, g, h, i, j)  { SL_##a, b, c, d, e, f, g, h, i, j},
        #include "list-slays.h"
        #undef SLAY
};

/*
 * Structure for elements
 */
struct element {
	u16b index;		/* Numerical index (EL_FOO) */
	int cap;		/* Breath damage cap */
	int divisor;		/* Breath divisor (monhp / this) */
	int br_flag;		/* Monster flag for breath */
	int res_flag;		/* Object flag for resistance */
	int opp_flag;		/* Timed flag for temporary resistance */
	int imm_flag;		/* Object flag for total immunity */
	int vuln_flag;		/* Object flag for vulnerability */
	int mon_res_flag;	/* Monster flag for resistance */
	int mon_vuln_flag;	/* Monster flag for vulnerability */
};
	
/*
 * Details of the different elemental attacks in the game.
 *
extern const elem_t elem_table[] =
{
        #define ELEMENT(a, b, c, d, e, f, g, h, i, j)  { EL_##a, b, c, d, e, f, g, h, i, j},
        #include "list-elements.h"
        #undef ELEMENT
};
.... not ready yet */
