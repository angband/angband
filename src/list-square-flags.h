/* list-square-flags.h - special grid flags
 *
 * Adjusting these flags does not break savefiles. Flags below start from 1
 * on line 11, so a flag's sequence number is its line number minus 10.
 *
 *
 */

/*  symbol     descr */
SQUARE(NONE,     "")
SQUARE(MARK,     "memorized feature")
SQUARE(GLOW,     "self-illuminating")
SQUARE(VAULT,    "part of a vault")
SQUARE(ROOM,     "part of a room")
SQUARE(SEEN,     "seen flag")
SQUARE(VIEW,     "view flag")
SQUARE(WASSEEN,  "previously seen (during update)")
SQUARE(WALL,     "wall flag")
SQUARE(DTRAP,    "trap detected square")
SQUARE(FEEL,     "hidden points to trigger feelings")
SQUARE(DEDGE,    "border of trap detected area")
SQUARE(VERT,     "use an alternate visual for this square")
