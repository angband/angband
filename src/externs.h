/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-virt.h, z-util.h, z-form.h, term.h, random.h)
 */


/*
 * Automatically generated "variable" declarations
 */

/* tables.c */
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern char hexsym[16];
extern byte adj_val_min[];
extern byte adj_val_max[];
extern byte adj_mag_study[];
extern byte adj_mag_mana[];
extern byte adj_mag_fail[];
extern byte adj_mag_stat[];
extern byte adj_chr_gold[];
extern byte adj_int_dev[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte adj_con_mhp[];
extern byte blows_table[12][12];
extern owner_type owners[MAX_STORES][MAX_OWNERS];
extern byte extract_energy[200];
extern s32b player_exp[PY_MAX_LEVEL];
extern player_race race_info[MAX_RACES];
extern player_class class_info[MAX_CLASS];
extern player_magic magic_info[MAX_CLASS];
extern u32b spell_flags[2][9][2];
extern cptr spell_names[2][64];
extern byte chest_traps[64];
extern cptr player_title[MAX_CLASS][PY_MAX_LEVEL];
extern cptr color_names[16];
extern cptr sound_names[SOUND_MAX];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern option_type options[];

/* variable.c */
extern cptr copyright[5];
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte version_extra;
extern u32b sf_xtra;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern bool arg_wizard;
extern bool arg_fiddle;
extern bool arg_force_original;
extern bool arg_force_roguelike;
extern bool character_generated;
extern bool character_dungeon;
extern bool character_loaded;
extern bool character_saved;
extern bool character_icky;
extern bool character_xtra;
extern u32b seed_flavor;
extern u32b seed_town;
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_see;
extern s16b command_gap;
extern s16b command_wrk;
extern s16b command_new;
extern s16b energy_use;
extern s16b choose_default;
extern bool create_up_stair;
extern bool create_down_stair;
extern bool msg_flag;
extern bool alive;
extern bool death;
extern s16b running;
extern s16b resting;
extern s16b cur_hgt;
extern s16b cur_wid;
extern s16b dun_level;
extern s16b num_repro;
extern s16b object_level;
extern s16b monster_level;
extern s32b turn;
extern s32b old_turn;
extern bool wizard;
extern bool to_be_wizard;
extern bool can_be_wizard;
extern u16b total_winner;
extern u16b panic_save;
extern u16b noscore;
extern s16b signal_count;
extern bool inkey_base;
extern bool inkey_xtra;
extern bool inkey_scan;
extern bool inkey_flag;
extern s16b coin_type;
extern bool opening_chest;
extern bool use_graphics;
extern bool use_sound;
extern bool scan_monsters;
extern bool scan_objects;
extern s16b total_weight;
extern s16b inven_nxt;
extern s16b inven_cnt;
extern s16b equip_cnt;
extern s16b i_cnt;
extern s16b m_cnt;
extern s16b i_nxt;
extern s16b m_nxt;
extern s16b i_max;
extern s16b m_max;
extern bool rogue_like_commands;
extern bool quick_messages;
extern bool other_query_flag;
extern bool carry_query_flag;
extern bool always_pickup;
extern bool always_throw;
extern bool always_repeat;
extern bool use_old_target;
extern bool show_equip_label;
extern bool equippy_chars;
extern bool depth_in_feet;
extern bool notice_seams;
extern bool use_color;
extern bool compress_savefile;
extern bool hilite_player;
extern bool ring_bell;
extern bool find_ignore_stairs;
extern bool find_ignore_doors;
extern bool find_cut;
extern bool find_examine;
extern bool disturb_near;
extern bool disturb_move;
extern bool disturb_enter;
extern bool disturb_leave;
extern bool disturb_panel;
extern bool disturb_other;
extern bool flush_disturb;
extern bool flush_failure;
extern bool flush_command;
extern bool fresh_before;
extern bool fresh_after;
extern bool fresh_find;
extern bool filch_message;
extern bool filch_disturb;
extern bool alert_hitpoint;
extern bool alert_failure;
extern bool view_yellow_lite;
extern bool view_bright_lite;
extern bool view_perma_grids;
extern bool view_torch_grids;
extern bool flow_by_sound;
extern bool flow_by_smell;
extern bool track_follow;
extern bool track_target;
extern bool no_haggle_flag;
extern bool shuffle_owners;
extern bool show_inven_weight;
extern bool show_equip_weight;
extern bool show_store_weight;
extern bool plain_descriptions;
extern bool stack_allow_items;
extern bool stack_allow_wands;
extern bool stack_force_notes;
extern bool stack_force_costs;
extern bool auto_combine_pack;
extern bool auto_reorder_pack;
extern bool view_reduce_lite;
extern bool view_reduce_lite_town;
extern bool view_reduce_view;
extern bool view_reduce_view_town;
extern bool optimize_running;
extern bool optimize_resting;
extern bool optimize_display;
extern bool optimize_various;
extern bool scum_always;
extern bool scum_sometimes;
extern bool dungeon_align;
extern bool dungeon_stair;
extern bool show_spell_info;
extern bool show_health_bar;
extern bool smart_learn;
extern bool smart_cheat;
extern bool recall_show_desc;
extern bool recall_show_kill;
extern bool use_mirror_recent;
extern bool use_mirror_normal;
extern bool use_mirror_choose;
extern bool use_mirror_spells;
extern bool use_recall_recent;
extern bool use_choice_normal;
extern bool use_choice_choose;
extern bool use_choice_spells;
extern bool show_choose_info;
extern bool show_choose_prompt;
extern bool show_choose_weight;
extern bool show_choose_label;
extern bool cheat_peek;
extern bool cheat_hear;
extern bool cheat_room;
extern bool cheat_xtra;
extern bool cheat_know;
extern bool cheat_live;
extern s16b hitpoint_warn;
extern s16b delay_spd;
extern term *term_screen;
extern term *term_mirror;
extern term *term_recall;
extern term *term_choice;
extern s16b feeling;
extern s16b rating;
extern bool good_item_flag;
extern bool new_level_flag;
extern bool teleport_flag;
extern s16b teleport_dist;
extern s16b teleport_to_y;
extern s16b teleport_to_x;
extern bool closing_flag;
extern s16b max_panel_rows, max_panel_cols;
extern s16b panel_row, panel_col;
extern s16b panel_row_min, panel_row_max;
extern s16b panel_col_min, panel_col_max;
extern s16b panel_col_prt, panel_row_prt;
extern s16b py;
extern s16b px;
extern s16b target_who;
extern s16b target_col;
extern s16b target_row;
extern s16b health_who;
extern int player_uid;
extern int player_euid;
extern int player_egid;
extern char player_name[32];
extern char player_base[32];
extern char died_from[80];
extern char history[4][60];
extern char savefile[1024];
extern s16b lite_n;
extern byte lite_y[LITE_MAX];
extern byte lite_x[LITE_MAX];
extern s16b view_n;
extern byte view_y[VIEW_MAX];
extern byte view_x[VIEW_MAX];
extern s16b temp_n;
extern byte temp_y[TEMP_MAX];
extern byte temp_x[TEMP_MAX];
extern s16b macro__num;
extern cptr *macro__pat;
extern cptr *macro__act;
extern bool *macro__cmd;
extern char *macro__buf;
extern s16b quark__num;
extern cptr *quark__str;
extern u16b message__next;
extern u16b message__last;
extern u16b message__head;
extern u16b message__tail;
extern u16b *message__ptr;
extern char *message__buf;
extern cave_type *cave[MAX_HGT];
extern inven_type *i_list;
extern monster_type *m_list;
extern quest q_list[MAX_Q_IDX];
extern store_type *store;
extern inven_type *inventory;
extern s16b alloc_kind_size;
extern s16b *alloc_kind_index;
extern kind_entry *alloc_kind_table;
extern s16b alloc_race_size;
extern s16b *alloc_race_index;
extern race_entry *alloc_race_table;
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern byte keymap_cmds[256];
extern byte keymap_dirs[256];
extern byte color_table[256][4];
extern player_type *p_ptr;
extern player_race *rp_ptr;
extern player_class *cp_ptr;
extern player_magic *mp_ptr;
extern u32b spell_learned1;
extern u32b spell_learned2;
extern u32b spell_worked1;
extern u32b spell_worked2;
extern u32b spell_forgotten1;
extern u32b spell_forgotten2;
extern byte spell_order[64];
extern s16b player_hp[PY_MAX_LEVEL];
extern header *v_head;
extern vault_type *v_info;
extern char *v_name;
extern char *v_text;
extern header *f_head;
extern feature_type *f_info;
extern char *f_name;
extern char *f_text;
extern header *k_head;
extern inven_kind *k_info;
extern char *k_name;
extern char *k_text;
extern header *a_head;
extern artifact_type *a_info;
extern char *a_name;
extern char *a_text;
extern header *e_head;
extern ego_item_type *e_info;
extern char *e_name;
extern char *e_text;
extern header *r_head;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern cptr ANGBAND_SYS;
extern cptr ANGBAND_DIR;
extern cptr ANGBAND_DIR_APEX;
extern cptr ANGBAND_DIR_BONE;
extern cptr ANGBAND_DIR_DATA;
extern cptr ANGBAND_DIR_EDIT;
extern cptr ANGBAND_DIR_FILE;
extern cptr ANGBAND_DIR_HELP;
extern cptr ANGBAND_DIR_INFO;
extern cptr ANGBAND_DIR_SAVE;
extern cptr ANGBAND_DIR_USER;
extern cptr ANGBAND_DIR_XTRA;
extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(inven_type *i_ptr);
extern bool Rand_quick;
extern u32b Rand_value;
extern u16b Rand_place;
extern u32b Rand_state[RAND_DEG];
extern bool (*ang_sort_comp)(vptr u, vptr v, int a, int b);
extern void (*ang_sort_swap)(vptr u, vptr v, int a, int b);
extern bool (*get_mon_num_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);





/*
 * Automatically generated "function declarations"
 */

/* birth.c */
extern void player_birth(void);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int y1, int x1, int y2, int x2);
extern bool player_can_see_bold(int y, int x);
extern bool no_lite(void);
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void forget_lite(void);
extern void update_lite(void);
extern void forget_view(void);
extern void update_view(void);
extern void lite_room(int y1, int x1);
extern void unlite_room(int y1, int x1);
extern void forget_flow(void);
extern void update_flow(void);
extern void map_area(void);
extern void wiz_lite(void);
extern void wiz_dark(void);
extern void do_cmd_view_map(void);

