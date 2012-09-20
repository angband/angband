/* generate.c: initialize/create a dungeon or town level

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"
#include "monster.h"

#ifdef USG
#if !defined(ATARIST_MWC)
#ifndef __TURBOC__
#include <memory.h>
#else
#include <mem.h>
#endif
#endif
#endif

typedef struct coords {
  int x, y;
} coords;

static coords doorstk[100];
static int doorindex;

static void build_type2();

int rating;

/* prototypes: ARG_* will collapse to nothing except on TURBOC -CFT */
static void place_broken_door(ARG_INT ARG_COMMA ARG_INT);
static void place_open_door(ARG_INT ARG_COMMA ARG_INT);
static void place_closed_door(ARG_INT ARG_COMMA ARG_INT);
static void place_stuck_door(ARG_INT ARG_COMMA ARG_INT);
static void place_locked_door(ARG_INT ARG_COMMA ARG_INT);
static void place_secret_door(ARG_INT ARG_COMMA ARG_INT);
static void place_up_stairs(ARG_INT ARG_COMMA ARG_INT);
static void place_down_stairs(ARG_INT ARG_COMMA ARG_INT);
static void vault_monster(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_trap(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_jelly(ARG_INT ARG_COMMA ARG_INT);
static void vault_orc(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_troll(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_undead(ARG_INT ARG_COMMA ARG_INT);
static void vault_dragon(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_demon(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void vault_nasty(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void correct_dir(ARG_INT_PTR ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void rand_dir(ARG_INT_PTR ARG_COMMA ARG_INT_PTR);
static int next_to(ARG_INT ARG_COMMA ARG_INT);
static void special_pit(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void build_type1(ARG_INT ARG_COMMA ARG_INT);
static void build_type2(ARG_INT ARG_COMMA ARG_INT);
static void build_type3(ARG_INT ARG_COMMA ARG_INT);
static void build_type5(ARG_INT ARG_COMMA ARG_INT);
static void build_pit(ARG_INT ARG_COMMA ARG_INT);
static void build_room(ARG_INT ARG_COMMA ARG_INT);
static void build_tunnel(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void fill_cave(ARG_INT);
static void place_streamer(ARG_INT ARG_COMMA ARG_INT);
static void place_boundry(ARG_VOID);
static void try_door(ARG_INT ARG_COMMA ARG_INT);
static void place_stairs(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void new_spot(ARG_INT16_PTR ARG_COMMA ARG_INT16_PTR);
static void build_store(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void tlink(ARG_VOID);
static void mlink(ARG_VOID);
static void blank_cave(ARG_VOID);
static void town_gen(ARG_VOID);
static void cave_gen(ARG_VOID);
static void place_boundary(ARG_VOID);
static void place_door(ARG_INT ARG_COMMA ARG_INT);

/* misc. prototypes */
void *malloc(ARG_INT16U);
void free(ARG_VOID_PTR);
int strcmp(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR);


/* Always picks a correct direction		*/
static void correct_dir(rdir, cdir, y1, x1, y2, x2)
int *rdir, *cdir;
register int y1, x1, y2, x2;
{
  if (y1 < y2)
    *rdir =  1;
  else if (y1 == y2)
    *rdir =  0;
  else
    *rdir = -1;
  if (x1 < x2)
    *cdir =  1;
  else if (x1 == x2)
    *cdir =  0;
  else
    *cdir = -1;
  if ((*rdir != 0) && (*cdir != 0))
    {
      if (randint (2) == 1)
	*rdir = 0;
      else
	*cdir = 0;
    }
}


/* Chance of wandering direction			*/
static void rand_dir(rdir, cdir)
int *rdir, *cdir;
{
  register int tmp;

  tmp = randint(4);
  if (tmp < 3)
    {
      *cdir = 0;
      *rdir = -3 + (tmp << 1); /* tmp=1 -> *rdir=-1; tmp=2 -> *rdir=1 */
    }
  else
    {
      *rdir = 0;
      *cdir = -7 + (tmp << 1); /* tmp=3 -> *cdir=-1; tmp=4 -> *cdir=1 */
    }
}


/* Blanks out entire cave				-RAK-	*/
static void blank_cave()
{
#ifndef USG
  bzero ((char *)&cave[0][0], sizeof (cave));
#else
#ifdef MAC
  /* On the mac, cave is a pointer, so sizeof(cave) = 4! */
  (void)memset((char *)&cave[0][0], 0, sizeof(cave_type)*MAX_HEIGHT*MAX_WIDTH);
#else
  (void)memset((char *)&cave[0][0], 0, sizeof (cave));
#endif
#endif
}


/* Fills in empty spots with desired rock		-RAK-	*/
/* Note: 9 is a temporary value.				*/
static void fill_cave(fval)
register int fval;
{
  register int i, j;
  register cave_type *c_ptr;

  /* no need to check the border of the cave */

  for (i = cur_height - 2; i > 0; i--)
    {
      c_ptr = &cave[i][1];
      for (j = cur_width - 2; j > 0; j--)
	{
	  if ((c_ptr->fval == NULL_WALL) || (c_ptr->fval == TMP1_WALL) ||
	      (c_ptr->fval == TMP2_WALL))
	    c_ptr->fval = fval;
	  c_ptr++;
	}
    }
}

/* Places indestructible rock around edges of dungeon	-RAK-	*/
static void place_boundary()
{
  register int i;
  register cave_type *top_ptr, *bottom_ptr;
  cave_type (*left_ptr)[MAX_WIDTH];
  cave_type (*right_ptr)[MAX_WIDTH];

  /* put permanent wall on leftmost row and rightmost row */
  left_ptr = (cave_type (*)[MAX_WIDTH]) &cave[0][0];
  right_ptr = (cave_type (*)[MAX_WIDTH]) &cave[0][cur_width - 1];

  for (i = 0; i < cur_height; i++)
    {
      ((cave_type *)left_ptr)->fval	= BOUNDARY_WALL;
      left_ptr++;
      ((cave_type *)right_ptr)->fval	= BOUNDARY_WALL;
      right_ptr++;
    }

  /* put permanent wall on top row and bottom row */
  top_ptr = &cave[0][0];
  bottom_ptr = &cave[cur_height - 1][0];

  for (i = 0; i < cur_width; i++)
    {
      top_ptr->fval	= BOUNDARY_WALL;
      top_ptr++;
      bottom_ptr->fval	= BOUNDARY_WALL;
      bottom_ptr++;
    }
}


/* Places "streamers" of rock through dungeon		-RAK-	*/
static void place_streamer(fval, treas_chance)
int fval;
int treas_chance;
{
  register int i, tx, ty;
  int y, x, t1, t2, dir;
  register cave_type *c_ptr;

  /* Choose starting point and direction		*/
  y = (cur_height / 2) + 11 - randint(23);
  x = (cur_width / 2)  + 16 - randint(33);

  dir = randint(8);	/* Number 1-4, 6-9	*/
  if (dir > 4)
    dir = dir + 1;

  /* Place streamer into dungeon			*/
  t1 = 2*DUN_STR_RNG + 1;	/* Constants	*/
  t2 =	 DUN_STR_RNG + 1;
  do
    {
      for (i = 0; i < DUN_STR_DEN; i++)
	{
	  ty = y + randint(t1) - t2;
	  tx = x + randint(t1) - t2;
	  if (in_bounds(ty, tx))
	    {
	      c_ptr = &cave[ty][tx];
	      if (c_ptr->fval == GRANITE_WALL)
		{
		  c_ptr->fval = fval;
		  if (randint(treas_chance) == 1)
		    place_gold(ty, tx);
		}
	    }
	}
    }
  while (mmove(dir, &y, &x));
}


static void place_open_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_OPEN_DOOR);
  cave_ptr->fval  = CORR_FLOOR;
  cave_ptr->lr  = TRUE;
}


static void place_broken_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_OPEN_DOOR);
  cave_ptr->fval  = CORR_FLOOR;
  t_list[cur_pos].p1 = 1;
  cave_ptr->lr  = TRUE;
}


static void place_closed_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_CLOSED_DOOR);
  cave_ptr->fval  = BLOCKED_FLOOR;
  cave_ptr->lr  = TRUE;
}


static void place_locked_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_CLOSED_DOOR);
  cave_ptr->fval  = BLOCKED_FLOOR;
  t_list[cur_pos].p1 = randint(10) + 10;
  cave_ptr->lr  = TRUE;
}


static void place_stuck_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_CLOSED_DOOR);
  cave_ptr->fval  = BLOCKED_FLOOR;
  t_list[cur_pos].p1 = -randint(10) - 10;
  cave_ptr->lr  = TRUE;
}


static void place_secret_door(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cur_pos = popt();
  cave_ptr = &cave[y][x];
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_SECRET_DOOR);
  cave_ptr->fval  = BLOCKED_FLOOR;
  cave_ptr->lr  = TRUE;
}


static void place_door(y, x)
int y, x;
{
  register int tmp;

  tmp = randint(8);
  if (tmp < 4)
    {
      if (randint(4) == 1)
	place_broken_door(y, x);
      else
	place_open_door(y, x);
    }
  else if (tmp < 7)
    {
      tmp = randint(100);
      if (tmp > 25)
	place_closed_door(y, x);
      else if (tmp == 3)
	place_stuck_door(y, x);
      else
	place_locked_door(y, x);
    }
  else
    place_secret_door(y, x);
}


/* Place an up staircase at given y, x			-RAK-	*/
static void place_up_stairs(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  cave_ptr = &cave[y][x];
  if (cave_ptr->tptr != 0)
    (void) delete_object(y, x);
  cur_pos = popt();
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_UP_STAIR);
}


