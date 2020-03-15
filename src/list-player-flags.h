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
PF(NONE)
PF(FAST_SHOT)
PF(BRAVERY_30)
PF(BLESS_WEAPON)
PF(ZERO_FAIL)
PF(BEAM)
PF(CHOOSE_SPELLS)
PF(KNOW_MUSHROOM)
PF(KNOW_ZAPPER)
PF(SEE_ORE)
PF(NO_MANA)
PF(CHARM)
PF(UNLIGHT)
PF(ROCK)
PF(STEAL)
PF(SHIELD_BASH)
PF(EVIL)
PF(CROWD_FIGHT)
