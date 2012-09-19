/* store1.c: store code, updating store inventory, pricing objects

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#ifndef ATARIST_MWC
#include <string.h>
#endif
#else
#include <strings.h>
#endif

#if defined(LINT_ARGS)
static void insert_store(int, int, int32, struct inven_type *);
static void store_create(int);
#else
static void insert_store();
static void store_create();
#endif

extern int is_home;

/* Returns the value for any given object		-RAK-	*/
int32 item_value(i_ptr)
register inven_type *i_ptr;
{
  register int32 value;

  value = i_ptr->cost;
  /* don't purchase known cursed items */
  if (i_ptr->ident & ID_DAMD)
    value = 0;
  else if (((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD)) ||
	   ((i_ptr->tval >= TV_BOOTS) && (i_ptr->tval <= TV_SOFT_ARMOR)))
    {	/* Weapons and armor	*/
      if (!known2_p(i_ptr))
	value = object_list[i_ptr->index].cost;
      else if ((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD))
	{
	  if (i_ptr->tohit < 0)
	    value = 0;
	  else if (i_ptr->todam < 0)
	    value = 0;
	  else if (i_ptr->toac < 0)
	    value = 0;
	  else
	    value = i_ptr->cost+(i_ptr->tohit+i_ptr->todam+i_ptr->toac)*100;
	}
      else
	{
	  if (i_ptr->toac < 0)
	    value = 0;
	  else
	    value = i_ptr->cost+i_ptr->toac*100;
	}
    }
  else if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_SPIKE))
    {	/* Ammo			*/
      if (!known2_p(i_ptr))
	value = object_list[i_ptr->index].cost;
      else
	{
	  if (i_ptr->tohit < 0)
	    value = 0;
	  else if (i_ptr->todam < 0)
	    value = 0;
	  else if (i_ptr->toac < 0)
	    value = 0;
	  else
	    /* use 5, because missiles generally appear in groups of 20,
	       so 20 * 5 == 100, which is comparable to weapon bonus above */
	    value = i_ptr->cost+(i_ptr->tohit+i_ptr->todam+i_ptr->toac)*5;
	}
    }
  else if ((i_ptr->tval == TV_SCROLL1) || (i_ptr->tval == TV_SCROLL2) ||
	   (i_ptr->tval == TV_POTION1) || (i_ptr->tval == TV_POTION2))
    {	/* Potions, Scrolls, and Food	*/
      if (!known1_p(i_ptr))
	value = 20;
    }
  else if (i_ptr->tval == TV_FOOD)
    {
      if ((i_ptr->subval < (ITEM_SINGLE_STACK_MIN + MAX_MUSH))
	  && !known1_p(i_ptr))
	value = 1;
    }
  else if ((i_ptr->tval == TV_AMULET) || (i_ptr->tval == TV_RING))
    {	/* Rings and amulets	*/
      if (!known1_p(i_ptr))
	/* player does not know what type of ring/amulet this is */
	value = 45;
      else if (!known2_p(i_ptr))
	/* player knows what type of ring, but does not know whether it is
	   cursed or not, if refuse to buy cursed objects here, then
	   player can use this to 'identify' cursed objects */
	value = object_list[i_ptr->index].cost;
    }
  else if ((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND))
    {	/* Wands and staffs*/
      if (!known1_p(i_ptr))
	{
	  if (i_ptr->tval == TV_WAND)
	    value = 50;
	  else
	    value = 70;
	}
      else if (known2_p(i_ptr))
	value = i_ptr->cost + (i_ptr->cost / 20) * i_ptr->p1;
    }
  /* picks and shovels */
  else if (i_ptr->tval == TV_DIGGING)
    {
      if (!known2_p(i_ptr))
	value = object_list[i_ptr->index].cost;
      else
	{
	  if (i_ptr->p1 < 0)
	    value = 0;
	  else
	    {
	      /* some digging tools start with non-zero p1 values, so only
		 multiply the plusses by 100, make sure result is positive */
	      value = i_ptr->cost
		+ (i_ptr->p1 - object_list[i_ptr->index].p1) * 100;
	      if (value < 0)
		value = 0;
	    }
	}
    }
  /* multiply value by number of items if it is a group stack item */
  if (i_ptr->subval > ITEM_GROUP_MIN) /* do not include torches here */
    value = value * i_ptr->number;
  return(value);
}


