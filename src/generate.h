/* generate.h - dungeon generation interface */

#ifndef GENERATE_H
#define GENERATE_H

void place_object(struct cave *c, int y, int x, int level, bool good, bool great);
void place_gold(struct cave *c, int y, int x, int level);
void place_secret_door(struct cave *c, int y, int x);
void place_closed_door(struct cave *c, int y, int x);
void place_random_door(struct cave *c, int y, int x);

extern struct vault *random_vault(void);

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
* room_builder is a function pointer which builds rooms in the cave given
* anchor coordinates.
*/
typedef void (*cave_builder) (struct cave *c, struct player *p);

struct cave_profile {
	const char *name;
	  	
	/* Function used to build the level */
	cave_builder builder;

    /* Number of rooms to attempt */
    int dun_rooms;

    /* Level/chance of unusual room */
    int dun_unusual;

	/* Max number of rarity levels used in room generation */
    int max_rarity;

	/* Profiles for building tunnels and streamers */
	const struct tunnel_profile tun;
	const struct streamer_profile str;

	/* Profiles used to build rooms */
    int n_room_profiles;
    const struct room_profile *room_profiles;
};


/*
* room_builder is a function pointer which builds rooms in the cave given
* anchor coordinates.
*/
typedef bool (*room_builder) (struct cave *c, int y0, int x0);


/*
 * This is a more advanced replacement for room_data -- it tracks information
 * needed to generate the room, including the funciton used to build it.
 */
struct room_profile {
	const char *name;
	  	
	/* the function used to build the room */
	room_builder builder;
	
	/* required size in blocks */
	int height, width;
	
	/* minimum level */
	int level;
	
	/* whether the room is crowded or not */
	bool crowded;
	
	/* how unusual the room is */
	int rarity;
	
	/* used to decide which room of a given rarity to generate */
	int cutoff;
};

#endif /* !GENERATE_H */
