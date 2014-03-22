/** \file generate.h
	\brief Dungeon generation.
*/


#ifndef GENERATE_H
#define GENERATE_H

#include "mon-constants.h"
#include "monster.h"

#if  __STDC_VERSION__ < 199901L
#define ROOM_LOG  if (OPT(cheat_room)) msg
#else
#define ROOM_LOG(...) if (OPT(cheat_room)) msg(__VA_ARGS__);
#endif

/**
 * Dungeon allocation places and types, used with alloc_object().
 */
#define SET_CORR   1 /*!< Hallway */
#define SET_ROOM   2 /*!< Room */
#define SET_BOTH   3 /*!< Anywhere */

#define TYP_RUBBLE 1 /*!< Rubble */
#define TYP_TRAP   3 /*!< Trap */
#define TYP_GOLD   4 /*!< Gold */
#define TYP_OBJECT 5 /*!< Object */
#define TYP_GOOD   6 /*!< Good object */
#define TYP_GREAT  7 /*!< Great object */

#define AMT_ROOM   9 /*!< Number of objects for rooms */
#define AMT_ITEM   3 /*!< Number of objects for rooms/corridors */
#define AMT_GOLD   3 /*!< Amount of treasure for rooms/corridors */

#define MAX_PIT 2 /*!< Maximum number of pits or nests allowed */

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX 100
#define DOOR_MAX 200
#define WALL_MAX 500
#define TUNN_MAX 900

struct pit_monster_profile {
    struct pit_monster_profile *next;

    struct monster_base *base;
};

struct pit_color_profile {
    struct pit_color_profile *next;

    byte color;
};

struct pit_forbidden_monster {
    struct pit_forbidden_monster *next;

    monster_race *race;
};

/**
 * Profile for choosing monsters for pits, nests or other themed areas
 */
typedef struct pit_profile {
    struct pit_profile *next; /*!< Pointer to next pit profile */

    int pit_idx;              /**< Index in pit_info */
    const char *name;
    int room_type;            /**< Is this a pit or a nest? */
    int ave;                  /**< Level where this pit is most common */
    int rarity;               /**< How unusual this pit is */
    int obj_rarity;           /**< How rare objects are in this pit */
    bitflag flags[RF_SIZE];   /**< Required flags */
    bitflag forbidden_flags[RF_SIZE];         /**< Forbidden flags */
    bitflag spell_flags[RSF_SIZE];            /**< Required spell flags */
    bitflag forbidden_spell_flags[RSF_SIZE];  /**< Forbidden spell flags */
    struct pit_monster_profile *bases;     /**< List of vaild monster bases */
    struct pit_color_profile *colors;      /**< List of valid monster colors */
    struct pit_forbidden_monster *forbidden_monsters; /**< Forbidden monsters */
} pit_profile;

extern struct pit_profile *pit_info;


/**
 * Structure to hold all "dungeon generation" data
 */
struct dun_data {
    /*!< The profile used to generate the level */
    const struct cave_profile *profile;

    /*!< Array of centers of rooms */
    int cent_n;
    struct loc cent[CENT_MAX];

    /*!< Array of possible door locations */
    int door_n;
    struct loc door[DOOR_MAX];

    /*!< Array of wall piercing locations */
    int wall_n;
    struct loc wall[WALL_MAX];

    /*!< Array of tunnel grids */
    int tunn_n;
    struct loc tunn[TUNN_MAX];

	/*!< Number of grids in each block (vertically) */
	int block_hgt;

	/*!< Number of grids in each block (horizontally) */
	int block_wid;

    /*!< Number of blocks along each axis */
    int row_blocks;
    int col_blocks;

    /*!< Array of which blocks are used */
    bool **room_map;

    /*!< Number of pits/nests on the level */
    int pit_num;

	/*!< Current pit profile in use */
	pit_profile *pit_type;
};


