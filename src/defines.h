/* File: defines.h */

/* Purpose: global constants */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * Note to the Wizard.  Do *NOT* change these values unless you
 * know *exactly* what you are doing.  Changing some of these values
 * will GREATLY change game balance, and other changes will induce
 * crashes or memory errors or savefile mis-reads.  The comments
 * next to various defines are meant as reminders, NOT as complete
 * descriptions, and a constant may easily have unstated effects,
 * and may even be hard-coded due to related constants elsewhere.
 *
 * For example, there are MANY things that depend on the screen being
 * 80x24, with the top line used for messages, the bottom line being
 * used for status, and exactly 22 lines used to show the dungeon.
 * Just because your screen can hold 46 lines does not mean that the
 * game will work if you try to use 44 lines to show the dungeon.
 *
 * If you change anything below, without understanding EXACTLY how the
 * game uses the number, the program may stop working correctly.  Modify
 * the constants at your own risk.  Also note that not all constants in
 * the code are written using the proper symbolic constant.  Several hard
 * coded values are lurking around.
 */


/*
 * Note that versions 2.7.0 - 2.7.2 were a little "unreliable"
 * due to major code level changes between 2.6.2 to 2.7.3.
 * Luckily, this will only affect people who were helping to
 * test the new versions, and they have recovered by now.
 * Savefiles from 2.7.0 and 2.7.1 and 2.7.2 are thus "iffy".
 */


/*
 * Current version number of Angband: 2.7.8
 */
#define CUR_VERSION_MAJ 2
#define CUR_VERSION_MIN 7
#define CUR_PATCH_LEVEL 8


/*
 * Maximum range of various data types
 */
#define MAX_UCHAR       255
#define MAX_SHORT       32767
#define MAX_LONG        0xFFFFFFFFL


/*
 * Maximum Dungeon size
 */
#define MAX_HGT		66	/* Multiple of 11; >= 22 */
#define MAX_WID		198	/* Multiple of 33; >= 66 */


/*
 * Some Array Bounds
 */
#define MAX_STORES	8	/* Number of stores */
#define MAX_OWNERS	4	/* Number of possible owners per store */
#define MAX_RACES           10	/* Number of defined races                 */
#define MAX_CLASS            6	/* Number of defined classes               */
#define MAX_BACKGROUND     128	/* Number of types of histories for univ   */

/*
 * More Array Bounds
 */
#define MAX_K_IDX	512	/* Max number of object kinds */
#define MAX_R_IDX       549	/* Max number of monster races */
#define MAX_Q_IDX	4	/* Max number of quests */
#define MAX_I_IDX	400	/* Max objects per level */
#define MAX_M_IDX	600	/* Max monsters per level */



/*
 * Lower Limits of various kinds
 */
#define MIN_I_IDX           1   /* Minimum i_list index used                */
#define MIN_M_IDX           2   /* Minimum index in m_list (player is #1)   */

/*
 * Upper Limits of various kinds
 */
#define MAX_K_LEV         128   /* Maximum level of objects	*/
#define MAX_R_LEV         128   /* Maximum level of monsters	*/


/*
 * Store constants
 */
#define STORE_INVEN_MAX	24	/* Max number of discrete objs in inven */
#define STORE_CHOICES	30	/* Number of items to choose stock from */
#define STORE_OBJ_LEVEL	5	/* Magic Level for normal stores */
#define STORE_TURNOVER	9	/* Normal shop turnover, per day */
#define STORE_MIN_KEEP	6	/* Min slots to "always" keep full */
#define STORE_MAX_KEEP	18	/* Max slots to "always" keep full */
#define STORE_SHUFFLE	100	/* 1/Chance (per day) of an owner changing */
#define STORE_TURNS	1000	/* Number of turns between turnovers */


/*
 * Screen area used to show dungeon
 */
#define SCREEN_HGT	22	/* Multiple of 11; Divisor of MAX_WID */
#define SCREEN_WID	66	/* Multiple of 33; Divisor of MAX_HGT */


/*
 * Misc constants
 */
#define TOWN_DAWN	10000	/* Number of turns from dawn to dawn XXX */
#define GREAT_OBJ	20	/* 1/Chance of inflated item level */
#define NASTY_MON	50	/* 1/chance of inflated monster level */
#define BREAK_GLYPH	550	/* Rune of protection resistance */
#define BTH_PLUS_ADJ    3       /* Adjust BTH per plus-to-hit */
#define MAX_HISCORES	100	/* Maximum number of high scores */
#define MON_MULT_ADJ	8	/* High value slows multiplication */
#define MON_SUMMON_ADJ	2	/* Adjust level of summoned creatures */
#define MON_DRAIN_LIFE	2	/* Percent of player exp drained per hit */
#define USE_DEVICE      3	/* x> Harder devices x< Easier devices     */






/*
 * Magic Treasure Generation constants
 */
#define MAG_BASE_MAGIC   15     /* Base magic_chance                 */
#define MAG_BASE_MAX     70     /* Maximum magic_chance              */
#define MAG_STD_MIN       7     /* Minimum STD                       */
#define MAG_DIV_SPECIAL  40     /* 10*magic_chance/# special magic   */
#define MAG_DIV_CURSED   13     /* 10*magic_chance/# cursed items    */
#define MAG_MAX_LEVEL	100	/* Maximum "level" for "mbonus()" */


/*
 * Refueling constants
 */
#define FUEL_TORCH	 5000	/* Maximum amount of fuel a torch can have */
#define FUEL_LAMP	15000   /* Maximum amount that lamp can be filled  */


/*
 * More maximum values
 */
#define MAX_SIGHT	20	/* Maximum view distance */
#define MAX_RANGE	18	/* Maximum range (spells, etc) */



/*
 * Spontaneous monster generation
 */
#define MAX_M_ALLOC_CHANCE 160  /* 1/chance of new monster each round       */
#define MIN_M_ALLOC_LEVEL  14   /* Minimum number of monsters/level         */
#define MIN_M_ALLOC_TD      4   /* Number of people on town level (day)     */
#define MIN_M_ALLOC_TN      8   /* Number of people on town level (night)   */



/*
 * Player constants
 */
#define PY_MAX_EXP   99999999L	/* Maximum experience			*/
#define PY_MAX_GOLD 999999999L	/* Maximum gold				*/
#define PY_MAX_LEVEL	50	/* Maximum player level			*/
#define PY_FOOD_MAX	15000	/* Food value (Bloated)			*/
#define PY_FOOD_FULL	10000	/* Food value (Normal)			*/
#define PY_FOOD_ALERT	2000	/* Food value (Hungry)			*/
#define PY_FOOD_WEAK	1000	/* Food value (Weak)			*/
#define PY_FOOD_FAINT	300	/* Food value (Fainting)		*/
#define PY_REGEN_NORMAL 197	/* Regen factor*2^16 when full             */
#define PY_REGEN_WEAK   98	/* Regen factor*2^16 when weak             */
#define PY_REGEN_FAINT  33	/* Regen factor*2^16 when fainting         */
#define PY_REGEN_HPBASE 1442	/* Min amount hp regen*2^16               */
#define PY_REGEN_MNBASE 524	/* Min amount mana regen*2^16              */



/*
 * Maximum number of "normal" pack slots (22 or 23).
 *
 * This value is also used as the index of the "overflow" slot,
 * thus it must be LESS than "INVEN_WIELD".
 *
 * In 2.7.4, the pack was expanded to hold 24 items, though the player
 * is never allowed to actually hold 24 items, mainly because the item
 * selection screens only hold 23 items.  So the "24th slot" is used to
 * hold the "pack overflow" item, which is automatically "dropped".
 */
#define INVEN_PACK	23

/*
 * Hard-coded equipment slot indexes.
 */
#define INVEN_WIELD	24
#define INVEN_BOW       25
#define INVEN_LEFT      26
#define INVEN_RIGHT     27
#define INVEN_NECK      28
#define INVEN_LITE      29
#define INVEN_BODY      30
#define INVEN_OUTER     31
#define INVEN_ARM       32
#define INVEN_HEAD      33
#define INVEN_HANDS     34
#define INVEN_FEET      35

/*
 * Total number of inventory slots (24 pack, 12 equip)
 */
#define INVEN_TOTAL	36


/*
 * Hardcoded stat array indexes (do NOT change these)
 */
#define A_STR	0
#define A_INT	1
#define A_WIS	2
#define A_DEX	3
#define A_CON	4
#define A_CHR	5


/*
 * definitions for the psuedo-normal distribution generation
 */
#define NORMAL_TABLE_SIZE       256	/* Size of the table */
#define NORMAL_TABLE_SD          64	/* Standard Dev of the table */


/*
 * Bit flags for the special "p_ptr->notice" variable
 */
#define PN_HUNGRY	0x00000001L
#define PN_WEAK		0x00000002L
#define PN_BLIND	0x00000004L
#define PN_CONFUSED	0x00000008L
#define PN_FEAR		0x00000010L
#define PN_POISONED	0x00000020L
#define PN_FAST		0x00000040L
#define PN_SLOW		0x00000080L
#define PN_PARALYSED	0x00000100L
#define PN_IMAGE	0x00000200L
#define PN_TIM_INVIS	0x00000400L
#define PN_TIM_INFRA	0x00000800L
#define PN_HERO		0x00001000L
#define PN_SHERO	0x00002000L
#define PN_BLESSED	0x00004000L
#define PN_INVULN	0x00008000L
#define PN_OPP_ACID	0x00010000L
#define PN_OPP_ELEC	0x00020000L
#define PN_OPP_FIRE	0x00040000L
#define PN_OPP_COLD	0x00080000L
#define PN_OPP_POIS	0x00100000L


/*
 * Bit flags for the "p_ptr->update" variable
 */
#define PU_BONUS	0x00000001L	/* Calculate bonuses */
#define PU_HP		0x00000002L	/* Calculate chp and mhp */
#define PU_MANA		0x00000004L	/* Calculate csp and msp */
#define PU_SPELLS	0x00000008L	/* Calculate spells */
/* xxx (many) */
#define PU_VIEW		0x01000000L	/* Update view (if legal) */
#define PU_LITE		0x02000000L	/* Update lite (if legal) */
#define PU_FLOW		0x04000000L	/* Update flow (if legal) */
/* xxx */
#define PU_MONSTERS	0x10000000L	/* Update monsters (if legal) */
#define PU_DISTANCE	0x20000000L	/* Update distances (if legal) */
/* xxx */
/* xxx */


/*
 * Bit flags for the "p_ptr->redraw" variable
 */