/* cmd1.c */
extern bool is_a_vowel(int ch);
extern int index_to_label(int i);
extern int label_to_inven(int c);
extern int label_to_equip(int c);
extern cptr mention_use(int i);
extern cptr describe_use(int i);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern void inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool inven_carry_okay(inven_type *i_ptr);
extern s16b inven_carry(inven_type *i_ptr);
extern bool item_tester_okay(inven_type *i_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven(void);
extern void show_equip(void);
extern s16b wield_slot(inven_type *i_ptr);
extern bool get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor);

/* cmd2.c */
extern bool is_quest(int level);
extern void monster_death(int m_idx);
extern bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);
extern void py_attack(int y, int x);
extern void drop_near(inven_type *i_ptr, int chance, int y, int x);
extern void take_hit(int damage, cptr hit_from);
extern bool twall(int y, int x);
extern void ang_sort_swap_distance(vptr u, vptr v, int a, int b);
extern bool ang_sort_comp_distance(vptr u, vptr v, int a, int b);
extern bool target_able(int m_idx);
extern bool target_okay(void);
extern s16b target_pick(int y1, int x1, int dy, int dx);
extern bool target_set(void);
extern bool get_aim_dir(int *dp);
extern bool get_rep_dir(int *dp);
extern void pick_trap(int y, int x);
extern void search_on(void);
extern void search_off(void);
extern void disturb(int stop_search, int flush_output);
extern void search(void);
extern void panel_bounds(void);
extern void verify_panel(void);
extern void carry(int pickup);
extern void move_player(int dir, int do_pickup);
extern void do_cmd_run(void);
extern void do_cmd_fire(void);
extern void do_cmd_throw(void);

