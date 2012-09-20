/* File: misc1.c */

/* Purpose: misc utility and initialization code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


#ifndef FSCALE
# define FSCALE (1<<8)
#endif


#ifdef CHECK_HOURS

typedef struct statstime {
    int                 cp_time[4];
    int                 dk_xfer[4];
    unsigned int        v_pgpgin;
    unsigned int        v_pgpgout;
    unsigned int        v_pswpin;
    unsigned int        v_pswpout;
    unsigned int        v_intr;
    int                 if_ipackets;
    int                 if_ierrors;
    int                 if_opackets;
    int                 if_oerrors;
    int                 if_collisions;
    unsigned int        v_swtch;
    long                avenrun[3];
    struct timeval      boottime;
    struct timeval      curtime;

} statstime;

#endif


/*
 * The standard R.N.G. state array (used below)
 */
static char *old_state = NULL;

/*
 * Hack -- The dummy R.N.G. state array
 * Used to keep consistent object colors and town layout
 */
static char *dummy_state = NULL;


/*
 * Gets a new random seed for the random number generator 
 * Hack -- saves seeds for the town layout and object colors
 */
void init_seeds(void)
{
    static u32b old_buf[256/4];
    static u32b dummy_buf[8/4];

    /* Allocate some RNG arrays */
    old_state = (char*)old_buf; /* excellent R.N.G. */
    dummy_state = (char*)dummy_buf; /* simple R.N.G. */

    /* Grab a random seed from the clock -- is this ignored by unix? */
    (void)initstate(time(NULL), dummy_state, 8);

    /* Note that "getpid()" is less than informative except on Unix */
    /* This may need to be changed.  It's fine for PCs, anyways... -CFT */

#ifdef SET_UID

    /* Grab a random seed from the clock & PID... */
    (void)initstate(((getpid() << 1) * (time(NULL) >> 3)), old_state, 256);

#else /* SET_UID */

    /* Use the seed we got above to re-seed */
    (void)initstate(random(), old_state, 256);

#endif /* SET_UID */

    /* Hack -- Extract seeds for the town layout and object colors */
    town_seed = random();
    randes_seed = random();
}


/*
 * change to different random number generator state
 * Hack -- used to keep consistent object colors and town layout
 */
void set_seed(u32b seed)
{
    setstate(dummy_state);
    srandom((seed % 2147483646L) + 1);
}


/*
 * restore the normal random generator state
 */
void reset_seed(void)
{
    (void)setstate(old_state);
}


/*
 * Check the day-time strings to see if open		-RAK-	
 */
int check_time(void)
{

#ifdef CHECK_HOURS

# if !defined(time_t)
#  define time_t long
# endif

    time_t              c;
    register struct tm *tp;

#ifndef __MINT__
    struct statstime    st;
#endif

    c = time((time_t *)0);
    tp = localtime(&c);
    if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') {
	return FALSE;
    }

#if !(defined(__MINT__) || defined(NCR3K) || defined(linux) || \
      defined(__386BSD__) || defined (__osf__))

    else {
	if (!rstat("localhost", &st)) {
	    if (((int)((double)st.avenrun[2] / (double)FSCALE)) >= (int)LOAD) {
		return FALSE;
	    }
	}
    }

#endif /* MINT, etc */

#endif /* CHECK_HOURS - [cjh] */

    return TRUE;
}



/*
 * Generates a random integer number of NORMAL distribution -RAK- 
 *
 * XXX This should probably be retested just for paranoia's sake.
 *
 * There has been some suspicion that this function actually uses
 * a standard deviation of about 1.25 times the one given. XXX XXX
 */
int randnor(int mean, int stand)
{
    register int tmp, offset, low, iindex, high;

    tmp = randint(MAX_SHORT);

    /* off scale, assign random value between 4 and 5 times SD */
    if (tmp == MAX_SHORT) {

	offset = 4 * stand + randint(stand);

	/* one half are negative */
	if (rand_int(2)) offset = (-offset);

	return (mean + offset);
    }


    /* binary search normal normal_table to get index that matches tmp */
    low = 0;
    iindex = NORMAL_TABLE_SIZE >> 1;
    high = NORMAL_TABLE_SIZE;

    /* this takes up to 8 iterations */
    while (TRUE) {

	if ((normal_table[iindex] == tmp) || (high == (low + 1))) {
	    break;
	}

	if (normal_table[iindex] > tmp) {
	    high = iindex;
	    iindex = low + ((iindex - low) >> 1);
	}
	else {
	    low = iindex;
	    iindex = iindex + ((high - iindex) >> 1);
	}
    }

    /* might end up one below target, check that here */
    if (normal_table[iindex] < tmp) iindex = iindex + 1;

    /* normal_table is based on SD of 64, so adjust the index value here, */
    /* round the half way case up */
    offset = ((stand * iindex) + (NORMAL_TABLE_SD >> 1)) / NORMAL_TABLE_SD;


    /* one half should be negative */
    if (rand_int(2)) offset = (-offset);

    return (mean + offset);
}


