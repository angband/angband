/**
 * \file list-project-environs.h
 * \brief Spell types used by project() and related functions.
 *
 * Fields:
 * name - type name
 * color - color of the effect
 * description - description of the effect
 */

/* name  				color	 		description*/
PROJ_ENV(LIGHT_WEAK,	COLOUR_ORANGE,	"light")
PROJ_ENV(DARK_WEAK,		COLOUR_L_DARK,	"darkness")
PROJ_ENV(KILL_WALL,		COLOUR_WHITE,	"rock remover")
PROJ_ENV(KILL_DOOR,		COLOUR_WHITE,	"destroys doors")
PROJ_ENV(KILL_TRAP,		COLOUR_WHITE,	"destroys traps")
PROJ_ENV(MAKE_DOOR,		COLOUR_WHITE,	"creates doors")
PROJ_ENV(MAKE_TRAP,		COLOUR_WHITE,	"creates traps")
