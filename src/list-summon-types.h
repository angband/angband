/**
 * \file list-summon-types.h
 * \brief Summon method details

 * Fields:
 * name    - summon type name
 * message - message type
 * uniq    - whether uniques are allowed
 * base1-3 - allowed monster bases if any
 * flag    - allowed racial flag if any
 *
 * Note that if base1 and flag are both set, any allowed race must the flag 
 * and a valid base
 */
/*  name	message type	uniq	base1			base2	base3	flag */
S(ANY,		MSG_SUM_MONSTER,TRUE,	NULL,			NULL,	NULL,	0)
S(KIN,		MSG_SUM_MONSTER,FALSE,	NULL,			NULL,	NULL,	0)
S(MONSTER,	MSG_SUM_MONSTER,FALSE,	NULL,			NULL,	NULL,	0)
S(MONSTERS,	MSG_SUM_MONSTER,FALSE,	NULL,			NULL,	NULL,	0)
S(ANIMAL,	MSG_SUM_ANIMAL,	FALSE,	NULL,			NULL,	NULL,	RF_ANIMAL)
S(SPIDER,	MSG_SUM_SPIDER,	FALSE,	"spider",		NULL,	NULL,	0)
S(HOUND,	MSG_SUM_HOUND,	FALSE,	"zephyr hound",	"canine",NULL,	0)
S(HYDRA,	MSG_SUM_HYDRA,	FALSE,	"hydra",		NULL,	NULL,	0)
S(AINU,		MSG_SUM_AINU,	FALSE,	"ainu",			NULL,	NULL,	0)
S(DEMON,	MSG_SUM_DEMON,	FALSE,	NULL,			NULL,	NULL,	RF_DEMON)
S(UNDEAD,	MSG_SUM_UNDEAD,	FALSE,	NULL,			NULL,	NULL,	RF_UNDEAD)
S(DRAGON,	MSG_SUM_DRAGON,	FALSE,	NULL,			NULL,	NULL,	RF_DRAGON)
S(HI_DEMON,	MSG_SUM_HI_DEMON,TRUE,	"major demon",	NULL,	NULL,	0)
S(HI_UNDEAD,MSG_SUM_HI_UNDEAD,TRUE,	"vampire",		"wraith","lich",0)
S(HI_DRAGON,MSG_SUM_HI_DRAGON,TRUE,	"ancient dragon",NULL,	NULL,	0)
S(WRAITH,	MSG_SUM_WRAITH,	TRUE,	"wraith",		NULL,	NULL,	RF_UNIQUE)
S(UNIQUE,	MSG_SUM_UNIQUE,	TRUE,	NULL,			NULL,	NULL,	RF_UNIQUE)
S(MAX,		MSG_SUM_MONSTER,FALSE,	NULL,			NULL,	NULL,	0)
