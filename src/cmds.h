#ifndef INCLUDED_CMDS_H
#define INCLUDED_CMDS_H

#include "cave.h"
#include "cmd-core.h"
struct store;

/* cmd-cave.c */
void do_cmd_go_up(struct command *cmd);
void do_cmd_go_down(struct command *cmd);
bool search(bool verbose);
void do_cmd_search(struct command *cmd);
void do_cmd_toggle_search(struct command *cmd);
void do_cmd_open(struct command *cmd);
void do_cmd_close(struct command *cmd);
void do_cmd_tunnel(struct command *cmd);
void do_cmd_disarm(struct command *cmd);
void do_cmd_alter_aux(int dir);
void do_cmd_alter(struct command *cmd);
void do_cmd_walk(struct command *cmd);
void do_cmd_jump(struct command *cmd);
void do_cmd_run(struct command *cmd);
void do_cmd_pathfind(struct command *cmd);
void do_cmd_hold(struct command *cmd);
void do_cmd_rest(struct command *cmd);
void display_feeling(bool obj_only);
void do_cmd_feeling(void);

void textui_cmd_rest(void);

/* cmd-context.c */
int context_menu_player(int mx, int my);
int context_menu_cave(struct cave *c, int y, int x, int adjacent, int mx, int my);
int context_menu_object(const object_type *o_ptr, const int slot);
int context_menu_store(struct store *store, const int oid, int mx, int my);
int context_menu_store_item(struct store *store, const int oid, int mx, int my);

/* cmd-know.c */
void do_cmd_redraw(void);
void do_cmd_change_name(void);
void do_cmd_message_one(void);
void do_cmd_messages(void);
void do_cmd_inven(void);
void do_cmd_equip(void);
void do_cmd_look(void);
void do_cmd_locate(void);
int cmp_monsters(const void *a, const void *b);
void do_cmd_query_symbol(void);
void do_cmd_center_map(void);
void do_cmd_monlist(void);
void do_cmd_itemlist(void);

/* cmd-misc.c */
void do_cmd_wizard(void);
void do_cmd_try_debug(void);
void do_cmd_quit(struct command *cmd);
void do_cmd_xxx_options(void);
void do_cmd_unknown(void);
void do_cmd_suicide(struct command *cmd);
void textui_cmd_suicide(void);
void do_cmd_save_game(struct command *cmd);
void do_cmd_target(void);
void do_cmd_target_closest(void);
void do_cmd_load_screen(void);
void do_cmd_save_screen(void);
void do_cmd_version(void);
void do_cmd_pref(void);
void do_cmd_note(void);

/* cmd-obj.c */
void do_cmd_uninscribe(struct command *cmd);
void do_cmd_inscribe(struct command *cmd);
void do_cmd_takeoff(struct command *cmd);
void wield_item(object_type *o_ptr, int item, int slot);
void do_cmd_wield(struct command *cmd);
void do_cmd_drop(struct command *cmd);
void do_cmd_destroy(struct command *cmd);
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
void textui_cmd_destroy_menu(int item);
void textui_cmd_destroy(void);
void textui_cmd_toggle_ignore(void);
void textui_obj_examine(void);

/* cmd-pickup.c */
void do_cmd_pickup(struct command *cmd);
void do_cmd_autopickup(struct command *cmd);
int do_autopickup(void);
byte py_pickup(int pickup);
byte py_pickup_item(int pickup, int item);
void move_player(int dir, bool disarm);

/* cmd-process.c */
void cmd_init(void);
unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode);
cmd_code cmd_lookup(unsigned char key, int mode);
int context_menu_command(int mx, int my);
void textui_process_command(bool no_request);
bool key_confirm_command(unsigned char c);
unsigned char cmd_lookup_key_unktrl(cmd_code lookup_cmd, int mode);

/* XXX none under here should be here */

/* store.c */
extern void do_cmd_store(struct command *cmd);
extern void do_cmd_sell(struct command *cmd);
extern void do_cmd_stash(struct command *cmd);
extern void do_cmd_buy(struct command *cmd);
extern void do_cmd_retrieve(struct command *cmd);

/* ui-knowledge.c */
extern int big_pad(int col, int row, byte a, wchar_t c);
extern void textui_browse_object_knowledge(const char *name, int row);
extern void textui_knowledge_init(void);
extern void textui_browse_knowledge(void);

/* ui-option.c */
void do_cmd_options(void);

#endif
