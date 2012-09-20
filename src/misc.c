/* File: misc.c */

/* Purpose: misc code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * Angband 2.7.9 introduced a new (optimized) random number generator,
 * based loosely on the old "random.c" from Berkeley but with some major
 * optimizations and algorithm changes.  See below for more details.
 *
 * Code by myself (benh@voicenet.com) and Randy (randy@stat.tamu.edu).
 *
 * This code provides (1) a "decent" RNG, based on the "BSD-degree-63-RNG"
 * used in Angband 2.7.8, but rather optimized, and (2) a "simple" RNG,
 * based on the simple "LCRNG" currently used in Angband, but "corrected"
 * to give slightly better values.  Both of these are available in two
 * flavors, first, the simple "mod" flavor, which is fast, but slightly
 * biased at high values, and second, the simple "div" flavor, which is
 * less fast (and potentially non-terminating) but which is not biased
 * and is much less subject to low-bit-non-randomness problems.
 *
 * You can select your favorite flavor by proper definition of the
 * "rand_int()" macro in the "defines.h" file.
 *
 * Note that, in Angband 2.8.0, the "state" table will be saved in the
 * savefile, so a special "initialization" will be necessary.
 *
 * Note the use of the "simple" RNG, first you activate it via
 * "Rand_quick = TRUE" and "Rand_value = seed" and then it is used
 * automatically used instead of the "complex" RNG, and when you are
 * done, you de-activate it via "Rand_quick = FALSE" or choose a new
 * seed via "Rand_value = seed".
 */


/*
 * Random Number Generator -- Linear Congruent RNG
 */
