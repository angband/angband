/*
 * File: list-spell-effects.h
 * Purpose: List of side effects for each ranged attack type.
 *
 * Fields:
 * - numerical index (RSE_this)
 * - which attack type has this effect (GF_ if it applies to all attacks of
 *    this type, or RSF_ if it is specific to a single spell)
 * - is this effect temporary?
 * - flag for the effect
 * - basic duration (or impact if permanent)
 * - damage-dependent duration (or impact)
 * - chance of this effect arising from this attack (0 means always)
 * - does the player get a save against this effect?
 * - what object flag resists this effect? (in addition to gf_ptr->res_flag)
 */

/* index attack			timed	effect			base			dam		 		chance	save 	res_flag */
RSE(0,	GF_NONE,		FALSE,	0,				RV(0,0,0,0),	RV(0,0,0,0),	0,		FALSE, 	0)
RSE(1,	GF_POIS,		TRUE,	TMD_POISONED,	RV(10,0,0,0),	RV(0,1,100,0),	0,		FALSE,	0)
RSE(2,	GF_LIGHT,		TRUE,	TMD_BLIND,		RV(2,1,5,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_BLIND)
RSE(3,	GF_DARK,		TRUE,	TMD_BLIND,		RV(2,1,5,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_BLIND)
RSE(4,	GF_SOUND,		TRUE,	TMD_STUN,		RV(0,0,0,0),	RV(5,1,33,35),	0,		FALSE,	OF_RES_STUN)
RSE(5,	GF_SHARDS,		TRUE,	TMD_CUT,		RV(0,0,0,0),	RV(0,0,100,0),	0,		FALSE,	0)
RSE(6,	GF_CHAOS,		TRUE,	TMD_IMAGE,		RV(0,1,10,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(7,	GF_CHAOS,		TRUE,	TMD_CONFUSED,	RV(9,1,20,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_CONFU)
RSE(8,	GF_INERTIA,		TRUE,	TMD_SLOW,		RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(9,	GF_GRAVITY,		TRUE,	TMD_STUN,		RV(0,0,0,0),	RV(5,1,33,35),	0,		FALSE,	OF_RES_STUN)
RSE(10,	GF_GRAVITY,		TRUE,	TMD_SLOW,		RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(11,	GF_FORCE,		TRUE,	TMD_STUN,		RV(0,1,20,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_STUN)
RSE(12,	GF_PLASMA,		TRUE,	TMD_STUN,		RV(0,0,0,0),	RV(5,1,75,35),	0,		FALSE,	OF_RES_STUN)
RSE(13,	GF_WATER,		TRUE,	TMD_CONFUSED,	RV(5,1,5,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_CONFU)
RSE(14,	GF_WATER,		TRUE,	TMD_STUN,		RV(0,1,40,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_STUN)
RSE(15,	GF_ICE,			TRUE,	TMD_CUT,		RV(0,5,8,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_SHARD)
RSE(16,	GF_ICE,			TRUE,	TMD_STUN,		RV(0,1,15,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_STUN)
RSE(17,	RSF_CAUSE_4,	TRUE,	TMD_CUT,		RV(0,10,10,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(18,	RSF_MIND_BLAST,	TRUE,	TMD_CONFUSED,	RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_CONFU)
RSE(19,	GF_ACID,		FALSE,	S_INV_DAM,		RV(1,0,0,0),	RV(0,0,0,0),	0,		FALSE,	OF_IM_ACID)
RSE(20,	GF_ELEC,		FALSE,	S_INV_DAM,		RV(1,0,0,0),	RV(0,0,0,0),	0,		FALSE,	OF_IM_ELEC)
RSE(21,	GF_FIRE,		FALSE,	S_INV_DAM,		RV(1,0,0,0),	RV(0,0,0,0),	0,		FALSE,	OF_IM_FIRE)
RSE(22,	GF_COLD,		FALSE,	S_INV_DAM,		RV(1,0,0,0),	RV(0,0,0,0),	0,		FALSE,	OF_IM_COLD)
RSE(23,	GF_NEXUS,		FALSE,	S_TELEPORT,		RV(200,0,0,0),	RV(0,0,0,0),	43,		FALSE,	0)
RSE(24,	GF_NEXUS,		FALSE,	S_TELE_TO,		RV(0,0,0,0),	RV(0,0,0,0),	29,		FALSE,	0)
RSE(25,	GF_NEXUS,		FALSE,	S_TELE_LEV,		RV(1,0,0,0),	RV(0,0,0,0),	14,		TRUE,	0)
RSE(26,	GF_NEXUS,		FALSE,	S_SWAP_STAT,	RV(1,0,0,0),	RV(0,0,0,0),	14,		TRUE,	0)
RSE(27,	GF_NETHER,		FALSE,	S_DRAIN_LIFE,	RV(200,0,1,0),	RV(0,0,0,0),	0,		FALSE,	OF_HOLD_LIFE)
RSE(28,	GF_CHAOS,		FALSE,	S_DRAIN_LIFE,	RV(5000,0,1,0),	RV(0,0,0,0),	0,		FALSE,	OF_HOLD_LIFE)
RSE(29,	GF_DISEN,		FALSE,	S_INV_DAM,		RV(1,0,0,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(30,	GF_GRAVITY,		FALSE,	S_TELEPORT,		RV(5,0,0,0),	RV(0,0,0,0),	0,		FALSE,	0)
RSE(31,	GF_TIME,		FALSE,	S_DRAIN_LIFE,	RV(100,0,1,0),	RV(0,0,0,0),	50,		FALSE,	OF_HOLD_LIFE)
RSE(32,	GF_TIME,		FALSE,	S_DRAIN_STAT,	RV(2,0,0,0),	RV(0,0,0,0),	40,		FALSE,	0)
RSE(33,	GF_TIME,		FALSE,	S_DRAIN_ALL,	RV(1,0,0,0),	RV(0,0,0,0),	10,		FALSE,	0)
RSE(34,	RSF_BRAIN_SMASH,TRUE,	TMD_BLIND,		RV(7,1,8,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_BLIND)
RSE(35,	RSF_BRAIN_SMASH,TRUE,	TMD_CONFUSED,	RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	OF_RES_CONFU)
RSE(36,	RSF_BRAIN_SMASH,TRUE,	TMD_PARALYZED,	RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	OF_FREE_ACT)
RSE(37,	RSF_BRAIN_SMASH,TRUE,	TMD_SLOW,		RV(3,1,4,0),	RV(0,0,0,0),	0,		FALSE,	0)