/* cmd3.c */
extern void do_cmd_look(void);
extern void do_cmd_examine(void);
extern void do_cmd_locate(void);
extern void do_cmd_open(void);
extern void do_cmd_close(void);
extern void do_cmd_tunnel(void);
extern void do_cmd_disarm(void);
extern void do_cmd_bash(void);
extern void do_cmd_spike(void);
extern void do_cmd_walk(int pickup);
extern void do_cmd_stay(int pickup);
extern void do_cmd_search(void);
extern void do_cmd_rest(void);
extern void do_cmd_feeling(void);
extern void do_cmd_uninscribe(void);
extern void do_cmd_inscribe(void);
extern void do_cmd_check_artifacts(void);
extern void do_cmd_check_uniques(void);

/* cmd4.c */
extern void do_cmd_go_up(void);
extern void do_cmd_go_down(void);
extern void do_cmd_suicide(void);
extern void do_cmd_redraw(void);
extern void do_cmd_change_name(void);
extern void do_cmd_toggle_search(void);
extern void do_cmd_refill(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_target(void);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colors(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_save_game(void);
extern void do_cmd_destroy(void);
extern void do_cmd_observe(void);
extern void do_cmd_toggle_choose(void);
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_drop(void);
extern void do_cmd_wield(void);
extern void do_cmd_takeoff(void);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);

/* cmd5.c */
extern void do_cmd_browse(void);
extern void do_cmd_study(void);
extern void do_cmd_cast(void);
extern void do_cmd_pray(void);

/* cmd6.c */
extern void process_command(void);
extern void request_command(void);

/* dungeon.c */
extern void play_game(bool new_game);

/* effects.c */
extern void do_cmd_eat_food(void);
extern void do_cmd_quaff_potion(void);
extern void do_cmd_read_scroll(void);
extern void do_cmd_aim_wand(void);
extern void do_cmd_use_staff(void);
extern void do_cmd_zap_rod(void);
extern void do_cmd_activate(void);

/* files.c */
extern void safe_setuid_drop(void);
extern void safe_setuid_grab(void);
extern s16b tokenize(char *buf, s16b num, char **tokens);
extern errr file_character(cptr name, bool full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern errr check_time_init(void);
extern errr check_load_init(void);
extern errr check_time(void);
extern errr check_load(void);
extern void read_times(void);
extern void show_news(void);
extern void do_cmd_help(cptr name);
extern void process_player_name(bool sf);
extern void get_name(void);
extern long total_points(void);
extern void display_scores(int from, int to);
extern void close_game(void);
extern void exit_game_panic(void);

/* generate.c */
extern void generate_cave(void);

/* init.c */
extern void init_file_paths(char *path);
extern void init_some_arrays(void);

/* io.c */
extern void flush(void);
extern void bell(void);
extern void sound(int num);
extern void move_cursor(int row, int col);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern void keymap_init(void);
extern void macro_add(cptr pat, cptr act, bool cmd_flag);
extern char inkey(void);
extern cptr quark_str(s16b num);
extern s16b quark_add(cptr str);
extern s16b message_num(void);
extern cptr message_str(s16b age);
extern void message_add(cptr msg);
extern void msg_print(cptr msg);
extern void msg_format(cptr fmt, ...);
extern void clear_screen(void);
extern void clear_from(int row);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern bool askfor_aux(char *buf, int len);
extern bool askfor(char *buf, int len);
extern bool get_check(cptr prompt);
extern bool get_com(cptr prompt, char *command);
extern s16b get_quantity(cptr prompt, int max);
extern void pause_line(int row);

/* melee.c */
extern bool make_attack_normal(int m_idx);
extern bool make_attack_spell(int m_idx);

/* misc.c */
extern void Rand_state_init(u32b seed);
extern s32b Rand_mod(s32b m);
extern s32b Rand_div(s32b m);
extern s16b randnor(int mean, int stand);
extern s16b damroll(int num, int sides);
extern s16b maxroll(int num, int sides);
extern void ang_sort_aux(vptr u, vptr v, int p, int q);
extern void ang_sort(vptr u, vptr v, int n);
extern void scatter(int *yp, int *xp, int y, int x, int d, int m);
extern bool enter_wiz_mode(void);
extern void extract_cur_view(void);
extern void extract_cur_lite(void);
extern s16b modify_stat_value(int value, int amount);
extern bool inc_stat(int stat);
extern bool dec_stat(int stat, int amount, int permanent);
extern bool res_stat(int stat);
extern void cnv_stat(int val, char *out_val);
extern cptr likert(int x, int y);
extern void display_player(bool do_hist);
extern void health_redraw(void);
extern void health_track(int m_idx);
extern void recent_track(int r_idx);
extern void recent_fix(void);
extern void choose_fix(void);
extern void check_experience(void);
extern void gain_exp(s32b amount);
extern void lose_exp(s32b amount);
extern s16b spell_chance(int spell);
extern void handle_stuff(void);
extern bool set_blind(int v);
extern bool set_confused(int v);
extern bool set_poisoned(int v);
extern bool set_afraid(int v);
extern bool set_paralyzed(int v);
extern bool set_image(int v);
extern bool set_fast(int v);
extern bool set_slow(int v);
extern bool set_shield(int v);
extern bool set_blessed(int v);
extern bool set_hero(int v);
extern bool set_shero(int v);
extern bool set_protevil(int v);
extern bool set_invuln(int v);
extern bool set_tim_invis(int v);
extern bool set_tim_infra(int v);
extern bool set_oppose_acid(int v);
extern bool set_oppose_elec(int v);
extern bool set_oppose_fire(int v);
extern bool set_oppose_cold(int v);
extern bool set_oppose_pois(int v);
extern bool set_stun(int v);
extern bool set_cut(int v);
extern bool set_food(int v);

/* mon-desc.c */
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);
extern void do_cmd_query_symbol(void);

/* monster.c */
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void reorder_monsters(void);
extern void wipe_m_list(void);
extern s16b m_pop(void);
extern s16b get_mon_num(int level);
extern void monster_desc(char *desc, monster_type *m_ptr, int mode);
extern void lore_do_probe(int m_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool dist);
extern void update_monsters(bool dist);
extern bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp);
extern bool place_monster(int y, int x, bool slp, bool grp);
extern bool alloc_ghost(void);
extern bool alloc_monster(int dis, int slp);
extern bool summon_specific(int y1, int x1, int lev, int type);
extern bool multiply_monster(int m_idx);
extern void process_monsters(void);