#define PR_MISC		0x00000001L	/* Display Race/Class */
#define PR_TITLE	0x00000002L	/* Display Title */
#define PR_LEV		0x00000004L	/* Display Level */
#define PR_EXP		0x00000008L	/* Display Experience */
#define PR_STATS	0x00000010L	/* Display Stats */
#define PR_ARMOR	0x00000020L	/* Display Armor */
#define PR_HP		0x00000040L	/* Display Hitpoints */
#define PR_MANA		0x00000080L	/* Display Mana */
#define PR_GOLD		0x00000100L	/* Display Gold */
#define PR_DEPTH	0x00000200L	/* Display Depth */
#define PR_EQUIPPY	0x00000400L	/* Display Equippy Chars */
#define PR_HEALTH	0x00000800L	/* Display Health Bar */
#define PR_CUT		0x00001000L	/* Display Extra (Cut) */
#define PR_STUN		0x00002000L	/* Display Extra (Stun) */
#define PR_HUNGER	0x00004000L	/* Display Extra (Hunger) */
/* xxx */
#define PR_BLIND	0x00010000L	/* Display Extra (Blind) */
#define PR_CONFUSED	0x00020000L	/* Display Extra (Confused) */
#define PR_AFRAID	0x00040000L	/* Display Extra (Afraid) */
#define PR_POISONED	0x00080000L	/* Display Extra (Poisoned) */
#define PR_STATE	0x00100000L	/* Display Extra (State) */
#define PR_SPEED	0x00200000L	/* Display Extra (Speed) */
#define PR_STUDY	0x00400000L	/* Display Extra (Study) */
/* xxx */
#define PR_EXTRA	0x01000000L	/* Display Extra Info */
#define PR_BASIC	0x02000000L	/* Display Basic Info */
#define PR_MAP		0x04000000L	/* Display Map */
#define PR_CAVE		0x08000000L	/* Display Map + Basic + Extra */
/* xxx */
/* xxx */
#define PR_RECALL	0x40000000L	/* Display Recall Window */
#define PR_CHOICE	0x80000000L	/* Display Choice Window */






/*
 * Some screen locations for various display routines
 * Currently, row 8 and 15 are the only "blank" rows.
 * That leaves a "border" around the "stat" values.
 */

#define ROW_RACE	1
#define COL_RACE	0	/* <race name> */

#define ROW_CLASS	2
#define COL_CLASS	0	/* <class name> */

#define ROW_TITLE	3
#define COL_TITLE	0	/* <title> or <mode> */

#define ROW_LEVEL	4
#define COL_LEVEL	0	/* "LEVEL xxxxxx" */

#define ROW_EXP		5
#define COL_EXP		0	/* "EXP xxxxxxxx" */

#define ROW_GOLD	6
#define COL_GOLD	0	/* "AU xxxxxxxxx" */

#define ROW_STAT	8
#define COL_STAT	0	/* "xxx   xxxxxx" */

#define ROW_AC		15
#define COL_AC		0	/* "Cur AC xxxxx" */

#define ROW_MAXHP	16
#define COL_MAXHP	0	/* "Max HP xxxxx" */

#define ROW_CURHP	17
#define COL_CURHP	0	/* "Cur HP xxxxx" */

#define ROW_MAXSP	18
#define COL_MAXSP	0	/* "Max SP xxxxx" */

#define ROW_CURSP	19
#define COL_CURSP	0	/* "Cur SP xxxxx" */

#define ROW_INFO	20
#define COL_INFO	0	/* "xxxxxxxxxxxx" */

#define ROW_CUT		21
#define COL_CUT		0	/* <cut> */

#define ROW_STUN	22
#define COL_STUN	0	/* <stun> */

#define ROW_HUNGRY	23
#define COL_HUNGRY	0	/* "Weak" or "Hungry" or "Full" or "Gorged" */

#define ROW_BLIND	23
#define COL_BLIND	7	/* "Blind" */

#define ROW_CONFUSED	23
#define COL_CONFUSED	13	/* "Confused" */

#define ROW_AFRAID	23
#define COL_AFRAID	22	/* "Afraid" */

#define ROW_POISONED	23
#define COL_POISONED	29	/* "Poisoned" */

#define ROW_STATE	23
#define COL_STATE	38	/* <state> */

#define ROW_SPEED	23
#define COL_SPEED	49	/* "Slow (-NN)" or "Fast (+NN)" */

#define ROW_STUDY	23
#define COL_STUDY	64	/* "Study" */

#define ROW_DEPTH	23
#define COL_DEPTH	70	/* "Lev NNN" or "NNNN ft" (right justified!) */



/*
 * No stack can grow to "MAX_STACK_SIZE" items.
 * This should be a number between 50 and 256.
 * Actually, it should probably always be 100.
 */
#define MAX_STACK_SIZE			100


/*
 * Special "Item Flags"
 */
#define ID_SENSE	0x01	/* Item has been "sensed" */
#define ID_FIXED	0x02	/* Item has been "haggled" */
#define ID_EMPTY	0x04	/* Item charges are known */
#define ID_KNOWN	0x08	/* Item abilities are known */
#define ID_RUMOUR	0x10	/* Item background is known */
#define ID_MENTAL	0x20	/* Item information is known */
#define ID_CURSED	0x40	/* Item is temporarily cursed */
#define ID_BROKEN	0x80	/* Item is permanently worthless */


/*
 * Ego-Item indexes
 *
 * All the "Bad" Ego-Items are at the end.
 * The holes were left by artifacts and old ego-items.
 */

#define EGO_RESIST		1
#define EGO_RESIST_A		2
#define EGO_RESIST_F		3
#define EGO_RESIST_C		4
#define EGO_RESIST_E		5
#define EGO_HA			6
#define EGO_DF			7
#define EGO_SLAY_ANIMAL		8
#define EGO_SLAY_DRAGON		9
#define EGO_SLAY_EVIL		10
#define EGO_SLAY_UNDEAD		11
#define EGO_FT			12
#define EGO_FB			13
#define EGO_FREE_ACTION		14
#define EGO_SLAYING		15

#define EGO_SLOW_DESCENT	18
#define EGO_SPEED		19
#define EGO_STEALTH		20

#define EGO_INTELLIGENCE	24
#define EGO_WISDOM		25
#define EGO_INFRAVISION		26
#define EGO_MIGHT		27
#define EGO_LORDLINESS		28
#define EGO_MAGI		29
#define EGO_BEAUTY		30
#define EGO_SEEING		31
#define EGO_REGENERATION	32

#define EGO_ROBE_MAGI		38	/* (new) */
#define EGO_PROTECTION		39

#define EGO_FIRE		43	/* weapon/digger */
#define EGO_AMMO_EVIL		44
#define EGO_AMMO_DRAGON		45

#define EGO_AMMO_FIRE		50	/* new */
#define EGO_AMMO_SLAYING	52	/* new */

#define EGO_AMMO_ANIMAL		55

#define EGO_EXTRA_MIGHT		60	/* launcher */
#define EGO_EXTRA_SHOTS		61	/* launcher */

#define EGO_VELOCITY		64	/* launcher (new) */
#define EGO_ACCURACY		65	/* launcher */

#define EGO_SLAY_ORC		67	/* weapon */
#define EGO_POWER		68

#define EGO_WEST		71
#define EGO_BLESS_BLADE		72
#define EGO_SLAY_DEMON		73
#define EGO_SLAY_TROLL		74

#define EGO_AMMO_WOUNDING	77

#define EGO_LITE		81
#define EGO_AGILITY		82

#define EGO_SLAY_GIANT		85
#define EGO_TELEPATHY		86

#define EGO_ELVENKIND		87	/* Was 97 */

#define EGO_ATTACKS		90	/* Was 179 */

#define EGO_AMAN		91

#define EGO_MIN_WORTHLESS	96	/* First "worthless" Ego-Item */

#define EGO_WEAKNESS		104
#define EGO_STUPIDITY		105
#define EGO_DULLNESS		106
#define EGO_SICKLINESS		107
#define EGO_CLUMSINESS		108
#define EGO_UGLINESS		109
#define EGO_TELEPORTATION	110

#define EGO_IRRITATION		112
#define EGO_VULNERABILITY	113
#define EGO_ENVELOPING		114

#define EGO_SLOWNESS		116
#define EGO_NOISE		117
#define EGO_GREAT_MASS		118

#define EGO_BACKBITING		120

#define EGO_MORGUL		124

#define EGO_SHATTERED		126
#define EGO_BLASTED		127

#define EGO_MAX			128



/*
 * Artifact indexes
 */

        /* Lites */
#define ART_GALADRIEL		1
#define ART_ELENDIL		2
#define ART_THRAIN		3

        /* Amulets */
#define ART_CARLAMMAS		4
#define ART_INGWE		5
#define ART_DWARVES		6

        /* Rings */
#define ART_BARAHIR		8
#define ART_TULKAS		9
#define ART_NARYA		10
#define ART_NENYA		11
#define ART_VILYA		12
#define ART_POWER		13

        /* Dragon Scale */
#define ART_RAZORBACK		16
#define ART_BLADETURNER		17

        /* Hard Armour */
#define ART_SOULKEEPER		19
#define ART_ISILDUR		20
#define ART_ROHIRRIM		21
#define ART_BELEGENNON		22
#define ART_CELEBORN		23
#define ART_ARVEDUI		24
#define ART_CASPANION		25

        /* Soft Armour */
#define ART_HITHLOMIR		27
#define ART_THALKETTOTH		28

        /* Shields */
#define ART_THORIN		30
#define ART_CELEGORM		31
#define ART_ANARION		32

        /* Helms and Crowns */
#define ART_MORGOTH		34
#define ART_BERUTHIEL		35
#define ART_THRANDUIL		36
#define ART_THENGEL		37
#define ART_HAMMERHAND		38
#define ART_DOR			39
#define ART_HOLHENNETH		40
#define ART_GORLIM		41
#define ART_GONDOR		42

        /* Cloaks */
#define ART_COLLUIN		44
#define ART_HOLCOLLETH		45
#define ART_THINGOL		46
#define ART_THORONGIL		47
#define ART_COLANNON		48
#define ART_LUTHIEN		49
#define ART_TUOR		50

        /* Gloves */
#define ART_CAMBELEG		52
#define ART_CAMMITHRIM		53
#define ART_PAURHACH		54
#define ART_PAURNIMMEN		55
#define ART_PAURAEGEN		56
#define ART_PAURNEN		57
#define ART_CAMLOST		58
#define ART_FINGOLFIN		59

        /* Boots */
#define ART_FEANOR		60
#define ART_DAL			61
#define ART_THROR		62


        /* Swords */
#define ART_MAEDHROS		64
#define ART_ANGRIST		65
#define ART_NARTHANC		66
#define ART_NIMTHANC		67
#define ART_DETHANC		68
#define ART_RILIA		69
#define ART_BELANGIL		70
#define ART_CALRIS		71
#define ART_ARUNRUTH		72
#define ART_GLAMDRING		73
#define ART_AEGLIN		74
#define ART_ORCRIST		75
#define ART_GURTHANG		76
#define ART_ZARCUTHRA		77
#define ART_MORMEGIL		78
#define ART_GONDRICAM		79
#define ART_CRISDURIAN		80
#define ART_AGLARANG		81
#define ART_RINGIL		82
#define ART_ANDURIL		83
#define ART_ANGUIREL		84
#define ART_ELVAGIL		85
#define ART_FORASGIL		86
#define ART_CARETH		87
#define ART_STING		88
#define ART_HARADEKKET		89
#define ART_GILETTAR		90
#define ART_DOOMCALLER		91

        /* Polearms */
#define ART_THEODEN		93
#define ART_PAIN		94
#define ART_OSONDIR		95
#define ART_TIL			96
#define ART_AEGLOS		97
#define ART_OROME		98
#define ART_NIMLOTH		99
#define ART_EORLINGAS		100
#define ART_DURIN		101
#define ART_EONWE		102
#define ART_BALLI		103
#define ART_LOTHARANG		104
#define ART_MUNDWINE		105
#define ART_BARUKKHELED		106
#define ART_WRATH		107
#define ART_ULMO		108
#define ART_AVAVIR		109

        /* Hafted */
#define ART_GROND		111
#define ART_TOTILA		112
#define ART_THUNDERFIST		113
#define ART_BLOODSPIKE		114
#define ART_FIRESTAR		115
#define ART_TARATOL		116
#define ART_AULE		117
#define ART_NAR			118
#define ART_ERIRIL		119
#define ART_OLORIN		120
#define ART_DEATHWREAKER	121
#define ART_TURMIL		122

        /* Bows */