/* Asking price for an item				-RAK-	*/
int32 sell_price(snum, max_sell, min_sell, item)
int snum;
int32 *max_sell, *min_sell;
inven_type *item;
{
  register int32 i;
  register store_type *s_ptr;

  s_ptr = &store[snum];
  i = item_value(item);
  /* check item->cost in case it is cursed, check i in case it is damaged */
  if ((item->cost > 0) && (i > 0))
    {
      i = i * rgold_adj[owners[s_ptr->owner].owner_race][py.misc.prace] / 100;
      if (i < 1)  i = 1;
      *max_sell = i * owners[s_ptr->owner].max_inflate / 100;
      *min_sell = i * owners[s_ptr->owner].min_inflate / 100;
      if (snum==6) *max_sell*=2, *min_sell*=2;
      if (min_sell > max_sell)	min_sell = max_sell;
      return(i);
    }
  else
    /* don't let the item get into the store inventory */
    return(0);
}


/* Check to see if he will be carrying too many objects	-RAK-	*/
int store_check_num(t_ptr, store_num)
inven_type *t_ptr;
int store_num;
{
  register int store_check, i;
  register store_type *s_ptr;
  register inven_type *i_ptr;

  store_check = FALSE;
  s_ptr = &store[store_num];
  if (s_ptr->store_ctr < STORE_INVEN_MAX)
    store_check = TRUE;
  else if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN)
    for (i = 0; i < s_ptr->store_ctr; i++)
      {
	i_ptr = &s_ptr->store_inven[i].sitem;
	/* note: items with subval of gte ITEM_SINGLE_STACK_MAX only stack
	   if their subvals match */
	if (i_ptr->tval == t_ptr->tval && i_ptr->subval == t_ptr->subval
	    && ((int)i_ptr->number + (int)t_ptr->number < 256)
	    && (t_ptr->subval < ITEM_GROUP_MIN
		|| (i_ptr->p1 == t_ptr->p1)))
	  store_check = TRUE;
      }
  return(store_check);
}


/* Insert INVEN_MAX at given location	*/
static void insert_store(store_num, pos, icost, i_ptr)
register int pos;
int store_num;
int32 icost;
inven_type *i_ptr;
{
  register int i;
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  for (i = s_ptr->store_ctr-1; i >= pos; i--)
    s_ptr->store_inven[i+1] = s_ptr->store_inven[i];
  s_ptr->store_inven[pos].sitem = *i_ptr;
  s_ptr->store_inven[pos].scost = -icost;
  s_ptr->store_ctr++;
}


/* Add the item in INVEN_MAX to stores inventory.	-RAK-	*/
void store_carry(store_num, ipos, t_ptr)
int store_num;
int *ipos;
inven_type *t_ptr;
{
  int item_num, item_val, flag;
  register int typ, subt;
  int32 icost, dummy;
  register inven_type *i_ptr;
  register store_type *s_ptr;

  *ipos = -1;
  if (sell_price(store_num, &icost, &dummy, t_ptr) > 0 || is_home)
    {
      s_ptr = &store[store_num];
      item_val = 0;
      item_num = t_ptr->number;
      flag = FALSE;
      typ  = t_ptr->tval;
      subt = t_ptr->subval;
      do
	{
	  i_ptr = &s_ptr->store_inven[item_val].sitem;
	  if (typ == i_ptr->tval)
	    {
	      if (subt == i_ptr->subval && /* Adds to other item	*/
		  subt >= ITEM_SINGLE_STACK_MIN
		  && (subt < ITEM_GROUP_MIN || i_ptr->p1 == t_ptr->p1))
		{
		  *ipos = item_val;
		  i_ptr->number += item_num;
		  /* must set new scost for group items, do this only for items
		     strictly greater than group_min, not for torches, this
		     must be recalculated for entire group */
		  if (subt > ITEM_GROUP_MIN)
		    {
		      (void) sell_price (store_num, &icost, &dummy, i_ptr);
		      s_ptr->store_inven[item_val].scost = -icost;
		    }
		  /* must let group objects (except torches) stack over 24
		     since there may be more than 24 in the group */
		  else if (i_ptr->number > 24)
		    i_ptr->number = 24;
		  flag = TRUE;
		}
	    }
	  else if (typ > i_ptr->tval)
	    {		/* Insert into list		*/
	      insert_store(store_num, item_val, icost, t_ptr);
	      flag = TRUE;
	      *ipos = item_val;
	    }
	  item_val++;
	}
      while ((item_val < s_ptr->store_ctr) && (!flag));
      if (!flag)	/* Becomes last item in list	*/
	{
	  insert_store(store_num, (int)s_ptr->store_ctr, icost, t_ptr);
	  *ipos = s_ptr->store_ctr - 1;
	}
    }
}