/* Place a down staircase at given y, x			-RAK-	*/
static void place_down_stairs(y, x)
int y, x;
{
  register int cur_pos;
  register cave_type *cave_ptr;

  if (is_quest(dun_level)) {
    place_up_stairs(y, x);
    return;
  }
  cave_ptr = &cave[y][x];
  if (cave_ptr->tptr != 0)
    (void) delete_object(y, x);
  cur_pos = popt();
  cave_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_DOWN_STAIR);
}


/* Places a staircase 1=up, 2=down			-RAK-	*/
static void place_stairs(typ, num, walls)
int typ, num, walls;
{
  register cave_type *cave_ptr;
  int i, j, flag;
  register int y1, x1, y2, x2;

  for (i = 0; i < num; i++)
    {
      flag = FALSE;
      do
	{
	  j = 0;
	  do
	    {
	      /* Note: don't let y1/x1 be zero, and don't let y2/x2 be equal
		 to cur_height-1/cur_width-1, these values are always
		 BOUNDARY_ROCK. */
	      y1 = randint(cur_height - 14);
	      x1 = randint(cur_width  - 14);
	      y2 = y1 + 12;
	      x2 = x1 + 12;
	      do
		{
		  do
		    {
		      cave_ptr = &cave[y1][x1];
		      if (cave_ptr->fval <= MAX_OPEN_SPACE
			  && (cave_ptr->tptr == 0)
			  && (next_to_walls(y1, x1) >= walls))
			{
			  flag = TRUE;
			  if (typ == 1)
			    place_up_stairs(y1, x1);
			  else
			    place_down_stairs(y1, x1);
			}
		      x1++;
		    }
		  while ((x1 != x2) && (!flag));
		  x1 = x2 - 12;
		  y1++;
		}
	      while ((y1 != y2) && (!flag));
	      j++;
	    }
	  while ((!flag) && (j <= 30));
	  walls--;
	}
      while (!flag);
    }
}


/* Place a trap with a given displacement of point	-RAK-	*/
static void vault_trap(y, x, yd, xd, num)
int y, x, yd, xd, num;
{
  register int count, y1, x1;
  int i, flag;
  register cave_type *c_ptr;

  for (i = 0; i < num; i++)
    {
      flag = FALSE;
      count = 0;
      do
	{
	  y1 = y - yd - 1 + randint(2*yd+1);
	  x1 = x - xd - 1 + randint(2*xd+1);
	  c_ptr = &cave[y1][x1];
	  if ((c_ptr->fval != NULL_WALL) && (c_ptr->fval <= MAX_CAVE_FLOOR)
	      && (c_ptr->tptr == 0))
	    {
	      place_trap(y1, x1, randint(MAX_TRAP)-1);
	      flag = TRUE;
	    }
	  count++;
	}
      while ((!flag) && (count <= 5));
    }
}


/* Place a monster with a given displacement of point	-RAK-	*/
static void vault_monster(y, x, num)
int y, x, num;
{
  register int i;
  int y1, x1;

  for (i = 0; i < num; i++)
    {
      y1 = y;
      x1 = x;
      (void) summon_monster(&y1, &x1, TRUE);
    }
}

static void vault_jelly(y, x)
int y, x;
{
  int l, m, summon;

  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
      m = randint(l) - 1;
      if (((c_list[m].cchar == 'j') || (c_list[m].cchar == ',')
	 || (c_list[m].cchar == 'i') || (c_list[m].cchar == 'm'))
	 && !(c_list[m].cdefense & EVIL)) {
	summon = TRUE;
	place_monster(y, x, m, TRUE);
      }
    } while (!summon);
}

static void vault_orc(y, x, rank)
int y, x, rank;
{
  register int i;

  i=0;
  switch (rank) {
  case 1:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Snaga")) break;
      i++;
    }
    break;
  case 2: case 3:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Black orc")) break;
      i++;
    }
    break;
  case 4: case 5:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Uruk-Hai")) break;
      i++;
    }
    break;
  case 6:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Orc captain")) break;
      i++;
    }
    break;
  }
  place_monster(y,x,i,FALSE);
}

static void vault_troll(y, x, rank)
int y, x, rank;
{
  register int i;

  i=0;
  switch (rank) {
  case 1:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Forest troll")) break;
      i++;
    }
    break;
  case 2:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Stone troll")) break;
      i++;
    }
    break;
  case 3:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Ice troll")) break;
      i++;
    }
    break;
  case 4:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Cave troll")) break;
      i++;
    }
    break;
  case 5:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Water troll")) break;
      i++;
    }
    break;
  case 6:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Olog-Hai")) break;
      i++;
    }
    break;
  }
  place_monster(y,x,i,FALSE);
}

static void vault_undead(y, x)
int y, x;
{
  int l, m, summon;

  summon = FALSE;
  l = m_level[MAX_MONS_LEVEL];
  do {
      m = randint(l) - 1;
      if ((c_list[m].cdefense & UNDEAD) && !(c_list[m].cdefense & UNIQUE)) {
	summon = TRUE;
	place_monster(y, x, m, TRUE);
      }
    } while (!summon);
}


static void vault_dragon(y, x, rank, type)
int y, x, rank, type;
{
  register int i;

  i=0;
  switch (rank) {
  case 1: case 2: case 3:
    switch (type) {
    case 1:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young blue dragon")) break;
	i++;
      }
      break;
    case 2:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young white dragon")) break;
	i++;
      }
      break;
    case 3:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young green dragon")) break;
	i++;
      }
      break;
    case 4:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young black dragon")) break;
	i++;
      }
      break;
    case 5:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young red dragon")) break;
	i++;
      }
      break;
    case 6:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Young Multi-Hued Dragon")) break;
	i++;
      }
      break;
    }
    break;
  case 4: case 5:
    switch (type) {
    case 1:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature blue Dragon")) break;
	i++;
      }
      break;
    case 2:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature white Dragon")) break;
	i++;
      }
      break;
    case 3:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature green Dragon")) break;
	i++;
      }
      break;
    case 4:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature black Dragon")) break;
	i++;
      }
      break;
    case 5:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature red Dragon")) break;
	i++;
      }
      break;
    case 6:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Mature Multi-Hued Dragon")) break;
	i++;
      }
      break;
    }
    break;
  case 6:
    switch (type) {
    case 1:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient blue Dragon")) break;
	i++;
      }
      break;
    case 2:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient white Dragon")) break;
	i++;
      }
      break;
    case 3:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient green Dragon")) break;
	i++;
      }
      break;
    case 4:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient black Dragon")) break;
	i++;
      }
      break;
    case 5:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient red Dragon")) break;
	i++;
      }
      break;
    case 6:
      while (i<MAX_CREATURES) {
	if (!strcmp(c_list[i].name, "Ancient Multi-Hued Dragon")) break;
	i++;
      }
      break;
    }
    break;
  }
  place_monster(y,x,i,FALSE);
}

static void vault_demon(y, x, rank)
int y, x, rank;
{
  register int i;

  i=0;
  switch (rank) {
  case 1:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Vrock")) break;
      i++;
    }
    break;
  case 2:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Hezrou")) break;
      i++;
    }
    break;
  case 3:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Glabrezu")) break;
      i++;
    }
    break;
  case 4:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Nalfeshnee")) break;
      i++;
    }
    break;
  case 5:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Marilith")) break;
      i++;
    }
    break;
  case 6:
    while (i<MAX_CREATURES) {
      if (!strcmp(c_list[i].name, "Balor")) break;
      i++;
    }
    break;
  }
  place_monster(y,x,i,FALSE);
}


