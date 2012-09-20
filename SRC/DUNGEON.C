/* dungeon.c: the main command interpreter, updating player status

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
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#include <strings.h>
#endif

/* ARG_* will collapse to nothing except on TURBOC -CFT */
static char original_commands(ARG_CHAR);
static void do_command(ARG_CHAR);
static int valid_countcommand(ARG_CHAR);
static void regenhp(ARG_INT);
static void regenmana(ARG_INT);
static int enchanted();
#ifdef LINT_ARGS  /* It's defined it in externs.h now, but I didn't
		     change the LINT_ARGS section... -CFT */
int special_check();
#endif
static void examine_book(ARG_VOID);
static void activate(ARG_VOID);
static void go_up(ARG_VOID);
static void go_down(ARG_VOID);
static void jamdoor(ARG_VOID);
static void refill_lamp(ARG_VOID);
static int regen_monsters(ARG_VOID);
int kbhit(ARG_VOID);

extern int8u peek;
extern int rating;

#ifdef MSDOS
int8u good_item_flag=FALSE;
int8u create_up_stair=FALSE;
int8u create_down_stair=FALSE;
#else
int good_item_flag=FALSE;
int create_up_stair=FALSE;
int create_down_stair=FALSE;
#endif


/* Lights up given location				-RAK-	*/
void lite_spot(y, x)
register int y, x;
{
  if (panel_contains(y, x))
    print(loc_symbol(y, x), y, x);
#ifdef TC_COLOR
  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif
}

