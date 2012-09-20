/* sets.c: code to emulate the original Pascal sets

   Copyright (c) 1989 James E. Wilson

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"

int set_room(element)
register int element;
{
  if ((element == DARK_FLOOR) || (element == LIGHT_FLOOR) ||
      (element == NT_DARK_FLOOR) || (element == NT_LIGHT_FLOOR))
    return(TRUE);
  return(FALSE);
}

int set_corr(element)
register int element;
{
  if (element == CORR_FLOOR || element == BLOCKED_FLOOR)
    return(TRUE);
  return(FALSE);
}

int set_floor(element)
int element;
{
  if (element <= MAX_CAVE_FLOOR)
    return(TRUE);
  else
    return(FALSE);
}

int set_corrodes(inven_type *e) /* changed -CFT */
{
  int element = e->tval;

  switch(element)
    {
    case TV_SWORD: case TV_HELM: case TV_SHIELD: case TV_HARD_ARMOR:
      if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_ACID) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_ACID)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
    case TV_WAND:
      return(TRUE);
    }
  return(FALSE);
}


int set_flammable(inven_type *e) /* changed -CFT */
{
  int element = e->tval;
  
  switch(element)
    {
    case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
    case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
      if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_FIRE) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_FIRE)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
    case TV_STAFF: case TV_SCROLL1: case TV_SCROLL2:
      return(TRUE);
    case TV_FLASK:
      return(TRUE); /* oil burns... -CFT */
    case TV_LIGHT:
      if (e->subval >= 192) /* only torches... -CFT */
        return(TRUE);
      else
        return(FALSE);
    }
  return(FALSE);
}


int set_frost_destroy(inven_type *e) /* changed -CFT */
{
  int element = e->tval;
  
  if ((element == TV_POTION1) || (element == TV_POTION2)
      || (element == TV_FLASK))
    return(TRUE);
  return(FALSE);
}


int set_acid_affect(inven_type *e) /* changed -CFT */
{
  int element = e->tval;

  switch(element)
    {
    case TV_BOLT: case TV_ARROW:
    case TV_BOW: case TV_HAFTED: case TV_POLEARM: case TV_BOOTS:
    case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
      if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_ACID) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_ACID)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
    case TV_MISC: case TV_CHEST:
      return(TRUE);
    }
  return(FALSE);
}


int set_lightning_destroy(inven_type *e) /* changed -CFT */
{
  int element = e->tval;

  switch(element)
    {
    case TV_RING:
      if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_LIGHT) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_LIGHT)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
   case TV_WAND: 
      return(TRUE);
    }
    return(FALSE);
}


/*ARGSUSED*/	/* to shut up lint about unused argument */
int set_null(inven_type *e)
{
  return(FALSE);
}


int set_acid_destroy(inven_type *e) /* changed -CFT */
{
  int element = e->tval;

  switch(element)
    {
    case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
    case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_HELM:
    case TV_SHIELD: case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
      if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_ACID) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_ACID)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
    case TV_SCROLL1: case TV_SCROLL2: case TV_FOOD: case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR: case TV_STAFF:
      return(TRUE);
    }
  return(FALSE);
}


int set_fire_destroy(inven_type *e) /* changed -CFT */
{
  int element = e->tval;

  switch(element)
    {
    case TV_ARROW: case TV_BOW: case TV_HAFTED: case TV_POLEARM:
    case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_SOFT_ARMOR:
       if ((e->flags2 & TR_ARTIFACT)   /* shouldn't kill artifacts -CFT */
	|| (e->flags & TR_RES_FIRE) /* can't check outside, because flags */
	|| (e->flags2 & TR_IM_FIRE)) /* used differently in potions/etc */
        return(FALSE);
      return(TRUE);
    case TV_STAFF: case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1:
    case TV_POTION2: case TV_FLASK: case TV_FOOD: case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
      return(TRUE);
    case TV_LIGHT:
      if (e->subval >= 192) /* only torches... -CFT */
        return(TRUE);
      else
        return(FALSE);
    }
  return(FALSE);
}


int general_store(element)
int element;
{
  switch(element)
    {
    case TV_DIGGING: case TV_BOOTS: case TV_CLOAK: case TV_FOOD:
    case TV_FLASK: case TV_LIGHT: case TV_SPIKE:
      return(TRUE);
    }
  return(FALSE);
}


int armory(element)
int element;
{
  switch(element)
    {
    case TV_BOOTS: case TV_GLOVES: case TV_HELM: case TV_SHIELD:
    case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
      return(TRUE);
    }
  return(FALSE);
}


int weaponsmith(element)
int element;
{
  switch(element)
    {
    case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW: case TV_BOW:
    case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
      return(TRUE);
    }
  return(FALSE);
}


int temple(element)
int element;
{
  switch(element)
    {
    case TV_HAFTED: case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1:
    case TV_POTION2: case TV_PRAYER_BOOK:
      return(TRUE);
    }
  return(FALSE);
}


int alchemist(element)
int element;
{
  switch(element)
    {
    case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1: case TV_POTION2:
      return(TRUE);
    }
  return(FALSE);
}


int magic_shop(element)
int element;
{
  switch(element)
    {
    case TV_AMULET: case TV_RING: case TV_STAFF: case TV_WAND:
    case TV_SCROLL1: case TV_SCROLL2: case TV_POTION1: case TV_POTION2:
    case TV_MAGIC_BOOK: case TV_ROD:
      return(TRUE);
    }
  return(FALSE);
}

int blackmarket(element)
int element;
{
  return(TRUE);
}

int home(element)
int element;
{
  return(TRUE);
}
