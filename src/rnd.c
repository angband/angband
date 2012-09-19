/* rnd.c: random number generator

   Copyright (c) 1989 James E. Wilson

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"

/* Define this to compile as a standalone test */
/* #define TEST_RNG */

/* This alg uses a prime modulus multiplicative congruential generator
   (PMMLCG), also known as a Lehmer Grammer, which satisfies the following
   properties

   (i)	 modulus: m - a large prime integer
   (ii)	 multiplier: a - an integer in the range 2, 3, ..., m - 1
   (iii) z[n+1] = f(z[n]), for n = 1, 2, ...
   (iv)	 f(z) = az mod m
   (v)	 u[n] = z[n] / m, for n = 1, 2, ...

   The sequence of z's must be initialized by choosing an initial seed
   z[1] from the range 1, 2, ..., m - 1.  The sequence of z's is a pseudo-
   random sequence drawn without replacement from the set 1, 2, ..., m - 1.
   The u's form a psuedo-random sequence of real numbers between (but not
   including) 0 and 1.

   Schrage's method is used to compute the sequence of z's.
   Let m = aq + r, where q = m div a, and r = m mod a.
   Then f(z) = az mod m = az - m * (az div m) =
	     = gamma(z) + m * delta(z)
   Where gamma(z) = a(z mod q) - r(z div q)
   and	 delta(z) = (z div q) - (az div m)

   If r < q, then for all z in 1, 2, ..., m - 1:
   (1) delta(z) is either 0 or 1
   (2) both a(z mod q) and r(z div q) are in 0, 1, ..., m - 1
   (3) absolute value of gamma(z) <= m - 1
   (4) delta(z) = 1 iff gamma(z) < 0

   Hence each value of z can be computed exactly without overflow as long
   as m can be represented as an integer.
 */

/* a good random number generator, correct on any machine with 32 bit integers,
   this algorithm is from:

Stephen K. Park and Keith W. Miller, "Random Number Generators:
	Good ones are hard to find", Communications of the ACM, October 1988,
	vol 31, number 10, pp. 1192-1201.

   If this algorithm is implemented correctly, then if z[1] = 1, then
   z[10001] will equal 1043618065

   Has a full period of 2^31 - 1.
   Returns integers in the range 1 to 2^31-1.
 */

#define RNG_M 2147483647L  /* m = 2^31 - 1 */
#define RNG_A 16807L
#define RNG_Q 127773L	   /* m div a */
#define RNG_R 2836L	   /* m mod a */

/* 32 bit seed */
static int32u rnd_seed;

int32u get_rnd_seed ()
{
  return rnd_seed;
}

void set_rnd_seed (seedval)
int32u seedval;
{
  /* set seed to value between 1 and m-1 */

  rnd_seed = (seedval % (RNG_M - 1)) + 1;
}

/* returns a pseudo-random number from set 1, 2, ..., RNG_M - 1 */
int32 rnd ()
{
  register long low, high, test;

  high = rnd_seed / RNG_Q;
  low = rnd_seed % RNG_Q;
  test = RNG_A * low - RNG_R * high;
  if (test > 0)
    rnd_seed = test;
  else
    rnd_seed = test + RNG_M;
  return rnd_seed;
}

#ifdef TEST_RNG

main ()
{
  long i, random;

  set_rnd_seed (0L);

  for (i = 1; i < 10000; i++)
    (void) rnd ();

  random = rnd ();
  printf ("z[10001] = %ld, should be 1043618065\n", random);
  if (random == 1043618065L)
    printf ("success!!!\n");
}

#endif