/* obj-desc.c */
extern void flavor_init(void);
extern void reset_visuals(void);
extern void objdes(char *buf, inven_type *i_ptr, int pref, int mode);
extern void objdes_store(char *buf, inven_type *i_ptr, int pref, int mode);

/* object.c */
extern void delete_object_idx(int i);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void reorder_objects(void);
extern void wipe_i_list(void);
extern s16b i_pop(void);
extern s16b get_obj_num(int level);
extern void inven_known(inven_type *i_ptr);
extern void inven_aware(inven_type *i_ptr);
extern void inven_tried(inven_type *i_ptr);
extern s32b item_value(inven_type *i_ptr);
extern bool item_similar(inven_type *i_ptr, inven_type *j_ptr);
extern void item_absorb(inven_type *i_ptr, inven_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void invwipe(inven_type *i_ptr);
extern void invcopy(inven_type *i_ptr, int k_idx);
extern void inven_flags(inven_type *i_ptr, u32b *f1, u32b *f2, u32b *f3);
extern void apply_magic(inven_type *i_ptr, int lev, bool okay, bool good, bool great);
extern void place_object(int y, int x, bool good, bool great);
extern void acquirement(int y1, int x1, int num, bool great);
extern void place_trap(int y, int x);
extern void place_gold(int y, int x);
extern void process_objects(void);

/* save-old.c */
extern bool load_player(void);

/* save.c */
extern bool save_player(void);

/* signals.c */
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);

