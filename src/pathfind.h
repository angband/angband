/* pathfind.h */

#ifndef PATHFIND_H
#define PATHFIND_H

#include "z-type.h"

extern int pathfind_direction_to(struct loc from, struct loc to);
extern bool findpath(int y, int x);
extern void run_step(int dir);

#endif /* !PATHFIND_H */
