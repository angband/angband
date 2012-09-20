/* spells.c: code for player and creature spells, breaths, wands, scrolls, etc.

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "monster.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#ifndef VMS
#include <strings.h>
#endif
#endif

#if defined(LINT_ARGS)
static void replace_spot(int, int, int);
#else
static void replace_spot(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
#endif

char *pain_message();

#ifdef MSDOS /* then need prototype... -CFT */
static char bolt_shape(int); /* added for bolts, minor nicety... -CFT */
#ifdef TC_COLOR
static int8u bolt_color(int); /* ditto -CFT */
#endif
#endif

/* Following are spell procedure/functions			-RAK-	*/
/* These routines are commonly used in the scroll, potion, wands, and	 */
/* staves routines, and are occasionally called from other areas.	  */
/* Now included are creature spells also.		       -RAK    */

static char bolt_shape(int dir){ /* added for bolts, minor nicety... -CFT */
  switch (dir){
    case 1: case 9:  return '/';
    case 2: case 8:  return '|';
    case 3: case 7:  return '\\';
    case 4: case 6:  return '-';
  }
  return '*'; /* should never happen... -CFT */
}

#ifdef TC_COLOR
static int8u bolt_color(int typ){
  switch(typ){
    case GF_MAGIC_MISSILE: case GF_MANA:
      return LIGHTCYAN;
    case GF_LIGHTNING: case GF_LIGHT:
      return YELLOW;
    case GF_POISON_GAS: case GF_CONFUSION:
      return GREEN;
    case GF_ACID:
      return LIGHTGREEN;
    case GF_FROST: case GF_WATER:
      return LIGHTBLUE;
    case GF_FIRE:
      return RED;
    case GF_HOLY_ORB: case GF_NETHER: case GF_CHAOS: case GF_TIME:
    case GF_DARK:
      return DARKGRAY;
    case GF_ARROW:
      return BROWN;
    case GF_PLASMA: case GF_METEOR:
      return LIGHTRED;
    case GF_SHARDS: case GF_SOUND:
      return WHITE;
    case GF_DISENCHANT:
      return CYAN;
    case GF_NEXUS: case GF_GRAVITY:
      return MAGENTA;
    case GF_FORCE: case GF_INERTIA:
      return LIGHTGRAY;
  }
  return randint(15); /* should never happen... -CFT */
}
#endif

void monster_name (m_name, m_ptr, r_ptr)
char *m_name;
monster_type *m_ptr;
creature_type *r_ptr;
{
  if (!m_ptr->ml)
    (void) strcpy (m_name, "It");
  else {
    if (r_ptr->cdefense & UNIQUE)
      (void) sprintf (m_name, "%s", r_ptr->name);
    else
      (void) sprintf (m_name, "The %s", r_ptr->name);
  }
}

void lower_monster_name (m_name, m_ptr, r_ptr)
char *m_name;
monster_type *m_ptr;
creature_type *r_ptr;
{
  if (!m_ptr->ml)
    (void) strcpy (m_name, "it");
  else {
    if (r_ptr->cdefense & UNIQUE)
      (void) sprintf (m_name, "%s", r_ptr->name);
    else
      (void) sprintf (m_name, "the %s", r_ptr->name);
  }
}

/* teleport you a level (or three:-) */
void tele_level() {
  if (dun_level==Q_PLANE)
    dun_level = 0;
  else if (is_quest(dun_level))
    dun_level -= 1;
  else
    dun_level += (-3) + 2*randint(2);
  if (dun_level < 0)
    dun_level = 0;
  new_level_flag = TRUE;
}

/* Sleep creatures adjacent to player			-RAK-	*/
int sleep_monsters1(y, x)
int y, x;
{
  register int i, j;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int sleep;
  vtype out_val, m_name;

  sleep = FALSE;
  for (i = y-1; i <= y+1; i++)
    for (j = x-1; j <= x+1; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->cptr > 1)
	  {
	    m_ptr = &m_list[c_ptr->cptr];
	    r_ptr = &c_list[m_ptr->mptr];

	    monster_name (m_name, m_ptr, r_ptr);
	    if ((r_ptr->level >
		 randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) ||
		(CHARM_SLEEP & r_ptr->cdefense) || (r_ptr->cdefense & UNIQUE))
	      {
		if (m_ptr->ml && (r_ptr->cdefense & CHARM_SLEEP))
		  c_recall[m_ptr->mptr].r_cdefense |= CHARM_SLEEP;
		(void) sprintf(out_val, "%s is unaffected.", m_name);
		msg_print(out_val);
	      }
	    else
	      {
		sleep = TRUE;
		m_ptr->csleep = 500;
		(void) sprintf(out_val, "%s falls asleep.", m_name);
		msg_print(out_val);
	      }
	  }
      }
  return(sleep);
}

int lose_all_info() {
  int i;

  for (i=0; i<=INVEN_AUX; i++) {
    if (inventory[i].tval != TV_NOTHING)
      inventory[i].ident &= ~(ID_KNOWN2);
  }
  wizard_light(-1);
}

void identify_pack() {
  int i;
  inven_type *i_ptr;

  for (i=0; i<=INVEN_AUX; i++) {
    if (inventory[i].tval != TV_NOTHING)
      identify(&i);
      i_ptr = &inventory[i];
      known2(i_ptr);
  }
}

/* Detect any treasure on the current panel		-RAK-	*/
int detect_treasure()
{
  register int i, j, detect;
  register cave_type *c_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
	if ((c_ptr->tptr != 0) && (t_list[c_ptr->tptr].tval == TV_GOLD) &&
	    !test_light(i, j))
	  {
	    c_ptr->fm = TRUE;
	    lite_spot(i, j);
	    detect = TRUE;
	  }
      }
  return(detect);
}

int special_check();

int detect_enchantment()
{
  register int i, j, detect;
  register cave_type *c_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
	if ((c_ptr->tptr != 0) && (t_list[c_ptr->tptr].tval < TV_MAX_OBJECT)
	    && !test_light(i, j))
	  {
	    if (special_check(&(t_list[c_ptr->tptr])))
	    {
	    	c_ptr->fm = TRUE;
	    	lite_spot(i, j);
	    	detect = TRUE;
	    }
	  }
      }
  return(detect);
}

int detection()
{
  register int i, detect;
  register monster_type *m_ptr;

  detect_treasure();
  detect_object();
  detect_trap();
  detect_sdoor();

  detect = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx))
	{
	  m_ptr->ml = TRUE;
	  /* works correctly even if hallucinating */
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(c_list[m_ptr->mptr].color);
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#else
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
#endif
	  detect = TRUE;
	}
    }
  if (detect)
    {
      msg_print("You sense the presence of monsters!");
      msg_print(NULL);
      /* must unlight every monster just lighted */
      creatures(FALSE);
    }
  return(detect);
}

/* Detect all objects on the current panel		-RAK-	*/
int detect_object()
{
  register int i, j, detect;
  register cave_type *c_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
	if ((c_ptr->tptr != 0) && (t_list[c_ptr->tptr].tval < TV_MAX_OBJECT)
	    && !test_light(i, j))
	  {
	    c_ptr->fm = TRUE;
	    lite_spot(i, j);
	    detect = TRUE;
	  }
      }
  return(detect);
}


/* Locates and displays traps on current panel		-RAK-	*/
int detect_trap()
{
  register int i, j;
  int detect;
  register cave_type *c_ptr;
  register inven_type *t_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->tptr != 0)
	  if (t_list[c_ptr->tptr].tval == TV_INVIS_TRAP)
	    {
	      c_ptr->fm = TRUE;
	      change_trap(i, j);
	      detect = TRUE;
	    }
	  else if (t_list[c_ptr->tptr].tval == TV_CHEST)
	    {
	      t_ptr = &t_list[c_ptr->tptr];
	      known2(t_ptr);
	    }
      }
  return(detect);
}

int stair_creation()
{
    register cave_type *c_ptr;
    register int cur_pos;

    c_ptr = &cave[char_row][char_col];
 
    if ((c_ptr->tptr == 0) ||
         ((t_list[c_ptr->tptr].tval != TV_STORE_DOOR) && /* if not store, or */
	   ((t_list[c_ptr->tptr].tval < TV_MIN_WEAR) || /* if no artifact here -CFT */
	    (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
	    !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)))){ 
      if (c_ptr->tptr != 0)
        (void) delete_object(char_row, char_col);
      cur_pos = popt();
      c_ptr->tptr = cur_pos;
      if ((randint(2)==1 || is_quest(dun_level)) && (dun_level > 0))
        invcopy(&t_list[cur_pos], OBJ_UP_STAIR);
      else
        invcopy(&t_list[cur_pos], OBJ_DOWN_STAIR);
      }
    else
      msg_print("The object resists the spell.");
}

/* Surround the player with doors.			-RAK-	*/
int door_creation()
{
  register int i, j, door;
  int k;
  register cave_type *c_ptr;

  door = FALSE;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <=  char_col+1; j++)
      if ((i != char_row) || (j != char_col))
	{
	  c_ptr = &cave[i][j];
	  if (c_ptr->fval <= MAX_CAVE_FLOOR)
	    {
    if ((c_ptr->tptr == 0) ||
        (t_list[c_ptr->tptr].tval < TV_MIN_WEAR) ||
        (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
        !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)) { /* if no artifact here -CFT */
	      door = TRUE;
	      if (c_ptr->tptr != 0)
		(void) delete_object(i, j);
	      k = popt();
	      c_ptr->fval = BLOCKED_FLOOR;
	      c_ptr->tptr = k;
	      invcopy(&t_list[k], OBJ_CLOSED_DOOR);
	      lite_spot(i, j);
      }
    else
      msg_print("The object resists the spell.");
	    }
	}
  return(door);
}

/* Locates and displays all secret doors on current panel -RAK-	*/
int detect_sdoor()
{
  register int i, j, detect;
  register cave_type *c_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->tptr != 0)
	  /* Secret doors  */
	  if (t_list[c_ptr->tptr].tval == TV_SECRET_DOOR)
	    {
	      c_ptr->fm = TRUE;
	      change_trap(i, j);
	      detect = TRUE;
	    }
	/* Staircases	 */
	  else if (((t_list[c_ptr->tptr].tval == TV_UP_STAIR) ||
		    (t_list[c_ptr->tptr].tval == TV_DOWN_STAIR)) &&
		   !c_ptr->fm)
	    {
	      c_ptr->fm = TRUE;
	      lite_spot(i, j);
	      detect = TRUE;
	    }
      }
  return(detect);
}


/* Locates and displays all invisible creatures on current panel -RAK-*/
int detect_invisible()
{
  register int i, flag;
  register monster_type *m_ptr;

  flag = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) &&
	  (CM_INVISIBLE & c_list[m_ptr->mptr].cmove))
	{
	  m_ptr->ml = TRUE;
	  /* works correctly even if hallucinating */
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(c_list[m_ptr->mptr].color);
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#else
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
#endif
	  flag = TRUE;
	}
    }
  if (flag)
    {
      msg_print("You sense the presence of invisible creatures!");
      msg_print(NULL);
      /* must unlight every monster just lighted */
      creatures(FALSE);
    }
  return(flag);
}


/* Light an area: 1.  If corridor  light immediate area -RAK-*/
/*		  2.  If room  light entire room.	     */
int light_area(y, x)
register int y, x;
{
  register int i, j, light;

  if (py.flags.blind < 1)
    msg_print("You are surrounded by a white light.");
  light = TRUE;
  if (cave[y][x].lr && (dun_level > 0))
    light_room(y, x);
  for (i = y-1; i <= y+1; i++)
    for (j = x-1; j <=  x+1; j++) {
      cave[i][j].pl = TRUE;
      lite_spot(i, j);
    }
  return(light);
}


/* Darken an area, opposite of light area		-RAK-	*/
int unlight_area(y, x)
int y, x;
{
  register int i, j;
  int tmp1, tmp2, unlight;
  int start_row, start_col, end_row, end_col;
  register cave_type *c_ptr;

  unlight = FALSE;
  if (cave[y][x].lr && (dun_level > 0)) {
    darken_room(y, x);
    unlight = TRUE; /* this isn't really good, as it returns true, even if rm
    		       was already dark, but at least scrolls of darkness
    		       will be IDed when used -CFT */
    }
  else
    for (i = y-1; i <= y+1; i++)
      for (j = x-1; j <= x+1; j++)
	{
	  c_ptr = &cave[i][j];
	  if ((c_ptr->fval == CORR_FLOOR) && c_ptr->pl)
	    {
	      /* pl could have been set by star-lite wand, etc */
	      c_ptr->pl = FALSE;
	      unlight = TRUE;
	    }
	}

  if (unlight && py.flags.blind <= 0)
    msg_print("Darkness surrounds you.");

  return(unlight);
}


