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

SLAY(XXX, 		FLAG_END,       FLAG_END,  FLAG_END,	0,	NULL,	    		NULL,      			NULL,         		NULL, 										NULL)
SLAY(ANIMAL2,	OF_SLAY_ANIMAL, RF_ANIMAL, FLAG_END,	2,	"pierces",  		"smite",   			"glows",      		"animals", 									NULL)
SLAY(EVIL2,		OF_SLAY_EVIL,   RF_EVIL,   FLAG_END,	2,	"pierces",  		"smite",   			"glows",      		"evil creatures", 							NULL)
SLAY(ACID2,		OF_BRAND_FIZZ,  FLAG_END,  RF_IM_ACID,	2,	"corrodes",			"corrode", 			"fizzes",   		"creatures not resistant to acid", 			"weak acid")
SLAY(ELEC2,		OF_BRAND_BUZZ,  FLAG_END,  RF_IM_ELEC,	2,	"zaps",     		"zap",     			"buzzes",			"creatures not resistant to electricity", 	"weak lightning")
SLAY(FIRE2,		OF_BRAND_WARM,  FLAG_END,  RF_IM_FIRE,	2,	"singes",			"singe",			"grows warm", 		"creatures not resistant to fire", 			"weak flames")
SLAY(COLD2,		OF_BRAND_COOL,  FLAG_END,  RF_IM_COLD,	2,	"chills" , 			"chill",  			"grows cool", 		"creatures not resistant to cold", 			"weak frost")
SLAY(POISON2,	OF_BRAND_ICKY,  FLAG_END,  RF_IM_POIS,	2,	"sickens", 			"sicken",  			"glows green",  	"creatures not resistant to poison", 		"weak venom")
SLAY(UNDEAD3,	OF_SLAY_UNDEAD, RF_UNDEAD, FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"undead", 									NULL)
SLAY(DEMON3,	OF_SLAY_DEMON,  RF_DEMON,  FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"demons", 									NULL)
SLAY(ORC3,		OF_SLAY_ORC,    RF_ORC,    FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"orcs", 									NULL)
SLAY(TROLL3,	OF_SLAY_TROLL,  RF_TROLL,  FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"trolls", 									NULL)
SLAY(GIANT3,	OF_SLAY_GIANT,  RF_GIANT,  FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"giants", 									NULL)
SLAY(DRAGON3,	OF_SLAY_DRAGON, RF_DRAGON, FLAG_END,	3,	"pierces",  		"smite",   			"glows",      		"dragons", 									NULL)
SLAY(ACID3,		OF_BRAND_ACID,  FLAG_END,  RF_IM_ACID,	3,	"dissolves", 		"dissolve", 		"spits",      		"creatures not resistant to acid", 			"acid")
SLAY(ELEC3,		OF_BRAND_ELEC,  FLAG_END,  RF_IM_ELEC,	3,	"shocks",     		"shock",     		"crackles",   		"creatures not resistant to electricity", 	"lightning")
SLAY(FIRE3,		OF_BRAND_FIRE,  FLAG_END,  RF_IM_FIRE,	3,	"burns",    		"burn",    			"flares",     		"creatures not resistant to fire", 			"flames")
SLAY(COLD3,		OF_BRAND_COLD,  FLAG_END,  RF_IM_COLD,	3,	"freezes" , 		"freeze",  			"grows cold", 		"creatures not resistant to cold", 			"frost")
SLAY(POISON3,	OF_BRAND_POIS,  FLAG_END,  RF_IM_POIS,	3,	"poisons",  		"poison",  			"seethes",    		"creatures not resistant to poison", 		"venom")
SLAY(DRAGON5, 	OF_KILL_DRAGON, RF_DRAGON, FLAG_END,	5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"dragons", 									NULL)
SLAY(DEMON5, 	OF_KILL_DEMON,  RF_DEMON,  FLAG_END,	5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"demons", 									NULL)
SLAY(UNDEAD5, 	OF_KILL_UNDEAD, RF_UNDEAD, FLAG_END,	5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"undead", 									NULL)