/* spells1.c */
extern byte spell_color(int type);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool projectable(int y1, int x1, int y2, int x2);
extern bool apply_disenchant(int mode);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg);
extern bool fire_bolt(int typ, int dir, int dam);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool lite_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool heal_monster(int dir);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir);
extern bool confuse_monster(int dir, int plev);
extern bool fear_monster(int dir, int plev);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(void);
extern bool destroy_doors_touch(void);
extern bool sleep_monsters_touch(void);
extern bool speed_monsters(void);
extern bool slow_monsters(void);
extern bool sleep_monsters(void);
extern bool lite_area(int dam, int rad);
extern bool unlite_area(int dam, int rad);

/* spells2.c */
extern void aggravate_monsters(int who);
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern bool genocide(void);
extern bool mass_genocide(void);
extern bool hp_player(int num);
extern bool banish_evil(int dist);
extern bool probing(void);
extern bool dispel_evil(int dam);
extern bool dispel_undead(int dam);
extern bool dispel_monsters(int dam);
extern bool turn_undead(void);
extern void warding_glyph(void);
extern bool do_dec_stat(int stat);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern void message_pain(int m_idx, int dam);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
extern void self_knowledge(void);
extern void tele_level(void);
extern bool lose_all_info(void);
extern bool detect_treasure(void);
extern bool detect_magic(void);
extern bool detect_invisible(void);
extern bool detect_evil(void);
extern bool detect_monsters(void);
extern bool detection(void);
extern bool detect_object(void);
extern bool detect_trap(void);
extern bool detect_sdoor(void);
extern void stair_creation(void);
extern cptr item_activation(inven_type *i_ptr);
extern bool identify_fully_aux(inven_type *i_ptr);
extern bool enchant(inven_type *i_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern bool ident_spell(void);
extern bool identify_fully(void);
extern bool recharge(int num);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);

/* store.c */
extern void store_enter(int which);
extern void store_shuffle(void);
extern void store_maint(void);
extern void store_init(void);

/* util.c */
extern void delay(int t);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fputs(FILE *fff, cptr buf, huge n);
extern errr my_fclose(FILE *fff);
extern int fd_open(cptr file, int flags, int mode);
extern errr fd_lock(int fd, int what);
extern errr fd_seek(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);

/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

#ifndef HAS_MEMSET
/* util.c */
extern char *memset(char*, int, huge);
#endif

#ifndef HAS_STRICMP
/* util.c */
extern int stricmp(cptr a, cptr b);
#endif

#ifdef MACINTOSH
/* main-mac.c */
/* extern void delay(int x); */
/* extern void main(void); */
#endif

#ifdef WINDOWS
/* main-win.c */
/* extern void delay(int x); */
/* extern int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, ...); */
#endif

#ifdef ACORN
/* main-acn.c */
/* extern void delay(int x); */
#endif

