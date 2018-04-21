/**
 * \file list-summon-types.h
 * \brief Summon method details

 * Fields:
 * name    - summon type name
 * message - message type
 * uniq    - whether uniques are allowed
 * base1-3 - allowed monster bases if any
 * flag    - allowed racial flag if any
 * description
 *
 * Note that if base1 and flag are both set, any allowed race must the flag 
 * and a valid base
 */
/*  name	message type		uniq	base1			base2	base3	flag 		description*/
S(ANY,		MSG_SUM_MONSTER,	TRUE,	NULL,			NULL,	NULL,	0,			"a monster")
S(KIN,		MSG_SUM_MONSTER,	FALSE,	NULL,			NULL,	NULL,	0,			"similar monsters")
S(MONSTER,	MSG_SUM_MONSTER,	FALSE,	NULL,			NULL,	NULL,	0,			"a monster")
S(MONSTERS,	MSG_SUM_MONSTER,	FALSE,	NULL,			NULL,	NULL,	0,			"monsters")
S(ANIMAL,	MSG_SUM_ANIMAL,		FALSE,	NULL,			NULL,	NULL,	RF_ANIMAL,	"animals")
S(SPIDER,	MSG_SUM_SPIDER,		FALSE,	"spider",		NULL,	NULL,	0,			"spiders")
S(HOUND,	MSG_SUM_HOUND,		FALSE,	"zephyr hound",	"canine",NULL,	0,			"hounds")
S(HYDRA,	MSG_SUM_HYDRA,		FALSE,	"hydra",		NULL,	NULL,	0,			"hydrae")
S(AINU,		MSG_SUM_AINU,		FALSE,	"ainu",			NULL,	NULL,	0,			"ainu")
S(DEMON,	MSG_SUM_DEMON,		FALSE,	NULL,			NULL,	NULL,	RF_DEMON,	"demons")
S(UNDEAD,	MSG_SUM_UNDEAD,		FALSE,	NULL,			NULL,	NULL,	RF_UNDEAD,	"undead")
S(DRAGON,	MSG_SUM_DRAGON,		FALSE,	NULL,			NULL,	NULL,	RF_DRAGON,	"dragons")
S(HI_DEMON,	MSG_SUM_HI_DEMON,	TRUE,	"major demon",	NULL,	NULL,	0,			"greater demons")
S(HI_UNDEAD,MSG_SUM_HI_UNDEAD,	TRUE,	"vampire",		"wraith","lich",0,			"greater undead")
S(HI_DRAGON,MSG_SUM_HI_DRAGON,	TRUE,	"ancient dragon",NULL,	NULL,	0,			"ancient dragons")
S(WRAITH,	MSG_SUM_WRAITH,		TRUE,	"wraith",		NULL,	NULL,	RF_UNIQUE,	"ringwraiths")
S(UNIQUE,	MSG_SUM_UNIQUE,		TRUE,	NULL,			NULL,	NULL,	RF_UNIQUE,	"unique monsters")
S(MAX,		MSG_SUM_MONSTER,	FALSE,	NULL,			NULL,	NULL,	0,			"")
