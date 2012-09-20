/* desc.c: handle object descriptions, mostly string handling code
 
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
static void randes(void);
static void rantitle(char *);
static void unsample(char *);
#else
static void unsample(ARG_INV_PTR);
#endif

char titles[MAX_TITLES][10];

/* Object descriptor routines					*/

int is_a_vowel(ch)
char ch;
{
  switch(ch)
    {
    case 'a': case 'e': case 'i': case 'o': case 'u':
    case 'A': case 'E': case 'I': case 'O': case 'U':
      return(TRUE);
    default:
      return(FALSE);
    }
}

/* Initialize all Potions, wands, staves, scrolls, etc.	*/
void magic_init()
{
  register int h, i, j, k;
  register char *tmp;
  vtype string;
#ifdef TC_COLOR
  int8u ctmp;
#endif

  set_seed(randes_seed);

  /* The first 3 entries for colors are fixed, (slime & apple juice, water) */
  for (i = 3; i < MAX_COLORS; i++)
    {
      j = randint(MAX_COLORS - 3) + 2;
      tmp = colors[i];
      colors[i] = colors[j];
      colors[j] = tmp;
#ifdef TC_COLOR
      ctmp = tccolors[i];
      tccolors[i] = tccolors[j];
      tccolors[j] = ctmp;
#endif      
    }
  for (i = 0; i < MAX_WOODS; i++)
    {
      j = randint(MAX_WOODS) - 1;
      tmp = woods[i];
      woods[i] = woods[j];
      woods[j] = tmp;
#ifdef TC_COLOR
      ctmp = tcwoods[i];
      tcwoods[i] = tcwoods[j];
      tcwoods[j] = ctmp;
#endif      
    }
  for (i = 0; i < MAX_METALS; i++)
    {
      j = randint(MAX_METALS) - 1;
      tmp = metals[i];
      metals[i] = metals[j];
      metals[j] = tmp;
#ifdef TC_COLOR
      ctmp = tcmetals[i];
      tcmetals[i] = tcmetals[j];
      tcmetals[j] = ctmp;
#endif      
    }
  for (i = 0; i < MAX_ROCKS; i++)
    {
      j = randint(MAX_ROCKS) - 1;
      tmp = rocks[i];
      rocks[i] = rocks[j];
      rocks[j] = tmp;
#ifdef TC_COLOR
      ctmp = tcrocks[i];
      tcrocks[i] = tcrocks[j];
      tcrocks[j] = ctmp;
#endif      
    }
  for (i = 0; i < MAX_AMULETS; i++)
    {
      j = randint(MAX_AMULETS) - 1;
      tmp = amulets[i];
      amulets[i] = amulets[j];
      amulets[j] = tmp;
#ifdef TC_COLOR
      ctmp = tcamulets[i];
      tcamulets[i] = tcamulets[j];
      tcamulets[j] = ctmp;
#endif      
    }
  for (i = 0; i < MAX_MUSH; i++)
    {
      j = randint(MAX_MUSH) - 1;
      tmp = mushrooms[i];
      mushrooms[i] = mushrooms[j];
      mushrooms[j] = tmp;
#ifdef TC_COLOR
      ctmp = tcmushrooms[i];
      tcmushrooms[i] = tcmushrooms[j];
      tcmushrooms[j] = ctmp;
#endif      
    }
  for (h = 0; h < MAX_TITLES; h++)
    {
      string[0] = '\0';
      k = randint(2) + 1;
      for (i = 0; i < k; i++)
	{
	  for (j = randint(2); j > 0; j--)
	    (void) strcat(string, syllables[randint(MAX_SYLLABLES) - 1]);
	  if (i < k-1)
	    (void) strcat(string, " ");
	}
      if (string[8] == ' ')
	string[8] = '\0';
      else
	string[9] = '\0';
      (void) strcpy(titles[h], string);
    }
  reset_seed();
}

int16 object_offset(t_ptr)
inven_type *t_ptr;
{
  switch (t_ptr->tval)
    {
    case TV_ROD:        return(7); /* -CFT */
    case TV_AMULET:	return(0);
    case TV_RING:	return(1);
    case TV_STAFF:	return(2);
    case TV_WAND:	return(3);
    case TV_SCROLL1:
    case TV_SCROLL2:	return(4);
    case TV_POTION1:
    case TV_POTION2:	return(5);
    case TV_FOOD:
      if ((t_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1)) < MAX_MUSH)
	return(6);
      return(-1);
    default:  return(-1);
    }
}

