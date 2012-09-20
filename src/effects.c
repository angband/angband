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
 * This code now correctly handles the unstacking of wands, staffs,
 * and rods.  Note the overly paranoid warning about potential pack
 * overflow, which allows the player to use and drop a stacked item.
 *
 * In all "unstacking" scenarios, the "used" object is "carried" as if
 * the player had just picked it up.  In particular, this means that if
 * the use of an item induces pack overflow, that item will be dropped.
 *
 * For simplicity, these routines induce a full "pack reorganization"
 * which not only combines similar items, but also reorganizes various
 * items to obey the current "sorting" method.  This may require about
 * 400 item comparisons, but only occasionally.
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "i_ptr" to
 * no longer point at the correct item, with horrifying results.
 *
 * XXX XXX XXX Someone needs to verify all of these effects.
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
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
static void identify_pack(void)
{
    int                 i;
    inven_type         *i_ptr;

    /* Simply identify and know every item */
    for (i = 0; i < INVEN_TOTAL; i++) {
        i_ptr = &inventory[i];
        if (i_ptr->tval) {
            inven_aware(i_ptr);
            inven_known(i_ptr);
        }
    }
}


/*
 * Add to the players food time				-RAK-	
 */
void add_food(int num)
{
    int           extra, penalty;

    if (p_ptr->food < 0) p_ptr->food = 0;
    p_ptr->food += num;

    /* overflow check */
    if ((num > 0) && (p_ptr->food <= 0)) p_ptr->food = 32000;

    if (p_ptr->food > PLAYER_FOOD_MAX) {

        msg_print("You are bloated from overeating. ");

    /*
     * Calculate how much of num is responsible for the bloating. Give the
     * player food credit for 1/50, and slow him for that many turns also.
     */
        extra = p_ptr->food - PLAYER_FOOD_MAX;
        if (extra > num) extra = num;
        penalty = extra / 50;

        p_ptr->slow += penalty;
        if (extra == num) p_ptr->food = p_ptr->food - num + penalty;
        else p_ptr->food = PLAYER_FOOD_MAX + penalty;
    }

    else if (p_ptr->food > PLAYER_FOOD_FULL) {
        msg_print("You are full. ");
    }
}




/*
 * Can the player "eat" a given item?
 */
static bool item_tester_hook_eat(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_FOOD);
}


/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(void)
{
    u32b		flg;
    int			oy, ox, item, j, ident, lev;
    bool		floor;
    inven_type		*i_ptr;


    /* Assume the turn will be free */
    energy_use = 0;


    /* Save the player location */
    oy = py, ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* See if we can eat the item on the floor */
    floor = item_tester_hook_eat(i_ptr);

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_eat;

    /* Get a potion */
    if (!get_item(&item, "Eat which item? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have nothing to eat.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* Get the item level */
    lev = k_list[i_ptr->k_idx].level;

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
                p_ptr->poisoned += rand_int(10) + lev + 1;
                ident = TRUE;
            }
            break;

          case 2:
            if (!p_ptr->resist_blind) {
                p_ptr->blind += rand_int(250) + 10 * lev + 100;
                msg_print("A veil of darkness surrounds you.");
                ident = TRUE;
            }
            break;

          case 3:
            if (!p_ptr->resist_fear) {
                p_ptr->afraid += rand_int(10) + lev + 1;
                msg_print("You feel terrified!");
                ident = TRUE;
            }
            break;

          case 4:
            if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
                p_ptr->confused += rand_int(10) + lev + 1;
                msg_print("You feel drugged.");
                ident = TRUE;
            }
            break;

          case 5:
            p_ptr->image += rand_int(200) + 25 * lev + 200;
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

    /* The turn is not free */
    energy_use = 100;

    /* We have tried it */
    inven_tried(i_ptr);

    /* The player is now aware of the object */
    if (ident && !inven_aware_p(i_ptr)) {
        p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
        check_experience();
        inven_aware(i_ptr);
    }

    /* Consume the food */
    add_food(i_ptr->pval);


    /* Destroy a food on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_optimize(oy, ox);
    }

    /* Destroy a food in the pack */
    else {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Hack -- combine the pack */
    combine_pack();
}




/*
 * Can the player "quaff" a given item?
 */
static bool item_tester_hook_quaff(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_POTION);
}


