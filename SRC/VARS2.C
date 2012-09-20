/* vars2.c: Global variables, only cave array...
  cave array is barely small enough to fit into a 64k segment by
  itself.  Don't put any new variables here...  Leave vars2.c alone, unless
  the 64k segment barrier is overcome, or unless cave[][] gets
  smaller..  -CFT */

#include "constant.h"
#include "config.h"
#include "types.h"

#ifdef MAC
cave_type (*cave)[MAX_WIDTH];
#else
cave_type cave[MAX_HEIGHT][MAX_WIDTH];
#endif

