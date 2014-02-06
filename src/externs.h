#ifndef INCLUDED_EXTERNS_H
#define INCLUDED_EXTERNS_H

#include "mon-constants.h"
#include "monster.h"
#include "player.h"
#include "store.h"
#include "trap.h"
#include "z-term.h"
#include "z-file.h"
#include "z-msg.h"
#include "spells.h"

/* This file was automatically generated. It is now obsolete (it was never a
 * good idea to begin with; you should include only what you use instead of
 * including everything everywhere) and is being slowly destroyed. Do not add
 * new entries to this file.
 */

/* tables.c */
extern const s16b ddd[9];
extern const s16b ddx[10];
extern const s16b ddy[10];
extern const s16b ddx_ddd[9];
extern const s16b ddy_ddd[9];
extern const byte extract_energy[200];
extern const s32b player_exp[PY_MAX_LEVEL];
extern const player_sex sex_info[MAX_SEXES];
extern const char *stat_names[A_MAX];
extern const char *stat_names_reduced[A_MAX];
extern const char *window_flag_desc[32];
extern const char *inscrip_text[];

/* variable.c */
extern const char *copyright;
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
extern s32b turn;
extern int use_graphics;
extern s16b signal_count;
extern bool msg_flag;
extern u32b inkey_scan;
extern bool inkey_flag;
extern s16b o_max;
extern s16b o_cnt;
extern char savefile[1024];
extern monster_lore *l_list;
extern struct store *stores;
extern int store_knowledge;
extern const char *** name_sections;
extern player_other *op_ptr;
extern player_type *player;
extern feature_type *f_info;
extern trap_kind *trap_info;
extern object_base *kb_info;
extern object_kind *k_info;
extern artifact_type *a_info;
extern ego_item_type *e_info;
extern monster_base *rb_info;
extern monster_race *r_info;
extern monster_pain *pain_messages;
extern struct player_race *races;
extern struct player_class *classes;
extern struct flavor *flavors;
extern struct vault *vaults;
extern struct object_kind *objkinds;
extern spell_type *s_info;
extern struct hint *hints;
extern struct pit_profile *pit_info;
extern u16b lazymove_delay;

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
extern char *ANGBAND_DIR_XTRA_ICON;

extern byte item_tester_tval;
extern bool (*item_tester_hook)(const object_type *);
extern bool use_transparency;
extern void (*sound_hook)(int);

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

/* store.c */
void do_cmd_store_knowledge(void);

/* xtra2.c */
bool modify_panel(term *t, int wy, int wx);
bool change_panel(int dir);
void verify_panel(void);
void center_panel(void);
int motion_dir(int y1, int x1, int y2, int x2);
int target_dir(struct keypress ch);
int target_dir_allow(struct keypress ch, bool allow_5);
bool get_rep_dir(int *dp, bool allow_5);

/* xtra3.c */
byte monster_health_attr(void);
void cnv_stat(int val, char *out_val, size_t out_len);
void toggle_inven_equip(void);
void subwindows_set_flags(u32b *new_flags, size_t n_subwindows);
char* random_hint(void);

/* wiz-spoil.c */
bool make_fake_artifact(object_type *o_ptr, struct artifact *artifact);

#endif /* !INCLUDED_EXTERNS_H */
