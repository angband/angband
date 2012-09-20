/* Misc1.c: misc utility and initialization code, magic objects code

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */
#include <sys/time.h>
#define FSCALE (1<<8)
#ifdef Pyramid
#include <sys/time.h>
#else
#include <time.h>
#endif
#if !defined(GEMDOS) && !defined(MAC)
#ifndef VMS
#include <sys/types.h>
#else
#include <types.h>
#endif
#endif

#include <stdio.h>
#include "constant.h"
#include "monster.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#else
#include "string.h"
#endif
#else
#include <strings.h>
#endif

#ifndef MSDOS
typedef struct statstime {
  int cp_time[4];
  int dk_xfer[4];
  unsigned int v_pgpgin;
  unsigned int v_pgpgout;
  unsigned int v_pswpin;
  unsigned int v_pswpout;
  unsigned int v_intr;
  int if_ipackets;
  int if_ierrors;
  int if_opackets;
  int if_oerrors;
  int if_collisions;
  unsigned int v_swtch;
  long avenrun[3];
  struct timeval boottime;
  struct timeval curtime;
} statstime;
#endif

#if defined(LINT_ARGS)
static void compact_monsters(void);
static void compact_objects(void);
#else
static int compact_monsters(ARG_VOID);
static void compact_objects(ARG_VOID);
static char *cap(ARG_CHAR_PTR);
#endif

#if !defined(ATARIST_MWC) && !defined(MAC)
long time();
#endif
struct tm *localtime();
extern int8u peek;
extern int rating;

/* gets a new random seed for the random number generator */
void init_seeds(seed)
int32u seed;
{
  register int32u clock;

  if (seed == 0)
    clock = time((long *)0);
  else
    clock = seed;
  randes_seed = (unsigned int)clock;

  clock += 8762;
  town_seed = (unsigned int)clock;

  clock += 113452L;
  set_rnd_seed(clock);
  /* make it a little more random */
  for (clock = randint(100); clock != 0; clock--)
    (void) rnd();
}

/* holds the previous rnd state */
static int32u old_seed;

/* change to different random number generator state */
void set_seed(seed)
int32u seed;
{
  old_seed = get_rnd_seed ();

  /* want reproducible state here */
  set_rnd_seed (seed);
}


/* restore the normal random generator state */
void reset_seed()
{
  set_rnd_seed (old_seed);
}


/* Check the day-time strings to see if open		-RAK-	*/
int check_time()
{
  long clock;
  register struct tm *tp;
#ifndef MSDOS
  struct statstime st;
#endif

  clock = time((long *)0);
  tp = localtime(&clock);
  if (days[tp->tm_wday][tp->tm_hour+4] != 'X') {
    return FALSE;
  }
#ifndef MSDOS 
 else {
    if (!rstat("localhost", &st)) {
      if (((int) ((double)st.avenrun[2]/(double) FSCALE)) >= (int)LOAD)
	return FALSE;
    }
  }
#endif
  return TRUE;
}


/* Generates a random integer x where 1<=X<=MAXVAL	-RAK-	*/
int randint(maxval)
int maxval;
{
  register int32u randval;

  if (maxval<1) return 1;
  randval = rnd ();
  return ((int)((randval % maxval) + 1));
}

/* Generates a random integer number of NORMAL distribution -RAK-*/
int randnor(mean, stand)
int mean, stand;
{
  register int tmp, offset, low, iindex, high;

#if 0
  /* alternate randnor code, slower but much smaller since no table */
  /* 2 per 1,000,000 will be > 4*SD, max is 5*SD */
  tmp = damroll(8, 99);	 /* mean 400, SD 81 */
  tmp = (tmp - 400) * stand / 81;
  return tmp + mean;
#endif

  tmp = randint(MAX_SHORT);

  /* off scale, assign random value between 4 and 5 times SD */
  if (tmp == MAX_SHORT)
    {
      offset = 4 * stand + randint(stand);

      /* one half are negative */
      if (randint(2) == 1)
	offset = -offset;

      return mean + offset;
    }

  /* binary search normal normal_table to get index that matches tmp */
  /* this takes up to 8 iterations */
  low = 0;
  iindex = NORMAL_TABLE_SIZE >> 1;
  high = NORMAL_TABLE_SIZE;
  while (TRUE)
    {
      if ((normal_table[iindex] == tmp) || (high == (low+1)))
	break;
      if (normal_table[iindex] > tmp)
	{
	  high = iindex;
	  iindex = low + ((iindex - low) >> 1);
	}
      else
	{
	  low = iindex;
	  iindex = iindex + ((high - iindex) >> 1);
	}
    }

  /* might end up one below target, check that here */
  if (normal_table[iindex] < tmp)
    iindex = iindex + 1;

  /* normal_table is based on SD of 64, so adjust the index value here,
     round the half way case up */
  offset = ((stand * iindex) + (NORMAL_TABLE_SD >> 1)) / NORMAL_TABLE_SD;

  /* one half should be negative */
  if (randint(2) == 1)
    offset = -offset;

  return mean + offset;
}


/* Returns position of first set bit			-RAK-	*/
/*     and clears that bit */
int bit_pos(test)
int32u *test;
{
  register int i;
  register int32u mask = 0x1L;

  for (i = 0; i < sizeof(*test)*8; i++) {
    if (*test & mask) {
      *test &= ~mask;
      return(i);
    }
    mask <<= 0x1L;
  }

  /* no one bits found */
  return(-1);
}

/* Checks a co-ordinate for in bounds status		-RAK-	*/
int in_bounds(y, x)
int y, x;
{
  if ((y > 0) && (y < cur_height-1) && (x > 0) && (x < cur_width-1))
    return(TRUE);
  else
    return(FALSE);
}


/* Calculates current boundaries				-RAK-	*/
void panel_bounds()
{
  panel_row_min = panel_row*(SCREEN_HEIGHT/2);
  panel_row_max = panel_row_min + SCREEN_HEIGHT - 1;
  panel_row_prt = panel_row_min - 1;
  panel_col_min = panel_col*(SCREEN_WIDTH/2);
  panel_col_max = panel_col_min + SCREEN_WIDTH - 1;
  panel_col_prt = panel_col_min - 13;
}


/* Given an row (y) and col (x), this routine detects  -RAK-	*/
/* when a move off the screen has occurred and figures new borders.
   Force forcses the panel bounds to be recalculated, useful for 'W'here. */
int get_panel(y, x, force)
int y, x, force;
{
  register int prow, pcol;
  register int panel;

  prow = panel_row;
  pcol = panel_col;
  if (force || (y < panel_row_min + 2) || (y > panel_row_max - 2))
    {
      prow = ((y - SCREEN_HEIGHT/4)/(SCREEN_HEIGHT/2));
      if (prow > max_panel_rows)
	prow = max_panel_rows;
      else if (prow < 0)
	prow = 0;
    }
  if (force || (x < panel_col_min + 3) || (x > panel_col_max - 3))
    {
      pcol = ((x - SCREEN_WIDTH/4)/(SCREEN_WIDTH/2));
      if (pcol > max_panel_cols)
	pcol = max_panel_cols;
      else if (pcol < 0)
	pcol = 0;
    }
  if ((prow != panel_row) || (pcol != panel_col))
    {
      panel_row = prow;
      panel_col = pcol;
      panel_bounds();
      panel = TRUE;
      /* stop movement if any */
      if (find_bound)
	end_find();
    }
  else
    panel = FALSE;
  return(panel);
}


/* Tests a given point to see if it is within the screen -RAK-	*/
/* boundaries.							  */
int panel_contains(y, x)
register int y, x;
{
  if ((y >= panel_row_min) && (y <= panel_row_max) &&
      (x >= panel_col_min) && (x <= panel_col_max))
    return (TRUE);
  else
    return (FALSE);
}


/* Distance between two points				-RAK-	*/
int distance(y1, x1, y2, x2)
int y1, x1, y2, x2;
{
  register int dy, dx;

  dy = y1 - y2;
  if (dy < 0)
    dy = -dy;
  dx = x1 - x2;
  if (dx < 0)
    dx = -dx;

  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}

/* Checks points north, south, east, and west for a wall -RAK-	*/
/* note that y,x is always in_bounds(), i.e. 0 < y < cur_height-1, and
   0 < x < cur_width-1	*/
int next_to_walls(y, x)
register int y, x;
{
  register int i;
  register cave_type *c_ptr;

  i = 0;
  c_ptr = &cave[y-1][x];
  if (c_ptr->fval >= MIN_CAVE_WALL)
    i++;
  c_ptr = &cave[y+1][x];
  if (c_ptr->fval >= MIN_CAVE_WALL)
    i++;
  c_ptr = &cave[y][x-1];
  if (c_ptr->fval >= MIN_CAVE_WALL)
    i++;
  c_ptr = &cave[y][x+1];
  if (c_ptr->fval >= MIN_CAVE_WALL)
    i++;

  return(i);
}


/* Checks all adjacent spots for corridors		-RAK-	*/
/* note that y, x is always in_bounds(), hence no need to check that
   j, k are in_bounds(), even if they are 0 or cur_x-1 is still works */
int next_to_corr(y, x)
register int y, x;
{
  register int k, j, i;
  register cave_type *c_ptr;

  i = 0;
  for (j = y - 1; j <= (y + 1); j++)
    for (k = x - 1; k <= (x + 1); k++)
      {
	c_ptr = &cave[j][k];
	/* should fail if there is already a door present */
	if (c_ptr->fval == CORR_FLOOR
	    && (c_ptr->tptr == 0 || t_list[c_ptr->tptr].tval < TV_MIN_DOORS))
	  i++;
      }
  return(i);
}


/* generates damage for 2d6 style dice rolls */
int damroll(num, sides)
int num, sides;
{
  register int i, sum = 0;

  for (i = 0; i < num; i++)
    sum += randint(sides);
  return(sum);
}

int pdamroll(array)
int8u *array;
{
  return damroll((int)array[0], (int)array[1]);
}


/* A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
   4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.

   Returns TRUE if a line of sight can be traced from x0, y0 to x1, y1.

   The LOS begins at the center of the tile [x0, y0] and ends at
   the center of the tile [x1, y1].  If los() is to return TRUE, all of
   the tiles this line passes through must be transparent, WITH THE
   EXCEPTIONS of the starting and ending tiles.

   We don't consider the line to be "passing through" a tile if
   it only passes across one corner of that tile. */

/* Because this function uses (short) ints for all calculations, overflow
   may occur if deltaX and deltaY exceed 90. */

int los(fromY, fromX, toY, toX)
int fromY, fromX, toY, toX;
{
  register int tmp, deltaX, deltaY;

  deltaX = toX - fromX;
  deltaY = toY - fromY;

  /* Adjacent? */
  if ((deltaX < 2) && (deltaX > -2) && (deltaY < 2) && (deltaY > -2))
    return TRUE;

  /* Handle the cases where deltaX or deltaY == 0. */
  if (deltaX == 0)
    {
      register int p_y;	/* y position -- loop variable	*/

      if (deltaY < 0)
	{
	  tmp = fromY;
	  fromY = toY;
	  toY = tmp;
	}
      for (p_y = fromY + 1; p_y < toY; p_y++)
	if (cave[p_y][fromX].fval >= MIN_CLOSED_SPACE)
	  return FALSE;
      return TRUE;
    }
  else if (deltaY == 0)
    {
      register int px;	/* x position -- loop variable	*/

      if (deltaX < 0)
	{
	  tmp = fromX;
	  fromX = toX;
	  toX = tmp;
	}
      for (px = fromX + 1; px < toX; px++)
	if (cave[fromY][px].fval >= MIN_CLOSED_SPACE)
	  return FALSE;
      return TRUE;
    }

  /* Now, we've eliminated all the degenerate cases.
     In the computations below, dy (or dx) and m are multiplied by a
     scale factor, scale = abs(deltaX * deltaY * 2), so that we can use
     integer arithmetic. */

  {
    register int px,	/* x position				*/
     p_y,		/* y position				*/
     scale2;		/* above scale factor / 2		*/
    int scale,		/* above scale factor			*/
     xSign,		/* sign of deltaX			*/
     ySign,		/* sign of deltaY			*/
     m;			/* slope or 1/slope of LOS		*/

    scale2 = abs(deltaX * deltaY);
    scale = scale2 << 1;
    xSign = (deltaX < 0) ? -1 : 1;
    ySign = (deltaY < 0) ? -1 : 1;

    /* Travel from one end of the line to the other, oriented along
       the longer axis. */

    if (abs(deltaX) >= abs(deltaY))
      {
	register int dy;		/* "fractional" y position	*/
	/* We start at the border between the first and second tiles,
	   where the y offset = .5 * slope.  Remember the scale
	   factor.  We have:

	   m = deltaY / deltaX * 2 * (deltaY * deltaX)
	     = 2 * deltaY * deltaY. */

	dy = deltaY * deltaY;
	m = dy << 1;
	px = fromX + xSign;

	/* Consider the special case where slope == 1. */
	if (dy == scale2)
	  {
	    p_y = fromY + ySign;
	    dy -= scale;
	  }
	else
	  p_y = fromY;

	while (toX - px)
	  {
	    if (cave[p_y][px].fval >= MIN_CLOSED_SPACE)
	      return FALSE;

	    dy += m;
	    if (dy < scale2)
	      px += xSign;
	    else if (dy > scale2)
	      {
		p_y += ySign;
		if (cave[p_y][px].fval >= MIN_CLOSED_SPACE)
		  return FALSE;
		px += xSign;
		dy -= scale;
	      }
	    else
	      {
		/* This is the case, dy == scale2, where the LOS
		   exactly meets the corner of a tile. */
		px += xSign;
		p_y += ySign;
		dy -= scale;
	      }
	  }
	return TRUE;
      }
    else
      {
	register int dx;		/* "fractional" x position	*/
	dx = deltaX * deltaX;
	m = dx << 1;

	p_y = fromY + ySign;
	if (dx == scale2)
	  {
	    px = fromX + xSign;
	    dx -= scale;
	  }
	else
	  px = fromX;

	while (toY - p_y)
	  {
	    if (cave[p_y][px].fval >= MIN_CLOSED_SPACE)
	      return FALSE;
	    dx += m;
	    if (dx < scale2)
	      p_y += ySign;
	    else if (dx > scale2)
	      {
		px += xSign;
		if (cave[p_y][px].fval >= MIN_CLOSED_SPACE)
		  return FALSE;
		p_y += ySign;
		dx -= scale;
	      }
	    else
	      {
		px += xSign;
		p_y += ySign;
		dx -= scale;
	      }
	  }
	return TRUE;
      }
  }
}


