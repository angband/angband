/* player-birth.h */

#ifndef PLAYER_BIRTH_H
#define PLAYER_BIRTH_H

#include "cmd-core.h"

extern void player_init(struct player *p);
extern void player_generate(struct player *p, const player_sex *s,
                            const struct player_race *r,
                            const struct player_class *c);
extern char *get_history(struct history_chart *h);
extern void wield_all(struct player *p);

void do_cmd_birth_init(struct command *cmd);
void do_cmd_birth_reset(struct command *cmd);
void do_cmd_choose_sex(struct command *cmd);
void do_cmd_choose_race(struct command *cmd);
void do_cmd_choose_class(struct command *cmd);
void do_cmd_buy_stat(struct command *cmd);
void do_cmd_sell_stat(struct command *cmd);
void do_cmd_reset_stats(struct command *cmd);
void do_cmd_roll_stats(struct command *cmd);
void do_cmd_prev_stats(struct command *cmd);
void do_cmd_choose_name(struct command *cmd);
void do_cmd_accept_character(struct command *cmd);

char *find_roman_suffix_start(const char *buf);

#endif /* !PLAYER_BIRTH_H */
