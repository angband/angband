#ifndef INCLUDED_UI_GAME_H
#define INCLUDED_UI_GAME_H

#include "angband.h"

byte monster_health_attr(void);
void cnv_stat(int val, char *out_val, size_t out_len);
void toggle_inven_equip(void);
void subwindows_set_flags(u32b *new_flags, size_t n_subwindows);

#endif /* INCLUDED_UI_GAME_H */