/* Returns symbol for given row, column			-RAK-	*/
unsigned char loc_symbol(y, x)
int y, x;
{
  register cave_type *cave_ptr;
  register struct flags *f_ptr;

#ifdef TC_COLOR
  textcolor(LIGHTGRAY);
#endif

  cave_ptr = &cave[y][x];
  f_ptr = &py.flags;

  if ((cave_ptr->cptr == 1) && (!find_flag || find_prself))
    return '@';
  else if (f_ptr->status & PY_BLIND)
    return ' ';
  else if ((f_ptr->image > 0) && (randint (12) == 1))
#ifdef TC_COLOR
    { textcolor( randint(15) );  return randint (95) + 31; }
#else
    return randint (95) + 31;
#endif
  else if ((cave_ptr->cptr > 1) && (m_list[cave_ptr->cptr].ml))
#ifdef TC_COLOR
  { textcolor(c_list[m_list[cave_ptr->cptr].mptr].color);
    return c_list[m_list[cave_ptr->cptr].mptr].cchar; }
#else
    return c_list[m_list[cave_ptr->cptr].mptr].cchar;
#endif
  else if (!cave_ptr->pl && !cave_ptr->tl && !cave_ptr->fm)
    return ' ';
  else if ((cave_ptr->tptr != 0)
	   && (t_list[cave_ptr->tptr].tval != TV_INVIS_TRAP))
#ifdef TC_COLOR
    {  if (t_list[cave_ptr->tptr].color != 0)
         textcolor(t_list[cave_ptr->tptr].color);
       else switch(t_list[cave_ptr->tptr].tval){
       	 case TV_POTION1:
       	 case TV_POTION2:
	   textcolor(tccolors[t_list[cave_ptr->tptr].subval
					- ITEM_SINGLE_STACK_MIN]);
	   break;
	 case TV_WAND:
	 case TV_ROD:
	   textcolor(tcmetals[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_STAFF:
	   textcolor(tcwoods[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_RING:
	   textcolor(tcrocks[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_AMULET:
	   textcolor(tcamulets[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_FOOD:
	   textcolor(tcmushrooms[t_list[cave_ptr->tptr].subval
	   				- ITEM_SINGLE_STACK_MIN]);
	   break;
	 default: textcolor( randint(15) ); /* should never happen. -CFT */
       } /* switch */
    return t_list[cave_ptr->tptr].tchar;
    } /* else-if clause */	 
#else
    return t_list[cave_ptr->tptr].tchar;
#endif
  else if (cave_ptr->fval <= MAX_CAVE_FLOOR)
#ifdef MSDOS
      return floorsym;
#else
    {
      return '.';
    }
#endif
  else if (cave_ptr->fval == GRANITE_WALL || cave_ptr->fval == BOUNDARY_WALL
	   || highlight_seams == FALSE)
    {
#ifdef MSDOS
      return wallsym;
#else
#ifndef ATARIST_MWC
      return '#';
#else
      return (unsigned char)240;
#endif
#endif
    }
  else	/* Originally set highlight bit, but that is not portable, now use
	   the percent sign instead. */
    {
#ifdef TC_COLOR
      if (cave_ptr->fval == MAGMA_WALL)
        textcolor(DARKGRAY);
      else
        textcolor(WHITE);
      return wallsym;
#else
      return '%';
#endif
    }
}


/* Tests a spot for light or field mark status		-RAK-	*/
int test_light(y, x)
int y, x;
{
  register cave_type *cave_ptr;

  cave_ptr = &cave[y][x];
  if (cave_ptr->pl || cave_ptr->tl || cave_ptr->fm)
    return(TRUE);
  else
    return(FALSE);
}


/* Prints the map of the dungeon			-RAK-	*/
void prt_map()
{
  register int i, j, k;
  register unsigned char tmp_char;

  k = 0;
  for (i = panel_row_min; i <= panel_row_max; i++)  /* Top to bottom */
    {
      k++;
      erase_line (k, 13);

      for (j = panel_col_min; j <= panel_col_max; j++)	/* Left to right */
	{
	  tmp_char = loc_symbol(i, j);
	  if (tmp_char != ' ')
	    print(tmp_char, i, j);
#ifdef TC_COLOR
	  textcolor(LIGHTGRAY); /* clear colors */
#endif	  
	}
    }
}


/* Compact monsters					-RAK-	*/
/* Return TRUE if any monsters were deleted, FALSE if could not delete any
   monsters. */
static int compact_monsters()
{
  register int i;
  int cur_dis, delete_any;
  register monster_type *mon_ptr;

  msg_print("Compacting monsters...");

  cur_dis = 66;
  delete_any = FALSE;
  do
    {
      for (i = mfptr - 1; i >= MIN_MONIX; i--)
	{
	  mon_ptr = &m_list[i];
	  if ((cur_dis < mon_ptr->cdis) && (randint(3) == 1))
	    {
	      /* in case this is called from within creatures(), this is a
		 horrible hack, the m_list/creatures() code needs to be
		 rewritten */
	      if (hack_monptr < i)
		{
		  delete_monster(i);
		  delete_any = TRUE;
		}
	      else
		/* fix1_delete_monster() does not decrement mfptr, so
		   don't set delete_any if this was called */
		fix1_delete_monster(i);
	    }
	}
      if (!delete_any)
	{
	  cur_dis -= 6;
	  /* can't do anything else but abort, if can't delete any monsters */
	  if (cur_dis < 0)
	    return FALSE;
	}
    }
  while (!delete_any);
  return TRUE;
}


/* Add to the players food time				-RAK-	*/
void add_food(num)
int num;
{
  register struct flags *p_ptr;

  p_ptr = &py.flags;
  if (p_ptr->food < 0)	p_ptr->food = 0;
  p_ptr->food += num;
  if (num>0 && p_ptr->food<=0) p_ptr->food = 32000;
  if (p_ptr->food > PLAYER_FOOD_MAX)
    {
      msg_print("You are bloated from overeating.");
      p_ptr->slow += (p_ptr->food - PLAYER_FOOD_MAX) / 50;
      p_ptr->food = PLAYER_FOOD_MAX + p_ptr->slow;
    }
  else if (p_ptr->food > PLAYER_FOOD_FULL)
    msg_print("You are full.");
}


/* Returns a pointer to next free space			-RAK-	*/
int popm()
{
  if (mfptr == MAX_MALLOC)
    compact_monsters();
  return (mfptr++);
}


/* Gives Max hit points					-RAK-	*/
int max_hp(array)
int8u *array;
{
  return((int)(array[0]) * (int)(array[1]));
}


/* Places a monster at given location			-RAK-	*/
void place_monster(y, x, z, slp)
register int y, x, z;
int slp;
{
  register int cur_pos;
  register monster_type *mon_ptr;
  char buf[100];

  if (c_list[z].cdefense & UNIQUE) {
    if (u_list[z].exist) {
      if (wizard) {
	(void) sprintf(buf, "Tried to create %s but exists.", c_list[z].name);
	msg_print(buf);
      }
      return;
    }
    if (u_list[z].dead) {
      if (wizard) {
	(void) sprintf(buf, "Tried to create %s but dead.", c_list[z].name);
	msg_print(buf);
      }
      return;
    }
    u_list[z].exist = 1;
  }

  if ((wizard || peek) && (c_list[z].cdefense & UNIQUE))
    msg_print(c_list[z].name);
  if (c_list[z].level > dun_level) {
    int c;

    rating += ((c=c_list[z].level-dun_level)>30)? 15 : c/2;
    if (c_list[z].cdefense & UNIQUE)
      rating += (c_list[z].level-dun_level)/2;
  }
  cur_pos = popm();
  mon_ptr = &m_list[cur_pos];
  mon_ptr->fy = y;
  mon_ptr->fx = x;
  mon_ptr->mptr = z;
  if ((z<0)||(z>=MAX_CREATURES)) {
#ifndef MSDOS 
      char *wibble; /* Why? -CFT */
      wibble=NULL;
      msg_print("FOUL UP IN PLACE_MONSTER");
      *wibble=3; /* Are you intentionally causing a Seg. Fault to exit? -CFT */
#else
      msg_print("FOUL UP IN PLACE_MONSTER");
#endif
  }
  if ((c_list[z].cdefense & MAX_HP) || be_nasty)
    mon_ptr->hp = max_hp(c_list[z].hd);
  else
    mon_ptr->hp = pdamroll(c_list[z].hd);
  mon_ptr->maxhp=mon_ptr->hp;
  mon_ptr->cspeed = c_list[z].speed - 10 + py.flags.speed;
  mon_ptr->stunned = 0;
  mon_ptr->cdis = distance(char_row, char_col,y,x);
  mon_ptr->ml = FALSE;
  cave[y][x].cptr = cur_pos;
  if (slp)
    {
      if (c_list[z].sleep == 0)
	mon_ptr->csleep = 0;
      else
	mon_ptr->csleep = (c_list[z].sleep * 2) +
	  randint((int)c_list[z].sleep*10);
    }
  else
    mon_ptr->csleep = 0;
}

/* Places a monster at given location			-RAK-	*/
void place_win_monster()
{
  register int y, x, cur_pos;
  register monster_type *mon_ptr;

  if (wizard || peek) msg_print("Placing win monster");
  if (!total_winner) {
    cur_pos = popm();
    mon_ptr = &m_list[cur_pos];
    do {
      y = randint(cur_height-2);
      x = randint(cur_width-2);
    }
    while ((cave[y][x].fval >= MIN_CLOSED_SPACE) || (cave[y][x].cptr != 0)
	   || (cave[y][x].tptr != 0) ||
	   (distance(y,x,char_row, char_col) <= MAX_SIGHT));
    mon_ptr->fy = y;
    mon_ptr->fx = x;
    mon_ptr->mptr = MAX_CREATURES-2;
    if (c_list[mon_ptr->mptr].cdefense & MAX_HP)
      mon_ptr->hp = max_hp(c_list[mon_ptr->mptr].hd);
    else
      mon_ptr->hp = pdamroll(c_list[mon_ptr->mptr].hd);
    /* the c_list speed value is 10 greater, so that it can be a int8u */
    mon_ptr->cspeed = c_list[mon_ptr->mptr].speed - 10 + py.flags.speed;
    mon_ptr->stunned = 0;
    mon_ptr->cdis = distance(char_row, char_col,y,x);
    cave[y][x].cptr = cur_pos;
    mon_ptr->csleep = 0;
  }
}

static char *cap(str)
  char *str;
{
  if ((*str>='a') && (*str<='z'))
    *str = *str-'a'+'A';
  return str;
}

void set_ghost(g, name, r, c, l)
  creature_type *g;
  char *name;
  int r, c, l;
{
  char tmp[100];
  char race[20];
  char class[20];

  switch (r) {
  case 0:
  case 8:
    strcpy(race, "human");
    break;
  case 1:
  case 2:
  case 9:
    strcpy(race, "elf");
    break;
  case 3:
    strcpy(race, "hobbit");
    break;
  case 4:
    strcpy(race, "gnome");
    break;
  case 5:
    strcpy(race, "dwarf");
    break;
  case 6:
    strcpy(race, "orc");
    break;
  case 7:
    strcpy(race, "troll");
    break;
  }
  switch (c) {
  case 0:
    strcpy(class, "warrior");
    break;
  case 1:
    strcpy(class, "mage");
    break;
  case 2:
    strcpy(class, "priest");
    break;
  case 3:
    strcpy(class, "rogue");
    break;
  case 4:
    strcpy(class, "ranger");
    break;
  case 5:
    strcpy(class, "paladin");
    break;
  }
  g->level = l;
  g->sleep = 0;
  g->aaf = 100;
  g->mexp = l*5+5;
  g->spells2 = NONE8;
  if (!dun_level) {
    sprintf(g->name, "%s, the %s %s", cap(name), cap(race), cap(class));
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_90|HAS_60|GOOD);
    if (g->level>10) g->cmove |= (HAS_1D2);
    if (g->level>18) g->cmove |= (HAS_2D2);
    if (g->level>23) g->cmove |= (HAS_4D2);
    switch (c) {
    case 0: /* Warrior */
      g->spells = NONE8;
      break;
    case 1: /* Mage */
      g->spells |= (0x3L|BLINK|MAG_MISS|SLOW|CONFUSION);
      if (l>5) g->spells2 |= ST_CLOUD;
      if (l>7)  g->spells2 |= LIGHT_BOLT;
      if (l>10) g->spells |= FROST_BOLT;
      if (l>12) g->spells |= TELE;
      if (l>15) g->spells |= ACID_BOLT;
      if (l>20) g->spells |= FIRE_BOLT;
      if (l>25) g->spells |= FROST_BALL;
      if (l>25) g->spells2 |= HASTE;
      if (l>30) g->spells |= FIRE_BALL;
      if (l>40) g->spells |= MANA_BOLT;
      break;
    case 3: /* Rogue */
      g->spells |= (0x5L|BLINK);
      if (l>10) g->spells |= CONFUSION;
      if (l>18) g->spells |= SLOW;
      if (l>25) g->spells |= TELE;
      if (l>30) g->spells |= HOLD_PERSON;
      if (l>35) g->spells |= TELE_TO;
      break;
    case 4: /* Ranger */
      g->spells |= (0x8L|MAG_MISS);
      if (l>5) g->spells2 |= ST_CLOUD;
      if (l>7)  g->spells2 |= LIGHT_BOLT;
      if (l>10) g->spells |= FROST_BOLT;
      if (l>18) g->spells |= ACID_BOLT;
      if (l>25) g->spells |= FIRE_BOLT;
      if (l>30) g->spells |= FROST_BALL;
      if (l>35) g->spells |= FIRE_BALL;
      break;
    case 2: /* Priest */
    case 5: /* Paladin */
      g->spells |= (0x4L|CAUSE_LIGHT|FEAR);
      if (l>5) g->spells2 |= HEAL;
      if (l>10) g->spells |= (CAUSE_SERIOUS|BLINDNESS);
      if (l>18) g->spells |= HOLD_PERSON;
      if (l>25) g->spells |= CONFUSION;
      if (l>30) g->spells |= CAUSE_CRIT;
      if (l>35) g->spells |= MANA_DRAIN;
      break;
    }
    g->cdefense |= (CHARM_SLEEP|EVIL);
    if (r==6)
      g->cdefense |= ORC;
    else if (r==7)
      g->cdefense |= TROLL;
    g->ac = 15+randint(15);
    if (c==0 || c>=3)
      g->ac += randint(60);
    if ((c==1 || c==3) && l>25) /* High level mages and rogues are fast... */
      g->speed = 12;
    else
      g->speed = 11;
    g->cchar = 'p';
    g->hd[1] = 1;
    g->damage[0] = 5+((l>18)? 18 : l);
    g->damage[1] = g->damage[0];
    switch (c) {
    case 0:
      g->damage[2] = ((l<30)? (5+((l>18)? 18 : l)) : 235);
      g->damage[3] = g->damage[2];
      break;
    case 1:
    case 2:
      g->damage[2] = 0;
      g->damage[3] = 0;
      break;
    case 3:
      g->damage[2] = g->damage[3] = ((l<30)? 149 : 232);
      break;
    case 5:
    case 4:
      g->damage[2] = g->damage[3] = g->damage[1];
      break;
    }
    return;
  }
  switch ((g->level/4)+randint(3)) {
  case 1:
  case 2:
  case 3:
    sprintf(g->name, "%s, the Skeleton %s", name, race);
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_90);
    g->spells |= (NONE8);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    if (r==6)
      g->cdefense |= ORC;
    else if (r==7)
      g->cdefense |= TROLL;
    g->ac = 26;
    g->speed = 11;
    g->cchar = 's';
    g->hd[1] = 1;
    g->damage[0] = 5;
    g->damage[1] = 5;
    g->damage[2] = 0;
    g->damage[3] = 0;
    break;
  case 4:
  case 5:
    sprintf(g->name, "%s, the %s zombie", name, cap(race));
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_60|HAS_90);
    g->spells |= (NONE8);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    if (r==6)
      g->cdefense |= ORC;
    else if (r==7)
      g->cdefense |= TROLL;
    g->ac = 30;
    g->speed = 11;
    g->cchar = 'z';
    g->hd[1] = 2;
    g->damage[0] = 8;
    g->damage[1] = 0;
    g->damage[2] = 0;
    g->damage[3] = 0;
    break;
  case 6:
    sprintf(g->name, "%s, the Poltergeist", name);
    g->cmove |= (MV_INVIS|MV_ATT_NORM|CARRY_OBJ|HAS_1D2|MV_75|THRO_WALL);
    g->spells |= (NONE8);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    g->ac = 20;
    g->speed = 13;
    g->cchar = 'G';
    g->hd[1] = 1;
    g->damage[0] = 5;
    g->damage[1] = 5;
    g->damage[2] = 93;
    g->damage[3] = 93;
    g->mexp = (g->mexp*3)/2;
    break;
  case 7:
  case 8:
    sprintf(g->name, "%s, the Mummified %s", name, cap(race));
    g->cmove |= (MV_ATT_NORM|CARRY_OBJ|HAS_1D2);
    g->spells |= (NONE8);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    if (r==6)
      g->cdefense |= ORC;
    else if (r==7)
      g->cdefense |= TROLL;
    g->ac = 35;
    g->speed = 11;
    g->cchar = 'M';
    g->hd[1] = 2;
    g->damage[0] = 16;
    g->damage[1] = 16;
    g->damage[2] = 16;
    g->damage[3] = 0;
    g->mexp = (g->mexp*3)/2;
    break;
  case 9:
  case 10:
    sprintf(g->name, "%s%s spirit", name, (name[strlen(name)-1]=='s')?
	    "'":"'s");
    g->cmove |= (MV_INVIS|THRO_WALL|MV_ATT_NORM|CARRY_OBJ|HAS_1D2);
    g->spells |= (NONE8);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    g->ac = 20;
    g->speed = 11;
    g->cchar = 'G';
    g->hd[1] = 2;
    g->damage[0] = 19;
    g->damage[1] = 185;
    g->damage[2] = 99;
    g->damage[3] = 178;
    g->mexp = g->mexp*3;
    break;
  case 11:
    sprintf(g->name, "%s%s ghost", name, (name[strlen(name)-1]=='s')?
	    "'":"'s");
    g->cmove |= (MV_INVIS|THRO_WALL|MV_ATT_NORM|CARRY_OBJ|HAS_1D2);
    g->spells |= (0xFL|HOLD_PERSON|MANA_DRAIN|BLINDNESS);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    g->ac = 40;
    g->speed = 12;
    g->cchar = 'G';
    g->hd[1] = 2;
    g->damage[0] = 99;
    g->damage[1] = 99;
    g->damage[2] = 192;
    g->damage[3] = 184;
    g->mexp = (g->mexp*7)/2;
    break;
  case 12:
    sprintf(g->name, "%s, the Vampire", name);
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_2D2);
    g->spells |= (0x8L|HOLD_PERSON|FEAR|TELE_TO|CAUSE_SERIOUS);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|HURT_LIGHT);
    g->ac = 40;
    g->speed = 11;
    g->cchar = 'V';
    g->hd[1] = 3;
    g->damage[0] = 20;
    g->damage[1] = 20;
    g->damage[2] = 190;
    g->damage[3] = 0;
    g->mexp = g->mexp*3;
    break;
  case 13:
    sprintf(g->name, "%s%s Wraith", name, (name[strlen(name)-1]=='s')?
	    "'":"'s");
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_4D2|HAS_2D2);
    g->spells |= (0x7L|HOLD_PERSON|FEAR|BLINDNESS|CAUSE_CRIT);
    g->spells2 |= (NETHER_BOLT);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|HURT_LIGHT);
    g->ac = 60;
    g->speed = 12;
    g->cchar = 'W';
    g->hd[1] = 3;
    g->damage[0] = 20;
    g->damage[1] = 20;
    g->damage[2] = 190;
    g->damage[3] = 0;
    g->mexp = g->mexp*5;
    break;
  case 14:
    sprintf(g->name, "%s, the Vampire Lord", name);
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_2D2);
    g->spells |= (0x8L|HOLD_PERSON|FEAR|TELE_TO|CAUSE_CRIT);
    g->spells2 |= (NETHER_BOLT);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|HURT_LIGHT);
    g->ac = 80;
    g->speed = 11;
    g->cchar = 'V';
    g->hd[1] = 5;
    g->damage[0] = 20;
    g->damage[1] = 20;
    g->damage[2] = 20;
    g->damage[3] = 198;
    g->mexp = g->mexp*20;
    break;
  case 15:
     sprintf(g->name, "%s%s ghost", name, (name[strlen(name)-1]=='s')?
	    "'":"'s");
    g->cmove |= (MV_INVIS|THRO_WALL|MV_ATT_NORM|CARRY_OBJ|HAS_4D2|HAS_2D2);
    g->spells |= (0x5L|HOLD_PERSON|MANA_DRAIN|BLINDNESS|CONFUSION);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA);
    g->ac = 90;
    g->speed = 13;
    g->cchar = 'G';
    g->hd[1] = 3;
    g->damage[0] = 99;
    g->damage[1] = 99;
    g->damage[2] = 192;
    g->damage[3] = 184;
    g->mexp = g->mexp*20;
    break;
  case 17:
    sprintf(g->name, "%s, the Lich", name);
    g->cmove |= (THRO_DR|MV_ATT_NORM|CARRY_OBJ|HAS_4D2|HAS_2D2|HAS_1D2);
    g->spells |= (0x3L|FEAR|CAUSE_CRIT|TELE_TO|BLINK|S_UNDEAD|FIRE_BALL|
		  FROST_BALL|HOLD_PERSON|MANA_DRAIN|BLINDNESS|CONFUSION|TELE);
    g->spells2 |= (BRAIN_SMASH|RAZOR);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|IM_POISON
		    |INTELLIGENT);
    g->ac = 120;
    g->speed = 12;
    g->cchar = 'L';
    g->hd[1] = 6;
    g->damage[0] = 181;
    g->damage[1] = 201;
    g->damage[2] = 214;
    g->damage[3] = 181;
    g->mexp = g->mexp*50;
    break;
  case 18:
  default:
    sprintf(g->name, "%s%s ghost", name, (name[strlen(name)-1]=='s')?
	    "'":"'s");
    g->cmove |= (MV_INVIS|THRO_WALL|MV_ATT_NORM|CARRY_OBJ|HAS_4D2|HAS_2D2);
    g->spells |= (0x2L|HOLD_PERSON|MANA_DRAIN|BLINDNESS|CONFUSION|TELE_TO);
    g->spells2 |= (NETHER_BOLT|NETHER_BALL|BRAIN_SMASH|TELE_LEV);
    g->cdefense |= (CHARM_SLEEP|UNDEAD|EVIL|IM_FROST|NO_INFRA|INTELLIGENT);
    g->ac = 130;
    g->speed = 13;
    g->cchar = 'G';
    g->hd[1] = 5;
    g->damage[0] = 99;
    g->damage[1] = 99;
    g->damage[2] = 192;
    g->damage[3] = 184;
    g->mexp = g->mexp*30;
    break;
  }
}


/* Places a monster at given location			-RAK-	*/
int place_ghost()
{
  register int y, x, cur_pos;
  register monster_type *mon_ptr;
  creature_type *ghost = &c_list[MAX_CREATURES-1];
  char tmp[100];
  char name[100];
  int i, level;
  int race;
  int cl;

  if (!dun_level) {
    FILE *fp;

    if (py.misc.lev<5 || randint(10)>1) return 0;
    sprintf(tmp, "%s%d", ANGBAND_BONES, py.misc.lev);
    if ((fp = fopen(tmp, "r")) != NULL) {
      if (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &i, &race, &cl)<4) {
	fclose(fp);
	if (wizard)
	  msg_print("Town:Failed to scan in info properly!");
	return 0;
      }
      ghost->hd[0] = i;
      ghost->hd[1] = 1;
      fclose(fp);
      level = py.misc.lev;
    } else {
      return 0;
    }
  } else {
    if (14>randint((dun_level/2)+11)) return 0;
    if (randint(3)==1) {
      FILE *fp;

      sprintf(tmp, "%s%d", ANGBAND_BONES, dun_level);
      if ((fp = fopen(tmp, "r")) != NULL) {
	if (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &i, &race, &cl)<4) {
	  fclose(fp);
	  if (wizard)
	    msg_print("Ghost:Failed to scan in info properly!");
	  return 0;
	}
	fclose(fp);
	ghost->hd[0] = i;
	ghost->hd[1] = 1;
	level = dun_level;
      } else {
	return 0;
      }
    } else {
      return 0;
    }
  }
  set_ghost(ghost, name, race, cl, level);
  if (wizard || peek)
    msg_print(ghost->name);
  cur_pos = popm();
  mon_ptr = &m_list[cur_pos];
  do {
    y = randint(cur_height-2);
    x = randint(cur_width-2);
  } while ((cave[y][x].fval >= MIN_CLOSED_SPACE) || (cave[y][x].cptr != 0)
	   || (cave[y][x].tptr != 0) ||
	   (distance(y,x,char_row, char_col) <= MAX_SIGHT));
  mon_ptr->fy = y;
  mon_ptr->fx = x;
  mon_ptr->mptr = (MAX_CREATURES-1);
  mon_ptr->hp = i*ghost->hd[1];
  /* the c_list speed value is 10 greater, so that it can be a int8u */
  mon_ptr->cspeed = c_list[mon_ptr->mptr].speed - 10 + py.flags.speed;
  mon_ptr->stunned = 0;
  mon_ptr->cdis = distance(char_row, char_col,y,x);
  cave[y][x].cptr = cur_pos;
  mon_ptr->csleep = 0;
  return 1;
}


/* Return a monster suitable to be placed at a given level.  This makes
   high level monsters (up to the given level) slightly more common than
   low level monsters at any given level.   -CJS- */
