/*
 * File: generate.c
 * Purpose: Dungeon generation.
 *
 * Code for making, stocking, and populating levels when generated.  
 * Includes rooms of every kind, pits, vaults (inc. interpretation of 
 * vault.txt), streamers, tunnelling, etc.  Level feelings and other 
 * messages, autoscummer behavior.  Creation of the town.
 *
 * Copyright (c) 1997-2001 Ben Harrison, James E. Wilson, Robert A. Koeneke
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



/*
 * Level generation is not an important bottleneck, though it can be 
 * annoyingly slow on older machines...  Thus we emphasize simplicity 
 * and correctness over speed.  See individual functions for notes.
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * The "vault.txt" file is used to store vault generation info.
 */


/*
 * Dungeon generation values
 */
#define DUN_ROOMS			30	/* Number of rooms to attempt */
#define DEST_LEVEL_CHANCE	30	/* 1/chance of being a destroyed level */


/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND  	30	/* 1 in # chance of random direction */
#define DUN_TUN_ADJ  	10	/* 1 in # chance of adjusting direction */
#define DUN_TUN_PEN	35	/* Chance of doors at room entrances */
#define DUN_TUN_JCT	70     /* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_WID	2	/* Width of streamers (can sometimes be higher) */
#define DUN_STR_MAG	3	/* Number of magma streamers */
#define DUN_STR_MC	70	/* 1/chance of treasure per magma */
#define DUN_STR_QUA	2	/* Number of quartz streamers */
#define DUN_STR_QC	35	/* 1/chance of treasure per quartz */
#define DUN_STR_CHG	16	/* 1/(4 + chance) of altering direction */

/*
 * Dungeon treasure allocation values
 */
#define DUN_AMT_ROOM	9	/* Amount of objects for rooms */
#define DUN_AMT_ITEM	3	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD	3	/* Amount of treasure for rooms/corridors */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR		1	/* Hallway */
#define ALLOC_SET_ROOM		2	/* Room */
#define ALLOC_SET_BOTH		3	/* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1	/* Rubble */
#define ALLOC_TYP_TRAP		3	/* Trap */
#define ALLOC_TYP_GOLD		4	/* Gold */
#define ALLOC_TYP_OBJECT	5	/* Object */


/*
 * Maximum numbers of rooms along each axis (currently 6x18)
 */
#define MAX_ROOMS_ROW	(DUNGEON_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(DUNGEON_WID / BLOCK_WID)

/*
 * Maximal number of room types
 */
#define ROOM_MAX	8

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	DUN_ROOMS
#define DOOR_MAX	100
#define WALL_MAX	40
#define TUNN_MAX	300
#define STAIR_MAX	30

/*
 * Simple structure to hold a map location
 */
typedef struct coord coord;

struct coord
{
	byte y;
	byte x;
};

/*
 * Structure to hold all dungeon generation data
 */

typedef struct dun_data dun_data;

struct dun_data
{
	/* Array of centers of rooms */
	int cent_n;
	coord cent[CENT_MAX];

	/* Array to store whether rooms are connected or not. */
	bool connected[CENT_MAX];

	/* Array of possible door locations */
	int door_n;
	coord door[DOOR_MAX];

	/* Array of wall piercing locations */
	int wall_n;
	coord wall[WALL_MAX];

	/* Array of tunnel grids */
	int tunn_n;
	coord tunn[TUNN_MAX];

	/* Array of good potential stair grids */
	int stair_n;
	coord stair[STAIR_MAX];

	/* Number of blocks along each axis */
	int row_rooms;
	int col_rooms;

	/* Array to store block usage */
	int room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];
};

/*
 * Dungeon generation data -- see "cave_gen()"
 */
static dun_data *dun;


/*
 * Room type information
 */
typedef struct room_data room_data;

struct room_data
{
	/* Allocation information. */
	s16b room_gen_num[11];

	/* Minimum level on which room can appear. */
	byte min_level;
};

/*
 * Table of values that control how many times each type of room will, 
 * on average, appear on 100 levels at various depths.  Each type of room 
 * has its own row, and each column corresponds to dungeon levels 0, 10, 
 * 20, and so on.  The final value is the minimum depth the room can appear 
 * at.  -LM-
 *
 * Level 101 and below use the values for level 100.
 *
 * Rooms with lots of monsters or loot may not be generated if the object or 
 * monster lists are already nearly full.  Rooms will not appear above their 
 * minimum depth.  No type of room (other than type 1) can appear more than 
 * DUN_ROOMS/2 times in any level.
 *
 * The entries for room type 1 are blank because these rooms are built once 
 * all other rooms are finished -- until the level fills up, or the room 
 * count reaches the limit (DUN_ROOMS).
 */
static room_data room[ROOM_MAX] = 
{
   /* Depth:          0   10   20   30   40   50   60   70   80   90  100   min */

   /* Nothing */  {{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },  0 },
   /* Simple */   {{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },  0 },
   /* Overlap */  {{ 60,  80, 100, 120, 140, 165, 180, 200, 220, 240, 260 },  1 },
   /* Cross */    {{  0,  25,  50,  70,  85, 100, 110, 120, 130, 140, 150 },  3 },
   /* Large */    {{  0,  25,  50,  70,  85, 100, 110, 120, 130, 140, 150 },  3 },
   /* Pit      */ {{  0,   6,  13,  28,  35,  40,  45,  45,  45,  45,  45 },  5 },
   /* L. Vault */ {{  0,   1,   4,   9,  16,  27,  40,  55,  70,  80,  90 },  5 },
   /* G. Vault */ {{  0,   0,   1,   2,   3,   4,   6,   7,   8,  10,  12 }, 20 }
};



/*
 * This table takes a depth, and returns a suitable monster symbol.  Depth 
 * input is assumed to be player depth.  It is also assumed that monsters 
 * can be generated slightly out of depth.  -LM-
 *
 * Depths greater than 60 should map to the row for level 60.
 *
 * - Symbol '*' is any creature of a randomly-chosen racial char.
 * - Symbol '1' is any animal.
 * - Symbol '2' is any insect ('a', 'c', 'l', 'F', 'I', 'K').
 * - Symbol '3' is any naga, snake, hydra, or other reptile.
 * - Symbol '4' is any jelly, mold, icky thing ('i', 'j', or 'm').
 *
 * - Symbol '%' is any orc, ogre, troll, or giant.
 * - Symbol 'N' is any undead.  Upon occasion, deep in the dungeon, the 
 *   racial type may be forced to 'G', 'L', 'V', or 'W'.
 * - Symbols 'p' and 'h' may sometimes be found in combination.  If they 
 *   are, they are usually all of a given class (magical, pious, natural, 
 *   assassination/thievery, or warrior)
 * - Symbols 'E' and 'v' may be found in combination.  If they are, they 
 *   will always be of a given elemental type.
 * - Symbols 'd' and 'D' both mean dragons of either char.  Dragons are 
 *   often of a particular type (blue, red, gold, shadow/ethereal etc.).
 * - Symbols 'u' and 'U' may mean lesser demons, greater demons, or both, 
 *   depending on depth. 
 * - Symbol 'A' is angels.
 * 
 * - Other symbols usually represent specific racial characters.
 *
 * 80% of the time, one of the first seven characters will be chosen.  
 * 20% of the time, one of the last six will be.
 */
static char mon_symbol_at_depth[12][13] = 
{
	/*      common pits                            rare pits         */

	/* Levels 5, 10, 15, and 20 */
	{'1', '1', '4', '4', '4', 'k', 'y',   '*', '*', '2', 'a', '3', 'S' },
	{'1', '4', '4', 'o', 'o', 'N', '2',   '*', 'C', 'f', 'a', '3', 'S' },
	{'1', '4', 'o', 'o', 'o', 'u', '*',   '#', '#', 'S', 'E', '3', 'Z' },
	{'1', '4', '4', 'o', 'T', 'T', '#',   'p', 'h', 'f', '2', '*', 'Z' },

	/* Levels 25, 30, 35, and 40 */
	{'1', '4', 'T', 'T', 'u', 'P', 'P',   'p', 'v', 'd', '2', 'S', '3' },
	{'1', '1', 'T', 'P', 'P', 'N', 'd',   'p', 'h', 'f', 'v', 'g', 'Z' },
	{'1', '4', 'T', 'P', 'N', 'u', 'd',   'p', 'H', 'E', '2', '*', '3' },
	{'1', '1', 'T', 'P', 'N', 'u', 'd',   'p', 'h', 'g', 'E', '*', 'Z' },

	/* Levels 45, 50, 55, and 60 */
	{'1', 'P', 'N', 'u', 'd', 'd', '*',   'p', 'h', 'v', 'E', '*', 'Z' },
	{'N', 'N', 'U', 'U', 'D', 'D', '*',   'p', 'h', 'v', 'T', 'B', 'Z' },
	{'1', 'N', 'U', 'U', 'D', 'D', '*',   'p', 'h', 'W', 'G', '*', 'Z' },
	{'N', 'N', 'U', 'U', 'D', 'D', '*',   'p', 'h', 'v', '*', 'D', 'Z' } 
};

/* 
 * Restrictions on monsters, used in pits, etc.
 */
static bool allow_unique;
static char d_char_req[10];
static byte d_attr_req[4];
static u32b racial_flag_mask;
static u32b breath_flag_mask;



/*
 * Table of monster descriptions.  Used to make descriptions for kinds 
 * of pits and rooms of chambers that have no special names.
 */
cptr d_char_req_desc[] =
{
	"B:bird",
	"C:canine",
	"D:dragon",
	"E:elemental",
	"F:dragon fly",
	"G:ghost",
	"H:hybrid",
	"I:insect",
	"J:snake",
	"K:killer beetle",
	"L:lich",
	"M:multi-headed hydra",
	"O:ogre",
	"P:giant",
	"Q:quylthulg",
	"R:reptile",
	"S:spider",
	"T:troll",
	"U:demon",
	"V:vampire",
	"W:wraith",
	"Y:yeti",
	"Z:zephyr hound",
	"a:ant",
	"b:bat",
	"c:centipede",
	"d:dragon",
	"e:floating eye",
	"f:feline",
	"g:golem",
	"h:humanoid",
	"i:icky thing",
	"j:jelly",
	"k:kobold",
	"l:louse",
	"m:mold",
	"n:naga",
	"o:orc",
	"p:human",
	"q:quadruped",
	"r:rodent",
	"s:skeleton",
	"t:townsperson",
	"u:demon",
	"v:vortex",
	"w:worm",
	"y:yeek",
	"z:zombie",
	",:mushroom patch",
	NULL
};




/**************************************************************/
/*                                                            */
/*                 The monster-selection code                 */
/*                                                            */
/**************************************************************/


/*
 * Use various selection criteria (set elsewhere) to restrict monster 
 * generation.
 *
 * This function is capable of selecting monsters by:
 *   - racial symbol (may be any of the characters allowed)
 *   - symbol color (may be any of up to four colors).
 *   - racial flag(s) (monster may have any allowed flag)
 *   - breath flag(s) (monster must have exactly the flags specified)
 *
 * Uniques may be forbidden, or allowed on rare occasions.
 *
 * Some situations (like the elemental war themed level) require special 
 * processing; this is done in helper functions called from this one.
 */
