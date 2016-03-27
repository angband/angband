/**
 * \file cmds.h
 * \brief Header for game command files
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_CMDS_H
#define INCLUDED_CMDS_H

#include "cave.h"
#include "cmd-core.h"

/* cmd-cave.c */
void do_cmd_go_up(struct command *cmd);
void do_cmd_go_down(struct command *cmd);
void do_cmd_open(struct command *cmd);
void do_cmd_close(struct command *cmd);
void do_cmd_tunnel(struct command *cmd);
void do_cmd_disarm(struct command *cmd);
void do_cmd_alter_aux(int dir);
void do_cmd_alter(struct command *cmd);
void move_player(int dir, bool disarm);
void do_cmd_walk(struct command *cmd);
void do_cmd_jump(struct command *cmd);
void do_cmd_run(struct command *cmd);
void do_cmd_pathfind(struct command *cmd);
void do_cmd_hold(struct command *cmd);
void do_cmd_rest(struct command *cmd);
void do_cmd_sleep(struct command *cmd);
void display_feeling(bool obj_only);
void do_cmd_feeling(void);

/* cmd-misc.c */
void do_cmd_wizard(void);
void do_cmd_suicide(struct command *cmd);
void do_cmd_note(void);

/* cmd-obj.c */
void do_cmd_uninscribe(struct command *cmd);
void do_cmd_inscribe(struct command *cmd);
void do_cmd_autoinscribe(struct command *cmd);
void do_cmd_takeoff(struct command *cmd);
void do_cmd_wield(struct command *cmd);
void do_cmd_drop(struct command *cmd);
void do_cmd_read_scroll(struct command *cmd);
void do_cmd_use_staff(struct command *cmd);
void do_cmd_aim_wand(struct command *cmd);
void do_cmd_zap_rod(struct command *cmd);
void do_cmd_activate(struct command *cmd);
void do_cmd_eat_food(struct command *cmd);
void do_cmd_quaff_potion(struct command *cmd);
void do_cmd_use(struct command *cmd);
void do_cmd_refill(struct command *cmd);
void do_cmd_cast(struct command *cmd);
void do_cmd_study_spell(struct command *cmd);
void do_cmd_study_book(struct command *cmd);
void do_cmd_study(struct command *cmd);

/* cmd-pickup.c */
int do_autopickup(void);
void do_cmd_pickup(struct command *cmd);
void do_cmd_autopickup(struct command *cmd);

#endif
