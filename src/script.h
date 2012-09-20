/* File: script.h */

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#include "angband.h"


/*
 * Initalize the scripting support
 */
extern errr script_init(void);

/*
 * Free the resources for the scripting support
 */
extern errr script_free(void);

/*
 * Display the script debug menu
 */
extern void do_cmd_script(void);

/*
 * Execute a string of scripting code
 */
extern bool script_do_string(cptr script);

/*
 * Execute a file with scripting code
 */
extern bool script_do_file(cptr filename);

/*
 * Event handlers
 */
extern bool use_object(object_type *o_ptr, bool *ident);
extern int get_spell_index(const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern cptr get_spell_info(int tval, int index);
extern bool cast_spell(int tval, int index);
extern int get_store_choice(int store_num);
extern bool store_will_buy(int store_num, const object_type *o_ptr);
extern void player_birth_done_hook(void);
extern void player_calc_bonus_hook(void);
extern void start_game_hook(void);
extern void enter_level_hook(void);
extern void leave_level_hook(void);
extern void player_turn_hook(void);
extern bool process_command_hook(int command);
extern bool generate_level_hook(int level);
extern bool projection_hit_player(int who, int dam, int typ);

#endif /* INCLUDED_SCRIPT_H */
