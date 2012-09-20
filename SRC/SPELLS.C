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
static void spell_hit_monster(monster_type *, int, int *, int, int *, int *, int8u);
static void ball_destroy(int, int (**)() );
#else
static void replace_spot(ARG_INT ARG_COMMA ARG_INT ARG_COMMA ARG_INT);
static void spell_hit_monster(ARG_MON_PTR ARG_COMMA ARG_INT ARG_COMMA
		ARG_INT_PTR ARG_COMMA ARG_INT ARG_COMMA ARG_INT_PTR
		ARG_COMMA ARG_INT_PTR ARG_COMMA ARG_INT8U);
static void ball_destroy(ARG_INT ARG_COMMA ARG_INT_FN_PTR);
#endif

char *pain_message();

#ifdef MSDOS /* then need prototype... -CFT */
#ifdef TC_COLOR
static int8u bolt_color(int); /* ditto -CFT */
static char bolt_char(int,int,int,int);
#endif
#endif

/* Following are spell procedure/functions			-RAK-	*/
/* These routines are commonly used in the scroll, potion, wands, and	 */
/* staves routines, and are occasionally called from other areas.	  */
/* Now included are creature spells also.		       -RAK    */

#ifdef TC_COLOR
static char bolt_char(int y,int x,int ny,int nx){ /* this assumes only 1
							move apart -CFT */
  if (ny == y) return '-';
  if (nx == x) return '|';
  if ((ny-y) == (nx-x)) return '\\';
  return '/';
}

static int8u bolt_color(int typ){
  int8u choice=randint(6);

  switch(typ){      /* DGK */
    case GF_MAGIC_MISSILE: return LIGHTCYAN;
    case GF_MANA:          return (choice<=3?LIGHTCYAN:CYAN);
    case GF_LIGHTNING:     return (choice<=4?YELLOW:WHITE);
    case GF_LIGHT:         return (choice<=4?WHITE:YELLOW);
    case GF_POISON_GAS:    return (choice<=3?GREEN:LIGHTGREEN);
    case GF_CONFUSION: switch(choice){case 1:return RED;
      case 2:return BLUE; case 3:return GREEN; case 4:return YELLOW;
      case 5:return MAGENTA; case 6:return CYAN;}
    case GF_ACID:          return (choice<=3?YELLOW:LIGHTGREEN);
    case GF_FROST:         return (choice<=3?LIGHTBLUE:BLUE);
    case GF_WATER:         return (choice<=3?CYAN:BLUE);
    case GF_ICE:           return (choice<=3?LIGHTBLUE:WHITE);
    case GF_FIRE:          return (choice<=3?RED:(choice<=5?YELLOW:WHITE));
    case GF_HOLY_ORB:      return (choice<=5?DARKGRAY:RED);
    case GF_NETHER:        return DARKGRAY;
    case GF_CHAOS:         return (choice<=3?DARKGRAY:GREEN);
    case GF_TIME:    return (choice<=2?DARKGRAY:(choice<=4?LIGHTGRAY:WHITE));
    case GF_DARK:          return (choice<=3?DARKGRAY:0);
    case GF_ARROW:         return BROWN;
    case GF_PLASMA:        return LIGHTRED;
    case GF_METEOR:        return (choice<=3?LIGHTRED:WHITE);
    case GF_SHARDS:        return (choice<=3?WHITE:CYAN);
    case GF_SOUND:         return WHITE;
    case GF_DISENCHANT:    return (choice<=3?CYAN:LIGHTCYAN);
    case GF_NEXUS:         return (choice<=3?MAGENTA:LIGHTMAGENTA);
    case GF_GRAVITY:       return (choice<=3?MAGENTA:DARKGRAY);
    case GF_FORCE:         return (choice<=3?LIGHTGRAY:WHITE);
    case GF_INERTIA:       return (choice<=3?LIGHTGRAY:DARKGRAY);
    }
  return randint(15); /* should never happen... -CFT */
}
#endif

/* return the appropriate item destroy test to the typ.  All that's left of
   get_flags().  -CFT */
/* add new destroys?  maybe GF_FORCE destroy potions, GF_PLASMA as lightning,
   GF_SAHRDS and GF_ICE maybe break things (potions?), and GF_METEOR breaks
   potions and burns scrolls?  not yet, but it's an idea... -CFT */
static void ball_destroy(int typ, int (**destroy)() ){
  switch (typ) {
    case GF_FIRE:
      *destroy = set_fire_destroy;
      break;
    case GF_ACID:
      *destroy = set_acid_destroy;
      break;
    case GF_FROST:
      *destroy = set_frost_destroy;
      break;
    case GF_LIGHTNING:
      *destroy = set_lightning_destroy;
      break;
    case GF_MAGIC_MISSILE: case GF_POISON_GAS: case GF_HOLY_ORB:
    case GF_ARROW: case GF_PLASMA: case GF_NETHER: case GF_WATER:
    case GF_CHAOS: case GF_SHARDS: case GF_SOUND: case GF_CONFUSION:
    case GF_DISENCHANT: case GF_NEXUS: case GF_FORCE: case GF_INERTIA:
    case GF_LIGHT: case GF_DARK: case GF_TIME: case GF_GRAVITY:
    case GF_MANA: case GF_METEOR: case GF_ICE:
      *destroy = set_null;
      break;
    default:
      msg_print("Unknown typ in ball_destroy().  This may mean trouble.");
      *destroy = set_null;
      break;
    }
}

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

