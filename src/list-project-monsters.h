/*
 * File: src/list-project_monster.h
 * Purpose: Spell types directly affecting monsters used by project()
 * and related functions.
 *
 * Fields:
 * name - type index 
 * force_obv - TRUE to force obvious if seen in project_m(), FALSE to let the 
 *             handler decide
 * monster handler - handler affecting a monster
 */

/* name  				force_obv	monster handler*/
PROJ_MON(AWAY_UNDEAD,	FALSE,		MH(AWAY_UNDEAD))
PROJ_MON(AWAY_EVIL,		FALSE,		MH(AWAY_EVIL))
PROJ_MON(AWAY_ALL,		TRUE,		MH(AWAY_ALL))
PROJ_MON(TURN_UNDEAD,	FALSE,		MH(TURN_UNDEAD))
PROJ_MON(TURN_EVIL,		FALSE,		MH(TURN_EVIL))
PROJ_MON(TURN_ALL,		FALSE,		MH(TURN_ALL))
PROJ_MON(DISP_UNDEAD,	FALSE,		MH(DISP_UNDEAD))
PROJ_MON(DISP_EVIL,		FALSE,		MH(DISP_EVIL))
PROJ_MON(DISP_ALL,		TRUE,		MH(DISP_ALL))
PROJ_MON(OLD_CLONE,		TRUE,		MH(OLD_CLONE))
PROJ_MON(OLD_POLY,		FALSE,		MH(OLD_POLY))
PROJ_MON(OLD_HEAL,		TRUE,		MH(OLD_HEAL))
PROJ_MON(OLD_SPEED,		TRUE,		MH(OLD_SPEED))
PROJ_MON(OLD_SLOW,		TRUE,		MH(OLD_SLOW))
PROJ_MON(OLD_CONF,		FALSE,		MH(OLD_CONF))
PROJ_MON(OLD_SLEEP,		FALSE,		MH(OLD_SLEEP))
PROJ_MON(OLD_DRAIN,		TRUE,		MH(OLD_DRAIN))
PROJ_MON(MAX,			FALSE,		NULL)