/* Returns symbol for given row, column			-RAK-	*/
unsigned char loc_symbol(y, x)
int y, x;
{
  register cave_type *cave_ptr;
  register struct flags *f_ptr;

#ifdef TC_COLOR
  if (!no_color_flag) textcolor(LIGHTGRAY);
#endif

  cave_ptr = &cave[y][x];
  f_ptr = &py.flags;

  if ((cave_ptr->cptr == 1) && (!find_flag || find_prself))
    return '@';
  else if (f_ptr->status & PY_BLIND)
    return ' ';
  else if ((f_ptr->image > 0) && (randint (12) == 1))
#ifdef TC_COLOR
    { if (!no_color_flag) textcolor( randint(15) );
      return randint (95) + 31; }
#else
    return randint (95) + 31;
#endif
  else if ((cave_ptr->cptr > 1) && (m_list[cave_ptr->cptr].ml))
#ifdef TC_COLOR
    {
      if (!no_color_flag){
	textcolor(mon_color(m_list[cave_ptr->cptr].color));
      }
    return c_list[m_list[cave_ptr->cptr].mptr].cchar;
    }
#else
    return c_list[m_list[cave_ptr->cptr].mptr].cchar;
#endif
  else if (!cave_ptr->pl && !cave_ptr->tl && !cave_ptr->fm)
    return ' ';
  else if ((cave_ptr->tptr != 0)
	   && (t_list[cave_ptr->tptr].tval != TV_INVIS_TRAP))
#ifdef TC_COLOR
    {  if (t_list[cave_ptr->tptr].color != 0) {
         if (!no_color_flag) textcolor(t_list[cave_ptr->tptr].color);
       }
       else switch(t_list[cave_ptr->tptr].tval){
       	 case TV_POTION1:
       	 case TV_POTION2:
	   if (!no_color_flag) textcolor(tccolors[t_list[cave_ptr->tptr].subval
					- ITEM_SINGLE_STACK_MIN]);
	   break;
	 case TV_WAND:
	 case TV_ROD:
	   if (!no_color_flag) textcolor(tcmetals[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_STAFF:
	   if (!no_color_flag) textcolor(tcwoods[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_RING:
	   if (!no_color_flag) textcolor(tcrocks[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_AMULET:
	   if (!no_color_flag) textcolor(tcamulets[t_list[cave_ptr->tptr].subval]);
	   break;
	 case TV_FOOD:
	   if (!no_color_flag) textcolor(tcmushrooms[t_list[cave_ptr->tptr].subval
	   				- ITEM_SINGLE_STACK_MIN]);
	   break;
	 default: if (!no_color_flag) textcolor( randint(15) ); /* should never happen. -CFT */
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
        if (!no_color_flag) textcolor(DARKGRAY);
        else return '%';
      else
        if (!no_color_flag) textcolor(WHITE);
        else return '%';
      return wallsym;
#else
      return '%';
#endif
    }
}

#ifdef MSDOS
int mon_color(int c){
  if ((c >= 1) && (c <= 15))
    return c;
  if (c == ANY)
    return randint(15);
  if (c == MULTI)
    switch(randint(10)){
      case 1: case 2: return DARKGRAY; /* black d's all DARKGRAY */
      case 3: return LIGHTBLUE;
      case 4: return BLUE;
      case 5: return LIGHTGREEN;
      case 6: return GREEN;
      case 7: return LIGHTRED;
      case 8: return RED;
      case 9: case 10: return WHITE; /* white d's all WHITE */
      default: return randint(15); /* should never happen, but... */
    }
  return randint(15); /* if unknown color, make random -CFT */  
}
#endif

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


/* Generates a random integer x where 1<=X<=MAXVAL	-RAK-	*/
int randint(maxval)
int maxval;
{
  register int32u randval;

  if (maxval<1) return 1;
  randval = bsd_random();
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

/* Gives Max hit points					-RAK-	*/
int max_hp(array)
int8u *array;
{
  return((int)(array[0]) * (int)(array[1]));
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


/* ANGBAND game module					-RAK-	*/
/* The code in this section has gone through many revisions, and */
/* some of it could stand some more hard work.	-RAK-	       */

/* It has had a bit more hard work.			-CJS- */

void dungeon()
{
  int find_count, i;
  int regen_amount;	    /* Regenerate hp and mana*/
  char command;		/* Last command		 */
  int8u unfelt = TRUE; /* did we get a feeling yet? */
  register struct misc *p_ptr;
  register inven_type *i_ptr;
  register struct flags *f_ptr;

  /* Main procedure for dungeon.			-RAK-	*/
  /* Note: There is a lot of preliminary magic going on here at first*/

  /* init pointers. */
  f_ptr = &py.flags;
  p_ptr = &py.misc;

  /* Check light status for setup	   */
  i_ptr = &inventory[INVEN_LIGHT];
  if (i_ptr->p1 > 0 || f_ptr->light)
    player_light = TRUE;
  else
    player_light = FALSE;
  /* Check for a maximum level		   */
  /* Added check to avoid -50' being "deepest", since max_dlv unsigned -CFT */
  if ((dun_level >= 0) && (dun_level > p_ptr->max_dlv)) 
    p_ptr->max_dlv = dun_level;

  /* Reset flags and initialize variables  */
  command_count = 0;
  find_count = 0;
  new_level_flag    = FALSE;
  find_flag	= FALSE;
  teleport_flag = FALSE;
  mon_tot_mult	= 0;
  target_mode = FALSE;	/* target code taken from Morgul -CFT */
  cave[char_row][char_col].cptr = 1;

  if (create_up_stair && (dun_level == 0)) /* just in case... */
    create_up_stair = FALSE;
  if (create_up_stair || create_down_stair) {
    register cave_type *c_ptr;
    register int cur_pos;

    c_ptr = &cave[char_row][char_col];
    if ((c_ptr->tptr == 0) ||
	 ((t_list[c_ptr->tptr].tval != TV_STORE_DOOR) && /* if not store */
           ((t_list[c_ptr->tptr].tval < TV_MIN_WEAR) || /* if no artifact here -CFT */
      	    (t_list[c_ptr->tptr].tval > TV_MIN_WEAR) ||
            !(t_list[c_ptr->tptr].flags2 & TR_ARTIFACT)))) { 
      if (c_ptr->tptr != 0)
        (void) delete_object(char_row, char_col);
      cur_pos = popt();
      c_ptr->tptr = cur_pos;
      if (create_up_stair)
        invcopy(&t_list[cur_pos], OBJ_UP_STAIR);
      else if (create_down_stair && !is_quest(dun_level))
        invcopy(&t_list[cur_pos], OBJ_DOWN_STAIR);
      }
      else
        msg_print("The object resists your attempt to transform it into a stairway.");
    create_down_stair = FALSE;
    create_up_stair = FALSE;
  }
  /* Ensure we display the panel. Used to do this with a global var. -CJS- */
  panel_row = panel_col = -1;
  /* Light up the area around character	   */
  check_view ();
  /* must do this after panel_row/col set to -1, because search_off() will call
     check_view(), and so the panel_* variables must be valid before
     search_off() is called */
  if (search_flag)
    search_off();
  /* Light,  but do not move critters	    */
  creatures(FALSE);
  /* Print the depth			   */
  prt_depth();

#if 1  /* my temporary fix... (see below) -CFT */
  if (dun_level && ((rating>=0) || good_item_flag)) {
    msg_print(NULL);  /* make sure the mesg doesn't get lost */
#ifdef TC_COLOR
    if (!no_color_flag) textcolor(YELLOW); /* make easy to see */
#endif	  
    if (good_item_flag)
      msg_print("You feel there is something special about this level.");
    else if (rating>100)
      msg_print("You have a superb feeling about this level.");
    else if (rating>80)
      msg_print("You have an excellent feeling that your luck is turning...");
    else if (rating>60)
      msg_print("You have a very good feeling.");
    else if (rating>40)
      msg_print("You have a good feeling.");
    else if (rating>30)
      msg_print("You feel strangely lucky.");
    else if (rating>20)
      msg_print("You feel your luck is turning...");
    else if (rating>10)
      msg_print("You like the look of this place.");
    else if (rating>0)
      msg_print("This level feels pretty normal.");
    else msg_print("You look around and feel somewhat bored.");
#ifdef TC_COLOR
    if (!no_color_flag) textcolor(LIGHTGRAY); /* restore color */
#endif	  
    msg_print(NULL);  /* make sure the mesg doesn't get lost */
    }
#else /* this was here, but only applies with the below code -CFT */
  unfelt=TRUE;
  if ((good_item_flag || (rating >100)))
    feel_chance = 26-py.misc.lev/2;
  else
    feel_chance = (400-rating)/4;
#endif
  
  /* Loop until dead,  or new level		*/
  do
    {
      /* Increment turn counter			*/
      turn++;

#if 0  /* At the moment, this code has flaws in it.  So I moved it
	  back outside the main loop, and make it always appear
	  when you enter a new level.  This hopefully is only
	  temporary. -CFT */
	  
      /* Move the feeling code into the main do loop. Higher
         values of feel_chance give worse odds */
      if ((unfelt) && (!free_turn_flag) && (dun_level) &&
          (((good_item_flag) && (randint(feel_chance)<3)) ||
           (randint(feel_chance)==1)))
        {
          unfelt = FALSE;

	  msg_print(NULL);  /* make sure the mesg doesn't get lost */
#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(YELLOW); /* make easy to see */
#endif	  
	  if (good_item_flag)
	    msg_print("You feel there is something special about this level.");
	  else if (rating>100)
	    msg_print("You have a superb feeling about this level.");
	  else if (rating>80)
	    msg_print("You have an excellent feeling that your luck is turning...");
	  else if (rating>60)
	    msg_print("You have a very good feeling.");
	  else if (rating>40)
	    msg_print("You have a good feeling.");
	  else if (rating>30)
	    msg_print("You feel strangely lucky.");
	  else if (rating>20)
	    msg_print("You feel your luck is turning...");
	  else if (rating>10)
	    msg_print("You like the look of this place.");
          else if (rating>0)
            msg_print("This level feels pretty normal.");
          else msg_print("You look around and feel somewhat bored.");

#ifdef TC_COLOR
	  if (!no_color_flag) textcolor(LIGHTGRAY); /* restore color */
#endif	  
	  msg_print(NULL);  /* make sure the mesg doesn't get lost */
	  }
	if (unfelt) feel_chance = 1000; /* 1 in 1000 */
#endif /* -CFT */
	
#ifndef MAC
      /* The Mac ignores the game hours file		*/
      /* Check for game hours			       */
      if (((turn % 100) == 1) && !check_time())
	if (closing_flag > 2)
	  {
	    msg_print("The gates to ANGBAND are now closed.");
	    (void) strcpy (died_from, "(closing gate: saved)");
	    if (!save_char())
	      {
		(void) strcpy (died_from, "a slammed gate");
		death = TRUE;
	      }
	    exit_game();
	  }
	else
	  {
	    disturb (0, 0);
	    closing_flag++;
	    msg_print("The gates to ANGBAND are closing due to high load.");
	    msg_print("Please finish up or save your game.");
	  }
#endif

      /* turn over the store contents every, say, 1000 turns */
      if ((dun_level != 0) && ((turn % 1000) == 0)) {
	if (peek) msg_print("Store update: ");
	store_maint();
	if (peek) msg_print("Complete ");
      }

      /* Check for creature generation		*/
      if (randint(MAX_MALLOC_CHANCE) == 1)
	alloc_monster(1, MAX_SIGHT, FALSE);
      if (!(turn % 20))
	regen_monsters();
      /* Check light status			       */
      i_ptr = &inventory[INVEN_LIGHT];
      if (player_light)
        if (i_ptr->p1 > 0)
	  {
	    if (!(i_ptr->flags2 & TR_LIGHT)) i_ptr->p1--; /* don't dec if perm light -CFT */
	    if (i_ptr->p1 == 0)
	      {
		player_light = FALSE;
		disturb (0, 1);
		/* unlight creatures */
		creatures(FALSE);
		msg_print("Your light has gone out!");
	      }
	    else if ((i_ptr->p1 < 40) && (randint(5) == 1) &&
		     (py.flags.blind < 1) &&
		     !(i_ptr->flags2 & TR_LIGHT)) /* perm light doesn't dim -CFT */
	      {
		disturb (0, 0);
		msg_print("Your light is growing faint.");
	      }
	  }
	else
	  {
	    if (!f_ptr->light) {
	      player_light = FALSE;
	      disturb (0, 1);
	      /* unlight creatures */
	      creatures(FALSE);
	    }
	  }
      else if (i_ptr->p1 > 0 || f_ptr->light)
	{
	  if (!(i_ptr->flags2 & TR_LIGHT)) i_ptr->p1--; /* don't dec if perm light -CFT */
	  player_light = TRUE;
	  disturb (0, 1);
	  /* light creatures */
	  creatures(FALSE);
	}

      /* Update counters and messages			*/
      /* Check food status	       */
      regen_amount = PLAYER_REGEN_NORMAL;
      if (f_ptr->food < PLAYER_FOOD_ALERT)
	{
	  if (f_ptr->food < PLAYER_FOOD_WEAK)
	    {
	      if (f_ptr->food < 0)
		regen_amount = 0;
	      else if (f_ptr->food < PLAYER_FOOD_FAINT)
		regen_amount = PLAYER_REGEN_FAINT;
	      else if (f_ptr->food < PLAYER_FOOD_WEAK)
		regen_amount = PLAYER_REGEN_WEAK;
	      if ((PY_WEAK & f_ptr->status) == 0)
		{
		  f_ptr->status |= PY_WEAK;
		  msg_print("You are getting weak from hunger.");
		  disturb (0, 0);
		  prt_hunger();
		}
	      if ((f_ptr->food < PLAYER_FOOD_FAINT) && (randint(8) == 1))
		{
		  f_ptr->paralysis += randint(5);
		  msg_print("You faint from the lack of food.");
		  disturb (1, 0);
		}
	    }
	  else if ((PY_HUNGRY & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_HUNGRY;
	      msg_print("You are getting hungry.");
	      disturb (0, 0);
	      prt_hunger();
	    }
	}
      /* Food consumption	*/
      /* Note: Speeded up characters really burn up the food!  */
      if (f_ptr->speed < 0) /* now summation, not square, since spd less powerful -CFT */
	f_ptr->food -=	(f_ptr->speed*f_ptr->speed - f_ptr->speed)/2;
      f_ptr->food -= f_ptr->food_digested;
      if (f_ptr->food < 0)
	{
	  take_hit (-f_ptr->food/16, "starvation");   /* -CJS- */
	  disturb(1, 0);
	}
      /* Regenerate	       */
      if (f_ptr->regenerate)  regen_amount = regen_amount * 3 / 2;
      if (search_flag || f_ptr->rest > 0 || f_ptr->rest==-1 || f_ptr->rest==-2)
	regen_amount = regen_amount * 2;
      if ((py.flags.poisoned < 1) && (py.flags.cut < 1) &&
	  (p_ptr->chp < p_ptr->mhp))
	regenhp(regen_amount);
      if (p_ptr->cmana < p_ptr->mana)
	regenmana(regen_amount);
      /* Blindness	       */
      if (f_ptr->blind > 0)
	{
	  if ((PY_BLIND & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_BLIND;
	      prt_map();
	      prt_blind();
	      disturb (0, 1);
	      /* unlight creatures */
	      creatures (FALSE);
	    }
	  f_ptr->blind--;
	  if (f_ptr->blind == 0)
	    {
	      f_ptr->status &= ~PY_BLIND;
	      prt_blind();
	      prt_map();
	      /* light creatures */
	      disturb (0, 1);
	      creatures(FALSE);
	      msg_print("The veil of darkness lifts.");
	    }
	}
      /* Confusion	       */
      if (f_ptr->confused > 0)
	{
	  if ((PY_CONFUSED & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_CONFUSED;
	      prt_confused();
	    }
	  f_ptr->confused--;
	  if (f_ptr->confused == 0)
	    {
	      f_ptr->status &= ~PY_CONFUSED;
	      prt_confused();
	      msg_print("You feel less confused now.");
	      if (py.flags.rest > 0 || py.flags.rest == -1)
		rest_off ();
	    }
	}
      /* Afraid		       */
      if (f_ptr->afraid > 0)
	{
	  if ((PY_FEAR & f_ptr->status) == 0)
	    {
	      if ((f_ptr->shero+f_ptr->hero) > 0)
		f_ptr->afraid = 0;
	      else
		{
		  f_ptr->status |= PY_FEAR;
		  prt_afraid();
		}
	    }
	  else if ((f_ptr->shero+f_ptr->hero) > 0)
	    f_ptr->afraid = 1;
	  f_ptr->afraid--;
	  if (f_ptr->afraid == 0)
	    {
	      f_ptr->status &= ~PY_FEAR;
	      prt_afraid();
	      msg_print("You feel bolder now.");
	      disturb (0, 0);
	    }
	}

      /* Cut */
      if (f_ptr->cut > 0) {
	if (f_ptr->cut>1000) {
	  take_hit(3 , "a fatal wound");
	  disturb(1,0);
	} else if (f_ptr->cut>200) {
	  take_hit(3, "a fatal wound");
	  f_ptr->cut-=(con_adj()<0?1:con_adj())+1;
	  disturb(1,0);
	} else if (f_ptr->cut>100) {
	  take_hit(2, "a fatal wound");
	  f_ptr->cut-=(con_adj()<0?1:con_adj())+1;
	  disturb(1,0);
	} else {
	  take_hit(1, "a fatal wound");
	  f_ptr->cut-=(con_adj()<0?1:con_adj())+1;
	  disturb(1,0);
	}
	prt_cut();
	if (f_ptr->cut<=0) {
	  f_ptr->cut=0;
	  if (py.misc.chp>=0)
	    msg_print("Your wound heals.");
	}
      }
      prt_cut();

      /* Stun */
      if (f_ptr->stun > 0) {
        int oldstun = f_ptr->stun;

	f_ptr->stun -= (con_adj()<=0 ? 1 : (con_adj()/2+1)); /* fixes endless 
							stun if bad con. -CFT */
        if ((oldstun > 50) && (f_ptr->stun <= 50)){ /* if crossed 50 mark... */
	  p_ptr->ptohit +=15;
	  p_ptr->ptodam +=15;
	  p_ptr->dis_th +=15;
	  p_ptr->dis_td +=15;
	  }        	
	if (f_ptr->stun<=0) {
	  f_ptr->stun=0;
	  msg_print("Your head stops stinging.");
	  p_ptr->ptohit +=5;
	  p_ptr->ptodam +=5;
	  p_ptr->dis_th +=5;
	  p_ptr->dis_td +=5;
	}
      }
      prt_stun();

      /* Poisoned	       */
      if (f_ptr->poisoned > 0)
	{
	  if ((PY_POISONED & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_POISONED;
	      prt_poisoned();
	    }
	  f_ptr->poisoned--;
	  if (f_ptr->poisoned == 0 || f_ptr->poison_im ||
	      f_ptr->poison_resist || f_ptr->resist_poison)
	    {
	      f_ptr->poisoned = 0;
	      f_ptr->status &= ~PY_POISONED;
	      prt_poisoned();
	      msg_print("You feel better.");
	      disturb (0, 0);
	    }
	  else
	    {
	      switch(con_adj())
		{
		case -4:  i = 4;  break;
		case -3:
		case -2:  i = 3;  break;
		case -1:  i = 2;  break;
		case 0:	  i = 1;  break;
		case 1: case 2: case 3:
		  i = ((turn % 2) == 0);
		  break;
		case 4: case 5:
		  i = ((turn % 3) == 0);
		  break;
		case 6:
		  i = ((turn % 4) == 0);
		  break;
		case 7:
		  i = ((turn % 5) == 0);
		  break;
		default:
		  i = ((turn % 6) == 0);
		  break;
		}
	      take_hit (i, "poison");
	      disturb (1, 0);
	    }
	}
      /* Fast		       */
      if (f_ptr->fast > 0)
	{
	  if ((PY_FAST & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_FAST;
	      change_speed(-1);
	      msg_print("You feel yourself moving faster.");
	      disturb (0, 0);
	    }
	  f_ptr->fast--;
	  if (f_ptr->fast == 0)
	    {
	      f_ptr->status &= ~PY_FAST;
	      change_speed(1);
	      msg_print("You feel yourself slow down.");
	      disturb (0, 0);
	    }
	}
      /* Slow		       */
      if (f_ptr->slow > 0)
	{
	  if ((PY_SLOW & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_SLOW;
	      change_speed(1);
	      msg_print("You feel yourself moving slower.");
	      disturb (0, 0);
	    }
	  f_ptr->slow--;
	  if (f_ptr->slow == 0)
	    {
	      f_ptr->status &= ~PY_SLOW;
	      change_speed(-1);
	      msg_print("You feel yourself speed up.");
	      disturb (0, 0);
	    }
	}
      /* Resting is over?      */
      if (f_ptr->rest>0 || f_ptr->rest==-1 || f_ptr->rest==-2) {
	if (f_ptr->rest>0) {
	  f_ptr->rest--;
	  if (f_ptr->rest == 0)  /* Resting over */
	    rest_off();
	} else if (f_ptr->rest == -1) {
	  if (py.misc.chp==py.misc.mhp && py.misc.cmana==py.misc.mana) {
	    f_ptr->rest=0;
	    rest_off();
	  }
	} else if (f_ptr->rest == -2) { /* rest until blind/conf/stun/
					HP/mana/fear/halluc over */
	  if ((py.flags.blind < 1) && (py.flags.confused < 1) &&
	      (py.flags.afraid < 1) && (py.flags.stun < 1) &&
	      (py.flags.image < 1) && (py.flags.word_recall < 1) &&
	      (py.flags.slow < 1) && (py.misc.chp == py.misc.mhp) &&
	      (py.misc.cmana == py.misc.mana)) {
	    f_ptr->rest=0;
	    rest_off();
	    }
	}		
      }

      /* Check for interrupts to find or rest. */
      if ((command_count > 0 || find_flag || f_ptr->rest>0 || f_ptr->rest==-1
		|| f_ptr->rest == -2)
#if defined(MSDOS) || defined(VMS)  /* stolen from Um55 src -CFT */
	  && kbhit()
#else
	  && (check_input (find_flag ? 0 : 10000))
#endif
	  )
	{
#ifdef MSDOS
	  (void) msdos_getch();
#endif
#ifdef VMS
	  /* Get and ignore the key used to interrupt resting/running.  */
	  (void) vms_getch ();
#endif
	  disturb (0, 0);
	}

      /* Hallucinating?	 (Random characters appear!)*/
      if (f_ptr->image > 0)
	{
	  end_find ();
	  f_ptr->image--;
	  if (f_ptr->image == 0)
	    prt_map ();	 /* Used to draw entire screen! -CJS- */
	}
      /* Paralysis	       */
      if (f_ptr->paralysis > 0)
	{
	  /* when paralysis true, you can not see any movement that occurs */
	  f_ptr->paralysis--;
	  disturb (1, 0);
	}
      /* Protection from evil counter*/
      if (f_ptr->protevil > 0)
	{
	  f_ptr->protevil--;
	  if (f_ptr->protevil == 0)
	    msg_print ("You no longer feel safe from evil.");
	}
      /* Invulnerability	*/
      if (f_ptr->invuln > 0)
	{
	  if ((PY_INVULN & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_INVULN;
	      disturb (0, 0);
	      py.misc.ptoac += 100; /* changed to ptoac -CFT */
	      py.misc.dis_tac += 100;
	      msg_print("Your skin turns into steel!");
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	  f_ptr->invuln--;
	  if (f_ptr->invuln == 0)
	    {
	      f_ptr->status &= ~PY_INVULN;
	      disturb (0, 0);
	      py.misc.ptoac -= 100; /* changed to ptoac -CFT */
	      py.misc.dis_tac -= 100;
	      msg_print("Your skin returns to normal.");
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	}
      /* Heroism       */
      if (f_ptr->hero > 0)
	{
	  if ((PY_HERO & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_HERO;
	      disturb (0, 0);
	      p_ptr->mhp += 10;
	      p_ptr->chp += 10;
	      p_ptr->ptohit += 12;
	      p_ptr->dis_th += 12;
	      msg_print("You feel like a HERO!");
	      prt_mhp();
	      prt_chp();
	    }
	  f_ptr->hero--;
	  if (f_ptr->hero == 0)
	    {
	      f_ptr->status &= ~PY_HERO;
	      disturb (0, 0);
	      p_ptr->mhp -= 10;
	      if (p_ptr->chp > p_ptr->mhp)
		{
		  p_ptr->chp = p_ptr->mhp;
		  p_ptr->chp_frac = 0;
		  prt_chp();
		}
	      p_ptr->ptohit -= 12;
	      p_ptr->dis_th -= 12;
	      msg_print("The heroism wears off.");
	      prt_mhp();
	    }
	}
      /* Super Heroism */
      if (f_ptr->shero > 0)
	{
	  if ((PY_SHERO & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_SHERO;
	      disturb (0, 0);
	      p_ptr->mhp += 30;
	      p_ptr->chp += 30;
	      p_ptr->ptohit += 24;
	      p_ptr->dis_th += 24;
	      p_ptr->ptoac -= 10;
	      p_ptr->dis_tac -= 10;
	      msg_print("You feel like a killing machine!");
	      prt_mhp();
	      prt_chp();
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	  f_ptr->shero--;
	  if (f_ptr->shero == 0)
	    {
	      f_ptr->status &= ~PY_SHERO;
	      disturb (0, 0);
	      p_ptr->mhp -= 30;
	      p_ptr->ptoac += 10;
	      p_ptr->dis_tac += 10;
	      if (p_ptr->chp > p_ptr->mhp)
		{
		  p_ptr->chp = p_ptr->mhp;
		  p_ptr->chp_frac = 0;
		  prt_chp();
		}
	      p_ptr->ptohit -= 24;
	      p_ptr->dis_th -= 24;
	      msg_print("You feel less Berserk.");
	      prt_mhp();
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	}
      /* Blessed       */
      if (f_ptr->blessed > 0)
	{
	  if ((PY_BLESSED & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_BLESSED;
	      disturb (0, 0);
	      p_ptr->ptohit += 10;
	      p_ptr->dis_th += 10;
	      p_ptr->ptoac += 5; /* changed to ptoac -CFT */
	      p_ptr->dis_tac+= 5;
	      msg_print("You feel righteous!");
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	  f_ptr->blessed--;
	  if (f_ptr->blessed == 0)
	    {
	      f_ptr->status &= ~PY_BLESSED;
	      disturb (0, 0);
	      p_ptr->ptohit -= 10;
	      p_ptr->dis_th -= 10;
	      p_ptr->ptoac -= 5; /* changed to ptoac -CFT */
	      p_ptr->dis_tac -= 5;
	      msg_print("The prayer has expired.");
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	}
      /* Shield       */
      if (f_ptr->shield > 0)
	{
	  f_ptr->shield--;
	  if (f_ptr->shield == 0)
	    {
	      disturb (0, 0);
	      msg_print("Your mystic shield crumbles away.");
	      py.misc.ptoac -= 50; /* changed to ptoac -CFT */
	      py.misc.dis_tac -= 50;
	      f_ptr->status |= PY_ARMOR; /* have to update ac display */
	    }
	}
      /* Resist Heat   */
      if (f_ptr->resist_heat > 0)
	{
	  f_ptr->resist_heat--;
	  if (f_ptr->resist_heat == 0)
	    msg_print ("You no longer feel safe from flame.");
	}
      /* Resist Cold   */
      if (f_ptr->resist_cold > 0)
	{
	  f_ptr->resist_cold--;
	  if (f_ptr->resist_cold == 0)
	    msg_print ("You no longer feel safe from cold.");
	}
      /* Resist Acid   */
      if (f_ptr->resist_acid > 0)
	{
	  f_ptr->resist_acid--;
	  if (f_ptr->resist_acid == 0)
	    msg_print ("You no longer feel safe from acid.");
	}
      /* Resist Lightning   */
      if (f_ptr->resist_light > 0)
	{
	  f_ptr->resist_light--;
	  if (f_ptr->resist_light == 0)
	    msg_print ("You no longer feel safe from lightning.");
	}
      /* Resist Poison   */
      if (f_ptr->resist_poison > 0)
	{
	  f_ptr->resist_poison--;
	  if (f_ptr->resist_poison == 0)
	    msg_print ("You no longer feel safe from poison.");
	}

      /* Timeout Artifacts  */
      for (i = 22; i < (INVEN_ARRAY_SIZE-1); i++) {
	i_ptr = &inventory[i];
	if (i_ptr->tval != TV_NOTHING && (i_ptr->flags2 & TR_ACTIVATE)) {
	  if (i_ptr->timeout>0)
	    i_ptr->timeout--;
	  if ((i_ptr->tval==TV_RING) &&
	      (!stricmp(object_list[i_ptr->index].name,"Power")) &&
	      (randint(20)==1) && (py.misc.exp > 0))
	    py.misc.exp--, py.misc.max_exp--, prt_experience();
	}
      }

      /* Timeout rods  */
      for (i = 0; i < 22; i++) {
	i_ptr = &inventory[i];
	if (i_ptr->tval == TV_ROD && (i_ptr->flags2 & TR_ACTIVATE)) {
	  if (i_ptr->timeout>0)
	    i_ptr->timeout--;
	}
      }

      /* Detect Invisible      */
      if (f_ptr->detect_inv > 0)
	{
	  if ((PY_DET_INV & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_DET_INV;
	      f_ptr->see_inv = TRUE;
	      /* light but don't move creatures */
	      creatures (FALSE);
	    }
	  f_ptr->detect_inv--;
	  if (f_ptr->detect_inv == 0)
	    {
	      f_ptr->status &= ~PY_DET_INV;
	      /* may still be able to see_inv if wearing magic item */
	      if (py.misc.prace == 9)
	        f_ptr->see_inv = TRUE;
	      else {
	      	f_ptr->see_inv = FALSE; /* unless item grants it */
		for (i = INVEN_WIELD; i <= INVEN_LIGHT; i++)
		  if (TR_SEE_INVIS & inventory[i].flags)
		    f_ptr->see_inv = TRUE;
	      }
	      /* unlight but don't move creatures */
	      creatures (FALSE);
	    }
	}
      /* Timed infra-vision    */
      if (f_ptr->tim_infra > 0)
	{
	  if ((PY_TIM_INFRA & f_ptr->status) == 0)
	    {
	      f_ptr->status |= PY_TIM_INFRA;
	      f_ptr->see_infra++;
	      /* light but don't move creatures */
	      creatures (FALSE);
	    }
	  f_ptr->tim_infra--;
	  if (f_ptr->tim_infra == 0)
	    {
	      f_ptr->status &= ~PY_TIM_INFRA;
	      f_ptr->see_infra--;
	      /* unlight but don't move creatures */
	      creatures (FALSE);
	    }
	}
	/* Word-of-Recall  Note: Word-of-Recall is a delayed action	 */
      if (f_ptr->word_recall > 0)
	if (f_ptr->word_recall == 1)
	  {
	    new_level_flag = TRUE;
	    f_ptr->paralysis++;
	    f_ptr->word_recall = 0;
	    if (dun_level > 0)
	      {
		dun_level = 0;
		msg_print("You feel yourself yanked upwards!");
	      }
	    else if (py.misc.max_dlv != 0)
	      {
		dun_level = py.misc.max_dlv;
		msg_print("You feel yourself yanked downwards!");
	      }
	  }
	else
	  f_ptr->word_recall--;

      /* Random teleportation  */
      if ((py.flags.teleport) && (randint(100) == 1))
	{
	  disturb (0, 0);
	  teleport(40);
	}

      /* See if we are too weak to handle the weapon or pack.  -CJS- */
      if (py.flags.status & PY_STR_WGT)
	check_strength();
      if (py.flags.status & PY_STUDY)
	prt_study();
      if (py.flags.status & PY_SPEED)
	{
	  py.flags.status &= ~PY_SPEED;
	  prt_speed();
	}
      if ((py.flags.status & PY_PARALYSED) && (py.flags.paralysis < 1))
	{
	  prt_state();
	  py.flags.status &= ~PY_PARALYSED;
	}
      else if (py.flags.paralysis > 0)
	{
	  prt_state();
	  py.flags.status |= PY_PARALYSED;
	}
      else if (py.flags.rest > 0 || py.flags.rest==-1 || py.flags.rest==-2)
	prt_state();

      if ((py.flags.status & PY_ARMOR) != 0)
	{
	  py.misc.dis_ac = py.misc.pac + py.misc.dis_tac; /* use updated ac */
	  prt_pac();
	  py.flags.status &= ~PY_ARMOR;
	}
      if ((py.flags.status & PY_STATS) != 0)
	{
	  for (i = 0; i < 6; i++)
	    if ((PY_STR << i) & py.flags.status)
	      prt_stat(i);
	  py.flags.status &= ~PY_STATS;
	}
      if (py.flags.status & PY_HP)
	{
	  prt_mhp();
	  prt_chp();
	  py.flags.status &= ~PY_HP;
	}
      if (py.flags.status & PY_MANA)
	{
	  prt_cmana();
	  py.flags.status &= ~PY_MANA;
	}

      /* Allow for a slim chance of detect enchantment -CJS- */
      /* for 1st level char, check once every 2160 turns
	 for 40th level char, check once every 416 turns */
      if (py.misc.pclass==2?
	  ((f_ptr->confused == 0) && (py.misc.pclass!=0) &&
	   (randint((int)(10000/(py.misc.lev*py.misc.lev+40)) + 1) == 1))
	  :
	  (((turn & 0xF) == 0) && (f_ptr->confused == 0)
	  && (randint((int)(10 + 750 / (5 + py.misc.lev))) == 1))
	  )
	{
	  vtype tmp_str;

	  for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	    if (i == inven_ctr)
	      i = 22;
	    i_ptr = &inventory[i];
	    /* if in inventory, succeed 1 out of 50 times,
	    if in equipment list, success 1 out of 10 times,
	    unless your a priest or rogue... */
	    if (((i_ptr->tval >= TV_MIN_WEAR) && (i_ptr->tval <= TV_MAX_WEAR))
		&& special_check(i_ptr) &&
		((py.misc.pclass==2 || py.misc.pclass==3)?
		 (randint(i < 22 ? 5 : 1) == 1)
		 :
		 (randint(i < 22 ? 50 : 10) == 1)))
	      {
		extern char *describe_use();

		if (py.misc.pclass==0||py.misc.pclass==3||py.misc.pclass==5)
		  if ((i_ptr->tval==TV_SWORD) ||
		      (i_ptr->tval==TV_HAFTED) ||
		      (i_ptr->tval==TV_POLEARM) ||
		      (i_ptr->tval==TV_BOW) ||
		      (i_ptr->tval==TV_BOLT) ||
		      (i_ptr->tval==TV_ARROW) ||
		      (i_ptr->tval==TV_DIGGING) ||
		      (i_ptr->tval==TV_SLING_AMMO) ||
		      (i_ptr->tval==TV_SOFT_ARMOR) ||
		      (i_ptr->tval==TV_HARD_ARMOR) ||
		      (i_ptr->tval==TV_HELM) ||
		      (i_ptr->tval==TV_BOOTS) ||
		      (i_ptr->tval==TV_CLOAK) ||
		      (i_ptr->tval==TV_GLOVES) ||
		      (i_ptr->tval==TV_SHIELD))
		    continue;
		(void) sprintf(tmp_str,
			       "There's something %s about what you are %s...",
			       special_check(i_ptr)>0?"good":"bad",
			       describe_use(i));
		disturb(0, 0);
		msg_print(tmp_str);
		add_inscribe(i_ptr, (special_check(i_ptr)>0)?
			   ID_MAGIK:ID_DAMD);
	      }
	  }
	}

      /* Warriors, Rogues and paladins inbuilt ident */
      if (((py.misc.pclass==0) && (f_ptr->confused == 0) &&
	  (randint((int)(9000/(py.misc.lev*py.misc.lev+40)) + 1) == 1))
	  ||
	  ((py.misc.pclass==3) && (f_ptr->confused == 0) &&
	  (randint((int)(20000/(py.misc.lev*py.misc.lev+40)) + 1) == 1))
	  ||
	  ((py.misc.pclass==5) && (f_ptr->confused == 0) &&
	  (randint((int)(80000L/(py.misc.lev*py.misc.lev+40)) + 1) == 1))) {
	  vtype tmp_str;
	  char *value_check();

	  for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	    if (i == inven_ctr)
	      i = 22;
	    i_ptr = &inventory[i];
	    /* if in inventory, succeed 1 out of 5 times,
	    if in equipment list, always succeed! */
	    if (((i_ptr->tval==TV_SOFT_ARMOR) ||
		 (i_ptr->tval==TV_HARD_ARMOR) ||
		 (i_ptr->tval==TV_SWORD) ||
		 (i_ptr->tval==TV_HAFTED) ||
		 (i_ptr->tval==TV_POLEARM) ||
		 (i_ptr->tval==TV_SHIELD) ||
		 (i_ptr->tval==TV_HELM) ||
		 (i_ptr->tval==TV_BOOTS) ||
		 (i_ptr->tval==TV_GLOVES) ||
		 (i_ptr->tval==TV_DIGGING) ||
		 (i_ptr->tval==TV_SLING_AMMO) ||
		 (i_ptr->tval==TV_BOLT) ||
		 (i_ptr->tval==TV_ARROW) ||
		 (i_ptr->tval==TV_BOW) ||
		 (i_ptr->tval==TV_CLOAK))
		 && value_check(i_ptr) &&
		(randint(i < 22 ? 5 : 1) == 1))
	      {
		extern char *describe_use();
		char out_val[100], tmp[100], *ptr;
		int j;

		(void) strcpy(tmp, object_list[i_ptr->index].name);

		ptr = tmp;
		j=0;
		while (tmp[j]==' ' || tmp[j]=='&')
		  ptr=&tmp[++j];

		(void) strcpy(out_val, ptr);

		ptr=out_val;

		while (*ptr) {
		  if (*ptr == '~') *ptr = 's';
		  ptr++;
		}

		(void) sprintf(tmp_str,
			       "You feel the %s (%c) you are %s %s %s...",
			       out_val,
			       ((i<INVEN_WIELD)?i+'a':(i+'a'-INVEN_WIELD)),
			       describe_use(i),
			       ((i_ptr->tval==TV_BOLT) ||
				(i_ptr->tval==TV_ARROW) ||
				(i_ptr->tval==TV_SLING_AMMO) ||
				(i_ptr->tval==TV_BOOTS) ||
				(i_ptr->tval==TV_GLOVES))?"are":"is",
			       value_check(i_ptr));
		disturb(0, 0);
		msg_print(tmp_str);
		if (!stricmp(value_check(i_ptr), "terrible"))
		  add_inscribe(i_ptr, ID_DAMD);
		else if (!stricmp(value_check(i_ptr), "worthless"))
		  add_inscribe(i_ptr, ID_DAMD);
		else
		  inscribe(i_ptr, value_check(i_ptr));
	      }
	  }
	}

      /* Check the state of the monster list, and delete some monsters if
	 the monster list is nearly full.  This helps to avoid problems in
	 creature.c when monsters try to multiply.  Compact_monsters() is
	 much more likely to succeed if called from here, than if called
	 from within creatures().  */
      if (MAX_MALLOC - mfptr < 10)
	(void) compact_monsters ();

      if ((py.flags.paralysis < 1) &&	     /* Accept a command?     */
	  (py.flags.rest == 0) &&
	  (py.flags.stun < 100) &&
	  (!death))
	/* Accept a command and execute it				 */
	{
	  do
	    {
	      if (py.flags.status & PY_REPEAT)
		prt_state ();
	      default_dir = FALSE;
	      free_turn_flag = FALSE;

	      if (find_flag)
		{
		  find_run();
		  find_count--;
		  if (find_count == 0)
		    end_find();
		  put_qio();
		}
	      else if (doing_inven)
		inven_command (doing_inven);
	      else
		{
		  /* move the cursor to the players character */
		  move_cursor_relative (char_row, char_col);
		  if (command_count > 0)
		    {
		      command_count--;
		      msg_flag = FALSE;
		      default_dir = TRUE;
		    }
		  else
		    {
/* This bit of targetting code taken from Morgul -CFT */
/* If we are in targetting mode, with a creature target, make the targetted */
/* row and column match the creature's.  This optimizes a lot of code.  CDW */
		      if ((target_mode)&&(target_mon<mfptr))
			{
			  target_row = m_list[target_mon].fy;
			  target_col = m_list[target_mon].fx;
			}

		      msg_flag = FALSE;
		      command = inkey();
		      i = 0;
		      /* Get a count for a command. */
		      if ((rogue_like_commands
			   && command >= '0' && command <= '9')
			  || (!rogue_like_commands && command == '#'))
			{
			  char tmp[8];

			  prt("Repeat count:", 0, 0);
			  if (command == '#')
			    command = '0';
			  i = 0;
			  while (TRUE)
			    {
			      if (command == DELETE || command == CTRL('H'))
				{
				  i = i / 10;
				  (void) sprintf(tmp, "%d", i);
				  prt (tmp, 0, 14);
				}
			      else if (command >= '0' && command <= '9')
				{
			          if (i > 99)
				    bell ();
				  else
				    {
				      i = i * 10 + command - '0';
				      (void) sprintf (tmp, "%d", i);
				      prt (tmp, 0, 14);
				    }
				}
			      else
				break;
			      command = inkey();
			    }
			  if (i == 0)
			    {
			      i = 99;
			      (void) sprintf (tmp, "%d", i);
			      prt (tmp, 0, 14);
			    }
			  /* a special hack to allow numbers as commands */
			  if (command == ' ')
			    {
			      prt ("Command:", 0, 20);
			      command = inkey();
			    }
			}
		      /* Another way of typing control codes -CJS- */
		      if (command == '^')
			{
			  if (command_count > 0)
			    prt_state();
			  if (get_com("Control-", &command))
			    {
			      if (command >= 'A' && command <= 'Z')
				command -= 'A' - 1;
			      else if (command >= 'a' && command <= 'z')
				command -= 'a' - 1;
			      else
				{
			       msg_print("Type ^ <letter> for a control char");
				  command = ' ';
				}
			    }
			  else
			    command = ' ';
			}
		      /* move cursor to player char again, in case it moved */
		      move_cursor_relative (char_row, char_col);
		      /* Commands are always converted to rogue form. -CJS- */
		      if (rogue_like_commands == FALSE)
			command = original_commands (command);
		      if (i > 0)
			{
			  if (!valid_countcommand(command))
			    {
			      free_turn_flag = TRUE;
			      msg_print ("Invalid command with a count.");
			      command = ' ';
			    }
			  else
			    {
			      command_count = i;
			      prt_state ();
			      command_count--; /* count this pass as one */
			    }
			}
		    }
		  /* Flash the message line. */
		  erase_line(MSG_LINE, 0);
		  move_cursor_relative(char_row, char_col);
		  put_qio();

		  do_command (command);
		  /* Find is counted differently, as the command changes. */
		  if (find_flag)
		    {
		      find_count = command_count;
		      command_count = 0;
		    }
		  if (free_turn_flag)
		    command_count = 0;
		}
	      /* End of commands				     */
	    }
	  while (free_turn_flag && !new_level_flag && !eof_flag);
	}
      else
	{
	  /* if paralyzed, resting, or dead, flush output */
	  /* but first move the cursor onto the player, for aesthetics */
	  move_cursor_relative (char_row, char_col);
	  put_qio ();
	}

      /* Teleport?		       */
      if (teleport_flag)  teleport(100);
      /* Move the creatures	       */
      if (!new_level_flag)  creatures(TRUE);
      /* Exit when new_level_flag is set   */
    }
  while (!new_level_flag && !eof_flag);
}


static char original_commands(com_val)
char com_val;
{
  int dir_val;

  switch(com_val)
    {
    case CTRL('K'):	/*^K = exit    */
      com_val = 'Q';
      break;
    case CTRL('J'):
    case CTRL('M'):
      com_val = '+';
      break;
    case CTRL('R'):
    case CTRL('P'):	/*^P = repeat  */
    case CTRL('W'):	/*^W = password*/
    case CTRL('X'):	/*^X = save    */
    case ' ':
    case '!':
    case '*':
      break;
    case '.':
      { int temp = target_mode;  /* If in target_mode, player will not be
					given a chance to pick a direction.
					So we save it, force it off, and then
					ask for the direction -CFT */
        target_mode = FALSE;
      if (get_dir(NULL, &dir_val))
	switch (dir_val)
	  {
	  case 1:    com_val = 'B';    break;
	  case 2:    com_val = 'J';    break;
	  case 3:    com_val = 'N';    break;
	  case 4:    com_val = 'H';    break;
	  case 6:    com_val = 'L';    break;
	  case 7:    com_val = 'Y';    break;
	  case 8:    com_val = 'K';    break;
	  case 9:    com_val = 'U';    break;
	  default:   com_val = ' ';    break;
	  }
      else
	com_val = ' ';
      target_mode = temp; /* restore old target mode ... -CFT */
      }
      break;
    case '/':
    case '<':
    case '>':
    case '-':
    case '=':
    case '{':
    case '?':
    case 'A':
      break;
    case '1':
      com_val = 'b';
      break;
    case '2':
      com_val = 'j';
      break;
    case '3':
      com_val = 'n';
      break;
    case '4':
      com_val = 'h';
      break;
    case '5':	/* Rest one turn */
      com_val = '.';
      break;
    case '6':
      com_val = 'l';
      break;
    case '7':
      com_val = 'y';
      break;
    case '8':
      com_val = 'k';
      break;
    case '9':
      com_val = 'u';
      break;
    case 'B':
      com_val = 'f';
      break;
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'g':
      break;
    case 'L':
      com_val = 'W';
      break;
    case 'M':
      break;
    case 'R':
      break;
    case 'S':
      com_val = '#';
      break;
    case 'T':
      { int temp = target_mode;  /* If in target_mode, player will not be
					given a chance to pick a direction.
					So we save it, force it off, and then
					ask for the direction -CFT */
        target_mode = FALSE;
      if (get_dir(NULL, &dir_val))
	switch (dir_val)
	  {
	  case 1:    com_val = CTRL('B');    break;
	  case 2:    com_val = CTRL('J');    break;
	  case 3:    com_val = CTRL('N');    break;
	  case 4:    com_val = CTRL('H');    break;
	  case 6:    com_val = CTRL('L');    break;
	  case 7:    com_val = CTRL('Y');    break;
	  case 8:    com_val = CTRL('K');    break;
	  case 9:    com_val = CTRL('U');    break;
	  default:   com_val = ' ';	     break;
	  }
      else
	com_val = ' ';
      target_mode = temp;
      }
      break;
    case 'a':
      com_val = 'z';
      break;
    case 'b':
      com_val = 'P';
      break;
    case 'c':
    case 'd':
    case 'e':
      break;
    case 'f':
      com_val = 't';
      break;
    case 'h':
      com_val = '?';
      break;
    case 'i':
      break;
    case 'j':
      com_val = 'S';
      break;
    case 'l':
      com_val = 'x';
      break;
    case 'm':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
      break;
    case 't':
      com_val = 'T';
      break;
    case 'u':
      com_val = 'Z';
      break;
    case 'z':
      com_val = 'a';
      break;
    case 'V':
    case 'w':
      break;
    case 'x':
      com_val = 'X';
      break;

      /* wizard mode commands follow */
    case CTRL('A'): /*^A = cure all */
      break;
    case CTRL('B'):	/*^B = objects */
      com_val = CTRL('O');
      break;
    case CTRL('D'):	/*^D = up/down */
      break;
    case CTRL('H'):	/*^H = wizhelp */
      com_val = '\\';
      break;
    case CTRL('I'):	/*^I = identify*/
      break;
    case CTRL('^'):	/*^^ = identify all up to a level */
      break;
    case CTRL('L'):	/*^L = wizlight*/
      com_val = '$';
      break;
    case ':':
    case CTRL('T'):	/*^T = teleport*/
    case CTRL('E'):	/*^E = wizchar */
    case CTRL('F'):	/*^F = genocide*/
    case CTRL('G'):	/*^G = treasure*/
    case CTRL('V'):	/*^V = treasure*/
    case '@':
    case '+':
    case '%':  /* '%' == self knowledge */
    case '~': /* artifact list to file */
      break;
    case CTRL('U'):	/*^U = summon  */
      com_val = '&';
      break;
    default:
      com_val = '(';  /* Anything illegal. */
      break;
    }
  return com_val;
}

/* Is an item an enchanted weapon or armor and we don't know?  -CJS- */
/* returns positive if it is a good enchantment */
/* returns negative if a bad enchantment... */
int special_check(t_ptr)
register inven_type *t_ptr;
{
  if (t_ptr->tval==TV_NOTHING) return 0;
  if (known2_p(t_ptr))
    return 0;
  if (store_bought_p(t_ptr))
    return 0;
  if (t_ptr->ident & ID_MAGIK)
    return 0;
  if (t_ptr->ident & ID_DAMD)
    return 0;
  if (t_ptr->flags & TR_CURSED) return -1;
  if (t_ptr->tval!=TV_HARD_ARMOR && t_ptr->tval!=TV_SWORD &&
      t_ptr->tval!=TV_SOFT_ARMOR && t_ptr->tval!=TV_SHIELD &&
      t_ptr->tval!=TV_CLOAK && t_ptr->tval!=TV_GLOVES &&
      t_ptr->tval!=TV_BOOTS && t_ptr->tval!=TV_HELM &&
      t_ptr->tval!=TV_DIGGING && t_ptr->tval!=TV_SPIKE &&
      t_ptr->tval!=TV_SLING_AMMO && t_ptr->tval!=TV_BOLT &&
      t_ptr->tval!=TV_ARROW && t_ptr->tval!=TV_BOW &&
      t_ptr->tval!=TV_POLEARM && t_ptr->tval!=TV_HAFTED)
      return 0;
  if (t_ptr->tohit > 0 || t_ptr->todam > 0 || t_ptr->toac > 0)
    return 1;
  if ((t_ptr->tval == TV_DIGGING) && /* digging tools will pseudo ID, either
  					as {good} or {average} -CFT */
      (t_ptr->flags & TR_TUNNEL))
    return 1;
  return 0;
}


/* Is an item an enchanted weapon or armor and we don't know?  -CJS- */
/* returns a description */
char *value_check(t_ptr)
register inven_type *t_ptr;
{
  if (t_ptr->tval==TV_NOTHING) return 0;
  if (known2_p(t_ptr))
    return 0;
  if (store_bought_p(t_ptr))
    return 0;
  if (t_ptr->ident & ID_MAGIK)
    return 0;
  if (t_ptr->ident & ID_DAMD)
    return 0;
  if (t_ptr->inscrip[0] != '\0')
    return 0;
  if (t_ptr->flags&TR_CURSED && t_ptr->name2==SN_NULL) return "worthless";
  if (t_ptr->flags&TR_CURSED && t_ptr->name2!=SN_NULL) return "terrible";
  if ((t_ptr->tval == TV_DIGGING) &&  /* also, good digging tools -CFT */
      (t_ptr->flags & TR_TUNNEL) &&
      (t_ptr->p1 > object_list[t_ptr->index].p1)) /* better than normal for this
      						type of shovel/pick? -CFT */
    return "good";
  if ((t_ptr->tohit<=0 && t_ptr->todam<=0 && t_ptr->toac<=0) &&
      t_ptr->name2==SN_NULL)  /* normal shovels will also reach here -CFT */
    return "average";
  if (t_ptr->name2==SN_NULL)
    return "good";
  if ((t_ptr->name2==SN_R) || (t_ptr->name2==SN_RA) ||
      (t_ptr->name2==SN_RF) || (t_ptr->name2==SN_RC) ||
      (t_ptr->name2==SN_RL) || (t_ptr->name2==SN_SE) ||
      (t_ptr->name2==SN_HA) || (t_ptr->name2==SN_FT) ||
      (t_ptr->name2==SN_DF) || (t_ptr->name2==SN_FB) ||
      (t_ptr->name2==SN_SA) || (t_ptr->name2==SN_FREE_ACTION) ||
      (t_ptr->name2==SN_SD) || (t_ptr->name2==SN_SLAYING) ||
      (t_ptr->name2==SN_SU) || (t_ptr->name2==SN_SLOW_DESCENT) ||
      (t_ptr->name2==SN_SPEED) || (t_ptr->name2==SN_STEALTH) ||
      (t_ptr->name2==SN_INTELLIGENCE) || (t_ptr->name2==SN_WISDOM) ||
      (t_ptr->name2==SN_INFRAVISION) || (t_ptr->name2==SN_MIGHT) ||
      (t_ptr->name2==SN_LORDLINESS) || (t_ptr->name2==SN_MAGI) ||
      (t_ptr->name2==SN_BEAUTY) || (t_ptr->name2==SN_SEEING) ||
      (t_ptr->name2==SN_REGENERATION) || (t_ptr->name2==SN_PROTECTION) ||
      (t_ptr->name2==SN_FIRE) || (t_ptr->name2==SN_SLAY_EVIL) ||
      (t_ptr->name2==SN_DRAGON_SLAYING) || (t_ptr->name2==SN_SLAY_ANIMAL) ||
      (t_ptr->name2==SN_ACCURACY) || (t_ptr->name2==SN_SO) ||
      (t_ptr->name2==SN_POWER) || (t_ptr->name2==SN_WEST) ||
      (t_ptr->name2==SN_SDEM) || (t_ptr->name2==SN_ST) ||
      (t_ptr->name2==SN_LIGHT) || (t_ptr->name2==SN_AGILITY) ||
      (t_ptr->name2==SN_SG) || (t_ptr->name2==SN_TELEPATHY) ||
      (t_ptr->name2==SN_DRAGONKIND) || (t_ptr->name2==SN_AMAN) ||
      (t_ptr->name2==SN_ELVENKIND) || (t_ptr->name2==SN_WOUNDING))
    return "excellent";
  return "special";
}

static void do_command(com_val)
char com_val;
{
  int dir_val, do_pickup;
  int y, x, i, j;
  vtype out_val, tmp_str;
  register struct flags *f_ptr;

  /* hack for move without pickup.  Map '-' to a movement command. */
  if (com_val == '-')
    {
      do_pickup = FALSE;
      i = command_count;
      { int temp = target_mode;  /* If in target_mode, player will not be
					given a chance to pick a direction.
					So we save it, force it off, and then
					ask for the direction -CFT */
        target_mode = FALSE;
      if (get_dir(NULL, &dir_val))
	{
	  command_count = i;
	  switch (dir_val)
	    {
	    case 1:    com_val = 'b';	 break;
	    case 2:    com_val = 'j';	 break;
	    case 3:    com_val = 'n';	 break;
	    case 4:    com_val = 'h';	 break;
	    case 6:    com_val = 'l';	 break;
	    case 7:    com_val = 'y';	 break;
	    case 8:    com_val = 'k';	 break;
	    case 9:    com_val = 'u';	 break;
	    default:   com_val = '(';	 break;
	    }
	}
      else
	com_val = ' ';
      target_mode = temp;
      }
    }
  else
    do_pickup = TRUE;
  
  switch(com_val)
    {
    case 'Q':	/* (Q)uit		(^K)ill */
      flush();
      if ((!total_winner)?get_Yn("Do you really want to quit?")
	                 :get_Yn("Do you want to retire?"))
	{
	  new_level_flag = TRUE;
	  death = TRUE;
	  (void) strcpy(died_from, "Quitting");
	}
      free_turn_flag = TRUE;
      break;
    case CTRL('P'):	/* (^P)revious message. */
      if (command_count > 0)
	{
	  i = command_count;
	  if (i > MAX_SAVE_MSG)
	    i = MAX_SAVE_MSG;
	  command_count = 0;
	}
      else if (last_command != 16)
	i = 1;
      else
	i = MAX_SAVE_MSG;
      j = last_msg;
      if (i > 1) {
	save_screen();
	x = i;
	while (i > 0) {
	  i--;
	  prt(old_msg[j], i, 0);
	  if (j == 0)
	    j = MAX_SAVE_MSG-1;
	  else
	    j--;
	}
	erase_line (x, 0);
	pause_line(x);
	restore_screen();
      }
      else {
	/* Distinguish real and recovered messages with a '>'. -CJS- */
	put_buffer(">", 0, 0);
	prt(old_msg[j], 0, 1);
      }
      free_turn_flag = TRUE;
      break;
    case CTRL('W'):	/* (^W)izard mode */
      if (wizard)
	{
	  wizard = FALSE;
	  msg_print("Wizard mode off.");
	}
      else if (enter_wiz_mode())
	msg_print("Wizard mode on.");
      prt_winner();
      free_turn_flag = TRUE;
      break;
    case CTRL('X'):	/* e(^X)it and save */
      if (total_winner)
	{
       msg_print("You are a Total Winner,  your character must be retired.");
	  if (rogue_like_commands)
	    msg_print("Use 'Q' to when you are ready to retire.");
	  else
	    msg_print ("Use <Control>-K when you are ready to retire.");
	}
      else
	{
	  (void) strcpy (died_from, "(saved)");
	  msg_print ("Saving game...");
	  if (save_char ())
	    exit_game();
	  msg_print("Save failed...");
	  (void) strcpy (died_from, "(alive and well)");
	}
      free_turn_flag = TRUE;
      break;
    case CTRL('R'):
      if (py.flags.image > 0)
        msg_print("You cannot be sure what is real and what is not!");
      else {
	draw_cave(); /* thanks to David Kahane (dgk4@po.cwru.edu) for
			pointing out this better way to do this -CFT */
	creatures(FALSE);  /* draw monsters */
        }
      free_turn_flag = TRUE;
      break;
    case '*':	/* select a target (sorry, no intuitive letter keys were
    			left: a/A for aim, t/T for target, f/F for focus,
    			s/S for select, c/C for choose and p/P for pick
    			were all already taken.  Wiz light command moved
    			to '$', which was unused. -CFT */
      target();	/* target code taken from Morgul -CFT */
      free_turn_flag = TRUE;
      break;    			
    case '=':		/* (=) set options */
      save_screen();
      set_options();
      restore_screen();
      free_turn_flag = TRUE;
      break;
    case '{':		/* ({) inscribe an object    */
      scribe_object ();
      free_turn_flag = TRUE;
      break;
    case '!':		/* (!) escape to the shell */
      if (!wizard)
#ifdef MSDOS /* Let's be a little more accurate... */
	msg_print("Sorry, Angband doesn't leave enough free memory for a subshell.");
#else
	msg_print("Sorry, inferior shells are not allowed from ANGBAND.");
#endif
      else
	rerate();
      free_turn_flag = TRUE;
      break;
    case ESCAPE:	/* (ESC)   do nothing. */
    case ' ':		/* (space) do nothing. */
      free_turn_flag = TRUE;
      break;
    case 'b':		/* (b) down, left	(1) */
      move_char(1, do_pickup);
      break;
    case 'j':		/* (j) down		(2) */
      move_char(2, do_pickup);
      break;
    case 'n':		/* (n) down, right	(3) */
      move_char(3, do_pickup);
      break;
    case 'h':		/* (h) left		(4) */
      move_char(4, do_pickup);
      break;
    case 'l':		/* (l) right		(6) */
      move_char(6, do_pickup);
      break;
    case 'y':		/* (y) up, left		(7) */
      move_char(7, do_pickup);
      break;
    case 'k':		/* (k) up		(8) */
      move_char(8, do_pickup);
      break;
    case 'u':		/* (u) up, right	(9) */
      move_char(9, do_pickup);
      break;
    case 'B':		/* (B) run down, left	(. 1) */
      find_init(1);
      break;
    case 'J':		/* (J) run down		(. 2) */
      find_init(2);
      break;
    case 'N':		/* (N) run down, right	(. 3) */
      find_init(3);
      break;
    case 'H':		/* (H) run left		(. 4) */
      find_init(4);
      break;
    case 'L':		/* (L) run right	(. 6) */
      find_init(6);
      break;
    case 'Y':		/* (Y) run up, left	(. 7) */
      find_init(7);
      break;
    case 'K':		/* (K) run up		(. 8) */
      find_init(8);
      break;
    case 'U':		/* (U) run up, right	(. 9) */
      find_init(9);
      break;
    case '/':		/* (/) identify a symbol */
      ident_char();
      free_turn_flag = TRUE;
      break;
    case '.':		/* (.) stay in one place (5) */
      feel_chance = 10000;
      move_char (5, do_pickup);
      if (command_count > 1)
	{
	  command_count--;
	  rest();
	}
      break;
    case '<':		/* (<) go down a staircase */
      go_up();
      break;
    case '>':		/* (>) go up a staircase */
      go_down();
      break;
    case '?':		/* (?) help with commands */
      if (rogue_like_commands)
	helpfile(ANGBAND_HELP);
      else
	helpfile(ANGBAND_ORIG_HELP);
      free_turn_flag = TRUE;
      break;
    case 'f':		/* (f)orce		(B)ash */
      feel_chance = 100;
      bash();
      break;
    case 'A':		/* (A)ctivate		(A)ctivate */
      feel_chance = 30;
      activate();
      break;
    case 'C':		/* (C)haracter description */
      save_screen();
      change_name();
      restore_screen();
      free_turn_flag = TRUE;
      break;
    case 'D':		/* (D)isarm trap */
      feel_chance = 100;
      disarm_trap();
      break;
    case 'E':		/* (E)at food */
      feel_chance = 50;
      eat();
      break;
    case 'F':		/* (F)ill lamp */
      feel_chance = 50;
      refill_lamp();
      break;
    case 'G':		/* (G)ain magic spells */
      feel_chance = 30;
      gain_spells();
      break;
    case 'g':		/* (g)et an object... */
      feel_chance = 50;
      if (getkey_flag) {
	if (cave[char_row][char_col].tptr != 0) /* minor change -CFT */
	  carry(char_row, char_col, TRUE);
        }
      else
	free_turn_flag = TRUE;
      break;
    case 'W':		/* (W)here are we on the map	(L)ocate on map */
      if ((py.flags.blind > 0) || no_light())
	msg_print("You can't see your map.");
      else
	{
	  int cy, cx, p_y, p_x;
        int temp = target_mode;  /* If in target_mode, player will not be
					given a chance to pick a direction.
					So we save it, force it off, and then
					ask for the direction -CFT */
        target_mode = FALSE;

	  y = char_row;
	  x = char_col;
	  if (get_panel(y, x, TRUE))
	    prt_map();
	  cy = panel_row;
	  cx = panel_col;
	  for(;;)
	    {
	      p_y = panel_row;
	      p_x = panel_col;
	      if (p_y == cy && p_x == cx)
		tmp_str[0] = '\0';
	      else
		(void) sprintf(tmp_str, "%s%s of",
			       p_y < cy ? " North" : p_y > cy ? " South" : "",
			       p_x < cx ? " West" : p_x > cx ? " East" : "");
	      (void) sprintf(out_val,
	   "Map sector [%d,%d], which is%s your sector. Look which direction?",
			     p_y, p_x, tmp_str);
	      if (!get_dir(out_val, &dir_val))
		break;
/*								      -CJS-
// Should really use the move function, but what the hell. This
// is nicer, as it moves exactly to the same place in another
// section. The direction calculation is not intuitive. Sorry.
*/
	      for(;;){
		x += ((dir_val-1)%3 - 1) * SCREEN_WIDTH/2;
		y -= ((dir_val-1)/3 - 1) * SCREEN_HEIGHT/2;
		if (x < 0 || y < 0 || x >= cur_width || y >= cur_width)
		  {
		    msg_print("You've gone past the end of your map.");
		    x -= ((dir_val-1)%3 - 1) * SCREEN_WIDTH/2;
		    y += ((dir_val-1)/3 - 1) * SCREEN_HEIGHT/2;
		    break;
		  }
		if (get_panel(y, x, TRUE))
		  {
		    prt_map();
		    break;
		  }
	      }
	    }
	  /* Move to a new panel - but only if really necessary. */
	  if (get_panel(char_row, char_col, FALSE))
	    prt_map();
          target_mode = temp; /* restore target mode... */
	}
      free_turn_flag = TRUE;
      break;
    case 'R':		/* (R)est a while */
      feel_chance = 10000;
      rest();
      break;
    case '#':		/* (#) search toggle	(S)earch toggle */
      if (search_flag)
	search_off();
      else
	search_on();
      free_turn_flag = TRUE;
      break;
    case CTRL('B'):		/* (^B) tunnel down left	(T 1) */
      feel_chance = 10000;
      tunnel(1);
      break;
    case CTRL('M'):		/* cr must be treated same as lf. */
    case CTRL('J'):		/* (^J) tunnel down		(T 2) */
      feel_chance = 10000;
      tunnel(2);
      break;
    case CTRL('N'):		/* (^N) tunnel down right	(T 3) */
      feel_chance = 10000;
      tunnel(3);
      break;
    case CTRL('H'):		/* (^H) tunnel left		(T 4) */
      feel_chance = 10000;
      tunnel(4);
      break;
    case CTRL('L'):		/* (^L) tunnel right		(T 6) */
      feel_chance = 10000;
      tunnel(6);
      break;
    case CTRL('Y'):		/* (^Y) tunnel up left		(T 7) */
      feel_chance = 10000;
      tunnel(7);
      break;
    case CTRL('K'):		/* (^K) tunnel up		(T 8) */
      feel_chance = 10000;
      tunnel(8);
      break;
    case CTRL('U'):		/* (^U) tunnel up right		(T 9) */
      feel_chance = 10000;
      tunnel(9);
      break;
    case 'z':		/* (z)ap a wand		(a)im a wand */
      feel_chance = 30;
      aim();
      break;
    case 'a':		/* (a)ctivate a rod	(z)ap a rod */
      feel_chance = 30;
      activate_rod();
      break;
    case 'M':
      screen_map();
      free_turn_flag = TRUE;
      break;
    case 'P':		/* (P)eruse a book	(B)rowse in a book */
      examine_book();
      free_turn_flag = TRUE;
      break;
    case 'c':		/* (c)lose an object */
      feel_chance = 75;
      closeobject();
      break;
    case 'd':		/* (d)rop something */
      feel_chance = 50;
      inven_command('d');
      break;
    case 'e':		/* (e)quipment list */
      feel_chance = 100;
      inven_command('e');
      break;
    case 't':		/* (t)hrow something	(f)ire something */
      feel_chance = 30;
      throw_object();
      break;
    case 'i':		/* (i)nventory list */
      feel_chance = 100;
      inven_command('i');
      break;
    case 'S':		/* (S)pike a door	(j)am a door */
      feel_chance = 75;
      jamdoor();
      break;
    case 'x':		/* e(x)amine surrounds	(l)ook about */
      look();
      free_turn_flag = TRUE;
      break;
    case 'm':		/* (m)agic spells */
      feel_chance = 30;
      cast();
      break;
    case 'o':		/* (o)pen something */
      feel_chance = 30;
      openobject();
      break;
    case 'p':		/* (p)ray */
      feel_chance = 30;
      pray();
      break;
    case 'q':		/* (q)uaff */
      feel_chance = 30;
      quaff();
      break;
    case 'r':		/* (r)ead */
      feel_chance = 30;
      read_scroll();
      break;
    case 's':		/* (s)earch for a turn */
      feel_chance = 1000;
      search(char_row, char_col, py.misc.srh);
      break;
    case 'T':		/* (T)ake off something	(t)ake off */
      feel_chance = 100;
      inven_command('t');
      break;
    case 'Z':		/* (Z)ap a staff	(u)se a staff */
      feel_chance = 30;
      use();
      break;
    case 'V':		/* (V)ersion of game */
      helpfile(ANGBAND_VER);
      free_turn_flag = TRUE;
      break;
    case 'w':		/* (w)ear or wield */
      feel_chance = 30;
      inven_command('w');
      break;
    case 'X':		/* e(X)change weapons	e(x)change */
      feel_chance = 100;
      inven_command('x');
      break;
    default:
      if (wizard)
	{
	  free_turn_flag = TRUE; /* Wizard commands are free moves*/
	  switch(com_val)
	    {
	    case CTRL('A'):	/*^A = Cure all*/
	      (void) remove_curse();
	      (void) cure_blindness();
	      (void) cure_confusion();
	      (void) cure_poison();
	      (void) remove_fear();
	      (void) res_stat(A_STR);
	      (void) res_stat(A_INT);
	      (void) res_stat(A_WIS);
	      (void) res_stat(A_CON);
	      (void) res_stat(A_DEX);
	      (void) res_stat(A_CHR);
	      (void) restore_level();
	      (void) hp_player(2000);
	      f_ptr = &py.flags;
	      if (f_ptr->slow > 1)
		f_ptr->slow = 1;
	      if (f_ptr->image > 1)
		f_ptr->image = 1;
	      if (f_ptr->cut > 1)
		f_ptr->cut = 1;
	      if (f_ptr->stun > 1)
		f_ptr->stun = 1;
	      break;
	    case CTRL('E'):	/*^E = wizchar */
	      change_character();
	      erase_line(MSG_LINE, 0); /* from um55 -CFT */
	      break;
	    case CTRL('F'):	/*^F = genocide*/
	      (void) mass_genocide(FALSE);
	      break;
	    case CTRL('G'):	/*^G = treasure*/
	      if (command_count > 0)
		{
		  i = command_count;
		  command_count = 0;
		}
	      else
		i = 1;
	      random_object(char_row, char_col, i);
	      prt_map();
	      break;
	    case CTRL('V'):   	/*^V special treasure */
	      if (command_count > 0)
		{
		  i = command_count;
		  command_count = 0;
		}
	      else
		i = 1;
	      special_random_object(char_row, char_col, i);
	      prt_map();
	      break;
	    case CTRL('D'):	/*^D = up/down */
	      if (command_count > 0)
		{
		  if (command_count > 99)
		    i = 0;
		  else
		    i = command_count;
		  command_count = 0;
		}
	      else
		{
		  prt("Go to which level (0-10000) ? ", 0, 0);
		  i = -1;
		  if (get_string(tmp_str, 0, 27, 10))
		    i = atoi(tmp_str);
		    if (i>10000) i=10000;
		}
	      if (i > -1)
		{
		  dun_level = i;
		  if (dun_level > 10000)
		    dun_level = 10000;
		  new_level_flag = TRUE;
		}
	      else
		erase_line(MSG_LINE, 0);
	      break;
	    case CTRL('O'):	/*^O = objects */
	      print_objects();
	      break;
	    case '\\': /* \ wizard help */
	      if (rogue_like_commands)
		helpfile(ANGBAND_WIZ_HELP);
	      else
		helpfile(ANGBAND_OWIZ_HELP);
	      break;
	    case CTRL('I'):	/*^I = identify*/
	      (void) ident_spell();
	      break;
	    case CTRL('^'):	/*^^ = identify all up to a level */
	      prt("Identify objects upto which level (0-200) ? ", 0, 0);
	      i = -1;
	      if (get_string(tmp_str, 0, 47, 10))
		i = atoi(tmp_str);
	      if (i>200) i=200;
	      if (i>-1) {
		int j;
		inven_type inv;

		for (j=0; j<MAX_DUNGEON_OBJ; j++) {
		  if (object_list[j].level <= i) {
		    invcopy(&inv, j);
		    known1(&inv);
		  }
		}
	      }
	      erase_line(MSG_LINE, 0);
	      break;
	    case '$':	/* $ = wiz light */
	      wizard_light(TRUE);
	      break;
	    case ':':
	      map_area();
	      break;
	    case '~':
	      artifact_check();
	      break;
	    case CTRL('T'):	/*^T = teleport*/
	      teleport(100);
	      break;
	    case '+':
	      if (command_count > 0)
		{
		  py.misc.exp = command_count;
		  command_count = 0;
		}
	      else if (py.misc.exp == 0)
		py.misc.exp = 1;
	      else
		py.misc.exp = py.misc.exp * 2;
	      prt_experience();
	      break;
	    case '&':	/*& = summon  */
	      y = char_row;
	      x = char_col;
	      (void) summon_monster(&y, &x, TRUE);
	      creatures(FALSE);
	      break;
	    case '@':
	      wizard_create();
	      break;
	    case '%': /* self-knowledge */
	      self_knowledge();
	      break;
	    default:
	      if (rogue_like_commands)
		prt("Type '?' or '\\' for help.", 0, 0);
	      else
		prt("Type '?' or ^H for help.", 0, 0);
	    }
	}
      else
	{
	  prt("Type '?' for help.", 0, 0);
	  free_turn_flag = TRUE;
	}
    }
  last_command = com_val;
}

/* Check whether this command will accept a count.     -CJS-  */
static int valid_countcommand(c)
char c;
{
  switch(c)
    {
    case 'Q':
    case CTRL('W'):
    case CTRL('X'):
    case '=':
    case '{':
    case '/':
    case '<':
    case '>':
    case '?':
    case 'A':
    case 'C':
    case 'E':
    case 'F':
    case 'G':
    case '#':
    case 'z':
    case 'P':
    case 'c':
    case 'd':
    case 'e':
    case 't':
    case 'i':
    case 'x':
    case 'm':
    case 'p':
    case 'q':
    case 'r':
    case 'T':
    case 'Z':
    case 'V':
    case 'w':
    case 'W':
    case 'X':
    case CTRL('A'):
    case '\\':
    case CTRL('I'):
    case CTRL('^'):
    case '$':
    case '*':
    case ':':
    case CTRL('T'):
    case CTRL('E'):
    case CTRL('F'):
    case CTRL('S'):
    case CTRL('Q'):
    case CTRL('R'):
      return FALSE;
    case ESCAPE:
    case ' ':
    case '-':
    case 'b':
    case 'f':
    case 'j':
    case 'n':
    case 'h':
    case 'l':
    case 'y':
    case 'k':
    case 'u':
    case '.':
    case 'B':
    case 'J':
    case 'N':
    case 'H':
    case 'L':
    case 'Y':
    case 'K':
    case 'U':
    case 'D':
    case 'R':
    case CTRL('Y'):
    case CTRL('K'):
    case CTRL('U'):
    case CTRL('N'):
    case CTRL('J'):
    case CTRL('B'):
    case CTRL('H'):
    case CTRL('L'):
    case 'S':
    case 'o':
    case 's':
    case CTRL('D'):
    case CTRL('G'):
    case '+':
      return TRUE;
    default:
      return FALSE;
    }
}


/* Regenerate hit points				-RAK-	*/
static void regenhp(percent)
int percent;
{
  register struct misc *p_ptr;
  register int32 new_chp, new_chp_frac;
  int old_chp;

  p_ptr = &py.misc;
  old_chp = p_ptr->chp;
  new_chp = ((long)p_ptr->mhp) * percent + PLAYER_REGEN_HPBASE;
  p_ptr->chp += new_chp >> 16;	/* div 65536 */
  /* check for overflow */
  if (p_ptr->chp < 0 && old_chp > 0)
    p_ptr->chp = MAX_SHORT;
  new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac; /* mod 65536 */
  if (new_chp_frac >= 0x10000L)
    {
      p_ptr->chp_frac = new_chp_frac - 0x10000L;
      p_ptr->chp++;
    }
  else
    p_ptr->chp_frac = new_chp_frac;

  /* must set frac to zero even if equal */
  if (p_ptr->chp >= p_ptr->mhp)
    {
      p_ptr->chp = p_ptr->mhp;
      p_ptr->chp_frac = 0;
    }
  if (old_chp != p_ptr->chp)
    prt_chp();
}


/* Regenerate mana points				-RAK-	*/
static void regenmana(percent)
int percent;
{
  register struct misc *p_ptr;
  register int32 new_mana, new_mana_frac;
  int old_cmana;

  p_ptr = &py.misc;
  old_cmana = p_ptr->cmana;
  new_mana = ((long)p_ptr->mana) * percent + PLAYER_REGEN_MNBASE;
  p_ptr->cmana += new_mana >> 16;  /* div 65536 */
  /* check for overflow */
  if (p_ptr->cmana < 0 && old_cmana > 0)
    p_ptr->cmana = MAX_SHORT;
  new_mana_frac = (new_mana & 0xFFFF) + p_ptr->cmana_frac; /* mod 65536 */
  if (new_mana_frac >= 0x10000L)
    {
      p_ptr->cmana_frac = new_mana_frac - 0x10000L;
      p_ptr->cmana++;
    }
  else
    p_ptr->cmana_frac = new_mana_frac;

  /* must set frac to zero even if equal */
  if (p_ptr->cmana >= p_ptr->mana)
    {
      p_ptr->cmana = p_ptr->mana;
      p_ptr->cmana_frac = 0;
    }
  if (old_cmana != p_ptr->cmana)
    prt_cmana();
}


/* Is an item an enchanted weapon or armor and we don't know?  -CJS- */
/* only returns true if it is a good enchantment */
static int enchanted (t_ptr)
register inven_type *t_ptr;
{
  if (t_ptr->tval < TV_MIN_ENCHANT || t_ptr->tval > TV_MAX_ENCHANT
      || t_ptr->flags & TR_CURSED)
    return FALSE;
  if (known2_p(t_ptr))
    return FALSE;
  if (t_ptr->ident & ID_MAGIK)
    return FALSE;
  if (t_ptr->tohit > 0 || t_ptr->todam > 0 || t_ptr->toac > 0)
    return TRUE;
  if ((0x4000107fL & t_ptr->flags) && t_ptr->p1 > 0)
    return TRUE;
  if (0x07ffe980L & t_ptr->flags)
    return TRUE;

  return FALSE;
}

int ruin_stat(stat)
register int stat;
{
  register int tmp_stat;

  tmp_stat = py.stats.cur_stat[stat];
  if (tmp_stat > 3) {
    if (tmp_stat > 6) {
      if (tmp_stat < 19) {
	tmp_stat-=3;
      } else {
	tmp_stat /= 2;
	if (tmp_stat < 18)
	  tmp_stat = 18;
      }
    } else tmp_stat--;

    py.stats.cur_stat[stat] = tmp_stat;
    py.stats.max_stat[stat] = tmp_stat;
    set_use_stat (stat);
    prt_stat (stat);
    return TRUE;
  } else return FALSE;
}

static void activate() {
  int i, a, flag, first, num, j, redraw, dir, test;
  char out_str[200], tmp[200], tmp2[200], choice;
  inven_type *i_ptr;
  struct misc *m_ptr;

  flag=FALSE;
  redraw=FALSE;
  num=0;
  first=0;
  for (i=22; i<(INVEN_ARRAY_SIZE-1); i++) {
    if ((inventory[i].flags2 & TR_ACTIVATE) && (known2_p(&(inventory[i])))) {
      num++;
      if (!flag) first=i;
      flag=TRUE;
    }
  }
  if (!flag) {
    msg_print("You are not wearing/wielding anything that can be activated.");
    free_turn_flag = TRUE;
    return;
  }
  sprintf(out_str, "Activate which item? (%c-%c, * to list, ESC to exit) ?",
	  'a', 'a'+(num-1));
  flag=FALSE;
  while (!flag){
    if (!get_com(out_str, &choice))  /* on escape, get_com returns false: */
      choice = '\033';	/* so it's set here to ESC.  Wierd huh? -CFT */
    if ((choice=='*') && !redraw) { /* don't save screen again if it's already
    					listed, OW it doesn't clear -CFT */
      save_screen();
      j=0;
      if (!redraw) {
	for (i=first; i<(INVEN_ARRAY_SIZE-1); i++) {
	  if ((inventory[i].flags2 & TR_ACTIVATE) && known2_p(&(inventory[i]))) {
	    objdes(tmp2, &inventory[i], TRUE);
	    sprintf(tmp, "%c) %-61s", 'a'+j, tmp2);
	    erase_line(1+j, 13); /* we display at 15, but erase from 13 to
	    				give a couple of spaces, so it looks
	    				tidy.  -CFT */
#ifdef TC_COLOR
	    if (!no_color_flag) textcolor( inven_color( inventory[i].tval ) );
#endif	    
	    prt(tmp, 1+j, 15);	/* No need to check for bottom of screen,
	    			   since only have 11 items in equip, so
	    			   will never reach bottom... -CFT */
#ifdef TC_COLOR
	    if (!no_color_flag) textcolor( LIGHTGRAY );
#endif	    
	    j++;
	  }
	}
	redraw=TRUE;
	continue;
      }
    } else {
      test = FALSE; 
      if (choice>='A' && choice<=('A'+(num-1))) {
	choice-='A';
	test = TRUE; /* test to see if means it... */
        }
      else if (choice>='a' && choice<=('a'+(num-1)))
	choice-='a';
      else if (choice=='\033') {
        if (redraw) {
	  restore_screen();
	  redraw = FALSE;
          }
	free_turn_flag = TRUE;
	break;
        }
      else {
	bell();
	continue; /* try another letter */
        }

      if (redraw) {
	restore_screen();
	redraw = FALSE;
        }
        
      if (choice>num) continue;
      j=0;
      for (i=first; i<(INVEN_ARRAY_SIZE-1); i++) {
	  if ((inventory[i].flags2 & TR_ACTIVATE) && known2_p(&(inventory[i]))) {
	    if (j==choice) break;
	    j++;
	  }
	}
      if ( (test && verify("Activate", i)) || !test)
        flag = TRUE;
      else { 
	flag = TRUE; /* exit loop, he didn't want to try it... */
	free_turn_flag = TRUE; /* but he didn't do anything either */
        continue;
        }

      if (inventory[i].timeout>0) {
	msg_print("It whines, glows and fades...");
	break;
      }
      if (py.stats.use_stat[A_INT]<randint(18) &&
	  randint(object_list[inventory[i].index].level)>py.misc.lev) {
	msg_print("You fail to activate it properly.");
	break;
      }
      msg_print("You activate it...");
      switch (inventory[i].index) {
      case (29):
      case (395):
      case (396): /* The dreaded daggers:-> */
      case (397):
	if (inventory[i].name2 == SN_NARTHANC) {
	  msg_print("Your Dagger is covered in fire...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_bolt(GF_FIRE, dir, char_row, char_col, damroll(9,8));
	    inventory[i].timeout=5+randint(10);
	  }
	}
	else if (inventory[i].name2 == SN_NIMTHANC) {
	  msg_print("Your Dagger is covered in frost...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_bolt(GF_FROST, dir, char_row, char_col, damroll(6,8));
	    inventory[i].timeout=4+randint(8);
	  }
	}
	else if (inventory[i].name2 == SN_DETHANC) {
	  msg_print("Your Dagger is covered in sparks...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_bolt(GF_LIGHTNING, dir, char_row, char_col, damroll(4,8));
	    inventory[i].timeout=3+randint(7);
	  }
	}
	else if (inventory[i].name2 == SN_RILIA) {
	  msg_print("Your Dagger throbs deep green...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_POISON_GAS, dir, char_row, char_col, 12, 2);
	    inventory[i].timeout=3+randint(3);
	  }
	}
	else if (inventory[i].name2 == SN_BELANGIL) {
	  msg_print("Your Dagger is covered in frost...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_FROST, dir, char_row, char_col, 48, 2);
	    inventory[i].timeout=3+randint(7);
	  }
	}
	break;
      case (91):
	if (inventory[i].name2 == SN_DAL) {
	  msg_print("You feel energy flow through your feet...");
	  remove_fear();
	  cure_poison();
	  inventory[i].timeout = 5;
	}
	break;
      case (42):
      case (43):
	if (inventory[i].name2 == SN_RINGIL) {
	  msg_print("Your sword glows an intense blue...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_FROST, dir, char_row, char_col, 100, 3);
	    inventory[i].timeout=300;
	  }
	} else if (inventory[i].name2 == SN_ANDURIL) {
	  msg_print("Your sword glows an intense red...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_FIRE, dir, char_row, char_col, 72, 3);
	    inventory[i].timeout=400;
	  }
	}
	break;
      case (52):
	if (inventory[i].name2 == SN_FIRESTAR) {
	  msg_print("Your Morningstar rages in fire...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_FIRE, dir, char_row, char_col, 72, 3);
	    inventory[i].timeout=100;
	  }
	}
	break;
      case (92):
	if (inventory[i].name2 == SN_FEANOR) {
	  py.flags.fast += randint(25) + 15;
	  inventory[i].timeout=200;
	}
	break;
      case (59):
	if (inventory[i].name2 == SN_THEODEN) {
	  msg_print("The blade of your axe glows black...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
 	      do {dir = randint(9);} while (dir == 5);
	    }
	    drain_life(dir, char_row, char_col, 120);
	    inventory[i].timeout=400;
	  }
	}
	break;
      case (62):
	if (inventory[i].name2 == SN_TURMIL) {
          msg_print("The head of your hammer glows white...");
 	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    drain_life(dir, char_row, char_col, 90);
	    inventory[i].timeout = 70;
	  }
	}
	break;
      case (111):
	if (inventory[i].name2 == SN_CASPANION) {
	  msg_print("Your mail magically disarms traps...");
	  td_destroy();
	  inventory[i].timeout=10;
	}
	break;
      case (71):
	if (inventory[i].name2 == SN_AVAVIR) {
	      char c; int f = TRUE;
	      do { /* loop, so RET or other key doesn't accidently exit */
	      	f = get_com("Do you really want to return?", &c);
	        } while (f && (c != 'y') && (c != 'Y') && (c != 'n') &&
	        		(c != 'N'));
	      if (f && (c != 'n') && (c != 'N')) {
	    if (py.flags.word_recall == 0)
	      py.flags.word_recall = 15 + randint(20);
	    msg_print("The air about you becomes charged...");
	    }
	  inventory[i].timeout=200;
	}
	break;
      case (53):
	if (inventory[i].name2 == SN_TARATOL) {
	  if (py.flags.fast == 0)
	  py.flags.fast += randint(30) + 15;
	  inventory[i].timeout=166;
	}
	break;
      case (54):
	if (inventory[i].name2 == SN_ERIRIL) {
	  ident_spell();
	  inventory[i].timeout=10;
	}
        else if (inventory[i].name2 == SN_OLORIN) {
          probing();
          inventory[i].timeout=20;
        }
	break;
      case (67):
	if (inventory[i].name2 == SN_EONWE) {
	  msg_print("Your axe lets out a long, shrill note...");
	  mass_genocide(TRUE);
          inventory[i].timeout=1000;
        }
	break;
      case (68):
	if (inventory[i].name2 == SN_LOTHARANG) {
	  msg_print("Your Battle Axe radiates deep purple...");
	  hp_player(damroll(4, 7));
	  if (py.flags.cut>0) {
	    py.flags.cut=(py.flags.cut/2)-50;
	    if (py.flags.cut<0) py.flags.cut=0;
	    msg_print("You wounds heal.");
	  }
	  inventory[i].timeout=2+randint(2);
	}
	break;
      case (75):
	if (inventory[i].name2 == SN_CUBRAGOL) {
	  for (a=0; a<INVEN_WIELD; a++)
	    if ((inventory[a].tval == TV_BOLT) &&
		!(inventory[a].flags & TR_CURSED)) /* the "taint" of a cursed
						item inteferes with the
						branding -CFT */
	      break;
	  if (a<INVEN_WIELD && (inventory[a].name2 == SN_NULL)) {
	    int i,j;
	    i_ptr = &inventory[a];
	    msg_print("Your bolts are covered in a fiery shield!");
	    i_ptr->name2 = SN_FIRE;
	    i_ptr->flags |= (TR_FLAME_TONGUE|TR_RES_FIRE);
	    i_ptr->cost+=25;
	    enchant(i_ptr, 3+randint(3), ENCH_TOHIT|ENCH_TODAM);
	  } else 
	    msg_print("The fiery enchantment fails.");
	  inventory[i].timeout=999;
	}
	break;
      case (34):
      case (35):
	if (inventory[i].name2 == SN_ARUNRUTH) {
	  msg_print("Your sword glows a pale blue...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    line_spell(GF_FROST, dir, char_row, char_col, damroll(12,8));
	    inventory[i].timeout=500;
	  }
	}
	break;
      case (64):
	if (inventory[i].name2 == SN_AEGLOS) {
	  msg_print("Your spear glows a bright white...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_FROST, dir, char_row, char_col, 100, 3);
	    inventory[i].timeout=500;
	  }
	} else if (inventory[i].name2 == SN_OROME) {
	    msg_print("Your spear pulsates...");
	    if (get_dir(NULL, &dir)) {
	      if (py.flags.confused > 0) {
		msg_print("You are confused.");
		do {dir = randint(9);} while (dir == 5);
	      }
	    wall_to_mud(dir, char_row, char_col);
	    inventory[i].timeout=5;
	  }
	}
	break;
      case (118):
	if (inventory[i].name2 == SN_SOULKEEPER) {
	  msg_print("Your armour glows a bright white...");
	  msg_print("You feel much better...");
	  hp_player(1000);
	  inventory[i].timeout=888;
	}
	break;
      case (120):
	if (inventory[i].name2 == SN_BELEGENNON) {
	  teleport(10);
	  inventory[i].timeout=2;
      }
      break;
      case (119):
        if (inventory[i].name2 == SN_CELEBORN) {
          genocide(TRUE);
          inventory[i].timeout=500;
        }
        break;
      case (124):
	if (inventory[i].name2 == SN_LUTHIEN) {
	  restore_level();
	  inventory[i].timeout=450;
	}
	break;
      case (65):
	if (inventory[i].name2 == SN_ULMO) {
	  msg_print("Your trident glows deep red...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    teleport_monster(dir, char_row, char_col);
	    inventory[i].timeout=150;
	  }
	}
	break;
      case (123): /* Cloak */
      case (411):
	if (inventory[i].name2 == SN_COLLUIN) {
	  msg_print("Your cloak glows many colours...");
	  msg_print("You feel you can resist anything.");
	  py.flags.resist_heat += randint(20) + 20;
	  py.flags.resist_cold += randint(20) + 20;
	  py.flags.resist_light += randint(20) + 20;
	  py.flags.resist_poison += randint(20) + 20;
	  py.flags.resist_acid += randint(20) + 20;
	  inventory[i].timeout=111;
	} else if (inventory[i].name2 == SN_HOLCOLLETH) {
	  msg_print("You momentarily disappear...");
	  sleep_monsters1(char_row, char_col);
	  inventory[i].timeout=55;
	} else if (inventory[i].name2 == SN_THINGOL) {
	  msg_print("You hear a low humming noise...");
	  recharge(60);
	  inventory[i].timeout=70;
	} else if (inventory[i].name2 == SN_COLANNON) {
	  teleport(100);
	  inventory[i].timeout=45;
	}
	break;
      case (50): /*Flail*/
	if (inventory[i].name2 == SN_TOTILA) {
	  msg_print("Your Flail glows in scintillating colours...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    confuse_monster(dir, char_row, char_col);
	    inventory[i].timeout=15;
	  }
	}
	break;
      case (125): /*Gloves*/
	if (inventory[i].name2 == SN_CAMMITHRIM) {
	  msg_print("Your Gloves glow extremely brightly...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_bolt(GF_MAGIC_MISSILE, dir, char_row, char_col, damroll(2,6));
	    inventory[i].timeout=2;
	  }
	}
	break;
      case (126): /*Gauntlets*/
	if (inventory[i].name2 == SN_PAURHACH) {
	  msg_print("Your Gauntlets are covered in fire...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    if (randint(4)==1)
	      line_spell(GF_FIRE, dir, char_row, char_col, damroll(9,8));
	    else
	      fire_bolt(GF_FIRE, dir, char_row, char_col, damroll(9,8));
	    inventory[i].timeout=5+randint(10);
	  }
	}
	else if (inventory[i].name2 == SN_PAURNIMMEN) {
	  msg_print("Your Gauntlets are covered in frost...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    if (randint(5)==1)
	      line_spell(GF_FROST, dir, char_row, char_col, damroll(6,8));
	    else
	      fire_bolt(GF_FROST, dir, char_row, char_col, damroll(6,8));
	    inventory[i].timeout=4+randint(8);
	  }
	}
	else if (inventory[i].name2 == SN_PAURAEGEN) {
	  msg_print("Your Gauntlets are covered in sparks...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    if (randint(6)==1)
	      line_spell(GF_LIGHTNING, dir, char_row, char_col, damroll(4,8));
	    else
	      fire_bolt(GF_LIGHTNING, dir, char_row, char_col, damroll(4,8));
	    inventory[i].timeout=3+randint(7);
	  }
	}
	else if (inventory[i].name2 == SN_PAURNEN) {
	  msg_print("Your Gauntlets look very acidic...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    if (randint(5)==1)
	      line_spell(GF_ACID, dir, char_row, char_col, damroll(5,8));
	    else
	      fire_bolt(GF_ACID, dir, char_row, char_col, damroll(5,8));
	    inventory[i].timeout=4+randint(7);
	  }
	}
	break;
      case (127):
	if (inventory[i].name2 == SN_FINGOLFIN) {
	  msg_print("Magical spikes appear on your Cesti...");
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_bolt(GF_MANA, dir, char_row, char_col, 150);
	    inventory[i].timeout=88+randint(88);
	  }
	}
	break;
      case (96):
	if (inventory[i].name2 == SN_HOLHENNETH) {
	  msg_print("You close your eyes, an image forms in your mind...");
	  detection();
	  inventory[i].timeout=55+randint(55);
	}
	break;
      case (99):
	if (inventory[i].name2 == SN_GONDOR) {
	msg_print("You feel a warm tingling inside...");
	hp_player(500);
	inventory[i].timeout=500;
	}
	break;
      case (SPECIAL_OBJ-1): /* Narya */
	msg_print("The ring glows deep red...");
        if (get_dir(NULL, &dir)) {
          if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_FIRE, dir, char_row, char_col, 120, 3);
	  inventory[i].timeout=222+randint(222);
	}
	break;
      case (SPECIAL_OBJ): /* Nenya */
	msg_print("The ring glows bright white...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_FROST, dir, char_row, char_col, 200, 3);
	  inventory[i].timeout=222+randint(333);
	}
	break;
      case (SPECIAL_OBJ+1): /* Vilya */
	msg_print("The ring glows deep blue...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_LIGHTNING, dir, char_row, char_col, 250, 3);
	  inventory[i].timeout=222+randint(444);
	}
	break;
      case (SPECIAL_OBJ+2): /* Power */
	msg_print("The ring glows intensely black...");
	switch (randint(17)+(8-py.misc.lev/10)) {
	case 5:
	  dispel_creature(0xFFFFFFFL, 1000);
	  break;
	case 6:
	case 7:
	  msg_print("You are surrounded by a malignant aura");
	  m_ptr = &py.misc;
	  m_ptr->lev--;
	  m_ptr->exp=player_exp[m_ptr->lev-2]*m_ptr->expfact/100 +
	    randint((player_exp[m_ptr->lev-1]*m_ptr->expfact/100) -
		    (player_exp[m_ptr->lev-2]*m_ptr->expfact/100));
	  m_ptr->max_exp=m_ptr->exp;
	  prt_experience();
	  ruin_stat(A_STR);
	  ruin_stat(A_INT);
	  ruin_stat(A_WIS);
	  ruin_stat(A_DEX);
	  ruin_stat(A_CON);
	  ruin_stat(A_CHR);
	  calc_hitpoints();
	  if (class[m_ptr->pclass].spell == MAGE) {
	    calc_spells(A_INT);
	    calc_mana(A_INT);
	  } else if (class[m_ptr->pclass].spell == PRIEST) {
	    calc_spells(A_WIS);
	    calc_mana(A_WIS);
	  }
	  prt_level();
	  prt_title();
	  take_hit((py.misc.chp>2)?py.misc.chp/2:0, "malignant aura");
	  break;
	case 8:
	case 9:
	case 10:
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    fire_ball(GF_MANA, dir, char_row, char_col, 300, 3);
	  }
	  break;
	default:
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    if (randint(2)==1)
	      line_spell(GF_MANA, dir, char_row, char_col, 250);
	    else
	      fire_bolt(GF_MANA, dir, char_row, char_col, 250);
	  }
	}
	inventory[i].timeout=444+randint(444);
	break;
      case (389): /* Blue */
	msg_print("You breathe Lightning...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_LIGHTNING, dir, char_row, char_col, 100, 3);
	  inventory[i].timeout=444+randint(444);
	}
	break;
      case (390): /* White */
	msg_print("You breathe Frost...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_FROST, dir, char_row, char_col, 110, 3);
	  inventory[i].timeout=444+randint(444);
	}
	break;
      case (391): /* Black */
	msg_print("You breathe Acid...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_ACID, dir, char_row, char_col, 130, 3);
	  inventory[i].timeout=444+randint(444);
	}
	break;
      case (392): /* Gas */
	msg_print("You breathe Poison Gas...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_POISON_GAS, dir, char_row, char_col, 150, 3);
	  inventory[i].timeout=444+randint(444);
	}
	break;
      case (393): /* Fire */
	msg_print("You breathe Fire...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_FIRE, dir, char_row, char_col, 200, 3);
	  inventory[i].timeout=444+randint(444);
	}
	break;
      case (394): /* Multi-hued */
	if (inventory[i].name2 == SN_RAZORBACK) {
	  msg_print("A storm of lightning spikes fires in all directions...");
	  starball(char_row, char_col);
	  inventory[i].timeout=1000;
        }
	else {
	  if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
	      msg_print("You are confused.");
	      do {dir = randint(9);} while (dir == 5);
	    }
	    choice=randint(5);
	    sprintf(tmp2, "You breathe %s...",
	            ((choice==1)?"Lightning":
	             ((choice==2)?"Frost":
		      ((choice==3)?"Acid":
		       ((choice==4)?"Poison Gas":"Fire")))));
	    msg_print(tmp2);
	    fire_ball(((choice==1)?GF_LIGHTNING:
	               ((choice==2)?GF_FROST:
	                ((choice==3)?GF_ACID:
	                 ((choice==4)?GF_POISON_GAS:GF_FIRE)))),
		      dir, char_row, char_col, 250, 3);
	    inventory[i].timeout=222+randint(222);
	  }
	}
	break;
      case (408): /* Bronze */
	msg_print("You breathe Confusion...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_CONFUSION, dir, char_row, char_col, 120, 3);
	  inventory[i].timeout=444+randint(444);
	}
      break;
      case (409): /* Gold */
	msg_print("You breathe Sound...");
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  fire_ball(GF_SOUND, dir, char_row, char_col, 130, 3);
	  inventory[i].timeout=444+randint(444);
	}
      break;
      case (415): /* Chaos */
	if (get_dir(NULL, &dir)) {
	  if (py.flags.confused > 0) {
	    msg_print("You are confused.");
	    do {dir = randint(9);} while (dir == 5);
	  }
	  choice = randint(2);
	  sprintf(tmp2, "You breathe %s...",
	          ((choice==1?"Chaos":"Disenchantment")));
	  msg_print(tmp2);
	  fire_ball((choice==1 ? GF_CHAOS : GF_DISENCHANT), dir,
		char_row, char_col, 220, 3);
	  inventory[i].timeout=300+randint(300);
	}
        break;
      case (416): /* Law */
        if (get_dir(NULL, &dir)) {
          if (py.flags.confused > 0) {
            msg_print("You are confused.");
            do {dir = randint(9);} while (dir == 5);
          }
          choice = randint(2);
          sprintf(tmp2, "You breathe %s...",
                  ((choice==1?"Sound":"Shards")));
          msg_print(tmp2);
          fire_ball((choice==1 ? GF_SOUND : GF_SHARDS), dir,
		char_row, char_col, 230, 3);
          inventory[i].timeout=300+randint(300);
        }
        break;
      case (417): /* Balance */
	if (get_dir(NULL, &dir)) {
          if (py.flags.confused > 0) {
            msg_print("You are confused.");
            do {dir = randint(9);} while (dir == 5);
          }
          choice=randint(4);
          sprintf(tmp2, "You breathe %s...",
                  ((choice==1)?"Chaos":
                   ((choice==2)?"Disenchantment":
                    ((choice==3)?"Sound":"Shards"))));
          msg_print(tmp2);
          fire_ball(((choice==1) ? GF_CHAOS : ((choice==2) ? GF_DISENCHANT :
		((choice==3) ? GF_SOUND : GF_SHARDS))), dir,
		char_row, char_col, 250, 3);
          inventory[i].timeout=300+randint(300);
        }
        break;
      case (418): /* Shining */
        if (get_dir(NULL, &dir)) {
          if (py.flags.confused > 0) {
            msg_print("You are confused.");
            do {dir = randint(9);} while (dir == 5);
          }
          choice = randint(2);
          sprintf(tmp2, "You breathe %s...",
                  ((choice==1?"Light":"Darkness")));
          msg_print(tmp2);
          fire_ball((choice==1 ? GF_LIGHT : GF_DARK), dir,
		char_row, char_col, 200, 3);
          inventory[i].timeout=300+randint(300);
        }
        break;
      case (419): /* Power Dragon Scale Mail */
	if (inventory[i].name2 == SN_BLADETURNER) {
	  msg_print("Your armour glows many colours...");
	  msg_print("You enter a berserk rage...");
	  py.flags.hero += randint(50)+50;
	  py.flags.shero += randint(50)+50;
	  bless(randint(50)+50);
	  py.flags.resist_heat += randint(50)+50;
	  py.flags.resist_cold += randint(50)+50;
	  py.flags.resist_light += randint(50)+50;
	  py.flags.resist_acid += randint(50)+50;
	  inventory[i].timeout=400;
        }
        else {
          msg_print("You breathe the Elements...");
          if (get_dir(NULL, &dir)) {
            if (py.flags.confused > 0) {
              msg_print("You are confused.");
              do {dir = randint(9);} while (dir == 5);
            }
            fire_ball(GF_MAGIC_MISSILE, dir, char_row, char_col, 300, 3);
            inventory[i].timeout=300+randint(300);
          }
	}
        break;
      case (SPECIAL_OBJ+3):
	msg_print("The phial wells with clear light...");
	light_area(char_row, char_col);
	inventory[i].timeout=10+randint(10);
	break;
      case (SPECIAL_OBJ+4):
	msg_print("An aura of good floods the area...");
	dispel_creature(EVIL, (int)(5*py.misc.lev));
	inventory[i].timeout=444+randint(222);
	break;
      case (SPECIAL_OBJ+5):
        msg_print("The amulet lets out a shrill wail...");
	msg_print("You feel somewhat safer...");
	protect_evil();
	inventory[i].timeout=222+randint(222);
	break;
      case (SPECIAL_OBJ+6):
	msg_print("The star shines brightly...");
	msg_print("And you sense your surroundings...");
	map_area();
	inventory[i].timeout=50+randint(50);
        break;
      case (SPECIAL_OBJ+7):
        msg_print("The stone glows a deep green");
        wizard_light(TRUE);
        inventory[i].timeout=100+randint(100);
        break;
      case (SPECIAL_OBJ+8):
	msg_print("The ring glows brightly...");
	py.flags.fast += randint(100) + 50;
	inventory[i].timeout=200;
	break;
      default:
	(void) sprintf(tmp2, "Inventory num %d, index %d", i,
		       inventory[i].index);
	msg_print(tmp2);
      }
    }
  }

  if (redraw) {
    restore_screen();
    redraw = FALSE;
    }

  if (!flag) /* if flag still false, then user aborted.  So we don't charge
  		him a turn. -CFT */
    free_turn_flag = TRUE; 
}

/* Examine a Book					-RAK-	*/
static void examine_book()
{
  int32u j1;
  int32u j2;
  int i, k, item_val, flag;
  int spell_index[63];
  register inven_type *i_ptr;
  register spell_type *s_ptr;
  int first_spell;
  
  if (!find_range(TV_MAGIC_BOOK, TV_PRAYER_BOOK, &i, &k))
    msg_print("You are not carrying any books.");
  else if (py.flags.blind > 0)
    msg_print("You can't see to read your spell book!");
  else if (no_light())
    msg_print("You have no light to read by.");
  else if (py.flags.confused > 0)
    msg_print("You are too confused.");
  else if (get_item(&item_val, "Which Book?", i, k, 0))
    {
      flag = TRUE;
      i_ptr = &inventory[item_val];
      if (class[py.misc.pclass].spell == MAGE)
	{
	  if (i_ptr->tval != TV_MAGIC_BOOK)
	    flag = FALSE;
	}
      else if (class[py.misc.pclass].spell == PRIEST)
	{
	  if (i_ptr->tval != TV_PRAYER_BOOK)
	    flag = FALSE;
	}
      else
	flag = FALSE;

      if (!flag)
	msg_print("You do not understand the language.");
      else
	{
	  i = 0;
	  j1 = (int32u) inventory[item_val].flags;
	  first_spell = bit_pos(&j1); /* check which spell was first */
	  j1 = (int32u) inventory[item_val].flags; /* restore j1 value */
	  while (j1)
	    {
	      k = bit_pos(&j1);
	      s_ptr = &magic_spell[py.misc.pclass-1][k];
	      if (s_ptr->slevel < 99)
		{
		  spell_index[i] = k;
		  i++;
		}
	    }
	  j2 = (int32u) inventory[item_val].flags2;
	  if (first_spell == -1) { /* if none from other set of flags */
	    first_spell = 32 + bit_pos (&j2); /* get 1st spell # */
	    j2 = (int32u) inventory[item_val].flags2; /* and restore j2 */
	    }
	  while (j2)
	    {
	      k = bit_pos(&j2);
	      s_ptr = &magic_spell[py.misc.pclass-1][k+32];
	      if (s_ptr->slevel < 99)
		{
		  spell_index[i] = (k+32);
		  i++;
		}
	    }
	  save_screen();
	  print_spells(spell_index, i, TRUE, first_spell);
	  pause_line(0);
	  restore_screen();
	}
    }
}

/* Go up one level					-RAK-	*/
static void go_up()
{
  register cave_type *c_ptr;
  register int no_stairs = FALSE;

  c_ptr = &cave[char_row][char_col];
  if (c_ptr->tptr != 0)
    if (t_list[c_ptr->tptr].tval == TV_UP_STAIR)
      {
	if (dun_level == Q_PLANE) {
	  dun_level=0;
	  new_level_flag=TRUE;
	  msg_print("You enter an inter-dimensional staircase.");
	} else {
	  dun_level--;
	  new_level_flag = TRUE;
	  if (dun_level>0) create_down_stair = TRUE;
	  msg_print("You enter a maze of up staircases.");
	}
      }
    else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs)
    {
      msg_print("I see no up staircase here.");
      free_turn_flag = TRUE;
    }
}


/* Go down one level					-RAK-	*/
static void go_down()
{
  register cave_type *c_ptr;
  register int no_stairs = FALSE;

  c_ptr = &cave[char_row][char_col];
  if (c_ptr->tptr != 0)
    if (t_list[c_ptr->tptr].tval == TV_DOWN_STAIR)
      {
	if (dun_level == Q_PLANE) {
	  dun_level=0;
	  new_level_flag=TRUE;
	  msg_print("You enter an inter-dimensional staircase.");
	} else {
	  dun_level++;
	  new_level_flag = TRUE;
	  create_up_stair = TRUE;
	  msg_print("You enter a maze of down staircases.");
	}
      }
    else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs)
    {
      msg_print("I see no down staircase here.");
      free_turn_flag = TRUE;
    }
}


/* Jam a closed door					-RAK-	*/
static void jamdoor()
{
  int y, x, dir, i, j;
  register cave_type *c_ptr;
  register inven_type *t_ptr, *i_ptr;
  char tmp_str[80];
  int temp = target_mode; /* targetting will screw up get_dir.. -CFT */
  
  free_turn_flag = TRUE;
  y = char_row;
  x = char_col;
  target_mode = FALSE; /* turn off target mode, restore later */
  if (get_dir(NULL, &dir))
    {
      (void) mmove(dir, &y, &x);
      c_ptr = &cave[y][x];
      if (c_ptr->tptr != 0)
	{
	  t_ptr = &t_list[c_ptr->tptr];
	  if (t_ptr->tval == TV_CLOSED_DOOR)
	    if (c_ptr->cptr == 0)
	      {
		if (find_range(TV_SPIKE, TV_NEVER, &i, &j))
		  {
		    free_turn_flag = FALSE;
		    count_msg_print("You jam the door with a spike.");
		    if (t_ptr->p1 > 0)
		      t_ptr->p1 = -t_ptr->p1;	/* Make locked to stuck. */
		    /* Successive spikes have a progressively smaller effect.
		       Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
		    t_ptr->p1 -= 1 + 190 / (10 - t_ptr->p1);
		    i_ptr = &inventory[i];
		    if (i_ptr->number > 1)
		      {
		        i_ptr->number--;
			inven_weight -= i_ptr->weight;
		      }
		    else
		      inven_destroy(i);
		  }
		else
		  msg_print("But you have no spikes.");
	      }
	    else
	      {
		free_turn_flag = FALSE;
		(void) sprintf(tmp_str, "The %s is in your way!",
			       c_list[m_list[c_ptr->cptr].mptr].name);
		msg_print(tmp_str);
	      }
	  else if (t_ptr->tval == TV_OPEN_DOOR)
	    msg_print("The door must be closed first.");
	  else
	    msg_print("That isn't a door!");
	}
      else
	msg_print("That isn't a door!");
    }
  target_mode = temp;
}


/* Refill the players lamp				-RAK-	*/
static void refill_lamp()
{
  int i, j;
  register int k;
  register inven_type *i_ptr;

  free_turn_flag = TRUE;
  k = inventory[INVEN_LIGHT].subval;
  if (k != 0)
    msg_print("But you are not using a lamp.");
  else if (!find_range(TV_FLASK, TV_NEVER, &i, &j))
    msg_print("You have no oil.");
  else
    {
      free_turn_flag = FALSE;
      i_ptr = &inventory[INVEN_LIGHT];
      i_ptr->p1 += inventory[i].p1;
      if (i_ptr->p1 > OBJ_LAMP_MAX)
	{
	  i_ptr->p1 = OBJ_LAMP_MAX;
	  msg_print ("Your lamp overflows, spilling oil on the ground.");
	  msg_print("Your lamp is full.");
	}
      else if (i_ptr->p1 > OBJ_LAMP_MAX/2)
	msg_print ("Your lamp is more than half full.");
      else if (i_ptr->p1 == OBJ_LAMP_MAX/2)
	msg_print ("Your lamp is half full.");
      else
	msg_print ("Your lamp is less than half full.");
      desc_remain(i);
      inven_destroy(i);
    }
}

int is_quest(level)
  int level;
{
  if (level == Q_PLANE)
    return FALSE;
  if (quests[SAURON_QUEST] && (level==quests[SAURON_QUEST]))
    return TRUE;
  return FALSE;
}

static int regen_monsters() {
  register int i;
  int frac;

  for (i=0; i<MAX_MALLOC; i++) {
    if (m_list[i].hp >= 0) {
      if (m_list[i].maxhp == 0){ /* then we're just going to fix it! -CFT */
        if ((c_list[m_list[i].mptr].cdefense & MAX_HP) || be_nasty)
	  m_list[i].maxhp = max_hp(c_list[m_list[i].mptr].hd);
        else
	  m_list[i].maxhp = pdamroll(c_list[m_list[i].mptr].hd);
        }
      if (m_list[i].hp < m_list[i].maxhp)
	m_list[i].hp += ((frac=2*m_list[i].maxhp/100)>0)? frac : 1;
      if (m_list[i].hp > m_list[i].maxhp)
	  m_list[i].hp=m_list[i].maxhp;
      } /* if hp >= 0 */
    } /* for loop */
}

