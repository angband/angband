/*
 * File: src/list-object-modifiers.h
 * Purpose: object modifiers (plusses and minuses) for all objects
 *
 * Changing order will break savefiles. Modifiers
 * below start from 0 on line 17, so a mod's sequence number is its line
 * number minus 17.  This info is left from flags, and needs work.
 *
 * index: the mod number
 * id: when the mod is IDd
 * power: base power rating for the mod (0 means it is unused or derived)
 * mod_mult: weight of this modifier relative to others
 * message: what is printed when the mod is IDd (but see also identify.c 
 * and list-slays.h)
 */
/* index       		power	p_m		message */
OBJ_MOD(NONE,       0,      0,  	"")
OBJ_MOD(STR,        9,		13,		"")
OBJ_MOD(INT,        5,		10,		"")
OBJ_MOD(WIS,        5,		10,		"")
OBJ_MOD(DEX,        8,		10,		"")
OBJ_MOD(CON,        12,		15,		"")
OBJ_MOD(STEALTH,    8,		12,		"Your %s glows.")
OBJ_MOD(SEARCH,     2,		5,		"Your %s glows.")
OBJ_MOD(INFRA,      4,		8,		"")
OBJ_MOD(TUNNEL,     3,		8,		"")
OBJ_MOD(SPEED,      20,		6,		"")
OBJ_MOD(BLOWS,      0,		50,		"")
OBJ_MOD(SHOTS,      0,		50,		"")
OBJ_MOD(MIGHT,      0,		30,		"")
OBJ_MOD(LIGHT,      3,		6,		"")
OBJ_MOD(MAX,        0,		0,		"")
