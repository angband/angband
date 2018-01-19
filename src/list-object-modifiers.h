/**
 * \file list-object-modifiers.h
 * \brief object modifiers (plusses and minuses) for all objects
 *
 * Changing modifier order will break savefiles. Modifiers
 * below start from 5 on line 11, so a modifier's sequence number is its line
 * number minus 6.
 *
 * Modifier properties are defined in lib/gamedata/object_property.txt
 */
OBJ_MOD(STEALTH)
OBJ_MOD(SEARCH)
OBJ_MOD(INFRA)
OBJ_MOD(TUNNEL)
OBJ_MOD(SPEED)
OBJ_MOD(BLOWS)
OBJ_MOD(SHOTS)
OBJ_MOD(MIGHT)
OBJ_MOD(LIGHT)
OBJ_MOD(DAM_RED)
OBJ_MOD(MOVES)