/* Map the current area plus some			-RAK-	*/
void map_area()
{
  register cave_type *c_ptr;
  register int i7, i8, n, m;
  int i, j, k, l;

  i = panel_row_min - randint(10);
  j = panel_row_max + randint(10);
  k = panel_col_min - randint(20);
  l = panel_col_max + randint(20);
  for (m = i; m <= j; m++)
    for (n = k; n <= l; n++)
      if (in_bounds(m, n) && (cave[m][n].fval <= MAX_CAVE_FLOOR))
	for (i7 = m-1; i7 <= m+1; i7++)
	  for (i8 = n-1; i8 <= n+1; i8++)
	    {
	      c_ptr = &cave[i7][i8];
	      if (c_ptr->fval >= MIN_CAVE_WALL)
		c_ptr->pl = TRUE;
	      else if ((c_ptr->tptr != 0) &&
		       (t_list[c_ptr->tptr].tval >= TV_MIN_VISIBLE) &&
		       (t_list[c_ptr->tptr].tval <= TV_MAX_VISIBLE))
		c_ptr->fm = TRUE;
	    }
  prt_map();
}


/* Identify an object					-RAK-	*/
int ident_spell()
{
  int item_val;
  bigvtype out_val, tmp_str;
  register int ident;
  register inven_type *i_ptr;

  ident = FALSE;
  if (get_item(&item_val, "Item you wish identified?", 0, INVEN_ARRAY_SIZE, 0))
    {
      ident = TRUE;
      identify(&item_val);
      i_ptr = &inventory[item_val];
      known2(i_ptr);
      objdes(tmp_str, i_ptr, TRUE);
      if (item_val >= INVEN_WIELD)
	{
	  calc_bonuses();
	  (void) sprintf (out_val, "%s: %s", describe_use(item_val), tmp_str);
	}
      else
	(void) sprintf(out_val, "%c %s", item_val+97, tmp_str);
      msg_print(out_val);
    }
  return(ident);
}


/* Get all the monsters on the level pissed off.	-RAK-	*/
int aggravate_monster (dis_affect)
int dis_affect;
{
  register int i, aggravate;
  register monster_type *m_ptr;

  aggravate = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      m_ptr->csleep = 0;
      if ((m_ptr->cdis <= dis_affect) && (m_ptr->cspeed < 2))
	{
	  m_ptr->cspeed++;
	  aggravate = TRUE;
	}
    }
  if (aggravate)
    msg_print ("You hear a sudden stirring in the distance!");
  return(aggravate);
}


/* Surround the fool with traps (chuckle)		-RAK-	*/
int trap_creation()
{
  register int i, j, trap;
  register cave_type *c_ptr;

  trap = FALSE;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->fval <= MAX_CAVE_FLOOR)
	  {
    if ((c_ptr->tptr == 0) ||
        (t_list[c_ptr->tptr].tval < TV_MIN_WEAR) ||
        (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
        !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)) { /* if no artifact here -CFT */
	    trap = TRUE;
	    if (c_ptr->tptr != 0)
	      (void) delete_object(i, j);
	    place_trap(i, j, randint(MAX_TRAP)-1);
	    /* don't let player gain exp from the newly created traps */
	    t_list[c_ptr->tptr].p1 = 0;
	    /* open pits are immediately visible, so call lite_spot */
	    lite_spot(i, j);
      }
    else
      msg_print("The object resists the spell.");
	  }
      }
  return(trap);
}


/* Destroys any adjacent door(s)/trap(s)		-RAK-	*/
int td_destroy()
{
  register int i, j, destroy;
  register cave_type *c_ptr;

  destroy = FALSE;
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++)
      {
	c_ptr = &cave[i][j];
	if (c_ptr->tptr != 0)
	  {
	    if (((t_list[c_ptr->tptr].tval >= TV_INVIS_TRAP) &&
		 (t_list[c_ptr->tptr].tval <= TV_CLOSED_DOOR) &&
		 (t_list[c_ptr->tptr].tval != TV_RUBBLE)) ||
		(t_list[c_ptr->tptr].tval == TV_SECRET_DOOR))
	      {
		if (delete_object(i, j))
		  destroy = TRUE;
	      }
	    else if (t_list[c_ptr->tptr].tval == TV_CHEST)
	      {
		/* destroy traps on chest and unlock */
		t_list[c_ptr->tptr].flags &= ~(CH_TRAPPED|CH_LOCKED);
		t_list[c_ptr->tptr].name2 = SN_DISARMED;
		msg_print ("You have disarmed the chest.");
		known2(&t_list[c_ptr->tptr]);
		destroy = TRUE;
	      }
	  }
      }
  return(destroy);
}


/* Display all creatures on the current panel		-RAK-	*/
int detect_monsters()
{
  register int i, detect;
  register monster_type *m_ptr;

  detect = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) &&
	  ((CM_INVISIBLE & c_list[m_ptr->mptr].cmove) == 0))
	{
	  m_ptr->ml = TRUE;
	  /* works correctly even if hallucinating */
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(c_list[m_ptr->mptr].color);
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#else
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
#endif
	  detect = TRUE;
	}
    }
  if (detect)
    {
      msg_print("You sense the presence of monsters!");
      msg_print(NULL);
      /* must unlight every monster just lighted */
      creatures(FALSE);
    }
  return(detect);
}


/* Leave a line of light in given dir, blue light can sometimes	*/
/* hurt creatures.				       -RAK-   */
void light_line(dir, y, x)
int dir, y, x;
{
  register int i;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int dist, flag;
  vtype out_val, m_name;

  dist = -1;
  flag = FALSE;
  do
    {
      /* put mmove at end because want to light up current spot */
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else
	{
	  if (!c_ptr->pl && !c_ptr->tl)
	    {
	      /* set pl so that lite_spot will work */
	      c_ptr->pl = TRUE;
	      if (c_ptr->fval == LIGHT_FLOOR)
		{
		  if (panel_contains(y, x))
		    light_room(y, x);
		}
	      else
		lite_spot(y, x);
	    }
	  /* set pl in case tl was true above */
	  c_ptr->pl = TRUE;
	  if (c_ptr->cptr > 1)
	    {
	      m_ptr = &m_list[c_ptr->cptr];
	      r_ptr = &c_list[m_ptr->mptr];
	      /* light up and draw monster */
	      update_mon ((int)c_ptr->cptr);
	      monster_name (m_name, m_ptr, r_ptr);
	      m_ptr->csleep = 0;
	      if (HURT_LIGHT & r_ptr->cdefense)
		{
		  if (m_ptr->ml)
		    c_recall[m_ptr->mptr].r_cdefense |= HURT_LIGHT;
		  i = mon_take_hit((int)c_ptr->cptr, damroll(6, 8));
		  if (i >= 0)
		    {
		      (void) sprintf(out_val,
			     "%s shrivels away in the light!", m_name);
		      msg_print(out_val);
		      prt_experience();
		    }
		  else
		    {
		      (void) sprintf(out_val, "%s cringes from the light!",
				     m_name);
		      msg_print (out_val);
		    }
		}
	    }
	}
      (void) mmove(dir, &y, &x);
    }
  while (!flag);
}


/* Light line in all directions				-RAK-	*/
void starlite(y, x)
register int y, x;
{
  register int i;

  if (py.flags.blind < 1)
    msg_print("The end of the staff bursts into a blue shimmering light.");
  for (i = 1; i <= 9; i++)
    if (i != 5)
      light_line(i, y, x);
}

/* Disarms all traps/chests in a given direction	-RAK-	*/
int disarm_all(dir, y, x)
int dir, y, x;
{
  register cave_type *c_ptr;
  register inven_type *t_ptr;
  register int disarm, dist;

  disarm = FALSE;
  dist = -1;
  do
    {
      /* put mmove at end, in case standing on a trap */
      dist++;
      c_ptr = &cave[y][x];
      /* note, must continue upto and including the first non open space,
	 because secret doors have fval greater than MAX_OPEN_SPACE */
      if (c_ptr->tptr != 0)
	{
	  t_ptr = &t_list[c_ptr->tptr];
	  if ((t_ptr->tval == TV_INVIS_TRAP) || (t_ptr->tval == TV_VIS_TRAP))
	    {
	      if (delete_object(y, x))
		disarm = TRUE;
	    }
	  else if (t_ptr->tval == TV_CLOSED_DOOR)
	    t_ptr->p1 = 0;  /* Locked or jammed doors become merely closed. */
	  else if (t_ptr->tval == TV_SECRET_DOOR)
	    {
	      c_ptr->fm = TRUE;
	      change_trap(y, x);
	      disarm = TRUE;
	    }
	  else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0))
	    {
	      msg_print("Click!");
	      t_ptr->flags &= ~(CH_TRAPPED|CH_LOCKED);
	      disarm = TRUE;
	      t_ptr->name2 = SN_UNLOCKED;
	      known2(t_ptr);
	    }
	}
      (void) mmove(dir, &y, &x);
    }
  while ((dist <= OBJ_BOLT_RANGE) && c_ptr->fval <= MAX_OPEN_SPACE);
  return(disarm);
}


/* Return flags for given type area affect		-RAK-	*/
void get_flags(typ, weapon_type, harm_type, destroy)
int typ;
int32u *weapon_type; int32u *harm_type;
#ifdef MSDOS
int (**destroy)(inven_type *);
#else
int (**destroy)();
#endif
{
  switch(typ)
    {
    case GF_MAGIC_MISSILE:
      *weapon_type = 0;
      *harm_type   = 0;
      *destroy	   = set_null;
      break;
    case GF_LIGHTNING:
      *weapon_type = CS_BR_LIGHT;
      *harm_type   = IM_LIGHTNING;
      *destroy	   = set_lightning_destroy;
      break;
    case GF_POISON_GAS:
      *weapon_type = CS_BR_GAS;
      *harm_type   = IM_POISON;
      *destroy	   = set_null;
      break;
    case GF_ACID:
      *weapon_type = CS_BR_ACID;
      *harm_type   = IM_ACID;
      *destroy	   = set_acid_destroy;
      break;
    case GF_FROST:
      *weapon_type = CS_BR_FROST;
      *harm_type   = IM_FROST;
      *destroy	   = set_frost_destroy;
      break;
    case GF_FIRE:
      *weapon_type = CS_BR_FIRE;
      *harm_type   = IM_FIRE;
      *destroy	   = set_fire_destroy;
      break;
    case GF_HOLY_ORB:
      *weapon_type = 0;
      *harm_type   = EVIL;
      *destroy	   = set_null;
      break;
    default:
      msg_print("ERROR in get_flags()\n");
    }
}


/* Shoot a bolt in a given direction			-RAK-	*/
void fire_bolt(typ, dir, y, x, dam, bolt_typ)
int typ, dir, y, x, dam;
char *bolt_typ;
{
  int i, oldy, oldx, dist, flag;
  int32u weapon_type, harm_type;
  int (*dummy)();
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;
#ifdef TC_COLOR
  int ttyp;
#endif

  flag = FALSE;
#ifdef TC_COLOR
  switch( typ ){
    case GF_ARROW: case GF_PLASMA: case GF_NETHER: case GF_WATER:
    case GF_CHAOS: case GF_SHARDS: case GF_SOUND: case GF_CONFUSION:
    case GF_DISENCHANT: case GF_NEXUS: case GF_FORCE: case GF_INERTIA:
    case GF_LIGHT: case GF_DARK: case GF_TIME: case GF_GRAVITY:
    case GF_METEOR: case GF_MANA:
      ttyp = GF_MAGIC_MISSILE; /* use m.m. for dam/destroy calcs... */
      break;
    default: ttyp = typ;
  }
  get_flags(ttyp, &weapon_type, &harm_type, &dummy);
#else
  get_flags(typ, &weapon_type, &harm_type, &dummy);
#endif
  oldy = y;
  oldx = x;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      lite_spot(oldy, oldx);
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else
	{
	  if (c_ptr->cptr > 1)
	    {
	      flag = TRUE;
	      m_ptr = &m_list[c_ptr->cptr];
	      r_ptr = &c_list[m_ptr->mptr];

	      /* light up monster and draw monster, temporarily set
		 pl so that update_mon() will work */
	      i = c_ptr->pl;
	      c_ptr->pl = TRUE;
	      update_mon ((int)c_ptr->cptr);
	      c_ptr->pl = i;
	      /* draw monster and clear previous bolt */
	      put_qio();

	      lower_monster_name(m_name, m_ptr, r_ptr);
	      (void) sprintf(out_val, "The %s strikes %s.", bolt_typ, m_name);
	      msg_print(out_val);
	      if (harm_type & r_ptr->cdefense)
		{
		  if (harm_type == EVIL)
		    dam = (dam*3)/2;
		  else
		    dam = dam/9;
		  c_recall[m_ptr->mptr].r_cdefense |= harm_type;
		}
	      monster_name(m_name, m_ptr, r_ptr);
	      i = mon_take_hit((int)c_ptr->cptr, dam);
	      if (i >= 0)
		{
		  if (r_ptr->cdefense & UNDEAD)
		    (void) sprintf(out_val, "%s is destroyed.",
				   m_name);
		  else if (r_ptr->cdefense & DEMON)
/* better message? */    (void) sprintf(out_val, "%s is destroyed.",
				   m_name);
		  else (void) sprintf(out_val, "%s dies in a fit of agony.",
				      m_name);
		  msg_print(out_val);
		  prt_experience();
		}
	      else if (dam > 0)
		{
		  if (harm_type & r_ptr->cdefense &
		      (IM_FIRE|IM_POISON|IM_FROST|IM_LIGHTNING|IM_ACID))
		    (void) sprintf (out_val, "%s looks unharmed.", m_name);
		  else
		    (void) sprintf (out_val,
			pain_message((int)c_ptr->cptr,dam),m_name);
		    msg_print (out_val);
		}
	    }
	  else if (panel_contains(y, x) && (py.flags.blind < 1))
	    {
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(bolt_color(typ));
#endif
	      print(bolt_shape(dir), y, x);
	      /* show the bolt */
	      put_qio();
#ifdef MSDOS
	      delay(23); /* slow it down, so we can actually see it! -CFT */
#endif
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
	    }
	}
      oldy = y;
      oldx = x;
    }
  while (!flag);
}



