/**
 * \file list-elements.h
 * \brief Elements used in spells and other attacks.
 *
 * Fields:
 * name - text name 
 * desc - text description of attack
 * player desc - text description of player attack
 * blind desc - text description of attack if blind
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * divisor - what to divide hp by to give breath damage
 * cap - breath damage cap
 * message type message type of attack
 * color - color of the effect
 */

/* name  		desc				player desc			blind desc			num denom           divisor	cap		message type	color */
ELEM(ACID,		"acid",				"acid",				"acid",				1,  RV(3,0,0,0),	3,		1600,	MSG_BR_ACID,	COLOUR_SLATE)
ELEM(ELEC,		"lightning",		"lightning",		"lightning",		1,  RV(3,0,0,0),	3,		1600,	MSG_BR_ELEC,	COLOUR_BLUE)
ELEM(FIRE,		"fire",				"fire",				"fire",				1,  RV(3,0,0,0),	3,		1600,	MSG_BR_FIRE,	COLOUR_RED)
ELEM(COLD,		"cold",				"frost",			"cold",				1,  RV(3,0,0,0),	3,		1600,	MSG_BR_FROST,	COLOUR_WHITE)
ELEM(POIS,		"poison",			"poison gas",		"poison",			1,  RV(3,0,0,0),	3,		800,	MSG_BR_GAS,		COLOUR_GREEN)
ELEM(LIGHT,		"light",			"light",			"something",		4,  RV(6,1,6,0),	6,		400,	MSG_BR_LIGHT,	COLOUR_ORANGE)
ELEM(DARK,		"dark",				"dark",				"something",		4,  RV(6,1,6,0),	6,		400,	MSG_BR_DARK,	COLOUR_L_DARK)
ELEM(SOUND,		"sound",			"sound",			"noise",			5,  RV(6,1,6,0),	6,		500,	MSG_BR_SOUND,	COLOUR_YELLOW)
ELEM(SHARD,		"shards",			"shards",			"something sharp",	6,  RV(6,1,6,0),	6,		500,	MSG_BR_SHARDS,	COLOUR_UMBER)
ELEM(NEXUS,		"nexus",			"nexus",			"something strange",6,  RV(6,1,6,0),	6,		400,	MSG_BR_NEXUS,	COLOUR_L_RED)
ELEM(NETHER,	"nether",			"nether",			"something cold",	6,  RV(6,1,6,0),	6,		550,	MSG_BR_NETHER,	COLOUR_L_GREEN)
ELEM(CHAOS,		"chaos",			"chaos",			"something strange",6,	RV(6,1,6,0),	6,		500,	MSG_BR_CHAOS,	COLOUR_VIOLET)
ELEM(DISEN,		"disenchantment",	"disenchantment",	"something strange",6,  RV(6,1,6,0),	6,		500,	MSG_BR_DISEN,	COLOUR_VIOLET)
ELEM(WATER,		"water",			"water",			"water",			0,	RV(0,0,0,0),	6,		0,		0,				COLOUR_SLATE)
ELEM(ICE,		"ice",				"ice",				"something sharp",	1,  RV(3,0,0,0),	6,		0,		0,				COLOUR_WHITE)
ELEM(GRAVITY,	"gravity",			"gravity",			"something strange",0,	RV(0,0,0,0),	3,		200,	MSG_BR_GRAVITY,	COLOUR_L_WHITE)
ELEM(INERTIA,	"inertia",			"inertia",			"something strange",0,	RV(0,0,0,0),	6,		200,	MSG_BR_INERTIA,	COLOUR_L_WHITE)
ELEM(FORCE,		"force",			"force",			"something hard",	0,	RV(0,0,0,0),	6,		200,	MSG_BR_FORCE,	COLOUR_UMBER)
ELEM(TIME,		"time",				"time",				"something strange",0,	RV(0,0,0,0),	3,		150,	MSG_BR_TIME,	COLOUR_L_BLUE)
ELEM(PLASMA,	"plasma",			"plasma",			"something",		0,	RV(0,0,0,0),	6,		150,	MSG_BR_PLASMA,	COLOUR_RED)
ELEM(METEOR,	"a meteor",			"meteor",			"something",		0,	RV(0,0,0,0),	6,		0,		0,				COLOUR_RED)
ELEM(MISSILE,	"a missile",		"magical energy",	"something",		0,	RV(0,0,0,0),	6,		0,		0,				COLOUR_VIOLET)
ELEM(MANA,		"mana",				"the elements",		"something",		0,	RV(0,0,0,0),	3,		1600,	MSG_BR_ELEMENTS,COLOUR_L_DARK)
ELEM(HOLY_ORB,	"a holy orb",		"holy power",		"something",		0,	RV(0,0,0,0),	6,		0,		0,				COLOUR_L_DARK)
ELEM(ARROW,		"an arrow",			"arrows",			"something sharp",	0,	RV(0,0,0,0),	6,		0,		0,				COLOUR_WHITE)