int get_mons_num (level)
int level;
{
  register int i, j, num;
  int old;

  old=level;
 again:
  if (level == 0)
    i = randint (m_level[0]) - 1;
  else
    {
      if (level > MAX_MONS_LEVEL)
	level = MAX_MONS_LEVEL;
      if (randint (MON_NASTY) == 1)
	{
	  i = randnor (0, 4);
	  level = level + abs(i) + 1;
	  if (level > MAX_MONS_LEVEL)
	    level = MAX_MONS_LEVEL;
	}
      else
	{
	  /* This code has been added to make it slightly more likely to
	     get the higher level monsters. Originally a uniform
	     distribution over all monsters of level less than or equal to the
	     dungeon level. This distribution makes a level n monster occur
	     approx 2/n% of the time on level n, and 1/n*n% are 1st level. */

	  num = m_level[level] - m_level[0];
	  i = randint (num) - 1;
	  j = randint (num) - 1;
	  if (j > i)
	    i = j;
	  level = c_list[i + m_level[0]].level;
	}
      i = m_level[level]-m_level[level-1];
      if (i==0) i++;
      i = randint(i) - 1 + m_level[level-1];
    }
  if ((c_list[i].level>old) && (c_list[i].cdefense&UNIQUE)) goto again;
  if ((c_list[i].level>dun_level) && (c_list[i].cdefense&QUESTOR)) goto again;
  return i;
}

int get_nmons_num (level)
int level;
{
  register int i, j, num;
  int old;

  old=level;
 again:
  if (level == 0)
    i = randint (m_level[0]) - 1;
  else {
    if (level > MAX_MONS_LEVEL)
      level = MAX_MONS_LEVEL;
    num = m_level[level] - m_level[0];
    i = randint (num) - 1;
    i+=15;
    if (i>=num) i=num-1;
    j = randint (num) - 1;
    if (j>i) i=j;
    j = randint (num) - 1;
    if (j>i) i=j;
    level = c_list[i + m_level[0]].level;
    i = m_level[level]-m_level[level-1];
    if (i==0) i=1;
    i = randint(i) - 1 + m_level[level-1];
  }
  if ((c_list[i].level>old) && (c_list[i].cdefense&UNIQUE)) goto again;
  if ((c_list[i].level>dun_level) && (c_list[i].cdefense&QUESTOR)) goto again;
  return i;
}

/* Ludwig's Brainstorm */
int test_place(y,x)
int y,x;
{
  if (cave[y][x].fval >= MIN_CLOSED_SPACE || (cave[y][x].cptr != 0) ||
      y<0 || x<0 || y>(cur_height-2) || x>(cur_width-2) ||
      (y==char_row && x==char_col))
    return 0;
  return 1;
}

void place_group(y,x,mon,slp)
  int y,x,mon,slp;
{
  int old;

  old = rating;

  switch (randint(13)) {
  case 13:
    if (test_place(y,x-2)) place_monster(y,x-2,mon,0);
    rating = old;
  case 12:
    if (test_place(y,x+2)) place_monster(y,x+2,mon,0);
    rating = old;
  case 11:
    if (test_place(y+2,x)) place_monster(y+2,x,mon,0);
    rating = old;
  case 10:
    if (test_place(y-2,x)) place_monster(y-2,x,mon,0);
    rating = old;
  case 9:
    if (test_place(y+1,x+1)) place_monster(y+1,x+1,mon,0);
    rating = old;
  case 8:
    if (test_place(y+1,x-1)) place_monster(y+1,x-1,mon,0);
    rating = old;
  case 7:
    if (test_place(y-1,x-1)) place_monster(y-1,x-1,mon,0);
    rating = old;
  case 6:
    if (test_place(y-1,x+1)) place_monster(y-1,x+1,mon,0);
    rating = old;
  case 5:
    if (test_place(y,x+1)) place_monster(y,x+1,mon,0);
    rating = old;
  case 4:
    if (test_place(y,x-1)) place_monster(y,x-1,mon,0);
    rating = old;
  case 3:
    if (test_place(y+1,x)) place_monster(y+1,x,mon,0);
    rating = old;
  case 2:
    if (test_place(y-1,x)) place_monster(y-1,x,mon,0);
    rating = old;
  case 1:
    if (test_place(y,x)) place_monster(y,x,mon,0);
  }
}


/* Allocates a random monster				-RAK-	*/
void alloc_monster(num, dis, slp)
int num, dis;
int slp;
{
  register int y, x, i;
  int k, mon;

  for (i = 0; i < num; i++)
    {
      do
	{
	  y = randint(cur_height-2);
	  x = randint(cur_width-2);
	}
      while (cave[y][x].fval >= MIN_CLOSED_SPACE || (cave[y][x].cptr != 0) ||
	     (distance(y, x, char_row, char_col) <= dis));
      do {
	mon = get_mons_num(dun_level);
      } while (randint(c_list[mon].rarity)>1);

      if (!(c_list[mon].cdefense & GROUP))
	place_monster(y, x, mon, slp);
      else place_group(y,x,mon,slp);
    }
}


/* Places creature adjacent to given location		-RAK-	*/
int summon_monster(y, x, slp)
int *y, *x;
int slp;
{
  register int i, j, k;
  int l, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = get_mons_num (dun_level + MON_SUMMON_ADJ);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j, k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      if (c_list[l].cdefense & GROUP) place_group(j,k,l,slp);
	      else place_monster(j, k, l, slp);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}


/* Places undead adjacent to given location		-RAK-	*/
int summon_undead(y, x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
    m = randint(l) - 1;
    ctr = 0;
    do {
      if ((c_list[m].cdefense&UNDEAD)&&!(c_list[m].cdefense&UNIQUE)&&
	  (c_list[m].level<dun_level+5)) {
	    ctr = 20;
	    l = 0;
	  }
      else {
	m++;
	if (m > l)
	  ctr = 20;
	else
	  ctr++;
      }
    } while (ctr <= 19);
  } while(l != 0);
  do {
    j = *y - 2 + randint(3);
    k = *x - 2 + randint(3);
    if (in_bounds(j, k)) {
      cave_ptr = &cave[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0)) {
	place_monster(j, k, m, FALSE);
	summon = TRUE;
	i = 9;
	*y = j;
	*x = k;
      }
    }
    i++;
  } while(i <= 9);
  return(summon);
}

/* As for summon undead */
int summon_demon(lev, y, x)
  int lev;
  int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
    m = randint(l) - 1;
    ctr = 0;
    do {
      if (c_list[m].cdefense & DEMON && !(c_list[m].cdefense & UNIQUE) &&
	  (c_list[m].level <= lev)) {
	ctr = 20;
	l = 0;
      } else {
	m++;
	if (m > l)
	  ctr = 20;
	else
	  ctr++;
      }
    } while (ctr <= 19);
  } while(l != 0);
  do {
    j = *y - 2 + randint(3);
    k = *x - 2 + randint(3);
    if (in_bounds(j, k)) {
      cave_ptr = &cave[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0)) {
	place_monster(j, k, m, FALSE);
	summon = TRUE;
	i = 9;
	*y = j;
	*x = k;
      }
    }
    i++;
  } while(i <= 9);
  return(summon);
}

/* As for summon demon:-) ~Ludwig */
int summon_dragon(y, x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
	  if (c_list[m].cdefense & DRAGON && !(c_list[m].cdefense & UNIQUE))
	    {
	      ctr = 20;
	      l	 = 0;
	    }
	  else
	    {
	      m++;
	      if (m > l)
		ctr = 20;
	      else
		ctr++;
	    }
	}
      while (ctr <= 19);
    }
  while(l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j, k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster(j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while(i <= 9);
  return(summon);
}

/* Summon ringwraiths */
int summon_wraith(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
     m = randint(l) - 1;
     ctr = 0;
     do
       {
	 if (c_list[m].cchar == 'W' && (c_list[m].cdefense & UNIQUE))
	   {
	      ctr = 20;
	      l = 0;
	   }
         else
	   {
	      m++;
              if (m > 1)
		ctr = 20;
              else
                ctr++;
           }
       }
     while (ctr <= 19);
    }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* Summon reptiles */
int summon_reptile(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
     m = randint(l) - 1;
     ctr = 0;
     do
       {
         if (c_list[m].cchar == 'R' && !(c_list[m].cdefense & UNIQUE))
	   {
	     ctr = 20;
	     l = 0;
	   }
	 else
	   {
	     m++;
	     if (m > 1)
	       ctr = 20;
	     else
	       ctr++;
	   }
       }
     while (ctr <= 19);
    }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
        }
      i++;
    }
  while (i <= 9);
  return(summon);
}


/* As for summon dragon, but keys on character ~Decado */
int summon_spider(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;
  char buf[20];

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
          if (c_list[m].cchar == 'S' && !(c_list[m].cdefense & UNIQUE))
	     /*   &&
	    (sscanf(c_list[m].name,"%*s %*s %s",buf)?
	     strcasecmp(buf,"scorpion"):TRUE)) */
	    {
	      ctr = 20;
	      l	 = 0;
	    }
	  else
	    {
	      m++;
	      if (m > l)
		ctr = 20;
	      else
		ctr++;
	    }
	}
      while (ctr <= 19);
    }
  while(l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j, k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster(j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while(i <= 9);
  return(summon);
}

/* As for summon dragon, but keys on character ~Decado */
int summon_angel(y,x)
  int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
    m = randint(l) - 1;
    ctr = 0;
    do {
      if (c_list[m].cchar == 'A' && !(c_list[m].cdefense & UNIQUE)) {
	ctr=20;
	l=0;
      } else {
	m++;
	if (m > l)
	  ctr = 20;
	else
	  ctr++;
      }
    } while (ctr <= 19);
  }
  while(l != 0);
  do {
    j = *y - 2 + randint(3);
    k = *x - 2 + randint(3);
    if (in_bounds(j, k)) {
      cave_ptr = &cave[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0)) {
	place_monster(j, k, m, FALSE);
	summon = TRUE;
	i = 9;
	*y = j;
	*x = k;
      }
    }
    i++;
  } while(i <= 9);
  return(summon);
}

/* Summon ants */
int summon_ant(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
     m = randint(l) - 1;
     ctr = 0;
     do
       {
	 if (c_list[m].cchar == 'a' && !(c_list[m].cdefense & UNIQUE))
	   {
	      ctr = 20;
	      l = 0;
	   }
	 else
	   {
	      m++;
	      if (m > 1)
		ctr = 20;
	      else
		ctr++;
	   }
       }
     while (ctr <= 19);
    }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* Summon uniques */
int summon_unique(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
	  if (!(c_list[m].cchar == 'P') && (c_list[m].cdefense & UNIQUE))
	    {
	      ctr = 20;
	      l = 0;
	    }
	  else
	    {
	      m++;
	      if (m > 1)
	        ctr = 20;
	      else
	        ctr++;
	    }
	}
      while (ctr <= 19);
     }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* Summon jabberwocks, for extra effect to the summon_unique spell */
int summon_jabberwock(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
	  if (c_list[m].cchar == 'J' && !(c_list[m].cdefense & UNIQUE))
	    {
	      ctr = 20;
	      l = 0;
	    }
	  else
	    {
	      m++;
	      if (m > 1)
	        ctr = 20;
	      else
	        ctr++;
	    }
	}
      while (ctr <= 19);
     }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* Summon greater undead */
int summon_gundead(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
        {
	   if ((c_list[m].cchar == 'L') || (c_list[m].cchar == 'V')
	       || (c_list[m].cchar == 'W'))
	     {
	       ctr = 20;
	       l = 0;
	     }
	   else
	     {
	       m++;
	       if (m > 1)
		 ctr = 20;
	       else
		 ctr++;
	     }
	}
      while (ctr <= 19);
    }
  while (l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* Summon ancient dragons */
int summon_ancientd(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
	  if (c_list[m].cchar == 'D')
	    {
	      ctr = 20;
	      l = 0;
	    }
	  else
	    {
	      m++;
	      if (m > 1)
		ctr = 20;
	      else
		ctr++;
	    }
	}
      while (ctr <= 19);
    }
  while (l !=0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j,k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster (j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while (i <= 9);
  return(summon);
}

/* As for summon hound, but keys on character ~Decado */
int summon_hound(y,x)
int *y, *x;
{
  register int i, j, k;
  int l, m, ctr, summon;
  register cave_type *cave_ptr;

  i = 0;
  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do
    {
      m = randint(l) - 1;
      ctr = 0;
      do
	{
          if ((c_list[m].cchar=='C' || c_list[m].cchar=='Z')
	      && !(c_list[m].cdefense & UNIQUE))
	    {
	      ctr = 20;
	      l	 = 0;
	    }
	  else
	    {
	      m++;
	      if (m > l)
		ctr = 20;
	      else
		ctr++;
	    }
	}
      while (ctr <= 19);
    }
  while(l != 0);
  do
    {
      j = *y - 2 + randint(3);
      k = *x - 2 + randint(3);
      if (in_bounds(j, k))
	{
	  cave_ptr = &cave[j][k];
	  if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->cptr == 0))
	    {
	      place_monster(j, k, m, FALSE);
	      summon = TRUE;
	      i = 9;
	      *y = j;
	      *x = k;
	    }
	}
      i++;
    }
  while(i <= 9);
  return(summon);
}

/* Place a sleepy jelly at the required coordinates ~Ludwig */
int summon_jelly(y, x)
int *y, *x;
{
  int l, m, summon;

  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
      m = randint(l) - 1;
      if (c_list[m].cchar == 'J') {
	summon = TRUE;
	place_monster(*y, *x, m, TRUE);
      }
    } while (!summon);
  return(summon);
}

/* If too many objects on floor level, delete some of them */
static void compact_objects()
{
  register int i, j;
  int ctr, cur_dis, chance;
  register cave_type *cave_ptr;

  msg_print("Compacting objects...");

  ctr = 0;
  cur_dis = 66;
  do
    {
      for (i = 0; i < cur_height; i++)
	for (j = 0; j < cur_width; j++)
	  {
	    cave_ptr = &cave[i][j];
	    if ((cave_ptr->tptr != 0)
		&& (distance(i, j, char_row, char_col) > cur_dis))
	      {
		switch(t_list[cave_ptr->tptr].tval)
		  {
		  case TV_VIS_TRAP:
		    chance = 15;
		    break;
		  case TV_INVIS_TRAP:
		  case TV_RUBBLE:
		  case TV_OPEN_DOOR: case TV_CLOSED_DOOR:
		    chance = 5;
		    break;
		  case TV_UP_STAIR:
		  case TV_DOWN_STAIR:
		  case TV_STORE_DOOR:
		    chance = 0;
		    break;
		  case TV_SECRET_DOOR: /* secret doors */
		    chance = 3;
		    break;
		  default:
		    if ((t_list[cave_ptr->tptr].tval >= TV_MIN_WEAR) &&
		    	(t_list[cave_ptr->tptr].tval <= TV_MAX_WEAR) &&
		    	(t_list[cave_ptr->tptr].flags2 & TR_ARTIFACT))
		      chance = 0; /* don't compact artifacts -CFT */
		    else
		      chance = 10;
		  }
		if (randint (100) <= chance)
		  {
		    (void) delete_object(i, j);
		    ctr++;
		  }
	      }
	  }
      if (ctr == 0)  cur_dis -= 6;
    }
  while (ctr <= 0);
  if (cur_dis < 66)  prt_map();
}

/* Gives pointer to next free space			-RAK-	*/
int popt()
{
  if (tcptr == MAX_TALLOC)
    compact_objects();
  return (tcptr++);
}


/* Pushs a record back onto free space list		-RAK-	*/
/* Delete_object() should always be called instead, unless the object in
   question is not in the dungeon, e.g. in store1.c and files.c */
void pusht(x)
register int8u x;
{
  register int i, j;

  if (x != tcptr - 1)
    {
      t_list[x] = t_list[tcptr - 1];

      /* must change the tptr in the cave of the object just moved */
      for (i = 0; i < cur_height; i++)
	for (j = 0; j < cur_width; j++)
	  if (cave[i][j].tptr == tcptr - 1)
	    cave[i][j].tptr = x;
    }
  tcptr--;
  invcopy(&t_list[tcptr], OBJ_NOTHING);
}


/* Boolean : is object enchanted	  -RAK- */
int magik(chance)
int chance;
{
  if (randint(100) <= chance)
    return(TRUE);
  else
    return(FALSE);
}


/* Enchant a bonus based on degree desired -RAK- */
int m_bonus(base, max_std, level)
int base, max_std, level;
{
  register int x, stand_dev, tmp;

  stand_dev = (OBJ_STD_ADJ * level / 100) + OBJ_STD_MIN;
  if (stand_dev > max_std)
    stand_dev = max_std;
  /* abs may be a macro, don't call it with randnor as a parameter */
  tmp = randnor(0, stand_dev);
  x = (abs(tmp) / 10) + base;
  if (x < base)
    return(base);
  else
    return(x);
}

