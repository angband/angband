/* player/player.h - player interface */

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include "player/types.h"

/* calcs.c */
extern const byte adj_chr_gold[STAT_RANGE];
extern const byte adj_str_blow[STAT_RANGE];
extern const byte adj_dex_safe[STAT_RANGE];
extern const byte adj_con_fix[STAT_RANGE];
extern const byte adj_str_hold[STAT_RANGE];

void calc_bonuses(object_type inventory[], player_state *state, bool id_only);
int calc_blows(const object_type *o_ptr, player_state *state);
void notice_stuff(void);
void update_stuff(void);
void redraw_stuff(void);
void handle_stuff(void);
int weight_remaining(void);

/* player.c */
extern bool player_stat_inc(struct player *p, int stat);
extern bool player_stat_dec(struct player *p, int stat, bool permanent);

/* timed.c */
bool set_timed(int idx, int v, bool notify);
bool inc_timed(int idx, int v, bool notify);
bool dec_timed(int idx, int v, bool notify);
bool clear_timed(int idx, bool notify);
bool set_food(int v);

/* util.c */
s16b modify_stat_value(int value, int amount);
bool player_can_cast(void);
bool player_can_study(void);
bool player_can_read(void);

#endif /* !PLAYER_PLAYER_H */
