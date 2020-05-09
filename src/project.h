/**
 * \file project.h
 * \brief projection and helpers
 */

#ifndef PROJECT_H
#define PROJECT_H

#include "source.h"

/**
 * Spell types used by project(), and related functions.
 */
enum
{
	#define ELEM(a) PROJ_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ(a) PROJ_##a,
	#include "list-projections.h"
	#undef PROJ
	PROJ_MAX
};

/**
 * Element struct
 */
struct projection {
	int index;
	char *name;
	char *type;
	char *desc;
	char *player_desc;
	char *blind_desc;
	char *lash_desc;
	int numerator;
	random_value denominator;
	int divisor;
	int damage_cap;
	int msgt;
	bool obvious;
	int color;
	struct projection *next;
};

extern struct projection *projections;

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
 *   INFO: Use believed map rather than truth for player ui
 *   SHORT: Use one quarter of max_range
 *   SELF: May affect the player, even when cast by the player
 */
enum
{
	PROJECT_NONE  = 0x0000,
	PROJECT_JUMP  = 0x0001,
	PROJECT_BEAM  = 0x0002,
	PROJECT_THRU  = 0x0004,
	PROJECT_STOP  = 0x0008,
	PROJECT_GRID  = 0x0010,
	PROJECT_ITEM  = 0x0020,
	PROJECT_KILL  = 0x0040,
	PROJECT_HIDE  = 0x0080,
	PROJECT_AWARE = 0x0100,
	PROJECT_SAFE  = 0x0200,
	PROJECT_ARC   = 0x0400,
	PROJECT_PLAY  = 0x0800,
	PROJECT_INFO  = 0x1000,
	PROJECT_SHORT = 0x2000,
	PROJECT_SELF  = 0x4000,
};

/* Display attrs and chars */
extern byte proj_to_attr[PROJ_MAX][BOLT_MAX];
extern wchar_t proj_to_char[PROJ_MAX][BOLT_MAX];

void thrust_away(struct loc centre, struct loc target, int grids_away);
int inven_damage(struct player *p, int type, int cperc);
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect,
			   int resist, bool actual);

bool project_f(struct source, int r, struct loc grid, int dam, int typ);
bool project_o(struct source, int r, struct loc grid, int dam, int typ,
			   const struct object *protected_obj);
void project_m(struct source, int r, struct loc grid, int dam, int typ, int flg,
               bool *did_hit, bool *was_obvious);
bool project_p(struct source, int r, struct loc grid, int dam, int typ,
			   int power, bool self);

int project_path(struct loc *gp, int range, struct loc grid1, struct loc grid2,
				 int flg);
bool projectable(struct chunk *c, struct loc grid1, struct loc grid2, int flg);
int proj_name_to_idx(const char *name);
const char *proj_idx_to_name(int type);

struct loc origin_get_loc(struct source origin);

bool project(struct source origin, int rad, struct loc finish, int dam, int typ,
			 int flg, int degrees_of_arc, byte diameter_of_source,
			 const struct object *obj);

#endif /* !PROJECT_H */
