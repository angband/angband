/* calcs.c */
void calc_bonuses(object_type inventory[], player_state *state, bool id_only);
int calc_blows(object_type *o_ptr, player_state *state);
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
bool player_can_cast(void);
bool player_can_study(void);
bool player_can_read(void);