#define LCRNG(X)        ((X) * 1103515245 + 12345)


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
    for (i = 0; i < RAND_DEG * 10; i++) {

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
 * Extract a "random" number from 0 to m-1, via "modulus"
 *
 * Note that "m" should probably be less than 500000, or the
 * results may be rather biased towards low values.
 */
s32b Rand_mod(s32b m)
{
    int j;
    u32b r;

    /* Hack -- simple case */
    if (m <= 1) return (0);

    /* Use the "simple" RNG */
    if (Rand_quick) {

        /* Cycle the generator */
        r = (Rand_value = LCRNG(Rand_value));
        
        /* Mutate a 28-bit "random" number */
        r = ((r >> 4) % m);
    }

    /* Use the "complex" RNG */
    else {
    
        /* Acquire the next index */
        j = Rand_place + 1;
        if (j == RAND_DEG) j = 0;

        /* Update the table, extract an entry */
        r = (Rand_state[j] += Rand_state[Rand_place]);

        /* Advance the index */
        Rand_place = j;

        /* Extract a "random" number */
        r = ((r >> 4) % m);
    }

    /* Use the value */
    return (r);
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
 */
s32b Rand_div(s32b m)
{
    u32b r, n;

    /* Hack -- simple case */
    if (m <= 1) return (0);

    /* Partition size */
    n = (0x10000000 / m);

    /* Use a simple RNG */
    if (Rand_quick) {

        /* Wait for it */
        do {

            /* Cycle the generator */
            r = (Rand_value = LCRNG(Rand_value));
        
            /* Mutate a 28-bit "random" number */
            r = (r >> 4) / n;

        } while (r >= m);
    }

    /* Use a complex RNG */
    else {
    
        /* Wait for it */
        do {

            int j;

            /* Acquire the next index */
            j = Rand_place + 1;
            if (j == RAND_DEG) j = 0;

            /* Update the table, extract an entry */
            r = (Rand_state[j] += Rand_state[Rand_place]);

            /* Hack -- extract a 28-bit "random" number */
            r = (r >> 4) / n;

            /* Advance the index */
            Rand_place = j;

        } while (r >= m);
    }

    /* Use the value */
    return (r);
}




/*
 * The number of entries in the "randnor_table"
 */
#define RANDNOR_NUM	256

/*
 * The standard deviation of the "randnor_table"
 */
#define RANDNOR_STD	64

/*
 * The normal distribution table for the "randnor()" function (below)
 */
static s16b randnor_table[RANDNOR_NUM] = {

     206,     613,    1022,    1430,	1838,	 2245,	  2652,	   3058,
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
s16b randnor(int mean, int stand)
{
    s16b tmp;
    s16b offset;

    s16b low = 0;
    s16b high = RANDNOR_NUM;

    /* Paranoia */
    if (stand < 1) return (mean);
    
    /* Roll for probability */
    tmp = rand_int(32768);

    /* Binary Search */
    while (low < high) {

        int mid = (low + high) >> 1;

        /* Move right if forced */
        if (randnor_table[mid] < tmp) {
            low = mid + 1;
        }

        /* Move left otherwise */
        else {
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
 * Generates damage for "2d6" style dice rolls
 */
s16b damroll(int num, int sides)
{
    int i, sum = 0;
    for (i = 0; i < num; i++) sum += randint(sides);
    return (sum);
}


/*
 * Same as above, but always maximal
 */
s16b maxroll(int num, int sides)
{
    return (num * sides);
}



/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(vptr u, vptr v, int p, int q)
{
    int z, a, b;
    
    /* Done sort */
    if (p >= q) return;

    /* Pivot */
    z = p;

    /* Begin */
    a = p;
    b = q;

    /* Partition */
    while (TRUE) {

        /* Slide i2 */
        while (!(*ang_sort_comp)(u, v, b, z)) b--;

        /* Slide i1 */
        while (!(*ang_sort_comp)(u, v, z, a)) a++;

        /* Done partition */
        if (a >= b) break;

        /* Swap */
        (*ang_sort_swap)(u, v, a, b);

        /* Advance */
        a++, b--;
    }

    /* Recurse left side */
    ang_sort_aux(u, v, p, b);

    /* Recurse right side */
    ang_sort_aux(u, v, b+1, q);    
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(vptr u, vptr v, int n)
{
    /* Sort the array */
    ang_sort_aux(u, v, 0, n-1);
}


/*
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * Currently the "m" parameter is unused.
 */
void scatter(int *yp, int *xp, int y, int x, int d, int m)
{
    int nx, ny;

    /* Pick a location */
    while (TRUE) {

        /* Pick a new location */
        ny = rand_spread(y, d);
        nx = rand_spread(x, d);

        /* Ignore illegal locations and outer walls */
        if (!in_bounds(y, x)) continue;

        /* Ignore "excessively distant" locations */
        if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;

        /* Require "line of sight" */
        if (los(y, x, ny, nx)) break;
    }

    /* Save the location */
    (*yp) = ny;
    (*xp) = nx;
}


/*
 * Verify desire to be a wizard, and do so if verified
 * This routine should only be called if "can_be_wizard"
 */
bool enter_wiz_mode(void)
{
    int answer = FALSE;

    /* Already been asked */
    if (noscore & 0x0002) return (TRUE);

    /* Mention effects */
    msg_print("Wizard mode is for debugging and experimenting.");
    msg_print("The game will not be scored if you enter wizard mode.");

    /* Verify request */
    answer = get_check("Are you sure you want to enter wizard mode? ");

    /* Never Mind */
    if (!answer) return (FALSE);

    /* Remember old setting */
    noscore |= 0x0002;

    /* Make me a wizard */
    return (TRUE);
}


/*
 * Extract and set the current "view radius"
 */
void extract_cur_view(void)
{
    /* Assume "full" view */
    p_ptr->cur_view = MAX_SIGHT;
    
    /* Paranoia -- maximum view of 20 */
    if (p_ptr->cur_view > 20) p_ptr->cur_view = 20;
    
    /* Reduce view when running if requested */
    if (running &&
        (view_reduce_view || (!dun_level && view_reduce_view_town))) {

        /* Reduce the view (based on distance run) */
        p_ptr->cur_view -= ((running < 10) ? running : 10);
    }

    /* Notice changes in the "view radius" */
    if (p_ptr->old_view != p_ptr->cur_view) {

        /* Update the lite */
        p_ptr->update |= (PU_VIEW);

        /* Update the monsters */
        p_ptr->update |= (PU_MONSTERS);

        /* Remember the old view */
        p_ptr->old_view = p_ptr->cur_view;
    }
}


/*
 * Extract and set the current "lite radius"
 */
void extract_cur_lite(void)
{
    inven_type *i_ptr = &inventory[INVEN_LITE];

    /* Assume no light */
    p_ptr->cur_lite = 0;

    /* Player is glowing */
    if (p_ptr->lite) p_ptr->cur_lite = 1;

    /* Examine actual lites */
    if (i_ptr->tval == TV_LITE) {

        /* Torches (with fuel) provide some lite */
        if ((i_ptr->sval == SV_LITE_TORCH) && (i_ptr->pval > 0)) {
            p_ptr->cur_lite = 1;
        }

        /* Lanterns (with fuel) provide more lite */
        if ((i_ptr->sval == SV_LITE_LANTERN) && (i_ptr->pval > 0)) {
            p_ptr->cur_lite = 2;
        }

        /* Artifact Lites provide permanent, bright, lite */
        if (artifact_p(i_ptr)) p_ptr->cur_lite = 3;
    }
    
    /* Reduce lite when running if requested */
    if (running &&
        (view_reduce_lite || (!dun_level && view_reduce_lite_town))) {

        /* Reduce the lite radius if needed */
        if (p_ptr->cur_lite > 1) p_ptr->cur_lite = 1;
    }

    /* Notice changes in the "lite radius" */
    if (p_ptr->old_lite != p_ptr->cur_lite) {

        /* Update the lite */
        p_ptr->update |= (PU_LITE);

        /* Update the monsters */
        p_ptr->update |= (PU_MONSTERS);

        /* Remember the old lite */
        p_ptr->old_lite = p_ptr->cur_lite;
    }
}



#if 0

/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * XXX Could be made more efficient, especially in the
 * case where "insert" is smaller than "target".
 */
static bool insert_str(char *buf, cptr target, cptr insert)
{
    int   i, len;
    int		   b_len, t_len, i_len;

    /* Attempt to find the target (modify "buf") */
    buf = strstr(buf, target);

    /* No target found */
    if (!buf) return (FALSE);

    /* Be sure we have an insertion string */
    if (!insert) insert = "";

    /* Extract some lengths */
    t_len = strlen(target);
    i_len = strlen(insert);
    b_len = strlen(buf);

    /* How much "movement" do we need? */
    len = i_len - t_len;

    /* We need less space (for insert) */
    if (len < 0) {
        for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
    }

    /* We need more space (for insert) */
    else if (len > 0) {
        for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
    }

    /* If movement occured, we need a new terminator */
    if (len) buf[b_len+len] = '\0';

    /* Now copy the insertion string */
    for (i = 0; i < i_len; ++i) buf[i] = insert[i];

    /* Successful operation */
    return (TRUE);
}


#endif



/*
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
s16b modify_stat_value(int value, int amount)
{
    int    i;

    /* Reward */
    if (amount > 0) {

        /* Apply each point */
        for (i = 0; i < amount; i++) {

            /* One point at a time */
            if (value < 18) value++;

            /* Ten "points" at a time */
            else value += 10;
        }
    }

    /* Penalty */
    else if (amount < 0) {

        /* Apply each point */
        for (i = 0; i < (0 - amount); i++) {

            /* Ten points at a time */
            if (value >= 18+10) value -= 10;

            /* Hack -- prevent weirdness */
            else if (value > 18) value = 18;

            /* One point at a time */
            else if (value > 3) value--;
        }
    }

    /* Return new value */
    return (value);
}



/*
 * Increases a stat by one randomized level		-RAK-	
 *
 * Note that this function (used by stat potions) now restores
 * the stat BEFORE increasing it.
 */
bool inc_stat(int stat)
{
    int value, gain;

    /* Then augment the current/max stat */
    value = p_ptr->stat_cur[stat];

    /* Cannot go above 18/100 */
    if (value < 18+100) {

        /* Gain one (sometimes two) points */
        if (value < 18) {	
            gain = ((rand_int(100) < 75) ? 1 : 2);
            value += gain;
        }

        /* Gain 1/6 to 1/3 of distance to 18/100 */
        else if (value < 18+98) {

            /* Approximate gain value */
            gain = (((18+100) - value) / 2 + 3) / 2;

            /* Paranoia */
            if (gain < 1) gain = 1;
            
            /* Apply the bonus */
            value += randint(gain) + gain / 2;

            /* Maximal value */
            if (value > 18+99) value = 18 + 99;
        }

        /* Gain one point at a time */
        else {
            value++;
        }

        /* Save the new value */
        p_ptr->stat_cur[stat] = value;

        /* Bring up the maximum too */
        if (value > p_ptr->stat_max[stat]) {
            p_ptr->stat_max[stat] = value;
        }

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to gain */
    return (FALSE);
}



/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Amount could be a little higher in extreme cases to mangle very high
 * stats from massive assaults.  -CWS
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool dec_stat(int stat, int amount, int permanent)
{
    int cur, max, loss, same, res = FALSE;


    /* Acquire current value */
    cur = p_ptr->stat_cur[stat];
    max = p_ptr->stat_max[stat];

    /* Note when the values are identical */
    same = (cur == max);

    /* Damage "current" value */
    if (cur > 3) {

        /* Handle "low" values */
        if (cur <= 18) {

            if (amount > 90) cur--;
            if (amount > 50) cur--;
            if (amount > 20) cur--;
            cur--;
        }

        /* Handle "high" values */
        else {

            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((cur-18) / 2 + 1) / 2 + 1);
            
            /* Paranoia */
            if (loss < 1) loss = 1;
            
            /* Randomize the loss */
            loss = ((randint(loss) + loss) * amount) / 100;

            /* Maximal loss */
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            cur = cur - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (cur < 18) cur = (amount <= 20) ? 18 : 17;
        }

        /* Prevent illegal values */
        if (cur < 3) cur = 3;

        /* Something happened */
        if (cur != p_ptr->stat_cur[stat]) res = TRUE;
    }

    /* Damage "max" value */
    if (permanent && (max > 3)) {

        /* Handle "low" values */
        if (max <= 18) {

            if (amount > 90) max--;
            if (amount > 50) max--;
            if (amount > 20) max--;
            max--;
        }

        /* Handle "high" values */
        else {

            /* Hack -- Decrement by a random amount between one-quarter */
            /* and one-half of the stat bonus times the percentage, with a */
            /* minimum damage of half the percentage. -CWS */
            loss = (((max-18) / 2 + 1) / 2 + 1);
            loss = ((randint(loss) + loss) * amount) / 100;
            if (loss < amount/2) loss = amount/2;

            /* Lose some points */
            max = max - loss;

            /* Hack -- Only reduce stat to 17 sometimes */
            if (max < 18) max = (amount <= 20) ? 18 : 17;
        }

        /* Hack -- keep it clean */
        if (same || (max < cur)) max = cur;

        /* Something happened */
        if (max != p_ptr->stat_max[stat]) res = TRUE;
    }

    /* Apply changes */
    if (res) {

        /* Actually set the stat to its new value. */
        p_ptr->stat_cur[stat] = cur;
        p_ptr->stat_max[stat] = max;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    }

    /* Done */
    return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
    /* Restore if needed */
    if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat]) {

        /* Restore */
        p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Success */
        return (TRUE);
    }

    /* Nothing to restore */
    return (FALSE);
}




/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val)
{
    /* Above 18 */
    if (val > 18) {

        int bonus = (val - 18);

        if (bonus >= 220) {
            sprintf(out_val, "18/%3s", "***");
        }
        else if (bonus >= 100) {
            sprintf(out_val, "18/%03d", bonus);
        }
        else {
            sprintf(out_val, " 18/%02d", bonus);
        }
    }

    /* From 3 to 18 */
    else {
        sprintf(out_val, "    %2d", val);
    }
}




/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(cptr info, int row, int col)
{
    /* Dump 13 spaces to clear */
    c_put_str(TERM_WHITE, "             ", row, col);

    /* Dump the info itself */
    c_put_str(TERM_L_BLUE, info, row, col);
}




/*
 * Print character stat in given row, column
 */
static void prt_stat(int stat)
{
    char tmp[32];

    /* Display "injured" stat */
    if (p_ptr->stat_cur[stat] < p_ptr->stat_max[stat]) {
        put_str(stat_names_reduced[stat], ROW_STAT + stat, 0);
        cnv_stat(p_ptr->stat_use[stat], tmp);
        c_put_str(TERM_YELLOW, tmp, ROW_STAT + stat, COL_STAT + 6);
    }

    /* Display "healthy" stat */
    else {
        put_str(stat_names[stat], ROW_STAT + stat, 0);
        cnv_stat(p_ptr->stat_use[stat], tmp);
        c_put_str(TERM_L_GREEN, tmp, ROW_STAT + stat, COL_STAT + 6);
    }
}




/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title()
{
    /* Wizard */
    if (wizard) {
        prt_field("[=-WIZARD-=]", ROW_TITLE, COL_TITLE);
    }

    /* Winner */
    else if (total_winner || (p_ptr->lev > PY_MAX_LEVEL)) {
        prt_field((p_ptr->male ? "**KING**" : "**QUEEN**"),
                  ROW_TITLE, COL_TITLE);
    }

#ifdef ALLOW_TITLES

    /* Oops */
    else if (p_ptr->lev < 1) {
        prt_field("Novice", ROW_TITLE, COL_TITLE);
    }

    /* Normal */    
    else {
        prt_field(player_title[p_ptr->pclass][p_ptr->lev - 1],
                  ROW_TITLE, COL_TITLE);
    }

#else

    /* Nothing */
    else {

        prt_field("", ROW_TITLE, COL_TITLE);
    }
    
#endif

}


/*
 * Prints level
 */
static void prt_level()
{
    char tmp[32];

    sprintf(tmp, "%6d", p_ptr->lev);

    if (p_ptr->lev >= p_ptr->max_plv) {
        put_str("LEVEL ", ROW_LEVEL, 0);
        c_put_str(TERM_L_GREEN, tmp, ROW_LEVEL, COL_LEVEL + 6);
    }
    else {
        put_str("Level ", ROW_LEVEL, 0);
        c_put_str(TERM_YELLOW, tmp, ROW_LEVEL, COL_LEVEL + 6);
    }
}


/*
 * Display the experience
 */
static void prt_exp()
{
    char out_val[32];

    (void)sprintf(out_val, "%8ld", (long)p_ptr->exp);

    if (p_ptr->exp >= p_ptr->max_exp) {
        put_str("EXP ", ROW_EXP, 0);
        c_put_str(TERM_L_GREEN, out_val, ROW_EXP, COL_EXP + 4);
    }
    else {
        put_str("Exp ", ROW_EXP, 0);
        c_put_str(TERM_YELLOW, out_val, ROW_EXP, COL_EXP + 4);
    }
}


/*
 * Prints current gold
 */
static void prt_gold()
{
    char tmp[32];

    put_str("AU ", ROW_GOLD, COL_GOLD);
    sprintf(tmp, "%9ld", (long)p_ptr->au);
    c_put_str(TERM_L_GREEN, tmp, ROW_GOLD, COL_GOLD + 3);
}



/*
 * Prints current AC
 */
static void prt_ac()
{
    char tmp[32];

    put_str("Cur AC ", ROW_AC, COL_AC);
    sprintf(tmp, "%5d", p_ptr->dis_ac + p_ptr->dis_to_a);
    c_put_str(TERM_L_GREEN, tmp, ROW_AC, COL_AC + 7);
}


/*
 * Prints Cur/Max hit points
 */
static void prt_hp()
{
    char tmp[32];

    byte color;


    put_str("Max HP ", ROW_MAXHP, COL_MAXHP);

    sprintf(tmp, "%5d", p_ptr->mhp);
    color = TERM_L_GREEN;

    c_put_str(color, tmp, ROW_MAXHP, COL_MAXHP + 7);


    put_str("Cur HP ", ROW_CURHP, COL_CURHP);

    sprintf(tmp, "%5d", p_ptr->chp);

    if (p_ptr->chp >= p_ptr->mhp) {
        color = TERM_L_GREEN;
    }
    else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10) {
        color = TERM_YELLOW;
    }
    else {
        color = TERM_RED;
    }

    c_put_str(color, tmp, ROW_CURHP, COL_CURHP + 7);
}


/*
 * Prints players max/cur spell points
 */
static void prt_sp()
{
    char tmp[32];
    byte color;


    /* Do not show mana unless it matters */
    if (!mp_ptr->spell_book) return;


    put_str("Max SP ", ROW_MAXSP, COL_MAXSP);

    sprintf(tmp, "%5d", p_ptr->msp);
    color = TERM_L_GREEN;

    c_put_str(color, tmp, ROW_MAXSP, COL_MAXSP + 7);


    put_str("Cur SP ", ROW_CURSP, COL_CURSP);

    sprintf(tmp, "%5d", p_ptr->csp);

    if (p_ptr->csp >= p_ptr->msp) {
        color = TERM_L_GREEN;
    }
    else if (p_ptr->csp > (p_ptr->msp * hitpoint_warn) / 10) {
        color = TERM_YELLOW;
    }
    else {
        color = TERM_RED;
    }

    /* Show mana */
    c_put_str(color, tmp, ROW_CURSP, COL_CURSP + 7);
}


/*
 * Prints depth in stat area
 */
static void prt_depth()
{
    char depths[32];

    if (!dun_level) {
        (void)strcpy(depths, "Town");
    }
    else if (depth_in_feet) {
        (void)sprintf(depths, "%d ft", dun_level * 50);
    }
    else {
        (void)sprintf(depths, "Lev %d", dun_level);
    }

    /* Right-Adjust the "depth", and clear old values */
    prt(format("%7s", depths), 23, COL_DEPTH);
}


/*
 * Prints status of hunger
 */
static void prt_hunger()
{
    /* Fainting / Starving */
    if (p_ptr->food < PY_FOOD_FAINT) {
        c_put_str(TERM_RED, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
    }

    /* Weak */
    else if (p_ptr->food < PY_FOOD_WEAK) {
        c_put_str(TERM_ORANGE, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
    }

    /* Hungry */
    else if (p_ptr->food < PY_FOOD_ALERT) {
        c_put_str(TERM_YELLOW, "Hungry", ROW_HUNGRY, COL_HUNGRY);
    }

    /* Normal */
    else if (p_ptr->food < PY_FOOD_FULL) {
        c_put_str(TERM_L_GREEN, "      ", ROW_HUNGRY, COL_HUNGRY);
    }

    /* Full */
    else if (p_ptr->food < PY_FOOD_MAX) {
        c_put_str(TERM_L_GREEN, "Full  ", ROW_HUNGRY, COL_HUNGRY);
    }

    /* Gorged */
    else {
        c_put_str(TERM_GREEN, "Gorged", ROW_HUNGRY, COL_HUNGRY);
    }
}


/*
 * Prints Blind status
 */
static void prt_blind(void)
{
    if (p_ptr->blind) {
        c_put_str(TERM_ORANGE, "Blind", ROW_BLIND, COL_BLIND);
    }
    else {
        put_str("     ", ROW_BLIND, COL_BLIND);
    }
}


/*
 * Prints Confusion status
 */
static void prt_confused(void)
{
    if (p_ptr->confused) {
        c_put_str(TERM_ORANGE, "Confused", ROW_CONFUSED, COL_CONFUSED);
    }
    else {
        put_str("        ", ROW_CONFUSED, COL_CONFUSED);
    }
}


/*
 * Prints Fear status
 */
static void prt_afraid()
{
    if (p_ptr->afraid) {
        c_put_str(TERM_ORANGE, "Afraid", ROW_AFRAID, COL_AFRAID);
    }
    else {
        put_str("      ", ROW_AFRAID, COL_AFRAID);
    }
}


/*
 * Prints Poisoned status
 */
static void prt_poisoned(void)
{
    if (p_ptr->poisoned) {
        c_put_str(TERM_ORANGE, "Poisoned", ROW_POISONED, COL_POISONED);
    }
    else {
        put_str("        ", ROW_POISONED, COL_POISONED);
    }
}


/*
 * Prints Searching, Resting, Paralysis, or 'count' status
 * Display is always exactly 10 characters wide (see below)
 */
static void prt_state(void)
{
    byte attr = TERM_WHITE;

    char text[16];


    /* Paralysis */
    if (p_ptr->paralyzed) {

        attr = TERM_RED;

        strcpy(text, "Paralyzed!");
    }

    /* Resting */
    else if (resting) {

        int i;

        /* Start with "Rest" */
        strcpy(text, "Rest      ");

        /* Extensive (timed) rest */
        if (resting >= 1000) {
            i = resting / 100;
            text[9] = '0';
            text[8] = '0';
            text[7] = '0' + (i % 10);
            if (i >= 10) {
                i = i / 10;
                text[6] = '0' + (i % 10);
                if (i >= 10) {
                    text[5] = '0' + (i / 10);
                }
            }
        }

        /* Long (timed) rest */
        else if (resting >= 100) {
            i = resting;
            text[9] = '0' + (i % 10);
            i = i / 10;
            text[8] = '0' + (i % 10);
            text[7] = '0' + (i / 10);
        }

        /* Medium (timed) rest */
        else if (resting >= 10) {
            i = resting;
            text[9] = '0' + (i % 10);
            text[8] = '0' + (i / 10);
        }

        /* Short (timed) rest */
        else if (resting > 0) {
            i = resting;
            text[9] = '0' + (i);
        }

        /* Rest until healed */
        else if (resting == -1) {
            text[5] = text[6] = text[7] = text[8] = text[9] = '*';
        }

        /* Rest until done */
        else if (resting == -2) {
            text[5] = text[6] = text[7] = text[8] = text[9] = '&';
        }
    }

    /* Repeating */
    else if (command_rep) {

        if (command_rep > 999) {
            (void)sprintf(text, "Rep. %3d00", command_rep / 100);
        }
        else {
            (void)sprintf(text, "Repeat %3d", command_rep);
        }
    }

    /* Searching */
    else if (p_ptr->searching) {

        strcpy(text, "Searching ");
    }

    /* Nothing interesting */
    else {

        strcpy(text, "          ");
    }

    /* Display the info (or blanks) */
    c_put_str(attr, text, ROW_STATE, COL_STATE);
}


/*
 * Prints the speed of a character.			-CJS-
 */
static void prt_speed()
{
    int i = p_ptr->pspeed;

    int attr = TERM_WHITE;
    char buf[32] = "";

    /* Hack -- Visually "undo" the Search Mode Slowdown */
    if (p_ptr->searching) i += 10;

    /* Fast */
    if (i > 110) {
        attr = TERM_L_GREEN;
        sprintf(buf, "Fast (+%d)", (i - 110));
    }

    /* Slow */
    else if (i < 110) {
        attr = TERM_L_UMBER;
        sprintf(buf, "Slow (-%d)", (110 - i));
    }

    /* Display the speed */
    c_put_str(attr, format("%-14s", buf), ROW_SPEED, COL_SPEED);
}


static void prt_study()
{
    if (p_ptr->new_spells) {
        put_str("Study", ROW_STUDY, 64);
    }
    else {
        put_str("     ", ROW_STUDY, COL_STUDY);
    }
}


static void prt_cut()
{
    int c = p_ptr->cut;

    if (c > 1000) {
        c_put_str(TERM_L_RED, "Mortal wound", ROW_CUT, COL_CUT);
    }
    else if (c > 200) {
        c_put_str(TERM_RED, "Deep gash   ", ROW_CUT, COL_CUT);
    }
    else if (c > 100) {
        c_put_str(TERM_RED, "Severe cut  ", ROW_CUT, COL_CUT);
    }
    else if (c > 50) {
        c_put_str(TERM_ORANGE, "Nasty cut   ", ROW_CUT, COL_CUT);
    }
    else if (c > 25) {
        c_put_str(TERM_ORANGE, "Bad cut     ", ROW_CUT, COL_CUT);
    }
    else if (c > 10) {
        c_put_str(TERM_YELLOW, "Light cut   ", ROW_CUT, COL_CUT);
    }
    else if (c) {
        c_put_str(TERM_YELLOW, "Graze       ", ROW_CUT, COL_CUT);
    }
    else {
        put_str("            ", ROW_CUT, COL_CUT);
    }
}



static void prt_stun(void)
{
    int s = p_ptr->stun;

    if (s > 100) {
        c_put_str(TERM_RED, "Knocked out ", ROW_STUN, COL_STUN);
    }
    else if (s > 50) {
        c_put_str(TERM_ORANGE, "Heavy stun  ", ROW_STUN, COL_STUN);
    }
    else if (s) {
        c_put_str(TERM_ORANGE, "Stun        ", ROW_STUN, COL_STUN);
    }
    else {
        put_str("            ", ROW_STUN, COL_STUN);
    }
}



/*
 * Display the "equippy chars"
 */
static void prt_equippy_chars(void)
{
    int i, j;
    inven_type *i_ptr;

    byte attr;
    char out_val[2];


    /* Hack -- skip if requested */
    if (!equippy_chars) return;


    /* Wipe the equippy chars */
    put_str("            ", ROW_INFO, COL_INFO);

    /* Analyze the pack */
    for (j = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++, j++) {

        /* Get the item */
        i_ptr = &inventory[i];

        /* Skip empty slots */
        if (!i_ptr->k_idx) continue;

        /* Hack -- remap unless using graphics */
        if (use_graphics) {

            /* Extract the color */
            attr = inven_attr(i_ptr);

            /* Extract the symbol */
            out_val[0] = inven_char(i_ptr);
        }

        /* Normal display */
        else {

            /* Extract the attr */
            attr = tval_to_attr[i_ptr->tval];

            /* Extract the char */
            out_val[0] = tval_to_char[i_ptr->tval];
        }

        /* Hack -- terminate the string */
        out_val[1] = '\0';

        /* Display the item symbol */
        c_put_str(attr, out_val, ROW_INFO, COL_INFO + j);
    }
}


/*
 * Redraw the "monster health bar"	-DRS-
 * Rather extensive modifications by	-BEN-
 *
 * The "monster health bar" is drawn instead of the "equippy chars"
 * and provides visual feedback on the "health" of the "tracked"
 * monster.  Attacking a monster auto-tracks it, and otherwise the
 * health bat tracks the current target monster if any.
 *
 * This is a spoiler, but not a major one, since you can get most
 * of that information by "looking" at the monster.
 *
 * Display the monster health bar (affectionately known as the
 * "health-o-meter").  Clear health bar if nothing is being tracked.
 * Auto-track current target monster when bored.  Note that the
 * health-bar stops tracking any monster that "disappears".
 */
void health_redraw(void)
{

#ifdef DRS_SHOW_HEALTH_BAR

    /* Disabled */
    if (!show_health_bar) return;

    /* Hack -- not tracking, auto-track current "target" */
    if (!health_who) {

        /* Examine legal current monster targets */
        if ((target_who > 0) && target_okay()) {

            /* Auto-track */
            health_who = target_who;
        }
    }

    /* Not tracking */
    if (!health_who) {

        /* Erase the health bar */
        Term_erase(COL_INFO, ROW_INFO, 12, 1);

        /* Mega-Hack -- Equippy chars (if enabled) */
        if (equippy_chars) prt_equippy_chars();
    }

    /* Tracking an unseen monster */
    else if (!m_list[health_who].ml) {

        /* Indicate that the monster health is "unknown" */
        Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
    }

    /* Tracking a hallucinatory monster */
    else if (p_ptr->image) {

        /* Indicate that the monster health is "unknown" */
        Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
    }

    /* Tracking a dead monster (???) */
    else if (!m_list[health_who].hp < 0) {

        /* Indicate that the monster health is "unknown" */
        Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
    }

    /* Tracking a visible monster */
    else {

        int pct, len;

        monster_type *m_ptr = &m_list[health_who];

        /* Default to almost dead */
        byte attr = TERM_RED;

        /* Extract the "percent" of health */
        pct = 100L * m_ptr->hp / m_ptr->maxhp;

        /* Badly wounded */
        if (pct >= 10) attr = TERM_L_RED;

        /* Wounded */
        if (pct >= 25) attr = TERM_ORANGE;

        /* Somewhat Wounded */
        if (pct >= 60) attr = TERM_YELLOW;

        /* Healthy */
        if (pct >= 100) attr = TERM_L_GREEN;

        /* Afraid */
        if (m_ptr->monfear) attr = TERM_VIOLET;

        /* Asleep */
        if (m_ptr->csleep) attr = TERM_BLUE;

        /* Convert percent into "health" */
        len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;

        /* Default to "unknown" */
        Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");

        /* Dump the current "health" (use '*' symbols) */
        Term_putstr(COL_INFO + 1, ROW_INFO, len, attr, "**********");
    }

#endif

}



/*
 * Display basic info (mostly left of map)
 */
static void prt_frame_basic()
{
    int i;

    /* Race and Class */
    prt_field(rp_ptr->title, ROW_RACE, COL_RACE);
    prt_field(cp_ptr->title, ROW_CLASS, COL_CLASS);

    /* Title */
    prt_title();

    /* Level/Experience */
    prt_level();
    prt_exp();

    /* All Stats */
    for (i = 0; i < 6; i++) prt_stat(i);

    /* Armor */
    prt_ac();

    /* Hitpoints */
    prt_hp();

    /* Spell points (mana) */
    prt_sp();

    /* Gold */
    prt_gold();

    /* Special */
    if (equippy_chars) prt_equippy_chars();
    if (show_health_bar) health_redraw();

    /* Current depth (bottom right) */
    prt_depth();
}


/*
 * Display extra info (mostly below map)
 */
static void prt_frame_extra()
{
    /* Cut/Stun */
    prt_cut();
    prt_stun();

    /* Food */
    prt_hunger();

    /* Various */
    prt_blind();
    prt_confused();
    prt_afraid();
    prt_poisoned();

    /* State */
    prt_state();

    /* Speed */
    prt_speed();

    /* Study spells */
    prt_study();
}



/*
 * Print long number with header at given row, column
 * Use the color for the number, not the header
 */
static void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    (void)sprintf(out_val, "%9ld", (long)num);
    c_put_str(color, out_val, row, col + len);
}

/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    put_str("   ", row, col + len);
    (void)sprintf(out_val, "%6ld", (long)num);
    c_put_str(color, out_val, row, col + len + 3);
}





/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
static void display_player_middle(void)
{
    int show_tohit = p_ptr->dis_to_h;
    int show_todam = p_ptr->dis_to_d;

    inven_type *i_ptr = &inventory[INVEN_WIELD];

    /* Hack -- add in weapon info if known */
    if (inven_known_p(i_ptr)) show_tohit += i_ptr->to_h;
    if (inven_known_p(i_ptr)) show_todam += i_ptr->to_d;

    /* Dump the bonuses to hit/dam */
    prt_num("+ To Hit    ", show_tohit, 9, 1, TERM_L_BLUE);
    prt_num("+ To Damage ", show_todam, 10, 1, TERM_L_BLUE);

    /* Dump the armor class bonus */
    prt_num("+ To AC     ", p_ptr->dis_to_a, 11, 1, TERM_L_BLUE);

    /* Dump the total armor class */
    prt_num("  Base AC   ", p_ptr->dis_ac, 12, 1, TERM_L_BLUE);

    prt_num("Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN);

    if (p_ptr->exp >= p_ptr->max_exp) {
        prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_L_GREEN);
    }
    else {
        prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_YELLOW);
    }

    prt_lnum("Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);

    if (p_ptr->lev >= PY_MAX_LEVEL) {
        put_str("Exp to Adv.", 12, 28);
        c_put_str(TERM_L_GREEN, "    *****", 12, 28+11);
    }
    else {
        prt_lnum("Exp to Adv.",
                 (s32b)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L),
                 12, 28, TERM_L_GREEN);
    }

    prt_lnum("Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);

    prt_num("Max Hit Points ", p_ptr->mhp, 9, 52, TERM_L_GREEN);

    if (p_ptr->chp >= p_ptr->mhp) {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_L_GREEN);
    }
    else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10) {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_YELLOW);
    }
    else {
        prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_RED);
    }

    prt_num("Max SP (Mana)  ", p_ptr->msp, 11, 52, TERM_L_GREEN);

    if (p_ptr->csp >= p_ptr->msp) {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_L_GREEN);
    }
    else if (p_ptr->csp > (p_ptr->msp * hitpoint_warn) / 10) {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_YELLOW);
    }
    else {
        prt_num("Cur SP (Mana)  ", p_ptr->csp, 12, 52, TERM_RED);
    }
}




