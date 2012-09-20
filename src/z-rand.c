/* File: z-rand.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */


/*
 * This file provides an optimized random number generator.
 *
 *
 * This code provides both a "quick" random number generator (4 bytes of
 * state), and a "decent" random number generator (256 bytes of state),
 * both available in two flavors, first, the simple "mod" flavor, which
 * is fast, but slightly biased at high values, and second, the simple
 * "div" flavor, which is less fast (and potentially non-terminating)
 * but which is not biased and is much less subject to non-randomness
 * problems in the low bits.  Note the "rand_int()" macro in "z-rand.h",
 * which must specify a "default" flavor.
 *
 * Note the use of the "simple" RNG, first you activate it via
 * "Rand_quick = TRUE" and "Rand_value = seed" and then it is used
 * automatically used instead of the "complex" RNG, and when you are
 * done, you de-activate it via "Rand_quick = FALSE" or choose a new
 * seed via "Rand_value = seed".
 *
 *
 * This (optimized) random number generator is based loosely on the old
 * "random.c" file from Berkeley but with some major optimizations and
 * algorithm changes.  See below for more details.
 *
 * Some code by Ben Harrison (benh@phial.com).
 *
 * Some code by Randy (randy@stat.tamu.edu).
 */



#include "z-rand.h"


/*
 * Random Number Generator -- Linear Congruent RNG
 */
#define LCRNG(X)        ((X) * 1103515245 + 12345)



/*
 * Use the "simple" LCRNG
 */
bool Rand_quick = TRUE;


/*
 * Current "value" of the "simple" RNG
 */
u32b Rand_value;


/*
 * Current "index" for the "complex" RNG
 */
u16b Rand_place;

/*
 * Current "state" table for the "complex" RNG
 */
u32b Rand_state[RAND_DEG];



/*
 * Initialize the "complex" RNG using a new seed
 */
void Rand_state_init(u32b seed)
{
	int i, j;

	/* Seed the table */
	Rand_state[0] = seed;

	/* Propagate the seed */
	for (i = 1; i < RAND_DEG; i++) Rand_state[i] = LCRNG(Rand_state[i-1]);

	/* Cycle the table ten times per degree */
	for (i = 0; i < RAND_DEG * 10; i++)
	{
		/* Acquire the next index */
		j = Rand_place + 1;
		if (j == RAND_DEG) j = 0;

		/* Update the table, extract an entry */
		Rand_state[j] += Rand_state[Rand_place];

		/* Advance the index */
		Rand_place = j;
	}
}


/*
 * Extract a "random" number from 0 to m-1, via "division"
 *
 * This method selects "random" 28-bit numbers, and then uses
 * division to drop those numbers into "m" different partitions,
 * plus a small non-partition to reduce bias, taking as the final
 * value the first "good" partition that a number falls into.
 *
 * This method has no bias, and is much less affected by patterns
 * in the "low" bits of the underlying RNG's.
 *
 * Note that "m" must not be greater than 0x1000000, or division
 * by zero will result.
 *
 * ToDo: Check for m > 0x1000000.
 */
u32b Rand_div(u32b m)
{
	u32b r, n;

	/* Hack -- simple case */
	if (m <= 1) return (0);

	/* Partition size */
	n = (0x10000000 / m);

	/* Use a simple RNG */
	if (Rand_quick)
	{
		/* Wait for it */
		while (1)
		{
			/* Cycle the generator */
			r = (Rand_value = LCRNG(Rand_value));

			/* Mutate a 28-bit "random" number */
			r = ((r >> 4) & 0x0FFFFFFF) / n;

			/* Done */
			if (r < m) break;
		}
	}

	/* Use a complex RNG */
	else
	{
		/* Wait for it */
		while (1)
		{
			int j;

			/* Acquire the next index */
			j = Rand_place + 1;
			if (j == RAND_DEG) j = 0;

			/* Update the table, extract an entry */
			r = (Rand_state[j] += Rand_state[Rand_place]);

			/* Hack -- extract a 28-bit "random" number */
			r = ((r >> 4) & 0x0FFFFFFF) / n;

			/* Advance the index */
			Rand_place = j;

			/* Done */
			if (r < m) break;
		}
	}

	/* Use the value */
	return (r);
}




/*
 * The number of entries in the "Rand_normal_table"
 */
#define RANDNOR_NUM	256

/*
 * The standard deviation of the "Rand_normal_table"
 */
#define RANDNOR_STD	64

/*
 * The normal distribution table for the "Rand_normal()" function (below)
 */