/* Shoot a bolt in a given direction			-RAK-	*/
/* At the player, it looks like... -CFT
	Changed from void to int.  ret val of 1 means hit player, ret val
	of 0 means did not hit player.  This value used in mon_cast_spell()
	to control secondary effects of the bolt (ie lose experience from
	nether bolt).  If it doesn't hit the player, then no 2ndary effects
	should happen. -CFT	
 */
int bolt(typ, y, x, dam_hp, ddesc, ptr, monptr)
  int typ, y, x, dam_hp;
  char *ddesc;
  monster_type *ptr;
  int monptr;
{
  register int i=ptr->fy, j=ptr->fx, k;
  int dam, max_dis;
  int32u weapon_type, harm_type;
  int32u tmp, treas;
  register cave_type *c_ptr;
  int (*destroy)();
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int y2, x2; /* deltas */
  int xstep, ystep; /* 1 or -1 */
  int dy, dx; /* 1, 0, or -1 */
  char bolt_char;
  int retval = 0; /* return value -CFT */
#ifdef TC_COLOR
  int ttyp;
#endif

#ifdef TC_COLOR
  switch( typ ){
    case GF_ARROW: case GF_PLASMA: case GF_NETHER: case GF_WATER:
    case GF_CHAOS: case GF_SHARDS: case GF_SOUND: case GF_CONFUSION:
    case GF_DISENCHANT: case GF_NEXUS: case GF_FORCE: case GF_INERTIA:
    case GF_LIGHT: case GF_DARK: case GF_TIME: case GF_GRAVITY:
    case GF_METEOR: case GF_MANA:
      ttyp = GF_MAGIC_MISSILE; /* use m.m. for dam/destroy calcs... */
      break;
    default: ttyp = typ;
  }
  get_flags(ttyp, &weapon_type, &harm_type, &destroy);
#else
  get_flags(typ, &weapon_type, &harm_type, &destroy);
#endif

  y2 = y - ptr->fy; /* calc deltas to target (player... *grin*) */
  x2 = x - ptr->fx;

  if (y2 < 0) { /* calc x, y step vals, and make x2, y2 abs value */
    ystep = -1;
    y2 *= -1; /* and make positive */
  } else {
    ystep = 1;
  }
  if (x2 < 0) {
    xstep = -1;
    x2 *= -1; /* and make positive */
  } else {
    xstep = 1;
  }
  
  while ( (x2 + y2) > 0 ) { /* keep going until hits target, or break  -CFT */
    dy = ystep; dx = xstep; /* defaults... */
    if ((y2*100) > (x2*241)) /* 2.41 is tan(45/2 deg) */
      { dy = ystep; dx = 0; } /* only move in y... */
    if ((x2*100) > (y2*241)) /* 2.41 is tan(45/2 deg) */
      { dx = xstep; dy = 0; } /* only move in x... */
    i += dy;  y2 -= (dy != 0 ? 1 : 0); /* adjust coords */
    j += dx;  x2 -= (dx != 0 ? 1 : 0);

  /* choose the right shape for the bolt... -CFT */
  if (dy == 0) bolt_char = '-';
  else if (dx == 0) bolt_char = '|';
  else if (dy == dx) bolt_char = '\\';
  else bolt_char = '/';
    
    if (in_bounds(i, j) && los(y, x, i, j)) {
#ifndef MSDOS
      usleep(50000); /* u-secs */
#endif
      c_ptr = &cave[i][j];
      if (c_ptr->fval <= MAX_OPEN_SPACE) {
	if (panel_contains(i, j) && !(py.flags.status & PY_BLIND)) {
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(bolt_color(typ));
#endif	  
	  print(bolt_char, i, j);
	  put_qio();
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif	  
#ifdef MSDOS
	  delay(23); /* milli-secs -CFT */
#endif
	  lite_spot(i, j);
	}
	if (c_ptr->cptr > 1 && c_ptr->cptr != monptr) {
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  dam = dam_hp;
	  if (harm_type & r_ptr->cdefense) {
	    if (harm_type == EVIL)
	      dam = dam*2;
	    else
	      dam = dam/9;
	  }
	  m_ptr->hp = m_ptr->hp - dam;
	  m_ptr->csleep = 0;
	  if ((r_ptr->cdefense & UNIQUE) && (m_ptr->hp < 0))
	    m_ptr->hp = 0; /* prevent unique monster from death by other
	    		     monsters.  It causes trouble (monster not
	    		     marked as dead, quest monsters don't satisfy
	    		     quest, etc).  So, we let then live, but
	    		     extremely wimpy.  This isn't great, because
	    		     monster might heal itself before player's
	    		     next swing... -CFT */
	  if (m_ptr->hp < 0) {
	    treas = monster_death((int)m_ptr->fy, (int)m_ptr->fx,
				  r_ptr->cmove, 0, 0);
	    if (m_ptr->ml) {
	      tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE)
		>> CM_TR_SHIFT;
	      if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
		treas = (treas & ~CM_TREASURE)|(tmp<<CM_TR_SHIFT);
	      c_recall[m_ptr->mptr].r_cmove = treas |
		(c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE);
	    }

	    if (monptr < c_ptr->cptr)
	      delete_monster((int) c_ptr->cptr);
	    else
	      fix1_delete_monster((int) c_ptr->cptr);
	  }
	  break;
	} else if (c_ptr->cptr == 1) {
	  retval = 1; /* it hit the player, activate 2ndary effects -CFT */
	  if (dam_hp == 0)
	    dam_hp = 1;
#ifdef TC_COLOR
	  switch(ttyp) {
#else
	  switch(typ) {
#endif
	  case GF_LIGHTNING:
	    light_dam(dam_hp, ddesc);
	    break;
	  case GF_POISON_GAS:
	    poison_gas(dam_hp, ddesc);
	    break;
	  case GF_ACID:
	    acid_dam(dam_hp, ddesc);
	    break;
	  case GF_FROST:
	    cold_dam(dam_hp, ddesc);
	    break;
	  case GF_FIRE:
	    fire_dam(dam_hp, ddesc);
	    break;
	  case GF_MAGIC_MISSILE:
	    take_hit(dam_hp, ddesc);
	    break;
	  }
	  disturb(1, 0);
	  break;
	}
      }
    }
  }
  return retval; /* tell calling code if hit the player or not... -CFT */
}

#if 0  /* old code, using flt. pt. math.  Above code should do same thing
	  with integers... -CFT */
/* Shoot a bolt in a given direction			-RAK-	*/
void bolt(typ, y, x, dam_hp, ddesc, ptr, monptr)
  int typ, y, x, dam_hp;
  char *ddesc;
  monster_type *ptr;
  int monptr;
{
  register int i, j, k;
  int dam, max_dis;
  int32u weapon_type, harm_type;
  int32u tmp, treas;
  register cave_type *c_ptr;
  int (*destroy)();
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  double y1, x1, ystep, xstep;
  int y2, x2;
  int step;

  get_flags(typ, &weapon_type, &harm_type, &destroy);

  y2 = y - ptr->fy;
  x2 = x - ptr->fx;
  step = (((y2<0)?-y2:y2)>((x2<0)?-x2:x2))?((y2<0)?-y2:y2):((x2<0)?-x2:x2);

  ystep = ((double)y2)/((double)step);
  xstep = ((double)x2)/((double)step);

  for (k=(step-1); k>=0; --k) {
    y1=(double)(ptr->fy)+((double)(step-k))*ystep;
    x1=(double)(ptr->fx)+((double)(step-k))*xstep;
    i=(int)y1;
    j=(int)x1;

    if (in_bounds(i, j) && los(y, x, i, j)) {
#ifdef MSDOS
      delay(50); /* milli-secs -CFT */
#else
      usleep(50000); /* u-secs */
#endif
      c_ptr = &cave[i][j];
      if (c_ptr->fval <= MAX_OPEN_SPACE) {
	if (panel_contains(i, j) && !(py.flags.status & PY_BLIND)) {
	  print('*', i, j);
	  put_qio();
	  lite_spot(i, j);
	}
	if (c_ptr->cptr > 1 && c_ptr->cptr != monptr) {
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  dam = dam_hp;
	  if (harm_type & r_ptr->cdefense) {
	    if (harm_type == EVIL)
	      dam = dam*2;
	    else
	      dam = dam/9;
	  }
	  m_ptr->hp = m_ptr->hp - dam;
	  m_ptr->csleep = 0;
	  if (m_ptr->hp < 0) {
	    treas = monster_death((int)m_ptr->fy, (int)m_ptr->fx,
				  r_ptr->cmove, 0, 0);
	    if (m_ptr->ml) {
	      tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE)
		>> CM_TR_SHIFT;
	      if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
		treas = (treas & ~CM_TREASURE)|(tmp<<CM_TR_SHIFT);
	      c_recall[m_ptr->mptr].r_cmove = treas |
		(c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE);
	    }

	    if (monptr < c_ptr->cptr)
	      delete_monster((int) c_ptr->cptr);
	    else
	      fix1_delete_monster((int) c_ptr->cptr);
	  }
	  break;
	} else if (c_ptr->cptr == 1) {
	  if (dam_hp == 0)
	    dam_hp = 1;
	  switch(typ) {
	  case GF_LIGHTNING:
	    light_dam(dam_hp, ddesc);
	    break;
	  case GF_POISON_GAS:
	    poison_gas(dam_hp, ddesc);
	    break;
	  case GF_ACID:
	    acid_dam(dam_hp, ddesc);
	    break;
	  case GF_FROST:
	    cold_dam(dam_hp, ddesc);
	    break;
	  case GF_FIRE:
	    fire_dam(dam_hp, ddesc);
	    break;
	  case GF_MAGIC_MISSILE:
	    take_hit(dam_hp, ddesc);
	    break;
	  }
	  disturb(1, 0);
	  break;
	}
      }
    }
  }
}
#endif