/* Builds a room at a row, column coordinate		-RAK-	*/
static void build_room(yval, xval)
int yval, xval;
{
  register int i, j, y_depth, x_right, x, y;
  int y_height, x_left, width,height,area;
  int8u floor;
  register cave_type *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/

  y_height = yval - randint(4);
  y_depth  = yval + randint(3);
  x_left   = xval - randint(11);
  x_right  = xval + randint(11);

/* the x dim of rooms tends to be much larger than the y dim, so don't bother
     rewriting the y loop */

  for (i = y_height; i <= y_depth; i++)
    {
      c_ptr = &cave[i][x_left];
      for (j = x_left; j <= x_right; j++)
	{
	  c_ptr->fval  = floor;
	  c_ptr->lr = TRUE;
	  c_ptr++;
	}
    }

  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      c_ptr = &cave[i][x_left-1];
      c_ptr->fval   = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr = &cave[i][x_right+1];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
    }

  c_ptr = &cave[y_height - 1][x_left];
  d_ptr = &cave[y_depth + 1][x_left];
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr++;
      d_ptr->fval   = GRANITE_WALL;
      d_ptr->lr = TRUE;
      d_ptr++;
    }

/* Every so often fill a normal room with pillars - Decado */

    if (randint(20)==2) {
      for (i=y_height; i<=y_depth; i+=2) {
	  for (j=x_left; j<=x_right; j+=2) {
	      c_ptr = &cave[i][j];
      	      c_ptr->fval = TMP1_WALL;
      	      c_ptr->lr = TRUE;
	  }
     }
  }

}

/* Builds a room at a row, column coordinate		-RAK-	*/
/* Type 1 unusual rooms are several overlapping rectangular ones	*/
static void build_type1(yval, xval)
int yval, xval;
{
  int y_height, y_depth;
  int x_left, x_right, limit;
  register int i0, i, j;
  int8u floor;
  register cave_type *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  limit = 1 + randint(2);
  for (i0 = 0; i0 < limit; i0++)
    {
      y_height = yval - randint(4);
      y_depth  = yval + randint(3);
      x_left   = xval - randint(11);
      x_right  = xval + randint(11);

      /* the x dim of rooms tends to be much larger than the y dim, so don't
	 bother rewriting the y loop */

      for (i = y_height; i <= y_depth; i++)
	{
	  c_ptr = &cave[i][x_left];
	  for (j = x_left; j <= x_right; j++)
	    {
	      c_ptr->fval  = floor;
	      c_ptr->lr = TRUE;
	      c_ptr++;
	    }
	}
      for (i = (y_height - 1); i <= (y_depth + 1); i++)
	{
	  c_ptr = &cave[i][x_left-1];
	  if (c_ptr->fval != floor)
	    {
	      c_ptr->fval  = GRANITE_WALL;
	      c_ptr->lr = TRUE;
	    }
	  c_ptr = &cave[i][x_right+1];
	  if (c_ptr->fval != floor)
	    {
	      c_ptr->fval  = GRANITE_WALL;
	      c_ptr->lr = TRUE;
	    }
	}
      c_ptr = &cave[y_height - 1][x_left];
      d_ptr = &cave[y_depth + 1][x_left];
      for (i = x_left; i <= x_right; i++)
	{
	  if (c_ptr->fval != floor)
	    {
	      c_ptr->fval  = GRANITE_WALL;
	      c_ptr->lr = TRUE;
	    }
	  c_ptr++;
	  if (d_ptr->fval != floor)
	    {
	      d_ptr->fval  = GRANITE_WALL;
	      d_ptr->lr = TRUE;
	    }
	  d_ptr++;
	}
    }
}

/* Wish me LUCK! - Decado */

