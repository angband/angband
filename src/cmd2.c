/* File: cmd2.c */

/* Purpose: misc code, mainly to handle player commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * Swap the creature records of two locations.
 * This routine can be used to "move" or to "swap".
 */
void move_rec(int y1, int x1, int y2, int x2)
{
    int m_idx = cave[y1][x1].m_idx;
    int n_idx = cave[y2][x2].m_idx;
    
    /* Must be some movement */
    if ((y1 != y2) || (x1 != x2)) {

        /* Move index 2 to location 1 */
        cave[y1][x1].m_idx = n_idx;

        /* Move index 1 to location 2 */
        cave[y2][x2].m_idx = m_idx;

        /* Update new contents of location 1 */
        if (n_idx == 1) {

            /* Save the new location */
            py = y1;
            px = x1;
        }

        /* Move a monster */
        else if (n_idx) {

            monster_type *m_ptr = &m_list[n_idx];

            /* Save the new location */
            m_ptr->fy = y1;
            m_ptr->fx = x1;
        }

        /* Update new contents of location 2 */
        if (m_idx == 1) {

            /* Save the new location */
            py = y2;
            px = x2;
        }

        /* Move a monster */
        else if (m_idx) {

            monster_type *m_ptr = &m_list[m_idx];

            /* Save the new location */
            m_ptr->fy = y2;
            m_ptr->fx = x2;
        }

        /* Hack -- update screen */
        lite_spot(y1, x1);

        /* Hack -- update screen */
        lite_spot(y2, x2);
    }
}



/*
 * Hack -- Check if a level is a "quest" level
 */
int is_quest(int level)
{
    int i;
    if (!level) return (FALSE);
    for (i = 0; i < MAX_Q_IDX; i++) {
        if (q_list[i].level == level) return TRUE;
    }
    return FALSE;
}





/*
 * A monster (possibly a questor) has been destroyed
 *
 * Note that EVERY "questor" monster MUST be a Unique.
 * Note that Questor monsters should NEVER be created out of depth.
 *
 * Note that in a few, very rare, circumstances, killing Morgoth
 * may result in the Iron Crown of Morgoth crushing the Lead-Filled
 * Mace "Grond", since the Iron Crown is more important.
 */
static void check_quest(monster_type *m_ptr)
{
    int			i, ty, tx, ny, nx;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    monster_race *r_ptr = &r_list[m_ptr->r_idx];


    int total = 0;


    /* Only process "Quest Monsters" */
    if (!(r_ptr->rflags1 & RF1_QUESTOR)) return;


    /* Hack -- Mark quests as complete */
    for (i = 0; i < MAX_Q_IDX; i++) {

        /* Hack -- note completed quests */
        if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

        /* Count incomplete quests */
        if (q_list[i].level) total++;
    }


    /* Need some stairs */
    if (total) {

        /* Start on the dead monster */
        ty = m_ptr->fy;
        tx = m_ptr->fx;

        /* Stagger around until we find a legal grid */
        while (!valid_grid(ty, tx)) {

            int d = 1;

            /* Pick a location */	
            while (1) {
                ny = rand_spread(ty, d);
                nx = rand_spread(tx, d);
                if (!in_bounds(ny,nx)) continue;
                if (distance(ty, tx, ny, nx) > d) continue;
                if (los(ty, tx, ny, nx)) break;
            }

            ty = ny; tx = nx;
        }

        /* Delete any old object */
        delete_object(ty, tx);

        /* Make an object */
        c_ptr = &cave[ty][tx];
        c_ptr->i_idx = i_pop();
        i_ptr = &i_list[c_ptr->i_idx];
        invcopy(i_ptr, OBJ_DOWN_STAIR);
        i_ptr->iy = ty;
        i_ptr->ix = tx;

        /* Stairs are permanent */
        c_ptr->info |= GRID_PERM;

        /* Explain the stairway */
        msg_print("A magical stairway appears...");

        /* Remember to update everything */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    }


    /* Nothing left, game over... */
    else {

        total_winner = TRUE;

        p_ptr->redraw |= PR_BLOCK;

        msg_print("*** CONGRATULATIONS ***");
        msg_print("You have won the game!");
        msg_print("You may retire (suicide) when you are ready.");
    }
}




/*
 * Allocates basic objects upon a creatures death.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.  If "examine"
 * is FALSE, that is, the monster was killed by rubble or something,
 * do NOT look at the GOOD or SPECIAL or WINNER flags.
 *
 * Note first that Unique's cannot be killed by other monsters, so we
 * do not have to worry about "great treasure" disappearing.
 *
 * It looks like there may be a problem if a monster defines SPECIAL
 * without also defining GOOD.  I will have to check... XXX XXX
 */