/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(void)
{
    u32b	flg;
    int		oy, ox, item, j, ident;
    bool	floor;
    inven_type	*i_ptr;


    /* Assume the turn will be free */
    energy_use = 0;


    /* Save the old player location */
    oy = py, ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* See if we can quaff the item on the floor */
    floor = item_tester_hook_quaff(i_ptr);

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_quaff;

    /* Get a potion */
    if (!get_item(&item, "Quaff which potion? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have no potions to quaff.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* Not identified yet */
    ident = FALSE;

    /* Note potions with no effects */
    if ((i_ptr->flags1 == 0) && (i_ptr->flags2 == 0)) {
        msg_print("You feel less thirsty.");
        ident = TRUE;
    }

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
            if (p_ptr->cut) {
                msg_print("Your wounds heal.");
                p_ptr->cut -= 10;
                if (p_ptr->cut < 0) p_ptr->cut = 0;
                ident = TRUE;
            }
            break;

          case 14:
            if (hp_player(damroll(4, 7))) ident = TRUE;
            if (p_ptr->cut) {
                msg_print("Your wounds heal.");
                p_ptr->cut = (p_ptr->cut / 2) - 50;
                if (p_ptr->cut < 0) p_ptr->cut = 0;
                ident = TRUE;
            }
            break;

          case 15:
            if (hp_player(damroll(6, 7))) ident = TRUE;
            if (p_ptr->cut) {
                msg_print("Your wounds heal.");
                p_ptr->cut = 0;
                ident = TRUE;
            }
            if (p_ptr->stun) {
                msg_print("Your head stops stinging.");
                p_ptr->stun = 0;
                p_ptr->update |= (PU_BONUS);
                ident = TRUE;
            }
            break;

          case 16:
            if (hp_player(400)) ident = TRUE;
            if (p_ptr->stun) {
                msg_print("Your head stops stinging.");
                p_ptr->stun = 0;
                p_ptr->update |= (PU_BONUS | PU_HP);
                ident = TRUE;
            }
            if (p_ptr->cut) {
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
                u32b ee;
                ee = (p_ptr->exp / 2) + 10;
                if (ee > 100000L) ee = 100000L;
                p_ptr->exp += ee;
                msg_print("You feel more experienced.");
                check_experience();
                ident = TRUE;
            }
            break;

          case 19:
            if (!p_ptr->free_act) {
                if (p_ptr->paralysis == 0) {
                    msg_print("You fall asleep.");
                    ident = TRUE;
                }
                p_ptr->paralysis += rand_int(4) + 4;
            }
            break;

          case 20:
            if (!p_ptr->resist_blind) {
                if (p_ptr->blind == 0) {
                    msg_print("You are covered by a veil of darkness.");
                    ident = TRUE;
                }
                p_ptr->blind += rand_int(100) + 100;
            }
            break;

          case 21:
            if (!p_ptr->resist_conf) {
                if (p_ptr->confused == 0) {
                    msg_print("Hey!  This is good stuff!  *Hick!*");
                    ident = TRUE;
                }
                p_ptr->confused += rand_int(20) + 15;
            }
            break;

          case 22:
            if (!(p_ptr->immune_pois ||
                  p_ptr->resist_pois ||
                  p_ptr->oppose_pois)) {
                msg_print("You feel very sick.");
                p_ptr->poisoned += rand_int(15) + 10;
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
            if (!p_ptr->hold_life && (p_ptr->exp > 0)) {
                s32b               m, scale;

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
            if (hp_player(10)) ident = TRUE;	/* XXX */
            if (p_ptr->hero == 0) ident = TRUE;
            p_ptr->hero += randint(25) + 25;
            break;

          case 38:
            if (hp_player(30)) ident = TRUE;	/* XXX */
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
                p_ptr->redraw |= PR_MANA;
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
            p_ptr->stun = 0;
            message("You feel life flow through your body! ", 0);
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
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
            if (p_ptr->stun) {
                p_ptr->stun = 0;
                msg_print("Your head stops stinging.");
                p_ptr->update |= (PU_BONUS);
                ident = TRUE;
            }
            if (p_ptr->cut) {
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


    /* The turn is not free */
    energy_use = 100;

    /* The item has been tried */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        int lev = k_list[i_ptr->k_idx].level;
        p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
        check_experience();
        inven_aware(i_ptr);
    }

    /* Potions can feed the player */
    add_food(i_ptr->pval);


    /* Destroy a potion on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_optimize(oy, ox);
    }

    /* Destroy a potion in the pack */
    else {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Hack -- combine the pack */
    combine_pack();
}



/*
 * Can the player "read" a given item?
 */
static bool item_tester_hook_read(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_SCROLL);
}


/*
 * Read a scroll (from the pack or floor).
 *
 * Certain scrolls can be "aborted" without losing the scroll.  These
 * include scrolls with no effects but recharge or identify, which are
 * cancelled before use.  XXX Reading them still takes a turn, though.
 */
void do_cmd_read_scroll(void)
{
    u32b                flg;
    int			oy, ox, item, j, k;
    int			used_up, ident, l;
    int			tmp[6];
    bool		floor;

    inven_type		*i_ptr, *j_ptr;

    char		tmp_str[160];


    /* Assume the turn will be free */
    energy_use = 0;

    /* Check some conditions */
    if (p_ptr->blind) {
        msg_print("You can't see anything.");
        return;
    }
    if (no_lite()) {
        msg_print("You have no light to read by.");
        return;
    }
    if (p_ptr->confused) {
        msg_print("You are too confused to read anything.");
        return;
    }


    /* Save the old player location */
    oy = py, ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* See if we can read the item on the floor */
    floor = item_tester_hook_read(i_ptr);

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_read;

    /* Get a potion */
    if (!get_item(&item, "Read which scroll? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have no scrolls to read.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


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
            msg_print("This is a scroll of enchant weapon.");
            if (!enchant_spell(1, 0, 0)) used_up = FALSE;
            ident = TRUE;
            break;

          case 2:
            msg_print("This is a scroll of enchant weapon.");
            if (!enchant_spell(0, 1, 0)) used_up = FALSE;
            ident = TRUE;
            break;

          case 3:
            msg_print("This is a scroll of enchant armour.");
            ident = TRUE;
            if (!enchant_spell(0, 0, 1)) used_up = FALSE;
            break;

          case 4:
            msg_print("This is an identify scroll.");
            ident = TRUE;
            if (!ident_spell()) used_up = FALSE;
            break;

          case 5:
            if (remove_curse()) {
                msg_print("You feel as if someone is watching over you.");
                ident = TRUE;
            }
            break;

          case 6:
            ident = lite_area(py, px, damroll(2, 12), 2) || ident;
            break;

          case 7:
            for (k = 0; k < randint(3); k++) {
                if (summon_monster(py, px, dun_level + MON_SUMMON_ADJ)) {
                    ident = TRUE;
                }
            }
            break;

          case 8:
            teleport_flag = TRUE;
            teleport_dist = 10;
            ident = TRUE;
            break;

          case 9:
            teleport_flag = TRUE;
            teleport_dist = 100;
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
            if (sleep_monsters1(py, px)) ident = TRUE;
            break;

          case 14:
            warding_glyph();
            ident = TRUE;
            break;

          case 15:
            if (detect_treasure()) ident = TRUE;
            break;

          case 16:
            if (detect_object()) ident = TRUE;
            break;

          case 17:
            if (detect_trap()) ident = TRUE;
            break;

          case 18:
            if (detect_sdoor()) ident = TRUE;
            break;

          case 19:
            msg_print("This is a mass genocide scroll.");
            mass_genocide(TRUE);
            ident = TRUE;
            break;

          case 20:
            if (detect_invisible()) ident = TRUE;
            break;

          case 21:
            msg_print("There is a high pitched humming noise.");
            aggravate_monsters();
            ident = TRUE;
            break;

          case 22:
            if (trap_creation()) ident = TRUE;
            break;

          case 23:
            if (td_destroy()) ident = TRUE;
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
            (void)genocide(TRUE);
            ident = TRUE;
            break;

          case 27:
            if (unlite_area(py, px)) ident = TRUE;
            if (!p_ptr->resist_blind) {
                p_ptr->blind += 3 + randint(5);
            }
            break;

          case 28:
            if (protect_evil()) ident = TRUE;
            break;

          case 29:
            satisfy_hunger();
            ident = TRUE;
            break;

          case 30:
            /* Dispel (undead) monsters */
            if (dispel_monsters(60, FALSE, TRUE)) ident = TRUE;
            break;

          case 31:
            remove_all_curse();
            ident = TRUE;
            break;

          case 32:
            msg_print("This is an *identify* scroll.");
            ident = TRUE;

            /* Identify something fully or preserve the scroll */
            if (!identify_fully()) used_up = FALSE;

            break;
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
            msg_print("This is a scroll of *enchant* weapon.");
            if (!enchant_spell(randint(3), randint(3), 0)) used_up = FALSE;
            ident = TRUE;
            break;

          case 34:

            /* Pick a weapon (or bow) */
            j_ptr = &inventory[INVEN_WIELD];
            if (!j_ptr->tval) j_ptr = &inventory[INVEN_BOW];

            if (j_ptr->tval) {

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

                    /* Shatter the weapon */
                    j_ptr->name1 = 0;
                    j_ptr->name2 = EGO_SHATTERED;
                    j_ptr->tohit = 0 - randint(5) - randint(5);
                    j_ptr->todam = 0 - randint(5) - randint(5);
                    j_ptr->flags3 = TR3_CURSED;
                    j_ptr->flags2 = 0L;
                    j_ptr->flags1 = 0L;
                    j_ptr->dd = j_ptr->ds = 1;
                    j_ptr->toac = 0;
                    j_ptr->cost = 0L;

                    /* Recalculate bonuses */
                    p_ptr->update |= (PU_BONUS);
                }

                ident = TRUE;
            }
            break;

          case 35:
            msg_print("This is a scroll of *enchant* armour.");
            if (!enchant_spell(0, 0, randint(3) + 2)) used_up = FALSE;
            ident = TRUE;
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
            if (artifact_p(j_ptr) && (rand_int(7) < 3)) {
                message("A terrible black aura tries to surround your", 0x02);
                message(tmp_str, 0x02);
                message(", but it resists the effects!", 0);
            }

            /* not artifact or failed save... */
            else {

                /* Oops */
                message(format("A terrible black aura blasts your %s!",
                               tmp_str), 0);

                /* Blast the armor */
                j_ptr->name1 = 0;
                j_ptr->name2 = EGO_BLASTED;
                j_ptr->flags3 = TR3_CURSED;
                j_ptr->flags2 = 0L;
                j_ptr->flags1 = 0L;
                j_ptr->toac = 0 - randint(5) - randint(5);
                j_ptr->tohit = j_ptr->todam = 0;
                j_ptr->ac = (j_ptr->ac > 9) ? 1 : 0;
                j_ptr->cost = 0L;

                /* Recalculate bonuses */
                p_ptr->update |= (PU_BONUS);

                /* Well, you know all about it */
                ident = TRUE;
            }
            break;

          case 37:
            for (k = 0; k < randint(3); k++) {
                if (summon_specific(py, px, dun_level, SUMMON_UNDEAD)) {
                    ident = TRUE;
                }
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
                p_ptr->word_recall = randint(20) + 15;
                msg_print("The air about you becomes charged...");
            }
            else {
                p_ptr->word_recall = 0;
                msg_print("A tension leaves the air around you...");
            }
            ident = TRUE;
            break;

          case 42:
            destroy_area(py, px);
            ident = TRUE;
            break;

          case 43:
            acquirement(py, px, 1, TRUE);
            ident = TRUE;
            break;

          case 44:
            acquirement(py, px, randint(2) + 1, TRUE);
            ident = TRUE;
            break;
        }
    }


    /* The turn is not free */
    energy_use = 100;

    /* The item was tried */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        int lev = k_list[i_ptr->k_idx].level;
        /* round half-way case up */
        p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
        check_experience();
        inven_aware(i_ptr);
    }


    /* Hack -- allow certain scrolls to be "preserved" */
    if (!used_up) return;


    /* Destroy a scroll on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_optimize(oy, ox);
    }

    /* Destroy a scroll in the pack */
    else {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Hack -- combine the pack */
    combine_pack();
}



/*
 * Hack -- fire a bolt, or a beam if lucky
 * There is another copy of this in "magic.c"
 */
static void bolt_or_beam(int prob, int typ, int dir, int dam)
{
    if (randint(100) < prob) {
        line_spell(typ, dir, py, px, dam);
    }
    else {
        fire_bolt(typ, dir, py, px, dam);
    }
}



/*
 * Hack -- do "inven_item_charges()" on a floor item.
 */
static void floor_item_charges(int y, int x)
{
    int		rem_num;

    char	out_val[80];

    inven_type *i_ptr = &i_list[cave[y][x].i_idx];

    /* Only if known */
    if (inven_known_p(i_ptr)) {
        rem_num = i_ptr->pval;
        sprintf(out_val, "There are %d charges remaining.", rem_num);
        msg_print(out_val);
    }
}


/*
 * Is the given item a wand?
 */
static bool item_tester_hook_wand(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_WAND);
}


/*
 * Aim a wand (from the pack or floor).
 *
 * Use a single charge from a single item.
 * Handle "unstacking" in a logical manner.
 *
 * For simplicity, you cannot use a stack of items from the
 * ground.  This would require too much nasty code.
 *
 * All wands can be "cancelled" at the "Direction?" prompt.
 */
void do_cmd_aim_wand(void)
{
    int			ident, chance, dir, sval;
    int			ox, oy, item, lev;
    bool		floor;
    inven_type		*i_ptr;


    /* Assume free turn */
    energy_use = 0;

    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_wand;

    /* See if we can use the item on the floor */
    floor = item_tester_hook(i_ptr);

    /* Get a potion */
    if (!get_item(&item, "Aim which wand? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have no wand to aim.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* Mega-Hack -- refuse to aim a pile from the ground */
    if ((item < 0) && (i_ptr->number > 1)) {
        msg_print("You must first pick up the wands.");
        return;
    }

    /* Hack -- verify potential overflow */
    if ((i_ptr->number > 1) && (inven_ctr >= INVEN_PACK)) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack might overflow.  Continue?")) return;
    }


    /* Allow direction to be cancelled for free */
    if (!get_dir_c(NULL, &dir)) return;


    /* The turn is not free */
    energy_use = 100;

    /* Get the level */
    lev = k_list[i_ptr->k_idx].level;

    /* Chance of success */
    chance = (p_ptr->save + stat_adj(A_INT) - (int)(lev > 42 ? 42 : lev) +
              (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused) chance = chance / 2;

    /* Give everyone a slight chance */
    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
        chance = USE_DEVICE;
    }

    if (chance <= 0) chance = 1;

    if (randint(chance) < USE_DEVICE) {
        if (flush_failure) flush();
        msg_print("You failed to use the wand properly.");
        return;
    }

    /* The wand is already empty! */
    if (i_ptr->pval <= 0) {
        if (flush_failure) flush();
        msg_print("The wand has no charges left.");
        if (!inven_known_p(i_ptr)) {
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

    /* Various effects */
    switch (sval) {

        case SV_WAND_LITE:
            msg_print("A line of blue shimmering light appears.");
            lite_line(dir, py, px);
            ident = TRUE;
            break;

        case SV_WAND_ACID:
            bolt_or_beam(20, GF_ACID, dir, damroll(5,8));
            ident = TRUE;
            break;

        case SV_WAND_ELEC:
            bolt_or_beam(15, GF_ELEC, dir, damroll(3,8));
            ident = TRUE;
            break;

        case SV_WAND_COLD:
            bolt_or_beam(15, GF_COLD, dir, damroll(3,8));
            ident = TRUE;
            break;

        case SV_WAND_FIRE:
            bolt_or_beam(25, GF_FIRE, dir, damroll(6,8));
            ident = TRUE;
            break;

        case SV_WAND_STONE_TO_MUD:
            ident = wall_to_mud(dir,py,px) || ident;
            break;

        case SV_WAND_POLYMORPH:
            ident = poly_monster(dir,py,px) || ident;
            break;

        case SV_WAND_HEAL_MONSTER:
            ident = heal_monster(dir,py,px) || ident;
            break;

        case SV_WAND_HASTE_MONSTER:
            ident = speed_monster(dir,py,px) || ident;
            break;

        case SV_WAND_SLOW_MONSTER:
            ident = slow_monster(dir,py,px) || ident;
            break;

        case SV_WAND_CONFUSE_MONSTER:
            ident = confuse_monster(dir,py,px,10) || ident;
            break;

        case SV_WAND_SLEEP_MONSTER:
            ident = sleep_monster(dir,py,px) || ident;
            break;

        case SV_WAND_DRAIN_LIFE:
            ident = drain_life(dir,py,px,75) || ident;
            break;

        case SV_WAND_TRAP_DOOR_DEST:
            ident = td_destroy2(dir,py,px) || ident;
            break;

        case SV_WAND_MAGIC_MISSILE:
            bolt_or_beam(15, GF_MISSILE, dir, damroll(2,6));
            ident = TRUE;
            break;

        case SV_WAND_FEAR_MONSTER:
            ident = fear_monster(dir,py,px,10) || ident;
            break;

        case SV_WAND_CLONE_MONSTER:
            ident = clone_monster(dir,py,px) || ident;
            break;

        case SV_WAND_TELEPORT_AWAY:
            ident = teleport_monster(dir,py,px) || ident;
            break;

        case SV_WAND_DISARMING:
            ident = disarm_all(dir,py,px) || ident;
            break;

        case SV_WAND_ELEC_BALL:
            fire_ball(GF_ELEC, dir,py,px,32,2);
            ident = TRUE;
            break;

        case SV_WAND_COLD_BALL:
            fire_ball(GF_COLD,dir,py,px,48,2);
            ident = TRUE;
            break;

        case SV_WAND_FIRE_BALL:
            fire_ball(GF_FIRE,dir,py,px,72,2);
            ident = TRUE;
            break;

        case SV_WAND_STINKING_CLOUD:
            fire_ball(GF_POIS,dir,py,px,12,2);
            ident = TRUE;
            break;

        case SV_WAND_ACID_BALL:
            fire_ball(GF_ACID,dir,py,px,60,2);
            ident = TRUE;
            break;

        case SV_WAND_WONDER:
            message("Oops.  Wand of wonder activated.", 0);
            break;

        case SV_WAND_DRAGON_FIRE:
            fire_ball(GF_FIRE, dir, py, px, 100, 3);
            ident = TRUE;
            break;

        case SV_WAND_DRAGON_COLD:
            fire_ball(GF_COLD, dir, py, px, 80, 3);
            ident = TRUE;
            break;

        case SV_WAND_DRAGON_BREATH:
            switch (randint(5)) {
              case 1:
                fire_ball(GF_FIRE, dir, py, px, 100, 3);
                break;
              case 2:
                fire_ball(GF_COLD, dir, py, px, 80, 3);
                break;
              case 3:
                fire_ball(GF_ACID, dir, py, px, 90, 3);
                break;
              case 4:
                fire_ball(GF_ELEC, dir, py, px, 70, 3);
                break;
              default:
                fire_ball(GF_POIS, dir, py, px, 70, 3);
                break;
            }
            ident = TRUE;
            break;

        case SV_WAND_ANNIHILATION:
            ident = drain_life(dir,py,px,125) || ident;
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
        check_experience();
        inven_aware(i_ptr);
    }

    /* Use a single charge */
    i_ptr->pval--;

    /* Hack -- unstack if necessary */
    if ((item >= 0) && (i_ptr->number > 1)) {

        /* Make a fake item */
        inven_type tmp_obj;
        tmp_obj = *i_ptr;
        tmp_obj.number = 1;

        /* Restore the charges */
        i_ptr->pval++;

        /* Unstack the used item */
        i_ptr->number--;
        inven_weight -= tmp_obj.weight;
        item = inven_carry(&tmp_obj);
    }

    /* Describe the remaining charges */
    if (item < 0) {
        floor_item_charges(oy, ox);
    }
    else {
        inven_item_charges(item);
    }

    /* Hack -- combine the pack */
    combine_pack();
}



/*
 * Is the given item a staff?
 */
static bool item_tester_hook_staff(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_STAFF);
}




/*
 * Use a staff.			-RAK-	
 *
 * One charge of one staff disappears.
 *
 * Hack -- staffs of identify can be "cancelled".
 */
void do_cmd_use_staff(void)
{
    int			ident, chance, k, lev;
    int			oy, ox, item;
    bool		floor;
    inven_type		*i_ptr;

    /* Hack -- let staffs of identify get aborted */
    bool use_charge = TRUE;


    /* Assume free turn */
    energy_use = 0;

    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_staff;

    /* See if we can use the item on the floor */
    floor = item_tester_hook(i_ptr);

    /* Get a potion */
    if (!get_item(&item, "Use which staff? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have no staff to use.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* Mega-Hack -- refuse to use a pile from the ground */
    if ((item < 0) && (i_ptr->number > 1)) {
        msg_print("You must first pick up the staffs.");
        return;
    }

    /* Hack -- verify potential overflow */
    if ((i_ptr->number > 1) && (inven_ctr >= INVEN_PACK)) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack might overflow.  Continue?")) return;
    }


    /* Extract the item level */
    lev = k_list[i_ptr->k_idx].level;

    /* Chance of success */
    chance = (p_ptr->save + stat_adj(A_INT) - (int)(lev > 50 ? 50 : lev) +
              (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused) chance = chance / 2;

    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
        chance = USE_DEVICE;   /* Give everyone a slight chance */
    }

    if (chance <= 0) chance = 1;

    if (randint(chance) < USE_DEVICE) {
        if (flush_failure) flush();
        msg_print("You failed to use the staff properly.");
        energy_use = 100;
        return;
    }

    if (i_ptr->pval <= 0) {
        if (flush_failure) flush();
        msg_print("The staff has no charges left.");
        if (!inven_known_p(i_ptr)) i_ptr->ident |= ID_EMPTY;
        energy_use = 100;
        return;
    }

    ident = FALSE;

    switch (i_ptr->sval) {

      case SV_STAFF_HEALING:
        ident = hp_player(300);
        if (p_ptr->stun) {
            p_ptr->stun = 0;
            msg_print("Your head stops stinging.");
            p_ptr->update |= (PU_BONUS);
            ident = TRUE;
        }
        if (p_ptr->cut) {
            p_ptr->cut = 0;
            msg_print("You feel better.");
            ident = TRUE;
        }
        break;

      case SV_STAFF_GENOCIDE:
        (void)genocide(TRUE);
        ident = TRUE;
        break;

      case SV_STAFF_PROBING:
        probing();
        ident = TRUE;
        break;

      case SV_STAFF_IDENTIFY:
        if (!ident_spell()) use_charge = FALSE;
        ident = TRUE;
        break;

      case SV_STAFF_HOLINESS:
        /* Dispel (evil) monsters */
        dispel_monsters(120, TRUE, FALSE);
        protect_evil();
        cure_poison();
        remove_fear();
        hp_player(50);
        if (p_ptr->stun) {
            p_ptr->stun = 0;
            msg_print("Your head stops stinging.");
            p_ptr->update |= (PU_BONUS);
        }
        if (p_ptr->cut) {
            p_ptr->cut = 0;
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
            p_ptr->redraw |= PR_MANA;
        }
        break;

      case SV_STAFF_POWER:
        /* Dispel (all) monsters */
        dispel_monsters(120, FALSE, FALSE);
        break;

      case SV_STAFF_MAPPING:
        map_area();
        ident = TRUE;
        break;

      case SV_STAFF_LITE:
        ident = lite_area(py, px, damroll(2, 10), 2);
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
        teleport_flag = TRUE;
        teleport_dist = 100;
        ident = TRUE;
        break;

      case SV_STAFF_EARTHQUAKES:
        earthquake();
        ident = TRUE;
        break;

      case SV_STAFF_SUMMONING:
        for (k = 0; k < randint(4); k++) {
            if (summon_monster(py, px, dun_level + MON_SUMMON_ADJ)) {
                ident = TRUE;
            }
        }
        break;

      case SV_STAFF_DESTRUCTION:
        destroy_area(py, px);
        ident = TRUE;
        break;

      case SV_STAFF_STARLITE:
        starlite(py, px);
        ident = TRUE;
        break;

      case SV_STAFF_HASTE_MONSTERS:
        ident = speed_monsters() || ident;
        break;

      case SV_STAFF_SLOW_MONSTERS:
        ident = slow_monsters() || ident;
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
        if (p_ptr->stun) {
            msg_print("Your head stops stinging.");
            p_ptr->stun = 0;
            p_ptr->update |= (PU_BONUS);
            ident = TRUE;
        }
        if (p_ptr->cut) {
            msg_print("You feel better.");
            p_ptr->cut = 0;
            ident = TRUE;
        }
        break;

      case SV_STAFF_DISPEL_EVIL:
        /* Dispel (evil) monsters */
        if (dispel_monsters(60, TRUE, FALSE)) ident = TRUE;
        break;

      case SV_STAFF_DARKNESS:
        ident = unlite_area(py, px) || ident;
        break;

      default:
        msg_print("Oops.  Undefined staff.");
        break;
    }


    /* Take a turn */
    energy_use = 100;

    /* Tried the item */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
        check_experience();
        inven_aware(i_ptr);
    }

    /* Hack -- some uses are "free" */
    if (!use_charge) return;


    /* Use a single charge */
    i_ptr->pval--;

    /* XXX Hack -- unstack if necessary */
    if ((item >= 0) && (i_ptr->number > 1)) {

        /* Make a fake item */
        inven_type tmp_obj;
        tmp_obj = *i_ptr;
        tmp_obj.number = 1;

        /* Restore the charges */
        i_ptr->pval++;

        /* Unstack the used item */
        i_ptr->number--;
        inven_weight -= tmp_obj.weight;
        item = inven_carry(&tmp_obj);
    }

    /* Describe the remaining charges */
    if (item < 0) {
        floor_item_charges(oy, ox);
    }
    else {
        inven_item_charges(item);
    }

    /* Hack -- combine the pack */
    combine_pack();
}







/*
 * Is the given item a rod?
 */
static bool item_tester_hook_rod(inven_type *i_ptr)
{
    return (i_ptr->tval == TV_ROD);
}



/*
 * Activate (zap) a Rod
 *
 * Unstack fully charged rods as needed.
 *
 * Hack -- rods of perception/genocide can be "cancelled"
 * All rods can be cancelled at the "Direction?" prompt
 */
void do_cmd_zap_rod(void)
{
    int                 ident, chance, dir, lev;
    int			oy, ox, item;
    bool		floor;
    inven_type		*i_ptr;

    /* Hack -- let perception get aborted */
    bool use_charge = TRUE;


    /* Assume free turn */
    energy_use = 0;

    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Prepare the item tester hook */
    item_tester_hook = item_tester_hook_rod;

    /* See if we can use the item on the floor */
    floor = item_tester_hook(i_ptr);

    /* Get a potion */
    if (!get_item(&item, "Zap which rod? ", 0, inven_ctr-1, floor)) {
        if (item == -2) msg_print("You have no rod to zap.");
        return;
    }

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* Mega-Hack -- refuse to zap a pile from the ground */
    if ((item < 0) && (i_ptr->number > 1)) {
        msg_print("You must first pick up the rods.");
        return;
    }

    /* Hack -- verify potential overflow */
    if ((i_ptr->number > 1) && (inven_ctr >= INVEN_PACK)) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack might overflow.  Continue?")) return;
    }


    /* Extract the item level */
    lev = k_list[i_ptr->k_idx].level;

    /* Calculate the chance */
    chance = (p_ptr->save + (stat_adj(A_INT) * 2) - (int)((lev > 70) ? 70 : lev) +
              (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3));

    if (p_ptr->confused) chance = chance / 2;

    /* Give everyone a slight chance */
    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
        chance = USE_DEVICE;
    }

    /* Prevent errors in "randint" */
    if (chance <= 0) chance = 1;

    /* Fail to use */
    if (randint(chance) < USE_DEVICE) {
        if (flush_failure) flush();
        msg_print("You failed to use the rod properly.");
        energy_use = 100;
        return;
    }

    /* Still charging */
    if (i_ptr->pval) {
        if (flush_failure) flush();
        msg_print("The rod is still charging.");
        energy_use = 100;
        return;
    }

    /* Not identified yet */
    ident = FALSE;


    /* Assume the turn is free */
    energy_use = 100;

    /* Activate it */
    switch (i_ptr->sval) {

      case SV_ROD_LIGHT:
        if (!get_dir_c(NULL, &dir)) return;
        msg_print("A line of blue shimmering light appears.");
        lite_line(dir, py, px);
        ident = TRUE;
        i_ptr->pval = 9;
        break;

      case SV_ROD_ILLUMINATION:
        lite_area(py, px, damroll(2, 8), 2);
        ident = TRUE;
        i_ptr->pval = 30;
        break;

      case SV_ROD_ACID:
        if (!get_dir_c(NULL, &dir)) return;
        bolt_or_beam(10, GF_ACID, dir, damroll(6,8));
        ident = TRUE;
        i_ptr->pval = 12;
        break;

      case SV_ROD_ELEC:
        if (!get_dir_c(NULL, &dir)) return;
        bolt_or_beam(8, GF_ELEC, dir, damroll(3,8));
        ident = TRUE;
        i_ptr->pval = 11;
        break;

      case SV_ROD_COLD:
        if (!get_dir_c(NULL, &dir)) return;
        bolt_or_beam(10, GF_COLD, dir, damroll(5,8));
        ident = TRUE;
        i_ptr->pval = 13;
        break;

      case SV_ROD_FIRE:
        if (!get_dir_c(NULL, &dir)) return;
        bolt_or_beam(12, GF_FIRE, dir, damroll(8,8));
        ident = TRUE;
        i_ptr->pval = 15;
        break;

      case SV_ROD_POLYMORPH:
        if (!get_dir_c(NULL, &dir)) return;
        ident = poly_monster(dir, py, px);
        i_ptr->pval = 25;
        break;

      case SV_ROD_SLOW_MONSTER:
        if (!get_dir_c(NULL, &dir)) return;
        ident = slow_monster(dir, py, px);
        i_ptr->pval = 20;
        break;

      case SV_ROD_SLEEP_MONSTER:
        if (!get_dir_c(NULL, &dir)) return;
        ident = sleep_monster(dir, py, px);
        i_ptr->pval = 18;
        break;

      case SV_ROD_DRAIN_LIFE:
        if (!get_dir_c(NULL, &dir)) return;
        ident = drain_life(dir, py, px, 75);
        i_ptr->pval = 23;
        break;

      case SV_ROD_TELEPORT_AWAY:
        if (!get_dir_c(NULL, &dir)) return;
        ident = teleport_monster(dir, py, px);
        i_ptr->pval = 25;
        break;

      case SV_ROD_DISARMING:
        if (!get_dir_c(NULL, &dir)) return;
        ident = disarm_all(dir, py, px);
        i_ptr->pval = 30;
        break;

      case SV_ROD_ELEC_BALL:
        if (!get_dir_c(NULL, &dir)) return;
        fire_ball(GF_ELEC, dir, py, px, 32, 2);
        ident = TRUE;
        i_ptr->pval = 23;
        break;

      case SV_ROD_COLD_BALL:
        if (!get_dir_c(NULL, &dir)) return;
        fire_ball(GF_COLD, dir, py, px, 48, 2);
        ident = TRUE;
        i_ptr->pval = 25;
        break;

      case SV_ROD_FIRE_BALL:
        if (!get_dir_c(NULL, &dir)) return;
        fire_ball(GF_FIRE, dir, py, px, 72, 2);
        ident = TRUE;
        i_ptr->pval = 30;
        break;

      case SV_ROD_ACID_BALL:
        if (!get_dir_c(NULL, &dir)) return;
        fire_ball(GF_ACID, dir, py, px, 60, 2);
        ident = TRUE;
        i_ptr->pval = 27;
        break;

      case SV_ROD_MAPPING:
        map_area();
        ident = TRUE;
        i_ptr->pval = 99;
        break;

      case SV_ROD_IDENTIFY:
        ident = TRUE;
        if (!ident_spell()) use_charge = FALSE;
        i_ptr->pval = 10;
        break;

      case SV_ROD_CURING:
        if (cure_blindness()) ident = TRUE;
        if (cure_poison()) ident = TRUE;
        if (cure_confusion()) ident = TRUE;
        if (p_ptr->stun) {
            msg_print("Your head stops stinging.");
            p_ptr->stun = 0;
            p_ptr->update |= (PU_BONUS);
            ident = TRUE;
        }
        if (p_ptr->cut) {
            msg_print("You feel better.");
            p_ptr->cut = 0;
            ident = TRUE;
        }
        i_ptr->pval = 888;
        break;

      case SV_ROD_HEALING:
        ident = hp_player(500);
        if (p_ptr->stun) {
            msg_print("Your head stops stinging.");
            p_ptr->stun = 0;
            p_ptr->update |= (PU_BONUS);
            ident = TRUE;
        }
        if (p_ptr->cut) {
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
        ident = build_wall(dir, py, px);
        i_ptr->pval = 999;
        break;
#endif

      default:
        msg_print("Oops.  Undefined rod.");
        break;
    }


    /* The turn is not free */
    energy_use = 100;

    /* Tried the object */
    inven_tried(i_ptr);

    /* Successfully determined the object function */
    if (ident && !inven_aware_p(i_ptr)) {
        p_ptr->exp += (lev + (p_ptr->lev >> 1)) / p_ptr->lev;
        check_experience();
        inven_aware(i_ptr);
    }


    /* Hack -- deal with cancelled zap */
    if (!use_charge) {
        i_ptr->pval = 0;
        return;
    }


    /* XXX Hack -- unstack if necessary */
    if ((item >= 0) && (i_ptr->number > 1)) {

        /* Make a fake item */
        inven_type tmp_obj;
        tmp_obj = *i_ptr;
        tmp_obj.number = 1;

        /* Restore "charge" */
        i_ptr->pval = 0;

        /* Unstack the used item */
        i_ptr->number--;
        inven_weight -= tmp_obj.weight;
        item = inven_carry(&tmp_obj);
    }


    /* Hack -- combine the pack */
    combine_pack();
}




/*
 * Hook to determine if an object is activatable
 */
static bool item_tester_hook_activate(inven_type *i_ptr)
{
    /* Not wieldable */
    if (!wearable_p(i_ptr)) return (FALSE);

    /* Not known */
    if (!inven_known_p(i_ptr)) return (FALSE);

    /* Check activation flag */
    if (i_ptr->flags3 & TR3_ACTIVATE) return (TRUE);

    /* Assume not */
    return (FALSE);
}



/*
 * Activate a wielded object.  Wielded objects never stack.
 * And even if they did, activatable objects never stack.
 *
 * Currently, only (some) artifacts, and Dragon Scale Mail, can be activated.
 * But one could, for example, easily make an activatable "Ring of Plasma".
 */
void do_cmd_activate(void)
{
    int         i1, i2, i;
    int         a, dir, chance, what;
    inven_type  *i_ptr;
    char        tmp2[200];

    /* Assume the turn is free */
    energy_use = 0;

    /* None found yet */
    i1 = i2 = -1;

    /* Scan for something to activate */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Nothing */
        i_ptr = &inventory[i];

        /* Must be activatable, and we must know it */
        if (!item_tester_hook_activate(i_ptr)) continue;

        /* Save the minimum index */
        if (i1 < 0) i1 = i;

        /* Save the maximum index */
        i2 = i;
    }

    /* Nothing found */
    if (i1 < 0) {
        msg_print("You are not wearing anything that can be activated.");
        return;
    }


    /* Prepare the hook */
    item_tester_hook = item_tester_hook_activate;

    /* Get an activatable item */
    if (!get_item(&what, "Activate which item? ", i1, i2, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the item */
    i_ptr = &inventory[what];

    /* Check the recharge */
    if (i_ptr->timeout) {
        msg_print("It whines, glows and fades...");
        energy_use = 100;
        return;
    }

    /* Are we smart enough? */
    if (p_ptr->use_stat[A_INT] < randint(18) &&
        randint(k_list[i_ptr->k_idx].level) > p_ptr->lev) {
        if (flush_failure) flush();
        msg_print("You failed to activate it properly.");
        energy_use = 100;
        return;
    }


    /* Wonder Twin Powers... Activate! */
    msg_print("You activate it...");


    /* Artifacts activate by name */
    if (i_ptr->name1) {

        /* This needs to be changed */
        switch (i_ptr->name1) {

            case ART_NARTHANC:
                msg_print("Your dagger is covered in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_FIRE, dir, py, px, damroll(9, 8));
                i_ptr->timeout = rand_int(8) + 8;
                break;

            case ART_NIMTHANC:
                msg_print("Your dagger is covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_COLD, dir, py, px, damroll(6, 8));
                i_ptr->timeout = rand_int(7) + 7;
                break;

            case ART_DETHANC:
                msg_print("Your dagger is covered in sparks...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ELEC, dir, py, px, damroll(4, 8));
                i_ptr->timeout = rand_int(6) + 6;
                break;

            case ART_RILIA:
                msg_print("Your dagger throbs deep green...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_POIS, dir, py, px, 12, 3);
                i_ptr->timeout = rand_int(4) + 4;
                break;

            case ART_BELANGIL:
                msg_print("Your dagger is covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, py, px, 48, 2);
                i_ptr->timeout = rand_int(5) + 5;
                break;

            case ART_DAL:
                msg_print("You feel energy flow through your feet...");
                remove_fear();
                cure_poison();
                i_ptr->timeout = 5;
                break;

            case ART_RINGIL:
                msg_print("Your sword glows an intense blue...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, py, px, 100, 2);
                i_ptr->timeout = 300;
                break;

            case ART_ANDURIL:
                msg_print("Your sword glows an intense red...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, py, px, 72, 2);
                i_ptr->timeout = 400;
                break;

            case ART_FIRESTAR:
                msg_print("Your morningstar rages in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, py, px, 72, 3);
                i_ptr->timeout = 100;
                break;

            case ART_FEANOR:
                p_ptr->fast += randint(20) + 20;
                i_ptr->timeout = 200;
                break;

            case ART_THEODEN:
                msg_print("The blade of your axe glows black...");
                if (!get_dir_c(NULL, &dir)) return;
                drain_life(dir, py, px, 120);
                i_ptr->timeout = 400;
                break;

            case ART_TURMIL:
                msg_print("The head of your hammer glows white...");
                if (!get_dir_c(NULL, &dir)) return;
                drain_life(dir, py, px, 90);
                i_ptr->timeout = 70;
                break;

            case ART_CASPANION:
                msg_print("Your mail magically disarms traps...");
                td_destroy();
                i_ptr->timeout = 10;
                break;

            case ART_AVAVIR:
                if (p_ptr->word_recall == 0) {
                    p_ptr->word_recall = randint(20) + 15;
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
                    p_ptr->fast += randint(20) + 20;
                }
                i_ptr->timeout = rand_int(100) + 100;
                break;

            case ART_ERIRIL:
                /* Identify and combine pack */
                if (ident_spell()) combine_pack();
                /* XXX Note that the artifact is always de-charged */
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
                if (p_ptr->cut) {
                    p_ptr->cut = (p_ptr->cut / 2) - 50;
                    if (p_ptr->cut < 0) p_ptr->cut = 0;
                    msg_print("Your wounds heal.");
                }
                i_ptr->timeout = rand_int(3) + 3;
                break;

            case ART_CUBRAGOL:

                /* Use the first (XXX) acceptable bolts */
                for (a = 0; a < inven_ctr; a++) {
                    inven_type *j_ptr = &inventory[a];
                    if ((j_ptr->tval == TV_BOLT) &&
                        (!j_ptr->name1) && (!j_ptr->name2) &&
                        (!cursed_p(j_ptr))) break;
                }

                /* Enchant the bolts (or fail) */
                if ((a < inven_ctr) && (rand_int(4) == 0)) {
                    inven_type *j_ptr = &inventory[a];
                    msg_print("Your bolts are covered in a fiery aura!");
                    j_ptr->name2 = EGO_AMMO_FIRE;
                    j_ptr->flags1 |= (TR1_BRAND_FIRE);
                    j_ptr->flags3 |= (TR3_IGNORE_FIRE);
                    j_ptr->cost += 25;
                    enchant(j_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);
                }
                else {
                    if (flush_failure) flush();
                    msg_print("The fiery enchantment failed.");
                }

                i_ptr->timeout = 999;
                break;

            case ART_ARUNRUTH:
                msg_print("Your sword glows a pale blue...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_COLD,
                          dir, py, px, damroll(12, 8));
                i_ptr->timeout = 500;
                break;

            case ART_AEGLOS:
                msg_print("Your spear glows a bright white...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, py, px, 100, 2);
                i_ptr->timeout = 500;
                break;

            case ART_OROME:
                msg_print("Your spear pulsates...");
                if (!get_dir_c(NULL, &dir)) return;
                wall_to_mud(dir, py, px);
                i_ptr->timeout = 5;
                break;

            case ART_SOULKEEPER:
                msg_print("Your armour glows a bright white...");
                msg_print("You feel much better...");
                hp_player(1000);
                if (p_ptr->cut) {
                    p_ptr->cut = 0;
                    msg_print("Your wounds heal.");
                }
                i_ptr->timeout = 888;
                break;

            case ART_BELEGENNON:
                teleport_flag = TRUE;
                teleport_dist = 10;
                i_ptr->timeout = 2;
                break;

            case ART_CELEBORN:
                (void)genocide(TRUE);
                i_ptr->timeout = 500;
                break;

            case ART_LUTHIEN:
                restore_level();
                i_ptr->timeout = 450;
                break;

            case ART_ULMO:
                msg_print("Your trident glows deep red...");
                if (!get_dir_c(NULL, &dir)) return;
                teleport_monster(dir, py, px);
                i_ptr->timeout = 150;
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
                sleep_monsters1(py, px);
                i_ptr->timeout = 55;
                break;

            case ART_THINGOL:
                msg_print("You hear a low humming noise...");
                recharge(60);
                i_ptr->timeout = 70;
                break;

            case ART_COLANNON:
                teleport_flag = TRUE;
                teleport_dist = 100;
                i_ptr->timeout = 45;
                break;

            case ART_TOTILA:
                msg_print("Your flail glows in scintillating colours...");
                if (!get_dir_c(NULL, &dir)) return;
                confuse_monster(dir, py, px, 20);
                i_ptr->timeout = 15;
                break;

            case ART_CAMMITHRIM:
                msg_print("Your gloves glow extremely brightly...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_MISSILE, dir, py, px,
                          damroll(2, 6));
                i_ptr->timeout = 2;
                break;

            case ART_PAURHACH:
                msg_print("Your gauntlets are covered in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                bolt_or_beam(25, GF_FIRE, dir, damroll(9,8));
                i_ptr->timeout = rand_int(8) + 8;
                break;

            case ART_PAURNIMMEN:
                msg_print("Your gauntlets are covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_COLD,
                          dir, py, px, damroll(6, 8));
                i_ptr->timeout = rand_int(7) + 7;
                break;

            case ART_PAURAEGEN:
                msg_print("Your gauntlets are covered in sparks...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ELEC, dir, py, px, damroll(4, 8));
                i_ptr->timeout = rand_int(6) + 6;
                break;

            case ART_PAURNEN:
                msg_print("Your gauntlets look very acidic...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ACID, dir, py, px, damroll(5, 8));
                i_ptr->timeout = rand_int(5) + 5;
                break;

            case ART_FINGOLFIN:
                msg_print("Magical spikes appear on your cesti...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ARROW, dir, py, px, 150);
                i_ptr->timeout = rand_int(90) + 90;
                break;

            case ART_HOLHENNETH:
                message("You close your eyes ", 0x02);
                message("and an image forms in your mind...", 0);
                detection();
                i_ptr->timeout = rand_int(55) + 55;
                break;

            case ART_GONDOR:
                msg_print("You feel a warm tingling inside...");
                hp_player(500);
                if (p_ptr->cut) {
                    p_ptr->cut = 0;
                    msg_print("Your wounds heal.");
                }
                i_ptr->timeout = 500;
                break;

            case ART_RAZORBACK:
                message("A storm of lightning spikes ", 0x02);
                message("fires in all directions...", 0);
                starball(py, px);
                i_ptr->timeout = 1000;
                break;

            case ART_BLADETURNER:
                msg_print("Your armour glows many colours...");
                msg_print("You enter a berserk rage...");
                msg_print("You feel you can resist anything...");
                hp_player(40);	/* XXX */
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
                lite_area(py, px, damroll(2, 15), 3);
                i_ptr->timeout = rand_int(10) + 10;
                break;

            case ART_ELENDIL:
                msg_print("The star shines brightly...");
                map_area();
                i_ptr->timeout = rand_int(50) + 50;
                break;

            case ART_THRAIN:
                msg_print("The stone glows a deep green...");
                wiz_lite();
                (void)detect_sdoor();
                (void)detect_trap();
                i_ptr->timeout = rand_int(100) + 100;
                break;


            case ART_INGWE:
                msg_print("An aura of good floods the area...");
                /* Dispel (evil) monsters */
                dispel_monsters((int)(5 * p_ptr->lev), TRUE, FALSE);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case ART_CARLAMMAS:
                msg_print("The amulet lets out a shrill wail...");
                msg_print("You feel somewhat safer...");
                protect_evil();
                i_ptr->timeout = rand_int(225) + 225;
                break;


            case ART_TULKAS:
                msg_print("The ring glows brightly...");
                p_ptr->fast += randint(75) + 75;
                i_ptr->timeout = rand_int(150) + 150;
                break;

            case ART_NARYA:
                msg_print("The ring glows deep red...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, py, px, 120, 3);
                i_ptr->timeout = rand_int(225) + 225;
                break;

            case ART_NENYA:
                msg_print("The ring glows bright white...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, py, px, 200, 3);
                i_ptr->timeout = rand_int(325) + 325;
                break;

            case ART_VILYA:
                msg_print("The ring glows deep blue...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_ELEC, dir, py, px, 250, 3);
                i_ptr->timeout = rand_int(425) + 425;
                break;

            case ART_POWER:
                msg_print("The ring glows intensely black...");

                if (!get_dir_c(NULL, &dir)) return;

                switch (randint(17) + (8 - p_ptr->lev / 10)) {
                  case 5:
                    /* Dispel (all) monsters */
                    dispel_monsters(1000, FALSE, FALSE);
                    break;
                  case 6:
                  case 7:
                    msg_print("You are surrounded by a malignant aura");
                    p_ptr->lev--;
                    p_ptr->exp = (player_exp[p_ptr->lev - 2] *
                                  p_ptr->expfact / 100) +
                                 randint((player_exp[p_ptr->lev - 1] *
                                          p_ptr->expfact / 100) -
                                         (player_exp[p_ptr->lev - 2] *
                                          p_ptr->expfact / 100));
                    p_ptr->max_exp = p_ptr->exp;
                    check_experience();
                    dec_stat(A_STR, 50, TRUE);
                    dec_stat(A_INT, 50, TRUE);
                    dec_stat(A_WIS, 50, TRUE);
                    dec_stat(A_DEX, 50, TRUE);
                    dec_stat(A_CON, 50, TRUE);
                    dec_stat(A_CHR, 50, TRUE);
                    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
                    p_ptr->redraw |= (PR_BLOCK);
                    take_hit((p_ptr->chp > 2) ? p_ptr->chp / 2 : 0,
                             "malignant aura");
                    break;
                  case 8:
                  case 9:
                  case 10:
                    fire_ball(GF_MANA, dir, py, px, 300, 3);
                    break;
                  default:
                    fire_bolt(GF_MANA, dir, py, px, 250);
                }
                i_ptr->timeout = rand_int(450) + 450;
                break;


            default:
                message("Oops.  Non-Activatable Artifact.", 0);
        }

        /* Take a turn */
        energy_use = 100;

        /* Done */
        return;
    }


    /* Hack -- Dragon Scale Mail can be activated as well */
    if (i_ptr->tval == TV_DRAG_ARMOR) {

        /* Get a direction for breathing (or abort) */
        if (!get_dir_c(NULL, &dir)) return;

        /* Branch on the sub-type */
        switch (i_ptr->sval) {

            case SV_DRAGON_BLUE:
                msg_print("You breathe lightning.");
                fire_ball(GF_ELEC, dir, py, px, 100, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_WHITE:
                msg_print("You breathe frost.");
                fire_ball(GF_COLD, dir, py, px, 110, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_BLACK:
                msg_print("You breathe acid.");
                fire_ball(GF_ACID, dir, py, px, 130, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_GREEN:
                msg_print("You breathe poison gas.");
                fire_ball(GF_POIS, dir, py, px, 150, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_RED:
                msg_print("You breathe fire.");
                fire_ball(GF_FIRE, dir, py, px, 200, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_MULTIHUED:
                chance = rand_int(5);
                sprintf(tmp2, "You breathe %s.",
                        ((chance == 1) ? "lightning" :
                         ((chance == 2) ? "frost" :
                          ((chance == 3) ? "acid" :
                           ((chance == 4) ? "poison gas" : "fire")))));
                msg_print(tmp2);
                fire_ball(((chance == 1) ? GF_ELEC :
                           ((chance == 2) ? GF_COLD :
                            ((chance == 3) ? GF_ACID :
                             ((chance == 4) ? GF_POIS : GF_FIRE)))),
                          dir, py, px, 250, 2);
                i_ptr->timeout = rand_int(225) + 225;
                break;

            case SV_DRAGON_BRONZE:
                msg_print("You breathe confusion.");
                fire_ball(GF_CONFUSION, dir, py, px, 120, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_GOLD:
                msg_print("You breathe sound.");
                fire_ball(GF_SOUND, dir, py, px, 130, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_CHAOS:
                chance = rand_int(2);
                sprintf(tmp2, "You breathe %s.",
                        ((chance == 0 ? "chaos" : "disenchantment")));
                msg_print(tmp2);
                fire_ball((chance == 0 ? GF_CHAOS : GF_DISENCHANT), dir,
                          py, px, 220, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_LAW:
                chance = rand_int(2);
                sprintf(tmp2, "You breathe %s.",
                        ((chance == 0 ? "sound" : "shards")));
                msg_print(tmp2);
                fire_ball((chance == 0 ? GF_SOUND : GF_SHARDS), dir,
                          py, px, 230, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_BALANCE:
                chance = rand_int(4);
                sprintf(tmp2, "You breathe %s.",
                        ((chance == 1) ? "chaos" :
                         ((chance == 2) ? "disenchantment" :
                          ((chance == 3) ? "sound" : "shards"))));
                msg_print(tmp2);
                fire_ball(((chance == 1) ? GF_CHAOS :
                           ((chance == 2) ? GF_DISENCHANT :
                            ((chance == 3) ? GF_SOUND : GF_SHARDS))),
                          dir, py, px, 250, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_SHINING:
                chance = rand_int(2);
                sprintf(tmp2, "You breathe %s.",
                        ((chance == 0 ? "light" : "darkness")));
                msg_print(tmp2);
                fire_ball((chance == 0 ? GF_LITE : GF_DARK),
                          dir, py, px, 200, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_POWER:
                msg_print("You breathe the elements.");
                fire_ball(GF_MISSILE,
                          dir, py, px, 300, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            default:
                msg_print("Oops.  You have bad breath.");
        }

        /* Take a turn */
        energy_use = 100;

        /* Success */
        return;
    }


    /* Mistake */
    msg_print("Oops.  That object cannot be activated.");
}