/* Remove "Secret" symbol for identity of object			*/
void known1(i_ptr)
inven_type *i_ptr;
{
  int16 offset;
  int8u indexx;

  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident[offset + indexx] |= OD_KNOWN1;
  /* clear the tried flag, since it is now known */
  object_ident[offset + indexx] &= ~OD_TRIED;
}

int known1_p(i_ptr)
inven_type *i_ptr;
{
  int16 offset;
  int8u indexx;

  if ((offset = object_offset(i_ptr)) < 0)
    return FALSE;
  if (store_bought_p(i_ptr))
    return OD_KNOWN1;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  return(object_ident[offset + indexx] & OD_KNOWN1);
}

/* Remove "Secret" symbol for identity of plusses			*/
void known2(i_ptr)
inven_type *i_ptr;
{
  unsample(i_ptr);
  i_ptr->ident |= ID_KNOWN2;
}

int known2_p(i_ptr)
inven_type *i_ptr;
{
  return (i_ptr->ident & ID_KNOWN2);
}  

void clear_known2(i_ptr)
inven_type *i_ptr;
{
  i_ptr->ident &= ~ID_KNOWN2;
}

void clear_empty(i_ptr)
inven_type *i_ptr;
{
  i_ptr->ident &= ~ID_EMPTY;
}

void store_bought(i_ptr)
inven_type *i_ptr;
{
  i_ptr->ident |= ID_STOREBOUGHT;
  known2(i_ptr);
}

int store_bought_p(i_ptr)
inven_type *i_ptr;
{
  return (i_ptr->ident & ID_STOREBOUGHT);
}

/*	Remove an automatically generated inscription.	-CJS- */
static void unsample(i_ptr)
inven_type *i_ptr;
{
  int16 offset;
  int8u indexx;

  /* used to clear ID_DAMD flag, but I think it should remain set */
  i_ptr->ident &= ~(ID_MAGIK|ID_EMPTY);
  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident[offset + indexx] &= ~OD_TRIED;
}

/* unquote() is no longer needed */

/* Somethings been sampled -CJS- */
void sample (i_ptr)
inven_type *i_ptr;
{
  int16 offset;
  int8u indexx;

  if ((offset = object_offset(i_ptr)) < 0)
    return;
  offset <<= 6;
  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  object_ident[offset + indexx] |= OD_TRIED;
}

/* Somethings been identified					*/
/* extra complexity by CJS so that it can merge store/dungeon objects
   when appropriate */
void identify(item)
int *item;
{
  register int i, x1, x2;
  int j;
  register inven_type *i_ptr, *t_ptr;

  i_ptr = &inventory[*item];

  if ((i_ptr->flags & TR_CURSED) && (i_ptr->tval != TV_MAGIC_BOOK) &&
      (i_ptr->tval != TV_PRAYER_BOOK))
    add_inscribe(i_ptr, ID_DAMD);

  if (!known1_p(i_ptr))
    {
      known1(i_ptr);
      x1 = i_ptr->tval;
      x2 = i_ptr->subval;
      if (x2 < ITEM_SINGLE_STACK_MIN || x2 >= ITEM_GROUP_MIN)
	/* no merging possible */
	;
      else
	for (i = 0; i < inven_ctr; i++)
	  {
	    t_ptr = &inventory[i];
	    if (t_ptr->tval == x1 && t_ptr->subval == x2 && i != *item
		&& ((int)t_ptr->number + (int)i_ptr->number < 256))
	      {
		/* make *item the smaller number */
		if (*item > i)
		  {
		    j = *item; *item = i; i = j;
		  }
	  msg_print ("You combine similar objects from the shop and dungeon.");

		inventory[*item].number += inventory[i].number;
		inven_ctr--;
		for (j = i; j < inven_ctr; j++)
		  inventory[j] = inventory[j+1];
		invcopy(&inventory[j], OBJ_NOTHING);
	      }
	  }
    }
}

/* If an object has lost magical properties,
 * remove the appropriate portion of the name.	       -CJS-
 */
void unmagic_name(i_ptr)
inven_type *i_ptr;
{
  i_ptr->name2 = SN_NULL;
}

/* defines for p1_use, determine how the p1 field is printed */
#define IGNORED  0
#define CHARGES  1
#define PLUSSES  2
#define LIGHT    3
#define FLAGS    4
#define Z_PLUSSES 5

/* Returns a description of item for inventory			*/
/* pref indicates that there should be an article added (prefix) */
/* note that since out_val can easily exceed 80 characters, objdes must
   always be called with a bigvtype as the first paramter */
