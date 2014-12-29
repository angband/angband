#ifndef DUNGEON_H
#define DUNGEON_H

#include "angband.h"

extern u16b daycount;
extern u32b seed_randart;
extern u32b seed_flavor;
extern s32b turn;
extern bool character_generated;
extern bool character_dungeon;
extern bool character_saved;
extern s16b character_xtra;

extern bool is_daytime(void);
extern void dungeon_change_level(int dlev);
void do_animation(void);
extern void play_game(bool new_game);
extern void save_game(void);
extern void close_game(void);

#endif /* !DUNGEON_H */