/* Destroy an item in the stores inventory.  Note that if	*/
/* "one_of" is false, an entire slot is destroyed	-RAK-	*/
void store_destroy(store_num, item_val, one_of)
int store_num, item_val;
int one_of;
{
  register int j, number;
  register store_type *s_ptr;
  register inven_type *i_ptr;

  s_ptr = &store[store_num];
  i_ptr = &s_ptr->store_inven[item_val].sitem;

  /* for single stackable objects, only destroy one half on average,
     this will help ensure that general store and alchemist have 
     reasonable selection of objects */
  if ((i_ptr->subval >= ITEM_SINGLE_STACK_MIN) &&
      (i_ptr->subval <= ITEM_SINGLE_STACK_MAX))
    {
      if (one_of)
	number = 1;
      else
	number = randint((int)i_ptr->number);
    }
  else
    number = i_ptr->number;

  if (number != i_ptr->number)
    i_ptr->number -= number;
  else
    {
      for (j = item_val; j < s_ptr->store_ctr-1; j++)
	s_ptr->store_inven[j] = s_ptr->store_inven[j+1];
      invcopy(&s_ptr->store_inven[s_ptr->store_ctr-1].sitem, OBJ_NOTHING);
      s_ptr->store_inven[s_ptr->store_ctr-1].scost = 0;
      s_ptr->store_ctr--;
    }
}


/* Initializes the stores with owners			-RAK-	*/
void store_init()
{
  register int i, j, k;
  register store_type *s_ptr;

  i = MAX_OWNERS / MAX_STORES;
  for (j = 0; j < MAX_STORES; j++)
    {
      s_ptr = &store[j];
      s_ptr->owner = MAX_STORES*(randint(i)-1) + j;
      s_ptr->insult_cur = 0;
      s_ptr->store_open = 0;
      s_ptr->store_ctr	= 0;
      s_ptr->good_buy = 0;
      s_ptr->bad_buy = 0;
      for (k = 0; k < STORE_INVEN_MAX; k++)
	{
	  invcopy(&s_ptr->store_inven[k].sitem, OBJ_NOTHING);
	  s_ptr->store_inven[k].scost = 0;
	}
    }
}