#define ART_BELTHRONDING	124
#define ART_BARD		125
#define ART_CUBRAGOL		126

        /* Number of entries */
#define ART_MAX			128

	/* First "normal" artifact */
#define ART_MIN_NORMAL		16


/*
 * Maximum size of the artifact table
 */
#define MAX_V_IDX		ART_MAX


/*
 * The values for the "tval" field of various objects.
 *
 * This value is the primary means by which items are sorted in the
 * player inventory.  It also groups things for MIN_WEAR/MAX_WEAR.
 *
 * Note that all items with tval from 10 to 50 are "wearable_p()",
 * which means that the special "TR#_*" flags apply to them.  Note
 * that shots/arrows/bolts have no slot to be wielded into, and that
 * bows are wielded into the special "bow slot".  This leaves the real
 * "weapon" slot for shovels, swords, polearms, and hafted weapons.
 *
 * Note that a "BOW" with tval = 19 and sval S = 10*N+P takes a missile
 * weapon with tval = 16+N, and does (xP) damage when so combined.
 *
 * Note that in 2.7.8 (hopefully), all items will use the same flags,
 * or rather, only wearable items will have any flags set (see below).
 */

#define TV_NOTHING       0	/* Nothing */
#define TV_SKELETON      1	/* Skeletons ('s') */
#define TV_BOTTLE	 2	/* Empty bottles ('!') */
#define TV_JUNK          3	/* Sticks, Pottery, etc ('~') */
#define TV_SPIKE         5	/* Spikes ('~') */
#define TV_CHEST         7	/* Chests ('~') */
#define TV_MIN_WEAR     10	/* Min tval for "wearable" items */
#define TV_SHOT		16	/* Ammo for slings */
#define TV_ARROW        17	/* Ammo for bows */
#define TV_BOLT         18	/* Ammo for x-bows */
#define TV_BOW          19	/* Slings/Bows/Xbows */
#define TV_DIGGING      20	/* Shovels/Picks */
#define TV_HAFTED       21	/* Priest Weapons */
#define TV_POLEARM      22	/* Axes and Pikes */
#define TV_SWORD        23	/* Edged Weapons */
#define TV_BOOTS        30	/* Boots */
#define TV_GLOVES       31	/* Gloves */
#define TV_HELM         32	/* Helms */
#define TV_CROWN        33	/* Crowns */
#define TV_SHIELD       34	/* Shields */
#define TV_CLOAK        35	/* Cloaks */
#define TV_SOFT_ARMOR   36	/* Soft Armor */
#define TV_HARD_ARMOR   37	/* Hard Armor */
#define TV_DRAG_ARMOR	38	/* Dragon Scale Mail */
#define TV_LITE         39	/* Lites (including Specials) */
#define TV_AMULET       40	/* Amulets (including Specials) */
#define TV_RING         45	/* Rings (including Specials) */
#define TV_MAX_WEAR     50	/* max tval for wearable items */
#define TV_STAFF        55
#define TV_WAND         65
#define TV_ROD          66
#define TV_SCROLL       70
#define TV_POTION       75
#define TV_FLASK        77
#define TV_FOOD         80
#define TV_MAGIC_BOOK   90
#define TV_PRAYER_BOOK  91
#define TV_MAX_OBJECT   99	/* This is the max TV monsters pick up */
#define TV_GOLD         100	/* Gold can only be picked up by players */
#define TV_MAX_PICK_UP  100     /* This is the max TV players pick up */
#define TV_INVIS_TRAP   101	/* Invisible traps -- see visible traps */
#define TV_MIN_VISIBLE  102	/* This is the first "visible landmark" */
#define TV_VIS_TRAP     102     /* Visible traps */
#define TV_OPEN_DOOR    104	/* Open doorway */
#define TV_UP_STAIR     107	/* Staircase up */
#define TV_DOWN_STAIR   108	/* Staircase down */
#define TV_STORE_DOOR   110	/* Entrance to store */
#define TV_MIN_BLOCK	112	/* This is the first "line of sight blocker" */
#define TV_SECRET_DOOR  117	/* Secret door -- treated as a "wall" */
#define TV_CLOSED_DOOR  118	/* Closed door -- treated as a "wall" */
#define TV_RUBBLE       119	/* Rubble pile -- treated as a "wall" */

/*
 * Some "fake" objects, used for "redefining" object pictures,
 * and for looking up those pictures in the object table.
 */
#define TV_HACK_FLOOR	101	/* Fake object -- floor XXX XXX */
#define TV_HACK_GRANITE	112	/* Fake object -- granite */
#define TV_HACK_QUARTZ	113	/* Fake object -- quartz */
#define TV_HACK_MAGMA	114	/* Fake object -- magma */

/*
 * A really fake object, used only for "redefining" the player picture,
 * and for looking up that picture in the object table.
 */
#define TV_HACK_PLAYER	109	/* Fake object -- player XXX XXX */


/* The "sval" codes for TV_SHOT/TV_ARROW/TV_BOLT */
#define SV_AMMO_LIGHT		0	/* pebbles */
#define SV_AMMO_NORMAL		1	/* shots, arrows, bolts */
#define SV_AMMO_HEAVY		2	/* seeker arrows and bolts */

/* The "sval" codes for TV_BOW (note information in "sval") */
#define SV_SLING		2	/* (x2) */
#define SV_SHORT_BOW		12	/* (x2) */
#define SV_LONG_BOW		13	/* (x3) */
#define SV_LIGHT_XBOW		23	/* (x3) */
#define SV_HEAVY_XBOW		24	/* (x4) */

/* The "sval" codes for TV_DIGGING */
#define SV_SHOVEL		1
#define SV_GNOMISH_SHOVEL	2
#define SV_DWARVEN_SHOVEL	3
#define SV_PICK			4
#define SV_ORCISH_PICK		5
#define SV_DWARVEN_PICK		6

/* The "sval" values for TV_HAFTED */
#define SV_WHIP			2	/* 1d6 */
#define SV_QUARTERSTAFF		3	/* 1d9 */
#define SV_MACE			5	/* 2d4 */
#define SV_BALL_AND_CHAIN	6	/* 2d4 */
#define SV_WAR_HAMMER		8	/* 3d3 */
#define SV_LUCERN_HAMMER	10	/* 2d5 */
#define SV_MORNING_STAR		12	/* 2d6 */
#define SV_FLAIL		13	/* 2d6 */
#define SV_LEAD_FILLED_MACE	15	/* 3d4 */
#define SV_TWO_HANDED_FLAIL	18	/* 3d6 */
#define SV_MACE_OF_DISRUPTION	20	/* 5d8 */
#define SV_GROND		50	/* 3d4 */

/* The "sval" values for TV_POLEARM */
#define SV_SPEAR		2	/* 1d6 */
#define SV_AWL_PIKE		4	/* 1d8 */
#define SV_TRIDENT		5	/* 1d9 */
#define SV_PIKE			8	/* 2d5 */
#define SV_BEAKED_AXE		10	/* 2d6 */
#define SV_BROAD_AXE		11	/* 2d6 */
#define SV_GLAIVE		13	/* 2d6 */
#define SV_HALBERD		15	/* 3d4 */
#define SV_SCYTHE		17	/* 5d3 */
#define SV_LANCE		20	/* 2d8 */
#define SV_BATTLE_AXE		22	/* 2d8 */
#define SV_GREAT_AXE		25	/* 4d4 */
#define SV_LOCHABER_AXE		28	/* 3d8 */
#define SV_SCYTHE_OF_SLICING	30	/* 8d4 */

/* The "sval" codes for TV_SWORD */
#define SV_BROKEN_DAGGER	1	/* 1d1 */
#define SV_BROKEN_SWORD		2	/* 1d2 */
#define SV_DAGGER		4	/* 1d4 */
#define SV_MAIN_GAUCHE		5	/* 1d5 */
#define SV_RAPIER		7	/* 1d6 */
#define SV_SMALL_SWORD		8	/* 1d6 */
#define SV_SHORT_SWORD		10	/* 1d7 */
#define SV_SABRE		11	/* 1d7 */
#define SV_CUTLASS		12	/* 1d7 */
#define SV_TULWAR		15	/* 2d4 */
#define SV_BROAD_SWORD		16	/* 2d5 */
#define SV_LONG_SWORD		17	/* 2d5 */
#define SV_SCIMITAR		18	/* 2d5 */
#define SV_KATANA		20	/* 3d4 */
#define SV_BASTARD_SWORD	21	/* 3d4 */
#define SV_TWO_HANDED_SWORD	25	/* 3d6 */
#define SV_EXECUTIONERS_SWORD	28	/* 4d5 */
#define SV_BLADE_OF_CHAOS	30	/* 6d5 */

/* The "sval" codes for TV_SHIELD */
#define SV_SMALL_LEATHER_SHIELD		2
#define SV_SMALL_METAL_SHIELD		3
#define SV_LARGE_LEATHER_SHIELD		4
#define SV_LARGE_METAL_SHIELD		5
#define SV_SHIELD_OF_DEFLECTION		10

/* The "sval" codes for TV_HELM */
#define SV_HARD_LEATHER_CAP		2
#define SV_METAL_CAP			3
#define SV_IRON_HELM			5
#define SV_STEEL_HELM			6
#define SV_IRON_CROWN			10
#define SV_GOLDEN_CROWN			11
#define SV_JEWELED_CROWN		12
#define SV_MORGOTH			50

/* The "sval" codes for TV_BOOTS */
#define SV_PAIR_OF_SOFT_LEATHER_BOOTS	2
#define SV_PAIR_OF_HARD_LEATHER_BOOTS	3
#define SV_PAIR_OF_METAL_SHOD_BOOTS	6

/* The "sval" codes for TV_CLOAK */
#define SV_CLOAK			1
#define SV_SHADOW_CLOAK			6

/* The "sval" codes for TV_GLOVES */
#define SV_SET_OF_LEATHER_GLOVES	1
#define SV_SET_OF_GAUNTLETS		2
#define SV_SET_OF_CESTI			5

/* The "sval" codes for TV_SOFT_ARMOR */
#define SV_FILTHY_RAG			1
#define SV_ROBE				2
#define SV_SOFT_LEATHER_ARMOR		4
#define SV_SOFT_STUDDED_LEATHER		5
#define SV_HARD_LEATHER_ARMOR		6
#define SV_HARD_STUDDED_LEATHER		7
#define SV_LEATHER_SCALE_MAIL		11

/* The "sval" codes for TV_HARD_ARMOR */
#define SV_RUSTY_CHAIN_MAIL		1	/* 14- */
#define SV_METAL_SCALE_MAIL		3	/* 13 */
#define SV_CHAIN_MAIL			4	/* 14 */
#define SV_AUGMENTED_CHAIN_MAIL		6	/* 16 */
#define SV_DOUBLE_CHAIN_MAIL		7	/* 16 */
#define SV_BAR_CHAIN_MAIL		8	/* 18 */
#define SV_METAL_BRIGANDINE_ARMOUR	9	/* 19 */
#define SV_PARTIAL_PLATE_ARMOUR		12	/* 22 */
#define SV_METAL_LAMELLAR_ARMOUR	13	/* 23 */
#define SV_FULL_PLATE_ARMOUR		15	/* 25 */
#define SV_RIBBED_PLATE_ARMOUR		18	/* 28 */
#define SV_MITHRIL_CHAIN_MAIL		20	/* 28+ */
#define SV_MITHRIL_PLATE_MAIL		25	/* 35+ */
#define SV_ADAMANTITE_PLATE_MAIL	30	/* 40+ */

