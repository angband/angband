/*
 * prayer.c: code for priest spells 
 *
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "constant.h"
#include "monster.h"
#include "config.h"
#include "types.h"
#include "externs.h"


/* Pray like HELL.					-RAK-	 */
void 
pray()
{
    int                 i, j, item_val, dir;
    int                 choice, chance, result;
    register spell_type *s_ptr;
    register struct misc *m_ptr;
    register struct flags *f_ptr;
    register inven_type *i_ptr;

    free_turn_flag = TRUE;
    if (py.flags.blind > 0)
	msg_print("You can't see to read your prayer!");
    else if (no_light())
	msg_print("You have no light to read by.");
    else if (py.flags.confused > 0)
	msg_print("You are too confused.");
    else if (class[py.misc.pclass].spell != PRIEST)
	msg_print("Pray hard enough and your prayers may be answered.");
    else if (inven_ctr == 0)
	msg_print("But you are not carrying anything!");
    else if (!find_range(TV_PRAYER_BOOK, TV_NEVER, &i, &j))
	msg_print("You are not carrying any Holy Books!");
    else if (get_item(&item_val, "Use which Holy Book?", i, j, 0)) {
	result = cast_spell("Recite which prayer?", item_val, &choice, &chance);
	if (result < 0)
	    msg_print("You don't know any prayers in that book.");
	else if (result > 0) {
	    s_ptr = &magic_spell[py.misc.pclass - 1][choice];
	    free_turn_flag = FALSE;

	    if (py.flags.stun > 50)
		chance += 25;
	    else if (py.flags.stun > 0)
		chance += 15;
	    if (randint(100) <= chance)	/* changed -CFT */
		msg_print("You lost your concentration!");
	    else {
	    /* Prayers.					 */
		switch (choice + 1) {
		  case 1:
		    (void)detect_evil();
		    break;
		  case 2:
		    (void)hp_player(damroll(3, 3));
		    if (py.flags.cut > 0) {
			py.flags.cut -= 10;
			if (py.flags.cut < 0)
			    py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 3:
		    bless(randint(12) + 12);
		    break;
		  case 4:
		    (void)remove_fear();
		    break;
		  case 5:
		    (void)light_area(char_row, char_col,
		     damroll(2, (py.misc.lev / 2)), (py.misc.lev / 10) + 1);
		    break;
		  case 6:
		    (void)detect_trap();
		    break;
		  case 7:
		    (void)detect_sdoor();
		    break;
		  case 8:
		    (void)slow_poison();
		    break;
		  case 9:
		    if (get_dir(NULL, &dir))
			(void)fear_monster(dir, char_row, char_col, py.misc.lev);
		    break;
		  case 10:
		    teleport((int)(py.misc.lev * 3));
		    break;
		  case 11:
		    (void)hp_player(damroll(4, 4));
		    if (py.flags.cut > 0) {
			py.flags.cut = (py.flags.cut / 2) - 20;
			if (py.flags.cut < 0)
			    py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 12:
		    bless(randint(24) + 24);
		    break;
		  case 13:
		    (void)sleep_monsters1(char_row, char_col);
		    break;
		  case 14:
		    create_food();
		    break;
		  case 15:
		    remove_curse();/* -CFT */
		    break;
		  case 16:
		    f_ptr = &py.flags;
		    f_ptr->resist_heat += randint(10) + 10;
		    f_ptr->resist_cold += randint(10) + 10;
		    break;
		  case 17:
		    (void)cure_poison();
		    break;
		  case 18:
		    if (get_dir(NULL, &dir))
			fire_ball(GF_HOLY_ORB, dir, char_row, char_col,
				  (int)(damroll(3, 6) + py.misc.lev), 2,
				  "Black Sphere");
		    break;
		  case 19:
		    (void)hp_player(damroll(8, 4));
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 20:
		    detect_inv2(randint(24) + 24);
		    break;
		  case 21:
		    (void)protect_evil();
		    break;
		  case 22:
		    earthquake();
		    break;
		  case 23:
		    map_area();
		    break;
		  case 24:
		    (void)hp_player(damroll(16, 4));
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 25:
		    (void)turn_undead();
		    break;
		  case 26:
		    bless(randint(48) + 48);
		    break;
		  case 27:
		    (void)dispel_creature(UNDEAD, (int)(3 * py.misc.lev));
		    break;
		  case 28:
		    (void)hp_player(200);
		    if (py.flags.stun > 0) {
			if (py.flags.stun > 50) {
			    py.misc.ptohit += 20;
			    py.misc.ptodam += 20;
			} else {
			    py.misc.ptohit += 5;
			    py.misc.ptodam += 5;
			}
			py.flags.stun = 0;
			msg_print("Your head stops stinging.");
		    }
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("You feel better.");
		    }
		    break;
		  case 29:
		    (void)dispel_creature(EVIL, (int)(3 * py.misc.lev));
		    break;
		  case 30:
		    warding_glyph();
		    break;
		  case 31:
		    (void)dispel_creature(EVIL, (int)(4 * py.misc.lev));
		    (void)remove_fear();
		    (void)cure_poison();
		    (void)hp_player(1000);
		    if (py.flags.stun > 0) {
			if (py.flags.stun > 50) {
			    py.misc.ptohit += 20;
			    py.misc.ptodam += 20;
			} else {
			    py.misc.ptohit += 5;
			    py.misc.ptodam += 5;
			}
			py.flags.stun = 0;
			msg_print("Your head stops stinging.");
		    }
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("You feel better.");
		    }
		    break;
		  case 32:
		    (void)detect_monsters();
		    break;
		  case 33:
		    (void)detection();
		    break;
		  case 34:
		    (void)ident_spell();
		    break;
		  case 35:	   /* probing */
		    (void)probing();
		    break;
		  case 36:	   /* Clairvoyance */
		    wizard_light(TRUE);
		    break;
		  case 37:
		    (void)hp_player(damroll(8, 4));
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 38:
		    (void)hp_player(damroll(16, 4));
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("Your wounds heal.");
		    }
		    break;
		  case 39:
		    (void)hp_player(2000);
		    if (py.flags.stun > 0) {
			if (py.flags.stun > 50) {
			    py.misc.ptohit += 20;
			    py.misc.ptodam += 20;
			} else {
			    py.misc.ptohit += 5;
			    py.misc.ptodam += 5;
			}
			py.flags.stun = 0;
			msg_print("Your head stops stinging.");
		    }
		    if (py.flags.cut > 0) {
			py.flags.cut = 0;
			msg_print("You feel better.");
		    }
		    break;
		  case 40:	   /* restoration */
		    if (res_stat(A_STR))
			msg_print("You feel warm all over.");
		    if (res_stat(A_INT))
			msg_print("You have a warm feeling.");
		    if (res_stat(A_WIS))
			msg_print("You feel your wisdom returning.");
		    if (res_stat(A_DEX))
			msg_print("You feel less clumsy.");
		    if (res_stat(A_CON))
			msg_print("You feel your health returning!");
		    if (res_stat(A_CHR))
			msg_print("You feel your looks returning.");
		    break;
		  case 41:	   /* rememberance */
		    (void)restore_level();
		    break;
		  case 42:	   /* dispel undead */
		    (void)dispel_creature(UNDEAD, (int)(4 * py.misc.lev));
		    break;
		  case 43:	   /* dispel evil */
		    (void)dispel_creature(EVIL, (int)(4 * py.misc.lev));
		    break;
		  case 44:	   /* banishment */
		    if (banish_creature(EVIL, 100))
			msg_print("The Power of your god banishes the creatures!");
		    break;
		  case 45:	   /* word of destruction */
		    destroy_area(char_row, char_col);
		    break;
		  case 46:	   /* annihilation */
		    if (get_dir(NULL, &dir))
			drain_life(dir, char_row, char_col, 200);
		    break;
		  case 47:	   /* unbarring ways */
		    (void)td_destroy();
		    break;
		  case 48:	   /* recharging */
		    (void)recharge(15);
		    break;
		  case 49:	   /* dispel curse */
		    (void)remove_all_curse();
		    break;
		  case 50:	   /* enchant weapon */
		    i_ptr = &inventory[INVEN_WIELD];
		    if (i_ptr->tval != TV_NOTHING) {
			int                 flag, k;
			char                tmp_str[100], out_val[100];

			objdes(tmp_str, i_ptr, FALSE);
			if ((i_ptr->flags2 & TR_ARTIFACT) && randint(2) == 1) {	/* DGK */
			    sprintf(out_val, "Your %s resists enchantment!",
				    tmp_str);
			    msg_print(out_val);
			} else {
			    int                 maxench;

			    sprintf(out_val, "Your %s glows brightly!", tmp_str);
			    msg_print(out_val);
			    flag = FALSE;
			    for (k = 0; k < randint(4); k++)
				if (enchant(&i_ptr->tohit, 10))
				    flag = TRUE;
			    for (k = 0; k < randint(4); k++) {
				if ((i_ptr->tval >= TV_HAFTED) &&
				    (i_ptr->tval <= TV_DIGGING))
				    maxench = i_ptr->damage[0] * i_ptr->damage[1];
				else	/* Bows' and arrows' enchantments
					 * should not be limited by their low
					 * base damages */
				    maxench = 10;
				if (enchant(&i_ptr->todam, maxench))
				    flag = TRUE;
			    }
			    if (flag)
				calc_bonuses();
			/* used to clear TR_CURSED; should remain set -DGK- */
			    else
				msg_print("The enchantment fails.");
			} /* DGK */
		    }
		    break;
		  case 51:	   /* enchant armor */
		    if (1) {
			int                 k = 0;
			int                 l = 0;
			int                 tmp[100];

			if (inventory[INVEN_BODY].tval != TV_NOTHING)
			    tmp[k++] = INVEN_BODY;
			if (inventory[INVEN_ARM].tval != TV_NOTHING)
			    tmp[k++] = INVEN_ARM;
			if (inventory[INVEN_OUTER].tval != TV_NOTHING)
			    tmp[k++] = INVEN_OUTER;
			if (inventory[INVEN_HANDS].tval != TV_NOTHING)
			    tmp[k++] = INVEN_HANDS;
			if (inventory[INVEN_HEAD].tval != TV_NOTHING)
			    tmp[k++] = INVEN_HEAD;
		    /* also enchant boots */
			if (inventory[INVEN_FEET].tval != TV_NOTHING)
			    tmp[k++] = INVEN_FEET;

			if (k > 0)
			    l = tmp[randint(k) - 1];
			if (TR_CURSED & inventory[INVEN_BODY].flags)
			    l = INVEN_BODY;
			else if (TR_CURSED & inventory[INVEN_ARM].flags)
			    l = INVEN_ARM;
			else if (TR_CURSED & inventory[INVEN_OUTER].flags)
			    l = INVEN_OUTER;
			else if (TR_CURSED & inventory[INVEN_HEAD].flags)
			    l = INVEN_HEAD;
			else if (TR_CURSED & inventory[INVEN_HANDS].flags)
			    l = INVEN_HANDS;
			else if (TR_CURSED & inventory[INVEN_FEET].flags)
			    l = INVEN_FEET;

			if (l > 0) {
			    char                out_val[100], tmp_str[100];

			    i_ptr = &inventory[l];
			    objdes(tmp_str, i_ptr, FALSE);	/* DGK */
			    if ((i_ptr->flags2 & TR_ARTIFACT) && randint(2) == 1) {
				sprintf(out_val, "Your %s resists enchantment!",
					tmp_str);
				msg_print(out_val);
			    } else {
				sprintf(out_val, "Your %s glows faintly!", tmp_str);
				msg_print(out_val);
				if (enchant(&i_ptr->toac, 10))
				    calc_bonuses();
			    /*
			     * used to clear TR_CURSED; should remain set
			     * -DGK- 
			     */
				else
				    msg_print("The enchantment fails.");
			    } /* DGK */
			}
		    }
		    break;
		  case 52:	   /* Elemental brand */
		    i_ptr = &inventory[INVEN_WIELD];
		    if (i_ptr->tval != TV_NOTHING &&
			i_ptr->name2 == SN_NULL) {
			int                 hot = randint(2) - 1;
			char                tmp_str[100], out_val[100];

			objdes(tmp_str, i_ptr, FALSE);
			if (hot) {
			    sprintf(out_val,
				    "Your %s is covered in a fiery shield!",
				    tmp_str);
			    i_ptr->name2 |= SN_FT;
			    i_ptr->flags |= (TR_FLAME_TONGUE | TR_RES_FIRE);
			} else {
			    sprintf(out_val, "Your %s glows deep, icy blue!",
				    tmp_str);
			    i_ptr->name2 |= SN_FB;
			    i_ptr->flags |= (TR_FROST_BRAND | TR_RES_COLD);
			}
			msg_print(out_val);
			i_ptr->tohit += 3 + randint(3);
			i_ptr->todam += 3 + randint(3);
		    /* i_ptr->flags &= ~TR_CURSED; */
			calc_bonuses();
		    } else {
			msg_print("The Branding fails.");
		    }
		    break;
		  case 53:	   /* blink */
		    teleport(10);
		    break;
		  case 54:	   /* teleport */
		    teleport((int)(py.misc.lev * 8));
		    break;
		  case 55:	   /* teleport away */
		    if (get_dir(NULL, &dir))
			(void)teleport_monster(dir, char_row, char_col);
		    break;
		  case 56:	   /* teleport level */
		    (void)tele_level();
		    break;
		  case 57:	   /* word of recall */
		    if (py.flags.word_recall == 0) {
			py.flags.word_recall = 15 + randint(20);
			msg_print("The air about you becomes charged...");
		    } else {
			py.flags.word_recall = 0;
			msg_print("A tension leaves the air around you...");
		    }
		    break;
		  case 58:	   /* alter reality */
		    new_level_flag = TRUE;
		    break;
		  default:
		    break;
		}
	    /* End of prayers.				 */
		if (!free_turn_flag) {
		    m_ptr = &py.misc;
		    if (choice < 32) {
			if ((spell_worked & (1L << choice)) == 0) {
			    m_ptr->exp += s_ptr->sexp << 2;
			    spell_worked |= (1L << choice);
			    prt_experience();
			}
		    } else {
			if ((spell_worked2 & (1L << (choice - 32))) == 0) {
			    m_ptr->exp += s_ptr->sexp << 2;
			    spell_worked2 |= (1L << (choice - 32));
			    prt_experience();
			}
		    }
		}
	    }
	    m_ptr = &py.misc;
	    if (!free_turn_flag) {
		if (s_ptr->smana > m_ptr->cmana) {
		    msg_print("You faint from fatigue!");
		    py.flags.paralysis =
			randint((int)(5 * (s_ptr->smana - m_ptr->cmana)));
		    m_ptr->cmana = 0;
		    m_ptr->cmana_frac = 0;
		    if (randint(3) == 1) {
			msg_print("You have damaged your health!");
			(void)dec_stat(A_CON);
		    }
		} else
		    m_ptr->cmana -= s_ptr->smana;
		prt_cmana();
	    }
	}
    }
}
