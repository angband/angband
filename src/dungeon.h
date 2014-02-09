#ifndef DUNGEON_H
#define DUNGEON_H

extern u16b daycount;

extern void dungeon_change_level(int dlev);
extern void play_game(void);
extern int value_check_aux1(const object_type *o_ptr);
extern void idle_update(void);

#endif /* !DUNGEON_H */