struct tunnel_profile {
    const char *name;
    int rnd; /*!< % chance of choosing random direction */
    int chg; /*!< % chance of changing direction */
    int con; /*!< % chance of extra tunneling */
    int pen; /*!< % chance of placing doors at room entrances */
    int jct; /*!< % chance of doors at tunnel junctions */
};

struct streamer_profile {
    const char *name;
    int den; /*!< Density of streamers */
    int rng; /*!< Width of streamers */
    int mag; /*!< Number of magma streamers */
    int mc;  /*!< 1/chance of treasure per magma */
    int qua; /*!< Number of quartz streamers */
    int qc;  /*!< 1/chance of treasure per quartz */
};

/*
 * cave_builder is a function pointer which builds a level.
 */
typedef struct chunk * (*cave_builder) (struct player *p);


struct cave_profile {
    const char *name;
    cave_builder builder; /*!< Function used to build the level */
	int block_size;       /*!< Default height and width of dungeon blocks */
    int dun_rooms;        /*!< Number of rooms to attempt */
    int dun_unusual;      /*!< Level/chance of unusual room */
    int max_rarity; /*!< Max number of rarity levels used in room generation */
    int n_room_profiles; /*!< Number of room profiles */
    struct tunnel_profile tun; /*!< Used to build tunnels */
    struct streamer_profile str; /*!< Used to build mineral streamers*/
    const struct room_profile *room_profiles; /*!< Used to build rooms */
    int cutoff;    /*!< Used to see if we should try this dungeon */
};


/**
 * room_builder is a function pointer which builds rooms in the cave given
 * anchor coordinates.
 */
typedef bool (*room_builder) (struct chunk *c, int y0, int x0);


/**
 * This tracks information needed to generate the room, including the room's
 * name and the function used to build it.
 */
struct room_profile {
    const char *name;
    room_builder builder; /*!< Function used to build fixed size rooms */
    int height, width; /*!< Space required in grids */
    int level; /*!< Minimum dungeon level */
    bool pit; /*!< Whether this room is a pit/nest or not */
    int rarity; /*!< How unusual this room is */
    int cutoff; /*!< Upper limit of 1-100 random roll for room generation */
};


/*
 * Information about "vault generation"
 */
struct vault {
    struct vault *next; /*!< Pointer to next vault template */
    unsigned int vidx;  /*!< Vault index */
    char *name;         /*!< Vault name */
    char *text;         /*!< Grid by grid description of vault layout */

    byte typ;			/*!< Vault type */

    byte rat;			/*!< Vault rating */

    byte hgt;			/*!< Vault height */
    byte wid;			/*!< Vault width */

    byte min_lev;		/*!< Minimum allowable level, if specified. */
    byte max_lev;		/*!< Maximum allowable level, if specified. */
};

extern struct vault *vaults;


/*
 * Information about "room generation"
 */
typedef struct room_template {
    struct room_template *next; /*!< Pointer to next room template */
    unsigned int tidx;  /*!< Template index */
    char *name;         /*!< Room name */
    char *text;         /*!< Grid by grid description of room layout */

    byte typ;			/*!< Room type */

    byte rat;			/*!< Room rating */

    byte hgt;			/*!< Room height */
    byte wid;			/*!< Room width */
    byte dor;           /*!< Random door options */
    byte tval;			/*!< tval for objects in this room */
} room_template_type;

/**
 * This is the global structure representing dungeon generation info.
 */
struct dun_data *dun;
struct room_template *room_templates;

struct chunk *town_gen(struct player *p);
struct chunk *classic_gen(struct player *p);
struct chunk *labyrinth_gen(struct player *p);
struct chunk *cavern_gen(struct player *p);
struct chunk *modified_gen(struct player *p);
struct chunk *hard_centre_gen(struct player *p);
struct chunk *lair_gen(struct player *p);
struct chunk *gauntlet_gen(struct player *p);

struct chunk *chunk_write(int y0, int x0, int height, int width, bool monsters,
						 bool objects, bool traps, bool delete_old);