/* The "sval" codes for TV_DRAG_ARMOR */
#define SV_DRAGON_BLACK			1
#define SV_DRAGON_BLUE			2
#define SV_DRAGON_WHITE			3
#define SV_DRAGON_RED			4
#define SV_DRAGON_GREEN			5
#define SV_DRAGON_MULTIHUED		6
#define SV_DRAGON_SHINING		10
#define SV_DRAGON_LAW			12
#define SV_DRAGON_BRONZE		14
#define SV_DRAGON_GOLD			16
#define SV_DRAGON_CHAOS			18
#define SV_DRAGON_BALANCE		20
#define SV_DRAGON_POWER			30

/* The sval codes for TV_LITE */
#define SV_LITE_TORCH		0
#define SV_LITE_LANTERN		1
#define SV_LITE_GALADRIEL	4
#define SV_LITE_ELENDIL		5
#define SV_LITE_THRAIN		6

/* The "sval" codes for TV_AMULET */
#define SV_AMULET_DOOM		0
#define SV_AMULET_TELEPORT	1
#define SV_AMULET_ADORNMENT	2
#define SV_AMULET_SLOW_DIGEST	3
#define SV_AMULET_RESIST_ACID	4
#define SV_AMULET_SEARCHING	5
#define SV_AMULET_WISDOM	6
#define SV_AMULET_CHARISMA	7
#define SV_AMULET_THE_MAGI	8
/* xxx */
#define SV_AMULET_CARLAMMAS	10
#define SV_AMULET_INGWE		11
#define SV_AMULET_DWARVES	12

/* The sval codes for TV_RING */
#define SV_RING_WOE		0
#define SV_RING_AGGRAVATION	1
#define SV_RING_WEAKNESS	2
#define SV_RING_STUPIDITY	3
#define SV_RING_TELEPORTATION	4
/* xxx */
#define SV_RING_SLOW_DIGESTION	6
#define SV_RING_FEATHER_FALL	7
#define SV_RING_RESIST_FIRE	8
#define SV_RING_RESIST_COLD	9
#define SV_RING_SUSTAIN_STR	10
#define SV_RING_SUSTAIN_INT	11
#define SV_RING_SUSTAIN_WIS	12
#define SV_RING_SUSTAIN_DEX	13
#define SV_RING_SUSTAIN_CON	14
#define SV_RING_SUSTAIN_CHR	15
#define SV_RING_PROTECTION	16
#define SV_RING_ACID		17
#define SV_RING_FLAMES		18
#define SV_RING_ICE		19
#define SV_RING_RESIST_POIS	20
#define SV_RING_FREE_ACTION	21
#define SV_RING_SEE_INVIS	22
#define SV_RING_SEARCHING	23
#define SV_RING_STR		24
#define SV_RING_INT		25
#define SV_RING_DEX		26
#define SV_RING_CON		27
#define SV_RING_ACCURACY	28
#define SV_RING_DAMAGE		29
#define SV_RING_SLAYING		30
#define SV_RING_SPEED		31
#define SV_RING_BARAHIR		32
#define SV_RING_TULKAS		33
#define SV_RING_NARYA		34
#define SV_RING_NENYA		35
#define SV_RING_VILYA		36
#define SV_RING_POWER		37




/* The "sval" codes for TV_STAFF */
#define SV_STAFF_DARKNESS	0
#define SV_STAFF_SLOWNESS	1
#define SV_STAFF_HASTE_MONSTERS	2
#define SV_STAFF_SUMMONING	3
#define SV_STAFF_TELEPORTATION	4
#define SV_STAFF_IDENTIFY	5
#define SV_STAFF_REMOVE_CURSE	6
#define SV_STAFF_STARLITE	7
#define SV_STAFF_LITE		8
#define SV_STAFF_MAPPING	9
#define SV_STAFF_DETECT_GOLD	10
#define SV_STAFF_DETECT_ITEM	11
#define SV_STAFF_DETECT_TRAP	12
#define SV_STAFF_DETECT_DOOR	13
#define SV_STAFF_DETECT_INVIS	14
#define SV_STAFF_DETECT_EVIL	15
#define SV_STAFF_CURE_LIGHT	16
#define SV_STAFF_CURING		17
#define SV_STAFF_HEALING	18
#define SV_STAFF_THE_MAGI	19
#define SV_STAFF_SLEEP_MONSTERS	20
#define SV_STAFF_SLOW_MONSTERS	21
#define SV_STAFF_SPEED		22
#define SV_STAFF_PROBING	23
#define SV_STAFF_DISPEL_EVIL	24
#define SV_STAFF_POWER		25
#define SV_STAFF_HOLINESS	26
#define SV_STAFF_GENOCIDE	27
#define SV_STAFF_EARTHQUAKES	28
#define SV_STAFF_DESTRUCTION	29


/* The "sval" codes for TV_WAND */
#define SV_WAND_HEAL_MONSTER	0
#define SV_WAND_HASTE_MONSTER	1
#define SV_WAND_CLONE_MONSTER	2
#define SV_WAND_TELEPORT_AWAY	3
#define SV_WAND_DISARMING	4
#define SV_WAND_TRAP_DOOR_DEST	5
#define SV_WAND_STONE_TO_MUD	6
#define SV_WAND_LITE		7
#define SV_WAND_SLEEP_MONSTER	8
#define SV_WAND_SLOW_MONSTER	9
#define SV_WAND_CONFUSE_MONSTER	10
#define SV_WAND_FEAR_MONSTER	11
#define SV_WAND_DRAIN_LIFE	12
#define SV_WAND_POLYMORPH	13
#define SV_WAND_STINKING_CLOUD	14
#define SV_WAND_MAGIC_MISSILE	15
#define SV_WAND_ACID_BOLT	16
#define SV_WAND_ELEC_BOLT	17
#define SV_WAND_FIRE_BOLT	18
#define SV_WAND_COLD_BOLT	19
#define SV_WAND_ACID_BALL	20
#define SV_WAND_ELEC_BALL	21
#define SV_WAND_FIRE_BALL	22
#define SV_WAND_COLD_BALL	23
#define SV_WAND_WONDER		24
#define SV_WAND_ANNIHILATION	25
#define SV_WAND_DRAGON_FIRE	26
#define SV_WAND_DRAGON_COLD	27
#define SV_WAND_DRAGON_BREATH	28

/* The "sval" codes for TV_ROD */
#define SV_ROD_DETECT_TRAP	0
#define SV_ROD_DETECT_DOOR	1
#define SV_ROD_IDENTIFY		2
#define SV_ROD_RECALL		3
#define SV_ROD_ILLUMINATION	4
#define SV_ROD_MAPPING		5
#define SV_ROD_DETECTION	6
#define SV_ROD_PROBING		7
#define SV_ROD_CURING		8
#define SV_ROD_HEALING		9
#define SV_ROD_RESTORATION	10
#define SV_ROD_SPEED		11
/* xxx (aimed) */
#define SV_ROD_TELEPORT_AWAY	13
#define SV_ROD_DISARMING	14
#define SV_ROD_LITE		15
#define SV_ROD_SLEEP_MONSTER	16
#define SV_ROD_SLOW_MONSTER	17
#define SV_ROD_DRAIN_LIFE	18
#define SV_ROD_POLYMORPH	19
#define SV_ROD_ACID_BOLT	20
#define SV_ROD_ELEC_BOLT	21
#define SV_ROD_FIRE_BOLT	22
#define SV_ROD_COLD_BOLT	23
#define SV_ROD_ACID_BALL	24
#define SV_ROD_ELEC_BALL	25
#define SV_ROD_FIRE_BALL	26
#define SV_ROD_COLD_BALL	27


/* The "sval" codes for TV_SCROLL */

#define SV_SCROLL_DARKNESS		0
#define SV_SCROLL_AGGRAVATE_MONSTER	1
#define SV_SCROLL_CURSE_ARMOR		2
#define SV_SCROLL_CURSE_WEAPON		3
#define SV_SCROLL_SUMMON_MONSTER	4
#define SV_SCROLL_SUMMON_UNDEAD		5
/* xxx (summon?) */
#define SV_SCROLL_TRAP_CREATION		7
#define SV_SCROLL_PHASE_DOOR		8
#define SV_SCROLL_TELEPORT		9
#define SV_SCROLL_TELEPORT_LEVEL	10
#define SV_SCROLL_WORD_OF_RECALL	11
#define SV_SCROLL_IDENTIFY		12
#define SV_SCROLL_STAR_IDENTIFY		13
#define SV_SCROLL_REMOVE_CURSE		14
#define SV_SCROLL_STAR_REMOVE_CURSE	15
#define SV_SCROLL_ENCHANT_ARMOR		16
#define SV_SCROLL_ENCHANT_WEAPON_TO_HIT	17
#define SV_SCROLL_ENCHANT_WEAPON_TO_DAM	18
/* xxx enchant missile? */
#define SV_SCROLL_STAR_ENCHANT_ARMOR	20
#define SV_SCROLL_STAR_ENCHANT_WEAPON	21
#define SV_SCROLL_RECHARGING		22
/* xxx */
#define SV_SCROLL_LIGHT			24
#define SV_SCROLL_MAPPING		25
#define SV_SCROLL_DETECT_GOLD		26
#define SV_SCROLL_DETECT_ITEM		27
#define SV_SCROLL_DETECT_TRAP		28
#define SV_SCROLL_DETECT_DOOR		29
#define SV_SCROLL_DETECT_INVIS		30
/* xxx (detect evil?) */
#define SV_SCROLL_SATISFY_HUNGER	32
#define SV_SCROLL_BLESSING		33
#define SV_SCROLL_HOLY_CHANT		34
#define SV_SCROLL_HOLY_PRAYER		35
#define SV_SCROLL_MONSTER_CONFUSION	36
#define SV_SCROLL_PROTECTION_FROM_EVIL	37
#define SV_SCROLL_RUNE_OF_PROTECTION	38
#define SV_SCROLL_TRAP_DOOR_DESTRUCTION	39
/* xxx */
#define SV_SCROLL_STAR_DESTRUCTION	41
#define SV_SCROLL_DISPEL_UNDEAD		42
/* xxx */
#define SV_SCROLL_GENOCIDE		44
#define SV_SCROLL_MASS_GENOCIDE		45
#define SV_SCROLL_ACQUIREMENT		46
#define SV_SCROLL_STAR_ACQUIREMENT	47

