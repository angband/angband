/**
 * \file src/list-stats.h
 * \brief player stats
 *
 * index: the stat number
 * power: base power rating for the stat as an object modifier
 * sustain: object flag name for sustain
 * sust_p: power of sustain
 * mult: weight of this stat as an object modifier relative to others
 * pos_adj: adjective for gain of this stat
 * neg_adj: adjective for loss of this stat
 * name: name of the stat
 * sustain name: name of the sustain rune
 */
/* index       	power	sustain			sust_p	mult		pos adj		neg adj		name 			sustain name */
STAT(STR,        9,		SUST_STR,		9,		13,			"strong",	"weak",		"strength",		"sustain strength")
STAT(INT,        5,		SUST_INT,		4,		10,			"smart",	"stupid",	"intelligence",	"sustain intelligence")
STAT(WIS,        5,		SUST_WIS,		4,		10,			"wise",		"naive",	"wisdom",		"sustain wisdom")
STAT(DEX,        8,		SUST_DEX,		7,		10,			"dextrous",	"clumsy",	"dexterity",	"sustain dexterity")
STAT(CON,        12,	SUST_CON,		8,		15,			"healthy",	"sickly",	"constitution",	"sustain constitution")