void monster_death(monster_type *m_ptr, bool examine, bool visible)
{
    int			i, j, y1, x1, number;

    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    bool good = (examine && (r_ptr->rflags1 & RF1_DROP_GOOD));
    bool great = (examine && (r_ptr->rflags1 & RF1_DROP_GREAT));

    bool winner = (examine && (r_ptr->rflags1 & RF1_DROP_CHOSEN));

    int y = (int)m_ptr->fy;
    int x = (int)m_ptr->fx;

    bool	do_gold = (!(r_ptr->rflags1 & RF1_ONLY_ITEM));
    bool	do_item = (!(r_ptr->rflags1 & RF1_ONLY_GOLD));

    int		dump_item = 0;
    int		dump_gold = 0;


    /* Mark the monster as dead */
    m_ptr->dead = TRUE;


    /* Determine how much we can drop */
    number = 0;
    if ((r_ptr->rflags1 & RF1_DROP_60) && (randint(100) < 60)) number++;
    if ((r_ptr->rflags1 & RF1_DROP_90) && (randint(100) < 90)) number++;
    if (r_ptr->rflags1 & RF1_DROP_1D2) number += randint(2);
    if (r_ptr->rflags1 & RF1_DROP_2D2) number += damroll(2, 2);
    if (r_ptr->rflags1 & RF1_DROP_3D2) number += damroll(3, 2);
    if (r_ptr->rflags1 & RF1_DROP_4D2) number += damroll(4, 2);

    /* Drop some objects */
    for (j = 0; j < number; j++) {

        /* Try 20 times per item, increasing range */
        for (i = 0; i < 20; ++i) {

            int d = (i + 15) / 10;

            /* Pick a "correct" location */
            while (1) {
                y1 = rand_spread(y, d);
                x1 = rand_spread(x, d);
                if (!in_bounds(y1, x1)) continue;
                if (distance(y, x, y1, x1) > d) continue;
                if (los(y, x, y1, x1)) break;
            }

            /* Must be "clean" floor grid */
            if (!clean_grid_bold(y1, x1)) continue;

            /* Average dungeon and monster levels */
            object_level = (dun_level + r_ptr->level) / 2;

            /* Place something and count it if seen */
            if (good) {
                place_good(y1, x1, great);
                if (test_lite_bold(y1, x1)) dump_item++;
            }
            else if (do_gold && (rand_int(2) == 0)) {
                place_gold(y1, x1);
                if (test_lite_bold(y1, x1)) dump_gold++;
            }
            else if (do_item) {
                place_object(y1, x1);
                if (test_lite_bold(y1, x1)) dump_item++;
            }
            else if (do_gold) {
                place_gold(y1, x1);
                if (test_lite_bold(y1, x1)) dump_gold++;
            }

            /* Reset the object level */
            object_level = dun_level;

            /* Actually display the object's grid */
            lite_spot(y1, x1);

            break;
        }
    }



    /* Mega-Hack -- defeat the "WINNER" monster */
    if (winner) {

        /* Hack -- an "object holder" */
        inven_type prize;


        /* Prepare to make "Grond" */
        invcopy(&prize, OBJ_GROND);

        /* Actually create "Grond" */
        make_artifact(&prize);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);


        /* Prepare to make "Morgoth" */
        invcopy(&prize, OBJ_MORGOTH);

        /* Actually create "Morgoth" */
        make_artifact(&prize);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);
    }


    /* Take note of any dropped treasure */
    if (visible && (dump_item || dump_gold)) {

        /* Take notes on treasure */
        lore_treasure(m_ptr, dump_item, dump_gold);
    }
}




/*
 * XXX XXX Mega-Hack -- pass a fear code around
 *
 * This is used to delay messages in "py_attack()" until
 * all of the blows have been processed.
 */
static int monster_is_afraid = 0;



/*
 * Decreases monsters hit points and deletes monster if needed.
 * added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * Note that the player will get experience for any monsters killed here.
 * We return "TRUE" in this case, meaning "call check_experience() now...".
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Note that we only count the first 30000 kills per life, so that we
 * can distinguish between kills by this life and kills by past lives.
 */
bool mon_take_hit(int m_idx, int dam, bool print_fear)
{
    s32b		new_exp, new_exp_frac;
    bool		seen;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];


    /* Wake it up */
    m_ptr->csleep = 0;

    /* Hurt it */
    m_ptr->hp -= dam;

    /* It is dead now */
    if (m_ptr->hp < 0) {

        /* Hack -- always "see" unique monsters die */
        seen = ((m_ptr->ml) || (r_ptr->rflags1 & RF1_UNIQUE));

        /* Generate treasure (handle creeping coins) */
        coin_type = get_coin_type(r_ptr);
        monster_death(m_ptr, TRUE, seen);
        coin_type = 0;

        /* Give some experience */
        new_exp = ((long)r_ptr->mexp * r_ptr->level) / p_ptr->lev;
        new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % p_ptr->lev)
                        * 0x10000L / p_ptr->lev) + p_ptr->exp_frac;

        if (new_exp_frac >= 0x10000L) {
            new_exp++;
            p_ptr->exp_frac = new_exp_frac - 0x10000L;
        }
        else {
            p_ptr->exp_frac = new_exp_frac;
        }

        p_ptr->exp += new_exp;

        /* When drained, the player advances at 10% the normal rate */
        if (p_ptr->exp < p_ptr->max_exp) {
            p_ptr->max_exp += new_exp/10;
        }


        /* Check to see if a quest (or the game) is complete */
        check_quest(m_ptr);


        /* Delete the monster */
        delete_monster_idx(m_idx);

        /* When the player kills a Unique, it stays dead */
        if (r_ptr->rflags1 & RF1_UNIQUE) l_ptr->max_num = 0;

        /* XXX XXX Mega-Hack -- allow another ghost later */
        if (m_ptr->r_idx == MAX_R_IDX-1) l_ptr->max_num = 1;


        /* Always "see" dying uniques */
        if (r_ptr->rflags1 & RF1_UNIQUE) seen = TRUE;

        /* Recall even invisible uniques or winners */
        if (seen) {

            /* Only count the first 30000 kills per life */
            if (l_ptr->pkills < 30000) {

                /* Count kills this life */
                l_ptr->pkills++;

                /* Count kills in all lives */
                if (l_ptr->tkills < MAX_SHORT) l_ptr->tkills++;
            }

            /* Auto-recall if possible */
            if (use_recall_win && term_recall) {
                roff_recall(m_ptr->r_idx);
            }
        }

        /* No monster, so no fear */
        monster_is_afraid = 0;

        /* Monster is dead */
        return (TRUE);
    }


    /* Update the health bar (if enabled) */
    if (show_health_bar) health_redraw();



    /* Mega-Hack -- Pain cancels fear */
    if (m_ptr->monfear) {

        /* Pain makes you brave? */
        m_ptr->monfear -= randint(dam);

        /* Never recover fully */
        if (m_ptr->monfear <= 0) m_ptr->monfear = 1;
    }


#ifdef ALLOW_FEAR

    /* Sometimes a monster gets scared by damage */
    if (!m_ptr->monfear && !(r_ptr->rflags3 & RF3_NO_FEAR)) {

        int		percentage;

        /* Percentage of fully healthy */
        percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

        /*
         * Run (sometimes) if at 10% or less of max hit points,
         * or when hit for half its current hit points -DGK
         */
        if (((percentage <= 10) && (rand_int(10) < percentage)) ||
            ((dam >= m_ptr->hp) && (rand_int(5) != 0))) {

            /* Hack -- note fear */
            monster_is_afraid = 1;

            /* Take note */
            if (print_fear && m_ptr->ml) {

                char m_name[80];

                /* Get the monster name (or "it") */
                monster_desc(m_name, m_ptr, 0);

                message(m_name, 0x03);
                message(" flees in terror!", 0);
            }

            /* Hack -- Add some timed fear */
            m_ptr->monfear = (randint(10) +
                              (((dam >= m_ptr->hp) && (percentage > 7)) ?
                               20 : ((11 - percentage) * 5)));
        }
    }

