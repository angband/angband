/*
 * File: src/list-gf-types.h
 * Purpose: Spell types used by project() and related functions.
 *
 * Fields:
 * name - type index (GF_THIS)
 * desc - text description of attack if blind
 * resist - object flag for resistance
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * force_obv - TRUE to force obvious if seen in project_m(), FALSE to let the handler decide
 * color - color of the effect
 * opp - timed flag for temporary resistance ("opposition")
 * immunity - object flag for total immunity
 * side_immune - whether immunity protects from *all* side-effects
 * vuln - object flag for vulnerability
 * mon_res - monster flag for resistance
 * mon_vuln - monster flag for vulnerability
 * obj_hates - object flag for object vulnerability
 * obj_imm - object flag for object immunity
 * floor handler - handler affecting the floor
 * object handler - handler affecting an object
 * monster handler - handler affecting a monster
 * player handler - handler effecting the player
 */

/* name  		desc				resist          num denom           force_obv	color				opp             immunity    side_im	vuln            mon_resist  mon_vuln		obj_hates		obj_imm			floor handler	object handler	monster handler		player handler*/
GF(ARROW,		"something sharp",	0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
GF(MISSILE,		"something",		0,				0,	RV(0,0,0,0),	TRUE,		TERM_VIOLET,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
GF(MANA,		"something",		0,				0,	RV(0,0,0,0),	TRUE,		TERM_L_DARK,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			OH(MANA),		NULL,				NULL)
GF(HOLY_ORB,	"something",		0,				0,	RV(0,0,0,0),	TRUE,		TERM_L_DARK,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			OH(HOLY_ORB),	MH(HOLY_ORB),		NULL)
GF(LIGHT_WEAK,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_ORANGE,		0,				0,			TRUE,	0,				0,			RF_HURT_LIGHT,	0,				0,				FH(LIGHT),		NULL,			MH(LIGHT_WEAK),		NULL)
GF(DARK_WEAK,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_L_DARK,		0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(DARK),		NULL,			NULL,				NULL)
GF(WATER,		"water",			0,				0,	RV(0,0,0,0),	TRUE,		TERM_SLATE,			0,				0,			TRUE,	0,				RF_IM_WATER,0,				0,				0,				NULL,			NULL,			MH(WATER),			NULL)
GF(PLASMA,		"something",		0,				0,	RV(0,0,0,0),	TRUE,		TERM_RED,			0,				0,			TRUE,	0,				RF_RES_PLAS,0,				0,				0,				NULL,			OH(PLASMA),		MH(PLASMA),			NULL)
GF(METEOR,		"something",		0,				0,	RV(0,0,0,0),	TRUE,		TERM_RED,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			OH(METEOR),		NULL,				NULL)
GF(ICE,			"something sharp",	OF_RES_COLD,    1,  RV(3,0,0,0),	TRUE,		TERM_WHITE,		    TMD_OPP_COLD,   OF_IM_COLD, FALSE,	OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD,	OF_HATES_COLD,	OF_IGNORE_COLD,	NULL,			OH(shatter),	MH(ICE),			NULL)
GF(GRAVITY,		"something strange",0,				0,	RV(0,0,0,0),	TRUE,		TERM_L_WHITE,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(GRAVITY),		PH(GRAVITY))
GF(INERTIA,		"something strange",0,				0,	RV(0,0,0,0),	TRUE,		TERM_L_WHITE,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(INERTIA),		NULL)
GF(FORCE,		"something hard",	0,				0,	RV(0,0,0,0),	TRUE,		TERM_UMBER,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			OH(shatter),	MH(FORCE),			NULL)
GF(TIME,		"something strange",0,				0,	RV(0,0,0,0),	TRUE,		TERM_L_BLUE,		0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(TIME),			NULL)
GF(ACID,		"acid",				OF_RES_ACID,    1,  RV(3,0,0,0),	TRUE,		TERM_SLATE,		    TMD_OPP_ACID,   OF_IM_ACID, TRUE,	OF_VULN_ACID,   RF_IM_ACID, 0,				OF_HATES_ACID,	OF_IGNORE_ACID,	NULL,			OH(ACID),		MH(ACID),			NULL)
GF(ELEC,		"lightning",		OF_RES_ELEC,    1,  RV(3,0,0,0),	TRUE,		TERM_BLUE,		    TMD_OPP_ELEC,   OF_IM_ELEC, TRUE,	OF_VULN_ELEC,   RF_IM_ELEC, 0,				OF_HATES_ELEC,	OF_IGNORE_ELEC,	NULL,			OH(ELEC),		MH(ELEC),			NULL)
GF(FIRE,		"fire",				OF_RES_FIRE,    1,  RV(3,0,0,0),	TRUE,		TERM_RED,		    TMD_OPP_FIRE,   OF_IM_FIRE, TRUE,	OF_VULN_FIRE,   RF_IM_FIRE, RF_HURT_FIRE,	OF_HATES_FIRE,	OF_IGNORE_FIRE,	NULL,			OH(FIRE),		MH(FIRE),			NULL)
GF(COLD,		"cold",				OF_RES_COLD,    1,  RV(3,0,0,0),	TRUE,		TERM_WHITE,		    TMD_OPP_COLD,   OF_IM_COLD, TRUE,	OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD,	OF_HATES_COLD,	OF_IGNORE_COLD,	NULL,			OH(COLD),		MH(COLD),			NULL)
GF(POIS,		"poison",			OF_RES_POIS,    1,  RV(3,0,0,0),	TRUE,		TERM_GREEN,		    TMD_OPP_POIS,   0,          TRUE ,	0,              RF_IM_POIS, 0,				0,				0,				NULL,			NULL,			MH(POIS),			NULL)
GF(LIGHT,		"something",		OF_RES_LIGHT,   4,  RV(6,1,6,0),	TRUE,		TERM_ORANGE,	    0,              0,          TRUE,	0,              0,          RF_HURT_LIGHT,	0,				0,				FH(LIGHT),		NULL,			MH(LIGHT),			NULL)
GF(DARK,		"something",		OF_RES_DARK,    4,  RV(6,1,6,0),	TRUE,		TERM_L_DARK,	    0,              0,          TRUE,	0,              0,          0,				0,				0,				FH(DARK),		NULL,			MH(DARK),			NULL)
GF(CONFU,		"something",		OF_RES_CONFU,	6,	RV(6,1,6,0),	FALSE,		TERM_L_UMBER,		TMD_OPP_CONF,	0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
GF(SOUND,		"noise",			OF_RES_SOUND,   5,  RV(6,1,6,0),	TRUE,		TERM_YELLOW,	    0,              0,          TRUE,	0,              0,          0,				0,				0,				NULL,			OH(shatter),	MH(SOUND),			NULL)
GF(SHARD,		"something sharp",	OF_RES_SHARD,   6,  RV(6,1,6,0),	TRUE,		TERM_UMBER,		    0,              0,          TRUE,	0,              0,          0,				0,				0,				NULL,			OH(shatter),	MH(SHARD),			NULL)
GF(NEXUS,		"something strange",OF_RES_NEXUS,   6,  RV(6,1,6,0),	TRUE,		TERM_L_RED,		    0,              0,          TRUE,	0,              RF_RES_NEXUS,0,				0,				0,				NULL,			NULL,			MH(NEXUS),			NULL)
GF(NETHER,		"something cold",	OF_RES_NETHR,   6,  RV(6,1,6,0),	TRUE,		TERM_L_GREEN,	    0,              0,          TRUE,	0,              RF_RES_NETH,0,				0,				0,				NULL,			NULL,			MH(NETHER),			NULL)
GF(CHAOS,		"something strange",OF_RES_CHAOS,	6,	RV(6,1,6,0),	TRUE,		TERM_VIOLET,	 	0,              0,          TRUE,	0,              0,          0,				0,				0,				NULL,			NULL,			MH(CHAOS),			NULL)
GF(DISEN,		"something strange",OF_RES_DISEN,   6,  RV(6,1,6,0),	TRUE,		TERM_VIOLET,	    0,              0,          TRUE,	0,              RF_RES_DISE,0,				0,				0,				NULL,			NULL,			MH(DISEN),			NULL)
GF(KILL_WALL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(KILL_WALL),	NULL,			MH(KILL_WALL),		NULL)
GF(KILL_DOOR,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(KILL_DOOR),	OH(chest),		NULL,				NULL)
GF(KILL_TRAP,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(KILL_TRAP),	OH(chest),		NULL,				NULL)
GF(MAKE_WALL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
GF(MAKE_DOOR,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(MAKE_DOOR),	NULL,			NULL,				NULL)
GF(MAKE_TRAP,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				FH(MAKE_TRAP),	NULL,			NULL,				NULL)
GF(AWAY_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(AWAY_UNDEAD),	NULL)
GF(AWAY_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(AWAY_EVIL),		NULL)
GF(AWAY_ALL,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(AWAY_ALL),		NULL)
GF(TURN_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(TURN_UNDEAD),	NULL)
GF(TURN_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(TURN_EVIL),		NULL)
GF(TURN_ALL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(TURN_ALL),		NULL)
GF(DISP_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(DISP_UNDEAD),	NULL)
GF(DISP_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(DISP_EVIL),		NULL)
GF(DISP_ALL,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(DISP_ALL),		NULL)
GF(OLD_CLONE,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_CLONE),		NULL)
GF(OLD_POLY,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_POLY),		NULL)
GF(OLD_HEAL,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_HEAL),		NULL)
GF(OLD_SPEED,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_SPEED),		NULL)
GF(OLD_SLOW,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_SLOW),		NULL)
GF(OLD_CONF,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_CONF),		NULL)
GF(OLD_SLEEP,	NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_SLEEP),		NULL)
GF(OLD_DRAIN,	NULL,				0,				0,	RV(0,0,0,0),	TRUE,		TERM_WHITE,			0,				0,			TRUE,	0,				0,			0,				0,				0,				NULL,			NULL,			MH(OLD_DRAIN),		NULL)
GF(MAX,			NULL,				0,				0,	RV(0,0,0,0),	FALSE,		TERM_WHITE,			0,				0,			FALSE,	0,				0,			0,				0,				0,				NULL,			NULL,			NULL,				NULL)