/* Shoot a ball in a given direction.  Note that balls have an	*/
/* area affect.					      -RAK-   */
void fire_ball(typ, dir, y, x, dam_hp, descrip)
int typ, dir, y, x, dam_hp;
char *descrip;
{
  register int i, j;
  int dam, max_dis, thit, tkill, k, tmp, monptr;
  int oldy, oldx, dist, flag;
  int32u weapon_type, harm_type;
  int (*destroy)();
  register cave_type *c_ptr;
  register monster_type *m_ptr, *M_ptr;
  register creature_type *r_ptr, *R_ptr;
  vtype m_name;
  vtype out_val;
#ifdef TC_COLOR
  int ttyp, origy = y, origx = x;
#endif

  thit	 = 0;
  tkill	 = 0;
  max_dis = 2;
#ifdef TC_COLOR
  switch( typ ){
    case GF_ARROW: case GF_PLASMA: case GF_NETHER: case GF_WATER:
    case GF_CHAOS: case GF_SHARDS: case GF_SOUND: case GF_CONFUSION:
    case GF_DISENCHANT: case GF_NEXUS: case GF_FORCE: case GF_INERTIA:
    case GF_LIGHT: case GF_DARK: case GF_TIME: case GF_GRAVITY:
    case GF_METEOR: case GF_MANA:
      ttyp = GF_MAGIC_MISSILE; /* use m.m. for dam/destroy calcs... */
      break;
    default: ttyp = typ;
  }
  get_flags(ttyp, &weapon_type, &harm_type, &destroy);
#else
  get_flags(typ, &weapon_type, &harm_type, &destroy);
#endif
  flag = FALSE;
  oldy = y;
  oldx = x;
  dist = 0;
  do {
    (void) mmove(dir, &y, &x);
    dist++;
    lite_spot(oldy, oldx);
    if (dist > OBJ_BOLT_RANGE)
      flag = TRUE;
    else {
      c_ptr = &cave[y][x];
      if ((c_ptr->fval >= MIN_CLOSED_SPACE) || (c_ptr->cptr > 1)) {
	flag = TRUE;
	if (c_ptr->fval >= MIN_CLOSED_SPACE) {
	  y = oldy;
	  x = oldx;
	}
	/* The ball hits and explodes.		     */
	/* The explosion.			     */
#ifdef TC_COLOR /* do ball a little diff, so we can see the colors... */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(origy, origx, i, j) && los(y, x, i, j) &&
		(cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && (py.flags.blind < 1)) {
	      if (!no_color_flag) textcolor(bolt_color(typ));
	      print('*', i, j);
	      put_qio();
	      if (!no_color_flag) textcolor(LIGHTGRAY); /* prob don't need here, but... -CFT */
	      }
	if (py.flags.blind < 1)      
	  delay(75); /* millisecs, so we see the ball we just drew */
	/* now go over area of affect and DO something... */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      c_ptr = &cave[i][j];
	      if ((c_ptr->tptr != 0) &&
		  (*destroy)(&t_list[c_ptr->tptr]))
		(void) delete_object(i, j);
	      if (c_ptr->fval <= MAX_OPEN_SPACE) {
		if (c_ptr->cptr > 1) {
		  m_ptr = &m_list[c_ptr->cptr];
		  r_ptr = &c_list[m_ptr->mptr];
		  monptr = c_ptr->cptr;

		  /* lite up creature if visible, temp
		     set pl so that update_mon works */
		  tmp = c_ptr->pl;
		  c_ptr->pl = TRUE;
		  update_mon((int)c_ptr->cptr);

		  thit++;
		  dam = dam_hp;
		  if (harm_type & r_ptr->cdefense) {
		    if (harm_type == EVIL)
		      dam = dam*2;
		    else
		      dam = dam/9;
		    c_recall[m_ptr->mptr].r_cdefense |=harm_type;
		  }
		  dam = (dam/(distance(i, j, y, x)+1));
		  k = mon_take_hit((int)c_ptr->cptr, dam);
		  if (k >= 0)
		    tkill++;
		  c_ptr->pl = tmp;
		}
		else if (panel_contains(i, j) &&(py.flags.blind < 1))
		 lite_spot(i,j); /* erase the ball... */
	      }
	    }
	/* show ball of whatever */
	put_qio();

#else
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      c_ptr = &cave[i][j];
	      if ((c_ptr->tptr != 0) &&
		  (*destroy)(&t_list[c_ptr->tptr]))
		(void) delete_object(i, j);
	      if (c_ptr->fval <= MAX_OPEN_SPACE) {
		if (c_ptr->cptr > 1) {
		  m_ptr = &m_list[c_ptr->cptr];
		  r_ptr = &c_list[m_ptr->mptr];
		  monptr = c_ptr->cptr;

		  /* lite up creature if visible, temp
		     set pl so that update_mon works */
		  tmp = c_ptr->pl;
		  c_ptr->pl = TRUE;
		  update_mon((int)c_ptr->cptr);

		  thit++;
		  dam = dam_hp;
		  if (harm_type & r_ptr->cdefense) {
		    if (harm_type == EVIL)
		      dam = dam*2;
		    else
		      dam = dam/9;
		    c_recall[m_ptr->mptr].r_cdefense |=harm_type;
		  }
		  dam = (dam/(distance(i, j, y, x)+1));
		  k = mon_take_hit((int)c_ptr->cptr, dam);
		  if (k >= 0)
		    tkill++;
		  c_ptr->pl = tmp;
		}
		else if (panel_contains(i, j) &&(py.flags.blind < 1))
		  print('*', i, j);
	      }
	    }
#endif
	/* show ball of whatever */
	put_qio();

	for (i = (y - 2); i <= (y + 2); i++)
	  for (j = (x - 2); j <= (x + 2); j++)
	    if (in_bounds(i, j) && panel_contains(i, j) &&
		(distance(y, x, i, j) <= max_dis))
	      lite_spot(i, j);
	/* End  explosion.		     */
	if (thit == 1) {
	  lower_monster_name(m_name, m_ptr, r_ptr);
	  (void) sprintf(out_val,
			 "The %s envelopes %s!",
			 descrip, m_name);
	  msg_print(out_val);
	  if (!tkill) {
	    monster_name(m_name, m_ptr, r_ptr);
	    if (harm_type &
		(r_ptr->cdefense &
		 (IM_FIRE|IM_POISON|IM_FROST|IM_LIGHTNING|IM_ACID))) {
	      (void) sprintf (out_val, "%s looks unharmed.",
			      m_name);
	    } else {
	      sprintf(out_val,pain_message((int)monptr,dam),
		      m_name);
	    }
	    msg_print (out_val);
	  }
	}
	else if (thit > 1) {
	  (void) sprintf(out_val,
			 "The %s envelopes several creatures!",
			 descrip);
	  msg_print(out_val);
	}
	if (tkill == 1) {
	  msg_print("There is a scream of agony!");
	} else if (tkill > 1) {
	  msg_print("There are several screams of agony!");
	}
	if (tkill >= 0)
	  prt_experience();
	/* End ball hitting.		     */
      } else if (panel_contains(y, x) && (py.flags.blind < 1)) {
#ifdef TC_COLOR
	if (!no_color_flag) textcolor(bolt_color(typ));
#endif	  
	print(bolt_shape(dir), y, x);
	put_qio();
#ifdef TC_COLOR
	if (!no_color_flag) textcolor(LIGHTGRAY);
#endif	  
#ifdef MSDOS
	delay(23); /* milli-secs -CFT */
#endif
      }
      oldy = y;
      oldx = x;
    }
  } while (!flag);
}

/*Lightning ball in all directions			  SM   */
void starball(y,x)
register int y, x;
{
  register int i;

  if (py.flags.blind < 1)
  for (i = 1; i <= 9; i++)
    if (i != 5)
      fire_ball(GF_LIGHTNING, i, y, x, 150,
		"huge ball of Electricity");
}

/* Breath weapon works like a fire_ball, but affects the player. */
/* Note the area affect.			      -RAK-   */
void breath(typ, y, x, dam_hp, ddesc, monptr)
int typ, y, x, dam_hp;
char *ddesc;
int monptr;
{
  register int i, j;
  int dam, max_dis;
  int32u weapon_type, harm_type;
  int32u tmp, treas;
  int (*destroy)();
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
#ifdef TC_COLOR
  int ttyp;
#endif

  max_dis = 2;
#ifdef TC_COLOR
  switch( typ ){
    case GF_ARROW: case GF_PLASMA: case GF_NETHER: case GF_WATER:
    case GF_CHAOS: case GF_SHARDS: case GF_SOUND: case GF_CONFUSION:
    case GF_DISENCHANT: case GF_NEXUS: case GF_FORCE: case GF_INERTIA:
    case GF_LIGHT: case GF_DARK: case GF_TIME: case GF_GRAVITY:
    case GF_METEOR: case GF_MANA:
      ttyp = GF_MAGIC_MISSILE; /* use m.m. for dam/destroy calcs... */
      break;
    default: ttyp = typ;
  }
  get_flags(ttyp, &weapon_type, &harm_type, &destroy);
#else
  get_flags(typ, &weapon_type, &harm_type, &destroy);
#endif

#ifdef TC_COLOR /* do ball a little diff, so we can see the colors... */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(y, x, i, j) && (cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && !(py.flags.status & PY_BLIND)) {
	      if (!no_color_flag) textcolor(bolt_color(typ));
	      print('*', i, j);
	      put_qio();
	      if (!no_color_flag) textcolor(LIGHTGRAY); /* prob don't need here, but... -CFT */
	      }
        if (!(py.flags.status & PY_BLIND))
	  delay(75); /* millisecs, so we see the ball we just drew */
	/* now go over area of affect and DO something... */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      c_ptr = &cave[i][j];
	      if ((c_ptr->tptr != 0) &&
		  (*destroy)(&t_list[c_ptr->tptr]))
		(void) delete_object(i, j);
	      if (c_ptr->fval <= MAX_OPEN_SPACE) {
		if (c_ptr->cptr > 1) {
		  m_ptr = &m_list[c_ptr->cptr];
		  r_ptr = &c_list[m_ptr->mptr];
		  dam = dam_hp;
		  if (harm_type & r_ptr->cdefense) {
		    if (harm_type == EVIL)
		      dam = dam*2;
		    else
		      dam = dam/9;
		  }
		  dam = (dam/(distance(i, j, y, x)+1));
		  /* can not call mon_take_hit here, since player does not
		     get experience for kill */
		  m_ptr->hp = m_ptr->hp - dam;
		  m_ptr->csleep = 0;
	  if ((r_ptr->cdefense & UNIQUE) && (m_ptr->hp < 0))
	    m_ptr->hp = 0; /* prevent unique monster from death by other
	    		     monsters.  It causes trouble (monster not
	    		     marked as dead, quest monsters don't satisfy
	    		     quest, etc).  So, we let then live, but
	    		     extremely wimpy.  This isn't great, because
	    		     monster might heal itself before player's
	    		     next swing... -CFT */
		  if (m_ptr->hp < 0)
		    {
		      treas = monster_death((int)m_ptr->fy, (int)m_ptr->fx,
					    r_ptr->cmove, 0, 0);
		      if (m_ptr->ml)
			{
			  tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE)
			    >> CM_TR_SHIFT;
			  if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
			    treas = (treas & ~CM_TREASURE)|(tmp<<CM_TR_SHIFT);
			  c_recall[m_ptr->mptr].r_cmove = treas |
			    (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE);
			}

		      /* It ate an already processed monster.Handle normally.*/
		      if (monptr < c_ptr->cptr)
			delete_monster((int) c_ptr->cptr);
		      /* If it eats this monster, an already processed monster
			 will take its place, causing all kinds of havoc.
			 Delay the kill a bit. */
		      else
			fix1_delete_monster((int) c_ptr->cptr);
		    }
		}
	      else if (c_ptr->cptr == 1)
		{
		  dam = (dam_hp/(distance(i, j, y, x)+1));
		  /* let's do at least one point of damage */
		  /* prevents randint(0) problem with poison_gas, also */
		  if (dam <= 0)
		    dam = 1;
                  if (dam>1600)
		    dam = 1600;
		  switch(ttyp)
		    {
		    case GF_LIGHTNING: light_dam(dam, ddesc); break;
		    case GF_POISON_GAS: poison_gas(dam, ddesc); break;
		    case GF_ACID: acid_dam(dam, ddesc); break;
		    case GF_FROST: cold_dam(dam, ddesc); break;
		    case GF_FIRE: fire_dam(dam, ddesc); break;
		    case GF_MAGIC_MISSILE: take_hit(dam, ddesc); break;
		    }
		}
	    }
	}
#else
  for (i = y-2; i <= y+2; i++)
    for (j = x-2; j <= x+2; j++)
      if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
	  && los(y, x, i, j))
	{
	  c_ptr = &cave[i][j];
	  if ((c_ptr->tptr != 0) &&
	      (*destroy)(&t_list[c_ptr->tptr]))
	    (void) delete_object(i, j);
	  if (c_ptr->fval <= MAX_OPEN_SPACE)
	    {
	      /* must test status bit, not py.flags.blind here, flag could have
		 been set by a previous monster, but the breath should still
		 be visible until the blindness takes effect */
	      if (panel_contains(i, j) && !(py.flags.status & PY_BLIND))
		print('*', i, j);
	      if (c_ptr->cptr > 1)
		{
		  m_ptr = &m_list[c_ptr->cptr];
		  r_ptr = &c_list[m_ptr->mptr];
		  dam = dam_hp;
		  if (harm_type & r_ptr->cdefense) {
		    if (harm_type == EVIL)
		      dam = dam*2;
		    else
		      dam = dam/9;
		  }
		  dam = (dam/(distance(i, j, y, x)+1));
		  /* can not call mon_take_hit here, since player does not
		     get experience for kill */
		  m_ptr->hp = m_ptr->hp - dam;
		  m_ptr->csleep = 0;
	  if ((r_ptr->cdefense & UNIQUE) && (m_ptr->hp < 0))
	    m_ptr->hp = 0; /* prevent unique monster from death by other
	    		     monsters.  It causes trouble (monster not
	    		     marked as dead, quest monsters don't satisfy
	    		     quest, etc).  So, we let then live, but
	    		     extremely wimpy.  This isn't great, because
	    		     monster might heal itself before player's
	    		     next swing... -CFT */
		  if (m_ptr->hp < 0)
		    {
		      treas = monster_death((int)m_ptr->fy, (int)m_ptr->fx,
					    r_ptr->cmove, 0, 0);
		      if (m_ptr->ml)
			{
			  tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE)
			    >> CM_TR_SHIFT;
			  if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
			    treas = (treas & ~CM_TREASURE)|(tmp<<CM_TR_SHIFT);
			  c_recall[m_ptr->mptr].r_cmove = treas |
			    (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE);
			}

		      /* It ate an already processed monster.Handle normally.*/
		      if (monptr < c_ptr->cptr)
			delete_monster((int) c_ptr->cptr);
		      /* If it eats this monster, an already processed monster
			 will take its place, causing all kinds of havoc.
			 Delay the kill a bit. */
		      else
			fix1_delete_monster((int) c_ptr->cptr);
		    }
		}
	      else if (c_ptr->cptr == 1)
		{
		  dam = (dam_hp/(distance(i, j, y, x)+1));
		  /* let's do at least one point of damage */
		  /* prevents randint(0) problem with poison_gas, also */
		  if (dam <= 0)
		    dam = 1;
                  if (dam>1600)
		    dam = 1600;
		  switch(typ)
		    {
		    case GF_LIGHTNING: light_dam(dam, ddesc); break;
		    case GF_POISON_GAS: poison_gas(dam, ddesc); break;
		    case GF_ACID: acid_dam(dam, ddesc); break;
		    case GF_FROST: cold_dam(dam, ddesc); break;
		    case GF_FIRE: fire_dam(dam, ddesc); break;
		    case GF_MAGIC_MISSILE: take_hit(dam, ddesc); break;
		    }
		}
	    }
	}
#endif
  /* show the ball of gas */
  put_qio();

  for (i = (y - 2); i <= (y + 2); i++)
    for (j = (x - 2); j <= (x + 2); j++)
      if (in_bounds(i, j) && panel_contains(i, j) &&
	  (distance(y, x, i, j) <= max_dis))
	lite_spot(i, j);
}


