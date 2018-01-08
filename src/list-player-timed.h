/**
 * \file list-player-timed.h
 * \brief timed player properties
 *
 * Fields:
 * symbol - the effect name
 * flag_redraw - the things to be redrawn when the effect is set
 * flag_update - the things to be updated when the effect is set
 */

/* symbol		flag_redraw						flag_update */
TMD(FAST,		0,								PU_BONUS)
TMD(SLOW,		0,								PU_BONUS)
TMD(BLIND,		PR_MAP,							PU_UPDATE_VIEW | PU_MONSTERS) 
TMD(PARALYZED,	0,								0)
TMD(CONFUSED,	0,								PU_BONUS)
TMD(AFRAID,		0,								PU_BONUS)
TMD(IMAGE,		PR_MAP | PR_MONLIST | PR_ITEMLIST,	PU_BONUS)
TMD(POISONED,	0,								PU_BONUS)
TMD(CUT,		0,								0)
TMD(STUN,		0,								0)
TMD(PROTEVIL,	0,								0)
TMD(INVULN,		0,								PU_BONUS)
TMD(HERO,		0,								PU_BONUS)
TMD(SHERO,		0,								PU_BONUS)
TMD(SHIELD,		0,								PU_BONUS)
TMD(BLESSED,	0,								PU_BONUS)
TMD(SINVIS,		0,								PU_BONUS | PU_MONSTERS)
TMD(SINFRA,		0,								PU_BONUS | PU_MONSTERS)
TMD(OPP_ACID,	PR_STATUS,						PU_BONUS)
TMD(OPP_ELEC,	PR_STATUS,						PU_BONUS)
TMD(OPP_FIRE,	PR_STATUS,						PU_BONUS)
TMD(OPP_COLD,	PR_STATUS,						PU_BONUS)
TMD(OPP_POIS,	0,								PU_BONUS)
TMD(OPP_CONF,	PR_STATUS,						PU_BONUS)
TMD(AMNESIA,	0,								PU_BONUS)
TMD(TELEPATHY,	0,								PU_BONUS)
TMD(STONESKIN,	0,								PU_BONUS)
TMD(TERROR,		0,								PU_BONUS)
TMD(SPRINT,		0,								PU_BONUS)
TMD(BOLD,		0,								PU_BONUS)
TMD(SCRAMBLE,   PR_STATUS,		   				PU_BONUS)
TMD(TRAPSAFE,	0,								PU_BONUS)
TMD(FASTCAST,	0,								PU_BONUS)