static bool mon_select(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];
	bool ok = FALSE;


	/* Require that the monster symbol be correct. */
	if (d_char_req[0] != '\0')
	{
		if (strchr(d_char_req, r_ptr->d_char) == 0) return (FALSE);
	}

	/* Require correct racial type. */
	if (racial_flag_mask)
	{
		if (!(r_ptr->flags3 & (racial_flag_mask))) return (FALSE);

		/* Hack -- no invisible undead until deep. */
		if ((p_ptr->depth < 40) && (r_ptr->flags3 & (RF3_UNDEAD)) && 
			(r_ptr->flags2 & (RF2_INVISIBLE))) return (FALSE);
	}

	/* Require that monster breaths be exactly those specified. */
	if (breath_flag_mask)
	{
		if (r_ptr->flags4 != breath_flag_mask) return (FALSE);
	}

	/* Require that the monster color be correct. */
	if (d_attr_req[0])
	{
		/* Check all allowed colors, if given. */
		if ((d_attr_req[0]) && (r_ptr->d_attr == d_attr_req[0])) ok = TRUE;
		if ((d_attr_req[1]) && (r_ptr->d_attr == d_attr_req[1])) ok = TRUE;
		if ((d_attr_req[2]) && (r_ptr->d_attr == d_attr_req[2])) ok = TRUE;
		if ((d_attr_req[3]) && (r_ptr->d_attr == d_attr_req[3])) ok = TRUE;

		/* Hack -- No multihued dragons allowed in the arcane dragon pit. */
		if ((strchr(d_char_req, 'd') || strchr(d_char_req, 'D')) && 
			(d_attr_req[0] == TERM_VIOLET) && 
			(r_ptr->flags4 == (RF4_BR_ACID | 
				RF4_BR_ELEC | RF4_BR_FIRE | 
				RF4_BR_COLD | RF4_BR_POIS)))
		{
			return (FALSE);
		}

		/* Doesn't match any of the given colors?  Not good. */
		if (!ok) return (FALSE);
	}

	/* Usually decline unique monsters. */
	if (r_ptr->flags1 & (RF1_UNIQUE))
	{
		if (!allow_unique) return (FALSE);
		else if (rand_int(5) != 0) return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Accept characters representing a race or group of monsters and 
 * an (adjusted) depth, and use these to set values for required racial 
 * type, monster symbol, monster symbol color, and breath type.  -LM-
 *
 * This function is called to set restrictions, point the monster 
 * allocation function to "mon_select()", and remake monster allocation.  
 * It undoes all of this things when called with the symbol '\0'.
 * 
 * Describe the monsters (used by cheat_room) and determine if they 
 * should be neatly ordered or randomly placed (used in monster pits).
 */
static char *mon_restrict(char symbol, byte depth, bool *ordered, bool unique_ok)
{
	int i, j;

	/* Assume no definite name */
	char name[80] = "misc";

	/* Clear global monster restriction variables. */
	allow_unique = unique_ok;
	for (i = 0; i < 10; i++) d_char_req[i] = '\0';
	for (i = 0; i < 4; i++) d_attr_req[i] = 0;
	racial_flag_mask = 0; breath_flag_mask = 0;


	/* No symbol, no restrictions. */
	if (symbol == '\0')
	{
		get_mon_num_hook = NULL;
		get_mon_num_prep();
		return ("misc");
	}

	/* Handle the "wild card" symbol '*'  */
	if (symbol == '*')
	{
		for (i = 0; i < 2500; i++)
		{
			/* Get a random monster. */
			j = randint(z_info->r_max - 1);

			/* Must be a real monster */
			if (!r_info[j].rarity) continue;

			/* Try for close to depth, accept in-depth if necessary */
			if (i < 200)
			{
				if ((!(r_info[j].flags1 & RF1_UNIQUE)) && 
				      (r_info[j].level != 0) && 
				      (r_info[j].level <= depth) && 
				      (ABS(r_info[j].level - p_ptr->depth) < 
				      1 + (p_ptr->depth / 4))) break;
			}
			else
			{
				if ((!(r_info[j].flags1 & RF1_UNIQUE)) && 
				      (r_info[j].level != 0) && 
				      (r_info[j].level <= depth)) break;
			}
		}

		/* We've found a monster. */
		if (i < 2499)
		{
			/* ...use that monster's symbol for all monsters. */
			symbol = r_info[j].d_char;
		}
		else
		{
			/* Paranoia - pit stays empty if no monster is found */
			return (NULL);
		}
	}

	/* Apply monster restrictions according to symbol. */
	switch (symbol)
	{
		/* All animals */
		case '1':
		{
			strcpy(name, "animal");
			racial_flag_mask = RF3_ANIMAL;
			*ordered = FALSE;
			break;
		}

		/* Insects */
		case '2':
		{
			strcpy(name, "insect");
			strcpy(d_char_req, "aclFIK");
			*ordered = FALSE;
			break;
		}

		/* Reptiles */
		case '3':
		{
			strcpy(name, "reptile");
			strcpy(d_char_req, "nJRM");
			*ordered = FALSE;
			break;
		}

		/* Jellies, etc. */
		case '4':
		{
			strcpy(name, "jelly");
			strcpy(d_char_req, "ijm,");
			*ordered = FALSE;
			break;
		}

		/* Humans and humaniods */
		case 'p':
		case 'h':
		{
			/* 'p's and 'h's can coexist. */
			if (rand_int(3) == 0) 
			{
				strcpy(d_char_req, "ph");

				/* If so, they will usually all be of similar classes. */
				if (rand_int(4) != 0)
				{
					/* Randomizer. */
					i = rand_int(5);

					/* Magicians and necromancers */
					if (i == 0)
					{
						d_attr_req[0] = TERM_RED;
						d_attr_req[1] = TERM_L_RED;
						d_attr_req[2] = TERM_VIOLET;
						strcpy(name, "school of sorcery");
					}
					/* Priests and paladins */
					else if (i == 1)
					{
						d_attr_req[0] = TERM_GREEN;
						d_attr_req[1] = TERM_L_GREEN;
						d_attr_req[2] = TERM_WHITE;
						d_attr_req[3] = TERM_L_WHITE;
						strcpy(name, "temple of piety");
					}
					/* Druids and ninjas */
					else if (i == 2)
					{
						d_attr_req[0] = TERM_ORANGE;
						d_attr_req[1] = TERM_YELLOW;
						strcpy(name, "gathering of nature");
					}
					/* Thieves and assassins */
					else if (i == 3)
					{
						d_attr_req[0] = TERM_BLUE;
						d_attr_req[1] = TERM_L_BLUE;
						d_attr_req[2] = TERM_SLATE;
						d_attr_req[3] = TERM_L_DARK;
						strcpy(name, "den of thieves");
					}
					/* Warriors and rangers */
					else 
					{
						d_attr_req[0] = TERM_UMBER;
						d_attr_req[1] = TERM_L_UMBER;
						strcpy(name, "fighter's hall");
					}
				}
				else
				{
					strcpy(name, "humans and humaniods");
				}
			}

			/* Usually, just accept the symbol. */
			else 
			{
				d_char_req[0] = symbol;

				if (symbol == 'p') strcpy(name, "human");
				else if (symbol == 'h') strcpy(name, "humanoid");
			}

			*ordered = FALSE;
			break;
		}

		/* Orcs */
		case 'o':
		{
			strcpy(name, "orc");
			strcpy(d_char_req, "o");
			*ordered = TRUE;
			break;
		}

		/* Trolls */
		case 'T':
		{
			strcpy(name, "troll");
			strcpy(d_char_req, "T");
			*ordered = TRUE;
			break;
		}

		/* Giants (sometimes ogres at low levels) */
		case 'P':
		{
			strcpy(name, "giant");
			if ((p_ptr->depth < 30) && (rand_int(3) == 0)) 
			     strcpy(d_char_req, "O");
			else strcpy(d_char_req, "P");
			*ordered = TRUE;
			break;
		}

		/* Orcs, ogres, trolls, or giants */
		case '%':
		{
			strcpy(name, "moria");
			strcpy(d_char_req, "oOPT");
			*ordered = FALSE;
			break;
		}

		/* Monsters found in caves */
		case '0':
		{
			strcpy(name, "dungeon monsters");
			strcpy(d_char_req, "ykoOT");
			*ordered = FALSE;
			break;
		}



		/* Undead */
		case 'N':
		{
			/* Sometimes, restrict by symbol. */
			if ((depth > 40) && (rand_int(3) == 0))
			{
				for (i = 0; i < 500; i++)
				{
					/* Find a suitable monster near depth. */
					j = randint(z_info->r_max - 1);

					/* Require a non-unique undead. */
					if ((r_info[j].flags3 & RF3_UNDEAD) && 
					    (!(r_info[j].flags1 & RF1_UNIQUE)) && 
					    (strchr("GLWV", r_info[j].d_char)) && 
					    (ABS(r_info[j].level - p_ptr->depth) < 
					    1 + (p_ptr->depth / 4)))
					{
						break;
					}
				}

				/* If we find a monster, */
				if (i < 499)
				{
					/* Use that monster's symbol for all monsters */
					d_char_req[0] = r_info[j].d_char;

					/* No pit name (yet) */

					/* In this case, we do order the monsters */
					*ordered = TRUE;
				}
				else
				{
					/* Accept any undead. */
					strcpy(name, "undead");
					racial_flag_mask = RF3_UNDEAD;
					*ordered = FALSE;
				}
			}
			else
			{
				/* No restrictions on symbol. */
				strcpy(name, "undead");
				racial_flag_mask = RF3_UNDEAD;
				*ordered = FALSE;
			}
			break;
		}

		/* Demons */
		case 'u':
		case 'U':
		{
			strcpy(name, "demon");
			if (depth > 55)      strcpy(d_char_req, "U");
			else if (depth < 40) strcpy(d_char_req, "u");
			else                 strcpy(d_char_req, "uU");
			*ordered = TRUE;
			break;
		}

		/* Dragons */
		case 'd':
		case 'D':
		{
			strcpy(d_char_req, "dD");

			/* Dragons usually associate with others of their kind. */
			if (rand_int(6) != 0)
			{
				/* Dragons of a single kind are ordered. */
				*ordered = TRUE;

				/* Some dragon types are not found everywhere */
				if (depth > 70) i = rand_int(35);
				else if (depth > 45) i = rand_int(32);
				else if (depth > 32) i = rand_int(30);
				else if (depth > 23) i = rand_int(28);
				else i = rand_int(24);

				if (i < 4)
				{
					breath_flag_mask = (RF4_BR_ACID);
					strcpy(name, "dragon - acid");
				}
				else if (i < 8)
				{
					breath_flag_mask = (RF4_BR_ELEC);
					strcpy(name, "dragon - electricity");
				}
				else if (i < 12)
				{
					breath_flag_mask = (RF4_BR_FIRE);
					strcpy(name, "dragon - fire");
				}
				else if (i < 16)
				{
					breath_flag_mask = (RF4_BR_COLD);
					strcpy(name, "dragon - cold");
				}
				else if (i < 20)
				{
					breath_flag_mask = (RF4_BR_POIS);
					strcpy(name, "dragon - poison");
				}
				else if (i < 24)
				{
					breath_flag_mask = (RF4_BR_ACID | 
					    RF4_BR_ELEC | RF4_BR_FIRE | 
					    RF4_BR_COLD | RF4_BR_POIS);
					strcpy(name, "dragon - multihued");
				}
				else if (i < 26)
				{
					breath_flag_mask = (RF4_BR_CONF);
					strcpy(name, "dragon - confusion");
				}
				else if (i < 28)
				{
					breath_flag_mask = (RF4_BR_SOUN);
					strcpy(name, "dragon - sound");
				}
				else if (i < 30)
				{
					breath_flag_mask = (RF4_BR_LITE | 
					                    RF4_BR_DARK);
					strcpy(name, "dragon - ethereal");
				}

				/* Chaos, Law, Balance, Power, etc.) */
				else
				{
					d_attr_req[0] = TERM_VIOLET;
					d_attr_req[1] = TERM_L_BLUE;
					d_attr_req[2] = TERM_L_GREEN;
					strcpy(name, "dragon - arcane");
				}
			}
			else
			{
				strcpy(name, "dragon - mixed");

				/* Dragons of all kinds are not ordered. */
				*ordered = FALSE;
			}
			break;
		}

		/* Angels */
		case 'A':
		{
			strcpy(name, "angelic");
			strcpy(d_char_req, "A");
			*ordered = TRUE;
			break;
		}

		/* Vortexes and elementals */
		case 'v':
		case 'E':
		{
			/* Usually, just have any kind of 'v' or 'E' */
			if (rand_int(3) != 0)
			{
				d_char_req[0] = symbol;

				if (symbol == 'v') strcpy(name, "vortex");
				if (symbol == 'E') strcpy(name, "elemental");
			}

			/* Sometimes, choose both 'v' and 'E's of one element */
			else
			{
				strcpy(d_char_req, "vE");

				i = rand_int(4);

				/* Fire */
				if (i == 0)
				{
					d_attr_req[0] = TERM_RED;
					strcpy(name, "fire");
				}
				/* Frost */
				if (i == 1)
				{
					d_attr_req[0] = TERM_L_WHITE;
					d_attr_req[1] = TERM_WHITE;
					strcpy(name, "frost");
				}
				/* Air/electricity */
				if (i == 2)
				{
					d_attr_req[0] = TERM_L_BLUE;
					d_attr_req[1] = TERM_BLUE;
					strcpy(name, "air");
				}
				/* Acid/water/earth */
				if (i == 3)
				{
					d_attr_req[0] = TERM_GREEN;
					d_attr_req[1] = TERM_L_UMBER;
					d_attr_req[2] = TERM_UMBER;
					d_attr_req[3] = TERM_SLATE;
					strcpy(name, "earth & water");
				}
			}

			*ordered = FALSE;
			break;
		}

		/* Special case:  mimics and treasure */
		case '!':
		case '?':
		case '=':
		case '~':
		case '|':
		case '.':
		case '$':
		{
			if (symbol == '$')
			{
				strcpy(name, "treasure");

				/* Nothing but loot! */
				if (rand_int(3) == 0) strcpy(d_char_req, "$");

				/* Guard the money well. */
				else strcpy(d_char_req, "$!?=~|.");
			}
			else
			{
				/* No treasure. */
				strcpy(d_char_req, "!?=~|.");
				strcpy(name, "mimic");
			}

			*ordered = FALSE;
			break;
		}

		/* Special case:  creatures of earth. */
		case 'X':
		case '#':
		{
			strcpy(d_char_req, "X#");
			strcpy(name, "creatures of earth");
			*ordered = FALSE;
			break;
		}

		/* Space for more monster types here. */


		/* Any symbol not handled elsewhere. */
		default:
		{
			/* Accept the character. */
			d_char_req[0] = symbol;

			/* Some monsters should logically be ordered. */
			if (strchr("knosuyzGLMOPTUVW", symbol)) *ordered = TRUE;

			/* Most should not */
			else *ordered = FALSE;
			
			break;
		}
	}

	/* If monster pit hasn't been named already, get a name. */
	if (streq(name, "misc"))
	{
		/* Search a table for a description of the symbol */
		for (i = 0; d_char_req_desc[i]; ++i)
		{
			if (symbol == d_char_req_desc[i][0]) 
			{
				/* Get all but the 1st 2 characters of the text. */
				sprintf(name, "%s", d_char_req_desc[i] + 2);
				break;
			}
		}
	}

	/* Apply our restrictions */
	get_mon_num_hook = mon_select;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Return the name. */
	return (format("%s", name));
}





/**************************************************************/
/*                                                            */
/*            General dungeon-generation functions            */
/*                                                            */
/**************************************************************/


/*
 * Count the number of walls adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds_fully(y, x)"
 */
static int next_to_walls(int y, int x)
{
	int k = 0;

	if (cave_feat[y+1][x] >= FEAT_MAGMA) k++;
	if (cave_feat[y-1][x] >= FEAT_MAGMA) k++;
	if (cave_feat[y][x+1] >= FEAT_MAGMA) k++;
	if (cave_feat[y][x-1] >= FEAT_MAGMA) k++;

	return (k);
}


/*
 * Returns co-ordinates for the player.  Player prefers to be near 
 * walls, because large open spaces are dangerous.
 */
static void new_player_spot(void)
{
	int i = 0;
	int y, x;

	/* No stairs down from Quest */
	if (is_quest(p_ptr->depth))
        p_ptr->create_down_stair = FALSE;


	/*
	 * Check stored stair locations, then search at random.
	 */
	while (TRUE)
	{
		i++;

		/* Scan stored locations first. */
		if (i < dun->stair_n)
		{
			/* Get location */
			y = dun->stair[i].y;
			x = dun->stair[i].x;

			/* Require exactly three adjacent walls */
			if (next_to_walls(y, x) != 3) continue;

			/* If character starts on stairs, ... */
			if (!adult_no_stairs)
			{
				/* Accept stairs going the right way or floors. */
				if (p_ptr->create_down_stair)
				{
					/* Accept down stairs */
					if (cave_feat[y][x] == FEAT_MORE) break;

					/* Accept floors, build down stairs. */
					if (cave_naked_bold(y, x))
					{
						cave_set_feat(y, x, FEAT_MORE);
						break;
					}
				}
				else
				{
					/* Accept up stairs */
					if (cave_feat[y][x] == FEAT_LESS) break;

					/* Accept floors, build up stairs. */
					if (cave_naked_bold(y, x))
					{
						cave_set_feat(y, x, FEAT_LESS);
						break;
					}
				}
			}

			/* If character doesn't start on stairs, ... */
			else
			{
				/* Accept only "naked" floor grids */
				if (cave_naked_bold(y, x)) break;
			}
		}

		/* Then, search at random */
		else
		{
			/* Pick a random grid */
			y = rand_int(DUNGEON_HGT);
			x = rand_int(DUNGEON_WID);

			/* Refuse to start on anti-teleport (vault) grids */
			if (cave_info[y][x] & (CAVE_ICKY)) continue;

			/* Must be a "naked" floor grid */
			if (!cave_naked_bold(y, x)) continue;

			/* Player prefers to be near walls. */
			if (i < 300 && (next_to_walls(y, x) < 2)) continue;
			else if (i < 600 && (next_to_walls(y, x) < 1)) continue;

			/* Success */
			break;
		}
	}

	/* Cancel stair requests */
	p_ptr->create_down_stair = p_ptr->create_up_stair = FALSE;

	/* Place the player */
	player_place(y, x);
}


/*
 * Convert existing terrain type to rubble
 */
static void place_rubble(int y, int x)
{
	/* Create rubble */
	cave_set_feat(y, x, FEAT_RUBBLE);
}


/*
 * Convert existing terrain type to "up stairs"
 */
static void place_up_stairs(int y, int x)
{
	/* Create up stairs */
	cave_set_feat(y, x, FEAT_LESS);
}


/*
 * Convert existing terrain type to "down stairs"
 */
static void place_down_stairs(int y, int x)
{
	/* Create down stairs */
	cave_set_feat(y, x, FEAT_MORE);
}


/*
 * Place an up/down staircase at given location
 */
static void place_random_stairs(int y, int x)
{
	/* Paranoia */
	if (!cave_clean_bold(y, x)) return;

	/* Choose a staircase */
	if (!p_ptr->depth)
	{
		place_down_stairs(y, x);
	}
	else if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
	{
		place_up_stairs(y, x);
	}
	else if (rand_int(100) < 50)
	{
		place_down_stairs(y, x);
	}
	else
	{
		place_up_stairs(y, x);
	}
}


/*
 * Places some staircases near walls
 */
static void alloc_stairs(int feat, int num, int walls)
{
	int y, x, i, j;

	/* Place "num" stairs */
	for (i = 0; i < num; i++)
	{
		/* Try hard to place the stair */
		for (j = 0; j < 3000; j++)
		{
			/* Cut some slack if necessary */
			if ((j > dun->stair_n) && (walls > 2)) walls = 2;
			if ((j > 1000) && (walls > 1)) walls = 1;
			if (j > 2000) walls = 0;

			/* Use the stored stair locations first */
			if (j < dun->stair_n)
			{
				y = dun->stair[j].y;
				x = dun->stair[j].x;
			}

			/* Then, search at random */
			else
			{
				/* Pick a random grid */
				y = rand_int(DUNGEON_HGT);
				x = rand_int(DUNGEON_WID);
			}

			/* Require "naked" floor grid */
			if (!cave_naked_bold(y, x)) continue;

			/* Require a certain number of adjacent walls */
			if (next_to_walls(y, x) < walls) continue;

			/* Town -- must go down */
			if (!p_ptr->depth)
			{
				/* Clear previous contents, add down stairs */
				cave_set_feat(y, x, FEAT_MORE);
			}

			/* Quest -- must go up */
			else if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
			{
				/* Clear previous contents, add up stairs */
				cave_set_feat(y, x, FEAT_LESS);
			}

			/* Requested type */
			else
			{
				/* Clear previous contents, add stairs */
				cave_set_feat(y, x, feat);
			}

			/* Finished with this staircase. */
			break;
		}
	}
}


/*
 * Allocates some objects (using "place" and "type")
 */
static void alloc_object(int set, int typ, int num, int depth)
{
	int y, x, k;

	/* Place some objects */
	for (k = 0; k < num; k++)
	{
		/* Pick a "legal" spot */
		while (TRUE)
		{
			bool room;

			/* Location */
			y = rand_int(DUNGEON_HGT);
			x = rand_int(DUNGEON_WID);

			/* Require "naked" floor grid */
			if (!cave_naked_bold(y, x)) continue;

			/* Check for "room" */
			room = (cave_info[y][x] & (CAVE_ROOM)) ? TRUE : FALSE;

			/* Require corridor? */
			if ((set == ALLOC_SET_CORR) && room) continue;

			/* Require room? */
			if ((set == ALLOC_SET_ROOM) && !room) continue;

			/* Accept it */
			break;
		}

		/* Place something */
		switch (typ)
		{
			case ALLOC_TYP_RUBBLE:
			{
				place_rubble(y, x);
				break;
			}

			case ALLOC_TYP_TRAP:
			{
				place_trap(y, x);
				break;
			}

			case ALLOC_TYP_GOLD:
			{
				place_gold(y, x, depth);
				break;
			}

			case ALLOC_TYP_OBJECT:
			{
				place_object(y, x, depth, FALSE, FALSE);
				break;
			}
		}
	}
}


/*
 * Value "1" means the grid will be changed, value "0" means it won't.
 *
 * We have 47 entries because 47 is not divisible by any reasonable 
 * figure for streamer width.
 */
static bool streamer_change_grid[47] = 
{
	0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 
	1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0
};


/*
 * Places "streamers" of rock through dungeon.
 *
 * Note that there are actually six different terrain features used
 * to represent streamers.  Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold.  The
 * hidden gold types are currently unused.
 */
static void build_streamer(int feat, int chance)
{
	int table_start;
	int i;
	int y, x, dy, dx;
	int start_dir, dir;
	int out1, out2;
	bool change;

	/* Initialize time until next turn, and time until next treasure */
	int time_to_treas = randint(chance * 2);
	int time_to_turn = randint(DUN_STR_CHG * 2);


	/* Set standard width.  Vary width sometimes. */
	int width = 2 * DUN_STR_WID + 1;
	if (rand_int(6) == 0) width += randint(3);
	else if (rand_int(6) == 0) width -= randint(3);
	if (width < 1) width = 1;

	/* Set expansion outward from centerline. */
	out1 = width / 2;
	out2 = (width + 1) / 2;


	/* Hack -- Choose starting point */
	y = rand_spread(DUNGEON_HGT / 2, DUNGEON_HGT / 4);
	x = rand_spread(DUNGEON_WID / 2, DUNGEON_WID / 4);

	/* Choose a random compass direction */
	dir = start_dir = ddd[rand_int(8)];

	/* Get an initial start position on the grid alteration table. */
	table_start = rand_int(47);

	/* Place streamer into dungeon */
	while (TRUE)
	{
		/* Advance streamer width steps on the table. */
		table_start += width;

		/*
		 * Change grids outwards along sides.  If moving diagonally, 
		 * change a cross-shaped area.
		 */
		if (ddy[dir]) 
		{
			for (dx = x - out1; dx <= x + out2; dx++)
			{
				/* Stay within dungeon. */
				if (!in_bounds(y, dx)) continue;

				/* Only convert "granite" walls */
				if (cave_feat[y][dx] < FEAT_WALL_EXTRA) continue;
				if (cave_feat[y][dx] > FEAT_WALL_SOLID) continue;

				i = table_start + dx - x;

				if ((i < 47) && (i >= 0)) change = streamer_change_grid[i];
				else change = streamer_change_grid[i % 47];

				/* No change to be made. */
				if (!change) continue;

				/* Clear previous contents, add proper vein type */
				cave_set_feat(y, dx, feat);

				/* Count down time to next treasure. */
				time_to_treas--;

				/* Hack -- Add some (known) treasure */
				if (time_to_treas == 0)
				{
					time_to_treas = randint(chance * 2);
					cave_feat[y][dx] += 0x04;
				}
			}
		}

		if (ddx[dir]) 
		{
			for (dy = y - out1; dy <= y + out2; dy++)
			{
				/* Stay within dungeon. */
				if (!in_bounds(dy, x)) continue;

				/* Only convert "granite" walls */
				if (cave_feat[dy][x] < FEAT_WALL_EXTRA) continue;
				if (cave_feat[dy][x] > FEAT_WALL_SOLID) continue;

				i = table_start + dy - y;

				if ((i < 47) && (i >= 0)) change = streamer_change_grid[i];
				else change = streamer_change_grid[i % 47];

				/* No change to be made. */
				if (!change) continue;

				/* Clear previous contents, add proper vein type */
				cave_set_feat(dy, x, feat);

				/* Count down time to next treasure. */
				time_to_treas--;

				/* Hack -- Add some (known) treasure */
				if (time_to_treas == 0)
				{
					time_to_treas = randint(chance * 2);
					cave_feat[dy][x] += 0x04;
				}
			}
		}

		/* Count down to next direction change. */
		time_to_turn--;

		/* Sometimes, vary direction slightly. */
		if (time_to_turn == 0)
		{
			/* Get time until next turn. */
			time_to_turn = randint(DUN_STR_CHG * 2);

			/* Randomizer. */
			i = rand_int(3);

			/* New direction is always close to start direction. */
			if (start_dir == 2) 
			{
				if (i == 0) dir = 2;
				if (i == 1) dir = 1;
				else        dir = 3;
			}
			else if (start_dir == 8)
			{
				if (i == 0) dir = 8;
				if (i == 1) dir = 9;
				else        dir = 7;
			}
			else if (start_dir == 6)
			{
				if (i == 0) dir = 6;
				if (i == 1) dir = 3;
				else        dir = 9;
			}
			else if (start_dir == 4)
			{
				if (i == 0) dir = 4;
				if (i == 1) dir = 7;
				else        dir = 1;
			}
			else if (start_dir == 3)
			{
				if (i == 0) dir = 3;
				if (i == 1) dir = 2;
				else        dir = 6;
			}
			else if (start_dir == 1)
			{
				if (i == 0) dir = 1;
				if (i == 1) dir = 4;
				else        dir = 2;
			}
			else if (start_dir == 9)
			{
				if (i == 0) dir = 9;
				if (i == 1) dir = 6;
				else        dir = 8;
			}
			else if (start_dir == 7)
			{
				if (i == 0) dir = 7;
				if (i == 1) dir = 8;
				else        dir = 4;
			}
		}

		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Stop at dungeon edge */
		if (!in_bounds(y, x)) break;
	}
}


/*
 * Build a destroyed level
 */
static void destroy_level(void)
{
	int y1, x1, y, x, k, t, n;


	/* Note destroyed levels */
	if (cheat_room) msg_print("Destroyed Level");

	/* Drop a few epi-centers (usually about two) */
	for (n = 0; n < randint(5); n++)
	{
		/* Pick an epi-center */
		x1 = rand_range(5, DUNGEON_WID-1 - 5);
		y1 = rand_range(5, DUNGEON_HGT-1 - 5);

		/* Big area of affect */
		for (y = (y1 - 15); y <= (y1 + 15); y++)
		{
			for (x = (x1 - 15); x <= (x1 + 15); x++)
			{
				/* Skip illegal grids */
				if (!in_bounds_fully(y, x)) continue;

				/* Extract the distance */
				k = distance(y1, x1, y, x);

				/* Stay in the circle of death */
				if (k >= 16) continue;

				/* Delete the monster (if any) */
				delete_monster(y, x);

				/* Destroy valid grids */
				if (cave_valid_bold(y, x))
				{
					/* Delete objects */
					delete_object(y, x);

					/* Wall (or floor) type */
					t = rand_int(200);

					/* Granite */
					if (t < 20)
					{
						/* Create granite wall */
						cave_set_feat(y, x, FEAT_WALL_EXTRA);
					}

					/* Quartz */
					else if (t < 70)
					{
						/* Create quartz vein */
						cave_set_feat(y, x, FEAT_QUARTZ);
					}

					/* Magma */
					else if (t < 100)
					{
						/* Create magma vein */
						cave_set_feat(y, x, FEAT_MAGMA);
					}

					/* Floor */
					else
					{
						/* Create floor */
						cave_set_feat(y, x, FEAT_FLOOR);
					}

					/* No longer part of a room or vault */
					cave_info[y][x] &= ~(CAVE_ROOM | CAVE_ICKY);

					/* No longer illuminated */
					cave_info[y][x] &= ~(CAVE_GLOW);
				}
			}
		}
	}
}



/**************************************************************/
/*                                                            */
/*                   The room-building code                   */
/*                                                            */
/**************************************************************/


/*
 * Place objects, up to the number asked for, in a rectangle centered on 
 * y0, x0.  Accept values for maximum vertical and horizontal displacement.
 *
 * Return prematurely if the code starts looping too much (this may happen 
 * if y0 or x0 are out of bounds, or the floor is already filled).
 */
static void spread_objects(int depth, int num, int y0, int x0, int dy, int dx)
{
	int i, j;	/* Limits on loops */
	int count;
	int y = y0, x = x0;


	/* Try to place objects within our rectangle of effect. */
	for (count = 0, i = 0; ((count < num) && (i < 50)); i++)
	{
		/* Get a location */
		if ((dy == 0) && (dx == 0))
		{
			y = y0; x = x0;
			if (!in_bounds(y, x)) return;
		}
		else
		{
			for (j = 0; j < 10; j++)
			{
				y = rand_spread(y0, dy);
				x = rand_spread(x0, dx);
				if (!in_bounds(y, x))
				{
					if (j < 9) continue;
					else return;
				}
				break;
			}
		}

		/* Require "clean" floor space */
		if (!cave_clean_bold(y, x)) continue;

		/* Place an item */
		if (rand_int(100) < 67)
		{
			place_object(y, x, depth, FALSE, FALSE);
		}

		/* Place gold */
		else
		{
			place_gold(y, x, depth);
		}

		/* Count the object, reset the loop count */
		count++;
		i = 0;
	}
}


/*
 * Place traps, up to the number asked for, in a rectangle centered on 
 * y0, x0.  Accept values for maximum vertical and horizontal displacement.
 *
 * Return prematurely if the code starts looping too much (this may happen 
 * if y0 or x0 are out of bounds, or the floor is already filled).
 */
static void spread_traps(int num, int y0, int x0, int dy, int dx)
{
	int i, j;	/* Limits on loops */
	int count;
	int y = y0, x = x0;

	/* Try to create traps within our rectangle of effect. */
	for (count = 0, i = 0; ((count < num) && (i < 50)); i++)
	{
		/* Get a location */
		if ((dy == 0) && (dx == 0))
		{
			y = y0; x = x0;
			if (!in_bounds(y, x)) return;
		}
		else
		{
			for (j = 0; j < 10; j++)
			{
				y = rand_spread(y0, dy);
				x = rand_spread(x0, dx);
				if (!in_bounds(y, x))
				{
					if (j < 9) continue;
					else return;
				}
				break;
			}
		}

		/* Require "naked" floor grids */
		if (!cave_naked_bold(y, x)) continue;

		/* Place the trap */
		place_trap(y, x);

		/* Count the trap, reset the loop count */
		count++;
		i = 0;
	}
}


/*
 * Place monsters, up to the number asked for, in a rectangle centered on 
 * y0, x0.  Accept values for monster depth, symbol, and maximum vertical 
 * and horizontal displacement.  Call monster restriction functions if 
 * needed.
 *
 * Return prematurely if the code starts looping too much (this may happen 
 * if y0 or x0 are out of bounds, or the area is already occupied).
 */
static void spread_monsters(char symbol, int depth, int num, 
	int y0, int x0, int dy, int dx)
{
	int i, j;	/* Limits on loops */
	int count;
	int y = y0, x = x0;
	int start_mon_num = mon_max;
	bool dummy;

	/* Restrict monsters.  Allow uniques. */
	(void)mon_restrict(symbol, (byte)depth, &dummy, TRUE);

	/* Build the monster probability table. */
	if (!get_mon_num(depth)) return;


	/* Try to summon monsters within our rectangle of effect. */
	for (count = 0, i = 0; ((count < num) && (i < 50)); i++)
	{
		/* Get a location */
		if ((dy == 0) && (dx == 0))
		{
			y = y0; x = x0;
			if (!in_bounds(y, x)) return;
		}
		else
		{
			for (j = 0; j < 10; j++)
			{
				y = rand_spread(y0, dy);
				x = rand_spread(x0, dx);
				if (!in_bounds(y, x))
				{
					if (j < 9) continue;
					else return;
				}
				break;
			}
		}

		/* Require "empty" floor grids */
		if (!cave_empty_bold(y, x)) continue;

		/* Place the monster (sleeping, allow groups) */
		(void)place_monster(y, x, depth, TRUE, TRUE);

		/* Rein in monster groups and escorts a little. */
		if (mon_max - start_mon_num > num * 2) break;

		/* Count the monster(s), reset the loop count */
		count++;
		i = 0;
	}

	/* Remove monster restrictions. */
	(void)mon_restrict('\0', (byte)depth, &dummy, TRUE);
}



/*
 * Generate helper -- create a new room with optional light
 * 
 * Return FALSE if the room is not fully within the dungeon.
 */
static bool generate_room(int y1, int x1, int y2, int x2, int light)
{
	int y, x;

	/* Confirm that room is in bounds. */
	if ((!in_bounds(y1, x1)) || (!in_bounds(y2, x2))) return (FALSE);

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_info[y][x] |= (CAVE_ROOM);
			if (light) cave_info[y][x] |= (CAVE_GLOW);
		}
	}

	/* Success. */
	return (TRUE);
}