static void build_type5(yval,xval)
int yval,xval;
{
  register int i,j,k,type,x,y,x1,y1,vault;
  int width,height;
  char *template;
  char buf[50];
  int8u floor;
  int8u wall;
  register cave_type *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;     /* Dark floor		*/


  vault=0;
  switch (randint(8)) {
  case 1:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template, "%s%s%s%s%s%s%s%s%s%s%s%s",
	    "       %%%%%%       ",
	    "    %%%..##..%%%    ",
	    "  %%....####....%%  ",
	    " %......#**#......% ",
	    "%...,.##+##+##.,...%",
	    "%.,.,.#*#*&#*#.,.,.%",
	    "%.,.,.#*#&*#*#.,.,.%",
	    "%...,.##+##+##.,...%",
	    " %......#**#......% ",
	    "  %%....####....%%  ",
	    "    %%%..##..%%%    ",
	    "       %%%%%%       ");
    break;
  case 2:
    width=20;
    height=14;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	    "   %%%%%%%%%%%%%%   ",
            "  %%.##########.%%  ",
            " %%..#..,,,,..#..%% ",
	    "%%,..#.,####,.#..,%%",
	    "%....#.,#**#,.#....%",
	    "%.###+,##&&##,+###.%",
	    "%.#..,,#*&**#,,..#.%",
	    "%.#..,,#**&*#,,..#.%",
	    "%.###+,##&&##,+###.%",
	    "%....#.,#**#,.#....%",
	    "%%,..#.,####,.#..,%%",
            " %%..#..,,,,..#..%% ",
	    "  %%.##########.%%  ",
            "   %%%%%%%%%%%%%%   ");
    break;
  case 3:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s",
	    "    %%%%%%%%%%%%    ",
	    " %%%%..........%%%% ",
	    " %...###+##+###...% ",
	    "%%...#,,#,,#,,#...%%",
	    "%.###+##+##+##+###.%",
	    "%.#,,#&&#**#&&#,,#.%",
	    "%.#,,#&&#**#&&#,,#.%",
	    "%.###+##+##+##+###.%",
	    "%%...#,,#,,#,,#...%%",
	    " %...###+##+###...% ",
	    " %%%%..........%%%% ",
	    "    %%%%%%%%%%%%    ");
    break;
  case 4:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s",
	    "%%%%%%%%%%%%%%%%%%%%",
	    "%*.......&........*%",
	    "%.################.%",
	    "%.#,.,.,.,.,.,.,.#.%",
	    "%.#.############,#.%",
	    "%.#,+,&&+**#&*,#.#&%",
	    "%&#.#,*&#**+&&,+,#.%",
	    "%.#,############.#.%",
	    "%.#.,.,.,.,.,.,.,#.%",
	    "%.################.%",
	    "%*........&.......*%",
	    "%%%%%%%%%%%%%%%%%%%%");
    break;
  case 5:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s",
	    "%%%%%%%%%%%%%%%%%   ",
	    "%,,,##,,,,##....%%  ",
	    "%,,,,##,,,,##....%% ",
	    "%#,,,,##,,,,##....%%",
	    "%##,,,,##,,,,##....%",
	    "%.##,,,,,,,,,,#+...%",
	    "%..#+,,,,,,,,,,##..%",
	    "%...##,,,,##,,,,##.%",
	    "%%...##,,,,##,,,,##%",
	    " %%...##,,,,##,,,,#%",
	    "  %%...##,,,,##,,,,%",
	    "   %%%%%%%%%%%%%%%%%");
    break;
  case 6:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s",
	    "   %%%%%%%%%%%%%%%%%",
	    "  %%....##,,,,##,,,%",
	    " %%....##,,,,##,,,,%",
	    "%%....##,,,,##,,,,#%",
            "%....##,,,,##,,,,##%",
            "%...+#,,,,,,,,,,##.%",
            "%..##,,,,,,,,,,+#..%",
            "%.##,,,,##,,,,##...%",
            "%##,,,,##,,,,##...%%",
            "%#,,,,##,,,,##...%% ",
            "%,,,,##,,,,##...%%  ",
            "%%%%%%%%%%%%%%%%%   ");
    break;
  case 7:
    width=20;
    height=12;
    template = (char *)malloc(width*height+1);
    rating += 5;
    sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s",
	    "%%%%%%%%%%%%%%%%%%%%",
	    "%,################,%",
	    "%^#.*...&..,....,#^%",
	    "%^#...,......&...#^%",
	    "%^#######++#######^%",
	    "%^+.,..&+,*+*....+^%",
	    "%^+..*.,+.&+.,.&.+^%",
	    "%^#######++#######^%",
	    "%^#....,.,.....,.#^%",
	    "%^#..&......*....#^%",
	    "%,################,%",
	    "%%%%%%%%%%%%%%%%%%%%");
    break;
  case 8:
    vault=TRUE;
    switch (randint(4)) {
      case 4:
	width=40;
	height=18;
	template = (char *)malloc(width*height+1);
	rating += 25;
	sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
		"%######################################%",
		"%#*8..&##***++^^^^^^^^^^^^++***##&..*8#%",
		"%#8..&##,,,,,##^^^^^^^^^^##,,,,,X#&..*#%",
		"%#..&X#.....,.##^^^^^^^^##..&....##&..#%",
		"%#.&##..,.&....##^^^^^^##..,...&..##&.#%",
		"%#&##..*...&.^..##^^^^##..*....,..,##&#%",
		"%####+#############++#############+####%",
		"%+....,.,.#&&&&***+88+***&&&&#,.,.,...+%",
		"%+...,.,.,#&&&&***+88+***&&&&#.,.,....+%",
		"%####+#############++#############+####%",
		"%#&##..*....&...##^^^^##...*...&,..X#&#%",
		"%#.&##..&.^....##^^^^^^##....&....##&.#%",
		"%#..&##....&..##^^^^^^^^##..,..*.##&..#%",
		"%#*..&X#,,,,,##^^^^^^^^^^##,,,,,##&..8#%",
		"%#8*..&##***++^^^^^^^^^^^^++***##&..*8#%",
		"%######################################%",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
	break;
      case 3:
	width=39;
	height=17;
	template = (char *)malloc(width*height+1);
	rating += 35;
	sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
		"%#####################################%",
		"%+&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
		"%###################################X#%",
		"%#OX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&#%",
		"%#X###################################%",
		"%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
		"%###################################X#%",
		"%#OX&XOX&XOX&XOOOOOOOOOOOXOX&XOX&XOX&#%",
		"%#X###################################%",
		"%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XO#%",
		"%###################################X#%",
		"%#OX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&#%",
		"%#X###################################%",
		"%#&XOX&XOX&XOX&XOX&XOX&XOX&XOX&XOX&X&+%",
		"%#####################################%",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
	break;
      case 2:
	width=40;
	height=18;
	template = (char *)malloc(width*height+1);
	rating += 30;
	sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
		"%######################################%",
		"%#,X,X,X,X,X,X,X,X*@@*X,X,X,X,X,X,X,X,#%",
		"%#+################XX################+#%",
		"%#.,..,.#&.&.,*##******##*,.&.&#.,...,X%",
		"%#..,.^^#....,##***@@***##,....#^^..,.X%",
		"%######+#^&.&##***@XX@***##&.&^#+######%",
		"%#,.&.^^#+####***@X##X@***####+#^^.,..#%",
		"%#..,&,.#^^^@#**@X#OO#X@**X@^^^#.,..&,#%",
		"%#.,....#^^^@X**@X#OO#X@**#@^^^#.&.,..#%",
		"%#...,^^#+####***@X##X@***####+#^^..,.#%",
		"%######+#^&.&##***@XX@***##&.&^#+######%",
		"%X.,..^^#.....##***@@***##,....#^^.,..#%",
		"%X...,..#&.&.,*##******##*,.&.&#..,..,#%",
		"%#+################XX################+#%",
		"%#,X,X,X,X,X,X,X,X*@@*X,X,X,X,X,X,X,X,#%",
		"%######################################%",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
	break;
      case 1:
    	width=40;
     	height=15;
	template = (char *)malloc(width*height+1);
	rating += 25;
        sprintf(template,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
                "%&+.^..^..^..^..^..^..^..^..^..^..^..+&%",
		"%+####################################+%",
		"%.#.&.^,#&^&^#****+^*^@^#.*.&..#..*.,#.%",
		"%^#.,.&^+^&^@#^^^^#@^*^*#....*^+.^...#^%",
		"%.#*..,.###+####+####+###.&.^..#..&,.#.%",
		"%^#..^.*#*..^&&@@*#,,,,,####+###,....#^%",
		"%.##+##############,*O*,#,,,,,,###+###.%",
		"%^#*&#.&,*.#,*&^*^#,,,,,#,,,,,,#....,#^%",
		"%.#&,+....*+,*&^*^##########+###.,...+.%",
		"%^#.,#.*.&.#,*&^*^+.,.&.^*.&^&^#.....#^%",
		"%.#^*#.,..,#,*&^*^#*.^*.,..&&&^#,..,.#.%",
		"%+####################################+%",
		"%&+..^..^..^..^..^..^..^..^..^..^..^.+&%",
		"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
        break;
    }
    break;
  }

  /* check these rooms will fit on the map! */

  wall=TMP1_WALL;
  if (vault) {
    xval+=4;  /* neat kludge deccy... */
    yval+=4;
    wall=BOUNDARY_WALL;
    if (floor==LIGHT_FLOOR)
      floor=NT_LIGHT_FLOOR;
    else
      floor=NT_DARK_FLOOR;
  }

  /* DO NOT CHANGE yval or xval after this check... */
  if ( !( in_bounds(yval-(height/2),xval-(width/2)) &&
	  in_bounds(yval+(height/2),xval+(width/2)) )) {
	  void build_type2();

	  free(template);
	  build_type2(yval, xval);
	  return;
  }

  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      x1=xval-(width/2)+x;
      y1=yval-(height/2)+y;
      c_ptr = &cave[y1][x1];
      switch (*template++) {
      case '#': /* lit up wall */
	c_ptr->fval = wall;
	c_ptr->lr = TRUE;
	break;
      case 'X':
        c_ptr->fval = TMP1_WALL;
        c_ptr->lr = TRUE;
        break;
      case '%': /* lit up wall */
	c_ptr->fval = GRANITE_WALL; /* Not temp, since this may have doors in*/
	c_ptr->lr = TRUE;
	break;
      case '.': /* lit up/ not lit up floor */
	c_ptr->fval = floor;
	c_ptr->lr = TRUE;
	break;
      case '*': /* treasure/trap */
	c_ptr->fval = floor;
	if (randint(20)>7)
	  place_object(y1, x1);
	else if (randint(10)>2)
	  place_trap(y1, x1, randint(MAX_TRAP)-1);
	else if (randint(2)==1 && !vault)
	  place_down_stairs(y1, x1);
	else if (!vault)
	  place_up_stairs(y1, x1);
	c_ptr->lr = TRUE;
	break;
      case '+': /* secret doors */
	place_secret_door(y1, x1);
	c_ptr->lr = TRUE;
	break;
      case '&': /* Monster */
	c_ptr->fval = floor;
	place_monster(y1, x1,
		get_mons_num(dun_level+MON_SUMMON_ADJ+2+vault),TRUE);
	c_ptr->lr = TRUE;
	break;
      case '@': /* Meaner monster */
	c_ptr->fval = floor;
	place_monster(y1, x1, get_nmons_num(dun_level+MON_SUMMON_ADJ+7),TRUE);
	c_ptr->lr = TRUE;
	break;
      case '8': /* Meaner monster */
	c_ptr->fval = floor;
	place_monster(y1, x1, get_nmons_num(dun_level+MON_SUMMON_ADJ+7),TRUE);
        place_special(y1,x1,1);
	c_ptr->lr = TRUE;
	break;
      case 'O': /* Nasty monster and treasure */
	c_ptr->fval = floor;
        place_monster(y1,x1,get_nmons_num(dun_level+MON_SUMMON_ADJ+40),TRUE);
        place_special(y1,x1,666);
	c_ptr->lr = TRUE;
	break;
      case ',': /* Monster/object */
	c_ptr->fval = floor;
	if (randint(2)==1)
 		place_monster(y1, x1,
		get_mons_num(dun_level+MON_SUMMON_ADJ+vault), TRUE);
	if (randint(2)==1)
	  	place_object(y1, x1);
	c_ptr->lr = TRUE;
	break;
      case '^':
	c_ptr->fval = floor;
	place_trap(y1, x1, randint(MAX_TRAP)-1);
	c_ptr->lr = TRUE;
	break;
      case ' ':
	break;
      default:
	sprintf(buf, "Cockup='%c'", *template);
	msg_print(buf);
	break;
      }
    }
  }
  free(template);
}