int unique_weapon(t_ptr)
inven_type *t_ptr;
{
  char *name;

  if (be_nasty) return 0;
  name = object_list[t_ptr->index].name;
  if (!strcmp("& Longsword", name)) {
    switch (randint(15)) {
    case 1:
      if (RINGIL) return 0;
      if (wizard || peek) msg_print("Ringil");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_RINGIL;
      t_ptr->tohit = 22;
      t_ptr->todam = 25;
      t_ptr->damage[0] = 4;
      t_ptr->flags = (TR_SEE_INVIS|TR_SLAY_UNDEAD|TR_SLAY_EVIL|TR_REGEN|
		      TR_SPEED|TR_RES_COLD|TR_FROST_BRAND|TR_FREE_ACT|
		      TR_SLOW_DIGEST);
      t_ptr->flags2|= (TR_SLAY_DEMON|TR_SLAY_TROLL|TR_LIGHT|TR_ACTIVATE
		      |TR_ARTIFACT|TR_RES_LT);
      t_ptr->p1    = 1;
      t_ptr->cost  = 300000L;
      RINGIL = 1;
      return 1;
    case 2:
    case 3:
    case 4:
      if (ANDURIL) return 0;
      if (wizard || peek) msg_print("Anduril");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ANDURIL;
      t_ptr->tohit = 10;
      t_ptr->todam = 15;
      t_ptr->flags = (TR_SEE_INVIS|TR_SLAY_EVIL|TR_FREE_ACT|
		      TR_SUST_STAT|TR_STR|TR_RES_FIRE|TR_FLAME_TONGUE);
      t_ptr->flags2|= (TR_SLAY_TROLL|TR_ACTIVATE|TR_SLAY_ORC|TR_ARTIFACT);
      t_ptr->p1    = 4;
      t_ptr->toac    = 5;
      t_ptr->cost  = 80000L;
      ANDURIL = 1;
      return 1;
    case 5:
    case 6:
    case 7:
    case 8:
      if (ANGUIREL) return 0;
      if (wizard || peek) msg_print("Anguirel");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ANGUIREL;
      t_ptr->tohit = 8;
      t_ptr->todam = 12;
      t_ptr->flags = (TR_SEE_INVIS|TR_SLAY_EVIL|TR_FREE_ACT|TR_RES_LIGHT
			|TR_STR|TR_CON);
      t_ptr->flags2 |= (TR_ARTIFACT|
		TR_LIGHTNING|TR_LIGHT|TR_SLAY_DEMON|TR_RES_LT);
      t_ptr->p1 = 2;
      t_ptr->cost = 40000L;
      ANGUIREL = 1;
      return 1;
    default:
      if (ELVAGIL) return 0;
      if (wizard || peek) msg_print("Elvagil");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ELVAGIL;
      t_ptr->tohit = 2;
      t_ptr->todam = 7;
      t_ptr->flags |= (TR_SEE_INVIS|TR_CHR|TR_DEX|TR_STEALTH|TR_FFALL);
      t_ptr->flags2|= (TR_SLAY_TROLL|TR_SLAY_ORC|TR_ARTIFACT);
      t_ptr->p1    = 2;
      t_ptr->cost  = 30000L;
      ELVAGIL = 1;
      return 1;
    }
  }
  else if (!strcmp("& Two-Handed Sword", name)) {
    switch (randint(8)) {
    case 1:
    case 2:
      if (GURTHANG) return 0;
      if (wizard || peek) msg_print("Gurthang");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_GURTHANG;
      t_ptr->tohit = 13;
      t_ptr->todam = 17;
      t_ptr->flags = (TR_REGEN|TR_SLAY_X_DRAGON|TR_STR|
		      TR_FREE_ACT|TR_SLOW_DIGEST);
      t_ptr->flags2 |= (TR_SLAY_TROLL|TR_ARTIFACT);
      t_ptr->p1    = 2;
      t_ptr->cost  = 100000L;
      GURTHANG= 1;
      return 1;
    case 3:
      if (ZARCUTHRA) return 0;
      if (randint(3)>1) return 0;
      if (wizard || peek) msg_print("Zarcuthra");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ZARCUTHRA;
      t_ptr->tohit = 19;
      t_ptr->todam = 21;
      t_ptr->flags = (TR_SLAY_X_DRAGON|TR_STR|TR_SLAY_EVIL|TR_SLAY_ANIMAL|
		      TR_SLAY_UNDEAD|TR_AGGRAVATE|TR_CHR|TR_FLAME_TONGUE|
		      TR_RES_FIRE|TR_FREE_ACT|TR_INFRA);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_TROLL|TR_SLAY_ORC|TR_SLAY_GIANT
		|TR_SLAY_DEMON|TR_RES_CHAOS);
      t_ptr->p1 = 4;
      t_ptr->damage[0] = 6;
      t_ptr->damage[1] = 4;
      t_ptr->cost = 200000L;
      ZARCUTHRA = 1;
      return 1;
    default:
      if (MORMEGIL) return 0;
      if (wizard || peek) msg_print("Mormegil");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_MORMEGIL;
      t_ptr->tohit = -40;
      t_ptr->todam = -60;
      t_ptr->flags = (TR_SPEED|TR_AGGRAVATE|TR_CURSED);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->p1    = -1;
      t_ptr->toac  = -50;
      t_ptr->cost  = 10000L;
      MORMEGIL = 1;
      return 1;
    }
  }
  else if (!strcmp("& Broadsword", name)) {
    switch (randint(12)) {
    case 1:
    case 2:
      if (ARUNRUTH) return 0;
      if (wizard || peek) msg_print("Arunruth");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ARUNRUTH;
      t_ptr->tohit = 20;
      t_ptr->todam = 12;
      t_ptr->damage[0] = 3;
      t_ptr->flags = (TR_FFALL|TR_DEX|
          	      TR_FREE_ACT|TR_SLOW_DIGEST);
      t_ptr->flags2 |= (TR_SLAY_DEMON|TR_SLAY_ORC|TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->p1    = 4;
      t_ptr->cost  = 50000L;
      ARUNRUTH = 1;
      return 1;
    case 3:
    case 4:
    case 5:
    case 6:
      if (GLAMDRING) return 0;
      if (wizard || peek) msg_print("Glamdring");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_GLAMDRING;
      t_ptr->tohit = 10;
      t_ptr->todam = 15;
      t_ptr->flags = (TR_SLAY_EVIL|TR_SLOW_DIGEST|TR_SEARCH|TR_FLAME_TONGUE|
		      TR_RES_FIRE);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_ORC|TR_LIGHT|TR_RES_LT);
      t_ptr->cost  = 40000L;
      GLAMDRING = 1;
      return 1;
    case 7:
      if (AEGLIN) return 0;
      if (wizard || peek) msg_print("Aeglin");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_AEGLIN;
      t_ptr->tohit = 12;
      t_ptr->todam = 16;
      t_ptr->flags = (TR_SLOW_DIGEST|TR_SEARCH|TR_RES_LIGHT);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_ORC|TR_LIGHT|TR_LIGHTNING);
      t_ptr->cost  = 45000L;
      AEGLIN = 1;
      return 1;
    default:
      if (ORCRIST) return 0;
      if (wizard || peek) msg_print("Orcrist");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ORCRIST;
      t_ptr->tohit = 10;
      t_ptr->todam = 15;
      t_ptr->flags = (TR_SLAY_EVIL|TR_SLOW_DIGEST|TR_STEALTH|TR_FROST_BRAND|
		      TR_RES_COLD);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_ORC|TR_LIGHT);
      t_ptr->p1    = 3;
      t_ptr->cost  = 40000L;
      ORCRIST = 1;
      return 1;
    }
  }
  else if (!strcmp("& Bastard Sword", name)) {
    if (CALRIS) return 0;
    if (wizard || peek) msg_print("Calris");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_CALRIS;
    t_ptr->tohit = -20;
    t_ptr->todam = 20;
    t_ptr->damage[0] = 3;
    t_ptr->damage[1] = 7;
    t_ptr->flags = (TR_SLAY_X_DRAGON|TR_CON|TR_AGGRAVATE|
		    TR_CURSED|TR_SLAY_EVIL);
    t_ptr->flags2 |= (TR_SLAY_DEMON|TR_SLAY_TROLL|TR_RES_DISENCHANT|TR_ARTIFACT);
    t_ptr->p1    = 5;
    t_ptr->cost  = 100000L;
    CALRIS = 1;
    return 1;
  }
  else if (!strcmp("& Main Gauche", name)) {
    if (randint(4)>1) return 0;
    if (MAEDHROS) return 0;
    if (wizard || peek) msg_print("Maedhros");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_MAEDHROS;
    t_ptr->tohit = 12;
    t_ptr->todam = 15;
    t_ptr->damage[0] = 2;
    t_ptr->damage[1] = 6;
    t_ptr->flags = (TR_DEX|TR_INT|TR_FREE_ACT|TR_SEE_INVIS);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_GIANT|TR_SLAY_TROLL);
    t_ptr->p1 = 3;
    t_ptr->cost = 20000L;
    MAEDHROS = 1;
    return 1;
  }
  else if (!strcmp("& Glaive", name)) {
    if (randint(3)>1) return 0;
    if (PAIN) return 0;
    if (wizard || peek) msg_print("Pain!");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_PAIN;
    t_ptr->tohit = 0;
    t_ptr->todam = 30;
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->damage[0] = 10;
    t_ptr->damage[1] = 6;
    t_ptr->cost  = 50000L;
    PAIN = 1;
    return 1;
  }
  else if (!strcmp("& Halberd", name)) {
    if (OSONDIR) return 0;
    if (wizard || peek) msg_print("Osondir");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_OSONDIR;
    t_ptr->tohit = 6;
    t_ptr->todam = 9;
    t_ptr->flags = (TR_FLAME_TONGUE|TR_SLAY_UNDEAD|TR_RES_FIRE|TR_SEE_INVIS|
				TR_FFALL|TR_CHR);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_RES_SOUND|TR_SLAY_GIANT);
    t_ptr->p1 = 3;
    t_ptr->cost = 22000L;
    OSONDIR = 1;
    return 1;
  }
  else if (!strcmp("& Lucerne Hammer", name)) {
    if (randint(2)>1) return 0;
    if (TURMIL) return 0;
    if (wizard || peek) msg_print("Turmil");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_TURMIL;
    t_ptr->tohit = 10;
    t_ptr->todam = 6;
    t_ptr->flags = (TR_WIS|TR_REGEN|TR_FROST_BRAND|TR_RES_COLD|TR_INFRA);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_ORC|TR_LIGHT|TR_ACTIVATE|TR_RES_LT);
    t_ptr->p1 = 4;
    t_ptr->cost = 30000L;
    t_ptr->toac = 8;
    TURMIL = 1;
    return 1;
  }
  else if (!strcmp("& Pike", name)) {
    if (randint(2)>1) return 0;
    if (TIL) return 0;
    if (wizard || peek) msg_print("Til-i-arc");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_TIL;
    t_ptr->tohit = 10;
    t_ptr->todam = 12;
    t_ptr->toac = 10;
    t_ptr->flags = (TR_FROST_BRAND|TR_FLAME_TONGUE|TR_RES_FIRE|TR_RES_COLD|
			TR_SLOW_DIGEST|TR_INT|TR_SUST_STAT);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_DEMON|TR_SLAY_GIANT|TR_SLAY_TROLL);
    t_ptr->p1 = 2;
    t_ptr->cost = 32000L;
    TIL = 1;
    return 1;
  }
  else if (!strcmp("& Mace of Disruption", name)) {
    if (randint(5)>1) return 0;
    if (DEATHWREAKER) return 0;
    if (wizard || peek) msg_print("Deathwreaker");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_DEATHWREAKER;
    t_ptr->tohit = 18;
    t_ptr->todam = 18;
    t_ptr->damage[1] = 12;
    t_ptr->flags = (TR_STR|TR_FLAME_TONGUE|TR_SLAY_EVIL|TR_SLAY_DRAGON|
		    TR_SLAY_ANIMAL|TR_TUNNEL|TR_AGGRAVATE);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_IM_FIRE|TR_RES_CHAOS
		|TR_RES_DISENCHANT|TR_RES_DARK);
    t_ptr->p1 = 6;
    t_ptr->cost = 400000L;
    DEATHWREAKER = 1;
    return 1;
  }
  else if (!strcmp("& Scythe", name)) {
    if (AVAVIR) return 0;
    if (wizard || peek) msg_print("Avavir");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_AVAVIR;
    t_ptr->tohit = 8;
    t_ptr->todam = 8;
    t_ptr->toac = 10;
    t_ptr->flags = (TR_DEX|TR_CHR|TR_FREE_ACT|TR_RES_FIRE|TR_RES_COLD|
		    TR_SEE_INVIS|TR_FLAME_TONGUE|TR_FROST_BRAND);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_LIGHT|TR_ACTIVATE|TR_RES_LT);
    t_ptr->p1 = 3;
    t_ptr->cost = 18000L;
    AVAVIR = 1;
    return 1;
  }
  else if (!strcmp("& Mace", name)) {
    if (randint(2)>1) return 0;
    if (TARATOL) return 0;
    if (wizard || peek) msg_print("Taratol");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_TARATOL;
    t_ptr->tohit = 12;
    t_ptr->todam = 12;
    t_ptr->weight = 200;
    t_ptr->damage[1] = 7;
    t_ptr->flags = (TR_SLAY_X_DRAGON|TR_RES_LIGHT);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_LIGHTNING|TR_ACTIVATE|TR_RES_DARK);
    t_ptr->cost = 20000L;
    TARATOL = 1;
    return 1;
  }
  else if (!strcmp("& Lance", name)) {
    if (randint(3)>1) return 0;
    if (EORLINGAS) return 0;
    if (wizard || peek) msg_print("Eorlingas");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_EORLINGAS;
    t_ptr->tohit = 3;
    t_ptr->todam = 21;
    t_ptr->weight = 360;
    t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_DEX);
    t_ptr->flags2 |= (TR_SLAY_TROLL|TR_SLAY_ORC|TR_ARTIFACT);
    t_ptr->p1 = 2;
    t_ptr->damage[1] = 12;
    t_ptr->cost  = 55000L;
    EORLINGAS = 1;
    return 1;
  }
  else if (!strcmp("& Broad Axe", name)) {
    if (BARUKKHELED) return 0;
    if (wizard || peek) msg_print("Barukkheled");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_BARUKKHELED;
    t_ptr->tohit = 13;
    t_ptr->todam = 19;
    t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_CON);
    t_ptr->flags2 |= (TR_SLAY_ORC|TR_SLAY_TROLL|TR_SLAY_GIANT|TR_ARTIFACT);
    t_ptr->p1 = 3;
    t_ptr->cost  = 50000L;
    BARUKKHELED = 1;
    return 1;
  }
  else if (!strcmp("& Trident", name)) {
    switch (randint(3)) {
    case 1:
    case 2:
      if (randint(3)>1) return 0;
      if (WRATH) return 0;
      if (wizard || peek) msg_print("Wrath");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_WRATH;
      t_ptr->tohit = 16;
      t_ptr->todam = 18;
      t_ptr->weight = 300;
      t_ptr->damage[0] = 3;
      t_ptr->damage[1] = 9;
      t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_STR|TR_DEX|
				TR_SLAY_UNDEAD);
      t_ptr->flags2 |= (TR_RES_DARK|TR_RES_LT|TR_ARTIFACT);
      t_ptr->p1 = 2;
      t_ptr->cost  = 90000L;
      WRATH = 1;
      return 1;
    case 3:
      if (randint(4)>1) return 0;
      if (ULMO) return 0;
      if (wizard || peek) msg_print("Ulmo");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ULMO;
      t_ptr->tohit = 15;
      t_ptr->todam = 19;
      t_ptr->damage[0] = 4;
      t_ptr->damage[1] = 10;
      t_ptr->flags = (TR_SEE_INVIS|TR_FREE_ACT|TR_DEX|TR_REGEN|TR_SLOW_DIGEST
			|TR_SLAY_ANIMAL|TR_SLAY_DRAGON);
      t_ptr->flags2 |= (TR_IM_ACID|TR_HOLD_LIFE|TR_ACTIVATE
		|TR_RES_NETHER|TR_ARTIFACT);
      t_ptr->p1 = 4;
      t_ptr->cost = 120000L;
      ULMO = 1;
      return 1;
    }
  }
  else if (!strcmp("& Scimitar", name)) {
    if (HARADEKKET) return 0;
    if (wizard || peek) msg_print("Haradekket");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_HARADEKKET;
    t_ptr->tohit = 9;
    t_ptr->todam = 11;
    t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_DEX|TR_SLAY_UNDEAD
		     |TR_SLAY_ANIMAL);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->p1 = 2;
    t_ptr->cost  = 20000L;
    HARADEKKET = 1;
    return 1;
  }
  else if (!strcmp("& Lochaber Axe", name)) {
    if (MUNDWINE) return 0;
    if (wizard || peek) msg_print("Mundwine");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_MUNDWINE;
    t_ptr->tohit = 12;
    t_ptr->todam = 17;
    t_ptr->flags |= (TR_SLAY_EVIL|TR_RES_FIRE|TR_RES_COLD
		     |TR_RES_LIGHT|TR_RES_ACID);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->cost  = 30000L;
    MUNDWINE= 1;
    return 1;
  }
  else if (!strcmp("& Cutlass", name)) {
    if (GONDRICAM) return 0;
    if (wizard || peek) msg_print("Gondricam");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_GONDRICAM;
    t_ptr->tohit = 10;
    t_ptr->todam = 11;
    t_ptr->flags |= (TR_SEE_INVIS|TR_FFALL|TR_REGEN|TR_STEALTH|TR_RES_FIRE|
		     TR_RES_COLD|TR_RES_ACID|TR_RES_LIGHT|TR_DEX);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->p1 = 3;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->cost  = 28000L;
    GONDRICAM = 1;
    return 1;
  }
  else if (!strcmp("& Sabre", name)) {
    if (CARETH) return 0;
    if (wizard || peek) msg_print("Careth Asdriag");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_CARETH;
    t_ptr->tohit = 6;
    t_ptr->todam = 8;
    t_ptr->flags |= (TR_SLAY_DRAGON|TR_SLAY_ANIMAL);
    t_ptr->flags2 |= (TR_SLAY_GIANT|TR_SLAY_ORC|TR_SLAY_TROLL|TR_ARTIFACT);
    t_ptr->cost  = 20000L;
    CARETH = 1;
    return 1;
  }
  else if (!strcmp("& Rapier", name)) {
    if (FORASGIL) return 0;
    if (wizard || peek) msg_print("Forasgil");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_FORASGIL;
    t_ptr->tohit = 12;
    t_ptr->todam = 19;
    t_ptr->flags |= (TR_RES_COLD|TR_FROST_BRAND|TR_SLAY_ANIMAL);
    t_ptr->flags2 |= (TR_LIGHT|TR_RES_LT|TR_ARTIFACT);
    t_ptr->cost  = 15000L;
    FORASGIL = 1;
    return 1;
  }
  else if (!strcmp("& Executioner's Sword", name)) {
    if (randint(2)>1) return 0;
    if (CRISDURIAN) return 0;
    if (wizard || peek) msg_print("Crisdurian");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_CRISDURIAN;
    t_ptr->tohit = 18;
    t_ptr->todam = 19;
    t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_SLAY_UNDEAD|TR_SLAY_DRAGON);
    t_ptr->flags2 |= (TR_SLAY_GIANT|TR_SLAY_ORC|TR_SLAY_TROLL|TR_ARTIFACT);
    t_ptr->cost  = 100000L;
    CRISDURIAN = 1;
    return 1;
  }
  else if (!strcmp("& Flail", name)) {
    if (TOTILA) return 0;
    if (wizard || peek) msg_print("Totila");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_TOTILA;
    t_ptr->tohit = 6;
    t_ptr->todam = 8;
    t_ptr->damage[1] = 9;
    t_ptr->flags = (TR_STEALTH|TR_RES_FIRE|TR_FLAME_TONGUE|TR_SLAY_EVIL);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_ACTIVATE|TR_RES_CONF);
    t_ptr->p1    = 2;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->cost  = 55000L;
    TOTILA = 1;
    return 1;
  }
  else if (!strcmp("& Short sword", name)) {
    if (GILETTAR) return 0;
    if (wizard || peek) msg_print("Gilettar");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_GILETTAR;
    t_ptr->tohit = 3;
    t_ptr->todam = 7;
    t_ptr->flags = (TR_REGEN|TR_SLOW_DIGEST|TR_SLAY_ANIMAL);
    t_ptr->flags = (TR_ARTIFACT);
    t_ptr->cost  = 10000L;
    GILETTAR = 1;
    return 1;
  }
  else if (!strcmp("& Katana", name)) {
    if (randint(3)>1) return 0;
    if (AGLARANG) return 0;
    if (wizard || peek) msg_print("Aglarang");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_AGLARANG;
    t_ptr->tohit = 0;
    t_ptr->todam = 0;
    t_ptr->damage[0] = 6;
    t_ptr->damage[1] = 8;
    t_ptr->weight = 50;
    t_ptr->flags = (TR_DEX|TR_SUST_STAT);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->p1    = 5;
    t_ptr->cost  = 40000L;
    AGLARANG = 1;
    return 1;
  }
  else if (!strcmp("& Spear", name)) {
    switch (randint(6)) {
    case 1:
      if (AEGLOS) return 0;
      if (wizard || peek) msg_print("Aeglos");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_AEGLOS;
      t_ptr->tohit = 15;
      t_ptr->todam = 25;
      t_ptr->damage[0] = 1;
      t_ptr->damage[1] = 20;
      t_ptr->flags = (TR_WIS|TR_FROST_BRAND|
		      TR_RES_COLD|TR_FREE_ACT|TR_SLOW_DIGEST);
      t_ptr->flags2 |= (TR_SLAY_TROLL|TR_SLAY_ORC|TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->toac  = 5;
      t_ptr->p1    = 4;
      t_ptr->cost  = 140000L;
      AEGLOS = 1;
      return 1;
    case 2:
    case 3:
    case 4:
    case 5:
      if (NIMLOTH) return 0;
      if (wizard || peek) msg_print("Nimloth");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_NIMLOTH;
      t_ptr->tohit = 11;
      t_ptr->todam = 13;
      t_ptr->flags = (TR_FROST_BRAND|TR_RES_COLD|TR_SLAY_UNDEAD|TR_STEALTH);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->p1    = 3;
      t_ptr->cost  = 30000L;
      NIMLOTH = 1;
      return 1;
    case 6:
      if (OROME) return 0;
      if (wizard || peek) msg_print("Orome");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_OROME;
      t_ptr->tohit = 15;
      t_ptr->todam = 15;
      t_ptr->flags = (TR_FLAME_TONGUE|TR_SEE_INVIS|TR_SEARCH|TR_INT|
		      TR_RES_FIRE|TR_FFALL|TR_INFRA);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_LIGHT|TR_SLAY_GIANT|TR_RES_LT
		|TR_ARTIFACT);
      t_ptr->p1 = 4;
      t_ptr->cost = 60000L;
      OROME = 1;
      return 1;
    }
  }
  else if (!strcmp("& Dagger", name)) {
    switch (randint(11)) {
    case 1:
      if (ANGRIST) return 0;
      if (wizard || peek) msg_print("Angrist");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ANGRIST;
      t_ptr->tohit = 10;
      t_ptr->todam = 15;
      t_ptr->damage[0] = 2;
      t_ptr->damage[1] = 5;
      t_ptr->flags = (TR_DEX|TR_SLAY_EVIL|TR_SUST_STAT|
		      TR_FREE_ACT);
      t_ptr->flags2 |= (TR_SLAY_TROLL|TR_SLAY_ORC|TR_RES_DARK|TR_ARTIFACT);
      t_ptr->toac  = 5;
      t_ptr->p1    = 4;
      t_ptr->cost  = 100000L;
      ANGRIST = 1;
      return 1;
    case 2:
    case 3:
      if (NARTHANC) return 0;
      if (wizard || peek) msg_print("Narthanc");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_NARTHANC;
      t_ptr->tohit = 4;
      t_ptr->todam = 6;
      t_ptr->flags = (TR_FLAME_TONGUE|TR_RES_FIRE);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->cost  = 12000L;
      NARTHANC = 1;
      return 1;
    case 4:
    case 5:
      if (NIMTHANC) return 0;
      if (wizard || peek) msg_print("Nimthanc");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_NIMTHANC;
      t_ptr->tohit = 4;
      t_ptr->todam = 6;
      t_ptr->flags = (TR_FROST_BRAND|TR_RES_COLD);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->cost  = 11000L;
      NIMTHANC = 1;
      return 1;
    case 6:
    case 7:
      if (DETHANC) return 0;
      if (wizard || peek) msg_print("Dethanc");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_DETHANC;
      t_ptr->tohit = 4;
      t_ptr->todam = 6;
      t_ptr->flags = (TR_RES_LIGHT);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_LIGHTNING|TR_ARTIFACT);
      t_ptr->cost  = 13000L;
      DETHANC = 1;
      return 1;
    case 8:
    case 9:
      if (RILIA) return 0;
      if (wizard || peek) msg_print("Rilia");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_RILIA;
      t_ptr->tohit = 4;
      t_ptr->todam = 3;
      t_ptr->damage[0] = 2;
      t_ptr->damage[1] = 4;
      t_ptr->flags2 |= (TR_ACTIVATE|TR_RES_DISENCHANT|TR_ARTIFACT);
      t_ptr->cost  = 15000L;
      RILIA = 1;
      return 1;
    case 10:
    case 11:
      if (BELANGIL) return 0;
      if (wizard || peek) msg_print("Belangil");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_BELANGIL;
      t_ptr->tohit = 6;
      t_ptr->todam = 9;
      t_ptr->damage[0] = 3;
      t_ptr->damage[1] = 2;
      t_ptr->flags = (TR_FROST_BRAND|TR_RES_COLD|TR_REGEN|TR_SLOW_DIGEST|
		      TR_DEX|TR_SEE_INVIS);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->p1    = 2;
      t_ptr->cost  = 40000L;
      BELANGIL = 1;
      return 1;
    }
  } else if (!strcmp("& Small sword", name)) {
    if (STING) return 0;
    if (wizard || peek) msg_print("Sting");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_STING;
    t_ptr->tohit = 7;
    t_ptr->todam = 8;
    t_ptr->flags = (TR_SEE_INVIS|TR_DEX);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_ORC|TR_LIGHT|TR_RES_LT);
    t_ptr->p1    = 2;
    t_ptr->cost  = 12000L;
    STING = 1;
    return 1;
  } else if (!strcmp("& Great Axe", name)) {
      switch (randint(2)) {
      case 1:
        if (randint(6)>1) return 0;
        if (DURIN) return 0;
        if (wizard || peek) msg_print("Durin");
        else good_item_flag = TRUE;
        t_ptr->name2 = SN_DURIN;
        t_ptr->tohit = 10;
        t_ptr->todam = 20;
        t_ptr->toac  = 15;
        t_ptr->flags = (TR_SLAY_X_DRAGON|TR_CON|TR_FREE_ACT|
                	    TR_RES_FIRE|TR_RES_ACID);
        t_ptr->flags2 |= (TR_SLAY_DEMON|TR_SLAY_TROLL|TR_SLAY_ORC|TR_RES_DARK
			|TR_RES_LT|TR_RES_CHAOS|TR_ARTIFACT);
        t_ptr->p1    = 3;
        t_ptr->cost  = 150000L;
        DURIN = 1;
        return 1;
      case 2:
	if (randint(8)>1) return 0;
	if (EONWE) return 0;
        if (wizard || peek) msg_print("Eonwe");
        else good_item_flag = TRUE;
        t_ptr->name2 = SN_EONWE;
        t_ptr->tohit = 15;
	t_ptr->todam = 18;
	t_ptr->toac  = 8;
        t_ptr->flags = (TR_STATS|TR_SLAY_EVIL|TR_SLAY_UNDEAD|TR_FROST_BRAND|
				TR_FREE_ACT|TR_SEE_INVIS);
	t_ptr->flags2 |= (TR_IM_COLD|TR_SLAY_ORC|TR_ACTIVATE|TR_ARTIFACT);
	t_ptr->p1    = 2;
        t_ptr->cost  = 200000L;
	EONWE = 1;
        return 1;
      }
  } else if (!strcmp("& Battle Axe", name)) {
    switch (randint(2)) {
    case 1:
      if (BALLI) return 0;
      if (wizard || peek) msg_print("Balli Stonehand");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_BALLI;
      t_ptr->tohit = 8;
      t_ptr->todam = 11;
      t_ptr->damage[0] = 3;
      t_ptr->damage[1] = 6;
      t_ptr->toac  = 5;
      t_ptr->flags = (TR_FFALL|TR_RES_LIGHT|TR_SEE_INVIS|TR_STR|TR_CON
		      |TR_FREE_ACT|TR_RES_COLD|TR_RES_ACID
		      |TR_RES_FIRE|TR_REGEN|TR_STEALTH);
      t_ptr->flags2 |= (TR_SLAY_DEMON|TR_SLAY_TROLL|TR_SLAY_ORC|TR_RES_BLIND
		|TR_ARTIFACT);
      t_ptr->p1    = 3;
      t_ptr->cost  = 90000L;
      BALLI = 1;
      return 1;
    case 2:
      if (LOTHARANG) return 0;
      if (wizard || peek) msg_print("Lotharang");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_LOTHARANG;
      t_ptr->tohit = 4;
      t_ptr->todam = 3;
      t_ptr->flags = (TR_STR|TR_DEX);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_SLAY_TROLL|TR_SLAY_ORC|TR_ARTIFACT);
      t_ptr->p1    = 1;
      t_ptr->cost  = 21000L;
      LOTHARANG = 1;
      return 1;
    }
  } else if (!strcmp("& War Hammer", name)) {
    if (randint(10)>1) return 0;
    if (AULE) return 0;
    if (wizard || peek) msg_print("Aule");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_AULE;
    t_ptr->damage[0] = 5;
    t_ptr->damage[1] = 5;
    t_ptr->tohit = 19;
    t_ptr->todam = 21;
    t_ptr->toac = 5;
    t_ptr->flags = (TR_SLAY_X_DRAGON|TR_SLAY_EVIL|TR_SLAY_UNDEAD|
		    TR_RES_FIRE|TR_RES_ACID|TR_RES_COLD|TR_RES_LIGHT|
		    TR_FREE_ACT|TR_SEE_INVIS|TR_WIS);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_DEMON|TR_LIGHTNING|TR_RES_NEXUS);
    t_ptr->p1 = 4;
    t_ptr->cost = 250000L;
    AULE = 1;
    return 1;
  } else if (!strcmp("& Beaked Axe", name)) {
    if (randint(2)>1) return 0;
    if (THEODEN) return 0;
    if (wizard || peek) msg_print("Theoden");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_THEODEN;
    t_ptr->tohit = 8;
    t_ptr->todam = 10;
    t_ptr->flags = (TR_WIS|TR_CON|TR_SEARCH|TR_SLOW_DIGEST|TR_SLAY_DRAGON);
    t_ptr->flags2 |= (TR_TELEPATHY|TR_ACTIVATE|TR_ARTIFACT);
    t_ptr->p1 = 3;
    t_ptr->cost = 40000L;
    THEODEN = 1;
    return 1;
  } else if (!strcmp("& Two-Handed Great Flail", name)) {
    if (randint(5)>1) return 0;
    if (THUNDERFIST) return 0;
    if (wizard || peek) msg_print("Thunderfist");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_THUNDERFIST;
    t_ptr->tohit = 5;
    t_ptr->todam = 18;
    t_ptr->flags = (TR_SLAY_ANIMAL|TR_STR|TR_FLAME_TONGUE|
		    TR_RES_FIRE|TR_RES_LIGHT);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_TROLL|TR_SLAY_ORC
		|TR_LIGHTNING|TR_RES_DARK);
    t_ptr->p1 = 4;
    t_ptr->cost = 160000L;
    THUNDERFIST = 1;
    return 1;
  } else if (!strcmp("& Morningstar", name)) {
    switch (randint(2)) {
    case 1:
      if (randint(2)>1) return 0;
      if (BLOODSPIKE) return 0;
      if (wizard || peek) msg_print("Bloodspike");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_BLOODSPIKE;
      t_ptr->tohit = 8;
      t_ptr->todam = 22;
      t_ptr->flags = (TR_SLAY_ANIMAL|TR_STR|TR_SEE_INVIS);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_TROLL|TR_SLAY_ORC|TR_RES_NEXUS);
      t_ptr->p1 = 4;
      t_ptr->cost = 30000L;
      BLOODSPIKE = 1;
      return 1;
    case 2:
      if (FIRESTAR) return 0;
      if (wizard || peek) msg_print("Firestar");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_FIRESTAR;
      t_ptr->tohit = 5;
      t_ptr->todam = 7;
      t_ptr->flags = (TR_FLAME_TONGUE|TR_RES_FIRE);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_ACTIVATE);
      t_ptr->toac = 2;
      t_ptr->cost = 35000L;
      FIRESTAR = 1;
      return 1;
    }
  } else if (!strcmp("& Blade of Chaos", name)) {
    if (DOOMCALLER) return 0;
    if (randint(3)>1) return 0;
    if (wizard || peek) msg_print("Doomcaller");
    else good_item_flag = TRUE;
     t_ptr->name2 = SN_DOOMCALLER;
    t_ptr->tohit = 18;
    t_ptr->todam = 28;
    t_ptr->flags = (TR_CON|TR_SLAY_ANIMAL|TR_SLAY_X_DRAGON|
		    TR_FROST_BRAND|TR_SLAY_EVIL|TR_FREE_ACT|TR_SEE_INVIS|
		    TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|TR_RES_ACID|
		    TR_AGGRAVATE);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_SLAY_TROLL|TR_SLAY_ORC|TR_TELEPATHY);
    t_ptr->p1 = -5;
    t_ptr->cost = 200000L;
    DOOMCALLER = 1;
    return 1;
  } else if (!strcmp("& Quarterstaff", name)) {
    switch (randint(7)) {
    case 1:
    case 2:
    case 3:
      if (NAR) return 0;
      if (wizard || peek) msg_print("Nar-i-vagil");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_NAR;
      t_ptr->tohit = 10;
      t_ptr->todam = 20;
      t_ptr->flags = (TR_INT|TR_SLAY_ANIMAL|TR_FLAME_TONGUE|TR_RES_FIRE);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->p1 = 3;
      t_ptr->ident |= ID_SHOW_P1;
      t_ptr->cost = 70000L;
      NAR = 1;
      return 1;
    case 4:
    case 5:
    case 6:
      if (ERIRIL) return 0;
      if (wizard || peek) msg_print("Eriril");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_ERIRIL;
      t_ptr->tohit = 3;
      t_ptr->todam = 5;
      t_ptr->flags = (TR_SLAY_EVIL|TR_SEE_INVIS|TR_INT|TR_WIS);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_LIGHT|TR_ACTIVATE|TR_RES_LT);
      t_ptr->p1 = 4;
      t_ptr->cost = 20000L;
      ERIRIL = 1;
      return 1;
    case 7:
      if (OLORIN) return 0;
      if (randint(2)>1) return 0;
      if (wizard || peek) msg_print("Olorin");
      else good_item_flag = TRUE;
      t_ptr->name2 = SN_OLORIN;
      t_ptr->tohit = 10;
      t_ptr->todam = 13;
      t_ptr->damage[0] = 2;
      t_ptr->damage[1] = 10;
      t_ptr->flags = (TR_SLAY_EVIL|TR_SEE_INVIS|TR_WIS|TR_INT|TR_CHR
			|TR_FLAME_TONGUE|TR_RES_FIRE);
      t_ptr->flags2 |= (TR_ARTIFACT|TR_HOLD_LIFE|TR_SLAY_ORC|TR_SLAY_TROLL
		|TR_ACTIVATE|TR_RES_NETHER);
      t_ptr->p1 = 4;
      t_ptr->cost = 130000L;
      OLORIN = 1;
      return 1;
    }
  }
  return 0;
}

