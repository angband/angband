#ifndef INCLUDED_EXTERNS_H
#define INCLUDED_EXTERNS_H

/*
 * Automatically generated "variable" declarations
 */

extern int max_macrotrigger;
extern char *macro_template;
extern char *macro_modifier_chr;
extern char *macro_modifier_name[MAX_MACRO_MOD];
extern char *macro_trigger_name[MAX_MACRO_TRIGGER];
extern char *macro_trigger_keycode[2][MAX_MACRO_TRIGGER];

/* pathfind.c */
extern char pf_result[];
extern int pf_result_index;

/* tables.c */
extern const s16b ddd[9];
extern const s16b ddx[10];
extern const s16b ddy[10];
extern const s16b ddx_ddd[9];
extern const s16b ddy_ddd[9];
extern const char hexsym[16];
extern const int adj_mag_study[];
extern const int adj_mag_mana[];
extern const byte adj_mag_fail[];
extern const int adj_mag_stat[];
extern const byte adj_chr_gold[];
extern const byte adj_int_dev[];
extern const byte adj_wis_sav[];
extern const byte adj_dex_dis[];
extern const byte adj_int_dis[];
extern const byte adj_dex_ta[];
extern const byte adj_str_td[];
extern const byte adj_dex_th[];
extern const byte adj_str_th[];
extern const byte adj_str_wgt[];
extern const byte adj_str_hold[];
extern const byte adj_str_dig[];
extern const byte adj_str_blow[];
extern const byte adj_dex_blow[];
extern const byte adj_dex_safe[];
extern const byte adj_con_fix[];
extern const int adj_con_mhp[];
extern const byte blows_table[12][12];
extern const byte extract_energy[200];
extern const s32b player_exp[PY_MAX_LEVEL];
extern const player_sex sex_info[MAX_SEXES];
extern const byte chest_traps[64];
extern cptr color_names[BASIC_COLORS];
extern cptr stat_names[A_MAX];
extern cptr stat_names_reduced[A_MAX];
extern cptr stat_names_full[A_MAX];
extern const char *window_flag_desc[32];
extern const char *inscrip_text[];
extern const grouper object_text_order[];

/* variable.c */
extern cptr copyright;
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte version_extra;
extern byte sf_major;
extern byte sf_minor;
extern byte sf_patch;
extern byte sf_extra;
extern bool arg_wizard;
extern bool arg_rebalance;
extern int arg_graphics;
extern bool character_generated;
extern bool character_existed;
extern bool character_dungeon;
extern bool character_saved;
extern s16b character_icky;
extern s16b character_xtra;
extern u32b seed_randart;
extern u32b seed_flavor;
extern u32b seed_town;
extern s16b num_repro;
extern char summon_kin_type;
extern s32b turn;
extern s32b old_turn;
extern int use_graphics;
extern bool use_bigtile;
extern s16b signal_count;
extern bool msg_flag;
extern bool inkey_base;
extern bool inkey_xtra;
extern u32b inkey_scan;
extern bool inkey_flag;
extern bool opening_chest;
extern bool shimmer_monsters;
extern bool shimmer_objects;
extern bool repair_mflag_nice;
extern bool repair_mflag_show;
extern bool repair_mflag_mark;
extern s16b o_max;
extern s16b o_cnt;
extern s16b mon_max;
extern s16b mon_cnt;
extern byte feeling;
extern s16b rating;
extern bool good_item_flag;
extern bool closing_flag;
extern char savefile[1024];
extern char panic_savefile[1024];
extern s16b macro__num;
extern char **macro__pat;
extern char **macro__act;
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][16];
extern byte angband_color_table[MAX_COLORS][4];
extern const cptr angband_sound_name[MSG_MAX];
extern int view_n;
extern u16b *view_g;
extern int temp_n;
extern u16b *temp_g;
extern byte *temp_y;
extern byte *temp_x;
extern byte (*cave_info)[256];
extern byte (*cave_info2)[256];
extern byte (*cave_feat)[DUNGEON_WID];
extern s16b (*cave_o_idx)[DUNGEON_WID];
extern s16b (*cave_m_idx)[DUNGEON_WID];
extern byte (*cave_cost)[DUNGEON_WID];
extern byte (*cave_when)[DUNGEON_WID];
extern maxima *z_info;
extern object_type *o_list;
extern monster_type *mon_list;
extern s32b tot_mon_power;
extern monster_lore *l_list;
extern quest *q_list;
extern store_type *store;
extern object_type *inventory;
extern s16b alloc_ego_size;
extern alloc_entry *alloc_ego_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char macro_buffer[1024];
extern char *keymap_act[KEYMAP_MODES][256];
extern const player_sex *sp_ptr;
extern const player_race *rp_ptr;
extern const player_class *cp_ptr;
extern const player_magic *mp_ptr;
extern player_other *op_ptr;
extern player_type *p_ptr;
extern vault_type *v_info;
extern char *v_name;
extern char *v_text;
extern feature_type *f_info;
extern char *f_name;
extern char *f_text;
extern object_kind *k_info;
extern char *k_name;
extern char *k_text;
extern artifact_type *a_info;
extern char *a_name;
extern char *a_text;
extern ego_item_type *e_info;
extern char *e_name;
extern char *e_text;
extern monster_race *r_info;
extern char *r_name;
extern char *r_text;
extern player_race *p_info;
extern char *p_name;
extern char *p_text;
extern player_class *c_info;
extern char *c_name;
extern char *c_text;
extern hist_type *h_info;
extern char *h_text;
extern owner_type *b_info;
extern char *b_name;
extern char *b_text;
extern byte *g_info;
extern char *g_name;
extern char *g_text;
extern flavor_type *flavor_info;
extern char *flavor_name;
extern char *flavor_text;
extern spell_type *s_info;
extern char *s_name;
extern char *s_text;
extern s16b spell_list[MAX_REALMS][BOOKS_PER_REALM][SPELLS_PER_BOOK];