#endif

    /* Not dead yet */
    return (FALSE);
}



/*
 * Critical hits (by player)
 *
 * Weight of weapon, plusses to hit, and character level all
 * contribute to the chance of a critical blow
 */
int critical_blow(int weight, int plus, int dam, int attack_type)
{
    int critical;

    critical = dam;

    if (randint(5000) <= (int)(weight + 5 * plus
                             + (class_level_adj[p_ptr->pclass][attack_type]
                                * p_ptr->lev))) {
        weight += randint(650);
        if (weight < 400) {
            critical = 2 * dam + 5;
            msg_print("It was a good hit!");
        }
        else if (weight < 700) {
            critical = 2 * dam + 10;
            msg_print("It was an excellent hit!");
        }
        else if (weight < 900) {
            critical = 3 * dam + 15;
            msg_print("It was a superb hit!");
        }
        else if (weight < 1300) {
            critical = 3 * dam + 20;
            msg_print("It was a *GREAT* hit!");
        }
        else {
            critical = ((7 * dam) / 2) + 25;
            msg_print("It was a *SUPERB* hit!");
        }
    }

    return (critical);
}






/*
 * Let an item 'i_ptr' fall to the ground at or near (y,x).
 * The initial location is assumed to be "in_bounds()".
 *
 * This function takes a parameter "chance".  This is the percentage
 * chance that the item will "disappear" instead of drop.  If the object
 * has been thrown, then this is the chance of disappearance on contact.
 */
void drop_near(inven_type *i_ptr, int chance, int y, int x)
{
    int		i, j, k, d, y1, x1;

    cave_type	*c_ptr;

    char	out_val[160];
    char	tmp_str[160];

    bool flag = FALSE;


    /* Start at the drop point */
    i = y1 = y;  j = x1 = x;

    /* See if the object "survives" the fall */
    if (artifact_p(i_ptr) || (randint(100) > chance)) {

        /* Start at the drop point */
        i = y1 = y; j = x1 = x;

        /* Try (20 times) to find an adjacent usable location */
        for (k = 0; !flag && (k < 20); ++k) {

            /* Distance distribution */
            d = ((k + 14) / 15);

            /* Pick a "nearby" location */
            while (1) {
                i = rand_spread(y1, d);
                j = rand_spread(x1, d);
                if (!in_bounds2(i, j)) continue;
                if (distance(y1, x1, i, j) > d) continue;
                if (los(y1, x1, i, j)) break;
            }

            /* Require clean floor space */
            if (!clean_grid_bold(i, j)) continue;

            /* Here looks good */
            flag = TRUE;
        }
    }

    /* Try really hard to place an artifact */
    if (!flag && artifact_p(i_ptr)) {

        /* Start at the drop point */
        i = y1 = y;  j = x1 = x;

        /* Try really hard to drop it */
        for (k = 0; !flag && (k < 1000); k++) {

            d = 1;

            /* Pick a location */
            while (1) {
                i = rand_spread(y1, d);
                j = rand_spread(x1, d);
                if (!in_bounds2(i, j)) continue;
                if (distance(y1, x1, i, j) > d) continue;
                if (los(y1, x1, i, j)) break;
            }

            /* Do not move through walls */
            if (!floor_grid_bold(i,j)) continue;

            /* Hack -- "bounce" to that location */
            y1 = i;  x1 = j;

            /* Get the cave grid */
            c_ptr = &cave[i][j];

            /* Nothing here?  Use it */
            if (!(c_ptr->i_idx)) flag = TRUE;

            /* After trying 99 places, crush any (normal) object */
            else if ((k>99) && valid_grid(i,j)) flag = TRUE;
        }

        /* XXX Artifacts will destroy ANYTHING to stay alive */
        if (!flag) {
            i = y, j = x, flag = TRUE;
            objdes(tmp_str, i_ptr, FALSE);
            (void)sprintf(out_val, "The %s crashes to the floor.", tmp_str);
            message(out_val, 0);
        }
    }

    /* Successful drop */
    if (flag) {

        bool old_floor = floor_grid_bold(i, j);

        /* Crush anything under us (for artifacts) */
        delete_object(i,j);

        /* Make a new object */
        c_ptr = &cave[i][j];
        c_ptr->i_idx = i_pop();
        i_list[c_ptr->i_idx] = *i_ptr;
        i_ptr = &i_list[c_ptr->i_idx];
        i_ptr->iy = i;
        i_ptr->ix = j;

        /* Under the player.  Mega-Hack -- no message if "dropped". */
        if (chance && (c_ptr->m_idx == 1)) {
            msg_print("You feel something roll beneath your feet.");
        }

        /* Update the display */
        lite_spot(i, j);

        /* Hack -- react to disappearing doors, etc */
        if (old_floor != floor_grid_bold(i, j)) {

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
        }
    }

    /* Poor little object */
    else {
        objdes(tmp_str, i_ptr, FALSE);
        (void)sprintf(out_val, "The %s disappears.", tmp_str);
        msg_print(out_val);
    }
}







/*
 * Attacker's level and plusses,  defender's AC
 * Note: pth could be less than 0 if weapon is too heavy
 * Always miss 1 out of 20, always hit 1 out of 20
 */
int test_hit(int bth, int level, int pth, int ac, int attack_type)
{
    int i;

    /* Hack -- disturb the player */
    disturb(1, 0);

    /* Hack -- 1 in 20 always miss, 1 in 20 always hit */
    i = randint(20);
    if (i == 1) return (FALSE);
    if (i == 20) return (TRUE);

    /* Calculate the "attack quality" */
    i = (bth + pth * BTH_PLUS_ADJ +
         (level * class_level_adj[p_ptr->pclass][attack_type]));

    /* Apply the "pth" against the "ac" */
    if ((i > 0) && (randint(i) > ((3 * ac) / 4))) {
        return TRUE;
    }

    /* Assume miss */
    return FALSE;
}


/*
 * Decreases players hit points and sets death flag if necessary
 */
