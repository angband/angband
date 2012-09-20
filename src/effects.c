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
 * Note that food/potions/scrolls no longer use bit-flags for effects,
 * but instead use the "sval" (which is also used to sort the objects).
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


#if 0

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

#endif


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
        if (i_ptr->k_idx) {
            inven_aware(i_ptr);
            inven_known(i_ptr);
        }
    }
}


/*
 * Add to the players food time				-RAK-	
 *
 * Eating a food ration (5000 food) while totally full will
 * gorge you by 5000/50 = 100 food.  While gorged, your speed
 * is lowered by 10 points, and you digest at normal speed even
 * if resting.  A normal speed player (digestion 2) will thus
 * be slow for about 50*10 game turns, in which time he will get
 * about 25 full player moves.  Faster players will spend fewer
 * game turns gorged, but will have more player moves in that
 * same amount of time.
 */
void add_food(int num)
{
    int old, extra;

    /* Cancel starvation */
    if (p_ptr->food < 0) p_ptr->food = 0;

    /* Save the old value */
    old = p_ptr->food;
    
    /* Add the food */
    p_ptr->food += num;

    /* Paranoia -- Overflow check */
    if ((num > 0) && (p_ptr->food <= 0)) p_ptr->food = 32000;

    /* Check for bloating */
    if (p_ptr->food >= PY_FOOD_MAX) {

        /* Message */
        msg_print("You have gorged yourself into a bloated state!");

        /* Obtain the "bloat value" */
        extra = p_ptr->food - PY_FOOD_MAX;
        if (extra > num) extra = num;

        /* Do not get "full" effect from bloat food */
        p_ptr->food = ((old > PY_FOOD_MAX) ? old : PY_FOOD_MAX);
        
        /* Get a little bit bloated (slow down player) */
        p_ptr->food += extra / 50;
    }

    /* Check for "full" */
    else if (p_ptr->food >= PY_FOOD_FULL) {

        /* Message */
        msg_print("You are full.");
    }
}




/*
 * Hack -- do "inven_item_charges()" on a floor item.
 */
static void floor_item_charges(int y, int x)
{
    inven_type *i_ptr = &i_list[cave[y][x].i_idx];

    /* Require staff/wand */
    if ((i_ptr->tval != TV_STAFF) && (i_ptr->tval != TV_WAND)) return;
    
    /* Require known item */
    if (!inven_known_p(i_ptr)) return;

    /* Multiple charges */
    if (i_ptr->pval != 1) {

        /* Print a message */
        msg_format("There are %d charges remaining.", i_ptr->pval);
    }
        
    /* Single charge */
    else {

        /* Print a message */
        msg_format("There is %d charge remaining.", i_ptr->pval);
    }
}




/*
 * Hack -- do "inven_item_describe()" on a floor item.
 */
static void floor_item_describe(int y, int x)
{
    inven_type *i_ptr = &i_list[cave[y][x].i_idx];

    char	i_name[80];
    
    /* Get a description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Multiple items */
    if (i_ptr->number != 1) {

        /* Print a message */
        msg_format("There are %s.", i_name);
    }

    /* Single item */
    else {

        /* Print a message */
        msg_format("There is %s.", i_name);
    }
}






