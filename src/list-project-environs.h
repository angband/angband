/*
 * File: src/list-project-environs.h
 * Purpose: Spell types used by project() and related functions.
 *
 * Fields:
 * name - type index 
 * color - color of the effect
 * mon_vuln - monster flag for vulnerability
 * feature handler - handler affecting the environment
 * object handler - handler affecting an object
 * monster handler - handler affecting a monster
 */

/* name  			color			mon_vuln		feature handler	object handler	monster handler */
PROJ_ENV(LIGHT_WEAK,TERM_ORANGE,	RF_HURT_LIGHT,	FH(LIGHT),		NULL,			MH(LIGHT_WEAK))
PROJ_ENV(DARK_WEAK,	TERM_L_DARK,	0,				FH(DARK),		NULL,			NULL)
PROJ_ENV(KILL_WALL,	TERM_WHITE,		0,				FH(KILL_WALL),	NULL,			MH(KILL_WALL))
PROJ_ENV(KILL_DOOR,	TERM_WHITE,		0,				FH(KILL_DOOR),	OH(chest),		NULL)
PROJ_ENV(KILL_TRAP,	TERM_WHITE,		0,				FH(KILL_TRAP),	OH(chest),		NULL)
PROJ_ENV(MAKE_DOOR,	TERM_WHITE,		0,				FH(MAKE_DOOR),	NULL,			NULL)
PROJ_ENV(MAKE_TRAP,	TERM_WHITE,		0,				FH(MAKE_TRAP),	NULL,			NULL)
