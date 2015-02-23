/**
 * \file cave.h
 * \brief Matters relating to the current dungeon level
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef CAVE_H
#define CAVE_H

#include "z-type.h"
#include "z-bitflag.h"

struct player;
struct monster;

const s16b ddd[9];
const s16b ddx[10];
const s16b ddy[10];
const s16b ddx_ddd[9];
const s16b ddy_ddd[9];

/**
 * Square flags
 */

enum
{
	#define SQUARE(a,b) SQUARE_##a,
	#include "list-square-flags.h"
	#undef SQUARE
	SQUARE_MAX
};

#define SQUARE_SIZE                FLAG_SIZE(SQUARE_MAX)

#define sqinfo_has(f, flag)        flag_has_dbg(f, SQUARE_SIZE, flag, #f, #flag)
#define sqinfo_next(f, flag)       flag_next(f, SQUARE_SIZE, flag)
#define sqinfo_is_empty(f)         flag_is_empty(f, SQUARE_SIZE)
#define sqinfo_is_full(f)          flag_is_full(f, SQUARE_SIZE)
#define sqinfo_is_inter(f1, f2)    flag_is_inter(f1, f2, SQUARE_SIZE)
#define sqinfo_is_subset(f1, f2)   flag_is_subset(f1, f2, SQUARE_SIZE)
#define sqinfo_is_equal(f1, f2)    flag_is_equal(f1, f2, SQUARE_SIZE)
#define sqinfo_on(f, flag)         flag_on_dbg(f, SQUARE_SIZE, flag, #f, #flag)
#define sqinfo_off(f, flag)        flag_off(f, SQUARE_SIZE, flag)
#define sqinfo_wipe(f)             flag_wipe(f, SQUARE_SIZE)
#define sqinfo_setall(f)           flag_setall(f, SQUARE_SIZE)
#define sqinfo_negate(f)           flag_negate(f, SQUARE_SIZE)
#define sqinfo_copy(f1, f2)        flag_copy(f1, f2, SQUARE_SIZE)
#define sqinfo_union(f1, f2)       flag_union(f1, f2, SQUARE_SIZE)
#define sqinfo_comp_union(f1, f2)  flag_comp_union(f1, f2, SQUARE_SIZE)
#define sqinfo_inter(f1, f2)       flag_inter(f1, f2, SQUARE_SIZE)
#define sqinfo_diff(f1, f2)        flag_diff(f1, f2, SQUARE_SIZE)


/**
 * Terrain flags
 */
enum
{
	#define TF(a,b) TF_##a,
	#include "list-terrain-flags.h"
	#undef TF
	TF_MAX
};

#define TF_SIZE                FLAG_SIZE(TF_MAX)

#define tf_has(f, flag)        flag_has_dbg(f, TF_SIZE, flag, #f, #flag)

/**
 * Information about terrain features.
 *
 * At the moment this isn't very much, but eventually a primitive flag-based
 * information system will be used here.
 */
typedef struct feature
{
	char *name;
	int fidx;

	struct feature *next;

	byte mimic;    /**< Feature to mimic */
	byte priority; /**< Display priority */

	byte shopnum;  /**< Which shop does it take you to? */
	byte dig;      /**< How hard is it to dig through? */

	bitflag flags[TF_SIZE];    /**< Terrain flags */

	byte d_attr;   /**< Default feature attribute */
	wchar_t d_char;/**< Default feature character */
} feature_type;

extern feature_type *f_info;

enum grid_light_level
{
	LIGHTING_LOS = 0,   /* line of sight */
	LIGHTING_TORCH,     /* torchlight */
	LIGHTING_LIT,       /* permanently lit (when not in line of sight) */
	LIGHTING_DARK,      /* dark */
	LIGHTING_MAX
};

typedef struct
{
	u32b m_idx;				/* Monster index */
	u32b f_idx;				/* Feature index */
	struct object_kind *first_kind;	/* The kind of the first item on the grid */
	struct trap *trap;		/* Trap */
	bool multiple_objects;	/* Is there more than one item there? */
	bool unseen_object;		/* Is there an unaware object there? */
	bool unseen_money;		/* Is there some unaware money there? */

	enum grid_light_level lighting; /* Light level */
	bool in_view; 			/* Can the player can currently see the grid? */
	bool is_player;
	bool hallucinate;
	bool trapborder;
} grid_data;

struct square {
	byte feat;
	bitflag *info;
	byte cost;
	byte when;
	s16b mon;
	struct object *obj;
	struct trap *trap;
};

struct chunk {
	char *name;
	s32b created_at;
	int depth;