/* Recharge a wand, staff, or rod.  Sometimes the item breaks. -RAK-*/
int recharge(num)
register int num;
{
  int i, j, item_val;
  register int res;
  register inven_type *i_ptr;

  res = FALSE;
  if (!find_range(TV_STAFF, TV_WAND, &i, &j))
    msg_print("You have nothing to recharge.");
  else if (get_item(&item_val, "Recharge which item?", i, j, 0))
    {
      i_ptr = &inventory[item_val];
      res = TRUE;
      /* recharge I = recharge(20) = 1/6 failure for empty 10th level wand */
      /* recharge II = recharge(60) = 1/10 failure for empty 10th level wand */
      /* make it harder to recharge high level, and highly charged wands */
      if (randint((num+100-(int)i_ptr->level-(10*i_ptr->p1))/15) == 1)
	{
	  msg_print("There is a bright flash of light.");
	  inven_destroy(item_val);
	}
      else
	{
	  num = (num/(i_ptr->level+2)) + 1;
	  i_ptr->p1 += 2 + randint(num);
	  if (known2_p(i_ptr))
	    clear_known2(i_ptr);
	  clear_empty(i_ptr);
	}
    }
  return(res);
}


/* Increase or decrease a creatures hit points		-RAK-	*/
int hp_monster(dir, y, x, dam)
int dir, y, x, dam;
{
  register int i;
  int flag, dist, monster;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  monster = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  flag = TRUE;
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  monster_name (m_name, m_ptr, r_ptr);
	  monster = TRUE;
	  i = mon_take_hit((int)c_ptr->cptr, dam);
	  if (i >= 0)
	    {
	      (void) sprintf(out_val, "%s dies in a fit of agony.", m_name);
	      msg_print(out_val);
	      prt_experience();
	    }
	  else if (dam > 0)
	    {
	      (void) sprintf (out_val,
	          pain_message((int)c_ptr->cptr,dam),m_name);
	      msg_print(out_val);
	    }
	}
    }
  while (!flag);
  return(monster);
}


/* Drains life; note it must be living.		-RAK-	*/
int drain_life(dir, y, x, dam)
int dir, y, x, dam;
{
  register int i;
  int flag, dist, drain;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  drain = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  flag = TRUE;
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  if (((r_ptr->cdefense & UNDEAD) == 0) &&
	      ((r_ptr->cdefense & DEMON) == 0))
	    {
	      drain = TRUE;
	      monster_name (m_name, m_ptr, r_ptr);
	      i = mon_take_hit((int)c_ptr->cptr, dam);
	      if (i >= 0)
		{
		  (void) sprintf(out_val, "%s dies in a fit of agony.",m_name);
		  msg_print(out_val);
		  prt_experience();
		}
	      else
		{
		  (void) sprintf (out_val,
			pain_message((int)c_ptr->cptr,dam),m_name);
		  msg_print(out_val);
		}
	    }
	  else {
	    if (r_ptr->cdefense & UNDEAD)
	      c_recall[m_ptr->mptr].r_cdefense |= UNDEAD;
	    else
	      c_recall[m_ptr->mptr].r_cdefense |= DEMON;
	  }
	}
    }
  while (!flag);
  return(drain);
}


/* Increase or decrease a creatures speed		-RAK-	*/
/* NOTE: cannot slow a winning creature (BALROG)		 */
int speed_monster(dir, y, x, spd)
int dir, y, x, spd;
{
  int flag, dist, speed;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  speed = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  flag = TRUE;
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  monster_name (m_name, m_ptr, r_ptr);
	  if (spd > 0)
	    {
	      m_ptr->cspeed += spd;
	      m_ptr->csleep = 0;
	      (void) sprintf (out_val, "%s starts moving faster.", m_name);
	      msg_print (out_val);
	      speed = TRUE;
	    }
	  else if ((r_ptr->level >
		    randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) ||
		   (r_ptr->cdefense & UNIQUE))
	    {
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	  else
	    {
	      m_ptr->cspeed += spd;
	      m_ptr->csleep = 0;
	      (void) sprintf (out_val, "%s starts moving slower.", m_name);
	      msg_print (out_val);
	      speed = TRUE;
	    }
	}
    }
  while (!flag);
  return(speed);
}


/* Confuse a creature					-RAK-	*/
int confuse_monster(dir, y, x)
int dir, y, x;
{
  int flag, dist, confuse;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  confuse = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  monster_name (m_name, m_ptr, r_ptr);
	  flag = TRUE;
	  if ((r_ptr->level >
	       randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) ||
	      (r_ptr->cdefense & UNIQUE))
	    {
	      if (m_ptr->ml && (r_ptr->cdefense & CHARM_SLEEP))
		c_recall[m_ptr->mptr].r_cdefense |= CHARM_SLEEP;
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	  else
	    {
	      m_ptr->confused = TRUE;
	      confuse = TRUE;
	      m_ptr->csleep = 0;
	      (void) sprintf(out_val, "%s appears confused.", m_name);
	      msg_print(out_val);
	    }
	}
    }
  while (!flag);
  return(confuse);
}


/* Sleep a creature.					-RAK-	*/
int sleep_monster(dir, y, x)
int dir, y, x;
{
  int flag, dist, sleep;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  sleep = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];



	  flag = TRUE;
	  monster_name (m_name, m_ptr, r_ptr);
	  if ((r_ptr->level >
	       randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) ||
	      (r_ptr->cdefense & UNIQUE)||(r_ptr->cdefense & CHARM_SLEEP))
	    {
	      if (m_ptr->ml && (r_ptr->cdefense & CHARM_SLEEP))
		c_recall[m_ptr->mptr].r_cdefense |= CHARM_SLEEP;
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	  else
	    {
	      m_ptr->csleep = 500;
	      sleep = TRUE;
	      (void) sprintf(out_val, "%s falls asleep.", m_name);
	      msg_print(out_val);
	    }
	}
    }
  while (!flag);
  return(sleep);
}


/* Turn stone to mud, delete wall.			-RAK-	*/
int wall_to_mud(dir, y, x)
int dir, y, x;
{
  int i, wall, dist;
  bigvtype out_val, tmp_str;
  register int flag;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype m_name;

  wall = FALSE;
  flag = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      /* note, this ray can move through walls as it turns them to mud */
      if (dist == OBJ_BOLT_RANGE)
	flag = TRUE;
      if (c_ptr->fval == BOUNDARY_WALL)
	{
	  flag = TRUE;
	  if (test_light(y, x)) msg_print("The wall resists your spell.");
	}
      else if ((c_ptr->fval >= MIN_CAVE_WALL))
	{
	  flag = TRUE;
	  (void) twall(y, x, 1, 0);
	  if (test_light(y, x))
	    {
	      msg_print("The wall turns into mud.");
	      wall = TRUE;
	    }
	}
      else if ((c_ptr->tptr != 0) && (c_ptr->fval >= MIN_CLOSED_SPACE))
	{
	  flag = TRUE;
	  if (panel_contains(y, x) && test_light(y, x))
	    {
	      objdes(tmp_str, &t_list[c_ptr->tptr], FALSE);
	      (void) sprintf(out_val, "The %s turns into mud.", tmp_str);
	      msg_print(out_val);
	      wall = TRUE;
	    }
	  (void) delete_object(y, x);
	}
      if (c_ptr->cptr > 1)
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  if (HURT_ROCK & r_ptr->cdefense)
	    {
	      monster_name (m_name, m_ptr, r_ptr);
	      flag = m_ptr->ml;
	      i = mon_take_hit((int)c_ptr->cptr, (20 + randint(30)));
	      if (flag)
		{
		  if (i >= 0)
		    {
		      c_recall[i].r_cdefense |= HURT_ROCK;
		      (void) sprintf(out_val, "%s dissolves!", m_name);
		      msg_print(out_val);
		      prt_experience(); /* print msg before calling prt_exp */
		    }
		  else
		    {
		      c_recall[m_ptr->mptr].r_cdefense |= HURT_ROCK;
		      (void) sprintf(out_val, "%s grunts in pain!",m_name);
		      msg_print(out_val);
		    }
		}
	      flag = TRUE;
	    }
	}
    }
  while (!flag);
  return(wall);
}


/* Destroy all traps and doors in a given direction	-RAK-	*/
int td_destroy2(dir, y, x)
int dir, y, x;
{
  register int destroy2, dist;
  register cave_type *c_ptr;
  register inven_type *t_ptr;

  destroy2 = FALSE;
  dist= 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      /* must move into first closed spot, as it might be a secret door */
      if (c_ptr->tptr != 0)
	{
	  t_ptr = &t_list[c_ptr->tptr];
	  if ((t_ptr->tval == TV_CHEST) || (t_ptr->tval == TV_INVIS_TRAP) ||
	      (t_ptr->tval == TV_VIS_TRAP) || (t_ptr->tval == TV_OPEN_DOOR) ||
	      (t_ptr->tval == TV_CLOSED_DOOR)
	      || (t_ptr->tval == TV_SECRET_DOOR))
	    {
	      if (delete_object(y, x))
		{
		  msg_print("There is a bright flash of light!");
		  destroy2 = TRUE;
		}
	    }
	}
    }
  while ((dist <= OBJ_BOLT_RANGE) || c_ptr->fval <= MAX_OPEN_SPACE);
  return(destroy2);
}


/* Polymorph a monster					-RAK-	*/
/* NOTE: cannot polymorph a winning creature (BALROG)		 */
int poly_monster(dir, y, x)
int dir, y, x;
{
  int dist, flag, flag2, poly;
  register cave_type *c_ptr;
  register creature_type *r_ptr;
  register monster_type *m_ptr;
  vtype out_val, m_name;

  poly = FALSE;
  flag = FALSE;
  flag2= FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  if ((r_ptr->level <
	       randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) &&
	       !(r_ptr->cdefense & UNIQUE))
	    {
	      flag2=FALSE;
	      do {
		delete_monster((int)c_ptr->cptr);
		place_monster(y, x, randint(m_level[MAX_MONS_LEVEL]-m_level[0])
			      - 1 + m_level[0], FALSE);
		/* don't test c_ptr->fm here, only pl/tl */
                c_ptr = &cave[y][x];
		if (!(c_list[m_list[c_ptr->cptr].mptr].cdefense & UNIQUE))
		  flag2 = TRUE;
              } while (!flag2);
	      if (panel_contains(y, x) && (c_ptr->tl || c_ptr->pl))
		poly = TRUE;
	    }
	  else
	    {
	      monster_name (m_name, m_ptr, r_ptr);
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	}
    }
  while (!flag);
  return(poly);
}


/* Create a wall.					-RAK-	*/
int build_wall(dir, y, x)
int dir, y, x;
{
  register int i;
  int build, damage, dist, flag;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype m_name, out_val;

  build = FALSE;
  dist = 0;
  flag = FALSE;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else
	{
	  if (c_ptr->cptr > 1)
	    {
	      /* stop the wall building */
	      flag = TRUE;
	      m_ptr = &m_list[c_ptr->cptr];
	      r_ptr = &c_list[m_ptr->mptr];

	      if (!(r_ptr->cmove & CM_PHASE))
		{
		  /* monster does not move, can't escape the wall */
		  if (r_ptr->cmove & CM_ATTACK_ONLY)
		    damage = 250; /* this will kill everything */
		  else
		    damage = damroll (4, 8);

		  monster_name (m_name, m_ptr, r_ptr);
		  (void) sprintf (out_val, "%s wails out in pain!", m_name);
		  msg_print (out_val);
		  i = mon_take_hit((int)c_ptr->cptr, damage);
		  if (i >= 0)
		    {
		      (void) sprintf (out_val, "%s is embedded in the rock.",
				      m_name);
		      msg_print (out_val);
		      /* prt_experience(); */
		    }
		}
	      else if (r_ptr->cchar == 'E' || r_ptr->cchar == 'X')
		{
		  /* must be an earth elemental or an earth spirit, or a Xorn
		     increase its hit points */
		  m_ptr->hp += damroll(4, 8);
		}
	    }
	  if (c_ptr->tptr != 0)
	    if ((t_list[c_ptr->tptr].tval >= TV_MIN_WEAR) &&
	        (t_list[c_ptr->tptr].tval <= TV_MAX_WEAR) &&
	        (t_list[c_ptr->tptr].flags2 & TR_ARTIFACT))
	      continue; /* don't bury the artifact... */
	    else
	      (void) delete_object(y, x);
	  c_ptr->fval  = MAGMA_WALL;
	  c_ptr->fm = FALSE;
	  lite_spot(y, x);
	  i++;
	  build = TRUE;
	}
    }
  while (!flag);
  return(build);
}


/* Replicate a creature					-RAK-	*/
int clone_monster(dir, y, x)
int dir, y, x;
{
  register cave_type *c_ptr;
  register int dist, flag;

  dist = 0;
  flag = FALSE;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  m_list[c_ptr->cptr].csleep = 0;
	  /* monptr of 0 is safe here, since can't reach here from creatures */
	  return multiply_monster(y, x, (int)m_list[c_ptr->cptr].mptr, 0);
	}
    }
  while (!flag);
  return(FALSE);
}