/* Builds an unusual room at a row, column coordinate	-RAK-	*/
/* Type 2 unusual rooms all have an inner room:			*/
/*   1 - Just an inner room with one door			*/
/*   2 - An inner room within an inner room			*/
/*   3 - An inner room with pillar(s)				*/
/*   4 - Inner room has a maze					*/
/*   5 - A set of four inner rooms				*/
static void build_type2(yval, xval)
int yval, xval;
{
  register int i, j, y_height, x_left;
  int y_depth, x_right, tmp;
  int8u floor;
  register cave_type *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  y_height = yval - 4;
  y_depth  = yval + 4;
  x_left   = xval - 11;
  x_right  = xval + 11;

  /* the x dim of rooms tends to be much larger than the y dim, so don't bother
     rewriting the y loop */

  for (i = y_height; i <= y_depth; i++)
    {
      c_ptr = &cave[i][x_left];
      for (j = x_left; j <= x_right; j++)
	{
	  c_ptr->fval  = floor;
	  c_ptr->lr = TRUE;
	  c_ptr++;
	}
    }
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      c_ptr = &cave[i][x_left-1];
      c_ptr->fval   = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr = &cave[i][x_right+1];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
    }
  c_ptr = &cave[y_height - 1][x_left];
  d_ptr = &cave[y_depth + 1][x_left];
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr++;
      d_ptr->fval   = GRANITE_WALL;
      d_ptr->lr = TRUE;
      d_ptr++;
    }
  /* The inner room		*/
  y_height = y_height + 2;
  y_depth  = y_depth  - 2;
  x_left   = x_left   + 2;
  x_right  = x_right  - 2;
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      cave[i][x_left-1].fval = TMP1_WALL;
      cave[i][x_right+1].fval = TMP1_WALL;
    }
  c_ptr = &cave[y_height-1][x_left];
  d_ptr = &cave[y_depth+1][x_left];
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr->fval = TMP1_WALL;
      c_ptr++;
      d_ptr->fval = TMP1_WALL;
      d_ptr++;
    }
  /* Inner room variations		*/
  switch(randint(5))
    {
    case 1:	/* Just an inner room.	*/
      tmp = randint(4);
      if (tmp < 3) {	/* Place a door	*/
	if (tmp == 1) place_secret_door(y_height-1, xval);
	else	      place_secret_door(y_depth+1, xval);
      } else {
	if (tmp == 3) place_secret_door(yval, x_left-1);
	else	      place_secret_door(yval, x_right+1);
      }
      vault_monster(yval, xval, 1);
      break;

    case 2:	/* Treasure Vault	*/
      tmp = randint(4);
      if (tmp < 3) {	/* Place a door	*/
	if (tmp == 1) place_secret_door(y_height-1, xval);
	else	      place_secret_door(y_depth+1, xval);
      } else {
	if (tmp == 3) place_secret_door(yval, x_left-1);
	else	      place_secret_door(yval, x_right+1);
      }

      for (i = yval-1; i <= yval+1; i++)
	{
	  cave[i][xval-1].fval	 = TMP1_WALL;
	  cave[i][xval+1].fval	 = TMP1_WALL;
	}
      cave[yval-1][xval].fval  = TMP1_WALL;
      cave[yval+1][xval].fval  = TMP1_WALL;

      tmp = randint(4);	/* Place a door	*/
      if (tmp < 3)
	place_locked_door(yval-3+(tmp<<1), xval); /* 1 -> yval-1; 2 -> yval+1*/
      else
	place_locked_door(yval, xval-7+(tmp<<1));

      /* Place an object in the treasure vault	*/
      tmp = randint(10);
      if (tmp > 2)
	place_object(yval, xval);
      else if (tmp == 2)
	place_down_stairs(yval, xval);
      else
	place_up_stairs(yval, xval);

      /* Guard the treasure well		*/
      vault_monster(yval, xval, 2+randint(3));
      /* If the monsters don't get 'em.	*/
      vault_trap(yval, xval, 4, 10, 2+randint(3));
      break;

    case 3:	/* Inner pillar(s).	*/
      tmp = randint(4);
      if (tmp < 3) {	/* Place a door	*/
	if (tmp == 1) place_secret_door(y_height-1, xval);
	else	      place_secret_door(y_depth+1, xval);
      } else {
	if (tmp == 3) place_secret_door(yval, x_left-1);
	else	      place_secret_door(yval, x_right+1);
      }

      for (i = yval-1; i <= yval+1; i++)
	{
	  c_ptr = &cave[i][xval-1];
	  for (j = xval-1; j <= xval+1; j++)
	    {
	      c_ptr->fval = TMP1_WALL;
	      c_ptr++;
	    }
	}
      if (randint(2) == 1)
	{
	  tmp = randint(2);
	  for (i = yval-1; i <= yval+1; i++)
	    {
	      c_ptr = &cave[i][xval-5-tmp];
	      for (j = xval-5-tmp; j <= xval-3-tmp; j++)
		{
		  c_ptr->fval = TMP1_WALL;
		  c_ptr++;
		}
	    }
	  for (i = yval-1; i <= yval+1; i++)
	    {
	      c_ptr = &cave[i][xval+3+tmp];
	      for (j = xval+3+tmp; j <= xval+5+tmp; j++)
		{
		  c_ptr->fval  = TMP1_WALL;
		  c_ptr++;
		}
	    }
	}

      if (randint(3) == 1)	/* Inner rooms	*/
	{
	  c_ptr = &cave[yval-1][xval-5];
	  d_ptr = &cave[yval+1][xval-5];
	  for (i = xval-5; i <= xval+5; i++)
	    {
	      c_ptr->fval  = TMP1_WALL;
	      c_ptr++;
	      d_ptr->fval  = TMP1_WALL;
	      d_ptr++;
	    }
	  cave[yval][xval-5].fval = TMP1_WALL;
	  cave[yval][xval+5].fval = TMP1_WALL;
	  place_secret_door(yval-3+(randint(2)<<1), xval-3);
	  place_secret_door(yval-3+(randint(2)<<1), xval+3);
	  if (randint(3) == 1)	place_object(yval, xval-2);
	  if (randint(3) == 1)	place_object(yval, xval+2);
	  vault_monster(yval, xval-2, randint(2));
	  vault_monster(yval, xval+2, randint(2));
	}
      break;

    case 4:	/* Maze inside.	*/
      tmp = randint(4);
      if (tmp < 3) {	/* Place a door	*/
	if (tmp == 1) place_secret_door(y_height-1, xval);
	else	      place_secret_door(y_depth+1, xval);
      } else {
	if (tmp == 3) place_secret_door(yval, x_left-1);
	else	      place_secret_door(yval, x_right+1);
      }

      for (i = y_height; i <= y_depth; i++)
	for (j = x_left; j <= x_right; j++)
	  if (0x1 & (j+i))
	    cave[i][j].fval = TMP1_WALL;

      /* Monsters just love mazes.		*/
      vault_monster(yval, xval-5, randint(3));
      vault_monster(yval, xval+5, randint(3));
      /* Traps make them entertaining.	*/
      vault_trap(yval, xval-3, 2, 8, randint(3));
      vault_trap(yval, xval+3, 2, 8, randint(3));
      /* Mazes should have some treasure too..	*/
      for (i = 0; i < 3; i++)
	random_object(yval, xval, 1);
      break;

    case 5:	/* Four small rooms.	*/
      for (i = y_height; i <= y_depth; i++)
	cave[i][xval].fval = TMP1_WALL;

      c_ptr = &cave[yval][x_left];
      for (i = x_left; i <= x_right; i++)
	{
	  c_ptr->fval = TMP1_WALL;
	  c_ptr++;
	}

      if (randint(2) == 1)
	{
	  i = randint(10);
	  place_secret_door(y_height-1, xval-i);
	  place_secret_door(y_height-1, xval+i);
	  place_secret_door(y_depth+1, xval-i);
	  place_secret_door(y_depth+1, xval+i);
	}
      else
	{
	  i = randint(3);
	  place_secret_door(yval+i, x_left-1);
	  place_secret_door(yval-i, x_left-1);
	  place_secret_door(yval+i, x_right+1);
	  place_secret_door(yval-i, x_right+1);
	}

      /* Treasure in each one.		*/
      random_object(yval, xval, 2+randint(2));
      /* Gotta have some monsters.		*/
      vault_monster(yval+2, xval-4, randint(2));
      vault_monster(yval+2, xval+4, randint(2));
      vault_monster(yval-2, xval-4, randint(2));
      vault_monster(yval-2, xval+4, randint(2));
      break;
    }
}

