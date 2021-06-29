/**
 * \file list-square-flags.h
 * \brief special grid flags
 *
 * Adding flags to the end will not break savefiles (the added flags will be
 * read but not used when a savefile is loaded into an older version);
 * inserting into, deleting, or rearranging the existing flags will break
 * savefiles.  Flags below start from 1 on line 14, so a flag's sequence
 * number is its line number minus 13.
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
SQUARE(MON_RESTRICT,	"no random monster flag")
SQUARE(NO_TELEPORT,	"player can't teleport from this square")
SQUARE(NO_MAP,		"square can't be magically mapped")
SQUARE(NO_ESP,		"telepathy doesn't work on this square")
SQUARE(PROJECT,		"marked for projection processing")
SQUARE(DTRAP,		"trap detected square")
SQUARE(NO_STAIRS,	"square is not suitable for placing stairs")
SQUARE(CLOSE_PLAYER,	"square is seen and in player's light radius or UNLIGHT detection radius")