void objdes(out_val, i_ptr, pref)
char *out_val;
register inven_type *i_ptr;
int pref;
{
  /* base name, modifier string*/
  register char *basenm, *modstr;
  bigvtype tmp_val;
  vtype tmp_str, damstr;
  int indexx, p1_use, modify;

  indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);
  basenm = object_list[i_ptr->index].name;
  modstr = NULL;
  damstr[0] = '\0';
  p1_use = IGNORED;
  modify = (known1_p(i_ptr) ? FALSE : TRUE);
  switch(i_ptr->tval)
    {
    case  TV_MISC:
    case  TV_CHEST:
      break;
    case  TV_SLING_AMMO:
    case  TV_BOLT:
    case  TV_ARROW:
      (void) sprintf(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
      break;
    case  TV_LIGHT:
      p1_use = LIGHT;
      break;
    case  TV_SPIKE:
      break;
    case  TV_BOW:
      if (!strcmp("& Light Crossbow", object_list[i_ptr->index].name))	
	(void) strcpy(damstr, " (x3)");
      else if (!strcmp("& Heavy Crossbow", object_list[i_ptr->index].name))
	(void) strcpy(damstr, " (x4)");
      else if (!strcmp("& Sling", object_list[i_ptr->index].name))
	(void) strcpy(damstr, " (x2)");
      else if (!strcmp("& Short Bow", object_list[i_ptr->index].name))
	(void) strcpy(damstr, " (x2)");
      else if (!strcmp("& Long Bow", object_list[i_ptr->index].name))
	(void) strcpy(damstr, " (x3)");
      break;
    case  TV_HAFTED:
    case  TV_POLEARM:
    case  TV_SWORD:
      (void) sprintf(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
      p1_use = FLAGS;
      break;
    case  TV_DIGGING:
      p1_use = Z_PLUSSES;
      (void) sprintf(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
      break;
    case  TV_BOOTS:
    case  TV_GLOVES:
    case  TV_CLOAK:
    case  TV_HELM:
    case  TV_SHIELD:
    case  TV_HARD_ARMOR:
    case  TV_SOFT_ARMOR:
      break;
    case  TV_AMULET:
      if (modify)
	{
	  basenm = "& %s Amulet";	
	  modstr = amulets[indexx];
	}
      else
	basenm = "& Amulet";
      p1_use = PLUSSES;
      break;
    case  TV_RING:
      if (modify)
	{
	  basenm = "& %s Ring";	
	  modstr = rocks[indexx];
	}
      else
	basenm = "& Ring";
      p1_use = PLUSSES;
      break;
    case  TV_STAFF:
      if (modify)
	{
	  basenm = "& %s Staff";
	  modstr = woods[indexx];
	}
      else
	basenm = "& Staff";
      p1_use = CHARGES;
      break;
    case  TV_WAND:
      if (modify)
	{
	  basenm = "& %s Wand";		
	  modstr = metals[indexx];
	}
      else
	basenm = "& Wand";
      p1_use = CHARGES;
      break;
    case  TV_ROD:
      if (modify)
	{
	  basenm = "& %s Rod";		
	  modstr = metals[indexx];
	}
      else
	basenm = "& Rod";
      break;
    case  TV_SCROLL1:
    case  TV_SCROLL2:
      if (modify)
	{
	  basenm =  "& Scroll~ titled \"%s\"";
	  modstr = titles[indexx];
	}
      else
	basenm = "& Scroll~";
      break;
    case  TV_POTION1:
    case  TV_POTION2:
      if (modify)
	{
	  basenm = "& %s Potion~";
	  modstr = colors[indexx];
	}
      else
	basenm = "& Potion~";
      break;
    case  TV_FLASK:
      break;
    case  TV_FOOD:
      if (modify)
	{
	  if (indexx <= 15)
	    basenm = "& %s Mushroom~";
	  else if (indexx <= 20)
	    basenm = "& Hairy %s Mold~";
	  if (indexx <= 20)
	    modstr = mushrooms[indexx];
	}
      else
	if (indexx <= 15)
	  basenm = "& Mushroom~";
	else if (indexx <= 20)
	  basenm = "& Hairy Mold~";
      break;
    case  TV_MAGIC_BOOK:
      modstr = basenm;
      basenm = "& Book~ of Magic Spells %s";
      break;
    case  TV_PRAYER_BOOK:
      modstr = basenm;
      basenm = "& Holy Book~ of Prayers %s";
      break;
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
    case TV_SECRET_DOOR:
    case TV_RUBBLE:
      break;
    case TV_GOLD:
    case TV_INVIS_TRAP:
    case TV_VIS_TRAP:
    case TV_UP_STAIR:
    case TV_DOWN_STAIR:
      (void) strcpy(out_val, object_list[i_ptr->index].name);
      (void) strcat(out_val, ".");
      return;
    case TV_STORE_DOOR:
      (void) sprintf(out_val, "the entrance to the %s.",
		     object_list[i_ptr->index].name);
      return;
    default:
      (void) strcpy(out_val, "Error in objdes()");
      return;
    }
  if (modstr != NULL)
    (void) sprintf(tmp_val, basenm, modstr);
  else
    (void) strcpy(tmp_val, basenm);
  if (known1_p(i_ptr))
    {
      (void) strcat(tmp_val, " of ");
      (void) strcat(tmp_val, object_list[i_ptr->index].name);
    }
  if (i_ptr->number != 1)
    {
      insert_str(tmp_val, "ch~", "ches");
      insert_str(tmp_val, "~", "s");
    }
  else
    insert_str(tmp_val, "~", NULL);
  if (!pref)
    {
      if (!strncmp("some", tmp_val, 4))
	(void) strcpy(out_val, &tmp_val[5]);
      else if (tmp_val[0] == '&')
	/* eliminate the '& ' at the beginning */
	(void) strcpy(out_val, &tmp_val[2]);
      else
	(void) strcpy(out_val, tmp_val);
    }
  else
    {
      if (i_ptr->name2 != SN_NULL && known2_p(i_ptr))
	{
	  (void) strcat(tmp_val, " ");
	  (void) strcat(tmp_val, special_names[i_ptr->name2]);
	}
      if (damstr[0] != '\0')
	(void) strcat(tmp_val, damstr);
      if (known2_p(i_ptr))
	{
	  /* originally used %+d, but several machines don't support it */
	  if (i_ptr->ident & ID_SHOW_HITDAM)
	    (void) sprintf(tmp_str, " (%c%d,%c%d)",
			   (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit),
			   (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
	  else if (i_ptr->tohit != 0)
	    (void) sprintf(tmp_str, " (%c%d)",
			   (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit));
	  else if (i_ptr->todam != 0)
	    (void) sprintf(tmp_str, " (%c%d)",
			   (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
	  else
	    tmp_str[0] = '\0';
	  (void) strcat(tmp_val, tmp_str);
	}
      /* Crowns have a zero base AC, so make a special test for them. */
      if (i_ptr->ac != 0 || (i_ptr->tval == TV_HELM))
	{
	  (void) sprintf(tmp_str, " [%d", i_ptr->ac);
	  (void) strcat(tmp_val, tmp_str);
	  if (known2_p(i_ptr))
	    {
	      /* originally used %+d, but several machines don't support it */
	      (void) sprintf(tmp_str, ",%c%d",
			     (i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
	      (void) strcat(tmp_val, tmp_str);
	    }
	  (void) strcat(tmp_val, "]");
	}
      else if ((i_ptr->toac != 0) && known2_p(i_ptr))
	{
	  /* originally used %+d, but several machines don't support it */
	  (void) sprintf(tmp_str, " [%c%d]",
			 (i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
	  (void) strcat(tmp_val, tmp_str);
	}

      /* override defaults, check for p1 flags in the ident field */
      if (i_ptr->ident & ID_NOSHOW_P1)
	p1_use = IGNORED;
      else if (i_ptr->ident & ID_SHOW_P1)
	p1_use = Z_PLUSSES;
      tmp_str[0] = '\0';
      if ((p1_use==LIGHT) && !(i_ptr->flags2 & TR_ARTIFACT))
	(void) sprintf(tmp_str, " with %d turns of light", i_ptr->p1);
      else if (p1_use == IGNORED)
	;
      else if (known2_p(i_ptr))
	{
	  if (p1_use == Z_PLUSSES)
	  /* originally used %+d, but several machines don't support it */
	    (void) sprintf(tmp_str, " (%c%d)",
			   (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
	  else if (p1_use == CHARGES)
	    (void) sprintf(tmp_str, " (%d charges)", i_ptr->p1);
	  else if (i_ptr->p1 != 0)
	    {
	      if (p1_use == PLUSSES)
	        (void) sprintf(tmp_str, " (%c%d)",
			       (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
	      else if (p1_use == FLAGS)
		{
		  if (i_ptr->flags & TR_STEALTH)
		    (void) sprintf(tmp_str, " (%c%d to stealth)",
				   (i_ptr->p1 < 0) ? '-' : '+',abs(i_ptr->p1));
		}
	    }
	}
      (void) strcat(tmp_val, tmp_str);

      /* ampersand is always the first character */
      if (tmp_val[0] == '&')
	{
	  /* use &tmp_val[1], so that & does not appear in output */
	  if (i_ptr->number > 1)
	    (void) sprintf(out_val, "%d%s", (int)i_ptr->number, &tmp_val[1]);
	  else if (i_ptr->number < 1)
	    (void) sprintf(out_val, "%s%s", "no more", &tmp_val[1]);
	  else if (is_a_vowel(tmp_val[2]))
	    (void) sprintf(out_val, "an%s", &tmp_val[1]);
	  else
	    (void) sprintf(out_val, "a%s", &tmp_val[1]);
	}
      /* handle 'no more' case specially */
      else if (i_ptr->number < 1)
	{
	  /* check for "some" at start */
	  if (!strncmp("some", tmp_val, 4))
	    (void) sprintf(out_val, "no more %s", &tmp_val[5]);
	  /* here if no article */
	  else
	    (void) sprintf(out_val, "no more %s", tmp_val);
	}
      else
	(void) strcpy(out_val, tmp_val);

      tmp_str[0] = '\0';
      if ((indexx = object_offset(i_ptr)) >= 0)
	{
	  indexx = (indexx <<= 6) +
	    (i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1));
	  /* don't print tried string for store bought items */
	  if ((object_ident[indexx] & OD_TRIED) && !store_bought_p(i_ptr))
	    (void) strcat(tmp_str, "tried ");
	}
      if ((i_ptr->ident & (ID_MAGIK|ID_EMPTY|ID_DAMD)) && 
	  i_ptr->tval!=TV_MAGIC_BOOK && i_ptr->tval!=TV_PRAYER_BOOK)
	{
	  if (i_ptr->ident & ID_MAGIK)
	    (void) strcat(tmp_str, "blessed ");
	  if (i_ptr->ident & ID_EMPTY)
	    (void) strcat(tmp_str, "empty ");
	  if (i_ptr->ident & ID_DAMD)
	    (void) strcat(tmp_str, "cursed ");
	}
      if ((known2_p(i_ptr) || store_bought_p(i_ptr)) && 
	  ((!strncmp(i_ptr->inscrip, "average", 7)) ||
	   (!strncmp(i_ptr->inscrip, "good", 4)) ||
	   (!strncmp(i_ptr->inscrip, "excellent", 9)) ||
	   (!strncmp(i_ptr->inscrip, "special", 7))))
	i_ptr->inscrip[0]='\0';
      if (i_ptr->inscrip[0] != '\0')
	(void) strcat(tmp_str, i_ptr->inscrip);
      else if ((indexx = strlen(tmp_str)) > 0)
	  /* remove the extra blank at the end */
	  tmp_str[indexx-1] = '\0';
      if (tmp_str[0])
	{
	  (void) sprintf(tmp_val, " {%s}", tmp_str);
	  (void) strcat(out_val, tmp_val);
	}
      (void) strcat(out_val, ".");
    }
}

void invcopy(to, from_index)
register inven_type *to;
int from_index;
{
  register treasure_type *from;
  
  from = &object_list[from_index];
  to->index	= from_index;
  to->name2     = SN_NULL;
  to->inscrip[0] = '\0';
  to->flags     = from->flags;
  to->flags2    = from->flags2;
  to->tval      = from->tval;
  to->tchar     = from->tchar;
  to->p1        = from->p1;
  to->cost	= from->cost;
  to->subval    = from->subval;
  to->number    = from->number;
  to->weight    = from->weight;
  to->tohit     = from->tohit;
  to->todam     = from->todam;
  to->ac        = from->ac;
  to->toac      = from->toac;
  to->damage[0] = from->damage[0];
  to->damage[1] = from->damage[1];
  to->level     = from->level;
  to->ident	= 0;
#ifdef TC_COLOR
  to->color     = from->color;
#endif
}


/* Describe number of remaining charges.		-RAK-	*/
void desc_charges(item_val)
int item_val;
{
  register int rem_num;
  vtype out_val;

  if (known2_p(&inventory[item_val]))
    {
      rem_num = inventory[item_val].p1;
      (void) sprintf(out_val, "You have %d charges remaining.", rem_num);
      msg_print(out_val);
    }
}


/* Describe amount of item remaining.			-RAK-	*/
void desc_remain(item_val)
int item_val;
{
  bigvtype out_val, tmp_str;
  register inven_type *i_ptr;

  i_ptr = &inventory[item_val];
  i_ptr->number--;
  objdes(tmp_str, i_ptr, TRUE);
  i_ptr->number++;
  /* the string already has a dot at the end. */
  (void) sprintf(out_val, "You have %s", tmp_str);
  msg_print(out_val);
}