/*
 * Hack -- pass color info around this file
 */
static byte likert_color = TERM_WHITE;


/*
 * Returns a "rating" of x depending on y
 */
cptr likert(int x, int y)
{
    /* Paranoia */
    if (y <= 0) y = 1;

    /* Negative value */
    if (x < 0) {
        likert_color = TERM_RED;
        return ("Very Bad");
    }

    /* Analyze the value */
    switch ((x / y)) {
      case 0:
      case 1:
        likert_color = TERM_RED;
        return ("Bad");
      case 2:
        likert_color = TERM_RED;
        return ("Poor");
      case 3:
      case 4:
        likert_color = TERM_YELLOW;
        return ("Fair");
      case 5:
        likert_color = TERM_YELLOW;
        return ("Good");
      case 6:
        likert_color = TERM_YELLOW;
        return ("Very Good");
      case 7:
      case 8:
        likert_color = TERM_L_GREEN;
        return ("Excellent");
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
        likert_color = TERM_L_GREEN;
        return ("Superb");
      case 14:
      case 15:
      case 16:
      case 17:
        likert_color = TERM_L_GREEN;
        return ("Heroic");
      default:
        likert_color = TERM_L_GREEN;
        return ("Legendary");
    }
}


/*
 * Prints ratings on certain abilities
 *
 * This code is "imitated" elsewhere to "dump" a character sheet.
 */
static void display_player_various(void)
{
    int			tmp;
    int			xthn, xthb, xfos, xsrh;
    int			xdis, xdev, xsav, xstl;
    cptr		desc;

    inven_type		*i_ptr;
    

    /* Fighting Skill (with current weapon) */
    i_ptr = &inventory[INVEN_WIELD];
    tmp = p_ptr->to_h + i_ptr->to_h;
    xthn = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);

    /* Shooting Skill (with current bow and normal missile) */
    i_ptr = &inventory[INVEN_BOW];
    tmp = p_ptr->to_h + i_ptr->to_h;
    xthb = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);

    /* Basic abilities */
    xdis = p_ptr->skill_dis;
    xdev = p_ptr->skill_dev;
    xsav = p_ptr->skill_sav;
    xstl = p_ptr->skill_stl;
    xsrh = p_ptr->skill_srh;
    xfos = p_ptr->skill_fos;


    put_str("Fighting    :", 16, 1);
    desc = likert(xthn, 12);
    c_put_str(likert_color, desc, 16, 15);

    put_str("Bows/Throw  :", 17, 1);
    desc = likert(xthb, 12);
    c_put_str(likert_color, desc, 17, 15);

    put_str("Saving Throw:", 18, 1);
    desc = likert(xsav, 6);
    c_put_str(likert_color, desc, 18, 15);

    put_str("Stealth     :", 19, 1);
    desc = likert(xstl, 1);
    c_put_str(likert_color, desc, 19, 15);


    put_str("Perception  :", 16, 28);
    desc = likert(xfos, 6);
    c_put_str(likert_color, desc, 16, 42);

    put_str("Searching   :", 17, 28);
    desc = likert(xsrh, 6);
    c_put_str(likert_color, desc, 17, 42);

    put_str("Disarming   :", 18, 28);
    desc = likert(xdis, 8);
    c_put_str(likert_color, desc, 18, 42);

    put_str("Magic Device:", 19, 28);
    desc = likert(xdev, 6);
    c_put_str(likert_color, desc, 19, 42);


    put_str("Blows/Round:", 16, 55);
    put_str(format("%d", p_ptr->num_blow), 16, 69);
    
    put_str("Shots/Round:", 17, 55);
    put_str(format("%d", p_ptr->num_fire), 17, 69);
    
    put_str("Infra-Vision:", 19, 55);
    put_str(format("%d feet", p_ptr->see_infra * 10), 19, 69);
}



/*
 * Mega-Hack -- stat modifiers
 */
static s16b stat_add[6];
    

/*
 * Display the character on the screen (with optional history)
 *
 * The top two and bottom two lines are left blank.
 */
