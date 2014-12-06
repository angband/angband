/**
 * \file list-project_monster.h
 * \brief Spell types directly affecting monsters used by project()
 * and related functions.
 *
 * Fields:
 * name - type name
 * force_obv - TRUE to force obvious if seen in project_m(), FALSE to let the 
 *             handler decide
 */

/* name  				force_obv */
PROJ_MON(AWAY_UNDEAD,	FALSE)
PROJ_MON(AWAY_EVIL,		FALSE)
PROJ_MON(AWAY_ALL,		TRUE)
PROJ_MON(TURN_UNDEAD,	FALSE)
PROJ_MON(TURN_EVIL,		FALSE)
PROJ_MON(TURN_ALL,		FALSE)
PROJ_MON(DISP_UNDEAD,	FALSE)
PROJ_MON(DISP_EVIL,		FALSE)
PROJ_MON(DISP_ALL,		TRUE)
PROJ_MON(OLD_CLONE,		TRUE)
PROJ_MON(OLD_POLY,		FALSE)
PROJ_MON(OLD_HEAL,		TRUE)
PROJ_MON(OLD_SPEED,		TRUE)
PROJ_MON(OLD_SLOW,		TRUE)
PROJ_MON(OLD_CONF,		FALSE)
PROJ_MON(OLD_SLEEP,		FALSE)
PROJ_MON(OLD_DRAIN,		TRUE)