extern const char *ANGBAND_SYS;
extern const char *ANGBAND_GRAF;

extern char *ANGBAND_DIR;
extern char *ANGBAND_DIR_APEX;
extern char *ANGBAND_DIR_BONE;
extern char *ANGBAND_DIR_DATA;
extern char *ANGBAND_DIR_EDIT;
extern char *ANGBAND_DIR_FILE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_PREF;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_XTRA;

extern char *ANGBAND_DIR_XTRA_FONT;
extern char *ANGBAND_DIR_XTRA_GRAF;
extern char *ANGBAND_DIR_XTRA_SOUND;
extern char *ANGBAND_DIR_XTRA_HELP;
extern char *ANGBAND_DIR_XTRA_ICON;

extern bool item_tester_full;
extern byte item_tester_tval;
extern bool (*item_tester_hook)(const object_type*);
extern bool (*get_mon_num_hook)(int r_idx);
extern bool (*get_obj_num_hook)(int k_idx);
extern ang_file *text_out_file;
extern void (*text_out_hook)(byte a, cptr str);
extern int text_out_wrap;
extern int text_out_indent;
extern bool use_transparency;
extern void (*sound_hook)(int);
extern autoinscription *inscriptions;
extern u16b inscriptions_count;

extern flag_cache *slay_cache;

/* history.c */
extern history_info *history_list;

/* squelch.c */
extern byte squelch_level[];
const size_t squelch_size;


/*
 * Automatically generated "function declarations"
 */

/* attack.c */
extern int breakage_chance(const object_type *o_ptr);
extern bool test_hit(int chance, int ac, int vis);
extern void py_attack(int y, int x);

/* birth.c */
extern void player_birth(bool quickstart_allowed);

/* button.c */
int button_add_text(const char *label, unsigned char keypress);
int button_add(const char *label, unsigned char keypress);
void button_backup_all(void);
void button_restore(void);
int button_kill_text(unsigned char keypress);
int button_kill(unsigned char keypress);
void button_kill_all(void);
void button_init(button_add_f add, button_kill_f kill);
char button_get_key(int x, int y);
size_t button_print(int row, int col);

/* cave.c */
extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int y1, int x1, int y2, int x2);
extern bool no_lite(void);
extern bool cave_valid_bold(int y, int x);
extern bool feat_supports_lighting(int feat);
extern void map_info(unsigned x, unsigned y, grid_data *g);
extern void move_cursor_relative(int y, int x);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern errr vinfo_init(void);
extern void forget_view(void);
extern void update_view(void);
extern void forget_flow(void);
extern void update_flow(void);
extern void map_area(void);
extern void wiz_lite(void);
extern void wiz_dark(void);
extern void town_illuminate(bool daytime);
extern void cave_set_feat(int y, int x, int feat);
extern int project_path(u16b *gp, int range, \
                         int y1, int x1, int y2, int x2, int flg);
extern bool projectable(int y1, int x1, int y2, int x2, int flg);
extern void scatter(int *yp, int *xp, int y, int x, int d, int m);
extern void health_track(int m_idx);
extern void monster_race_track(int r_idx);
extern void track_object(int item);
extern void track_object_kind(int k_idx);
extern void disturb(int stop_search, int unused_flag);
extern bool is_quest(int level);
extern bool dtrap_edge(int y, int x);