/*
 * Generate helper -- fill a rectangle with a feature
 */
static void generate_fill(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_set_feat(y, x, feat);
		}
	}
}


/*
 * Generate helper -- draw a rectangle with a feature
 */
static void generate_draw(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(y, x1, feat);
		cave_set_feat(y, x2, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(y1, x, feat);
		cave_set_feat(y2, x, feat);
	}
}


/*
 * Generate helper -- split a rectangle with a feature
 */
static void generate_plus(int y1, int x1, int y2, int x2, int feat)
{
	int y, x;
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(y, x0, feat);
	}

	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(y0, x, feat);
	}
}


/*
 * Generate helper -- open all sides of a rectangle with a feature
 */
static void generate_open(int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open all sides */
	cave_set_feat(y1, x0, feat);
	cave_set_feat(y0, x1, feat);
	cave_set_feat(y2, x0, feat);
	cave_set_feat(y0, x2, feat);
}


/*
 * Generate helper -- open one side of a rectangle with a feature
 */
static void generate_hole(int y1, int x1, int y2, int x2, int feat)
{
	int y0, x0;

	/* Center */
	y0 = (y1 + y2) / 2;
	x0 = (x1 + x2) / 2;

	/* Open random side */
	switch (rand_int(4))
	{
		case 0:
		{
			cave_set_feat(y1, x0, feat);
			break;
		}
		case 1:
		{
			cave_set_feat(y0, x1, feat);
			break;
		}
		case 2:
		{
			cave_set_feat(y2, x0, feat);
			break;
		}
		case 3:
		{
			cave_set_feat(y0, x2, feat);
			break;
		}
	}
}


/*
 * Find a good spot for the next room.  
 *
 * Find and allocate a free space in the dungeon large enough to hold 
 * the room calling this function.
 *
 * We allocate space in 11x11 blocks, but want to make sure that rooms 
 * align neatly on the standard screen.  Therefore, we make them use 
 * blocks in few 11x33 rectangles as possible.
 *
 * Be careful to include the edges of the room in height and width!
 *
 * Return TRUE and values for the center of the room if all went well.  
 * Otherwise, return FALSE.
 */