/* Builds a room at a row, column coordinate		-RAK-	*/
/* Type 3 unusual rooms are cross shaped				*/
static void build_type3(yval, xval)
int yval, xval;
{
  int y_height, y_depth;
  int x_left, x_right;
  register int tmp, i, j;
  int8u floor;
  register cave_type *c_ptr;

  if (dun_level <= randint(25))
    floor = LIGHT_FLOOR;	/* Floor with light	*/
  else
    floor = DARK_FLOOR;		/* Dark floor		*/
  tmp = 2 + randint(2);
  y_height = yval - tmp;
  y_depth  = yval + tmp;
  x_left   = xval - 1;
  x_right  = xval + 1;
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++)
      {
	c_ptr = &cave[i][j];
	c_ptr->fval = floor;
	c_ptr->lr = TRUE;
      }
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      c_ptr = &cave[i][x_left-1];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr = &cave[i][x_right+1];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
    }
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr = &cave[y_height-1][i];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr = &cave[y_depth+1][i];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
    }
  tmp = 2 + randint(9);
  y_height = yval - 1;
  y_depth  = yval + 1;
  x_left   = xval - tmp;
  x_right  = xval + tmp;
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++)
      {
	c_ptr = &cave[i][j];
	c_ptr->fval = floor;
	c_ptr->lr = TRUE;
      }
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      c_ptr = &cave[i][x_left-1];
      if (c_ptr->fval != floor)
	{
	  c_ptr->fval  = GRANITE_WALL;
	  c_ptr->lr = TRUE;
	}
      c_ptr = &cave[i][x_right+1];
      if (c_ptr->fval != floor)
	{
	  c_ptr->fval  = GRANITE_WALL;
	  c_ptr->lr = TRUE;
	}
    }
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr = &cave[y_height-1][i];
      if (c_ptr->fval != floor)
	{
	  c_ptr->fval  = GRANITE_WALL;
	  c_ptr->lr = TRUE;
	}
      c_ptr = &cave[y_depth+1][i];
      if (c_ptr->fval != floor)
	{
	  c_ptr->fval  = GRANITE_WALL;
	  c_ptr->lr = TRUE;
	}
    }
  /* Special features.			*/
  switch(randint(4))
    {
    case 1:	/* Large middle pillar		*/
      for (i = yval-1; i <= yval+1; i++)
	{
	  c_ptr = &cave[i][xval-1];
	  for (j = xval-1; j <= xval+1; j++)
	    {
	      c_ptr->fval = TMP1_WALL;
	      c_ptr++;
	    }
	}
      break;

    case 2:	/* Inner treasure vault		*/
      for (i = yval-1; i <= yval+1; i++)
	{
	  cave[i][xval-1].fval	 = TMP1_WALL;
	  cave[i][xval+1].fval	 = TMP1_WALL;
	}
      cave[yval-1][xval].fval  = TMP1_WALL;
      cave[yval+1][xval].fval  = TMP1_WALL;

      tmp = randint(4);	/* Place a door	*/
      if (tmp < 3)
	place_secret_door(yval-3+(tmp<<1), xval);
      else
	place_secret_door(yval, xval-7+(tmp<<1));

      /* Place a treasure in the vault		*/
      place_object(yval, xval);
      /* Let's guard the treasure well.	*/
      vault_monster(yval, xval, 2+randint(2));
      /* Traps naturally			*/
      vault_trap(yval, xval, 4, 4, 1+randint(3));
      break;

    case 3:
      if (randint(3) == 1)
	{
	  cave[yval-1][xval-2].fval = TMP1_WALL;
	  cave[yval+1][xval-2].fval = TMP1_WALL;
	  cave[yval-1][xval+2].fval = TMP1_WALL;
	  cave[yval+1][xval+2].fval = TMP1_WALL;
	  cave[yval-2][xval-1].fval = TMP1_WALL;
	  cave[yval-2][xval+1].fval = TMP1_WALL;
	  cave[yval+2][xval-1].fval = TMP1_WALL;
	  cave[yval+2][xval+1].fval = TMP1_WALL;
	  if (randint(3) == 1)
	    {
	      place_secret_door(yval, xval-2);
	      place_secret_door(yval, xval+2);
	      place_secret_door(yval-2, xval);
	      place_secret_door(yval+2, xval);
	    }
	}
      else if (randint(3) == 1)
	{
	  cave[yval][xval].fval = TMP1_WALL;
	  cave[yval-1][xval].fval = TMP1_WALL;
	  cave[yval+1][xval].fval = TMP1_WALL;
	  cave[yval][xval-1].fval = TMP1_WALL;
	  cave[yval][xval+1].fval = TMP1_WALL;
	}
      else if (randint(3) == 1)
	cave[yval][xval].fval = TMP1_WALL;
      break;

    case 4:
      break;
    }
}

static void vault_nasty(j,i,type,rank,colour)
int j,i,type,rank,colour;
{
  switch (type) {
  case 1:
    vault_jelly(j,i);
    break;
  case 2:
    vault_orc(j,i,rank);
    break;
  case 3:
    vault_troll(j,i,rank);
    break;
  case 4:
    vault_undead(j,i);
    break;
  case 5:
    vault_dragon(j,i,rank,colour);
    break;
  case 6:
    vault_demon(j,i,rank);
    break;
  }
}

static void special_pit(yval, xval, type)
int yval, xval, type;
{
  register int i, j, y_height, x_left;
  int y_depth, x_right, tmp, colour;
  int8u floor;
  register cave_type *c_ptr, *d_ptr;

  floor = DARK_FLOOR;
  y_height = yval - 4;
  y_depth  = yval + 4;
  x_left   = xval - 11;
  x_right  = xval + 11;

  /* the x dim of rooms tends to be much larger than the y dim, so don't bother
     rewriting the y loop */

  for (i = y_height; i <= y_depth; i++)
    {
      c_ptr = &cave[i][x_left];
      for (j = x_left; j <= x_right; j++)
	{
	  c_ptr->fval  = floor;
	  c_ptr->lr = TRUE;
	  c_ptr++;
	}
    }
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      c_ptr = &cave[i][x_left-1];
      c_ptr->fval   = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr = &cave[i][x_right+1];
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
    }
  c_ptr = &cave[y_height - 1][x_left];
  d_ptr = &cave[y_depth + 1][x_left];
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr->fval  = GRANITE_WALL;
      c_ptr->lr = TRUE;
      c_ptr++;
      d_ptr->fval   = GRANITE_WALL;
      d_ptr->lr = TRUE;
      d_ptr++;
    }
  /* The inner room		*/
  y_height = y_height + 2;
  y_depth  = y_depth  - 2;
  x_left   = x_left   + 2;
  x_right  = x_right  - 2;
  for (i = (y_height - 1); i <= (y_depth + 1); i++)
    {
      cave[i][x_left-1].fval = TMP1_WALL;
      cave[i][x_right+1].fval = TMP1_WALL;
    }
  c_ptr = &cave[y_height-1][x_left];
  d_ptr = &cave[y_depth+1][x_left];
  for (i = x_left; i <= x_right; i++)
    {
      c_ptr->fval = TMP1_WALL;
      c_ptr++;
      d_ptr->fval = TMP1_WALL;
      d_ptr++;
    }
  switch (randint(4)) {
  case 1:
    place_secret_door(y_height-1, xval);
    break;
  case 2:
    place_secret_door(y_depth+1, xval);
    break;
  case 3:
    place_secret_door(yval, x_left-1);
    break;
  case 4:
    place_secret_door(yval, x_right+1);
    break;
  }
  colour= randint(6);
  if (wizard) {
    switch (type) {
    case 1:
      msg_print("A Slime Pit");
      break;
    case 2:
      msg_print("An Orc Pit");
      break;
    case 3:
      msg_print("A Troll Pit");
      break;
    case 4:
      msg_print("A Graveyard");
      break;
    case 5:
      switch (colour) {
      case 1:
	msg_print("A Blue Dragon Pit");
	break;
      case 2:
	msg_print("A White Dragon Pit");
	break;
      case 3:
	msg_print("A Green Dragon Pit");
	break;
      case 4:
	msg_print("A Black Dragon Pit");
	break;
      case 5:
	msg_print("A Red Dragon Pit");
	break;
      case 6:
	msg_print("A Multi-Hued Dragon Pit");
	break;
      }
      break;
    case 6:
      msg_print("A Demon Pit");
      break;
    }
  }
  j=y_height;
  for (i=x_left; i<=x_right; i++)
    vault_nasty(j,i,type,1,colour);
  j=y_depth;
  for (i=x_left; i<=x_right; i++)
    vault_nasty(j,i,type,1,colour);
  i=x_left;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,1,colour);
  i=x_right;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,1,colour);
  i=x_left+1;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,2,colour);
  i=x_left+2;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,2,colour);
  i=x_right-1;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,2,colour);
  i=x_right-2;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,2,colour);
  i=x_left+3;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,3,colour);
  i=x_left+4;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,3,colour);
  i=x_right-3;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,3,colour);
  i=x_right-4;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,3,colour);
  i=x_left+5;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,4,colour);
  i=x_left+6;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,4,colour);
  i=x_right-5;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,4,colour);
  i=x_right-6;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,4,colour);
  i=x_left+7;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,5,colour);
  i=x_left+8;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,5,colour);
  i=x_right-7;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,5,colour);
  i=x_right-8;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,5,colour);
  i=x_right-9;
  for (j=(y_height+1); j<=(y_depth-1); j++)
    vault_nasty(j,i,type,6,colour);
}

