/*
 * wands.c: wand code 
 *
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

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

/* Wands for the aiming.				 */
void 
aim()
{
    int32u                i;
    register int          l, ident;
    int                   item_val, done_effect, j, k, chance, dir;
    register inven_type  *i_ptr;
    register struct misc *m_ptr;

    free_turn_flag = TRUE;
    if (inven_ctr == 0)
	msg_print("But you are not carrying anything.");
    else if (!find_range(TV_WAND, TV_NEVER, &j, &k))
	msg_print("You are not carrying any wands.");
    else if (get_item(&item_val, "Aim which wand?", j, k, 0)) {
	i_ptr = &inventory[item_val];
	free_turn_flag = FALSE;
	if (get_dir(NULL, &dir)) {
	    if (py.flags.confused > 0) {
		msg_print("You are confused.");
		do {
		    dir = randint(9);
		}
		while (dir == 5);
	    }
	    ident = FALSE;
	    m_ptr = &py.misc;
	    chance = m_ptr->save + stat_adj(A_INT)
		- (int)(i_ptr->level>42?42:i_ptr->level)
		+ (class_level_adj[m_ptr->pclass][CLA_DEVICE] * m_ptr->lev / 3);
	    if (py.flags.confused > 0)
		chance = chance / 2;
	    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1))
		chance = USE_DEVICE;	/* Give everyone a slight chance */
	    if (chance <= 0)
		chance = 1;
	    if (randint(chance) < USE_DEVICE)
		msg_print("You failed to use the wand properly.");
	    else if (i_ptr->p1 > 0) {
		i = i_ptr->flags;
		done_effect = 0;
		(i_ptr->p1)--;
		while (!done_effect) {
		    k = char_row;
		    l = char_col;
		    switch (i) {
		      case WD_LT:
			msg_print("A line of blue shimmering light appears.");
			light_line(dir, char_row, char_col);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_DRG_FIRE:
			fire_ball(GF_FIRE, dir, k, l, 100, 3);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_DRG_FRST:
			fire_ball(GF_FROST, dir, k, l, 80, 3);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_DRG_BREA:
			switch (randint(5)) {
			  case 1:
			    fire_ball(GF_FIRE, dir, k, l, 100, 3);
			    break;
			  case 2:
			    fire_ball(GF_FROST, dir, k, l, 80, 3);
			    break;
			  case 3:
			    fire_ball(GF_ACID, dir, k, l, 90, 3);
			    break;
			  case 4:
			    fire_ball(GF_LIGHTNING, dir, k, l, 70, 3);
			    break;
			  default:
			    fire_ball(GF_POISON_GAS, dir, k, l, 70, 3);
			    break;
			}
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_AC_BLTS:	/* Acid , New */
			if (randint(5)==1)
			    line_spell(GF_ACID,dir,k,l,damroll(5,8));
			else
			    fire_bolt(GF_ACID,dir,k,l,damroll(5,8));
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_LT_BLTS:	/* Lightning */
			if (randint(6)==1)
			    line_spell(GF_LIGHTNING,dir,k,l,damroll(3,8));
			else
			    fire_bolt(GF_LIGHTNING, dir, k, l, damroll(3, 8));
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_FT_BLTS:	/* Frost */
			if (randint(6)==1)
			    line_spell(GF_LIGHTNING,dir,k,l,damroll(3,8));
			else
			    fire_bolt(GF_LIGHTNING, dir, k, l, damroll(3, 8));
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_FR_BLTS:	/* Fire */
			if (randint(4)==1)
			    line_spell(GF_FIRE,dir,k,l,damroll(6,8));
			else
			    fire_bolt(GF_FIRE, dir, k, l, damroll(6, 8));
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_ST_MUD:
			ident = wall_to_mud(dir, k, l);
			done_effect = 1;
			break;
		      case WD_POLY:
			ident = poly_monster(dir, k, l);
			done_effect = 1;
			break;
		      case WD_HEAL_MN:
			ident = hp_monster(dir, k, l, -damroll(4, 6));
			done_effect = 1;
			break;
		      case WD_HAST_MN:
			ident = speed_monster(dir, k, l, 1);
			done_effect = 1;
			break;
		      case WD_SLOW_MN:
			ident = speed_monster(dir, k, l, -1);
			done_effect = 1;
			break;
		      case WD_CONF_MN:
			ident = confuse_monster(dir, k, l, 10);
			done_effect = 1;
			break;
		      case WD_SLEE_MN:
			ident = sleep_monster(dir, k, l);
			done_effect = 1;
			break;
		      case WD_DRAIN:
			ident = drain_life(dir, k, l, 75);
			done_effect = 1;
			break;
		      case WD_ANHIL:
			ident = drain_life(dir, k, l, 125);
			done_effect = 1;
			break;
		      case WD_TR_DEST:
			ident = td_destroy2(dir, k, l);
			done_effect = 1;
			break;
		      case WD_MAG_MIS:
			if (randint(6)==1)
			    line_spell(GF_MAGIC_MISSILE,dir,k,l,damroll(2,6));
			else
			    fire_bolt(GF_MAGIC_MISSILE, dir, k, l, damroll(2, 6));
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_FEAR_MN:	/* Fear Monster */
			ident = fear_monster(dir, k, l, 10);
			done_effect = 1;
			break;
		      case WD_CLONE:
			ident = clone_monster(dir, k, l);
			done_effect = 1;
			break;
		      case WD_TELE:
			ident = teleport_monster(dir, k, l);
			done_effect = 1;
			break;
		      case WD_DISARM:
			ident = disarm_all(dir, k, l);
			done_effect = 1;
			break;
		      case WD_LT_BALL:
			fire_ball(GF_LIGHTNING, dir, k, l, 32, 2);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_CD_BALL:
			fire_ball(GF_FROST, dir, k, l, 48, 2);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_FR_BALL:
			fire_ball(GF_FIRE, dir, k, l, 72, 2);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_ST_CLD:
			fire_ball(GF_POISON_GAS, dir, k, l, 12, 2);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_AC_BALL:
			fire_ball(GF_ACID, dir, k, l, 60, 2);
			ident = TRUE;
			done_effect = 1;
			break;
		      case WD_WONDER:
			i = randint(23);
			break;
		      default:
			msg_print("Internal error in wands() ");
			done_effect = 1;
			break;
		    }
		/* End of Wands.		    */
		}
		if (ident) {
		    if (!known1_p(i_ptr)) {
			m_ptr = &py.misc;
		    /* round half-way case up */
			m_ptr->exp += (i_ptr->level + (m_ptr->lev >> 1)) /
			    m_ptr->lev;
			prt_experience();

			identify(&item_val);
			i_ptr = &inventory[item_val];
		    }
		} else if (!known1_p(i_ptr))
		    sample(i_ptr);
		desc_charges(item_val);
	    } else {
		msg_print("The wand has no charges left.");
		if (!known2_p(i_ptr))
		    add_inscribe(i_ptr, ID_EMPTY);
	    }
	}
    }
}