/* Move the creature record to a new location		-RAK-	*/
void teleport_away(monptr, dis)
int monptr, dis;
{
  register int yn, xn, ctr;
  register monster_type *m_ptr;

  m_ptr = &m_list[monptr];
  ctr = 0;
  do
    {
      do
	{
	  yn = m_ptr->fy + (randint(2*dis+1) - (dis + 1));
	  xn = m_ptr->fx + (randint(2*dis+1) - (dis + 1));
	}
      while (!in_bounds(yn, xn));
      ctr++;
      if (ctr > 9)
	{
	  ctr = 0;
	  dis += 5;
	}
    }
  while ((cave[yn][xn].fval >= MIN_CLOSED_SPACE) || (cave[yn][xn].cptr != 0));
  move_rec((int)m_ptr->fy, (int)m_ptr->fx, yn, xn);
  lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
  m_ptr->fy = yn;
  m_ptr->fx = xn;
  /* this is necessary, because the creature is not currently visible
     in its new position */
  m_ptr->ml = FALSE;
  m_ptr->cdis = distance (char_row, char_col, yn, xn);
  update_mon (monptr);
}


/* Teleport player to spell casting creature		-RAK-	*/
void teleport_to(ny, nx)
int ny, nx;
{
  int dis, ctr, y, x;
  register int i, j;
  register cave_type *c_ptr;

  dis = 1;
  ctr = 0;
  do
    {
      do { /* bounds check added -CFT */
        y = ny + (randint(2*dis+1) - (dis + 1));
        x = nx + (randint(2*dis+1) - (dis + 1));
      } while (!in_bounds(ny, nx));
      ctr++;
      if (ctr > 9)
	{
	  ctr = 0;
	  dis++;
	}
    }
  while ((cave[y][x].fval >= MIN_CLOSED_SPACE) || (cave[y][x].cptr >= 2));
  move_rec(char_row, char_col, y, x);
  for (i = char_row-1; i <= char_row+1; i++)
    for (j = char_col-1; j <= char_col+1; j++)
      {
	c_ptr = &cave[i][j];
	c_ptr->tl = FALSE;
	lite_spot(i, j);
      }
  lite_spot(char_row, char_col);
  char_row = y;
  char_col = x;
  check_view();
  /* light creatures */
  creatures(FALSE);
}


/* Teleport all creatures in a given direction away	-RAK-	*/
int teleport_monster(dir, y, x)
int dir, y, x;
{
  register int flag, result, dist;
  register cave_type *c_ptr;

  flag = FALSE;
  result = FALSE;
  dist = 0;
  do
    {
      (void) mmove(dir, &y, &x);
      dist++;
      c_ptr = &cave[y][x];
      if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
	flag = TRUE;
      else if (c_ptr->cptr > 1)
	{
	  m_list[c_ptr->cptr].csleep = 0; /* wake it up */
	  teleport_away((int)c_ptr->cptr, MAX_SIGHT*5);
	  result = TRUE;
	}
    }
  while (!flag);
  return(result);
}


/* Delete all creatures within max_sight distance	-RAK-	*/
/* NOTE : Winning creatures cannot be genocided			 */
int mass_genocide(spell)
  int spell;
{
  register int i, result;
  register monster_type *m_ptr;
  register creature_type *r_ptr;

  result = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      if (((m_ptr->cdis <= MAX_SIGHT) && ((r_ptr->cmove & CM_WIN)==0) &&
	  ((r_ptr->cdefense & UNIQUE)==0))||(wizard &&
					     (m_ptr->cdis <= MAX_SIGHT)))
	{
	  delete_monster(i);
	  if (spell) {
	    take_hit(randint(3),"the strain of casting Mass Genocide");
	    prt_chp();
	    put_qio();
#ifdef MSDOS
      delay(50); /* milli-secs -CFT */
#else
      usleep(50000); /* u-secs */
#endif
          }
	  result = TRUE;
	}
    }
  return(result);
}

/* Delete all creatures of a given type from level.	-RAK-	*/
/* This does not keep creatures of type from appearing later.	 */
/* NOTE : Winning creatures can not be genocided. */
int genocide(spell)
  int spell;
{
  register int i, killed;
  char typ;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val;

  killed = FALSE;
  if (get_com("Which type of creature do you wish exterminated?", &typ))
    for (i = mfptr - 1; i >= MIN_MONIX; i--)
      {
	m_ptr = &m_list[i];
	r_ptr = &c_list[m_ptr->mptr];
	if (typ == c_list[m_ptr->mptr].cchar)
	  if ((r_ptr->cmove & CM_WIN) == 0)
	    {
	      delete_monster(i);
              if (spell) {
	        take_hit(randint(4),"the strain of casting Genocide");
	        prt_chp();
                put_qio();
#ifdef MSDOS
      delay(50); /* milli-secs -CFT */
#else
      usleep(50000); /* u-secs */
#endif
              }
	      killed = TRUE;
	    }
	  else
	    {
	      /* genocide is a powerful spell, so we will let the player
		 know the names of the creatures he did not destroy,
		 this message makes no sense otherwise */
	      if (r_ptr->cdefense & UNIQUE)
		(void) sprintf(out_val, "%s is unaffected.", r_ptr->name);
	      else
		(void) sprintf(out_val, "The %s is unaffected.", r_ptr->name);
	      msg_print(out_val);
	    }
      }
  return(killed);
}


/* Change speed of any creature .			-RAK-	*/
/* NOTE: cannot slow a winning creature (BALROG)		 */
int speed_monsters(spd)
int spd;
{
  register int i, speed;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  speed = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      monster_name (m_name, m_ptr, r_ptr);

      if (!los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	/* do nothing */
	;
      else if (spd > 0)
	{
	  m_ptr->cspeed += spd;
	  m_ptr->csleep = 0;
	  if (m_ptr->ml)
	    {
	      speed = TRUE;
	      (void) sprintf (out_val, "%s starts moving faster.", m_name);
	      msg_print (out_val);
	    }
	}
      else if ((r_ptr->level <
	       randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) &&
	       !(r_ptr->cdefense & UNIQUE))
  	{
	  m_ptr->cspeed += spd;
	  m_ptr->csleep = 0;
	  if (m_ptr->ml)
	    {
	      (void) sprintf (out_val, "%s starts moving slower.", m_name);
	      msg_print (out_val);
	      speed = TRUE;
	    }
	}
      else if (m_ptr->ml)
	{
	  (void) sprintf(out_val, "%s is unaffected.", m_name);
	  msg_print(out_val);
	}
    }
  return(speed);
}


/* Sleep any creature .		-RAK-	*/
int sleep_monsters2()
{
  register int i, sleep;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  sleep = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      monster_name (m_name, m_ptr, r_ptr);
      if (!los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	/* do nothing */
	;
      else if ((r_ptr->level >
		randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10) ||
	       (r_ptr->cdefense & UNIQUE)||(r_ptr->cdefense & CHARM_SLEEP))
	{
	  if (m_ptr->ml)
	    {
	      if (r_ptr->cdefense & CHARM_SLEEP)
		c_recall[m_ptr->mptr].r_cdefense |= CHARM_SLEEP;
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	}
      else
	{
	  m_ptr->csleep = 500;
	  if (m_ptr->ml)
	    {
	      (void) sprintf(out_val, "%s falls asleep.", m_name);
	      msg_print(out_val);
	      sleep = TRUE;
	    }
	}
    }
  return(sleep);
}

/* Polymorph any creature that player can see.	-RAK-	*/
/* NOTE: cannot polymorph a winning creature (BALROG)		 */
int mass_poly()
{
  register int i;
  int y, x, mass;
  register monster_type *m_ptr;
  register creature_type *r_ptr;

  mass = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->cdis < MAX_SIGHT)
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  if ((r_ptr->cmove & CM_WIN) == 0)
	    {
	      y = m_ptr->fy;
	      x = m_ptr->fx;
	      delete_monster(i);
	      place_monster(y, x, randint(m_level[MAX_MONS_LEVEL]-m_level[0])
			    - 1 + m_level[0], FALSE);
	      mass = TRUE;
	    }
	}
    }
  return(mass);
}

int chaos(m_ptr2)
monster_type *m_ptr2;
{
  register int i;
  int y, x, mass;
  register monster_type *m_ptr;
  register creature_type *r_ptr;

  mass = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->cdis < MAX_SIGHT)
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  if (!(r_ptr->cdefense & UNIQUE) && m_ptr!=m_ptr2)
	    {
	      y = m_ptr->fy;
	      x = m_ptr->fx;
	      delete_monster(i);
	      place_monster(y, x, randint(m_level[MAX_MONS_LEVEL]-m_level[0])
			    - 1 + m_level[0], FALSE);
	      mass = TRUE;
	    }
	}
    }
  return(mass);
}

/* Display evil creatures on current panel		-RAK-	*/
int detect_evil()
{
  register int i, flag;
  register monster_type *m_ptr;

  flag = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) &&
	  (EVIL & c_list[m_ptr->mptr].cdefense))
	{
	  m_ptr->ml = TRUE;
	  /* works correctly even if hallucinating */
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(c_list[m_ptr->mptr].color);
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#else
	  print((char)c_list[m_ptr->mptr].cchar, (int)m_ptr->fy,
		(int)m_ptr->fx);
#endif
	  flag = TRUE;
	}
    }
  if (flag)
    {
      msg_print("You sense the presence of evil!");
      msg_print(NULL);
      /* must unlight every monster just lighted */
      creatures(FALSE);
    }
  return(flag);
}


/* Change players hit points in some manner		-RAK-	*/
int hp_player(num)
int num;
{
  register int res;
  register struct misc *m_ptr;

  res = FALSE;
  m_ptr = &py.misc;
  if (m_ptr->chp < m_ptr->mhp)
    {
      m_ptr->chp += num;
      if (m_ptr->chp > m_ptr->mhp)
	{
	  m_ptr->chp = m_ptr->mhp;
	  m_ptr->chp_frac = 0;
	}
      prt_chp();

      num = num / 5;
      if (num < 3) {
	if (num == 0) msg_print("You feel a little better.");
	else	      msg_print("You feel better.");
      } else {
	if (num < 7) msg_print("You feel much better.");
	else	     msg_print("You feel very good.");
      }
      res = TRUE;
    }
  return(res);
}


/* Cure players confusion				-RAK-	*/
int cure_confusion()
{
  register int cure;
  register struct flags *f_ptr;

  cure = FALSE;
  f_ptr = &py.flags;
  if (f_ptr->confused > 1)
    {
      f_ptr->confused = 1;
      cure = TRUE;
    }
  return(cure);
}


/* Cure players blindness				-RAK-	*/
int cure_blindness()
{
  register int cure;
  register struct flags *f_ptr;

  cure = FALSE;
  f_ptr = &py.flags;
  if (f_ptr->blind > 1)
    {
      f_ptr->blind = 1;
      cure = TRUE;
    }
  return(cure);
}


/* Cure poisoning					-RAK-	*/
int cure_poison()
{
  register int cure;
  register struct flags *f_ptr;

  cure = FALSE;
  f_ptr = &py.flags;
  if (f_ptr->poisoned > 1)
    {
      f_ptr->poisoned = 1;
      cure = TRUE;
    }
  return(cure);
}


/* Cure the players fear				-RAK-	*/
int remove_fear()
{
  register int result;
  register struct flags *f_ptr;

  result = FALSE;
  f_ptr = &py.flags;
  if (f_ptr->afraid > 1)
    {
      f_ptr->afraid = 1;
      result = TRUE;
    }
  return(result);
}


/* This is a fun one.  In a given block, pick some walls and	*/
/* turn them into open spots.  Pick some open spots and turn	 */
/* them into walls.  An "Earthquake" effect.	       -RAK-   */
void earthquake()
{
  register int i, j;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int kill, damage, tmp, y, x;
  vtype out_val, m_name;

  for (i = char_row-8; i <= char_row+8; i++)
    for (j = char_col-8; j <= char_col+8; j++)
      if (((i != char_row) || (j != char_col)) &&
	  in_bounds(i, j) && (randint(8) == 1))
	{
	  c_ptr = &cave[i][j];
	  if (c_ptr->tptr != 0)
	    if ((t_list[c_ptr->tptr].tval >= TV_MIN_WEAR) &&
	    	(t_list[c_ptr->tptr].tval <= TV_MAX_WEAR) &&
	    	(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT))
	      continue; /* don't touch artifacts... */
	    else
	      (void) delete_object(i, j);
	  if (c_ptr->cptr > 1)
	    {
	      m_ptr = &m_list[c_ptr->cptr];
	      r_ptr = &c_list[m_ptr->mptr];

	      if (!(r_ptr->cmove&CM_PHASE) && !(r_ptr->cdefense&BREAK_WALL))
		{
		  if ((movement_rate (m_ptr->cspeed) == 0) ||
		      (r_ptr->cmove & CM_ATTACK_ONLY))
		    /* monster can not move to escape the wall */
		    kill = TRUE;
		  else
		    {
		      /* only kill if there is nowhere for the monster to
			 escape to */
		      kill = TRUE;
		      for (y = i-1; y <= i+1; y++)
			for (x = j-1; x <= j+1; x++)
			  if (cave[y][x].fval >= MIN_CLOSED_SPACE)
			    kill = FALSE;
		    }
		  if (kill)
		    damage = 320;  /* this will kill everything */
		  else
		    damage = damroll (4, 8);
		  monster_name (m_name, m_ptr, r_ptr);
		  (void) sprintf (out_val, "%s wails out in pain!", m_name);
		  msg_print (out_val);
		  i = mon_take_hit((int)c_ptr->cptr, damage);
		  if (i >= 0)
		    {
		      (void) sprintf (out_val, "%s is embedded in the rock.",
				      m_name);
		      msg_print (out_val);
		      /* prt_experience(); */
		    }
		}
	    }

	  if ((c_ptr->fval >= MIN_CAVE_WALL) && (c_ptr->fval != BOUNDARY_WALL))
	    {
	      c_ptr->fval  = CORR_FLOOR;
	      c_ptr->pl = FALSE;
	      c_ptr->fm = FALSE;
	    }
	  else if (c_ptr->fval <= MAX_CAVE_FLOOR)
	    {
	      tmp = randint(10);
	      if (tmp < 6)
		c_ptr->fval  = QUARTZ_WALL;
	      else if (tmp < 9)
		c_ptr->fval  = MAGMA_WALL;
	      else
		c_ptr->fval  = GRANITE_WALL;

	      c_ptr->fm = FALSE;
	    }
	  lite_spot(i, j);
	}
}


