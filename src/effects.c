/* File: effects.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"



/*
 * This file includes code for eating food, drinking potions,
 * reading scrolls, aiming wands, using staffs, zapping rods,
 * and activating artifacts.
 *
 * In all cases, if the player becomes "aware" of the item's use
 * by testing it, mark it as "aware" and reward some experience
 * based on the object's level, always rounding up.  If the player
 * remains "unaware", mark that object "kind" as "tried".
 *
 * This code now correctly handles the stacking/unstacking of wands,
 * staffs, and rods.  But note that with a "full" pack, it may not be
 * possible to "unstack" ahead of time, even though if the unstacking
 * was done, the command would succeed.  Perhaps we need a "drop the
 * extra items" option (with auto-re-pick-up if possible), or a "You
 * know you will use a lot of charges?" verification step.
 *
 * Similarly, consider the logic behind the game refusing to let you
 * "drop" onto an arrow, even though you MIGHT have chosen to drop arrows.
 *
 * In all "unstacking" scenarios, the "used" object maintains its location,
 * and the "extra" items are added at the "end" of the inventory (usually in
 * the slot following the one that was used).  If the item "becomes aware",
 * it is allowed to "collect" other similar objects.  Finally, if it appears
 * as though "other" identifications may have taken place, the pack is
 * "totally combined" (this is a not very efficient function!).
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" will cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * XXX XXX XXX Someone needs to verify all of these effects. XXX XXX
 */



/*
 * Lose a strength point.				-RAK-	 
 */
static void lose_str()
{
    if (!p_ptr->sustain_str) {
	(void)dec_stat(A_STR, 10, FALSE);
	msg_print("You feel very weak.");
    }
    else {
	msg_print("You feel weak for a moment but it passes.");
    }
}


/*
 * Lose an intelligence point.				-RAK-	 
 */
static void lose_int()
{
    if (!p_ptr->sustain_int) {
	(void)dec_stat(A_INT, 10, FALSE);
	msg_print("You become very dizzy.");
    }
    else {
	msg_print("You become dizzy for a moment but it passes.");
    }
}


/*
 * Lose a wisdom point.					-RAK-	 
 */
static void lose_wis()
{
    if (!p_ptr->sustain_wis) {
	(void)dec_stat(A_WIS, 10, FALSE);
	msg_print("You feel very naive.");
    }
    else {
	msg_print("You feel naive for a moment but it passes.");
    }
}


/*
 * Lose a dexterity point.				-RAK-	 
 */
static void lose_dex()
{
    if (!p_ptr->sustain_dex) {
	(void)dec_stat(A_DEX, 10, FALSE);
	msg_print("You feel very sore.");
    }
    else {
	msg_print("You feel sore for a moment but it passes.");
    }
}


/*
 * Lose a constitution point.				-RAK-	 
 */
static void lose_con()
{
    if (!p_ptr->sustain_con) {
	(void)dec_stat(A_CON, 10, FALSE);
	msg_print("You feel very sick.");
    }
    else {
	msg_print("You feel sick for a moment but it passes.");
    }
}


/*
 * Lose a charisma point.				-RAK-	 
 */
static void lose_chr()
{
    if (!p_ptr->sustain_chr) {
	(void)dec_stat(A_CHR, 10, FALSE);
	msg_print("Your skin starts to itch.");
    }
    else {
	msg_print("Your skin itches for a moment, then feels better.");
    }
}



/*
 * Eat some food. -RAK-
 * A single food object disappears.
 * Food uses "pval" for "calories".
 */