static s16b Rand_normal_table[RANDNOR_NUM] =
{
	206,     613,    1022,    1430,		1838,	 2245,	  2652,	   3058,
	3463,    3867,    4271,    4673,	5075,	 5475,	  5874,	   6271,
	6667,    7061,    7454,    7845,	8234,	 8621,	  9006,	   9389,
	9770,   10148,   10524,   10898,   11269,	11638,	 12004,	  12367,
	12727,   13085,   13440,   13792,   14140,	14486,	 14828,	  15168,
	15504,   15836,   16166,   16492,   16814,	17133,	 17449,	  17761,
	18069,   18374,   18675,   18972,   19266,	19556,	 19842,	  20124,
	20403,   20678,   20949,   21216,   21479,	21738,	 21994,	  22245,

	22493,   22737,   22977,   23213,   23446,	23674,	 23899,	  24120,
	24336,   24550,   24759,   24965,   25166,	25365,	 25559,	  25750,
	25937,   26120,   26300,   26476,   26649,	26818,	 26983,	  27146,
	27304,   27460,   27612,   27760,   27906,	28048,	 28187,	  28323,
	28455,   28585,   28711,   28835,   28955,	29073,	 29188,	  29299,
	29409,   29515,   29619,   29720,   29818,	29914,	 30007,	  30098,
	30186,   30272,   30356,   30437,   30516,	30593,	 30668,	  30740,
	30810,   30879,   30945,   31010,   31072,	31133,	 31192,	  31249,

	31304,   31358,   31410,   31460,   31509,	31556,	 31601,	  31646,
	31688,   31730,   31770,   31808,   31846,	31882,	 31917,	  31950,
	31983,   32014,   32044,   32074,   32102,	32129,	 32155,	  32180,
	32205,   32228,   32251,   32273,   32294,	32314,	 32333,	  32352,
	32370,   32387,   32404,   32420,   32435,	32450,	 32464,	  32477,
	32490,   32503,   32515,   32526,   32537,	32548,	 32558,	  32568,
	32577,   32586,   32595,   32603,   32611,	32618,	 32625,	  32632,
	32639,   32645,   32651,   32657,   32662,	32667,	 32672,	  32677,

	32682,   32686,   32690,   32694,   32698,	32702,	 32705,	  32708,
	32711,   32714,   32717,   32720,   32722,	32725,	 32727,	  32729,
	32731,   32733,   32735,   32737,   32739,	32740,	 32742,	  32743,
	32745,   32746,   32747,   32748,   32749,	32750,	 32751,	  32752,
	32753,   32754,   32755,   32756,   32757,	32757,	 32758,	  32758,
	32759,   32760,   32760,   32761,   32761,	32761,	 32762,	  32762,
	32763,   32763,   32763,   32764,   32764,	32764,	 32764,	  32765,
	32765,   32765,   32765,   32766,   32766,	32766,	 32766,	  32767,
};



/*
 * Generate a random integer number of NORMAL distribution
 *
 * The table above is used to generate a psuedo-normal distribution,
 * in a manner which is much faster than calling a transcendental
 * function to calculate a true normal distribution.
 *
 * Basically, entry 64*N in the table above represents the number of
 * times out of 32767 that a random variable with normal distribution
 * will fall within N standard deviations of the mean.  That is, about
 * 68 percent of the time for N=1 and 95 percent of the time for N=2.
 *
 * The table above contains a "faked" final entry which allows us to
 * pretend that all values in a normal distribution are strictly less
 * than four standard deviations away from the mean.  This results in
 * "conservative" distribution of approximately 1/32768 values.
 *
 * Note that the binary search takes up to 16 quick iterations.
 */
s16b Rand_normal(int mean, int stand)
{
	s16b tmp;
	s16b offset;

	s16b low = 0;
	s16b high = RANDNOR_NUM;

	/* Paranoia */
	if (stand < 1) return (mean);

	/* Roll for probability */
	tmp = (s16b)rand_int(32768);

	/* Binary Search */
	while (low < high)
	{
		int mid = (low + high) >> 1;

		/* Move right if forced */
		if (Rand_normal_table[mid] < tmp)
		{
			low = mid + 1;
		}

		/* Move left otherwise */
		else
		{
			high = mid;
		}
	}

	/* Convert the index into an offset */
	offset = (long)stand * (long)low / RANDNOR_STD;

	/* One half should be negative */
	if (rand_int(100) < 50) return (mean - offset);

	/* One half should be positive */
	return (mean + offset);
}


/*
 * Extract a "random" number from 0 to m-1, using the "simple" RNG.
 *
 * This function should be used when generating random numbers in
 * "external" program parts like the main-*.c files.  It preserves
 * the current RNG state to prevent influences on game-play.
 *
 * Could also use rand() from <stdlib.h> directly. XXX XXX XXX
 */
u32b Rand_simple(u32b m)
{
	static bool initialized = FALSE;
	static u32b simple_rand_value;
	bool old_rand_quick;
	u32b old_rand_value;
	u32b result;


	/* Save RNG state */
	old_rand_quick = Rand_quick;
	old_rand_value = Rand_value;

	/* Use "simple" RNG */
	Rand_quick = TRUE;

	if (initialized)
	{
		/* Use stored seed */
		Rand_value = simple_rand_value;
	}
	else
	{
		/* Initialize with new seed */
		Rand_value = time(NULL);
		initialized = TRUE;
	}

	/* Get a random number */
	result = rand_int(m);

	/* Store the new seed */
	simple_rand_value = Rand_value;

	/* Restore RNG state */
	Rand_quick = old_rand_quick;
	Rand_value = old_rand_value;

	/* Use the value */
	return (result);
}
