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
/* index       		id			power	p_m		message */
OBJ_MOD(NONE,       0,          0,      0,  	"")
OBJ_MOD(STR,        OFID_WIELD,	9,		13,		"")
OBJ_MOD(INT,        OFID_WIELD,	5,		10,		"")
OBJ_MOD(WIS,        OFID_WIELD,	5,		10,		"")
OBJ_MOD(DEX,        OFID_WIELD,	8,		10,		"")
OBJ_MOD(CON,        OFID_WIELD,	12,		15,		"")
OBJ_MOD(STEALTH,    OFID_WIELD,	8,		12,		"Your %s glows.")
OBJ_MOD(SEARCH,     OFID_WIELD,	2,		5,		"Your %s glows.")
OBJ_MOD(INFRA,      OFID_WIELD,	4,		8,		"")
OBJ_MOD(TUNNEL,     OFID_WIELD,	3,		8,		"")
OBJ_MOD(SPEED,      OFID_WIELD,	20,		6,		"")
OBJ_MOD(BLOWS,      OFID_WIELD,	0,		50,		"")
OBJ_MOD(SHOTS,      OFID_WIELD,	0,		50,		"")
OBJ_MOD(MIGHT,      OFID_WIELD,	0,		30,		"")
OBJ_MOD(LIGHT,      OFID_WIELD,	3,		6,		"")
OBJ_MOD(MAX,        0,			0,		0,		"")
