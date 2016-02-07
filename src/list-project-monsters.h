/**
 * \file list-project-monsters.h
 * \brief Spell types directly affecting monsters used by project()
 * and related functions.
 *
 * Fields:
 * name - type name
 * force_obv - true to force obvious if seen in project_m(), false to let the 
 *             handler decide
 */

/* name  				force_obv */
PROJ_MON(AWAY_UNDEAD,	false,	"teleports undead away")
PROJ_MON(AWAY_EVIL,		false,	"teleports evil monsters away")
PROJ_MON(AWAY_ALL,		true,	"teleports monsters away")
PROJ_MON(TURN_UNDEAD,	false,	"turns undead")
PROJ_MON(TURN_EVIL,		false,	"frightens evil monsters")
PROJ_MON(TURN_ALL,		false,	"causes monsters to flee")
PROJ_MON(DISP_UNDEAD,	false,	"damages undead")
PROJ_MON(DISP_EVIL,		false,	"damages evil monsters")
PROJ_MON(DISP_ALL,		true,	"damages all monsters")
PROJ_MON(OLD_CLONE,		true,	"hastes, heals and magically duplicates monsters")
PROJ_MON(OLD_POLY,		false,	"polymorphs monsters into other kinds of creatures")
PROJ_MON(OLD_HEAL,		true,	"heals monsters")
PROJ_MON(OLD_SPEED,		true,	"hastes monsters")
PROJ_MON(OLD_SLOW,		true,	"attempts to slow monsters")
PROJ_MON(OLD_CONF,		false,	"attempts to confuse monsters")
PROJ_MON(OLD_SLEEP,		false,	"attempts to put monsters to sleep")
PROJ_MON(OLD_DRAIN,		true,	"damages living monsters")
