/* cave.h - cave interface */

#ifndef CAVE_H
#define CAVE_H

#include "defines.h"
#include "types.h"
#include "z-type.h"

struct player;
struct monster;

struct cave {
	s32b created_at;
	int depth;

	byte feeling;
	u32b obj_rating;
	u32b mon_rating;
	bool good_item;

	int height;
	int width;
	
	u16b feeling_squares; /* Keep track of how many feeling squares the player has visited */

	byte (*info)[256];
	byte (*info2)[256];
	byte (*feat)[DUNGEON_WID];
	byte (*cost)[DUNGEON_WID];
	byte (*when)[DUNGEON_WID];
	s16b (*m_idx)[DUNGEON_WID];
	s16b (*o_idx)[DUNGEON_WID];

	struct monster *monsters;
	int mon_max;
	int mon_cnt;
};

extern int distance(int y1, int x1, int y2, int x2);
extern bool los(int y1, int x1, int y2, int x2);
extern bool no_light(void);
extern bool cave_valid_bold(int y, int x);
extern byte get_color(byte a, int attr, int n);
extern void map_info(unsigned x, unsigned y, grid_data *g);
extern void move_cursor_relative(int y, int x);
extern void print_rel(wchar_t c, byte a, int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern void forget_view(struct cave *c);
extern void update_view(struct cave *c, struct player *p);
extern void map_area(void);
extern void wiz_light(bool full);
extern void wiz_dark(void);
extern int project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg);
extern bool projectable(int y1, int x1, int y2, int x2, int flg);
extern void scatter(int *yp, int *xp, int y, int x, int d, int m);
extern void disturb(struct player *p, int stop_search, int unused_flag);
extern bool is_quest(int level);
extern bool dtrap_edge(int y, int x);

#define CAVE_INFO_Y	DUNGEON_HGT
#define CAVE_INFO_X	256

/* XXX: temporary while I refactor */
extern struct cave *cave;

extern struct cave *cave_new(void);
extern void cave_free(struct cave *c);

extern void cave_set_feat(struct cave *c, int y, int x, int feat);
extern void cave_note_spot(struct cave *c, int y, int x);
extern void cave_light_spot(struct cave *c, int y, int x);
extern void cave_update_flow(struct cave *c);
extern void cave_forget_flow(struct cave *c);
extern void cave_illuminate(struct cave *c, bool daytime);

/**
 * cave_predicate is a function pointer which tests a given square to
 * see if the predicate in question is true.
 */
typedef bool (*cave_predicate)(struct cave *c, int y, int x);

/* FEATURE PREDICATES */
extern bool cave_isfloor(struct cave *c, int y, int x);
extern bool cave_isrock(struct cave *c, int y, int x);
extern bool cave_isperm(struct cave *c, int y, int x);
extern bool cave_ismagma(struct cave *c, int y, int x);
extern bool cave_isquartz(struct cave *c, int y, int x);
extern bool cave_ismineral(struct cave *c, int y, int x);
extern bool cave_hasgoldvein(struct cave *c, int y, int x);
extern bool feat_is_treasure(int feat);
extern bool cave_issecretdoor(struct cave *c, int y, int x);
extern bool cave_isopendoor(struct cave *c, int y, int x);
extern bool cave_iscloseddoor(struct cave *c, int y, int x);
extern bool cave_islockeddoor(struct cave *c, int y, int x);
extern bool cave_isjammeddoor(struct cave *c, int y, int x);
extern bool cave_isbrokendoor(struct cave *c, int y, int x);
extern bool cave_isdoor(struct cave *c, int y, int x);
extern bool cave_issecrettrap(struct cave *c, int y, int x);
extern bool feat_is_known_trap(int feat);
extern bool cave_isknowntrap(struct cave *c, int y, int x);
extern bool cave_istrap(struct cave *c, int y, int x);
extern bool feature_isshop(int feat);
extern bool cave_isstairs(struct cave *c, int y, int x);
extern bool cave_isupstairs(struct cave *c, int y, int x);
extern bool cave_isdownstairs(struct cave *c, int y, int x);
extern bool cave_isshop(struct cave *c, int y, int x);
extern bool cave_isglyph(struct cave *c, int y, int x);

/* BEHAVIOR PREDICATES */
extern bool cave_isopen(struct cave *c, int y, int x);
extern bool cave_isempty(struct cave *c, int y, int x);
extern bool cave_canputitem(struct cave *c, int y, int x);
extern bool cave_isdiggable(struct cave *c, int y, int x);
extern bool cave_ispassable(struct cave *c, int y, int x);
extern bool cave_iswall(struct cave *c, int y, int x);
extern bool cave_isstrongwall(struct cave *c, int y, int x);
extern bool cave_isvault(struct cave *c, int y, int x);
extern bool cave_isroom(struct cave *c, int y, int x);
extern bool cave_isrubble(struct cave *c, int y, int x);
extern bool cave_isfeel(struct cave *c, int y, int x);
extern bool feat_isboring(feature_type *f_ptr);
extern bool cave_isboring(struct cave *c, int y, int x);
extern bool cave_isview(struct cave *c, int y, int x);
extern bool cave_isseen(struct cave *c, int y, int x);
extern bool cave_wasseen(struct cave *c, int y, int x);
extern bool cave_isglow(struct cave *c, int y, int x);

extern void cave_generate(struct cave *c, struct player *p);

extern bool cave_in_bounds(struct cave *c, int y, int x);
extern bool cave_in_bounds_fully(struct cave *c, int y, int x);

extern struct monster *cave_monster(struct cave *c, int idx);
extern struct monster *cave_monster_at(struct cave *c, int y, int x);
extern int cave_monster_max(struct cave *c);
extern int cave_monster_count(struct cave *c);

void upgrade_mineral(struct cave *c, int y, int x);

void cave_jam_door(struct cave *c, int y, int x);
int cave_can_jam_door(struct cave *c, int y, int x);
int cave_door_power(struct cave *c, int y, int x);
void cave_open_door(struct cave *c, int y, int x);
void cave_close_door(struct cave *c, int y, int x);
void cave_smash_door(struct cave *c, int y, int x);
void cave_lock_door(struct cave *c, int y, int x, int power);

void cave_destroy_trap(struct cave *c, int y, int x);

void cave_tunnel_wall(struct cave *c, int y, int x);

#endif /* !CAVE_H */
