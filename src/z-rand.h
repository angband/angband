#ifndef INCLUDED_Z_RAND_H
#define INCLUDED_Z_RAND_H

#include "h-basic.h"


/**** Constants ****/

/**
 * The "degree" of the "complex" Random Number Generator.
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63



/**** Available macros ****/

/**
 * Generates a random signed long integer X where "0 <= X < M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define rand_int(M)		((s32b) Rand_div(M))


/**
 * Generates a random signed long integer X where "1 <= X <= M" holds.
 *
 * The integer X falls along a uniform distribution.
 */
#define randint(M)		((s32b) Rand_div(M) + 1)
#define rand_die(M)		(rand_int(M) + 1)


/**
 * Generates a random signed long integer X where "A <= X <= B"
 * Note that "rand_range(0, N-1)" == "rand_int(N)"
 *
 * The integer X falls along a uniform distribution.
 */
#define rand_range(A,B)		((A) + (rand_int(1+(B)-(A))))


/**
 * Generate a random signed long integer X where "A-D <= X <= A+D" holds.
 * Note that "rand_spread(A, D)" == "rand_range(A-D, A+D)"
 *
 * The integer X falls along a uniform distribution.
 */
#define rand_spread(A,D)	((A) + (rand_int(1+(D)+(D))) - (D))



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


#endif /* INCLUDED_Z_RAND_H */