/* Evil creatures don't like this.		       -RAK-   */
int protect_evil()
{
  register int res;
  register struct flags *f_ptr;

  f_ptr = &py.flags;
  if (f_ptr->protevil == 0)
    res = TRUE;
  else
    res = FALSE;
  f_ptr->protevil += randint(25) + 3*py.misc.lev;
  return(res);
}


/* Create some high quality mush for the player.	-RAK-	*/
void create_food()
{
  register cave_type *c_ptr;

  c_ptr = &cave[char_row][char_col];
  if (c_ptr->tptr != 0)
    {
      /* take no action here, don't want to destroy object under player */
      msg_print ("There is already an object under you.");
      /* set free_turn_flag so that scroll/spell points won't be used */
      free_turn_flag = TRUE;
    }
  else
    {
      int cur_pos, tmp;

      cur_pos = popt();
      cave[char_row][char_col].tptr = cur_pos;
      tmp = get_obj_num(dun_level, FALSE);
      invcopy(&t_list[cur_pos], OBJ_MUSH);
      msg_print ("You feel something roll beneath your feet.");
    }
}

int banish_creature(cflag, distance)
int32u cflag;
int distance;
{
  register int i;
  int k, dispel, visible;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  dispel = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if ((cflag & c_list[m_ptr->mptr].cdefense) &&
	  los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  c_recall[m_ptr->mptr].r_cdefense |= cflag;
	  (void) teleport_away(i,distance);
	  dispel=TRUE;
	}
    }
  return (dispel);
}

int probing()
{
  register int i;
  int probe;
  char ch;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  msg_print("Probing...");
  probe = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      if (los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	{
	  if (!m_ptr->ml) {
	    break;
	  } else {
	    if (r_ptr->cdefense & UNIQUE)
	      sprintf(m_name, "%s", r_ptr->name);
	    else
	      sprintf(m_name, "The %s", r_ptr->name);
	  }
	  sprintf(out_val,"%s has %d hit points.", m_name, m_ptr->hp);
	  move_cursor_relative(m_ptr->fy, m_ptr->fx);
	  msg_print(out_val);
	  probe=TRUE;
	}
    }
  if (probe)
    msg_print("That's all.");
  else
    msg_print("You find nothing to probe.");
  move_cursor_relative(char_row,char_col);
  return (probe);
}

/* Attempts to destroy a type of creature.  Success depends on	*/
/* the creatures level VS. the player's level		 -RAK-	 */
int dispel_creature(cflag, damage)
#ifdef MSDOS
int32u cflag; /* ring o power passes 0xffffffffL */
#else
int cflag;
#endif
int damage;
{
  register int i;
  int k, dispel, visible;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  dispel = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if ((cflag & c_list[m_ptr->mptr].cdefense) &&
	  los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  c_recall[m_ptr->mptr].r_cdefense |= cflag;
	  monster_name (m_name, m_ptr, r_ptr);
	  visible = m_ptr->ml;  /* set this before call mon_take_hit */
	  k = mon_take_hit (i, randint(damage));
	  if (visible)
	    {
	      if (k >= 0)
		(void) sprintf(out_val, "%s dissolves!", m_name);
	      else
		(void) sprintf(out_val, "%s shudders.", m_name);
	      msg_print(out_val);
	      dispel = TRUE;
	    }
	  if (k >= 0)
	    prt_experience();
	}
    }
  return(dispel);
}


/* Attempt to turn (confuse) undead creatures.	-RAK-	*/
int turn_undead()
{
  register int i, turn_und;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  turn_und = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      if ((UNDEAD & r_ptr->cdefense)
	  && (los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx)))
	{
	  monster_name (m_name, m_ptr, r_ptr);
	  if (((py.misc.lev+1) > r_ptr->level) ||
	      (randint(5) == 1))
	    {
	      if (m_ptr->ml)
		{
		  (void) sprintf(out_val, "%s runs frantically!", m_name);
		  msg_print(out_val);
		  turn_und = TRUE;
		  c_recall[m_ptr->mptr].r_cdefense |= UNDEAD;
		}
	      m_ptr->confused = TRUE;
	    }
	  else if (m_ptr->ml)
	    {
	      (void) sprintf(out_val, "%s is unaffected.", m_name);
	      msg_print(out_val);
	    }
	}
    }
  return(turn_und);
}


/* Leave a glyph of warding. Creatures will not pass over! -RAK-*/
void warding_glyph()
{
  register int i;
  register cave_type *c_ptr;

  c_ptr = &cave[char_row][char_col];
  if (c_ptr->tptr == 0)
    {
      i = popt();
      c_ptr->tptr = i;
      invcopy(&t_list[i], OBJ_SCARE_MON);
    }
}


/* Lose a strength point.				-RAK-	*/
void lose_str()
{
  if (!py.flags.sustain_str)
    {
      (void) dec_stat (A_STR);
      msg_print("You feel very sick.");
    }
  else
    msg_print("You feel sick for a moment,  it passes.");
}


/* Lose an intelligence point.				-RAK-	*/
void lose_int()
{
  if (!py.flags.sustain_int)
    {
      (void) dec_stat(A_INT);
      msg_print("You become very dizzy.");
    }
  else
    msg_print("You become dizzy for a moment,  it passes.");
}


/* Lose a wisdom point.					-RAK-	*/
void lose_wis()
{
  if (!py.flags.sustain_wis)
    {
      (void) dec_stat(A_WIS);
      msg_print("You feel very naive.");
    }
  else
    msg_print("You feel naive for a moment,  it passes.");
}


/* Lose a dexterity point.				-RAK-	*/
void lose_dex()
{
  if (!py.flags.sustain_dex)
    {
      (void) dec_stat(A_DEX);
      msg_print("You feel very sore.");
    }
  else
    msg_print("You feel sore for a moment,  it passes.");
}


/* Lose a constitution point.				-RAK-	*/
void lose_con()
{
  if (!py.flags.sustain_con)
    {
      (void) dec_stat(A_CON);
      msg_print("You feel very sick.");
    }
  else
    msg_print("You feel sick for a moment,  it passes.");
}


/* Lose a charisma point.				-RAK-	*/
void lose_chr()
{
  if (!py.flags.sustain_chr)
    {
      (void) dec_stat(A_CHR);
      msg_print("Your skin starts to itch.");
    }
  else
    msg_print("Your skin starts to itch, but feels better now.");
}


/* Lose experience					-RAK-	*/
void lose_exp(amount)
int32 amount;
{
  register int i;
  register struct misc *m_ptr;
  register class_type *c_ptr;

  m_ptr = &py.misc;
  if (amount > m_ptr->exp)
    m_ptr->exp = 0;
  else
    m_ptr->exp -= amount;
  prt_experience();

  i = 0;
  while (((player_exp[i]*m_ptr->expfact/100)<=m_ptr->exp)
    && (i<MAX_PLAYER_LEVEL))
    i++;
  /* increment i once more, because level 1 exp is stored in player_exp[0] */
  i++;
  if (i>MAX_PLAYER_LEVEL) i=MAX_PLAYER_LEVEL;
  if (m_ptr->lev != i)
    {
      m_ptr->lev = i;

      calc_hitpoints();
      c_ptr = &class[m_ptr->pclass];
      if (c_ptr->spell == MAGE)
	{
	  calc_spells(A_INT);
	  calc_mana(A_INT);
	}
      else if (c_ptr->spell == PRIEST)
	{
	  calc_spells(A_WIS);
	  calc_mana(A_WIS);
	}
      prt_level();
      prt_title();
    }
}


/* Slow Poison						-RAK-	*/
int slow_poison()
{
  register int slow;
  register struct flags *f_ptr;

  slow = FALSE;
  f_ptr = &py.flags;
  if (f_ptr->poisoned > 0)
    {
      f_ptr->poisoned = f_ptr->poisoned / 2;
      if (f_ptr->poisoned < 1)	f_ptr->poisoned = 1;
      slow = TRUE;
      msg_print("The effect of the poison has been reduced.");
    }
  return(slow);
}


/* Bless						-RAK-	*/
void bless(amount)
int amount;
{
  py.flags.blessed += amount;
}


/* Detect Invisible for period of time			-RAK-	*/
void detect_inv2(amount)
int amount;
{
  py.flags.detect_inv += amount;
}


static void replace_spot(y, x, typ)
int y, x, typ;
{
  register cave_type *c_ptr;

  c_ptr = &cave[y][x];
  switch(typ)
    {
    case 1: case 2: case 3:
      c_ptr->fval  = CORR_FLOOR;
      break;
    case 4: case 7: case 10:
      c_ptr->fval  = GRANITE_WALL;
      break;
    case 5: case 8: case 11:
      c_ptr->fval  = MAGMA_WALL;
      break;
    case 6: case 9: case 12:
      c_ptr->fval  = QUARTZ_WALL;
      break;
    }
  c_ptr->pl = FALSE;
  c_ptr->fm = FALSE;
  c_ptr->lr = FALSE;  /* this is no longer part of a room */
  if (c_ptr->tptr != 0)
    (void) delete_object(y, x);
  if (c_ptr->cptr > 1)
    delete_monster((int)c_ptr->cptr);
}


/* The spell of destruction.				-RAK-	*/
/* NOTE : Winning creatures that are deleted will be considered	 */
/*	  as teleporting to another level.  This will NOT win the*/
/*	  game.						       */
void destroy_area(y, x)
register int y, x;
{
  register int i, j, k;

  if (dun_level > 0)
    {
      for (i = (y-15); i <= (y+15); i++)
	for (j = (x-15); j <= (x+15); j++)
	  if (in_bounds(i, j) && (cave[i][j].fval != BOUNDARY_WALL) &&
	      ((i != y) || (j != x)))
	    {
	      k = distance(i, j, y, x);
	      if (k < 13)
		replace_spot(i, j, randint(6));
	      else if (k < 16)
		replace_spot(i, j, randint(9));
	    }
    }
  msg_print("There is a searing blast of light!");
  if (!py.flags.blindness_resist)
    py.flags.blind += 10 + randint(10);
}


/* Enchants a plus onto an item.			-RAK-	*/
int enchant(plusses)
int16 *plusses;
{
  register int chance, res;

  chance = 0;
  res = FALSE;
  if (*plusses > 0)
    switch(*plusses)
      {
      case 1:  chance = 010; break;
      case 2:  chance = 050; break;
      case 3:  chance = 100; break;
      case 4:  chance = 200; break;
      case 5:  chance = 300; break;
      case 6:  chance = 400; break;
      case 7:  chance = 500; break;
      case 8:  chance = 700; break;
      case 9:  chance = 950; break;
      case 10: chance = 990; break;
      case 11: chance = 992; break;
      case 12: chance = 995; break;
      case 13: chance = 997; break;
      default: chance = 1000; break;
      }
  if (randint(1000) > chance)
    {
      *plusses += 1;
      res = TRUE;
    }
  return(res);
}

