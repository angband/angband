/* cave.h - cave interface */

#ifndef CAVE_H
#define CAVE_H

#include "z-type.h"

struct player;
struct monster;

/*** Constants ***/

/*
 * Maximum dungeon level.  The player can never reach this level
 * in the dungeon, and this value is used for various calculations
 * involving object and monster creation.  It must be at least 100.
 * Setting it below 128 may prevent the creation of some objects.
 */
#define MAX_DEPTH	128

/*
 * Max number of grids in each dungeon (vertically)
 * Must be less or equal to 256
 */
#define DUNGEON_HGT		66

/*
 * Max number of grids in each dungeon (horizontally)
 * Must be less or equal to 256
 */
#define DUNGEON_WID		198

#define TOWN_WID 66
#define TOWN_HGT 22


/*
 * Maximum sight and projection values
 */
#define MAX_SIGHT_LGE   20      /* Maximum view distance */
#define MAX_RANGE_LGE   20      /* Maximum projection range */
#define MAX_SIGHT_SML   10      /* Maximum view distance (small devices) */
#define MAX_RANGE_SML   10      /* Maximum projection range (small devices) */
#define MAX_SIGHT (OPT(birth_small_range) ? MAX_SIGHT_SML : MAX_SIGHT_LGE)  
#define MAX_RANGE (OPT(birth_small_range) ? MAX_RANGE_SML : MAX_RANGE_LGE)


/* 
 * Information for Feelings 
 */
#define FEELING_TOTAL	100		/* total number of feeling squares per level */ 
#define FEELING1		10		/* Squares needed to see to get first feeling */



/*** Feature Indexes (see "lib/edit/terrain.txt") ***/

/* Nothing */
#define FEAT_NONE 0x00

/* Various */
#define FEAT_FLOOR 0x01
#define FEAT_INVIS 0x02
#define FEAT_OPEN 0x04
#define FEAT_BROKEN 0x05
#define FEAT_LESS 0x06
#define FEAT_MORE 0x07

/* Shops */
#define FEAT_SHOP_HEAD 0x08
#define FEAT_SHOP_TAIL 0x0F

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
#define FEAT_GRANITE 0x38
#define FEAT_PERM 0x39

/* Special trap detect features  - should be replaced with square flags */
#define FEAT_DTRAP_FLOOR 0x40
#define FEAT_DTRAP_WALL 0x41



/*
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


/*
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

	byte locked;   /**< How locked is it? */
	byte jammed;   /**< How jammed is it? */
	byte shopnum;  /**< Which shop does it take you to? */
	byte dig;      /**< How hard is it to dig through? */

	bitflag flags[TF_SIZE];    /**< Terrain flags */

	byte d_attr;   /**< Default feature attribute */
	wchar_t d_char;   /**< Default feature character */

	byte x_attr[4];   /**< Desired feature attribute (set by user/pref file) */
	wchar_t x_char[4];   /**< Desired feature character (set by user/pref file) */
} feature_type;

extern feature_type *f_info;

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
    u32b trap;          /* Trap index */
	bool multiple_objects;	/* Is there more than one item there? */
	bool unseen_object;	/* Is there an unaware object there? */
	bool unseen_money; /* Is there some unaware money there? */

	enum grid_light_level lighting; /* Light level */
	bool in_view; /* TRUE when the player can currently see the grid. */
	bool is_player;
	bool hallucinate;
	bool trapborder;
} grid_data;



struct cave {
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

	bitflag ***info;
	byte **feat;
	byte **cost;
	byte **when;
	s16b **m_idx;
	s16b **o_idx;

	struct monster *monsters;
	int mon_max;
	int mon_cnt;

	struct trap_type *traps;
	s16b trap_max;
};

extern int distance(int y1, int x1, int y2, int x2);
extern bool los(struct cave *c, int y1, int x1, int y2, int x2);
extern bool no_light(void);
extern bool square_valid_bold(int y, int x);
extern byte get_color(byte a, int attr, int n);
extern void map_info(unsigned x, unsigned y, grid_data *g);
extern void grid_data_as_text(grid_data *g, int *ap, wchar_t *cp, int *tap, wchar_t *tcp);
extern void move_cursor_relative(int y, int x);
extern void print_rel(wchar_t c, byte a, int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern void forget_view(struct cave *c);
extern bool player_has_los_bold(int y, int x);
extern bool player_can_see_bold(int y, int x);
extern void update_view(struct cave *c, struct player *p);
extern void map_area(void);
extern void wiz_light(struct cave *c, bool full);
extern void wiz_dark(void);
extern int project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg);
extern bool projectable(struct cave *c, int y1, int x1, int y2, int x2, int flg);
extern void scatter(struct cave *c, int *yp, int *xp, int y, int x, int d, bool need_los);
extern void disturb(struct player *p, int stop_search, int unused_flag);
extern bool is_quest(int level);
extern bool dtrap_edge(int y, int x);

/* XXX: temporary while I refactor */
extern struct cave *cave;

extern struct cave *cave_new(void);
extern void cave_free(struct cave *c);

extern struct feature *square_feat(struct cave *c, int y, int x);
extern void square_set_feat(struct cave *c, int y, int x, int feat);
extern void square_note_spot(struct cave *c, int y, int x);
extern void square_light_spot(struct cave *c, int y, int x);
extern void cave_update_flow(struct cave *c);
extern void cave_forget_flow(struct cave *c);
extern void cave_illuminate(struct cave *c, bool daytime);

/**
 * square_predicate is a function pointer which tests a given square to
 * see if the predicate in question is true.
 */
typedef bool (*square_predicate)(struct cave *c, int y, int x);

