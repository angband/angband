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
/* index       		power	p_m		name */
OBJ_MOD(STR,        9,		13,		"strength")
OBJ_MOD(INT,        5,		10,		"intelligence")
OBJ_MOD(WIS,        5,		10,		"wisdom")
OBJ_MOD(DEX,        8,		10,		"dexterity")
OBJ_MOD(CON,        12,		15,		"constitution")
OBJ_MOD(STEALTH,    8,		12,		"stealth")
OBJ_MOD(SEARCH,     2,		5,		"searching skill")
OBJ_MOD(INFRA,      4,		8,		"infravision")
OBJ_MOD(TUNNEL,     3,		8,		"tunneling")
OBJ_MOD(SPEED,      20,		6,		"speed")
OBJ_MOD(BLOWS,      0,		50,		"attack speed")
OBJ_MOD(SHOTS,      0,		50,		"shooting speed")
OBJ_MOD(MIGHT,      0,		30,		"shooting power")
OBJ_MOD(LIGHT,      3,		6,		"")