/* Creates an item and inserts it into store's inven	-RAK-	*/
static void store_create(store_num)
int store_num;
{
  register int i, tries;
  int cur_pos, dummy;
  register store_type *s_ptr;
  register inven_type *t_ptr;

  tries = 0;
  cur_pos = popt();
  s_ptr = &store[store_num];
  do
    {
      if (store_num!=6) {
	i = store_choice[store_num][randint(STORE_CHOICES)-1];
	invcopy(&t_list[cur_pos], i);
	magic_treasure(cur_pos, OBJ_TOWN_LEVEL, FALSE, TRUE);
	t_ptr = &t_list[cur_pos];
	if (store_check_num(t_ptr, store_num))
	  {
	    if ((t_ptr->cost > 0) &&	/* Item must be good	*/
		(t_ptr->cost < owners[s_ptr->owner].max_cost))
	      {
		/* equivalent to calling ident_spell(), except will not
		  change the object_ident array */
		store_bought(t_ptr);
		special_offer(t_ptr);
		store_carry(store_num, &dummy, t_ptr);
		tries = 10;
	      }
	  }
	tries++;
      } else {
	i = get_obj_num(40, FALSE);
	invcopy(&t_list[cur_pos], i);
	magic_treasure(cur_pos, 40, FALSE, TRUE);
	t_ptr = &t_list[cur_pos];
	if (store_check_num(t_ptr, store_num))
	  {
	    if (t_ptr->cost > 0)	/* Item must be good	*/
	      {
		/* equivalent to calling ident_spell(), except will not
		  change the object_ident array */
		store_bought(t_ptr);
		special_offer(t_ptr);
		store_carry(store_num, &dummy, t_ptr);
		tries = 10;
	      }
	  }
	tries++;
      }	
    }
  while (tries <= 3);
  pusht((int8u)cur_pos);
}

int special_offer(i_ptr) 
  inven_type *i_ptr;
{
  if (randint(30)==1) {
    i_ptr->cost = (i_ptr->cost*3)/4;
    if (i_ptr->cost < 1) i_ptr->cost=1;
    inscribe(i_ptr, "25% discount");
  } else if (randint(150)==1) {
    i_ptr->cost /= 2;
    if (i_ptr->cost < 1) i_ptr->cost=1;
    inscribe(i_ptr, "50% discount");
  } else if (randint(300)==1) {
    i_ptr->cost /= 4;
    if (i_ptr->cost < 1) i_ptr->cost=1;
    inscribe(i_ptr, "75% discount");
  } else if (randint(500)==1) {
    i_ptr->cost /= 10;
    if (i_ptr->cost < 1) i_ptr->cost=1;
    inscribe(i_ptr, "to clear");
  }
}   

/* Initialize and up-keep the store's inventory.		-RAK-	*/
void store_maint()
{
  register int i, j;
  register store_type *s_ptr;

  for (i = 0; i < (MAX_STORES-1); i++)
    {
      s_ptr = &store[i];
      s_ptr->insult_cur = 0;
      if (s_ptr->store_ctr >= STORE_MIN_INVEN)
	{
	  j = randint(STORE_TURN_AROUND);
	  if (s_ptr->store_ctr >= STORE_MAX_INVEN)
	    j += 1 + s_ptr->store_ctr - STORE_MAX_INVEN;
	  while (--j >= 0)
	    store_destroy(i, randint((int)s_ptr->store_ctr)-1, FALSE);
	}

      if (s_ptr->store_ctr <= STORE_MAX_INVEN)
	{
	  j = randint(STORE_TURN_AROUND);
	  if (s_ptr->store_ctr < STORE_MIN_INVEN)
	    j += STORE_MIN_INVEN - s_ptr->store_ctr;
	  while (--j >= 0)
	    store_create(i);
	}
    }
}

/* eliminate need to bargain if player has haggled well in the past   -DJB- */
int noneedtobargain(store_num, minprice)
int store_num;
int32 minprice;
{
  register int flagnoneed;
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  flagnoneed = ((s_ptr->good_buy == MAX_SHORT)
		|| ((s_ptr->good_buy > 2 * s_ptr->bad_buy + 3) &&
		    (minprice < 5000)));

  return (flagnoneed);
}


/* update the bargin info					-DJB- */
void updatebargain(store_num, price, minprice)
int store_num;
int32 price, minprice;
{
  register store_type *s_ptr;

  s_ptr = &store[store_num];
  if ((minprice > 9) && (minprice < 1000))
    if (price == minprice)
      {
	if (s_ptr->good_buy < MAX_SHORT)
	  s_ptr->good_buy++;
      }
    else
      {
	if (s_ptr->bad_buy < MAX_SHORT)
	  s_ptr->bad_buy++;
      }
}