/* Constructs a tunnel between two points		*/
static void build_tunnel(row1, col1, row2, col2)
int row1, col1, row2, col2;
{
  register int tmp_row, tmp_col, i, j;
  register cave_type *c_ptr;
  cave_type *d_ptr;
  coords tunstk[1000], wallstk[1000];
  coords *tun_ptr;
  int row_dir, col_dir, tunindex, wallindex;
  int stop_flag, door_flag, main_loop_count;
  int start_row, start_col;

  /* Main procedure for Tunnel			*/
  /* Note: 9 is a temporary value		*/
  stop_flag = FALSE;
  door_flag = FALSE;
  tunindex    = 0;
  wallindex   = 0;
  main_loop_count = 0;
  start_row = row1;
  start_col = col1;
  correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

  do
    {
      /* prevent infinite loops, just in case */
      main_loop_count++;
      if (main_loop_count > 2000)
	stop_flag = TRUE;

      if (randint(100) > DUN_TUN_CHG)
	{
	  if (randint(DUN_TUN_RND) == 1)
	    rand_dir(&row_dir, &col_dir);
	  else
	    correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
	}
      tmp_row = row1 + row_dir;
      tmp_col = col1 + col_dir;
      while (!in_bounds(tmp_row, tmp_col))
	{
	  if (randint(DUN_TUN_RND) == 1)
	    rand_dir(&row_dir, &col_dir);
	  else
	    correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
	  tmp_row = row1 + row_dir;
	  tmp_col = col1 + col_dir;
	}
      c_ptr = &cave[tmp_row][tmp_col];
      if (c_ptr->fval == NULL_WALL)
	{
	  row1 = tmp_row;
	  col1 = tmp_col;
	  if (tunindex < 1000)
	    {
	      tunstk[tunindex].y = row1;
	      tunstk[tunindex].x = col1;
	      tunindex++;
	    }
	  door_flag = FALSE;
	}
      else if (c_ptr->fval == TMP2_WALL)
	/* do nothing */
	;
      else if (c_ptr->fval == GRANITE_WALL)
	{
	  row1 = tmp_row;
	  col1 = tmp_col;
	  if (wallindex < 1000)
	    {
	      wallstk[wallindex].y = row1;
	      wallstk[wallindex].x = col1;
	      wallindex++;
	    }
	  for (i = row1-1; i <= row1+1; i++)
	    for (j = col1-1; j <= col1+1; j++)
	      if (in_bounds(i, j))
		{
		  d_ptr = &cave[i][j];
		  /* values 11 and 12 are impossible here, place_streamer
		     is never run before build_tunnel */
		  if (d_ptr->fval == GRANITE_WALL)
		    d_ptr->fval = TMP2_WALL;
		}
	}
      else if (c_ptr->fval == CORR_FLOOR || c_ptr->fval == BLOCKED_FLOOR)
	{
	  row1 = tmp_row;
	  col1 = tmp_col;
	  if (!door_flag)
	    {
	      if (doorindex < 100)
		{
		  doorstk[doorindex].y = row1;
		  doorstk[doorindex].x = col1;
		  doorindex++;
		}
	      door_flag = TRUE;
	    }
	  if (randint(100) > DUN_TUN_CON)
	    {
	      /* make sure that tunnel has gone a reasonable distance
		 before stopping it, this helps prevent isolated rooms */
	      tmp_row = row1 - start_row;
	      if (tmp_row < 0) tmp_row = -tmp_row;
	      tmp_col = col1 - start_col;
	      if (tmp_col < 0) tmp_col = -tmp_col;
	      if (tmp_row > 10 || tmp_col > 10)
		stop_flag = TRUE;
	    }
	}
      else  /* c_ptr->fval != NULL, TMP2, GRANITE, CORR */
	{
	  row1 = tmp_row;
	  col1 = tmp_col;
	}
    }
  while (((row1 != row2) || (col1 != col2)) && (!stop_flag));

  tun_ptr = &tunstk[0];
  for (i = 0; i < tunindex; i++)
    {
      d_ptr = &cave[tun_ptr->y][tun_ptr->x];
      d_ptr->fval  = CORR_FLOOR;
      tun_ptr++;
    }
  for (i = 0; i < wallindex; i++)
    {
      c_ptr = &cave[wallstk[i].y][wallstk[i].x];
      if (c_ptr->fval == TMP2_WALL)
	{
	  if (randint(100) < DUN_TUN_PEN)
	    place_door(wallstk[i].y, wallstk[i].x);
	  else
	    {
	      /* these have to be doorways to rooms */
	      c_ptr->fval  = CORR_FLOOR;
	    }
	}
    }
}


static int next_to(y, x)
register int y, x;
{
  register int next;

  if (next_to_corr(y, x) > 2)
    if ((cave[y-1][x].fval >= MIN_CAVE_WALL)
	&& (cave[y+1][x].fval >= MIN_CAVE_WALL))
      next = TRUE;
    else if ((cave[y][x-1].fval >= MIN_CAVE_WALL)
	     && (cave[y][x+1].fval >= MIN_CAVE_WALL))
      next = TRUE;
    else
      next = FALSE;
  else
    next = FALSE;
  return(next);
}

/* Places door at y, x position if at least 2 walls found	*/
static void try_door(y, x)
register int y, x;
{
  if ((cave[y][x].fval == CORR_FLOOR) && (randint(100) > DUN_TUN_JCT)
      && next_to(y, x))
    place_door(y, x);
}


/* Returns random co-ordinates				-RAK-	*/
static void new_spot(y, x)
int16 *y, *x;
{
  register int i, j;
  register cave_type *c_ptr;

  do
    {
      i = randint(cur_height - 2);
      j = randint(cur_width - 2);
      c_ptr = &cave[i][j];
    }
  while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->cptr != 0)
	 || (c_ptr->tptr != 0) || (c_ptr->fval == NT_LIGHT_FLOOR)
	 || (c_ptr->fval == NT_DARK_FLOOR));
  *y = i;
  *x = j;
}

static void build_pit(yval, xval)
  int yval, xval;
{
  int tmp;

#ifndef MSDOS
  if (randint((dun_level*dun_level*dun_level)+1)<25000) good_item_flag = TRUE;
#else
  if (randint((dun_level*dun_level+1))<650) good_item_flag = TRUE;
#endif
  tmp = randint(dun_level>80?80:dun_level);
  rating += 10;
  if (tmp < 10)      special_pit(yval, xval,1);
  else if (tmp < 20) special_pit(yval, xval,2);
  else if (tmp < 40) special_pit(yval, xval,3);
  else if (tmp < 55) special_pit(yval, xval,4);
  else if (tmp < 72) special_pit(yval, xval,5);
  else               special_pit(yval, xval,6);
}