void display_player(bool do_hist)
{
    int i;

    char	buf[80];


    /* Clear the screen */
    clear_screen();

    /* Name, Sex, Race, Class */
    put_str("Name        :", 2, 1);
    put_str("Sex         :", 3, 1);
    put_str("Race        :", 4, 1);
    put_str("Class       :", 5, 1);

    c_put_str(TERM_L_BLUE, player_name, 2, 15);
    c_put_str(TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
    c_put_str(TERM_L_BLUE, rp_ptr->title, 4, 15);
    c_put_str(TERM_L_BLUE, cp_ptr->title, 5, 15);
    
    /* Age, Height, Weight, Social */
    prt_num("Age          ", (int)p_ptr->age, 2, 32, TERM_L_BLUE);
    prt_num("Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE);
    prt_num("Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE);
    prt_num("Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE);

    /* Display the stats */
    for (i = 0; i < 6; i++) {

        /* Special treatment of "injured" stats */
        if (p_ptr->stat_cur[i] < p_ptr->stat_max[i]) {

            int value;
            
            /* Use lowercase stat name */
            put_str(stat_names_reduced[i], 2 + i, 61);

            /* Get the current stat */
            value = p_ptr->stat_use[i];

            /* Obtain the current stat (modified) */
            cnv_stat(value, buf);

            /* Display the current stat (modified) */
            c_put_str(TERM_YELLOW, buf, 2 + i, 66);

            /* Acquire the max stat */
            value = modify_stat_value(p_ptr->stat_max[i], stat_add[i]);
                                       
            /* Obtain the maximum stat (modified) */
            cnv_stat(value, buf);

            /* Display the maximum stat (modified) */
            c_put_str(TERM_L_GREEN, buf, 2 + i, 73);
        }

        /* Normal treatment of "normal" stats */
        else {

            /* Assume uppercase stat name */
            put_str(stat_names[i], 2 + i, 61);

            /* Obtain the current stat (modified) */
            cnv_stat(p_ptr->stat_use[i], buf);

            /* Display the current stat (modified) */
            c_put_str(TERM_L_GREEN, buf, 2 + i, 66);
        }
    }

    /* Extra info */
    display_player_middle();

    /* Display "history" info */
    if (do_hist) {

        put_str("(Character Background)", 15, 25);

        for (i = 0; i < 4; i++) {
            put_str(history[i], i + 16, 10);
        }
    }
    
    /* Display "various" info */
    else {

        put_str("(Miscellaneous Abilities)", 15, 25);

        display_player_various();
    }
}





/*
 * Track a new monster
 */
void health_track(int m_idx)
{
    /* Track a new guy */
    health_who = m_idx;

    /* Redraw (later) */
    p_ptr->redraw |= (PR_HEALTH);
}



/*
 * Mega-Hack -- monster track
 */
static int last_r_idx = 0;


/*
 * Hack -- track the given monster race
 */
void recent_track(int r_idx)
{
    /* Save this monster ID */
    last_r_idx = r_idx;

    /* Update later */
    p_ptr->redraw |= (PR_RECENT);
}


/*
 * Hack -- describe the current monster race
 */
void recent_fix(void)
{

#ifdef GRAPHIC_RECALL

    /* Hack -- require a special window */
    if (term_recall && use_recall_recent) {
    
        /* Use the recall window */
        Term_activate(term_recall);

        /* Display monster info */
        if (last_r_idx) display_roff(last_r_idx);

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Hack -- require a special window */
    if (term_mirror && use_mirror_recent) {
    
        /* Use the recall window */
        Term_activate(term_mirror);

        /* Display monster info */
        if (last_r_idx) display_roff(last_r_idx);

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

}


/*
 * Hack -- display inventory/equipment in the choice/mirror window
 */
void choose_fix(void)
{

#ifdef GRAPHIC_CHOICE

    /* Show the inven/equip in "term_choice" */
    if (term_choice && use_choice_normal) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Hack -- show inventory */
        if (!choose_default) {

            /* Display the inventory */
            display_inven();
        }

        /* Hack -- show equipment */
        else {

            /* Display the equipment */
            display_equip();
        }

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Show the equip/inven in "term_mirror" */
    if (term_mirror && use_mirror_normal) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Hack -- show inventory */
        if (choose_default) {

            /* Display the inventory */
            display_inven();
        }

        /* Hack -- show equipment */
        else {

            /* Display the equipment */
            display_equip();
        }

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

}


/*
 * Advance experience levels and print experience
 */
void check_experience()
{
    int		i;


    /* Note current level */
    i = p_ptr->lev;


    /* Hack -- lower limit */
    if (p_ptr->exp < 0) p_ptr->exp = 0;

    /* Hack -- lower limit */
    if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

    /* Hack -- upper limit */
    if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

    /* Hack -- upper limit */
    if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;


    /* Hack -- maintain "max" experience */
    if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

    /* Display experience */
    prt_exp();


    /* Lose levels while possible */
    while ((p_ptr->lev > 1) &&
           (p_ptr->exp < (player_exp[p_ptr->lev-2] *
                          p_ptr->expfact / 100L))) {

        /* Lose a level */
        p_ptr->lev--;

        /* Update some stuff */
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

        /* Redraw some stuff */
        p_ptr->redraw |= (PR_LEV | PR_TITLE);

        /* Handle stuff */
        handle_stuff();
    }


    /* Gain levels while possible */
    while ((p_ptr->lev < PY_MAX_LEVEL) &&
           (p_ptr->exp >= (player_exp[p_ptr->lev-1] *
                           p_ptr->expfact / 100L))) {

        /* Gain a level */
        p_ptr->lev++;

        /* Save the highest level */
        if (p_ptr->lev > p_ptr->max_plv) p_ptr->max_plv = p_ptr->lev;

        /* Sound */
        sound(SOUND_LEVEL);

        /* Message */
        msg_format("Welcome to level %d.", p_ptr->lev);

        /* Update some stuff */
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

        /* Redraw some stuff */
        p_ptr->redraw |= (PR_LEV | PR_TITLE);

        /* Handle stuff */
        handle_stuff();
    }
}


/*
 * Gain experience
 */
void gain_exp(s32b amount)
{
    /* Gain some experience */
    p_ptr->exp += amount;

    /* Slowly recover from experience drainage */
    if (p_ptr->exp < p_ptr->max_exp) {

        /* Gain max experience (10%) */
        p_ptr->max_exp += amount / 10;
    }

    /* Check Experience */
    check_experience();
}


/*
 * Lose experience
 */
void lose_exp(s32b amount)
{
    /* Never drop below zero experience */
    if (amount > p_ptr->exp) amount = p_ptr->exp;

    /* Lose some experience */
    p_ptr->exp -= amount;

    /* Check Experience */
    check_experience();
}







/*
 * Returns spell chance of failure for spell		-RAK-	
 */
s16b spell_chance(int spell)
{
    int		chance, minfail;

    magic_type	*s_ptr;


    /* Paranoia -- must be literate */
    if (!mp_ptr->spell_book) return (100);

    /* Access the spell */
    s_ptr = &mp_ptr->info[spell];

    /* Extract the base spell failure rate */
    chance = s_ptr->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (p_ptr->lev - s_ptr->slevel);

    /* Reduce failure rate by INT/WIS adjustment */
    chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[mp_ptr->spell_stat]] - 1);

    /* Not enough mana to cast */
    if (s_ptr->smana > p_ptr->csp) {
        chance += 5 * (s_ptr->smana - p_ptr->csp);
    }

    /* Extract the minimum failure rate */
    minfail = adj_mag_fail[p_ptr->stat_ind[mp_ptr->spell_stat]];

    /* Non mage/priest characters never get too good */
    if ((p_ptr->pclass != 1) && (p_ptr->pclass != 2)) {
        if (minfail < 5) minfail = 5;
    }

    /* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
    if ((p_ptr->pclass == 2) && (p_ptr->icky_wield)) chance += 25;

    /* Minimum failure rate */
    if (chance < minfail) chance = minfail;

    /* Stunning makes spells harder */
    if (p_ptr->stun > 50) chance += 25;
    else if (p_ptr->stun) chance += 15;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    /* Return the chance */
    return (chance);
}




/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
    int			i, j, k, levels;
    int			num_allowed, num_known;

    magic_type		*s_ptr;

    cptr p = ((mp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");


    /* Hack -- must be literate */
    if (!mp_ptr->spell_book) return;

    /* Hack -- wait for creation */
    if (!character_generated) return;
    

    /* Determine the number of spells allowed */
    levels = p_ptr->lev - mp_ptr->spell_first + 1;

    /* Hack -- no negative spells */
    if (levels < 0) levels = 0;

    /* Extract total allowed spells */
    num_allowed = (adj_mag_study[p_ptr->stat_ind[mp_ptr->spell_stat]] *
                   levels / 2);

    /* Assume none known */
    num_known = 0;

    /* Count the number of spells we know */
    for (j = 0; j < 64; j++) {

        /* Count known spells */
        if ((j < 32) ?
            (spell_learned1 & (1L << j)) :
            (spell_learned2 & (1L << (j - 32)))) {
            num_known++;
        }
    }

    /* See how many spells we must forget or may learn */
    p_ptr->new_spells = num_allowed - num_known;



    /* Forget spells which are too hard */
    for (i = 63; i >= 0; i--) {

        /* Efficiency -- all done */
        if (!spell_learned1 && !spell_learned2) break;

        /* Access the spell */
        j = spell_order[i];

        /* Skip non-spells */
        if (j >= 99) continue;

        /* Get the spell */
        s_ptr = &mp_ptr->info[j];

        /* Skip spells we are allowed to know */
        if (s_ptr->slevel <= p_ptr->lev) continue;

        /* Is it known? */
        if ((j < 32) ?
            (spell_learned1 & (1L << j)) :
            (spell_learned2 & (1L << (j - 32)))) {

            /* Mark as forgotten */
            if (j < 32) {
                spell_forgotten1 |= (1L << j);
            }
            else {
                spell_forgotten2 |= (1L << (j - 32));
            }

            /* No longer known */
            if (j < 32) {
                spell_learned1 &= ~(1L << j);
            }
            else {
                spell_learned2 &= ~(1L << (j - 32));
            }

            /* Message */
            msg_format("You have forgotten the %s of %s.", p,
                       spell_names[mp_ptr->spell_type][j]);

            /* One more can be learned */
            p_ptr->new_spells++;
        }
    }


    /* Forget spells if we know too many spells */
    for (i = 63; i >= 0; i--) {

        /* Stop when possible */
        if (p_ptr->new_spells >= 0) break;

        /* Efficiency -- all done */
        if (!spell_learned1 && !spell_learned2) break;

        /* Get the (i+1)th spell learned */
        j = spell_order[i];

        /* Skip unknown spells */
        if (j >= 99) continue;

        /* Forget it (if learned) */
        if ((j < 32) ?
            (spell_learned1 & (1L << j)) :
            (spell_learned2 & (1L << (j - 32)))) {

            /* Mark as forgotten */
            if (j < 32) {
                spell_forgotten1 |= (1L << j);
            }
            else {
                spell_forgotten2 |= (1L << (j - 32));
            }

            /* No longer known */
            if (j < 32) {
                spell_learned1 &= ~(1L << j);
            }
            else {
                spell_learned2 &= ~(1L << (j - 32));
            }

            /* Message */
            msg_format("You have forgotten the %s of %s.", p,
                       spell_names[mp_ptr->spell_type][j]);

            /* One more can be learned */
            p_ptr->new_spells++;
        }
    }


    /* Check for spells to remember */
    for (i = 0; i < 64; i++) {

        /* None left to remember */
        if (p_ptr->new_spells <= 0) break;

        /* Efficiency -- all done */
        if (!spell_forgotten1 && !spell_forgotten2) break;

        /* Get the next spell we learned */
        j = spell_order[i];

        /* Skip unknown spells */
        if (j >= 99) break;

        /* Access the spell */
        s_ptr = &mp_ptr->info[j];

        /* Skip spells we cannot remember */
        if (s_ptr->slevel > p_ptr->lev) continue;

        /* First set of spells */
        if ((j < 32) ?
            (spell_forgotten1 & (1L << j)) :
            (spell_forgotten2 & (1L << (j - 32)))) {

            /* No longer forgotten */
            if (j < 32) {
                spell_forgotten1 &= ~(1L << j);
            }
            else {
                spell_forgotten2 &= ~(1L << (j - 32));
            }

            /* Known once more */
            if (j < 32) {
                spell_learned1 |= (1L << j);
            }
            else {
                 spell_learned2 |= (1L << (j - 32));
            }

            /* Message */
            msg_format("You have remembered the %s of %s.",
                       p, spell_names[mp_ptr->spell_type][j]);

            /* One less can be learned */
            p_ptr->new_spells--;
        }
    }


    /* Assume no spells available */
    k = 0;

    /* Count spells that can be learned */
    for (j = 0; j < 64; j++) {

        /* Access the spell */
        s_ptr = &mp_ptr->info[j];

        /* Skip spells we cannot remember */
        if (s_ptr->slevel > p_ptr->lev) continue;

        /* Skip spells we already know */
        if ((j < 32) ?
            (spell_learned1 & (1L << j)) :
            (spell_learned2 & (1L << (j - 32)))) {

            continue;
        }

        /* Count it */
        k++;
    }

    /* Cannot learn more spells than exist */
    if (p_ptr->new_spells > k) p_ptr->new_spells = k;

    /* Learn new spells */
    if (p_ptr->new_spells && !p_ptr->old_spells) {

        /* Message */
        msg_format("You can learn some new %ss now.", p);

        /* Display "study state" later */
        p_ptr->redraw |= (PR_STUDY);
    }
    
    /* No more spells */
    else if (!p_ptr->new_spells && p_ptr->old_spells) {

        /* Display "study state" later */
        p_ptr->redraw |= (PR_STUDY);
    }
    
    /* Save the new_spells value */
    p_ptr->old_spells = p_ptr->new_spells;
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
    int		new_mana, levels, cur_wgt, max_wgt;

    inven_type	*i_ptr;


    /* Hack -- Must be literate */
    if (!mp_ptr->spell_book) return;


    /* Extract "effective" player level */
    levels = (p_ptr->lev - mp_ptr->spell_first) + 1;

    /* Hack -- no negative mana */
    if (levels < 0) levels = 0;

    /* Extract total mana */
    new_mana = adj_mag_mana[p_ptr->stat_ind[mp_ptr->spell_stat]] * levels / 2;

    /* Hack -- usually add one mana */
    if (new_mana) new_mana++;


    /* Only mages are affected */
    if (mp_ptr->spell_book == TV_MAGIC_BOOK) {

        u32b f1, f2, f3;

        /* Assume player is not encumbered by gloves */
        p_ptr->cumber_glove = FALSE;

        /* Get the gloves */
        i_ptr = &inventory[INVEN_HANDS];

        /* Examine the gloves */
        inven_flags(i_ptr, &f1, &f2, &f3);

        /* Normal gloves hurt mage-type spells */
        if (i_ptr->k_idx &&
            !(f2 & TR2_FREE_ACT) &&
            !((f1 & TR1_DEX) && (i_ptr->pval > 0))) {

            /* Encumbered */
            p_ptr->cumber_glove = TRUE;

            /* Reduce mana */
            new_mana = (3 * new_mana) / 4;
        }
    }


    /* Assume player not encumbered by armor */
    p_ptr->cumber_armor = FALSE;

    /* Weigh the armor */
    cur_wgt = 0;
    cur_wgt += inventory[INVEN_BODY].weight;
    cur_wgt += inventory[INVEN_HEAD].weight;
    cur_wgt += inventory[INVEN_ARM].weight;
    cur_wgt += inventory[INVEN_OUTER].weight;
    cur_wgt += inventory[INVEN_HANDS].weight;
    cur_wgt += inventory[INVEN_FEET].weight;

    /* Determine the weight allowance */
    max_wgt = mp_ptr->spell_weight;

    /* Heavy armor penalizes mana */
    if (((cur_wgt - max_wgt) / 10) > 0) {

        /* Encumbered */
        p_ptr->cumber_armor = TRUE;

        /* Reduce mana */
        new_mana -= ((cur_wgt - max_wgt) / 10);
    }


    /* Mana can never be negative */
    if (new_mana < 0) new_mana = 0;


    /* Maximum mana has changed */
    if (p_ptr->msp != new_mana) {

        /* Player has no mana now */
        if (!new_mana) {

            /* No mana left */
            p_ptr->csp = 0;
            p_ptr->csp_frac = 0;
        }

        /* Player had no mana, has some now */
        else if (!p_ptr->msp) {

            /* Reset mana */
            p_ptr->csp = new_mana;
            p_ptr->csp_frac = 0;
        }

        /* Player had some mana, adjust current mana */
        else {

            s32b value;

            /* change current mana proportionately to change of max mana, */
            /* divide first to avoid overflow, little loss of accuracy */
            value = ((((long)p_ptr->csp << 16) + p_ptr->csp_frac) /
                     p_ptr->msp * new_mana);

            /* Extract mana components */
            p_ptr->csp = (value >> 16);
            p_ptr->csp_frac = (value & 0xFFFF);
        }

        /* Save new mana */
        p_ptr->msp = new_mana;

        /* Display mana later */
        p_ptr->redraw |= (PR_MANA);
    }


    /* Take note when "glove state" changes */
    if (p_ptr->old_cumber_glove != p_ptr->cumber_glove) {
    
        /* Message */
        if (p_ptr->cumber_glove) {
            msg_print("Your covered hands feel unsuitable for spellcasting.");
        }
        else {
            msg_print("Your hands feel more suitable for spellcasting.");
        }

        /* Save it */
        p_ptr->old_cumber_glove = p_ptr->cumber_glove;
    }


    /* Take note when "armor state" changes */
    if (p_ptr->old_cumber_armor != p_ptr->cumber_armor) {
    
        /* Message */
        if (p_ptr->cumber_armor) {
            msg_print("The weight of your armor encumbers your movement.");
        }
        else {
            msg_print("You feel able to move more freely.");
        }

        /* Save it */
        p_ptr->old_cumber_armor = p_ptr->cumber_armor;
    }        
}



/*
 * Calculate the players (maximal) hit points
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints()
{
    int bonus, mhp;

    /* Un-inflate "half-hitpoint bonus per level" value */
    bonus = ((int)(adj_con_mhp[p_ptr->stat_ind[A_CON]]) - 128);

    /* Calculate hitpoints */
    mhp = player_hp[p_ptr->lev-1] + (bonus * p_ptr->lev / 2);

    /* Always have at least one hitpoint per level */
    if (mhp < p_ptr->lev + 1) mhp = p_ptr->lev + 1;

    /* Factor in the hero / superhero settings */
    if (p_ptr->hero) mhp += 10;
    if (p_ptr->shero) mhp += 30;

    /* New maximum hitpoints */
    if (mhp != p_ptr->mhp) {

        s32b value;

        /* change current hit points proportionately to change of mhp */
        /* divide first to avoid overflow, little loss of accuracy */
        value = (((long)p_ptr->chp << 16) + p_ptr->chp_frac) / p_ptr->mhp;
        value = value * mhp;
        p_ptr->chp = (value >> 16);
        p_ptr->chp_frac = (value & 0xFFFF);

        /* Save the new max-hitpoints */
        p_ptr->mhp = mhp;

        /* Display hitpoints (later) */
        p_ptr->redraw |= (PR_HP);
    }
}



/*
 * Computes current weight limit.
 */
static int weight_limit(void)
{
    int i;

    /* Weight limit based only on strength */
    i = adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100;

    /* Return the result */
    return (i);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * This function induces various "status" messages.
 */
static void calc_bonuses(void)
{
    int			i, j, hold;

    int			old_speed;

    int			old_telepathy;
    int			old_see_inv;
    
    int			old_dis_ac;
    int			old_dis_to_a;

    int			extra_blows;
    int			extra_shots;

    inven_type		*i_ptr;

    u32b		f1, f2, f3;


    /* Save the old speed */
    old_speed = p_ptr->pspeed;

    /* Save the old vision stuff */
    old_telepathy = p_ptr->telepathy;
    old_see_inv = p_ptr->see_inv;

    /* Save the old armor class */
    old_dis_ac = p_ptr->dis_ac;
    old_dis_to_a = p_ptr->dis_to_a;


    /* Clear extra blows/shots */
    extra_blows = extra_shots = 0;

    /* Clear the stat modifiers */
    for (i = 0; i < 6; i++) stat_add[i] = 0;


    /* Clear the Displayed/Real armor class */
    p_ptr->dis_ac = p_ptr->ac = 0;

    /* Clear the Displayed/Real Bonuses */
    p_ptr->dis_to_h = p_ptr->to_h = 0;
    p_ptr->dis_to_d = p_ptr->to_d = 0;
    p_ptr->dis_to_a = p_ptr->to_a = 0;


    /* Clear all the flags */
    p_ptr->aggravate = FALSE;
    p_ptr->teleport = FALSE;
    p_ptr->exp_drain = FALSE;
    p_ptr->bless_blade = FALSE;
    p_ptr->xtra_might = FALSE;
    p_ptr->see_inv = FALSE;
    p_ptr->free_act = FALSE;
    p_ptr->slow_digest = FALSE;
    p_ptr->regenerate = FALSE;
    p_ptr->ffall = FALSE;
    p_ptr->hold_life = FALSE;
    p_ptr->telepathy = FALSE;
    p_ptr->lite = FALSE;
    p_ptr->sustain_str = FALSE;
    p_ptr->sustain_int = FALSE;
    p_ptr->sustain_wis = FALSE;
    p_ptr->sustain_con = FALSE;
    p_ptr->sustain_dex = FALSE;
    p_ptr->sustain_chr = FALSE;
    p_ptr->resist_acid = FALSE;
    p_ptr->resist_elec = FALSE;
    p_ptr->resist_fire = FALSE;
    p_ptr->resist_cold = FALSE;
    p_ptr->resist_pois = FALSE;
    p_ptr->resist_conf = FALSE;
    p_ptr->resist_sound = FALSE;
    p_ptr->resist_lite = FALSE;
    p_ptr->resist_dark = FALSE;
    p_ptr->resist_chaos = FALSE;
    p_ptr->resist_disen = FALSE;
    p_ptr->resist_shard = FALSE;
    p_ptr->resist_nexus = FALSE;
    p_ptr->resist_blind = FALSE;
    p_ptr->resist_neth = FALSE;
    p_ptr->resist_fear = FALSE;
    p_ptr->immune_acid = FALSE;
    p_ptr->immune_elec = FALSE;
    p_ptr->immune_fire = FALSE;
    p_ptr->immune_cold = FALSE;



    /* Base infravision (purely racial) */
    p_ptr->see_infra = rp_ptr->infra;


    /* Base skill -- disarming */
    p_ptr->skill_dis = rp_ptr->r_dis + cp_ptr->c_dis;

    /* Base skill -- magic devices */
    p_ptr->skill_dev = rp_ptr->r_dev + cp_ptr->c_dev;

    /* Base skill -- saving throw */
    p_ptr->skill_sav = rp_ptr->r_sav + cp_ptr->c_sav;

    /* Base skill -- stealth */
    p_ptr->skill_stl = rp_ptr->r_stl + cp_ptr->c_stl;

    /* Base skill -- searching ability */
    p_ptr->skill_srh = rp_ptr->r_srh + cp_ptr->c_srh;

    /* Base skill -- searching frequency */
    p_ptr->skill_fos = rp_ptr->r_fos + cp_ptr->c_fos;

    /* Base skill -- combat (normal) */
    p_ptr->skill_thn = rp_ptr->r_thn + cp_ptr->c_thn;

    /* Base skill -- combat (shooting) */
    p_ptr->skill_thb = rp_ptr->r_thb + cp_ptr->c_thb;

    /* Base skill -- combat (throwing) */
    p_ptr->skill_tht = rp_ptr->r_thb + cp_ptr->c_thb;

    /* Base skill -- digging */
    p_ptr->skill_dig = 0;


    /* Elf */
    if (p_ptr->prace == RACE_ELF) p_ptr->resist_lite = TRUE;

    /* Hobbit */
    if (p_ptr->prace == RACE_HOBBIT) p_ptr->sustain_dex = TRUE;

    /* Gnome */
    if (p_ptr->prace == RACE_GNOME) p_ptr->free_act = TRUE;

    /* Dwarf */
    if (p_ptr->prace == RACE_DWARF) p_ptr->resist_blind = TRUE;

    /* Half-Orc */
    if (p_ptr->prace == RACE_HALF_ORC) p_ptr->resist_dark = TRUE;

    /* Half-Troll */
    if (p_ptr->prace == RACE_HALF_TROLL) p_ptr->sustain_str = TRUE;

    /* Dunadan */
    if (p_ptr->prace == RACE_DUNADAN) p_ptr->sustain_con = TRUE;

    /* High Elf */
    if (p_ptr->prace == RACE_HIGH_ELF) p_ptr->resist_lite = TRUE;
    if (p_ptr->prace == RACE_HIGH_ELF) p_ptr->see_inv = TRUE;


    /* Start with "normal" speed */
    p_ptr->pspeed = 110;

    /* Start with a single blow per turn */
    p_ptr->num_blow = 1;

    /* Start with a single shot per turn */
    p_ptr->num_fire = 1;

    /* Reset the "xtra" tval */
    p_ptr->tval_xtra = 0;

    /* Reset the "ammo" tval */
    p_ptr->tval_ammo = 0;


    /* Hack -- apply racial/class stat maxes */
    if (p_ptr->maximize) {

        /* Apply the racial modifiers */
        for (i = 0; i < 6; i++) {

            /* Modify the stats for "race" */
            stat_add[i] += (rp_ptr->r_adj[i] + cp_ptr->c_adj[i]);
        }
    }


    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        i_ptr = &inventory[i];

        /* Skip missing items */
        if (!i_ptr->k_idx) continue;

        /* Extract the item flags */
        inven_flags(i_ptr, &f1, &f2, &f3);
        
        /* Affect stats */
        if (f1 & TR1_STR) stat_add[A_STR] += i_ptr->pval;
        if (f1 & TR1_INT) stat_add[A_INT] += i_ptr->pval;
        if (f1 & TR1_WIS) stat_add[A_WIS] += i_ptr->pval;
        if (f1 & TR1_DEX) stat_add[A_DEX] += i_ptr->pval;
        if (f1 & TR1_CON) stat_add[A_CON] += i_ptr->pval;
        if (f1 & TR1_CHR) stat_add[A_CHR] += i_ptr->pval;

        /* Affect stealth */
        if (f1 & TR1_STEALTH) p_ptr->skill_stl += i_ptr->pval;

        /* Affect searching ability (factor of five) */
        if (f1 & TR1_SEARCH) p_ptr->skill_srh += (i_ptr->pval * 5);

        /* Affect searching frequency (factor of five) */
        if (f1 & TR1_SEARCH) p_ptr->skill_fos += (i_ptr->pval * 5);

        /* Affect infravision */
        if (f1 & TR1_INFRA) p_ptr->see_infra += i_ptr->pval;

        /* Affect digging (factor of 20) */
        if (f1 & TR1_TUNNEL) p_ptr->skill_dig += (i_ptr->pval * 20);

        /* Affect speed */
        if (f1 & TR1_SPEED) p_ptr->pspeed += i_ptr->pval;

        /* Affect blows */
        if (f1 & TR1_BLOWS) extra_blows += i_ptr->pval;

        /* Boost shots */
        if (f3 & TR3_XTRA_SHOTS) extra_shots++;

        /* Various flags */
        if (f3 & TR3_AGGRAVATE) p_ptr->aggravate = TRUE;
        if (f3 & TR3_TELEPORT) p_ptr->teleport = TRUE;
        if (f3 & TR3_DRAIN_EXP) p_ptr->exp_drain = TRUE;
        if (f3 & TR3_BLESSED) p_ptr->bless_blade = TRUE;
        if (f3 & TR3_XTRA_MIGHT) p_ptr->xtra_might = TRUE;
        if (f3 & TR3_SLOW_DIGEST) p_ptr->slow_digest = TRUE;
        if (f3 & TR3_REGEN) p_ptr->regenerate = TRUE;
        if (f3 & TR3_TELEPATHY) p_ptr->telepathy = TRUE;
        if (f3 & TR3_LITE) p_ptr->lite = TRUE;
        if (f3 & TR3_SEE_INVIS) p_ptr->see_inv = TRUE;
        if (f3 & TR3_FEATHER) p_ptr->ffall = TRUE;
        if (f2 & TR2_FREE_ACT) p_ptr->free_act = TRUE;
        if (f2 & TR2_HOLD_LIFE) p_ptr->hold_life = TRUE;
        
        /* Immunity flags */
        if (f2 & TR2_IM_FIRE) p_ptr->immune_fire = TRUE;
        if (f2 & TR2_IM_ACID) p_ptr->immune_acid = TRUE;
        if (f2 & TR2_IM_COLD) p_ptr->immune_cold = TRUE;
        if (f2 & TR2_IM_ELEC) p_ptr->immune_elec = TRUE;

        /* Resistance flags */
        if (f2 & TR2_RES_ACID) p_ptr->resist_acid = TRUE;
        if (f2 & TR2_RES_ELEC) p_ptr->resist_elec = TRUE;
        if (f2 & TR2_RES_FIRE) p_ptr->resist_fire = TRUE;
        if (f2 & TR2_RES_COLD) p_ptr->resist_cold = TRUE;
        if (f2 & TR2_RES_POIS) p_ptr->resist_pois = TRUE;
        if (f2 & TR2_RES_CONF) p_ptr->resist_conf = TRUE;
        if (f2 & TR2_RES_SOUND) p_ptr->resist_sound = TRUE;
        if (f2 & TR2_RES_LITE) p_ptr->resist_lite = TRUE;
        if (f2 & TR2_RES_DARK) p_ptr->resist_dark = TRUE;
        if (f2 & TR2_RES_CHAOS) p_ptr->resist_chaos = TRUE;
        if (f2 & TR2_RES_DISEN) p_ptr->resist_disen = TRUE;
        if (f2 & TR2_RES_SHARDS) p_ptr->resist_shard = TRUE;
        if (f2 & TR2_RES_NEXUS) p_ptr->resist_nexus = TRUE;
        if (f2 & TR2_RES_BLIND) p_ptr->resist_blind = TRUE;
        if (f2 & TR2_RES_NETHER) p_ptr->resist_neth = TRUE;

        /* Sustain flags */
        if (f2 & TR2_SUST_STR) p_ptr->sustain_str = TRUE;
        if (f2 & TR2_SUST_INT) p_ptr->sustain_int = TRUE;
        if (f2 & TR2_SUST_WIS) p_ptr->sustain_wis = TRUE;
        if (f2 & TR2_SUST_DEX) p_ptr->sustain_dex = TRUE;
        if (f2 & TR2_SUST_CON) p_ptr->sustain_con = TRUE;
        if (f2 & TR2_SUST_CHR) p_ptr->sustain_chr = TRUE;

        /* Modify the base armor class */
        p_ptr->ac += i_ptr->ac;

        /* The base armor class is always known */
        p_ptr->dis_ac += i_ptr->ac;

        /* Apply the bonuses to armor class */
        p_ptr->to_a += i_ptr->to_a;

        /* Apply the mental bonuses to armor class, if known */
        if (inven_known_p(i_ptr)) p_ptr->dis_to_a += i_ptr->to_a;

        /* Hack -- do not apply "weapon" bonuses */
        if (i == INVEN_WIELD) continue;

        /* Hack -- do not apply "bow" bonuses */
        if (i == INVEN_BOW) continue;

        /* Apply the bonuses to hit/damage */
        p_ptr->to_h += i_ptr->to_h;
        p_ptr->to_d += i_ptr->to_d;

        /* Apply the mental bonuses tp hit/damage, if known */
        if (inven_known_p(i_ptr)) p_ptr->dis_to_h += i_ptr->to_h;
        if (inven_known_p(i_ptr)) p_ptr->dis_to_d += i_ptr->to_d;
    }


    /* Calculate the "stat_use" values */
    for (i = 0; i < 6; i++) {

        int use, ind;

        /* Extract the new "stat_use" value for the stat */
        use = modify_stat_value(p_ptr->stat_cur[i], stat_add[i]);

        /* Ignore non-changes */
        if (p_ptr->stat_use[i] == use) continue;

        /* Save the new value */
        p_ptr->stat_use[i] = use;

        /* Redisplay the stats later */
        p_ptr->redraw |= (PR_STATS);

        /* Values: 3, 4, ..., 17 */
        if (use <= 18) ind = (use - 3);

        /* Ranges: 18/00-18/09, ..., 18/210-18/219 */
        else if (use <= 18+219) ind = (15 + (use - 18) / 10);

        /* Range: 18/220+ */
        else ind = (37);

        /* Ignore non-changes */
        if (p_ptr->stat_ind[i] == ind) continue;
        
        /* Save the new index */
        p_ptr->stat_ind[i] = ind;

        /* Change in CON affects Hitpoints */
        if (i == A_CON) {
            p_ptr->update |= (PU_HP);
        }

        /* Change in INT may affect Mana/Spells */
        else if (i == A_INT) {
            if (mp_ptr->spell_stat == A_INT) {
                p_ptr->update |= (PU_MANA | PU_SPELLS);
            }
        }

        /* Change in WIS may affect Mana/Spells */
        else if (i == A_WIS) {
            if (mp_ptr->spell_stat == A_WIS) {
                p_ptr->update |= (PU_MANA | PU_SPELLS);
            }
        }
    }


    /* Apply temporary "stun" */
    if (p_ptr->stun > 50) {
        p_ptr->to_h -= 20;
        p_ptr->dis_to_h -= 20;
        p_ptr->to_d -= 20;
        p_ptr->dis_to_d -= 20;
    }
    else if (p_ptr->stun) {
        p_ptr->to_h -= 5;
        p_ptr->dis_to_h -= 5;
        p_ptr->to_d -= 5;
        p_ptr->dis_to_d -= 5;
    }


    /* Invulnerability */
    if (p_ptr->invuln) {
        p_ptr->to_a += 100;
        p_ptr->dis_to_a += 100;
    }

    /* Temporary blessing */
    if (p_ptr->blessed) {
        p_ptr->to_a += 5;
        p_ptr->dis_to_a += 5;
        p_ptr->to_h += 10;
        p_ptr->dis_to_h += 10;
    }

    /* Temprory shield */
    if (p_ptr->shield) {
        p_ptr->to_a += 50;
        p_ptr->dis_to_a += 50;
    }

    /* Temporary "Hero" */
    if (p_ptr->hero) {
        p_ptr->to_h += 12;
        p_ptr->dis_to_h += 12;
    }

    /* Temporary "Beserk" */
    if (p_ptr->shero) {
        p_ptr->to_h += 24;
        p_ptr->dis_to_h += 24;
        p_ptr->to_a -= 10;
        p_ptr->dis_to_a -= 10;
    }

    /* Temporary "fast" */
    if (p_ptr->fast) {
        p_ptr->pspeed += 10;
    }

    /* Temporary "slow" */
    if (p_ptr->slow) {
        p_ptr->pspeed -= 10;
    }

    /* Temporary see invisible */
    if (p_ptr->tim_invis) {
        p_ptr->see_inv = TRUE;
    }

    /* Temporary infravision boost */
    if (p_ptr->tim_infra) {
        p_ptr->see_infra++;
    }


    /* Hack -- Res Chaos -> Res Conf */
    if (p_ptr->resist_chaos) {
        p_ptr->resist_conf = TRUE;
    }

    /* Hack -- Hero/Shero -> Res fear */
    if (p_ptr->hero || p_ptr->shero) {
        p_ptr->resist_fear = TRUE;
    }


    /* Hack -- Telepathy Change */
    if (p_ptr->telepathy != old_telepathy) {
        p_ptr->update |= (PU_MONSTERS);
    }

    /* Hack -- See Invis Change */
    if (p_ptr->see_inv != old_see_inv) {
        p_ptr->update |= (PU_MONSTERS);
    }


    /* Extract the current weight (in tenth pounds) */
    j = total_weight;

    /* Extract the "weight limit" (in tenth pounds) */
    i = weight_limit();

    /* XXX XXX XXX Apply "encumbrance" from weight */
    if (j > i/2) p_ptr->pspeed -= ((j - (i/2)) / (i / 10));

    /* Bloating slows the player down (a little) */
    if (p_ptr->food >= PY_FOOD_MAX) p_ptr->pspeed -= 10;

    /* Searching slows the player down */
    if (p_ptr->searching) p_ptr->pspeed -= 10;

    /* Display the speed (if needed) */
    if (p_ptr->pspeed != old_speed) p_ptr->redraw |= (PR_SPEED);


    /* Actual Modifier Bonuses (Un-inflate stat bonuses) */
    p_ptr->to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
    p_ptr->to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
    p_ptr->to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
    p_ptr->to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);

    /* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
    p_ptr->dis_to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
    p_ptr->dis_to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
    p_ptr->dis_to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
    p_ptr->dis_to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);


    /* Redraw armor (if needed) */
    if (p_ptr->dis_ac != old_dis_ac) p_ptr->redraw |= (PR_ARMOR);
    if (p_ptr->dis_to_a != old_dis_to_a) p_ptr->redraw |= (PR_ARMOR);



    /* Obtain the "hold" value */
    hold = adj_str_hold[p_ptr->stat_ind[A_STR]];


    /* Examine the "current bow" */
    i_ptr = &inventory[INVEN_BOW];


    /* Assume not heavy */
    p_ptr->heavy_shoot = FALSE;

    /* It is hard to carholdry a heavy bow */
    if (hold < i_ptr->weight / 10) {

        /* Hard to wield a heavy bow */
        p_ptr->to_h += 2 * (hold - i_ptr->weight / 10);
        p_ptr->dis_to_h += 2 * (hold - i_ptr->weight / 10);

        /* Heavy Bow */
        p_ptr->heavy_shoot = TRUE;
    }


    /* Compute "extra shots" if needed */
    if (i_ptr->k_idx && !p_ptr->heavy_shoot) {

        /* Take note of required "tval" for missiles */
        switch (i_ptr->sval) {

            case SV_SLING:
                p_ptr->tval_ammo = TV_SHOT;
                break;

            case SV_SHORT_BOW:
            case SV_LONG_BOW:
                p_ptr->tval_ammo = TV_ARROW;
                break;

            case SV_LIGHT_XBOW:
            case SV_HEAVY_XBOW:
                p_ptr->tval_ammo = TV_BOLT;
                break;
        }

        /* Hack -- Reward High Level Rangers using Bows */
        if ((p_ptr->pclass == 4) && (p_ptr->tval_ammo == TV_ARROW)) {

            /* Extra shot at level 20 */
            if (p_ptr->lev >= 20) p_ptr->num_fire++;

            /* Extra shot at level 40 */
            if (p_ptr->lev >= 40) p_ptr->num_fire++;
        }

        /* Add in the "bonus shots" */
        p_ptr->num_fire += extra_shots;

        /* Require at least one shot */
        if (p_ptr->num_fire < 1) p_ptr->num_fire = 1;
    }



    /* Examine the "main weapon" */
    i_ptr = &inventory[INVEN_WIELD];


    /* Assume not heavy */
    p_ptr->heavy_wield = FALSE;
    
    /* It is hard to hold a heavy weapon */
    if (hold < i_ptr->weight / 10) {

        /* Hard to wield a heavy weapon */
        p_ptr->to_h += 2 * (hold - i_ptr->weight / 10);
        p_ptr->dis_to_h += 2 * (hold - i_ptr->weight / 10);

        /* Heavy weapon */
        p_ptr->heavy_wield = TRUE;
    }


    /* Normal weapons */
    if (i_ptr->k_idx && !p_ptr->heavy_wield) {

        int str_index, dex_index;

        int num = 0, wgt = 0, mul = 0, div = 0;

        /* Analyze the class */
        switch (p_ptr->pclass) {

            /* Warrior */
            case CLASS_WARRIOR: num = 6; wgt = 30; mul = 5; break;

            /* Mage */
            case CLASS_MAGE:    num = 4; wgt = 40; mul = 2; break;

            /* Priest */
            case CLASS_PRIEST:  num = 5; wgt = 35; mul = 3; break;

            /* Rogue */
            case CLASS_ROGUE:   num = 5; wgt = 30; mul = 3; break;

            /* Ranger */
            case CLASS_RANGER:  num = 5; wgt = 35; mul = 4; break;

            /* Paladin */
            case CLASS_PALADIN: num = 5; wgt = 30; mul = 4; break;
        }

        /* Enforce a minimum "weight" (tenth pounds) */
        div = ((i_ptr->weight < wgt) ? wgt : i_ptr->weight);

        /* Access the strength vs weight */
        str_index = (adj_str_blow[p_ptr->stat_ind[A_STR]] * mul / div);

        /* Maximal value */
        if (str_index > 11) str_index = 11;

        /* Index by dexterity */
        dex_index = (adj_dex_blow[p_ptr->stat_ind[A_DEX]]);

        /* Maximal value */
        if (dex_index > 11) dex_index = 11;

        /* Use the blows table */
        p_ptr->num_blow = blows_table[str_index][dex_index];

        /* Maximal value */
        if (p_ptr->num_blow > num) p_ptr->num_blow = num;

        /* Add in the "bonus blows" */
        p_ptr->num_blow += extra_blows;

        /* Require at least one blow */
        if (p_ptr->num_blow < 1) p_ptr->num_blow = 1;

        /* Boost digging skill by weapon weight */
        p_ptr->skill_dig += (i_ptr->weight / 10);
    }


    /* Assume okay */
    p_ptr->icky_wield = FALSE;
    
    /* Priest weapon penalty for non-blessed edged weapons */
    if ((p_ptr->pclass == 2) && (!p_ptr->bless_blade) &&
        ((i_ptr->tval == TV_SWORD) || (i_ptr->tval == TV_POLEARM))) {

        /* Reduce the real bonuses */
        p_ptr->to_h -= 2;
        p_ptr->to_d -= 2;

        /* Reduce the mental bonuses */
        p_ptr->dis_to_h -= 2;
        p_ptr->dis_to_d -= 2;

        /* Icky weapon */
        p_ptr->icky_wield = TRUE;
    }


    /* Affect Skill -- stealth (bonus one) */
    p_ptr->skill_stl += 1;

    /* Affect Skill -- disarming (DEX and INT) */
    p_ptr->skill_dis += adj_dex_dis[p_ptr->stat_ind[A_DEX]];
    p_ptr->skill_dis += adj_int_dis[p_ptr->stat_ind[A_INT]];

    /* Affect Skill -- magic devices (INT) */
    p_ptr->skill_dev += adj_int_dev[p_ptr->stat_ind[A_INT]];

    /* Affect Skill -- saving throw (WIS) */
    p_ptr->skill_sav += adj_wis_sav[p_ptr->stat_ind[A_WIS]];

    /* Affect Skill -- digging (STR) */
    p_ptr->skill_dig += adj_str_dig[p_ptr->stat_ind[A_STR]];


    /* Affect Skill -- disarming (Level, by Class) */
    p_ptr->skill_dis += (cp_ptr->x_dis * p_ptr->lev / 10);

    /* Affect Skill -- magic devices (Level, by Class) */
    p_ptr->skill_dev += (cp_ptr->x_dev * p_ptr->lev / 10);

    /* Affect Skill -- saving throw (Level, by Class) */
    p_ptr->skill_sav += (cp_ptr->x_sav * p_ptr->lev / 10);

    /* Affect Skill -- stealth (Level, by Class) */
    p_ptr->skill_stl += (cp_ptr->x_stl * p_ptr->lev / 10);

    /* Affect Skill -- search ability (Level, by Class) */
    p_ptr->skill_srh += (cp_ptr->x_srh * p_ptr->lev / 10);

    /* Affect Skill -- search frequency (Level, by Class) */
    p_ptr->skill_fos += (cp_ptr->x_fos * p_ptr->lev / 10);

    /* Affect Skill -- combat (normal) (Level, by Class) */
    p_ptr->skill_thn += (cp_ptr->x_thn * p_ptr->lev / 10);

    /* Affect Skill -- combat (shooting) (Level, by Class) */
    p_ptr->skill_thb += (cp_ptr->x_thb * p_ptr->lev / 10);

    /* Affect Skill -- combat (throwing) (Level, by Class) */
    p_ptr->skill_tht += (cp_ptr->x_thb * p_ptr->lev / 10);


    /* Limit Skill -- stealth from 0 to 30 */
    if (p_ptr->skill_stl > 30) p_ptr->skill_stl = 30;
    if (p_ptr->skill_stl < 0) p_ptr->skill_stl = 0;

    /* Limit Skill -- digging from 1 up */
    if (p_ptr->skill_dig < 1) p_ptr->skill_dig = 1;


    /* Take note when "heavy bow" changes */
    if (p_ptr->old_heavy_shoot != p_ptr->heavy_shoot) {

        /* Message */
        if (p_ptr->heavy_shoot) {
            msg_print("You have trouble wielding such a heavy bow.");
        }
        else if (inventory[INVEN_BOW].k_idx) {
            msg_print("You have no trouble wielding your bow.");
        }
        else {
            msg_print("You feel relieved to put down your heavy bow.");
        }
        
        /* Save it */
        p_ptr->old_heavy_shoot = p_ptr->heavy_shoot;
    }


    /* Take note when "heavy weapon" changes */
    if (p_ptr->old_heavy_wield != p_ptr->heavy_wield) {

        /* Message */
        if (p_ptr->heavy_wield) {
            msg_print("You have trouble wielding such a heavy weapon.");
        }
        else if (inventory[INVEN_WIELD].k_idx) {
            msg_print("You have no trouble wielding your weapon.");
        }
        else {
            msg_print("You feel relieved to put down your heavy weapon.");
        }
        
        /* Save it */
        p_ptr->old_heavy_wield = p_ptr->heavy_wield;
    }


    /* Take note when "illegal weapon" changes */
    if (p_ptr->old_icky_wield != p_ptr->icky_wield) {

        /* Message */
        if (p_ptr->icky_wield) {
            msg_print("You do not feel comfortable with your weapon.");
        }
        else if (inventory[INVEN_WIELD].k_idx) {
            msg_print("You feel comfortable with your weapon.");
        }
        else {
            msg_print("You feel more comfortable after removing your weapon.");
        }
        
        /* Save it */
        p_ptr->old_icky_wield = p_ptr->icky_wield;
    }
}



/*
 * Combine items in the pack (if legal)
 *
 * Note special handling of the "overflow" slot
 */
static void combine_pack(void)
{
    int i, j;

    inven_type *i_ptr, *j_ptr;

    bool	flag = FALSE;
    
    
    /* Combine the pack (backwards) */
    for (i = INVEN_PACK; i > 0; i--) {

        /* Get the item */
        i_ptr = &inventory[i];

        /* Skip empty items */
        if (!i_ptr->k_idx) continue;

        /* Scan the items above that item */
        for (j = 0; j < i; j++) {

            /* Get the item */
            j_ptr = &inventory[j];

            /* Skip empty items */
            if (!j_ptr->k_idx) continue;
            
            /* Can we drop "i_ptr" onto "j_ptr"? */
            if (item_similar(j_ptr, i_ptr)) {

		/* Take note */
		flag = TRUE;

                /* Add together the item counts */
                item_absorb(j_ptr, i_ptr);

                /* One object is gone */
                inven_cnt--;
                
                /* Erase the last object */
                invwipe(&inventory[i]);

                /* Redraw the choice window */
                p_ptr->redraw |= (PR_CHOOSE);

                /* XXX XXX XXX Reorder the pack */
                p_ptr->update |= (PU_REORDER);

		/* Done */
		break;
            }
        }
    }

    /* Message */
    if (flag) msg_print("You combine some items in your pack.");
}


/*
 * Reorder items in the pack (if legal)
 *
 * Note special handling of the "overflow" slot
 *
 * Note special handling of empty slots
 */
static void reorder_pack(void)
{
    int		i, j, k;

    s32b	i_value, j_value;

    inven_type	*i_ptr, *j_ptr;

    inven_type	temp;

    bool	flag = FALSE;
    

    /* Re-order the pack (forwards) */
    for (i = 0; i < INVEN_PACK; i++) {

	/* Mega-Hack -- allow "proper" over-flow */
	if ((i == INVEN_PACK) && (inven_cnt == INVEN_PACK)) break;

        /* Get the item */
        i_ptr = &inventory[i];

        /* Skip empty slots */
        if (!i_ptr->k_idx) continue;

        /* Get the "value" of the item */
        i_value = item_value(i_ptr);

        /* Scan every occupied slot */
        for (j = 0; j < INVEN_PACK; j++) {

            /* Get the item already there */
            j_ptr = &inventory[j];

            /* Use empty slots */
            if (!j_ptr->k_idx) break;

            /* Hack -- readable books always come first */
            if ((i_ptr->tval == mp_ptr->spell_book) &&
                (j_ptr->tval != mp_ptr->spell_book)) break;
            if ((j_ptr->tval == mp_ptr->spell_book) &&
                (i_ptr->tval != mp_ptr->spell_book)) continue;

            /* Objects sort by decreasing type */
            if (i_ptr->tval > j_ptr->tval) break;
            if (i_ptr->tval < j_ptr->tval) continue;

            /* Non-aware (flavored) items always come last */
            if (!inven_aware_p(i_ptr)) continue;
            if (!inven_aware_p(j_ptr)) break;

            /* Objects sort by increasing sval */
            if (i_ptr->sval < j_ptr->sval) break;
            if (i_ptr->sval > j_ptr->sval) continue;

            /* Unidentified objects always come last */
            if (!inven_known_p(i_ptr)) continue;
            if (!inven_known_p(j_ptr)) break;

            /* Determine the "value" of the pack item */
            j_value = item_value(j_ptr);

            /* Objects sort by decreasing value */
            if (i_value > j_value) break;
            if (i_value < j_value) continue;
        }

        /* Never move down */
        if (j >= i) continue;

        /* Take note */
        flag = TRUE;

        /* Save the moving item */
        temp = inventory[i];

        /* Structure slide (make room) */
        for (k = i; k > j; k--) {

            /* Slide the item */
            inventory[k] = inventory[k-1];
        }

	/* Insert the moved item */
	inventory[j] = temp;

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);
    }

    /* Message */
    if (flag) msg_print("You reorder some items in your pack.");
}




/*
 * Handle "p_ptr->update" and "p_ptr->redraw".
 *
 * Note the use of two passes to avoid accidentally drawing the
 * dungeon while in a store or other full screen situation.
 */
void handle_stuff(void)
{
    /* Update (first pass) */
    if (p_ptr->update) {

        if (p_ptr->update & PU_BONUS) {
            p_ptr->update &= ~(PU_BONUS);
            calc_bonuses();
        }

        if (p_ptr->update & PU_HP) {
            p_ptr->update &= ~(PU_HP);
            calc_hitpoints();
        }

        if (p_ptr->update & PU_MANA) {
            p_ptr->update &= ~(PU_MANA);
            calc_mana();
        }

        if (p_ptr->update & PU_SPELLS) {
            p_ptr->update &= ~(PU_SPELLS);
            calc_spells();
        }

        if (p_ptr->update & PU_COMBINE) {
            p_ptr->update &= ~(PU_COMBINE);
            if (auto_combine_pack) combine_pack();
        }
        
        if (p_ptr->update & PU_REORDER) {
            p_ptr->update &= ~(PU_REORDER);
            if (auto_reorder_pack) reorder_pack();
        }
    }


    /* Redraw (first pass) */
    if (p_ptr->redraw) {

        /* Hack -- Redraw "recent" monster race */
        if (p_ptr->redraw & PR_RECENT) {
            p_ptr->redraw &= ~(PR_RECENT);
            recent_fix();
        }

        /* Hack -- Redraw "choices" or whatever */
        if (p_ptr->redraw & PR_CHOOSE) {
            p_ptr->redraw &= ~(PR_CHOOSE);
            choose_fix();
        }
    }


    /* Character is not ready yet, no screen updates */
    if (!character_generated) return;


    /* Character is in "icky" mode, no screen updates */
    if (character_icky) return;


    /* Update (second pass) */
    if (p_ptr->update) {

        if (p_ptr->update & PU_NOTE) {
            p_ptr->update &= ~(PU_NOTE);
            forget_lite();
            forget_view();
        }


        if (p_ptr->update & PU_VIEW) {
            p_ptr->update &= ~(PU_VIEW);
            update_view();
        }

        if (p_ptr->update & PU_LITE) {
            p_ptr->update &= ~(PU_LITE);
            update_lite();
        }


        if (p_ptr->update & PU_FLOW) {
            p_ptr->update &= ~(PU_FLOW);
            update_flow();
        }


        if (p_ptr->update & PU_DISTANCE) {
            p_ptr->update &= ~(PU_DISTANCE);
            p_ptr->update &= ~(PU_MONSTERS);
            update_monsters(TRUE);
        }

        if (p_ptr->update & PU_MONSTERS) {
            p_ptr->update &= ~(PU_MONSTERS);
            update_monsters(FALSE);
        }
    }


    /* Redraw (second pass) */
    if (p_ptr->redraw) {


        /* Hack -- clear the screen */
        if (p_ptr->redraw & PR_WIPE) {
            p_ptr->redraw &= ~PR_WIPE;
            msg_print(NULL);
            clear_screen();
        }


        if (p_ptr->redraw & PR_MAP) {
            p_ptr->redraw &= ~(PR_MAP);
            prt_map();
        }


        if (p_ptr->redraw & PR_BASIC) {
            p_ptr->redraw &= ~(PR_BASIC);
            p_ptr->redraw &= ~(PR_MISC | PR_TITLE | PR_STATS);
            p_ptr->redraw &= ~(PR_LEV | PR_EXP | PR_GOLD);
            p_ptr->redraw &= ~(PR_ARMOR | PR_HP | PR_MANA);
            p_ptr->redraw &= ~(PR_DEPTH | PR_EQUIPPY | PR_HEALTH);
            prt_frame_basic();
        }

        if (p_ptr->redraw & PR_MISC) {
            p_ptr->redraw &= ~(PR_MISC);
            prt_field(rp_ptr->title, ROW_RACE, COL_RACE);
            prt_field(cp_ptr->title, ROW_CLASS, COL_CLASS);
        }

        if (p_ptr->redraw & PR_TITLE) {
            p_ptr->redraw &= ~(PR_TITLE);
            prt_title();
        }

        if (p_ptr->redraw & PR_LEV) {
            p_ptr->redraw &= ~(PR_LEV);
            prt_level();
        }

        if (p_ptr->redraw & PR_EXP) {
            p_ptr->redraw &= ~(PR_EXP);
            prt_exp();
        }

        if (p_ptr->redraw & PR_STATS) {
            p_ptr->redraw &= ~(PR_STATS);
            prt_stat(A_STR);
            prt_stat(A_INT);
            prt_stat(A_WIS);
            prt_stat(A_DEX);
            prt_stat(A_CON);
            prt_stat(A_CHR);
        }

        if (p_ptr->redraw & PR_ARMOR) {
            p_ptr->redraw &= ~(PR_ARMOR);
            prt_ac();
        }

        if (p_ptr->redraw & PR_HP) {
            p_ptr->redraw &= ~(PR_HP);
            prt_hp();
        }

        if (p_ptr->redraw & PR_MANA) {
            p_ptr->redraw &= ~(PR_MANA);
            prt_sp();
        }

        if (p_ptr->redraw & PR_GOLD) {
            p_ptr->redraw &= ~(PR_GOLD);
            prt_gold();
        }

        if (p_ptr->redraw & PR_DEPTH) {
            p_ptr->redraw &= ~(PR_DEPTH);
            prt_depth();
        }

        if (p_ptr->redraw & PR_EQUIPPY) {
            p_ptr->redraw &= ~(PR_EQUIPPY);
            if (equippy_chars) prt_equippy_chars();
        }

        if (p_ptr->redraw & PR_HEALTH) {
            p_ptr->redraw &= ~(PR_HEALTH);
            if (show_health_bar) health_redraw();
        }


        if (p_ptr->redraw & PR_EXTRA) {
            p_ptr->redraw &= ~(PR_EXTRA);
            p_ptr->redraw &= ~(PR_CUT | PR_STUN);
            p_ptr->redraw &= ~(PR_HUNGER);
            p_ptr->redraw &= ~(PR_BLIND | PR_CONFUSED);
            p_ptr->redraw &= ~(PR_AFRAID | PR_POISONED);
            p_ptr->redraw &= ~(PR_STATE | PR_SPEED | PR_STUDY);
            prt_frame_extra();
        }

        if (p_ptr->redraw & PR_CUT) {
            p_ptr->redraw &= ~(PR_CUT);
            prt_cut();
        }

        if (p_ptr->redraw & PR_STUN) {
            p_ptr->redraw &= ~(PR_STUN);
            prt_stun();
        }

        if (p_ptr->redraw & PR_HUNGER) {
            p_ptr->redraw &= ~(PR_HUNGER);
            prt_hunger();
        }

        if (p_ptr->redraw & PR_BLIND) {
            p_ptr->redraw &= ~(PR_BLIND);
            prt_blind();
        }

        if (p_ptr->redraw & PR_CONFUSED) {
            p_ptr->redraw &= ~(PR_CONFUSED);
            prt_confused();
        }

        if (p_ptr->redraw & PR_AFRAID) {
            p_ptr->redraw &= ~(PR_AFRAID);
            prt_afraid();
        }

        if (p_ptr->redraw & PR_POISONED) {
            p_ptr->redraw &= ~(PR_POISONED);
            prt_poisoned();
        }

        if (p_ptr->redraw & PR_STATE) {
            p_ptr->redraw &= ~(PR_STATE);
            prt_state();
        }

        if (p_ptr->redraw & PR_SPEED) {
            p_ptr->redraw &= ~(PR_SPEED);
            prt_speed();
        }

        if (p_ptr->redraw & PR_STUDY) {
            p_ptr->redraw &= ~(PR_STUDY);
            prt_study();
        }
    }
}




/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_NOTE", which is needed to memorize any terrain
 * features which suddenly become "visible".  Note that blindness is
 * currently the only thing which can affect "player_can_see_bold()".
 */
bool set_blind(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->blind) {
            msg_print("You are blind!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->blind) {
            msg_print("You can see again.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->blind = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Update stuff */
    p_ptr->update |= (PU_NOTE | PU_VIEW | PU_LITE);

    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw the map */
    p_ptr->redraw |= (PR_MAP);
    
    /* Redraw the "blind" */
    p_ptr->redraw |= (PR_BLIND);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->confused", notice observable changes
 */
bool set_confused(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->confused) {
            msg_print("You are confused!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->confused) {
            msg_print("You feel less confused now.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->confused = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Redraw the "confused" */
    p_ptr->redraw |= (PR_CONFUSED);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool set_poisoned(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->poisoned) {
            msg_print("You are poisoned!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->poisoned) {
            msg_print("You are no longer poisoned.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->poisoned = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Redraw the "poisoned" */
    p_ptr->redraw |= (PR_POISONED);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->afraid", notice observable changes
 */
bool set_afraid(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->afraid) {
            msg_print("You are terrified!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->afraid) {
            msg_print("You feel bolder now.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->afraid = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Redraw the "afraid" */
    p_ptr->redraw |= (PR_AFRAID);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool set_paralyzed(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->paralyzed) {
            msg_print("You are paralyzed!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->paralyzed) {
            msg_print("You can move again.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->paralyzed = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Redraw the state */
    p_ptr->redraw |= (PR_STATE);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool set_image(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->image) {
            msg_print("You feel drugged!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->image) {
            msg_print("You can see clearly again.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->image = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Redraw the map */
    p_ptr->redraw |= (PR_MAP);
    
    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->fast", notice observable changes
 */
bool set_fast(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->fast) {
            msg_print("You feel yourself moving faster!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->fast) {
            msg_print("You feel yourself slow down.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->fast = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool set_slow(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->slow) {
            msg_print("You feel yourself moving slower!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->slow) {
            msg_print("You feel yourself speed up.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->slow = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->shield", notice observable changes
 */
bool set_shield(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->shield) {
            msg_print("A mystic shield forms around your body!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->shield) {
            msg_print("Your mystic shield crumbles away.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->shield = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool set_blessed(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->blessed) {
            msg_print("You feel righteous!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->blessed) {
            msg_print("The prayer has expired.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->blessed = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool set_hero(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->hero) {
            msg_print("You feel like a hero!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->hero) {
            msg_print("The heroism wears off.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->hero = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Recalculate hitpoints */
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->shero", notice observable changes
 */
bool set_shero(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->shero) {
            msg_print("You feel like a killing machine!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->shero) {
            msg_print("You feel less Berserk.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->shero = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Recalculate hitpoints */
    p_ptr->update |= (PU_HP);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool set_protevil(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->protevil) {
            msg_print("You feel safe from evil!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->protevil) {
            msg_print("You no longer feel safe from evil.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->protevil = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool set_invuln(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->invuln) {
            msg_print("You feel invulnerable!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->invuln) {
            msg_print("You feel vulnerable once more.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->invuln = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool set_tim_invis(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->tim_invis) {
            msg_print("Your eyes feel very sensitive!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->tim_invis) {
            msg_print("Your eyes feel less sensitive.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_invis = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool set_tim_infra(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->tim_infra) {
            msg_print("Your eyes begin to tingle!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->tim_infra) {
            msg_print("Your eyes stop tingling.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->tim_infra = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);
    
    /* Update the monsters */
    p_ptr->update |= (PU_MONSTERS);
    
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool set_oppose_acid(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->oppose_acid) {
            msg_print("You feel resistant to acid!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->oppose_acid) {
            msg_print("You feel less resistant to acid.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_acid = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool set_oppose_elec(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->oppose_elec) {
            msg_print("You feel resistant to electricity!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->oppose_elec) {
            msg_print("You feel less resistant to electricity.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_elec = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool set_oppose_fire(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->oppose_fire) {
            msg_print("You feel resistant to fire!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->oppose_fire) {
            msg_print("You feel less resistant to fire.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_fire = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool set_oppose_cold(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->oppose_cold) {
            msg_print("You feel resistant to cold!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->oppose_cold) {
            msg_print("You feel less resistant to cold.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_cold = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool set_oppose_pois(int v)
{
    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Open */
    if (v) {
        if (!p_ptr->oppose_pois) {
            msg_print("You feel resistant to poison!");
            notice = TRUE;
        }
    }

    /* Shut */
    else {
        if (p_ptr->oppose_pois) {
            msg_print("You feel less resistant to poison.");
            notice = TRUE;
        }
    }

    /* Use the value */
    p_ptr->oppose_pois = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_stun(int v)
{
    int old_aux, new_aux;

    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Knocked out */
    if (p_ptr->stun > 100) {
        old_aux = 3;
    }

    /* Heavy stun */
    else if (p_ptr->stun > 50) {
        old_aux = 2;
    }

    /* Stun */
    else if (p_ptr->stun > 0) {
        old_aux = 1;
    }

    /* None */
    else {
        old_aux = 0;
    }
    
    /* Knocked out */
    if (v > 100) {
        new_aux = 3;
    }

    /* Heavy stun */
    else if (v > 50) {
        new_aux = 2;
    }

    /* Stun */
    else if (v > 0) {
        new_aux = 1;
    }

    /* None */
    else {
        new_aux = 0;
    }
    
    /* Increase cut */
    if (new_aux > old_aux) {

        /* Describe the state */
        switch (new_aux) {

            /* Stun */
            case 1:
                msg_print("You have been stunned.");
                break;

            /* Heavy stun */
            case 2:
                msg_print("You have been heavily stunned.");
                break;

            /* Knocked out */
            case 3:
                msg_print("You have been knocked out.");
                break;
        }

        /* Notice */
        notice = TRUE;
    }

    /* Decrease cut */
    else if (new_aux < old_aux) {

        /* Describe the state */
        switch (new_aux) {

            /* None */
            case 0:
                msg_print("You are no longer stunned.");
                if (disturb_other) disturb(0, 0);
                break;
        }

        /* Notice */
        notice = TRUE;
    }

    /* Use the value */
    p_ptr->stun = v;

    /* No change */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw the "stun" */
    p_ptr->redraw |= (PR_STUN);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_cut(int v)
{
    int old_aux, new_aux;

    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    /* Mortal wound */
    if (p_ptr->cut > 1000) {
        old_aux = 7;
    }

    /* Deep gash */
    else if (p_ptr->cut > 200) {
        old_aux = 6;
    }
    
    /* Severe cut */
    else if (p_ptr->cut > 100) {
        old_aux = 5;
    }

    /* Nasty cut */
    else if (p_ptr->cut > 50) {
        old_aux = 4;
    }

    /* Bad cut */
    else if (p_ptr->cut > 25) {
        old_aux = 3;
    }

    /* Light cut */
    else if (p_ptr->cut > 10) {
        old_aux = 2;
    }

    /* Graze */
    else if (p_ptr->cut > 0) {
        old_aux = 1;
    }

    /* None */
    else {
        old_aux = 0;
    }

    /* Mortal wound */
    if (v > 1000) {
        new_aux = 7;
    }

    /* Deep gash */
    else if (v > 200) {
        new_aux = 6;
    }
    
    /* Severe cut */
    else if (v > 100) {
        new_aux = 5;
    }

    /* Nasty cut */
    else if (v > 50) {
        new_aux = 4;
    }

    /* Bad cut */
    else if (v > 25) {
        new_aux = 3;
    }

    /* Light cut */
    else if (v > 10) {
        new_aux = 2;
    }

    /* Graze */
    else if (v > 0) {
        new_aux = 1;
    }

    /* None */
    else {
        new_aux = 0;
    }

    /* Increase cut */
    if (new_aux > old_aux) {

        /* Describe the state */
        switch (new_aux) {

            /* Graze */
            case 1:
                msg_print("You have been given a graze.");
                break;

            /* Light cut */
            case 2:
                msg_print("You have been given a light cut.");
                break;

            /* Bad cut */
            case 3:
                msg_print("You have been given a bad cut.");
                break;

            /* Nasty cut */
            case 4:
                msg_print("You have been given a nasty cut.");
                break;

            /* Severe cut */
            case 5:
                msg_print("You have been given a severe cut.");
                break;

            /* Deep gash */
            case 6:
                msg_print("You have been given a deep gash.");
                break;

            /* Mortal wound */
            case 7:
                msg_print("You have been given a mortal wound.");
                break;
        }

        /* Notice */
        notice = TRUE;
    }

    /* Decrease cut */
    else if (new_aux < old_aux) {

        /* Describe the state */
        switch (new_aux) {

            /* None */
            case 0:
                msg_print("You are no longer bleeding.");
                if (disturb_other) disturb(0, 0);
                break;
        }

        /* Notice */
        notice = TRUE;
    }

    /* Use the value */
    p_ptr->cut = v;

    /* No change */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw the "cut" */
    p_ptr->redraw |= (PR_CUT);

    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool set_food(int v)
{
    int old_aux, new_aux;

    bool notice = FALSE;

    /* Hack -- Force good values */
    v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

    /* Fainting / Starving */
    if (p_ptr->food < PY_FOOD_FAINT) {
        old_aux = 0;
    }

    /* Weak */
    else if (p_ptr->food < PY_FOOD_WEAK) {
        old_aux = 1;
    }

    /* Hungry */
    else if (p_ptr->food < PY_FOOD_ALERT) {
        old_aux = 2;
    }

    /* Normal */
    else if (p_ptr->food < PY_FOOD_FULL) {
        old_aux = 3;
    }

    /* Full */
    else if (p_ptr->food < PY_FOOD_MAX) {
        old_aux = 4;
    }

    /* Gorged */
    else {
        old_aux = 5;
    }

    /* Fainting / Starving */
    if (v < PY_FOOD_FAINT) {
        new_aux = 0;
    }

    /* Weak */
    else if (v < PY_FOOD_WEAK) {
        new_aux = 1;
    }

    /* Hungry */
    else if (v < PY_FOOD_ALERT) {
        new_aux = 2;
    }

    /* Normal */
    else if (v < PY_FOOD_FULL) {
        new_aux = 3;
    }

    /* Full */
    else if (v < PY_FOOD_MAX) {
        new_aux = 4;
    }

    /* Gorged */
    else {
        new_aux = 5;
    }

    /* Food increase */
    if (new_aux > old_aux) {

        /* Describe the state */
        switch (new_aux) {
        
            /* Hungry */
            case 2:
                msg_print("You are still hungry.");
                break;

            /* Normal */
            case 3:
                msg_print("You are no longer hungry.");
                break;

            /* Full */
            case 4:
                msg_print("You are full.");
                break;

            /* Bloated */
            case 5:
                msg_print("You have gorged yourself into a bloated state!");
                break;
        }
        
        /* Change */
        notice = TRUE;
    }
    
    /* Food decrease */
    else if (new_aux < old_aux) {

        /* Describe the state */
        switch (new_aux) {
        
            /* Fainting / Starving */
            case 0:
                msg_print("You are getting faint from hunger.");
                break;

            /* Weak */
            case 1:
                msg_print("You are getting weak from hunger.");
                break;

            /* Hungry */
            case 2:
                msg_print("You are getting hungry.");
                break;
        }

        /* Change */
        notice = TRUE;
    }

    /* Use the value */
    p_ptr->food = v;

    /* Nothing to notice */
    if (!notice) return (FALSE);

    /* Disturb */
    if (disturb_other) disturb(0,0);

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Redraw hunger */
    p_ptr->redraw |= (PR_HUNGER);
        
    /* Handle stuff */
    handle_stuff();

    /* Result */
    return (TRUE);
}