int unique_armour(t_ptr)
inven_type *t_ptr;
{
  char *name;

  if (be_nasty) return 0;
  name = object_list[t_ptr->index].name;
  if (!strncmp("Adamantite", name, 10)) {
    if (SOULKEEPER) return 0;
    if (randint(3)>1) return 0;
    if (wizard || peek) msg_print("Soulkeeper");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_COLD);
    t_ptr->flags2 |= (TR_HOLD_LIFE|TR_ACTIVATE|TR_RES_CHAOS|TR_RES_DARK|
		      TR_RES_NEXUS|TR_RES_NETHER|TR_ARTIFACT);
    t_ptr->name2 |= SN_SOULKEEPER;
    t_ptr->toac += 20;
    t_ptr->cost = 300000L;
    SOULKEEPER = 1;
    return 1;
  }/* etc.....*/
  else if (!strncmp("Multi-Hued", name, 10)) {
    if (RAZORBACK) return 0;
    if (wizard || peek) msg_print("Razorback");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_FIRE|TR_RES_COLD|TR_RES_ACID|TR_POISON|TR_RES_LIGHT
		     |TR_FREE_ACT|TR_SEE_INVIS|TR_INT|TR_WIS|TR_STEALTH
		     |TR_AGGRAVATE);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_LIGHT|TR_IM_LIGHT|TR_RES_LT|TR_ARTIFACT);
    t_ptr->toac += 25;
    t_ptr->p1 = -2;
    t_ptr->weight = 400;
    t_ptr->ac = 40;
    t_ptr->tohit = -3;
    t_ptr->cost = 400000L;
    t_ptr->name2 |= RAZORBACK;
    RAZORBACK = 1;
    return 1;
  }
  else if (!strncmp("Power Drag", name, 10)) {
    if (BLADETURNER) return 0;
    if (wizard || peek) msg_print("Bladeturner");
    else good_item_flag =  TRUE;
    t_ptr->flags |= (TR_RES_FIRE|TR_RES_COLD|TR_RES_ACID|TR_POISON|TR_RES_LIGHT
		     |TR_DEX|TR_SEARCH|TR_REGEN);
    t_ptr->flags2 |= (TR_HOLD_LIFE|TR_RES_CONF|TR_RES_SOUND|TR_RES_LT
		     |TR_RES_DARK|TR_RES_CHAOS|TR_RES_DISENCHANT
		     |TR_RES_SHARDS|TR_RES_BLIND|TR_RES_NEXUS|TR_RES_NETHER
		     |TR_ARTIFACT);
    t_ptr->toac += 35;
    t_ptr->p1 = -3;
    t_ptr->ac = 50;
    t_ptr->tohit = -4;
    t_ptr->weight = 500;
    t_ptr->cost = 500000L;
    t_ptr->name2 |= BLADETURNER;
    BLADETURNER = 1;
    return 1;
  }
  else if (!strcmp("& Pair of Hard Leather Boots", name)) {
    if (FEANOR) return 0;
    if (randint(5)>1) return 0;
    if (wizard || peek) msg_print("Feanor");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_SPEED|TR_STEALTH);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_RES_NEXUS|TR_ARTIFACT);
    t_ptr->name2 |= SN_FEANOR;
    t_ptr->p1 = 1;
    t_ptr->toac += 20;
    t_ptr->cost = 130000L;
    FEANOR = 1;
    return 1;
  }
  else if (!strcmp("& Pair of Soft Leather Boots", name)) {
    if (DAL) return 0;
    if (wizard || peek) msg_print("Dal-i-thalion");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_FREE_ACT|TR_DEX|TR_SUST_STAT|TR_RES_ACID);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
    t_ptr->name2 |= SN_DAL;
    t_ptr->p1 = 5;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->toac += 15;
    t_ptr->cost = 40000L;
    DAL = 1;
    return 1;
  }
  else if (!strcmp("& Small Metal Shield", name)) {
    if (THORIN) return 0;
    if (randint(2)>1) return 0;
    if (wizard || peek) msg_print("Thorin");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_CON|TR_FREE_ACT|TR_STR|
				TR_RES_ACID|TR_SEARCH);
    t_ptr->flags2 |= (TR_IM_ACID|TR_RES_SOUND|TR_RES_CHAOS|TR_ARTIFACT);
    t_ptr->name2 |= SN_THORIN;
    t_ptr->tohit = 0;
    t_ptr->p1 = 4;
    t_ptr->toac += 25;
    t_ptr->cost = 60000L;
    THORIN = 1;
    return 1;
  }
  else if (!strcmp("Full Plate Armour", name)) {
    if (ISILDUR) return 0;
    if (wizard || peek) msg_print("Isildur");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT);
    t_ptr->flags2 |= (TR_RES_SOUND|TR_ARTIFACT);
    t_ptr->name2 |= SN_ISILDUR;
    t_ptr->tohit = 0;
    t_ptr->toac += 25;
    t_ptr->cost = 40000L;
    ISILDUR = 1;
    return 1;
  }
  else if (!strcmp("Metal Brigandine Armour", name)) {
    if (ROHAN) return 0;
    if (wizard || peek) msg_print("Rohan");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
			TR_STR|TR_DEX);
    t_ptr->flags2 |= (TR_RES_SOUND|TR_RES_CONF|TR_ARTIFACT);
    t_ptr->name2 |= SN_ROHAN;
    t_ptr->tohit = 0;
    t_ptr->p1 = 2;
    t_ptr->toac += 15;
    t_ptr->cost = 30000L;
    ROHAN = 1;
    return 1;
  }
  else if (!strcmp("& Large Metal Shield", name)) {
    if (ANARION) return 0;
    if (randint(3)>1) return 0;
    else good_item_flag = TRUE;
    if (wizard || peek) msg_print("Anarion");
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		     TR_SUST_STAT);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_ANARION;
    t_ptr->p1 = 10;
    t_ptr->tohit = 0;
    t_ptr->toac += 20;
    t_ptr->cost = 160000L;
    ANARION = 1;
    return 1;
  }
  else if (!strcmp("& Set of Cestus", name)) {
    if (FINGOLFIN) return 0;
    if (randint(3)>1) return 0;
    if (wizard || peek) msg_print("Fingolfin");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_DEX|TR_FREE_ACT);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
    t_ptr->name2 |= SN_FINGOLFIN;
    t_ptr->ident |= ID_SHOW_HITDAM;
    t_ptr->p1 = 4;
    t_ptr->tohit = 10;
    t_ptr->todam = 10;
    t_ptr->toac += 20;
    t_ptr->cost = 110000L;
    FINGOLFIN = 1;
    return 1;
  }
  else if (!strcmp("& Set of Leather Gloves", name)) {
    if (randint(3)==1) {
      if (CAMBELEG) return 0;
      if (wizard || peek) msg_print("Cambeleg");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_STR|TR_CON|TR_FREE_ACT);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->name2 |= SN_CAMBELEG;
      t_ptr->ident |= ID_SHOW_HITDAM;
      t_ptr->p1 = 2;
      t_ptr->tohit = 8;
      t_ptr->todam = 8;
      t_ptr->toac += 15;
      t_ptr->cost = 36000L;
      CAMBELEG = 1;
      return 1;
    } else {
      if (CAMMITHRIM) return 0;
      if (wizard || peek) msg_print("Cammithrim");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_SUST_STAT);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_LIGHT|TR_RES_LT|TR_ARTIFACT);
      t_ptr->name2 |= SN_CAMMITHRIM;
      t_ptr->p1 = 5;
      t_ptr->toac += 10;
      t_ptr->cost = 30000L;
      CAMMITHRIM = 1;
      return 1;
    }
  }
  else if (!strcmp("& Set of Gauntlets", name)) {
    switch (randint(6)) {
    case 1:
      if (PAURHACH) return 0;
      if (wizard || peek) msg_print("Paurhach");
      else good_item_flag = TRUE;
      t_ptr->flags |= TR_RES_FIRE;
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->name2 |= SN_PAURHACH;
      t_ptr->toac += 15;
      t_ptr->cost = 15000L;
      PAURHACH = 1;
      return 1;
    case 2:
      if (PAURNIMMEN) return 0;
      if (wizard || peek) msg_print("Paurnimmen");
      else good_item_flag = TRUE;
      t_ptr->flags |= TR_RES_COLD;
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->name2 |= SN_PAURNIMMEN;
      t_ptr->toac += 15;
      t_ptr->cost = 13000L;
      PAURNIMMEN = 1;
      return 1;
    case 3:
      if (PAURAEGEN) return 0;
      if (wizard || peek) msg_print("Pauraegen");
      else good_item_flag = TRUE;
      t_ptr->flags |= TR_RES_LIGHT;
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->name2 |= SN_PAURAEGEN;
      t_ptr->toac += 15;
      t_ptr->cost = 11000L;
      PAURAEGEN = 1;
      return 1;
    case 4:
      if (PAURNEN) return 0;
      if (wizard || peek) msg_print("Paurnen");
      else good_item_flag = TRUE;
      t_ptr->flags |= TR_RES_ACID;
      t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
      t_ptr->name2 |= SN_PAURNEN;
      t_ptr->toac += 15;
      t_ptr->cost = 12000L;
      PAURNEN = 1;
      return 1;
    default:
      if (CAMLOST) return 0;
      if (wizard || peek) msg_print("Camlost");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_STR|TR_DEX|TR_AGGRAVATE|TR_CURSED);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->name2 |= SN_CAMLOST;
      t_ptr->p1 = -5;
      t_ptr->tohit = -11;
      t_ptr->todam = -12;
      t_ptr->ident |=(ID_SHOW_HITDAM|ID_SHOW_P1);
      t_ptr->cost = 0L;
      CAMLOST = 1;
      return 1;
    }
  }
  else if (!strcmp("Mithril Chain Mail", name)) {
    if (BELEGENNON) return 0;
    if (wizard || peek) msg_print("Belegennon");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		     TR_STEALTH);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
    t_ptr->name2 |= SN_BELEGENNON;
    t_ptr->p1 = 4;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->toac += 20;
    t_ptr->cost = 105000L;
    BELEGENNON = 1;
    return 1;
  }
  else if (!strcmp("Mithril Plate Mail", name)) {
    if (CELEBORN) return 0;
    if (wizard || peek) msg_print("Celeborn");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		     TR_STR|TR_CHR);
    t_ptr->flags2 |= (TR_ACTIVATE|TR_RES_DISENCHANT|TR_RES_DARK|TR_ARTIFACT);
    t_ptr->name2 |= SN_CELEBORN;
    t_ptr->p1 = 4;
    t_ptr->toac += 25;
    t_ptr->cost = 150000L;
    CELEBORN = 1;
    return 1;
  }
  else if (!strcmp("Augmented Chain Mail", name)) {
    if (randint(3)>1) return 0;
    if (CASPANION) return 0;
    if (wizard || peek) msg_print("Caspanion");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_POISON|TR_CON|TR_WIS|TR_INT);
    t_ptr->flags2 |= (TR_RES_CONF|TR_ACTIVATE|TR_ARTIFACT);
    t_ptr->name2 |= SN_CASPANION;
    t_ptr->p1 = 3;
    t_ptr->toac += 20;
    t_ptr->cost = 40000L;
    CASPANION = 1;
    return 1;
  }
  else if (!strcmp("Soft Leather Armour", name)) {
    if (HITHLOMIR) return 0;
    if (wizard || peek) msg_print("Hithlomir");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		     TR_STEALTH);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_HITHLOMIR;
    t_ptr->p1 = 4;
    t_ptr->toac += 20;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->cost = 45000L;
    HITHLOMIR = 1;
    return 1;
  }
  else if (!strcmp("Leather Scale Mail", name)) {
    if (THALKETTOTH) return 0;
    if (wizard || peek) msg_print("Thalkettoth");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_DEX);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_THALKETTOTH;
    t_ptr->toac += 25;
    t_ptr->p1 = 3;
    t_ptr->cost = 25000L;
    THALKETTOTH = 1;
    return 1;
  }
  else if (!strcmp("Chain Mail", name)) {
    if (ARVEDUI) return 0;
    if (wizard || peek) msg_print("Arvedui");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		     TR_STR|TR_CHR);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_ARVEDUI;
    t_ptr->p1 = 2;
    t_ptr->toac += 15;
    t_ptr->cost = 32000L;
    ARVEDUI = 1;
    return 1;
  }
  else if (!strcmp("& Hard Leather Cap", name)) {
    if (THRANDUIL) return 0;
    if (wizard || peek) msg_print("Thranduil");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_INT|TR_WIS);
    t_ptr->flags2 |= (TR_TELEPATHY|TR_RES_BLIND|TR_ARTIFACT);
    t_ptr->name2 |= SN_THRANDUIL;
    t_ptr->p1 = 2;
    t_ptr->toac += 10;
    t_ptr->cost = 50000L;
    THRANDUIL = 1;
    return 1;
  }
  else if (!strcmp("& Metal Cap", name)) {
    if (THENGEL) return 0;
    if (wizard || peek) msg_print("Thengel");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_WIS|TR_CHR);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_THENGEL;
    t_ptr->p1 = 3;
    t_ptr->toac += 12;
    t_ptr->cost = 22000L;
    THENGEL = 1;
    return 1;
  }
  else if (!strcmp("& Steel Helm", name)) {
    if (HAMMERHAND) return 0;
    if (wizard || peek) msg_print("Hammerhand");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_STR|TR_CON|TR_DEX|TR_RES_ACID);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_HAMMERHAND;
    t_ptr->p1 = 3;
    t_ptr->toac += 20;
    t_ptr->cost = 45000L;
    HAMMERHAND = 1;
    return 1;
  }
  else if (!strcmp("& Large Leather Shield", name)) {
    if (CELEFARN) return 0;
    if (wizard || peek) msg_print("Celefarn");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_CELEFARN;
    t_ptr->toac += 20;
    t_ptr->cost = 12000L;
    CELEFARN = 1;
    return 1;
  }
  else if (!strcmp("& Pair of Metal Shod Boots", name)) {
    if (THROR) return 0;
    if (wizard || peek) msg_print("Thror");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_CON|TR_STR|TR_RES_ACID);
    t_ptr->flags2 |= (TR_ARTIFACT);
    t_ptr->name2 |= SN_THROR;
    t_ptr->p1 = 3;
    t_ptr->toac += 20;
    t_ptr->cost = 12000L;
    THROR = 1;
    return 1;
  }
  else if (!strcmp("& Iron Helm", name)) {
    if (randint(6)==1) {
      if (DOR_LOMIN) return 0;
      if (wizard || peek) msg_print("Dor-Lomin");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_RES_ACID|TR_RES_FIRE|TR_RES_COLD|TR_RES_LIGHT|
		       TR_CON|TR_DEX|TR_STR|TR_SEE_INVIS);
      t_ptr->flags2 |= (TR_TELEPATHY|TR_LIGHT|TR_RES_LT|TR_RES_BLIND
		|TR_ARTIFACT);
      t_ptr->name2 |= SN_DOR_LOMIN;
      t_ptr->p1 = 4;
      t_ptr->toac += 20;
      t_ptr->cost = 300000L;
      DOR_LOMIN = 1;
      return 1;
    } else if (randint(2)==1) {
      if (HOLHENNETH) return 0;
      if (wizard || peek) msg_print("Holhenneth");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_INT|TR_WIS|TR_SEE_INVIS|TR_SEARCH|TR_RES_ACID);
      t_ptr->flags2 |= (TR_ACTIVATE|TR_RES_BLIND|TR_ARTIFACT);
      t_ptr->name2 |= SN_HOLHENNETH;
      t_ptr->ident |= ID_SHOW_P1;
      t_ptr->p1 = 2;
      t_ptr->toac += 10;
      t_ptr->cost = 100000L;
      HOLHENNETH = 1;
      return 1;
    } else {
      if (GORLIM) return 0;
      if (wizard || peek) msg_print("Gorlim");
      else good_item_flag = TRUE;
      t_ptr->flags |= (TR_INT|TR_WIS|TR_SEE_INVIS|TR_SEARCH|TR_CURSED
		       |TR_AGGRAVATE);
      t_ptr->flags2 |= (TR_ARTIFACT);
      t_ptr->name2 |= SN_GORLIM;
      t_ptr->ident |= ID_SHOW_P1;
      t_ptr->p1 = -125;
      t_ptr->toac += 10;
      t_ptr->cost = 0L;
      GORLIM = 1;
      return 1;
    }
  }
  else if (!strcmp("& Golden Crown", name)) {
    if (randint(3)>1) return 0;
    if (GONDOR) return 0;
    if (wizard || peek) msg_print("Gondor");
    else good_item_flag = TRUE;
    t_ptr->name2 = SN_GONDOR;
    t_ptr->flags = (TR_STR|TR_CON|TR_WIS|TR_SEE_INVIS|TR_REGEN
				|TR_RES_ACID|TR_RES_FIRE);
    t_ptr->flags2 |= (TR_ARTIFACT|TR_ACTIVATE|TR_LIGHT|TR_RES_LT|TR_RES_BLIND);
    t_ptr->p1 = 3;
    t_ptr->ident = ID_SHOW_P1;
    t_ptr->toac += 15;
    t_ptr->cost = 100000L;
    GONDOR = 1;
    return 1;
  }
  else if (!strcmp("& Iron Crown", name)) {
    if (BERUTHIEL) return 0;
    if (wizard || peek) msg_print("Beruthiel");
    else good_item_flag = TRUE;
    t_ptr->flags |= (TR_STR|TR_DEX|TR_CON|
		TR_RES_ACID|TR_SEE_INVIS|TR_FREE_ACT|TR_CURSED);
    t_ptr->flags2 |= (TR_TELEPATHY|TR_ARTIFACT);
    t_ptr->name2 |= SN_BERUTHIEL;
    t_ptr->p1 = -125;
    t_ptr->ident |= ID_SHOW_P1;
    t_ptr->toac += 20;
    t_ptr->cost = 10000L;
    BERUTHIEL = 1;
    return 1;
  }
  return 0;
}

