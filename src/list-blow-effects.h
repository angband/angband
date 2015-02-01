/**
 * \file list-blow-effects.h
 * \brief monster race blow effects
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 16, so a flag's sequence number is its line number minus 15.
 *
 * Fields:
 * pow - used for attack quality in check_hit()
 * eval - used for power evaluation in eval_blow_effect()
 * desc - description for monster recall
 */

/*  symbol		pow		eval	desc */
RBE(NONE,		0,		0,		"")
RBE(HURT,		40,		0,		"attack")
RBE(POISON,		20,		10,		"poison")
RBE(DISENCHANT,	10,		30,		"disenchant")
RBE(DRAIN_CHARGES,	10,	30,		"drain charges")
RBE(EAT_GOLD,	0,		5,		"steal gold")
RBE(EAT_ITEM,	0,		5,		"steal items")
RBE(EAT_FOOD,	0,		5,		"eat your food")
RBE(EAT_LIGHT,	0,		5,		"absorb light")
RBE(ACID,		20,		20,		"shoot acid")
RBE(ELEC,		40,		10,		"electrify")
RBE(FIRE,		40,		10,		"burn")
RBE(COLD,		40,		10,		"freeze")
RBE(BLIND,		0,		20,		"blind")
RBE(CONFUSE,	20,		20,		"confuse")
RBE(TERRIFY,	0,		10,		"terrify")
RBE(PARALYZE,	0,		40,		"paralyze")
RBE(LOSE_STR,	0,		20,		"reduce strength")
RBE(LOSE_INT,	0,		20,		"reduce intelligence")
RBE(LOSE_WIS,	0,		20,		"reduce wisdom")
RBE(LOSE_DEX,	0,		20,		"reduce dexterity")
RBE(LOSE_CON,	0,		30,		"reduce constitution")
RBE(LOSE_ALL,	0,		40,		"reduce all stats")
RBE(SHATTER,	60,		300,	"shatter")
RBE(EXP_10,		20,		5,		"lower experience")
RBE(EXP_20,		20,		5,		"lower experience")
RBE(EXP_40,		20,		10,		"lower experience")
RBE(EXP_80,		20,		10,		"lower experience")
RBE(HALLU,		0,		20,		"cause hallucinations")
RBE(MAX,		0,		0,		"")