void chunk_list_add(struct chunk *c);
bool chunk_list_remove(char *name);
struct chunk *chunk_find_name(char *name);
bool chunk_find(struct chunk *c);
bool chunk_copy(struct chunk *dest, struct chunk *source, int y0, int x0,
				int rotate, bool reflect);


void fill_rectangle(struct chunk *c, int y1, int x1, int y2, int x2, int feat,
					int flag);
void generate_mark(struct chunk *c, int y1, int x1, int y2, int x2, int flag);
void draw_rectangle(struct chunk *c, int y1, int x1, int y2, int x2, int feat, 
					int flag);
void set_marked_granite(struct chunk *c, int y, int x, int flag);

struct vault *random_vault(int depth, int typ);
bool build_vault(struct chunk *c, int y0, int x0, struct vault *v);

bool build_simple(struct chunk *c, int y0, int x0);
bool build_circular(struct chunk *c, int y0, int x0);
bool build_overlap(struct chunk *c, int y0, int x0);
bool build_crossed(struct chunk *c, int y0, int x0);
bool build_large(struct chunk *c, int y0, int x0);
bool mon_pit_hook(monster_race *r_ptr);
void set_pit_type(int depth, int type);
bool build_nest(struct chunk *c, int y0, int x0);
bool build_pit(struct chunk *c, int y0, int x0);
bool build_template(struct chunk *c, int y0, int x0);
bool build_interesting(struct chunk *c, int y0, int x0);
bool build_lesser_vault(struct chunk *c, int y0, int x0);
bool build_medium_vault(struct chunk *c, int y0, int x0);
bool build_greater_vault(struct chunk *c, int y0, int x0);
bool build_moria(struct chunk *c, int y0, int x0);
bool build_room_of_chambers(struct chunk *c, int y0, int x0);
bool build_huge(struct chunk *c, int y0, int x0);

byte get_angle_to_grid[41][41];

void ensure_connectedness(struct chunk *c);
int yx_to_i(int y, int x, int w);
void i_to_yx(int i, int w, int *y, int *x);
void shuffle(int *arr, int n);
bool cave_find(struct chunk *c, int *y, int *x, square_predicate pred);
bool find_empty(struct chunk *c, int *y, int *x);
bool find_empty_range(struct chunk *c, int *y, int y1, int y2, int *x, int x1, int x2);
bool find_nearby_grid(struct chunk *c, int *y, int y0, int yd, int *x, int x0, int xd);
void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2);
void rand_dir(int *rdir, int *cdir);
void new_player_spot(struct chunk *c, struct player *p);
void place_object(struct chunk *c, int y, int x, int level, bool good,
				  bool great, byte origin, int tval);
void place_gold(struct chunk *c, int y, int x, int level, byte origin);
void place_secret_door(struct chunk *c, int y, int x);
void place_closed_door(struct chunk *c, int y, int x);
void place_random_door(struct chunk *c, int y, int x);
void place_random_stairs(struct chunk *c, int y, int x);
void alloc_stairs(struct chunk *c, int feat, int num, int walls);
void vault_objects(struct chunk *c, int y, int x, int depth, int num);
void vault_traps(struct chunk *c, int y, int x, int yd, int xd, int num);
void vault_monsters(struct chunk *c, int y1, int x1, int depth, int num);
bool room_build(struct chunk *c, int by0, int bx0, struct room_profile profile,
	bool finds_own_space);

void alloc_objects(struct chunk *c, int set, int typ, int num, int depth, byte origin);
bool alloc_object(struct chunk *c, int set, int typ, int depth, byte origin);
bool mon_restrict(const char *monster_type, int depth, bool unique_ok);
void spread_monsters(struct chunk *c, const char *type, int depth, int num, 
					 int y0, int x0, int dy, int dx, byte origin);
void get_vault_monsters(struct chunk *c, char racial_symbol[], byte vault_type, const char *data, int y1, int y2, int x1, int x2);
void get_chamber_monsters(struct chunk *c, int y1, int x1, int y2, int x2, char *name, int area);


#endif /* !GENERATE_H */