/* Cave logic flow for generation of new dungeon		*/
static void cave_gen()
{
  struct spot_type
    {
      int endx;
      int endy;
    };
  int room_map[20][20];
  register int i, j, k;
  int y1, x1, y2, x2, pick1, pick2, tmp;
  int row_rooms, col_rooms, alloc_level;
  int16 yloc[400], xloc[400];
  int pit_ok;

  rating=0;
  pit_ok=TRUE;
  row_rooms = 2*(cur_height/SCREEN_HEIGHT);
  col_rooms = 2*(cur_width /SCREEN_WIDTH);
  for (i = 0; i < row_rooms; i++)
    for (j = 0; j < col_rooms; j++)
      room_map[i][j] = FALSE;
  k = randnor(DUN_ROO_MEA, 2);
  for (i = 0; i < k; i++)
    room_map[randint(row_rooms)-1][randint(col_rooms)-1] = TRUE;
  k = 0;
  for (i = 0; i < row_rooms; i++)
    for (j = 0; j < col_rooms; j++)
      if (room_map[i][j] == TRUE)
	{
	  yloc[k] = i * (SCREEN_HEIGHT >> 1) + QUART_HEIGHT;
	  xloc[k] = j * (SCREEN_WIDTH >> 1) + QUART_WIDTH;
	  if (dun_level > randint(DUN_UNUSUAL))
	    {
	      tmp = randint(5);
	      if (tmp == 1)	 build_type1(yloc[k], xloc[k]);
	      else if (tmp == 2) build_type2(yloc[k], xloc[k]);
	      else if (tmp == 3) build_type3(yloc[k], xloc[k]);
	      else if ((tmp == 4) && dun_level > randint(DUN_UNUSUAL)) {
		build_type5(yloc[k], xloc[k]);
		if (j+1<col_rooms) room_map[i][j+1]=FALSE;
		if (j+1<col_rooms && i+1<row_rooms) room_map[i+1][j+1]=FALSE;
		if (j>0 && i+1<row_rooms) room_map[i+1][j-1]=FALSE;
		if (i+1<row_rooms) room_map[i+1][j]=FALSE;
	      } else if (dun_level > randint(DUN_UNUSUAL) && pit_ok) {
		build_pit(yloc[k], xloc[k]);
		pit_ok=FALSE;
	      } else {
		build_room(yloc[k], xloc[k]);
	      }
	    }
	  else
	    build_room(yloc[k], xloc[k]);
	  k++;
	}
  for (i = 0; i < k; i++)
    {
      pick1 = randint(k) - 1;
      pick2 = randint(k) - 1;
      y1 = yloc[pick1];
      x1 = xloc[pick1];
      yloc[pick1] = yloc[pick2];
      xloc[pick1] = xloc[pick2];
      yloc[pick2] = y1;
      xloc[pick2] = x1;
      }
  doorindex = 0;
  /* move zero entry to k, so that can call build_tunnel all k times */
  yloc[k] = yloc[0];
  xloc[k] = xloc[0];
  for (i = 0; i < k; i++)
    {
      y1 = yloc[i];
      x1 = xloc[i];
      y2 = yloc[i+1];
      x2 = xloc[i+1];
      build_tunnel(y2, x2, y1, x1);
      }
  fill_cave(GRANITE_WALL);
  for (i = 0; i < DUN_STR_MAG; i++)
    place_streamer(MAGMA_WALL, DUN_STR_MC);
  for (i = 0; i < DUN_STR_QUA; i++)
    place_streamer(QUARTZ_WALL, DUN_STR_QC);
  place_boundary();
  /* Place intersection doors	*/
  for (i = 0; i < doorindex; i++)
    {
      try_door(doorstk[i].y, doorstk[i].x-1);
      try_door(doorstk[i].y, doorstk[i].x+1);
      try_door(doorstk[i].y-1, doorstk[i].x);
      try_door(doorstk[i].y+1, doorstk[i].x);
    }
  alloc_level = (dun_level/3);
  if (alloc_level < 2)
    alloc_level = 2;
  else if (alloc_level > 10)
    alloc_level = 10;
  place_stairs(2, randint(2)+2, 3);
  place_stairs(1, randint(2), 3);
  /* Set up the character co-ords, used by alloc_monster, place_win_monster */
  new_spot(&char_row, &char_col);
  alloc_monster((randint(8)+MIN_MALLOC_LEVEL+alloc_level), 0, TRUE);
  alloc_object(set_corr, 3, randint(alloc_level));
  alloc_object(set_room, 5, randnor(TREAS_ROOM_ALLOC, 3));
  alloc_object(set_floor, 5, randnor(TREAS_ANY_ALLOC, 3));
  alloc_object(set_floor, 4, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_object(set_floor, 1, randint(alloc_level));
  if (place_ghost()) good_item_flag = TRUE;
  if (randint(5)<4 && dun_level>=WIN_MON_APPEAR)  place_win_monster();
}


/* Builds a store at a row, column coordinate			*/
static void build_store(store_num, y, x)
int store_num, y, x;
{
  int yval, y_height, y_depth;
  int xval, x_left, x_right;
  register int i, j;
  int cur_pos, tmp;
  register cave_type *c_ptr;

  yval	   = y*10 + 5;
  xval	   = x*14 + 12;
  y_height = yval - randint(3);
  y_depth  = yval + randint(4);
  x_left   = xval - randint(4);
  x_right  = xval + randint(4);
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++)
      cave[i][j].fval	 = BOUNDARY_WALL;
  tmp = randint(4);
  if (tmp < 3)
    {
      i = randint(y_depth-y_height) + y_height - 1;
      if (tmp == 1) j = x_left;
      else	    j = x_right;
    }
  else
    {
      j = randint(x_right-x_left) + x_left - 1;
      if (tmp == 3) i = y_depth;
      else	    i = y_height;
    }
  c_ptr = &cave[i][j];
  c_ptr->fval  = CORR_FLOOR;
  cur_pos = popt();
  c_ptr->tptr = cur_pos;
  invcopy(&t_list[cur_pos], OBJ_STORE_DOOR + store_num);
}


/* Link all free space in treasure list together		*/
static void tlink()
{
  register int i;

  for (i = 0; i < MAX_TALLOC; i++)
    invcopy(&t_list[i], OBJ_NOTHING);
  tcptr = MIN_TRIX;
}


/* Link all free space in monster list together			*/
static void mlink()
{
  register int i;

  for (i = 0; i < MAX_MALLOC; i++)
    if (m_list[i].mptr) delete_monster(i);
  for (i = 0; i < MAX_MALLOC; i++)
    m_list[i] = blank_monster;
  delete_unique(); /* Kludgey Fix ~Ludwig */
  mfptr = MIN_MONIX;
}


/* Town logic flow for generation of new town		*/
static void town_gen()
{
  register int i, j, l, m;
  register cave_type *c_ptr;
  int rooms[MAX_STORES], k;

  set_seed(town_seed);
  for (i = 0; i < MAX_STORES; i++)
    rooms[i] = i;
  l = MAX_STORES;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 4; j++)
      {
	k = randint(l) - 1;
	build_store(rooms[k], i, j);
	for (m = k; m < l-1; m++)
	  rooms[m] = rooms[m+1];
	l--;
      }
  fill_cave(DARK_FLOOR);
  /* make stairs before reset_seed, so that they don't move around */
  place_boundary();
  place_stairs(2, 1, 0);
  reset_seed();
  /* Set up the character co-ords, used by alloc_monster below */
  new_spot(&char_row, &char_col);
  if (0x1 & (turn / 5000))
    {		/* Night	*/
      for (i = 0; i < cur_height; i++)
	{
	  c_ptr = &cave[i][0];
	  for (j = 0; j < cur_width; j++)
	    {
	      if (c_ptr->fval != DARK_FLOOR)
		c_ptr->pl = TRUE;
	      c_ptr++;
	    }
	}
      alloc_monster(MIN_MALLOC_TN, 3, TRUE);
    }
  else
    {		/* Day	*/
      for (i = 0; i < cur_height; i++)
	{
	  c_ptr = &cave[i][0];
	  for (j = 0; j < cur_width; j++)
	    {
	      c_ptr->pl = TRUE;
	      c_ptr++;
	    }
	}
      alloc_monster(MIN_MALLOC_TD, 3, TRUE);
    }
  store_maint();
  place_ghost();
}


/* Generates a random dungeon level			-RAK-	*/
void generate_cave()
{
  panel_row_min	= 0;
  panel_row_max	= 0;
  panel_col_min	= 0;
  panel_col_max	= 0;
  char_row = -1;
  char_col = -1;

  tlink();
  mlink();
  blank_cave();

  if (dun_level == 0)
    {
      cur_height = SCREEN_HEIGHT;
      cur_width	 = SCREEN_WIDTH;
      max_panel_rows = (cur_height/SCREEN_HEIGHT)*2 - 2;
      max_panel_cols = (cur_width /SCREEN_WIDTH )*2 - 2;
      panel_row = max_panel_rows;
      panel_col = max_panel_cols;
      town_gen();
    }
  else
    {
      cur_height = MAX_HEIGHT;
      cur_width	 = MAX_WIDTH;
      max_panel_rows = (cur_height/SCREEN_HEIGHT)*2 - 2;
      max_panel_cols = (cur_width /SCREEN_WIDTH )*2 - 2;
      panel_row = max_panel_rows;
      panel_col = max_panel_cols;
      cave_gen();
    }
}
