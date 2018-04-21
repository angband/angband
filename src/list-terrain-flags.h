/**
 * \file list-terrain-flags.h
 * \brief monster race blow effects
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 13, so a flag's sequence number is its line number minus 12.
 *
 *
 */

/*  symbol     descr */
TF(NONE,        "")
TF(LOS,        "Allows line of sight")
TF(PROJECT,    "Allows projections to pass through")
TF(PASSABLE,   "Can be passed through by all creatures")
TF(INTERESTING,"Is noticed on looking around")
TF(PERMANENT,  "Is permanent")
TF(EASY,       "Is easily passed through")
TF(TRAP,       "Can hold a trap")
TF(RUNE,       "Can hold a rune")
TF(NO_SCENT,   "Cannot store scent")
TF(NO_FLOW,    "No flow through")
TF(OBJECT,     "Can hold objects")
TF(TORCH,      "Becomes bright when torch-lit")
TF(HIDDEN,     "Can be found by searching")
TF(GOLD,       "Contains treasure")
TF(CLOSABLE,   "Can be closed")
TF(FLOOR,      "Is a clear floor")
TF(WALL,       "Is a solid wall")
TF(ROCK,       "Is rocky")
TF(GRANITE,    "Is a granite rock wall")
TF(DOOR_ANY,   "Is any door")
TF(DOOR_CLOSED,"Is a closed door")
TF(SHOP,       "Is a shop")
TF(DOOR_JAMMED,"Is a jammed door")
TF(DOOR_LOCKED,"Is a locked door")
TF(MAGMA,      "Is a magma seam")
TF(QUARTZ,     "Is a quartz seam")
TF(STAIR,      "Is a stair")
TF(UPSTAIR,    "Is a down staircase")
TF(DOWNSTAIR,  "Is an up staircase")
TF(SMOOTH,     "Should have smooth boundaries")
TF(BRIGHT,     "Is internally lit")
