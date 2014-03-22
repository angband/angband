/* trap.h - trap interface */

#ifndef TRAP_H
#define TRAP_H

/*** Trap Indexes (see "lib/edit/trap.txt") ***/

/** Nothing */
#define TRAP_NONE	0x00

/* Runes  */
#define RUNE_PROTECT    0x03

/* Traps */
#define TRAP_HEAD	    0x10
#define TRAP_TAIL	    0x1F
#define TRAP_TRAPDOOR	0x10
#define TRAP_PIT_OPEN   0x11
#define TRAP_PIT_SPIKED 0x12
#define TRAP_PIT_POISON 0x13
#define TRAP_SUMMON	    0x14
#define TRAP_PORTAL	    0x15
#define TRAP_SPOT_FIRE  0x16
#define TRAP_SPOT_ACID  0x17
#define TRAP_DART_SLOW  0x18
#define TRAP_DART_STR   0x19
#define TRAP_DART_DEX   0x1A
#define TRAP_DART_CON   0x1B
#define TRAP_GAS_BLIND  0x1C
#define TRAP_GAS_CONFU  0x1D
#define TRAP_GAS_POISON 0x1E
#define TRAP_GAS_SLEEP  0x1F


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


/*
 * A trap template.
 */
typedef struct trap
{
    char *name;		      /**< Name  */
    char *text;		      /**< Text  */
	
    struct trap *next;
    u32b tidx;

    byte d_attr;              /**< Default trap attribute */
    wchar_t d_char;              /**< Default trap character */

    byte x_attr;              /**< Desired trap attribute */
    wchar_t x_char;              /**< Desired trap character */

    byte rarity;              /**< Rarity */
    byte min_depth;           /**< Minimum depth */
    byte max_num;             /**< Unused */

    bitflag flags[TRF_SIZE]; /**< Trap flags (all traps of this kind) */

	u32b effect;   /**< Effect on entry to grid */
} trap_kind;

extern trap_kind *trap_info;

/*
 * An actual trap.
 */
typedef struct trap_type
{
    byte t_idx;               /**< Trap kind index */
    struct trap *kind;

    byte fy;                  /**< Location of trap */
    byte fx;
	
    byte xtra;
	
    bitflag flags[TRF_SIZE]; /**< Trap flags (only this particular trap) */
} trap_type;

bool square_trap_specific(struct chunk *c, int y, int x, int t_idx);
bool square_visible_trap(struct chunk *c, int y, int x);
bool square_invisible_trap(struct chunk *c, int y, int x);
bool square_player_trap(struct chunk *c, int y, int x);
int square_visible_trap_idx(struct chunk *c, int y, int x);
bool get_trap_graphics(struct chunk *c, int t_idx, int *a, wchar_t *ch, bool require_visible);
bool square_reveal_trap(struct chunk *c, int y, int x, int chance, bool domsg);
bool trap_check_hit(int power);
extern void hit_trap(int y, int x);
void place_trap(struct chunk *c, int y, int x, int t_idx, int trap_level);
void wipe_trap_list(struct chunk *c);
bool square_remove_trap(struct chunk *c, int y, int x, bool domsg, int t_idx);
void square_remove_trap_kind(struct chunk *c, int y, int x, bool domsg, int t_idx);

#endif /* !TRAP_H */
