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

/* name  			color			mon handler */
PROJ_ENV(LIGHT_WEAK,TERM_ORANGE,	MH(LIGHT_WEAK))
PROJ_ENV(DARK_WEAK,	TERM_L_DARK,	NULL)
PROJ_ENV(KILL_WALL,	TERM_WHITE,		MH(KILL_WALL))
PROJ_ENV(KILL_DOOR,	TERM_WHITE,		NULL)
PROJ_ENV(KILL_TRAP,	TERM_WHITE,		NULL)
PROJ_ENV(MAKE_DOOR,	TERM_WHITE,		NULL)
PROJ_ENV(MAKE_TRAP,	TERM_WHITE,		NULL)
