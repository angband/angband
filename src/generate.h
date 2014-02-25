/* generate.h - dungeon generation interface */

#ifndef GENERATE_H
#define GENERATE_H

#include "mon-constants.h"
#include "monster.h"

#if  __STDC_VERSION__ < 199901L
#define ROOM_LOG  if (OPT(cheat_room)) msg
#else
#define ROOM_LOG(...) if (OPT(cheat_room)) msg(__VA_ARGS__);
#endif

/*
 * Dungeon allocation places and types, used with alloc_object().
 */
#define SET_CORR   1 /* Hallway */
#define SET_ROOM   2 /* Room */
#define SET_BOTH   3 /* Anywhere */

#define TYP_RUBBLE 1 /* Rubble */
#define TYP_TRAP   3 /* Trap */
#define TYP_GOLD   4 /* Gold */
#define TYP_OBJECT 5 /* Object */
#define TYP_GOOD   6 /* Good object */
#define TYP_GREAT  7 /* Great object */

#define AMT_ROOM   9 /* Number of objects for rooms */
#define AMT_ITEM   3 /* Number of objects for rooms/corridors */
#define AMT_GOLD   3 /* Amount of treasure for rooms/corridors */

/*
 * Number of grids in each block (vertically)
 */
#define BLOCK_HGT	11

/*
 * Number of grids in each block (horizontally)
 */
#define BLOCK_WID	11

#define MAX_PIT 2 /* Maximum number of pits or nests allowed */

/**
 * Maximum number of rvals (monster templates) that a pit can specify.
 */
#define MAX_RVALS 6

/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX 100
#define DOOR_MAX 200
#define WALL_MAX 500
#define TUNN_MAX 900

struct pit_color_profile {
    struct pit_color_profile *next;

    byte color;
};

struct pit_forbidden_monster {
    struct pit_forbidden_monster *next;

    monster_race *race;
};

typedef struct pit_profile {
    struct pit_profile *next;

    int pit_idx; /* Index in pit_info */
    const char *name;
    int room_type; /* Is this a pit or a nest? */
    int ave; /* Level where this pit is most common */
    int rarity; /* How unusual this pit is */
    int obj_rarity; /* How rare objects are in this pit */
    bitflag flags[RF_SIZE];         /* Required flags */
    bitflag forbidden_flags[RF_SIZE];
    bitflag spell_flags[RSF_SIZE];  /* Required spell flags */
    bitflag forbidden_spell_flags[RSF_SIZE];
    int n_bases;
    struct monster_base *base[MAX_RVALS];
    struct pit_color_profile *colors;
    struct pit_forbidden_monster *forbidden_monsters;
} pit_profile;

extern struct pit_profile *pit_info;


/**
 * Structure to hold all "dungeon generation" data
 */
struct dun_data {
    /* The profile used to generate the level */
    const struct cave_profile *profile;

    /* Array of centers of rooms */
    int cent_n;
    struct loc cent[CENT_MAX];

    /* Array of possible door locations */
    int door_n;
    struct loc door[DOOR_MAX];

    /* Array of wall piercing locations */
    int wall_n;
    struct loc wall[WALL_MAX];

    /* Array of tunnel grids */
    int tunn_n;
    struct loc tunn[TUNN_MAX];

    /* Number of blocks along each axis */
    int row_rooms;
    int col_rooms;

    /* Array of which blocks are used */
    bool **room_map;

    /* Number of pits/nests on the level */
    int pit_num;

	/* Current pit profile in use */
	pit_profile *pit_type;
};


extern struct room_template *random_room_template(int typ);
extern struct vault *random_vault(int typ);

struct tunnel_profile {
    const char *name;
    int rnd; /* % chance of choosing random direction */
    int chg; /* % chance of changing direction */
    int con; /* % chance of extra tunneling */
    int pen; /* % chance of placing doors at room entrances */
    int jct; /* % chance of doors at tunnel junctions */
};

struct streamer_profile {
    const char *name;
    int den; /* Density of streamers */    
    int rng; /* Width of streamers */
    int mag; /* Number of magma streamers */
    int mc; /* 1/chance of treasure per magma */
    int qua; /* Number of quartz streamers */
    int qc; /* 1/chance of treasure per quartz */
};

/*
 * cave_builder is a function pointer which builds a level.
 */
typedef bool (*cave_builder) (struct cave *c, struct player *p);


struct cave_profile {
    const char *name;
    cave_builder builder; /* Function used to build the level */
    int dun_rooms; /* Number of rooms to attempt */
    int dun_unusual; /* Level/chance of unusual room */
    int max_rarity; /* Max number of rarity levels used in room generation */
    int n_room_profiles; /* Number of room profiles */
    struct tunnel_profile tun; /* Used to build tunnels */
    struct streamer_profile str; /* Used to build mineral streamers*/
    const struct room_profile *room_profiles; /* Used to build rooms */
    int cutoff; /* Used to see if we should try this dungeon */
};


/**
 * room_builder is a function pointer which builds rooms in the cave given
 * anchor coordinates.
 */
typedef bool (*room_builder) (struct cave *c, int y0, int x0);


/**
 * This tracks information needed to generate the room, including the room's
 * name and the function used to build it.
 */
