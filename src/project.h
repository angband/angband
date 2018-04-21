/**
 * \file project.h
 * \brief projection and helpers
 */

#ifndef PROJECT_H
#define PROJECT_H

/**
 * Spell types used by project(), and related functions.
 */
enum
{
	#define ELEM(a, b, c, d, e, f, g, h, i, col) GF_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ_ENV(a, col, desc) GF_##a,
	#include "list-project-environs.h"
	#undef PROJ_ENV
	#define PROJ_MON(a, obv, desc) GF_##a,
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


/**
 *   NONE: No flags
 *   JUMP: Jump directly to the target location without following a path
 *   BEAM: Work as a beam weapon (affect every grid passed through)
 *   THRU: May continue through the target (used for bolts and beams)
 *   STOP: Stop as soon as we hit a monster (used for bolts)
 *   GRID: May affect terrain in the blast area in some way
 *   ITEM: May affect objects in the blast area in some way
 *   KILL: May affect monsters in the blast area in some way
 *   HIDE: Disable visual feedback from projection
 *   AWARE: Effects are already obvious to the player
 *   SAFE: Doesn't affect monsters of the same race as the caster
 *   ARC: Projection is a sector of circle radiating from the caster
 *   PLAY: May affect the player
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
#define PROJECT_PLAY  0x800

int project_m_n;
int project_m_x;
int project_m_y;

bool project_f(int who, int r, int y, int x, int dam, int typ);
int inven_damage(struct player *p, int type, int cperc);
bool project_o(int who, int r, int y, int x, int dam, int typ);
bool project_m(int who, int r, int y, int x, int dam, int typ, int flg);
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect, int resist);
bool project_p(int who, int r, int y, int x, int dam, int typ);


/* project.c */
byte gf_to_attr[GF_MAX][BOLT_MAX];
wchar_t gf_to_char[GF_MAX][BOLT_MAX];

int project_path(struct loc *gp, int range, int y1, int x1, int y2, int x2, int flg);
bool projectable(struct chunk *c, int y1, int x1, int y2, int x2, int flg);
bool gf_force_obvious(int type);
int gf_color(int type);
int gf_num(int type);
random_value gf_denom(int type);
const char *gf_desc(int type);
const char *gf_blind_desc(int type);
int gf_name_to_idx(const char *name);
const char *gf_idx_to_name(int type);
bool project(int who, int rad, int y, int x, int dam, int typ, int flg,
			 int degrees_of_arc, byte diameter_of_source);

#endif /* !PROJECT_H */
