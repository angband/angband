/**
 * \file list-object-modifiers.h
 * \brief object modifiers (plusses and minuses) for all objects
 *
 * index: the mod number
 * power: base power rating for the mod (0 means it is unused or derived)
 * mult: weight of this modifier relative to others
 * message: what is printed when the mod is IDd (but see also identify.c 
 * and list-slays.h)
 */
/* index       		name */
OBJ_MOD(STEALTH,    "stealth")
OBJ_MOD(INFRA,      "infravision")
OBJ_MOD(TUNNEL,     "tunneling")
OBJ_MOD(SPEED,      "speed")
OBJ_MOD(BLOWS,      "attack speed")
OBJ_MOD(SHOTS,      "shooting speed")
OBJ_MOD(MIGHT,      "shooting power")
OBJ_MOD(LIGHT,      "light")