void take_hit(int damage, cptr hit_from)
{
    /* Hack -- Apply "invulnerability" */
    if (p_ptr->invuln && (damage < 9000)) return;

    /* Hurt the player */
    p_ptr->chp -= damage;

    /* Dead player */
    if (p_ptr->chp < 0) {

        /* Cheat -- avoid death */
        if ((wizard || cheat_live) && !get_check("Die?")) {
            noscore |= 0x0001;
            msg_print("You invoke wizard mode and cheat death.");
            p_ptr->chp = p_ptr->mhp;
            p_ptr->redraw |= PR_HP;
            return;
        }

        /* New death */
        if (!death) {
            death = TRUE;
            (void)strcpy(died_from, hit_from);
            total_winner = FALSE;
        }

        /* Dead */
        return;
    }

    /* Display the hitpoints */
    p_ptr->redraw |= PR_HP;

    /* Hack -- hitpoint warning */
    if (p_ptr->chp <= p_ptr->mhp * hitpoint_warn / 10) {
        msg_print("*** LOW HITPOINT WARNING! ***");
        msg_print(NULL);
    }
}





/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 */
static int tot_dam_aux(inven_type *i_ptr, int tdam, monster_type *m_ptr)
{
    monster_race *r_ptr = &r_list[m_ptr->r_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* Only "missiles" and "weapons" can use these flags */
    if ((i_ptr->tval == TV_SHOT) ||
        (i_ptr->tval == TV_ARROW) ||
        (i_ptr->tval == TV_BOLT) ||
        (i_ptr->tval == TV_HAFTED) ||
        (i_ptr->tval == TV_POLEARM) ||
        (i_ptr->tval == TV_SWORD) ||
        (i_ptr->tval == TV_DIGGING)) {

        int mult = 1;

        /* Execute Dragon */
        if ((i_ptr->flags1 & TR1_KILL_DRAGON) &&
            (r_ptr->rflags3 & RF3_DRAGON)) {

            if (mult < 5) mult = 5;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_DRAGON;
        }

        /* Slay Dragon  */
        if ((i_ptr->flags1 & TR1_SLAY_DRAGON) &&
            (r_ptr->rflags3 & RF3_DRAGON)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_DRAGON;
        }

        /* Slay Undead */
        if ((i_ptr->flags1 & TR1_SLAY_UNDEAD) &&
            (r_ptr->rflags3 & RF3_UNDEAD)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_UNDEAD;
        }

        /* Slay Orc */
        if ((i_ptr->flags1 & TR1_SLAY_ORC) &&
            (r_ptr->rflags3 & RF3_ORC)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_ORC;
        }

        /* Slay Troll */
        if ((i_ptr->flags1 & TR1_SLAY_TROLL) &&
            (r_ptr->rflags3 & RF3_TROLL)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_TROLL;
        }

        /* Slay Giant */
        if ((i_ptr->flags1 & TR1_SLAY_GIANT) &&
            (r_ptr->rflags3 & RF3_GIANT)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_GIANT;
        }

        /* Slay Demon */
        if ((i_ptr->flags1 & TR1_SLAY_DEMON) &&
            (r_ptr->rflags3 & RF3_DEMON)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_DEMON;
        }

        /* Slay Evil */
        if ((i_ptr->flags1 & TR1_SLAY_EVIL) &&
            (r_ptr->rflags3 & RF3_EVIL)) {

            if (mult < 2) mult = 2;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_EVIL;
        }

        /* Slay Animal */
        if ((i_ptr->flags1 & TR1_SLAY_ANIMAL) &&
            (r_ptr->rflags3 & RF3_ANIMAL)) {

            if (mult < 2) mult = 2;
            if (m_ptr->ml) l_ptr->flags3 |= RF3_ANIMAL;
        }


        /* Lightning Brand */
        if (i_ptr->flags1 & TR1_BRAND_ELEC) {

            /* Notice immunity */
            if (r_ptr->rflags3 & RF3_IM_ELEC) {
                if (m_ptr->ml) l_ptr->flags3 |= RF3_IM_ELEC;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 5) mult = 5;
            }
        }

        /* Frost Brand */
        if (i_ptr->flags1 & TR1_BRAND_COLD) {

            /* Notice immunity */
            if (r_ptr->rflags3 & RF3_IM_COLD) {
                if (m_ptr->ml) l_ptr->flags3 |= RF3_IM_COLD;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 3) mult = 3;
            }
        }

        /* Flame Tongue */
        if (i_ptr->flags1 & TR1_BRAND_FIRE) {

            /* Notice immunity */
            if (r_ptr->rflags3 & RF3_IM_FIRE) {
                if (m_ptr->ml) l_ptr->flags3 |= RF3_IM_FIRE;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 3) mult = 3;
            }
        }


        /* Apply the damage multiplier */
        tdam *= mult;
    }


    /* Return the total damage */
    return (tdam);
}


/*
 * Player attacks a (poor, defenseless) creature	-RAK-	
 */