static bool find_space(int *y, int *x, int height, int width)
{
	int i;
	int by, bx, by1, bx1, by2, bx2;
	int block_y, block_x;

	bool filled;


	/* Find out how many blocks we need. */
	int blocks_high = 1 + ((height - 1) / BLOCK_HGT);
	int blocks_wide = 1 + ((width - 1) / BLOCK_WID);

	/* Sometimes, little rooms like to have more space. */
	if ((blocks_wide == 2) && (rand_int(3) == 0)) blocks_wide = 3;
	else if ((blocks_wide == 1) && (rand_int(2) == 0)) 
		blocks_wide = 1 + randint(2);


	/* We'll allow twenty-five guesses. */
	for (i = 0; i < 25; i++)
	{
		filled = FALSE;

		/* Pick a top left block at random */
		block_y = rand_int(dun->row_rooms + blocks_high);
		block_x = rand_int(dun->col_rooms + blocks_wide);


		/* Itty-bitty rooms can shift about within their rectangle */
		if (blocks_wide < 3)
		{
			/* Rooms that straddle a border must shift. */
			if ((blocks_wide == 2) && ((block_x % 3) == 2))
			{
				if (rand_int(2) == 0) block_x--;
				else block_x++;
			}
		}

		/* Rooms with width divisible by 3 get fitted to a rectangle. */
		else if ((blocks_wide % 3) == 0)
		{
			/* Align to the left edge of a 11x33 rectangle. */
			if ((block_x % 3) == 2) block_x++;
			if ((block_x % 3) == 1) block_x--;
		}

		/*
		 * Big rooms that do not have a width divisible by 3 get 
		 * aligned towards the edge of the dungeon closest to them.
		 */
		else
		{
			/* Shift towards left edge of dungeon. */
			if (block_x + (blocks_wide / 2) <= dun->col_rooms / 2)
			{
				if (((block_x % 3) == 2) && ((blocks_wide % 3) == 2)) 
					block_x--;
				if ((block_x % 3) == 1) block_x--;
			}

			/* Shift toward right edge of dungeon. */
			else
			{
				if (((block_x % 3) == 2) && ((blocks_wide % 3) == 2)) 
					block_x++;
				if ((block_x % 3) == 1) block_x++;
			}
		}

		/* Extract blocks */
		by1 = block_y + 0;
		bx1 = block_x + 0;
		by2 = block_y + blocks_high;
		bx2 = block_x + blocks_wide;

		/* Never run off the screen */
		if ((by1 < 0) || (by2 > dun->row_rooms)) continue;
		if ((bx1 < 0) || (bx2 > dun->col_rooms)) continue;

		/* Verify available space */
		for (by = by1; by < by2; by++)
		{
			for (bx = bx1; bx < bx2; bx++)
			{
				if (dun->room_map[by][bx])
				{
					filled = TRUE;
				}
			}
		}

		/* If space filled, try again. */
		if (filled) continue;


		/* It is *extremely* important that the following calculation */
		/* be *exactly* correct to prevent memory errors XXX XXX XXX */

		/* Get the location of the room */
		(*y) = ((by1 + by2) * BLOCK_HGT) / 2;
		(*x) = ((bx1 + bx2) * BLOCK_WID) / 2;


		/* Save the room location */
		if (dun->cent_n < CENT_MAX)
		{
			dun->cent[dun->cent_n].y = *y;
			dun->cent[dun->cent_n].x = *x;
			dun->cent_n++;
		}

		/* Reserve some blocks.  Mark each with the room index. */
		for (by = by1; by < by2; by++)
		{
			for (bx = bx1; bx < bx2; bx++)
			{
				dun->room_map[by][bx] = dun->cent_n;
			}
		}

		/* Success. */
		return (TRUE);
	}

	/* Failure. */
	return (FALSE);
}



/*
 * Is a feature passable (with or without some work) by the character?
 */
static bool passable(int feat)
{
	/* Some kinds of terrain are passable. */
	if ((feat == FEAT_FLOOR ) || 
	    (feat == FEAT_SECRET) || 
	    (feat == FEAT_RUBBLE) || 
	    (feat == FEAT_INVIS ) || 
	    (feat == FEAT_OPEN  ) || 
	    (feat == FEAT_BROKEN) || 
	    (feat == FEAT_LESS  ) || 
	    (feat == FEAT_MORE  )) return (TRUE);

	/* Doors are passable. */
	if ((feat >= FEAT_DOOR_HEAD) && 
	    (feat <= FEAT_DOOR_TAIL)) return (TRUE);

	/* Everything else is not passable. */
	return (FALSE);
}


/*
 * Room building routines.
 *
 * Seven basic room types:
 *   1 -- normal
 *   2 -- overlapping
 *   3 -- cross shaped
 *   4 -- large room with features
 *   5 -- monster pits
 *   6 -- simple vaults
 *   7 -- greater vaults
 */


/*
 * Type 1 -- normal rectangular rooms
 *
 * These rooms have the lowest build priority (this means that they 
 * should not be very large), and are by far the most common type.
 */
