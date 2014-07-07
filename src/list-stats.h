/**
   \file src/list-stats.h
   \brief player stats
 *
 * index: the stat number
 * power: base power rating for the stat as an object modifier
 * mult: weight of this stat as an object modifier relative to others
 * message: what is printed when the stat is IDd (but see also obj-identify.c 
 * and list-slays.h)
 */
/* index       	power	mult	adjective	neg adjective	name */
STAT(STR,        9,		13,		"strong",	"weak",			"strength")
STAT(INT,        5,		10,		"smart",	"stupid",		"intelligence")
STAT(WIS,        5,		10,		"wise",		"naive",		"wisdom")
STAT(DEX,        8,		10,		"dextrous",	"clumsy",		"dexterity")
STAT(CON,        12,	15,		"healthy",	"sickly",		"constitution")