	byte feeling;
	u32b obj_rating;
	u32b mon_rating;
	bool good_item;

	int height;
	int width;
	
	u16b feeling_squares; /* How many feeling squares the player has visited */
	int *feat_count;

	byte **feat;
	s16b **m_idx;

	struct square **squares;

	struct monster *monsters;
	u16b mon_max;
	u16b mon_cnt;
	int mon_current;
};

/*** Feature Indexes (see "lib/edit/terrain.txt") ***/

/* Nothing */
int FEAT_NONE;

/* Various */
int FEAT_FLOOR;
int FEAT_CLOSED;
int FEAT_OPEN;
int FEAT_BROKEN;
int FEAT_LESS;
int FEAT_MORE;

/* Secret door */
int FEAT_SECRET;

/* Rubble */
int FEAT_RUBBLE;

/* Mineral seams */
int FEAT_MAGMA;
int FEAT_QUARTZ;
int FEAT_MAGMA_K;
int FEAT_QUARTZ_K;

/* Walls */
int FEAT_GRANITE;
int FEAT_PERM;

/* Special trap detect features  - should be replaced with square flags */
int FEAT_DTRAP_FLOOR;
int FEAT_DTRAP_WALL;


/* Real cave */
struct chunk *cave;
/* Known cave */
struct chunk *cave_k;
struct chunk **chunk_list;
u16b chunk_list_max;

/* cave-view.c */
int distance(int y1, int x1, int y2, int x2);
bool los(struct chunk *c, int y1, int x1, int y2, int x2);
void forget_view(struct chunk *c);
void update_view(struct chunk *c, struct player *p);
bool player_has_los_bold(int y, int x);
bool player_can_see_bold(int y, int x);
bool no_light(void);

/* cave-map.c */
void map_info(unsigned x, unsigned y, grid_data *g);
void square_note_spot(struct chunk *c, int y, int x);
void square_light_spot(struct chunk *c, int y, int x);
void light_room(int y1, int x1, bool light);
void wiz_light(struct chunk *c, bool full);
void wiz_dark(void);
void cave_illuminate(struct chunk *c, bool daytime);
void cave_update_flow(struct chunk *c);
void cave_forget_flow(struct chunk *c);

/* cave-square.c */
/**
 * square_predicate is a function pointer which tests a given square to
 * see if the predicate in question is true.
 */
typedef bool (*square_predicate)(struct chunk *c, int y, int x);

/* FEATURE PREDICATES */
bool feat_is_magma(int feat);
bool feat_is_quartz(int feat);
bool feat_is_treasure(int feat);
bool feat_is_wall(int feat);
bool feat_is_monster_walkable(int feat);
bool feat_is_shop(int feat);
bool feat_is_passable(int feat);
bool feat_is_projectable(int feat);

/* SQUARE FEATURE PREDICATES */
bool square_isfloor(struct chunk *c, int y, int x);
bool square_isrock(struct chunk *c, int y, int x);
bool square_isperm(struct chunk *c, int y, int x);
bool square_ismagma(struct chunk *c, int y, int x);
bool square_isquartz(struct chunk *c, int y, int x);
bool square_ismineral(struct chunk *c, int y, int x);
bool square_hasgoldvein(struct chunk *c, int y, int x);
bool square_isrubble(struct chunk *c, int y, int x);
bool square_issecretdoor(struct chunk *c, int y, int x);
bool square_isopendoor(struct chunk *c, int y, int x);
bool square_iscloseddoor(struct chunk *c, int y, int x);
bool square_islockeddoor(struct chunk *c, int y, int x);
bool square_isbrokendoor(struct chunk *c, int y, int x);
bool square_isdoor(struct chunk *c, int y, int x);
bool square_isstairs(struct chunk *c, int y, int x);
bool square_isupstairs(struct chunk *c, int y, int x);
bool square_isdownstairs(struct chunk *c, int y, int x);
bool square_isshop(struct chunk *c, int y, int x);
bool square_isplayer(struct chunk *c, int y, int x);

