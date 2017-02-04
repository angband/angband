/**
 * \file src/list-stats.h
 * \brief player stats
 *
 * index: the stat number
 * sustain: object flag name for sustain
 * sust_p: power of sustain
 * pos_adj: adjective for gain of this stat
 * neg_adj: adjective for loss of this stat
 * name: name of the stat
 * sustain name: name of the sustain rune
 */
/* index       	sustain			pos adj		neg adj		name 			sustain name */
STAT(STR,        SUST_STR,		"strong",	"weak",		"strength",		"sustain strength")
STAT(INT,        SUST_INT,		"smart",	"stupid",	"intelligence",	"sustain intelligence")
STAT(WIS,        SUST_WIS,		"wise",		"naive",	"wisdom",		"sustain wisdom")
STAT(DEX,        SUST_DEX,		"dextrous",	"clumsy",	"dexterity",	"sustain dexterity")
STAT(CON,        SUST_CON,		"healthy",	"sickly",	"constitution",	"sustain constitution")
