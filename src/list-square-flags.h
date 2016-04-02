/**
 * \file list-square-flags.h
 * \brief special grid flags
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 13, so a flag's sequence number is its line number minus 12.
 *
 *
 */

/*  symbol          descr */
SQUARE(NONE,		"")
SQUARE(MARK,		"memorized feature")
SQUARE(GLOW,		"self-illuminating")
SQUARE(VAULT,		"part of a vault")
SQUARE(ROOM,		"part of a room")
SQUARE(SEEN,		"seen flag")
SQUARE(VIEW,		"view flag")
SQUARE(WASSEEN,		"previously seen (during update)")
SQUARE(FEEL,		"hidden points to trigger feelings")
SQUARE(TRAP,		"square containing a known trap")
SQUARE(INVIS,		"square containing an unknown trap")
SQUARE(WALL_INNER,	"inner wall generation flag")
SQUARE(WALL_OUTER,	"outer wall generation flag")
SQUARE(WALL_SOLID,	"solid wall generation flag")
SQUARE(MON_RESTRICT,"no random monster flag")
SQUARE(NO_TELEPORT,	"player can't teleport from this square")
SQUARE(NO_MAP,		"square can't be magically mapped")
SQUARE(NO_ESP,		"telepathy doesn't work on this square")
SQUARE(PROJECT,		"marked for projection processing")