int detect_enchantment()
{
  register int i, j, detect, tv;
  register cave_type *c_ptr;

  detect = FALSE;
  for (i = panel_row_min; i <= panel_row_max; i++)
    for (j = panel_col_min; j <= panel_col_max; j++)
      {
	c_ptr = &cave[i][j];
        tv = t_list[c_ptr->tptr].tval;
	if ((c_ptr->tptr != 0) && !test_light(i, j) &&
	    ( ((tv > TV_MAX_ENCHANT) && (tv < TV_FLASK)) || /* misc items */
	      (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) || /* books */
	      ((tv >= TV_MIN_WEAR) && (tv <= TV_MAX_ENCHANT) && /* armor/weap */
	       ((t_list[c_ptr->tptr].flags2 & TR_ARTIFACT) || /* if Art., or */
	        (t_list[c_ptr->tptr].tohit>0) || /* has pluses, then show */
	        (t_list[c_ptr->tptr].todam>0) ||
	        (t_list[c_ptr->tptr].toac>0))) )){
	  c_ptr->fm = TRUE;
	  lite_spot(i, j);
	  detect = TRUE;
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
	  if (!no_color_flag) {
	    textcolor(mon_color(m_ptr->color));
	    } /* if !no_color_flag */
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
	((t_list[c_ptr->tptr].tval != TV_UP_STAIR)  /* if not stairs or a store */
	 && (t_list[c_ptr->tptr].tval != TV_DOWN_STAIR)
	 && (t_list[c_ptr->tptr].tval != TV_STORE_DOOR)
         && ((t_list[c_ptr->tptr].tval < TV_MIN_WEAR) ||
             (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
             !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)))) { /* if no artifact here -CFT */
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
	((t_list[c_ptr->tptr].tval != TV_UP_STAIR)  /* if not stairs or a store */
	 && (t_list[c_ptr->tptr].tval != TV_DOWN_STAIR)
	 && (t_list[c_ptr->tptr].tval != TV_STORE_DOOR)
         && ((t_list[c_ptr->tptr].tval < TV_MIN_WEAR) ||
             (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
             !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)))) { /* if no artifact here -CFT */
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
	  if (!no_color_flag) {
	    textcolor(mon_color(m_ptr->color));
	    } /* if !no_color_flag */
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
  int unlight;
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
	if ((i == char_row) && (j == char_col))
	  continue; /* no trap under player, from um55 -CFT */
	c_ptr = &cave[i][j];
	if (c_ptr->fval <= MAX_CAVE_FLOOR)
	  {
    if ((c_ptr->tptr == 0) ||
	((t_list[c_ptr->tptr].tval != TV_UP_STAIR)  /* if not stairs or a store */
	 && (t_list[c_ptr->tptr].tval != TV_DOWN_STAIR)
	 && (t_list[c_ptr->tptr].tval != TV_STORE_DOOR)
         && ((t_list[c_ptr->tptr].tval < TV_MIN_WEAR) ||
             (t_list[c_ptr->tptr].tval > TV_MAX_WEAR) ||
             !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)))) { /* if no artifact here -CFT */
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
	  if (!no_color_flag) {
	    textcolor(mon_color(m_ptr->color));
	    } /* if !no_color_flag */
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


/* Shoot a bolt in a given direction			-RAK-	*/
/* unused arg char *bolt_typ removed -CFT */
void fire_bolt(typ, dir, y, x, dam_hp)
int typ, dir, y, x, dam_hp;
{
  int i, oldy, oldx, dist, flag;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;
  int dam = dam_hp;
  int ny,nx;
  char b_c;
  
  flag = FALSE;
  oldy = y;
  oldx = x;
  dist = 0;
  do
    {
      ny = y; nx = x;
      (void) mmove(dir, &y, &x);
      b_c = bolt_char(ny, nx, y, x);

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

	      /* light up monster and draw monster, temporarily set
		 pl so that update_mon() will work */
	      i = c_ptr->pl;
	      c_ptr->pl = TRUE;
	      update_mon ((int)c_ptr->cptr);
	      c_ptr->pl = i;
	      /* draw monster and clear previous bolt */
	      put_qio();

	      spell_hit_monster(m_ptr, typ, &dam, 0, &ny, &nx, 1);
	      c_ptr = &cave[ny][nx]; /* may be new location if teleported by
	      			gravity warp... */
	      m_ptr = &m_list[c_ptr->cptr]; /* and even if not, may be new
	      			monster if choas polymorphed */
	      r_ptr = &c_list[m_ptr->mptr];
	      monster_name (m_name, m_ptr, r_ptr);
	      i = mon_take_hit((int)c_ptr->cptr, dam);

	      if (i >= 0) {
		  prt_experience();
		}
	      else if (dam > 0) {
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
	      print(b_c, y, x);
	      /* show the bolt */
	      put_qio();
#ifdef MSDOS
	      delay(35); /* slow it down, so we can actually see it! -CFT */
#else
	      usleep(23000); /* useconds */
#endif
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
	    }
	}
      oldy = y;
      oldx = x;
      if (target_mode && at_target(y,x))
        flag = TRUE; /* must have hit "targeted" area -CFT */
    }
  while (!flag);
  lite_spot(oldy, oldx); /* just in case, clear any leftover bolt images -CFT */
}


/* Shoot a bolt in a given direction			-RAK-	*/
/* heavily modified to include exotic bolts -CFT */
void bolt(typ, y, x, dam_hp, ddesc, ptr, monptr)
  int typ, y, x, dam_hp;
  char *ddesc;
  monster_type *ptr;
  int monptr;
{
  int i=ptr->fy, j=ptr->fx;
  int dam;
  int32u tmp, treas;
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  char b_c;
  int blind = (py.flags.status & PY_BLIND) ? 1 : 0;
  int ny, nx, sourcey, sourcex, dist;
  vtype m_name, out_val;
  
  sourcey = i;
  sourcex = j;
  dist = 0;
  do
    {
    /* This is going along a badly angled line so call mmove2 direct */
      ny = i; nx = j;
      mmove2(&i, &j, sourcey, sourcex, char_row, char_col);
      dist++;

      b_c = bolt_char(ny, nx, i, j);

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
	  print(b_c, i, j);
	  put_qio();
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif	  
#ifdef MSDOS
	  delay(35); /* milli-secs -CFT */
#endif
	  lite_spot(i, j);
	}
	if (c_ptr->cptr > 1 && c_ptr->cptr != monptr) {
	  m_ptr = &m_list[c_ptr->cptr];
	  dam = dam_hp;

	  spell_hit_monster(m_ptr, typ, &dam, 0, &ny, &nx, 0); /* process hit effects */
	  c_ptr = &cave[ny][nx]; /* may be new location if teleported by
	      			gravity warp... */
	  m_ptr = &m_list[c_ptr->cptr]; /* and even if not, may be new
	      			monster if choas polymorphed */
	  r_ptr = &c_list[m_ptr->mptr];
	  monster_name (m_name, m_ptr, r_ptr);

	  if (dam < 1) dam = 1; /* protect vs neg damage -CFT */
	  m_ptr->hp = m_ptr->hp - dam;
	  m_ptr->csleep = 0;
	  if ((r_ptr->cdefense & UNIQUE) && (m_ptr->hp < 0))
	    m_ptr->hp = 0; /* prevent unique monster from death by other
	    		     monsters.  It causes trouble (monster not
	    		     marked as dead, quest monsters don't satisfy
	    		     quest, etc).  So, we let then live, but
	    		     extremely wimpy. -CFT */
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
	  else {
	    (void)sprintf(out_val, pain_message((int)c_ptr->cptr, dam), m_name);
	    msg_print(out_val);
	    }
	  break;
	} else if (c_ptr->cptr == 1) {
	  if (dam_hp < 1)
	    dam_hp = 1;
	  m_ptr = &m_list[monptr];
	  switch(typ) {
	  case GF_LIGHTNING:
	    if (blind) 
	      msg_print("You are hit by electricity!");
	    light_dam(dam_hp, ddesc);
	    break;
	  case GF_POISON_GAS:
	    if (blind) 
	      msg_print("You are hit by a blast of noxious gases!");
	    poison_gas(dam_hp, ddesc);
	    break;
	  case GF_ACID:
	    if (blind) 
	      msg_print("You are hit by a jet of acidic fluid!");
	    acid_dam(dam_hp, ddesc);
	    break;
	  case GF_FROST:
	    if (blind) 
	      msg_print("You are hit by something cold!");
	    cold_dam(dam_hp, ddesc);
	    break;
	  case GF_FIRE:
	    if (blind) 
	      msg_print("You are hit by something hot!");
	    fire_dam(dam_hp, ddesc);
	    break;
	  case GF_MAGIC_MISSILE:
	    if (blind) 
	      msg_print("You are hit by something!");
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_HOLY_ORB:
	    if (blind) 
	      msg_print("You are hit by something!");
	    dam_hp /= 2; /* player should take less damage -CFT */
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_ARROW: /* maybe can miss? */
	    if (blind) 
	      msg_print("You are hit by something!");
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_PLASMA:  /* no resist to plasma? */
	    if (blind) 
	      msg_print("You are hit by something!");
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_NETHER:
	    if (blind) 
	      msg_print("You are hit by an unholy blast!");
	    if (py.flags.nether_resist) {
	      dam_hp *= 6; /* these 2 lines give avg dam of .655, ranging from */
	      dam_hp /= (randint(6)+6); /* .858 to .5 -CFT */
	      }
	    else { /* no resist */
	      if (py.flags.hold_life && randint(5)>1)
	        msg_print("You keep hold of your life force!");
	      else if (py.flags.hold_life) {
		msg_print("You feel your life slipping away!");
		lose_exp(200+(py.misc.exp / 1000)*MON_DRAIN_LIFE);
	        }
	      else {
		msg_print("You feel your life draining away!");
		lose_exp(200+(py.misc.exp / 100)*MON_DRAIN_LIFE);
		}
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_WATER:
	    if (blind) 
	      msg_print("You are hit by a jet of water!");
            if (!py.flags.sound_resist)
              stun_player(randint(15));
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_CHAOS:
	    if (blind) 
	      msg_print("You are hit by wave of anarchy!");
	    if (py.flags.chaos_resist) {
	      dam_hp *= 6; /* these 2 lines give avg dam of .655, ranging from */
	      dam_hp /= (randint(6)+6); /* .858 to .5 -CFT */
	      }
	    if ((!py.flags.confusion_resist) && (!py.flags.chaos_resist)) {
	      if (py.flags.confused > 0)
		py.flags.confused += 12;
	      else
		py.flags.confused = randint(20) + 10;
              }
	    if (!py.flags.chaos_resist)
	      py.flags.image += randint(10);
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_SHARDS:
	    if (blind) 
	      msg_print("You are cut by sharp fragments!");
	    if (py.flags.shards_resist) {
	      dam_hp *= 6; /* these 2 lines give avg dam of .655, ranging from */
	      dam_hp /= (randint(6)+6); /* .858 to .5 -CFT */
	      }
	    else {
	      cut_player(dam_hp); /* ouch! */
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_SOUND:
	    if (blind) 
	      msg_print("You are deafened by a blast of noise!");
	    if (py.flags.sound_resist) {
	      dam_hp *= 5;
	      dam_hp /= (randint(6)+6);
	      }
	    else {
	      stun_player( randint( (dam_hp > 60) ? 25 : (dam_hp/3+5) ) );
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_CONFUSION:
	    if (blind) 
	      msg_print("You are hit by a wave of dizziness!");
	    if (py.flags.confusion_resist) {
	      dam_hp *= 5;
	      dam_hp /= (randint(6)+6);
	      }
	    if (!py.flags.confusion_resist && !py.flags.chaos_resist) {
	      if (py.flags.confused > 0)
	        py.flags.confused += 8;
	      else
	        py.flags.confused = randint(15) + 5;
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_DISENCHANT:
	    if (blind) 
	      msg_print("You are hit by something!");
	    if (py.flags.disenchant_resist) {
	      dam_hp *= 6; /* these 2 lines give avg dam of .655, ranging from */
	      dam_hp /= (randint(6)+6); /* .858 to .5 -CFT */
	      }
	    else {
	      int8u disenchant = FALSE, chance = 1;
	      int t;
	      inven_type *i_ptr;

	      switch(randint(7)) {
		case 1: t = INVEN_BODY; break;
		case 2: t = INVEN_BODY;  break;
		case 3: t = INVEN_ARM;   break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_HANDS; break;
		case 6: t = INVEN_HEAD;  break;
		case 7: t = INVEN_FEET;  break;
		}
	      i_ptr = &inventory[t];
	      if (i_ptr->tval != TV_NOTHING){
	        if (i_ptr->flags2 & TR_ARTIFACT)
	          chance = randint(5);
	        if ((i_ptr->tohit > 0) && (chance < 3)){
	          i_ptr->tohit -= randint(2);
	          /* don't send it below zero */
	          if (i_ptr->tohit < 0)
	            i_ptr->tohit = 0;
	          disenchant = TRUE;
	        }
	        if ((i_ptr->todam > 0) && (chance < 3)) {
	          i_ptr->todam -= randint(2);
	          /* don't send it below zero */
	          if (i_ptr->todam < 0)
	            i_ptr->todam = 0;
	          disenchant = TRUE;
	        }
	        if ((i_ptr->toac > 0) && (chance < 3)) {
	          i_ptr->toac  -= randint(2);
	          /* don't send it below zero */
	          if (i_ptr->toac < 0)
	            i_ptr->toac = 0;
	          disenchant = TRUE;
	        }
	      if (disenchant || (chance > 2)) {
		    vtype t1, t2;
		    objdes(t1, &inventory[t], FALSE);
		    if (chance < 3)
		      sprintf(t2, "Your %s (%c) %s disenchanted!", t1,
			t+'a'-INVEN_WIELD,
			(inventory[t].number != 1) ? "were":"was");
		    else
		      sprintf(t2, "Your %s (%c) %s disenchantment!", t1,
			t+'a'-INVEN_WIELD,
			(inventory[t].number != 1) ? "resist":"resists");
		    msg_print (t2);
	        calc_bonuses ();
	        }
	      }
	    }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_NEXUS: /* no spec. effects from nexus bolt, only breath -CFT */
	    if (blind) 
	      msg_print("You are hit by something strange!");
	    if (py.flags.nexus_resist) {
	      dam_hp *= 6; /* these 2 lines give avg dam of .655, ranging from */
	      dam_hp /= (randint(6)+6); /* .858 to .5 -CFT */
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_FORCE:
	    if (blind) 
	      msg_print("You are hit hard by a sudden force!");
	    if (!py.flags.sound_resist)
	      stun_player(randint(15)+1);
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_INERTIA:
	    if (blind) 
	      msg_print("You are hit by something!");
	    if ((py.flags.slow > 0) && (py.flags.slow < 32000))
	      py.flags.slow += randint(5);
	    else {
	      msg_print("You feel less able to move.");
	      py.flags.slow = randint(5)+3;
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_LIGHT:
	    if (blind) 
	      msg_print("You are hit by something!");
	    if (py.flags.light_resist) {
	      dam_hp *= 4; /* these 2 lines give avg dam of .444, ranging from */
	      dam_hp /= (randint(6)+6); /* .556 to .333 -CFT */
	      }
	    else if (!blind && !py.flags.blindness_resist) {
	      msg_print("You are blinded by the flash!");
	      py.flags.blind = randint(5)+2;
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_DARK:
	    if (blind) 
	      msg_print("You are hit by something!");
	    if (py.flags.dark_resist) {
	      dam_hp *= 4; /* these 2 lines give avg dam of .444, ranging from */
	      dam_hp /= (randint(6)+6); /* .556 to .333 -CFT */
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_TIME: /* only some effects from time bolt -CFT */
	    if (blind) 
	      msg_print("You are hit by something!");
	    if (randint(2) == 1) {
	      msg_print("You feel life has clocked back.");
	      lose_exp(m_ptr->hp+(py.misc.exp / 300)*MON_DRAIN_LIFE);
	      }
	    else {
	      int t;
	      switch(randint(6)) {
	      	case 1:
	      	  t = A_STR;
	      	  msg_print("You're not as strong as you used to be...");
	      	  break;
	      	case 2:
	      	  t = A_INT;
	      	  msg_print("You're not as bright as you used to be...");
	      	  break;
	      	case 3:
	      	  t = A_WIS;
	      	  msg_print("You're not as wise as you used to be...");
	      	  break;
	      	case 4:
	      	  t = A_DEX;
	      	  msg_print("You're not as agile as you used to be...");
	      	  break;
	      	case 5:
	      	  t = A_CON;
	      	  msg_print("You're not as hale as you used to be...");
	      	  break;
	      	case 6:
	      	  t = A_CHR;
	      	  msg_print("You're not as beautiful as you used to be...");
	      	  break;
	        }
	      py.stats.cur_stat[t]=(py.stats.cur_stat[t]*3)/4;
	      if (py.stats.cur_stat[t] < 3) py.stats.cur_stat[t]=3;
	      set_use_stat(t);
	      prt_stat(t);
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_GRAVITY:
	    if (blind) 
	      msg_print("You are hit by a surge of gravity!");
	    if (!py.flags.sound_resist)
	      stun_player(randint(15)+1);
	    if ((py.flags.slow > 0) && (py.flags.slow < 32000))
	      py.flags.slow += randint(5);
	    else {
	      msg_print("You feel less able to move.");
	      py.flags.slow = randint(5)+3;
	      }
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_MANA:
	    if (blind) 
	      msg_print("You are hit by a beam of power!");
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_METEOR:
	    if (blind) 
	      msg_print("You are hit by something!");
	    take_hit(dam_hp, ddesc);
	    break;
	  case GF_ICE:
	    if (blind) 
	      msg_print("You are hit by something cold and sharp!");
	    cold_dam(dam_hp, ddesc);
	    if (!py.flags.sound_resist)
	      stun_player(randint(15)+1);
	    if (!py.flags.shards_resist)
	      cut_player(damroll(8,10));
	    break;
	  default:
	    msg_print("Unknown typ in bolt().  This may mean trouble.");
	  }
	  disturb(1, 0);
	  break;
	}
      }
    }
  } while ((i != char_row) || (j != char_col));
}

/* Shoot a ball in a given direction.  Note that balls have an	*/
/* area affect.					      -RAK-   */
/* unused arg char *descript removed -CFT */
void fire_ball(typ, dir, y, x, dam_hp, max_dis)
int typ, dir, y, x, dam_hp, max_dis;
{
  register int i, j;
  int dam, thit, tkill, k, tmp, monptr;
  int oldy, oldx, dist, flag;
  int (*destroy)();
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int ny,nx;
  char b_c;
  
  thit	 = 0;
  tkill	 = 0;

  ball_destroy(typ, &destroy);

  flag = FALSE;
  oldy = y;
  oldx = x;
  dist = 0;
  do {
    ny = y; nx = x;
    if (dir || !at_target(y,x))  /* we don't call mmove if targetting and
    				    at target.  This allow player to target
    				    a ball spell at his position, to explode
    				    it around himself -CFT */
      (void) mmove(dir, &y, &x);

    b_c = bolt_char(ny, nx, y, x);

    dist++;
    lite_spot(oldy, oldx);
    if (dist > OBJ_BOLT_RANGE)
      flag = TRUE;
    else {
      c_ptr = &cave[y][x];
/* targeting code stolen from Morgul -CFT */

/* This test has been overhauled (twice):  basically, it now says:
   if ((spell hits a wall) OR
       ((spell hits a creature) and
        ((not targetting) or (at the target anyway) or
         (no line-of-sight to target, so aiming unusable) or
         ((aiming at a monster) and (that monster is unseen, so aiming
							unusable)))) OR
       ((we are targetting) and (at the target location))) 
       	 THEN the ball explodes...       	 	-CFT  */

      if ((c_ptr->fval >= MIN_CLOSED_SPACE) ||
	  ((c_ptr->cptr > 1) &&
	   (!target_mode || at_target(y, x) ||
	    !los(target_row, target_col, char_row, char_col) ||
	    ((target_mon < MAX_MALLOC) && !m_list[target_mon].ml))) ||
	  (target_mode && at_target(y, x))) {
	flag = TRUE;  /* THEN we decide to explode here. -CFT */
	if (c_ptr->fval >= MIN_CLOSED_SPACE) {
	  y = oldy;
	  x = oldx;
	}
	/* The ball hits and explodes.		     */
	/* The explosion.			     */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(char_row, char_col, i, j) && los(y, x, i, j) &&
		(cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && (py.flags.blind < 1)) {
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(bolt_color(typ));
#endif
	      print('*', i, j);
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(LIGHTGRAY); /* prob don't need here, but... -CFT */
#endif
	      }
	if (py.flags.blind < 1) {
	  put_qio();
#ifdef MSDOS
	  delay(125); /* millisecs, so we see the ball we just drew */
#else
	  usleep(75000); /* useconds */
#endif
	  }
	/* now erase the ball, since effects below may use msg_print, and
	   pause indefinitely, so we want ball gone before then -CFT */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(char_row, char_col, i, j) && los(y, x, i, j) &&
		(cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && (py.flags.blind < 1)) {
	      lite_spot(i,j); /* draw what is below the '*' */
	      }
	put_qio();
	/* First go over the are of effect, and destroy items...  Any
	   preexisting items will be affected, but items dropped by killed
	   monsters are assummed to have been "shielded" from the effects
	   the the monster's corpse.  This means that you no longer have to
	   be SO paranoid about using fire/frost/acid balls. -CFT */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j) && (cave[i][j].tptr != 0) &&
		(*destroy)(&t_list[cave[i][j].tptr]))
	      (void) delete_object(i, j); /* burn/corrode or OW destroy items
	      					in area of effect */
	/* now go over area of affect and DO something to monsters... */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      c_ptr = &cave[i][j];
	      if (c_ptr->fval <= MAX_OPEN_SPACE) {
		if (c_ptr->cptr > 1) {
		  dam = dam_hp;
		  m_ptr = &m_list[c_ptr->cptr];
		  spell_hit_monster(m_ptr, typ, &dam, distance(i,j,y,x)+1,
		  		&ny, &nx, 1);
		  c_ptr = &cave[ny][nx]; /* may be new location if teleported by
		      			gravity warp... */
		  m_ptr = &m_list[c_ptr->cptr]; /* and even if not, may be new
		      			monster if choas polymorphed */
		  r_ptr = &c_list[m_ptr->mptr];
		  monptr = c_ptr->cptr;

		  /* lite up creature if visible, temp
		     set pl so that update_mon works */
		  tmp = c_ptr->pl;
		  c_ptr->pl = TRUE;
		  update_mon((int)c_ptr->cptr);

		  thit++;
		  if (dam < 1) dam = 1; /* protect vs neg damage -CFT */
		  k = mon_take_hit((int)c_ptr->cptr, dam);
		  if (k >= 0)
 		    tkill++;
		  c_ptr->pl = tmp;
		}
		lite_spot(i,j); /* erase the ball... */
	      }
	    }
	/* show ball of whatever */
	put_qio();

	/* End  explosion.		     */
	if (tkill >= 0)
	  prt_experience();
	/* End ball hitting.		     */
      } else if (panel_contains(y, x) && (py.flags.blind < 1)) {
#ifdef TC_COLOR
	if (!no_color_flag) textcolor(bolt_color(typ));
#endif	  
	print(b_c, y, x);
	put_qio();
#ifdef TC_COLOR
	if (!no_color_flag) textcolor(LIGHTGRAY);
#endif	  
#ifdef MSDOS
	delay(23); /* milli-secs -CFT */
#else
	usleep(23000);
#endif
      }
      oldy = y;
      oldx = x;
    }
    if (target_mode && at_target(y,x))
      flag = TRUE; /* must have hit "targetted" area -CFT */
  } while (!flag);
}

/*Lightning ball in all directions			  SM   */
void starball(y,x)
register int y, x;
{
  register int i;

  for (i = 1; i <= 9; i++)
    if (i != 5)
      fire_ball(GF_LIGHTNING, i, y, x, 150, 2);
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
  int32u tmp, treas;
  int (*destroy)();
  register cave_type *c_ptr;
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  int ny, nx;
  int blind = (py.flags.status & PY_BLIND) ? 1 : 0;
  
  r_ptr = &c_list[m_list[monptr].mptr];
  if (r_ptr->cchar == 'D') { /* ancient dragons/wyrms breath bigger */
      max_dis = 3;
    if (r_ptr->cdefense & UNIQUE) max_dis++; /* unique dragons breath
				bigger than normal -CFT */
    }
  else max_dis = 2; /* hounds, etc... */

  ball_destroy(typ, &destroy);

	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(y, x, i, j) && (cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && !(py.flags.status & PY_BLIND)) {
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(bolt_color(typ));
#endif
	      print('*', i, j);
#ifdef TC_COLOR
	      if (!no_color_flag) textcolor(LIGHTGRAY); /* prob don't need here, but... -CFT */
#endif
	      }
        if (!(py.flags.status & PY_BLIND)) {
          put_qio();
#ifdef MSDOS
	  delay(75); /* millisecs, so we see the ball we just drew */
#else
	  usleep(75000); /* useconds */
#endif
	  }
	/* now erase the ball, since effects below may use msg_print, and
	   pause indefinitely, so we want ball gone before then -CFT */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
		los(y, x, i, j) &&
		(cave[i][j].fval <= MAX_OPEN_SPACE) &&
		panel_contains(i, j) && !(py.flags.status & PY_BLIND)) {
	      lite_spot(i,j); /* draw what is below the '*' */
	      }
	put_qio();
	/* first, go over area of affect and destroy preexisting items.
	   This change means that any treasure droped by killed monsters
	   is safe from the effects of this ball (but not from any later
	   balls/breathes, even if they happen before the player gets a
	   chance to pick up that scroll of *Acquirement*).  The
	   assumption is made that this treasure was shielded from the
	   effects by the corpse of the killed monster. -CFT */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j) && (cave[i][j].tptr != 0)
		&& (*destroy)(&t_list[cave[i][j].tptr]))
	      delete_object(i,j);
	/* now go over area of affect and DO something to monsters */
	for (i = y-max_dis; i <= y+max_dis; i++)
	  for (j = x-max_dis; j <= x+max_dis; j++)
	    if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis)
		&& los(y, x, i, j)) {
	      c_ptr = &cave[i][j];
	      if ((c_ptr->tptr != 0) && (*destroy)(&t_list[c_ptr->tptr]))
		(void) delete_object(i, j);
	      if (c_ptr->fval <= MAX_OPEN_SPACE) {
		if ((c_ptr->cptr > 1) && (c_ptr->cptr != monptr)){
		  dam = dam_hp;
		  m_ptr = &m_list[c_ptr->cptr];
		  spell_hit_monster(m_ptr, typ, &dam, distance(i,j,y,x)+1,
		  		&ny, &nx, 0);
		  c_ptr = &cave[ny][nx]; /* may be new location if teleported by
		      			gravity warp... */
		  m_ptr = &m_list[c_ptr->cptr]; /* and even if not, may be new
		      			monster if choas polymorphed */
		  r_ptr = &c_list[m_ptr->mptr];

		  /* can not call mon_take_hit here, since player does not
		     get experience for kill */
		  if (dam < 1) dam = 1;
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
		  m_ptr = &m_list[monptr];
		  /* let's do at least one point of damage */
		  /* prevents randint(0) problem with poison_gas, also */
		  if (dam <= 0)
		    dam = 1;
                  if (dam>1600)
		    dam = 1600;
		  switch(typ) {
		  case GF_LIGHTNING:	light_dam(dam, ddesc); break;
		  case GF_POISON_GAS:	poison_gas(dam, ddesc); break;
		  case GF_ACID:		acid_dam(dam, ddesc); break;
		  case GF_FROST:	cold_dam(dam, ddesc); break;
		  case GF_FIRE:		fire_dam(dam, ddesc); break;
		  case GF_MAGIC_MISSILE: take_hit(dam, ddesc); break;
		  case GF_HOLY_ORB:
		    dam /= 2; /* player should take less damage
				from "good" power-CFT */
		    take_hit(dam, ddesc);
		    break;
		  case GF_ARROW: /* maybe can miss? */
		    take_hit(dam, ddesc);
		    break;
		  case GF_PLASMA:  /* no resist to plasma? */
		    take_hit(dam, ddesc);
	            if (!py.flags.sound_resist)
		      stun_player( randint(
			(dam_hp > 40) ? 35 : (dam_hp*3/4+5) ) );
		    break;
		  case GF_NETHER:
		    if (py.flags.nether_resist) {
		      dam *= 6; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    else { /* no resist */
		      if (py.flags.hold_life && randint(3)>1)
		        msg_print("You keep hold of your life force!");
		      else if (py.flags.hold_life) {
			msg_print("You feel your life slipping away!");
			lose_exp(200+(py.misc.exp / 1000)*MON_DRAIN_LIFE);
		        }
		      else {
			msg_print("You feel your life draining away!");
			lose_exp(200+(py.misc.exp / 100)*MON_DRAIN_LIFE);
			}
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_WATER:
	            if (!py.flags.sound_resist)
	              stun_player(randint(55));
		    if (!player_saves() && !py.flags.confusion_resist
		    	&& !py.flags.chaos_resist) {
		      if ((py.flags.confused > 0) && (py.flags.confused < 32000))
		        py.flags.confused += 6;
		      else
		        py.flags.confused = randint(8)+6;
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_CHAOS:
		    if (py.flags.chaos_resist) {
		      dam *= 6; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    if ((!py.flags.confusion_resist) && (!py.flags.chaos_resist)) {
		      if (py.flags.confused > 0)
			py.flags.confused += 12;
		      else
			py.flags.confused = randint(20) + 10;
	              }
		    if (!py.flags.chaos_resist)
		      py.flags.image += randint(10);
		    if (!py.flags.nether_resist && !py.flags.chaos_resist) {
		      if (py.flags.hold_life && randint(3)>1)
		        msg_print("You keep hold of your life force!");
		      else if (py.flags.hold_life) {
			msg_print("You feel your life slipping away!");
			lose_exp(500+(py.misc.exp / 1000)*MON_DRAIN_LIFE);
		        }
		      else {
			msg_print("You feel your life draining away!");
			lose_exp(5000+(py.misc.exp / 100)*MON_DRAIN_LIFE);
			}
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_SHARDS:
		    if (py.flags.shards_resist) {
		      dam *= 6; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    else {
		      cut_player(dam); /* ouch! */
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_SOUND:
		    if (py.flags.sound_resist) {
		      dam *= 5;
		      dam /= (randint(6)+6);
		      }
		    else {
		      stun_player( randint( (dam > 90) ? 35 : (dam/3+5) ) );
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_CONFUSION:
		    if (py.flags.confusion_resist) {
		      dam *= 5; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    if (!py.flags.confusion_resist && !py.flags.chaos_resist) {
		      if (py.flags.confused > 0)
		        py.flags.confused += 12;
		      else
		        py.flags.confused = randint(20) + 10;
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_DISENCHANT:
		    if (py.flags.disenchant_resist) {
		      dam *= 6; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    else {
		      int8u disenchant = FALSE, chance = 1;
		      int t;
		      inven_type *i_ptr;
	
		      switch(randint(7)) {
			case 1: t = INVEN_BODY; break;
			case 2: t = INVEN_BODY;  break;
			case 3: t = INVEN_ARM;   break;
			case 4: t = INVEN_OUTER; break;
			case 5: t = INVEN_HANDS; break;
			case 6: t = INVEN_HEAD;  break;
			case 7: t = INVEN_FEET;  break;
			}
		      i_ptr = &inventory[t];
	      if (i_ptr->tval != TV_NOTHING){
		if (i_ptr->flags2 & TR_ARTIFACT)
		  chance = randint(5);
		      if ((i_ptr->tohit > 0) && (chance < 3)) {
		        i_ptr->tohit -= randint(2);
		        /* don't send it below zero */
		        if (i_ptr->tohit < 0)
		          i_ptr->tohit = 0;
		        disenchant = TRUE;
		      }
		      if ((i_ptr->todam > 0) && (chance < 3)) {
		        i_ptr->todam -= randint(2);
		        /* don't send it below zero */
		        if (i_ptr->todam < 0)
		          i_ptr->todam = 0;
		        disenchant = TRUE;
		      }
		      if ((i_ptr->toac > 0) && (chance < 3)) {
		        i_ptr->toac  -= randint(2);
		        /* don't send it below zero */
		        if (i_ptr->toac < 0)
		          i_ptr->toac = 0;
		        disenchant = TRUE;
		      }
		    if (disenchant || (chance >2)) {
		    vtype t1, t2;
		    objdes(t1, &inventory[t], FALSE);
		    if (chance < 3)
		      sprintf(t2, "Your %s (%c) %s disenchanted!", t1,
			t+'a'-INVEN_WIELD,
			(inventory[t].number != 1) ? "were":"was");
		    else
		      sprintf(t2, "Your %s (%c) %s disenchantment!", t1,
			t+'a'-INVEN_WIELD,
			(inventory[t].number != 1) ? "resist":"resists");
		    msg_print (t2);
		      calc_bonuses ();
		      }
		     }
		    }
		    take_hit(dam, ddesc);
		    break;
		  case GF_NEXUS: /* no spec. effects from nexus bolt, only breath -CFT */
		    if (py.flags.nexus_resist) {
		      dam *= 6; /* these 2 lines give avg dam of .655, ranging from */
		      dam /= (randint(6)+6); /* .858 to .5 -CFT */
		      }
		    else { /* special effects */
		      switch (randint(7)) {
			case 1: case 2: case 3:
			  teleport(200);
			  break;
			case 4: case 5:
			  teleport_to((int)m_ptr->fy, (int)m_ptr->fx);
			  break;
			case 6:
			  if (player_saves())
			    msg_print("You resist the effects.");
			  else {
			    int k=dun_level;
			    if (dun_level==Q_PLANE) dun_level=0;
			    else if (is_quest(dun_level)) dun_level-=1;
			    else dun_level+=(-3)+2*randint(2);
			    if (dun_level<0) dun_level=0;
			    if (k==Q_PLANE)
			      msg_print("You warp through a cross-dimension gate.");
			    else if (k<dun_level)
			      msg_print("You sink through the floor.");
			    else
			      msg_print("You rise up through the ceiling.");
			    new_level_flag=TRUE;
			    }
			  break;
			case 7:
			  if (player_saves() && randint(2)==1)
			    msg_print("You resist the effects.");
			  else {
			    int max1, cur1, max2, cur2, i, j;
			    msg_print("Your body starts to scramble...");
			    i=randint(6)-1;
			    do { j=randint(6)-1; } while (i==j);
			    max1 = py.stats.max_stat[i];
			    cur1 = py.stats.cur_stat[i];
			    max2 = py.stats.max_stat[j];
			    cur2 = py.stats.cur_stat[j];
			    py.stats.max_stat[i] = max2;
			    py.stats.cur_stat[i] = cur2;
			    py.stats.max_stat[j] = max1;
			    py.stats.cur_stat[j] = cur1;
			    set_use_stat(i);
			    set_use_stat(j);
			    prt_stat(i);
			    prt_stat(j);
			    }
			  } /* switch for effects */
			}
		    take_hit(dam, ddesc);
		    break;
		  case GF_FORCE:
		    if (!py.flags.sound_resist)
		      stun_player(randint(20));
		    take_hit(dam, ddesc);
		    break;
		  case GF_INERTIA:
		    if ((py.flags.slow > 0) && (py.flags.slow < 32000))
		      py.flags.slow += randint(5);
		    else {
		      msg_print("You feel less able to move.");
		      py.flags.slow = randint(5)+3;
		      }
		    take_hit(dam, ddesc);
		    break;
		  case GF_LIGHT:
		    if (py.flags.light_resist) {
		      dam *= 4;
		      dam /= (randint(6)+6);
		      }
		    else if (!blind && !py.flags.blindness_resist) {
		      msg_print("You are blinded by the flash!");
		      py.flags.blind = randint(6)+3;
		      }
		    light_area(char_row, char_col);
		    take_hit(dam, ddesc);
		    break;
		  case GF_DARK:
		    if (py.flags.dark_resist) {
		      dam *= 4;
		      dam /= (randint(6)+6);
		      }
		    unlight_area(char_row, char_col);
		    take_hit(dam, ddesc);
		    break;
		  case GF_TIME: /* only some effects from time bolt -CFT */
		    switch(randint(10)) {
		      case 1: case 2: case 3: case 4: case 5:
		        msg_print("You feel life has clocked back.");
		        lose_exp(m_ptr->hp+(py.misc.exp / 300)*MON_DRAIN_LIFE);
			break;
		      case 6: case 7: case 8: case 9:
			{
		        int t;
		        switch(randint(6)) {
		          case 1:
		      	    t = A_STR;
		      	    msg_print("You're not as strong as you used to be...");
		      	    break;
		      	  case 2:
		      	    t = A_INT;
		      	    msg_print("You're not as bright as you used to be...");
		      	    break;
		      	  case 3:
		      	    t = A_WIS;
		      	    msg_print("You're not as wise as you used to be...");
		      	    break;
		      	  case 4:
		      	    t = A_DEX;
		      	    msg_print("You're not as agile as you used to be...");
		      	    break;
		      	  case 5:
		      	    t = A_CON;
		      	    msg_print("You're not as hale as you used to be...");
		      	    break;
		          case 6:
		      	    t = A_CHR;
		      	    msg_print("You're not as beautiful as you used to be...");
		      	    break;
		          }
		        py.stats.cur_stat[t]=(py.stats.cur_stat[t]*3)/4;
		        if (py.stats.cur_stat[t] < 3) py.stats.cur_stat[t]=3;
		        set_use_stat(t);
		        prt_stat(t);
		        }
		      break;
		      case 10:
		        { int i;
		          for (i=0;i<6;i++){
		            py.stats.cur_stat[i]=(py.stats.cur_stat[i]*3)/4;
		            if (py.stats.cur_stat[i] < 3) py.stats.cur_stat[i] = 3;
			    set_use_stat(i);
			    prt_stat(i);
			    }
			  }
		        msg_print("You're not as strong as you used to be...");
		        msg_print("You're not as bright as you used to be...");
		        msg_print("You're not as wise as you used to be...");
		        msg_print("You're not as agile as you used to be...");
		        msg_print("You're not as hale as you used to be...");
		        msg_print("You're not as beautiful as you used to be...");
		        break;
		      } /* randint(10) for effects */
		    take_hit(dam, ddesc);
		    break;
		  case GF_GRAVITY:
		    if (!py.flags.sound_resist)
		      stun_player( randint( (dam > 90) ? 35 : (dam/3+5) ) );
		    if ((py.flags.slow > 0) && (py.flags.slow < 32000))
		      py.flags.slow += randint(5);
		    else {
		      msg_print("You feel less able to move.");
		      py.flags.slow = randint(5)+3;
		      }
		    msg_print("Gravity warps around you.");
		    teleport(5);
		    take_hit(dam, ddesc);
		    break;
		  case GF_MANA:
		    take_hit(dam, ddesc);
		    break;
		  case GF_METEOR:
		    take_hit(dam, ddesc);
		    break;
		  case GF_ICE:
		    cold_dam(dam, ddesc);
		    if (!py.flags.sound_resist)
		      stun_player(randint(25));
		    if (!py.flags.shards_resist)
		      cut_player(damroll(8,10));
		    break;
		  default:
		    msg_print("Unknown typ in breath().  This may mean trouble.");
		  }
		}
	    }
	}
  /* show the ball of gas */
  put_qio();

/* erase ball and redraw */
  for (i = (y - max_dis); i <= (y + max_dis); i++)
    for (j = (x - max_dis); j <= (x + max_dis); j++)
      if (in_bounds(i, j) && panel_contains(i, j) &&
	  (distance(y, x, i, j) <= max_dis))
	lite_spot(i, j);
}


/* Recharge a wand, staff, or rod.  Sometimes the item breaks. -RAK-*/
int recharge(num)
register int num;
{
  int i, j, k, l, item_val;
  register int res;
  register inven_type *i_ptr;
  int found = FALSE;
  
  res = FALSE;
  if (find_range(TV_STAFF, TV_WAND, &i, &j))
    found = TRUE;
  if (find_range(TV_ROD, TV_NEVER, &k, &l))
    found = TRUE;

  if (!found)
    msg_print("You have nothing to recharge.");
  else if (get_item(&item_val, "Recharge which item?",
		(k > -1)?k:i, (j > -1)?j:l, 0))
    {
      i_ptr = &inventory[item_val];
      res = TRUE;
      if (i_ptr->tval == TV_ROD) { /* now allow players to speed up recharge
      					time of rods -CFT */
	int16u t_o = i_ptr->timeout, t;
	
      	if (randint((100 - i_ptr->level + num)/5) == 1) { /* not today... */
      	  msg_print("The recharge backfires, and drains the rod further!");
	  if (t_o < 32000) /* don't overflow... */
	    i_ptr->timeout = (t_o + 100) * 2;
	  }
	else {
	  t = (int16u)(num * damroll(2,4)); /* rechange amount */
	  if (t_o < t)
	    i_ptr->timeout = 0;
	  else
	    i_ptr->timeout = t_o - t;
	  }
        } /* if recharge rod... */
      else { /* recharge wand/staff */
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
	      m_ptr->csleep = 0;
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
	      m_ptr->csleep = 0;
	    }
	  else
	    {
	      if (m_ptr->confused < 230) 
		m_ptr->confused += (int8u)(damroll(3,5) + 1);
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
	      if ((t_list[c_ptr->tptr].tval == TV_RUBBLE) && (randint(10)==1)) {
	        delete_object(y,x);
	        place_object(y,x);
	        lite_spot(y,x);
	        (void) sprintf(out_val, "The %s turns into mud, revealing an object!", tmp_str);
	        }
	      else {
                (void) delete_object(y, x);
	        (void) sprintf(out_val, "The %s turns into mud.", tmp_str);
	        }
	      msg_print(out_val);
	      wall = TRUE;
	    }
	}
      if (c_ptr->cptr > 1)
	{
	  m_ptr = &m_list[c_ptr->cptr];
	  r_ptr = &c_list[m_ptr->mptr];
	  if (HURT_ROCK & r_ptr->cdefense)
	    {
	      monster_name (m_name, m_ptr, r_ptr);
	      i = mon_take_hit((int)c_ptr->cptr, 20 +
			damroll(randint(6),randint(30)) );
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


/* polymorph is now uniform for poly/mass poly/choas poly, and only
   as deadly as chaos poly is.  This still makes polymorphing a bad
   idea, but it won't be automatically fatal. -CFT */
static int poly(int mnum){
  register creature_type *c_ptr = &c_list[m_list[mnum].mptr];
  int y, x;
  int i,j,k;
  
  if (c_ptr->cdefense & UNIQUE) return 0;
  y = m_list[mnum].fy;
  x = m_list[mnum].fx;
  i = (randint(20)/randint(9))+1;
  k = j = c_ptr->level;
  if ((j -=i)<0) j = 0;
  if ((k +=i)>MAX_MONS_LEVEL) k = MAX_MONS_LEVEL;
  delete_monster(mnum);
  do {
    i = randint(m_level[k]-m_level[j])-1+m_level[j];  /* new creature index */
  } while (c_list[i].cdefense & UNIQUE);
  place_monster(y,x,i,FALSE);
  return 1;
}

/* polymorph now safer.  not safe, just safer -CFT */      
/* Polymorph a monster					-RAK-	*/
/* NOTE: cannot polymorph a winning creature (BALROG)		 */
int poly_monster(dir, y, x)
int dir, y, x;
{
  int dist, flag, flag2, p;
  register cave_type *c_ptr;
  register creature_type *r_ptr;
  register monster_type *m_ptr;
  vtype out_val, m_name;
  int i; /* temp used to check monster to see it it's a unique -CFT */
  
  p = FALSE;
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
	  if ((r_ptr->level < randint((py.misc.lev-10)<1?1:(py.misc.lev-10))+10)
		&& !(r_ptr->cdefense & UNIQUE)) {
	      poly(c_ptr->cptr);
	      if (panel_contains(y, x) && (c_ptr->tl || c_ptr->pl))
		p = TRUE;
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
  return(p);
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
	    if (((t_list[c_ptr->tptr].tval >= TV_MIN_WEAR) &&
	        (t_list[c_ptr->tptr].tval <= TV_MAX_WEAR) &&
	        (t_list[c_ptr->tptr].flags2 & TR_ARTIFACT))
	    	|| (t_list[c_ptr->tptr].tval == TV_UP_STAIR)
	    	|| (t_list[c_ptr->tptr].tval == TV_DOWN_STAIR)
	    	|| (t_list[c_ptr->tptr].tval == TV_STORE_DOOR))
	      continue; /* don't bury the artifact/stair/store */
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
      } while (!in_bounds(y, x));
      ctr++;
      if (ctr > (4*dis*dis + 4*dis + 1))
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
      if ((m_ptr->cdis > MAX_SIGHT) ||
	  !los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
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
  int j; /* used to hold monster index -CFT */

  mass = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if (m_ptr->cdis <= MAX_SIGHT)
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  if (((r_ptr->cmove & CM_WIN) == 0) && !(r_ptr->cdefense & UNIQUE)) {
            mass = poly(i);
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
	  if (!no_color_flag) {
	    textcolor(mon_color(m_ptr->color));
	    } /* if !no_color_flag */
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

  for (i = char_row-10; i <= char_row+10; i++)
    for (j = char_col-10; j <= char_col+10; j++)
      if (((i != char_row) || (j != char_col)) &&
	  in_bounds(i, j) && (distance(char_row, char_col, i, j)<=10) &&
	  (randint(8) == 1))
	{
	  c_ptr = &cave[i][j];
	  if (c_ptr->tptr != 0)
	    if (((t_list[c_ptr->tptr].tval >= TV_MIN_WEAR) &&
	    	(t_list[c_ptr->tptr].tval <= TV_MAX_WEAR) &&
	    	(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT))
	    	|| (t_list[c_ptr->tptr].tval == TV_UP_STAIR)
	    	|| (t_list[c_ptr->tptr].tval == TV_DOWN_STAIR)
	    	|| (t_list[c_ptr->tptr].tval == TV_STORE_DOOR))
	      continue; /* don't touch artifacts or stairs or stores -CFT */
	    else
	      (void) delete_object(i, j);
	  if (c_ptr->cptr > 1)
	    {
	      m_ptr = &m_list[c_ptr->cptr];
	      r_ptr = &c_list[m_ptr->mptr];

	      if (!(r_ptr->cmove&CM_PHASE) && !(r_ptr->cdefense&BREAK_WALL))
		{
		  if ((movement_rate (c_ptr->cptr) == 0) ||
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
		    damage = 320;
		  else
		    damage = damroll (3+randint(3), 8+randint(5));
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
      int cur_pos;

      cur_pos = popt();
      cave[char_row][char_col].tptr = cur_pos;
      invcopy(&t_list[cur_pos], OBJ_MUSH);
      msg_print ("You feel something roll beneath your feet.");
    }
}

int banish_creature(cflag, distance)
int32u cflag;
int distance;
{
  register int i;
  int dispel;
  register monster_type *m_ptr;

  dispel = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      if ((cflag & c_list[m_ptr->mptr].cdefense) &&
	  (m_ptr->cdis <= MAX_SIGHT) &&
	  los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	{
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
  register monster_type *m_ptr;
  register creature_type *r_ptr;
  vtype out_val, m_name;

  msg_print("Probing...");
  probe = FALSE;
  for (i = mfptr - 1; i >= MIN_MONIX; i--)
    {
      m_ptr = &m_list[i];
      r_ptr = &c_list[m_ptr->mptr];
      if ((m_ptr->cdis <= MAX_SIGHT) &&
	  los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
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
	  (m_ptr->cdis <= MAX_SIGHT) &&
	  los(char_row, char_col, (int)m_ptr->fy, (int)m_ptr->fx))
	{
	  r_ptr = &c_list[m_ptr->mptr];
	  c_recall[m_ptr->mptr].r_cdefense |= cflag;
	  visible = m_ptr->ml;  /* set this before call mon_take_hit */
	  k = mon_take_hit (i, randint(damage));
	  if (visible) 
	    monster_name (m_name, m_ptr, r_ptr);
	  else
	    strcpy(m_name, "It");	  
	  if (k >= 0)
	    (void) sprintf(out_val, "%s dissolves!", m_name);
	  else
	    (void) sprintf(out_val, "%s shudders.", m_name);
	  msg_print(out_val);
	  dispel = TRUE;
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
	  && (m_ptr->cdis <= MAX_SIGHT) 
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
	      m_ptr->confused = randint(py.misc.lev)*2;
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
	      ((cave[i][j].tptr == 0) || 
	       ((t_list[cave[i][j].tptr].tval != TV_UP_STAIR) &&
	        (t_list[cave[i][j].tptr].tval != TV_DOWN_STAIR))))
	    {
	      k = distance(i, j, y, x);
	      if (k == 0) /* player's spot... */
		replace_spot(i, j, 1); /* clear player's spot... from um55 -CFT */
	      else if (k < 13)
		replace_spot(i, j, randint(6));
	      else if (k < 16)
		replace_spot(i, j, randint(9));
	    }
    }
  msg_print("There is a searing blast of light!");
  if (!py.flags.blindness_resist)
    py.flags.blind += 10 + randint(10);
}


/* Revamped!  Now takes item pointer, number of times to try enchanting,
   and a flag of what to try enchanting.  Artifacts resist enchantment
   some of the time, and successful enchantment to at least +0 might
   break a curse on the item.  -CFT */
/* Enchants a plus onto an item.			-RAK-	*/
int enchant(inven_type *i_ptr, int n, int8u eflag){
  register int chance, res = FALSE, i, a = i_ptr->flags2 & TR_ARTIFACT;
  int table[13] = {  10,  50, 100, 200, 300, 400,
  			   500, 700, 950, 990, 992, 995, 997 };
  for(i=0; i<n; i++){
    chance = 0;
    if (eflag & ENCH_TOHIT) {
      if (i_ptr->tohit < 1) chance = 0;
      else if (i_ptr->tohit > 13) chance = 1000;
      else chance = table[i_ptr->tohit-1];
      if ((randint(1000)>chance) && (!a || randint(7)>3)) {
      	i_ptr->tohit++;
      	res = TRUE;
        if ((i_ptr->tohit >= 0) && (randint(4)==1) &&  /* only when you get */
		(i_ptr->flags & TR_CURSED)) { 	/*  it above -1 -CFT */
	  msg_print("The curse is broken!");
	  i_ptr->flags &= ~TR_CURSED;
	  i_ptr->ident &= ~ID_DAMD;
	  }
        }
      }
    if (eflag & ENCH_TODAM) {
      if (i_ptr->todam < 1) chance = 0;
      else if (i_ptr->todam > 13) chance = 1000;
      else chance = table[i_ptr->todam-1];
      if ((randint(1000)>chance) && (!a || randint(7)>3)) {
      	i_ptr->todam++;
      	res = TRUE;
        if ((i_ptr->todam >= 0) && (randint(4)==1) &&  /* only when you get */
		(i_ptr->flags & TR_CURSED)) { 	/*  it above -1 -CFT */
	  msg_print("The curse is broken!");
	  i_ptr->flags &= ~TR_CURSED;
	  i_ptr->ident &= ~ID_DAMD;
	  }
        }
      }
    if (eflag & ENCH_TOAC) {
      if (i_ptr->toac < 1) chance = 0;
      else if (i_ptr->toac > 13) chance = 1000;
      else chance = table[i_ptr->toac-1];
      if ((randint(1000)>chance) && (!a || randint(7)>3)) {
      	i_ptr->toac++;
      	res = TRUE;
        if ((i_ptr->toac >= 0) && (randint(4)==1) &&  /* only when you get */
		(i_ptr->flags & TR_CURSED)) { 	/*  it above -1 -CFT */
	  msg_print("The curse is broken!");
	  i_ptr->flags &= ~TR_CURSED;
	  i_ptr->ident &= ~ID_DAMD;
	  }
        }
      }
  } /* for loop */
  if (res) calc_bonuses ();
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
      (c_ptr->cchar=='m') || ((c_ptr->cchar=='e') && stricmp(c_ptr->name,
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
	  if (!(!stricmp(object_list[i_ptr->index].name, "Power") &&
	    (i_ptr->tval == TV_RING))) {
	      i_ptr->flags &= ~TR_CURSED;
	      i_ptr->ident &= ~ID_DAMD; /* DGK */
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
	  if (!(!stricmp(object_list[i_ptr->index].name, "Power") &&
	   (i_ptr->tval == TV_RING))) {
	     i_ptr->flags &= ~TR_CURSED;
	     i_ptr->ident &= ~ID_DAMD; /* DGK */
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
    prt("You are resistant to storms of chaos.", i++, j);    
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

/* msgs about STR,INT,etc bonuses removed, since the player will see this
   on the stat display anyways -CFT */
   
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


#define NO_RES 0
#define SOME_RES 1
#define RESIST 2
#define IMMUNE 3
#define SUSCEPT 4
#define CHANGED 5
#define CONFUSED 6
#define MORE_CONF 7
#define DAZED 8
#define MORE_DAZED 16
#define DEAD 32
/* This function will process a bolt/ball/breath spell hitting a monster.
   It checks for resistances, and reduces damage accordingly, and also
   adds in what "special effects" apply to the monsters.  'rad' is used to
   indicate the distance from "ground 0" for ball spells.  For bolts, rad
   should be a 0 (this flags off some of the messages).  dam is changed
   to reflect resistances and range. -CFT */
static void spell_hit_monster(monster_type *m_ptr, int typ, int *dam, int rad,
		int *y, int *x, int8u by_player){
  register creature_type *r_ptr;
  int blind = (py.flags.status & PY_BLIND) ? 1 : 0;
  int res; /* controls messages, using above #defines -CFT */
  vtype cdesc, outval;

  if (rad)
    *dam /= rad; /* adjust damage for range... */

  *y = m_ptr->fy; /* these only change if mon gets teleported */
  *x = m_ptr->fx; 
  r_ptr = &c_list[m_ptr->mptr];
  if (m_ptr->ml){
    if (r_ptr->cdefense & UNIQUE)
      sprintf(cdesc, "%s ", r_ptr->name);
    else
      sprintf(cdesc, "The %s ", r_ptr->name);
    }
  else
    strcpy(cdesc, "It ");

  res = NO_RES; /* assume until we know different -CFT */
  switch ( typ ){ /* check for resists... */
    case GF_MAGIC_MISSILE: /* pure damage, no resist possible */
      break;
    case GF_LIGHTNING:
      if (r_ptr->cdefense & IM_LIGHTNING) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_LIGHTNING;
        }
      break;
    case GF_POISON_GAS:
      if (r_ptr->cdefense & IM_POISON) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_POISON;
        }
      break;
    case GF_ACID:
      if (r_ptr->cdefense & IM_ACID) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_ACID;
        }
      break;
    case GF_FROST:
      if (r_ptr->cdefense & IM_FROST) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_FROST;
        }
      break;
    case GF_FIRE:
      if (r_ptr->cdefense & IM_FIRE) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_FIRE;
        }
      break;
    case GF_HOLY_ORB:
      if (r_ptr->cdefense & EVIL) {
	*dam *= 2;
	res = SUSCEPT;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= EVIL;
        }
      break;
    case GF_ARROW: /* for now, no defense... maybe it should have a
    			chance of missing? -CFT */
      break;
    case GF_PLASMA: /* maybe IM_LIGHTNING (ball lightning is supposed
		to be plasma) or IM_FIRE (since it's hot)? -CFT */
      if (!strncmp("Plasma", r_ptr->name, 6) ||
          (r_ptr->spells3 & BREATH_PL)){  /* if is a "plasma" monster,
          				or can breathe plasma, then
          				we assume it should be immune.
          				plasma bolts don't count, since
          				mage-types could have them, and
          				not deserve plasma-resist -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
	}
      break;
    case GF_NETHER: /* I assume nether is an evil, necromantic force,
    			so it doesn't hurt undead, and hurts evil less -CFT */
      if (r_ptr->cdefense & UNDEAD) {
	res = IMMUNE;
        *dam = 0;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= UNDEAD;
        }
      else if (r_ptr->spells2 & BREATH_LD) { /* if can breath nether, should get
      						good resist to damage -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
      }
      else if (r_ptr->cdefense & EVIL) {
        *dam /= 2;	/* evil takes *2 for holy, so /2 for this... -CFT */
	res = SOME_RES;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= EVIL;
        }
      break;
    case GF_WATER:	/* water elementals should resist.  anyone else? -CFT */
      if ((r_ptr->cchar == 'E') && (r_ptr->name[0] == 'W')){
	res = IMMUNE;
        *dam = 0; /* water spirit, water ele, and Waldern -CFT */
        }
      break;
    case GF_CHAOS:
      if (r_ptr->spells2 & BREATH_CH){ /* assume anything that breathes
		choas is chaotic enough to deserve resistance... -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      if ((*dam <= m_ptr->hp) && /* don't bother if it's gonna die */
	  !(r_ptr->spells2 & BREATH_CH) &&
	  !(r_ptr->cdefense & UNIQUE) &&
	  (randint(90) > r_ptr->level)) { /* then we'll polymorph it -CFT */
	res = CHANGED;
        if (poly(cave[*y][*x].cptr))
  	  *dam = 0; /* new monster was not hit by choas breath.  This also
			makes things easier to handle */
      } /* end of choas-poly.  If was poly-ed don't bother confuse... it's
		too hectic to keep track of... -CFT */
    else if (!(r_ptr->cdefense & CHARM_SLEEP) &&
	  !(r_ptr->spells2 & BREATH_CH) && /* choatics hard to confuse */
	  !(r_ptr->spells2 & BREATH_CO)){   /* so are bronze dragons */
	if (m_ptr->confused > 0) { 
	  res = MORE_CONF;
	  if (m_ptr->confused < 240){ /* make sure not to overflow -CFT */
	    m_ptr->confused += 7/(rad>0 ? rad : 1);
	    }
	  }
	else {
	  res = CONFUSED;
	  m_ptr->confused = (randint(11)+5)/(rad>0 ? rad : 1);
	  }
	}
      break;
    case GF_SHARDS:
      if (r_ptr->spells2 & BREATH_SH){ /* shard breathers resist -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      break;
    case GF_SOUND:
      if (r_ptr->spells2 & BREATH_SD){ /* ditto for sound -CFT */
	res = RESIST;
	*dam *= 2;
	*dam /= (randint(6)+6);
        }
      if ((*dam <= m_ptr->hp) && /* don't bother if it's dead */
	  !(r_ptr->spells2 & BREATH_SD) &&
	  !(r_ptr->spells3 & BREATH_WA)) { /* sound and impact breathers
	  					should not stun -CFT */
	if (m_ptr->confused > 0) { 
	  res = MORE_DAZED;
	  if (m_ptr->confused < 220){ /* make sure not to overflow -CFT */
	    m_ptr->confused += (randint(5)*2)/(rad>0 ? rad : 1);
	    }
	  }
	else {
	  res = DAZED;
	  m_ptr->confused = (randint(15)+10)/(rad>0 ? rad : 1);
	  }
  	}
      break;
    case GF_CONFUSION:
      if (r_ptr->spells2 & BREATH_CO){ 
	res = RESIST;
	*dam *= 2;
	*dam /= (randint(6)+6);
        }
      else if (r_ptr->cdefense & CHARM_SLEEP){
	res = SOME_RES;
        *dam /= 2; /* only some resist, but they also avoid confuse -CFT */
        }
      if ((*dam <= m_ptr->hp) && /* don't bother if it's dead */
	  !(r_ptr->cdefense & CHARM_SLEEP) &&
	  !(r_ptr->spells2 & BREATH_CH) && /* choatics hard to confuse */
	  !(r_ptr->spells2 & BREATH_CO)) {  /* so are bronze dragons */
	if (m_ptr->confused > 0) { 
	  res = MORE_CONF;
	  if (m_ptr->confused < 240){ /* make sure not to overflow -CFT */
	    m_ptr->confused += 7/(rad>0 ? rad : 1);
	    }
	  }
	else {
	  res = CONFUSED;
	  m_ptr->confused = (randint(11)+5)/(rad>0 ? rad : 1);
	  }
	}
        break;
    case GF_DISENCHANT:
      if ((r_ptr->spells2 & BREATH_DI) ||
          !strncmp("Disen", r_ptr->name, 5)) {
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      break;
    case GF_NEXUS:
      if ((r_ptr->spells2 & BREATH_NE) ||
          !strncmp("Nexus", r_ptr->name, 5)) {
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      break;
    case GF_FORCE:
      if (r_ptr->spells3 & BREATH_WA){ /* breath ele force resists
      					 ele force -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      if ((*dam <= m_ptr->hp) &&
	  !(r_ptr->spells2 & BREATH_SD) &&
	  !(r_ptr->spells3 & BREATH_WA)){ /* sound and impact breathers
	  					should not stun -CFT */
	if (m_ptr->confused > 0) { 
	  res = MORE_DAZED;
	  if (m_ptr->confused < 220){ /* make sure not to overflow -CFT */
	    m_ptr->confused += (randint(5)+1)/(rad>0 ? rad : 1);
	    }
	  }
	else {
	  res = DAZED;
	  m_ptr->confused = randint(15)/(rad>0 ? rad : 1);
	  }
	}
      break;
    case GF_INERTIA:
      if (r_ptr->spells3 & BREATH_SL){ /* if can breath inertia, then
      					resist it. */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      break;
    case GF_LIGHT:
      if (r_ptr->spells3 & BREATH_LT){ /* breathe light to res light */
	res = RESIST;
	*dam *= 2;
	*dam /= (randint(6)+6);
        }
      else if (r_ptr->cdefense & HURT_LIGHT){
	res = SUSCEPT;
      	*dam *= 2; /* hurt bad by light */
        }
      else if (r_ptr->spells3 & BREATH_DA){ /* breathe dark gets hurt */
	res = SUSCEPT;
	*dam = (*dam * 3)/2;
        }
      break;
    case GF_DARK:
      if (r_ptr->spells2 & BREATH_DA){ /* shard breathers resist -CFT */
	res = RESIST;
	*dam *= 2;
	*dam /= (randint(6)+6);
        }
      else if (r_ptr->cdefense & HURT_LIGHT){
	res = SOME_RES;
      	*dam /= 2; /* hurt bad by light, so not hurt bad by dark */
        }
      else if (r_ptr->spells3 & BREATH_LT){ /* breathe light gets hurt */
	res = SUSCEPT;
	*dam = (*dam * 3)/2;
        }
      break;
    case GF_TIME:
      if (r_ptr->spells3 & BREATH_TI){ /* time breathers resist -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      break;
    case GF_GRAVITY:
      if (r_ptr->spells3 & BREATH_GR){ /* breathers resist -CFT */
	res = RESIST;
	*dam *= 3;  /* these 2 lines give avg dam of .33, ranging */
	*dam /= (randint(6)+6); /* from .427 to .25 -CFT */
        }
      else {
        if (*dam <= m_ptr->hp) {
	  teleport_away(cave[m_ptr->fy][m_ptr->fx].cptr, 5);
	  *y = m_ptr->fy; /* teleported, so let outside world know monster moved! */
	  *x = m_ptr->fx; 
	  }
        }
      break;
    case GF_MANA: /* raw blast of power. no way to resist, is there? */
      break;
    case GF_METEOR: /* GF_METEOR is basically a powerful magic-missile
    			ball spell.  I only made it a different type
    			so I could make it a different color -CFT */
      break;
    case GF_ICE: /* ice is basically frost + cuts + stun -CFT */
      if (r_ptr->cdefense & IM_FROST) {
	res = RESIST;
	*dam /= 9;
        if (m_ptr->ml)
          c_recall[m_ptr->mptr].r_cdefense |= IM_FROST;
        }
      if ((*dam <= m_ptr->hp) &&
	  !(r_ptr->spells2 & BREATH_SD) &&
	  !(r_ptr->spells3 & BREATH_WA)){  /* sound and impact breathers
	  					should not stun -CFT */
	if (m_ptr->confused > 0) { 
	  res += MORE_DAZED;
	  if (m_ptr->confused < 220){ /* make sure not to overflow -CFT */
	    m_ptr->confused += (randint(5)+1)/(rad>0 ? rad : 1);
	    }
	  }
	else {
	  res += DAZED;
	  m_ptr->confused = randint(15)/(rad>0 ? rad : 1);
	  }
	}
      break;
    default:
      msg_print("Unknown typ in spell_hit_monster.  This may mean trouble.");
  } /* end switch for saving throws and extra effects */

  if (res == CHANGED)
    sprintf(outval, "%schanges!",cdesc);
  else if ((*dam > m_ptr->hp) &&
	   (by_player || !(c_list[m_ptr->mptr].cdefense & UNIQUE))) {
    res = DEAD;
    if ((c_list[m_ptr->mptr].cdefense & (DEMON|UNDEAD)) ||
        (c_list[m_ptr->mptr].cchar == 'E') ||
        (c_list[m_ptr->mptr].cchar == 'v') ||
        (c_list[m_ptr->mptr].cchar == 'g') ||
        (c_list[m_ptr->mptr].cchar == 'X'))
      sprintf(outval, "%sis destroyed.", cdesc);
    else
      sprintf(outval, "%sdies.", cdesc);
   }
  else switch (res) {
    case NO_RES:	sprintf(outval, "%sis hit.",cdesc); break;
    case SOME_RES:	sprintf(outval, "%sresists somewhat.",cdesc); break;
    case RESIST:	sprintf(outval, "%sresists.",cdesc); break;
    case IMMUNE:	sprintf(outval, "%sis immune.",cdesc); break;
    case SUSCEPT:	sprintf(outval, "%sis hit hard.",cdesc); break;
    case CONFUSED:	sprintf(outval, "%sis confused.",cdesc); break;
    case MORE_CONF:	sprintf(outval, "%sis more confused.",cdesc); break;
    case DAZED:		sprintf(outval, "%sis dazed.",cdesc); break;
    case MORE_DAZED:	sprintf(outval, "%sis more dazed.",cdesc); break;
    case (DAZED+RESIST):	sprintf(outval,
			"%sresists, but is dazed anyway.",cdesc); break;
    case (MORE_DAZED+RESIST):	sprintf(outval,
			"%sresists, but still is more dazed.",cdesc); break;
    default: sprintf(outval,"%sis affected in a mysterious way.",cdesc);
    }
  if (rad || (res != NO_RES)) { /* don't show normal hit msgs for bolts -CFT */
    if (!blind)
      msg_print(outval);
  }	
}

/* This fn provides the ability to have a spell blast a line of creatures
   for damage.  It should look pretty neat, too... -CFT */
void line_spell(int typ, int dir, int y, int x, int dam){
  int ny,nx, dis = 0, flag = FALSE;
  int t, tdam;
  monster_type *m_ptr;
  cave_type *c_ptr;
  int8u path[OBJ_BOLT_RANGE+1][2]; /* pre calculate "flight" path, makes bolt
			calc faster because fns more likely to be in mem.
			Also allows redraw at reasonable spd -CFT */  

  path[0][0] = y;  path[0][1] = x; /* orig point */
  do {
    (void)mmove(dir, &y, &x);
    dis++;
    path[dis][0] = y;  path[dis][1] = x;
    if ((dis>OBJ_BOLT_RANGE) || (cave[y][x].fval >= MIN_CLOSED_SPACE))
      flag = TRUE;
    } while (!flag);

  flag = FALSE;
  dis = 0;
  do {
    dis++;
    y = path[dis][0];  x = path[dis][1];
    c_ptr = &cave[y][x];
    if ((dis > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE; /* then stop */
    else {
      if (c_ptr->cptr > 1) { /* hit a monster! */
	tdam = dam;
        m_ptr = &m_list[c_ptr->cptr];

	if (!(py.flags.status & PY_BLIND) && panel_contains(y,x)){
	  /* temp light monster to show it... */
          t = c_ptr->pl;
          c_ptr->pl = TRUE;
          update_mon((int)c_ptr->cptr);
          c_ptr->pl = t;
          put_qio(); /* draw monster */
	  }
	  
        spell_hit_monster(m_ptr, typ, &tdam, 1, &ny, &nx, 1); /* check resists */
	c_ptr = &cave[ny][nx]; /* may be new loc if tele by grav warp */

        (void) mon_take_hit((int)c_ptr->cptr, tdam); /* hurt it */
      }
    if (!(py.flags.status & PY_BLIND)) {
      for(t=1;t<=dis;t++)
        if (panel_contains(path[t][0],path[t][1])){
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(bolt_color(typ));
#endif
	  print(bolt_char(path[t][0],path[t][1],path[t-1][0], path[t-1][1]),
		path[t][0], path[t][1]);
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
	  }
	put_qio(); /* show line */
#ifdef MSDOS
        delay(25);
#else
        usleep(25000);
#endif      
      } /* if !blind */
    }  /* if hit monster */
  } while (!flag); /* end of effects loop */
  
  if (!(py.flags.status & PY_BLIND)) { /* now erase it -CFT */
    for(t=1;t<=dis;t++){ /* erase piece-by-piece... */
      lite_spot(path[t][0], path[t][1]);
      for(tdam=t+1;tdam<dis;tdam++){
        if (panel_contains(path[tdam][0], path[tdam][1])){
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(bolt_color(typ));
#endif
	  print(bolt_char(path[tdam][0],path[tdam][1],path[tdam-1][0],
		path[tdam-1][1]), path[tdam][0], path[tdam][1]);
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
	  }
        }
      put_qio();
#ifdef MSDOS
      delay(25);
#else
      usleep(25000);
#endif      
      } /* for each piece */
    } /* if !blind */
}  