/* The "sval" codes for TV_POTION */
#define SV_POTION_WATER			0
#define SV_POTION_APPLE_JUICE		1
#define SV_POTION_SLIME_MOLD		2
/* xxx (fixed color) */
#define SV_POTION_SLOWNESS		4
#define SV_POTION_SALT_WATER		5
#define SV_POTION_POISON		6
#define SV_POTION_BLINDNESS		7
/* xxx */
#define SV_POTION_CONFUSION		9
/* xxx */
#define SV_POTION_SLEEP			11
/* xxx */
#define SV_POTION_LOSE_MEMORIES		13
/* xxx */
#define SV_POTION_RUINATION		15
#define SV_POTION_DEC_STR		16
#define SV_POTION_DEC_INT		17
#define SV_POTION_DEC_WIS		18
/* xxx DEC_DEX */
/* xxx DEC_CON */
#define SV_POTION_DEC_CHR		21
#define SV_POTION_DETONATIONS		22
#define SV_POTION_DEATH			23
#define SV_POTION_INFRAVISION		24
#define SV_POTION_DETECT_INVIS		25
#define SV_POTION_SLOW_POISON		26
#define SV_POTION_CURE_POISON		27
#define SV_POTION_BOLDNESS		28
#define SV_POTION_SPEED			29
#define SV_POTION_RESIST_HEAT		30
#define SV_POTION_RESIST_COLD		31
#define SV_POTION_HEROISM		32
#define SV_POTION_BESERK_STRENGTH	33
#define SV_POTION_CURE_LIGHT		34
#define SV_POTION_CURE_SERIOUS		35
#define SV_POTION_CURE_CRITICAL		36
#define SV_POTION_HEALING		37
#define SV_POTION_STAR_HEALING		38
#define SV_POTION_LIFE			39
#define SV_POTION_RESTORE_MANA		40
#define SV_POTION_RESTORE_EXP		41
#define SV_POTION_RES_STR		42
#define SV_POTION_RES_INT		43
#define SV_POTION_RES_WIS		44
#define SV_POTION_RES_DEX		45
#define SV_POTION_RES_CON		46
#define SV_POTION_RES_CHR		47
#define SV_POTION_INC_STR		48
#define SV_POTION_INC_INT		49
#define SV_POTION_INC_WIS		50
#define SV_POTION_INC_DEX		51
#define SV_POTION_INC_CON		52
#define SV_POTION_INC_CHR		53
/* xxx */
#define SV_POTION_AUGMENTATION		55
#define SV_POTION_ENLIGHTENMENT		56
#define SV_POTION_STAR_ENLIGHTENMENT	57
#define SV_POTION_SELF_KNOWLEDGE	58
#define SV_POTION_EXPERIENCE		59

/* The "sval" codes for TV_FOOD */
#define SV_FOOD_POISON		0
#define SV_FOOD_BLINDNESS	1
#define SV_FOOD_PARANOIA	2
#define SV_FOOD_CONFUSION	3
#define SV_FOOD_HALLUCINATION	4
#define SV_FOOD_PARALYSIS	5
#define SV_FOOD_WEAKNESS	6
#define SV_FOOD_SICKNESS	7
#define SV_FOOD_STUPIDITY	8
#define SV_FOOD_NAIVETY		9
#define SV_FOOD_UNHEALTH	10
#define SV_FOOD_DISEASE		11
#define SV_FOOD_CURE_POISON	12
#define SV_FOOD_CURE_BLINDNESS	13
#define SV_FOOD_CURE_PARANOIA	14
#define SV_FOOD_CURE_CONFUSION	15
#define SV_FOOD_CURE_SERIOUS	16
#define SV_FOOD_RESTORE_STR	17
#define SV_FOOD_RESTORE_CON	18
#define SV_FOOD_RESTORING	19
/* many missing mushrooms */
#define SV_FOOD_BISCUIT		32
#define SV_FOOD_JERKY		33
#define SV_FOOD_RATION		35
#define SV_FOOD_SLIME_MOLD	36
#define SV_FOOD_WAYBREAD	37
#define SV_FOOD_PINT_OF_ALE	38
#define SV_FOOD_PINT_OF_WINE	39

/* The "sval" codes for traps */
#define SV_TRAP_PIT		0
#define SV_TRAP_SPIKED_PIT	1
#define SV_TRAP_TRAP_DOOR	2
#define SV_TRAP_ARROW		3
#define SV_TRAP_DART_SLOW	4
#define SV_TRAP_DART_DEX	5
#define SV_TRAP_DART_STR	6
#define SV_TRAP_DART_CON	7
#define SV_TRAP_GAS_POISON	8
#define SV_TRAP_GAS_BLIND	9
#define SV_TRAP_GAS_CONFUSE	10
#define SV_TRAP_GAS_SLEEP	11
#define SV_TRAP_FIRE		12
#define SV_TRAP_ACID		13
#define SV_TRAP_TELEPORT	14
#define SV_TRAP_SUMMON		15
#define SV_TRAP_FALLING_ROCK	16
#define SV_TRAP_LOOSE_ROCK	17
/* xxx */
#define SV_TRAP_GLYPH           63


/*
 * Special "sval" limit -- first "normal" food
 */
#define SV_FOOD_MIN_FOOD	32

/*
 * Special "sval" limit -- first "aimed" rod
 */
#define SV_ROD_MIN_DIRECTION	12

/*
 * Special "sval" limit -- first "large" chest
 */
#define SV_CHEST_MIN_LARGE	4

/*
 * Special "sval" limit -- first "good" magic/prayer book
 */
#define SV_BOOK_MIN_GOOD	4


/*
 * The "TR_xxx" values apply ONLY to the items with tval's between
 * TV_MIN_WEAR and TV_MAX_WEAR, that is, items which can be wielded
 * or worn.  Use the macro "wearable_p()" to check this condition.
 *
 * As of 2.7.8, only "wearable" items may use the flag fields, but every
 * other item is guaranteed to have "zeros" in all three flag sets.
 *
 * Note that "flags1" contains all flags dependant on "pval" (including
 * stat bonuses, but NOT stat sustainers), plus all "extra attack damage"
 * flags (SLAY_XXX and BRAND_XXX).
 *
 * Note that "flags2" contains all "resistances" (including "Stat Sustainers",
 * actual immunities, and resistances).  Note that "Hold Life" is really an
 * "immunity" to ExpLoss, and "Free Action" is "immunity to paralysis".
 *
 * Note that "flags3" contains everything else -- including the three "CURSED"
 * flags, and the "BLESSED" flag, several "item display" parameters, some new
 * flags for powerful Bows, and flags which affect the player in a "general"
 * way (LITE, TELEPATHY, SEE_INVIS, SLOW_DIGEST, REGEN), including the "general"
 * curses (TELEPORT, AGGRAVATE, EXP_DRAIN).  It also contains four new flags
 * called "ITEM_IGNORE_XXX" which lets an item specify that it can not be
 * affected by various forms of destruction.  This is NOT as powerful as
 * actually granting resistance to the wearer.  Also "FEATHER" floating.
 */

#define TR1_STR			0x00000001L	/* Uses "pval" */
#define TR1_INT			0x00000002L	/* Uses "pval" */
#define TR1_WIS			0x00000004L	/* Uses "pval" */
#define TR1_DEX			0x00000008L	/* Uses "pval" */
#define TR1_CON			0x00000010L	/* Uses "pval" */
#define TR1_CHR			0x00000020L	/* Uses "pval" */
#define TR1_PVAL_XXX1		0x00000040L	/* Later */
#define TR1_PVAL_XXX2		0x00000080L	/* Later */
#define TR1_STEALTH		0x00000100L	/* Uses "pval" */
#define TR1_SEARCH		0x00000200L	/* Uses "pval" */
#define TR1_INFRA		0x00000400L	/* Uses "pval" */
#define TR1_TUNNEL		0x00000800L	/* Uses "pval" */
#define TR1_SPEED		0x00001000L	/* Uses "pval" */
#define TR1_BLOWS		0x00002000L	/* Uses "pval" */
#define TR1_PVAL_XXX3		0x00004000L
#define TR1_PVAL_XXX4		0x00008000L
#define TR1_SLAY_ANIMAL		0x00010000L
#define TR1_SLAY_EVIL		0x00020000L
#define TR1_SLAY_UNDEAD		0x00040000L
#define TR1_SLAY_DEMON		0x00080000L
#define TR1_SLAY_ORC		0x00100000L
#define TR1_SLAY_TROLL		0x00200000L
#define TR1_SLAY_GIANT		0x00400000L
#define TR1_SLAY_DRAGON		0x00800000L
#define TR1_KILL_DRAGON		0x01000000L	/* Execute Dragon */
#define TR1_XXX1		0x02000000L	/* Later */
#define TR1_IMPACT		0x04000000L	/* Start Earthquakes */
#define TR1_XXX2		0x08000000L	/* Later */
#define TR1_BRAND_ACID		0x10000000L
#define TR1_BRAND_ELEC		0x20000000L
#define TR1_BRAND_FIRE		0x40000000L
#define TR1_BRAND_COLD		0x80000000L

#define TR2_SUST_STR		0x00000001L
#define TR2_SUST_INT		0x00000002L
#define TR2_SUST_WIS		0x00000004L
#define TR2_SUST_DEX		0x00000008L
#define TR2_SUST_CON		0x00000010L
#define TR2_SUST_CHR		0x00000020L
#define TR2_XXX1		0x00000040L	/* Later */
#define TR2_XXX2		0x00000080L	/* Later */
#define TR2_IM_ACID		0x00000100L
#define TR2_IM_ELEC		0x00000200L
#define TR2_IM_FIRE		0x00000400L
#define TR2_IM_COLD		0x00000800L
#define TR2_IM_POIS		0x00001000L
#define TR2_IM_XXX1		0x00002000L	/* Later */
#define TR2_FREE_ACT		0x00004000L	/* Free Action */
#define TR2_HOLD_LIFE	 	0x00008000L	/* Hold Life */

#define TR2_RES_ACID		0x00010000L
#define TR2_RES_ELEC		0x00020000L
#define TR2_RES_FIRE		0x00040000L
#define TR2_RES_COLD		0x00080000L
#define TR2_RES_POIS		0x00100000L
#define TR2_RES_XXX1		0x00200000L	/* Later */
#define TR2_RES_LITE		0x00400000L
#define TR2_RES_DARK		0x00800000L

#define TR2_RES_BLIND		0x01000000L
#define TR2_RES_CONF		0x02000000L
#define TR2_RES_SOUND		0x04000000L
#define TR2_RES_SHARDS		0x08000000L

#define TR2_RES_NETHER		0x10000000L
#define TR2_RES_NEXUS		0x20000000L
#define TR2_RES_CHAOS		0x40000000L
#define TR2_RES_DISEN		0x80000000L


#define TR3_FLAG_XXX1		0x00000001L	/* Later */
#define TR3_FLAG_XXX2		0x00000002L	/* Later */
#define TR3_FLAG_XXX3		0x00000004L	/* Later */
#define TR3_FLAG_XXX4		0x00000008L	/* Later */
#define TR3_FLAG_XXX5		0x00000010L	/* Later */
#define TR3_FLAG_XXX6		0x00000020L	/* Later */
#define TR3_FLAG_XXX7		0x00000040L	/* Later */
#define TR3_FLAG_XXX8		0x00000080L	/* Later */
#define TR3_EASY_KNOW		0x00000100L	/* Aware -> Known */
#define TR3_HIDE_TYPE		0x00000200L	/* Change (+x to yyy) to (+x) */
#define TR3_SHOW_MODS		0x00000400L	/* Always show Tohit/Todam */
#define TR3_INSTA_ART		0x00000800L	/* This item MUST be an artifact */
#define TR3_FEATHER	 	0x00001000L	/* Feather Falling */
#define TR3_LITE		0x00002000L	/* Permanent Light */
#define TR3_SEE_INVIS		0x00004000L	/* See Invisible */
#define TR3_TELEPATHY		0x00008000L	/* Telepathy */
#define TR3_SLOW_DIGEST		0x00010000L	/* Item slows down digestion */
#define TR3_REGEN		0x00020000L	/* Item induces regeneration */
#define TR3_XTRA_MIGHT		0x00040000L	/* Bows get extra multiplier */
#define TR3_XTRA_SHOTS		0x00080000L	/* Bows get extra shots */
#define TR3_IGNORE_ACID		0x00100000L	/* Item ignores Acid Damage */
#define TR3_IGNORE_ELEC		0x00200000L	/* Item ignores Elec Damage */
#define TR3_IGNORE_FIRE		0x00400000L	/* Item ignores Fire Damage */
#define TR3_IGNORE_COLD		0x00800000L	/* Item ignores Cold Damage */
#define TR3_ACTIVATE		0x01000000L	/* Item can be activated */
#define TR3_DRAIN_EXP		0x02000000L	/* Item drains Experience */
#define TR3_TELEPORT		0x04000000L	/* Item teleports player */
#define TR3_AGGRAVATE		0x08000000L	/* Item aggravates monsters */
#define TR3_BLESSED		0x10000000L	/* Item is Blessed by the Gods */
#define TR3_CURSED		0x20000000L	/* Item is Cursed */
#define TR3_HEAVY_CURSE		0x40000000L	/* Item is Heavily Cursed */
#define TR3_PERMA_CURSE		0x80000000L	/* Item is Permanently Cursed */


