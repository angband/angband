/**
 * \file list-trap-flags.h
 * \brief trap properties
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 13, so a flag's sequence number is its line number minus 12.
 *
 *
 */

/*  symbol		descr */
TRF(NONE,		"")
TRF(RUNE,		"Is a rune")
TRF(TRAP,		"Is a player trap")
TRF(VISIBLE,	"Is visible")
TRF(INVISIBLE,	"Is invisible")
TRF(FLOOR,		"Can be set on a floor")
TRF(DOWN,		"Taked the player down a level")
TRF(LOCK,		"Is a door lock")
