/**
 * \file trap.h 
 * \brief trap predicates, structs and functions
 */

#ifndef TRAP_H
#define TRAP_H

/*** Trap flags ***/

enum
{
	#define TRF(a,b) TRF_##a,
	#include "list-trap-flags.h"
	#undef TRF
	TRF_MAX
};

#define TRF_SIZE                FLAG_SIZE(TRF_MAX)

#define trf_has(f, flag)        flag_has_dbg(f, TRF_SIZE, flag, #f, #flag)
#define trf_next(f, flag)       flag_next(f, TRF_SIZE, flag)
#define trf_is_empty(f)         flag_is_empty(f, TRF_SIZE)
#define trf_is_full(f)          flag_is_full(f, TRF_SIZE)
#define trf_is_inter(f1, f2)    flag_is_inter(f1, f2, TRF_SIZE)
#define trf_is_subset(f1, f2)   flag_is_subset(f1, f2, TRF_SIZE)
#define trf_is_equal(f1, f2)    flag_is_equal(f1, f2, TRF_SIZE)
#define trf_on(f, flag)         flag_on_dbg(f, TRF_SIZE, flag, #f, #flag)
#define trf_off(f, flag)        flag_off(f, TRF_SIZE, flag)
#define trf_wipe(f)             flag_wipe(f, TRF_SIZE)
#define trf_setall(f)           flag_setall(f, TRF_SIZE)
#define trf_negate(f)           flag_negate(f, TRF_SIZE)
#define trf_copy(f1, f2)        flag_copy(f1, f2, TRF_SIZE)
#define trf_union(f1, f2)       flag_union(f1, f2, TRF_SIZE)
#define trf_comp_union(f1, f2)  flag_comp_union(f1, f2, TRF_SIZE)
#define trf_inter(f1, f2)       flag_inter(f1, f2, TRF_SIZE)
#define trf_diff(f1, f2)        flag_diff(f1, f2, TRF_SIZE)


/**
 * A trap template.
 */
struct trap_kind
{
	char *name;					/**< Name  */
	char *text;					/**< Text  */
	char *desc;					/**< Short description  */

	struct trap_kind *next;
	int tidx;					/**< Trap kind index */

	byte d_attr;				/**< Default trap attribute */
	wchar_t d_char;				/**< Default trap character */

	int rarity;					/**< Rarity */
	int min_depth;				/**< Minimum depth */
	int max_num;				/**< Unused */

	bitflag flags[TRF_SIZE];	/**< Trap flags (all traps of this kind) */

	struct effect *effect;		/**< Effect on entry to grid */
};

struct trap_kind *trap_info;

/**
 * An actual trap.
 */
struct trap
{
	byte t_idx;					/**< Trap kind index */
	struct trap_kind *kind;		/**< Trap kind */
	struct trap *next;			/**< Next trap in this location */

	byte fy;					/**< Location of trap */
	byte fx;

	byte xtra;					/**< Used for door locks, so far */

	bitflag flags[TRF_SIZE];	/**< Trap flags (only this particular trap) */
};

struct trap_kind *lookup_trap(const char *desc);
bool square_trap_specific(struct chunk *c, int y, int x, int t_idx);
bool square_trap_flag(struct chunk *c, int y, int x, int flag);
bool square_reveal_trap(struct chunk *c, int y, int x, int chance, bool domsg);
bool trap_check_hit(int power);
void hit_trap(int y, int x);
bool square_player_trap_allowed(struct chunk *c, int y, int x);
void place_trap(struct chunk *c, int y, int x, int t_idx, int trap_level);
void square_free_trap(struct chunk *c, int y, int x);
void wipe_trap_list(struct chunk *c);
bool square_remove_trap(struct chunk *c, int y, int x, bool domsg, int t_idx);
void square_set_door_lock(struct chunk *c, int y, int x, int power);
int square_door_power(struct chunk *c, int y, int x);

#endif /* !TRAP_H */