/*
 * Hack -- flag set 1 -- mask for "pval-dependant" flags.
 * Note that all "pval" dependant flags must be in "flags1".
 */
#define TR1_PVAL_MASK	\
        (TR1_STR | TR1_INT | TR1_WIS | TR1_DEX | \
         TR1_CON | TR1_CHR | TR1_PVAL_XXX1 | TR1_PVAL_XXX2 | \
         TR1_STEALTH | TR1_SEARCH | TR1_INFRA | TR1_TUNNEL | \
         TR1_SPEED | TR1_BLOWS | TR1_PVAL_XXX3 | TR1_PVAL_XXX4)



/*
 * Chest trap flags (see "tables.c")
 */
#define CHEST_LOSE_STR		0x01
#define CHEST_LOSE_CON		0x02
#define CHEST_POISON		0x04
#define CHEST_PARALYZE		0x08
#define CHEST_EXPLODE		0x10
#define CHEST_SUMMON		0x20



/*
 * Legal restrictions for "summon_specific()"
 */
#define SUMMON_ANT		11
#define SUMMON_SPIDER		12
#define SUMMON_HOUND		13
#define SUMMON_REPTILE		14
#define SUMMON_ANGEL		15
#define SUMMON_DEMON		16
#define SUMMON_UNDEAD		17
#define SUMMON_DRAGON		18
#define SUMMON_HI_UNDEAD	21
#define SUMMON_HI_DRAGON	22
#define SUMMON_WRAITH		31
#define SUMMON_UNIQUE		32


/*
 * Spell types used by project(), and related functions.
 */
#define GF_ELEC         1
#define GF_POIS         2
#define GF_ACID         3
#define GF_COLD         4
#define GF_FIRE         5
#define GF_MISSILE      10
#define GF_ARROW        11
#define GF_PLASMA       12
#define GF_HOLY_ORB     13
#define GF_WATER        14
#define GF_LITE         15	/* Lite, plus Lite damage */
#define GF_DARK         16	/* Dark, plus Dark damage */
#define GF_LITE_WEAK	17	/* Lite, plus Lite damage if susceptible */
#define GF_DARK_WEAK	18	/* Dark, plus Dark damage if susceptible */
#define GF_SHARDS       20
#define GF_SOUND        21
#define GF_CONFUSION    22
#define GF_FORCE        23
#define GF_INERTIA      24
#define GF_MANA         26
#define GF_METEOR       27
#define GF_ICE          28
#define GF_CHAOS        30
#define GF_NETHER       31
#define GF_DISENCHANT   32
#define GF_NEXUS        33
#define GF_TIME         34
#define GF_GRAVITY      35
#define GF_KILL_WALL	40
#define GF_KILL_DOOR	41
#define GF_KILL_TRAP	42
#define GF_MAKE_WALL	45
#define GF_MAKE_DOOR	46
#define GF_MAKE_TRAP	47
#define GF_OLD_CLONE	50
#define GF_OLD_TPORT	51
#define GF_OLD_POLY	52
#define GF_OLD_HEAL	53
#define GF_OLD_SPEED	54
#define GF_OLD_SLOW	55
#define GF_OLD_CONF	56
#define GF_OLD_SCARE	57
#define GF_OLD_SLEEP	58
#define GF_OLD_DRAIN	59


/*
 * Bit flags for the "project()" function
 *
 *   BEAM: Work as a beam weapon -- affect every grid passed through
 *   HIDE: Do not "show" our progress (no visual feedback to player)
 *   STOP: Stop as soon as we hit a monster (used for "bolts")
 *   THRU: Continue "through" the targer (used for "bolts"/"beams")
 *   GRID: Affect each grid in the "blast area" in some way
 *   ITEM: Affect each item in the "blast area" in some way
 *   ONLY: Only affect the grid/item if there is no monster there
 *   XTRA: Do "extra" damage to player if he is in the "blast area"
 *
 * Notes:
 *   THRU, without STOP, means "go until we hit a wall or something"
 */
#define PROJECT_BEAM	0x01
#define PROJECT_HIDE	0x02
#define PROJECT_STOP	0x04
#define PROJECT_THRU	0x08
#define PROJECT_GRID	0x10
#define PROJECT_ITEM	0x20
#define PROJECT_ONLY	0x40
#define PROJECT_XTRA	0x80


/*
 * Bit flags for the "enchant()" function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOAC    0x04


/*
 * Some bit-flags for the "smart" field
 */
#define SM_RES_ACID	0x00000001
#define SM_RES_ELEC	0x00000002
#define SM_RES_FIRE	0x00000004
#define SM_RES_COLD	0x00000008
#define SM_RES_POIS	0x00000010
#define SM_RES_NETH	0x00000020
#define SM_RES_LITE	0x00000040
#define SM_RES_DARK	0x00000080
#define SM_RES_FEAR	0x00000100
#define SM_RES_CONF	0x00000200
#define SM_RES_CHAOS	0x00000400
#define SM_RES_DISEN	0x00000800
#define SM_RES_BLIND	0x00001000
#define SM_RES_NEXUS	0x00002000
#define SM_RES_SOUND	0x00004000
#define SM_RES_SHARD	0x00008000
#define SM_OPP_ACID	0x00010000
#define SM_OPP_ELEC	0x00020000
#define SM_OPP_FIRE	0x00040000
#define SM_OPP_COLD	0x00080000
#define SM_OPP_POIS	0x00100000
#define SM_OPP_XXX1	0x00200000
#define SM_OPP_XXX2	0x00400000
#define SM_OPP_XXX3	0x00800000
#define SM_IMM_ACID	0x01000000
#define SM_IMM_ELEC	0x02000000
#define SM_IMM_FIRE	0x04000000
#define SM_IMM_COLD	0x08000000
#define SM_IMM_POIS	0x10000000
#define SM_IMM_XXX4	0x20000000
#define SM_IMM_FREE	0x40000000
#define SM_IMM_MANA	0x80000000


/*
 * Some things which induce learning
 */
#define DRS_ACID	1
#define DRS_ELEC	2
#define DRS_FIRE	3
#define DRS_COLD	4
#define DRS_POIS	5
#define DRS_NETH	6
#define DRS_LITE	7
#define DRS_DARK	8
#define DRS_FEAR	9
#define DRS_CONF	10
#define DRS_CHAOS	11
#define DRS_DISEN	12
#define DRS_BLIND	13
#define DRS_NEXUS	14
#define DRS_SOUND	15
#define DRS_SHARD	16
#define DRS_FREE	30
#define DRS_MANA	31



/*
 * New monster blow methods
 */
#define RBM_HIT		1
#define RBM_TOUCH	2
#define RBM_PUNCH	3
#define RBM_KICK	4
#define RBM_CLAW	5
#define RBM_BITE	6
#define RBM_STING	7
#define RBM_XXX1	8
#define RBM_BUTT	9
#define RBM_CRUSH	10
#define RBM_ENGULF	11
#define RBM_XXX2	12
#define RBM_CRAWL	13
#define RBM_DROOL	14
#define RBM_SPIT	15
#define RBM_XXX3	16
#define RBM_GAZE	17
#define RBM_WAIL	18
#define RBM_SPORE	19
#define RBM_XXX4	20
#define RBM_BEG		21
#define RBM_INSULT	22
#define RBM_MOAN	23
#define RBM_XXX5	24


/*
 * New monster blow effects
 */
#define RBE_HURT	1
#define RBE_POISON	2
#define RBE_UN_BONUS	3
#define RBE_UN_POWER	4
#define RBE_EAT_GOLD	5
#define RBE_EAT_ITEM	6
#define RBE_EAT_FOOD	7
#define RBE_EAT_LITE	8
#define RBE_ACID	9
#define RBE_ELEC	10
#define RBE_FIRE	11
#define RBE_COLD	12
#define RBE_BLIND	13
#define RBE_CONFUSE	14
#define RBE_TERRIFY	15
#define RBE_PARALYZE	16
#define RBE_LOSE_STR	17
#define RBE_LOSE_INT	18
#define RBE_LOSE_WIS	19
#define RBE_LOSE_DEX	20
#define RBE_LOSE_CON	21
#define RBE_LOSE_CHR	22
#define RBE_LOSE_ALL	23
#define RBE_XXX1	24
#define RBE_EXP_10	25
#define RBE_EXP_20	26
#define RBE_EXP_40	27
#define RBE_EXP_80	28



/*
 * New monster race bit flags
 */
#define RF1_UNIQUE		0x00000001	/* Unique Monster */
#define RF1_QUESTOR		0x00000002	/* Quest Monster */
#define RF1_MALE		0x00000004	/* Male gender */
#define RF1_FEMALE		0x00000008	/* Female gender */
#define RF1_CHAR_CLEAR		0x00000010	/* Absorbs symbol */
#define RF1_CHAR_MULTI		0x00000020	/* Changes symbol */
#define RF1_ATTR_CLEAR		0x00000040	/* Absorbs color */
#define RF1_ATTR_MULTI		0x00000080	/* Changes color */
#define RF1_FORCE_DEPTH		0x00000100	/* Start at "correct" depth */
#define RF1_FORCE_MAXHP		0x00000200	/* Start with max hitpoints */
#define RF1_FORCE_SLEEP		0x00000400	/* Start out sleeping */
#define RF1_FORCE_EXTRA		0x00000800	/* Start out something */
#define RF1_FRIEND		0x00001000	/* Arrive with a friend */
#define RF1_FRIENDS		0x00002000	/* Arrive with some friends */
#define RF1_ESCORT		0x00004000	/* Arrive with an escort */
#define RF1_ESCORTS		0x00008000	/* Arrive with some escorts */
#define RF1_NEVER_BLOW		0x00010000	/* Never make physical blow */
#define RF1_NEVER_MOVE		0x00020000	/* Never make physical move */
#define RF1_RAND_25		0x00040000	/* Moves randomly (25%) */
#define RF1_RAND_50		0x00080000	/* Moves randomly (50%) */
#define RF1_ONLY_GOLD		0x00100000	/* Drop only gold */
#define RF1_ONLY_ITEM		0x00200000	/* Drop only items */
#define RF1_DROP_60		0x00400000	/* Drop an item/gold (60%) */
#define RF1_DROP_90		0x00800000	/* Drop an item/gold (90%) */
#define RF1_DROP_1D2		0x01000000	/* Drop 1d2 items/gold */
#define RF1_DROP_2D2		0x02000000	/* Drop 2d2 items/gold */
#define RF1_DROP_3D2		0x04000000	/* Drop 3d2 items/gold */
#define RF1_DROP_4D2		0x08000000	/* Drop 4d2 items/gold */
#define RF1_DROP_GOOD		0x10000000	/* Drop good items */
#define RF1_DROP_GREAT		0x20000000	/* Drop great items */
#define RF1_DROP_USEFUL		0x40000000	/* Drop "useful" items (unused) */
#define RF1_DROP_CHOSEN		0x80000000	/* Drop "chosen" items */

