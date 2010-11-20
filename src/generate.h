/* generate.h - dungeon generation interface */

#ifndef GENERATE_H
#define GENERATE_H

extern int level_hgt;
extern int level_wid;
void place_object(int y, int x, int level, bool good, bool great);
void place_gold(int y, int x, int level);
void place_secret_door(int y, int x);
void place_closed_door(int y, int x);
void place_random_door(int y, int x);
extern void generate_cave(void);

#endif /* !GENERATE_H */
