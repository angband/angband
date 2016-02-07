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
S(ANY,		MSG_SUM_MONSTER,	true,	NULL,			NULL,	NULL,	0,			"a monster")
S(KIN,		MSG_SUM_MONSTER,	false,	NULL,			NULL,	NULL,	0,			"similar monsters")
S(MONSTER,	MSG_SUM_MONSTER,	false,	NULL,			NULL,	NULL,	0,			"a monster")
S(MONSTERS,	MSG_SUM_MONSTER,	false,	NULL,			NULL,	NULL,	0,			"monsters")
S(ANIMAL,	MSG_SUM_ANIMAL,		false,	NULL,			NULL,	NULL,	RF_ANIMAL,	"animals")
S(SPIDER,	MSG_SUM_SPIDER,		false,	"spider",		NULL,	NULL,	0,			"spiders")
S(HOUND,	MSG_SUM_HOUND,		false,	"zephyr hound",	"canine",NULL,	0,			"hounds")
S(HYDRA,	MSG_SUM_HYDRA,		false,	"hydra",		NULL,	NULL,	0,			"hydrae")
S(AINU,		MSG_SUM_AINU,		false,	"ainu",			NULL,	NULL,	0,			"ainu")
S(DEMON,	MSG_SUM_DEMON,		false,	NULL,			NULL,	NULL,	RF_DEMON,	"demons")
S(UNDEAD,	MSG_SUM_UNDEAD,		false,	NULL,			NULL,	NULL,	RF_UNDEAD,	"undead")
S(DRAGON,	MSG_SUM_DRAGON,		false,	NULL,			NULL,	NULL,	RF_DRAGON,	"dragons")
S(HI_DEMON,	MSG_SUM_HI_DEMON,	true,	"major demon",	NULL,	NULL,	0,			"greater demons")
S(HI_UNDEAD,MSG_SUM_HI_UNDEAD,	true,	"vampire",		"wraith","lich",0,			"greater undead")
S(HI_DRAGON,MSG_SUM_HI_DRAGON,	true,	"ancient dragon",NULL,	NULL,	0,			"ancient dragons")
S(WRAITH,	MSG_SUM_WRAITH,		true,	"wraith",		NULL,	NULL,	RF_UNIQUE,	"ringwraiths")
S(UNIQUE,	MSG_SUM_UNIQUE,		true,	NULL,			NULL,	NULL,	RF_UNIQUE,	"unique monsters")
S(MAX,		MSG_SUM_MONSTER,	false,	NULL,			NULL,	NULL,	0,			"")
