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
 * opp - timed flag for temporary resistance ("opposition")
 * immunity - object flag for total immunity
 * side_immune - whether immunity protects from *all* side-effects
 * vuln - object flag for vulnerability
 * mon_res - monster flag for resistance
 * mon_vuln - monster flag for vulnerability
 * obj_hates - object flag for object vulnerability
 * obj_imm - object flag for object immunity
 */

/* name  		desc				resist          num denom           opp             immunity    side_im	vuln            mon_resist  mon_vuln		obj_hates		obj_imm */
GF(ARROW,		"something sharp",	0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MISSILE,		"something",		0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MANA,		"something",		0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(HOLY_ORB,	"something",		0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(LIGHT_WEAK,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			RF_HURT_LIGHT,	0,				0)
GF(DARK_WEAK,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(WATER,		"water",			0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				RF_IM_WATER,0,				0,				0)
GF(PLASMA,		"something",		0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				RF_RES_PLAS,0,				0,				0)
GF(METEOR,		"something",		0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(ICE,			"something sharp",	OF_RES_COLD,    1,  RV(3,0,0,0),    TMD_OPP_COLD,   OF_IM_COLD, FALSE,	OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD,	OF_HATES_COLD,	OF_IGNORE_COLD)
GF(GRAVITY,		"something strange",0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(INERTIA,		"something strange",0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(FORCE,		"something hard",	0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(TIME,		"something strange",0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(ACID,		"acid",				OF_RES_ACID,    1,  RV(3,0,0,0),    TMD_OPP_ACID,   OF_IM_ACID, TRUE,	OF_VULN_ACID,   RF_IM_ACID, 0,				OF_HATES_ACID,	OF_IGNORE_ACID)
GF(ELEC,		"lightning",		OF_RES_ELEC,    1,  RV(3,0,0,0),    TMD_OPP_ELEC,   OF_IM_ELEC, TRUE,	OF_VULN_ELEC,   RF_IM_ELEC, 0,				OF_HATES_ELEC,	OF_IGNORE_ELEC)
GF(FIRE,		"fire",				OF_RES_FIRE,    1,  RV(3,0,0,0),    TMD_OPP_FIRE,   OF_IM_FIRE, TRUE,	OF_VULN_FIRE,   RF_IM_FIRE, RF_HURT_FIRE,	OF_HATES_FIRE,	OF_IGNORE_FIRE)
GF(COLD,		"cold",				OF_RES_COLD,    1,  RV(3,0,0,0),    TMD_OPP_COLD,   OF_IM_COLD, TRUE,	OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD,	OF_HATES_COLD,	OF_IGNORE_COLD)
GF(POIS,		"poison",			OF_RES_POIS,    1,  RV(3,0,0,0),    TMD_OPP_POIS,   0,          TRUE,	0,              RF_IM_POIS, 0,				0,				0)
GF(LIGHT,		"something",		OF_RES_LIGHT,   4,  RV(6,1,6,0),    0,              0,          TRUE,	0,              0,          RF_HURT_LIGHT,	0,				0)
GF(DARK,		"something",		OF_RES_DARK,    4,  RV(6,1,6,0),    0,              0,          TRUE,	0,              0,          0,				0,				0)
GF(CONFU,		"something",		OF_RES_CONFU,	6,	RV(6,1,6,0),	TMD_OPP_CONF,	0,			TRUE,	0,				0,			0,				0,				0)
GF(SOUND,		"noise",			OF_RES_SOUND,   5,  RV(6,1,6,0),    0,              0,          TRUE,	0,              0,          0,				0,				0)
GF(SHARD,		"something sharp",	OF_RES_SHARD,   6,  RV(6,1,6,0),    0,              0,          TRUE,	0,              0,          0,				0,				0)
GF(NEXUS,		"something strange",OF_RES_NEXUS,   6,  RV(6,1,6,0),    0,              0,          TRUE,	0,              RF_RES_NEXUS,0,				0,				0)
GF(NETHER,		"something cold",	OF_RES_NETHR,   6,  RV(6,1,6,0),    0,              0,          TRUE,	0,              RF_RES_NETH,0,				0,				0)
GF(CHAOS,		"something strange",OF_RES_CHAOS,	6,	RV(6,1,6,0), 	0,              0,          TRUE,	0,              0,          0,				0,				0)
GF(DISEN,		"something strange",OF_RES_DISEN,   6,  RV(6,1,6,0),    0,              0,          TRUE,	0,              RF_RES_DISE,0,				0,				0)
GF(KILL_WALL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(KILL_DOOR,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(KILL_TRAP,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MAKE_WALL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MAKE_DOOR,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MAKE_TRAP,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(AWAY_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(AWAY_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(AWAY_ALL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(TURN_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(TURN_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(TURN_ALL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(DISP_UNDEAD,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(DISP_EVIL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(DISP_ALL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_CLONE,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_POLY,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_HEAL,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_SPEED,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_SLOW,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_CONF,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_SLEEP,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(OLD_DRAIN,	NULL,				0,				0,	RV(0,0,0,0),	0,				0,			TRUE,	0,				0,			0,				0,				0)
GF(MAX,			NULL,				0,				0,	RV(0,0,0,0),	0,				0,			FALSE,	0,				0,			0,				0,				0)
