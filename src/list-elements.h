/*
 * File: src/list-elements.h
 * Purpose: Elements used in spells and other attacks.
 *
 * Fields:
 * name - type index 
 * desc - text description of attack if blind
 * resist - object flag for resistance
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * color - color of the effect
 * opp - timed flag for temporary resistance ("opposition")
 * immunity - object flag for total immunity
 * vuln - object flag for vulnerability
 * obj_hates - object flag for object vulnerability
 * obj_imm - object flag for object immunity
 * feature handler - handler affecting the environment
 * object handler - handler affecting an object
 * monster handler - handler affecting a monster
 * player handler - handler effecting the player
 */

/* name  		desc				resist          num denom           color				opp             immunity    vuln            obj_hates		obj_imm			feature handler	object handler	monster handler		player handler*/
ELEM(ACID,		"acid",				OF_RES_ACID,    1,  RV(3,0,0,0),	TERM_SLATE,		    TMD_OPP_ACID,   OF_IM_ACID, OF_VULN_ACID,   OF_HATES_ACID,	OF_IGNORE_ACID,	NULL,			OH(ACID),		MH(ACID),			PH(ACID))
ELEM(ELEC,		"lightning",		OF_RES_ELEC,    1,  RV(3,0,0,0),	TERM_BLUE,		    TMD_OPP_ELEC,   OF_IM_ELEC, OF_VULN_ELEC,   OF_HATES_ELEC,	OF_IGNORE_ELEC,	NULL,			OH(ELEC),		MH(ELEC),			PH(ELEC))
ELEM(FIRE,		"fire",				OF_RES_FIRE,    1,  RV(3,0,0,0),	TERM_RED,		    TMD_OPP_FIRE,   OF_IM_FIRE, OF_VULN_FIRE,   OF_HATES_FIRE,	OF_IGNORE_FIRE,	NULL,			OH(FIRE),		MH(FIRE),			PH(FIRE))
ELEM(COLD,		"cold",				OF_RES_COLD,    1,  RV(3,0,0,0),	TERM_WHITE,		    TMD_OPP_COLD,   OF_IM_COLD, OF_VULN_COLD,   OF_HATES_COLD,	OF_IGNORE_COLD,	NULL,			OH(COLD),		MH(COLD),			PH(COLD))
ELEM(POIS,		"poison",			OF_RES_POIS,    1,  RV(3,0,0,0),	TERM_GREEN,		    TMD_OPP_POIS,   0,          0,              0,				0,				NULL,			NULL,			MH(POIS),			PH(POIS))
ELEM(LIGHT,		"something",		OF_RES_LIGHT,   4,  RV(6,1,6,0),	TERM_ORANGE,	    0,              0,          0,              0,				0,				FH(LIGHT),		NULL,			MH(LIGHT),			PH(LIGHT))
ELEM(DARK,		"something",		OF_RES_DARK,    4,  RV(6,1,6,0),	TERM_L_DARK,	    0,              0,          0,              0,				0,				FH(DARK),		NULL,			MH(DARK),			PH(DARK))
ELEM(SOUND,		"noise",			OF_RES_SOUND,   5,  RV(6,1,6,0),	TERM_YELLOW,	    0,              0,          0,              0,				0,				NULL,			OH(shatter),	MH(SOUND),			PH(SOUND))
ELEM(SHARD,		"something sharp",	OF_RES_SHARD,   6,  RV(6,1,6,0),	TERM_UMBER,		    0,              0,          0,              0,				0,				NULL,			OH(shatter),	MH(SHARD),			PH(SHARD))
ELEM(NEXUS,		"something strange",OF_RES_NEXUS,   6,  RV(6,1,6,0),	TERM_L_RED,		    0,              0,          0,              0,				0,				NULL,			NULL,			MH(NEXUS),			PH(NEXUS))
ELEM(NETHER,	"something cold",	OF_RES_NETHER,  6,  RV(6,1,6,0),	TERM_L_GREEN,	    0,              0,          0,              0,				0,				NULL,			NULL,			MH(NETHER),			PH(NETHER))
ELEM(CHAOS,		"something strange",OF_RES_CHAOS,	6,	RV(6,1,6,0),	TERM_VIOLET,	 	0,              0,          0,              0,				0,				NULL,			NULL,			MH(CHAOS),			PH(CHAOS))
ELEM(DISEN,		"something strange",OF_RES_DISEN,   6,  RV(6,1,6,0),	TERM_VIOLET,	    0,              0,          0,              0,				0,				NULL,			NULL,			MH(DISEN),			PH(DISEN))
ELEM(WATER,		"water",			0,				0,	RV(0,0,0,0),	TERM_SLATE,			0,				0,			0,				0,				0,				NULL,			NULL,			MH(WATER),			PH(WATER))
ELEM(ICE,		"something sharp",	OF_RES_COLD,    1,  RV(3,0,0,0),	TERM_WHITE,		    TMD_OPP_COLD,   OF_IM_COLD, OF_VULN_COLD,   OF_HATES_COLD,	OF_IGNORE_COLD,	NULL,			OH(shatter),	MH(ICE),			PH(ICE))
ELEM(GRAVITY,	"something strange",0,				0,	RV(0,0,0,0),	TERM_L_WHITE,		0,				0,			0,				0,				0,				NULL,			NULL,			MH(GRAVITY),		PH(GRAVITY))
ELEM(INERTIA,	"something strange",0,				0,	RV(0,0,0,0),	TERM_L_WHITE,		0,				0,			0,				0,				0,				NULL,			NULL,			MH(INERTIA),		PH(INERTIA))
ELEM(FORCE,		"something hard",	0,				0,	RV(0,0,0,0),	TERM_UMBER,			0,				0,			0,				0,				0,				NULL,			OH(shatter),	MH(FORCE),			PH(FORCE))
ELEM(TIME,		"something strange",0,				0,	RV(0,0,0,0),	TERM_L_BLUE,		0,				0,			0,				0,				0,				NULL,			NULL,			MH(TIME),			PH(TIME))
ELEM(PLASMA,	"something",		0,				0,	RV(0,0,0,0),	TERM_RED,			0,				0,			0,				0,				0,				NULL,			OH(PLASMA),		MH(PLASMA),			PH(PLASMA))
ELEM(METEOR,	"something",		0,				0,	RV(0,0,0,0),	TERM_RED,			0,				0,			0,				0,				0,				NULL,			OH(METEOR),		NULL,				NULL)
ELEM(MISSILE,	"something",		0,				0,	RV(0,0,0,0),	TERM_VIOLET,		0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
ELEM(MANA,		"something",		0,				0,	RV(0,0,0,0),	TERM_L_DARK,		0,				0,			0,				0,				0,				NULL,			OH(MANA),		NULL,				NULL)
ELEM(HOLY_ORB,	"something",		0,				0,	RV(0,0,0,0),	TERM_L_DARK,		0,				0,			0,				0,				0,				NULL,			OH(HOLY_ORB),	MH(HOLY_ORB),		NULL)
ELEM(ARROW,		"something sharp",	0,				0,	RV(0,0,0,0),	TERM_WHITE,			0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