/* FEATURE PREDICATES */
extern bool square_isfloor(struct cave *c, int y, int x);
extern bool square_isrock(struct cave *c, int y, int x);
extern bool square_isperm(struct cave *c, int y, int x);
extern bool feat_is_magma(int feat);
extern bool square_ismagma(struct cave *c, int y, int x);
extern bool feat_is_quartz(int feat);
extern bool square_isquartz(struct cave *c, int y, int x);
extern bool square_ismineral(struct cave *c, int y, int x);
extern bool square_hassecretvein(struct cave *c, int y, int x);
extern bool square_hasgoldvein(struct cave *c, int y, int x);
extern bool feat_is_treasure(int feat);
extern bool square_issecretdoor(struct cave *c, int y, int x);
extern bool square_isopendoor(struct cave *c, int y, int x);
extern bool square_iscloseddoor(struct cave *c, int y, int x);
extern bool square_islockeddoor(struct cave *c, int y, int x);
extern bool square_isbrokendoor(struct cave *c, int y, int x);
extern bool square_isdoor(struct cave *c, int y, int x);
extern bool square_issecrettrap(struct cave *c, int y, int x);
extern bool feat_is_wall(int feat);
extern bool square_isknowntrap(struct cave *c, int y, int x);
extern bool square_istrap(struct cave *c, int y, int x);
extern bool feature_isshop(int feat);
extern bool square_isstairs(struct cave *c, int y, int x);
extern bool square_isupstairs(struct cave *c, int y, int x);
extern bool square_isdownstairs(struct cave *c, int y, int x);
extern bool square_isshop(struct cave *c, int y, int x);
extern bool square_isglyph(struct cave *c, int y, int x);

/* BEHAVIOR PREDICATES */
extern bool square_isopen(struct cave *c, int y, int x);
extern bool square_isempty(struct cave *c, int y, int x);
extern bool square_canputitem(struct cave *c, int y, int x);
extern bool square_isdiggable(struct cave *c, int y, int x);
extern bool feat_is_monster_walkable(feature_type *feature);
extern bool square_is_monster_walkable(struct cave *c, int y, int x);
extern bool feat_ispassable(feature_type *f_ptr);
extern bool square_ispassable(struct cave *c, int y, int x);
extern bool feat_isprojectable(feature_type *f_ptr);
extern bool square_isprojectable(struct cave *c, int y, int x);
extern bool square_iswall(struct cave *c, int y, int x);
extern bool square_isstrongwall(struct cave *c, int y, int x);
extern bool square_isvault(struct cave *c, int y, int x);
extern bool square_isroom(struct cave *c, int y, int x);
extern bool square_isrubble(struct cave *c, int y, int x);
extern bool square_isfeel(struct cave *c, int y, int x);
extern bool feat_isboring(feature_type *f_ptr);
extern bool square_isboring(struct cave *c, int y, int x);
extern bool square_isview(struct cave *c, int y, int x);
extern bool square_isseen(struct cave *c, int y, int x);
extern bool square_wasseen(struct cave *c, int y, int x);
extern bool square_isglow(struct cave *c, int y, int x);
extern bool square_iswarded(struct cave *c, int y, int x);
extern bool square_canward(struct cave *c, int y, int x);

extern bool square_seemslikewall(struct cave *c, int y, int x);
/* interesting to memorize when mapping */
extern bool square_isinteresting(struct cave *c, int y, int x);
/* noticeable when running */
extern bool square_noticeable(struct cave *c, int y, int x);

/* Feature placers */
extern void square_add_trap(struct cave *c, int y, int x);
extern void square_add_ward(struct cave *c, int y, int x);
extern void square_add_stairs(struct cave *c, int y, int x, int depth);
extern void square_add_door(struct cave *c, int y, int x, bool closed);

extern void square_remove_ward(struct cave *c, int y, int x);

extern void cave_generate(struct cave *c, struct player *p);

extern bool square_in_bounds(struct cave *c, int y, int x);
extern bool square_in_bounds_fully(struct cave *c, int y, int x);

extern struct monster *cave_monster(struct cave *c, int idx);
extern struct monster *square_monster(struct cave *c, int y, int x);
extern int cave_monster_max(struct cave *c);
extern int cave_monster_count(struct cave *c);

extern struct trap_type *cave_trap(struct cave *c, int idx);
extern int cave_trap_max(struct cave *c);

void upgrade_mineral(struct cave *c, int y, int x);

/* Feature modifiers */
int square_door_power(struct cave *c, int y, int x);
void square_open_door(struct cave *c, int y, int x);
void square_close_door(struct cave *c, int y, int x);
void square_smash_door(struct cave *c, int y, int x);
void square_lock_door(struct cave *c, int y, int x, int power);
void square_unlock_door(struct cave *c, int y, int x);
void square_destroy_door(struct cave *c, int y, int x);

void square_show_trap(struct cave *c, int y, int x, int type);
void square_destroy_trap(struct cave *c, int y, int x);

void square_tunnel_wall(struct cave *c, int y, int x);
void square_destroy_wall(struct cave *c, int y, int x);

void square_show_vein(struct cave *c, int y, int x);

/* destroy this cell, as destruction spell */
void square_destroy(struct cave *c, int y, int x);
void square_earthquake(struct cave *c, int y, int x);

int square_shopnum(struct cave *c, int y, int x);
const char *square_apparent_name(struct cave *c, struct player *p, int y, int x);

void square_destroy_rubble(struct cave *c, int y, int x);

void square_force_floor(struct cave *c, int y, int x);

int count_feats(int *y, int *x, bool (*test)(struct cave *cave, int y, int x), bool under);

#endif /* !CAVE_H */
