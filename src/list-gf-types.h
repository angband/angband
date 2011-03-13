/*
 * File: src/list-gf-types.h
 * Purpose: Spell types used by project() and related functions.
 *
 * Fields:
 * name - type index (GF_THIS) 
 * resist - object flag for resistance
 * num - numerator for resistance
 * denom - denominator for resistance (random_value)
 * opp - timed flag for temporary resistance ("opposition")
 * immunity - object flag for total immunity
 * vuln - object flag for vulnerability
 * mon_res - monster flag for resistance
 * mon_vuln - monster flag for vulnerability
 */

/* name  		resist          num denom           opp             immunity    vuln            mon resist  mon vuln */
GF(ARROW,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MISSILE,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MANA,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(HOLY_ORB,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(LIGHT_WEAK,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			RF_HURT_LIGHT)
GF(DARK_WEAK,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(WATER,		0,				0,	RV(0,0,0,0),	0,				0,			0,				RF_IM_WATER,0)
GF(PLASMA,		0,				0,	RV(0,0,0,0),	0,				0,			0,				RF_RES_PLAS,0)
GF(METEOR,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(ICE,			OF_RES_COLD,    1,  RV(3,0,0,0),    TMD_OPP_COLD,   OF_IM_COLD, OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD)
GF(GRAVITY,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(INERTIA,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(FORCE,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(TIME,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(ACID,		OF_RES_ACID,    1,  RV(3,0,0,0),    TMD_OPP_ACID,   OF_IM_ACID, OF_VULN_ACID,   RF_IM_ACID, 0)
GF(ELEC,		OF_RES_ELEC,    1,  RV(3,0,0,0),    TMD_OPP_ELEC,   OF_IM_ELEC, OF_VULN_ELEC,   RF_IM_ELEC, 0)
GF(FIRE,		OF_RES_FIRE,    1,  RV(3,0,0,0),    TMD_OPP_FIRE,   OF_IM_FIRE, OF_VULN_FIRE,   RF_IM_FIRE, RF_HURT_FIRE)
GF(COLD,		OF_RES_COLD,    1,  RV(3,0,0,0),    TMD_OPP_COLD,   OF_IM_COLD, OF_VULN_COLD,   RF_IM_COLD, RF_HURT_COLD)
GF(POIS,		OF_RES_POIS,    1,  RV(3,0,0,0),    TMD_OPP_POIS,   0,          0,              RF_IM_POIS, 0)
GF(LIGHT,		OF_RES_LIGHT,   4,  RV(6,1,6,0),    0,              0,          0,              0,          RF_HURT_LIGHT)
GF(DARK,		OF_RES_DARK,    4,  RV(6,1,6,0),    0,              0,          0,              0,          0)
GF(CONFU,		0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(SOUND,		OF_RES_SOUND,   5,  RV(6,1,6,0),    0,              0,          0,              0,          0)
GF(SHARD,		OF_RES_SHARD,   6,  RV(6,1,6,0),    0,              0,          0,              0,          0)
GF(NEXUS,		OF_RES_NEXUS,   6,  RV(6,1,6,0),    0,              0,          0,              RF_RES_NEXUS,0)
GF(NETHER,		OF_RES_NETHR,   6,  RV(6,1,6,0),    0,              0,          0,              RF_RES_NETH,0)
GF(CHAOS,		OF_RES_CHAOS,   6,  RV(6,1,6,0),    0,              0,          0,              0,          0)
GF(DISEN,		OF_RES_DISEN,   6,  RV(6,1,6,0),    0,              0,          0,              RF_RES_DISE,0)
GF(KILL_WALL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(KILL_DOOR,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(KILL_TRAP,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MAKE_WALL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MAKE_DOOR,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MAKE_TRAP,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(AWAY_UNDEAD,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(AWAY_EVIL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(AWAY_ALL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(TURN_UNDEAD,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(TURN_EVIL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(TURN_ALL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(DISP_UNDEAD,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(DISP_EVIL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(DISP_ALL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_CLONE,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_POLY,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_HEAL,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_SPEED,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_SLOW,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_CONF,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_SLEEP,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(OLD_DRAIN,	0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
GF(MAX,			0,				0,	RV(0,0,0,0),	0,				0,			0,				0,			0)
