/* monster/monster.h - monster interface */

#ifndef MONSTER_MONSTER_H
#define MONSTER_MONSTER_H

#include "cave.h"
#include "monster/types.h"
#include "player/types.h"

/* monster1.c */
extern bool mon_inc_timed(int m_idx, int idx, int v, u16b flag);
extern bool mon_dec_timed(int m_idx, int idx, int v, u16b flag);
extern bool mon_clear_timed(int m_idx, int idx, u16b flag);
extern void cheat_monster_lore(int r_idx, monster_lore *l_ptr);
extern void describe_monster(int r_idx, bool spoilers);
extern void roff_top(int r_idx);
extern void screen_roff(int r_idx);
extern void display_roff(int r_idx);
extern int lookup_monster(const char *name);
extern int rval_find_idx(const char *name);
extern const char *rval_find_name(int rval);
extern int lookup_monster_base(const char *name);

/* monster2.c */
extern void plural_aux(char *name, size_t max);
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_mon_list(struct cave *c, struct player *p);
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
extern void player_place(struct cave *c, struct player *p, int y, int x);
extern s16b monster_place(int y, int x, monster_type *n_ptr);
extern bool place_monster_aux(struct cave *, int y, int x, int r_idx, bool slp, bool grp);
extern bool place_monster(struct cave *c, int y, int x, int depth, bool slp, bool grp);
extern bool alloc_monster(struct cave *c, struct loc loc, int dis, bool slp, int depth);
extern bool summon_specific(int y1, int x1, int lev, int type, int delay);
extern bool multiply_monster(int m_idx);
extern void message_pain(int m_idx, int dam);
extern bool add_monster_message(char *mon_name, int m_idx, int msg_code);
extern void flush_monster_messages(void);
extern void update_smart_learn(int m_idx, int what);
void monster_death(int m_idx);
bool mon_take_hit(int m_idx, int dam, bool *fear, const char *note);
extern void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr, bitflag flags[RF_SIZE]);

extern void process_monsters(struct cave *c, byte min_energy);



/* The codified monster messages */
enum {
	MON_MSG_NONE = 0,

	/* project_m */
	MON_MSG_DIE,
	MON_MSG_DESTROYED,
	MON_MSG_RESIST_A_LOT,
	MON_MSG_HIT_HARD,
	MON_MSG_RESIST,
	MON_MSG_IMMUNE,
	MON_MSG_RESIST_SOMEWHAT,
	MON_MSG_UNAFFECTED,
	MON_MSG_SPAWN,
	MON_MSG_HEALTHIER,
	MON_MSG_FALL_ASLEEP,
	MON_MSG_WAKES_UP,
	MON_MSG_CRINGE_LIGHT,
	MON_MSG_SHRIVEL_LIGHT,
	MON_MSG_LOSE_SKIN,
	MON_MSG_DISSOLVE,
	MON_MSG_CATCH_FIRE,
	MON_MSG_BADLY_FROZEN,
	MON_MSG_SHUDDER,
	MON_MSG_CHANGE,
	MON_MSG_DISAPPEAR,
	MON_MSG_MORE_DAZED,
	MON_MSG_DAZED,
	MON_MSG_NOT_DAZED,
	MON_MSG_MORE_CONFUSED,
	MON_MSG_CONFUSED,
	MON_MSG_NOT_CONFUSED,
	MON_MSG_MORE_SLOWED,
	MON_MSG_SLOWED,
	MON_SNG_NOT_SLOWED,
	MON_MSG_MORE_HASTED,
	MON_MSG_HASTED,
	MON_MSG_NOT_HASTED,
	MON_MSG_MORE_AFRAID,
	MON_MSG_FLEE_IN_TERROR,
	MON_MSG_NOT_AFRAID,
	MON_MSG_MORIA_DEATH,
	MON_MSG_DISENTEGRATES,
	MON_MSG_FREEZE_SHATTER,
	MON_MSG_MANA_DRAIN,
	MON_MSG_MAINTAIN_SHAPE,
	MON_MSG_BRIEF_PUZZLE,
	
	/* message_pain */
	MON_MSG_UNHARMED,
	MON_MSG_95,
	MON_MSG_75,
	MON_MSG_50,
	MON_MSG_35,
	MON_MSG_20,
	MON_MSG_10,
	MON_MSG_0,

	/* Always leave this at the end */
	MAX_MON_MSG
};


/* Maxinum number of stacked monster messages */
#define MAX_STORED_MON_MSG		200
#define MAX_STORED_MON_CODES	400


/* Flags for the monster timed functions */

#define MON_TMD_FLG_SEEN		0x0001	/* Monster is seen */
#define MON_TMD_FLG_NOTIFY		0x0002	/* Give notification */
#define MON_TMD_MON_SOURCE		0x0004  /* Monster is causing the damage */
#define MON_TMD_FLG_NOMESSAGE	0x0010  /*  Never show a message */
#define MON_TMD_FLG_NOFAIL		0x0020	/*  Never Fail the tests. */
/* XXX */





#endif /* !MONSTER_MONSTER_H */