/* cmd1.c */
extern void search(void);
extern byte py_pickup(int pickup);
extern void move_player(int dir);

/* cmd5.c */
s16b spell_chance(int spell);
bool spell_okay(int spell, bool known, bool browse);
bool spell_cast(int spell, int dir);
void spell_learn(int spell);

int get_spell(const object_type *o_ptr, cptr prompt, bool known, bool browse);
void do_cmd_browse_aux(const object_type *o_ptr, int item);

/* death.c */
void death_screen(void);

/* dungeon.c */
extern void dungeon_change_level(int dlev);
extern void play_game(void);
extern int value_check_aux1(const object_type *o_ptr);

/* files.c */
extern void html_screenshot(cptr name, int mode);
extern void player_flags(u32b f[OBJ_FLAG_N]);
extern void display_player(int mode);
extern void display_player_stat_info(void);
extern void display_player_xtra_info(void);
extern errr file_character(cptr name, bool full);
extern bool show_file(cptr name, cptr what, int line, int mode);
extern void do_cmd_help(void);
extern void process_player_name(bool sf);
extern bool get_name(char *buf, size_t buflen);
extern void save_game(void);
extern void close_game(void);
extern void exit_game_panic(void);

/* generate.c */
void place_object(int y, int x, int level, bool good, bool great);
void place_gold(int y, int x, int level);
void place_secret_door(int y, int x);
void place_closed_door(int y, int x);
void place_random_door(int y, int x);
extern void generate_cave(void);

/* history.c */
void history_clear(void);
size_t history_get_num(void);
bool history_add_full(u16b type, byte a_idx, s16b dlev, s16b clev, s32b turn, const char *text);
bool history_add(const char *event, u16b type, byte a_idx);
bool history_add_artifact(byte a_idx, bool known);
void history_unmask_unknown(void);
bool history_lose_artifact(byte a_idx);
void history_display(void);
void dump_history(ang_file *file);

/* init2.c */
extern void init_file_paths(const char *path);
extern void create_user_dirs(void);
extern bool init_angband(void);
extern void cleanup_angband(void);

/* load.c */
extern bool old_load(void);

/* melee1.c */
bool check_hit(int power, int level);
bool make_attack_normal(int m_idx);

/* melee2.c */
extern bool make_attack_spell(int m_idx);
extern void process_monsters(byte minimum_energy);

/* monster1.c */
extern void describe_monster(int r_idx, bool spoilers);
extern void roff_top(int r_idx);
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);
extern int lookup_monster(const char *name);

/* monster2.c */
extern bool wake_monster(monster_type *m_ptr);
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_mon_list(void);
extern s16b mon_pop(void);
extern void get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void display_monlist(void);
extern void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode);
extern void lore_do_probe(int m_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool full);
extern void update_monsters(bool full);
extern s16b monster_carry(int m_idx, object_type *j_ptr);
extern void monster_swap(int y1, int x1, int y2, int x2);
extern s16b player_place(int y, int x);
extern s16b monster_place(int y, int x, monster_type *n_ptr);
extern bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp);
extern bool place_monster(int y, int x, int depth, bool slp, bool grp);
extern bool alloc_monster(int dis, bool slp, int depth);
extern bool summon_specific(int y1, int x1, int lev, int type);
extern bool multiply_monster(int m_idx);
extern void message_pain(int m_idx, int dam);
extern void update_smart_learn(int m_idx, int what);
void monster_death(int m_idx);
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);

/* obj-util.c */
extern void display_itemlist(void);
extern void display_object_idx_recall(s16b o_idx);
extern void display_object_kind_recall(s16b k_idx);

/* pathfind.c */
extern bool findpath(int y, int x);
extern byte get_angle_to_grid[41][41];
extern int get_angle_to_target(int y0, int x0, int y1, int x1, int dir);
extern void get_grid_using_angle(int angle, int y0, int x0,
	int *ty, int *tx);
extern void run_step(int dir);

/* prefs.c */
void autoinsc_dump(ang_file *fff);
void squelch_dump(ang_file *fff);
void option_dump(ang_file *fff);
void macro_dump(ang_file *fff);
void keymap_dump(ang_file *fff);
void dump_monsters(ang_file *fff);
void dump_objects(ang_file *fff);
void dump_features(ang_file *fff);
void dump_flavors(ang_file *fff);
void dump_colors(ang_file *fff);
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title);
s16b tokenize(char *buf, s16b num, char **tokens);
errr process_pref_file_command(char *buf);
errr process_pref_file(cptr name);

/* randart.c */
extern errr do_randart(u32b randart_seed, bool full);