static bool build_type1(void)
{
	int y, x, rand;
	int y0, x0;

	int y1, x1, y2, x2;

	bool light = FALSE;

	/* Occasional light */
	if (p_ptr->depth <= randint(35)) light = TRUE;


	/* Pick a room size (less border walls) */
	x = 1 + randint(11) + randint(11);
	y = 1 + randint(4) + randint(4);

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y0, &x0, y+2, x+2)) return (FALSE);

	/* Locate the room */
	y1 = y0 - y / 2;
	x1 = x0 - x / 2;
	y2 =  y1 + y - 1;
	x2 =  x1 + x - 1;


	/* Generate new room.  Quit immediately if out of bounds. */
	if (!generate_room(y1-1, x1-1, y2+1, x2+1, light)) return (FALSE);


	/* Generate outer walls */
	generate_draw(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Make a standard room. */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);

	/* Sometimes, we get creative. */
	if (rand_int(36) == 0)
	{
		/* Choose a room type.  Some types require odd dimensions. */
		if ((y % 2 == 0) || (x % 2 == 0)) rand = 60 + rand_int(40);
		else rand = rand_int(100);

		/* Pillar room (requires odd dimensions) */
		if (rand < 45)
		{
			int offsety = 0;
			int offsetx = 0;
			if (rand_int(2) == 0) offsety = 1;
			if (rand_int(2) == 0) offsetx = 1;

			for (y = y1 + offsety; y <= y2 - offsety; y += 2)
			{
				for (x = x1 + offsetx; x <= x2 - offsetx; x += 2)
				{
					cave_set_feat(y, x, FEAT_WALL_INNER);
				}
			}
		}

		/* Ragged-edge room (requires odd dimensions) */
		else if (rand < 90)
		{
			int offset = 0;
			if (rand_int(2) == 0) offset = 1;

			for (y = y1 + offset; y <= y2 - offset; y += 2)
			{
				cave_set_feat(y, x1, FEAT_WALL_INNER);
				cave_set_feat(y, x2, FEAT_WALL_INNER);
			}

			for (x = x1 + offset; x <= x2 - offset; x += 2)
			{
				cave_set_feat(y1, x, FEAT_WALL_INNER);
				cave_set_feat(y2, x, FEAT_WALL_INNER);
			}
		}

		/* The ceiling has collapsed. */
		else
		{
			for (y = y1; y <= y2; y++)
			{
				for (x = x1; x <= x2; x++)
				{
					/* Wall (or floor) type */
					int t = rand_int(100);

					/* Granite */
					if (t < 5)
					{
						/* Create granite wall */
						cave_set_feat(y, x, FEAT_WALL_EXTRA);
					}

					/* Quartz */
					else if (t < 12)
					{
						/* Create quartz vein */
						cave_set_feat(y, x, FEAT_QUARTZ);
					}

					/* Magma */
					else if (t < 20)
					{
						/* Create magma vein */
						cave_set_feat(y, x, FEAT_MAGMA);
					}

					/* Rubble. */
					else if (t < 40)
					{
						/* Create rubble */
						cave_set_feat(y, x, FEAT_RUBBLE);
					}

					/* Floor */
					else
					{
						/* Create floor */
						cave_set_feat(y, x, FEAT_FLOOR);
					}
				}
			}

			/* Here, creatures of Earth dwell. */
			if ((p_ptr->depth > 35) && (rand_int(3) == 0))
			{
				spread_monsters('X', p_ptr->depth, 2 + randint(3), 
					y0, x0, 3, 9);
			}
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Type 2 -- Overlapping rectangular rooms
 */
static bool build_type2(void)
{
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;
	int y0, x0;
	int height, width;

	int light = FALSE;

	/* Occasional light */
	if (p_ptr->depth <= randint(35)) light = TRUE;


	/* Determine extents of room (a) */
	y1a = randint(4);
	x1a = randint(13);
	y2a = randint(3);
	x2a = randint(9);

	/* Determine extents of room (b) */
	y1b = randint(3);
	x1b = randint(9);
	y2b = randint(4);
	x2b = randint(13);


	/* Calculate height */
	height = 11;

	/* Calculate width */
	if ((x1a < 8) && (x2a < 9) && (x1b < 8) && (x2b < 9)) width = 22;
	else width = 33;

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y0, &x0, height, width)) return (FALSE);

	/* locate room (a) */
	y1a = y0 - y1a;
	x1a = x0 - x1a;
	y2a = y0 + y2a;
	x2a = x0 + x2a;

	/* locate room (b) */
	y1b = y0 - y1b;
	x1b = x0 - x1b;
	y2b = y0 + y2b;
	x2b = x0 + x2b;


	/* Generate new room (a).  Quit immediately if out of bounds. */
	if (!generate_room(y1a-1, x1a-1, y2a+1, x2a+1, light)) return (FALSE);

	/* Generate new room (b).  Quit immediately if out of bounds. */
	if (!generate_room(y1b-1, x1b-1, y2b+1, x2b+1, light)) return (FALSE);


	/* Generate outer walls (a) */
	generate_draw(y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_draw(y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(y1b, x1b, y2b, x2b, FEAT_FLOOR);

	/* Success */
	return (TRUE);
}



/*
 * Type 3 -- Cross shaped rooms
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So a "central pillar" would run from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work for 5x5 (and perhaps even for asymmetric
 * values like 4x3 or 5x3 or 3x4 or 3x5).
 */
static bool build_type3(void)
{
	int y, x;
	int y0, x0;
	int height, width;

	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	int dy, dx, wy, wx;

	int light = FALSE;

	/* Occasional light */
	if (p_ptr->depth <= randint(35)) light = TRUE;


	/* Pick inner dimension */
	wy = 1;
	wx = 1;

	/* Pick outer dimension */
	dy = rand_range(3, 4);
	dx = rand_range(3, 11);

	/* Determine extents of room (a) */
	y1a = dy;
	x1a = wx;
	y2a = dy;
	x2a = wx;

	/* Determine extents of room (b) */
	y1b = wy;
	x1b = dx;
	y2b = wy;
	x2b = dx;

	/* Calculate height */
	if ((y1a + y2a + 1) > (y1b + y2b + 1)) height = y1a + y2a + 1;
	else height = y1b + y2b + 1;

	/* Calculate width */
	if ((x1a + x2a + 1) > (x1b + x2b + 1)) width = x1a + x2a + 1;
	else width = x1b + x2b + 1;

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y0, &x0, height, width)) return (FALSE);

	/* Locate room (a) */
	y1a = y0 - dy;
	x1a = x0 - wx;
	y2a = y0 + dy;
	x2a = x0 + wx;

	/* Locate room (b) */
	y1b = y0 - wy;
	x1b = x0 - dx;
	y2b = y0 + wy;
	x2b = x0 + dx;


	/* Generate new room (a).  Quit immediately if out of bounds. */
	if (!generate_room(y1a-1, x1a-1, y2a+1, x2a+1, light)) return (FALSE);

	/* Generate new room (b).  Quit immediately if out of bounds. */
	if (!generate_room(y1b-1, x1b-1, y2b+1, x2b+1, light)) return (FALSE);


	/* Generate outer walls (a) */
	generate_draw(y1a-1, x1a-1, y2a+1, x2a+1, FEAT_WALL_OUTER);

	/* Generate outer walls (b) */
	generate_draw(y1b-1, x1b-1, y2b+1, x2b+1, FEAT_WALL_OUTER);

	/* Generate inner floors (a) */
	generate_fill(y1a, x1a, y2a, x2a, FEAT_FLOOR);

	/* Generate inner floors (b) */
	generate_fill(y1b, x1b, y2b, x2b, FEAT_FLOOR);


	/* Special features */
	switch (randint(4))
	{
		/* Nothing */
		case 1:
		{
			break;
		}

		/* Large solid middle pillar */
		case 2:
		{
			/* Generate a small inner solid pillar */
			generate_fill(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			break;
		}

		/* Inner treasure vault */
		case 3:
		{
			/* Generate a small inner vault */
			generate_draw(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);

			/* Open the inner vault with a secret door */
			generate_hole(y1b, x1a, y2b, x2a, FEAT_SECRET);

			/* Place a slightly out-of-depth treasure in the vault */
			place_object(y0, x0, p_ptr->depth + 2, FALSE, FALSE);

			/* Let's guard the treasure well */
			(void)place_monster(y0, x0, p_ptr->depth + 4, TRUE, TRUE);

			/* Traps, naturally. */
			spread_traps(randint(3), y0, x0, 4, 4);

			break;
		}

		/* Something else */
		case 4:
		{
			/* Occasionally pinch the center shut */
			if (rand_int(3) == 0)
			{
				/* Pinch the east/west sides */
				for (y = y1b; y <= y2b; y++)
				{
					if (y == y0) continue;
					cave_set_feat(y, x1a - 1, FEAT_WALL_INNER);
					cave_set_feat(y, x2a + 1, FEAT_WALL_INNER);
				}

				/* Pinch the north/south sides */
				for (x = x1a; x <= x2a; x++)
				{
					if (x == x0) continue;
					cave_set_feat(y1b - 1, x, FEAT_WALL_INNER);
					cave_set_feat(y2b + 1, x, FEAT_WALL_INNER);
				}

				/* Open sides with secret doors */
				if (rand_int(3) == 0)
				{
					generate_open(y1b-1, x1a-1, y2b+1, x2a+1, FEAT_SECRET);
				}
			}

			/* Occasionally put a "plus" in the center */
			else if (rand_int(3) == 0)
			{
				generate_plus(y1b, x1a, y2b, x2a, FEAT_WALL_INNER);
			}

			/* Occasionally put a "pillar" in the center */
			else if (rand_int(3) == 0)
			{
				cave_set_feat(y0, x0, FEAT_WALL_INNER);
			}

			break;
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Type 4 -- Large room with an inner room
 *
 * Possible sub-types:
 *	1 - An inner room with a small inner room
 *	2 - An inner room with a pillar or pillars
 *	3 - An inner room with a checkerboard
 *	4 - An inner room with four compartments
 */
static bool build_type4(void)
{
	int y, x, y1, x1, y2, x2;
	int y0, x0;

	int light = FALSE;

	/* Occasional light */
	if (p_ptr->depth <= randint(35)) light = TRUE;


	/* Pick a room size (less border walls) */
	y = 9;
	x = 23;

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y0, &x0, y+2, x+2)) return (FALSE);

	/* Locate the room */
	y1 = y0 - y / 2;
	x1 = x0 - x / 2;
	y2 =  y1 + y - 1;
	x2 =  x1 + x - 1;


	/* Generate new room.  Quit immediately if out of bounds. */
	if (!generate_room(y1-1, x1-1, y2+1, x2+1, light)) return (FALSE);


	/* Generate outer walls */
	generate_draw(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);


	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_draw(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Inner room variations */
	switch (randint(4))
	{
		/* An inner room with a small inner room */
		case 1:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Place another inner room */
			generate_draw(y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Open the inner room with a locked door */
			generate_hole(y0-1, x0-1, y0+1, x0+1, FEAT_DOOR_HEAD + randint(7));

			/* Monsters on guard */
			spread_monsters('\0', p_ptr->depth + 2, 4, y0, x0, 2, 6);

			/* Object (80%), slightly out-of-depth value */
			if (rand_int(100) < 80)
			{
				place_object(y0, x0, p_ptr->depth + 2, FALSE, FALSE);
			}

			/* Stairs (20%) */
			else
			{
				place_random_stairs(y0, x0);
			}

			/* Traps */
			spread_traps(rand_int(3) + 1, y0, x0, 2, 4);

			break;
		}


		/* An inner room with an inner pillar or pillars */
		case 2:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Inner pillar */
			generate_fill(y0-1, x0-1, y0+1, x0+1, FEAT_WALL_INNER);

			/* Occasionally, two more large inner pillars */
			if (rand_int(2) == 0)
			{
				/* Three spaces */
				if (rand_int(100) < 50)
				{
					/* Inner pillar */
					generate_fill(y0-1, x0-7, y0+1, x0-5, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(y0-1, x0+5, y0+1, x0+7, FEAT_WALL_INNER);
				}

				/* Two spaces */
				else
				{
					/* Inner pillar */
					generate_fill(y0-1, x0-6, y0+1, x0-4, FEAT_WALL_INNER);

					/* Inner pillar */
					generate_fill(y0-1, x0+4, y0+1, x0+6, FEAT_WALL_INNER);
				}
			}

			/* Occasionally, some inner rooms */
			if (rand_int(3) == 0)
			{
				/* Inner rectangle */
				generate_draw(y0-1, x0-5, y0+1, x0+5, FEAT_WALL_INNER);

				/* Secret doors (random top/bottom) */
				place_secret_door(y0 - 3 + (randint(2) * 2), x0 - 3);
				place_secret_door(y0 - 3 + (randint(2) * 2), x0 + 3);

				/* Monsters */
				spread_monsters('\0', p_ptr->depth, randint(4), y0, x0, 2, 7);

				/* Objects */
				if (rand_int(3) == 0)
					place_object(y0, x0 - 2, p_ptr->depth, FALSE, FALSE);
				if (rand_int(3) == 0)
					place_object(y0, x0 + 2, p_ptr->depth, FALSE, FALSE);
			}

			break;
		}

		/* An inner room with a checkerboard */
		case 3:
		{
			/* Open the inner room with a secret door */
			generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);

			/* Checkerboard */
			for (y = y1; y <= y2; y++)
			{
				for (x = x1; x <= x2; x++)
				{
					if ((x + y) & 0x01)
					{
						cave_set_feat(y, x, FEAT_WALL_INNER);
					}
				}
			}

			/* Monsters (especially undead) just love mazes. */
			if (rand_int(3) == 0)
				spread_monsters('N', p_ptr->depth, randint(6), y0, x0, 2, 9);
			else if (rand_int(3) == 0)
				spread_monsters('*', p_ptr->depth, randint(6), y0, x0, 2, 9);
			else
				spread_monsters('\0', p_ptr->depth, randint(6), y0, x0, 2, 9);


			/* Traps make them entertaining. */
			spread_traps(2 + rand_int(4), y0, x0, 2, 9);

			/* Mazes should have some pretty good treasure too. */
			spread_objects(p_ptr->depth, 2 + rand_int(4), y0, x0, 2, 9);

			break;
		}

		/* Four small rooms. */
		case 4:
		{
			/* Inner "cross" */
			generate_plus(y1, x1, y2, x2, FEAT_WALL_INNER);

			/* Doors into the rooms */
			if (rand_int(100) < 50)
			{
				int i = randint(10);
				place_secret_door(y1 - 1, x0 - i);
				place_secret_door(y1 - 1, x0 + i);
				place_secret_door(y2 + 1, x0 - i);
				place_secret_door(y2 + 1, x0 + i);
			}
			else
			{
				int i = randint(3);
				place_secret_door(y0 + i, x1 - 1);
				place_secret_door(y0 - i, x1 - 1);
				place_secret_door(y0 + i, x2 + 1);
				place_secret_door(y0 - i, x2 + 1);
			}

			/* Treasure, centered at the center of the cross */
			spread_objects(p_ptr->depth, 2 + randint(2), y0, x0, 1, 1);

			/* Gotta have some monsters */
			spread_monsters('\0', p_ptr->depth, 6 + rand_int(11), y0, x0, 2, 9);

			break;
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Type 5 -- Monster pits
 *
 * A monster pit is a 11x33 room, with an inner room filled with monsters.
 * 
 * The type of monsters is determined by inputing the current dungeon 
 * level into "mon_symbol_at_depth", and accepting the character returned.
 * After translating this into a set of selection criteria, monsters are 
 * chosen and arranged in the inner room.
 *
 * Monster pits will never contain unique monsters.
 *
 */
static bool build_type5(void)
{
	int y, x, y0, x0, y1, x1, y2, x2;
	int i, j;
	int depth;
	int choice;

	char *name;
	char symbol;

	bool ordered = FALSE;
	bool dummy;
	int light = FALSE;


	/* Pick a room size (less border walls) */
	y = 9;
	x = 23;

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y0, &x0, y+2, x+2)) return (FALSE);

	/* Locate the room */
	y1 = y0 - y / 2;
	x1 = x0 - x / 2;
	y2 =  y1 + y - 1;
	x2 =  x1 + x - 1;


	/* Generate new room.  Quit immediately if out of bounds. */
	if (!generate_room(y1-1, x1-1, y2+1, x2+1, light)) return (FALSE);


	/* Generate outer walls */
	generate_draw(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_OUTER);

	/* Generate inner floors */
	generate_fill(y1, x1, y2, x2, FEAT_FLOOR);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* Generate inner walls */
	generate_draw(y1-1, x1-1, y2+1, x2+1, FEAT_WALL_INNER);

	/* Open the inner room with a secret door */
	generate_hole(y1-1, x1-1, y2+1, x2+1, FEAT_SECRET);


	/* Get a legal depth. */
	depth = p_ptr->depth + rand_int(11) - 5;
	if (depth > 60) depth = 60;
	if (depth <  5) depth =  5;

	/* Pick a table position at random, favoring the first seven. */
	if (rand_int(5) == 0) choice = 7 + rand_int(6);
	else                  choice = rand_int(7);

	/* Choose a monster type, using that depth and table position. */
	symbol = mon_symbol_at_depth[depth / 5 - 1][choice];


	/* Allow tougher monsters. */
	depth = p_ptr->depth + 3 + (p_ptr->depth < 70 ? p_ptr->depth/7 : 10);


	/* 
	 * Set monster generation restrictions.  Decide how to order 
	 * monsters.  Get a description of the monsters.
	 */
	name = mon_restrict(symbol, (byte)depth, &ordered, FALSE);

	/* A default description probably means trouble, so stop. */
	if (streq(name, "misc") || !name[0]) return (TRUE);

	/* Build the monster probability table.  Leave the room empty on failure. */
	if (!get_mon_num(depth)) return (TRUE);


	/* Arrange the monsters in the room randomly. */
	if (!ordered)
	{
		int r_idx = 0;

		/* Place some monsters */
		for (y = y0 - 2; y <= y0 + 2; y++)
		{
			for (x = x0 - 9; x <= x0 + 9; x++)
			{
				/* Get a monster index */
				r_idx = get_mon_num(depth);

				/* Place a single monster */
				(void)place_monster_aux(y, x, r_idx, FALSE, FALSE);
			}
		}
	}

	/* Arrange the monsters in the room in an orderly fashion. */
	else
	{
		s16b what[16];

		/* Pick some monster types */
		for (i = 0; i < 16; i++)
		{
			/* Get a monster index */
			what[i] = get_mon_num(depth);
		}

		/* Sort the monsters */
		for (i = 0; i < 16 - 1; i++)
		{
			for (j = 0; j < 16 - 1; j++)
			{
				int i1 = j;
				int i2 = j + 1;

				int p1 = r_info[what[i1]].level;
				int p2 = r_info[what[i2]].level;

				/* Bubble sort */
				if (p1 > p2)
				{
					int tmp = what[i1];
					what[i1] = what[i2];
					what[i2] = tmp;
				}
			}
		}


		/* Top and bottom rows (outer) */
		for (x = x0 - 9; x <= x0 - 4; x++)
		{
			place_monster_aux(y0 - 2, x, what[2], FALSE, FALSE);
			place_monster_aux(y0 + 2, x, what[2], FALSE, FALSE);
		}
		for (x = x0 + 4; x <= x0 + 9; x++)
		{
			place_monster_aux(y0 - 2, x, what[2], FALSE, FALSE);
			place_monster_aux(y0 + 2, x, what[2], FALSE, FALSE);
		}

		/* Top and bottom rows (inner) */
		for (x = x0 - 3; x <= x0 + 3; x++)
		{
			place_monster_aux(y0 - 2, x, what[3], FALSE, FALSE);
			place_monster_aux(y0 + 2, x, what[3], FALSE, FALSE);
		}

		/* Middle columns */
		for (y = y0 - 1; y <= y0 + 1; y++)
		{
			place_monster_aux(y, x0 - 9, what[2], FALSE, FALSE);
			place_monster_aux(y, x0 + 9, what[2], FALSE, FALSE);

			place_monster_aux(y, x0 - 8, what[4], FALSE, FALSE);
			place_monster_aux(y, x0 + 8, what[4], FALSE, FALSE);

			place_monster_aux(y, x0 - 7, what[5], FALSE, FALSE);
			place_monster_aux(y, x0 + 7, what[5], FALSE, FALSE);

			place_monster_aux(y, x0 - 6, what[6], FALSE, FALSE);
			place_monster_aux(y, x0 + 6, what[6], FALSE, FALSE);

			place_monster_aux(y, x0 - 5, what[7], FALSE, FALSE);
			place_monster_aux(y, x0 + 5, what[7], FALSE, FALSE);

			place_monster_aux(y, x0 - 4, what[8], FALSE, FALSE);
			place_monster_aux(y, x0 + 4, what[8], FALSE, FALSE);

			place_monster_aux(y, x0 - 3, what[9], FALSE, FALSE);
			place_monster_aux(y, x0 + 3, what[9], FALSE, FALSE);

			place_monster_aux(y, x0 - 2, what[11], FALSE, FALSE);
			place_monster_aux(y, x0 + 2, what[11], FALSE, FALSE);
		}

		/* Above/Below the center monster */
		for (x = x0 - 1; x <= x0 + 1; x++)
		{
			place_monster_aux(y0 + 1, x, what[12], FALSE, FALSE);
			place_monster_aux(y0 - 1, x, what[12], FALSE, FALSE);
		}

		/* Next to the center monster */
		place_monster_aux(y0, x0 + 1, what[14], FALSE, FALSE);
		place_monster_aux(y0, x0 - 1, what[14], FALSE, FALSE);

		/* Center monster */
		place_monster_aux(y0, x0, what[15], FALSE, FALSE);
	}

	/* Remove restrictions */
	(void)mon_restrict('\0', (byte)depth, &dummy, FALSE);


	/* Describe */
	if (cheat_room)
	{
		/* Room type */
		msg_format("Monster pit (%s)", name);
	}

	/* Increase the level rating */
	rating += 10;

	/* Sometimes cause a special feeling */
	if ((randint(50) >= p_ptr->depth) && (rand_int(2) == 0))
	{
		good_item_flag = TRUE;
	}


	/* Success */
	return (TRUE);
}



/*
 * Use information from the "v_info" array to fill in vault rooms.
 *
 * We mark grids "icky" to indicate the presence of a vault.
 */
static bool build_vault(int y0, int x0, int ymax, int xmax, cptr data, 
	byte vault_type)
{
	int x, y;
	int y1, x1, y2, x2;

	cptr t;


	/* Calculate the borders of the vault */
	y1 = y0 - (ymax / 2);
	x1 = x0 - (xmax / 2);
	y2 = y1 + ymax - 1;
	x2 = x1 + xmax - 1;

	/* Make certain that the vault does not cross the dungeon edge */
	if ((!in_bounds(y1, x1)) || (!in_bounds(y2, x2))) return (FALSE);


	/* Place dungeon features and objects */
	for (t = data, y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++, t++)
		{
			/* Hack -- skip "non-grids" */
			if (*t == ' ') 
			{
				continue;
			}

			/* Lay down a floor */
			cave_set_feat(y, x, FEAT_FLOOR);

			/* Part of a vault */
			cave_info[y][x] |= (CAVE_ROOM | CAVE_ICKY);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
				case '%':
				{
					cave_set_feat(y, x, FEAT_WALL_OUTER);
					break;
				}

				/* Granite wall (inner) */
				case '#':
				{
					cave_set_feat(y, x, FEAT_WALL_INNER);
					break;
				}
				/* Permanent wall (inner) */
				case 'X':
				{
					cave_set_feat(y, x, FEAT_PERM_INNER);
					break;
				}

				/* Treasure/trap */
				case '*':
				{
					if (rand_int(100) < 75)
					{
						place_object(y, x, p_ptr->depth, FALSE, FALSE);
					}
					else
					{
						place_trap(y, x);
					}
					break;
				}

				/* Secret doors */
				case '+':
				{
					place_secret_door(y, x);
					break;
				}

				/* Trap */
				case '^':
				{
					place_trap(y, x);
					break;
				}
			}
		}
	}

	/* Place dungeon monsters and objects */
	for (t = data, y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++, t++)
		{
			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Analyze the grid */
			switch (*t)
			{
				/* Monster */
				case '&':
				{
					place_monster(y, x, p_ptr->depth + 5, TRUE, TRUE);
					break;
				}

				/* Meaner monster */
				case '@':
				{
					place_monster(y, x, p_ptr->depth + 11, TRUE, TRUE);
					break;
				}

				/* Meaner monster, plus treasure */
				case '9':
				{
					place_monster(y, x, p_ptr->depth + 9, TRUE, TRUE);
					place_object(y, x, p_ptr->depth + 7, TRUE, FALSE);
					break;
				}

				/* Nasty monster and treasure */
				case '8':
				{
					place_monster(y, x, p_ptr->depth + 40, TRUE, TRUE);
					place_object(y, x, p_ptr->depth + 20, TRUE, TRUE);
					break;
				}

				/* Monster and/or object */
				case ',':
				{
					if (rand_int(100) < 50)
					{
						place_monster(y, x, p_ptr->depth + 3, TRUE, TRUE);
					}
					if (rand_int(100) < 50)
					{
						/* Moderately out-of-depth item */
						place_object(y, x, p_ptr->depth + 7, FALSE, FALSE);
					}
					break;
				}
			}
		}
	}

	/* Success. */
	return (TRUE);
}

/*
 * Type 6 -- lesser vaults.
 */
