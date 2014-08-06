 /* File: src/project.h
 * Purpose: projection and helpers
 */

#ifndef PROJECT_H
#define PROJECT_H

/*
 * Spell types used by project(), and related functions.
 */
enum
{
	#define ELEM(a, b, c, d, e, f, g, col, h, fh, oh, mh, ph) GF_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ_ENV(a, col, fh, oh, mh) GF_##a,
	#include "list-project-environs.h"
	#undef PROJ_ENV
	#define PROJ_MON(a, obv, mh) GF_##a, 
	#include "list-project-monsters.h"
	#undef PROJ_MON
	GF_MAX
};

/**
 * Bolt motion (used in prefs.c, project.c)
 */
enum
{
    BOLT_NO_MOTION,
    BOLT_0,
    BOLT_45,
    BOLT_90,
    BOLT_135,
    BOLT_MAX
};


/*
 * Convert a "location" (Y,X) into a "grid" (G)
 */
#define GRID(Y,X) \
	(256 * (Y) + (X))

/*
 * Convert a "grid" (G) into a "location" (Y)
 */
#define GRID_Y(G) \
	((int)((G) / 256U))

/*
 * Convert a "grid" (G) into a "location" (X)
 */
#define GRID_X(G) \
	((int)((G) % 256U))


/* TODO: these descriptions are somewhat wrong/misleading */
/*
 * Bit flags for the "project()" function
 *
 *   NONE: No flags
 *   JUMP: Jump directly to the target location (this is a hack)
 *   BEAM: Work as a beam weapon (affect every grid passed through)
 *   THRU: Continue "through" the target (used for "bolts"/"beams")
 *   STOP: Stop as soon as we hit a monster (used for "bolts")
 *   GRID: Affect each grid in the "blast area" in some way
 *   ITEM: Affect each object in the "blast area" in some way
 *   KILL: Affect each monster in the "blast area" in some way
 *   HIDE: Hack -- disable "visual" feedback from projection
 *   AWARE: Effects are already obvious to the player
 */
#define PROJECT_NONE  0x000
#define PROJECT_JUMP  0x001
#define PROJECT_BEAM  0x002
#define PROJECT_THRU  0x004
#define PROJECT_STOP  0x008
#define PROJECT_GRID  0x010
#define PROJECT_ITEM  0x020
#define PROJECT_KILL  0x040
#define PROJECT_HIDE  0x080
#define PROJECT_AWARE 0x100
#define PROJECT_SAFE  0x200
#define PROJECT_ARC   0x400
#define PROJECT_PLAY  0x800 /* Needs significant changes to implement */

/* project.c */
extern byte gf_to_attr[GF_MAX][BOLT_MAX];
extern wchar_t gf_to_char[GF_MAX][BOLT_MAX];

int gf_name_to_idx(const char *name);
const char *gf_idx_to_name(int type);
int inven_damage(struct player *p, int type, int cperc);
int adjust_dam(int type, int dam, aspect dam_aspect, int resist);
bool project(int who, int rad, int y, int x, int dam, int typ, int flg,
			 int degrees_of_arc, byte diameter_of_source);

#endif /* !PROJECT_H */