/* score.c */
extern void enter_score(time_t *death_time);
extern void show_scores(void);
extern void predict_score(void);


/* signals.c */
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);

/* save.c */
extern bool old_save(void);

/* spells1.c */
extern s16b poly_r_idx(int r_idx);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int dis);
extern void teleport_player_to(int ny, int nx);
extern void teleport_player_level(void);
extern void take_hit(int dam, cptr kb_str);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool inc_stat(int stat);
extern bool dec_stat(int stat, bool permanent);
extern bool res_stat(int stat);
extern bool apply_disenchant(int mode);
extern bool project(int who, int rad, int y, int x, int dam, int typ, int flg);

/* spells2.c */
extern bool hp_player(int num);
extern bool heal_player(int perc, int min);
extern void warding_glyph(void);
extern bool do_dec_stat(int stat, bool perma);
extern bool do_res_stat(int stat);
extern bool do_inc_stat(int stat);
extern void identify_pack(void);
extern bool remove_curse(void);
extern bool remove_all_curse(void);
extern bool restore_level(void);
extern void self_knowledge(bool spoil);
extern bool lose_all_info(void);
extern void set_recall(void);
extern bool detect_traps(bool aware);
extern bool detect_doorstairs(bool aware);
extern bool detect_treasure(bool aware);
extern bool detect_objects_magic(bool aware);
extern bool detect_monsters_normal(bool aware);
extern bool detect_monsters_invis(bool aware);
extern bool detect_monsters_evil(bool aware);
extern bool detect_all(bool aware);
extern void stair_creation(void);
extern bool enchant(object_type *o_ptr, int n, int eflag);
extern bool enchant_spell(int num_hit, int num_dam, int num_ac);
extern void do_ident_item(int item, object_type *o_ptr);
extern bool ident_spell(void);
extern bool recharge(int num);
extern bool speed_monsters(void);
extern bool slow_monsters(void);
extern bool confuse_monsters(void);
extern bool sleep_monsters(void);
extern bool banish_evil(int dist);
extern bool turn_undead(void);
extern bool dispel_undead(int dam);
extern bool dispel_evil(int dam);
extern bool dispel_monsters(int dam);
extern void aggravate_monsters(int who);
extern bool banishment(void);
extern bool mass_banishment(void);
extern bool probing(void);
extern void destroy_area(int y1, int x1, int r, bool full);
extern void earthquake(int cy, int cx, int r);
extern void lite_room(int y1, int x1);
extern void unlite_room(int y1, int x1);
extern bool lite_area(int dam, int rad);
extern bool unlite_area(int dam, int rad);
extern bool fire_ball(int typ, int dir, int dam, int rad);
extern bool fire_swarm(int num, int typ, int dir, int dam, int rad);
extern bool fire_bolt(int typ, int dir, int dam);
extern bool fire_beam(int typ, int dir, int dam);
extern bool fire_bolt_or_beam(int prob, int typ, int dir, int dam);
extern bool project_los(int typ, int dam);
extern bool lite_line(int dir);
extern bool strong_lite_line(int dir);
extern bool drain_life(int dir, int dam);
extern bool wall_to_mud(int dir);
extern bool destroy_door(int dir);
extern bool disarm_trap(int dir);
extern bool heal_monster(int dir);
extern bool speed_monster(int dir);
extern bool slow_monster(int dir);
extern bool sleep_monster(int dir);
extern bool confuse_monster(int dir, int plev);
extern bool poly_monster(int dir);
extern bool clone_monster(int dir);
extern bool fear_monster(int dir, int plev);
extern bool teleport_monster(int dir);
extern bool door_creation(void);
extern bool trap_creation(void);
extern bool destroy_doors_touch(void);
extern bool sleep_monsters_touch(void);
extern bool curse_armor(void);
extern bool curse_weapon(void);
extern void brand_object(object_type *o_ptr, byte brand_type);
extern void brand_weapon(void);
extern bool brand_ammo(void);
extern bool brand_bolts(void);
extern void ring_of_power(int dir);

/* squelch.c */
void squelch_init(void);
void squelch_birth_init(void);
int get_autoinscription_index(s16b k_idx);
const char *get_autoinscription(s16b kind_idx);
int apply_autoinscription(object_type *o_ptr);
int remove_autoinscription(s16b kind);
int add_autoinscription(s16b kind, cptr inscription);
void autoinscribe_ground(void);
void autoinscribe_pack(void);

bool squelch_interactive(const object_type *o_ptr);
void ignore_artifact(const object_type *o_ptr);

