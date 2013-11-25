/* cave.h - cave interface */

#ifndef CAVE_H
#define CAVE_H

#include "defines.h"
#include "z-type.h"

struct player;
struct monster;

/*** Constants ***/


/*** Feature Indexes (see "lib/edit/terrain.txt") ***/

/* Nothing */
#define FEAT_NONE 0x00

/* Various */
#define FEAT_FLOOR 0x01
#define FEAT_INVIS 0x02
#define FEAT_GLYPH 0x03
#define FEAT_OPEN 0x04
#define FEAT_BROKEN 0x05
#define FEAT_LESS 0x06
#define FEAT_MORE 0x07

/* Shops */
#define FEAT_SHOP_HEAD 0x08
#define FEAT_SHOP_TAIL 0x0F

/* Traps */
#define FEAT_TRAP_HEAD 0x10
#define FEAT_TRAP_TAIL 0x1F

/* Doors */
#define FEAT_DOOR_HEAD 0x20
#define FEAT_DOOR_TAIL 0x2F

/* Secret door */
#define FEAT_SECRET 0x30

/* Rubble */
#define FEAT_RUBBLE 0x31

/* Mineral seams */
#define FEAT_MAGMA 0x32
#define FEAT_QUARTZ 0x33
#define FEAT_MAGMA_H 0x34
#define FEAT_QUARTZ_H 0x35
#define FEAT_MAGMA_K 0x36
#define FEAT_QUARTZ_K 0x37

/* Walls */
#define FEAT_WALL_EXTRA 0x38
#define FEAT_WALL_INNER 0x39
#define FEAT_WALL_OUTER 0x3A
#define FEAT_WALL_SOLID 0x3B
#define FEAT_PERM_EXTRA 0x3C
#define FEAT_PERM_INNER 0x3D
#define FEAT_PERM_OUTER 0x3E
#define FEAT_PERM_SOLID 0x3F



/*
 * Special cave grid flags
 */
#define CAVE_MARK		0x01 	/* memorized feature */
#define CAVE_GLOW		0x02 	/* self-illuminating */
#define CAVE_VAULT		0x04 	/* part of a vault */
#define CAVE_ROOM		0x08 	/* part of a room */
#define CAVE_SEEN		0x10 	/* seen flag */
#define CAVE_VIEW		0x20 	/* view flag */
#define CAVE_WASSEEN		0x40 	/* previously seen (during update) */
#define CAVE_WALL		0x80 	/* wall flag */

#define CAVE2_DTRAP		0x01	/* trap detected grid */
#define CAVE2_FEEL		0x02	/* hidden points to trigger feelings*/
#define CAVE2_DEDGE		0x04	/* border of trap detected area */
#define CAVE2_VERT		0x08	/* use an alternate visual for this grid */


/*
 * Terrain flags
 */
enum
{
	FF_NONE,
	FF_PWALK,
	FF_PPASS,
	FF_MWALK,
	FF_MPASS,
	FF_LOOK,
	FF_DIG,
	FF_DOOR,
	FF_EXIT_UP,
	FF_EXIT_DOWN,
	FF_PERM,
	FF_TRAP,
	FF_SHOP,
	FF_HIDDEN,
	FF_BORING,
	FF_MAX
};

#define FF_SIZE               FLAG_SIZE(FF_MAX)

#define ff_has(f, flag)        flag_has_dbg(f, FF_SIZE, flag, #f, #flag)
#define ff_next(f, flag)       flag_next(f, FF_SIZE, flag)
#define ff_is_empty(f)         flag_is_empty(f, FF_SIZE)
#define ff_is_full(f)          flag_is_full(f, FF_SIZE)
#define ff_is_inter(f1, f2)    flag_is_inter(f1, f2, FF_SIZE)
#define ff_is_subset(f1, f2)   flag_is_subset(f1, f2, FF_SIZE)
#define ff_is_equal(f1, f2)    flag_is_equal(f1, f2, FF_SIZE)
#define ff_on(f, flag)         flag_on_dbg(f, FF_SIZE, flag, #f, #flag)
#define ff_off(f, flag)        flag_off(f, FF_SIZE, flag)
#define ff_wipe(f)             flag_wipe(f, FF_SIZE)
#define ff_setall(f)           flag_setall(f, FF_SIZE)
#define ff_negate(f)           flag_negate(f, FF_SIZE)
#define ff_copy(f1, f2)        flag_copy(f1, f2, FF_SIZE)
#define ff_union(f1, f2)       flag_union(f1, f2, FF_SIZE)
#define ff_comp_union(f1, f2)  flag_comp_union(f1, f2, FF_SIZE)
#define ff_inter(f1, f2)       flag_inter(f1, f2, FF_SIZE)
#define ff_diff(f1, f2)        flag_diff(f1, f2, FF_SIZE)




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

	byte locked;   /**< How locked is it? */
	byte jammed;   /**< How jammed is it? */
	byte shopnum;  /**< Which shop does it take you to? */
	byte dig;      /**< How hard is it to dig through? */

	u32b effect;   /**< Effect on entry to grid */
	bitflag flags[FF_SIZE];    /**< Terrain flags */

	byte d_attr;   /**< Default feature attribute */
	wchar_t d_char;   /**< Default feature character */

	byte x_attr[4];   /**< Desired feature attribute (set by user/pref file) */
	wchar_t x_char[4];   /**< Desired feature character (set by user/pref file) */
} feature_type;

