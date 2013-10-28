/* calcs.c */
void calc_bonuses(object_type inventory[], player_state *state);
void notice_stuff(void);
void update_stuff(void);
void redraw_stuff(void);
void handle_stuff(void);

/* timed.c */
bool set_timed(int idx, int v, bool notify);
bool inc_timed(int idx, int v, bool notify);
bool dec_timed(int idx, int v, bool notify);
bool clear_timed(int idx, bool notify);
bool set_food(int v);

/* util.c */
s16b modify_stat_value(int value, int amount);
