/**
 * \file list-player-flags.h
 * \brief player race and class flags
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 16, so a flag's sequence number is its line number minus 15.
 *
 * Fields:
 * symbol - the flag name
 * descr - description of the flag effect
 * birth-descr - description of the flag for use in the birth menus
 */

/* symbol            descr                                                        birth-descr */
PF(NONE,             "",                                                          NULL)
PF(EXTRA_SHOT,       "receive extra shots with tension bows at levels 20 and 40", "Gains extra shots with bow")
PF(BRAVERY_30,       "become immune to fear at level 30",                         "Gains immunity to fear")
PF(BLESS_WEAPON,     "may only wield blessed or hafted weapons",                  "Prefers blunt/blessed weapons")
PF(CUMBER_GLOVE,     "have difficulty using magic with covered hands",            NULL)
PF(ZERO_FAIL,        "may obtain a perfect success rate with magic",              "Advanced spellcasting")
PF(BEAM,             "frequently turn bolt spells into beams",                    NULL)
PF(CHOOSE_SPELLS,    "may choose their own spells to study",                      NULL)
PF(KNOW_MUSHROOM,    "easily recognize mushrooms",                                "Identifies mushrooms")
PF(KNOW_ZAPPER,      "easily recognize magic devices",                            "Identifies magic devices")
PF(SEE_ORE,          "can sense ore in the walls",                                "Senses ore/minerals")
PF(NO_MANA,          "cannot cast spells",                                        NULL)
PF(CHARM,            "is extra persuasive to monsters",                           "Charms monsters")
PF(UNLIGHT,          "gains stealth in, can see in, and resists the dark",        "Likes the dark")
PF(ROCK,             "is made of rock",                                           "Is made of rock")