void py_attack(int y, int x)
{
    int			k, blows;
    int                 tot_tohit, base_tohit;

    int			m_idx = cave[y][x].m_idx;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];

    inven_type		*i_ptr;

    char		m_name[80];
    char		m_poss[80];

    char		out_val[160];


    /* Wake him up */
    m_ptr->csleep = 0;


    /* Extract monster name (or "it") and possessive */
    monster_desc(m_name, m_ptr, 0);

    /* Extract monster possessive (or "its") using gender if visible */
    monster_desc(m_poss, m_ptr, 0x22);


    /* Auto-Recall if possible and visible */
    if (use_recall_win && term_recall) {
        if (m_ptr->ml) roff_recall(m_ptr->r_idx);
    }

    /* Track a new monster */
    if (m_ptr->ml) health_track(m_idx);


    /* Access the weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Start with base bonus */
    tot_tohit = p_ptr->ptohit;

    /* Proper weapon */
    if (i_ptr->tval) {

        /* Calculate blows */
        blows = attack_blows((int)i_ptr->weight);

        /* Good weapon yields bonus to hit */
        tot_tohit += inventory[INVEN_WIELD].tohit;
    }

    /* Fists */
    else {

        /* Two blows */
        blows = 2;

        /* Hard to hit */
        tot_tohit -= 3;
    }

    /* If creature is lit, use base rates, else, make it harder to hit */
    if (m_ptr->ml) {
        base_tohit = p_ptr->bth;
    }
    else {
        base_tohit = (p_ptr->bth / 2) - (tot_tohit * (BTH_PLUS_ADJ - 1)) -
                     (p_ptr->lev * class_level_adj[p_ptr->pclass][CLA_BTH] / 2);
    }


    /* Reset the "fear message" catcher */
    monster_is_afraid = 0;

    /* Loop for number of blows, trying to hit the critter. */
    while (TRUE) {

        bool do_quake = FALSE;

        /* Use up one blow */
        blows--;

        /* We hit it! */
        if (test_hit(base_tohit, p_ptr->lev, tot_tohit,
                     r_ptr->ac, CLA_BTH)) {

            /* Message */
            sprintf(out_val, "You hit %s.", m_name);
            msg_print(out_val);

            /* Normal weapon.  Hack -- handle "earthquake brand" */
            if (i_ptr->tval) {
                k = damroll(i_ptr->dd, i_ptr->ds);
                k = tot_dam_aux(i_ptr, k, m_ptr);
                if ((i_ptr->flags1 & TR1_IMPACT) && (k > 50)) do_quake = TRUE;
                k = critical_blow(i_ptr->weight, tot_tohit, k, CLA_BTH);
                k += i_ptr->todam;
            }

            /* Bare hands */
            else {
                k = damroll(1, 1);
                k = critical_blow(1, 0, k, CLA_BTH);
            }

            /* Apply the player damage bonuses */
            k += p_ptr->ptodam;

            /* No negative damage (no "sword of healing!") */
            if (k < 0) k = 0;


            /* Complex message */
            if (wizard) {
                sprintf(out_val, "You do %d (out of %d) damage.",
                        k, m_ptr->hp);
                msg_print(out_val);
            }

            /* Confusion attack */
            if (p_ptr->confusing) {

                p_ptr->confusing = FALSE;
                msg_print("Your hands stop glowing.");

                if ((r_ptr->rflags3 & RF3_NO_CONF) ||
                    (randint(100) < r_ptr->level)) {
                    message(m_name, 0x03);
                    message(" is unaffected.", 0);
                }
                else {
                    m_ptr->confused = TRUE;
                    message(m_name, 0x03);
                    message("  appears confused.", 0);
                }

                if (m_ptr->ml && (rand_int(4) == 0)) {
                    if (r_ptr->rflags3 & RF3_NO_CONF) {
                        l_ptr->flags3 |= RF3_NO_CONF;
                    }
                }
            }

            /* Is it dead yet? */
            if (mon_take_hit(m_idx, k, FALSE)) {

                if ((r_ptr->rflags3 & RF3_DEMON) ||
                    (r_ptr->rflags3 & RF3_UNDEAD) ||
                    (r_ptr->rflags2 & RF2_STUPID) ||
                    (strchr("EvgX", r_ptr->r_char))) {
                    (void)sprintf(out_val, "You have destroyed %s.", m_name);
                }
                else {
                    (void)sprintf(out_val, "You have slain %s.", m_name);
                }
                msg_print(out_val);
                check_experience();

                /* No more attacks */
                blows = 0;
            }

            /* Mega-Hack -- apply earthquake brand */
            if (do_quake) earthquake();
        }

        else {
            (void)sprintf(out_val, "You miss %s.", m_name);
            msg_print(out_val);
        }

        /* Stop when out of blows */
        if (blows <= 0) break;
    }


    /* Hack -- delay the fear messages until here */
    if (monster_is_afraid == 1) {
        message(m_name, 0x03);
        message(" flees in terror!", 0);
    }

    /* Hack -- delay the fear messages until here */
    if (monster_is_afraid == -1) {
        message(m_name, 0x03);
        message(" recovers ", 0x02);
        message(m_poss, 0x02);
        message(" courage.", 0);
    }
}


/*
 * Make a bash attack on someone.  -CJS-
 * Used to be part of bash (below).
 *
 * This function should probably access "p_ptr->ptohit" and the shield
 * bonus "inventory[INVEN_ARM].tohit".
 */
void py_bash(int y, int x)
{
    int			k, avg_max_hp, base_tohit;

    int			m_idx = cave[y][x].m_idx;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    char		m_name[80];
    char		out_val[160];


    /* Wake him up */
    m_ptr->csleep = 0;


    /* Extract the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);


    /* Auto-Recall if possible and visible */
    if (use_recall_win && term_recall) {
        if (m_ptr->ml) roff_recall(m_ptr->r_idx);
    }

    /* Track a new monster */
    if (m_ptr->ml) health_track(m_idx);


    /* Attempt to bash */
    base_tohit = (p_ptr->use_stat[A_STR] +
                  inventory[INVEN_ARM].weight / 2 +
                  p_ptr->wt / 10);

    /* Harder to bash invisible monsters */
    if (!m_ptr->ml) {
        base_tohit = ((base_tohit / 2) -
             (p_ptr->use_stat[A_DEX] * (BTH_PLUS_ADJ - 1)) -
             (p_ptr->lev * class_level_adj[p_ptr->pclass][CLA_BTH] / 2));
    }

    /* Hack -- test for contact */
    if (test_hit(base_tohit, p_ptr->lev,
                 p_ptr->use_stat[A_DEX], r_ptr->ac, CLA_BTH)) {

        /* Message */
        (void)sprintf(out_val, "You hit %s.", m_name);
        msg_print(out_val);

        /* Calculate damage */
        k = damroll(inventory[INVEN_ARM].dd, inventory[INVEN_ARM].ds);
        k = critical_blow((int)(inventory[INVEN_ARM].weight / 4) +
                          p_ptr->use_stat[A_STR], 0, k, CLA_BTH);
        k += p_ptr->wt / 60 + 3;

        /* No negative damage */
        if (k < 0) k = 0;

        /* See if we done it in.				     */
        if (mon_take_hit(m_idx, k, TRUE)) {

            /* Appropriate message */
            if ((r_ptr->rflags3 & RF3_DEMON) ||
                (r_ptr->rflags3 & RF3_UNDEAD) ||
                (r_ptr->rflags2 & RF2_STUPID) ||
                (strchr("EvgX", r_ptr->r_char))) {
                (void)sprintf(out_val, "You have destroyed %s.", m_name);
            }
            else {
                (void)sprintf(out_val, "You have slain %s.", m_name);
            }
            msg_print(out_val);
            check_experience();
        }

        else {

            /* Powerful monsters cannot be stunned */
            avg_max_hp = ((r_ptr->rflags1 & RF1_FORCE_MAXHP) ?
                           (r_ptr->hdice * r_ptr->hside) :
                           ((r_ptr->hdice * (r_ptr->hside + 1)) >> 1));

            /* Start the message */
            message(m_name, 0x03);

            /* Apply saving throw */
            if ((100 + randint(400) + randint(400)) >
                (m_ptr->hp + avg_max_hp)) {

                m_ptr->stunned += rand_int(3) + 2;
                if (m_ptr->stunned > 24) m_ptr->stunned = 24;
                message(" appears stunned!", 0);
            }
            else {

                message(" ignores your bash!", 0);
            }
        }
    }
    else {
        (void)sprintf(out_val, "You miss %s.", m_name);
        msg_print(out_val);
    }

    /* Stumble -- if DEX less than 18/150 */
    if (randint(168) > p_ptr->use_stat[A_DEX]) {
        msg_print("You are off balance.");
        p_ptr->paralysis = rand_int(2) + 2;
    }
}



