/*
 * File: src/object/list-flag-types.h
 * Purpose: object flag types for all objects
 *
 * index: the flag type number
 * pval: are these quantitative flags? FALSE means they're just on/off
 * desc: description
 */
/* index 	pval 	desc */
OFT(NONE,	FALSE,	NULL)
OFT(PVAL,	TRUE,	"granular flag")	/* pval-related but not to a stat */
OFT(STAT,	TRUE,	"stat")				/* affects a stat */
OFT(SUST,	FALSE,	"sustain")			/* sustains a stat */
OFT(SLAY,	FALSE,	"creature slay")	/* a "normal" creature-type slay */
OFT(BRAND,	FALSE,	"brand")			/* a brand against monsters lacking the resist */
OFT(KILL,	FALSE,	"creature bane")	/* a powerful creature-type slay */
OFT(VULN,	FALSE,	"vulnerability")	/* lowers resistance to an element */
OFT(IMM,	FALSE,	"immunity")			/* offers immunity to an element */
OFT(LRES,	FALSE,	"base resistance")	/* a "base" elemental resistance */
OFT(HRES,	FALSE,	"high resistance")	/* a "high" elemental resistance */
OFT(IGNORE,	FALSE,	"element-proofing")	/* object ignores an element */
OFT(HATES,	FALSE,	"susceptibility")	/* object can be destroyed by element */
OFT(PROT,	FALSE,	"protection")		/* protection from an effect */
OFT(MISC,	FALSE,	"power")			/* a good property, suitable for egos */
OFT(LIGHT,	FALSE,	"light power")		/* applicable only to light sources */
OFT(MELEE,	FALSE,	"melee power")		/* applicable only to melee weapons */
OFT(CURSE,	FALSE,	"curse")			/* a "sticky" curse */
OFT(BAD,	FALSE,	"malus")			/* an undesirable flag that isn't a curse */
OFT(INT,	FALSE,	"internal flag")	/* an internal flag, not shown in game */
OFT(MAX,	FALSE,	NULL)
