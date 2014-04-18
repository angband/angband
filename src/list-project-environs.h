/*
 * File: src/list-project-environs.h
 * Purpose: Spell types used by project() and related functions.
 *
 * Fields:
 * name - type index 
 * color - color of the effect
 * feature handler - handler affecting the environment
 * object handler - handler affecting an object
 * monster handler - handler affecting a monster
 */

/* name  			color			feat handler	obj handler	mon handler */
PROJ_ENV(LIGHT_WEAK,TERM_ORANGE,	FH(LIGHT),		NULL,		MH(LIGHT_WEAK))
PROJ_ENV(DARK_WEAK,	TERM_L_DARK,	FH(DARK),		NULL,		NULL)
PROJ_ENV(KILL_WALL,	TERM_WHITE,		FH(KILL_WALL),	NULL,		MH(KILL_WALL))
PROJ_ENV(KILL_DOOR,	TERM_WHITE,		FH(KILL_DOOR),	OH(chest),	NULL)
PROJ_ENV(KILL_TRAP,	TERM_WHITE,		FH(KILL_TRAP),	OH(chest),	NULL)
PROJ_ENV(MAKE_DOOR,	TERM_WHITE,		FH(MAKE_DOOR),	NULL,		NULL)
PROJ_ENV(MAKE_TRAP,	TERM_WHITE,		FH(MAKE_TRAP),	NULL,		NULL)