/* Chance of treasure having magic abilities		-RAK-	*/
/* Chance increases with each dungeon level			 */
void magic_treasure(x, level, good, not_unique)
int x, level, good, not_unique;
{
  register inven_type *t_ptr;
#ifdef MSDOS
  register int chance, special, cursed;
  register int32u i;
#else
  register int32u chance, special, cursed, i;
#endif
  int32u tmp;

  chance = OBJ_BASE_MAGIC + level;
  if (chance > OBJ_BASE_MAX)
    chance = OBJ_BASE_MAX;
  special = chance / OBJ_DIV_SPECIAL;
  cursed  = (10 * chance) / OBJ_DIV_CURSED;
  t_ptr = &t_list[x];

  /* some objects appear multiple times in the object_list with different
     levels, this is to make the object occur more often, however, for
     consistency, must set the level of these duplicates to be the same
     as the object with the lowest level */

  /* Depending on treasure type, it can have certain magical properties*/
  switch (t_ptr->tval)
    {
    case TV_SHIELD: case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
      if ((t_ptr->index>=389 && t_ptr->index<=394)
      || (t_ptr->index>=408 && t_ptr->index<=409)
      || (t_ptr->index>=415 && t_ptr->index<=419)) {
	  t_ptr->toac += m_bonus(1, 30, level);
	}
      else if (magik(chance)||good)
	{
	  t_ptr->toac += m_bonus(1, 30, level);
	  if (!strcmp(object_list[t_ptr->index].name, "& Robe") &&
	      ((magik(special) && randint(30)==1)
	       ||(good==666 && magik(special))))
	    {
	    t_ptr->flags |= (TR_RES_LIGHT|TR_RES_COLD|TR_RES_ACID|TR_RES_FIRE|
			     TR_SUST_STAT);
	    if (wizard || peek) msg_print("Robe of the Magi");
	    rating += 30;
	    t_ptr->flags2 |= TR_HOLD_LIFE;
	    t_ptr->p1 = 10;
	    t_ptr->toac += 10+randint(5);
	    t_ptr->name2 = SN_MAGI;
	    t_ptr->cost = 10000L + (t_ptr->toac * 100);
	  } else if ((t_ptr->index>=389 && t_ptr->index<=394)
		 || (t_ptr->index>=408 && t_ptr->index<=409)
   		 || (t_ptr->index>=415 && t_ptr->index<=419)) {
	      if (wizard || peek) msg_print("Dragon Scale Mail");
	      rating += 30;
	      t_ptr->toac += randint(10);
	      t_ptr->cost += (t_ptr->toac * 500);
	    }
	  else if (magik(special)||good==666)
	    switch(randint(9))
	      {
	      case 1:
		if ((randint(3)==1 || good==666) && !not_unique &&
		    unique_armour(t_ptr)) break;
		t_ptr->flags |= (TR_RES_LIGHT|TR_RES_COLD|TR_RES_ACID|
				 TR_RES_FIRE);
		if (randint(3)==1) {
		  if (peek) msg_print("Elvenkind");
		  rating += 25;
		  t_ptr->flags |= TR_STEALTH;
		  t_ptr->ident |= ID_SHOW_P1;
		  t_ptr->p1 = randint(3);
		  t_ptr->name2 = SN_ELVENKIND;
		  t_ptr->toac += 15;
		  t_ptr->cost += 15000L;
		} else {
		  if (peek) msg_print("Resist");
		  rating += 20;
		  t_ptr->name2 = SN_R;
		  t_ptr->toac += 8;
		  t_ptr->cost += 12500L;
		}
		break;
	      case 2:	 /* Resist Acid	  */
		if ((randint(3)==1 || good==666) && !not_unique &&
		    unique_armour(t_ptr)) break;
		if (!strncmp(object_list[t_ptr->index].name,
			    "Mithril", 7) ||
		    !strncmp(object_list[t_ptr->index].name,
			    "Adamantite", 10)) break;
		if (peek) msg_print("Resist Acid");
		rating += 15;
		t_ptr->flags |= TR_RES_ACID;
		t_ptr->name2 = SN_RA;
		t_ptr->cost += 1000L;
		break;
	      case 3: case 4:	 /* Resist Fire	  */
		if ((randint(3)==1 || good==666) && !not_unique &&
		    unique_armour(t_ptr)) break;
		if (peek) msg_print("Resist Fire");
		rating += 17;
		t_ptr->flags |= TR_RES_FIRE;
		t_ptr->name2 = SN_RF;
		t_ptr->cost += 600L;
		break;
	      case 5: case 6:	/* Resist Cold	 */
		if ((randint(3)==1 || good==666) && !not_unique &&
		    unique_armour(t_ptr)) break;
		if (peek) msg_print("Resist Cold");
		rating += 16;
		t_ptr->flags |= TR_RES_COLD;
		t_ptr->name2 = SN_RC;
		t_ptr->cost += 600L;
		break;
	      case 7: case 8: case 9:  /* Resist Lightning*/
		if ((randint(3)==1 || good==666) && !not_unique &&
		    unique_armour(t_ptr)) break;
		if (peek) msg_print("Resist Lightning");
		rating += 15;
		t_ptr->flags |= TR_RES_LIGHT;
		t_ptr->name2 = SN_RL;
		t_ptr->cost += 500L;
		break;
	      }
	}
      else if (magik(cursed))
	{
	  t_ptr->toac -= m_bonus(1, 40, level);
	  t_ptr->cost = 0L;
	  t_ptr->flags |= TR_CURSED;
	}
      break;

    case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
      /* always show tohit/todam values if identified */
      t_ptr->ident |= ID_SHOW_HITDAM;
      if (magik(chance)||good)
	{
	  t_ptr->tohit += m_bonus(0, 40, level);
	  t_ptr->todam += m_bonus(0, 40, level);
	  /* the 3*special/2 is needed because weapons are not as common as
	     before change to treasure distribution, this helps keep same
	     number of ego weapons same as before, see also missiles */
	  if (magik(3*special/2)||good==666) { /* was 2 */
	    if (!strcmp("& Whip", object_list[t_ptr->index].name)) {
		if (peek) msg_print("Whip of Fire");
		rating += 20;
		t_ptr->name2 = SN_FIRE;
		t_ptr->flags |=TR_FLAME_TONGUE;
		if (randint(10)==1)
		  t_ptr->damage[0]=2;
		t_ptr->tohit += 5;
		t_ptr->todam += 5;
	      }
	    else {
	      switch(randint(27)) /* was 16 */
		{
		case 27: /* of Westernesse */
		  if (((randint(2) == 1) || (good == 666)) && !not_unique &&
		      unique_weapon(t_ptr)) break;
		  if (peek) msg_print("Westernesse");
		  rating += 30;
		  t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_EVIL|TR_SLAY_ORC|
				   TR_SLAY_UNDEAD|TR_DEX|TR_CON|TR_STR|
				   TR_FREE_ACT);
		  t_ptr->tohit += randint(5)+3;
		  t_ptr->todam += randint(5)+3;
		  t_ptr->p1 = 1;
		  t_ptr->cost += 15000L;
		  t_ptr->cost *= 2;
		  t_ptr->name2 = SN_WEST;
		  break;
		case 1:	/* Holy Avenger	 */
		  if (((randint(2) == 1) || (good == 666)) && !not_unique &&
		      unique_weapon(t_ptr)) break;
		  if (peek) msg_print("Holy Avenger");
		  rating += 26;
		  t_ptr->flags |= (TR_SEE_INVIS|TR_SUST_STAT|TR_SLAY_UNDEAD|
				   TR_SLAY_EVIL|TR_STR);
		  t_ptr->flags2 |= TR_SLAY_DEMON;
		  t_ptr->tohit += 5;
		  t_ptr->todam += 5;
		  t_ptr->toac  += randint(4);
		  /* the value in p1 is used for strength increase */
		  /* p1 is also used for sustain stat */
		  t_ptr->p1    = randint(4);
		  t_ptr->name2 = SN_HA;
		  t_ptr->cost += t_ptr->p1*500;
		  t_ptr->cost += 10000L;
		  t_ptr->cost *= 2;
		  break;
		case 2:	/* Defender	 */
		  if (((randint(2) == 1) || (good == 666)) && !not_unique &&
		      unique_weapon(t_ptr)) break;
		  if (peek) msg_print("Defender");
		  rating += 23;
		  t_ptr->flags |= (TR_FFALL|TR_RES_LIGHT|TR_SEE_INVIS
				   |TR_FREE_ACT|TR_RES_COLD|TR_RES_ACID
				   |TR_RES_FIRE|TR_REGEN|TR_STEALTH);
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->toac  += 5 + randint(5);
		  t_ptr->name2 = SN_DF;
		  /* the value in p1 is used for stealth */
		  t_ptr->p1    = randint(3);
		  t_ptr->cost += t_ptr->p1*500;
		  t_ptr->cost += 7500L;
		  t_ptr->cost *= 2;
		  break;
		case 3: case 4:   /* Flame Tongue  */
		  if (((randint(2) == 1) || (good == 666)) && !not_unique &&
		      unique_weapon(t_ptr)) break;
		  rating += 20;
		  t_ptr->flags |= TR_FLAME_TONGUE|TR_RES_FIRE;
		  if (peek) msg_print("Flame");
		  t_ptr->tohit += 2;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_FT;
		  t_ptr->cost += 3000L;
		  break;
		case 5: case 6:   /* Frost Brand   */
		  if (((randint(2) == 1) || (good == 666)) && !not_unique &&
		      unique_weapon(t_ptr)) break;
		  if (peek) msg_print("Frost");
		  rating += 20;
		  t_ptr->flags |= TR_FROST_BRAND|TR_RES_COLD;
		  t_ptr->tohit+=2;
		  t_ptr->todam+=2;
		  t_ptr->name2 = SN_FB;
		  t_ptr->cost += 2200L;
		  break;
		case 7: case 8:	 /* Slay Animal  */
		  t_ptr->flags |= TR_SLAY_ANIMAL;
		  rating += 15;
		  if (peek) msg_print("Slay Animal");
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_SA;
		  t_ptr->cost += 2000L;
		  break;
		case 9: case 10:	/* Slay Dragon	 */
		  t_ptr->flags |= TR_SLAY_DRAGON;
		  if (peek) msg_print("Slay Dragon");
		  rating += 18;
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_SD;
		  t_ptr->cost += 4000L;
		  break;
		case 11: case 12:	/* Slay Evil   */
		  t_ptr->flags |= TR_SLAY_EVIL;
		  if (peek) msg_print("Slay Evil");
		  rating += 18;
		  t_ptr->tohit += 3;
		  t_ptr->todam += 3;
		  t_ptr->name2 = SN_SE;
		  t_ptr->cost += 4000L;
		  break;
		case 13: case 14:	 /* Slay Undead	  */
		  t_ptr->flags |= (TR_SEE_INVIS|TR_SLAY_UNDEAD);
		  if (peek) msg_print("Slay Undead");
		  rating += 18;
		  t_ptr->tohit += 2;
		  t_ptr->todam += 2;
		  t_ptr->name2 = SN_SU;
		  t_ptr->cost += 3000L;
		  break;
		case 15: case 16: case 17: /* Slay Orc */
		  t_ptr->flags2 |= TR_SLAY_ORC;
		  if (peek) msg_print("Slay Orc");
		  rating += 13;
		  t_ptr->tohit+=2;
		  t_ptr->todam+=2;
		  t_ptr->name2 = SN_SO;
		  t_ptr->cost += 1200L;
		  break;
		case 18: case 19: case 20: /* Slay Troll */
		  t_ptr->flags2 |= TR_SLAY_TROLL;
		  if (peek) msg_print("Slay Troll");
		  rating += 13;
		  t_ptr->tohit+=2;
		  t_ptr->todam+=2;
		  t_ptr->name2 = SN_ST;
		  t_ptr->cost += 1200L;
		  break;
	        case 21: case 22: case 23:
		  t_ptr->flags2 |= TR_SLAY_GIANT;
		  if (peek) msg_print("Slay Giant");
		  rating += 14;
		  t_ptr->tohit+=2;
		  t_ptr->todam+=2;
		  t_ptr->name2 = SN_SG;
		  t_ptr->cost += 1200L;
		  break;
		case 24: case 25: case 26:
		  t_ptr->flags2 |= TR_SLAY_DEMON;
		  if (peek) msg_print("Slay Demon");
		  rating += 16;
		  t_ptr->tohit+=2;
		  t_ptr->todam+=2;
		  t_ptr->name2 = SN_SDEM;
		  t_ptr->cost += 1200L;
		  break;
		}
	    }
	  }
	} else if (magik(cursed)) {
	    t_ptr->tohit -= m_bonus(1, 55, level);
	    t_ptr->todam -= m_bonus(1, 55, level);
	    t_ptr->flags |= TR_CURSED;
	    if (level > (20 + randint(15)) && randint(10)==1) {
	      t_ptr->name2 = SN_MORGUL;
	      t_ptr->flags |= (TR_SEE_INVIS|TR_AGGRAVATE);
	      t_ptr->tohit -= 15;
	      t_ptr->todam -= 15;
	      t_ptr->toac  = -10;
	      t_ptr->weight += 100;
	    }
	    t_ptr->cost = 0L;
	  }
	  break;

	case TV_BOW:
	  /* always show tohit/todam values if identified */
	  t_ptr->ident |= ID_SHOW_HITDAM;
	  if (magik(chance)||good) {
	    t_ptr->tohit += m_bonus(1, 30, level);
	    t_ptr->todam += m_bonus(1, 20, level);
	    switch (randint(15)) {
	    case 1:
	      if (((randint(3)==1)||(good==666)) && !not_unique &&
		  !strcmp(object_list[t_ptr->index].name, "& Long Bow")) {
		    switch (randint(2)) {
		    case 1:
		      if (BELEG) break;
		      if (wizard || peek) msg_print("Beleg Cuthalion");
		      t_ptr->name2 = SN_BELEG;
		      t_ptr->tohit += 20;
		      t_ptr->todam += 22;
		      t_ptr->p1 = 3;
		      t_ptr->flags |= (TR_STEALTH|TR_DEX);
		      t_ptr->flags2 |= (TR_ARTIFACT);
		      t_ptr->ident |= ID_SHOW_P1;
		      t_ptr->cost = 25000L;
		      BELEG = 1;
		      break;
		    case 2:
		      if (BARD) break;
		      if (wizard || peek) msg_print("Bard");
		      t_ptr->name2 = SN_BARD;
		      t_ptr->tohit += 17;
		      t_ptr->todam += 19;
		      t_ptr->p1 = 3;
		      t_ptr->flags |= (TR_FREE_ACT|TR_DEX);
		      t_ptr->flags2 |= (TR_ARTIFACT);
		      t_ptr->cost = 20000L;
		      BARD = 1;
		      break;
		    }
		    break;
	      }
	      if (((randint(5)==1)||(good==666)) && !not_unique &&
		  !strcmp(object_list[t_ptr->index].name, "& Light Crossbow"))
		{
		  if (CUBRAGOL) break;
		  if (wizard || peek) msg_print("Cubragol");
		  t_ptr->name2 = SN_CUBRAGOL;
		  t_ptr->tohit += 10;
		  t_ptr->todam += 14;
		  t_ptr->p1 = 1;
		  t_ptr->flags |= (TR_SPEED|TR_RES_FIRE);
		  t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
		  t_ptr->ident |= ID_SHOW_P1;
		  t_ptr->cost = 38000L;
		  CUBRAGOL = 1;
		  break;
		}
	      t_ptr->name2 = SN_MIGHT;
	      if (peek) msg_print("Bow of Might");
	      rating += 11;
	      t_ptr->tohit += 7;
	      t_ptr->todam += 13;
	      break;
	    case 2:
	      t_ptr->name2 = SN_ACCURACY;
	      rating += 11;
	      if (peek) msg_print("Accuracy");
	      t_ptr->tohit += 15;
	      t_ptr->todam += 5;
	      break;
	    }
	  }
	  else if (magik(cursed)) {
	    t_ptr->tohit -= m_bonus(1, 50, level);
	    t_ptr->todam -= m_bonus(1, 30, level); /* add damage. -CJS- */
	    t_ptr->flags |= TR_CURSED;
	    t_ptr->cost = 0L;
	  }
	  break;

	case TV_DIGGING:
	  /* always show tohit/todam values if identified */
	  t_ptr->ident |= ID_SHOW_HITDAM;
	  if (magik(chance)||(good==666)) {
	    tmp = randint(3);
	    if (tmp < 3)
	      t_ptr->p1 += m_bonus(0, 25, level);
	    else
	      {
		/* a cursed digging tool */
		t_ptr->p1 = -m_bonus(1, 30, level);
		t_ptr->cost = 0L;
		t_ptr->flags |= TR_CURSED;
	      }
	  }
	  break;

	case TV_GLOVES:
	  if (magik(chance)||good) {
	    t_ptr->toac += m_bonus(1, 20, level);
	    if ((((randint(2) == 1) && magik(5*special/2)) || (good == 666)) &&
		!strcmp(object_list[t_ptr->index].name,
			"& Set of Leather of Gloves") &&
		!not_unique && unique_armour(t_ptr)) ;
	    else if ((((randint(4) == 1) && magik(special)) || (good == 666))
		     && !strcmp(object_list[t_ptr->index].name,
				"& Set of Gauntlets") &&
		     !not_unique && unique_armour(t_ptr)) ;
	    else if (magik(special)||(good==666)) {
              switch(randint(10)) {
	      case 1: case 2: case 3:
		if (peek) msg_print("Free action");
		rating += 11;
		t_ptr->flags |= TR_FREE_ACT;
		t_ptr->name2 = SN_FREE_ACTION;
		t_ptr->cost += 1000L;
		break;
	      case 4: case 5: case 6:
		t_ptr->ident |= ID_SHOW_HITDAM;
		rating += 17;
		if (peek) msg_print("Slaying");
	        t_ptr->tohit += 1 + randint(4);
	        t_ptr->todam += 1 + randint(4);
	        t_ptr->name2 = SN_SLAYING;
	        t_ptr->cost += (t_ptr->tohit+t_ptr->todam)*250;
		break;
	      case 7: case 8: case 9:
		t_ptr->name2 = SN_AGILITY;
		if (peek) msg_print("Agility");
		rating += 14;
		t_ptr->p1 = 2+randint(2);
		t_ptr->flags |= TR_DEX;
		t_ptr->ident |= ID_SHOW_P1;
		t_ptr->cost += (t_ptr->p1)*400;
		break;
	      case 10:
		if (((randint(3) == 1) || (good == 666)) && !not_unique &&
		    unique_armour(t_ptr)) break;
		if (peek) msg_print("Power");
		rating += 22;
		t_ptr->name2 = SN_POWER;
		t_ptr->p1 = 1+randint(4);
		t_ptr->tohit += 1+randint(4);
		t_ptr->todam += 1+randint(4);
		t_ptr->flags |= TR_STR;
	        t_ptr->ident |= ID_SHOW_HITDAM;
		t_ptr->ident |= ID_SHOW_P1;
		t_ptr->cost += (t_ptr->tohit+t_ptr->todam+t_ptr->p1)*300;
		break;
	      }
	  }
	}
	  else if (magik(cursed))
	    {
	      if (magik(special))
		{
		  if (randint(2) == 1)
		    {
		      t_ptr->flags |= TR_DEX;
		      t_ptr->name2 = SN_CLUMSINESS;
		    }
		  else
		    {
		      t_ptr->flags |= TR_STR;
		      t_ptr->name2 = SN_WEAKNESS;
		    }
		  t_ptr->ident |= ID_SHOW_P1;
		  t_ptr->p1   = -m_bonus(1, 10, level);
		}
	      t_ptr->toac -= m_bonus(1, 40, level);
	      t_ptr->flags |= TR_CURSED;
	      t_ptr->cost = 0;
	    }
	  break;

	case TV_BOOTS:
	  if (magik(chance)||good)
	    {
	      t_ptr->toac += m_bonus(1, 20, level);
	      if (magik(special)||(good==666))
		{
		  tmp = randint(12);
		  if (tmp > 5)
		    {
		      t_ptr->flags |= TR_FFALL;
		      rating += 7;
		      t_ptr->name2 = SN_SLOW_DESCENT;
		      t_ptr->cost += 250;
		    }
		  else if (tmp == 1)
		    {
		      if (!((randint(2) == 1) && !not_unique
			    && unique_armour(t_ptr))) {
			t_ptr->flags |= TR_SPEED;
			if (wizard || peek) msg_print("Boots of Speed");
			t_ptr->name2 = SN_SPEED;
			rating += 30;
			t_ptr->ident |= ID_SHOW_P1;
			t_ptr->p1 = 1;
			t_ptr->cost += 1000000L;
		      }
		    }
		  else /* 2 - 5 */
		    {
		      if (strcmp("& Pair of Metal Shod Boots",
				 object_list[t_ptr->index].name))
			{
			  t_ptr->flags |= TR_STEALTH;
			  rating += 16;
			  t_ptr->ident |= ID_SHOW_P1;
			  t_ptr->p1 = randint(3);
			  t_ptr->name2 = SN_STEALTH;
			  t_ptr->cost += 500;
			}
		    }
		}
	    }
	  else if (magik(cursed))
	    {
	      tmp = randint(3);
	      if (tmp == 1)
		{
		  t_ptr->flags |= TR_SPEED;
		  t_ptr->name2 = SN_SLOWNESS;
		  t_ptr->ident |= ID_SHOW_P1;
		  t_ptr->p1 = -1;
		}
	      else if (tmp == 2)
		{
		  t_ptr->flags |= TR_AGGRAVATE;
		  t_ptr->name2 = SN_NOISE;
		}
	      else
		{
		  t_ptr->name2 = SN_GREAT_MASS;
		  t_ptr->weight = t_ptr->weight * 5;
		}
	      t_ptr->cost = 0;
	      t_ptr->toac -= m_bonus(2, 45, level);
	      t_ptr->flags |= TR_CURSED;
	    }
	  break;

	case TV_HELM:  /* Helms */
	  if ((t_ptr->subval >= 6) && (t_ptr->subval <= 8))
	    {
	      /* give crowns a higher chance for magic */
	      chance += t_ptr->cost / 100;
	      special += special;
	    }
	  if (magik(chance)||good)
	    {
	      t_ptr->toac += m_bonus(1, 20, level);
	      if (magik(special)||(good==666))
		{
		  if (t_ptr->subval < 6)
		    {
		      tmp = randint(12);
		      if (tmp < 3)
			{
			  if (!((randint(2) == 1) && !not_unique &&
				unique_armour(t_ptr))) {
			    if (peek) msg_print("Intelligence");
			    t_ptr->p1 = randint(2);
			    rating += 13;
			    t_ptr->flags |= TR_INT;
			    t_ptr->name2 = SN_INTELLIGENCE;
			    t_ptr->cost += t_ptr->p1*500;
			    t_ptr->ident |= ID_SHOW_P1;
			  }
			}
		      else if (tmp < 6)
			{
			  if (!((randint(2) == 1) && !not_unique &&
				unique_armour(t_ptr))) {
			    if (peek) msg_print("Wisdom");
			    rating += 13;
			    t_ptr->p1 = randint(2);
			    t_ptr->flags |= TR_WIS;
			    t_ptr->name2 = SN_WISDOM;
			    t_ptr->cost += t_ptr->p1*500;
			    t_ptr->ident |= ID_SHOW_P1;
			  }
			}
		      else if (tmp < 10)
			{
			  if (!((randint(2) == 1) && !not_unique &&
				unique_armour(t_ptr))) {
			    t_ptr->p1 = 1 + randint(4);
			    rating += 11;
			    t_ptr->flags |= TR_INFRA;
			    t_ptr->name2 = SN_INFRAVISION;
			    t_ptr->cost += t_ptr->p1*250;
			    t_ptr->ident |= ID_SHOW_P1;
			  }
			}
		      else if (tmp < 12)
			{
			  if (!((randint(2) == 1) && !not_unique &&
				unique_armour(t_ptr))) {
			    if (peek) msg_print("Light");
			    t_ptr->flags2 |= TR_LIGHT;
			    rating += 6;
			    t_ptr->name2 = SN_LIGHT;
			    t_ptr->cost += 500;
			  }
			}
		      else
			{
			  if (!((randint(2) == 1) && !not_unique &&
				unique_armour(t_ptr))) {
			    if (peek) msg_print("Telepathy");
			    rating += 20;
			    t_ptr->flags2 |= TR_TELEPATHY;
			    t_ptr->name2 = SN_TELEPATHY;
			    t_ptr->cost += 50000L;
			  }
			}
		    }
		  else
		    {
		      switch(randint(6))
			{
			case 1:
			  if (!(((randint(2) == 1) || (good == 666)) &&
				!not_unique && unique_armour(t_ptr))) {
				  if (peek) msg_print("Crown of Might");
				  t_ptr->ident |= ID_SHOW_P1;
				  rating += 19;
				  t_ptr->p1 = randint(3);
				  t_ptr->flags |= (TR_FREE_ACT|TR_CON|
						   TR_DEX|TR_STR);
				  t_ptr->name2 = SN_MIGHT;
				  t_ptr->cost += 1000 + t_ptr->p1*500;
				}
			  break;
			case 2:
			  t_ptr->ident |= ID_SHOW_P1;
			  if (peek) msg_print("Lordliness");
			  t_ptr->p1 = randint(3);
			  rating += 17;
			  t_ptr->flags |= (TR_CHR|TR_WIS);
			  t_ptr->name2 = SN_LORDLINESS;
			  t_ptr->cost += 1000 + t_ptr->p1*500;
			  break;
			case 3:
			  t_ptr->ident |= ID_SHOW_P1;
			  if (peek) msg_print("Crown of the Magi");
			  rating += 15;
			  t_ptr->p1 = randint(3);
			  t_ptr->flags |= (TR_RES_LIGHT|TR_RES_COLD
					   |TR_RES_ACID|TR_RES_FIRE|TR_INT);
			  t_ptr->name2 = SN_MAGI;
			  t_ptr->cost += 3000 + t_ptr->p1*500;
			  break;
			case 4:
			  t_ptr->ident |= ID_SHOW_P1;
			  rating += 8;
			  if (peek) msg_print("Beauty");
			  t_ptr->p1 = randint(3);
			  t_ptr->flags |= TR_CHR;
			  t_ptr->name2 = SN_BEAUTY;
			  t_ptr->cost += 750;
			  break;
			case 5:
			  t_ptr->ident |= ID_SHOW_P1;
			  if (peek) msg_print("Seeing");
			  rating += 8;
			  t_ptr->p1 = 5*(1 + randint(4));
			  t_ptr->flags |= (TR_SEE_INVIS|TR_SEARCH);
			  t_ptr->name2 = SN_SEEING;
			  t_ptr->cost += 1000 + t_ptr->p1*100;
			  break;
			case 6:
			  t_ptr->flags |= TR_REGEN;
			  rating += 10;
			  if (peek) msg_print("Regeneration");
			  t_ptr->name2 = SN_REGENERATION;
			  t_ptr->cost += 1500;
			  break;

			}
		    }
		}
	    }
	  else if (magik(cursed))
	    {
	      t_ptr->toac -= m_bonus(1, 45, level);
	      t_ptr->flags |= TR_CURSED;
	      t_ptr->cost = 0;
	      if (magik(special))
		switch(randint(7))
		  {
		  case 1:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = -randint (5);
		    t_ptr->flags |= TR_INT;
		    t_ptr->name2 = SN_STUPIDITY;
		    break;
		  case 2:
		  case 3:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = -randint (5);
		    t_ptr->flags |= TR_WIS;
		    t_ptr->name2 = SN_DULLNESS;
		    break;
		  case 4:
		  case 5:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = -randint (5);
		    t_ptr->flags |= TR_STR;
		    t_ptr->name2 = SN_WEAKNESS;
		    break;
		  case 6:
		    t_ptr->flags |= TR_TELEPORT;
		    t_ptr->name2 = SN_TELEPORTATION;
		    break;
		  case 7:
		    t_ptr->ident |= ID_SHOW_P1;
		    t_ptr->p1 = -randint (5);
		    t_ptr->flags |= TR_CHR;
		    t_ptr->name2 = SN_UGLINESS;
		    break;
		  }
	    }
	  break;

    case TV_RING: /* Rings	      */
      if (!((randint(10) == 1) && !not_unique && unique_armour(t_ptr))) {
	switch(t_ptr->subval)
	  {
	  case 0: case 1: case 2: case 3:
	    if (magik(cursed))
	      {
		t_ptr->p1 = -m_bonus(1, 20, level);
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    else
	      {
		t_ptr->p1 = m_bonus(1, 10, level);
		t_ptr->cost += t_ptr->p1*100;
	      }
	    break;
	  case 4:
	    if (magik(cursed))
	      {
		t_ptr->p1 = -randint(3);
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    else {
	      if (peek) msg_print("Ring of Speed");
	      rating += 35;
	      if (randint(888)==1)
		t_ptr->p1 = 2;
	      else
		t_ptr->p1 = 1;
	    }
	    break;
	  case 5:
	    t_ptr->p1 = 5 * m_bonus(1, 20, level);
	    t_ptr->cost += t_ptr->p1*30;
	    if (magik (cursed))
	      {
		t_ptr->p1 = -t_ptr->p1;
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    break;
	  case 19:     /* Increase damage	      */
	    t_ptr->todam += m_bonus(1, 20, level);
	    t_ptr->todam += 3 + randint(10);
	    t_ptr->cost += t_ptr->todam*100;
	    if (magik(cursed))
	      {
		t_ptr->todam = -t_ptr->todam;
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    break;
	  case 20:     /* Increase To-Hit	      */
	    t_ptr->tohit += m_bonus(1, 20, level);
	    t_ptr->tohit += 3 + randint(10);
	    t_ptr->cost += t_ptr->tohit*100;
	    if (magik(cursed))
	      {
		t_ptr->tohit = -t_ptr->tohit;
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    break;
	  case 21:     /* Protection	      */
	    t_ptr->toac += m_bonus(1, 20, level);
	    t_ptr->toac += 3 + randint(10);
	    t_ptr->cost += t_ptr->toac*100;
	    if (magik(cursed))
	      {
		t_ptr->toac = -t_ptr->toac;
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    break;
	  case 24: case 25: case 26:
	  case 27: case 28: case 29:
	    t_ptr->ident |= ID_NOSHOW_P1;
	    break;
	  case 30:     /* Slaying	      */
	    t_ptr->ident |= ID_SHOW_HITDAM;
	    t_ptr->todam += m_bonus(1, 25, level)*2;
	    t_ptr->todam += randint(3);
	    t_ptr->tohit += m_bonus(1, 25, level)*2;
	    t_ptr->tohit += randint(3);
	    t_ptr->cost += (t_ptr->tohit+t_ptr->todam)*100;
	    if (magik(cursed))
	      {
		t_ptr->tohit = -t_ptr->tohit;
		t_ptr->todam = -t_ptr->todam;
		t_ptr->flags |= TR_CURSED;
		t_ptr->cost = -t_ptr->cost;
	      }
	    break;
	  default:
	    break;
	  }
      }
      break;

	case TV_AMULET: /* Amulets	      */
	  if (t_ptr->subval < 2)
	    {
	      if (magik(cursed))
		{
		  t_ptr->p1 = -m_bonus(1, 20, level);
		  t_ptr->flags |= TR_CURSED;
		  t_ptr->cost = -t_ptr->cost;
		}
	      else
		{
		  t_ptr->p1 = m_bonus(1, 10, level);
		  t_ptr->cost += t_ptr->p1*100;
		}
	    }
	  else if (t_ptr->subval == 2)
	    {
	  t_ptr->p1 = 5 * m_bonus(1, 25, level);
	  if (magik(cursed))
	    {
	      t_ptr->p1 = -t_ptr->p1;
	      t_ptr->cost = -t_ptr->cost;
	      t_ptr->flags |= TR_CURSED;
	    }
	  else
	    t_ptr->cost += 20*t_ptr->p1;
	}
	  else if (t_ptr->subval == 8)
	    {
	      rating += 25;
	      /* amulet of the magi is never cursed */
	      t_ptr->p1 = 5 * m_bonus(1, 25, level);
	      t_ptr->cost += 20*t_ptr->p1;
	    }
	  break;

	  /* Subval should be even for store, odd for dungeon*/
	  /* Dungeon found ones will be partially charged	 */
	case TV_LIGHT:
	  if ((t_ptr->subval % 2) == 1)
	    {
	      t_ptr->p1 = randint(t_ptr->p1);
	      t_ptr->subval -= 1;
	    }
	  break;

	case TV_WAND:
	  switch(t_ptr->subval)
	    {
        case 0:	   t_ptr->p1 = randint(10) +	 6; break;
	case 1:	   t_ptr->p1 = randint(8)  +	 6; break;
	case 2:	   t_ptr->p1 = randint(5)  +	 6; break;
	case 3:	   t_ptr->p1 = randint(8)  +	 6; break;
	case 4:	   t_ptr->p1 = randint(4)  +	 3; break;
	case 5:	   t_ptr->p1 = randint(8)  +	 6; break;
	case 6:	   t_ptr->p1 = randint(20) +    12; break;
	case 7:	   t_ptr->p1 = randint(20) +    12; break;
	case 8:	   t_ptr->p1 = randint(10) +	 6; break;
	case 9:	   t_ptr->p1 = randint(12) +	  6; break;
	case 10:   t_ptr->p1 = randint(10) +    12; break;
	case 11:   t_ptr->p1 = randint(3)  +	 3; break;
	case 12:   t_ptr->p1 = randint(8)  +	 6; break;
	case 13:   t_ptr->p1 = randint(10) +	 6; break;
	case 14:   t_ptr->p1 = randint(5)  +	 3; break;
	case 15:   t_ptr->p1 = randint(5)  +	 3; break;
	case 16:   t_ptr->p1 = randint(5)  +	 6; break;
	case 17:   t_ptr->p1 = randint(5)  +	 4; break;
	case 18:   t_ptr->p1 = randint(8)  +	 4; break;
	case 19:   t_ptr->p1 = randint(6)  +	 2; break;
	case 20:   t_ptr->p1 = randint(4)  +	 2; break;
	case 21:   t_ptr->p1 = randint(8)  +	 6; break;
	case 22:   t_ptr->p1 = randint(5)  +	 2; break;
	case 23:   t_ptr->p1 = randint(12) +    12; break;
	case 24:   t_ptr->p1 = randint(3)  +     1; break;
	case 25:   t_ptr->p1 = randint(3)  +     1; break;
	case 26:   t_ptr->p1 = randint(3)  +     1; break;
	case 27:   t_ptr->p1 = randint(2)  +     1; break;
	case 28:   t_ptr->p1 = randint(8)  +     6; break;
	default:
	  break;
	}
      break;

    case TV_STAFF:
      switch(t_ptr->subval)
	{
	case 0:	  t_ptr->p1 = randint(20) +	 12; break;
	case 1:	  t_ptr->p1 = randint(8)  +	 6; break;
	case 2:	  t_ptr->p1 = randint(5)  +	 6; break;
	case 3:	  t_ptr->p1 = randint(20) +	 12; break;
	case 4:	  t_ptr->p1 = randint(15) +	 6; break;
	case 5:	  t_ptr->p1 = randint(4)  +	 5; break;
	case 6:	  t_ptr->p1 = randint(5)  +	 3; break;
	case 7:	  t_ptr->p1 = randint(3)  +	 1;
	  	  t_ptr->level = 10;
	  	  break;
	case 8:	  t_ptr->p1 = randint(3)  +	 1; break;
	case 9:	  t_ptr->p1 = randint(5)  +	 6; break;
	case 10:   t_ptr->p1 = randint(10) +	 12; break;
	case 11:   t_ptr->p1 = randint(5)  +	 6; break;
	case 12:   t_ptr->p1 = randint(5)  +	 6; break;
	case 13:   t_ptr->p1 = randint(5)  +	 6; break;
	case 14:   t_ptr->p1 = randint(10) +	 12; break;
	case 15:   t_ptr->p1 = randint(3)  +	 4; break;
	case 16:   t_ptr->p1 = randint(5)  +	 6; break;
	case 17:   t_ptr->p1 = randint(5)  +	 6; break;
	case 18:   t_ptr->p1 = randint(3)  +	 4; break;
	case 19:   t_ptr->p1 = randint(10) +	 12; break;
	case 20:   t_ptr->p1 = randint(3)  +	 4; break;
	case 21:   t_ptr->p1 = randint(3)  +	 4; break;
	case 22:   t_ptr->p1 = randint(10) + 6;
	  	   t_ptr->level = 5;
	  	   break;
	case 23:   t_ptr->p1 = randint(2)  +	1; break;
	case 24:   t_ptr->p1 = randint(3)  +	1; break;
	case 25:   t_ptr->p1 = randint(2)  +	2; break;
	case 26:   t_ptr->p1 = randint(15)  +	5; break;
	case 27:   t_ptr->p1 = randint(2)  +	2; break;
	case 28:   t_ptr->p1 = randint(5)  +	5; break;
	case 29:   t_ptr->p1 = randint(2)  +	1; break;
        case 30:   t_ptr->p1 = randint(6)  +    2; break;
	default: break;
	}
      break;

    case TV_CLOAK:
      if (magik(chance)||good) {
	  int made_art_cloak;
	  made_art_cloak=0;
	  if (magik(special)||(good==666))
	      {
		  if (!not_unique &&
		      !strcmp(object_list[t_ptr->index].name, "& Cloak")
		      && randint(10)==1) {
		      switch (randint(9)) {
		      case 1:
		      case 2:
			  if (COLLUIN) break;
			  if (wizard || peek) msg_print("Colluin");
			  else good_item_flag = TRUE;
			  t_ptr->name2 = SN_COLLUIN;
			  t_ptr->toac += 15;
			  t_ptr->flags |= (TR_RES_FIRE|TR_RES_COLD|
					   TR_RES_LIGHT|TR_RES_ACID);
			  t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
			  t_ptr->cost = 10000L;
			  made_art_cloak=1;
			  COLLUIN = 1;
			  break;
		      case 3:
		      case 4:
			  if (HOLCOLLETH) break;
			  if (wizard || peek) msg_print("Holcolleth");
			  else good_item_flag = TRUE;
			  t_ptr->name2 = SN_HOLCOLLETH;
			  t_ptr->toac += 4;
			  t_ptr->p1 = 2;
			  t_ptr->flags |= (TR_INT|TR_WIS|TR_STEALTH|
					   TR_RES_ACID);
			  t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
			  t_ptr->ident |= ID_SHOW_P1;
			  t_ptr->cost = 13000L;
			  made_art_cloak=1;
			  HOLCOLLETH = 1;
			  break;
		      case 5:
			  if (THINGOL) break;
			  if (wizard || peek) msg_print("Thingol");
			  else good_item_flag = TRUE;
                          t_ptr->name2 = SN_THINGOL;
			  t_ptr->toac += 18;
    			  t_ptr->flags = (TR_DEX|TR_CHR|TR_RES_FIRE|
				    TR_RES_ACID|TR_RES_COLD|TR_FREE_ACT);
			  t_ptr->flags2 |= (TR_ARTIFACT|TR_ACTIVATE);
			  t_ptr->p1 = 3;
			  t_ptr->cost = 35000L;
			  made_art_cloak=1;
			  THINGOL = 1;
                          break;
		      case 6:
		      case 7:
			  if (THORONGIL) break;
			  if (wizard || peek) msg_print("Thorongil");
			  else good_item_flag = TRUE;
			  t_ptr->name2 = SN_THORONGIL;
			  t_ptr->toac += 10;
			  t_ptr->flags = (TR_SEE_INVIS|TR_FREE_ACT|
					  TR_RES_ACID);
			  t_ptr->flags2 |= (TR_ARTIFACT);
			  t_ptr->cost = 8000L;
			  made_art_cloak=1;
			  THORONGIL = 1;
			  break;
		      case 8:
		      case 9:
			  if (COLANNON) break;
			  if (wizard || peek) msg_print("Colannon");
			  else good_item_flag = TRUE;
			  t_ptr->name2 = SN_COLANNON;
			  t_ptr->toac += 15;
			  t_ptr->flags |= (TR_STEALTH|TR_RES_ACID);
			  t_ptr->flags2 |= (TR_ACTIVATE|TR_ARTIFACT);
			  t_ptr->p1 = 3;
			  t_ptr->ident |= ID_SHOW_P1;
			  t_ptr->cost = 11000L;
			  made_art_cloak=1;
			  COLANNON = 1;
			  break;
		      }

		} else if (!not_unique &&
			   !strcmp(object_list[t_ptr->index].name,
			    "& Shadow Cloak")
			   && randint(20)==1) {
			   switch (randint(2)) {
			   case 1:
			     if (LUTHIEN) break;
			     if (wizard || peek) msg_print("Luthien");
                             else good_item_flag = TRUE;
			     t_ptr->name2 = SN_LUTHIEN;
			     t_ptr->toac += 20;
			     t_ptr->flags = (TR_RES_FIRE|TR_RES_COLD|
					TR_INT|TR_WIS|TR_CHR|TR_RES_ACID);
			     t_ptr->flags2 |= (TR_ARTIFACT|TR_ACTIVATE);
			     t_ptr->p1 = 2;
			     t_ptr->cost = 45000L;
			     made_art_cloak = 1;
			     LUTHIEN = 1;
			     break;
			   case 2:
			     if (TUOR) break;
			     if (wizard || peek) msg_print("Tuor");
			     else good_item_flag = TRUE;
			     t_ptr->name2 = SN_TUOR;
			     t_ptr->toac += 12;
			     t_ptr->flags = (TR_STEALTH|TR_IM_ACID|
					TR_FREE_ACT|TR_SEE_INVIS|TR_RES_ACID);
			     t_ptr->flags2 |= (TR_ARTIFACT);
			     t_ptr->p1 = 4;
			     t_ptr->ident = ID_SHOW_P1;
			     t_ptr->cost = 35000L;
			     made_art_cloak = 1;
			     TUOR = 1;
			     break;
			   }
		  }
		  if (!made_art_cloak) {
		      if (randint(2) == 1)
			  {
			      t_ptr->name2 = SN_PROTECTION;
			      t_ptr->toac += m_bonus(2, 40, level);
			      t_ptr->toac += (5 + randint(3));
			      t_ptr->cost += 250L;
			      rating += 8;
			  }
		      else if (randint(10) < 10)
			  {
			      t_ptr->toac += m_bonus(1, 20, level);
			      t_ptr->ident |= ID_SHOW_P1;
			      t_ptr->p1 = randint(3);
			      t_ptr->flags |= TR_STEALTH;
			      t_ptr->name2 = SN_STEALTH;
			      t_ptr->cost += 500;
			      rating += 9;
			  }
		      else
			  {
			      t_ptr->toac += 20;
			      t_ptr->p1 = randint(3);
			      t_ptr->ident |= ID_SHOW_P1;
			      t_ptr->flags |= (TR_STEALTH|TR_RES_ACID);
			      t_ptr->name2 = SN_AMAN;
			      t_ptr->cost += 5000;
			      rating += 16;
			  }
		  }
		  else
		      {
			  t_ptr->toac += m_bonus(1, 20, level);
		      }
	      }
      } else if (magik(cursed)) {
	  tmp = randint(3);
	  if (tmp == 1)
	    {
	      t_ptr->flags |= TR_AGGRAVATE;
	      t_ptr->name2 = SN_IRRITATION;
	      t_ptr->toac  -= m_bonus(1, 10, level);
	      t_ptr->ident |= ID_SHOW_HITDAM;
	      t_ptr->tohit -= m_bonus(1, 10, level);
	      t_ptr->todam -= m_bonus(1, 10, level);
	      t_ptr->cost =  0;
	    }
	  else if (tmp == 2)
	    {
	      t_ptr->name2 = SN_VULNERABILITY;
	      t_ptr->toac -= m_bonus(10, 100, level+50);
	      t_ptr->cost = 0;
	    }
	  else
	    {
	      t_ptr->name2 = SN_ENVELOPING;
	      t_ptr->toac  -= m_bonus(1, 10, level);
	      t_ptr->ident |= ID_SHOW_HITDAM;
	      t_ptr->tohit -= m_bonus(2, 40, level+10);
	      t_ptr->todam -= m_bonus(2, 40, level+10);
	      t_ptr->cost = 0;
	    }
	  t_ptr->flags |= TR_CURSED;
      }
      break;

    case TV_CHEST:
      switch(randint(level+4))
	{
	case 1:
	  t_ptr->flags = 0;
	  t_ptr->name2 = SN_EMPTY;
	  break;
	case 2:
	  t_ptr->flags |= CH_LOCKED;
	  t_ptr->name2 = SN_LOCKED;
	  break;
	case 3: case 4:
	  t_ptr->flags |= (CH_LOSE_STR|CH_LOCKED);
	  t_ptr->name2 = SN_POISON_NEEDLE;
	  break;
	case 5: case 6:
	  t_ptr->flags |= (CH_POISON|CH_LOCKED);
	  t_ptr->name2 = SN_POISON_NEEDLE;
	  break;
	case 7: case 8: case 9:
	  t_ptr->flags |= (CH_PARALYSED|CH_LOCKED);
	  t_ptr->name2 = SN_GAS_TRAP;
	  break;
	case 10: case 11:
	  t_ptr->flags |= (CH_EXPLODE|CH_LOCKED);
	  t_ptr->name2 = SN_EXPLOSION_DEVICE;
	  break;
	case 12: case 13: case 14:
	  t_ptr->flags |= (CH_SUMMON|CH_LOCKED);
	  t_ptr->name2 = SN_SUMMONING_RUNES;
	  break;
	case 15: case 16: case 17:
	  t_ptr->flags |= (CH_PARALYSED|CH_POISON|CH_LOSE_STR|CH_LOCKED);
	  t_ptr->name2 = SN_MULTIPLE_TRAPS;
	  break;
	default:
	  t_ptr->flags |= (CH_SUMMON|CH_EXPLODE|CH_LOCKED);
	  t_ptr->name2 = SN_MULTIPLE_TRAPS;
	  break;
	}
      break;

    case TV_SLING_AMMO: case TV_SPIKE:
    case TV_BOLT: case TV_ARROW:
      if (t_ptr->tval == TV_SLING_AMMO || t_ptr->tval == TV_BOLT
	  || t_ptr->tval == TV_ARROW)
	{
	  /* always show tohit/todam values if identified */
	  t_ptr->ident |= ID_SHOW_HITDAM;

	  if (magik(chance)||good)
	    {
	      t_ptr->tohit += m_bonus(1, 35, level);
	      t_ptr->todam += m_bonus(1, 35, level);
	      /* see comment for weapons */
	      if (magik(5*special/2)||(good==666))
		switch(randint(11))
		  {
		  case 1: case 2: case 3:
		    t_ptr->name2 = SN_SLAYING;
		    t_ptr->tohit += 5;
		    t_ptr->todam += 5;
		    t_ptr->cost += 20;
		    rating += 5;
		    break;
		  case 4: case 5:
		    t_ptr->flags |= TR_FLAME_TONGUE;
		    t_ptr->tohit += 2;
		    t_ptr->todam += 4;
		    t_ptr->name2 = SN_FIRE;
		    t_ptr->cost += 25;
		    rating += 6;
		    break;
		  case 6: case 7:
		    t_ptr->flags |= TR_SLAY_EVIL;
		    t_ptr->tohit += 3;
		    t_ptr->todam += 3;
		    t_ptr->name2 = SN_SLAY_EVIL;
		    t_ptr->cost += 25;
		    rating += 7;
		    break;
		  case 8: case 9:
		    t_ptr->flags |= TR_SLAY_ANIMAL;
		    t_ptr->tohit += 2;
		    t_ptr->todam += 2;
		    t_ptr->name2 = SN_SLAY_ANIMAL;
		    t_ptr->cost += 30;
		    rating += 5;
		    break;
		  case 10:
		    t_ptr->flags |= TR_SLAY_DRAGON;
		    t_ptr->tohit += 3;
		    t_ptr->todam += 3;
		    t_ptr->name2 = SN_DRAGON_SLAYING;
		    t_ptr->cost += 35;
		    rating += 9;
		    break;
		  case 11:
		    t_ptr->tohit += 15;
		    t_ptr->todam += 15;
		    t_ptr->name2 = SN_WOUNDING;
		    t_ptr->cost += 100;
		    rating += 10;
		    break;
		  }
	    }
	  else if (magik(cursed))
	    {
	      t_ptr->tohit -= m_bonus(5, 55, level);
	      t_ptr->todam -= m_bonus(5, 55, level);
	      t_ptr->flags |= TR_CURSED;
	      t_ptr->cost = 0;
	      if (randint(5)==1) {
		t_ptr->name2 |= SN_BACKBITING;
		t_ptr->tohit -= 20;
		t_ptr->todam -= 20;
	      }
	    }
	}

      t_ptr->number = 0;
      for (i = 0; i < 7; i++)
	t_ptr->number += randint(6);
      if (missile_ctr == MAX_SHORT)
	missile_ctr = -MAX_SHORT - 1;
      else
	missile_ctr++;
      t_ptr->p1 = missile_ctr;
      break;

    case TV_FOOD:
      /* make sure all food rations have the same level */
      if (t_ptr->subval == 90)
	t_ptr->level = 0;
      /* give all elvish waybread the same level */
      else if (t_ptr->subval == 92)
	t_ptr->level = 6;
      break;

    case TV_SCROLL1:
      /* give all identify scrolls the same level */
      if (t_ptr->subval == 67)
	t_ptr->level = 1;
      /* scroll of light */
      else if (t_ptr->subval == 69)
	t_ptr->level = 0;
      /* scroll of trap detection */
      else if (t_ptr->subval == 80)
	t_ptr->level = 5;
      /* scroll of door/stair location */
      else if (t_ptr->subval == 81)
	t_ptr->level = 5;
      break;

    case TV_POTION1:  /* potions */
      /* cure light */
      if (t_ptr->subval == 76)
	t_ptr->level = 0;
      break;

    default:
      break;
  }
}

static struct opt_desc { char *o_prompt; int8u *o_var; } options[] = {
  { "Running: cut known corners",		&find_cut },
  { "Running: examine potential corners",	&find_examine },
  { "Running: print self during run",		&find_prself },
  { "Running: stop when map sector changes",	&find_bound },
  { "Running: run through open doors",		&find_ignore_doors },
  { "(g)et-key to pickup objects",		&prompt_carry_flag },
  { "Rogue like commands",			&rogue_like_commands },
  { "Show weights in inventory",		&show_weight_flag },
  { "Highlight and notice mineral seams",	&highlight_seams },
#ifdef MSDOS
  { "Beep for invalid character",		&sound_beep_flag },
  { "Turn off haggling",			&no_haggle_flag },
#endif
  { 0, 0 } };


/* Set or unset various boolean options.		-CJS- */
void set_options()
{
  register int i, max;
  vtype string;

  prt("  ESC when finished, y/n to set options, <return> or - to move cursor",
		0, 0);
  for (max = 0; options[max].o_prompt != 0; max++)
    {
      (void) sprintf(string, "%-38s: %s", options[max].o_prompt,
		     (*options[max].o_var ? "yes" : "no "));
      prt(string, max+1, 0);
    }
  erase_line(max+1, 0);
  i = 0;
  for(;;)
    {
      move_cursor(i+1, 40);
      switch(inkey())
	{
	case ESCAPE:
	  return;
	case '-':
	  if (i > 0)
	    i--;
	  else
	    i = max-1;
	  break;
	case ' ':
	case '\n':
	case '\r':
	  if (i+1 < max)
	    i++;
	  else
	    i = 0;
	  break;
	case 'y':
	case 'Y':
	  put_buffer("yes", i+1, 40);
	  *options[i].o_var = TRUE;
	  if (i+1 < max)
	    i++;
	  else
	    i = 0;
	  break;
	case 'n':
	case 'N':
	  put_buffer("no ", i+1, 40);
	  *options[i].o_var = FALSE;
	  if (i+1 < max)
	    i++;
	  else
	    i = 0;
	  break;
	default:
	  bell();
	  break;
	}
    }
}
