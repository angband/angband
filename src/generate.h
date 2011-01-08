/* generate.h - dungeon generation interface */

#ifndef GENERATE_H
#define GENERATE_H

void place_object(struct cave *c, int y, int x, int level, bool good, bool great);
void place_gold(struct cave *c, int y, int x, int level);
void place_secret_door(struct cave *c, int y, int x);
void place_closed_door(struct cave *c, int y, int x);
void place_random_door(struct cave *c, int y, int x);

extern struct vault *random_vault(void);


struct dungeon_profile {
    /* Number of rooms to attempt */
    int DUN_ROOMS;
    /* Level/chance of unusual room */
    int DUN_UNUSUAL;

    /* Chance of random direction */
    int DUN_TUN_RND;
    /* Chance of changing direction */
    int DUN_TUN_CHG;
	  	/* Chance of extra tunneling */
    int DUN_TUN_CON;
    /* Chance of doors at room entrances */
    int DUN_TUN_PEN;
	  	/* Chance of doors at tunnel junctions */
    int DUN_TUN_JCT;
    
    /* Density of streamers */
    int DUN_STR_DEN;
    /* Width of streamers */
    int DUN_STR_RNG;
    /* Number of magma streamers */
    int DUN_STR_MAG;
    /* 1/chance of treasure per magma */
    int DUN_STR_MC;
    /* Number of quartz streamers */
    int DUN_STR_QUA;
    /* 1/chance of treasure per quartz */
    int DUN_STR_QC;

    int GV_PROFILE;
    int MAX_RARITY;

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
