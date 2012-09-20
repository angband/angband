/* eat.c: food code

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#ifdef USG
#include <string.h>
#else
#include <strings.h>
#endif

/* Eat some food.					-RAK-	*/
void eat()
{
  int32u i;
  int j, k, item_val, ident;
  register struct flags *f_ptr;
  register struct misc *m_ptr;
  register inven_type *i_ptr;

  free_turn_flag = TRUE;
  if (inven_ctr == 0)
    msg_print("But you are not carrying anything.");
  else if (!find_range(TV_FOOD, TV_NEVER, &j, &k))
    msg_print("You are not carrying any food.");
  else if (get_item(&item_val, "Eat what?", j, k, 0))
    {
      i_ptr = &inventory[item_val];
      free_turn_flag = FALSE;
      i = i_ptr->flags;
      ident = FALSE;
      while (i != 0)
	{
	  j = bit_pos(&i) + 1;
	  /* Foods					*/
	  switch(j)
	    {
	    case 1:
	      f_ptr = &py.flags;
	      if (!f_ptr->poison_resist)
		f_ptr->poisoned += randint(10) + i_ptr->level;
	      ident = TRUE;
	      break;
	    case 2:
	      f_ptr = &py.flags;
	      if (!py.flags.blindness_resist) {
	        f_ptr->blind += randint(250) + 10*i_ptr->level + 100;
	        draw_cave();
	        msg_print("A veil of darkness surrounds you.");
	        ident = TRUE;
	      }
	      break;
	    case 3:
	      if (!py.flags.fear_resist)
	      {
		  f_ptr = &py.flags;
		  f_ptr->afraid += randint(10) + i_ptr->level;
		  msg_print("You feel terrified!");
		  ident = TRUE;
	      }
	      break;
	    case 4:
	      f_ptr = &py.flags;
  		if ((!py.flags.confusion_resist) && (!py.flags.chaos_resist)){
	          f_ptr->confused += randint(10) + i_ptr->level;
	          msg_print("You feel drugged.");
		}
	      ident = TRUE;
	      break;
	    case 5:
	      f_ptr = &py.flags;
	      f_ptr->image += randint(200) + 25*i_ptr->level + 200;
	      msg_print("You feel drugged.");
	      ident = TRUE;
	      break;
	    case 6:
	      ident = cure_poison();
	      break;
	    case 7:
	      ident = cure_blindness();
	      break;
	    case 8:
	      f_ptr = &py.flags;
	      if (f_ptr->afraid > 1)
		{
		  f_ptr->afraid = 1;
		  ident = TRUE;
		}
	      break;
	    case 9:
	      ident = cure_confusion();
	      break;
	    case 10:
	      ident = TRUE;
	      lose_str();
	      break;
	    case 11:
	      ident = TRUE;
	      lose_con();
	      break;
#if 0  /* 12 through 15 are not used */
	    case 12:
	      ident = TRUE;
	      lose_int();
	      break;
	    case 13:
	      ident = TRUE;
	      lose_wis();
	      break;
	    case 14:
	      ident = TRUE;
	      lose_dex();
	      break;
	    case 15:
	      ident = TRUE;
	      lose_chr();
	      break;
#endif
	    case 16:
	      if (res_stat (A_STR))
		{
		  msg_print("You feel your strength returning.");
		  ident = TRUE;
		}
	      break;
	    case 17:
	      if (res_stat (A_CON))
		{
		  msg_print("You feel your health returning.");
		  ident = TRUE;
		}
	      break;
	    case 18:
	      if (res_stat (A_INT))
		{
		  msg_print("Your head spins a moment.");
		  ident = TRUE;
		}
	      break;
	    case 19:
	      if (res_stat (A_WIS))
		{
		  msg_print("You feel your wisdom returning.");
		  ident = TRUE;
		}
	      break;
	    case 20:
	      if (res_stat (A_DEX))
		{
		  msg_print("You feel more dextrous.");
		  ident = TRUE;
		}
	      break;
	    case 21:
	      if (res_stat (A_CHR))
		{
		  msg_print("Your skin stops itching.");
		  ident = TRUE;
		}
	      break;
	    case 22:
	      ident = hp_player(randint(6));
	      break;
	    case 23:
	      ident = hp_player(randint(12));
	      break;
	    case 24:
	      ident = hp_player(randint(18));
	      break;
#if 0  /* 25 is not used */
	    case 25:
	      ident = hp_player(damroll(3, 6));
	      break;
#endif
	    case 26:
	      ident = hp_player(damroll(3, 12));
	      break;
	    case 27:
	      take_hit(randint(18), "poisonous food.");
	      ident = TRUE;
	      break;
#if 0 /* 28 through 30 are not used */
	    case 28:
	      take_hit(randint(8), "poisonous food.");
	      ident = TRUE;
	      break;
	    case 29:
	      take_hit(damroll(2, 8), "poisonous food.");
	      ident = TRUE;
	      break;
	    case 30:
	      take_hit(damroll(3, 8), "poisonous food.");
	      ident = TRUE;
	      break;
#endif
	    default:
	      msg_print("Internal error in eat()");
	      break;
	    }
	  /* End of food actions.				*/
	}
      if (ident)
	{
	  if (!known1_p(i_ptr))
	    {
	      /* use identified it, gain experience */
	      m_ptr = &py.misc;
	      /* round half-way case up */
	      m_ptr->exp += (i_ptr->level + (m_ptr->lev >> 1)) / m_ptr->lev;
	      prt_experience();

	      identify (&item_val);
	      i_ptr = &inventory[item_val];
	    }
	}
      else if (!known1_p(i_ptr))
	sample (i_ptr);
      add_food(i_ptr->p1);
      py.flags.status &= ~(PY_WEAK|PY_HUNGRY);
      prt_hunger();
      desc_remain(item_val);
      inven_destroy(item_val);
    }
}
