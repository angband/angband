#ifndef INCLUDED_CMDS_H
#define INCLUDED_CMDS_H

#include "game-cmd.h"
struct store;

/* cmd-cave.c */
void do_cmd_go_up(cmd_code code, cmd_arg args[]);
void do_cmd_go_down(cmd_code code, cmd_arg args[]);
bool search(bool verbose);
void do_cmd_search(cmd_code code, cmd_arg args[]);
void do_cmd_toggle_search(cmd_code code, cmd_arg args[]);
void do_cmd_open(cmd_code code, cmd_arg args[]);
void do_cmd_close(cmd_code code, cmd_arg args[]);
void do_cmd_tunnel(cmd_code code, cmd_arg args[]);
void do_cmd_disarm(cmd_code code, cmd_arg args[]);
void do_cmd_alter_aux(int dir);
void do_cmd_alter(cmd_code code, cmd_arg args[]);
void do_cmd_walk(cmd_code code, cmd_arg args[]);
void do_cmd_jump(cmd_code code, cmd_arg args[]);
void do_cmd_run(cmd_code code, cmd_arg args[]);
void do_cmd_pathfind(cmd_code code, cmd_arg args[]);
void do_cmd_hold(cmd_code code, cmd_arg args[]);
void do_cmd_rest(cmd_code code, cmd_arg args[]);
void display_feeling(bool obj_only);
void do_cmd_feeling(void);

void textui_cmd_rest(void);
int count_feats(int *y, int *x, bool (*test)(struct cave *cave, int y, int x), bool under);
int coords_to_dir(int y, int x);

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
void do_cmd_try_borg(void);
void do_cmd_quit(cmd_code code, cmd_arg args[]);
void do_cmd_xxx_options(void);
void do_cmd_unknown(void);
void do_cmd_suicide(cmd_code code, cmd_arg args[]);
void textui_cmd_suicide(void);
void do_cmd_save_game(cmd_code code, cmd_arg args[]);
void do_cmd_target(void);
void do_cmd_target_closest(void);
void do_cmd_load_screen(void);
void do_cmd_save_screen(void);
void do_cmd_version(void);
void do_cmd_pref(void);
void do_cmd_note(void);

/* cmd-obj.c */
void do_cmd_uninscribe(cmd_code code, cmd_arg args[]);
void do_cmd_inscribe(cmd_code code, cmd_arg args[]);
void do_cmd_takeoff(cmd_code code, cmd_arg args[]);
void wield_item(object_type *o_ptr, int item, int slot);
void do_cmd_wield(cmd_code code, cmd_arg args[]);
void do_cmd_drop(cmd_code code, cmd_arg args[]);
void do_cmd_destroy(cmd_code code, cmd_arg args[]);
void do_cmd_use(cmd_code code, cmd_arg args[]);
void do_cmd_refill(cmd_code code, cmd_arg args[]);
void do_cmd_study_spell(cmd_code code, cmd_arg args[]);
void do_cmd_cast(cmd_code code, cmd_arg args[]);
void do_cmd_study_book(cmd_code code, cmd_arg args[]);
void textui_cmd_destroy_menu(int item);
void textui_cmd_destroy(void);
void textui_cmd_toggle_ignore(void);
void textui_obj_examine(void);

/* cmd-pickup.c */
void do_cmd_pickup(cmd_code code, cmd_arg args[]);
void do_cmd_autopickup(cmd_code code, cmd_arg args[]);
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
extern void do_cmd_store(cmd_code code, cmd_arg args[]);
extern void do_cmd_sell(cmd_code code, cmd_arg args[]);
extern void do_cmd_stash(cmd_code code, cmd_arg args[]);
extern void do_cmd_buy(cmd_code code, cmd_arg args[]);
extern void do_cmd_retrieve(cmd_code code, cmd_arg args[]);

/* ui-spell.c -- just for now */
int get_spell(const object_type *o_ptr, const char *verb,
		bool (*spell_test)(int spell));
void textui_book_browse(const object_type *o_ptr);
void textui_spell_browse(void);
void textui_obj_study(void);
void textui_obj_cast(void);
int textui_obj_cast_ret(void);

/* ui-knowledge.c */
extern int big_pad(int col, int row, byte a, wchar_t c);
extern void textui_browse_object_knowledge(const char *name, int row);
extern void textui_knowledge_init(void);
extern void textui_browse_knowledge(void);

/* ui-option.c */
void do_cmd_options(void);

#endif
