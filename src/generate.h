/* generate.h - dungeon generation interface */

#ifndef GENERATE_H
#define GENERATE_H

void place_object(struct cave *c, int y, int x, int level, bool good, bool great);
void place_gold(int y, int x, int level);
void place_secret_door(int y, int x);
void place_closed_door(int y, int x);
void place_random_door(int y, int x);
extern void generate_cave(void);

extern struct vault *random_vault(void);

#endif /* !GENERATE_H */