static bool build_type6(void)
{
	vault_type *v_ptr;
	int y, x;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Get a random vault record */
		v_ptr = &v_info[rand_int(z_info->v_max)];

		/* Accept the first lesser vault */
		if (v_ptr->typ == 6) break;
	}

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y, &x, v_ptr->hgt, v_ptr->wid)) return (FALSE);


	/* Message */
	if (cheat_room) msg_format("Lesser vault (%s)", v_name + v_ptr->name);

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((p_ptr->depth <= 50) ||
	    (randint((p_ptr->depth - 40) * (p_ptr->depth - 40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}


	/* Build the vault */
	if (!build_vault(y, x, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text, 
		6)) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
 * Type 7 -- greater vaults.
 */
static bool build_type7(void)
{
	vault_type *v_ptr;
	int y, x;

	/* Pick a lesser vault */
	while (TRUE)
	{
		/* Get a random vault record */
		v_ptr = &v_info[rand_int(z_info->v_max)];

		/* Accept the first greater vault */
		if (v_ptr->typ == 7) break;
	}

	/* Find and reserve some space in the dungeon.  Get center of room. */
	if (!find_space(&y, &x, v_ptr->hgt, v_ptr->wid)) return (FALSE);


	/* Message */
	if (cheat_room) msg_format("Greater vault (%s)", v_name + v_ptr->name);

	/* Boost the rating */
	rating += v_ptr->rat;

	/* Greater vaults are special */
	good_item_flag = TRUE;


	/* Build the vault (never lit, icky, type 7) */
	if (!build_vault(y, x, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text, 
		7)) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Helper function that reads the room data table and returns the number 
 * of rooms, of a given type, we should build on this level.
 */
static int num_rooms_allowed(int room_type)
{
	int allowed = 0;
	int base_num, num_tries, mod, i;

	/* Point to the room information. */
	room_data *rm_ptr = &room[room_type];


	/* No rooms allowed above their minimum depth. */
	if (p_ptr->depth < rm_ptr->min_level) return (0);

	/* No "nothing" rooms. */
	if (room_type == 0) return (0);

	/* No special limit on ordinary rooms. */
	if (room_type == 1) return (DUN_ROOMS);


	/* If below level 100, use the rarity value for level 100. */
	if (p_ptr->depth > 100)
	{
		base_num = rm_ptr->room_gen_num[10];
	}
	else
	{
		mod = p_ptr->depth % 10;

		/* If depth is divisable by 10, use the appropriate table value. */
		if (mod == 0) 
		{
			base_num = rm_ptr->room_gen_num[p_ptr->depth / 10];
		}
		/* Otherwise, use a weighted average of the nearest values. */
		else
		{
			base_num = ((mod * rm_ptr->room_gen_num[(p_ptr->depth + 9) / 10]) + 
				((10 - mod) * rm_ptr->room_gen_num[p_ptr->depth / 10])) / 10;
		}
	}

	/* Find out how many times we'll try to boost the room count. */
	num_tries = 3 * base_num / 100;
	if (num_tries < 2) num_tries = (base_num < 12 ? 1 : 2);
	if (num_tries > DUN_ROOMS / 2) num_tries = DUN_ROOMS / 2;


	/* Try several times to increase the number of rooms to build. */
	for (i = 0; i < num_tries; i++)
	{
		if (rand_int(1000) < 10 * base_num / num_tries)
		{
			allowed++;
		}
	}

	/* Return the number of rooms of that type we should build. */
	return (allowed);
}


/*
 * Build a room of the given type.
 * 
 * Check to see if there will probably be enough space in the monster 
 * and object arrays.
 */
static bool room_build(int room_type)
{
	/* If trying to build a special room, check some limits first. */
	if (room_type > 4)
	{
		/* Help prevent object over-flow */
		if (o_max > 3 * z_info->o_max / 4)
		{
			return (FALSE);
		}

		/* Help prevent monster over-flow */
		if (mon_max > 3 * z_info->m_max / 4)
		{
			return (FALSE);
		}
	}


	/* Build a room */
	switch (room_type)
	{
		/* Find space for, position, and build the room asked for */
		case  7: if (!build_type7())  return (FALSE); break;
		case  6: if (!build_type6())  return (FALSE); break;
		case  5: if (!build_type5())  return (FALSE); break;
		case  4: if (!build_type4())  return (FALSE); break;
		case  3: if (!build_type3())  return (FALSE); break;
		case  2: if (!build_type2())  return (FALSE); break;
		case  1: if (!build_type1())  return (FALSE); break;

		/* Paranoia */
		default: return (FALSE);
	}

	/* Success */
	return (TRUE);
}



/**************************************************************/
/*                                                            */
/*                     The tunnelling code                    */
/*                                                            */
/**************************************************************/


/*
 * Given a current position (y1, x1), move towards the target grid 
 * (y2, x2) either vertically or horizontally.
 *
 * If both vertical and horizontal directions seem equally good, 
 * prefer to move horizontally.
 */
static void correct_dir(int *row_dir, int *col_dir, int y1, int x1, int y2, int x2)
{
	/* Move vertically if vertical distance to target is greater. */
	if (ABS(y1 - y2) > ABS(x1 - x2))
	{
		*row_dir = ((y1 < y2) ? 1 : -1);
		*col_dir = 0;
	}

	/* Prefer to move horizontally. */
	else
	{
		*row_dir = 0;
		*col_dir = ((x1 < x2) ? 1 : -1);
	}
}


/*
 * Go in a semi-random direction from current location to target location.  
 * Do not actually head away from the target grid.  Always make a turn.
 */
static void adjust_dir(int *row_dir, int *col_dir, int y1, int x1, int y2, int x2)
{
	/* Always turn 90 degrees. */
	if (*row_dir == 0)
	{
		*col_dir = 0;

		/* On the y-axis of target - freely choose a side to turn to. */
		if (y1 == y2) *row_dir = ((rand_int(2) == 0) ? - 1 : 1);

		/* Never turn away from target. */
		else *row_dir = ((y1 < y2) ? 1 : -1);
	}
	else
	{
		*row_dir = 0;

		/* On the x-axis of target - freely choose a side to turn to. */
		if (x1 == x2) *col_dir = ((rand_int(2) == 0) ? - 1 : 1);

		/* Never turn away from target. */
		else *col_dir = ((x1 < x2) ? 1 : -1);
	}
}


/*
 * Go in a completely random orthongonal direction.  If we turn around 
 * 180 degrees, save the grid; it may be a good place to place stairs 
 * and/or the player.
 */
static void rand_dir(int *row_dir, int *col_dir, int y, int x)
{
	/* Pick a random direction */
	int i = rand_int(4);

	/* Extract the dy/dx components */
	int row_dir_tmp = ddy_ddd[i];
	int col_dir_tmp = ddx_ddd[i];

	/* Save useful grids. */
	if ((-(*row_dir) == row_dir_tmp) && (-(*col_dir) == col_dir_tmp))
	{
		/* Save the current tunnel location if surrounded by walls. */
		if ((in_bounds_fully(y, x)) && (dun->stair_n < STAIR_MAX) && 
			(next_to_walls(y, x) == 4))
		{
			dun->stair[dun->stair_n].y = y;
			dun->stair[dun->stair_n].x = x;
			dun->stair_n++;
		}
	}

	/* Save the new direction. */
	*row_dir = row_dir_tmp;
	*col_dir = col_dir_tmp;
}


/* Terrain type is unalterable and impassable. */
static bool unalterable(int feat)
{
	/* A few features are unalterable. */
	if ((feat == FEAT_PERM_EXTRA) ||
	    (feat == FEAT_PERM_INNER) ||
	    (feat == FEAT_PERM_OUTER) ||
	    (feat == FEAT_PERM_SOLID))
	{
		return (TRUE);
	}

	/* Assume alterable */
	return (FALSE);
}

/*
 * Given a set of coordinates, return the index number of the room occupying 
 * the dungeon block this location is in.
 */
static int get_room_index(int y, int x)
{
	/* Which block are we in? */
	int by = y / BLOCK_HGT;
	int bx = x / BLOCK_WID;

	/* Paranoia -- confirm that block is in the dungeon. */
	if ((by > MAX_ROOMS_ROW) || (by > MAX_ROOMS_ROW)) return (-1);

	/* Get the room index. */
	return (dun->room_map[by][bx] - 1);
}



/*
 * Search for a vault entrance.
 *
 * Notes:
 * - This function looks in both directions, and chooses the nearest 
 *   entrance (if it has a choice).
 * - We assume rooms will have outer walls surrounding them.
 * - We assume the vault designer hasn't designed false entrances, or
 *   done something else really sneaky.
 */
static bool find_entrance(int row_dir, int col_dir, int *row1, int *col1)
{
	int i, j;
	int y;
	int x;
	int dy, dx;

	/*
	 * Initialize entrances found while looking in both directions, and 
	 * the distances to them.
	 */
	int target_y[2] = {0, 0};
	int target_x[2] = {0, 0};
	int grids[2]    = {0, 0};


	/* Search in both directions. */
	for (i = 0; i < 2; i++)
	{
		bool stop_loop = FALSE;

		y = *row1;
		x = *col1;

		dy = row_dir;
		dx = col_dir;

		/* Keep running through the steps. */
		while (TRUE)
		{
			int dy_tmp = dy;

			/* Search grids on both sides for more impassable walls. */
			for (j = i; j < 2 + i; j++)
			{
				if (dy_tmp == 0)
				{
					dy = ((j == 1) ? - 1 : 1);
					dx = 0;
				}
				else
				{
					dy = 0;
					dx = ((j == 1) ? - 1 : 1);
				}

				/* Look in chosen direction. */
				if ((!unalterable(cave_feat[y + dy][x + dx])) && 
					(cave_feat[y + dy][x + dx] != FEAT_WALL_OUTER))
				{
					/*
					 * Check the grid after this one.  If it belongs 
					 * to the same room, we've found an entrance.
					 */
					if (get_room_index(y + dy, x + dx) == 
						get_room_index(y + dy + dy, x + dx + dx))
					{
						target_y[i] = y + dy;
						target_x[i] = x + dx;
						break;
					}
				}

				/* Look again. */
				else if (unalterable(cave_feat[y + dy][x + dx]))
				{
					break;
				}

				/* We're out on some kind of weird spur. */
				else if (j == (1 + i))
				{
					/* Stop travelling in this direction. */
					stop_loop = TRUE;
					break;
				}
			}

			/* Success or (known) failure. */
			if (target_y[i] && target_x[i]) break;
			if (stop_loop) break;

			/* Keep heading in the same direction. */
			while (TRUE)
			{
				/* Advance to new grid in our direction of travel. */
				y += dy;
				x += dx;

				/* Count the number of grids we've travelled */
				grids[i]++;

				/*
				 * We're back where we started.  Room either has no 
				 * entrances, or we can't find them.
				 */
				if ((y == *row1) && (x == *col1))
				{
					stop_loop = TRUE;
					break;
				}

				/* We have hit the dungeon edge. */
				if (!in_bounds_fully(y + dy, x + dx))
				{
					stop_loop = TRUE;
					break;
				}

				/* Next grid is outer wall. */
				if (cave_feat[y + dy][x + dx] == FEAT_WALL_OUTER)
				{
					/* We need to make another turn. */
					break;
				}

				/* Next grid is alterable, and not outer wall. */
				else if (!unalterable(cave_feat[y + dy][x + dx]))
				{
					/*
					 * Check the grid after this one.  If it belongs 
					 * to the same room, we've found an entrance.
					 */
					if (get_room_index(y + dy, x + dx) == 
						get_room_index(y + dy + dy, x + dx + dx))
					{
						target_y[i] = y + dy;
						target_x[i] = x + dx;
						break;
					}

					/*
					 * If we're in the same room, our likely best move 
					 * is to keep moving along the permanent walls.
					 */
					else
					{
						break;
					}
				}
			}

			/* Success. */
			if (target_y[i] && target_x[i]) break;

			/* Failure. */
			if (stop_loop) break;
		}
	}

	/*
	 * Compare reports.  Pick the only target available, or choose 
	 * the target that took less travelling to get to.
	 */
	if ((target_y[0] && target_x[0]) && (target_y[1] && target_x[1]))
	{
		if (grids[0] < grids[1])
		{
			*row1 = target_y[0];
			*col1 = target_x[0];
		}
		else
		{
			*row1 = target_y[1];
			*col1 = target_x[1];
		}

		return (TRUE);
	}

	else if (target_y[0] && target_x[0])
	{
		*row1 = target_y[0];
		*col1 = target_x[0];
		return (TRUE);
	}
	else if (target_y[1] && target_x[1])
	{
		*row1 = target_y[1];
		*col1 = target_x[1];
		return (TRUE);
	}

	/* No entrances found. */
	else return (FALSE);
}

/*
 * Tests suitability of potential entranceways, and places doors if appropriate.
 */
static void try_entrance(int y0, int x0)
{
	int i, k;

	/* Require walls on at least two sides. */
	for (k = 0, i = 0; i < 4; i++)
	{
		/* Extract the location */
		int y = y0 + ddy_ddd[i];
		int x = x0 + ddx_ddd[i];

		/* Ignore non-walls. */
		if (cave_feat[y][x] < FEAT_MAGMA) continue;

		/* We require at least two walls. */
		if ((k++) == 2) place_random_door(y0, x0);
	}
}

/*
 * Places door at y, x position if at least 2 walls and two corridor spaces found
 */
static void try_door(int y0, int x0)
{
	int i, y, x;
	int k = 0;


	/* Ignore walls */
	if (cave_info[y0][x0] & (CAVE_WALL)) return;

	/* Ignore room grids */
	if (cave_info[y0][x0] & (CAVE_ROOM)) return;

	/* Occasional door (if allowed) */
	if (rand_int(100) < DUN_TUN_JCT)
	{
		/* Count the adjacent non-wall grids */
		for (i = 0; i < 4; i++)
		{
			/* Extract the location */
			y = y0 + ddy_ddd[i];
			x = x0 + ddx_ddd[i];

			/* Skip impassable grids (or trees) */
			if (cave_info[y][x] & (CAVE_WALL)) continue;

			/* Skip grids inside rooms */
			if (cave_info[y][x] & (CAVE_ROOM)) continue;

			/* We require at least two walls outside of rooms. */
			if ((k++) == 2) break;
		}

		if (k == 2)
		{
			/* Check Vertical */
			if ((cave_feat[y0-1][x0] >= FEAT_MAGMA) &&
			    (cave_feat[y0+1][x0] >= FEAT_MAGMA))
			{
				place_random_door(y0, x0);
			}

			/* Check Horizontal */
			else if ((cave_feat[y0][x0-1] >= FEAT_MAGMA) &&
			    (cave_feat[y0][x0+1] >= FEAT_MAGMA))
			{
				place_random_door(y0, x0);
			}
		}
	}
}




/*
 * Constructs a tunnel between two points. 
 *
 * The tunnelling code connects room centers together.  It is the respon-
 * sibility of rooms to ensure all grids in them are accessable from the 
 * center, or from a passable grid nearby if the center is a wall.
 *
 * (warnings)
 * This code is still beta-quality.  Use with care.  Known areas of 
 * weakness include: 
 * - A group of rooms may be connected to each other, and not to the rest 
 *   of the dungeon.  This problem is rare.
 * - While the entrance-finding code is very useful, sometimes the tunnel 
 *   gets lost on the way.
 * - On occasion, a tunnel will travel far too long.  It can even (rarely) 
 *   happen that it would lock up the game if not artifically stopped.
 * - There are number of minor but annoying problems, both old and new, 
 *   like excessive usage of tunnel grids, tunnels turning areas of the
 *   dungeon into Swiss cheese, and so on.
 * - This code is awfully, awfully long.
 *
 * (Handling the outer walls of rooms)
 * In order to place doors correctly, know when a room is connected, and 
 * keep entances and exits to rooms neat, we set and use several different 
 * kinds of granite.  Because of this, we must call this function before 
 * making streamers.
 * - "Outer" walls must surround rooms.  The code can handle outer walls 
 * up to two grids thick (which is common in non-rectangular rooms).
 * - When outer wall is pierced, "solid" walls are created along the axis 
 * perpendicular to the direction of movement for three grids in each 
 * direction.  This makes entrances tidy.
 * 
 * (Handling difficult terrain)
 * When an unalterable (permanent) wall is encountered, this code is 
 * capable of finding entrances and of using waypoints.  It is anticipated 
 * that this will make vaults behave better than they did.
 *
 * Useful terrain values:
 *   FEAT_WALL_EXTRA -- granite walls
 *   FEAT_WALL_INNER -- inner room walls
 *   FEAT_WALL_OUTER -- outer room walls
 *   FEAT_WALL_SOLID -- solid room walls
 *   FEAT_PERM_INNER -- inner room walls (perma)
 *   FEAT_PERM_OUTER -- outer room walls (perma)
 *   FEAT_PERM_SOLID -- dungeon border (perma)
 */
static void build_tunnel(int start_room, int end_room)
{
	int i = 0, j = 0, tmp, y, x;
	int y0, x0, y1, x1;
	int dy, dx;

	int row_dir, col_dir;


	/* Get start and target grids. */
	int row1 = dun->cent[start_room].y;
	int col1 = dun->cent[start_room].x;
	int row2 = dun->cent[end_room].y;
	int col2 = dun->cent[end_room].x;
	int tmp_row = row1, tmp_col = col1;


	/* Store initial target, because we may have to use waypoints. */
	int initial_row2 = row2;
	int initial_col2 = col2;

	/* Not yet worried about our progress */
	int desperation = 0;

	/* Start out not allowing the placement of doors */
	bool door_flag = FALSE;

	/* Don't leave just yet */
	bool leave = FALSE;

	/* Not heading for a known entrance. */
	bool head_for_entrance = FALSE;

	/* Initialize some movement counters */
	int adjust_dir_timer = randint(DUN_TUN_ADJ * 2);
	int rand_dir_timer   = randint(DUN_TUN_RND * 2);
	int correct_dir_timer = 0;


	/* Set number of tunnel grids and room entrances to zero. */
	dun->tunn_n = 0;
	dun->wall_n = 0;

	/* Start out heading in the correct direction */
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

	/* Keep going until done (or look like we're getting nowhere). */
	while ((row1 != initial_row2) || (col1 != initial_col2))
	{
		/* Stop when tunnel is too long, or we want to stop. */
		if ((leave) || (dun->tunn_n == TUNN_MAX) || (j++ == 400)) break;

		/* 
		 * If we've reached a waypoint, the source and destination rooms 
		 * should be connected to each other now, but they may not be to 
		 * the rest of the network.  Get another room center at random, 
		 * and move towards it.
		 */
		if ((row1 == row2) && (col1 == col2))
		{
			while (TRUE)
			{
				i = rand_int(dun->cent_n);
				if ((i != start_room) && (i != end_room)) break;
			}

			row2 = initial_row2 = dun->cent[i].y;
			col2 = initial_col2 = dun->cent[i].x;

			head_for_entrance = FALSE;
		}

		/* Try moving randomly if we seem stuck. */
		else if ((row1 != tmp_row) && (col1 != tmp_col))
		{
			desperation++;

			/* Try a 90 degree turn. */
			if (desperation == 1)
			{
				adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);
				adjust_dir_timer = 3;
			}

			/* Try turning randomly. */
			else if (desperation < 4)
			{
				rand_dir(&row_dir, &col_dir, row1, col1);
				correct_dir_timer = 2;
			}
			else
			{
				/* We've run out of ideas.  Stop wasting time. */
				break;
			}
		}

		/* We're making progress. */
		else
		{
			/* No worries. */
			desperation = 0;

			/* Check room. */
			tmp = get_room_index(row1, col1);

			/* We're in our destination room - head straight for target. */
			if ((tmp == end_room) && (cave_info[row1][col1] & (CAVE_ROOM)))
			{
				correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
			}

			else
			{
				/* Count down times until next movement changes. */
				if (adjust_dir_timer > 0) adjust_dir_timer--;
				if (rand_dir_timer > 0) rand_dir_timer--;
				if (correct_dir_timer > 0) correct_dir_timer--;

				/* Make a random turn, set timer. */
				if (rand_dir_timer == 0)
				{
					rand_dir(&row_dir, &col_dir, row1, col1);

					rand_dir_timer = randint(DUN_TUN_RND * 2);
					correct_dir_timer = randint(4);
				}

				/* Adjust direction, set timer. */
				else if (adjust_dir_timer == 0)
				{
					adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);

					adjust_dir_timer = randint(DUN_TUN_ADJ * 2);
				}


				/* Go in correct direction. */
				else if (correct_dir_timer == 0)
				{
					correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

					/* Don't use again unless needed. */
					correct_dir_timer = -1;
				}
			}
		}


		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;

		/* Do not leave the dungeon */
		if (!in_bounds_fully(tmp_row, tmp_col))
		{
			/* Adjust direction */
			adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;

			/* Our destination is illegal - stop. */
			if (!in_bounds_fully(tmp_row, tmp_col)) break;
		}

		/* Tunnel through dungeon granite. */
		if (cave_feat[tmp_row][tmp_col] == FEAT_WALL_EXTRA)
		{
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the current tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = TRUE;

			continue;
		}


		/* Pierce outer walls of rooms. */
		else if (cave_feat[tmp_row][tmp_col] == FEAT_WALL_OUTER)
		{
			/* Look ahead */
			y0 = tmp_row + row_dir;
			x0 = tmp_col + col_dir;

			/* No annoying little alcoves near edge. */
			if (!in_bounds_fully(y0, x0)) continue;


			/* Disallow door in next grid */
			door_flag = FALSE;

			/* Hack -- delay turns */
			adjust_dir_timer++;  rand_dir_timer++;

			/* Navigate around various kinds of walls */
			if ((cave_feat[y0][x0] == FEAT_WALL_SOLID) || 
			    (cave_feat[y0][x0] == FEAT_WALL_INNER) || 
			    (unalterable(cave_feat[y0][x0])))
			{
				for (i = 0; i < 2; i++)
				{
					if (i == 0)
					{
						/* Check the more direct route first. */
						adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);

						/* Verify that we haven't just been here. */
						if ((dun->tunn_n == 0) || !(dun->tunn[dun->tunn_n - 1].y == row1 + row_dir) || 
						    !(dun->tunn[dun->tunn_n - 1].x == col1 + col_dir))
						{
							tmp_row = row1 + row_dir;
							tmp_col = col1 + col_dir;
						}

						else continue;
					}

					else
					{
						/* If that didn't work, try the other side. */
						tmp_row = row1 - row_dir;
						tmp_col = col1 - col_dir;
					}

					if ((!unalterable(cave_feat[tmp_row][tmp_col])) && 
					    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_SOLID) && 
					    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_OUTER) && 
					    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_INNER))
					{
						/* Accept the location */
						row1 = tmp_row;
						col1 = tmp_col;

						/* Save the current tunnel location */
						if (dun->tunn_n < TUNN_MAX)
						{
							dun->tunn[dun->tunn_n].y = row1;
							dun->tunn[dun->tunn_n].x = col1;
							dun->tunn_n++;
						}

						/* Continue */
						break;
					}

					/* No luck. */
					if (i == 1) continue;
				}
			}

			/* Handle a double line of outer walls robustly. */
			else if (cave_feat[y0][x0] == FEAT_WALL_OUTER)
			{
				/* Look ahead (again). */
				y1 = y0 + row_dir;
				x1 = x0 + col_dir;

				/* We've found something passable. */
				if (passable(cave_feat[y1][x1]))
				{
					/* Turn both outer wall grids into floor. */
					cave_set_feat(tmp_row, tmp_col, FEAT_FLOOR);
					cave_set_feat(y0, x0, FEAT_FLOOR);

					/* Save the wall location */
					if (dun->wall_n < WALL_MAX)
					{
						dun->wall[dun->wall_n].y = tmp_row;
						dun->wall[dun->wall_n].x = tmp_col;
						dun->wall_n++;
					}

					/* Accept this location */
					row1 = tmp_row = y0;
					col1 = tmp_col = x0;
				}

				/* No luck - look at the sides. */
				else
				{
					for (i = 0; i < 2; i++)
					{
						if (i == 0)
						{
							/* Check the more direct route first. */
							adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);

							tmp_row = row1 + row_dir;
							tmp_col = col1 + col_dir;
						}
						else
						{
							/* If that didn't work, try the other side. */
							tmp_row = row1 - row_dir;
							tmp_col = col1 - col_dir;
						}

						if ((!unalterable(cave_feat[tmp_row][tmp_col])) && 
						    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_SOLID) && 
						    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_OUTER) && 
						    (cave_feat[tmp_row][tmp_col] != FEAT_WALL_INNER))
						{
							/* Accept the location */
							row1 = tmp_row;
							col1 = tmp_col;

							/* Save the current tunnel location */
							if (dun->tunn_n < TUNN_MAX)
							{
								dun->tunn[dun->tunn_n].y = row1;
								dun->tunn[dun->tunn_n].x = col1;
								dun->tunn_n++;
							}

							/* Continue */
							break;
						}
					}
				}
			}

			/* Second grid contains any other kind of terrain. */
			else
			{
				/* Accept this location */
				row1 = tmp_row;
				col1 = tmp_col;

				/* Convert to floor grid */
				cave_set_feat(row1, col1, FEAT_FLOOR);

				/* Save the wall location */
				if (dun->wall_n < WALL_MAX)
				{
					dun->wall[dun->wall_n].y = row1;
					dun->wall[dun->wall_n].x = col1;
					dun->wall_n++;
				}
			}

			/* Forbid re-entry near this piercing. */
			if ((!unalterable(cave_feat[row1 + row_dir][col1 + col_dir])) && 
				(cave_info[row1][col1] & (CAVE_ROOM)))
			{
				if (row_dir)
				{
					for (x = col1 - 3; x <= col1 + 3; x++)
					{
						/* Convert adjacent "outer" walls */
						if ((in_bounds(row1, x)) && 
						    (cave_feat[row1][x] == FEAT_WALL_OUTER))
						{
							/* Change the wall to a "solid" wall */
							cave_set_feat(row1, x, FEAT_WALL_SOLID);
						}
					}
				}
				else
				{
					for (y = row1 - 3; y <= row1 + 3; y++)
					{
						/* Convert adjacent "outer" walls */
						if ((in_bounds(y, col1)) && 
						    (cave_feat[y][col1] == FEAT_WALL_OUTER))
						{
							/* Change the wall to a "solid" wall */
							cave_set_feat(y, col1, FEAT_WALL_SOLID);
						}
					}
				}

				/* Get current room. */
				tmp = get_room_index(row1, col1);

				/* Record our success. */
				if ((tmp != start_room) && (tmp != -1))
				{
					/* If this room is connected, now our start room is too. */
					if (dun->connected[tmp])
					{
						dun->connected[start_room] = TRUE;

						/* If our destination room is connected, we're done. */
						if (dun->connected[end_room]) leave = TRUE;
					}

					/* If our start room was connected, this one is too. */
					else if (dun->connected[start_room]) 
						dun->connected[tmp] = TRUE;
				}

				continue;
			}
		}


		/*
		 * We've hit a feature that can't be altered.
		 */
		else if (unalterable(cave_feat[tmp_row][tmp_col]))
		{
			/* We don't know what to do. */
			if (!head_for_entrance)
			{
				/* Get the room that occupies this block. */
				tmp = get_room_index(tmp_row, tmp_col);

				/* We're in our starting room. */
				if (tmp == start_room)
				{
					/* Look at next grid. */
					y = tmp_row + row_dir;
					x = tmp_col + col_dir;

					/* If the next grid is outer wall, we know we need 
					 * to find an entrance.  Otherwise, travel through 
					 * the wall.
					 */
					if (cave_feat[y][x] != FEAT_WALL_OUTER)
					{
						row1 = tmp_row;
						col1 = tmp_col;
						continue;
					}
				}

				y = tmp_row;
				x = tmp_col;

				/* We need to find an entrance to this room. */
				if (!find_entrance(row_dir, col_dir, &y, &x))
				{
					/* No entrance means insoluable trouble. */
					leave = TRUE;
					continue;
				}

				/* We're in our starting room. */
				if (tmp == start_room)
				{
					/* Jump immediately to entrance. */
					row1 = tmp_row = y;
					col1 = tmp_col = x;

					/* Look for outer wall to head for. */
					for (i = 0; i < 4; i++)
					{
						y = row1 + ddy_ddd[i];
						x = col1 + ddx_ddd[i];

						if (cave_feat[y][x] == FEAT_WALL_OUTER)
						{
							/* Aim for outer wall. */
							row_dir = ddy_ddd[i];
							col_dir = ddx_ddd[i];

							adjust_dir_timer = 2;
						}
					}
				}

				/* We're anywhere else. */
				else
				{
					/* Aim for given waypoint. */
					row2 = y;
					col2 = x;

					/* Reset the final target. */
					initial_row2 = y;
					initial_col2 = x;

					/* Enter "head for entrance" mode. */
					head_for_entrance = TRUE;
				}
			}

			/* We're heading for an entrance to a vault. */
			if (head_for_entrance)
			{
				/* Check both sides. */
				for (i = 0; i < 2; i++)
				{
					/*
					 * Try going in the direction that best approches 
					 * the target first.  On the 2nd try, check the 
					 * opposite side.
					 */
					if (col_dir == 0)
					{
						dy = 0;
						if (i == 0) dx = ((col1 < col2) ?  1 : -1);
						else        dx = ((col1 < col2) ? -1 :  1);
					}
					else
					{
						dx = 0;
						if (i == 0) dy = ((row1 < row2) ?  1 : -1);
						else        dy = ((row1 < row2) ? -1 :  1);
					}

					/* Do not accept floor unless necessary. */
					/* if ((cave_feat[row1 + dy][col1 + dx] == FEAT_FLOOR)
						&& (i == 0)) continue; */


					/* Check to see if grid to this side is alterable. */
					if (!unalterable(cave_feat[row1 + dy][col1 + dx]))
					{
						/* Change direction. */
						row_dir = dy;
						col_dir = dx;

						/* Accept this location */
						row1 += row_dir;
						col1 += col_dir;

						/* Clear previous contents, add a floor */
						cave_set_feat(row1, col1, FEAT_FLOOR);

						/* Return to main loop. */
						break;
					}

					/* We seem to be in trouble. */
					else if (i == 1)
					{
						/* If we previously found floor, accept the floor. */
						if (cave_feat[row1 -(dy)][col1 -(dx)] == FEAT_FLOOR)
						{
							/* Change direction. */
							row_dir = -(dy);
							col_dir = -(dx);

							/* Accept this location */
							row1 += row_dir;
							col1 += col_dir;

							break;
						}

						/* Otherwise, go backwards. */
						{
							/* Change direction. */
							row_dir = -(row_dir);
							col_dir = -(col_dir);

							/* Accept this location */
							row1 += row_dir;
							col1 += col_dir;

							break;
						}
					}
				}
			}
		}

		/* We've hit a solid wall. */
		else if (cave_feat[tmp_row][tmp_col] == FEAT_WALL_SOLID)
		{
			/* check both sides, most direct route first. */
			for (i = 0; i < 2; i++)
			{
				if (i == 0)
				{
					/* Check the more direct route first. */
					adjust_dir(&row_dir, &col_dir, row1, col1, row2, col2);

					tmp_row = row1 + row_dir;
					tmp_col = col1 + col_dir;
				}
				else
				{
					/* If that didn't work, try the other side. */
					tmp_row = row1 - row_dir;
					tmp_col = col1 - col_dir;
				}

				if ((!unalterable(cave_feat[tmp_row][tmp_col])) && 
					(cave_feat[tmp_row][tmp_col] != FEAT_WALL_SOLID))
				{
					/* Accept the location */
					row1 = tmp_row;
					col1 = tmp_col;

					/* Save the current tunnel location */
					if (dun->tunn_n < TUNN_MAX)
					{
						dun->tunn[dun->tunn_n].y = row1;
						dun->tunn[dun->tunn_n].x = col1;
						dun->tunn_n++;
					}

					/* Move on. */
					i = 2;
				}
			}

			continue;
		}

		/* Travel quickly through rooms. */
		else if (cave_info[tmp_row][tmp_col] & (CAVE_ROOM))
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			continue;
		}

		/*
		 * Handle all passable terrain outside of rooms (this is 
		 * usually another corridor).
		 */
		else if (passable(cave_feat[tmp_row][tmp_col]))
		{
			/* We've hit another tunnel. */
			if (cave_feat[tmp_row][tmp_col] == FEAT_FLOOR)
			{
				/* Collect legal door locations */
				if (door_flag)
				{
					/* Save the door location */
					if (dun->door_n < DOOR_MAX)
					{
						dun->door[dun->door_n].y = tmp_row;
						dun->door[dun->door_n].x = tmp_col;
						dun->door_n++;
					}

					/* No door in next grid */
					door_flag = FALSE;
				}

				/* Mark start room connected. */
				dun->connected[start_room] = TRUE;

				/* 
				 * If our destination room isn't connected, jump to 
				 * its center, and head towards the start room.
				 */
				if (dun->connected[end_room] == FALSE)
				{
					/* Swap rooms. */
					tmp = end_room;
					end_room = start_room;
					start_room = tmp;

					/* Re-initialize */
					row1 = dun->cent[start_room].y;
					col1 = dun->cent[start_room].x;
					row2 = dun->cent[end_room].y;
					col2 = dun->cent[end_room].x;
					initial_row2 = row2;
					initial_col2 = col2;
					tmp_row = row1, tmp_col = col1;
				}
				else
				{
					/* All done. */
					leave = TRUE;
				}

				continue;
			}

			/* Grid is not another tunnel.  Advance, make no changes. */
			row1 = tmp_row;
			col1 = tmp_col;

			continue;
		}
	}

	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++)
	{
		/* Access the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Clear previous contents, add a floor */
		cave_set_feat(y, x, FEAT_FLOOR);
	}

	/* Make doors in entranceways. */
	for (i = 0; i < dun->wall_n; i++)
	{
		/* Access the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Sometimes, make a door in the entranceway */
		if (rand_int(100) < DUN_TUN_PEN) try_entrance(y, x);
	}


	/* We've reached the target.  If one room was connected, now both are. */
	if ((row1 == initial_row2) && (col1 == initial_col2))
	{
		if (dun->connected[start_room])
			dun->connected[end_room] = TRUE;
		else if (dun->connected[end_room])
			dun->connected[start_room] = TRUE;
	}
}


