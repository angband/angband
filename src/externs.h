#ifndef INCLUDED_EXTERNS_H
#define INCLUDED_EXTERNS_H

/*
 * Automatically generated "variable" declarations
 */

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
extern const byte blows_table[12][12];
extern const byte extract_energy[200];
extern const s32b player_exp[PY_MAX_LEVEL];
extern const player_sex sex_info[MAX_SEXES];
extern const byte chest_traps[64];
/*XYZ extern cptr color_names[BASIC_COLORS];*/
extern cptr stat_names[A_MAX];
extern cptr stat_names_reduced[A_MAX];
extern cptr stat_names_full[A_MAX];
extern const char *window_flag_desc[32];
extern const char *inscrip_text[];
extern const grouper object_text_order[];
extern const byte char_tables[256][CHAR_TABLE_SLOTS];
extern const xchar_type latin1_encode[];

/* variable.c */
extern cptr copyright;
extern bool arg_wizard;
extern bool arg_rebalance;
extern int arg_graphics;
extern bool arg_graphics_nice;
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
extern bool use_graphics_nice;
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
extern char savefile[1024];
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][16];
extern byte angband_color_table[MAX_COLORS][4];
extern color_type color_table[MAX_COLORS];
extern const cptr angband_sound_name[MSG_MAX];
extern int view_n;
extern u16b *view_g;
extern int temp_n;
extern u16b *temp_g;
extern byte *temp_y;
extern byte *temp_x;
extern byte (*cave_info)[256];
extern byte (*cave_feat)[DUNGEON_WID];
extern s16b (*cave_o_idx)[DUNGEON_WID];
extern s16b (*cave_m_idx)[DUNGEON_WID];
extern maxima *z_info;
extern object_type *o_list;
extern monster_type *mon_list;
extern s32b tot_mon_power;
extern monster_lore *l_list;
extern quest *q_list;
extern store_type *store;
extern int store_knowledge;
extern cptr** name_sections;
extern s16b alloc_ego_size;
extern alloc_entry *alloc_ego_table;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char macro_buffer[1024];
extern const player_sex *sp_ptr;
extern const player_race *rp_ptr;
extern const player_class *cp_ptr;
extern const player_magic *mp_ptr;
extern player_other *op_ptr;
extern player_type *p_ptr;
extern vault_type *v_info;
extern feature_type *f_info;
extern object_kind *k_info;
extern artifact_type *a_info;
extern ego_item_type *e_info;
extern monster_race *r_info;
extern player_race *p_info;
extern player_class *c_info;
extern hist_type *h_info;
extern owner_type *b_info;
extern byte *g_info;
extern flavor_type *flavor_info;
extern spell_type *s_info;
extern s16b spell_list[MAX_REALMS][BOOKS_PER_REALM][SPELLS_PER_BOOK];
extern struct hint *hints;

extern const char *ANGBAND_SYS;
extern const char *ANGBAND_GRAF;

extern char *ANGBAND_DIR_APEX;
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
extern int text_out_pad;
extern bool use_transparency;
extern void (*sound_hook)(int);
extern autoinscription *inscriptions;
extern u16b inscriptions_count;

extern flag_cache *slay_cache;

extern u16b daycount;

/* util.c */
extern cptr inkey_next;



/* birth.c */
extern void player_birth(bool quickstart_allowed);

/* cmd1.c */
extern bool search(bool verbose);
extern byte py_pickup(int pickup);
extern void move_player(int dir, bool disarm);

/* cmd2.c */
/* XXX should probably be moved to cave.c? */
bool is_open(int feat);
bool is_closed(int feat);
bool is_trap(int feat);
int count_feats(int *y, int *x, bool (*test)(int feat), bool under);
int count_chests(int *y, int *x, bool trapped);
int coords_to_dir(int y, int x);

/* death.c */
void death_screen(void);

/* dungeon.c */
extern void dungeon_change_level(int dlev);
extern void play_game(void);
extern int value_check_aux1(const object_type *o_ptr);
extern void idle_update(void);

/* melee1.c */
bool check_hit(int power, int level);
bool make_attack_normal(int m_idx);

/* melee2.c */
extern bool make_attack_spell(int m_idx);

/* pathfind.c */
extern bool findpath(int y, int x);
extern byte get_angle_to_grid[41][41];
extern int get_angle_to_target(int y0, int x0, int y1, int x1, int dir);
extern void get_grid_using_angle(int angle, int y0, int x0,
	int *ty, int *tx);
extern void run_step(int dir);

/* randart.c */
extern errr do_randart(u32b randart_seed, bool full);

/* randname.c */
extern size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***wordlist);

/* score.c */
extern void enter_score(time_t *death_time);
extern void show_scores(void);
extern void predict_score(void);


/* signals.c */
extern void signals_ignore_tstp(void);
extern void signals_handle_tstp(void);
extern void signals_init(void);

/* store.c */
void do_cmd_store_knowledge(void);

/* util.c */
extern void text_to_ascii(char *buf, size_t len, cptr str);
extern void ascii_to_text(char *buf, size_t len, cptr str);
extern char *find_roman_suffix_start(cptr buf);
extern int roman_to_int(const char *roman);
extern int int_to_roman(int n, char *roman, size_t bufsize);
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
extern char get_char(cptr prompt, const char *options, size_t len, char fallback);
extern bool get_check(cptr prompt);
extern bool (*get_file)(const char *suggested_name, char *path, size_t len);
extern bool get_com(cptr prompt, char *command);
extern bool get_com_ex(cptr prompt, ui_event_data *command);
extern void grid_data_as_text(grid_data *g, byte *ap, char *cp, byte *tap, char *tcp);
extern void pause_line(int row);
extern bool is_a_vowel(int ch);
extern int color_char_to_attr(char c);
extern int color_text_to_attr(cptr name);
extern cptr attr_to_text(byte a);

#ifdef SUPPORT_GAMMA
extern void build_gamma_table(int gamma);
extern byte gamma_table[256];
#endif /* SUPPORT_GAMMA */

/* x-char.c */
extern void xchar_trans_hook(char *s, int encoding);
extern void xstr_trans(char *str, int encoding);
extern void escape_latin1(char *dest, size_t max, cptr src);
extern const char seven_bit_translation[128];
extern char xchar_trans(byte c);

/* xtra2.c */
void check_experience(void);
void gain_exp(s32b amount);
void lose_exp(s32b amount);
bool modify_panel(term *t, int wy, int wx);
bool adjust_panel(int y, int x);
bool change_panel(int dir);
void verify_panel(void);
void center_panel(void);
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
char* random_hint(void);

/* wiz-spoil.c */
bool make_fake_artifact(object_type *o_ptr, byte name1);



/* borg.h */
#ifdef ALLOW_BORG
extern void do_cmd_borg(void);
#endif /* ALLOW_BORG */


extern u16b lazymove_delay;


#endif /* !INCLUDED_EXTERNS_H */

