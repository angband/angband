/* treasure.c: dungeon object definitions

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"

/* Following are treasure arrays	and variables			*/

/* Object description:	Objects are defined here.  Each object has
  the following attributes:
	Descriptor : Name of item and formats.
		& is replaced with 'a', 'an', or a number.
		~ is replaced with null or 's'.
	Character  : Character that represents the item.
	Type value : Value representing the type of object.
	Sub value  : separate value for each item of a type.
		0 - 63: object can not stack
		64 - 127: dungeon object, can stack with other D object
		128 - 191: unused, previously for store items
		192: stack with other iff have same p1 value, always
			treated as individual objects
		193 - 255: object can stack with others iff they have
			the same p1 value, usually considered one group
		Objects which have two type values, e.g. potions and
		scrolls, need to have distinct subvals for
		each item regardless of its tval
	Damage	   : amount of damage item can cause.
	Weight	   : relative weight of an item.
	Number	   : number of items appearing in group.
	To hit	   : magical plusses to hit.
	To damage  : magical plusses to damage.
	AC	   : objects relative armor class.
		1 is worse than 5 is worse than 10 etc.
	To AC	   : Magical bonuses to AC.
	P1	   : Catch all for magical abilities such as
		     plusses to strength, minuses to searching.
	Flags	   : Abilities of object.  Each ability is a
		     bit.  Bits 1-31 are used. (Signed integer)
	Level	   : Minimum level on which item can be found.
	Cost	   : Relative cost of item.
	Rarity     : Normally 0, but is 1/rare chance of it being found
	Flags3     : Even more abilities of objects...

	Special Abilities can be added to item by magic_init(),
	found in misc.c.

	Scrolls, Potions, and Food:
	Flags is used to define a function which reading/quaffing
	will cause.  Most scrolls and potions have only one bit
	set.  Potions will generally have some food value, found
	in p1.

	Wands and Staffs:
	Flags defines a function, p1 contains number of charges
	for item.  p1 is set in magic_init() in misc.c.

	Chests:
	Traps are added randomly by magic_init() in misc.c.	*/

