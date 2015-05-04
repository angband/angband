/**
 * \file list-project-monsters.h
 * \brief Spell types directly affecting monsters used by project()
 * and related functions.
 *
 * Fields:
 * name - type name
 * force_obv - TRUE to force obvious if seen in project_m(), FALSE to let the 
 *             handler decide
 */

/* name  				force_obv */
PROJ_MON(AWAY_UNDEAD,	FALSE,	"teleports undead away")
PROJ_MON(AWAY_EVIL,		FALSE,	"teleports evil monsters away")
PROJ_MON(AWAY_ALL,		TRUE,	"teleports monsters away")
PROJ_MON(TURN_UNDEAD,	FALSE,	"turns undead")
PROJ_MON(TURN_EVIL,		FALSE,	"frightens evil monsters")
PROJ_MON(TURN_ALL,		FALSE,	"causes monsters to flee")
PROJ_MON(DISP_UNDEAD,	FALSE,	"damages undead")
PROJ_MON(DISP_EVIL,		FALSE,	"damages evil monsters")
PROJ_MON(DISP_ALL,		TRUE,	"damages all monsters")
PROJ_MON(OLD_CLONE,		TRUE,	"hastes, heals and magically duplicates monsters")
PROJ_MON(OLD_POLY,		FALSE,	"polymorphs monsters into other kinds of creatures")
PROJ_MON(OLD_HEAL,		TRUE,	"heals monsters")
PROJ_MON(OLD_SPEED,		TRUE,	"hastes monsters")
PROJ_MON(OLD_SLOW,		TRUE,	"attempts to slow monsters")
PROJ_MON(OLD_CONF,		FALSE,	"attempts to confuse monsters")
PROJ_MON(OLD_SLEEP,		FALSE,	"attempts to put monsters to sleep")
PROJ_MON(OLD_DRAIN,		TRUE,	"damages living monsters")