/*
 * Determines the odds of an object breaking when thrown
 * Note that "impact" is true if the object hit a monster
 * Artifacts never break, see the "drop_near()" function.
 * Assume the object has NOT hit a wall or monster
 * Hitting a monster doubles the breakage chance
 * Hitting a wall less than 3 grids away does too.
 */
static int breakage_chance(inven_type *i_ptr)
{
    /* Examine the item type */
    switch (i_ptr->tval) {

      /* Burning flasks */
      case TV_FLASK:
        return (100);
        
      /* Very breakable objects */
      case TV_POTION:
      case TV_BOTTLE:
      case TV_FOOD:
      case TV_JUNK:
        return (50);

      /* Somewhat breakable objects */
      case TV_LITE:
      case TV_SCROLL:
      case TV_ARROW:
      case TV_SKELETON:
        return (30);

      /* Slightly breakable objects */
      case TV_WAND:
      case TV_SHOT:
      case TV_BOLT:
      case TV_SPIKE:
        return (20);
    }

    /* Normal objects */
    return (10);
}


/*
 * Obtain the "facts" about a thrown object (or missile), taking into
 * account factors such as the current bow.
 *
 * Extract base chance to hit, bonus to hit, total damage,
 * the maximum distance, and the maximum number of shots.
 *
 * The separation of normal weapons from missile launchers via the
 * "bow slot" of 2.7.4 allowed simplification of the missile code below.
 */
static void facts(inven_type *i_ptr, \
                  int *tbth, int *tpth, int *tdam, int *tdis, int *thits)
{
    int scatter, tmp_weight;

    /* Get the "bow" (if any) */
    inven_type *j_ptr = &inventory[INVEN_BOW];

    /* Paranoia -- everything has weight */
    tmp_weight = MAX(1,i_ptr->weight);

    /* Damage from thrown object */
    *tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->todam;

    /* Base chance to hit */
    *tbth = p_ptr->bthb * 3 / 4;

    /* Plusses to hit */
    *tpth = p_ptr->ptohit + i_ptr->tohit;

    /* Distance based on strength */
    *tdis = (((p_ptr->use_stat[A_STR] + 20) * 10) / tmp_weight);

    /* Max distance of 10, no matter how strong */
    if (*tdis > 10) *tdis = 10;

    /* Default to single shot or throw */
    *thits = 1;


    /* Hack -- Rangers get multiple shots with a bow and arrow */
    if ((p_ptr->pclass == 4) &&
        (i_ptr->tval == TV_ARROW) &&
        (j_ptr->tval == TV_BOW) &&
        ((j_ptr->sval == SV_SHORT_BOW) ||
         (j_ptr->sval == SV_LONG_BOW))) {

         /* Give the Ranger some extra shots */
         *thits = attack_blows(j_ptr->weight) / 2;

         /* Never lose shots */
         if (*thits < 1) *thits = 1;
    }



    /* Handle Firing a missile while wielding the proper launcher */
    /* The maximum range is increased, the launcher modifiers are */
    /* added in, and then the bow multiplier is applied.  Note that */
    /* Bows of "Extra Might" get extra range and an extra bonus for */
    /* the damage multiplier, and Bows of "Extra Shots" give an extra */
    /* shot.  These only work when the proper missile is used.        */

    /* Examine the launcher */
    if (j_ptr->tval == TV_BOW) {

        /* Extract the "Extra Might" flag */
        bool xm = (j_ptr->flags3 & TR3_XTRA_MIGHT) ? TRUE : FALSE;

        /* Extract the "Extra Shots" flag */
        bool xs = (j_ptr->flags3 & TR3_XTRA_SHOTS) ? TRUE : FALSE;

        /* Analyze the launcher */
        switch (j_ptr->sval) {

          /* Sling and ammo */
          case SV_SLING:
            if (i_ptr->tval != TV_SHOT) break;
            *tbth = p_ptr->bthb;
            *tpth += j_ptr->tohit;
            *tdam += j_ptr->todam;
            *tdam *= (xm ? 3 : 2);
            *tdis = (xm ? 25 : 20);
            if (xs) *thits += 1;
            break;

          /* Short Bow and Arrow */
          case SV_SHORT_BOW:
            if (i_ptr->tval != TV_ARROW) break;
            *tbth = p_ptr->bthb;
            *tpth += j_ptr->tohit;
            *tdam += j_ptr->todam;
            *tdam *= (xm ? 3 : 2);
            *tdis = (xm ? 30 : 25);
            if (xs) *thits += 1;
            break;

          /* Long Bow and Arrow */
          case SV_LONG_BOW:
            if (i_ptr->tval != TV_ARROW) break;
            *tbth = p_ptr->bthb;
            *tpth += j_ptr->tohit;
            *tdam += j_ptr->todam;
            *tdam *= (xm ? 4 : 3);
            *tdis = (xm ? 35 : 30);
            if (xs) *thits += 1;
            break;

          /* Light Crossbow and Bolt */
          case SV_LIGHT_XBOW:
            if (i_ptr->tval != TV_BOLT) break;
            *tbth = p_ptr->bthb;
            *tpth += j_ptr->tohit;
            *tdam += j_ptr->todam;
            *tdam *= (xm ? 4 : 3);
            *tdis = (xm ? 35 : 25);
            if (xs) *thits += 1;
            break;

          /* Heavy Crossbow and Bolt */
          case SV_HEAVY_XBOW:
            if (i_ptr->tval != TV_BOLT) break;
            *tbth = p_ptr->bthb;
            *tpth += j_ptr->tohit;
            *tdam += j_ptr->todam;
            *tdam *= (xm ? 5 : 4);
            *tdis = (xm ? 40 : 30);
            if (xs) *thits += 1;
            break;
        }
    }

    /* Hack -- Apply a small "scatter effect" to the maximum range -BEN- */
    scatter = (*tdis+8) / 16;
    if (scatter) *tdis = rand_spread(*tdis, scatter);
}


