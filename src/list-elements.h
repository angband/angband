/*
 * File: src/list-elements.h
 * Purpose: Elements used in spells and other attacks.
 *
 * Fields:
 * name - text name 
 * desc - text description of attack
 * desc - text description of attack if blind
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * divisor - what to divide hp by to give breath damage
 * cap - breath damage cap
 * color - color of the effect
 * opp - timed flag for temporary resistance ("opposition")
 */

/* name  		desc				blind desc			num denom           divisor	cap		color				opp */
ELEM(ACID,		"acid",				"acid",				1,  RV(3,0,0,0),	3,		1600,	TERM_SLATE,		    TMD_OPP_ACID)
ELEM(ELEC,		"lightning",		"lightning",		1,  RV(3,0,0,0),	3,		1600,	TERM_BLUE,		    TMD_OPP_ELEC)
ELEM(FIRE,		"fire",				"fire",				1,  RV(3,0,0,0),	3,		1600,	TERM_RED,		    TMD_OPP_FIRE)
ELEM(COLD,		"cold",				"cold",				1,  RV(3,0,0,0),	3,		1600,	TERM_WHITE,		    TMD_OPP_COLD)
ELEM(POIS,		"poison",			"poison",			1,  RV(3,0,0,0),	3,		800,	TERM_GREEN,		    TMD_OPP_POIS)
ELEM(LIGHT,		"light",			"something",		4,  RV(6,1,6,0),	6,		400,	TERM_ORANGE,	    0)
ELEM(DARK,		"dark",				"something",		4,  RV(6,1,6,0),	6,		400,	TERM_L_DARK,	    0)
ELEM(SOUND,		"sound",			"noise",			5,  RV(6,1,6,0),	6,		500,	TERM_YELLOW,	    0)
ELEM(SHARD,		"shards",			"something sharp",	6,  RV(6,1,6,0),	6,		500,	TERM_UMBER,		    0)
ELEM(NEXUS,		"nexus",			"something strange",6,  RV(6,1,6,0),	6,		500,	TERM_L_RED,		    0)
ELEM(NETHER,	"nether",			"something cold",	6,  RV(6,1,6,0),	6,		550,	TERM_L_GREEN,	    0)
ELEM(CHAOS,		"chaos",			"something strange",6,	RV(6,1,6,0),	6,		500,	TERM_VIOLET,	 	0)
ELEM(DISEN,		"disenchantment",	"something strange",6,  RV(6,1,6,0),	6,		500,	TERM_VIOLET,	    0)
ELEM(WATER,		"water",			"water",			0,	RV(0,0,0,0),	6,		0,		TERM_SLATE,			0)
ELEM(ICE,		"ice",				"something sharp",	1,  RV(3,0,0,0),	6,		0,		TERM_WHITE,		    TMD_OPP_COLD)
ELEM(GRAVITY,	"gravity",			"something strange",0,	RV(0,0,0,0),	3,		200,	TERM_L_WHITE,		0)
ELEM(INERTIA,	"inertia",			"something strange",0,	RV(0,0,0,0),	6,		200,	TERM_L_WHITE,		0)
ELEM(FORCE,		"force",			"something hard",	0,	RV(0,0,0,0),	6,		200,	TERM_UMBER,			0)
ELEM(TIME,		"time",				"something strange",0,	RV(0,0,0,0),	3,		150,	TERM_L_BLUE,		0)
ELEM(PLASMA,	"plasma",			"something",		0,	RV(0,0,0,0),	6,		150,	TERM_RED,			0)
ELEM(METEOR,	"a meteor",			"something",		0,	RV(0,0,0,0),	6,		0,		TERM_RED,			0)
ELEM(MISSILE,	"a missile",		"something",		0,	RV(0,0,0,0),	6,		0,		TERM_VIOLET,		0)
ELEM(MANA,		"mana",				"something",		0,	RV(0,0,0,0),	3,		1600,	TERM_L_DARK,		0)
ELEM(HOLY_ORB,	"a holy orb",		"something",		0,	RV(0,0,0,0),	6,		0,		TERM_L_DARK,		0)
ELEM(ARROW,		"an arrow",			"something sharp",	0,	RV(0,0,0,0),	6,		0,		TERM_WHITE,			0)