struct room_profile {
    const char *name;
    room_builder builder; /* Function used to build fixed size rooms */
    int height, width; /* Space required in blocks */
    int level; /* Minimum dungeon level */
    bool pit; /* Whether this room is a pit/nest or not */
    int rarity; /* How unusual this room is */
    int cutoff; /* Upper limit of 1-100 random roll for room generation */
};


/*
 * Information about "vault generation"
 */
struct vault {
    struct vault *next;
    unsigned int vidx;
    char *name;
    char *text;

    byte typ;			/* Vault type */

    byte rat;			/* Vault rating */

    byte hgt;			/* Vault height */
    byte wid;			/* Vault width */
};

extern struct vault *vaults;


/*
 * Information about "room generation"
 */
typedef struct room_template {
    struct room_template *next;
    unsigned int tidx;
    char *name;
    char *text;

    byte typ;			/* Room type */

    byte rat;			/* Room rating */

    byte hgt;			/* Room height */
    byte wid;			/* Room width */
    byte dor;           /* Random door options */
    byte tval;			/* tval for objects in this room */
} room_template_type;

/**
 * This is the global structure representing dungeon generation info.
 */
struct dun_data *dun;
struct room_template *room_templates;

/**
 * This is a global array of positions in the cave we're currently
 * generating. It's used to quickly randomize all the current cave positions.
 */
int *cave_squares;

bool town_gen(struct cave *c, struct player *p);

bool classic_gen(struct cave *c, struct player *p);
bool labyrinth_gen(struct cave *c, struct player *p);
bool cavern_gen(struct cave *c, struct player *p);

void fill_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat,
					int flag);
void generate_mark(struct cave *c, int y1, int x1, int y2, int x2, int flag);
void draw_rectangle(struct cave *c, int y1, int x1, int y2, int x2, int feat, 
					int flag);
void set_marked_granite(struct cave *c, int y, int x, int flag);
bool build_simple(struct cave *c, int y0, int x0);
bool build_circular(struct cave *c, int y0, int x0);
bool build_overlap(struct cave *c, int y0, int x0);
bool build_crossed(struct cave *c, int y0, int x0);
bool build_large(struct cave *c, int y0, int x0);
bool mon_pit_hook(monster_race *r_ptr);
void set_pit_type(int depth, int type);
bool build_nest(struct cave *c, int y0, int x0);
bool build_pit(struct cave *c, int y0, int x0);
bool build_template(struct cave *c, int y0, int x0);
bool build_lesser_vault(struct cave *c, int y0, int x0);
bool build_medium_vault(struct cave *c, int y0, int x0);
bool build_greater_vault(struct cave *c, int y0, int x0);
bool build_room_of_chambers(struct cave *c, int y0, int x0);
bool build_huge(struct cave *c, int y0, int x0);
/**
 * Profile used for generating the town level.
 */
struct cave_profile town_profile;


#define NUM_CLASSIC_ROOMS 11

/* name function width height min-depth pit? rarity %cutoff */
struct room_profile classic_rooms[NUM_CLASSIC_ROOMS];

#define NUM_CAVE_PROFILES 3

/**
 * Profiles used for generating dungeon levels.
 */
struct cave_profile cave_profiles[NUM_CAVE_PROFILES];

byte get_angle_to_grid[41][41];

void ensure_connectedness(struct cave *c);
void shuffle(int *arr, int n);
bool cave_find(struct cave *c, int *y, int *x, square_predicate pred);
bool find_empty(struct cave *c, int *y, int *x);
bool find_empty_range(struct cave *c, int *y, int y1, int y2, int *x, int x1, int x2);
bool find_nearby_grid(struct cave *c, int *y, int y0, int yd, int *x, int x0, int xd);
void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2);
void rand_dir(int *rdir, int *cdir);
void new_player_spot(struct cave *c, struct player *p);
void place_object(struct cave *c, int y, int x, int level, bool good,
				  bool great, byte origin, int tval);
void place_gold(struct cave *c, int y, int x, int level, byte origin);
void place_secret_door(struct cave *c, int y, int x);
void place_closed_door(struct cave *c, int y, int x);
void place_random_door(struct cave *c, int y, int x);
void place_random_stairs(struct cave *c, int y, int x);
void alloc_stairs(struct cave *c, int feat, int num, int walls);
void vault_objects(struct cave *c, int y, int x, int depth, int num);
void vault_traps(struct cave *c, int y, int x, int yd, int xd, int num);
void vault_monsters(struct cave *c, int y1, int x1, int depth, int num);
bool room_build(struct cave *c, int by0, int bx0, struct room_profile profile,
	bool finds_own_space);

void alloc_objects(struct cave *c, int set, int typ, int num, int depth, byte origin);
bool alloc_object(struct cave *c, int set, int typ, int depth, byte origin);
bool mon_restrict(const char *monster_type, int depth, bool unique_ok);
void spread_monsters(struct cave *c, const char *type, int depth, int num, 
					 int y0, int x0, int dy, int dx, byte origin);
void get_vault_monsters(struct cave *c, char racial_symbol[], byte vault_type, const char *data, int y1, int y2, int x1, int x2);
void get_chamber_monsters(struct cave *c, int y1, int x1, int y2, int x2, char *name, int area);


#endif /* !GENERATE_H */