/*
 * Can the player "fire" a given item?
 */
static bool item_tester_hook_fire(inven_type *i_ptr)
{
    if (i_ptr->tval == 0) return (FALSE);
    if (i_ptr->tval > TV_MAX_OBJECT) return (FALSE);
    return (TRUE);
}


/*
 * Fire (or Throw) an object from the pack or floor.
 *
 * Extra damage and chance of hitting when missiles are used
 * with correct weapon (xbow + bolt, bow + arrow, sling + shot).
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * Note: if target monster is unseen, make it much more difficult to
 * hit, subtract off most bonuses, and reduce bthb depending on distance
 *
 * Note that if an item is thrown for a distance of zero, then it has
 * a breakage chance of 100%.  So throwing an object at an adjacent wall,
 * or at one's feet, will break it unless it is an artifact.
 */
void do_cmd_fire()
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			tbth, tpth, tdam, tdis, thits;
    int			cur_dis, visible, shot, mshots;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    bool		floor = FALSE;

    bool		ok_throw = FALSE;

    bool		hit_body = FALSE;
    bool		hit_wall = FALSE;
    
    int			missile_attr;
    int			missile_char;

    char		out_val[160];
    char		tmp_str[160];



    /* Assume free turn */
    energy_use = 0;


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Check for use of the floor */
    floor = item_tester_hook_fire(i_ptr);

    /* Prepare the hook */
    item_tester_hook = item_tester_hook_fire;

    /* Get an item */
    if (!get_item(&item, "Fire/Throw which item? ", 0, inven_ctr-1, floor)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Access the item (if in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* User can request throw */
    if (always_throw) {

        ok_throw = TRUE;
    }

    /* Some things are just meant to be thrown */
    else if ((i_ptr->tval == TV_FLASK) || (i_ptr->tval == TV_SHOT) ||
             (i_ptr->tval == TV_ARROW) || (i_ptr->tval == TV_BOLT) ||
             (i_ptr->tval == TV_SPIKE) || (i_ptr->tval == TV_JUNK) ||
             (i_ptr->tval == TV_BOTTLE) || (i_ptr->tval == TV_SKELETON)) {

        ok_throw = TRUE;
    }

    /* Player knows (or feels) that it is cursed */
    else if (cursed_p(i_ptr) &&
             (inven_known_p(i_ptr) || (i_ptr->ident & ID_FELT))) {
        ok_throw = TRUE;
    }

    /* If the player knows that it is junk, throw it */
    else if (inven_known_p(i_ptr) && (i_ptr->cost <= 0)) {
        ok_throw = TRUE;
    }

    /* Known Artifacts and ego objects are never "okay" */
    else if (inven_known_p(i_ptr) &&
             ((i_ptr->name1) || (i_ptr->name2) || artifact_p(i_ptr))) {
        ok_throw = FALSE;
    }

    /* Normal weapons are okay to throw, since they do damage */
    /* (Moral of story: wield your weapon if you're worried */
    /* that you might throw it away!).  Anyway, there is */
    /* usually only a 1/5 chance of disappearing */
    else if ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_DIGGING)) {
        ok_throw = TRUE;
    }

    /* Most food/potions do 1d1 damage when thrown.  I want the code	*/
    /* to ask before throwing away potions of DEX, *Healing*, etc.	*/
    /* This also means it will ask before throwing potions of slow	*/
    /* poison, and other cheap items that the player is likely to	*/
    /* not care about.  This test means that mushrooms/molds of	*/
    /* unhealth, potions of detonations and death are the only	*/
    /* always-throwable food/potions (but see "bad items" below)	*/
    else if (((i_ptr->tval == TV_FOOD) || (i_ptr->tval == TV_POTION)) &&
             (inven_aware_p(i_ptr)) && ((i_ptr->dd * i_ptr->ds) > 1)) {
        ok_throw = TRUE;
    }


    /* If the object looks "not okay", verify it */
    if (!ok_throw) {
        char tmp_str[128], out_val[128];
        objdes(tmp_str, i_ptr, TRUE);
        sprintf(out_val, "Really throw %s?", tmp_str);
        if (!get_check(out_val)) return;
    }


    /* Get a direction (or Abort), apply confusion */
    if (!get_dir_c(NULL, &dir)) return;


    /* Take a turn and shoot the object */
    energy_use = 100;


    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);

    /* Count the maximum number of shouts */
    mshots = i_ptr->number;

    /* Keep shooting until out of arrows, count the shots */
    for (shot = 0; shot < mshots; shot++) {

        /* Create a "local missile object" */
        throw_obj = *i_ptr;
        throw_obj.number = 1;

        /* Reroll "damage" (etc) for each missile */
        facts(&throw_obj, &tbth, &tpth, &tdam, &tdis, &thits);

        /* Not everyone can shoot forever */
        if (shot >= thits) break;

        /* Verify "continued" shots (in the same direction) */
        if (shot && (other_query_flag && !get_check("Fire/Throw again?"))) break;


        /* Reduce and describe inventory */
        if (item >= 0) {
            inven_item_increase(item, -1);
            inven_item_describe(item);
            inven_item_optimize(item);
        }

        /* Reduce and describe floor item */
        else {
            floor_item_increase(py, px, -1);
            floor_item_optimize(py, px);
        }


        /* Start at the player */
        y = py;
        x = px;

        /* Predict the "target" location */
        tx = px + 99 * ddx[dir];
        ty = py + 99 * ddy[dir];

        /* Check for "target request" */
        if ((dir == 0) && target_okay()) {
            tx = target_col;
            ty = target_row;
        }


        /* Travel until stopped */
        for (cur_dis = 0; cur_dis < tdis; ) {

            /* Hack -- Stop at the target */
            if ((y == ty) && (x == tx)) break;

            /* Calculate the new location (see "project()") */
            ny = y;
            nx = x;
            mmove2(&ny, &nx, py, px, ty, tx);

            /* Stopped by walls/doors */
            if (!floor_grid_bold(ny,nx)) hit_wall = TRUE;

            /* Stop before the wall */
            if (hit_wall) break;
	    
            /* Advance the distance */
            cur_dis++;

            /* Save the new location */
            x = nx;
            y = ny;


            /* The player can see the (on screen) missile */
            if (panel_contains(y, x) && player_can_see_bold(y, x)) {

                /* Draw, Hilite, Fresh, Pause, Erase */
                handle_stuff(TRUE);
                mh_print_rel(missile_char, missile_attr, 0, y, x);
                move_cursor_relative(y, x);
                Term_fresh();
                delay(10 * delay_spd);
                lite_spot(y, x);
                Term_fresh();
            }

            /* The player cannot see the missile */
            else {

                /* Pause anyway, for consistancy */
                delay(10 * delay_spd);
            }


            /* Monster here, Try to hit it */
            if (cave[y][x].m_idx > 1) {

                int m_idx = cave[y][x].m_idx;

                monster_type *m_ptr = &m_list[m_idx];
                monster_race *r_ptr = &r_list[m_ptr->r_idx];

                /* Reduce "tbth" by range */
                tbth = tbth - cur_dis;

                /* Check the visibility */
                visible = m_ptr->ml;

                /* Unseen monsters are "hard to hit" (see note above) */
                if (!visible) {

                    /* Reduce "tbth" by some amount */
                    tbth = ((tbth / (cur_dis + 2)) -
                            (tpth * (BTH_PLUS_ADJ - 1)) -
                            (p_ptr->lev *
                             class_level_adj[p_ptr->pclass][CLA_BTHB] / 2));
                }

                /* Did we hit it? */
                if (test_hit(tbth, p_ptr->lev, tpth, r_ptr->ac, CLA_BTHB)) {

                    char m_name[80];

                    /* Get "the monster" or "it" */
                    monster_desc(m_name, m_ptr, 0);

                    /* Describe the object */
                    objdes(tmp_str, &throw_obj, FALSE);

                    /* Handle unseen monster */
                    if (!visible) {
                        (void)sprintf(out_val, "The %s finds a mark.",
                                      tmp_str);
                        msg_print(out_val);
                    }

                    /* Handle visible monster */
                    else {

                        char m_name[80];

                        /* Get "the monster" or "it" */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        (void)sprintf(out_val, "The %s hits %s.",
                                      tmp_str, m_name);
                        msg_print(out_val);

                        /* Auto-Recall if possible and visible */
                        if (use_recall_win && term_recall) {
                            if (m_ptr->ml) roff_recall(m_ptr->r_idx);
                        }

                        /* Hack -- Track this monster */
                        if (m_ptr->ml) health_track(m_idx);
                    }

                    /* Apply special damage */
                    tdam = tot_dam_aux(&throw_obj, tdam, m_ptr);
                    tdam = critical_blow((int)throw_obj.weight,
                                         tpth, tdam, CLA_BTHB);

                    /* No negative damage */
                    if (tdam < 0) tdam = 0;

                    /* Describe the pain/death result.  Display fear msg's */
                    if (mon_take_hit(m_idx, tdam, TRUE)) {
                        sprintf(out_val, "You have killed %s!", m_name);
                        message(out_val, 0);
                        check_experience();
                    }
                    else {
                        message_pain(m_idx, tdam);
                    }

                    /* Note the collision */
                    hit_body = TRUE;
                }

                /* Stop looking */
                break;
            }
        }

        /* Chance of breakage */
        j = breakage_chance(&throw_obj);

        /* Double the chance if we hit a monster */
        if (hit_body) j = j * 2;
        
        /* Double the chance if we hit a nearby wall */
        if (hit_wall && (cur_dis < 3)) j = j * 2;
        
        /* Hack -- zero distance throw always breaks */
        if (!cur_dis) j = 100;

        /* Paranoia -- maximum breakage chance */
        if (j > 100) j = 100;
        
        /* Drop (or break) near that location */
        drop_near(&throw_obj, j, y, x);
    }
}


