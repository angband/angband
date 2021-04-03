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
void do_cmd_alter(struct command *cmd);
void do_cmd_steal(struct command *cmd);
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
void do_cmd_mon_command(struct command *cmd);

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
int do_autopickup(struct player *p);
void do_cmd_pickup(struct command *cmd);
void do_cmd_autopickup(struct command *cmd);

/* cmd-spoil.c */
void do_cmd_spoil_artifact(struct command *cmd);
void do_cmd_spoil_monster(struct command *cmd);
void do_cmd_spoil_monster_brief(struct command *cmd);
void do_cmd_spoil_obj(struct command *cmd);

/* cmd-wizard.c */
void do_cmd_wiz_acquire(struct command *cmd);
void do_cmd_wiz_advance(struct command *cmd);
void do_cmd_wiz_banish(struct command *cmd);
void do_cmd_wiz_change_item_quantity(struct command *cmd);
void do_cmd_wiz_collect_disconnect_stats(struct command *cmd);
void do_cmd_wiz_collect_obj_mon_stats(struct command *cmd);
void do_cmd_wiz_collect_pit_stats(struct command *cmd);
void do_cmd_wiz_create_all_artifact(struct command *cmd);
void do_cmd_wiz_create_all_artifact_from_tval(struct command *cmd);
void do_cmd_wiz_create_all_obj(struct command *cmd);
void do_cmd_wiz_create_all_obj_from_tval(struct command *cmd);
void do_cmd_wiz_create_artifact(struct command *cmd);
void do_cmd_wiz_create_obj(struct command *cmd);
void do_cmd_wiz_create_trap(struct command *cmd);
void do_cmd_wiz_cure_all(struct command *cmd);
void do_cmd_wiz_curse_item(struct command *cmd);
void do_cmd_wiz_detect_all_local(struct command *cmd);
void do_cmd_wiz_detect_all_monsters(struct command *cmd);
void do_cmd_wiz_display_keylog(struct command *cmd);
void do_cmd_wiz_dump_level_map(struct command *cmd);
void do_cmd_wiz_edit_player_exp(struct command *cmd);
void do_cmd_wiz_edit_player_gold(struct command *cmd);
void do_cmd_wiz_edit_player_start(struct command *cmd);
void do_cmd_wiz_edit_player_stat(struct command *cmd);
void do_cmd_wiz_hit_all_los(struct command *cmd);
void do_cmd_wiz_increase_exp(struct command *cmd);
void do_cmd_wiz_jump_level(struct command *cmd);
void do_cmd_wiz_learn_object_kinds(struct command *cmd);
void do_cmd_wiz_magic_map(struct command *cmd);
void do_cmd_wiz_peek_noise_scent(struct command *cmd);
void do_cmd_wiz_perform_effect(struct command *cmd);
void do_cmd_wiz_play_item(struct command *cmd);
void do_cmd_wiz_push_object(struct command *cmd);
void do_cmd_wiz_query_feature(struct command *cmd);
void do_cmd_wiz_query_square_flag(struct command *cmd);
void do_cmd_wiz_quit_no_save(struct command *cmd);
void do_cmd_wiz_recall_monster(struct command *cmd);
void do_cmd_wiz_rerate(struct command *cmd);
void do_cmd_wiz_reroll_item(struct command *cmd);
void do_cmd_wiz_stat_item(struct command *cmd);
void do_cmd_wiz_summon_named(struct command *cmd);
void do_cmd_wiz_summon_random(struct command *cmd);
void do_cmd_wiz_teleport_random(struct command *cmd);
void do_cmd_wiz_teleport_to(struct command *cmd);
void do_cmd_wiz_tweak_item(struct command *cmd);
void do_cmd_wiz_wipe_recall(struct command *cmd);
void do_cmd_wiz_wizard_light(struct command *cmd);

#endif