/*
 * Generate a new dungeon level.  Determine if the level is destroyed.  
 * Build up to DUN_ROOMS rooms, type by type, in descending order of size 
 * and  difficulty.
 *
 * Build the dungeon borders, scramble and connect the rooms.  Place stairs, 
 * doors, and random monsters, objects, and traps.  Place any quest monsters.
 *
 * We mark grids "icky" to indicate the presence of a vault.
 *
 * Note that "dun_body" adds about 1100 bytes of memory to the stack.
 */
static void cave_gen(void)
{
	int i, j, k, y, x, y1, x1;
	int by, bx;
	int num_to_build;
	int room_type;
	int rooms_built = 0;

	/* Build rooms in descending order of difficulty. */
	byte room_build_order[ROOM_MAX] = {7, 6, 5, 4, 3, 2, 1, 0};

	bool destroyed = FALSE;
	bool dummy;

	dun_data dun_body;

	/* Global data */
	dun = &dun_body;


	/* It is possible for levels to be destroyed */
	if ((p_ptr->depth > 10) && (!is_quest(p_ptr->depth)) && 
		(rand_int(DEST_LEVEL_CHANCE) == 0))
	{
		destroyed = TRUE;

		/* Destroyed levels do not have certain kinds of rooms. */
		for (i = 0; i < ROOM_MAX; i++) 
		{
			if (room_build_order[i] > 5) room_build_order[i] = 0;
		}
	}


	/* Hack -- Start with basic granite (or floor, if empty) */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Create granite wall */
			cave_info[y][x] |= (CAVE_WALL);
			cave_feat[y][x] = FEAT_WALL_EXTRA;
		}
	}

	/* Actual maximum number of rooms on this level */
	dun->row_rooms = DUNGEON_HGT / BLOCK_HGT;
	dun->col_rooms = DUNGEON_WID / BLOCK_WID;

	/* No stair locations yet */
	dun->stair_n = 0;

	/* Initialize the room table */
	for (by = 0; by < dun->row_rooms; by++)
	{
		for (bx = 0; bx < dun->col_rooms; bx++)
		{
			dun->room_map[by][bx] = 0;
		}
	}

	/* No rooms are connected yet */
	for (i = 0; i < CENT_MAX; i++)
	{
		dun->connected[i] = FALSE;
	}

	/* No rooms yet */
	dun->cent_n = 0;


	/* 
	 * Build each type of room in turn until we cannot build any more.
	 */
	for (i = 0; i < ROOM_MAX; i++)
	{
		/* What type of room are we building now? */
		room_type = room_build_order[i];

		/* Find out how many rooms of this type we can build. */
		num_to_build = num_rooms_allowed(room_type);

		/* No vaults on Quest levels (for now) -BR- */
		if (is_quest(p_ptr->depth) && ((room_type == 6) || (room_type == 7)))
			num_to_build = 0;

		/* Try to build all we are allowed. */
		for (j = 0; j < num_to_build; j++)
		{
			/* Stop building rooms when we hit the maximum. */
			if (rooms_built >= DUN_ROOMS) break;

			/* Build the room. */
			if (room_build(room_type))
			{
				/* Increase the room built count. */
				if (room_type == 10) rooms_built += 5;
				else if ((room_type == 6) || (room_type == 9)) 
					rooms_built += 3;
				else if (room_type == 8) rooms_built += 2;
				else rooms_built++;
			}

			/* Go to next type of room on failure. */
			else break;
		}
	}

	/* Special boundary walls -- Top */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		y = 0;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- Bottom */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		y = DUNGEON_HGT - 1;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- Left */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		x = 0;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- Right */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		x = DUNGEON_WID - 1;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, x, FEAT_PERM_SOLID);
	}

	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++)
	{
		int pick1 = i;
		int pick2 = rand_int(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;

		/* XXX XXX - swap around room index numbers. */
		for (by = 0; by < 6; by++)
		{
			for (bx = 0; bx < 18; bx++)
			{
				if      (dun->room_map[by][bx] == pick2 + 1) 
				         dun->room_map[by][bx] =  pick1 + 1;
				else if (dun->room_map[by][bx] == pick1 + 1) 
				         dun->room_map[by][bx] =  pick2 + 1;
			}
		}
	}

	/* Start with no tunnel doors */
	dun->door_n = 0;

	/* Mark the first room as being connected. */
	dun->connected[0] = TRUE;

	/* Connect all the rooms together (and locate grids for tunnel doors) */
	for (i = 0; i < dun->cent_n; i++)
	{
		/* Connect the room to the next room. */
		if (i == dun->cent_n - 1) build_tunnel(dun->cent_n - 1, 0);
		else build_tunnel(i, i + 1);
	}

	/* Place tunnel doors */
	for (i = 0; i < dun->door_n; i++)
	{
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_door(y, x - 1);
		try_door(y, x + 1);
		try_door(y - 1, x);
		try_door(y + 1, x);
	}


	/* Add some magma streamers */
	for (i = 0; i < DUN_STR_MAG; i++)
	{
		build_streamer(FEAT_MAGMA, DUN_STR_MC);
	}

	/* Add some quartz streamers */
	for (i = 0; i < DUN_STR_QUA; i++)
	{
		build_streamer(FEAT_QUARTZ, DUN_STR_QC);
	}


	/* Destroy the level if necessary */
	if (destroyed) destroy_level();


	/* Basic "amount" */
	k = (p_ptr->depth / 3);
	if (k > 10) k = 10;
	if (k < 2) k = 2;


	/* Place 3 or 4 down stairs near some walls */
	alloc_stairs(FEAT_MORE, rand_range(3, 4), 3);

	/* Place 1 or 2 up stairs near some walls */
	alloc_stairs(FEAT_LESS, rand_range(1, 2), 3);

	/* Put some rubble in corridors */
	alloc_object(ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint(k), p_ptr->depth);

	/* Place some traps in the dungeon */
	alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint(k), p_ptr->depth);


	/* Determine the character location */
	new_player_spot();


	/* Paranoia -- Remove all monster restrictions. */
	mon_restrict('\0', (byte)p_ptr->depth, &dummy, TRUE);


	/* Pick a base number of monsters */
	i = MIN_M_ALLOC_LEVEL + randint(8);

	/* Put some monsters in the dungeon */
	for (j = i + k; j > 0; j--)
	{
		/* Place a random monster */
		(void)alloc_monster(0, TRUE, p_ptr->depth);
	}


	/* Ensure quest monsters */
	if (is_quest(p_ptr->depth))
	{
		/* Ensure quest monsters */
		for (i = 1; i < z_info->r_max; i++)
		{
			monster_race *r_ptr = &r_info[i];

			/* Ensure quest monsters */
			if ((r_ptr->flags1 & (RF1_QUESTOR)) &&
			    (r_ptr->level == p_ptr->depth) &&
			    (r_ptr->cur_num <= 0))
			{
				int y, x;

				/* Pick a location */
				while (1)
				{
					y = rand_int(DUNGEON_HGT);
					x = rand_int(DUNGEON_WID);

					if (cave_naked_bold(y, x)) break;
				}

				/* Place the questor */
				place_monster_aux(y, x, i, TRUE, TRUE);
			}
		}
	}

	/* Put some objects in rooms */
	alloc_object(ALLOC_SET_ROOM,
		ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ROOM, 3),
		p_ptr->depth);

	/* Put some objects/gold in the dungeon */
	alloc_object(ALLOC_SET_BOTH,
		ALLOC_TYP_OBJECT, Rand_normal(DUN_AMT_ITEM, 3),
		p_ptr->depth);
	alloc_object(ALLOC_SET_BOTH,
		ALLOC_TYP_GOLD, Rand_normal(DUN_AMT_GOLD, 3),
		p_ptr->depth);
}



