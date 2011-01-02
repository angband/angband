/*
 * File: list-slays.h
 * Purpose: List of slay/brand types
 */

/*
 * Entries in this table should be in ascending order of multiplier, to
 * ensure that the highest one takes precedence. Structure is name,
 * object flag, vulnerable flag, monster resist flag, multiplier, ranged verb,
 * melee verb, verb describing what the thing does when it is active,
 * description of affected creatures, brand
 */

SLAY(XXX, FLAG_END,       FLAG_END,  FLAG_END,	 0,	NULL,	    NULL,      NULL,         NULL, NULL)
SLAY(ANM, OF_SLAY_ANIMAL, RF_ANIMAL, FLAG_END,   2,	"pierces",  "smite",   "glows",      "animals", NULL)
SLAY(EVL, OF_SLAY_EVIL,   RF_EVIL,   FLAG_END,   2,	"pierces",  "smite",   "glows",      "evil creatures", NULL)
SLAY(UND, OF_SLAY_UNDEAD, RF_UNDEAD, FLAG_END,   3,	"pierces",  "smite",   "glows",      "undead", NULL)
SLAY(DMN, OF_SLAY_DEMON,  RF_DEMON,  FLAG_END,   3,	"pierces",  "smite",   "glows",      "demons", NULL)
SLAY(ORC, OF_SLAY_ORC,    RF_ORC,    FLAG_END,   3,	"pierces",  "smite",   "glows",      "orcs", NULL)
SLAY(TRL, OF_SLAY_TROLL,  RF_TROLL,  FLAG_END,   3,	"pierces",  "smite",   "glows",      "trolls", NULL)
SLAY(GIA, OF_SLAY_GIANT,  RF_GIANT,  FLAG_END,   3,	"pierces",  "smite",   "glows",      "giants", NULL)
SLAY(DRG, OF_SLAY_DRAGON, RF_DRAGON, FLAG_END,   3,	"pierces",  "smite",   "glows",      "dragons", NULL)
SLAY(ACD, OF_BRAND_ACID,  FLAG_END,  RF_IM_ACID, 3,	"corrodes", "corrode", "spits",      "creatures not resistant to acid", "acid")
SLAY(ELC, OF_BRAND_ELEC,  FLAG_END,  RF_IM_ELEC, 3,	"zaps",     "zap",     "crackles",   "creatures not resistant to electricity", "lightning")
SLAY(FIR, OF_BRAND_FIRE,  FLAG_END,  RF_IM_FIRE, 3,	"burns",    "burn",    "flares",     "creatures not resistant to fire", "flames")
SLAY(CLD, OF_BRAND_COLD,  FLAG_END,  RF_IM_COLD, 3,	"freezes" , "freeze",  "grows cold", "creatures not resistant to cold", "frost")
SLAY(POI, OF_BRAND_POIS,  FLAG_END,  RF_IM_POIS, 3,	"poisons",  "poison",  "seethes",    "creatures not resistant to poison", "venom")
SLAY(XDR, OF_KILL_DRAGON, RF_DRAGON, FLAG_END,   5,	"deeply pierces", "fiercely smite", "glows brightly", "dragons", NULL)
SLAY(XDM, OF_KILL_DEMON,  RF_DEMON,  FLAG_END,   5,	"deeply pierces", "fiercely smite", "glows brightly", "demons", NULL)
SLAY(XUN, OF_KILL_UNDEAD, RF_UNDEAD, FLAG_END,   5,	"deeply pierces", "fiercely smite", "glows brightly", "undead", NULL)
