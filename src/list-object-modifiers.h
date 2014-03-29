/*
 * File: src/list-object-modifiers.h
 * Purpose: object modifiers (plusses and minuses) for all objects
 *
 * Changing order will break savefiles. Modifiers
 * below start from 0 on line 19, so a mod's sequence number is its line
 * number minus 19.  This info is left from flags, and needs work.
 *
 * index: the flag number
 * id: when the flag is IDd
 * type: what type of flag is it?
 * power: base power rating for the flag (0 means it is unused or derived)
 * mod_mult: weight of this modifier relative to others
 * wpn/bow/ring/amu/light/body/cloak/shield/hat/gloves/boots: power multiplier for this slot
 * message: what is printed when the flag is IDd (but see also identify.c and list-slays.h)
 */
/* index       		id			type		power	p_m	wpn	bow	ring	amu	light	body	cloak	shield	hat	gloves	boots	message */
OBJ_MOD(NONE,       0,          0,          0,      0,  0,	0,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
OBJ_MOD(STR,        OFID_WIELD,	OFT_STAT,	9,		13,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(INT,        OFID_WIELD,	OFT_STAT,	5,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(WIS,        OFID_WIELD,	OFT_STAT,	5,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(DEX,        OFID_WIELD,	OFT_STAT,	8,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	2,		1,		"")
OBJ_MOD(CON,        OFID_WIELD,	OFT_STAT,	12,		15,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(STEALTH,    OFID_WIELD,	OFT_PVAL,	8,		12,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"Your %s glows.")
OBJ_MOD(SEARCH,     OFID_WIELD,	OFT_PVAL,	2,		5,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"Your %s glows.")
OBJ_MOD(INFRA,      OFID_WIELD,	OFT_PVAL,	4,		8,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(TUNNEL,     OFID_WIELD,	OFT_PVAL,	3,		8,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(SPEED,      OFID_WIELD,	OFT_PVAL,	20,		6,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(BLOWS,      OFID_WIELD,	OFT_PVAL,	0,		50,	1,	0,	3,		3,	3,		3,		3,		3,		3,	3,		3,		"")
OBJ_MOD(SHOTS,      OFID_WIELD,	OFT_PVAL,	0,		50,	0,	1,	4,		4,	4,		4,		4,		4,		4,	4,		4,		"")
OBJ_MOD(MIGHT,      OFID_WIELD,	OFT_PVAL,	0,		30,	0,	1,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
OBJ_MOD(LIGHT,      OFID_WIELD,	OFT_PVAL,	3,		6,	1,  1,  1,      1,  3,      1,      1,      1,      1,  1,      1,		"")
OBJ_MOD(MAX,        0,			0,			0,		0,	0,	0,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
