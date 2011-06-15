#ifndef INCLUDED_CMDS_H
#define INCLUDED_CMDS_H

#include "game-cmd.h"

/* 
 * Command handlers will take a pointer to the command structure
 * so that they can access any arguments supplied.
 */
typedef void (*cmd_handler_fn)(cmd_code code, cmd_arg args[]);

/* cmd0.c */
extern void cmd_init(void);
unsigned char cmd_lookup_key(cmd_code cmd);
cmd_code cmd_lookup(unsigned char key);

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

/* cmd1.c */
int do_autopickup(void);

/* cmd2.c */
void do_cmd_go_up(cmd_code code, cmd_arg args[]);
void do_cmd_go_down(cmd_code code, cmd_arg args[]);
void do_cmd_search(cmd_code code, cmd_arg args[]);
void do_cmd_toggle_search(cmd_code code, cmd_arg args[]);
void do_cmd_open(cmd_code code, cmd_arg args[]);
void do_cmd_close(cmd_code code, cmd_arg args[]);
void do_cmd_tunnel(cmd_code code, cmd_arg args[]);
void do_cmd_disarm(cmd_code code, cmd_arg args[]);
void do_cmd_bash(cmd_code code, cmd_arg args[]);
void do_cmd_alter(cmd_code code, cmd_arg args[]);
void do_cmd_spike(cmd_code code, cmd_arg args[]);
void do_cmd_walk(cmd_code code, cmd_arg args[]);
void do_cmd_jump(cmd_code code, cmd_arg args[]);
void do_cmd_run(cmd_code code, cmd_arg args[]);
void do_cmd_pathfind(cmd_code code, cmd_arg args[]);
void do_cmd_hold(cmd_code code, cmd_arg args[]);
void do_cmd_pickup(cmd_code code, cmd_arg args[]);
void do_cmd_autopickup(cmd_code code, cmd_arg args[]);
void do_cmd_rest(cmd_code code, cmd_arg args[]);
void do_cmd_suicide(cmd_code code, cmd_arg args[]);
void do_cmd_save_game(cmd_code code, cmd_arg args[]);

void do_cmd_alter_aux(int dir);
void textui_cmd_rest(void);
void textui_cmd_suicide(void);

/* cmd3.c */
void do_cmd_inven(void);
void do_cmd_equip(void);
void textui_cmd_destroy(void);
void textui_cmd_toggle_ignore(void);
void textui_obj_examine(void);
void do_cmd_target(void);
void do_cmd_target_closest(void);
void do_cmd_look(void);
void do_cmd_locate(void);
void do_cmd_query_symbol(void);
void do_cmd_center_map(void);

/* cmd4.c */
extern void do_cmd_redraw(void);
extern void do_cmd_resize(void);
extern void do_cmd_change_name(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_options(void);
extern void do_cmd_options_birth(void);
extern void do_cmd_pref(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(void);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);

/* cmd-misc.c */
void do_cmd_wizard(void);
void do_cmd_try_debug(void);
void do_cmd_try_borg(void);
void do_cmd_quit(cmd_code code, cmd_arg args[]);
void do_cmd_xxx_options(void);
void do_cmd_monlist(void);
void do_cmd_itemlist(void);
void do_cmd_unknown(void);

/* attack.c */
extern void do_cmd_fire(cmd_code code, cmd_arg args[]);
extern void textui_cmd_fire_at_nearest(void);
extern void do_cmd_throw(cmd_code code, cmd_arg args[]);
extern void textui_cmd_throw(void);

/* store.c */
extern void do_cmd_store(cmd_code code, cmd_arg args[]);
extern void do_cmd_sell(cmd_code code, cmd_arg args[]);
extern void do_cmd_stash(cmd_code code, cmd_arg args[]);
extern void do_cmd_buy(cmd_code code, cmd_arg args[]);
extern void do_cmd_retrieve(cmd_code code, cmd_arg args[]);

/* Types of item use */
typedef enum
{
	USE_TIMEOUT,
	USE_CHARGE,
	USE_SINGLE
} use_type;

/* XXX */
extern int cmp_monsters(const void *a, const void *b);



/* ui-spell.c -- just for now */
void textui_book_browse(const object_type *o_ptr);
void textui_spell_browse(void);
void textui_obj_study(void);
void textui_obj_cast(void);

/* ui-knowledge.c */
extern int big_pad(int col, int row, byte a, byte c);
extern void textui_browse_object_knowledge(const char *name, int row);
extern void textui_knowledge_init(void);
extern void textui_browse_knowledge(void);

#endif