/* SQUARE INFO PREDICATES */
bool square_ismark(struct chunk *c, int y, int x);
bool square_isglow(struct chunk *c, int y, int x);
bool square_isvault(struct chunk *c, int y, int x);
bool square_isroom(struct chunk *c, int y, int x);
bool square_isseen(struct chunk *c, int y, int x);
bool square_isview(struct chunk *c, int y, int x);
bool square_wasseen(struct chunk *c, int y, int x);
bool square_isdtrap(struct chunk *c, int y, int x);
bool square_isfeel(struct chunk *c, int y, int x);
bool square_isdedge(struct chunk *c, int y, int x);
bool square_istrap(struct chunk *c, int y, int x);
bool square_isinvis(struct chunk *c, int y, int x);
bool square_iswall_inner(struct chunk *c, int y, int x);
bool square_iswall_outer(struct chunk *c, int y, int x);
bool square_iswall_solid(struct chunk *c, int y, int x);
bool square_ismon_restrict(struct chunk *c, int y, int x);
bool square_isno_teleport(struct chunk *c, int y, int x);
bool square_isno_map(struct chunk *c, int y, int x);
bool square_isno_esp(struct chunk *c, int y, int x);
bool square_isproject(struct chunk *c, int y, int x);

/* SQUARE BEHAVIOR PREDICATES */
bool square_isopen(struct chunk *c, int y, int x);
bool square_isempty(struct chunk *c, int y, int x);
bool square_canputitem(struct chunk *c, int y, int x);
bool square_isdiggable(struct chunk *c, int y, int x);
bool square_is_monster_walkable(struct chunk *c, int y, int x);
bool square_ispassable(struct chunk *c, int y, int x);
bool square_isprojectable(struct chunk *c, int y, int x);
bool square_iswall(struct chunk *c, int y, int x);
bool square_isstrongwall(struct chunk *c, int y, int x);
bool square_iswarded(struct chunk *c, int y, int x);
bool square_canward(struct chunk *c, int y, int x);
bool square_seemslikewall(struct chunk *c, int y, int x);
bool square_isinteresting(struct chunk *c, int y, int x);
bool square_isplayertrap(struct chunk *c, int y, int x);
bool square_isvisibletrap(struct chunk *c, int y, int x);
bool square_issecrettrap(struct chunk *c, int y, int x);
bool square_isknowntrap(struct chunk *c, int y, int x);
bool square_changeable(struct chunk *c, int y, int x);
bool square_dtrap_edge(struct chunk *c, int y, int x);
bool square_in_bounds(struct chunk *c, int y, int x);
bool square_in_bounds_fully(struct chunk *c, int y, int x);

struct feature *square_feat(struct chunk *c, int y, int x);
struct monster *square_monster(struct chunk *c, int y, int x);
struct object *square_object(struct chunk *c, int y, int x);
bool square_holds_object(struct chunk *c, int y, int x, struct object *obj);
void square_excise_object(struct chunk *c, int y, int x, struct object *obj);

void square_set_feat(struct chunk *c, int y, int x, int feat);

/* Feature placers */
void square_add_trap(struct chunk *c, int y, int x);
void square_add_ward(struct chunk *c, int y, int x);
void square_add_stairs(struct chunk *c, int y, int x, int depth);
void square_add_door(struct chunk *c, int y, int x, bool closed);

/* Feature modifiers */
void square_open_door(struct chunk *c, int y, int x);
void square_close_door(struct chunk *c, int y, int x);
void square_smash_door(struct chunk *c, int y, int x);
void square_lock_door(struct chunk *c, int y, int x, int power);
void square_unlock_door(struct chunk *c, int y, int x);
void square_destroy_door(struct chunk *c, int y, int x);
void square_show_trap(struct chunk *c, int y, int x, int type);
void square_destroy_trap(struct chunk *c, int y, int x);
void square_tunnel_wall(struct chunk *c, int y, int x);
void square_destroy_wall(struct chunk *c, int y, int x);
void square_destroy(struct chunk *c, int y, int x);
void square_earthquake(struct chunk *c, int y, int x);
void square_remove_ward(struct chunk *c, int y, int x);
void square_upgrade_mineral(struct chunk *c, int y, int x);
void square_destroy_rubble(struct chunk *c, int y, int x);
void square_force_floor(struct chunk *c, int y, int x);


int square_shopnum(struct chunk *c, int y, int x);
int square_digging(struct chunk *c, int y, int x);
const char *square_apparent_name(struct chunk *c, struct player *p, int y, int x);

/* cave.c */
void set_terrain(void);
struct chunk *cave_new(int height, int width);
void cave_free(struct chunk *c);
void scatter(struct chunk *c, int *yp, int *xp, int y, int x, int d, bool need_los);

struct monster *cave_monster(struct chunk *c, int idx);
int cave_monster_max(struct chunk *c);
int cave_monster_count(struct chunk *c);

int count_feats(int *y, int *x, bool (*test)(struct chunk *cave, int y, int x), bool under);

void cave_generate(struct chunk **c, struct player *p);
bool is_quest(int level);

void cave_known(void);

#endif /* !CAVE_H */
