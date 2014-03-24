/*
 * File: src/list-object-modifiers.h
 * Purpose: object modifiers (plusses and minuses) for all objects
 *
 * Changing order will break savefiles. Modifiers
 * below start from 0 on line 19, so a mod's sequence number is its line
 * number minus 19.  This info is left from flags, and needs work.
 *
 * index: the flag number
 * pval: is it a quantitative flag? FALSE means it's just on/off
 * id: when the flag is IDd
 * type: what type of flag is it?
 * power: base power rating for the flag (0 means it is unused or derived)
 * pval_mult: weight of this flag relative to other pval flags
 * wpn/bow/ring/amu/light/body/cloak/shield/hat/gloves/boots: power multiplier for this slot
 * message: what is printed when the flag is IDd (but see also identify.c and list-slays.h)
 */
/* index       	pval	id			type		power	p_m	wpn	bow	ring	amu	light	body	cloak	shield	hat	gloves	boots	message */
OBJ_MOD(NONE,        FALSE, 0,          0,          0,      0,  0,	0,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
OBJ_MOD(STR,         TRUE,	OFID_WIELD,	OFT_STAT,	9,		13,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(INT,         TRUE,	OFID_WIELD,	OFT_STAT,	5,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(WIS,         TRUE,	OFID_WIELD,	OFT_STAT,	5,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(DEX,         TRUE,	OFID_WIELD,	OFT_STAT,	8,		10,	1,	1,	1,		1,	1,		1,		1,		1,		1,	2,		1,		"")
OBJ_MOD(CON,         TRUE,	OFID_WIELD,	OFT_STAT,	12,		15,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(STEALTH,     TRUE,	OFID_WIELD,	OFT_PVAL,	8,		12,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"Your %s glows.")
OBJ_MOD(SEARCH,      TRUE,	OFID_WIELD,	OFT_PVAL,	2,		5,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"Your %s glows.")
OBJ_MOD(INFRA,       TRUE,	OFID_WIELD,	OFT_PVAL,	4,		8,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(TUNNEL,      TRUE,	OFID_WIELD,	OFT_PVAL,	3,		8,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(SPEED,       TRUE,	OFID_WIELD,	OFT_PVAL,	20,		6,	1,	1,	1,		1,	1,		1,		1,		1,		1,	1,		1,		"")
OBJ_MOD(BLOWS,       TRUE,	OFID_WIELD,	OFT_PVAL,	0,		50,	1,	0,	3,		3,	3,		3,		3,		3,		3,	3,		3,		"")
OBJ_MOD(SHOTS,       TRUE,	OFID_WIELD,	OFT_PVAL,	0,		50,	0,	1,	4,		4,	4,		4,		4,		4,		4,	4,		4,		"")
OBJ_MOD(MIGHT,       TRUE,	OFID_WIELD,	OFT_PVAL,	0,		30,	0,	1,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
OBJ_MOD(LIGHT,       TRUE,	OFID_WIELD,	OFT_PVAL,	3,		6,	1,  1,  1,      1,  3,      1,      1,      1,      1,  1,      1,		"")
OBJ_MOD(MAX,        FALSE,	0,			0,			0,		0,	0,	0,	0,		0,	0,		0,		0,		0,		0,	0,		0,		"")
