/* monster/monster.h - monster interface */

#ifndef MONSTER_MONSTER_H
#define MONSTER_MONSTER_H

#include "cave.h"
#include "monster/types.h"
#include "player/types.h"

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
extern void player_place(struct cave *c, struct player *p, int y, int x);
extern s16b monster_place(int y, int x, monster_type *n_ptr);
extern bool place_monster_aux(struct cave *, int y, int x, int r_idx, bool slp, bool grp);
extern bool place_monster(struct cave *c, int y, int x, int depth, bool slp, bool grp);
extern bool alloc_monster(struct cave *c, int dis, bool slp, int depth);
extern bool summon_specific(int y1, int x1, int lev, int type, int delay);
extern bool multiply_monster(int m_idx);
extern void message_pain(int m_idx, int dam);
extern void update_smart_learn(int m_idx, int what);
void monster_death(int m_idx);
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note);
extern void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr, bitflag flags[RF_SIZE]);

extern void process_monsters(struct cave *c, byte min_energy);

#endif /* !MONSTER_MONSTER_H */
