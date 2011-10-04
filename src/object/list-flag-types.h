/*
 * File: src/object/list-flag-types.h
 * Purpose: object flag types for all objects
 *
 * index: the flag type number
 * pval: are these quantitative flags? FALSE means they're just on/off
 */
/* index 	pval */
OFT(NONE,	FALSE)
OFT(PVAL,	TRUE)	/* pval-related but not to a stat */
OFT(STAT,	TRUE)	/* affects a stat */
OFT(SUST,	FALSE)	/* sustains a stat */
OFT(SLAY,	FALSE)	/* a "normal" creature-type slay */
OFT(BRAND,	FALSE)	/* a brand against monsters lacking the resist */
OFT(KILL,	FALSE)	/* a powerful creature-type slay */
OFT(VULN,	FALSE)	/* lowers resistance to an element */
OFT(IMM,	FALSE)	/* offers immunity to an element */
OFT(LRES,	FALSE)	/* a "base" elemental resistance */
OFT(HRES,	FALSE)	/* a "high" elemental resistance */
OFT(IGNORE,	FALSE)	/* object ignores an element */
OFT(HATES,	FALSE)	/* object can be destroyed by element */
OFT(PROT,	FALSE)	/* protection from an effect */
OFT(MISC,	FALSE)	/* a good property, suitable for ego items */
OFT(LIGHT,	FALSE)	/* applicable only to light sources */
OFT(MELEE,	FALSE)	/* applicable only to melee weapons */
OFT(CURSE,	FALSE)	/* a "sticky" curse */
OFT(BAD,	FALSE)	/* an undesirable flag that isn't a curse */
OFT(INT,	FALSE)	/* an internal flag, not shown in the game */
OFT(MAX,	FALSE)
