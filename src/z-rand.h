#ifndef INCLUDED_Z_RAND_H
#define INCLUDED_Z_RAND_H

#include "h-basic.h"
#include "defines.h"


/**** Types ****/

typedef struct
{
	int base;
	int dice;
	int sides;
	int m_bonus;
} random_value;


/**** Constants ****/

/**
 * The "degree" of the "complex" Random Number Generator.
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63

/* Random aspects used by damcalc, m_bonus_calc, and ranvals */
typedef enum
{
	MINIMISE,
	AVERAGE,
	MAXIMISE,
	EXTREMIFY,
	RANDOMISE
} aspect;


/**** Available macros ****/

/**
 * Generates a random signed long integer X where "0 <= X < M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define randint0(M)		((s32b) Rand_div(M))


/**
 * Generates a random signed long integer X where "1 <= X <= M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define randint1(M)		((s32b) Rand_div(M) + 1)

/**
 * Generate a random signed long integer X where "A-D <= X <= A+D" holds.
 * Note that "rand_spread(A, D)" == "rand_range(A-D, A+D)"
 *
 * The integer X falls along a uniform distribution.
 */
#define rand_spread(A,D)	((A) + (randint0(1+(D)+(D))) - (D))


/**
 * Return TRUE one time in `x`.
 */
#define one_in_(x)		(!randint0(x))

/**** Available Variables ****/

/**
 * Do not tweak these variables.
 */
extern bool Rand_quick;
extern u32b Rand_value;
extern u16b Rand_place;
extern u32b Rand_state[RAND_DEG];


/**** Available Functions ****/

/**
 * Initialise the RNG state with the given seed.
 */
void Rand_state_init(u32b seed);

/**
 * Generates a random unsigned long integer X where "0 <= X < M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
u32b Rand_div(u32b m);

/**
 * Generate a signed random integer within `stand` standard deviations of
 * `mean`, following a normal distribution.
 */
s16b Rand_normal(int mean, int stand);

/**
 * Generate a semi-random number from 0 to m-1, in a way that doesn't affect
 * gameplay.  This is intended for use by external program parts like the
 * main-*.c files.
 */
u32b Rand_simple(u32b m);

/**
 * Emulate a number `num` of dice rolls of dice with `sides` sides.
 */
int damroll(int num, int sides);

/**
 * Calculation helper function for damroll
 */
int damcalc(int num, int sides, aspect dam_aspect);

/**
 * Generates a random signed long integer X where "A <= X <= B"
 * Note that "rand_range(0, N-1)" == "randint0(N)"
 *
 * The integer X falls along a uniform distribution.
 */
int rand_range(int A, int B);

/**
 * Function used to determine enchantment bonuses, see function header for
 * a more complete description.
 */
s16b m_bonus(int max, int level);

/**
 * Calculation helper function for m_bonus
 */
s16b m_bonus_calc(int max, int level, aspect bonus_aspect);

/**
 * Calculation helper function for random_value structs
 */
int randcalc(random_value v, int level, aspect rand_aspect);

/**
 * Test to see if a value is within a random_value's range
 */
bool randcalc_valid(random_value v, int test);

#endif /* INCLUDED_Z_RAND_H */