char *pain_message(monptr,dam)
{
  register monster_type *m_ptr;
  creature_type *c_ptr;
#ifdef MSDOS /* fix for 16bit ints... -CFT */
  int32 percentage,oldhp,newhp;
#else
  int percentage,oldhp,newhp;
#endif

  if (dam == 0) return "%s is unharmed."; /* avoid potential div by 0 */
  
  m_ptr=&m_list[monptr];
  c_ptr=&c_list[m_ptr->mptr];
#ifdef MSDOS /* more fix -CFT */
  newhp=(int32)(m_ptr->hp);
  oldhp=newhp+(int32)dam;
#else
  newhp=m_ptr->hp;
  oldhp=newhp+dam;
#endif
  percentage=(newhp*100)/oldhp;

  if ((c_ptr->cchar=='j') || /* Non-verbal creatures like molds*/
      (c_ptr->cchar=='Q') || (c_ptr->cchar=='v') ||
      (c_ptr->cchar=='m') || ((c_ptr->cchar=='e') && strcmp(c_ptr->name,
							    "Beholder"))) {
    if (percentage>95)
      return "%s barely notices.";
    if (percentage>75)
      return "%s flinches.";
    if (percentage>50)
      return "%s squelches.";
    if (percentage>35)
      return "%s draws back in pain.";
    if (percentage>20)
      return "%s writhes about.";
    if (percentage>10)
      return "%s writhes in agony.";
    return "%s jerks limply.";
  } else if (c_ptr->cchar=='C' || c_ptr->cchar=='Z') {
    if (percentage>95)
      return "%s shrugs off the attack.";
    if (percentage>75)
      return "%s snarls with pain.";
    if (percentage>50)
      return "%s yelps in pain.";
    if (percentage>35)
      return "%s howls in pain.";
    if (percentage>20)
      return "%s howls in agony.";
    if (percentage>10)
      return "%s writhes in agony.";
    return "%s yelps feebly.";
  } else if (c_ptr->cchar=='K' || c_ptr->cchar=='c' || c_ptr->cchar=='a' ||
	     c_ptr->cchar=='U' || c_ptr->cchar=='q' || c_ptr->cchar=='R' ||
	     c_ptr->cchar=='X' || c_ptr->cchar=='b' || c_ptr->cchar=='F' ||
	     c_ptr->cchar=='J' || c_ptr->cchar=='l' || c_ptr->cchar=='r' ||
	     c_ptr->cchar=='s' || c_ptr->cchar=='S' || c_ptr->cchar=='t') {
    if (percentage>95)
      return "%s ignores the attack.";
    if (percentage>75)
      return "%s grunts with pain.";
    if (percentage>50)
      return "%s squeals in pain.";
    if (percentage>35)
      return "%s shrieks in pain.";
    if (percentage>20)
      return "%s shrieks in agony.";
    if (percentage>10)
      return "%s writhes in agony.";
    return "%s cries out feebly.";
  } else {
    if (percentage>95)
      return "%s shrugs off the attack.";
    if (percentage>75)
      return "%s grunts with pain.";
    if (percentage>50)
      return "%s cries out in pain.";
    if (percentage>35)
      return "%s screams in pain.";
    if (percentage>20)
      return "%s screams in agony.";
    if (percentage>10)
      return "%s writhes in agony.";
    return "%s cries out feebly.";
  }
}

/* Removes curses from items in inventory		-RAK-	*/
int remove_curse()
{
  register int i, result;
  register inven_type *i_ptr;

  result = FALSE;
  for (i = INVEN_WIELD; i <= INVEN_OUTER; i++)
    {
      i_ptr = &inventory[i];
      if ((TR_CURSED & i_ptr->flags) &&
	  (i_ptr->name2 != SN_MORGUL) &&
	  (i_ptr->name2 != SN_CALRIS) &&
	  (i_ptr->name2 != SN_MORMEGIL))
	{
	  if (!(!strcmp(object_list[i_ptr->index].name, "Power") &&
	    (i_ptr->tval == TV_RING))) {
	      i_ptr->flags &= ~TR_CURSED;
	      calc_bonuses();
	      result = TRUE;
	    }
	}
    }
  return(result);
}

int remove_all_curse()
{
  register int i, result;
  register inven_type *i_ptr;

  result = FALSE;
  for (i = INVEN_WIELD; i <= INVEN_OUTER; i++)
    {
      i_ptr = &inventory[i];
      if (TR_CURSED & i_ptr->flags)
	{
	  if (!(!strcmp(object_list[i_ptr->index].name, "Power") &&
	   (i_ptr->tval == TV_RING))) {
	     i_ptr->flags &= ~TR_CURSED;
	     calc_bonuses();
	     result = TRUE;
	   } else {
	      msg_print("The One Ring resists all attempts to remove it!");
	    }
	}
    }
  return(result);
}


/* Restores any drained experience			-RAK-	*/
int restore_level()
{
  register int restore;
  register struct misc *m_ptr;

  restore = FALSE;
  m_ptr = &py.misc;
  if (m_ptr->max_exp > m_ptr->exp)
    {
      restore = TRUE;
      msg_print("You feel your life energies returning.");
      /* this while loop is not redundant, ptr_exp may reduce the exp level */
      while (m_ptr->exp < m_ptr->max_exp)
	{
	  m_ptr->exp = m_ptr->max_exp;
	  prt_experience();
	}
    }
  return(restore);
}


/* this fn only exists to avoid duplicating this code in the selfknowledge
   fn. -CFT */
static void pause_if_screen_full(int *i, int j){
  int t;
  if (*i == 22){ /* is screen full? */
    prt("-- more --", *i, j);
    inkey();
    for (t=2;t<23;t++) erase_line(t,j); /* don't forget to erase extra */
    prt("Your Attributes: (continued)", 1, j+5);
    *i = 2;
  }
}

/* self-knowledge... idea from nethack.  Useful for determining powers and
   resistences of items.  It saves the screen, clears it, then starts
   listing attributes, a screenful at a time.  (There are a LOT of attributes
   to list.  It will probably take 2 or 3 screens for a powerful character
   whose using several artifacts...) -CFT */
   
void self_knowledge(void){
  int i,j;
  int32u f = 0L, f2 = 0L;

  for(i=INVEN_WIELD;i <= INVEN_LIGHT; i++){ /* get flags from items */
    if (inventory[i].tval != TV_NOTHING){
      f |= inventory[i].flags; 
      f2 |= inventory[i].flags2;
    }
  }

  save_screen();

  j = 15; /* map starts at 13, but I want a couple of spaces.  This means
  	     must start by erasing map... */
  for(i=1;i<23;i++) erase_line(i, j-2); /* erase a couple of spaces to left */

  i = 1; 
  prt("Your Attributes:", i++, j+5);
  
  if (py.flags.blind > 0)
    prt("You cannot see.", i++, j);
  if (py.flags.confused > 0)
    prt("You are confused.", i++, j);
  if (py.flags.afraid > 0)
    prt("You are scared.", i++, j);
  if (py.flags.cut > 0)
    prt("You are bleeding.", i++, j);
  if (py.flags.stun > 0)
    prt("You are reeling.", i++, j);
  if (py.flags.poisoned > 0)
    prt("You are poisoned.", i++, j);
  if (py.flags.image > 0)
    prt("You are hallucinating.", i++, j);
  if (py.flags.aggravate)
    prt("You aggravate monsters.", i++, j);
  if (py.flags.teleport)
    prt("Your position is very uncertain.", i++, j);

  if (py.flags.blessed > 0)
    prt("You feel rightous.", i++, j);
  if (py.flags.hero > 0)
    prt("You feel heroic.", i++, j);
  if (py.flags.shero > 0)
    prt("You are in a battle rage.", i++, j);
  if (py.flags.protevil > 0)
    prt("You are protected from evil.", i++, j);
  if (py.flags.shield > 0)
    prt("You are protected by a mystic shield.", i++, j);
  if (py.flags.invuln > 0)
    prt("You are temporarily invulnerable.", i++, j);
  if (py.flags.confuse_monster)
    prt("Your hands are glowing dull red.", i++, j);
  if (py.flags.new_spells > 0)
    prt("You can learn some more spells.", i++, j);
  if (py.flags.word_recall > 0)
    prt("You will soon be recalled.", i++, j);
    
  if (f & TR_STEALTH)
    prt("You are hard to find.", i++, j);  
  if (f & TR_SEARCH){
    prt("You are perceptive.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if ((py.flags.see_infra) || (py.flags.tim_infra)){
    prt("Your eyes are sensitive to infrared.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if ((py.flags.see_inv) || (py.flags.detect_inv)){
    prt("You can see invisible creatures.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.ffall){
    prt("You land gently.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.free_act){
    prt("You have free action.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.regenerate){
    prt("You regenerate quickly.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.slow_digest){
    prt("Your appetite is small.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.telepathy){
    prt("You have ESP.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.hold_life){
    prt("You have a firm hold on your life force.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.light){
    prt("You are carrying a permanent light.", i++, j);
    pause_if_screen_full(&i, j);
  }

  if (py.flags.blindness_resist){
    prt("Your eyes are resistant to blindness.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.fire_im){
    prt("You are completely immune to fire.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.fire_resist) && (py.flags.resist_heat)){
    prt("You resist fire exceptionally well.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.fire_resist) || (py.flags.resist_heat)){
    prt("You are resistant to fire.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.cold_im){
    prt("You are completely immune to cold.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.cold_resist) && (py.flags.resist_cold)){
    prt("You resist cold exceptionally well.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.cold_resist) || (py.flags.resist_cold)){
    prt("You are resistant to cold.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.acid_im){
    prt("You are completely immune to acid.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.acid_resist) && (py.flags.resist_acid)){
    prt("You resist acid exceptionally well.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.acid_resist) || (py.flags.resist_acid)){
    prt("You are resistant to acid.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.poison_im){
    prt("You are completely immune to poison.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.poison_resist) && (py.flags.resist_poison)){
    prt("You resist poison exceptionally well.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.poison_resist) || (py.flags.resist_poison)){
    prt("You are resistant to poison.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.light_im){
    prt("You are completely immune to lightning.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.lght_resist) && (py.flags.resist_light)){
    prt("You resist lightning exceptionally well.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if ((py.flags.lght_resist) || (py.flags.resist_light)){
    prt("You are resistant to lightning.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.light_resist){
    prt("You are resistant to bright light.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.dark_resist){
    prt("You are resistant to darkness.", i++, j);   
    pause_if_screen_full(&i, j);
  }
  if (py.flags.confusion_resist){
    prt("You are resistant to confusion.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sound_resist){
    prt("You are resistant to sonic attacks.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.disenchant_resist){
    prt("You are resistant to disenchantment.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.chaos_resist){
    prt("You are resistant to blasts of chaos.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.shards_resist){
    prt("You are resistant to blasts of shards.", i++, j);    
    pause_if_screen_full(&i, j);
  }
  if (py.flags.nexus_resist){
    prt("You are resistant to nexus attacks.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.nether_resist){
    prt("You are resistant to nether forces.", i++, j);    
    pause_if_screen_full(&i, j);
  }

/* Are these needed?  The player can see this...  For now, in here for
   completeness... -CFT */
  if (f & TR_STR){
    prt("You have magical strength.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_INT){
    prt("You have magically enhanced intelligence.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_WIS){
    prt("You are magically wise.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_DEX){
    prt("You are magically agile.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_CON){
    prt("You are magically tough.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_CHR){
    prt("You are magically popular.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_str){
    prt("You will not become weaker.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_int){
    prt("You will not become dumber.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_wis){
    prt("You will not become less wise.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_con){
    prt("You will not become out of shape.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_dex){
    prt("You will not become clumsy.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (py.flags.sustain_chr){
    prt("You will not become less popular.", i++, j);
    pause_if_screen_full(&i, j);
  }
    
  if (inventory[INVEN_WIELD].tval != TV_NOTHING){ /* this IS a bit redundant,
  						but it prevents flags from
  						other items from affecting
  						the weapon stats... -CFT */
    f = inventory[INVEN_WIELD].flags;
    f2 = inventory[INVEN_WIELD].flags2;
  } else {
    f = 0L;
    f2 = 0L;
  }
  if (f & TR_CURSED){
    if (inventory[INVEN_WIELD].name2 == SN_MORGUL)
      prt("Your weapon is truly foul.", i++, j);
    else if (inventory[INVEN_WIELD].name2 == SN_CALRIS)
      prt("Your bastard sword is wickedly accursed.", i++, j);
    else if (inventory[INVEN_WIELD].name2 == SN_MORMEGIL)  
      prt("Your two-handed sword radiates an aura of unspeakable evil.", i++, j);
    else prt("Your weapon is accursed.", i++, j);
    pause_if_screen_full(&i, j);
    }
  if (f & TR_TUNNEL){
    prt("Your weapon is an effective digging tool.", i++, j);
    pause_if_screen_full(&i, j);
    }
  if (f2 & TR_SLAY_ORC){
    prt("Your weapon is especially deadly against orcs.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f2 & TR_SLAY_TROLL){
    prt("Your weapon is especially deadly against trolls.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f2 & TR_SLAY_GIANT){
    prt("Your weapon is especially deadly against giants.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_SLAY_ANIMAL){
    prt("Your weapon is especially deadly against Nature's creatures.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (f & TR_SLAY_X_DRAGON){
    prt("Your weapon is a great bane of dragons.", i++, j);
    pause_if_screen_full(&i, j);
  }
  else if (f & TR_SLAY_DRAGON){
    prt("Your weapon is especially deadly against dragons.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (f2 & TR_SLAY_DEMON){
    prt("Your weapon strikes at demons with holy wrath.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_SLAY_UNDEAD){
    prt("Your weapon strikes at undead with holy wrath.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_SLAY_EVIL){
    prt("Your weapon fights against evil with holy fury.", i++, j);  
    pause_if_screen_full(&i, j);
  }
  if (f & TR_FROST_BRAND){  
    prt("Your frigid weapon freezes your foes.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (f & TR_FLAME_TONGUE){
    prt("Your flaming weapon burns your foes.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (f2 & TR_LIGHTNING){
    prt("Your weapon electrocutes your foes.", i++, j);
    pause_if_screen_full(&i, j);
  }
  if (f2 & TR_IMPACT)
    prt("The unbelievable impact of your weapon can cause earthquakes.", i++, j);

  pause_line(i);
  restore_screen();
}