void eat_food(void)
{
    int32u                 flg;
    int			   j, ident, num, lev;
    int                    item_val, i1, i2;
    register inven_type   *i_ptr;

    /* Assume the turn is free */
    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    if (!find_range(TV_FOOD, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any food.");
	return;
    }

    if (!get_item(&item_val, "Eat what?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];
    num = i_ptr->number;
    lev = k_list[i_ptr->k_idx].level;

    /* The turn is not free */
    free_turn_flag = FALSE;

    /* Identity not known yet */
    ident = FALSE;

    /* Hack -- Note food with no "effects" */
    if (i_ptr->flags1 == 0) {
	msg_print("That tastes good.");
	ident = TRUE;
    }

    /* Apply all of the food flags */
    for (flg = i_ptr->flags1; flg; ) {

	/* Extract the next "effect" bit */
	j = bit_pos(&flg);

	/* Analyze the effect */
	switch (j + 1) {

	  case 1:
	    if (!p_ptr->immune_pois &&
		!p_ptr->oppose_pois &&
		!p_ptr->resist_pois) {
		p_ptr->poisoned += randint(10) + lev;
		ident = TRUE;
	    }
	    break;

	  case 2:
	    if (!p_ptr->resist_blind) {
		p_ptr->blind += randint(250) + 10 * lev + 100;
		draw_cave();
		msg_print("A veil of darkness surrounds you.");
		ident = TRUE;
	    }
	    break;

	  case 3:
	    if (!p_ptr->resist_fear) {
		p_ptr->afraid += randint(10) + lev;
		msg_print("You feel terrified!");
		ident = TRUE;
	    }
	    break;

	  case 4:
	    if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
		p_ptr->confused += randint(10) + lev;
		msg_print("You feel drugged.");
		ident = TRUE;
	    }
	    break;

	  case 5:
	    p_ptr->image += randint(200) + 25 * lev + 200;
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
	    if (p_ptr->afraid > 1) {
		p_ptr->afraid = 1;
		ident = TRUE;
	    }
	    break;

	  case 9:
	    ident = cure_confusion();
	    break;

	  case 10:
	    lose_str();
	    ident = TRUE;
	    break;

	  case 11:
	    lose_con();
	    ident = TRUE;
	    break;

	  case 12:
	    lose_int();
	    ident = TRUE;
	    break;

	  case 13:
	    lose_wis();
	    ident = TRUE;
	    break;

	  case 14:
	    lose_dex();
	    ident = TRUE;
	    break;

	  case 15:
	    lose_chr();
	    ident = TRUE;
	    break;

	  case 16:
	    if (res_stat(A_STR)) {
		msg_print("You feel your strength returning.");
		ident = TRUE;
	    }
	    break;

	  case 17:
	    if (res_stat(A_CON)) {
		msg_print("You feel your health returning.");
		ident = TRUE;
	    }
	    break;

	  case 18:
	    if (res_stat(A_INT)) {
		msg_print("Your head spins a moment.");
		ident = TRUE;
	    }
	    break;

	  case 19:
	    if (res_stat(A_WIS)) {
		msg_print("You feel your wisdom returning.");
		ident = TRUE;
	    }
	    break;

	  case 20:
	    if (res_stat(A_DEX)) {
		msg_print("You feel more dextrous.");
		ident = TRUE;
	    }
	    break;

	  case 21:
	    if (res_stat(A_CHR)) {
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

	  case 25:
	    ident = hp_player(damroll(3, 6));
	    break;

	  case 26:
	    ident = hp_player(damroll(3, 12));
	    break;

	  case 27:
	    take_hit(randint(18), "poisonous food.");
	    ident = TRUE;
	    break;

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

	  case 31:
	    break;

	  case 32:
	    break;
	}
    }

    /* We have tried it */
    inven_tried(i_ptr);

    /* The player is now aware of the object */
    if (ident && !inven_aware_p(i_ptr)) {
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }

    /* Consume the food */
    add_food(i_ptr->pval);
    
    /* Hack -- note loss of hunger */
    p_ptr->status &= ~(PY_WEAK | PY_HUNGRY);
    prt_hunger();

    /* Destroy the food */
    inven_item_increase(item_val, -1);
    inven_item_describe(item_val);
    inven_item_optimize(item_val);

    /* Combine (if any left) */
    if (num > 1) item_val = combine(item_val);

    /* XXX No need to combine the pack */
}





/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack()
{
    int                 i;
    inven_type         *i_ptr;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	i_ptr = &inventory[i];
	if (i_ptr->tval != TV_NOTHING) {
	    inven_aware(i_ptr);
	    known2(i_ptr);
	}
    }
}


/*
 * Quaff a potion
 * A single potion object disappears.
 * Potions use "pval" for "calories"
 * Hack -- there are two "types" of potions.
 */
void quaff_potion(void)
{
    int32u flg, l;
    int    j, ident, num, combo = -1;
    int    item_val, i1, i2;
    register inven_type   *i_ptr;

    /* Assume the turn will be free */
    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    if (!find_range(TV_POTION, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any potions.");
	return;
    }

    if (!get_item(&item_val, "Quaff which potion?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];
    num = i_ptr->number;

    /* Not identified yet */
    ident = FALSE;

    /* Note potions with no effects */
    if ((i_ptr->flags1 == 0) && (i_ptr->flags2 == 0)) {
	msg_print("You feel less thirsty.");
	ident = TRUE;
    }

    free_turn_flag = FALSE;

    /* Analyze the first set of effects */    
    for (flg = i_ptr->flags1; flg; ) {

	/* Extract the next effect bit */
	j = bit_pos(&flg);

	/* Analyze the effect */
	switch (j + 1) {

	  case 1:
	    if (inc_stat(A_STR)) {
		msg_print("Wow!  What bulging muscles! ");
		ident = TRUE;
	    }
	    break;

	  case 2:
	    ident = TRUE;
	    lose_str();
	    break;

	  case 3:
	    if (res_stat(A_STR)) {
		msg_print("You feel warm all over.");
		ident = TRUE;
	    }
	    break;

	  case 4:
	    if (inc_stat(A_INT)) {
		msg_print("Aren't you brilliant! ");
		ident = TRUE;
	    }
	    break;

	  case 5:
	    ident = TRUE;
	    lose_int();
	    break;

	  case 6:
	    if (res_stat(A_INT)) {
		msg_print("You have a warm feeling.");
		ident = TRUE;
	    }
	    break;

	  case 7:
	    if (inc_stat(A_WIS)) {
		msg_print("You suddenly have a profound thought! ");
		ident = TRUE;
	    }
	    break;

	  case 8:
	    ident = TRUE;
	    lose_wis();
	    break;

	  case 9:
	    if (res_stat(A_WIS)) {
		msg_print("You feel your wisdom returning.");
		ident = TRUE;
	    }
	    break;

	  case 10:
	    if (inc_stat(A_CHR)) {
		msg_print("Gee, ain't you cute! ");
		ident = TRUE;
	    }
	    break;

	  case 11:
	    ident = TRUE;
	    lose_chr();
	    break;

	  case 12:
	    if (res_stat(A_CHR)) {
		msg_print("You feel your looks returning.");
		ident = TRUE;
	    }
	    break;

	  case 13:
	    if (hp_player(damroll(2, 7))) ident = TRUE;
	    if (p_ptr->cut > 0) {
		msg_print("Your wounds heal.");
		p_ptr->cut -= 10;
		if (p_ptr->cut < 0) p_ptr->cut = 0;
		ident = TRUE;
	    }
	    break;

	  case 14:
	    if (hp_player(damroll(4, 7))) ident = TRUE;
	    if (p_ptr->cut > 0) {
		msg_print("Your wounds heal.");
		p_ptr->cut = (p_ptr->cut / 2) - 50;
		if (p_ptr->cut < 0) p_ptr->cut = 0;
		ident = TRUE;
	    }
	    break;
	    
	  case 15:
	    if (hp_player(damroll(6, 7))) ident = TRUE;
	    if (p_ptr->cut > 0) {
		msg_print("Your wounds heal.");
		p_ptr->cut = 0;
		ident = TRUE;
	    }
	    if (p_ptr->stun > 0) {
		msg_print("Your head stops stinging.");
		if (p_ptr->stun > 50) {
		    p_ptr->ptohit += 20;
		    p_ptr->ptodam += 20;
		}
		else {
		    p_ptr->ptohit += 5;
		    p_ptr->ptodam += 5;
		}
		p_ptr->stun = 0;
		ident = TRUE;
	    }
	    break;
	    
	  case 16:
	    if (hp_player(400)) ident = TRUE;
	    if (p_ptr->stun > 0) {
		msg_print("Your head stops stinging.");
		if (p_ptr->stun > 50) {
		    p_ptr->ptohit += 20;
		    p_ptr->ptodam += 20;
		}
		else {
		    p_ptr->ptohit += 5;
		    p_ptr->ptodam += 5;
		}
		p_ptr->stun = 0;
		ident = TRUE;
	    }
	    if (p_ptr->cut > 0) {
		msg_print("Your wounds heal.");
		p_ptr->cut = 0;
		ident = TRUE;
	    }
	    break;

	  case 17:
	    if (inc_stat(A_CON)) {
		msg_print("You feel tingly for a moment.");
		ident = TRUE;
	    }
	    break;

	  case 18:
	    if (p_ptr->exp < MAX_EXP) {
		l = (p_ptr->exp / 2) + 10;
		if (l > 100000L) l = 100000L;
		p_ptr->exp += l;
		msg_print("You feel more experienced.");
		prt_experience();
		ident = TRUE;
	    }
	    break;

	  case 19:
	    if (!p_ptr->free_act) {
		/* paralysis must be zero, we are drinking */
		/* but what about multiple potion effects? */
		msg_print("You fall asleep.");
		p_ptr->paralysis += randint(4) + 4;
		ident = TRUE;
	    }
	    break;

	  case 20:
	    if (!p_ptr->resist_blind) {
		if (p_ptr->blind == 0) {
		    msg_print("You are covered by a veil of darkness.");
		    ident = TRUE;
		}
		p_ptr->blind += randint(100) + 100;
	    }
	    break;

	  case 21:
	    if (!p_ptr->resist_conf) {
		if (p_ptr->confused == 0) {
		    msg_print("Hey!  This is good stuff!  *Hick!*");
		    ident = TRUE;
		}
		p_ptr->confused += randint(20) + 12;
	    }
	    break;

	  case 22:
	    if (!(p_ptr->immune_pois ||
		  p_ptr->resist_pois ||
		  p_ptr->oppose_pois)) {
		msg_print("You feel very sick.");
		p_ptr->poisoned += randint(15) + 10;
		dec_stat(A_CON, 10, TRUE);
	    }
	    else {
		msg_print("The poison has no effect.");
	    }
	    ident = TRUE;
	    break;

	  case 23:
	    /* Speed is now cumulative, like slow */
	    if (p_ptr->fast == 0) ident = TRUE;
	    p_ptr->fast += randint(25) + 15;
	    break;

	  case 24:
	    if (p_ptr->slow == 0) ident = TRUE;
	    p_ptr->slow += randint(25) + 15;
	    break;

	  case 26:
	    if (inc_stat(A_DEX)) {
		msg_print("You feel more limber! ");
		ident = TRUE;
	    }
	    break;
	    
	  case 27:
	    if (res_stat(A_DEX)) {
		msg_print("You feel less clumsy.");
		ident = TRUE;
	    }
	    break;
	    
	  case 28:
	    if (res_stat(A_CON)) {
		msg_print("You feel your health returning! ");
		ident = TRUE;
	    }
	    break;

	  case 29:
	    if (cure_blindness()) ident = TRUE;
	    break;
	    
	  case 30:
	    if (cure_confusion()) ident = TRUE;
	    break;
	    
	  case 31:
	    if (cure_poison()) ident = TRUE;
	    break;

	  case 32:
	    break;		/* Unused */
	}
    }


    /* Analyze the second set of effects */
    for (flg = i_ptr->flags2; flg; ) {

	/* Extract the next effect bit */
	j = bit_pos(&flg);

	/* Various effects from Potions */
	switch (j + 32 + 1) {

	  case 33:
	    break;	/* Unused */
	    
	  case 34:
	    if (!p_ptr->hold_life && p_ptr->exp > 0) {
		int32               m, scale;

		msg_print("You feel your memories fade.");
		m = p_ptr->exp / 5;
		if (p_ptr->exp > MAX_SHORT) {
		    scale = MAX_LONG / p_ptr->exp;
		    m += (randint((int)scale) * p_ptr->exp) / (scale * 5);
		}
		else {
		    m += randint((int)p_ptr->exp) / 5;
		}
		lose_exp(m);
	    }
	    else {
		message("You feel your memories fade ", 0x02);
		message("for a moment, but quickly return.", 0);
	    }
	    ident = TRUE;
	    break;

	  case 35:
	    ident = cure_poison();
	    if (p_ptr->food > 150) p_ptr->food = 150;
	    p_ptr->paralysis = 4;
	    msg_print("The potion makes you vomit! ");
	    ident = TRUE;
	    break;

	  case 37:
	    if (p_ptr->hero == 0) ident = TRUE;
	    p_ptr->hero += randint(25) + 25;
	    break;

	  case 38:
	    if (p_ptr->shero == 0) ident = TRUE;
	    p_ptr->shero += randint(25) + 25;
	    break;

	  case 39:
	    if (remove_fear()) ident = TRUE;
	    break;

	  case 40:
	    if (restore_level()) ident = TRUE;
	    break;

	  case 41:
	    if (!p_ptr->oppose_fire) ident = TRUE;
	    p_ptr->oppose_fire += randint(10) + 10;
	    break;

	  case 42:
	    if (!p_ptr->oppose_cold) ident = TRUE;
	    p_ptr->oppose_cold += randint(10) + 10;
	    break;

	  case 43:
	    if (!p_ptr->detect_inv) ident = TRUE;
	    detect_inv2(randint(12) + 12);
	    break;

	  case 44:
	    if (slow_poison()) ident = TRUE;
	    break;

	  case 45:
	    if (cure_poison()) ident = TRUE;
	    break;

	  case 46:
	    if (p_ptr->cmana < p_ptr->mana) {
		p_ptr->cmana = p_ptr->mana;
		msg_print("Your feel your head clear.");
		prt_cmana();
		ident = TRUE;
	    }
	    break;

	  case 47:
	    if (p_ptr->tim_infra == 0) {
		msg_print("Your eyes begin to tingle.");
		ident = TRUE;
	    }
	    p_ptr->tim_infra += 100 + randint(100);
	    break;

	  case 48:
	    wiz_lite();
	    if (!res_stat(A_WIS)) inc_stat(A_WIS);
	    if (!res_stat(A_INT)) inc_stat(A_INT);
	    msg_print("You feel more enlightened! ");
	    msg_print(NULL);
	    /* after all, what is the key to enlightenment? -CFT */
	    self_knowledge();
	    identify_pack();
	    (void)detect_treasure();
	    (void)detect_object();
	    (void)detect_sdoor();
	    (void)detect_trap();
	    combo = 99;		/* See Below */
	    ident = TRUE;
	    break;

	  case 49:
	    msg_print("Massive explosions rupture your body! ");
	    take_hit(damroll(50, 20), "a potion of Detonation");
	    cut_player(5000);
	    stun_player(75);
	    ident = TRUE;
	    break;

	  case 50:
	    msg_print("A feeling of Death flows through your body.");
	    take_hit(5000, "a potion of Death");
	    ident = TRUE;
	    break;

	  case 51:
	    restore_level();
	    res_stat(A_STR);
	    res_stat(A_CON);
	    res_stat(A_DEX);
	    res_stat(A_WIS);
	    res_stat(A_INT);
	    res_stat(A_CHR);
	    hp_player(5000);
	    cure_poison();
	    cure_blindness();
	    cure_confusion();
	    p_ptr->cut = 0;
	    p_ptr->image = 0;
	    if (p_ptr->stun > 0) {
		if (p_ptr->stun > 50) {
		    p_ptr->ptohit += 20;
		    p_ptr->ptodam += 20;
		}
		else {
		    p_ptr->ptohit += 5;
		    p_ptr->ptodam += 5;
		}
		p_ptr->stun = 0;
	    }
	    message("You feel life flow through your body! ", 0);
	    ident = TRUE;
	    break;

	  case 52:	   /* Augm */
	    if (inc_stat(A_DEX)) ident = TRUE;
	    if (inc_stat(A_WIS)) ident = TRUE;
	    if (inc_stat(A_INT)) ident = TRUE;
	    if (inc_stat(A_STR)) ident = TRUE;
	    if (inc_stat(A_CHR)) ident = TRUE;
	    if (inc_stat(A_CON)) ident = TRUE;
	    if (ident) {
		msg_print("You feel power flow through your body! ");
	    }
	    break;

	  case 53:	   /* Ruination */
	    take_hit(damroll(10, 10), "a potion of Ruination");
	    dec_stat(A_DEX, 25, TRUE);
	    dec_stat(A_WIS, 25, TRUE);
	    dec_stat(A_CON, 25, TRUE);
	    dec_stat(A_STR, 25, TRUE);
	    dec_stat(A_CHR, 25, TRUE);
	    dec_stat(A_INT, 25, TRUE);
	    ident = TRUE;
	    msg_print("Your nerves and muscles feel weak and lifeless! ");
	    break;

	  case 54:
	    msg_print("An image of your surroundings forms in your mind...");
	    wiz_lite();
	    ident = TRUE;
	    break;

	  case 55:
	    msg_print("You feel you know yourself a little better...");
	    msg_print(NULL);
	    self_knowledge();
	    ident = TRUE;
	    break;

	  case 56:
	    if (hp_player(1200)) ident = TRUE;
	    if (p_ptr->stun > 0) {
		if (p_ptr->stun > 50) {
		    p_ptr->ptohit += 20;
		    p_ptr->ptodam += 20;
		}
		else {
		    p_ptr->ptohit += 5;
		    p_ptr->ptodam += 5;
		}
		p_ptr->stun = 0;
		msg_print("Your head stops stinging.");
		ident = TRUE;
	    }
	    if (p_ptr->cut > 0) {
		p_ptr->cut = 0;
		msg_print("Your wounds heal.");
		ident = TRUE;
	    }
	    if (cure_blindness()) ident = TRUE;
	    if (cure_confusion()) ident = TRUE;
	    if (cure_poison()) ident = TRUE;
	    break;
	}
    }


    inven_tried(i_ptr);

    if (ident && !inven_aware_p(i_ptr)) {
	int lev = k_list[i_ptr->k_idx].level;
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }

    /* Potions can feed the player */
    add_food(i_ptr->pval);

    inven_item_increase(item_val, -1);
    inven_item_describe(item_val);
    inven_item_optimize(item_val);

    /* Combine (if any left) */
    if (num > 1) item_val = combine(item_val);

    /* Hack -- combine pack items after potion of self knowledge */
    if (combo >= 0) combine_pack();
}





/*
 * Read a scroll -RAK-
 *
 * Hack -- if an "identify" spell is sucessfully used, then at the end of the
 * function, run "combine_pack()" to combine any items that have become similar.
 *
 * A single scroll object disappears.
 *
 * Currently, no scrolls use effects from both flags1 and flags2, but
 * we are ready if they ever decide to do so.
 *
 * Certain scrolls can be "aborted" without losing the scroll.  These
 * include scrolls with no effects but recharge or identify, which are
 * cancelled before use.  XXX Reading them still takes a turn, though.
 *
 * Oops.  Do not use "i_ptr" as a "temporary variable" inside the
 * "scroll effect" loop.  This will cause really bad things to happen.
 */
void read_scroll(void)
{
    int32u                flg;
    int                   item_val, i1, i2, x, y;
    int			  j, k;
    int                   used_up, ident, l, num, combo = -1;
    int                   tmp[6];
    register inven_type  *i_ptr, *j_ptr;
    bigvtype              out_val, tmp_str;

    free_turn_flag = TRUE;

    if (p_ptr->blind > 0) {
	msg_print("You can't see to read the scroll.");
	return;
    }

    if (no_lite()) {
	msg_print("You have no light to read by.");
	return;
    }

    if (p_ptr->confused > 0) {
	msg_print("You are too confused to read a scroll.");
	return;
    }

    if (inven_ctr == 0) {
	msg_print("You are not carrying anything!");
	return;
    }

    if (!find_range(TV_SCROLL, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any scrolls!");
	return;
    }

    if (!get_item(&item_val, "Read which scroll?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];
    num = i_ptr->number;

    /* The turn is not free */
    free_turn_flag = FALSE;

    /* Assume the scroll will get used up */
    used_up = TRUE;

    /* Not identified yet */
    ident = FALSE;

    /* Apply the first set of scroll effects */
    for (flg = i_ptr->flags1; flg; ) {

	/* XXX XXX Hack -- only "pure" scrolls can be conserved */
	used_up = TRUE;

	/* Extract the next effect bit-flag */
	j = bit_pos(&flg);

	/* Scrolls. */
	switch (j+1) {

	  case 1:
	    j_ptr = &inventory[INVEN_WIELD];
	    if (j_ptr->tval != TV_NOTHING) {
		objdes(tmp_str, j_ptr, FALSE);
		(void) sprintf(out_val, "Your %s glows faintly! ", tmp_str);
		msg_print(out_val);
		if (!enchant(j_ptr, 1, ENCH_TOHIT)) {
		    msg_print("The enchantment fails. ");
		}
		ident = TRUE;
	    }
	    break;

	  case 2:
	    j_ptr = &inventory[INVEN_WIELD];
	    if (j_ptr->tval != TV_NOTHING) {
		objdes(tmp_str, j_ptr, FALSE);
		(void) sprintf(out_val, "Your %s glows faintly! ", tmp_str);
		msg_print(out_val);
		if (!enchant(j_ptr, 1, ENCH_TODAM)) {
		    msg_print("The enchantment fails. ");
		}
		ident = TRUE;
	    }
	    break;		

	  case 3:

	    /* Hack -- make a "list" of "armor" indexes, size is "k" */
	    k = 0;
	    if (inventory[INVEN_BODY].tval)  tmp[k++] = INVEN_BODY;
	    if (inventory[INVEN_ARM].tval)   tmp[k++] = INVEN_ARM;
	    if (inventory[INVEN_OUTER].tval) tmp[k++] = INVEN_OUTER;
	    if (inventory[INVEN_HANDS].tval) tmp[k++] = INVEN_HANDS;
	    if (inventory[INVEN_HEAD].tval)  tmp[k++] = INVEN_HEAD;
	    if (inventory[INVEN_FEET].tval)  tmp[k++] = INVEN_FEET;

	    /* Nothing to enchant */
	    if (!k) break;

	    /* Pick an item to enchant */
	    for (l = 0; l < 10; l++) {

		/* Pick a random item */
		int n = tmp[rand_int(k)];
		j_ptr = &inventory[n];

		/* Hack -- no other items */
		if (k <= 1) break;
		
		/* Lock instantly on to cursed items */
		if (cursed_p(j_ptr)) break;
	    }
	    
	    /* Visual effect (known) */
	    objdes(tmp_str, j_ptr, FALSE);
	    (void) sprintf(out_val, "Your %s glows faintly! ", tmp_str);
	    msg_print(out_val);
	    ident = TRUE;

	    /* Attempt to enchant */
	    if (!enchant(j_ptr, 1, ENCH_TOAC)) {
		msg_print("The enchantment fails. ");
	    }

	    break;

	  case 4:

	    /* Can be identified by "label" on scroll */
	    msg_print("This is an identify scroll.");
	    ident = TRUE;

	    /* Check the floor, and then Check the inventory */
	    if (!ident_floor()) {

		/* See what item is getting identified */
		combo = ident_spell();

		/* XXX Allow the scroll to be preserved */
		if (combo < 0) used_up = FALSE;
	    }
	    break;

	  case 5:
	    if (remove_curse()) {
		msg_print("You feel as if someone is watching over you.");
		ident = TRUE;
	    }
	    break;

	  case 6:
	    ident = lite_area(char_row, char_col, damroll(2, 12), 2) || ident;
	    break;

	  case 7:
	    for (k = 0; k < randint(3); k++) {
		y = char_row;
		x = char_col;
		ident = summon_monster(&y, &x, FALSE) || ident;
	    }
	    break;

	  case 8:
	    teleport(10);
	    ident = TRUE;
	    break;

	  case 9:
	    teleport(100);
	    ident = TRUE;
	    break;

	  case 10:
	    (void)tele_level();
	    ident = TRUE;
	    break;

	  case 11:
	    if (p_ptr->confusing == 0) {
		msg_print("Your hands begin to glow.");
		p_ptr->confusing = TRUE;
		ident = TRUE;
	    }
	    break;

	  case 12:
	    map_area();
	    ident = TRUE;
	    break;

	  case 13:
	    ident = sleep_monsters1(char_row, char_col) || ident;
	    break;

	  case 14:
	    warding_glyph();
	    ident = TRUE;
	    break;

	  case 15:
	    ident = detect_treasure() || ident;
	    break;
	    
	  case 16:
	    ident = detect_object() || ident;
	    break;
	    
	  case 17:
	    ident = detect_trap() || ident;
	    break;
	    
	  case 18:
	    ident = detect_sdoor() || ident;
	    break;

	  case 19:
	    msg_print("This is a mass genocide scroll.");
	    mass_genocide(TRUE);
	    ident = TRUE;
	    break;

	  case 20:
	    ident = detect_invisible() || ident;
	    break;

	  case 21:
	    if (aggravate_monster(20)) {
		msg_print("There is a high pitched humming noise.");
		ident = TRUE;
	    }
	    break;

	  case 22:
	    ident = trap_creation() || ident;
	    break;

	  case 23:
	    ident = td_destroy() || ident;
	    break;

	  /* Not Used, used to be door creation */
	  case 24:
	    break;

	  case 25:
	    msg_print("This is a Recharge-Item scroll.");
	    /* Hack -- Like identify, recharge can be cancelled */
	    used_up = recharge(60);
	    ident = TRUE;
	    break;

	  case 26:
	    msg_print("This is a genocide scroll.");
	    genocide(TRUE);
	    ident = TRUE;
	    break;

	  case 27:
	    ident = unlite_area(char_row, char_col) || ident;
	    if (!p_ptr->resist_blind) {
		p_ptr->blind += 3 + randint(5);
	    }
	    break;

	  case 28:
	    ident = protect_evil() || ident;
	    break;

	  case 29:
	    satisfy_hunger();
	    ident = TRUE;
	    break;

	  case 30:
	    ident = dispel_creature(MF2_UNDEAD, 60) || ident;
	    break;

	  case 31:
	    remove_all_curse();
	    ident = TRUE;
	    break;

	  case 32:
	    break;	/* Undefined */
	}
    }

    /* Apply the second set of scroll effects */
    for (flg = i_ptr->flags2; flg; ) {

	/* XXX Hack -- only "pure" scrolls can be conserved */
	used_up = TRUE;

	/* Extract the next "bit flag" */
	j = bit_pos(&flg);

	/* Analyze the effect */
	switch (j + 32 + 1) {

	  case 33:
	    j_ptr = &inventory[INVEN_WIELD];
	    if (j_ptr->tval != TV_NOTHING) {
		objdes(tmp_str, j_ptr, FALSE);
		(void) sprintf(out_val, "Your %s glows brightly!", tmp_str);
		msg_print(out_val);
		if (!enchant(j_ptr, randint(3), ENCH_TOHIT|ENCH_TODAM)) {
		    msg_print("The enchantment fails.");
		}
		ident = TRUE;
	    }
	    break;

	  case 34:
	    j_ptr = &inventory[INVEN_WIELD];
	    if (j_ptr->tval != TV_NOTHING) {
		objdes(tmp_str, j_ptr, FALSE);
		if (artifact_p(j_ptr) && (randint(7) < 4)) {
		    message("A terrible black aura tries to", 0x02);
		    message(" surround your weapon, but your ", 0x02);
		    message(tmp_str, 0x02);
		    message(" resists the effects!", 0);
		}

		/* not artifact or failed save... */
		else {
		
		    /* Oops */
		    message(format("A terrible black aura blasts your %s!",
				   tmp_str), 0);

		    /* XXX This may be "broken" */
		    py_bonuses(j_ptr, -1);
		    j_ptr->name1 = 0;
		    j_ptr->name2 = EGO_SHATTERED;
		    j_ptr->tohit = (-randint(5) - randint(5));
		    j_ptr->todam = (-randint(5) - randint(5));
		    j_ptr->flags3 = TR3_CURSED;
		    j_ptr->flags2 = 0L;
		    j_ptr->flags1 = 0L;
		    j_ptr->damage[0] = j_ptr->damage[1] = 1;
		    j_ptr->toac = 0;
		    j_ptr->cost = 0L;
		    py_bonuses(j_ptr, 1);
		    calc_bonuses();
		}

		ident = TRUE;
	    }
	    break;

	  case 35:

	    /* Hack -- make a "list" of "armor" indexes, size is "k" */
	    k = 0;
	    if (inventory[INVEN_BODY].tval)  tmp[k++] = INVEN_BODY;
	    if (inventory[INVEN_ARM].tval)   tmp[k++] = INVEN_ARM;
	    if (inventory[INVEN_OUTER].tval) tmp[k++] = INVEN_OUTER;
	    if (inventory[INVEN_HANDS].tval) tmp[k++] = INVEN_HANDS;
	    if (inventory[INVEN_HEAD].tval)  tmp[k++] = INVEN_HEAD;
	    if (inventory[INVEN_FEET].tval)  tmp[k++] = INVEN_FEET;

	    /* Nothing to enchant */
	    if (!k) break;

	    /* Pick an item to enchant */
	    for (l = 0; l < 10; l++) {

		/* Pick a random item */
		int n = tmp[rand_int(k)];
		j_ptr = &inventory[n];

		/* Hack -- no other items */
		if (k <= 1) break;
		
		/* Lock instantly on to cursed items */
		if (cursed_p(j_ptr)) break;
	    }
	    
	    /* Message (and knowledge) */
	    objdes(tmp_str, j_ptr, FALSE);
	    (void) sprintf(out_val,"Your %s glows brightly!", tmp_str);
	    msg_print(out_val);
	    ident = TRUE;

	    /* Attempt to enchant */
	    if (!enchant(j_ptr, randint(3)+1, ENCH_TOAC)) {
		msg_print("The enchantment fails.");
	    }

	    break;

	  case 36:

	    /* Hack -- make a "list" of "armor" indexes, size is "k" */
	    k = 0;
	    if (inventory[INVEN_BODY].tval)  tmp[k++] = INVEN_BODY;
	    if (inventory[INVEN_ARM].tval)   tmp[k++] = INVEN_ARM;
	    if (inventory[INVEN_OUTER].tval) tmp[k++] = INVEN_OUTER;
	    if (inventory[INVEN_HANDS].tval) tmp[k++] = INVEN_HANDS;
	    if (inventory[INVEN_HEAD].tval)  tmp[k++] = INVEN_HEAD;
	    if (inventory[INVEN_FEET].tval)  tmp[k++] = INVEN_FEET;

	    /* Nothing to fuck with */
	    if (!k) break;

	    /* Pick an item to fuck with */
	    for (l = 0; l < 10; l++) {

		/* Pick a random item */
		int n = tmp[rand_int(k)];
		j_ptr = &inventory[n];

		/* Hack -- no other items */
		if (k <= 1) break;
		
		/* Lock instantly on to "good" items */
		if (!cursed_p(j_ptr)) break;
	    }

	    /* Describe */
	    objdes(tmp_str, j_ptr, FALSE);

	    /* Attempt a saving throw for artifacts */
	    if (artifact_p(j_ptr) && (randint(7) < 4)) {
		message("A terrible black aura tries to surround your", 0x02);
		message(tmp_str, 0x02);
		message(", but it resists the effects!", 0);
	    }

	    /* not artifact or failed save... */
	    else {

		/* Oops */
		message(format("A terrible black aura blasts your %s!",
			       tmp_str), 0);

		/* XXX This may be "broken" */
		py_bonuses(j_ptr, -1);
		j_ptr->name1 = 0;
		j_ptr->name2 = EGO_BLASTED;
		j_ptr->flags3 = TR3_CURSED;
		j_ptr->flags2 = 0L;
		j_ptr->flags1 = 0L;
		j_ptr->toac = (-randint(5) - randint(5));
		j_ptr->tohit = j_ptr->todam = 0;
		j_ptr->ac = (j_ptr->ac > 9) ? 1 : 0;
		j_ptr->cost = 0L;

		/* now apply new "bonuses" -CFT */
		py_bonuses(j_ptr, 1);
		calc_bonuses();

		/* Well, you know all about it */
		ident = TRUE;
	    }
	    break;

	  case 37:
	    for (k = 0; k < randint(3); k++) {
		y = char_row;
		x = char_col;
		ident = summon_undead(&y, &x) || ident;
	    }
	    break;

	  case 38:
	    bless(randint(12) + 6);
	    ident = TRUE;
	    break;

	  case 39:
	    bless(randint(24) + 12);
	    ident = TRUE;
	    break;

	  case 40:
	    bless(randint(48) + 24);
	    ident = TRUE;
	    break;

	  case 41:
	    if (p_ptr->word_recall == 0) {
		p_ptr->word_recall = 15 + randint(20);
		msg_print("The air about you becomes charged...");
	    }
	    else {
		p_ptr->word_recall = 0;
		msg_print("A tension leaves the air around you...");
	    }
	    ident = TRUE;
	    break;

	  case 42:
	    destroy_area(char_row, char_col);
	    ident = TRUE;
	    break;

	  case 43:
	    place_good(char_row, char_col, TRUE);
	    prt_map();
	    ident = TRUE;
	    break;

	  case 44:
	    special_random_object(char_row, char_col, 1);
	    prt_map();
	    ident = TRUE;
	    break;
	}
    }


    inven_tried(i_ptr);

    if (ident && !inven_aware_p(i_ptr)) {
	int lev = k_list[i_ptr->k_idx].level;
	/* round half-way case up */
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }


    /* XXX Hack -- allow certain scrolls to be "preserved" */
    if (!used_up) return;

    /* Use a single scroll, describe the result */
    inven_item_increase(item_val, -1);
    inven_item_describe(item_val);
    inven_item_optimize(item_val);

    /* Combine (if any left) */
    if (num > 1) item_val = combine(item_val);

    /* Hack -- combine pack items after identify scroll. */
    if (combo >= 0) combine_pack();
}




/*
 * Aim a wand.  Be sure to unstack it first.
 * A single "charge" disappears.
 */
void aim_wand(void)
{
    int                   ident, chance, dir, sval;
    int                   item_val, i1, i2, x, y, lev;
    register inven_type  *i_ptr;
    inven_type		tmp_obj;

    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    if (!find_range(TV_WAND, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any wands.");
	return;
    }

    if (!get_item(&item_val, "Aim which wand?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Get the level */
    lev = k_list[i_ptr->k_idx].level;

    /* Hack -- prepare to unstack the extra wands, if possible */
    if (i_ptr->number > 1) {
	tmp_obj = *i_ptr;
	tmp_obj.number--;
	if (inven_ctr >= INVEN_WIELD) {
	    /* XXX Hack -- Overly pessimistic */
	    message("No room to unstack, drop something first.", 0);
	    return;
	}
    }

    /* Allow direction to be cancelled for free */
    if (!get_dir_c(NULL, &dir)) return;

    free_turn_flag = FALSE;

    chance = (p_ptr->save + stat_adj(A_INT) - (int)(lev > 42 ? 42 : lev) +
	      (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused > 0) chance = chance / 2;

    /* Give everyone a slight chance */
    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
	chance = USE_DEVICE;
    }

    if (chance <= 0) chance = 1;

    if (randint(chance) < USE_DEVICE) {
	msg_print("You failed to use the wand properly.");
	return;
    }

    /* The wand is already empty! */
    if (i_ptr->pval <= 0) {
	msg_print("The wand has no charges left.");
	if (!known2_p(i_ptr)) {
	    i_ptr->ident |= ID_EMPTY;
	}
	return;
    }

    /* Not identified yet */
    ident = FALSE;

    /* Extract the "sval" effect */
    sval = i_ptr->sval;

    /* XXX Hack -- Wand of wonder can do anything before it */
    if (sval == SV_WAND_WONDER) sval = rand_int(SV_WAND_WONDER);

    /* Start at the player */
    y = char_row;
    x = char_col;

    /* Various effects */
    switch (sval) {

	case SV_WAND_LITE:
	    msg_print("A line of blue shimmering light appears.");
	    lite_line(dir, char_row, char_col);
	    ident = TRUE;
	    break;

	case SV_WAND_ACID:
	    if (randint(5)==1)
		line_spell(GF_ACID,dir,y,x,damroll(5,8));
	    else
		fire_bolt(GF_ACID,dir,y,x,damroll(5,8));
	    ident = TRUE;
	    break;

	case SV_WAND_ELEC:
	    if (randint(6)==1)
		line_spell(GF_ELEC,dir,y,x,damroll(3,8));
	    else
		fire_bolt(GF_ELEC, dir,y,x,damroll(3, 8));
	    ident = TRUE;
	    break;

	case SV_WAND_COLD:
	    if (randint(6)==1)
		line_spell(GF_COLD,dir,y,x,damroll(3,8));
	    else
		fire_bolt(GF_COLD, dir,y,x,damroll(3, 8));
	    ident = TRUE;
	    break;

	case SV_WAND_FIRE:
	    if (randint(4)==1)
		line_spell(GF_FIRE,dir,y,x,damroll(6,8));
	    else
		fire_bolt(GF_FIRE,dir,y,x,damroll(6, 8));
	    ident = TRUE;
	    break;

	case SV_WAND_STONE_TO_MUD:
	    ident = wall_to_mud(dir,y,x) || ident;
	    break;

	case SV_WAND_POLYMORPH:
	    ident = poly_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_HEAL_MONSTER:
	    ident = heal_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_HASTE_MONSTER:
	    ident = speed_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_SLOW_MONSTER:
	    ident = slow_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_CONFUSE_MONSTER:
	    ident = confuse_monster(dir,y,x,10) || ident;
	    break;

	case SV_WAND_SLEEP_MONSTER:
	    ident = sleep_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_DRAIN_LIFE:
	    ident = drain_life(dir,y,x,75) || ident;
	    break;

	case SV_WAND_TRAP_DOOR_DEST:
	    ident = td_destroy2(dir,y,x) || ident;
	    break;

	case SV_WAND_MAGIC_MISSILE:
	    if (randint(6)==1)
		line_spell(GF_MISSILE,dir,y,x,damroll(2,6));
	    else
		fire_bolt(GF_MISSILE, dir,y,x,damroll(2, 6));
	    ident = TRUE;
	    break;

	case SV_WAND_FEAR_MONSTER:
	    ident = fear_monster(dir,y,x,10) || ident;
	    break;

	case SV_WAND_CLONE_MONSTER:
	    ident = clone_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_TELEPORT_AWAY:
	    ident = teleport_monster(dir,y,x) || ident;
	    break;

	case SV_WAND_DISARMING:
	    ident = disarm_all(dir,y,x) || ident;
	    break;

	case SV_WAND_ELEC_BALL:
	    fire_ball(GF_ELEC, dir,y,x,32,2);
	    ident = TRUE;
	    break;

	case SV_WAND_COLD_BALL:
	    fire_ball(GF_COLD,dir,y,x,48,2);
	    ident = TRUE;
	    break;

	case SV_WAND_FIRE_BALL:
	    fire_ball(GF_FIRE,dir,y,x,72,2);
	    ident = TRUE;
	    break;

	case SV_WAND_STINKING_CLOUD:
	    fire_ball(GF_POIS,dir,y,x,12,2);
	    ident = TRUE;
	    break;

	case SV_WAND_ACID_BALL:
	    fire_ball(GF_ACID,dir,y,x,60,2);
	    ident = TRUE;
	    break;

	case SV_WAND_WONDER:
	    message("Oops.  Wand of wonder activated.", 0);
	    break;

	case SV_WAND_DRAGON_FIRE:
	    fire_ball(GF_FIRE, dir, y, x, 100, 3);
	    ident = TRUE;
	    break;

	case SV_WAND_DRAGON_COLD:
	    fire_ball(GF_COLD, dir, y, x, 80, 3);
	    ident = TRUE;
	    break;

	case SV_WAND_DRAGON_BREATH:
	    switch (randint(5)) {
	      case 1:
		fire_ball(GF_FIRE, dir, y, x, 100, 3);
		break;
	      case 2:
		fire_ball(GF_COLD, dir, y, x, 80, 3);
		break;
	      case 3:
		fire_ball(GF_ACID, dir, y, x, 90, 3);
		break;
	      case 4:
		fire_ball(GF_ELEC, dir, y, x, 70, 3);
		break;
	      default:
		fire_ball(GF_POIS, dir, y, x, 70, 3);
		break;
	    }
	    ident = TRUE;
	    break;

	case SV_WAND_ANNIHILATION:
	    ident = drain_life(dir,y,x,125) || ident;
	    break;

	default:
	    msg_print("Oops.  Undefined wand effect.");
	    break;
    }


    /* Mark it as tried */
    inven_tried(i_ptr);

    /* Apply identification */
    if (ident && !inven_aware_p(i_ptr)) {
	/* round half-way case up */
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }

    /* Decrease the charges */
    i_ptr->pval--;

    /* Hack -- complete the "unstacking" */
    if (i_ptr->number > 1) {
	int where;
	i_ptr->number = 1;
	where = inven_carry(&tmp_obj);
	inven_weight -= tmp_obj.weight * tmp_obj.number;
    }

    /* Combine */
    item_val = combine(item_val);

    /* Describe the remaining charges */
    inven_item_charges(item_val);

    /* XXX No need to combine the pack */
}




/*
 * Use a staff.			-RAK-	
 *
 * One charge of one staff disappears.
 *
 * Hack -- staffs of genocide and identify can be safely cancelled.
 */
void use_staff(void)
{
    int			  ident, chance, k, lev, combo = -1;
    int                   item_val, i1, i2, x, y;
    register inven_type  *i_ptr;
    inven_type		tmp_obj;

    /* Hack -- let genocide get aborted */
    bool use_charge = TRUE;

    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    if (!find_range(TV_STAFF, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any staffs.");
	return;
    }

    if (!get_item(&item_val, "Use which staff?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];

    lev = k_list[i_ptr->k_idx].level;

    /* Hack -- prepare to unstack the extra staffs, if possible */
    if (i_ptr->number > 1) {
	tmp_obj = *i_ptr;
	tmp_obj.number--;
	if (inven_ctr >= INVEN_WIELD) {
	    /* XXX Hack -- Overly pessimistic */
	    message("No room to unstack, drop something first.", 0);
	    return;
	}
    }


    /* Take a turn */
    free_turn_flag = FALSE;

    chance = (p_ptr->save + stat_adj(A_INT) - (int)(lev > 50 ? 50 : lev) +
	      (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused > 0) chance = chance / 2;

    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
	chance = USE_DEVICE;   /* Give everyone a slight chance */
    }

    if (chance <= 0) chance = 1;

    if (randint(chance) < USE_DEVICE) {
	msg_print("You failed to use the staff properly.");
	return;
    }

    if (i_ptr->pval <= 0) {
	msg_print("The staff has no charges left.");
	if (!known2_p(i_ptr)) i_ptr->ident |= ID_EMPTY;
	return;
    }

    ident = FALSE;

    switch (i_ptr->sval) {

      case SV_STAFF_HEALING:
	ident = hp_player(300);
	if (p_ptr->stun > 0) {
	    if (p_ptr->stun > 50) {
		p_ptr->ptohit += 20;
		p_ptr->ptodam += 20;
	    }
	    else {
		p_ptr->ptohit += 5;
		p_ptr->ptodam += 5;
	    }
	    p_ptr->stun = 0;
	    ident = TRUE;
	    msg_print("Your head stops stinging.");
	}
	if (p_ptr->cut > 0) {
	    p_ptr->cut = 0;
	    ident = TRUE;
	    msg_print("You feel better.");
	}
	break;

      case SV_STAFF_GENOCIDE:
	genocide(FALSE);
	/* XXX Hack -- genocide sets "free_turn_flag" if cancelled */
	if (free_turn_flag) use_charge = FALSE;
	/* Hack -- take a turn to test the staff */
	free_turn_flag = FALSE;
	ident = TRUE;
	break;

      case SV_STAFF_PROBING:
	probing();
	ident = TRUE;
	break;

      case SV_STAFF_IDENTIFY:
	/* Check floor, or inventory plus combining */
	if (!ident_floor()) {
	    combo = ident_spell();
	    if (combo < 0) use_charge = FALSE;
	}
	ident = TRUE;
	break;

      case SV_STAFF_HOLINESS:
	dispel_creature(MF2_EVIL, 120);
	protect_evil();
	cure_poison();
	remove_fear();
	hp_player(50);
	if (p_ptr->stun > 0) {
	    if (p_ptr->stun > 50) {
		p_ptr->ptohit += 20;
		p_ptr->ptodam += 20;
	    }
	    else {
		p_ptr->ptohit += 5;
		p_ptr->ptodam += 5;
	    }
	    p_ptr->stun = 0;
	    ident = TRUE;
	    msg_print("Your head stops stinging.");
	}
	if (p_ptr->cut > 0) {
	    p_ptr->cut = 0;
	    ident = TRUE;
	    msg_print("You feel better.");
	}
	ident = TRUE;
	break;

      case SV_STAFF_THE_MAGI:
	if (res_stat(A_INT)) {
	    msg_print("You have a warm feeling.");
	    ident = TRUE;
	}
	if (p_ptr->cmana < p_ptr->mana) {
	    p_ptr->cmana = p_ptr->mana;
	    ident = TRUE;
	    msg_print("Your feel your head clear.");
	    prt_cmana();
	}
	break;

      case SV_STAFF_POWER:
	dispel_creature(0xFFFFFFFFL, 120);
	break;

      case SV_STAFF_MAPPING:
	map_area();
	ident = TRUE;
	break;

      case SV_STAFF_LITE:
	ident = lite_area(char_row, char_col, damroll(2, 10), 2);
	break;

      case SV_STAFF_DOOR_STAIR_LOC:
	ident = detect_sdoor();
	break;

      case SV_STAFF_TRAP_LOC:
	ident = detect_trap();
	break;

      case SV_STAFF_TREASURE_LOC:
	ident = detect_treasure();
	break;

      case SV_STAFF_OBJECT_LOC:
	ident = detect_object();
	break;

      case SV_STAFF_TELEPORTATION:
	teleport(100);
	ident = TRUE;
	break;

      case SV_STAFF_EARTHQUAKES:
	earthquake();
	ident = TRUE;
	break;

      case SV_STAFF_SUMMONING:
	for (k = 0; k < randint(4); k++) {
	    y = char_row;
	    x = char_col;
	    ident = summon_monster(&y, &x, FALSE) || ident;
	}
	break;

      case SV_STAFF_DESTRUCTION:
	destroy_area(char_row, char_col);
	ident = TRUE;
	break;

      case SV_STAFF_STARLITE:
	starlite(char_row, char_col);
	ident = TRUE;
	break;

      case SV_STAFF_HASTE_MONSTERS:
	ident = speed_monsters(1) || ident;
	break;

      case SV_STAFF_SLOW_MONSTERS:
	ident = speed_monsters(-1) || ident;
	break;

      case SV_STAFF_SLEEP_MONSTERS:
	ident = sleep_monsters2() || ident;
	break;

      case SV_STAFF_CURE_LIGHT:
	ident = hp_player(randint(8)) || ident;
	break;

      case SV_STAFF_DETECT_INVIS:
	ident = detect_invisible() || ident;
	break;

      case SV_STAFF_SPEED:
	if (p_ptr->fast == 0) ident = TRUE;
	p_ptr->fast += randint(30) + 15;
	break;

      case SV_STAFF_SLOWNESS:
	if (p_ptr->slow == 0) ident = TRUE;
	p_ptr->slow += randint(30) + 15;
	break;

      case SV_STAFF_REMOVE_CURSE:
	if (remove_curse()) {
	    if (p_ptr->blind < 1) {
		msg_print("The staff glows blue for a moment..");
	    }
	    ident = TRUE;
	}
	break;

      case SV_STAFF_DETECT_EVIL:
	ident = detect_evil() || ident;
	break;

      case SV_STAFF_CURING:
	if (cure_blindness()) ident = TRUE;
	if (cure_poison()) ident = TRUE;
	if (cure_confusion()) ident = TRUE;
	if (p_ptr->stun > 0) {
	    msg_print("Your head stops stinging.");
	    if (p_ptr->stun > 50) {
		p_ptr->ptohit += 20;
		p_ptr->ptodam += 20;
	    }
	    else {
		p_ptr->ptohit += 5;
		p_ptr->ptodam += 5;
	    }
	    p_ptr->stun = 0;
	    ident = TRUE;
	}
	if (p_ptr->cut > 0) {
	    msg_print("You feel better.");
	    p_ptr->cut = 0;
	    ident = TRUE;
	}
	break;

      case SV_STAFF_DISPEL_EVIL:
	ident = dispel_creature(MF2_EVIL, 60) || ident;
	break;

      case SV_STAFF_DARKNESS:
	ident = unlite_area(char_row, char_col) || ident;
	break;

      default:
	msg_print("Oops.  Undefined staff.");
	break;
    }


    /* Tried the item */
    inven_tried(i_ptr);

    if (ident && !inven_aware_p(i_ptr)) {
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }

    /* Hack -- some uses are "free" */
    if (!use_charge) return;


    /* Use up a charge */
    i_ptr->pval--;

    /* Hack -- complete the "unstacking" */
    if (i_ptr->number > 1) {
	int where;
	i_ptr->number = 1;
	where = inven_carry(&tmp_obj);
	inven_weight -= tmp_obj.weight * tmp_obj.number;
    }

    /* Describe the remaining charges */
    inven_item_charges(item_val);

    /* Hack -- Combine */
    item_val = combine(item_val);

    /* Combine the pack after "staff of perceptions" */
    if (combo >= 0) combine_pack();
}





/*
 * Activate (zap) a Rod
 *
 * Unstack fully charged rods as needed.
 */
void zap_rod(void)
{
    int                 ident, chance, dir, lev;
    int                 item_val, i1, i2, x, y;
    register inven_type *i_ptr;
    inven_type		tmp_obj;

    /* Hack -- let genocide get aborted */
    bool use_charge = TRUE;

    /* Allow "Rod of Identify" to combine items */
    int combo = -1;


    /* Assume free turn */
    free_turn_flag = TRUE;

    if (inven_ctr == 0) {
	msg_print("But you are not carrying anything.");
	return;
    }

    if (!find_range(TV_ROD, TV_NEVER, &i1, &i2)) {
	msg_print("You are not carrying any rods.");
	return;
    }

    if (!get_item(&item_val, "Activate which rod?", i1, i2, 0)) {
	return;
    }

    /* Get the item */
    i_ptr = &inventory[item_val];

    lev = k_list[i_ptr->k_idx].level;

    /* Hack -- prepare to unstack the extra rods, if possible */
    if (i_ptr->number > 1) {
	tmp_obj = *i_ptr;
	tmp_obj.number--;
	if (inven_ctr >= INVEN_WIELD) {
	    /* XXX Hack -- Overly pessimistic */
	    message("No room to unstack, drop something first.", 0);
	    return;
	}
    }

    /* Take a turn */
    free_turn_flag = FALSE;

    chance = (p_ptr->save + (stat_adj(A_INT) * 2) - (int)((lev > 70) ? 70 : lev) +
	      (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused > 0) chance = chance / 2;

    /* Give everyone a slight chance */
    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
	chance = USE_DEVICE;
    }

    /* Prevent errors in "randint" */
    if (chance <= 0) chance = 1;

    /* Fail to use */
    if (randint(chance) < USE_DEVICE) {
	msg_print("You failed to use the rod properly.");
	return;
    }

    /* Still charging */
    if (i_ptr->pval) {
	msg_print("The rod is still charging.");
	return;
    }

    /* Not identified yet */
    ident = FALSE;

    /* Starting location */
    y = char_row;
    x = char_col;

    /* Activate it */
    switch (i_ptr->sval) {

      case SV_ROD_LIGHT:
	if (!get_dir_c(NULL, &dir)) return;
	msg_print("A line of blue shimmering light appears.");
	lite_line(dir, char_row, char_col);
	ident = TRUE;
	i_ptr->pval = 9;
	break;

      case SV_ROD_ILLUMINATION:
	lite_area(y, x, damroll(2, 8), 2);
	ident = TRUE;
	i_ptr->pval = 30;
	break;

      case SV_ROD_ACID:
	if (!get_dir_c(NULL, &dir)) return;
	if (randint(10)==1)
	    line_spell(GF_ACID,dir,y,x,damroll(6,8));
	else
	    fire_bolt(GF_ACID,dir,y,x,damroll(6,8));
	ident = TRUE;
	i_ptr->pval = 12;
	break;

      case SV_ROD_ELEC:
	if (!get_dir_c(NULL, &dir)) return;
	if (randint(12)==1)
	    line_spell(GF_ELEC, dir, y, x, damroll(3, 8));
	else
	    fire_bolt(GF_ELEC, dir, y, x, damroll(3, 8));
	ident = TRUE;
	i_ptr->pval = 11;
	break;

      case SV_ROD_COLD:
	if (!get_dir_c(NULL, &dir)) return;
	if (randint(10)==1)
	    line_spell(GF_COLD, dir, y, x, damroll(5, 8));
	else
	    fire_bolt(GF_COLD, dir, y, x, damroll(5, 8));
	ident = TRUE;
	i_ptr->pval = 13;
	break;

      case SV_ROD_FIRE:
	if (!get_dir_c(NULL, &dir)) return;
	if (randint(8)==1)
	    line_spell(GF_FIRE, dir, y, x, damroll(8, 8));
	else
	    fire_bolt(GF_FIRE, dir, y, x, damroll(8, 8));
	ident = TRUE;
	i_ptr->pval = 15;
	break;

      case SV_ROD_POLYMORPH:
	if (!get_dir_c(NULL, &dir)) return;
	ident = poly_monster(dir, y, x);
	i_ptr->pval = 25;
	break;

      case SV_ROD_SLOW_MONSTER:
	if (!get_dir_c(NULL, &dir)) return;
	ident = slow_monster(dir, y, x);
	i_ptr->pval = 20;
	break;

      case SV_ROD_SLEEP_MONSTER:
	if (!get_dir_c(NULL, &dir)) return;
	ident = sleep_monster(dir, y, x);
	i_ptr->pval = 18;
	break;

      case SV_ROD_DRAIN_LIFE:
	if (!get_dir_c(NULL, &dir)) return;
	ident = drain_life(dir, y, x, 75);
	i_ptr->pval = 23;
	break;

      case SV_ROD_TELEPORT_AWAY:
	if (!get_dir_c(NULL, &dir)) return;
	ident = teleport_monster(dir, y, x);
	i_ptr->pval = 25;
	break;

      case SV_ROD_DISARMING:
	if (!get_dir_c(NULL, &dir)) return;
	ident = disarm_all(dir, y, x);
	i_ptr->pval = 30;
	break;

      case SV_ROD_ELEC_BALL:
	if (!get_dir_c(NULL, &dir)) return;
	fire_ball(GF_ELEC, dir, y, x, 32, 2);
	ident = TRUE;
	i_ptr->pval = 23;
	break;

      case SV_ROD_COLD_BALL:
	if (!get_dir_c(NULL, &dir)) return;
	fire_ball(GF_COLD, dir, y, x, 48, 2);
	ident = TRUE;
	i_ptr->pval = 25;
	break;

      case SV_ROD_FIRE_BALL:
	if (!get_dir_c(NULL, &dir)) return;
	fire_ball(GF_FIRE, dir, y, x, 72, 2);
	ident = TRUE;
	i_ptr->pval = 30;
	break;

      case SV_ROD_ACID_BALL:
	if (!get_dir_c(NULL, &dir)) return;
	fire_ball(GF_ACID, dir, y, x, 60, 2);
	ident = TRUE;
	i_ptr->pval = 27;
	break;

      case SV_ROD_MAPPING:
	map_area();
	ident = TRUE;
	i_ptr->pval = 99;
	break;

      case SV_ROD_IDENTIFY:

	/* We know what it is now */
	ident = TRUE;

	/* Check the floor, and then Check the inventory */
	if (!ident_floor()) {

	    /* See what item is getting identified */
	    combo = ident_spell();

	    /* Preserve the charge (later) */
	    if (combo < 0) use_charge = FALSE;
	}

	/* For now, decharge */
	i_ptr->pval = 10;
	break;

      case SV_ROD_CURING:
	if (cure_blindness()) ident = TRUE;
	if (cure_poison()) ident = TRUE;
	if (cure_confusion()) ident = TRUE;
	if (p_ptr->stun > 0) {
	    msg_print("Your head stops stinging.");
	    if (p_ptr->stun > 50) {
		p_ptr->ptohit += 20;
		p_ptr->ptodam += 20;
	    }
	    else {
		p_ptr->ptohit += 5;
		p_ptr->ptodam += 5;
	    }
	    p_ptr->stun = 0;
	    ident = TRUE;
	}
	if (p_ptr->cut > 0) {
	    msg_print("You feel better.");
	    p_ptr->cut = 0;
	    ident = TRUE;
	}
	i_ptr->pval = 888;
	break;

      case SV_ROD_HEALING:
	ident = hp_player(500);
	if (p_ptr->stun > 0) {
	    msg_print("Your head stops stinging.");
	    if (p_ptr->stun > 50) {
		p_ptr->ptohit += 20;
		p_ptr->ptodam += 20;
	    }
	    else {
		p_ptr->ptohit += 5;
		p_ptr->ptodam += 5;
	    }
	    p_ptr->stun = 0;
	    ident = TRUE;
	}
	if (p_ptr->cut > 0) {
	    msg_print("You feel better.");
	    p_ptr->cut = 0;
	    ident = TRUE;
	}
	i_ptr->pval = 888;
	break;

      case SV_ROD_RECALL:
	if (p_ptr->word_recall == 0) {
	    msg_print("The air about you becomes charged...");
	    p_ptr->word_recall = 15 + randint(20);
	}
	else {
	    msg_print("A tension leaves the air around you...");
	    p_ptr->word_recall = 0;
	}
	ident = TRUE;
	i_ptr->pval = 60;
	break;

      case SV_ROD_PROBING:
	probing();
	ident = TRUE;
	i_ptr->pval = 50;
	break;

      case SV_ROD_DETECTION:
	detection();
	ident = TRUE;
	i_ptr->pval = 99;
	break;

      case SV_ROD_RESTORATION:
	if (restore_level()) ident = TRUE;
	if (res_stat(A_STR)) ident = TRUE;
	if (res_stat(A_INT)) ident = TRUE;
	if (res_stat(A_WIS)) ident = TRUE;
	if (res_stat(A_DEX)) ident = TRUE;
	if (res_stat(A_CON)) ident = TRUE;
	if (res_stat(A_CHR)) ident = TRUE;
	i_ptr->pval = 999;
	break;

      case SV_ROD_SPEED:
	if (p_ptr->fast == 0) ident = TRUE;
	p_ptr->fast += randint(30) + 15;
	i_ptr->pval = 99;
	break;

      case SV_ROD_TRAP_LOC:
	if (detect_trap()) ident = TRUE;
	i_ptr->pval = 99;
	break;

#if 0
      case SV_ROD_MK_WALL:	   /* JLS */
	if (!get_dir_c(NULL, &dir)) return;
	ident = build_wall(dir, y, x);
	/* don't want people to abuse this -JLS */
	i_ptr->pval = 999;
	break;
#endif

      default:
	msg_print("Oops.  Undefined rod.");
	break;
    }


    /* Tried the object */
    inven_tried(i_ptr);

    /* Successfully determined the object function */
    if (ident && !inven_aware_p(i_ptr)) {
	p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
	prt_experience();
	inven_aware(i_ptr);
    }


    /* Hack -- deal with cancelled zap */
    if (!use_charge) {
	i_ptr->pval = 0;
	return;
    }


    /* Hack -- complete the "unstacking" */
    if (i_ptr->number > 1) {
	int where;
	i_ptr->number = 1;
	where = inven_carry(&tmp_obj);
	inven_weight -= tmp_obj.weight * tmp_obj.number;
    }

    /* Combine (probably not needed) */
    item_val = combine(item_val);

    /* Need to combine? */
    if (combo >= 0) combine_pack();
}






/*
 * Activate a wielded object.  Wielded objects never stack.
 * And even if they did, activatable objects never stack.
 *
 * Currently, only (some) artifacts, and Dragon Scale Mail, can be activated.
 * But one could, for example, easily make an activatable "Ring of Plasma".
 *
 * XXX XXX Hack -- we should REALLY try to call "get_item()"
 */
void do_cmd_activate(void)
{
    int         i1, i2, i, j;
    int         a, dir, chance, what;
    char        choice;
    inven_type  *i_ptr;
    char        out_str[200], tmp[200], tmp2[200];

    /* Was an invalid choice made? */
    int		error = FALSE;

    /* Do we need to redraw? */
    int		redraw = FALSE;

    /* Do we need to verify the choice? */
    int		test = FALSE;

    /* None found yet */
    i1 = i2 = -1;

    /* Assume the turn is free */
    free_turn_flag = TRUE;

    /* Scan for something to activate -- not including aux weapon */
    for (i = INVEN_WIELD; i <= INVEN_LITE; i++) {
	i_ptr = &inventory[i];

	/* Must be activatable, and we must know it */
	if ((i_ptr->flags3 & TR3_ACTIVATE) && (known2_p(i_ptr))) {

	    /* Save the minimum index */
	    if (i1 < 0) i1 = i;

	    /* Save the maximum index */
	    i2 = i;
	}
    }

    /* Nothing found */
    if (i1 < 0) {
	msg_print("You are not wearing anything that can be activated.");
	return;
    }

    /* Use the proper item indexes */
    sprintf(out_str, "Activate which item? (%c-%c, * to list, ESC to exit) ?",
	    index_to_label(i1), index_to_label(i2));

    /* Repeat until a choice is made (or aborted) */
    for (what = -1; what < 0; ) {

	/* Assume an error */
	error = TRUE;

	/* Get an artifact, or Abort */
	if (!get_com(out_str, &choice)) break;

	/* Display the choices (with holes!) */
	if ((choice=='*') && !redraw) {

	    /* No error */
	    error = FALSE;

	    /* We need to redraw the screen */
	    redraw = TRUE;

	    /* Save the screen */
	    save_screen();

	    /* Display the available options, starting in row 1 */
	    for (j = 1, i = i1; i <= i2; i++) {

		i_ptr = &inventory[i];

		/* Can this object be activated? */
		if (known2_p(i_ptr) &&
		    (i_ptr->flags3 & TR3_ACTIVATE)) {

		    /* Describe the object */
		    objdes(tmp2, &inventory[i], TRUE);

		    /* Prepare a meaningful description.  Balance --> ( */
		    sprintf(tmp, "  %c) %-60s", index_to_label(i), tmp2);

		    /* Draw at column 13, with a two space "border" */
		    prt(tmp, j++, 13);
		}
	    }

	    /* Try again */
	    continue;
	}

#ifdef TAGGER
	/* Process tags ('0' to '9') here */
	if ((choice >= '0') && (choice <= '9')) {

	    /* XXX Look up the item tag in the "equip" */
	    if (get_tag(&i, choice) && (i >= INVEN_WIELD)) {
		/* Extract a label from the index */
		choice = index_to_label(i);
	    }
	}
#endif

	/* Some choices must be verified */
	if (isupper(choice)) {
	    test = TRUE;
	    choice = tolower(choice);
	}

	/* Extract an item_val from that choice */
	i = label_to_equip(choice);

	/* Possibly valid choice */
	if ((i >= i1) && (i <= i2)) {

	    /* Get the item */
	    i_ptr = &inventory[i];

	    /* Legal item, ask for details */
	    if ((i_ptr->flags3 & TR3_ACTIVATE) && known2_p(i_ptr)) {

		/* No error */
		error = FALSE;

		/* Verify if desired, save the index */
		if (!test || !verify("Activate", i)) what = i;
	    }
	}

	/* Cheezy warning */
	if (error) bell();
    }

    /* Restore the screen if necessary */
    if (redraw) restore_screen();

    /* Nothing requested */
    if (what < 0) return;


    /* Get the item */
    i_ptr = &inventory[what];

    /* Check the recharge */
    if (i_ptr->timeout > 0) {
	msg_print("It whines, glows and fades...");
	return;
    }

    /* Are we smart enough? */
    if (p_ptr->use_stat[A_INT] < randint(18) &&
	randint(k_list[i_ptr->k_idx].level) > p_ptr->lev) {
	msg_print("You fail to activate it properly.");
	return;
    }


    /* Take a turn */
    free_turn_flag = FALSE;

    /* Wonder Twin Powers... Activate! */
    msg_print("You activate it...");


    /* All artifacts have a "name" field */
    if (i_ptr->name1) {

	/* This needs to be changed */
	switch (i_ptr->name1) {

	    case ART_NARTHANC:
		msg_print("Your dagger is covered in fire...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_FIRE, dir, char_row, char_col, damroll(9, 8));
		    i_ptr->timeout = 5 + randint(10);
		}
		break;

	    case ART_NIMTHANC:
		msg_print("Your dagger is covered in frost...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_COLD, dir, char_row, char_col, damroll(6, 8));
		    i_ptr->timeout = 4 + randint(8);
		}
		break;

	    case ART_DETHANC:
		msg_print("Your dagger is covered in sparks...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_ELEC, dir, char_row, char_col, damroll(4, 8));
		    i_ptr->timeout = 3 + randint(7);
		}
		break;

	    case ART_RILIA:
		msg_print("Your dagger throbs deep green...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_POIS, dir, char_row, char_col, 12, 3);
		    i_ptr->timeout = 3 + randint(3);
		}
		break;

	    case ART_BELANGIL:
		msg_print("Your dagger is covered in frost...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_COLD, dir, char_row, char_col, 48, 2);
		    i_ptr->timeout = 3 + randint(7);
		}
		break;

	    case ART_DAL:
		msg_print("You feel energy flow through your feet...");
		remove_fear();
		cure_poison();
		i_ptr->timeout = 5;
		break;

	    case ART_RINGIL:
		msg_print("Your sword glows an intense blue...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_COLD, dir, char_row, char_col, 100, 2);
		    i_ptr->timeout = 300;
		}
		break;

	    case ART_ANDURIL:
		msg_print("Your sword glows an intense red...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_FIRE, dir, char_row, char_col, 72, 2);
		    i_ptr->timeout = 400;
		}
		break;

	    case ART_FIRESTAR:
		msg_print("Your morningstar rages in fire...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_FIRE, dir, char_row, char_col, 72, 3);
		    i_ptr->timeout = 100;
		}
		break;

	    case ART_FEANOR:
		p_ptr->fast += randint(25) + 15;
		i_ptr->timeout = 200;
		break;

	    case ART_THEODEN:
		msg_print("The blade of your axe glows black...");
		if (get_dir_c(NULL, &dir)) {
		    drain_life(dir, char_row, char_col, 120);
		    i_ptr->timeout = 400;
		}
		break;

	    case ART_TURMIL:
		msg_print("The head of your hammer glows white...");
		if (get_dir_c(NULL, &dir)) {
		    drain_life(dir, char_row, char_col, 90);
		    i_ptr->timeout = 70;
		}
		break;

	    case ART_CASPANION:
		msg_print("Your mail magically disarms traps...");
		td_destroy();
		i_ptr->timeout = 10;
		break;

	    case ART_AVAVIR:
		if (p_ptr->word_recall == 0) {
		    p_ptr->word_recall = 15 + randint(20);
		    msg_print("The air about you becomes charged...");
		}
		else {
		    p_ptr->word_recall = 0;
		    msg_print("A tension leaves the air around you...");
		}
		i_ptr->timeout = 200;
		break;

	    case ART_TARATOL:
		if (p_ptr->fast == 0) {
		    p_ptr->fast += randint(30) + 15;
		}
		i_ptr->timeout = 166;
		break;

	    case ART_ERIRIL:
		/* Check floor, or inventory plus combining */
		/* XXX Note that the artifact is always de-charged */
		if (!ident_floor()) combine(ident_spell());
		i_ptr->timeout = 10;
		break;

	    case ART_OLORIN:
		probing();
		i_ptr->timeout = 20;
		break;

	    case ART_EONWE:
		msg_print("Your axe lets out a long, shrill note...");
		mass_genocide(TRUE);
		i_ptr->timeout = 1000;
		break;

	    case ART_LOTHARANG:
		msg_print("Your battle axe radiates deep purple...");
		hp_player(damroll(4, 7));
		if (p_ptr->cut > 0) {
		    p_ptr->cut = (p_ptr->cut / 2) - 50;
		    if (p_ptr->cut < 0) p_ptr->cut = 0;
		    msg_print("Your wounds heal.");
		}
		i_ptr->timeout = 2 + randint(2);
		break;

	    case ART_CUBRAGOL:
		/* Look for the XXX first non-cursed, non-blessed bolts */
		for (a = 0; a < inven_ctr; a++) {

		    i_ptr = &inventory[a];

		    /* Use the first (XXX) acceptable bolts */
		    if ((i_ptr->tval == TV_BOLT) &&
			(!artifact_p(i_ptr)) &&
			(!i_ptr->name1) &&
			(!i_ptr->name2) &&
			(!cursed_p(i_ptr))) break;
		}
		if (a < inven_ctr) {
		    i_ptr = &inventory[a];
		    msg_print("Your bolts are covered in a fiery aura!");
		    i_ptr->name2 = EGO_FIRE;
		    i_ptr->flags1 |= (TR1_BRAND_FIRE);
		    i_ptr->flags2 |= (TR2_RES_FIRE);
		    i_ptr->cost += 25;
		    enchant(i_ptr, 3+randint(3), ENCH_TOHIT | ENCH_TODAM);
		    calc_bonuses();
		}
		else {
		    msg_print("The fiery enchantment fails.");
		}
		i_ptr->timeout = 999;
		break;

	    case ART_ARUNRUTH:
		msg_print("Your sword glows a pale blue...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_COLD,
			      dir, char_row, char_col, damroll(12, 8));
		    i_ptr->timeout = 500;
		}
		break;

	    case ART_AEGLOS:
		msg_print("Your spear glows a bright white...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_COLD, dir, char_row, char_col, 100, 2);
		    i_ptr->timeout = 500;
		}
		break;

	    case ART_OROME:
		msg_print("Your spear pulsates...");
		if (get_dir_c(NULL, &dir)) {
		    wall_to_mud(dir, char_row, char_col);
		    i_ptr->timeout = 5;
		}
		break;

	    case ART_SOULKEEPER:
		msg_print("Your armour glows a bright white...");
		msg_print("You feel much better...");
		hp_player(1000);
		if (p_ptr->cut > 0) {
		    p_ptr->cut = 0;
		    msg_print("Your wounds heal.");
		}
		i_ptr->timeout = 888;
		break;

	    case ART_BELEGENNON:
		teleport(10);
		i_ptr->timeout = 2;
		break;

	    case ART_CELEBORN:
		genocide(TRUE);
		i_ptr->timeout = 500;
		break;

	    case ART_LUTHIEN:
		restore_level();
		i_ptr->timeout = 450;
		break;

	    case ART_ULMO:
		msg_print("Your trident glows deep red...");
		if (get_dir_c(NULL, &dir)) {
		    teleport_monster(dir, char_row, char_col);
		    i_ptr->timeout = 150;
		}
		break;

	    case ART_COLLUIN:
		msg_print("Your cloak glows many colours...");
		msg_print("You feel you can resist anything.");
		p_ptr->oppose_fire += randint(20) + 20;
		p_ptr->oppose_cold += randint(20) + 20;
		p_ptr->oppose_elec += randint(20) + 20;
		p_ptr->oppose_pois += randint(20) + 20;
		p_ptr->oppose_acid += randint(20) + 20;
		i_ptr->timeout = 111;
		break;

	    case ART_HOLCOLLETH:
		msg_print("You momentarily disappear...");
		sleep_monsters1(char_row, char_col);
		i_ptr->timeout = 55;

	    case ART_THINGOL:
		msg_print("You hear a low humming noise...");
		recharge(60);
		i_ptr->timeout = 70;

	    case ART_COLANNON:
		teleport(100);
		i_ptr->timeout = 45;
		break;

	    case ART_TOTILA:
		msg_print("Your flail glows in scintillating colours...");
		if (get_dir_c(NULL, &dir)) {
		    confuse_monster(dir, char_row, char_col, 20);
		    i_ptr->timeout = 15;
		}
		break;

	    case ART_CAMMITHRIM:
		msg_print("Your gloves glow extremely brightly...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_MISSILE, dir, char_row, char_col,
			      damroll(2, 6));
		    i_ptr->timeout = 2;
		}
		break;

	    case ART_PAURHACH:
		msg_print("Your gauntlets are covered in fire...");
		if (get_dir_c(NULL, &dir)) {
		    if (randint(4)==1) {
			line_spell(GF_FIRE,
				   dir, char_row, char_col, damroll(9,8));
		    }
		    else {
			fire_bolt(GF_FIRE,
				  dir, char_row, char_col, damroll(9,8));
		    }
		    i_ptr->timeout = 5 + randint(10);
		}
		break;

	    case ART_PAURNIMMEN:
		msg_print("Your gauntlets are covered in frost...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_COLD,
			      dir, char_row, char_col, damroll(6, 8));
		    i_ptr->timeout = 4 + randint(8);
		}
		break;

	    case ART_PAURAEGEN:
		msg_print("Your gauntlets are covered in sparks...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_ELEC, dir, char_row, char_col, damroll(4, 8));
		    i_ptr->timeout = 3 + randint(7);
		}
		break;

	    case ART_PAURNEN:
		msg_print("Your gauntlets look very acidic...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_ACID, dir, char_row, char_col, damroll(5, 8));
		    i_ptr->timeout = 4 + randint(7);
		}
		break;

	    case ART_FINGOLFIN:
		msg_print("Magical spikes appear on your cesti...");
		if (get_dir_c(NULL, &dir)) {
		    fire_bolt(GF_ARROW, dir, char_row, char_col, 150);
		    i_ptr->timeout = 88 + randint(88);
		}
		break;

	    case ART_HOLHENNETH:
		message("You close your eyes ", 0x02);
		message("and an image forms in your mind...", 0);
		detection();
		i_ptr->timeout = 55 + randint(55);
		break;

	    case ART_GONDOR:
		msg_print("You feel a warm tingling inside...");
		hp_player(500);
		if (p_ptr->cut > 0) {
		    p_ptr->cut = 0;
		    msg_print("Your wounds heal.");
		}
		i_ptr->timeout = 500;
		break;

	    case ART_RAZORBACK:
		message("A storm of lightning spikes ", 0x02);
		message("fires in all directions...", 0);
		starball(char_row, char_col);
		i_ptr->timeout = 1000;
		break;

	    case ART_BLADETURNER:
		msg_print("Your armour glows many colours...");
		msg_print("You enter a berserk rage...");
		msg_print("You feel you can resist anything...");
		p_ptr->hero += randint(50) + 50;
		p_ptr->shero += randint(50) + 50;
		bless(randint(50) + 50);
		p_ptr->oppose_fire += randint(50) + 50;
		p_ptr->oppose_cold += randint(50) + 50;
		p_ptr->oppose_elec += randint(50) + 50;
		p_ptr->oppose_acid += randint(50) + 50;
		p_ptr->resist_pois += randint(50) + 50;
		i_ptr->timeout = 400;
		break;


	    case ART_GALADRIEL:
		msg_print("The phial wells with clear light...");
		lite_area(char_row, char_col, damroll(2, 15), 3);
		i_ptr->timeout = 10 + randint(10);
		break;

	    case ART_ELENDIL:
		msg_print("The star shines brightly...");
		msg_print("And you sense your surroundings...");
		map_area();
		i_ptr->timeout = 50 + randint(50);
		break;

	    case ART_THRAIN:
		msg_print("The stone glows a deep green");
		wiz_lite();
		(void)detect_sdoor();
		(void)detect_trap();
		i_ptr->timeout = 100 + randint(100);
		break;


	    case ART_INGWE:
		msg_print("An aura of good floods the area...");
		dispel_creature(MF2_EVIL, (int)(5 * p_ptr->lev));
		i_ptr->timeout = 444 + randint(222);
		break;

	    case ART_CARLAMMAS:
		msg_print("The amulet lets out a shrill wail...");
		msg_print("You feel somewhat safer...");
		protect_evil();
		i_ptr->timeout = 222 + randint(222);
		break;


	    case ART_TULKAS:
		msg_print("The ring glows brightly...");
		p_ptr->fast += randint(100) + 50;
		i_ptr->timeout = 200;
		break;

	    case ART_NARYA:
		msg_print("The ring glows deep red...");
		if (!get_dir_c(NULL, &dir)) break;
		fire_ball(GF_FIRE, dir, char_row, char_col, 120, 3);
		i_ptr->timeout = 222 + randint(222);
		break;

	    case ART_NENYA:
		msg_print("The ring glows bright white...");
		if (!get_dir_c(NULL, &dir)) break;
		fire_ball(GF_COLD, dir, char_row, char_col, 200, 3);
		i_ptr->timeout = 222 + randint(333);
		break;

	    case ART_VILYA:
		msg_print("The ring glows deep blue...");
		if (!get_dir_c(NULL, &dir)) break;
		fire_ball(GF_ELEC, dir, char_row, char_col, 250, 3);
		i_ptr->timeout = 222 + randint(444);
		break;

	    case ART_POWER:
		msg_print("The ring glows intensely black...");
		if (!get_dir_c(NULL, &dir)) break;

		switch (randint(17) + (8 - p_ptr->lev / 10)) {
		  case 5:
		    dispel_creature(0xFFFFFFFL, 1000);
		    break;
		  case 6:
		  case 7:
		    msg_print("You are surrounded by a malignant aura");
		    p_ptr->lev--;
		    /* XXX Convert to "rand_range()" */
		    p_ptr->exp = (player_exp[p_ptr->lev - 2] *
				  p_ptr->expfact / 100) +
				 randint((player_exp[p_ptr->lev - 1] *
					  p_ptr->expfact / 100) -
					 (player_exp[p_ptr->lev - 2] *
					  p_ptr->expfact / 100));
		    p_ptr->max_exp = p_ptr->exp;
		    prt_experience();
		    dec_stat(A_STR, 50, TRUE);
		    dec_stat(A_INT, 50, TRUE);
		    dec_stat(A_WIS, 50, TRUE);
		    dec_stat(A_DEX, 50, TRUE);
		    dec_stat(A_CON, 50, TRUE);
		    dec_stat(A_CHR, 50, TRUE);
		    calc_hitpoints();
		    if (class[p_ptr->pclass].spell == MAGE) {
			calc_spells(A_INT);
			calc_mana(A_INT);
		    }
		    else if (class[p_ptr->pclass].spell == PRIEST) {
			calc_spells(A_WIS);
			calc_mana(A_WIS);
		    }
		    prt_level();
		    prt_title();
		    take_hit((p_ptr->chp > 2) ? p_ptr->chp / 2 : 0,
			     "malignant aura");
		    break;
		  case 8:
		  case 9:
		  case 10:
		    fire_ball(GF_MANA, dir, char_row, char_col, 300, 3);
		    break;
		  default:
		    fire_bolt(GF_MANA, dir, char_row, char_col, 250);
		}
		i_ptr->timeout = 444 + randint(444);
		break;



	    default:
		message("Oops.  Non-Activatable Special Name", 0);
	}
    }


    /* Hack -- Dragon Scale Mail can be activated as well */
    else if (i_ptr->tval == TV_DRAG_ARMOR) {

	/* Branch on the sub-type */
	switch (i_ptr->sval) {

	    case SV_DRAGON_BLUE:
		msg_print("You breathe lightning...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_ELEC, dir, char_row, char_col, 100, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_WHITE:
		msg_print("You breathe frost...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_COLD, dir, char_row, char_col, 110, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_BLACK:
		msg_print("You breathe acid...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_ACID, dir, char_row, char_col, 130, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_GREEN:
		msg_print("You breathe poison gas...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_POIS, dir, char_row, char_col, 150, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_RED:
		msg_print("You breathe fire...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_FIRE, dir, char_row, char_col, 200, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_MULTIHUED:
		msg_print("You breathe...");
		if (get_dir_c(NULL, &dir)) {
		    chance = randint(5);
		    sprintf(tmp2, "You breathe %s...",
			    ((chance == 1) ? "lightning" :
			     ((chance == 2) ? "frost" :
			      ((chance == 3) ? "acid" :
			       ((chance == 4) ? "poison gas" : "fire")))));
		    msg_print(tmp2);
		    fire_ball(((chance == 1) ? GF_ELEC :
			       ((chance == 2) ? GF_COLD :
				((chance == 3) ? GF_ACID :
				 ((chance == 4) ? GF_POIS : GF_FIRE)))),
			      dir, char_row, char_col, 250, 2);
		    i_ptr->timeout = 222 + randint(222);
		}
		break;

	    case SV_DRAGON_BRONZE:
		msg_print("You breathe confusion...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_CONFUSION, dir, char_row, char_col, 120, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_GOLD:
		msg_print("You breathe sound...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_SOUND, dir, char_row, char_col, 130, 2);
		    i_ptr->timeout = 444 + randint(444);
		}
		break;

	    case SV_DRAGON_CHAOS:
		msg_print("You breathe...");
		if (get_dir_c(NULL, &dir)) {
		    chance = randint(2);
		    sprintf(tmp2, "You breathe %s...",
			    ((chance == 1 ? "chaos" : "disenchantment")));
		    msg_print(tmp2);
		    fire_ball((chance == 1 ? GF_CHAOS : GF_DISENCHANT), dir,
			      char_row, char_col, 220, 2);
		    i_ptr->timeout = 300 + randint(300);
		}
		break;

	    case SV_DRAGON_LAW:
		msg_print("You breathe...");
		if (get_dir_c(NULL, &dir)) {
		    chance = randint(2);
		    sprintf(tmp2, "You breathe %s...",
			    ((chance == 1 ? "sound" : "shards")));
		    msg_print(tmp2);
		    fire_ball((chance == 1 ? GF_SOUND : GF_SHARDS), dir,
			      char_row, char_col, 230, 2);
		    i_ptr->timeout = 300 + randint(300);
		}
		break;

	    case SV_DRAGON_BALANCE:
		msg_print("You breathe...");
		if (get_dir_c(NULL, &dir)) {
		    chance = randint(4);
		    sprintf(tmp2, "You breathe %s...",
			    ((chance == 1) ? "chaos" :
			     ((chance == 2) ? "disenchantment" :
			      ((chance == 3) ? "sound" : "shards"))));
		    msg_print(tmp2);
		    fire_ball(((chance == 1) ? GF_CHAOS :
			       ((chance == 2) ? GF_DISENCHANT :
				((chance == 3) ? GF_SOUND : GF_SHARDS))),
			      dir, char_row, char_col, 250, 2);
		    i_ptr->timeout = 300 + randint(300);
		}
		break;

	    case SV_DRAGON_SHINING:
		msg_print("You breathe...");
		if (get_dir_c(NULL, &dir)) {
		    chance = randint(2);
		    sprintf(tmp2, "You breathe %s...",
			    ((chance == 1 ? "light" : "darkness")));
		    msg_print(tmp2);
		    fire_ball((chance == 1 ? GF_LITE : GF_DARK), dir,
			      char_row, char_col, 200, 2);
		    i_ptr->timeout = 300 + randint(300);
		}
		break;

	    case SV_DRAGON_POWER:
		msg_print("You breathe the elements...");
		if (get_dir_c(NULL, &dir)) {
		    fire_ball(GF_MISSILE,
			      dir, char_row, char_col, 300, 2);
		    i_ptr->timeout = 300 + randint(300);
		}
		break;

	    default:
		msg_print("Oops.  That armour cannot be activated.");
	}
    }

    /* Mistake */
    else {
	msg_print("Oops.  That object cannot be activated.");
    }
}