void kind_squelch_when_aware(object_kind *k_ptr);
void kind_squelch_when_unaware(object_kind *k_ptr);
bool kind_is_squelched_aware(const object_kind *k_ptr);
bool kind_is_squelched_unaware(const object_kind *k_ptr);

bool squelch_tval(int tval);
bool squelch_item_ok(const object_type *o_ptr);
bool squelch_hide_item(object_type *o_ptr);
void squelch_drop(void);
void squelch_items(void);
void do_cmd_options_item(void *, cptr);

/* store.c */
s32b price_item(const object_type *o_ptr, bool store_buying, int qty);
void store_init(void);
void store_shuffle(int which);
void store_maint(int which);

/* target.c */
bool target_able(int m_idx);
bool target_okay(void);
void target_set_monster(int m_idx);
void target_set_location(int y, int x);
bool target_set_interactive(int mode, int x, int y);
bool get_aim_dir(int *dp);
void target_get(s16b *col, s16b *row);
s16b target_get_monster(void);

/* trap.c */
extern void hit_trap(int y, int x);
extern void pick_trap(int y, int x);
extern void place_trap(int y, int x);

/* util.c */
extern void text_to_ascii(char *buf, size_t len, cptr str);
extern void ascii_to_text(char *buf, size_t len, cptr str);
extern char *find_roman_suffix_start(cptr buf);
extern int roman_to_int(const char *roman);
extern int int_to_roman(int n, char *roman, size_t bufsize);
extern int macro_find_exact(cptr pat);
extern errr macro_add(cptr pat, cptr act);
extern errr macro_init(void);
extern errr macro_free(void);
extern errr macro_trigger_free(void);
extern void flush(void);
extern void flush_fail(void);
extern char inkey(void);
extern ui_event_data inkey_ex(void);
extern char anykey(void);
extern void bell(cptr reason);
extern void sound(int val);
extern void msg_print(cptr msg);
extern void msg_format(cptr fmt, ...);
extern void message(u16b message_type, s16b extra, cptr message);
extern void message_format(u16b message_type, s16b extra, cptr fmt, ...);
extern void message_flush(void);
extern void screen_save(void);
extern void screen_load(void);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void prt(cptr str, int row, int col);
extern void text_out_to_file(byte attr, cptr str);
extern void text_out_to_screen(byte a, cptr str);
extern void text_out(const char *fmt, ...);
extern void text_out_c(byte a, const char *fmt, ...);
extern void text_out_e(const char *fmt, ...);
extern void clear_from(int row);
extern bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime);
extern bool askfor_aux(char *buf, size_t len, bool keypress_h(char *, size_t, size_t *, size_t *, char, bool));
extern bool get_string(cptr prompt, char *buf, size_t len);
extern s16b get_quantity(cptr prompt, int max);
extern bool get_check(cptr prompt);
extern bool (*get_file)(const char *suggested_name, char *path, size_t len);
extern bool get_com(cptr prompt, char *command);
extern bool get_com_ex(cptr prompt, ui_event_data *command);
extern void grid_data_as_text(grid_data *g, byte *ap, char *cp, byte *tap, char *tcp);
extern void pause_line(int row);
extern void request_command(void);
extern bool is_a_vowel(int ch);
extern int color_char_to_attr(char c);
extern int color_text_to_attr(cptr name);
extern cptr attr_to_text(byte a);

#ifdef SUPPORT_GAMMA
extern void build_gamma_table(int gamma);
extern byte gamma_table[256];
#endif /* SUPPORT_GAMMA */

/* x-spell.c */
extern int get_spell_index(const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index, int dir);
extern bool spell_needs_aim(int tval, int spell);

/* xtra2.c */
void check_experience(void);
void gain_exp(s32b amount);
void lose_exp(s32b amount);
bool modify_panel(term *t, int wy, int wx);
bool adjust_panel(int y, int x);
bool change_panel(int dir);
void verify_panel(void);
int motion_dir(int y1, int x1, int y2, int x2);
int target_dir(char ch);
bool get_rep_dir(int *dp);
bool confuse_dir(int *dp);

/* xtra3.c */
byte player_hp_attr(void);
byte player_sp_attr(void);
byte monster_health_attr(void);
void cnv_stat(int val, char *out_val, size_t out_len);
void toggle_inven_equip(void);
void subwindows_set_flags(u32b *new_flags, size_t n_subwindows);

/* wiz-spoil.c */
bool make_fake_artifact(object_type *o_ptr, byte name1);



/* borg.h */
#ifdef ALLOW_BORG
extern void do_cmd_borg(void);
#endif /* ALLOW_BORG */


extern u16b lazymove_delay;


#endif /* !INCLUDED_EXTERNS_H */