/*
 * Builds a store at a given pseudo-location
 *
 * As of 2.7.4 (?) the stores are placed in a more "user friendly"
 * configuration, such that the four "center" buildings always
 * have at least four grids between them, to allow easy running,
 * and the store doors tend to face the middle of town.
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 *
 * Note the use of "town_illuminate()" to handle all "illumination"
 * and "memorization" issues.
 */
static void build_store(int n, int yy, int xx)
{
	int y, x, y0, x0, y1, x1, y2, x2, tmp;


	/* Find the "center" of the store */
	y0 = yy * 9 + 6;
	x0 = xx * 14 + 12;

	/* Determine the store boundaries */
	y1 = y0 - randint((yy == 0) ? 3 : 2);
	y2 = y0 + randint((yy == 1) ? 3 : 2);
	x1 = x0 - randint(5);
	x2 = x0 + randint(5);

	/* Build an invulnerable rectangular building */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Create the building */
			cave_set_feat(y, x, FEAT_PERM_EXTRA);
		}
	}

	/* Pick a door direction (S,N,E,W) */
	tmp = rand_int(4);

	/* Re-roll "annoying" doors */
	if (((tmp == 0) && (yy == 1)) ||
	    ((tmp == 1) && (yy == 0)) ||
	    ((tmp == 2) && (xx == 3)) ||
	    ((tmp == 3) && (xx == 0)))
	{
		/* Pick a new direction */
		tmp = rand_int(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
		case 0:
		{
			y = y2;
			x = rand_range(x1, x2);
			break;
		}

		/* Top side */
		case 1:
		{
			y = y1;
			x = rand_range(x1, x2);
			break;
		}

		/* Right side */
		case 2:
		{
			y = rand_range(y1, y2);
			x = x2;
			break;
		}

		/* Left side */
		default:
		{
			y = rand_range(y1, y2);
			x = x1;
			break;
		}
	}

	/* Clear previous contents, add a store door */
	cave_set_feat(y, x, FEAT_SHOP_HEAD + n);
}




/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(void)
{
	int y, x, k, n;

	int rooms[MAX_STORES];


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant town layout */
	Rand_value = seed_town;


	/* Prepare an Array of "remaining stores", and count them */
	for (n = 0; n < MAX_STORES; n++) rooms[n] = n;

	/* Place two rows of stores */
	for (y = 0; y < 2; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 4; x++)
		{
			/* Pick a random unplaced store */
			k = ((n <= 1) ? 0 : rand_int(n));

			/* Build that store at the proper location */
			build_store(rooms[k], y, x);

			/* Shift the stores down, remove one store */
			rooms[k] = rooms[--n];
		}
	}


	/* Place the stairs */
	while (TRUE)
	{
		/* Pick a location at least "three" from the outer walls */
		y = rand_range(3, TOWN_HGT - 4);
		x = rand_range(3, TOWN_WID - 4);

		/* Require a "naked" floor grid */
		if (cave_naked_bold(y, x)) break;
	}

	/* Clear previous contents, add down stairs */
	cave_set_feat(y, x, FEAT_MORE);


	/* Place the player */
	player_place(y, x);


	/* Hack -- use the "complex" RNG */
	Rand_quick = FALSE;
}




/*
 * Town logic flow for generation of new town
 *
 * We start with a fully wiped cave of normal floors.
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * We place the player on the stairs at the same time we make them.
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
static void town_gen(void)
{
	int i, y, x;

	int residents;

	bool daytime;


	/* Day time */
	if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))
	{
		/* Day time */
		daytime = TRUE;

		/* Number of residents */
		residents = MIN_M_ALLOC_TD;
	}

	/* Night time */
	else
	{
		/* Night time */
		daytime = FALSE;

		/* Number of residents */
		residents = MIN_M_ALLOC_TN;
	}

	/* Start with solid walls */
	for (y = 0; y < DUNGEON_HGT; y++)
	{
		for (x = 0; x < DUNGEON_WID; x++)
		{
			/* Create "solid" perma-wall */
			cave_set_feat(y, x, FEAT_PERM_SOLID);
		}
	}

	/* Then place some floors */
	for (y = 1; y < TOWN_HGT - 1; y++)
	{
		for (x = 1; x < TOWN_WID - 1; x++)
		{
			/* Create empty floor */
			cave_set_feat(y, x, FEAT_FLOOR);
		}
	}

	/* Build stuff */
	town_gen_hack();

	/* Apply illumination */
	town_illuminate(daytime);

	/* Make some residents */
	for (i = 0; i < residents; i++)
	{
		/* Make a resident */
		(void)alloc_monster(3, TRUE, p_ptr->depth);
	}
}


/*
 * Generate a random dungeon level
 *
 * Hack -- regenerate any "overflow" levels
 *
 * Hack -- allow auto-scumming via a gameplay option.
 *
 * Note that this function resets flow data and grid flags directly.
 * Note that this function does not reset features, monsters, or objects.  
 * Features are left to the town and dungeon generation functions, and 
 * "wipe_m_list()" and "wipe_o_list()" handle monsters and objects.
 */
void generate_cave(void)
{
	int y, x, num;

	/* The dungeon is not ready */
	character_dungeon = FALSE;

	/* Generate */
	for (num = 0; TRUE; num++)
	{
		bool okay = TRUE;
		cptr why = NULL;

		/* Reset monsters and objects */
		o_max = 1;
		mon_max = 1;


		/* Clear flags and flow information. */
		for (y = 0; y < DUNGEON_HGT; y++)
		{
			for (x = 0; x < DUNGEON_WID; x++)
			{
				/* No flags */
				cave_info[y][x] = 0;
				cave_info2[y][x] = 0;

#ifdef MONSTER_FLOW
				/* No flow */
				cave_cost[y][x] = 0;
				cave_when[y][x] = 0;
#endif /* MONSTER_FLOW */

			}
		}


		/* Mega-Hack -- no player in dungeon yet */
		cave_m_idx[p_ptr->py][p_ptr->px] = 0;
		p_ptr->px = p_ptr->py = 0;

		/* Hack -- illegal panel */
		Term->offset_y = DUNGEON_HGT;
		Term->offset_x = DUNGEON_WID;


		/* Nothing special here yet */
		good_item_flag = FALSE;

		/* Nothing good here yet */
		rating = 0;

		/* Build the town */
		if (!p_ptr->depth)
		{
			/* Make a town */
			town_gen();
		}

		/* Build a real level */
		else
		{
			/* Make a dungeon */
			cave_gen();
		}

		okay = TRUE;


		/* Extract the feeling */
		if      (rating > 50 +     p_ptr->depth    ) feeling = 2;
		else if (rating > 40 + 4 * p_ptr->depth / 5) feeling = 3;
		else if (rating > 30 + 3 * p_ptr->depth / 5) feeling = 4;
		else if (rating > 20 + 2 * p_ptr->depth / 5) feeling = 5;
		else if (rating > 15 + 1 * p_ptr->depth / 3) feeling = 6;
		else if (rating > 10 + 1 * p_ptr->depth / 5) feeling = 7;
		else if (rating >  5 + 1 * p_ptr->depth /10) feeling = 8;
		else if (rating >  0) feeling = 9;
		else feeling = 10;

		/* Hack -- Have a special feeling sometimes */
		if (good_item_flag && adult_no_preserve) feeling = 1;

		/* It takes 1000 game turns for "feelings" to recharge */
		if (((turn - old_turn) < 1000) && (old_turn > 1)) feeling = 0;

		/* Hack -- no feeling in the town */
		if (!p_ptr->depth) feeling = 0;


		/* Prevent object over-flow */
		if (o_max >= z_info->o_max)
		{
			/* Message */
			why = "too many objects";

			/* Message */
			okay = FALSE;
		}

		/* Prevent monster over-flow */
		if (mon_max >= z_info->m_max)
		{
			/* Message */
			why = "too many monsters";

			/* Message */
			okay = FALSE;
		}

		/* Mega-Hack -- "auto-scum" */
		if (adult_autoscum && (num < 100))
		{
			/* Require "goodness" */
			if ((feeling > 9) ||
			    ((p_ptr->depth >= 5) && (feeling > 8)) ||
			    ((p_ptr->depth >= 10) && (feeling > 7)) ||
			    ((p_ptr->depth >= 20) && (feeling > 6)) ||
			    ((p_ptr->depth >= 40) && (feeling > 5)))
			{
				/* Give message to cheaters */
				if (cheat_room || cheat_hear ||
				    cheat_peek || cheat_xtra)
				{
					/* Message */
					why = "boring level";
				}

				/* Try again */
				okay = FALSE;
			}
		}

		/* Message */
		if ((cheat_room) && (why)) 
			msg_format("Generation restarted (%s)", why);

		/* Accept */
		if (okay) break;

		/* Wipe the objects */
		wipe_o_list();

		/* Wipe the monsters */
		wipe_mon_list();
	}


	/* The dungeon is ready */
	character_dungeon = TRUE;


	/* Remember when this level was "created" */
	old_turn = turn;
}
