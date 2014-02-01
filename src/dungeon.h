#ifndef DUNGEON_H
#define DUNGEON_H

#define TOWN_DAWN		10000	/* Number of turns from dawn to dawn */
#define TOWN_DUSK         5000    /* Number of turns from dawn to dusk */

extern u16b daycount;

extern bool is_daytime(void);
extern void dungeon_change_level(int dlev);
extern void play_game(void);
extern int value_check_aux1(const object_type *o_ptr);
extern void idle_update(void);

#endif /* !DUNGEON_H */
