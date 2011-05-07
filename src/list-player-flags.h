/* list-player-flags.h - player race and class flags
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 11, so a flag's sequence number is its line number minus 10.
 *
 *
 */

/* symbol            descr */
PF(NONE,             "")
PF(EXTRA_SHOT,       "receive extra shots with tension bows at levels 20 and 40")
PF(BRAVERY_30,       "become immune to fear at level 30")
PF(CUMBER_GLOVE,     "have difficulty using magic with covered hands")
PF(ZERO_FAIL,        "may obtain a perfect success rate with magic")
PF(BEAM,             "frequently turn bolt spells into beams")
PF(CHOOSE_SPELLS,    "may choose their own spells to study")
PF(PSEUDO_ID_IMPROV, "get better at psudo id with experience")
PF(KNOW_MUSHROOM,    "easily recognize mushrooms")
PF(KNOW_ZAPPER,      "easily recognize magic devices")
PF(SEE_ORE,          "can sense ore in the walls")
