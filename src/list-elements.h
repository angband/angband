/*
 * File: src/list-elements.h
 * Purpose: Elements used in spells and other attacks.
 *
 * Fields:
 * index - type index 
 * name - text name 
 * desc - text description of attack if blind
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * divisor - what to divide hp by to give breath damage
 * cap - breath damage cap
 * color - color of the effect
 * opp - timed flag for temporary resistance ("opposition")
 * feature handler - handler affecting the environment
 * object handler - handler affecting an object
 * monster handler - handler affecting a monster
 * player handler - handler effecting the player
 */

/* name  		desc				blind desc			num denom           divisor	cap		color				opp             object handler	monster handler		player handler*/
ELEM(ACID,		"acid",				"acid",				1,  RV(3,0,0,0),	3,		1600,	TERM_SLATE,		    TMD_OPP_ACID,   OH(ACID),		MH(ACID),			PH(ACID))
ELEM(ELEC,		"lightning",		"lightning",		1,  RV(3,0,0,0),	3,		1600,	TERM_BLUE,		    TMD_OPP_ELEC,   OH(ELEC),		MH(ELEC),			PH(ELEC))
ELEM(FIRE,		"fire",				"fire",				1,  RV(3,0,0,0),	3,		1600,	TERM_RED,		    TMD_OPP_FIRE,   OH(FIRE),		MH(FIRE),			PH(FIRE))
ELEM(COLD,		"cold",				"cold",				1,  RV(3,0,0,0),	3,		1600,	TERM_WHITE,		    TMD_OPP_COLD,   OH(COLD),		MH(COLD),			PH(COLD))
ELEM(POIS,		"poison",			"poison",			1,  RV(3,0,0,0),	3,		800,	TERM_GREEN,		    TMD_OPP_POIS,   NULL,			MH(POIS),			PH(POIS))
ELEM(LIGHT,		"light",			"something",		4,  RV(6,1,6,0),	6,		400,	TERM_ORANGE,	    0,              NULL,			MH(LIGHT),			PH(LIGHT))
ELEM(DARK,		"dark",				"something",		4,  RV(6,1,6,0),	6,		400,	TERM_L_DARK,	    0,              NULL,			MH(DARK),			PH(DARK))
ELEM(SOUND,		"sound",			"noise",			5,  RV(6,1,6,0),	6,		500,	TERM_YELLOW,	    0,              OH(shatter),	MH(SOUND),			PH(SOUND))
ELEM(SHARD,		"shards",			"something sharp",	6,  RV(6,1,6,0),	6,		500,	TERM_UMBER,		    0,              OH(shatter),	MH(SHARD),			PH(SHARD))
ELEM(NEXUS,		"nexus",			"something strange",6,  RV(6,1,6,0),	6,		500,	TERM_L_RED,		    0,              NULL,			MH(NEXUS),			PH(NEXUS))
ELEM(NETHER,	"nether",			"something cold",	6,  RV(6,1,6,0),	6,		550,	TERM_L_GREEN,	    0,              NULL,			MH(NETHER),			PH(NETHER))
ELEM(CHAOS,		"chaos",			"something strange",6,	RV(6,1,6,0),	6,		500,	TERM_VIOLET,	 	0,              NULL,			MH(CHAOS),			PH(CHAOS))
ELEM(DISEN,		"disenchantment",	"something strange",6,  RV(6,1,6,0),	6,		500,	TERM_VIOLET,	    0,              NULL,			MH(DISEN),			PH(DISEN))
ELEM(WATER,		"water",			"water",			0,	RV(0,0,0,0),	6,		0,		TERM_SLATE,			0,				NULL,			MH(WATER),			PH(WATER))
ELEM(ICE,		"ice",				"something sharp",	1,  RV(3,0,0,0),	6,		0,		TERM_WHITE,		    TMD_OPP_COLD,   OH(shatter),	MH(ICE),			PH(ICE))
ELEM(GRAVITY,	"gravity",			"something strange",0,	RV(0,0,0,0),	3,		200,	TERM_L_WHITE,		0,				NULL,			MH(GRAVITY),		PH(GRAVITY))
ELEM(INERTIA,	"inertia",			"something strange",0,	RV(0,0,0,0),	6,		200,	TERM_L_WHITE,		0,				NULL,			MH(INERTIA),		PH(INERTIA))
ELEM(FORCE,		"force",			"something hard",	0,	RV(0,0,0,0),	6,		200,	TERM_UMBER,			0,				OH(shatter),	MH(FORCE),			PH(FORCE))
ELEM(TIME,		"time",				"something strange",0,	RV(0,0,0,0),	3,		150,	TERM_L_BLUE,		0,				NULL,			MH(TIME),			PH(TIME))
ELEM(PLASMA,	"plasma",			"something",		0,	RV(0,0,0,0),	6,		150,	TERM_RED,			0,				OH(PLASMA),		MH(PLASMA),			PH(PLASMA))
ELEM(METEOR,	"a meteor",			"something",		0,	RV(0,0,0,0),	6,		0,		TERM_RED,			0,				OH(METEOR),		NULL,				NULL)
ELEM(MISSILE,	"a missile",		"something",		0,	RV(0,0,0,0),	6,		0,		TERM_VIOLET,		0,				NULL,			NULL,				NULL)
ELEM(MANA,		"mana",				"something",		0,	RV(0,0,0,0),	3,		1600,	TERM_L_DARK,		0,				OH(MANA),		NULL,				NULL)
ELEM(HOLY_ORB,	"a holy orb",		"something",		0,	RV(0,0,0,0),	6,		0,		TERM_L_DARK,		0,				OH(HOLY_ORB),	MH(HOLY_ORB),		NULL)
ELEM(ARROW,		"an arrow",			"something sharp",	0,	RV(0,0,0,0),	6,		0,		TERM_WHITE,			0,				NULL,			NULL,				NULL)