/*
 * New monster race bit flags
 */
#define RF2_STUPID		0x00000001	/* Monster is stupid */
#define RF2_SMART		0x00000002	/* Monster is smart */
#define RF2_XXX1X2		0x00000004	/* (?) */
#define RF2_XXX2X2		0x00000008	/* (?) */
#define RF2_INVISIBLE		0x00000010	/* Monster avoids vision */
#define RF2_COLD_BLOOD		0x00000020	/* Monster avoids infravision */
#define RF2_EMPTY_MIND		0x00000040	/* Monster avoids telepathy */
#define RF2_WEIRD_MIND		0x00000080	/* Monster avoids telepathy usually */
#define RF2_MULTIPLY		0x00000100	/* Monster reproduces */
#define RF2_REGENERATE		0x00000200	/* Monster regenerates */
#define RF2_XXX3X2		0x00000400	/* (?) */
#define RF2_XXX4X2		0x00000800	/* (?) */
#define RF2_POWERFUL		0x00001000	/* Monster has powerful breath */
#define RF2_XXX5X2		0x00002000	/* (?) */
#define RF2_DESTRUCT		0x00004000	/* Cause Earthquakes */
#define RF2_XXX6X2		0x00008000	/* (?) */
#define RF2_OPEN_DOOR		0x00010000	/* Monster can open doors */
#define RF2_BASH_DOOR		0x00020000	/* Monster can bash doors */
#define RF2_PASS_WALL		0x00040000	/* Monster can pass walls */
#define RF2_KILL_WALL		0x00080000	/* Monster can destroy walls */
#define RF2_MOVE_BODY		0x00100000	/* Monster can move other monsters */
#define RF2_KILL_BODY		0x00200000	/* Monster can kill other monsters */
#define RF2_TAKE_ITEM		0x00400000	/* Monster can pick up items */
#define RF2_KILL_ITEM		0x00800000	/* Monster can dissolve items */
#define RF2_BRAIN_1		0x01000000	
#define RF2_BRAIN_2		0x02000000	
#define RF2_BRAIN_3		0x04000000	
#define RF2_BRAIN_4		0x08000000	
#define RF2_BRAIN_5		0x10000000	
#define RF2_BRAIN_6		0x20000000	
#define RF2_BRAIN_7		0x40000000	
#define RF2_BRAIN_8		0x80000000	

/*
 * New monster race bit flags
 */
#define RF3_ORC			0x00000001	/* Orc */
#define RF3_TROLL		0x00000002	/* Troll */
#define RF3_GIANT		0x00000004	/* Giant */
#define RF3_DRAGON		0x00000008	/* Dragon */
#define RF3_DEMON		0x00000010	/* Demon */
#define RF3_UNDEAD		0x00000020	/* Undead */
#define RF3_EVIL		0x00000040	/* Evil */
#define RF3_ANIMAL		0x00000080	/* Animal */
#define RF3_XXX1X3		0x00000100	/* (?) */
#define RF3_XXX2X3		0x00000200	/* (?) */
#define RF3_XXX3X3		0x00000400	/* Non-Vocal (?) */
#define RF3_XXX4X3		0x00000800	/* Non-Living (?) */
#define RF3_HURT_LITE		0x00001000	/* Hurt by lite */
#define RF3_HURT_ROCK		0x00002000	/* Hurt by rock remover */
#define RF3_HURT_FIRE		0x00004000	/* Hurt badly by fire */
#define RF3_HURT_COLD		0x00008000	/* Hurt badly by cold */
#define RF3_IM_ACID		0x00010000	/* Resist acid a lot */
#define RF3_IM_ELEC		0x00020000	/* Resist elec a lot */
#define RF3_IM_FIRE		0x00040000	/* Resist fire a lot */
#define RF3_IM_COLD		0x00080000	/* Resist cold a lot */
#define RF3_IM_POIS		0x00100000	/* Resist poison a lot */
#define RF3_XXX5X3		0x00200000	/* Immune to (?) */
#define RF3_RES_NETH		0x00400000	/* Resist nether a lot */
#define RF3_RES_WATE		0x00800000	/* Resist water */
#define RF3_RES_PLAS		0x01000000	/* Resist plasma */
#define RF3_RES_NEXU		0x02000000	/* Resist nexus */
#define RF3_RES_DISE		0x04000000	/* Resist disenchantment */
#define RF3_XXX6X3		0x08000000	/* Resist (?) */
#define RF3_NO_FEAR		0x10000000	/* Cannot be scared */
#define RF3_NO_STUN		0x20000000	/* Cannot be stunned */
#define RF3_NO_CONF		0x40000000	/* Cannot be confused */
#define RF3_NO_SLEEP		0x80000000	/* Cannot be slept */

/*
 * New monster race bit flags
 */
#define RF4_SHRIEK		0x00000001	/* Shriek for help */
#define RF4_XXX2X4		0x00000002	/* (?) */
#define RF4_XXX3X4		0x00000004	/* (?) */
#define RF4_XXX4X4		0x00000008	/* (?) */
#define RF4_ARROW_1		0x00000010	/* Fire an arrow (light) */
#define RF4_ARROW_2		0x00000020	/* Fire an arrow (heavy) */
#define RF4_ARROW_3		0x00000040	/* Fire missiles (?) */
#define RF4_ARROW_4		0x00000080	/* Fire missiles (manticore) */
#define RF4_BR_ACID		0x00000100	/* Breathe Acid */
#define RF4_BR_ELEC		0x00000200	/* Breathe Elec */
#define RF4_BR_FIRE		0x00000400	/* Breathe Fire */
#define RF4_BR_COLD		0x00000800	/* Breathe Cold */
#define RF4_BR_POIS		0x00001000	/* Breathe Poison */
#define RF4_BR_NETH		0x00002000	/* Breathe Nether */
#define RF4_BR_LITE		0x00004000	/* Breathe Lite */
#define RF4_BR_DARK		0x00008000	/* Breathe Dark */
#define RF4_BR_CONF		0x00010000	/* Breathe Confusion */
#define RF4_BR_SOUN		0x00020000	/* Breathe Sound */
#define RF4_BR_CHAO		0x00040000	/* Breathe Chaos */
#define RF4_BR_DISE		0x00080000	/* Breathe Disenchant */
#define RF4_BR_NEXU		0x00100000	/* Breathe Nexus */
#define RF4_BR_TIME		0x00200000	/* Breathe Time */
#define RF4_BR_INER		0x00400000	/* Breathe Inertia */
#define RF4_BR_GRAV		0x00800000	/* Breathe Gravity */
#define RF4_BR_SHAR		0x01000000	/* Breathe Shards */
#define RF4_BR_PLAS		0x02000000	/* Breathe Plasma */
#define RF4_BR_WALL		0x04000000	/* Breathe Force */
#define RF4_BR_MANA		0x08000000	/* Breathe Mana */
#define RF4_XXX5X4		0x10000000	
#define RF4_XXX6X4		0x20000000	
#define RF4_XXX7X4		0x40000000	
#define RF4_XXX8X4		0x80000000	
 
/*
 * New monster race bit flags
 */
#define RF5_BA_ACID		0x00000001	/* Acid Ball */
#define RF5_BA_ELEC		0x00000002	/* Elec Ball */
#define RF5_BA_FIRE		0x00000004	/* Fire Ball */
#define RF5_BA_COLD		0x00000008	/* Cold Ball */
#define RF5_BA_POIS		0x00000010	/* Poison Ball */
#define RF5_BA_NETH		0x00000020	/* Nether Ball */
#define RF5_BA_WATE		0x00000040	/* Water Ball */
#define RF5_BA_MANA		0x00000080	/* Mana Storm */
#define RF5_BA_DARK		0x00000100	/* Darkness Storm */
#define RF5_DRAIN_MANA		0x00000200	/* Drain Mana */
#define RF5_MIND_BLAST		0x00000400	/* Blast Mind */
#define RF5_BRAIN_SMASH		0x00000800	/* Smash Brain */
#define RF5_CAUSE_1		0x00001000	/* Cause Light Wound */
#define RF5_CAUSE_2		0x00002000	/* Cause Serious Wound */
#define RF5_CAUSE_3		0x00004000	/* Cause Critical Wound */
#define RF5_CAUSE_4		0x00008000	/* Cause Mortal Wound (was RAZOR) */
#define RF5_BO_ACID		0x00010000	/* Acid Bolt */
#define RF5_BO_ELEC		0x00020000	/* Elec Bolt (unused) */
#define RF5_BO_FIRE		0x00040000	/* Fire Bolt */
#define RF5_BO_COLD		0x00080000	/* Cold Bolt */
#define RF5_BO_POIS		0x00100000	/* Poison Bolt (unused) */
#define RF5_BO_NETH		0x00200000	/* Nether Bolt */
#define RF5_BO_WATE		0x00400000	/* Water Bolt */
#define RF5_BO_MANA		0x00800000	/* Mana Bolt */
#define RF5_BO_PLAS		0x01000000	/* Plasma Bolt */
#define RF5_BO_ICEE		0x02000000	/* Ice Bolt */
#define RF5_MISSILE		0x04000000	/* Magic Missile */
#define RF5_SCARE		0x08000000	/* Frighten Player */
#define RF5_BLIND		0x10000000	/* Blind Player */
#define RF5_CONF		0x20000000	/* Confuse Player */
#define RF5_SLOW		0x40000000	/* Slow Player */
#define RF5_HOLD		0x80000000	/* Paralyze Player */

/*
 * New monster race bit flags
 */
#define RF6_HASTE		0x00000001	/* Speed self */
#define RF6_XXX1X6		0x00000002	/* Speed a lot (?) */
#define RF6_HEAL		0x00000004	/* Heal self */
#define RF6_XXX2X6		0x00000008	/* Heal a lot (?) */
#define RF6_BLINK		0x00000010	/* Teleport Short */
#define RF6_TPORT		0x00000020	/* Teleport Long */
#define RF6_XXX3X6		0x00000040	/* Move to Player (?) */
#define RF6_XXX4X6		0x00000080	/* Move to Monster (?) */
#define RF6_TELE_TO		0x00000100	/* Move player to monster */
#define RF6_TELE_AWAY		0x00000200	/* Move player far away */
#define RF6_TELE_LEVEL		0x00000400	/* Move player to other level */
#define RF6_XXX5		0x00000800	/* Move player (?) */
#define RF6_DARKNESS		0x00001000	/* Surround player with Darkness */
#define RF6_TRAPS		0x00002000	/* Surround player with Traps */
#define RF6_FORGET		0x00004000	/* Make player forget everything */
#define RF6_XXX6X6		0x00008000	/* Make player forget something? */
#define RF6_XXX7X6		0x00010000	/* Summon (?) */
#define RF6_XXX8X6		0x00020000	/* Summon (?) */
#define RF6_S_MONSTER		0x00040000	/* Summon Monster */
#define RF6_S_MONSTERS		0x00080000	/* Summon Monsters */
#define RF6_S_ANT		0x00100000	/* Summon Ant */
#define RF6_S_SPIDER		0x00200000	/* Summon Spider */
#define RF6_S_HOUND		0x00400000	/* Summon Hound */
#define RF6_S_REPTILE		0x00800000	/* Summon Reptile */
#define RF6_S_ANGEL		0x01000000	/* Summon Angel */
#define RF6_S_DEMON		0x02000000	/* Summon Demon */
#define RF6_S_UNDEAD		0x04000000	/* Summon Undead */
#define RF6_S_DRAGON		0x08000000	/* Summon Dragon */
#define RF6_S_HI_UNDEAD		0x10000000	/* Summon Greater Undead */
#define RF6_S_HI_DRAGON		0x20000000	/* Summon Ancient Dragon */
#define RF6_S_WRAITH		0x40000000	/* Summon Unique Wraith */
#define RF6_S_UNIQUE		0x80000000	/* Summon Unique Monster */



