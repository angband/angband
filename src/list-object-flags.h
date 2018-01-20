/**
 * \file list-object-flags.h
 * \brief object flags for all objects
 *
 * Changing flag order will break savefiles. Flags
 * below start from 1 on line 14, so a flag's sequence number is its line
 * number minus 13.
 *
 * Each sustain flag (SUST_*) has a matching stat in src/list-stats.h,
 * which should be at the same index in that file as the sustain in this file.
 *
 * Flag properties are defined in lib/gamedata/object_property.txt
 */
OF(SUST_STR)
OF(SUST_INT)
OF(SUST_WIS)
OF(SUST_DEX)
OF(SUST_CON)
OF(PROT_FEAR)
OF(PROT_BLIND)
OF(PROT_CONF)
OF(PROT_STUN)
OF(SLOW_DIGEST)
OF(FEATHER)
OF(REGEN)
OF(TELEPATHY)
OF(SEE_INVIS)
OF(FREE_ACT)
OF(HOLD_LIFE)
OF(IMPACT)
OF(BLESSED)
OF(BURNS_OUT)
OF(TAKES_FUEL)
OF(NO_FUEL)
OF(IMPAIR_HP)
OF(IMPAIR_MANA)
OF(AFRAID)
OF(NO_TELEPORT)
OF(AGGRAVATE)
OF(DRAIN_EXP)
OF(STICKY)
OF(FRAGILE)
OF(LIGHT_1)
OF(LIGHT_2)
OF(DIG_1)
OF(DIG_2)
OF(DIG_3)
OF(EXPLODE)
OF(TRAP_IMMUNE)
OF(MAX)