/*
 * Returns position of first set bit (and clears that bit) 
 */
int bit_pos(u32b *test)
{
    register int    i;
    register u32b mask = 0x1L;

    /* No bits set? */
    if (!(*test)) return (-1);
    
    /* Scan the input */
    for (i = 0; i < sizeof(*test) * 8; i++) {

	/* Test and clear */
	if (*test & mask) {
	    *test &= ~mask;
	    return (i);
	}

	/* Next! */
	mask <<= 0x1L;
    }

    /* Paranoia -- massive error */
    return (-99);
}


/*
 * Calculates current boundaries
 * Called below and from "do_cmd_locate()".
 */
void panel_bounds()
{
    panel_row_min = panel_row * (SCREEN_HEIGHT / 2);
    panel_row_max = panel_row_min + SCREEN_HEIGHT - 1;
    panel_row_prt = panel_row_min - 1;
    panel_col_min = panel_col * (SCREEN_WIDTH / 2);
    panel_col_max = panel_col_min + SCREEN_WIDTH - 1;
    panel_col_prt = panel_col_min - 13;
}


/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
int get_panel(int y, int x, int update)
{
    int prow = panel_row;
    int pcol = panel_col;

    if (update || (y < panel_row_min + 2) || (y > panel_row_max - 2)) {
	prow = ((y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2));
	if (prow > max_panel_rows) prow = max_panel_rows;
	else if (prow < 0) prow = 0;
    }

    if (update || (x < panel_col_min + 3) || (x > panel_col_max - 3)) {
	pcol = ((x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2));
	if (pcol > max_panel_cols) pcol = max_panel_cols;
	else if (pcol < 0) pcol = 0;
    }

    /* Check for "no change" */
    if (!update && (prow == panel_row) && (pcol == panel_col)) return (FALSE);

    /* Save the new panel info */
    panel_row = prow;
    panel_col = pcol;

    /* Recalculate the boundaries */
    panel_bounds();

    /* Redraw the map */
    prt_map();

    /* Hack -- optional disturb at "edge of panel" */
    if (find_bound) disturb(0, 0);

    /* The map was redrawn */
    return (TRUE);
}


/*
 * Approximate Distance between two points.
 *
 * When either the X or Y component dwarfs the other component,
 * this function is almost perfect, and otherwise, it tends to
 * over-estimate about one grid per fifteen grids of distance.
 */
int distance(int y1, int x1, int y2, int x2)
{
    register int dy, dx, d;

    /* Find the absolute y/x distance components */
    dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);

    /* Hack -- approximate the distance */
    d = (dy + dx + ((dy > dx) ? dy : dx)) >> 1;

    /* Return the distance */
    return (d);
}



/*
 * Generates damage for "2d6" style dice rolls
 */
int damroll(int num, int sides)
{
    register int i, sum = 0;
    for (i = 0; i < num; i++) sum += randint(sides);
    return (sum);
}


/* 
 * Old "array" format
 */
int pdamroll(byte *array)
{
    return damroll((int)array[0], (int)array[1]);
}


/*
 * Same as above, but always maximal
 */
int max_hp(byte *array)
{
    return ((int)(array[0]) * (int)(array[1]));
}



/*
 * Add to the players food time				-RAK-	 
 */
void add_food(int num)
{
    register int           extra, penalty;

    if (p_ptr->food < 0) p_ptr->food = 0;
    p_ptr->food += num;
    /* overflow check */
    if (num > 0 && p_ptr->food <= 0) p_ptr->food = 32000;

    if (p_ptr->food > PLAYER_FOOD_MAX) {

	msg_print("You are bloated from overeating. ");

    /*
     * Calculate how much of num is responsible for the bloating. Give the
     * player food credit for 1/50, and slow him for that many turns also.  
     */
	extra = p_ptr->food - PLAYER_FOOD_MAX;
	if (extra > num) extra = num;
	penalty = extra / 50;

	p_ptr->slow += penalty;
	if (extra == num) p_ptr->food = p_ptr->food - num + penalty;
	else p_ptr->food = PLAYER_FOOD_MAX + penalty;
    }

    else if (p_ptr->food > PLAYER_FOOD_FULL) {
	msg_print("You are full. ");
    }
}