enum grid_light_level
{
	FEAT_LIGHTING_LOS = 0,   /* line of sight */
	FEAT_LIGHTING_TORCH,     /* torchlight */
	FEAT_LIGHTING_LIT,       /* permanently lit (when not in line of sight) */
	FEAT_LIGHTING_DARK,      /* dark */
	FEAT_LIGHTING_MAX
};

typedef struct
{
	u32b m_idx;		/* Monster index */
	u32b f_idx;		/* Feature index */
	struct object_kind *first_kind;	/* The "kind" of the first item on the grid */
	bool multiple_objects;	/* Is there more than one item there? */
	bool unseen_object;	/* Is there an unaware object there? */
	bool unseen_money; /* Is there some unaware money there? */

	enum grid_light_level lighting; /* Light level */
	bool in_view; /* TRUE when the player can currently see the grid. */
	bool is_player;
	bool hallucinate;
	bool trapborder;
} grid_data;



/** An array of 256 bytes */
typedef byte byte_256[256];

/** An array of DUNGEON_WID bytes */
typedef byte byte_wid[DUNGEON_WID];

/** An array of DUNGEON_WID s16b's */
typedef s16b s16b_wid[DUNGEON_WID];

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
extern void grid_data_as_text(grid_data *g, int *ap, wchar_t *cp, int *tap, wchar_t *tcp);
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
extern void scatter(int *yp, int *xp, int y, int x, int d, bool need_los);
extern void disturb(struct player *p, int stop_search, int unused_flag);
extern bool is_quest(int level);
extern bool dtrap_edge(int y, int x);

/* XXX: temporary while I refactor */
extern struct cave *cave;

extern struct cave *cave_new(void);
extern void cave_free(struct cave *c);

extern struct feature *cave_feat(struct cave *c, int y, int x);
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
extern bool feat_is_magma(int feat);
extern bool cave_ismagma(struct cave *c, int y, int x);
extern bool feat_is_quartz(int feat);
extern bool cave_isquartz(struct cave *c, int y, int x);
extern bool cave_ismineral(struct cave *c, int y, int x);
extern bool cave_hassecretvein(struct cave *c, int y, int x);
extern bool cave_hasgoldvein(struct cave *c, int y, int x);
extern bool feat_is_treasure(int feat);
extern bool cave_issecretdoor(struct cave *c, int y, int x);
extern bool cave_isopendoor(struct cave *c, int y, int x);
extern bool cave_iscloseddoor(struct cave *c, int y, int x);
extern bool cave_islockeddoor(struct cave *c, int y, int x);
extern bool cave_isbrokendoor(struct cave *c, int y, int x);
extern bool cave_isdoor(struct cave *c, int y, int x);
extern bool cave_issecrettrap(struct cave *c, int y, int x);
extern bool feat_is_known_trap(int feat);
extern bool feat_is_wall(int feat);
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
extern bool feat_ispassable(feature_type *f_ptr);
extern bool feat_is_monster_walkable(feature_type *feature);
extern bool cave_is_monster_walkable(struct cave *c, int y, int x);
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
extern bool cave_iswarded(struct cave *c, int y, int x);
extern bool cave_canward(struct cave *c, int y, int x);

extern bool cave_seemslikewall(struct cave *c, int y, int x);
/* interesting to memorize when mapping */
extern bool cave_isinteresting(struct cave *c, int y, int x);
/* noticeable when running */
extern bool cave_noticeable(struct cave *c, int y, int x);

/* Feature placers */
extern void cave_add_trap(struct cave *c, int y, int x);
extern void cave_add_ward(struct cave *c, int y, int x);
extern void cave_add_stairs(struct cave *c, int y, int x, int depth);
extern void cave_add_door(struct cave *c, int y, int x, bool closed);

extern void cave_remove_ward(struct cave *c, int y, int x);

extern void cave_generate(struct cave *c, struct player *p);

extern bool cave_in_bounds(struct cave *c, int y, int x);
extern bool cave_in_bounds_fully(struct cave *c, int y, int x);

extern struct monster *cave_monster(struct cave *c, int idx);
extern struct monster *cave_monster_at(struct cave *c, int y, int x);
extern int cave_monster_max(struct cave *c);
extern int cave_monster_count(struct cave *c);

void upgrade_mineral(struct cave *c, int y, int x);

/* Feature modifiers */
int cave_door_power(struct cave *c, int y, int x);
void cave_open_door(struct cave *c, int y, int x);
void cave_close_door(struct cave *c, int y, int x);
void cave_smash_door(struct cave *c, int y, int x);
void cave_lock_door(struct cave *c, int y, int x, int power);
void cave_unlock_door(struct cave *c, int y, int x);
void cave_destroy_door(struct cave *c, int y, int x);

void cave_show_trap(struct cave *c, int y, int x, int type);
void cave_destroy_trap(struct cave *c, int y, int x);

void cave_tunnel_wall(struct cave *c, int y, int x);
void cave_destroy_wall(struct cave *c, int y, int x);

void cave_show_vein(struct cave *c, int y, int x);

/* destroy this cell, as destruction spell */
void cave_destroy(struct cave *c, int y, int x);
void cave_earthquake(struct cave *c, int y, int x);

int cave_shopnum(struct cave *c, int y, int x);
const char *cave_apparent_name(struct cave *c, struct player *p, int y, int x);

void cave_destroy_rubble(struct cave *c, int y, int x);

void cave_force_floor(struct cave *c, int y, int x);

#endif /* !CAVE_H */