/* Object list (All objects must be defined here)		 */
#ifdef MACGAME
treasure_type *object_list;
#else              0
treasure_type object_list[MAX_OBJECTS] = {
/* Dungeon items from 0 to MAX_DUNGEON_OBJ */
{"Poison"			,0x00000001L,	TV_FOOD, ',',	/*  0*/
  500,	   0,	64,   1,   1,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  , 0
#endif
},
{"Blindness"			,0x00000002L,	TV_FOOD, ',',	/*  1*/
  500,	   0,	65,   1,   1,	0,   0,	 0,   0, {0,0}	,  9, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Paranoia"			,0x00000004L,	TV_FOOD, ',',	/*  2*/
  500,	   0,	66,   1,   1,	0,   0,	 0,   0, {0,0}	,  9, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Confusion"			,0x00000008L,	TV_FOOD, ',',	/*  3*/
  500,	   0,	67,   1,   1,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Hallucination"		,0x00000010L,	TV_FOOD, ',',	/*  4*/
  500,	   0,	68,   1,   1,	0,   0,	 0,   0, {0,0}	, 13, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Poison"			,0x00000020L,	TV_FOOD, ',',	/*  5*/
  500,	  60,	69,   1,   1,	0,   0,	 0,   0, {0,0}	,  8, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Blindness"		,0x00000040L,	TV_FOOD, ',',	/*  6*/
  500,	  50,	70,   1,   1,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Paranoia"		,0x00000080L,	TV_FOOD, ',',	/*  7*/
  500,	  25,	71,   1,   1,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Confusion"		,0x00000100L,	TV_FOOD, ',',	/*  8*/
  500,	  50,	72,   1,   1,	0,   0,	 0,   0, {0,0}	,  6, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Weakness"			,0x04000200L,	TV_FOOD, ',',	/*  9*/
  500,	   0,	73,   1,   1,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Unhealth"			,0x04000400L,	TV_FOOD, ',',	/* 10*/
  500,	  50,	74,   1,   1,	0,   0,	 0,   0, {10,10}, 15, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Constitution"		,0x00010000L,	TV_FOOD, ',',	/* 11*/
  500,	 350,	75,   1,   1,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restoring"			,0x001F8040L,	TV_FOOD, ',',	/* 12*/
  500,	1000,	79,   1,   1,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restoring"			,0x001F8040L,	TV_FOOD, ',',	/* 13*/
  500,	1000,	79,   1,   1,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restoring"			,0x001F8040L,	TV_FOOD, ',',	/* 14*/
  500,	1000,	79,   1,   1,	0,   0,	 0,   0, {0,0}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Poison"			,0x00000001L,	TV_FOOD, ',',	/* 15*/
 1200,	   0,	80,   1,   1,	0,   0,	 0,   0, {0,0}	, 15, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Poison"			,0x00000001L,	TV_FOOD, ',',	/* 16*/
 1200,	   0,	80,   1,   1,	0,   0,	 0,   0, {0,0}	, 15, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Hallucinations"		,0x00000010L,	TV_FOOD, ',',	/* 17*/
 1200,	   0,	81,   1,   1,	0,   0,	 0,   0, {0,0}	, 18, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Poison"			,0x00000020L,	TV_FOOD, ',',	/* 18*/
 1200,	  75,	82,   1,   1,	0,   0,	 0,   0, {0,0}	, 19, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Unhealth"			,0x00000400L,	TV_FOOD, ',',	/* 19*/
 1200,	  25,	83,   1,   1,	0,   0,	 0,   0, {6,8}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Serious Wounds"		,0x02000000L,	TV_FOOD, ',',	/* 20*/
 1200,	  75,	84,   1,   2,	0,   0,	 0,   0, {0,0}	, 16, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Ration~ of Food"		,0x00000000L,	TV_FOOD, ',',	/* 21*/
 5000,	   3,	90,   1,  10,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Ration~ of Food"		,0x00000000L,	TV_FOOD, ',',	/* 22*/
 5000,	   3,	90,   1,  10,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Ration~ of Food"		,0x00000000L,	TV_FOOD, ',',	/* 23*/
 5000,	   3,	90,   1,  10,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Slime Mold~"		,0x00000000L,	TV_FOOD, ',',	/* 24*/
 3000,	   2,	91,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"& Piece~ of Elvish Waybread"	,0x02000020L,	TV_FOOD, ',',	/* 25*/
 7500,	  10,	92,   1,   3,	0,   0,	 0,   0, {0,0}	,  6, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Piece~ of Elvish Waybread"	,0x02000020L,	TV_FOOD, ',',	/* 26*/
 7500,	  10,	92,   1,   3,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Piece~ of Elvish Waybread"	,0x02000020L,	TV_FOOD, ',',	/* 27*/
 7500,	  10,	92,   1,   3,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Main Gauche"          	,0x00000000L,	TV_SWORD, '|',	/* 28*/
    0,	  25,	1,   1,  30,	0,   0,	 0,   0, {1,5}	,  2, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Dagger"	         	,0x00000000L,	TV_SWORD, '|',	/* 29*/
    0,	  10,	3,   1,  12,	0,   0,	 0,   0, {1,4}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Broken dagger"		,0x00000000L,	TV_SWORD, '|',	/* 30*/
    0,	   0,	5,   1,  15,  -2,  -4,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,CYAN
#endif
},
{"& Bastard Sword"		,0x00000000L,	TV_SWORD, '|',	/* 31*/
    0,	 350,	7,   1, 140,	0,   0,	 0,   0, {3,4}	, 14, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Scimitar"	         	,0x00000000L,	TV_SWORD, '|',	/* 32*/
    0,	 250,	25,   1, 130,	0,   0,	 0,   0, {2,5}	, 8, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Tulwar"		        ,0x00000000L,	TV_SWORD, '|',	/* 33*/
    0,	 200,	26,   1, 100,	0,   0,	 0,   0, {2,4}	, 6, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Broadsword"			,0x00000000L,	TV_SWORD, '|',	/* 34*/
    0,	 255,	10,   1, 150,	0,   0,	 0,   0, {2,5}	,  9, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Broadsword"			,0x00000000L,	TV_SWORD, '|',	/* 35*/
    0,	 255,	27,   1, 150,	0,   0,	 0,   0, {2,5}	,  15, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Blade of Chaos"		,0x00000000L,	TV_SWORD, '|',	/* 36*/
    0,	4000,	30,   1, 180,	0,   0,	 0,   0, {6,5}	,  70, 5, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"& Two-Handed Sword"           ,0x00000000L,	TV_SWORD, '|',	/* 37*/
    0,	 775,	11,   1, 200,	0,   0,	 0,   0, {3,6}	, 30, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Two-Handed Sword"           ,0x00000000L,	TV_SWORD, '|',	/* 38*/
    0,	 775,	29,   1, 200,	0,   0,	 0,   0, {3,6}	, 40, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Cutlass"			,0x00000000L,	TV_SWORD, '|',	/* 39*/
    0,	  85,	12,   1, 110,	0,   0,	 0,   0, {1,7}	,  7, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Executioner's Sword"	,0x00000000L,	TV_SWORD, '|',	/* 40*/
    0,	 850,	14,   1, 260,	0,   0,	 0,   0, {4,5}	, 40, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Katana"			,0x00000000L,	TV_SWORD, '|',	/* 41*/
    0,	 400,	17,   1, 120,	0,   0,	 0,   0, {3,4}	, 18, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Longsword"			,0x00000000L,	TV_SWORD, '|',	/* 42*/
    0,	 300,	18,   1, 130,	0,   0,	 0,   0, {2,5} , 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Longsword"			,0x00000000L,	TV_SWORD, '|',	/* 43*/
    0,	 300,	28,   1, 130,	0,   0,	 0,   0, {2,5} , 20, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Rapier"			,0x00000000L,	TV_SWORD, '|',	/* 44*/
    0,	  42,	20,   1,  40,	0,   0,	 0,   0, {1,6}	,  4, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Sabre"			,0x00000000L,	TV_SWORD, '|',	/* 45*/
    0,	  50,	21,   1,  50,	0,   0,	 0,   0, {1,7}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Small sword"		,0x00000000L,	TV_SWORD, '|',	/* 46*/
    0,	  48,	22,   1,  75,	0,   0,	 0,   0, {1,6}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Broken sword"		,0x00000000L,	TV_SWORD, '|',	/* 47*/
    0,	   0,	24,   1,  75,  -2,  -4,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Ball and Chain"		,0x00000000L,	TV_HAFTED, '\\',/* 48*/
    0,	 200,	1,   1, 150,	0,   0,	 0,   0, {2,4}	, 20, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Whip"	        	,0x00000000L,	TV_HAFTED, '\\',/* 49*/
    0,	 30,	10,   1, 30,	0,   0,	 0,   0, {1,6}	, 3, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Flail"			,0x00000000L,	TV_HAFTED, '\\',/* 50*/
    0,	 353,	4,   1, 150,	0,   0,	 0,   0, {2,6}	, 12, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Two-Handed Great Flail"	,0x00000000L,	TV_HAFTED, '\\',/* 51*/
    0,	 590,	5,   1, 280,	0,   0,	 0,   0, {3,6}	, 45, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Morningstar"		,0x00000000L,	TV_HAFTED, '\\',/* 52*/
    0,	 396,	6,   1, 150,	0,   0,	 0,   0, {2,6}	, 10, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Mace"			,0x00000000L,	TV_HAFTED, '\\',/* 53*/
    0,	 130,	7,   1, 120,	0,   0,	 0,   0, {2,4}	,  6, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Quarterstaff"		,0x00000000L,	TV_HAFTED, '\\',/* 54*/
    0,	 200,	11,   1, 150,	0,   0,	 0,   0, {1,10}	,  8, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& War Hammer"			,0x00000000L,	TV_HAFTED, '\\',/* 55*/
    0,	 225,	8,   1, 120,	0,   0,	 0,   0, {3,3}	,  5, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Lead-filled Mace"		,0x00000000L,	TV_HAFTED, '\\',/* 56*/
    0,	 502,	9,   1, 180,	0,   0,	 0,   0, {3,4}	, 15, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Mace of Disruption"		,0x00000000L,	TV_HAFTED, '\\',/* 57*/
    0,	4300,	12,   1, 400,	0,   0,	 0,   0, {4,10}	, 80, 4, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Awl-Pike"			,0x00000000L,	TV_POLEARM, '/',/* 58*/
    0,	 340,	1,   1, 160,	0,   0,	 0,   0, {1,8}	,  8, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Beaked Axe"			,0x00000000L,	TV_POLEARM, '/',/* 59*/
    0,	 408,	2,   1, 180,	0,   0,	 0,   0, {2,6}	, 15, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Glaive"			,0x00000000L,	TV_POLEARM, '/',/* 60*/
    0,	 363,	4,   1, 190,	0,   0,	 0,   0, {2,6}	, 20, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Halberd"			,0x00000000L,	TV_POLEARM, '/',/* 61*/
    0,	 430,	5,   1, 190,	0,   0,	 0,   0, {3,4}	, 22, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Lucerne Hammer"		,0x00000000L,	TV_HAFTED, '\\',/* 62*/
    0,	 376,	13,   1, 120,	0,   0,	 0,   0, {2,5}	, 11, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Pike"			,0x00000000L,	TV_POLEARM, '/',/* 63*/
    0,	 358,	7,   1, 160,	0,   0,	 0,   0, {2,5}	, 15, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Spear"			,0x00000000L,	TV_POLEARM, '/',/* 64*/
    0,	  36,	8,   1,  50,	0,   0,	 0,   0, {1,6}	,  5, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Trident"			,0x00000000L,	TV_POLEARM, '/',/* 65*/
 0,	  65,	15,   1,  70,	0,   0,	 0,   0, {1,9}	,  8, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Lance"			,0x00000000L,	TV_POLEARM, '/',/* 66*/
    0,	 230,	9,   1, 300,	0,   0,	 0,   0, {2,8}	, 10, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Great Axe"	                ,0x00000000L,	TV_POLEARM, '/',/* 67*/
    0,	 500,	11,   1, 230,	0,   0,	 0,   0, {4,4}	, 40, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Battle Axe"          	,0x00000000L,	TV_POLEARM, '/',/* 68*/
    0,	 334,	12,   1, 170,	0,   0,	 0,   0, {2,8}	, 13, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Lochaber Axe"          	,0x00000000L,	TV_POLEARM, '/',/* 69*/
    0,	 750,	13,   1, 250,	0,   0,	 0,   0, {3,8}	, 45, 0, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"& Broad Axe"			,0x00000000L,	TV_POLEARM, '/',/* 70*/
    0,	 304,	14,   1, 160,	0,   0,	 0,   0, {2,6}	, 17, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Scythe"			,0x00000000L,	TV_POLEARM, '/',/* 71*/
    0,	 800,	16,   1, 250,	0,   0,	 0,   0, {5,3}	, 45, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Scythe of Slicing"		,0x00000000L,	TV_POLEARM, '/',/* 72*/
    0,	3500,	17,   1, 250,	0,   0,	 0,   0, {8,4}	, 60, 4, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"& Short Bow"			,0x00000000L,	TV_BOW, '}',	/* 73*/
    2,	  50,	1,   1,  30,	0,   0,	 0,   0, {0,0}	,  3, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Long Bow"			,0x00000000L,	TV_BOW, '}',	/* 74*/
    3,	 120,	2,   1,  40,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Light Crossbow"		,0x00000000L,	TV_BOW, '}',	/* 75*/
    5,	 140,	10,   1, 110,	0,   0,	 0,   0, {0,0}	, 15, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Heavy Crossbow"		,0x00000000L,	TV_BOW, '}',	/* 76*/
    6,	 300,	11,   1, 200,	0,   0,	 0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Sling"			,0x00000000L,	TV_BOW, '}',	/* 77*/
    1,	   5,	20,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Arrow~"			,0x00000000L,	TV_ARROW, '{',	/* 78*/
    0,	   1, 193,   1,   2,	0,   0,	 0,   0, {1,4}	,  2, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Seeker Arrow~"		,0x00000000L,	TV_ARROW, '{',	/* 79*/
    0,	   20, 194,   1,   2,	0,   0,	 0,   0, {3,6}	,  55, 2, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Bolt~"			,0x00000000L,	TV_BOLT, '{',	/* 80*/
    0,	   2, 193,   1,   3,	0,   0,	 0,   0, {1,5}	,  2, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Seeker Bolt~"		,0x00000000L,	TV_BOLT, '{',	/* 81*/
    0,	   25, 194,   1,   3,	0,   0,	 0,   0, {3,7}	,  65, 3, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Rounded Pebble~"		,0x00000000L,	TV_SLING_AMMO, '{',/* 82*/
    0,	   1, 193,   1,   4,	0,   0,	 0,   0, {1,2}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Iron Shot~"			,0x00000000L,	TV_SLING_AMMO, '{',/* 83*/
    0,	   2, 194,   1,   5,	0,   0,	 0,   0, {1,3}	,  3, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Iron Spike~"		,0x00000000L,	TV_SPIKE, '~',	/* 84*/
    0,	   1, 193,   1,  10,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Brass Lantern~"		,0x00000000L,	TV_LIGHT, '~',	/* 85*/
 7500,	  35,	1,   1,  50,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"& Wooden Torch~"		,0x00000000L,	TV_LIGHT, '~',	/* 86*/
 4000,	   2, 193,   1,  30,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Orcish Pick"		,0x20000000L,	TV_DIGGING, '\\',/* 87*/
    2,	 500,	2,   1, 180,	0,   0,	 0,   0, {1,3}	, 20, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Dwarven Pick"	       ,0x20000000L,	TV_DIGGING, '\\',/* 88*/
    3,	1200,	3,   1, 200,	0,   0,	 0,   0, {1,4}	, 50, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Gnomish Shovel"		,0x20000000L,	TV_DIGGING, '\\',/* 89*/
    1,	 100,	5,   1,  50,	0,   0,	 0,   0, {1,2}	, 20, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Dwarven Shovel"		,0x20000000L,	TV_DIGGING, '\\',/* 90*/
    2,	 250,	6,   1, 120,	0,   0,	 0,   0, {1,3}	, 40, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Pair of Soft Leather Boots"	,0x00000000L,	TV_BOOTS, ']',	/* 91*/
    0,	   7,	2,   1,  20,	0,   0,	 2,   0, {1,1}	,  4, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Pair of Hard Leather Boots"	,0x00000000L,	TV_BOOTS, ']',	/* 92*/
    0,	  12,	3,   1,  40,	0,   0,	 3,   0, {1,1}	,  6, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Pair of Metal Shod Boots"	,0x00000000L,	TV_BOOTS, ']',	/* 93*/
    0,	  50,	4,   1,  80,	0,   0,	 6,   0, {1,1}	,  20, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Hard Leather Cap"		,0x00000000L,	TV_HELM, ']',	/* 94*/
    0,	  12,	2,   1,  15,	0,   0,	 2,   0, {0,0}	,  4, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Metal Cap"			,0x00000000L,	TV_HELM, ']',	/* 95*/
    0,	  30,	3,   1,  20,	0,   0,	 3,   0, {1,1}	,  7, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Iron Helm"			,0x00000000L,	TV_HELM, ']',	/* 96*/
    0,	  75,	4,   1,  75,	0,   0,	 5,   0, {1,3}	, 20, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Steel Helm"			,0x00000000L,	TV_HELM, ']',	/* 97*/
    0,	 200,	5,   1,  60,	0,   0,	 6,   0, {1,3}	, 40, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Iron Crown" 		,0x00000000L,	TV_HELM, ']',	/* 98*/
    0,	 500,	6,   1,  20,	0,   0,	 0,   0, {1,1}	, 44, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Golden Crown"		,0x00000000L,	TV_HELM, ']',	/* 99*/
    0,	1000,	7,   1,  30,	0,   0,	 0,   0, {1,1}	, 47, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"& Jewel Encrusted Crown"	,0x00000000L,	TV_HELM, ']',	/*100*/
    0,	2000,	8,   1,  40,	0,   0,	 0,   0, {1,1}	, 50, 0, 0L
#ifdef TC_COLOR
  ,MAGENTA
#endif
},
{"& Robe"			,0x00000000L,	TV_SOFT_ARMOR, '(',/*101*/
    0,	   4,	1,   1,  20,	0,   0,	 2,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Robe"		        ,0x00000000L,	TV_SOFT_ARMOR, '(',/*102*/
    0,	   4,	6,   1,  20,	0,   0,	 2,   0, {0,0}	,  50, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Soft Leather Armour"		,0x00000000L,	TV_SOFT_ARMOR, '(',/*103*/
    0,	  18,	2,   1,  80,	0,   0,	 4,   0, {0,0}	,  2, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Soft Studded Leather"		,0x00000000L,	TV_SOFT_ARMOR, '(',/*104*/
    0,	  35,	3,   1,  90,	0,   0,	 5,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Hard Leather Armour"		,0x00000000L,	TV_SOFT_ARMOR, '(',/*105*/
    0,	  55,	4,   1, 100,  -1,   0,	 6,   0, {1,1}	,  5, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Hard Studded Leather"		,0x00000000L,	TV_SOFT_ARMOR, '(',/*106*/
    0,	 100,	5,   1, 110,  -1,   0,	 7,   0, {1,2}	,  7, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Leather Scale Mail"		,0x00000000L,	TV_SOFT_ARMOR, '(',/*107*/
    0,	 330,	9,   1, 140,  -1,   0,	11,   0, {1,1}	, 14, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Metal Scale Mail"		,0x00000000L,	TV_HARD_ARMOR, '[',/*108*/
    0,	 430,	1,   1, 250,  -2,   0,	13,   0, {1,4}	, 24, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Chain Mail"			,0x00000000L,	TV_HARD_ARMOR, '[',/*109*/
    0,	 530,	2,   1, 220,  -2,   0,	14,   0, {1,4}	, 26, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Rusty Chain Mail"		,0x00000000L,	TV_HARD_ARMOR, '[',/*110*/
    0,	   0,	3,   1, 200,  -5,   0,	14,  -8, {1,4}	, 26, 0, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"Augmented Chain Mail"		,0x00000000L,	TV_HARD_ARMOR, '[',/*111*/
    0,	 675,	5,   1, 270,  -2,   0,	16,   0, {1,4}	, 30, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Bar Chain Mail"		,0x00000000L,	TV_HARD_ARMOR, '[',/*112*/
    0,	 720,	6,   1, 280,  -2,   0,	18,   0, {1,4}	, 34, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Metal Brigandine Armour"	,0x00000000L,	TV_HARD_ARMOR, '[',/*113*/
    0,	 775,	7,   1, 290,  -3,   0,	19,   0, {1,4}	, 36, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Partial Plate Armour"		,0x00000000L,	TV_HARD_ARMOR, '[',/*114*/
    0,	 900,	9,   1, 260,  -3,   0,	22,   0, {1,6}	, 42, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Metal Lamellar Armour"	,0x00000000L,	TV_HARD_ARMOR, '[',/*115*/
    0,	 950,	10,   1, 340,  -3,   0,	23,   0, {1,6}	, 44, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Full Plate Armour"		,0x00000000L,	TV_HARD_ARMOR, '[',/*116*/
    0,	1050,	11,   1, 380,  -3,   0,	25,   0, {2,4}	, 48, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Ribbed Plate Armour"		,0x00000000L,	TV_HARD_ARMOR, '[',/*117*/
    0,	1200,	12,   1, 380,  -3,   0,	28,   0, {2,4}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Adamantite Plate Mail"	,0x00100000L,	TV_HARD_ARMOR, '[',/*118*/
    0,	20000,	13,   1, 420,  -4,   0,	40,   0, {2,4}	, 75, 7, 0L
#ifdef TC_COLOR
  ,LIGHTMAGENTA
#endif
},
{"Mithril Plate Mail"		,0x00100000L,	TV_HARD_ARMOR, '[',/*119*/
    0,	15000,	14,   1, 300,  -3,   0,	35,   0, {2,4}	, 60, 5, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"Mithril Chain Mail"		,0x00100000L,	TV_HARD_ARMOR, '[',/*120*/
    0,	7000,	15,   1, 150,  -1,   0,	28,   0, {1,4}	, 55, 4, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"Double Chain Mail"    	,0x00000000L,	TV_HARD_ARMOR, '[',/*109*/
    0,	 630,	16,   1, 250,  -2,   0,	16,   0, {1,4}	, 30, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Shield of Deflection"		,0x00000000L,	TV_SHIELD, '[',/*122*/
    0,	10000,	17,   1, 100,  0,   0,	10,   10, {1,1}	, 70, 6, 0L
#ifdef TC_COLOR
  ,LIGHTMAGENTA
#endif
},
{"& Cloak"			,0x00000000L,	TV_CLOAK, '(',	/*123*/
    0,	   3,	1,   1,  10,	0,   0,	 1,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Shadow Cloak"		,0x00000000L,	TV_CLOAK, '(',	/*124*/
    0,	4000,	2,   1,   5,	0,   0,	 6,   4, {0,0}	,  60, 3, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Set of Leather Gloves"	,0x00000000L,	TV_GLOVES, ']',	/*125*/
    0,	   3,	1,   1,   5,	0,   0,	 1,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Set of Gauntlets"		,0x00000000L,	TV_GLOVES, ']',	/*126*/
    0,	  35,	2,   1,  25,	0,   0,	 2,   0, {1,1}	, 12, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Set of Cesti"		,0x00000000L,	TV_GLOVES, ']',	/*127*/
    0,	  100,	3,   1,  40,	0,   0,	 5,   0, {1,1}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Small Leather Shield"	,0x00000000L,	TV_SHIELD, ')',	/*128*/
    0,	  30,	1,   1,  50,	0,   0,	 2,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Large Leather Shield"	,0x00000000L,	TV_SHIELD, ')',	/*129*/
    0,	 120,	3,   1, 100,	0,   0,	 4,   0, {1,2}	, 15, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Small Metal Shield"		,0x00000000L,	TV_SHIELD, ')',	/*130*/
    0,	  50,	4,   1,  65,	0,   0,	 3,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Large Metal Shield"		,0x00000000L,	TV_SHIELD, ')',	/*131*/
    0,	 200,	6,   1, 120,	0,   0,	 5,   0, {1,3}	, 30, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Strength"	        	,0x00000001L,	TV_RING, '=',	/*132*/
    0,	 400,	0,   1,   2,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dexterity"     		,0x00000008L,	TV_RING, '=',	/*133*/
    0,	 400,	1,   1,   2,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Constitution"	        	,0x00000010L,	TV_RING, '=',	/*134*/
    0,	 400,	2,   1,   2,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Intelligence"	        	,0x00000002L,	TV_RING, '=',	/*135*/
    0,	 350,	3,   1,   2,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Speed"			,0x00001000L,	TV_RING, '=',	/*136*/
    0, 300000L,  4,   1,   2,	0,   0,	 0,   0, {0,0}	, 100, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Searching"			,0x00000040L,	TV_RING, '=',	/*137*/
    0,	 250,	5,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Teleportation"		,0x80000400L,	TV_RING, '=',	/*138*/
    0,	   0,	6,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Digestion"		,0x00000080L,	TV_RING, '=',	/*139*/
    0,	 250,	7,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Resist Fire"			,0x00080000L,	TV_RING, '=',	/*140*/
    0,	 250,	8,   1,   2,	0,   0,	 0,   0, {0,0}	, 14, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Resist Cold"			,0x00200000L,	TV_RING, '=',	/*141*/
    0,	 250,	9,   1,   2,	0,   0,	 0,   0, {0,0}	, 14, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Feather Falling"		,0x04000000L,	TV_RING, '=',	/*142*/
    0,	 200,	10,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Poison Resistance"		,0x10000000L,	TV_RING, '=',	/*143*/
    0,	16000,	11,   1,   2,	0,   0,	 0,   0, {0,0}	, 55, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Arrow~"			,0x00000000L,	TV_ARROW, '{',	/*144*/
    0,	   1, 193,   1,   2,	0,   0,	 0,   0, {1,4}	, 15, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Weakness"			,0x80000001L,	TV_RING, '=',	/*145*/
   -5,	   0,	13,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Flames"	                ,0x00080000L,	TV_RING, '=',	/*146*/
    0,	3000,	14,   1,   2,	0,   0,	 0,   15, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Acid"         	        ,0x00100000L,	TV_RING, '=',	/*147*/
    0,	3000,	15,   1,   2,	0,   0,	 0,   15, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Ice"          	        ,0x00200000L,	TV_RING, '=',	/*148*/
    0,	3000,	16,   1,   2,	0,   0,	 0,   15, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Woe"				,0x80000644L,	TV_RING, '=',	/*149*/
   -5,	   0,	17,   1,   2,	0,   0,	 0,  -3, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Stupidity"			,0x80000002L,	TV_RING, '=',	/*150*/
   -5,	   0,	18,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Damage"	        	,0x00000000L,	TV_RING, '=',	/*151*/
    0,	 500,	19,   1,   2,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Accuracy"	        	,0x00000000L,	TV_RING, '=',	/*152*/
    0,	 500,	20,   1,   2,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Protection"			,0x00000000L,	TV_RING, '=',	/*153*/
    0,	 500,	21,   1,   2,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Aggravate Monster"		,0x80000200L,	TV_RING, '=',	/*154*/
    0,	   0,	22,   1,   2,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"See Invisible"		,0x01000000L,	TV_RING, '=',	/*155*/
    0,	 340,	23,   1,   2,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Strength"		,0x00400000L,	TV_RING, '=',	/*156*/
    1,	 750,	24,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Intelligence"		,0x00400000L,	TV_RING, '=',	/*157*/
    2,	 600,	25,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Wisdom"		,0x00400000L,	TV_RING, '=',	/*158*/
    3,	 600,	26,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Constitution"		,0x00400000L,	TV_RING, '=',	/*159*/
    4,	 750,	27,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Dexterity"		,0x00400000L,	TV_RING, '=',	/*160*/
    5,	 750,	28,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sustain Charisma"		,0x00400000L,	TV_RING, '=',	/*161*/
    6,	 500,	29,   1,   2,	0,   0,	 0,   0, {0,0}	, 28, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slaying"			,0x00000000L,	TV_RING, '=',	/*162*/
    0,	1000,	30,   1,   2,	0,   0,	 0,   0, {0,0}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Wisdom"			,0x00000004L,	TV_AMULET, '"',	/*163*/
    0,	 300,	0,   1,   3,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Charisma"			,0x00000020L,	TV_AMULET, '"',	/*164*/
    0,	 250,	1,   1,   3,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Searching"			,0x00000040L,	TV_AMULET, '"',	/*165*/
    0,	 250,	2,   1,   3,	0,   0,	 0,   0, {0,0}	, 14, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Teleportation"		,0x80000400L,	TV_AMULET, '"',	/*166*/
    0,	   0,	3,   1,   3,	0,   0,	 0,   0, {0,0}	, 14, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Digestion"		,0x00000080L,	TV_AMULET, '"',	/*167*/
    0,	 200,	4,   1,   3,	0,   0,	 0,   0, {0,0}	, 14, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Resist Acid"			,0x00100000L,	TV_AMULET, '"',	/*168*/
    0,	 300,	5,   1,   3,	0,   0,	 0,   0, {0,0}	, 24, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Adornment"			,0x00000000L,	TV_AMULET, '"',	/*169*/
    0,	  20,	6,   1,   3,	0,   0,	 0,   0, {0,0}	, 16, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Bolt~"			,0x00000000L,	TV_BOLT, '{',	/*170*/
    0,	   2, 193,   1,   3,	0,   0,	 0,   0, {1,5}	, 25, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"the Magi"			,0x01800040L,	TV_AMULET, '"',	/*171*/
    0,	30000,	8,   1,   3,	0,   0,	 0,   3, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Doom"				,0x8000007FL,	TV_AMULET, '"',	/*172*/
   -5,	   0,	9,   1,   3,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Enchant Weapon To-Hit"	,0x00000001L,	TV_SCROLL1, '?',/*173*/
    0,	 125,	64,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Enchant Weapon To-Dam"	,0x00000002L,	TV_SCROLL1, '?',/*174*/
    0,	 125,	65,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Enchant Armour"		,0x00000004L,	TV_SCROLL1, '?',/*175*/
    0,	 125,	66,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Identify"			,0x00000008L,	TV_SCROLL1, '?',/*176*/
    0,	  50,	67,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Identify"			,0x00000008L,	TV_SCROLL1, '?',/*177*/
    0,	  50,	67,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Identify"			,0x00000008L,	TV_SCROLL1, '?',/*178*/
    0,	  50,	67,   1,   5,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Identify"			,0x00000008L,	TV_SCROLL1, '?',/*179*/
    0,	  50,	67,   1,   5,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Remove Curse"			,0x00000010L,	TV_SCROLL1, '?',/*180*/
    0,	 100,	68,   1,   5,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Light"			,0x00000020L,	TV_SCROLL1, '?',/*181*/
    0,	  15,	69,   1,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Light"			,0x00000020L,	TV_SCROLL1, '?',/*182*/
    0,	  15,	69,   1,   5,	0,   0,	 0,   0, {0,0}	,  3, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Light"			,0x00000020L,	TV_SCROLL1, '?',/*183*/
    0,	  15,	69,   1,   5,	0,   0,	 0,   0, {0,0}	,  7, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Summon Monster"		,0x00000040L,	TV_SCROLL1, '?',/*184*/
    0,	   0,	70,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Phase Door"			,0x00000080L,	TV_SCROLL1, '?',/*185*/
    0,	  15,	71,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Teleport"			,0x00000100L,	TV_SCROLL1, '?',/*186*/
    0,	  40,	72,   1,   5,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Teleport Level"		,0x00000200L,	TV_SCROLL1, '?',/*187*/
    0,	  50,	73,   1,   5,	0,   0,	 0,   0, {0,0}	, 20, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Monster Confusion"		,0x00000400L,	TV_SCROLL1, '?',/*188*/
    0,	  30,	74,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Magic Mapping"		,0x00000800L,	TV_SCROLL1, '?',/*189*/
    0,	  40,	75,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Rune of Protection"		,0x00002000L,	TV_SCROLL1, '?',/*190*/
    0,	 500,	77,   1,   5,	0,   0,	 0,   0, {0,0}	, 90, 3, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Rune of Protection"		,0x00002000L,	TV_SCROLL1, '?',/*191*/
    0,	 500,	77,   1,   5,	0,   0,	 0,   0, {0,0}	, 60, 2, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Treasure Detection"		,0x00004000L,	TV_SCROLL1, '?',/*192*/
    0,	  15,	78,   1,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Object Detection"		,0x00008000L,	TV_SCROLL1, '?',/*193*/
    0,	  15,	79,   1,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Trap Detection"		,0x00010000L,	TV_SCROLL1, '?',/*194*/
    0,	  35,	80,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Trap Detection"		,0x00010000L,	TV_SCROLL1, '?',/*195*/
    0,	  35,	80,   1,   5,	0,   0,	 0,   0, {0,0}	,  8, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},

{"Trap Location"		,RD_TRAP_LOC,  TV_ROD, '-',	/*196*/
    0,	 100,	25,   1,  15,	0,   0,	 0,   0, {1,1}	, 5, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Door/Stair Location"		,0x00020000L,	TV_SCROLL1, '?',/*197*/
    0,	  35,	81,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Door/Stair Location"		,0x00020000L,	TV_SCROLL1, '?',/*198*/
    0,	  35,	81,   1,   5,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Door/Stair Location"		,0x00020000L,	TV_SCROLL1, '?',/*199*/
    0,	  35,	81,   1,   5,	0,   0,	 0,   0, {0,0}	, 15, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Mass Genocide"		,0x00040000L,	TV_SCROLL1, '?',/*200*/
    0,	1000,	82,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 4, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Detect Invisible"		,0x00080000L,	TV_SCROLL1, '?',/*201*/
    0,	  15,	83,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Aggravate Monster"		,0x00100000L,	TV_SCROLL1, '?',/*202*/
    0,	   0,	84,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Trap Creation"		,0x00200000L,	TV_SCROLL1, '?',/*203*/
    0,	   0,	85,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Trap/Door Destruction"	,0x00400000L,	TV_SCROLL1, '?',/*204*/
    0,	  50,	86,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"*Enchant Armour*"		,0x00000004L,	TV_SCROLL2, '?',/*205*/
    0,	 500,	96,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Recharging"			,0x01000000L,	TV_SCROLL1, '?',/*206*/
    0,	 200,	88,   1,   5,	0,   0,	 0,   0, {0,0}	, 40, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Genocide"			,0x02000000L,	TV_SCROLL1, '?',/*207*/
    0,	 750,	89,   1,   5,	0,   0,	 0,   0, {0,0}	, 37, 3, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Darkness"			,0x04000000L,	TV_SCROLL1, '?',/*208*/
    0,	   0,	90,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Protection from Evil"		,0x08000000L,	TV_SCROLL1, '?',/*209*/
    0,	  50,	91,   1,   5,	0,   0,	 0,   0, {0,0}	, 30, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Create Food"			,0x10000000L,	TV_SCROLL1, '?',/*210*/
    0,	  10,	92,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Dispel Undead"		,0x20000000L,	TV_SCROLL1, '?',/*211*/
    0,	 200,	93,   1,   5,	0,   0,	 0,   0, {0,0}	, 40, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"*Enchant Weapon*"		,0x00000001L,	TV_SCROLL2, '?',/*212*/
    0,	 500,	94,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Curse Weapon"			,0x00000002L,	TV_SCROLL2, '?',/*213*/
    0,	   0,	95,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"*Enchant Armour*"		,0x00000004L,	TV_SCROLL2, '?',/*214*/
    0,	 500,	96,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Curse Armour"			,0x00000008L,	TV_SCROLL2, '?',/*215*/
    0,	   0,	97,   1,   5,	0,   0,	 0,   0, {0,0}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Summon Undead"		,0x00000010L,	TV_SCROLL2, '?',/*216*/
    0,	   0,	98,   1,   5,	0,   0,	 0,   0, {0,0}	, 15, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Blessing"			,0x00000020L,	TV_SCROLL2, '?',/*217*/
    0,	  15,	99,   1,   5,	0,   0,	 0,   0, {0,0}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Holy Chant"			,0x00000040L,	TV_SCROLL2, '?',/*218*/
    0,	  40, 100,   1,   5,	0,   0,	 0,   0, {0,0}	, 12, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Holy Prayer"			,0x00000080L,	TV_SCROLL2, '?',/*219*/
    0,	  80, 101,   1,   5,	0,   0,	 0,   0, {0,0}	, 24, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Word of Recall"		,0x00000100L,	TV_SCROLL2, '?',/*220*/
    0,	 150, 102,   1,   5,	0,   0,	 0,   0, {0,0}	,  5, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"*Destruction*"		,0x00000200L,	TV_SCROLL2, '?',/*221*/
    0,	 250, 103,   1,   5,	0,   0,	 0,   0, {0,0}	, 40, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Slime Mold Juice"		,0x30000000L,	TV_POTION1, '!',/*222*/
  400,	   2,	64,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Apple Juice"			,0x00000000L,	TV_POTION1, '!',/*223*/
  250,	   1,	65,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Water"			,0x00000000L,	TV_POTION1, '!',/*224*/
  200,	   0,	66,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Strength"	         	,0x00000001L,	TV_POTION1, '!',/*225*/
    0,	 8000,	67,   1,   4,	0,   0,	 0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Weakness"			,0x00000002L,	TV_POTION1, '!',/*226*/
    0,	   0,	68,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Strength"		,0x00000004L,	TV_POTION1, '!',/*227*/
    0,	 300,	69,   1,   4,	0,   0,	 0,   0, {1,1}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Intelligence" 		,0x00000008L,	TV_POTION1, '!',/*228*/
    0,	 8000,	70,   1,   4,	0,   0,	 0,   0, {1,1}	, 32, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Stupidity"      		,0x00000010L,	TV_POTION1, '!',/*229*/
    0,	   0,	71,   1,   4,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Intelligence"		,0x00000020L,	TV_POTION1, '!',/*230*/
    0,	 300,	72,   1,   4,	0,   0,	 0,   0, {1,1}	, 26, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Wisdom"			,0x00000040L,	TV_POTION1, '!',/*231*/
    0,	 8000,	73,   1,   4,	0,   0,	 0,   0, {1,1}	, 32, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Naivety"			,0x00000080L,	TV_POTION1, '!',/*232*/
    0,	   0,	74,   1,   4,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Wisdom"		,0x00000100L,	TV_POTION1, '!',/*233*/
    0,	 300,	75,   1,   4,	0,   0,	 0,   0, {1,1}	, 26, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Charisma"			,0x00000200L,	TV_POTION1, '!',/*234*/
    0,	 1000,	76,   1,   4,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Ugliness"			,0x00000400L,	TV_POTION1, '!',/*235*/
    0,	   0,	77,   1,   4,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Charisma"		,0x00000800L,	TV_POTION1, '!',/*236*/
    0,	 300,	78,   1,   4,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Light Wounds"		,0x10001000L,	TV_POTION1, '!',/*237*/
   50,	  15,	79,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Light Wounds"		,0x10001000L,	TV_POTION1, '!',/*238*/
   50,	  15,	79,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Light Wounds"		,0x10001000L,	TV_POTION1, '!',/*239*/
   50,	  15,	79,   1,   4,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Serious Wounds"		,0x30002000L,	TV_POTION1, '!',/*240*/
  100,	  40,	80,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Critical Wounds"		,0x70004000L,	TV_POTION1, '!',/*241*/
  100,	 100,	81,   1,   4,	0,   0,	 0,   0, {1,1}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Healing"			,0x70008000L,	TV_POTION1, '!',/*242*/
  200,	 200,	82,   1,   4,	0,   0,	 0,   0, {1,1}	, 12, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Constitution"	         	,0x00010000L,	TV_POTION1, '!',/*243*/
    0,	 8000,	83,   1,   4,	0,   0,	 0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Experience"		        ,0x00020000L,	TV_POTION1, '!',/*244*/
    0,	25000,	84,   1,   4,	0,   0,	 0,   0, {1,1}	, 65, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sleep"			,0x00040000L,	TV_POTION1, '!',/*245*/
  100,	   0,	85,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Blindness"			,0x00080000L,	TV_POTION1, '!',/*246*/
    0,	   0,	86,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Confusion"			,0x00100000L,	TV_POTION1, '!',/*247*/
   50,	   0,	87,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Poison"			,0x00200000L,	TV_POTION1, '!',/*248*/
    0,	   0,	88,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Speed"			,0x00400000L,	TV_POTION1, '!',/*249*/
    0,	  75,	89,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slowness"			,0x00800000L,	TV_POTION1, '!',/*250*/
   50,	   0,	90,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dexterity"	         	,0x02000000L,	TV_POTION1, '!',/*251*/
    0,	 8000,	91,   1,   4,	0,   0,	 0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Dexterity"		,0x04000000L,	TV_POTION1, '!',/*252*/
    0,	 300,	92,   1,   4,	0,   0,	 0,   0, {1,1}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Constitution"		,0x68000000L,	TV_POTION1, '!',/*253*/
    0,	 300,	93,   1,   4,	0,   0,	 0,   0, {1,1}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Lose Memories"		,0x00000002L,	TV_POTION2, '!',/*254*/
    0,	   0,	95,   1,   4,	0,   0,	 0,   0, {1,1}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Salt Water"			,0x00000004L,	TV_POTION2, '!',/*255*/
    0,	   0,	96,   1,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Speed"	        	,0x00400000L,	TV_POTION1, '!',/*256*/
    0,	  75,	89,   1,   4,	0,   0,	 0,   0, {1,1}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Heroism"			,0x00000010L,	TV_POTION2, '!',/*257*/
    0,	  35,	98,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Berserk Strength"		,0x00000020L,	TV_POTION2, '!',/*258*/
    0,	 100,	99,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Boldness"			,0x00000040L,	TV_POTION2, '!',/*259*/
    0,	  10, 100,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Life Levels"		,0x00000080L,	TV_POTION2, '!',/*260*/
    0,	 400, 101,   1,   4,	0,   0,	 0,   0, {1,1}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Resist Heat"			,0x00000100L,	TV_POTION2, '!',/*261*/
    0,	  30, 102,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Resist Cold"			,0x00000200L,	TV_POTION2, '!',/*262*/
    0,	  30, 103,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Detect Invisible"		,0x00000400L,	TV_POTION2, '!',/*263*/
    0,	  50, 104,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Poison"			,0x00000800L,	TV_POTION2, '!',/*264*/
    0,	  25, 105,   1,   4,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Neutralize Poison"		,0x00001000L,	TV_POTION2, '!',/*265*/
    0,	  75, 106,   1,   4,	0,   0,	 0,   0, {1,1}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restore Mana"			,0x00002000L,	TV_POTION2, '!',/*266*/
    0,	 350, 107,   1,   4,	0,   0,	 0,   0, {1,1}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Infra-vision"			,0x00004000L,	TV_POTION2, '!',/*267*/
    0,	  20, 108,   1,   4,	0,   0,	 0,   0, {1,1}	,  3, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Flask~ of oil"		,0x00040000L,	TV_FLASK, '!',	/*268*/
 7500,	   3,	64,   1,  10,	0,   0,	 0,   0, {2,6}	,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"Light"			,WD_LT,	        TV_WAND, '-',	/*269*/
    0,	 200,	0,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Lightning Bolts"		,WD_LT_BLTS,	TV_WAND, '-',	/*270*/
    0,	 600,	1,   1,  10,	0,   0,	 0,   0, {1,1}	, 15, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Frost Bolts"			,WD_FT_BLTS,	TV_WAND, '-',	/*271*/
    0,	 800,	2,   1,  10,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Fire Bolts"			,WD_FR_BLTS,	TV_WAND, '-',	/*272*/
    0,	1000,	3,   1,  10,	0,   0,	 0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Stone to Mud"			,WD_ST_MUD,	TV_WAND, '-',	/*273*/
    0,	 300,	4,   1,  10,	0,   0,	 0,   0, {1,1}	, 12, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Polymorph"			,WD_POLY,	TV_WAND, '-',	/*274*/
    0,	 400,	5,   1,  10,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Heal Monster"			,WD_HEAL_MN,	TV_WAND, '-',	/*275*/
    0,	   0,	6,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Haste Monster"		,WD_HAST_MN,	TV_WAND, '-',	/*276*/
    0,	   0,	7,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Monster"			,WD_SLOW_MN,	TV_WAND, '-',	/*277*/
    0,	 500,	8,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Confuse Monster"		,WD_CONF_MN,	TV_WAND, '-',	/*278*/
    0,	 400,	9,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sleep Monster"		,WD_SLEE_MN,	TV_WAND, '-',	/*279*/
    0,	 500,	10,   1,  10,	0,   0,	 0,   0, {1,1}	,  7, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Drain Life"			,WD_DRAIN,	TV_WAND, '-',	/*280*/
    0,	1200,	11,   1,  10,	0,   0,	 0,   0, {1,1}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Trap/Door Destruction"	,WD_TR_DEST,	TV_WAND, '-',	/*281*/
    0,	 100,	12,   1,  10,	0,   0,	 0,   0, {1,1}	, 12, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Magic Missile"		,WD_MAG_MIS,	TV_WAND, '-', 	/*282*/
    0,	 200,	13,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Clone Monster"		,WD_CLONE,	TV_WAND, '-',	/*283*/
    0,	   0,	15,   1,  10,	0,   0,	 0,   0, {1,1}	, 15, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Clone Monster"		,WD_CLONE,	TV_WAND, '-',	/*284*/
    0,	   0,	15,   1,  10,	0,   0,	 0,   0, {1,1}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Teleport Away"		,WD_TELE,	TV_WAND, '-',	/*285*/
    0,	 350,	16,   1,  10,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Disarming"			,WD_DISARM,	TV_WAND, '-',	/*286*/
    0,	 700,	17,   1,  10,	0,   0,	 0,   0, {1,1}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Lightning Balls"		,WD_LT_BALL,	TV_WAND, '-',	/*287*/
    0,	1200,	18,   1,  10,	0,   0,	 0,   0, {1,1}	, 35, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cold Balls"			,WD_CD_BALL,	TV_WAND, '-',	/*288*/
    0,	1500,	19,   1,  10,	0,   0,	 0,   0, {1,1}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Fire Balls"			,WD_FR_BALL,	TV_WAND, '-',	/*289*/
    0,	1800,	20,   1,  10,	0,   0,	 0,   0, {1,1}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Stinking Cloud"		,WD_ST_CLD,	TV_WAND, '-',	/*290*/
    0,	 400,	21,   1,  10,	0,   0,	 0,   0, {1,1}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Acid Balls"			,WD_AC_BALL,	TV_WAND, '-',	/*291*/
    0,	1650,	22,   1,  10,	0,   0,	 0,   0, {1,1}	, 48, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Wonder"			,WD_WONDER,	TV_WAND, '-',	/*292*/
    0,	 250,	23,   1,  10,	0,   0,	 0,   0, {1,1}	,  2, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Light"			,ST_LIGHT,	TV_STAFF, '_',	/*293*/
    0,	 250,	0,    1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Acid Bolts"			,WD_AC_BLTS,	TV_WAND, '-',   /*294*/
    0,	950,	28,   1,  10,   0,   0,  0,   0, {1,1}	, 30, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dragon's Flame"	     	,WD_DRG_FIRE,	TV_WAND, '-',   /*295*/
    0,	2400,	24,   1,  10,	0,   0,	 0,   0, {1,1}	, 50, 3, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dragon's Frost"       	,WD_DRG_FRST,	TV_WAND, '-',    /*296*/
    0,	2400,	25,   1,  10,	0,   0,	 0,   0, {1,1}	, 50, 3, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dragon's Breath"		,WD_DRG_BREA,	TV_WAND, '-',    /*297*/
    0,  2400,   26,   1,  10,   0,   0,  0,   0, {1,1}  , 50, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Annihilation"			,WD_ANHIL,	TV_WAND, '-',    /*298*/
    0,  3000,   27,   1,  10,   0,   0,  0,   0, {1,1}  , 50, 4, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Door/Stair Location"		,ST_DR_LC,	TV_STAFF, '_',	/*299*/
    0,	 350,	1,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Trap Location"		,ST_TRP_LC,	TV_STAFF, '_',	/*300*/
    0,	 350,	2,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Treasure Location"		,ST_TRE_LC,	TV_STAFF, '_',	/*301*/
    0,	 200,	3,   1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Object Location"		,ST_OBJ_LC,	TV_STAFF, '_',	/*302*/
    0,	 200,	4,   1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Teleportation"		,ST_TELE,	TV_STAFF, '_',	/*303*/
    0,	2000,	5,   1,  50,	0,   0,	 0,   0, {1,2}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Earthquakes"			,ST_EARTH,	TV_STAFF, '_',	/*304*/
    0,	 350,	6,   1,  50,	0,   0,	 0,   0, {1,2}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Summoning"			,ST_SUMMON,	TV_STAFF, '_',	/*305*/
    0,	   0,	7,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Summoning"			,ST_SUMMON,	TV_STAFF, '_',	/*306*/
    0,	   0,	7,   1,  50,	0,   0,	 0,   0, {1,2}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"*Destruction*"		,ST_DEST,	TV_STAFF, '_',	/*307*/
    0,	2500,	8,   1,  50,	0,   0,	 0,   0, {1,2}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Starlight"			,ST_STAR,	TV_STAFF, '_',	/*308*/
    0,	 800,	9,   1,  50,	0,   0,	 0,   0, {1,2}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Haste Monsters"		,ST_HAST_MN,	TV_STAFF, '_',	/*309*/
    0,	   0,	10,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Monsters"		,ST_SLOW_MN,	TV_STAFF, '_',	/*310*/
    0,	 800,	11,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sleep Monsters"		,ST_SLEE_MN,	TV_STAFF, '_',	/*311*/
    0,	 700,	12,   1,  50,	0,   0,	 0,   0, {1,2}	, 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cure Light Wounds"		,ST_CURE_LT,	TV_STAFF, '_',	/*312*/
    0,	 350,	13,   1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Detect Invisible"		,ST_DET_INV,	TV_STAFF, '_',	/*313*/
    0,	 200,	14,   1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Speed"			,ST_SPEED,	TV_STAFF, '_',	/*314*/
    0,	1000,	15,   1,  50,	0,   0,	 0,   0, {1,2}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slowness"			,ST_SLOW,	TV_STAFF, '_',	/*315*/
    0,	   0,	16,   1,  50,	0,   0,	 0,   0, {1,2}	, 40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"*Destruction*"		,ST_DEST,	TV_STAFF, '_',	/*316*/
    0,	2500,	8,   1,  50,	0,   0,	 0,   0, {1,2}	, 70, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Remove Curse"			,ST_REMOVE,	TV_STAFF, '_',	/*317*/
    0,	 500,	18,   1,  50,	0,   0,	 0,   0, {1,2}	, 47, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Detect Evil"			,ST_DET_EVI,	TV_STAFF, '_',	/*318*/
    0,	 350,	19,   1,  50,	0,   0,	 0,   0, {1,2}	, 20, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Curing"			,ST_CURING,	TV_STAFF, '_',	/*319*/
    0,	1000,	20,   1,  50,	0,   0,	 0,   0, {1,2}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Dispel Evil"			,ST_DSP_EVI,	TV_STAFF, '_',	/*320*/
    0,	1200,	21,   1,  50,	0,   0,	 0,   0, {1,2}	, 49, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Darkness"			,ST_DARK,	TV_STAFF, '_',	/*321*/
    0,	   0,	22,   1,  50,	0,   0,	 0,   0, {1,2}	, 50, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Darkness"			,ST_DARK,	TV_STAFF, '_',	/*322*/
    0,	   0,	22,   1,  50,	0,   0,	 0,   0, {1,2}	,  5, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Genocide"			,ST_GENOCIDE,	TV_STAFF, '_',	/*323*/
    0,	3500,	23,   1,  50,	0,   0,	 0,   0, {1,2}	, 70, 4, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Power"			,ST_POWER,	TV_STAFF, '_',	/*324*/
    0,  4000,   24,   1,  50,   0,   0,  0,   0, {1,2}  , 70, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"the Magi"			,ST_MAGI,	TV_STAFF, '_',	/*325*/
    0,  4500,   25,   1,  50,   0,   0,  0,   0, {1,2}  , 70, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Perceptions"			,ST_IDENTIFY,	TV_STAFF, '_',	/*326*/
    0,  400,    26,   1,  50,   0,   0,  0,   0, {1,2}  , 10, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Holiness"			,ST_HOLYNESS, 	TV_STAFF, '_',	/*327*/
    0,  4500,   27,   1,  50,   0,   0,  0,   0, {1,2}  , 70, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Enlightenment"		,ST_SURROUND,	TV_STAFF, '_',/*328*/
    0,  750,	28,   1,  50,   0,   0,  0,   0, {1,2}  , 22, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Healing"			,ST_HEALING,	TV_STAFF, '_',	/*329*/
    0,  5000,   29,   1,  50,   0,   0,  0,   0, {1,2}  , 70, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"[Magic for Beginners]"	,0x000001FFL,	TV_MAGIC_BOOK, '?',/*330*/
    0,	  25,	64,   1,  30,   0,   0,	 0,   0, {1,1}	, 5, 0, 0x00000000L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"[Conjurings and Tricks]"	,0x0003FE00L,	TV_MAGIC_BOOK, '?',/*331*/
    0,	 100,	65,   1,  30,   0,   0,	 0,   0, {1,1}	, 10, 0, 0x00000000L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"[Incantations and Illusions]"	,0x03FC0000L,	TV_MAGIC_BOOK, '?',/*332*/
    0,	 400,	66,   1,  30,   0,   0,	 0,   0, {1,1}	, 20, 0, 0x00000000L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"[Sorcery and Evocations]"	,0x7C000000L,	TV_MAGIC_BOOK, '?',/*333*/
    0,	 800,	67,   1,  30,   0,   0,	 0,   0, {1,1}	, 25, 0, 0x00000001L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"[Beginners Handbook]"		,0x000000FFL,	TV_PRAYER_BOOK, '?',/*334*/
    0,	  25,	64,   1,  30,   0,   0,	 0,   0, {1,1}	, 5, 0, 0x00000000L
#ifdef TC_COLOR
  ,LIGHTBLUE
#endif
},
{"[Words of Wisdom]"		,0x0000FF00L,	TV_PRAYER_BOOK, '?',/*335*/
    0,	 100,	65,   1,  30,   0,   0,	 0,   0, {1,1}	, 10, 0, 0x00000000L
#ifdef TC_COLOR
  ,LIGHTBLUE
#endif
},
{"[Chants and Blessings]"	,0x01FF0000L,	TV_PRAYER_BOOK, '?',/*336*/
    0,	 300,	66,   1,  30,   0,   0,	 0,   0, {1,1}	, 20, 0, 0x00000000L
#ifdef TC_COLOR
  ,CYAN
#endif
},
{"[Exorcism and Dispelling]"	,0x7E000000L,	TV_PRAYER_BOOK, '?',/*337*/
    0,	 900,	67,   1,  30,   0,   0,	 0,   0, {1,1}	, 25, 0, 0x00000000L
#ifdef TC_COLOR
  ,CYAN
#endif
},
{"& Small wooden chest"		,0x0F000000L,	 TV_CHEST, '~',	/*338*/
    0,	  20,	1,   1, 250,	0,   0,	 0,   0, {2,3}	,  7, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Large wooden chest"		,0x15000000L,	 TV_CHEST, '~',	/*339*/
    0,	  60,	4,   1, 500,	0,   0,	 0,   0, {2,5}	, 15, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Small iron chest"		,0x0F000000L,	 TV_CHEST, '~',	/*340*/
    0,	 100,	7,   1, 300,	0,   0,	 0,   0, {2,4}	, 25, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Large iron chest"		,0x1F000000L,	 TV_CHEST, '~',	/*341*/
    0,	 150,	10,   1,1000,	0,   0,	 0,   0, {2,6}	, 35, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Small steel chest"		,0x0F000000L,	 TV_CHEST, '~',	/*342*/
    0,	 200,	13,   1, 500,	0,   0,	 0,   0, {2,4}	, 45, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Large steel chest"		,0x23000000L,	 TV_CHEST, '~',	/*343*/
    0,	 250,	16,   1,1000,	0,   0,	 0,   0, {2,6}	, 50, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Rat Skeleton"		,0x00000000L,	 TV_MISC, 's',	/*344*/
    0,	   0,	1,   1,  10,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& Giant Centipede Skeleton"	,0x00000000L,	 TV_MISC, 's',	/*345*/
    0,	   0,	2,   1,  25,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"some filthy rags"		,0x00000000L,	TV_SOFT_ARMOR, '~',/*346*/
    0,	   0,	63,   1,  20,	0,   0,	 1,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& empty bottle"		,0x00000000L,	 TV_MISC, '!',	/*347*/
    0,	   0,	4,   1,   2,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"some shards of pottery"	,0x00000000L,	 TV_MISC, '~',	/*348*/
    0,	   0,	5,   1,   5,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTRED
#endif
},
{"& Human Skeleton"		,0x00000000L,	 TV_MISC, 's',	/*349*/
    0,	   0,	7,   1,  50,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Dwarf Skeleton"		,0x00000000L,	 TV_MISC, 's',	/*350*/
    0,	   0,	8,   1,  60,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Elf Skeleton"		,0x00000000L,	 TV_MISC, 's',	/*351*/
    0,	   0,	9,   1,  40,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& Gnome Skeleton"		,0x00000000L,	 TV_MISC, 's',	/*352*/
    0,	   0,	10,   1,  25,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& broken set of teeth"	,0x00000000L,	 TV_MISC, 's',	/*353*/
    0,	   0,	11,   1,   3,	0,   0,	 0,   0, {1,5}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& large broken bone"		,0x00000000L,	 TV_MISC, 's',	/*354*/
    0,	   0,	12,   1,   2,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"& broken stick"		,0x00000000L,	 TV_MISC, '~',	/*355*/
    0,	   0,	13,   1,   3,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Light"			,RD_LT,	        TV_ROD, '-',	/*356*/
    0,	 600,	0,   1,  15,	0,   0,	 0,   0, {1,1}	, 12, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Lightning Bolts"		,RD_LT_BLTS,	TV_ROD , '-',	/*357*/
    0,	1800,	1,   1,  15,	0,   0,	 0,   0, {1,1}	, 20, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Frost Bolts"			,RD_FT_BLTS,	TV_ROD , '-',	/*358*/
    0,	2400,	2,   1,  15,	0,   0,	 0,   0, {1,1}	, 23, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Fire Bolts"			,RD_FR_BLTS,	TV_ROD , '-',	/*359*/
    0,	3000,	3,   1,  15,	0,   0,	 0,   0, {1,1}	, 29, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Polymorph"			,RD_POLY,	TV_ROD , '-',	/*360*/
    0,	1200,	4,   1,  15,	0,   0,	 0,   0, {1,1}	, 35, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Monster"			,RD_SLOW_MN,	TV_ROD , '-',	/*361*/
    0,	1500,	5,   1,  15,	0,   0,	 0,   0, {1,1}	, 32, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Sleep Monster"		,RD_SLEE_MN,	TV_ROD , '-',	/*362*/
    0,	1500,	6,   1,  15,	0,   0,	 0,   0, {1,1}	, 33, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Drain Life"			,RD_DRAIN,	TV_ROD , '-',	/*363*/
    0,	3600,	7,   1,  15,	0,   0,	 0,   0, {1,1}	, 75, 3, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Teleport Away"		,RD_TELE,	TV_ROD , '-',	/*364*/
    0,	1400,	8,   1,  15,	0,   0,	 0,   0, {1,1}	, 45, 2, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Disarming"			,RD_DISARM,	TV_ROD , '-',	/*365*/
    0,	2100,	9,   1,  15,	0,   0,	 0,   0, {1,1}	, 35, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Lightning Balls"		,RD_LT_BALL,	TV_ROD , '-',	/*366*/
    0,	3600,	10,   1,  15,	0,   0,	 0,   0, {1,1}	, 55, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Cold Balls"			,RD_CD_BALL,	TV_ROD , '-',	/*367*/
    0,	4500,	11,   1,  15,	0,   0,	 0,   0, {1,1}	, 60, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Fire Balls"			,RD_FR_BALL,	TV_ROD , '-',	/*368*/
    0,	4800,	12,   1,  15,	0,   0,	 0,   0, {1,1}	, 75, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Acid Balls"			,RD_AC_BALL,	TV_ROD , '-',	/*369*/
    0,	4600,	13,   1,  15,	0,   0,	 0,   0, {1,1}	, 70, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Acid Bolts"			,RD_AC_BLTS,	TV_ROD , '-',   /*370*/
    0,	3000,   14,   1,  15,   0,   0,  0,   0, {1,1}	, 38, 0, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Enlightenment"        	,RD_MAPPING,	TV_ROD , '-',   /*371*/
    0,	10000,   15,   1,  15,   0,   0,  0,   0, {1,1}	, 63, 4, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Perceptions"			,RD_IDENT,	TV_ROD , '-',   /*372*/
    0,	13000,   16,   1,  15,   0,   0,  0,   0, {1,1}	, 49, 8, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Curing"			,RD_CURE,	TV_ROD , '-',   /*373*/
    0,	15000,   17,   1,  15,   0,   0,  0,   0, {1,1}	, 65, 8, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Healing"			,RD_HEAL,	TV_ROD , '-',   /*374*/
    0,	20000,   18,   1,  15,   0,   0,  0,   0, {1,1}	, 77,13, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Detection"			,RD_DETECT,	TV_ROD , '-',   /*375*/
    0,	3500,   19,   1,  15,   0,   0,  0,   0, {1,1}	, 30,11, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Restoration"			,RD_RESTORE,	TV_ROD , '-',   /*376*/
    0,	25000,   20,   1,  15,   0,   0,  0,   0, {1,1}	, 80,15, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Speed"			,RD_SPEED,	TV_ROD , '-',   /*377*/
    0,	30000,   21,   1,  15,   0,   0,  0,   0, {1,1}	, 99, 9, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"*Remove Curse*",               0x40000000L,	TV_SCROLL1, '?',/*378*/
    0,	 8000,	76,   1,  30,   0,   0,	 0,   0, {1,1}	, 50, 2, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"[Resistance of Scarabtarices]",0x00000000L,	TV_MAGIC_BOOK, '?',/*379*/
    0,	 8000,	68,   1,  30,   0,   0,	 0,   0, {1,1}	, 30, 0, 0x03E0000L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"[Mordenkainen's Escapes]"      ,0x00000000L,	TV_MAGIC_BOOK, '?',/*380*/
    0,	 10000,	71,   1,  30,   0,   0,	 0,   0, {1,1}	, 50, 0, 0x000003EL
#ifdef TC_COLOR
  ,LIGHTRED
#endif
},
{"[Kelek's Grimoire of Power]"   ,0x00000000L,	TV_MAGIC_BOOK, '?',/*381*/
    0,	 30000,	70,   1,  30,   0,   0,	 0,   0, {1,1}	, 60, 0, 0x001F000L
#ifdef TC_COLOR
  ,LIGHTRED
#endif
},
{"[Tenser's Transformations]"  	,0x00000000L,	TV_MAGIC_BOOK, '?',/*382*/
    0,	 50000L,	69,   1,  30,   0,   0,	 0,   0, {1,1}	, 80, 2, 0x7C00000L
#ifdef TC_COLOR
  ,LIGHTRED
#endif
},
{"[Raal's Tome of Destruction]"  ,0x00000000L,	TV_MAGIC_BOOK, '?',/*383*/
    0,	 100000L, 72,   1,  30,  0,  0,	0, 0, {1,1},     100, 4, 0x0000FC0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"[Ethereal Openings]"         	,0x00000000L,	TV_PRAYER_BOOK, '?',/*384*/
    0,	 7000,	68,   1,  30,   0,   0,	 0,   0, {1,1}	, 30, 0, 0x3F00000L
#ifdef TC_COLOR
  ,CYAN
#endif
},
{"[Godly Insights]"           	,0x80000000L,	TV_PRAYER_BOOK, '?',/*385*/
    0,	 9000,	69,   1,  30,   0,   0,	 0,   0, {1,1}	, 50, 0, 0x000000FL
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"[Purifications and Healing]" 	,0x00000000L,	TV_PRAYER_BOOK, '?',/*386*/
    0,	 25000,	70,   1,  30,   0,   0,	 0,   0, {1,1}	, 60, 0, 0x00001F0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"[Holy Infusions]"            ,0x00000000L,	TV_PRAYER_BOOK, '?',/*388*/
    0,	50000L,	71,   1,  30,   0,   0,	 0,   0, {1,1}	, 80, 3, 0x00FC000L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"[Wrath of God]"              ,0x00000000L,	TV_PRAYER_BOOK, '?',/*387*/
    0,	100000L,	72,   1,  30,   0,   0,	 0,   0, {1,1}	, 100, 4, 0x0003E00L
#ifdef TC_COLOR
  ,BLUE
#endif
},
{"Blue Dragon Scale Mail"        ,0x02100000L,	TV_HARD_ARMOR, '[',/*389*/
    0,	35000L,	17,   1, 200,  -2,   0,	30,   10, {2,4}	, 40, 8, 0x00000800L
#ifdef TC_COLOR
  ,BLUE
#endif
},
{"White Dragon Scale Mail"       ,0x00300000L,	TV_HARD_ARMOR, '[',/*390*/
    0,	40000L,	18,   1, 200,  -2,   0,	30,   10, {2,4}	, 50, 8, 0x00000800L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Black Dragon Scale Mail"       ,0x00100000L,	TV_HARD_ARMOR, '[',/*391*/
    0,	30000,	19,   1, 200,  -2,   0,	30,   10, {2,4}	, 60, 8, 0x00000800L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Green Dragon Scale Mail"       ,0x10100000L,	TV_HARD_ARMOR, '[',/*392*/
    0,	80000L,	20,   1, 200,  -2,   0,	30,   10, {2,4}	, 70,10, 0x00000800L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"Red Dragon Scale Mail"         ,0x00180000L,	TV_HARD_ARMOR, '[',/*393*/
    0,	100000L,	21,   1, 200,  -2,   0,	30,   10, {2,4}	, 80, 8, 0x00000800L
#ifdef TC_COLOR
  ,RED
#endif
},
{"Multi-Hued Dragon Scale Mail"  ,0x12380000L,	TV_HARD_ARMOR, '[',/*394*/
    0,	150000L,	22,   1, 200,  -2,   0,	30,   10, {2,4}	,100,20, 0x00000800L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Dagger"	         	,0x00000000L,	TV_SWORD, '|',	/*395*/
    0,	  10,	25,   1,  12,	0,   0,	 0,   0, {1,4}	,  5, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Dagger"	         	,0x00000000L,	TV_SWORD, '|',	/*396*/
    0,	  10,	26,   1,  12,	0,   0,	 0,   0, {1,4}	,  10, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Dagger"	         	,0x00000000L,	TV_SWORD, '|',	/*397*/
    0,	  10,	27,   1,  12,	0,   0,	 0,   0, {1,4}	,  20, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
{"& Short sword"		,0x00000000L,	TV_SWORD, '|',	/*398*/
    0,	  90,	28,   1,  80,	0,   0,	 0,   0, {1,7}	,  6, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},

/* ADD NEW ITEMS HERE!!!!!!!!! */

{"*Enlightenment*"		,0x00008000L,	TV_POTION2, '!',/*399*/
    0,	80000L,  94,   1,   4,	0,   0,	 0,   0, {1,1}	, 70, 4, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Detonations"		        ,0x00010000L,	TV_POTION2, '!',/*400*/
    0,  10000,  97,   1,   4,	0,   0,	 0,   0, {25,25}, 60, 6, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Death"                	,0x00020000L,	TV_POTION2, '!',/*401*/
    0,	12000,  109,   1,   4,	0,   0,	 0,   0, {30,30}, 55, 2, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Life"                 	,0x00040000L,	TV_POTION2, '!',/*402*/
    0,	 5000,  110,   1,   4,	0,   0,	 0,   0, {1,1}	, 60, 3, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Augmentation" 		,0x00080000L,	TV_POTION2, '!',/*403*/
    0,  60000L,  111,   1,   4,	0,   0,	 0,   0, {1,1}	, 40,12, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Ruination"    		,0x00100000L,	TV_POTION2, '!',/*404*/
    0,	    0,  112,   1,   4,	0,   0,	 0,   0, {1,1}	, 40,12, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Illumination" 		,RD_ILLUME,     TV_ROD, '-',	/*405*/
    0,	1000,  22,   1,  15,	0,   0,	 0,   0, {1,1}	, 20, 5, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Probing"			,RD_PROBE,	TV_ROD,	'-',	/*406*/
    0,	4000,  23,   1,  15,    0,   0,  0,   0, {1,1}  , 40, 4, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Probing"			,ST_PROBE,	TV_STAFF,  '_', /*407*/
    0,  2000,  30,   1,  50,    0,   0,  0,   0, {1,2}  , 30, 1, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Bronze Dragon Scale Mail"	,0x00100000L,  TV_HARD_ARMOR, '[',/*408*/
    0, 30000,  23,   1, 200,   -2,   0, 30,  10, {2,4} , 55,8, 0x00008800L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Gold Dragon Scale Mail"	,0x00100000L,  TV_HARD_ARMOR, '[',/*409*/
    0, 40000L,  24,   1, 200,   -2,   0, 30,  10, {2,4} , 65,8, 0x00010800L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"Recall"			,RD_RECALL,	TV_ROD, '-',	/*410*/
    0,  4000,  24,   1,  15,    0,   0,  0,   0, {1,1} , 30,  4, 0x800L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Cloak"		        ,0x00000000L,  TV_CLOAK,   '(', /*411*/
    0,     3,   1,   1,  10,    0,   0,  1,   0, {0,0},  20,  0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Acquirement"			,0x00000400L, TV_SCROLL2, '?', /*412*/
    0, 100000L,104,   1,   5,    0,   0,  0,   0, {0,0},  20,  10, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"*Acquirement*"		,0x00000800L, TV_SCROLL2, '?', /*413*/
    0, 200000L,105,   1,   5,    0,   0,  0,   0, {0,0},  60,  15, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Free Action"			,0x00800000L, TV_RING,  '=',  /*414*/
    0,   1500, 37,   1,   2,    0,   0,  0,   0, {0,0},  30,   0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Chaos Dragon Scale Mail"	,0x00100000L,  TV_HARD_ARMOR, '[',/*415*/
    0, 70000L,  25,   1, 200,   -2,   0, 30,  10, {2,4}, 75, 20, 0x00180800L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Law Dragon Scale Mail"	,0x00100000L,  TV_HARD_ARMOR, '[',/*416*/
    0, 80000L,  26,   1, 200,   -2,   0, 30,  10, {2,4}, 80, 20, 0x00210800L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Balance Dragon Scale Mail"    ,0x00100000L,  TV_HARD_ARMOR, '[',/*417*/
    0,100000L,  27,   1, 200,   -2,   0, 30,  10, {2,4}, 90, 20, 0x00390800L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Shining Dragon Scale Mail"    ,0x00100000L,  TV_HARD_ARMOR, '[',/*418*/
    0, 60000L,  28,   1, 200,   -2,   0, 30,  10, {2,4}, 65, 12, 0x00060800L
#ifdef TC_COLOR
  , YELLOW
#endif
},
{"Power Dragon Scale Mail"      ,0x12380000L,  TV_HARD_ARMOR, '[',/*419*/
    0,300000L,  29,   1, 200,   -3,   0, 40,  15, {2,4}, 110,  40, 0x017E0800L
#ifdef TC_COLOR
  , YELLOW
#endif
},
{"Enlightenment"		,0x00200000L,	TV_POTION2, '!',/*420*/
    0,	800,  113,   1,   4,	0,   0,	 0,   0, {1,1}	, 25, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Self Knowledge"               ,0x00400000L,   TV_POTION2, '!', /*421*/
    0, 2000,  114,   1,   4,     0,   0,  0,   0, {1,1},   40, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"*Healing*"			,0x00800000L,	TV_POTION2, '!',/*422*/
  200,	 1500,	115,   1,   4,	0,   0,	 0,   0, {1,1}	, 40, 3, 0L
#ifdef TC_COLOR
  ,0
#endif
},

/* end of Dungeon items */
/* Store items, which are not also dungeon items, some of these can be
   found above, except that the number is >1 below */

{"& Ration~ of Food"		,0x00000000L,	TV_FOOD, ',',	/*+0*/
 5000,	   3,	90,   5,  10,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Hard Biscuit~"		,0x00000000L,	TV_FOOD, ',',	/*+1*/
  500,	   1,	93,   5,   2,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Strip~ of Beef Jerky"	,0x00000000L,	TV_FOOD, ',',	/*+2*/
 1750,	   2,	94,   5,   2,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Pint~ of Fine Ale"		,0x00000000L,	TV_FOOD, ',',	/* +3 */
  500,	   1,	95,   3,  10,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"& Pint~ of Fine Wine"		,0x00000000L,	TV_FOOD, ',',
  400,	   2,	96,   1,  10,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTMAGENTA
#endif
},
{"& Pick"			,0x20000000L,	TV_DIGGING, '\\',
    1,	  50,	1,   1, 150,	0,   0,	 0,   0, {1,3}	,  0, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"& Shovel"			,0x20000000L,	TV_DIGGING, '\\',
    0,	  15,	4,   1,  60,	0,   0,	 0,   0, {1,2}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Identify"			,0x00000008L,	TV_SCROLL1, '?',
    0,	  50,  67,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Light"			,0x00000020L,	TV_SCROLL1, '?',
    0,	  15,  69,   3,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Phase Door"			,0x00000080L,	TV_SCROLL1, '?',
    0,	  15,  71,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Mapping"               	,0x00000800L,	TV_SCROLL1, '?',/* +10 */
    0,	  40,  75,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Treasure Detection"		,0x00004000L,	TV_SCROLL1, '?',
    0,	  15,  78,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Object Detection"		,0x00008000L,	TV_SCROLL1, '?',
    0,	  15,  79,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Detect Invisible"		,0x00080000L,	TV_SCROLL1, '?',
    0,	  15,  83,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Blessing"			,0x00000020L,	TV_SCROLL2, '?',
    0,	  15,  99,   2,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Word of Recall"		,0x00000100L,	TV_SCROLL2, '?',
    0,	 150, 102,   3,   5,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Cure Light Wounds"		,0x10001000L,	TV_POTION1, '!',
   50,	  15,  79,   2,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Heroism"			,0x00000010L,	TV_POTION2, '!',
    0,	  35,  98,   2,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Boldness"			,0x00000040L,	TV_POTION2, '!',
    0,	  10, 100,   2,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"Slow Poison"			,0x00000800L,	TV_POTION2, '!',
    0,	  25, 105,   2,   4,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,0
#endif
},
{"& Brass Lantern~"		,0x00000000L,	TV_LIGHT, '~',	/* +20 */
 7500,	  35,	0,   1,  50,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"& Wooden Torch~"		,0x00000000L,	TV_LIGHT, '~',
 4000,	   2, 192,   5,  30,	0,   0,	 0,   0, {1,1}	,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"& Flask~ of oil"		,0x00040000L,	TV_FLASK, '!',
 7500,	   3,	64,   5,  10,	0,   0,	 0,   0, {2,6}	,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
/* end store items */

/* start doors */
/* Secret door must have same subval as closed door in	*/
/* TRAP_LISTB.	See CHANGE_TRAP. Must use & because of stone_to_mud. */
{"& open door"			,0x00000000L, TV_OPEN_DOOR, '\'',
    0,	   0,	1,   1,   0,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& closed door"		,0x00000000L, TV_CLOSED_DOOR, '+',
    0,	   0,	19,   1,   0,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"& secret door"		,0x00000000L, TV_SECRET_DOOR, '#',
    0,	   0,	19,   1,   0,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* end doors */
/* stairs */
{"an up staircase"		,0x00000000L, TV_UP_STAIR, '<',
    0,	   0,	1,   1,   0,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"a down staircase"		,0x00000000L, TV_DOWN_STAIR, '>',
    0,	   0,	1,   1,   0,	0,   0,	 0,   0, {1,1}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* store door */
/* Stores are just special traps		*/
{"General Store"		,0x00000000L, TV_STORE_DOOR, '1',
    0,	   0, 101,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"Armoury"			,0x00000000L, TV_STORE_DOOR, '2',
    0,	   0, 102,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"Weapon Smiths"		,0x00000000L, TV_STORE_DOOR, '3', /* +30 */
    0,	   0, 103,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Temple"			,0x00000000L, TV_STORE_DOOR, '4',
    0,	   0, 104,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Alchemy Shop"			,0x00000000L, TV_STORE_DOOR, '5',
    0,	   0, 105,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,CYAN
#endif
},
{"Magic Shop"			,0x00000000L, TV_STORE_DOOR, '6',
    0,	   0, 106,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"Black Market"			,0x00000000L, TV_STORE_DOOR, '7',
    0,	   0, 107,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"Home"  			,0x00000000L, TV_STORE_DOOR, '8',
    0,	   0, 108,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTMAGENTA
#endif
},
/* end store door */
/* Traps are just Nasty treasures.				*/
/* Traps: Level represents the relative difficulty of disarming;	*/
/* and P1 represents the experienced gained when disarmed*/
{"an open pit"			,0x00000000L, TV_VIS_TRAP, ' ',	/* +36 */
    1,	   0,	1,   1,   0,	0,   0,	 0,   0, {2,6}	,50, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"an arrow trap"		,0x00000000L, TV_INVIS_TRAP, '^',
    3,	   0,	2,   1,   0,	0,   0,	 0,   0, {1,8}	,90, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"a covered pit"		,0x00000000L, TV_INVIS_TRAP, '^',
    2,	   0,	3,   1,   0,	0,   0,	 0,   0, {2,6}	,60, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"a trap door"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	4,   1,   0,	0,   0,	 0,   0, {2,8}	,75, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"a gas trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    3,	   0,	5,   1,   0,	0,   0,	 0,   0, {1,4}	,95, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a loose rock"			,0x00000000L, TV_INVIS_TRAP, ';',
    0,	   0,	6,   1,   0,	0,   0,	 0,   0, {0,0}	,10, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"a dart trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	7,   1,   0,	0,   0,	 0,   0, {1,4}	,110, 0, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"a strange rune"		,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	8,   1,   0,	0,   0,	 0,   0, {0,0}	,90, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"some loose rock"		,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	9,   1,   0,	0,   0,	 0,   0, {2,6}	,90, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"a gas trap"			,0x00000000L, TV_INVIS_TRAP, '^',
   10,	   0,	10,   1,   0,	0,   0,	 0,   0, {1,4}	,105, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a strange rune"		,0x00000000L, TV_INVIS_TRAP, '^', /* +46 */
    5,	   0,	11,   1,   0,	0,   0,	 0,   0, {0,0}	,90, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"a blackened spot"		,0x00000000L, TV_INVIS_TRAP, '^',
   10,	   0,	12,   1,   0,	0,   0,	 0,   0, {4,6}	,110, 0, 0L
#ifdef TC_COLOR
  ,LIGHTRED
#endif
},
{"some corroded rock"		,0x00000000L, TV_INVIS_TRAP, '^',
   10,	   0,	13,   1,   0,	0,   0,	 0,   0, {4,6}	,110, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGREEN
#endif
},
{"a gas trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	14,   1,   0,	0,   0,	 0,   0, {2,6}	,105, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a gas trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	15,   1,   0,	0,   0,	 0,   0, {1,4}	,110, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a gas trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	16,   1,   0,	0,   0,	 0,   0, {1,8}	,105, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a dart trap"			,0x00000000L, TV_INVIS_TRAP, '^',
    5,	   0,	17,   1,   0,	0,   0,	 0,   0, {1,8}	,110, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
{"a dart trap"			,0x00000000L, TV_INVIS_TRAP, '^',/* +53 */
    5,	   0,	18,   1,   0,	0,   0,	 0,   0, {1,8}	,110, 0, 0L
#ifdef TC_COLOR
  ,GREEN
#endif
},
/* rubble */
{"some rubble"			,0x00000000L, TV_RUBBLE, ':',	/* +54 */
    0,	   0,	1,   1,   0,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* mush */
{"& pint~ of fine grade mush"	,0x00000000L, TV_FOOD, ',',	/* +55 */
 3500,	   0,  97,   1,   1,   0,   0,   0,   0, {1,1}  ,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* Special trap	*/
{"a strange rune"		,0x00000000L, TV_VIS_TRAP, '^',	/* +56 */
    0,	   0,	99,   1,   0,	0,   0,	 0,   0, {0,0}	, 10, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* Gold list (All types of gold and gems are defined here)	*/
{"copper"			,0x00000000L, TV_GOLD, '$',	/* +57 */
    0,	   3,	1,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"copper"			,0x00000000L, TV_GOLD, '$',
    0,	   4,	2,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"copper"			,0x00000000L, TV_GOLD, '$',
    0,	   5,	3,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"silver"			,0x00000000L, TV_GOLD, '$',
    0,	   6,	4,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"silver"			,0x00000000L, TV_GOLD, '$',
    0,	   7,	5,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"silver"			,0x00000000L, TV_GOLD, '$',
    0,	   8,	6,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"garnets"			,0x00000000L, TV_GOLD, '*',
    0,	   9,	7,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"garnets"			,0x00000000L, TV_GOLD, '*',
    0,	  10,	8,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{"gold"				,0x00000000L, TV_GOLD, '$',
    0,	  12,	9,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"gold"				,0x00000000L, TV_GOLD, '$',      /* +66*/
    0,	  14,	10,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"gold"				,0x00000000L, TV_GOLD, '$',
    0,	  16,	11,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"opals"			,0x00000000L, TV_GOLD, '*',
    0,	  18,	12,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,DARKGRAY
#endif
},
{"sapphires"			,0x00000000L, TV_GOLD, '*',
    0,	  20,	13,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,BLUE
#endif
},
{"gold"				,0x00000000L, TV_GOLD, '$',
    0,	  24,	14,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,YELLOW
#endif
},
{"rubies"			,0x00000000L, TV_GOLD, '*',
    0,	  28,	15,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,RED
#endif
},
{"diamonds"			,0x00000000L, TV_GOLD, '*',
    0,	  32,	16,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"emeralds"			,0x00000000L, TV_GOLD, '*',
    0,	  40,	17,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGREEN
#endif
},
{"mithril"			,0x00000000L, TV_GOLD, '$',	/* +74 */
    0,	  80,	18,   1,   0,	0,   0,	 0,   0, {0,0}	  ,  1, 0, 0L
#ifdef TC_COLOR
  ,LIGHTCYAN
#endif
},
/* nothing, used as inventory place holder */
/* must be stackable, so that can be picked up by inven_carry */
{"nothing"			,0x00000000L,  TV_NOTHING, ' ',	/* +75 */
   0,       0,  64,   0,   0,   0,   0,   0,   0, {0,0}    ,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
/* these next two are needed only for the names */
{"& ruined chest"		,0x00000000L,   TV_CHEST, '~',	/* +76 */
   0,	   0,	0,   1, 250,	0,   0,	 0,   0, {0,0}	,  0, 0, 0L
#ifdef TC_COLOR
  ,BROWN
#endif
},
{""				,0x00000000L,  TV_NOTHING, ' ',	/* +77*/
   0,       0,   0,   0,   0,   0,   0,   0,   0, {0,0}  ,  0, 0, 0L
#ifdef TC_COLOR
  ,LIGHTGRAY
#endif
},
{"Power named Narya"		,0x01C008BFL,	TV_RING, '=',	/* +78 */
    1,	100000L,	31,   1,   2,	0,   0,	 0,   0, {0,0}	, 80, 0, 0x02000848L
#ifdef TC_COLOR
  ,0
#endif
},

/* For some reason THIS is where SPECIAL_OBJ should point */

{"Power named Nenya"		,0x05C008BFL,	TV_RING, '=',	/*467*/
    2,	200000L,	32,   1,   2,	0,   0,	 0,   0, {0,0}	, 90, 0, 0x02000888L
#ifdef TC_COLOR
  ,0
#endif
},
{"Power named Vilya"		,0x01C008BFL,	TV_RING, '=',	/*468*/
    3,	300000L,	33,   1,   2,	0,   0,	 0,   0, {0,0}	, 100, 0, 0x02000A08L
#ifdef TC_COLOR
  ,0
#endif
},
{"Power"			,0x83A80A3FL,	TV_RING, '=',	/*469*/
    5,5000000L,	34,   1,   2,	0,   0,	 0,   0, {0,0}	, 110, 0, 0x02000BC0L
#ifdef TC_COLOR
  ,0
#endif
},
{"The Phial of Galadriel"        ,0x00000000L,	TV_LIGHT, '~',	   /*470*/
    0,	10000,	4,    1,  10,	0,   0,	 0,   0, {1,1}	,  1, 0, 0x02000C00L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Ingwe"                         ,0x43380024L,  TV_AMULET, '"',    /*471*/
    3,  90000L,  10,   1,   3,   0,   0,  0,   0, {0,0}  ,  65, 0,0x02000800L
#ifdef TC_COLOR
  ,0
#endif
},
{"Carlammas"                     ,0x00080010L,  TV_AMULET, '"',    /*472*/
    2,  60000L,  11,   1,   3,   0,   0,  0,   0, {0,0}  ,  50, 0,0x02000800L
#ifdef TC_COLOR
  ,0
#endif
},
{"The Star of Elendil"		 ,0x00000000L,  TV_LIGHT,   '~',   /*473*/
    0,  25000,  6,    1,   5,   0,   0,  0,   0, {1,1}  ,  25, 0,0x02000C00L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"The Arkenstone of Thrain"	 ,0x0100000CL,  TV_LIGHT,   '~',   /*474*/
    2,  60000L,  8,    1,   5,   0,   0,  0,   0, {1,1}  ,  60, 0,0x02000C08L
#ifdef TC_COLOR
  ,WHITE
#endif
},
{"Tulkas"			 ,0x00000019L,  TV_RING, '=',     /*475*/
    4,  150000L, 35,   1,   2,   0,   0,  0,   0, {0,0}  ,  90, 0,0x02000800L
#ifdef TC_COLOR
  ,0
#endif
},
{"the Dwarves"			 ,0x41800811L,  TV_AMULET,  '"',  /*476*/
    3,  75000L,  12,   1,   3,   0,   0,  0,   0, {0,0},    70, 0,0x02000000L
#ifdef TC_COLOR
  ,0
#endif
},
{"Barahir"			 ,0x1000003FL,  TV_RING,   '=',   /*477*/
    1,  65000L,  36,   1,   2,   0,   0,  0,   0, {0,0},    50, 0,0x02000000L
#ifdef TC_COLOR
  ,0
#endif
}
};
#endif

char *special_names[SN_ARRAY_SIZE] = {
	NULL,			"of Resistance",	"of Resist Acid",
	"of Resist Fire",	"of Resist Cold",	"of Resist Lightning",
	"(Holy Avenger)",	"(Defender)",		"of Animal Slaying",
	"of Dragon Slaying",	"of Slay Evil",		"of Slay Undead",
	"of Flame",	        "of Frost",	        "of Free Action",
	"of Slaying",		"of Clumsiness",	"of Weakness",
	"of Slow Descent",	"of Speed",		"of Stealth",
	"of Slowness",		"of Noise",		"of Great Mass",
	"of Intelligence",	"of Wisdom",		"of Infra-Vision",
	"of Might",		"of Lordliness",	"of the Magi",
	"of Beauty",		"of Seeing",		"of Regeneration",
	"of Stupidity",		"of Dullness",		"of Blindness",
	"of Timidness",		"of Teleportation",	"of Ugliness",
	"of Protection",	"of Irritation",	"of Vulnerability",
	"of Enveloping",	"of Fire",		"of Slay Evil",
	"of Dragon Slaying",	"(Empty)",		"(Locked)",
	"(Poison Needle)",	"(Gas Trap)",		"(Explosion Device)",
	"(Summoning Runes)",	"(Multiple Traps)",	"(Disarmed)",
	"(Unlocked)",		"of Slay Animal",       "'Grond'",
	"'Ringil'",             "'Aeglos'",             "'Arunruth'",
        "'Mormegil'",       "of Morgul",            "'Angrist'",
	"'Gurthang'",       "'Calris'",         "of Accuracy",
	"'Anduril'",        "of Orc Slaying",       "of Power",
	"of Durin",             "of Aule",              "of Westernesse",
	"'Dor-Lomin'",      "of Demon Slaying",     "of Troll Slaying",
	"'Bloodspike'",     "'Thunderfist'",    "of Wounding",
	"'Orcrist'",        "'Glamdring'",      "'Sting'",
	"of Light",             "of Agility",           "of Backbiting",
	"'Doomcaller'",     "of Giant Slaying", 	"of Telepathy",
	"of Dragonkind",        "'Nenya'",		"'Narya'",
	"'Vilya'",		"of Aman",		"'Belegennon'",
	"of Feanor",		"of Anarion",		"of Isildur",
	"of Fingolfin",		"of Elvenkind",		"'Soulkeeper'",
	"of Dor-Lomin",         "of Morgoth",           "of Beleg Cuthalion",
	"'Dal-i-thalion'",  "'Paurhach'",       "'Paurnimmen'",
	"'Pauraegen'",      "'Cammithrim'",     "'Cambeleg'",
        "'Holhenneth'",     "'Paurnen'",        "'Aeglin'",
	"'Camlost'",        "'Nimloth'",        "'Nar-i-vagil'",
	"of Beruthiel",         "of Gorlim",            "'Narthanc'",
	"'Nimthanc'",       "'Dethanc'",        "'Gilettar'",
	"of Rilia",             "'Belangil'",       "of Balli Stonehand",
	"'Lotharang'",      "'Firestar'",       "'Eriril'",
	"'Cubragol'",       "of Bard",              "'Colluin'",
	"'Holcolleth'",     "'Totila'",         "of Pain",
	"'Elvagil'",        "'Aglarang'",	"of the Rohirrim",
	"of Eorlingas",     "'Barukkheled'",    "of Wrath",
	"'Haradekket'",     "'Mundwine'",       "'Gondricam'",
        "'Zarcuthra'",      "'Careth Asdriag'",	"'Forasgil'",
	"'Crisdurian'",     "'Colannon'",       "'Hithlomir'",
	"'Thalkettoth'",    "of Arvedui",     	"of Thranduil",
   	"of Thengel",           "of Hammerhand",     "of Celegorm",
  	"of Thror",             "of Maedhros",          "of Olorin",
 	"'Anguirel'",       "of Thorin",		"of Celeborn",
	"of Orome",		"of Eonwe",		"of Gondor",
        "of Theoden",		"of Thingol",		"of Thorongil",
        "of Luthien",           "of Tuor",              "of Ulmo",
	"'Osondir'",	"'Turmil'",		"of Caspanion",
	"'Til-i-arc'",	"'Deathwreaker'",	"'Avavir'",
	"'Taratol'",        "'Razorback'",      "'Bladeturner'",
	"(Shattered)",		"(Blasted)",
};

int16 sorted_objects[MAX_DUNGEON_OBJ];

/* Identified objects flags					*/
int8u object_ident[OBJECT_IDENT_SIZE];
int16 t_level[MAX_OBJ_LEVEL+1];
inven_type t_list[MAX_TALLOC];
inven_type inventory[INVEN_ARRAY_SIZE];

/* Treasure related values					*/
int16 inven_ctr = 0;		/* Total different obj's	*/
int16 inven_weight = 0;		/* Cur carried weight	*/
int16 equip_ctr = 0;		/* Cur equipment ctr	*/
int16 tcptr;			/* Cur treasure heap ptr	*/

