/* staffs.c: staff code

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "monster.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#include <string.h>
#else
#include <strings.h>
#endif


/* Use a staff.					-RAK-	*/
void use()
{
  int32u i;
  int j, k, item_val, chance, y, x;
  register int ident;
  register struct misc *m_ptr;
  register inven_type *i_ptr;

  free_turn_flag = TRUE;
  if (inven_ctr == 0)
    msg_print("But you are not carrying anything.");
  else if (!find_range(TV_STAFF, TV_NEVER, &j, &k))
    msg_print("You are not carrying any staffs.");
  else if (get_item(&item_val, "Use which staff?", j, k, 0))
    {
      i_ptr = &inventory[item_val];
      free_turn_flag = FALSE;
      m_ptr = &py.misc;
      chance = m_ptr->save + stat_adj(A_INT) - (int)i_ptr->level - 5
	+ (class_level_adj[m_ptr->pclass][CLA_DEVICE] * m_ptr->lev / 3);
      if (py.flags.confused > 0)
	chance = chance / 2;
      if (chance <= 0)	chance = 1;
      if (randint(chance) < USE_DEVICE)
	msg_print("You failed to use the staff properly.");
      else if (i_ptr->p1 > 0)
	{
	  i = i_ptr->flags;
	  ident = FALSE;
	  (i_ptr->p1)--;
	  switch(i) {
	  case ST_HEALING:
	    ident = hp_player(300);
	    if (py.flags.stun>0) {
	      if (py.flags.stun>50) {
		py.misc.ptohit+=20;
		py.misc.ptodam+=20;
	      } else {
		py.misc.ptohit+=5;
		py.misc.ptodam+=5;
	      }
	      py.flags.stun=0;
	      ident = TRUE;
	      msg_print("You're head stops stinging.");
	    }
	    if (py.flags.cut>0) {
	      py.flags.cut=0;
	      ident = TRUE;
	      msg_print("You feel better.");
	    }
	    break;
	  case ST_GENOCIDE:
	    genocide(FALSE);
	    ident = TRUE;
	    break;
	  case ST_PROBE:
	    probing();
	    ident = TRUE;
	    break;
          case ST_IDENTIFY:
	    ident_spell();
	    ident = TRUE;
	    break;
	  case ST_HOLYNESS:
	    dispel_creature(EVIL,120);
	    protect_evil();
	    cure_poison();
	    remove_fear();
	    hp_player(50);
	    if (py.flags.stun>0) {
	      if (py.flags.stun>50) {
		py.misc.ptohit+=20;
		py.misc.ptodam+=20;
	      } else {
		py.misc.ptohit+=5;
		py.misc.ptodam+=5;
	      }
	      py.flags.stun=0;
	      ident = TRUE;
	      msg_print("You're head stops stinging.");
	    }
	    if (py.flags.cut>0) {
	      py.flags.cut=0;
	      ident = TRUE;
	      msg_print("You feel better.");
	    }
	    ident = TRUE;
	    break;
	  case ST_MAGI:
            if (res_stat(A_INT)) {
	       msg_print("You have a warm feeling.");
	       ident = TRUE;
	    }
	    m_ptr = &py.misc;
	    if (m_ptr->cmana < m_ptr->mana) {
	      m_ptr->cmana = m_ptr->mana;
	      ident = TRUE;
              msg_print("Your feel your head clear.");
	      prt_cmana();
	    }
	    break;
	  case ST_POWER:
	    dispel_creature(0xFFFFFFFFL,120);
            break;
	  case ST_SURROUND:
	    map_area();
	    ident = TRUE;
	    break;
          case ST_LIGHT:
	    ident = light_area(char_row, char_col);
	    break;
	  case ST_DR_LC:
	    ident = detect_sdoor();
	    break;
	  case ST_TRP_LC:
	    ident = detect_trap();
	    break;
	  case ST_TRE_LC:
	    ident = detect_treasure();
	    break;
	  case ST_OBJ_LC:
	    ident = detect_object();
	    break;
          case ST_TELE:
	    teleport(100);
	    ident = TRUE;
	    break;
	  case ST_EARTH:
	    ident = TRUE;
	    earthquake();
	    break;
          case ST_SUMMON:
	    ident = FALSE;
	    for (k = 0; k < randint(4); k++)
	    {
	      y = char_row;
	      x = char_col;
	      ident |= summon_monster(&y, &x, FALSE);
	    }
	    break;
	  case ST_DEST:
	    ident = TRUE;
	    destroy_area(char_row, char_col);
            break;
	  case ST_STAR:
	    ident = TRUE;
	    starlite(char_row, char_col);
	    break;
	  case ST_HAST_MN:
	    ident = speed_monsters(1);
	    break;
	  case ST_SLOW_MN:
	    ident = speed_monsters(-1);
	    break;
	  case ST_SLEE_MN:
	    ident = sleep_monsters2();
	    break;
	  case ST_CURE_LT:
	    ident = hp_player(randint(8));
	    break;
	  case ST_DET_INV:
	    ident = detect_invisible();
	    break;
          case ST_SPEED:
	    if (py.flags.fast == 0) ident = TRUE;
	    if (py.flags.fast <= 0)
	      py.flags.fast += randint(30) + 15;
	    else
	      py.flags.fast += randint(5);
	    break;
          case ST_SLOW:
	    if (py.flags.slow == 0) ident = TRUE;
	    py.flags.slow += randint(30) + 15;
	    break;
	  case ST_REMOVE:
	    if (remove_curse())
	    {
	      if (py.flags.blind < 1)
	        msg_print("The staff glows blue for a moment..");
	      ident = TRUE;
	    }
	    break;
	  case ST_DET_EVI:
	    ident = detect_evil();
            break;
	  case ST_CURING:
	    if ((cure_blindness()) || (cure_poison()) ||
	    (cure_confusion()) || (py.flags.stun>0) || (py.flags.cut>0))
	      ident = TRUE;
	    if (py.flags.stun>0) {
	      if (py.flags.stun>50) {
		py.misc.ptohit+=20;
		py.misc.ptodam+=20;
	      } else {
		py.misc.ptohit+=5;
		py.misc.ptodam+=5;
	      }
	      py.flags.stun=0;
	      msg_print("You're head stops stinging.");
	    } else if (py.flags.cut>0) {
	      py.flags.cut=0;
	      msg_print("You feel better.");
	    }
	    break;
	  case ST_DSP_EVI:
	    ident = dispel_creature(EVIL, 60);
            break;
	  case ST_DARK:
	    ident = unlight_area(char_row, char_col);
            break;
	  default:
	    msg_print("Internal error in staffs()");
	    break;
	  }
	  if (ident)
	    {
	      if (!known1_p(i_ptr))
		{
		  m_ptr = &py.misc;
		  /* round half-way case up */
		  m_ptr->exp += (i_ptr->level + (m_ptr->lev >> 1)) /
		    m_ptr->lev;
		  prt_experience();

		  identify(&item_val);
		  i_ptr = &inventory[item_val];
		}
	    }
	  else if (!known1_p(i_ptr))
	    sample (i_ptr);
	  desc_charges(item_val);
	}
      else
	{
	  msg_print("The staff has no charges left.");
	  if (!known2_p(i_ptr))
	    add_inscribe(i_ptr, ID_EMPTY);
	}
    }
}
