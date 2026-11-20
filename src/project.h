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
	bool wake;
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
 * Bit flags for project()'s flg argument
 */
enum
{
	PROJECT_NONE  = 0x0000, /**< no flags */
	PROJECT_JUMP  = 0x0001, /**< jump directly to the target location
					without following a path */
	PROJECT_BEAM  = 0x0002,	/**< work as a beam weapon (affect every grid
					passed through */
	PROJECT_THRU  = 0x0004,	/**< may continue through the target (used for
					bolts and beams) */
	PROJECT_STOP  = 0x0008, /**< stop as soon as we hit a monster (used for
					bolts) */
	PROJECT_GRID  = 0x0010, /**< may affect terrain in the blast area in
					some way */
	PROJECT_ITEM  = 0x0020,	/**< may affect objects in the blast area in
					some way */
	PROJECT_KILL  = 0x0040, /**< may affect monsters in the blast area in
					some way */
	PROJECT_HIDE  = 0x0080, /**< disable visual feedback from projection */
	PROJECT_AWARE = 0x0100, /**< effects are already obvious to the player */
	PROJECT_SAFE  = 0x0200, /**< does not affect monsters of the same race
					as the caster */
	PROJECT_ARC   = 0x0400, /**< projection is a sector of a circle centered
					and expanding outward from the caster */
	PROJECT_PLAY  = 0x0800, /**< may affect the player */
	PROJECT_INFO  = 0x1000, /**< use believed map rather than truth for
					player UI */
	PROJECT_SHORT = 0x2000, /**< use one quarter of max_range */
	PROJECT_SELF  = 0x4000, /**< may affect the player, even when cast by
					the player */
	PROJECT_ROCK  = 0x8000, /**< do not stop at non-initial wall grids */
};

/** Display attributes for a projections visual feedback */
extern uint8_t proj_to_attr[PROJ_MAX][BOLT_MAX];
/** Display characters for a projections visual feedback */
extern wchar_t proj_to_char[PROJ_MAX][BOLT_MAX];

void thrust_away(struct loc centre, struct loc target, int grids_away);
int inven_damage(struct player *p, int type, int cperc);
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect,
			   int resist, bool actual);

bool project_f(struct source, struct loc centre, int r, struct loc grid,
		int dam, int typ);
bool project_o(struct source, struct loc centre, int r, struct loc grid,
		int dam, int typ, const struct object *protected_obj);
void project_m(struct source, struct loc centre, int r, struct loc grid,
		int dam, int typ, int flg, bool *did_hit, bool *was_obvious);
bool project_p(struct source, struct loc centre, int r, struct loc grid,
		int dam, int typ, int power, bool self);

int project_path(struct chunk *c, struct loc *gp, int range, struct loc grid1,
	struct loc grid2, int flg);
bool projectable(struct chunk *c, struct loc grid1, struct loc grid2, int flg);
int proj_name_to_idx(const char *name);
const char *proj_idx_to_name(int type);

struct loc origin_get_loc(struct source origin);

bool project(struct source origin, int rad, struct loc finish, int dam, int typ,
	int flg, int degrees_of_arc, uint8_t diameter_of_source,
	const struct object *obj);

#endif /* !PROJECT_H */
