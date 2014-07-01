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
/* index       	power	mult	name */
STAT(STR,        9,		13,		"strength")
STAT(INT,        5,		10,		"intelligence")
STAT(WIS,        5,		10,		"wisdom")
STAT(DEX,        8,		10,		"dexterity")
STAT(CON,        12,	15,		"constitution")