/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup)
{
    int dir;

    /* Get the initial direction (or Abort) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        energy_use = 0;
        disturb(0, 0);
        return;
    }

    /* Apply partial confusion */
    dir = command_dir;
    confuse_dir(&dir, 0x02);

    /* Actually move the character */
    move_player(dir, pickup);
}


/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_stay(int pickup)
{
    cave_type *c_ptr = &cave[py][px];


    /* Check to see if he notices something  */
    /* "fos" may be negative if have good rings of searching */
    if ((p_ptr->fos <= 1) || (0 == rand_int(p_ptr->fos))) {
        search(py, px, p_ptr->srh);
    }

    /* Allow "search" mode to always get a "bonus" search */
    if (p_ptr->searching) {
        search(py, px, p_ptr->srh);
    }


    /* Hack -- enter a store if we are on one */
    if (i_list[c_ptr->i_idx].tval == TV_STORE_DOOR) {
        disturb(0, 0);
        enter_store(i_list[c_ptr->i_idx].sval - 1);
    }


    /* Try to Pick up anything under us */
    carry(py, px, pickup);
}



/*
 * Do the first (or next) step of the run (given max distance)
 * If distance is non positive, assume max distance of 1000.
 */
void do_cmd_run()
{
    /* Get the initial direction (or Abort) */
    if (!get_a_dir(NULL, &command_dir, 0)) {

        /* Graceful abort */
        energy_use = 0;
        return;
    }

    /* Max run distance (assume 100) */
    if (command_arg <= 0) command_arg = 100;

    /* Prepare to Run */
    find_init();

    /* Take the first step */
    find_step();
}


/*
 * Tunneling through wall
 * Used by TUNNEL and WALL_TO_MUD
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 */
int twall(int y, int x, int t1, int t2)
{
    cave_type	*c_ptr;

    int res = FALSE;

    /* Allow chaining of "probability" calls */
    if (t1 > t2) {

        c_ptr = &cave[y][x];

        /* Clear the wall code */
        c_ptr->info &= ~GRID_WALL_MASK;

        /* Forget the "field mark" */
        c_ptr->info &= ~GRID_MARK;

        /* Redisplay the grid */
        lite_spot(y, x);

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

        /* Worked */
        res = TRUE;
    }

    /* Result */
    return (res);
}