/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(void)
{
    int			oy, ox, item, ident;

    inven_type		*i_ptr;


    /* Assume the turn will be free */
    energy_use = 0;


    /* Save the player location */
    oy = py, ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Restrict choices to food */
    item_tester_tval = TV_FOOD;

    /* Get an item */
    if (!get_item(&item, "Eat which item? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have nothing to eat.");
        return;
    }

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* The turn is not free */
    energy_use = 100;

    /* Identity not known yet */
    ident = FALSE;

    /* Analyze the food */
    switch (i_ptr->sval) {

        case SV_FOOD_POISON:
            if (add_poisoned(rand_int(10) + 10)) {
                msg_print("You are poisoned!");
                ident = TRUE;
            }
            break;

        case SV_FOOD_BLINDNESS:
            if (add_blind(rand_int(200) + 200)) {
                msg_print("A veil of darkness surrounds you.");
                ident = TRUE;
            }
            break;

        case SV_FOOD_PARANOIA:
            if (add_fear(rand_int(10) + 10)) {
                msg_print("You feel terrified!");
                ident = TRUE;
            }
            break;

        case SV_FOOD_CONFUSION:
            if (add_confused(rand_int(10) + 10)) {
                msg_print("You feel drugged.");
                ident = TRUE;
            }
            break;

        case SV_FOOD_HALLUCINATION:
            if (add_image(rand_int(250) + 250)) {
                msg_print("You feel drugged.");
                ident = TRUE;
            }
            break;

        case SV_FOOD_PARALYSIS:
            if (add_paralysis(rand_int(10) + 10)) {
                msg_print("You are paralyzed!");
                ident = TRUE;
            }
            break;

        case SV_FOOD_WEAKNESS:
            take_hit(damroll(6, 6), "poisonous food.");
            lose_str();
            ident = TRUE;
            break;

        case SV_FOOD_SICKNESS:
            take_hit(damroll(6, 6), "poisonous food.");
            lose_con();
            ident = TRUE;
            break;

        case SV_FOOD_STUPIDITY:
            take_hit(damroll(8, 8), "poisonous food.");
            lose_int();
            ident = TRUE;
            break;

        case SV_FOOD_NAIVETY:
            take_hit(damroll(8, 8), "poisonous food.");
            lose_wis();
            ident = TRUE;
            break;

        case SV_FOOD_UNHEALTH:
            take_hit(damroll(10, 10), "poisonous food.");
            lose_con();
            ident = TRUE;
            break;

        case SV_FOOD_DISEASE:
            take_hit(damroll(10, 10), "poisonous food.");
            lose_str();
            ident = TRUE;
            break;

        case SV_FOOD_CURE_POISON:
            if (cure_poison()) ident = TRUE;
            break;

        case SV_FOOD_CURE_BLINDNESS:
            if (cure_blindness()) ident = TRUE;
            break;

        case SV_FOOD_CURE_PARANOIA:
            if (cure_fear()) ident = TRUE;
            break;

        case SV_FOOD_CURE_CONFUSION:
            if (cure_confusion()) ident = TRUE;
            break;

        case SV_FOOD_CURE_SERIOUS:
            if (hp_player(damroll(4, 8))) ident = TRUE;
            break;

        case SV_FOOD_RESTORE_STR:
            if (res_stat(A_STR)) {
                msg_print("You feel your strength returning.");
                ident = TRUE;
            }
            break;
            
        case SV_FOOD_RESTORE_CON:
            if (res_stat(A_CON)) {
                msg_print("You feel your health returning.");
                ident = TRUE;
            }
            break;

        case SV_FOOD_RESTORING:
            if (res_stat(A_STR)) {
                msg_print("You feel your strength returning.");
                ident = TRUE;
            }
            if (res_stat(A_CON)) {
                msg_print("You feel your health returning.");
                ident = TRUE;
            }
            if (res_stat(A_INT)) {
                msg_print("Your head spins a moment.");
                ident = TRUE;
            }
            if (res_stat(A_WIS)) {
                msg_print("You feel your wisdom returning.");
                ident = TRUE;
            }
            if (res_stat(A_DEX)) {
                msg_print("You feel more dextrous.");
                ident = TRUE;
            }
            if (res_stat(A_CHR)) {
                msg_print("Your skin stops itching.");
                ident = TRUE;
            }
            break;


        case SV_FOOD_RATION:
        case SV_FOOD_BISCUIT:
        case SV_FOOD_JERKY:
        case SV_FOOD_SLIME_MOLD:
            msg_print("That tastes good.");
            ident = TRUE;
            break;

        case SV_FOOD_WAYBREAD:
            msg_print("That tastes good.");
            (void)cure_poison();
            (void)hp_player(damroll(4, 8));
            ident = TRUE;
            break;

        case SV_FOOD_PINT_OF_ALE:
        case SV_FOOD_PINT_OF_WINE:
            msg_print("That tastes good.");
            ident = TRUE;
            break;
            
        default:
            msg_print("Oops.  Undefined food effect.");
            break;
    }


    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);
    
    /* We have tried it */
    inven_tried(i_ptr);

    /* The player is now aware of the object */
    if (ident && !inven_aware_p(i_ptr)) {
        int lev = k_list[i_ptr->k_idx].level;
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
        inven_aware(i_ptr);
    }

    /* Consume the food */
    add_food(i_ptr->pval);


    /* Destroy a food on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_describe(oy, ox);
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
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(void)
{
    int		oy, ox, item, ident, old;

    inven_type	*i_ptr;


    /* Assume the turn will be free */
    energy_use = 0;


    /* Save the old player location */
    oy = py, ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Restrict choices to potions */
    item_tester_tval = TV_POTION;

    /* Get a potion */
    if (!get_item(&item, "Quaff which potion? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have no potions to quaff.");
        return;
    }

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* The turn is not free */
    energy_use = 100;

    /* Not identified yet */
    ident = FALSE;

    /* Analyze the potion */
    switch (i_ptr->sval) {
    
        case SV_POTION_SLIME_MOLD:
            msg_print("You feel less thirsty.");
            ident = TRUE;
            break;

        case SV_POTION_APPLE_JUICE:
            msg_print("You feel less thirsty.");
            ident = TRUE;
            break;

        case SV_POTION_WATER:
            msg_print("You feel less thirsty.");
            ident = TRUE;
            break;

        case SV_POTION_SLOWNESS:
            old = p_ptr->slow;
            if (add_slow(randint(25) + 15)) {
                if (!old) ident = TRUE;
            }
            break;

        case SV_POTION_SALT_WATER:
            msg_print("The potion makes you vomit!");
            if (p_ptr->food > 150) p_ptr->food = 150;
            (void)(cure_poison());
            add_paralysis(4);
            p_ptr->update |= (PU_BONUS);
            ident = TRUE;
            break;

        case SV_POTION_POISON:
            if (add_poisoned(rand_int(15) + 10)) {
                msg_print("You feel very sick.");
                ident = TRUE;
            }
            break;

        case SV_POTION_BLINDNESS:
            old = p_ptr->blind;
            if (add_blind(rand_int(100) + 100)) {
                if (!old) {
                    msg_print("You are covered by a veil of darkness.");
                    ident = TRUE;
                }
            }
            break;

        case SV_POTION_CONFUSION:
            old = p_ptr->confused;
            if (add_confused(rand_int(20) + 15)) {
                if (!old) {
                    msg_print("You mind becomes clouded and hazy.");
                    ident = TRUE;
                }
            }
            break;

        case SV_POTION_SLEEP:
            old = p_ptr->paralysis;
            if (add_paralysis(rand_int(4) + 4)) {
                if (!old) {
                    msg_print("You fall asleep.");
                    ident = TRUE;
                }
            }
            break;

        case SV_POTION_LOSE_MEMORIES:
            if (!p_ptr->hold_life && (p_ptr->exp > 0)) {
                msg_print("You feel your memories fade.");
                lose_exp(p_ptr->exp / 4);
            }
            else {
                msg_format("You feel %s, but quickly return.",
                           "your memories fade for a moment");
            }
            ident = TRUE;
            break;

        case SV_POTION_RUINATION:
            msg_print("Your nerves and muscles feel weak and lifeless!");
            take_hit(damroll(10, 10), "a potion of Ruination");
            dec_stat(A_DEX, 25, TRUE);
            dec_stat(A_WIS, 25, TRUE);
            dec_stat(A_CON, 25, TRUE);
            dec_stat(A_STR, 25, TRUE);
            dec_stat(A_CHR, 25, TRUE);
            dec_stat(A_INT, 25, TRUE);
            ident = TRUE;

        case SV_POTION_DEC_STR:
            ident = TRUE;
            lose_str();
            break;

        case SV_POTION_DEC_INT:
            ident = TRUE;
            lose_int();
            break;

        case SV_POTION_DEC_WIS:
            ident = TRUE;
            lose_wis();
            break;

        case SV_POTION_DEC_CHR:
            ident = TRUE;
            lose_chr();
            break;

        case SV_POTION_DETONATIONS:
            msg_print("Massive explosions rupture your body!");
            take_hit(damroll(50, 20), "a potion of Detonation");
            cut_player(5000);
            stun_player(75);
            ident = TRUE;
            break;

        case SV_POTION_DEATH:
            msg_print("A feeling of Death flows through your body.");
            take_hit(5000, "a potion of Death");
            ident = TRUE;
            break;

        case SV_POTION_INFRAVISION:
            old = p_ptr->tim_infra;
            if (add_tim_infra(100 + randint(100))) {
                if (!old) {
                    msg_print("Your eyes begin to tingle.");
                    ident = TRUE;
                }
            }
            break;

        case SV_POTION_DETECT_INVIS:
            old = p_ptr->tim_invis;
            if (add_tim_invis(12 + randint(12))) {
                if (!old) {
                    msg_print("Your eyes feel very sensitive.");
                    ident = TRUE;
                }
            }
            break;

        case SV_POTION_SLOW_POISON:
            if (p_ptr->poisoned) {
                msg_print("The effect of the poison has been reduced.");
                p_ptr->poisoned = (p_ptr->poisoned + 1) / 2;
                ident = TRUE;
            }
            break;

        case SV_POTION_CURE_POISON:
            if (cure_poison()) ident = TRUE;
            break;

        case SV_POTION_BOLDNESS:
            if (cure_fear()) ident = TRUE;
            break;
        
        case SV_POTION_SPEED:
            if (!p_ptr->fast) {
                add_fast(randint(25) + 15);
                ident = TRUE;
            }
            break;

        case SV_POTION_RESIST_HEAT:
            if (!p_ptr->oppose_fire) ident = TRUE;
            p_ptr->oppose_fire += randint(10) + 10;
            break;

        case SV_POTION_RESIST_COLD:
            if (!p_ptr->oppose_cold) ident = TRUE;
            p_ptr->oppose_cold += randint(10) + 10;
            break;

        case SV_POTION_HEROISM:
            if (hp_player(10)) ident = TRUE;	/* XXX */
            if (p_ptr->hero == 0) ident = TRUE;
            p_ptr->hero += randint(25) + 25;
            break;

        case SV_POTION_BESERK_STRENGTH:
            if (hp_player(30)) ident = TRUE;	/* XXX */
            if (p_ptr->shero == 0) ident = TRUE;
            p_ptr->shero += randint(25) + 25;
            break;

        case SV_POTION_CURE_LIGHT:
            if (cure_blindness()) ident = TRUE;
            if (hp_player(damroll(2, 8))) ident = TRUE;
            if (p_ptr->cut) ident = TRUE;
            if (p_ptr->cut > 10) {
                p_ptr->cut -= 10;
            }
            else {
                p_ptr->cut = 0;
            }
            break;

        case SV_POTION_CURE_SERIOUS:
            if (cure_blindness()) ident = TRUE;
            if (cure_confusion()) ident = TRUE;
            if (hp_player(damroll(4, 8))) ident = TRUE;
            if (p_ptr->cut) ident = TRUE;
            if (p_ptr->cut > 100) {
                p_ptr->cut = (p_ptr->cut / 2) - 50;
            }
            else {
                p_ptr->cut = 0;
            }
            break;

        case SV_POTION_CURE_CRITICAL:
            if (cure_blindness()) ident = TRUE;
            if (cure_confusion()) ident = TRUE;
            if (cure_poison()) ident = TRUE;
            if (hp_player(damroll(6, 8))) ident = TRUE;
            if (p_ptr->cut) ident = TRUE;
            if (p_ptr->stun) ident = TRUE;
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            break;

        case SV_POTION_HEALING:
            if (cure_blindness()) ident = TRUE;
            if (cure_confusion()) ident = TRUE;
            if (cure_poison()) ident = TRUE;
            if (hp_player(300)) ident = TRUE;
            if (p_ptr->cut) ident = TRUE;
            if (p_ptr->stun) ident = TRUE;
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            break;

        case SV_POTION_STAR_HEALING:
            if (hp_player(1200)) ident = TRUE;
            if (cure_blindness()) ident = TRUE;
            if (cure_confusion()) ident = TRUE;
            if (cure_poison()) ident = TRUE;
            if (p_ptr->cut) ident = TRUE;
            if (p_ptr->stun) ident = TRUE;
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            break;

        case SV_POTION_LIFE:
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
            p_ptr->image = 0;
            p_ptr->cut = 0;
            p_ptr->stun = 0;
            msg_print("You feel life flow through your body!");
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
            ident = TRUE;
            break;

        case SV_POTION_RESTORE_MANA:
            if (p_ptr->csp < p_ptr->msp) {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
                msg_print("Your feel your head clear.");
                p_ptr->redraw |= (PR_MANA);
                ident = TRUE;
            }
            break;
        
        case SV_POTION_RESTORE_EXP:
            if (restore_level()) ident = TRUE;
            break;

        case SV_POTION_RES_STR:
            if (res_stat(A_STR)) {
                msg_print("You feel warm all over.");
                ident = TRUE;
            }
            break;
        
        case SV_POTION_RES_INT:
            if (res_stat(A_INT)) {
                msg_print("You have a warm feeling.");
                ident = TRUE;
            }
            break;

        case SV_POTION_RES_WIS:
            if (res_stat(A_WIS)) {
                msg_print("You feel your wisdom returning.");
                ident = TRUE;
            }
            break;

        case SV_POTION_RES_DEX:
            if (res_stat(A_DEX)) {
                msg_print("You feel less clumsy.");
                ident = TRUE;
            }
            break;

        case SV_POTION_RES_CON:
            if (res_stat(A_CON)) {
                msg_print("You feel your health returning!");
                ident = TRUE;
            }
            break;

        case SV_POTION_RES_CHR:
            if (res_stat(A_CHR)) {
                msg_print("You feel your looks returning.");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_STR:
            if (inc_stat(A_STR)) {
                msg_print("Wow!  What bulging muscles!");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_INT:
            if (inc_stat(A_INT)) {
                msg_print("Aren't you brilliant!");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_WIS:
            if (inc_stat(A_WIS)) {
                msg_print("You suddenly have a profound thought!");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_DEX:
            if (inc_stat(A_DEX)) {
                msg_print("You feel more limber!");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_CON:
            if (inc_stat(A_CON)) {
                msg_print("You feel tingly for a moment.");
                ident = TRUE;
            }
            break;

        case SV_POTION_INC_CHR:
            if (inc_stat(A_CHR)) {
                msg_print("Gee, ain't you cute!");
                ident = TRUE;
            }
            break;

        case SV_POTION_AUGMENTATION:
            if (inc_stat(A_DEX)) ident = TRUE;
            if (inc_stat(A_WIS)) ident = TRUE;
            if (inc_stat(A_INT)) ident = TRUE;
            if (inc_stat(A_STR)) ident = TRUE;
            if (inc_stat(A_CHR)) ident = TRUE;
            if (inc_stat(A_CON)) ident = TRUE;
            if (ident) {
                msg_print("You feel power flow through your body!");
            }
            break;

        case SV_POTION_ENLIGHTENMENT:
            msg_print("An image of your surroundings forms in your mind...");
            wiz_lite();
            ident = TRUE;
            break;

        case SV_POTION_STAR_ENLIGHTENMENT:
            msg_print("You feel more enlightened!");
            msg_print(NULL);
            wiz_lite();
            (void)res_stat(A_WIS);
            (void)inc_stat(A_WIS);
            (void)res_stat(A_INT);
            (void)inc_stat(A_INT);
            self_knowledge();
            identify_pack();
            (void)detect_treasure();
            (void)detect_object();
            (void)detect_sdoor();
            (void)detect_trap();
            ident = TRUE;
            break;

        case SV_POTION_SELF_KNOWLEDGE:
            msg_print("You feel you know yourself a little better...");
            msg_print(NULL);
            self_knowledge();
            ident = TRUE;
            break;
        
        case SV_POTION_EXPERIENCE:
            if (p_ptr->exp < PY_MAX_EXP) {
                s32b ee = (p_ptr->exp / 2) + 10;
                if (ee > 100000L) ee = 100000L;
                msg_print("You feel more experienced.");
                gain_exp(ee);
                ident = TRUE;
            }
            break;

        default:
            msg_print("Oops.  Undefined potion.");
            break;
    }
    

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);
    
    /* The item has been tried */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        int lev = k_list[i_ptr->k_idx].level;
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
        inven_aware(i_ptr);
    }

    /* Potions can feed the player */
    add_food(i_ptr->pval);


    /* Destroy a potion on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_describe(oy, ox);
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
 * Curse the players armor
 */
static bool curse_armor(void)
{
    int t, k, tmp[6];

    inven_type *j_ptr;

    char i_name[80];
 
    
    /* Hack -- make a "list" of "armor" indexes, size is "k" */
    k = 0;
    if (inventory[INVEN_BODY].k_idx)  tmp[k++] = INVEN_BODY;
    if (inventory[INVEN_ARM].k_idx)   tmp[k++] = INVEN_ARM;
    if (inventory[INVEN_OUTER].k_idx) tmp[k++] = INVEN_OUTER;
    if (inventory[INVEN_HANDS].k_idx) tmp[k++] = INVEN_HANDS;
    if (inventory[INVEN_HEAD].k_idx)  tmp[k++] = INVEN_HEAD;
    if (inventory[INVEN_FEET].k_idx)  tmp[k++] = INVEN_FEET;

    /* Nothing to curse */
    if (!k) return (FALSE);

    /* Pick an item to curse */
    for (t = 0; t < 10; t++) {

        /* Pick a random item */
        int n = tmp[rand_int(k)];
        j_ptr = &inventory[n];

        /* Lock instantly on to "good" items */
        if (!cursed_p(j_ptr) && !broken_p(j_ptr)) break;
    }


    /* Describe */
    objdes(i_name, j_ptr, FALSE, 0);

    /* Attempt a saving throw for artifacts */
    if (artifact_p(j_ptr) && (rand_int(7) < 3)) {

        /* Cool */
        msg_format("A %s tries to surround your %s, but it resists the effects!",
                   "terrible black aura", i_name);
    }

    /* not artifact or failed save... */
    else {

        /* Oops */
        msg_format("A terrible black aura blasts your %s!", i_name);

        /* Blast the armor */
        j_ptr->name1 = 0;
        j_ptr->name2 = EGO_BLASTED;
        j_ptr->flags3 = TR3_CURSED;
        j_ptr->flags2 = 0L;
        j_ptr->flags1 = 0L;
        j_ptr->toac = 0 - randint(5) - randint(5);
        j_ptr->tohit = j_ptr->todam = 0;
        j_ptr->ac = (j_ptr->ac > 9) ? 1 : 0;

        /* Curse it */
        j_ptr->ident |= ID_CURSED;

        /* Break it */
        j_ptr->ident |= ID_BROKEN;
                
        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redraw choice window */
        p_ptr->redraw |= (PR_CHOICE);
    }

    return (TRUE);
}


/*
 * Curse the players weapon
 */
static bool curse_weapon(void)
{
    inven_type *j_ptr;

    char i_name[80];
 
    
    /* Normally curse the weapon */
    j_ptr = &inventory[INVEN_WIELD];

    /* But try the bow if necessary */
    if (!j_ptr->k_idx) j_ptr = &inventory[INVEN_BOW];

    /* Nothing to curse */
    if (!j_ptr->k_idx) return (FALSE);


    /* Describe */
    objdes(i_name, j_ptr, FALSE, 0);

    /* Attempt a saving throw */
    if (artifact_p(j_ptr) && (randint(7) < 4)) {

        /* Cool */
        msg_format("A %s tries to %s, but your %s resists the effects!",
                   "terrible black aura", "surround your weapon", i_name);
    }

    /* not artifact or failed save... */
    else {

        /* Oops */
        msg_format("A terrible black aura blasts your %s!", i_name);

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

        /* Curse it */
        j_ptr->ident |= ID_CURSED;
        
        /* Break it */
        j_ptr->ident |= ID_BROKEN;

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redraw choice window */
        p_ptr->redraw |= (PR_CHOICE);
    }

    /* Notice */
    return (TRUE);
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
    int			oy, ox, item, k;
    int			used_up, ident;

    inven_type		*i_ptr;


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

    /* Restrict choices to scrolls */
    item_tester_tval = TV_SCROLL;

    /* Get a scroll */
    if (!get_item(&item, "Read which scroll? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have no scrolls to read.");
        return;
    }

    /* Get the item (if it is in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* The turn is not free */
    energy_use = 100;

    /* Assume the scroll will get used up */
    used_up = TRUE;

    /* Not identified yet */
    ident = FALSE;

    /* Analyze the scroll */
    switch (i_ptr->sval) {

        case SV_SCROLL_DARKNESS:
            if (unlite_area(10, 3)) ident = TRUE;
            add_blind(3 + randint(5));
            break;

        case SV_SCROLL_AGGRAVATE_MONSTER:
            msg_print("There is a high pitched humming noise.");
            aggravate_monsters(1);
            ident = TRUE;
            break;

        case SV_SCROLL_CURSE_ARMOR:
            if (curse_armor()) ident = TRUE;
            break;
            
        case SV_SCROLL_CURSE_WEAPON:
            if (curse_weapon()) ident = TRUE;
            break;
        
        case SV_SCROLL_SUMMON_MONSTER:
            for (k = 0; k < randint(3); k++) {
                if (summon_monster(py, px, dun_level + MON_SUMMON_ADJ)) {
                    ident = TRUE;
                }
            }
            break;

        case SV_SCROLL_SUMMON_UNDEAD:
            for (k = 0; k < randint(3); k++) {
                if (summon_specific(py, px, dun_level, SUMMON_UNDEAD)) {
                    ident = TRUE;
                }
            }
            break;

        case SV_SCROLL_TRAP_CREATION:
            if (trap_creation()) ident = TRUE;
            break;

        case SV_SCROLL_PHASE_DOOR:
            teleport_flag = TRUE;
            teleport_dist = 10;
            ident = TRUE;
            break;

        case SV_SCROLL_TELEPORT:
            teleport_flag = TRUE;
            teleport_dist = 100;
            ident = TRUE;
            break;

        case SV_SCROLL_TELEPORT_LEVEL:
            (void)tele_level();
            ident = TRUE;
            break;

        case SV_SCROLL_WORD_OF_RECALL:
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

        case SV_SCROLL_IDENTIFY:
            msg_print("This is an identify scroll.");
            ident = TRUE;
            if (!ident_spell()) used_up = FALSE;
            break;

        case SV_SCROLL_STAR_IDENTIFY:
            msg_print("This is an *identify* scroll.");
            ident = TRUE;
            if (!identify_fully()) used_up = FALSE;
            break;

        case SV_SCROLL_REMOVE_CURSE:
            if (remove_curse()) {
                msg_print("You feel as if someone is watching over you.");
                ident = TRUE;
            }
            break;

        case SV_SCROLL_STAR_REMOVE_CURSE:
            remove_all_curse();
            ident = TRUE;
            break;

        case SV_SCROLL_ENCHANT_ARMOR:
            msg_print("This is a scroll of enchant armor.");
            ident = TRUE;
            if (!enchant_spell(0, 0, 1)) used_up = FALSE;
            break;

        case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
            msg_print("This is a scroll of enchant weapon to-hit.");
            if (!enchant_spell(1, 0, 0)) used_up = FALSE;
            ident = TRUE;
            break;

        case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
            msg_print("This is a scroll of enchant weapon to-dam.");
            if (!enchant_spell(0, 1, 0)) used_up = FALSE;
            ident = TRUE;
            break;

        case SV_SCROLL_STAR_ENCHANT_ARMOR:
            msg_print("This is a scroll of *enchant* weapon.");
            if (!enchant_spell(randint(3), randint(3), 0)) used_up = FALSE;
            ident = TRUE;
            break;

        case SV_SCROLL_STAR_ENCHANT_WEAPON:
            msg_print("This is a scroll of *enchant* armor.");
            if (!enchant_spell(0, 0, randint(3) + 2)) used_up = FALSE;
            ident = TRUE;
            break;

        case SV_SCROLL_RECHARGING:
            msg_print("This is a Recharge-Item scroll.");
            /* Hack -- Like identify, recharge can be cancelled */
            used_up = recharge(60);
            ident = TRUE;
            break;
        
        case SV_SCROLL_LIGHT:
            if (lite_area(damroll(2, 8), 2)) ident = TRUE;
            break;

        case SV_SCROLL_MAPPING:
            map_area();
            ident = TRUE;
            break;

        case SV_SCROLL_DETECT_GOLD:
            if (detect_treasure()) ident = TRUE;
            break;

        case SV_SCROLL_DETECT_ITEM:
            if (detect_object()) ident = TRUE;
            break;

        case SV_SCROLL_DETECT_TRAP:
            if (detect_trap()) ident = TRUE;
            break;

        case SV_SCROLL_DETECT_DOOR:
            if (detect_sdoor()) ident = TRUE;
            break;

        case SV_SCROLL_DETECT_INVIS:
            if (detect_invisible()) ident = TRUE;
            break;

        case SV_SCROLL_SATISFY_HUNGER:
            satisfy_hunger();
            ident = TRUE;
            break;

        case SV_SCROLL_BLESSING:
            add_bless(randint(12) + 6);
            ident = TRUE;
            break;

        case SV_SCROLL_HOLY_CHANT:
            add_bless(randint(24) + 12);
            ident = TRUE;
            break;

        case SV_SCROLL_HOLY_PRAYER:
            add_bless(randint(48) + 24);
            ident = TRUE;
            break;
        
        case SV_SCROLL_MONSTER_CONFUSION:
            if (p_ptr->confusing == 0) {
                msg_print("Your hands begin to glow.");
                p_ptr->confusing = TRUE;
                ident = TRUE;
            }
            break;

        case SV_SCROLL_PROTECTION_FROM_EVIL:
            if (protect_evil()) ident = TRUE;
            break;
        
        case SV_SCROLL_RUNE_OF_PROTECTION:
            warding_glyph();
            ident = TRUE;
            break;

        case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
            if (destroy_doors_touch()) ident = TRUE;
            break;

        case SV_SCROLL_STAR_DESTRUCTION:
            destroy_area(py, px, 15, TRUE);
            ident = TRUE;
            break;

        case SV_SCROLL_DISPEL_UNDEAD:
            if (dispel_undead(60)) ident = TRUE;
            break;

        case SV_SCROLL_GENOCIDE:
            msg_print("This is a genocide scroll.");
            (void)genocide(TRUE);
            ident = TRUE;
            break;

        case SV_SCROLL_MASS_GENOCIDE:
            msg_print("This is a mass genocide scroll.");
            mass_genocide(TRUE);
            ident = TRUE;
            break;

        case SV_SCROLL_ACQUIREMENT:
            acquirement(py, px, 1, TRUE);
            ident = TRUE;
            break;

        case SV_SCROLL_STAR_ACQUIREMENT:
            acquirement(py, px, randint(2) + 1, TRUE);
            ident = TRUE;
            break;

        default:
            msg_print("Oops.  Undefined scroll.");
            break;
    }
    

    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);
    
    /* The item was tried */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        int lev = k_list[i_ptr->k_idx].level;
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
        inven_aware(i_ptr);
    }


    /* Hack -- allow certain scrolls to be "preserved" */
    if (!used_up) return;


    /* Destroy a scroll on the floor */
    if (item < 0) {
        floor_item_increase(oy, ox, -1);
        floor_item_describe(oy, ox);
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

    inven_type		*i_ptr;

    /* Hack -- let staffs of identify get aborted */
    bool use_charge = TRUE;


    /* Assume free turn */
    energy_use = 0;


    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Restrict choices to wands */
    item_tester_tval = TV_STAFF;

    /* Get an item */
    if (!get_item(&item, "Use which staff? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have no staff to use.");
        return;
    }

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


    /* Take a turn */
    energy_use = 100;

    /* Extract the item level */
    lev = k_list[i_ptr->k_idx].level;

    /* Base chance of success */
    chance = p_ptr->skill_dev;
    
    /* Confusion hurts skill */
    if (p_ptr->confused) chance = chance / 2;

    /* Hight level objects are harder */
    chance = chance - ((lev > 50) ? 50 : lev);

    /* Give everyone a (slight) chance */
    if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0)) {
        chance = USE_DEVICE;
    }

    /* Roll for usage */
    if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE)) {
        if (flush_failure) flush();
        msg_print("You failed to use the staff properly.");
        return;
    }

    /* Notice empty staffs */
    if (i_ptr->pval <= 0) {
        if (flush_failure) flush();
        msg_print("The staff has no charges left.");
        i_ptr->ident |= ID_EMPTY;
        return;
    }


    /* Not identified yet */
    ident = FALSE;

    /* Analyze the staff */
    switch (i_ptr->sval) {

      case SV_STAFF_DARKNESS:
        if (unlite_area(10, 3)) ident = TRUE;
        break;

      case SV_STAFF_SLOWNESS:
        if (p_ptr->slow == 0) ident = TRUE;
        add_slow(randint(30) + 15);
        break;

      case SV_STAFF_HASTE_MONSTERS:
        if (speed_monsters()) ident = TRUE;
        break;

      case SV_STAFF_SUMMONING:
        for (k = 0; k < randint(4); k++) {
            if (summon_monster(py, px, dun_level + MON_SUMMON_ADJ)) {
                ident = TRUE;
            }
        }
        break;

      case SV_STAFF_TELEPORTATION:
        teleport_flag = TRUE;
        teleport_dist = 100;
        ident = TRUE;
        break;

      case SV_STAFF_IDENTIFY:
        if (!ident_spell()) use_charge = FALSE;
        ident = TRUE;
        break;

      case SV_STAFF_REMOVE_CURSE:
        if (remove_curse()) {
            if (!p_ptr->blind) {
                msg_print("The staff glows blue for a moment..");
            }
            ident = TRUE;
        }
        break;

      case SV_STAFF_STARLITE:
        if (!p_ptr->blind) {
            msg_print("The end of the staff bursts into a blue shimmering light.");
        }
        for (k = 0; k < 8; k++) lite_line(ddd[k]);
        ident = TRUE;
        break;

      case SV_STAFF_LITE:
        if (lite_area(damroll(2, 8), 2)) ident = TRUE;
        break;

      case SV_STAFF_MAPPING:
        map_area();
        ident = TRUE;
        break;

      case SV_STAFF_DETECT_GOLD:
        if (detect_treasure()) ident = TRUE;
        break;

      case SV_STAFF_DETECT_ITEM:
        if (detect_object()) ident = TRUE;
        break;

      case SV_STAFF_DETECT_TRAP:
        if (detect_trap()) ident = TRUE;
        break;

      case SV_STAFF_DETECT_DOOR:
        if (detect_sdoor()) ident = TRUE;
        break;

      case SV_STAFF_DETECT_INVIS:
        if (detect_invisible()) ident = TRUE;
        break;

      case SV_STAFF_DETECT_EVIL:
        if (detect_evil()) ident = TRUE;
        break;

      case SV_STAFF_CURE_LIGHT:
        if (hp_player(randint(8))) ident = TRUE;
        break;

      case SV_STAFF_CURING:
        if (cure_blindness()) ident = TRUE;
        if (cure_poison()) ident = TRUE;
        if (cure_confusion()) ident = TRUE;
        if (p_ptr->cut) ident = TRUE;
        if (p_ptr->stun) ident = TRUE;
        p_ptr->cut = 0;
        p_ptr->stun = 0;
        break;

      case SV_STAFF_HEALING:
        if (hp_player(300)) ident = TRUE;
        if (p_ptr->cut) ident = TRUE;
        if (p_ptr->stun) ident = TRUE;
        p_ptr->cut = 0;
        p_ptr->stun = 0;
        break;

      case SV_STAFF_THE_MAGI:
        if (res_stat(A_INT)) {
            msg_print("You have a warm feeling.");
            ident = TRUE;
        }
        if (p_ptr->csp < p_ptr->msp) {
            p_ptr->csp = p_ptr->msp;
            ident = TRUE;
            msg_print("Your feel your head clear.");
            p_ptr->redraw |= (PR_MANA);
        }
        break;

      case SV_STAFF_SLEEP_MONSTERS:
        if (sleep_monsters()) ident = TRUE;
        break;

      case SV_STAFF_SLOW_MONSTERS:
        if (slow_monsters()) ident = TRUE;
        break;

      case SV_STAFF_SPEED:
        if (!p_ptr->fast) {
            add_fast(randint(30) + 15);
            ident = TRUE;
        }
        break;

      case SV_STAFF_PROBING:
        probing();
        ident = TRUE;
        break;

      case SV_STAFF_DISPEL_EVIL:
        if (dispel_evil(60)) ident = TRUE;
        break;

      case SV_STAFF_POWER:
        if (dispel_monsters(120)) ident = TRUE;
        break;

      case SV_STAFF_HOLINESS:
        dispel_evil(120);
        protect_evil();
        cure_poison();
        cure_fear();
        hp_player(50);
        p_ptr->cut = 0;
        p_ptr->stun = 0;
        ident = TRUE;
        break;

      case SV_STAFF_GENOCIDE:
        (void)genocide(TRUE);
        ident = TRUE;
        break;

      case SV_STAFF_EARTHQUAKES:
        earthquake(py, px, 10);
        ident = TRUE;
        break;

      case SV_STAFF_DESTRUCTION:
        destroy_area(py, px, 15, TRUE);
        ident = TRUE;
        break;


      default:
        msg_print("Oops.  Undefined staff.");
        break;
    }


    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);

    /* Tried the item */
    inven_tried(i_ptr);

    /* An identification was made */
    if (ident && !inven_aware_p(i_ptr)) {
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
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

        /* Message */
        msg_print("You unstack your staff.");
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

    inven_type		*i_ptr;


    /* Assume free turn */
    energy_use = 0;


    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Restrict choices to wands */
    item_tester_tval = TV_WAND;

    /* Get an item */
    if (!get_item(&item, "Aim which wand? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have no wand to aim.");
        return;
    }

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


    /* Use a turn */
    energy_use = 100;
    
    /* Get the level */
    lev = k_list[i_ptr->k_idx].level;

    /* Base chance of success */
    chance = p_ptr->skill_dev;
    
    /* Confusion hurts skill */
    if (p_ptr->confused) chance = chance / 2;

    /* Hight level objects are harder */
    chance = chance - ((lev > 50) ? 50 : lev);

    /* Give everyone a (slight) chance */
    if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0)) {
        chance = USE_DEVICE;
    }

    /* Roll for usage */
    if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE)) {
        if (flush_failure) flush();
        msg_print("You failed to use the wand properly.");
        return;
    }

    /* The wand is already empty! */
    if (i_ptr->pval <= 0) {
        if (flush_failure) flush();
        msg_print("The wand has no charges left.");
        i_ptr->ident |= ID_EMPTY;
        return;
    }



    /* Not identified yet */
    ident = FALSE;

    /* XXX Hack -- Extract the "sval" effect */
    sval = i_ptr->sval;

    /* XXX Hack -- Wand of wonder can do anything before it */
    if (sval == SV_WAND_WONDER) sval = rand_int(SV_WAND_WONDER);

    /* Analyze the wand */
    switch (sval) {

        case SV_WAND_HEAL_MONSTER:
            if (heal_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_HASTE_MONSTER:
            if (speed_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_CLONE_MONSTER:
            if (clone_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_TELEPORT_AWAY:
            if (teleport_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_DISARMING:
            if (disarm_trap(dir)) ident = TRUE;
            break;

        case SV_WAND_TRAP_DOOR_DEST:
            if (destroy_door(dir)) ident = TRUE;
            break;

        case SV_WAND_STONE_TO_MUD:
            if (wall_to_mud(dir)) ident = TRUE;
            break;

        case SV_WAND_LITE:
            msg_print("A line of blue shimmering light appears.");
            lite_line(dir);
            ident = TRUE;
            break;

        case SV_WAND_SLEEP_MONSTER:
            if (sleep_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_SLOW_MONSTER:
            if (slow_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_CONFUSE_MONSTER:
            if (confuse_monster(dir, 10)) ident = TRUE;
            break;

        case SV_WAND_FEAR_MONSTER:
            if (fear_monster(dir, 10)) ident = TRUE;
            break;

        case SV_WAND_DRAIN_LIFE:
            if (drain_life(dir, 75)) ident = TRUE;
            break;

        case SV_WAND_POLYMORPH:
            if (poly_monster(dir)) ident = TRUE;
            break;

        case SV_WAND_STINKING_CLOUD:
            fire_ball(GF_POIS, dir, 12, 2);
            ident = TRUE;
            break;

        case SV_WAND_MAGIC_MISSILE:
            fire_bolt_or_beam(20, GF_MISSILE, dir, damroll(2,6));
            ident = TRUE;
            break;

        case SV_WAND_ACID_BOLT:
            fire_bolt_or_beam(20, GF_ACID, dir, damroll(5,8));
            ident = TRUE;
            break;

        case SV_WAND_ELEC_BOLT:
            fire_bolt_or_beam(20, GF_ELEC, dir, damroll(3,8));
            ident = TRUE;
            break;

        case SV_WAND_FIRE_BOLT:
            fire_bolt_or_beam(20, GF_FIRE, dir, damroll(6,8));
            ident = TRUE;
            break;

        case SV_WAND_COLD_BOLT:
            fire_bolt_or_beam(20, GF_COLD, dir, damroll(3,8));
            ident = TRUE;
            break;

        case SV_WAND_ACID_BALL:
            fire_ball(GF_ACID, dir, 60, 2);
            ident = TRUE;
            break;

        case SV_WAND_ELEC_BALL:
            fire_ball(GF_ELEC, dir, 32, 2);
            ident = TRUE;
            break;

        case SV_WAND_FIRE_BALL:
            fire_ball(GF_FIRE, dir, 72, 2);
            ident = TRUE;
            break;

        case SV_WAND_COLD_BALL:
            fire_ball(GF_COLD, dir, 48, 2);
            ident = TRUE;
            break;

        case SV_WAND_WONDER:
            msg_print("Oops.  Wand of wonder activated.");
            break;

        case SV_WAND_DRAGON_FIRE:
            fire_ball(GF_FIRE, dir, 100, 3);
            ident = TRUE;
            break;

        case SV_WAND_DRAGON_COLD:
            fire_ball(GF_COLD, dir, 80, 3);
            ident = TRUE;
            break;

        case SV_WAND_DRAGON_BREATH:
            switch (randint(5)) {
              case 1:
                fire_ball(GF_ACID, dir, 100, 3);
                break;
              case 2:
                fire_ball(GF_ELEC, dir, 80, 3);
                break;
              case 3:
                fire_ball(GF_FIRE, dir, 100, 3);
                break;
              case 4:
                fire_ball(GF_COLD, dir, 80, 3);
                break;
              default:
                fire_ball(GF_POIS, dir, 60, 3);
                break;
            }
            ident = TRUE;
            break;

        case SV_WAND_ANNIHILATION:
            if (drain_life(dir, 125)) ident = TRUE;
            break;

        default:
            msg_print("Oops.  Undefined wand effect.");
            break;
    }


    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);

    /* Mark it as tried */
    inven_tried(i_ptr);

    /* Apply identification */
    if (ident && !inven_aware_p(i_ptr)) {
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
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

        /* Message */
        msg_print("You unstack your wand.");
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

    inven_type		*i_ptr;

    /* Hack -- let perception get aborted */
    bool use_charge = TRUE;


    /* Assume free turn */
    energy_use = 0;


    /* Save the player location */
    oy = py; ox = px;

    /* Assume we will use the item on the floor */
    i_ptr = &i_list[cave[oy][ox].i_idx];

    /* Restrict choices to rods */
    item_tester_tval = TV_ROD;

    /* Get an item */
    if (!get_item(&item, "Zap which rod? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have no rod to zap.");
        return;
    }

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

    /* Get a direction (unless KNOWN not to need it) */
    if ((i_ptr->sval >= SV_ROD_MIN_DIRECTION) || !inven_aware_p(i_ptr)) {

        /* Get a direction, allow cancel */
        if (!get_dir_c(NULL, &dir)) return;
    }


    /* Use a full turn */
    energy_use = 100;

    /* Extract the item level */
    lev = k_list[i_ptr->k_idx].level;

    /* Base chance of success */
    chance = p_ptr->skill_dev;
    
    /* Confusion hurts skill */
    if (p_ptr->confused) chance = chance / 2;

    /* Hight level objects are harder */
    chance = chance - ((lev > 50) ? 50 : lev);

    /* Give everyone a (slight) chance */
    if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0)) {
        chance = USE_DEVICE;
    }

    /* Roll for usage */
    if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE)) {
        if (flush_failure) flush();
        msg_print("You failed to use the rod properly.");
        return;
    }

    /* Still charging */
    if (i_ptr->pval) {
        if (flush_failure) flush();
        msg_print("The rod is still charging.");
        return;
    }


    /* Not identified yet */
    ident = FALSE;

    /* Analyze the rod */
    switch (i_ptr->sval) {

      case SV_ROD_DETECT_TRAP:
        if (detect_trap()) ident = TRUE;
        i_ptr->pval = 50;
        break;

      case SV_ROD_DETECT_DOOR:
        if (detect_sdoor()) ident = TRUE;
        i_ptr->pval = 70;
        break;

      case SV_ROD_IDENTIFY:
        ident = TRUE;
        if (!ident_spell()) use_charge = FALSE;
        i_ptr->pval = 10;
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

      case SV_ROD_ILLUMINATION:
        lite_area(damroll(2, 8), 2);
        ident = TRUE;
        i_ptr->pval = 30;
        break;

      case SV_ROD_MAPPING:
        map_area();
        ident = TRUE;
        i_ptr->pval = 99;
        break;

      case SV_ROD_DETECTION:
        detection();
        ident = TRUE;
        i_ptr->pval = 99;
        break;

      case SV_ROD_PROBING:
        probing();
        ident = TRUE;
        i_ptr->pval = 50;
        break;

      case SV_ROD_CURING:
        if (cure_blindness()) ident = TRUE;
        if (cure_poison()) ident = TRUE;
        if (cure_confusion()) ident = TRUE;
        if (p_ptr->cut) ident = TRUE;
        if (p_ptr->stun) ident = TRUE;
        p_ptr->cut = 0;
        p_ptr->stun = 0;
        i_ptr->pval = 999;
        break;

      case SV_ROD_HEALING:
        if (hp_player(500)) ident = TRUE;
        if (p_ptr->cut) ident = TRUE;
        if (p_ptr->stun) ident = TRUE;
        p_ptr->cut = 0;
        p_ptr->stun = 0;
        i_ptr->pval = 999;
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
        if (!p_ptr->fast) {
            add_fast(randint(30) + 15);
            ident = TRUE;
        }
        i_ptr->pval = 99;
        break;

      case SV_ROD_TELEPORT_AWAY:
        if (teleport_monster(dir)) ident = TRUE;
        i_ptr->pval = 25;
        break;

      case SV_ROD_DISARMING:
        if (disarm_trap(dir)) ident = TRUE;
        i_ptr->pval = 30;
        break;

      case SV_ROD_LITE:
        msg_print("A line of blue shimmering light appears.");
        lite_line(dir);
        ident = TRUE;
        i_ptr->pval = 9;
        break;

      case SV_ROD_SLEEP_MONSTER:
        if (sleep_monster(dir)) ident = TRUE;
        i_ptr->pval = 18;
        break;

      case SV_ROD_SLOW_MONSTER:
        if (slow_monster(dir)) ident = TRUE;
        i_ptr->pval = 20;
        break;

      case SV_ROD_DRAIN_LIFE:
        if (drain_life(dir, 75)) ident = TRUE;
        i_ptr->pval = 23;
        break;

      case SV_ROD_POLYMORPH:
        if (poly_monster(dir)) ident = TRUE;
        i_ptr->pval = 25;
        break;

      case SV_ROD_ACID_BOLT:
        fire_bolt_or_beam(10, GF_ACID, dir, damroll(6,8));
        ident = TRUE;
        i_ptr->pval = 12;
        break;

      case SV_ROD_ELEC_BOLT:
        fire_bolt_or_beam(10, GF_ELEC, dir, damroll(3,8));
        ident = TRUE;
        i_ptr->pval = 11;
        break;

      case SV_ROD_FIRE_BOLT:
        fire_bolt_or_beam(10, GF_FIRE, dir, damroll(8,8));
        ident = TRUE;
        i_ptr->pval = 15;
        break;

      case SV_ROD_COLD_BOLT:
        fire_bolt_or_beam(10, GF_COLD, dir, damroll(5,8));
        ident = TRUE;
        i_ptr->pval = 13;
        break;

      case SV_ROD_ACID_BALL:
        fire_ball(GF_ACID, dir, 60, 2);
        ident = TRUE;
        i_ptr->pval = 27;
        break;

      case SV_ROD_ELEC_BALL:
        fire_ball(GF_ELEC, dir, 32, 2);
        ident = TRUE;
        i_ptr->pval = 23;
        break;

      case SV_ROD_FIRE_BALL:
        fire_ball(GF_FIRE, dir, 72, 2);
        ident = TRUE;
        i_ptr->pval = 30;
        break;

      case SV_ROD_COLD_BALL:
        fire_ball(GF_COLD, dir, 48, 2);
        ident = TRUE;
        i_ptr->pval = 25;
        break;

      default:
        msg_print("Oops.  Undefined rod.");
        break;
    }


    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOICE);

    /* Tried the object */
    inven_tried(i_ptr);

    /* Successfully determined the object function */
    if (ident && !inven_aware_p(i_ptr)) {
        gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
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

        /* Message */
        msg_print("You unstack your rod.");
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
 * Hack -- activate the ring of power
 */
static void ring_of_power(int dir)
{
    /* Pick a random effect */
    switch (randint(10)) {
                
        case 1:
        case 2:
            msg_print("You are surrounded by a malignant aura");

            /* Decrease all stats (permanently) */
            dec_stat(A_STR, 50, TRUE);
            dec_stat(A_INT, 50, TRUE);
            dec_stat(A_WIS, 50, TRUE);
            dec_stat(A_DEX, 50, TRUE);
            dec_stat(A_CON, 50, TRUE);
            dec_stat(A_CHR, 50, TRUE);

            /* Lose some experience (permanently) */
            p_ptr->exp -= (p_ptr->exp / 4);
            p_ptr->max_exp -= (p_ptr->exp / 4);
            check_experience();

            /* Update/Redraw stuff */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
            p_ptr->redraw |= (PR_STATS);
            break;

        case 3:
            dispel_monsters(1000);
            break;

        case 4:
        case 5:
        case 6:
            /* Mana Ball */
            fire_ball(GF_MANA, dir, 300, 3);
            break;

        default:
            /* Mana Bolt */
            fire_bolt(GF_MANA, dir, 250);
     }
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
    int         i1, i2, i, lev;
    int         a, dir, chance, what;
    inven_type  *i_ptr;


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
    if (!get_item(&what, "Activate which item? ", i1, i2, FALSE)) {
        if (what == -2) msg_print("You have nothing to activate.");
        return;
    }
    
    /* Get the item */
    i_ptr = &inventory[what];


    /* Take a turn */
    energy_use = 100;
    
    /* Extract the item level */
    lev = k_list[i_ptr->k_idx].level;

    /* Hack -- use artifact level instead */
    if (artifact_p(i_ptr)) lev = v_list[i_ptr->name1].level;
    
    /* Base chance of success */
    chance = p_ptr->skill_dev;
    
    /* Confusion hurts skill */
    if (p_ptr->confused) chance = chance / 2;

    /* Hight level objects are harder */
    chance = chance - ((lev > 50) ? 50 : lev);

    /* Give everyone a (slight) chance */
    if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0)) {
        chance = USE_DEVICE;
    }

    /* Roll for usage */
    if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE)) {
        if (flush_failure) flush();
        msg_print("You failed to activate it properly.");
        return;
    }

    /* Check the recharge */
    if (i_ptr->timeout) {
        msg_print("It whines, glows and fades...");
        return;
    }


    /* Wonder Twin Powers... Activate! */
    msg_print("You activate it...");


    /* Mega-Hack -- allow free cancel */
    energy_use = 0;
    
    /* Artifacts activate by name */
    if (i_ptr->name1) {

        /* This needs to be changed */
        switch (i_ptr->name1) {

            case ART_NARTHANC:
                msg_print("Your dagger is covered in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_FIRE, dir, damroll(9, 8));
                i_ptr->timeout = rand_int(8) + 8;
                break;

            case ART_NIMTHANC:
                msg_print("Your dagger is covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_COLD, dir, damroll(6, 8));
                i_ptr->timeout = rand_int(7) + 7;
                break;

            case ART_DETHANC:
                msg_print("Your dagger is covered in sparks...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ELEC, dir, damroll(4, 8));
                i_ptr->timeout = rand_int(6) + 6;
                break;

            case ART_RILIA:
                msg_print("Your dagger throbs deep green...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_POIS, dir, 12, 3);
                i_ptr->timeout = rand_int(4) + 4;
                break;

            case ART_BELANGIL:
                msg_print("Your dagger is covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, 48, 2);
                i_ptr->timeout = rand_int(5) + 5;
                break;

            case ART_DAL:
                msg_print("You feel energy flow through your feet...");
                cure_fear();
                cure_poison();
                i_ptr->timeout = 5;
                break;

            case ART_RINGIL:
                msg_print("Your sword glows an intense blue...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, 100, 2);
                i_ptr->timeout = 300;
                break;

            case ART_ANDURIL:
                msg_print("Your sword glows an intense red...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, 72, 2);
                i_ptr->timeout = 400;
                break;

            case ART_FIRESTAR:
                msg_print("Your morningstar rages in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, 72, 3);
                i_ptr->timeout = 100;
                break;

            case ART_FEANOR:
                if (!p_ptr->fast) {
                    add_fast(randint(20) + 20);
                }
                i_ptr->timeout = 200;
                break;

            case ART_THEODEN:
                msg_print("The blade of your axe glows black...");
                if (!get_dir_c(NULL, &dir)) return;
                drain_life(dir, 120);
                i_ptr->timeout = 400;
                break;

            case ART_TURMIL:
                msg_print("The head of your hammer glows white...");
                if (!get_dir_c(NULL, &dir)) return;
                drain_life(dir, 90);
                i_ptr->timeout = 70;
                break;

            case ART_CASPANION:
                msg_print("Your armor glows bright red...");
                destroy_doors_touch();
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
                if (!p_ptr->fast) {
                    add_fast(randint(20) + 20);
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
                hp_player(damroll(4, 8));
                if (p_ptr->cut > 100) {
                    p_ptr->cut = (p_ptr->cut / 2) - 50;
                }
                else {
                    p_ptr->cut = 0;
                }
                i_ptr->timeout = rand_int(3) + 3;
                break;

            case ART_CUBRAGOL:

                /* Use the first (XXX) acceptable bolts */
                for (a = 0; a < inven_ctr; a++) {
                    inven_type *j_ptr = &inventory[a];
                    if ((j_ptr->tval == TV_BOLT) &&
                        (!artifact_p(j_ptr)) && (!ego_item_p(j_ptr)) &&
                        (!cursed_p(j_ptr) && !broken_p(j_ptr))) break;
                }

                /* Enchant the bolts (or fail) */
                if ((a < inven_ctr) && (rand_int(4) == 0)) {
                    inven_type *j_ptr = &inventory[a];
                    msg_print("Your bolts are covered in a fiery aura!");
                    j_ptr->name2 = EGO_AMMO_FIRE;
                    j_ptr->flags1 |= (TR1_BRAND_FIRE);
                    j_ptr->flags3 |= (TR3_IGNORE_FIRE);
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
                fire_bolt(GF_COLD, dir, damroll(12, 8));
                i_ptr->timeout = 500;
                break;

            case ART_AEGLOS:
                msg_print("Your spear glows a bright white...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, 100, 2);
                i_ptr->timeout = 500;
                break;

            case ART_OROME:
                msg_print("Your spear pulsates...");
                if (!get_dir_c(NULL, &dir)) return;
                wall_to_mud(dir);
                i_ptr->timeout = 5;
                break;

            case ART_SOULKEEPER:
                msg_print("Your armor glows a bright white...");
                msg_print("You feel much better...");
                hp_player(1000);
                p_ptr->cut = 0;
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
                teleport_monster(dir);
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
                sleep_monsters_touch();
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
                confuse_monster(dir, 20);
                i_ptr->timeout = 15;
                break;

            case ART_CAMMITHRIM:
                msg_print("Your gloves glow extremely brightly...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_MISSILE, dir, damroll(2, 6));
                i_ptr->timeout = 2;
                break;

            case ART_PAURHACH:
                msg_print("Your gauntlets are covered in fire...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_FIRE, dir, damroll(9,8));
                i_ptr->timeout = rand_int(8) + 8;
                break;

            case ART_PAURNIMMEN:
                msg_print("Your gauntlets are covered in frost...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_COLD, dir, damroll(6, 8));
                i_ptr->timeout = rand_int(7) + 7;
                break;

            case ART_PAURAEGEN:
                msg_print("Your gauntlets are covered in sparks...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ELEC, dir, damroll(4, 8));
                i_ptr->timeout = rand_int(6) + 6;
                break;

            case ART_PAURNEN:
                msg_print("Your gauntlets look very acidic...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ACID, dir, damroll(5, 8));
                i_ptr->timeout = rand_int(5) + 5;
                break;

            case ART_FINGOLFIN:
                msg_print("Magical spikes appear on your cesti...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_bolt(GF_ARROW, dir, 150);
                i_ptr->timeout = rand_int(90) + 90;
                break;

            case ART_HOLHENNETH:
                msg_print("An image forms in your mind...");
                detection();
                i_ptr->timeout = rand_int(55) + 55;
                break;

            case ART_GONDOR:
                msg_print("You feel a warm tingling inside...");
                hp_player(500);
                p_ptr->cut = 0;
                i_ptr->timeout = 500;
                break;

            case ART_RAZORBACK:
                msg_print("You are surrounded by lightning!");
                for (i = 0; i < 8; i++) fire_ball(GF_ELEC, ddd[i], 150, 3);
                i_ptr->timeout = 1000;
                break;

            case ART_BLADETURNER:
                msg_print("Your armor glows many colours...");
                msg_print("You enter a berserk rage...");
                msg_print("You feel you can resist anything...");
                hp_player(30);	/* XXX */
                p_ptr->shero += randint(50) + 50;
                add_bless(randint(50) + 50);
                p_ptr->oppose_fire += randint(50) + 50;
                p_ptr->oppose_cold += randint(50) + 50;
                p_ptr->oppose_elec += randint(50) + 50;
                p_ptr->oppose_acid += randint(50) + 50;
                p_ptr->oppose_pois += randint(50) + 50;
                i_ptr->timeout = 400;
                break;


            case ART_GALADRIEL:
                msg_print("The phial wells with clear light...");
                lite_area(damroll(2, 15), 3);
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
                dispel_evil(p_ptr->lev * 5);
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
                if (!p_ptr->fast) {
                    add_fast(randint(75) + 75);
                }
                i_ptr->timeout = rand_int(150) + 150;
                break;

            case ART_NARYA:
                msg_print("The ring glows deep red...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_FIRE, dir, 120, 3);
                i_ptr->timeout = rand_int(225) + 225;
                break;

            case ART_NENYA:
                msg_print("The ring glows bright white...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_COLD, dir, 200, 3);
                i_ptr->timeout = rand_int(325) + 325;
                break;

            case ART_VILYA:
                msg_print("The ring glows deep blue...");
                if (!get_dir_c(NULL, &dir)) return;
                fire_ball(GF_ELEC, dir, 250, 3);
                i_ptr->timeout = rand_int(425) + 425;
                break;

            case ART_POWER:
                msg_print("The ring glows intensely black...");
                if (!get_dir_c(NULL, &dir)) return;
                ring_of_power(dir);
                i_ptr->timeout = rand_int(450) + 450;
                break;


            default:
                msg_print("Oops.  Non-Activatable Artifact.");
        }

        /* Take a turn */
        energy_use = 100;

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOICE);
    
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
                fire_ball(GF_ELEC, dir, 100, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_WHITE:
                msg_print("You breathe frost.");
                fire_ball(GF_COLD, dir, 110, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_BLACK:
                msg_print("You breathe acid.");
                fire_ball(GF_ACID, dir, 130, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_GREEN:
                msg_print("You breathe poison gas.");
                fire_ball(GF_POIS, dir, 150, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_RED:
                msg_print("You breathe fire.");
                fire_ball(GF_FIRE, dir, 200, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_MULTIHUED:
                chance = rand_int(5);
                msg_format("You breathe %s.",
                           ((chance == 1) ? "lightning" :
                            ((chance == 2) ? "frost" :
                             ((chance == 3) ? "acid" :
                              ((chance == 4) ? "poison gas" : "fire")))));
                fire_ball(((chance == 1) ? GF_ELEC :
                           ((chance == 2) ? GF_COLD :
                            ((chance == 3) ? GF_ACID :
                             ((chance == 4) ? GF_POIS : GF_FIRE)))),
                          dir, 250, 2);
                i_ptr->timeout = rand_int(225) + 225;
                break;

            case SV_DRAGON_BRONZE:
                msg_print("You breathe confusion.");
                fire_ball(GF_CONFUSION, dir, 120, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_GOLD:
                msg_print("You breathe sound.");
                fire_ball(GF_SOUND, dir, 130, 2);
                i_ptr->timeout = rand_int(450) + 450;
                break;

            case SV_DRAGON_CHAOS:
                chance = rand_int(2);
                msg_format("You breathe %s.",
                           ((chance == 1 ? "chaos" : "disenchantment")));
                fire_ball((chance == 1 ? GF_CHAOS : GF_DISENCHANT), dir, 220, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_LAW:
                chance = rand_int(2);
                msg_format("You breathe %s.",
                           ((chance == 1 ? "sound" : "shards")));
                fire_ball((chance == 1 ? GF_SOUND : GF_SHARDS), dir, 230, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_BALANCE:
                chance = rand_int(4);
                msg_format("You breathe %s.",
                           ((chance == 1) ? "chaos" :
                            ((chance == 2) ? "disenchantment" :
                             ((chance == 3) ? "sound" : "shards"))));
                fire_ball(((chance == 1) ? GF_CHAOS :
                           ((chance == 2) ? GF_DISENCHANT :
                            ((chance == 3) ? GF_SOUND : GF_SHARDS))),
                          dir, 250, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_SHINING:
                chance = rand_int(2);
                msg_format("You breathe %s.",
                           ((chance == 0 ? "light" : "darkness")));
                fire_ball((chance == 0 ? GF_LITE : GF_DARK), dir, 200, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            case SV_DRAGON_POWER:
                msg_print("You breathe the elements.");
                fire_ball(GF_MISSILE, dir, 300, 2);
                i_ptr->timeout = rand_int(300) + 300;
                break;

            default:
                msg_print("Oops.  You have bad breath.");
        }

        /* Take a turn */
        energy_use = 100;

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOICE);
    
        /* Success */
        return;
    }


    /* Mistake */
    msg_print("Oops.  That object cannot be activated.");
}