/*
 * Hack -- choose "intelligent" spells when desperate
 */

#define RF4_INT_MASK \
   0L
   
#define RF5_INT_MASK \
  (RF5_HOLD | RF5_SLOW | RF5_CONF | RF5_BLIND | RF5_SCARE)
  
#define RF6_INT_MASK \
   (RF6_BLINK |  RF6_TPORT | RF6_TELE_LEVEL | RF6_TELE_AWAY | \
    RF6_HEAL | RF6_HASTE | RF6_TRAPS | \
    RF6_S_MONSTER | RF6_S_MONSTERS | \
    RF6_S_ANT | RF6_S_SPIDER | RF6_S_HOUND | RF6_S_REPTILE | \
    RF6_S_ANGEL | RF6_S_DRAGON | RF6_S_UNDEAD | RF6_S_DEMON | \
    RF6_S_HI_DRAGON | RF6_S_HI_UNDEAD | RF6_S_WRAITH | RF6_S_UNIQUE)




/*
 * These values are determined from "k_list.txt".
 */

#define OBJ_PLAYER		440

#define OBJ_FLOOR		441
#define OBJ_GRANITE_WALL	442
#define OBJ_QUARTZ_VEIN		443
#define OBJ_MAGMA_VEIN		444

#define OBJ_RUBBLE              445
#define OBJ_OPEN_DOOR           446
#define OBJ_CLOSED_DOOR         447
#define OBJ_SECRET_DOOR         448
#define OBJ_UP_STAIR            449
#define OBJ_DOWN_STAIR          450


/*
 * The starting index for various sequences in "k_list.txt"
 */

#define OBJ_STORE_LIST          451	/* The first "store door" */
#define OBJ_TRAP_LIST           460	/* The first "trap" */
#define OBJ_GOLD_LIST           480	/* The first "gold" */


/*
 * The number of items in various sequences
 */
#define MAX_GOLD	18	/* Number of "gold" entries */
#define MAX_TRAP	18	/* Number of "trap" entries */




/*
 * New "cave grid" flags -- saved in savefile
 */
#define GRID_W_01	0x0001	/* Wall type (bit 1) */
#define GRID_W_02	0x0002	/* Wall type (bit 2) */
#define GRID_PERM	0x0004	/* Wall type is permanent */
#define GRID_QQQQ	0x0008	/* Unused */
#define GRID_MARK	0x0010	/* Grid is memorized */
#define GRID_GLOW	0x0020	/* Grid is illuminated */
#define GRID_ROOM	0x0040	/* Grid is part of a room */
#define GRID_ICKY	0x0080	/* Grid is anti-teleport */

/*
 * New "cave grid" flags -- temporary values
 */
#define GRID_SEEN	0x0100	/* Grid is in a special array */
#define GRID_LITE	0x0200	/* Grid is lit by torches */
#define GRID_VIEW	0x0400	/* Grid is in view of the player */
#define GRID_XTRA	0x0800	/* Grid is easily in view */
#define GRID_EXT1	0x1000	/* Temp #1 */
#define GRID_EXT2	0x2000	/* Temp #2 */
#define GRID_EXT3	0x4000	/* Temp #3 */
#define GRID_EXT4	0x8000	/* Temp #4 */

/*
 * Masks for the new grid types
 */
#define GRID_WALL_MASK	0x0003	/* Wall type */

/*
 * Legal results of GRID_WALL_MASK
 */
#define GRID_WALL_NONE		0x0000	/* No wall */
#define GRID_WALL_MAGMA		0x0001	/* Magma vein */
#define GRID_WALL_QUARTZ	0x0002	/* Quartz vein */
#define GRID_WALL_GRANITE	0x0003	/* Granite wall */






/*
 * And here are all of the global "macro-function" definitions
 */




/*
 * Determines if a map location is fully inside the outer walls
 */
#define in_bounds(Y,X) \
   (((Y) > 0) && ((X) > 0) && ((Y) < cur_hgt-1) && ((X) < cur_wid-1))

/*
 * Determines if a map location is on or inside the outer walls
 */
#define in_bounds2(Y,X) \
   (((Y) >= 0) && ((X) >= 0) && ((Y) < cur_hgt) && ((X) < cur_wid))


/*
 * Determine if a "legal" grid is a "floor" grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch rubble and closed/secret doors
 */
#define floor_grid_bold(Y,X) \
    (!(cave[Y][X].info & GRID_WALL_MASK) && \
     (i_list[cave[Y][X].i_idx].tval < TV_MIN_BLOCK))

/*
 * Determine if a "legal" grid is a "clean" floor grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all normal objects
 */
#define clean_grid_bold(Y,X) \
    (!(cave[Y][X].info & GRID_WALL_MASK) && \
     !(cave[Y][X].i_idx))

/*
 * Determine if a "legal" grid is an "empty" floor grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all normal monsters (and players)
 * Third test -- catch rubble and closed/secret doors
 */
#define empty_grid_bold(Y,X) \
    (!(cave[Y][X].info & GRID_WALL_MASK) && \
     !(cave[Y][X].m_idx) && \
     (i_list[cave[Y][X].i_idx].tval < TV_MIN_BLOCK))

/*
 * Determine if a "legal" grid is an "naked" floor grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all objects, monsters, and players
 */
#define naked_grid_bold(Y,X) \
    (!(cave[Y][X].info & GRID_WALL_MASK) && \
     !(cave[Y][X].m_idx) && !(cave[Y][X].i_idx))


/*
 * Determine if a "legal" grid is within "los" of the player
 */
#define player_has_los_bold(Y,X) \
    ((cave[Y][X].info & GRID_VIEW) != 0)



/*
 * Determine if a player "knows" about a grid
 * First test -- player has memorized the grid
 * Second test -- player can see the grid
 */
#define test_lite_bold(Y,X) \
    ((cave[Y][X].info & GRID_MARK) || \
     (player_can_see_bold(Y,X)))



/*
 * Is a given location "valid" for placing things?
 *
 * Note that "GRID_PERM" grids are never "valid".  This includes
 * permanent walls, permanent store doors, and permanent stairs.
 *
 * Hack -- a grid with an artifact in it is never valid.
 *
 * Note that granite, veins, doors, and rubble evaluate as "valid",
 * as do grids containing normal objects and/or monsters.
 *
 * This function is often "combined" with "floor_grid_bold(y,x)"
 * or one of the other similar "functions".
 */
#define valid_grid_bold(Y,X) \
    (!(cave[Y][X].info & GRID_PERM) && \
     !(artifact_p(&i_list[cave[Y][X].i_idx])))

/*
 * Same as above, but also require that the grid be "legal"
 */
#define valid_grid(Y,X) \
    (in_bounds(Y,X) && valid_grid_bold(Y,X))



/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(y,x)" always implies "in_bounds2(y,x)".
 */
#define panel_contains(y, x) \
  ((((y) >= panel_row_min) && ((y) <= panel_row_max) && \
    ((x) >= panel_col_min) && ((x) <= panel_col_max)) ? (TRUE) : (FALSE))


/*
 * Determine if a given inventory item is "aware"
 */
#define inven_aware_p(T) \
    (x_list[(T)->k_idx].aware)


/*
 * Determine if a given inventory item is "known"
 * Test One -- Check for special "known" tag
 * Test Two -- Check for "Easy Know" + "Aware"
 */
#define inven_known_p(T) \
    (((T)->ident & ID_KNOWN) || \
     (x_list[(T)->k_idx].easy_know && x_list[(T)->k_idx].aware))


/*
 * Return the "attr" for a given item.
 * Allow user redefinition of "aware" items.
  * Default to the "base" attr for unaware items
 */
#define inven_attr(T) \
    ((x_list[(T)->k_idx].aware) ? \
     (x_list[(T)->k_idx].x_attr) : \
     (x_list[(T)->k_idx].k_attr))

/*
 * Return the "char" for a given item.
 * Allow user redefinition of "aware" items.
 * Default to the "base" char for unaware items
 */
#define inven_char(T) \
    ((x_list[(T)->k_idx].aware) ? \
     (x_list[(T)->k_idx].x_char) : \
     (x_list[(T)->k_idx].k_char))




/*
 * Determine if a given object is "wearable".
 * This includes missiles, which are not really "wearable".
 * As of 2.7.8, this macro is no longer needed except to
 * parse old savefiles, and to understand the "pval" codes.
 */
#define wearable_p(T) \
        (((T)->tval >= TV_MIN_WEAR) && ((T)->tval <= TV_MAX_WEAR))


/*
 * Artifacts use the "name1" field
 */
#define artifact_p(T) \
        ((T)->name1 ? TRUE : FALSE)

/*
 * Ego-Items use the "name2" field
 */
#define ego_item_p(T) \
        ((T)->name2 ? TRUE : FALSE)


/*
 * Broken items.
 */
#define broken_p(T) \
        ((T)->ident & ID_BROKEN)

/*
 * Cursed items.
 */
#define cursed_p(T) \
        ((T)->ident & ID_CURSED)



/**** Available macros ****/

/*
 * Generates a random long integer X where O<=X<M.
 * The integer X falls along a uniform distribution.
 * For example, if M is 100, you get "percentile dice"
 */
#define rand_int(M) (random() % (M))

/*
 * Generates a random long integer X where A<=X<=B
 * The integer X falls along a uniform distribution.
 * Note: rand_range(0,N-1) == rand_int(N)
 */
#define rand_range(A,B) ((A) + (rand_int(1+(B)-(A))))

/*
 * Generate a random long integer X where A-D<=X<=A+D
 * The integer X falls along a uniform distribution.
 * Note: rand_spread(A,D) == rand_range(A-D,A+D)
 */
#define rand_spread(A,D) ((A) + (rand_int(1+(D)+(D))) - (D))


/*
 * Generate a random long integer X where 1<=X<=M
 * Also, "correctly" handle the case of M<=1
 */
#define randint(M) (((M) <= 1) ? (1) : (rand_int(M) + 1))


/*
 * Evaluate to TRUE "P" percent of the time
 */
#define magik(P)	(rand_int(100) < (P))





/*
 * Hack -- Prepare to use the "Secure" routines
 */
#if defined(SET_UID) && defined(SECURE)
  extern int PlayerUID;
# define getuid() PlayerUID
# define geteuid() PlayerUID
#endif


#ifdef WDT_TRACK_OPTIONS
# define MTB_DIRECT	0x01	/* Monster is Targetting something */
# define MTB_PLAYER	0x02	/* Monster is Targetting player */
#endif




/*
 * Hack -- some primitive sound support (see "main-win.c")
 * Some "sound" constants for "Term_xtra(TERM_XTRA_SOUND, val)"
 */
#define SOUND_HIT	1
#define SOUND_MISS	2
#define SOUND_FLEE	3
#define SOUND_DROP	4
#define SOUND_KILL	5
#define SOUND_LEVEL	6
#define SOUND_DEATH	7


