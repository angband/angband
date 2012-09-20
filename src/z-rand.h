/* File: z-rand.h */

#ifndef INCLUDED_Z_RAND_H
#define INCLUDED_Z_RAND_H

#include "h-basic.h"



/**** Available constants ****/


/*
 * Random Number Generator -- Degree of "complex" RNG -- see "misc.c"
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63




/**** Available macros ****/


/*
 * Generates a random long integer X where O<=X<M.
 * The integer X falls along a uniform distribution.
 * For example, if M is 100, you get "percentile dice"
 */
#define rand_int(M) \
	(Rand_mod(M))

/*
 * Generates a random long integer X where A<=X<=B
 * The integer X falls along a uniform distribution.
 * Note: rand_range(0,N-1) == rand_int(N)
 */
#define rand_range(A,B) \
	((A) + (rand_int(1+(B)-(A))))

/*
 * Generate a random long integer X where A-D<=X<=A+D
 * The integer X falls along a uniform distribution.
 * Note: rand_spread(A,D) == rand_range(A-D,A+D)
 */
#define rand_spread(A,D) \
	((A) + (rand_int(1+(D)+(D))) - (D))


/*
 * Generate a random long integer X where 1<=X<=M
 * Also, "correctly" handle the case of M<=1
 */
#define randint(M) \
	(rand_int(M) + 1)


/*
 * Evaluate to TRUE "P" percent of the time
 */
#define magik(P) \
	(rand_int(100) < (P))




/**** Available Variables ****/


extern bool Rand_quick;
extern u32b Rand_value;
extern u16b Rand_place;
extern u32b Rand_state[RAND_DEG];


/**** Available Functions ****/


extern void Rand_state_init(u32b seed);
extern s32b Rand_mod(s32b m);
extern s32b Rand_div(s32b m);
extern s16b randnor(int mean, int stand);
extern s16b damroll(int num, int sides);
extern s16b maxroll(int num, int sides);


#endif


