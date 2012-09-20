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
 * Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 * This is one of the few places that we reference monster names
 * However, note that the game will still "work" without this code
 */
static int get_coin_type(monster_race *r_ptr)
{
    cptr name = r_ptr->name;
    
    /* Analyze "coin" monsters */
    if (r_ptr->r_char == '$') {

        /* Look for textual clues */
        if (strstr(name, "copper")) return (2);
        if (strstr(name, "silver")) return (5);
        if (strstr(name, "gold")) return (10);
        if (strstr(name, "mithril")) return (16);
        if (strstr(name, "adamantite")) return (17);

        /* Look for textual clues */
        if (strstr(name, "Copper")) return (2);
        if (strstr(name, "Silver")) return (5);
        if (strstr(name, "Gold")) return (10);
        if (strstr(name, "Mithril")) return (16);
        if (strstr(name, "Adamantite")) return (17);
    }
    
    /* Assume nothing */
    return (0);
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that in a few, very rare, circumstances, killing Morgoth
 * may result in the Iron Crown of Morgoth crushing the Lead-Filled
 * Mace "Grond", since the Iron Crown is more important.
 */
void monster_death(monster_type *m_ptr)
{
    int			i, j, y, x, ny, nx;

    int			dump_item = 0;
    int			dump_gold = 0;

    int			number = 0;
    int			total = 0;

    cave_type		*c_ptr;
    inven_type		*i_ptr;


    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    bool visible = (m_ptr->ml || (r_ptr->rflags1 & RF1_UNIQUE));
    
    bool good = (r_ptr->rflags1 & RF1_DROP_GOOD) ? TRUE : FALSE;
    bool great = (r_ptr->rflags1 & RF1_DROP_GREAT) ? TRUE : FALSE;

    bool do_gold = (!(r_ptr->rflags1 & RF1_ONLY_ITEM));
    bool do_item = (!(r_ptr->rflags1 & RF1_ONLY_GOLD));

    int force_coin = get_coin_type(r_ptr);


    /* Get the location */
    y = m_ptr->fy;
    x = m_ptr->fx;
    
    /* Determine how much we can drop */
    if ((r_ptr->rflags1 & RF1_DROP_60) && (rand_int(100) < 60)) number++;
    if ((r_ptr->rflags1 & RF1_DROP_90) && (rand_int(100) < 90)) number++;
    if (r_ptr->rflags1 & RF1_DROP_1D2) number += damroll(1, 2);
    if (r_ptr->rflags1 & RF1_DROP_2D2) number += damroll(2, 2);
    if (r_ptr->rflags1 & RF1_DROP_3D2) number += damroll(3, 2);
    if (r_ptr->rflags1 & RF1_DROP_4D2) number += damroll(4, 2);

    /* Drop some objects */
    for (j = 0; j < number; j++) {

        /* Try 20 times per item, increasing range */
        for (i = 0; i < 20; ++i) {

            int d = (i + 14) / 15;

            /* Pick a "correct" location */
            scatter(&ny, &nx, y, x, d, 0);

            /* Must be "clean" floor grid */
            if (!clean_grid_bold(ny, nx)) continue;

            /* Hack -- handle creeping coins */
            coin_type = force_coin;

            /* Average dungeon and monster levels */
            object_level = (dun_level + r_ptr->level) / 2;

            /* Place Gold */
            if (do_gold && (!do_item || (rand_int(100) < 50))) {
                place_gold(ny, nx);
                if (test_lite_bold(ny, nx)) dump_gold++;
            }

            /* Place Object */
            else {
                place_object(ny, nx, good, great);
                if (test_lite_bold(ny, nx)) dump_item++;
            }

            /* Reset the object level */
            object_level = dun_level;

            /* Reset "coin" type */
            coin_type = 0;

            /* Actually display the object's grid */
            lite_spot(ny, nx);

            break;
        }
    }


    /* Take note of any dropped treasure */
    if (visible && (dump_item || dump_gold)) {

        /* Take notes on treasure */
        lore_treasure(m_ptr, dump_item, dump_gold);
    }


    /* Mega-Hack -- drop "winner" treasures */
    if (r_ptr->rflags1 & RF1_DROP_CHOSEN) {

        /* Hack -- an "object holder" */
        inven_type prize;


        /* Prepare to make "Grond" */
        invcopy(&prize, lookup_kind(TV_HAFTED, SV_GROND));

        /* Actually create "Grond" */
        make_artifact(&prize);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);


        /* Prepare to make "Morgoth" */
        invcopy(&prize, lookup_kind(TV_CROWN, SV_MORGOTH));

        /* Actually create "Morgoth" */
        make_artifact(&prize);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);
    }


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

        /* Stagger around until we find a legal grid */
        while (!valid_grid(y, x)) {

            /* Pick a location */	
            scatter(&ny, &nx, y, x, 1, 0);

            /* Stagger */
            y = ny; x = nx;
        }

        /* Delete any old object */
        delete_object(y, x);

        /* Make a new "stair" object */
        c_ptr = &cave[y][x];
        c_ptr->i_idx = i_pop();
        i_ptr = &i_list[c_ptr->i_idx];
        invcopy(i_ptr, OBJ_DOWN_STAIR);
        i_ptr->iy = y;
        i_ptr->ix = x;

        /* Stairs are permanent */
        c_ptr->info |= GRID_PERM;

        /* Explain the stairway */
        msg_print("A magical stairway appears...");

        /* Remember to update everything */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    }


    /* Nothing left, game over... */
    else {

        /* Total winner */
        total_winner = TRUE;

        /* Redraw the "title" */
        p_ptr->redraw |= (PR_TITLE);

        /* Congratulations */
        msg_print("*** CONGRATULATIONS ***");
        msg_print("You have won the game!");
        msg_print("You may retire (commit suicide) when you are ready.");
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
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Note that we only count the first 30000 kills per life, so that we
 * can distinguish between kills by this life and kills by past lives.
 */
bool mon_take_hit(int m_idx, int dam, bool print_fear, cptr note)
{
    s32b		new_exp, new_exp_frac;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];


    /* Redraw (later) if needed */
    if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);
    

    /* Wake it up */
    m_ptr->csleep = 0;

    /* Hurt it */
    m_ptr->hp -= dam;

    /* It is dead now */
    if (m_ptr->hp < 0) {

        char m_name[80];
        
        /* Make a sound */
        sound(SOUND_KILL);

        /* Extract monster name */
        monster_desc(m_name, m_ptr, 0);

        /* Message */
        if (note) {
            msg_format("%^s%s", m_name, note);
        }
        else if ((r_ptr->rflags3 & RF3_DEMON) ||
                 (r_ptr->rflags3 & RF3_UNDEAD) ||
                 (r_ptr->rflags2 & RF2_STUPID) ||
                 (strchr("EvgX", r_ptr->r_char))) {
            msg_format("You have destroyed %s.", m_name);
        }
        else {
            msg_format("You have slain %s.", m_name);
        }
                
        /* Give some experience */
        new_exp = ((long)r_ptr->mexp * r_ptr->level) / p_ptr->lev;
        new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % p_ptr->lev)
                        * 0x10000L / p_ptr->lev) + p_ptr->exp_frac;

        /* Keep track of experience */
        if (new_exp_frac >= 0x10000L) {
            new_exp++;
            p_ptr->exp_frac = new_exp_frac - 0x10000L;
        }
        else {
            p_ptr->exp_frac = new_exp_frac;
        }

        /* Gain experience */
        gain_exp(new_exp);


        /* Generate treasure */
        monster_death(m_ptr);


        /* When the player kills a Unique, it stays dead */
        if (r_ptr->rflags1 & RF1_UNIQUE) l_ptr->max_num = 0;

        /* XXX XXX Mega-Hack -- allow another ghost later */
        if (m_ptr->r_idx == MAX_R_IDX-1) l_ptr->max_num = 1;


        /* Recall even invisible uniques or winners */
        if (m_ptr->ml || (r_ptr->rflags1 & RF1_UNIQUE)) {

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


        /* Delete the monster */
        delete_monster_idx(m_idx);


        /* Monster is dead */
        return (TRUE);
    }



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

                /* Sound */
                sound(SOUND_FLEE);
                
                /* Get the monster name (or "it") */
                monster_desc(m_name, m_ptr, 0);

                /* Message */
                msg_format("%^s flees in terror!", m_name);
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
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_norm(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "blow" power */
    i = (weight + (plus * 5) + (p_ptr->lev * 3));

    /* Chance */
    if (randint(5000) <= i) {

        k = weight + randint(650);

        if (k < 400) {
            msg_print("It was a good hit!");
            dam = 2 * dam + 5;
        }
        else if (k < 700) {
            msg_print("It was a great hit!");
            dam = 2 * dam + 10;
        }
        else if (k < 900) {
            msg_print("It was a superb hit!");
            dam = 3 * dam + 15;
        }
        else if (k < 1300) {
            msg_print("It was a *GREAT* hit!");
            dam = 3 * dam + 20;
        }
        else {
            msg_print("It was a *SUPERB* hit!");
            dam = ((7 * dam) / 2) + 25;
        }
    }

    return (dam);
}


/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
static int critical_shot(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "shot" power */
    i = (weight + (plus * 4) + (p_ptr->lev * 2));

    /* Critical hit */
    if (randint(5000) <= i) {

        k = weight + randint(500);

        if (k < 500) {
            msg_print("It was a good hit!");
            dam = 2 * dam + 5;
        }
        else if (k < 1000) {
            msg_print("It was a great hit!");
            dam = 2 * dam + 10;
        }
        else {
            msg_print("It was a superb hit!");
            dam = 3 * dam + 15;
        }
    }

    return (dam);
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
    int		k, d, ny, nx, y1, x1;

    cave_type	*c_ptr;

    bool flag = FALSE;


    /* Start at the drop point */
    ny = y1 = y;  nx = x1 = x;

    /* See if the object "survives" the fall */
    if (artifact_p(i_ptr) || (randint(100) > chance)) {

        /* Start at the drop point */
        ny = y1 = y; nx = x1 = x;

        /* Try (20 times) to find an adjacent usable location */
        for (k = 0; !flag && (k < 20); ++k) {

            /* Distance distribution */
            d = ((k + 14) / 15);

            /* Pick a "nearby" location */
            scatter(&ny, &nx, y1, x1, d, 0);

            /* Require clean floor space */
            if (!clean_grid_bold(ny, nx)) continue;

            /* Here looks good */
            flag = TRUE;
        }
    }

    /* Try really hard to place an artifact */
    if (!flag && artifact_p(i_ptr)) {

        /* Start at the drop point */
        ny = y1 = y;  nx = x1 = x;

        /* Try really hard to drop it */
        for (k = 0; !flag && (k < 1000); k++) {

            d = 1;

            /* Pick a location */
            scatter(&ny, &nx, y1, x1, d, 0);

            /* Do not move through walls */
            if (!floor_grid_bold(ny,nx)) continue;

            /* Hack -- "bounce" to that location */
            y1 = ny;  x1 = nx;

            /* Get the cave grid */
            c_ptr = &cave[ny][nx];

            /* Nothing here?  Use it */
            if (!(c_ptr->i_idx)) flag = TRUE;

            /* After trying 99 places, crush any (normal) object */
            else if ((k>99) && valid_grid(ny,nx)) flag = TRUE;
        }

        /* XXX Artifacts will destroy ANYTHING to stay alive */
        if (!flag) {

            char i_name[80];

            /* Location */
            ny = y;
            nx = x;
            
            /* Always okay */
            flag = TRUE;

            /* Description */
            objdes(i_name, i_ptr, FALSE, 0);

            /* Message */
            msg_format("The %s crashes to the floor.", i_name);
        }
    }

    /* Successful drop */
    if (flag) {

        bool old_floor = floor_grid_bold(ny, nx);

        /* Crush anything under us (for artifacts) */
        delete_object(ny,nx);

        /* Make a new "stair" object */
        c_ptr = &cave[ny][nx];
        c_ptr->i_idx = i_pop();
        i_list[c_ptr->i_idx] = *i_ptr;
        i_ptr = &i_list[c_ptr->i_idx];
        i_ptr->iy = ny;
        i_ptr->ix = nx;

        /* Sound */
        sound(SOUND_DROP);
        
        /* Under the player.  Mega-Hack -- no message if "dropped". */
        if (chance && (c_ptr->m_idx == 1)) {
            msg_print("You feel something roll beneath your feet.");
        }

        /* Update the display */
        lite_spot(ny, nx);

        /* Hack -- react to disappearing doors, etc */
        if (old_floor != floor_grid_bold(ny, nx)) {

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
        }
    }

    /* Poor little object */
    else {

        char i_name[80];

        /* Describe */
        objdes(i_name, i_ptr, FALSE, 0);

        /* Message */
        msg_format("The %s disappear%s.",
                   i_name, ((i_ptr->number == 1) ? "s" : ""));
    }
}






/*
 * Determine if the player "hits" a monster (normal combat).
 * Always miss 1 out of 20, always hit 1 out of 20
 */
static int test_hit_norm(int bonus, int ac, int vis)
{
    int i, k;

    /* Roll a 20 sided die */
    k = rand_int(20);

    /* Hack -- Instant miss */
    if (k == 0) return (FALSE);

    /* Hack -- Instant hit */
    if (k == 1) return (TRUE);

    /* Calculate the "attack quality" */
    i = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));

    /* Penalize invisible targets */
    if (!vis) i = i / 2;
    
    /* Never hit */
    if (i <= 0) return (FALSE);
    
    /* Power competes against armor */
    if (randint(i) > (ac * 3 / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}


/*
 * Determine if the player "hits" a monster (bashing).
 * Always miss 1 out of 20, always hit 1 out of 20
 *
 * XXX XXX XXX Hack -- Bashing is a hack!
 */
static int test_hit_bash(int ac, int vis)
{
    int i, k;

    /* Roll a 20 sided die */
    k = rand_int(20);

    /* Hack -- Instant miss */
    if (k == 0) return (FALSE);

    /* Hack -- Instant hit */
    if (k == 1) return (TRUE);

    /* XXX XXX XXX Mega-Hack -- Calculate the "bash ability" */
    i = (p_ptr->skill_thn +
         (p_ptr->use_stat[A_STR] + p_ptr->use_stat[A_DEX]) +
         ((inventory[INVEN_ARM].weight / 2) + (p_ptr->wt / 10)));

    /* Penalize invisible targets */
    if (!vis) i = i / 2;
    
    /* Never hit */
    if (i <= 0) return (FALSE);
    
    /* Power competes against armor */
    if (randint(i) > (ac * 3 / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}


/*
 * Determine if the player "hits" a monster (normal combat).
 * Always miss 1 out of 20, always hit 1 out of 20
 */
static int test_hit_bow(int bonus, int range, int ac, int vis)
{
    int i, k;

    /* Roll a 20 sided die */
    k = rand_int(20);

    /* Hack -- Instant miss */
    if (k == 0) return (FALSE);

    /* Hack -- Instant hit */
    if (k == 1) return (TRUE);

    /* Calculate the "attack quality" */
    i = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

    /* Penalize range */
    if (range) i -= range;
    
    /* Invisible monsters are harder to hit */
    if (!vis) i = i / 2;
    
    /* Never hit */
    if (i <= 0) return (FALSE);
    
    /* Power competes against armor */
    if (randint(i) > (ac * 3 / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
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
            p_ptr->redraw |= (PR_HP);
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
    p_ptr->redraw |= (PR_HP);

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
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x)
{
    int			k, tot_tohit, blows = 1;

    int			m_idx = cave[y][x].m_idx;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];

    inven_type		*i_ptr;

    char		m_name[80];
    char		m_poss[80];


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

    /* Extract number of blows */
    blows = p_ptr->num_blow;
    
    /* Reward proper weapon (not fists) */
    if (i_ptr->tval) tot_tohit += i_ptr->tohit;



    /* Reset the "fear message" catcher */
    monster_is_afraid = 0;

    /* Loop for number of blows, trying to hit the critter. */
    while (TRUE) {

        bool do_quake = FALSE;

        /* Use up one blow */
        blows--;

        /* We hit it! */
        if (test_hit_norm(tot_tohit, r_ptr->ac, m_ptr->ml)) {

            /* Sound */
            sound(SOUND_HIT);
            
            /* Message */
            msg_format("You hit %s.", m_name);

            /* Hack -- bare hands do one damage */
            k = 1;
            
            /* Normal weapon.  Hack -- handle "earthquake brand" */
            if (i_ptr->k_idx) {
                k = damroll(i_ptr->dd, i_ptr->ds);
                k = tot_dam_aux(i_ptr, k, m_ptr);
                if ((i_ptr->flags1 & TR1_IMPACT) && (k > 50)) do_quake = TRUE;
                k = critical_norm(i_ptr->weight, tot_tohit, k);
                k += i_ptr->todam;
            }

            /* Apply the player damage bonuses */
            k += p_ptr->ptodam;

            /* No negative damage */
            if (k < 0) k = 0;


            /* Complex message */
            if (wizard) {
                msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
            }


            /* Confusion attack */
            if (p_ptr->confusing) {

                /* Cancel glowing hands */
                p_ptr->confusing = FALSE;

                /* Message */
                msg_print("Your hands stop glowing.");

                /* Confuse the monster */
                if (r_ptr->rflags3 & RF3_NO_CONF) {
                    if (m_ptr->ml) l_ptr->flags3 |= RF3_NO_CONF;
                    msg_format("%^s is unaffected.", m_name);
                }
                else if (rand_int(100) < r_ptr->level) {
                    msg_format("%^s is unaffected.", m_name);
                }
                else {
                    msg_format("%^s appears confused.", m_name);
                    m_ptr->confused += 10 + rand_int(p_ptr->lev) / 5;
                }
            }

            /* Is it dead yet? */
            if (mon_take_hit(m_idx, k, FALSE, NULL)) {

                /* No more attacks */
                blows = 0;
            }

            /* Mega-Hack -- apply earthquake brand */
            if (do_quake) earthquake(py, px, 10);
        }

        /* Player misses */
        else {

            /* Sound */
            sound(SOUND_MISS);
            
            /* Message */
            msg_format("You miss %s.", m_name);
        }

        /* Stop when out of blows */
        if (blows <= 0) break;
    }


    /* Hack -- delay the fear messages until here */
    if (monster_is_afraid == 1) {

        /* Sound */
        sound(SOUND_FLEE);
        
        /* Message */
        msg_format("%^s flees in terror!", m_name);
    }

    /* Hack -- delay the fear messages until here */
    if (monster_is_afraid == -1) {

        /* Message */
        msg_format("%^s recovers %s courage.", m_name, m_poss);
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
    int			k;

    int			m_idx = cave[y][x].m_idx;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    char		m_name[80];


    /* Wake up the monster */
    m_ptr->csleep = 0;


    /* Auto-Recall if possible and visible */
    if (use_recall_win && term_recall) {
        if (m_ptr->ml) roff_recall(m_ptr->r_idx);
    }

    /* Track a new monster if visible */
    if (m_ptr->ml) health_track(m_idx);


    /* Extract the monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);

    /* Test for contact */
    if (test_hit_bash(r_ptr->ac, m_ptr->ml)) {

        /* Sound */
        sound(SOUND_HIT);
        
        /* Message */
        msg_format("You bash %s.", m_name);

        /* Calculate base damage */
        k = damroll(inventory[INVEN_ARM].dd, inventory[INVEN_ARM].ds);

        /* Hack -- Reward player weight */
        k += (p_ptr->wt / 60) + 3;

        /* No negative damage */
        if (k < 0) k = 0;

        /* See if we done it in. */
        if (mon_take_hit(m_idx, k, TRUE, NULL)) {

            /* Dead monster */
        }

        /* React to bashing */
        else {

            /* Check for "stun" */
            if ((100 + randint(400) + randint(400)) > (m_ptr->hp + m_ptr->maxhp)) {

                /* Message */
                msg_format("%^s appears stunned!", m_name);

                /* Stun the monster */
                if (m_ptr->stunned < 25) m_ptr->stunned += rand_int(3) + 2;
            }
        }
    }

    /* Totally miss */
    else {

        /* Sound */
        sound(SOUND_MISS);
        
        /* Message */
        msg_format("You miss %s.", m_name);
    }

    /* Stumble (sometimes) -- Note -- ignore free action */
    if (rand_int(100) > adj_dex_safe[stat_index(A_DEX)]) {

        /* Message */
        msg_print("You are off balance.");

        /* Hack -- Bypass free action */
        p_ptr->paralysis = 2 + rand_int(2);
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
 *
 * Handle Firing a missile while wielding the proper launcher
 * The maximum range is increased, the launcher modifiers are
 * added in, and then the bow multiplier is applied.  Note that
 * Bows of "Extra Might" get extra range and an extra bonus for
 * the damage multiplier, and Bows of "Extra Shots" give an extra
 * shot.  These only work when the proper missile is used.
 *
 * See "calc_bonuses()" for more calculations and such.
 */
static void facts(inven_type *i_ptr, \
                  int *tpth, int *tdam, int *tdis, int *thits)
{
    /* Get the "bow" (if any) */
    inven_type *j_ptr = &inventory[INVEN_BOW];


    /* Paranoia -- require a weight */
    if (i_ptr->weight < 1) i_ptr->weight = 1;
    
    /* Hack -- Distance based on strength */
    *tdis = (((p_ptr->use_stat[A_STR] + 20) * 10) / i_ptr->weight);

    /* Max distance of 10, no matter how strong */
    if (*tdis > 10) *tdis = 10;


    /* Damage from thrown object */
    *tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->todam;

    /* Plusses to hit */
    *tpth = p_ptr->ptohit + i_ptr->tohit;

    /* Default to single shot or throw */
    *thits = 1;


    /* Proper combination of launcher and missile */
    if ((j_ptr->tval == TV_BOW) && (i_ptr->tval == p_ptr->tval_ammo)) {

        int wind;
        
        /* Extract the "Extra Might" flag */
        bool xm = (j_ptr->flags3 & TR3_XTRA_MIGHT) ? TRUE : FALSE;

        /* Use the proper number of shots */
        *thits = p_ptr->num_fire;

        /* Reward the launcher */        
        *tpth += j_ptr->tohit;
        *tdam += j_ptr->todam;

        /* Analyze the launcher */
        switch (j_ptr->sval) {

          /* Sling and ammo */
          case SV_SLING:
            *tdam *= (xm ? 3 : 2);
            *tdis = (xm ? 25 : 20);
            break;

          /* Short Bow and Arrow */
          case SV_SHORT_BOW:
            *tdam *= (xm ? 3 : 2);
            *tdis = (xm ? 30 : 25);
            break;

          /* Long Bow and Arrow */
          case SV_LONG_BOW:
            *tdam *= (xm ? 4 : 3);
            *tdis = (xm ? 35 : 30);
            break;

          /* Light Crossbow and Bolt */
          case SV_LIGHT_XBOW:
            *tdam *= (xm ? 4 : 3);
            *tdis = (xm ? 35 : 25);
            break;

          /* Heavy Crossbow and Bolt */
          case SV_HEAVY_XBOW:
            *tdam *= (xm ? 5 : 4);
            *tdis = (xm ? 40 : 30);
            break;
        }

        /* Extract a "wind" effect */
        wind = (*tdis+8) / 16;

        /* Hack -- apply "wind" -BEN- */
        if (wind) *tdis = rand_spread(*tdis, wind);
    }
}


/*
 * Can the player "fire" a given item?
 */
static bool item_tester_hook_fire(inven_type *i_ptr)
{
    /* Decline non-items */
    if (!i_ptr->k_idx) return (FALSE);

    /* Decline non-pickup-able items */
    if (i_ptr->tval > TV_MAX_OBJECT) return (FALSE);

    /* Okay */
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
 * Note: "unseen" monsters are very hard to hit.
 *
 * Note that if an item is thrown for a distance of zero, then it has
 * a breakage chance of 100%.  So throwing an object at an adjacent wall,
 * or at one's feet, will break it unless it is an artifact.
 */
void do_cmd_fire()
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			tpth, tdam, tdis, thits;
    int			cur_dis, visible, shot, mshots;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    bool		ok_throw = FALSE;

    bool		hit_body = FALSE;
    bool		hit_wall = FALSE;
    
    int			missile_attr;
    int			missile_char;



    /* Assume free turn */
    energy_use = 0;


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Prepare the hook */
    item_tester_hook = item_tester_hook_fire;

    /* Get an item */
    if (!get_item(&item, "Fire/Throw which item? ", 0, inven_ctr-1, TRUE)) {
        if (item == -2) msg_print("You have nothing to fire or throw.");
        return;
    }


    /* Access the item (if in the pack) */
    if (item >= 0) i_ptr = &inventory[item];


    /* User can request throw */
    if (always_throw) {
        ok_throw = TRUE;
    }

    /* Known/felt items which are broken/cursed are crappy */
    else if ((inven_known_p(i_ptr) || (i_ptr->ident & ID_SENSE)) &&
             (cursed_p(i_ptr) || broken_p(i_ptr))) {             
        ok_throw = TRUE;
    }

    /* Some things are just meant to be thrown */
    else if ((i_ptr->tval == TV_FLASK) ||
             (i_ptr->tval == TV_SHOT) ||
             (i_ptr->tval == TV_ARROW) ||
             (i_ptr->tval == TV_BOLT) ||
             (i_ptr->tval == TV_SPIKE) ||
             (i_ptr->tval == TV_JUNK) ||
             (i_ptr->tval == TV_BOTTLE) ||
             (i_ptr->tval == TV_SKELETON)) {
        ok_throw = TRUE;
    }

    /* Known Artifacts and Ego-Items are never "okay" */
    else if (inven_known_p(i_ptr) &&
             (artifact_p(i_ptr) || ego_item_p(i_ptr))) {
        ok_throw = FALSE;
    }

#if 0

    /* Normal weapons do damage when thrown */
    else if ((i_ptr->tval == TV_SWORD) ||
             (i_ptr->tval == TV_POLEARM) ||
             (i_ptr->tval == TV_HAFTED) ||
             (i_ptr->tval == TV_DIGGING)) {
        ok_throw = TRUE;
    }

#endif

    /* Food/Potions are only okay if they do MORE than one damage */
    else if (((i_ptr->tval == TV_FOOD) || (i_ptr->tval == TV_POTION)) &&
             (inven_aware_p(i_ptr)) && ((i_ptr->dd * i_ptr->ds) > 1)) {
        ok_throw = TRUE;
    }


    /* If the object looks "not okay", verify it */
    if (!ok_throw) {

        int old_number;
        
        char i_name[80];
        char out_val[160];

        /* Description */
        old_number = i_ptr->number;
        i_ptr->number = 1;
        objdes(i_name, i_ptr, TRUE, 3);
        i_ptr->number = old_number;

        /* Verify */
        sprintf(out_val, "Really throw %s?", i_name);
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
        facts(&throw_obj, &tpth, &tdam, &tdis, &thits);

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


        /* Hack -- Handle stuff */
        handle_stuff();


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
                print_rel(missile_char, missile_attr, y, x);
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

                /* Check the visibility */
                visible = m_ptr->ml;

                /* Did we hit it? */
                if (test_hit_bow(tpth, cur_dis, r_ptr->ac, m_ptr->ml)) {

                    char i_name[80];
                    char m_name[80];

                    /* Get "the monster" or "it" */
                    monster_desc(m_name, m_ptr, 0);

                    /* Describe the object */
                    objdes(i_name, &throw_obj, FALSE, 0);

                    /* Handle unseen monster */
                    if (!visible) {

                        /* Invisible monster */
                        msg_format("The %s finds a mark.", i_name);
                    }

                    /* Handle visible monster */
                    else {

                        char m_name[80];

                        /* Get "the monster" or "it" */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        msg_format("The %s hits %s.", i_name, m_name);

                        /* Auto-Recall if possible and visible */
                        if (use_recall_win && term_recall) {
                            if (m_ptr->ml) roff_recall(m_ptr->r_idx);
                        }

                        /* Hack -- Track this monster */
                        if (m_ptr->ml) health_track(m_idx);
                    }

                    /* Apply special damage */
                    tdam = tot_dam_aux(&throw_obj, tdam, m_ptr);
                    tdam = critical_shot(throw_obj.weight, tpth, tdam);

                    /* No negative damage */
                    if (tdam < 0) tdam = 0;

                    /* Hit the monster, check for death */
                    if (mon_take_hit(m_idx, tdam, TRUE, NULL)) {

                        /* Dead monster */
                    }

                    /* No death */
                    else {

                        /* Message */
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
        
        /* Double the chance if we hit the floor */
        if (!cur_dis) j = j * 2;

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


    /* Spontaneous Searching */
    if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos))) {
        search();
    }
    
    /* Continuous Searching */
    if (p_ptr->searching) {
        search();
    }


    /* Hack -- enter a store if we are on one */
    if (i_list[c_ptr->i_idx].tval == TV_STORE_DOOR) {
        disturb(0, 0);
        store_enter(i_list[c_ptr->i_idx].sval - 1);
    }


    /* Try to Pick up anything under us */
    carry(pickup);
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
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * This will, however, produce grids which are NOT illuminated
 * (or darkened) along with the rest of the room.
 */
bool twall(int y, int x, int t1, int t2)
{
    cave_type	*c_ptr = &cave[y][x];

    /* Allow chaining of "probability" calls */
    if (t1 <= t2) return (FALSE);

    /* Clear the wall code */
    c_ptr->info &= ~GRID_WALL_MASK;

    /* Forget the "field mark" */
    c_ptr->info &= ~GRID_MARK;

    /* Redisplay the grid */
    lite_spot(y, x);

    /* Update some things */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

    /* Result */
    return (TRUE);
}




/*** Targetting Code ***/


/*
 * This targetting code stolen from Morgul -CFT
 *
 * Player can target any location, or any visible, reachable, monster.
 */



/*
 * Determine is a monster makes a reasonable target
 */
bool target_able(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

    /* Monster MUST be visible */
    if (!(m_ptr->ml)) return (FALSE);

    /* XXX Monster MUST be seeable */
    /* if (CLEAR_ATTR && CLEAR_CHAR) return (FALSE); */

    /* Hack -- no targeting hallucinations */
    if (p_ptr->image) return (FALSE);
        
    /* Monster MUST be in a "reasonable" location */
    return (projectable(py, px, m_ptr->fy, m_ptr->fx));
}


/*
 * Be sure that the target_row & target_col vars are correct
 * Also be sure that a targetted creature, if any, is "legal".
 * Cancel current target is it is unreasonable.
 */
void target_update(void)
{
    /* Update moving targets */
    if (target_who > 0) {

        /* Update reasonable targets */
        if (target_able(target_who)) {

            monster_type *m_ptr = &m_list[target_who];

            /* Acquire monster location */
            target_row = m_ptr->fy;
            target_col = m_ptr->fx;
        }
    }
}




/*
 * Simple query -- is the "target" okay to use?
 * Obviously, if target mode is disabled, it is not.
 */
bool target_okay()
{
    /* Accept stationary targets */
    if (target_who < 0) return (TRUE);

    /* Check moving targets */
    if (target_who > 0) {

        /* Accept reasonable targets */
        if (target_able(target_who)) {

            monster_type *m_ptr = &m_list[target_who];

            /* Acquire monster location */
            target_row = m_ptr->fy;
            target_col = m_ptr->fx;

            /* Good target */
            return (TRUE);
        }
    }

    /* Assume no target */
    return (FALSE);
}






/*
 * Set a new target.  This code can be called from get_a_dir()
 * I think that targetting an outer border grid may be dangerous
 */
bool target_set()
{

#ifdef ALLOW_TARGET

    int		m_idx;

    int		row = py;
    int		col = px;

    char	query;

    char	out_val[160];

    int		who[1024];

    int		m = 0, n = 0;


    /* Go ahead and turn off target mode */
    target_who = 0;

    /* Turn off health tracking */
    health_track(0);


    /* Examine nearby monsters */
    for (m_idx = MIN_M_IDX; m_idx < m_max; m_idx++) {

        monster_type *m_ptr = &m_list[m_idx];

        /* Skip "dead" monsters */
        if (!m_ptr->r_idx) continue;

        /* Ignore "unreasonable" monsters */
        if (!target_able(m_idx)) continue;

        /* Save this monster index */
        who[n++] = m_idx;
    }
    

    /* Interact with the monsters (if any) */
    while (n) {

        monster_type *m_ptr = &m_list[who[m]];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        /* Access monster location */
        row = m_ptr->fy;
        col = m_ptr->fx;

        /* Hack -- Track that monster */
        health_track(who[m]);

        /* Hack -- handle stuff */
        handle_stuff();
        
        /* Auto-recall */
        if (use_recall_win && term_recall) {

            /* Describe the monster */
            roff_recall(m_ptr->r_idx);

            /* Describe */
            sprintf(out_val,
                    "%s [(t)arget, (o)ffset, (p)osition, or (q)uit]",
                    r_ptr->name);
            prt(out_val,0,0);

            /* Get a command */
            move_cursor_relative(row,col);
            query = inkey();
        }

        /* Optional recall */
        else {

            /* Describe, prompt for recall */
            sprintf(out_val,
                    "%s [(t)arget, (o)ffset, (p)osition, (r)ecall, or (q)uit]",
                    r_ptr->name);
            prt(out_val,0,0);

            /* Get a command */
            move_cursor_relative(row,col);
            query = inkey();

            /* Optional recall */
            while ((query == 'r') || (query == 'R')) {

                /* Recall on screen */
                save_screen();
                query = roff_recall(m_ptr->r_idx);
                restore_screen();

                /* Get a new keypress */
                move_cursor_relative(row,col);
                query = inkey();
            }
        }

        /* Hack -- cancel tracking */
        health_track(0);

        /* Analyze (non "recall") command */
        switch (query) {

            case ESCAPE:
            case 'Q': case 'q':
                return (FALSE);

            case 'T': case 't':
            case '5': case '.':
            case '0':
                target_who = who[m];
                target_row = row;
                target_col = col;
                return (TRUE);

            case '*':
            case ' ':
                if (++m == n) m = 0;
                break;

            case '-':
                if (m-- == 0) m = n - 1;
                break;

            case 'P': case 'p':
                row = py;
                col = px;
                n = 0;
                break;

            case 'O': case 'o':
                n = 0;
                break;
        }
    }


    /* Now try a location */
    prt("Use cursor to designate target. [(t)arget]",0,0);

    /* Query until done */
    while (TRUE) {

        /* Light up the current location */
        move_cursor_relative(row, col);

        /* Get a command, and convert it to standard form */
        query = inkey();

        /* Analyze the keypress */
        switch (query) {

            case ESCAPE:
            case 'Q': case 'q':
                return (FALSE);

            case '5': case '.':
            case 'T': case 't':
            case '0':
                target_who = -1;
                target_row = row;
                target_col = col;
                return (TRUE);

            case '1': case 'B': case 'b':
                col--;
            case '2': case 'J': case 'j':
                row++;
                break;
            case '3': case 'N': case 'n':
                row++;
            case '6': case 'L': case 'l':
                col++;
                break;
            case '7': case 'Y': case 'y':
                row--;
            case '4': case 'H': case 'h':
                col--;
                break;
            case '9': case 'U': case 'u':
                col++;
            case '8': case 'K': case 'k':
                row--;
                break;
        }

        /* Verify column */
        if ((col>=cur_wid-1) || (col>panel_col_max)) col--;
        else if ((col<=0) || (col<panel_col_min)) col++;

        /* Verify row */
        if ((row>=cur_hgt-1) || (row>panel_row_max)) row--;
        else if ((row<=0) || (row<panel_row_min)) row++;
    }

#endif

    /* Assume no target */
    return (FALSE);
}





/*
 * Given a direction, apply "confusion" to it
 *
 * Mode is as in "get_a_dir()" below, using:
 *   0x01 = Apply total Confusion
 *   0x02 = Apply partial Confusion (75%)
 *   0x04 = Allow the direction "5"
 *   0x08 = ??? Handle stun like confused
 */
void confuse_dir(int *dir, int mode)
{
    /* Check for confusion */
    if (p_ptr->confused) {

        /* Does the confusion get a chance to activate? */
        if ((mode & 0x01) ||
            ((mode & 0x02) && (randint(4) > 1))) {

            /* Warn the user */
            msg_print("You are confused.");

            /* Pick a random (valid) direction */
            do {
                *dir = randint(9);
            } while (!(mode & 0x04) && (*dir == 5));
        }
    }
}


/*
 * This function should be used by anyone who wants a direction
 * Hack -- it can also be used to get the "direction" of "here"
 *
 * It takes an optional prompt, a pointer to a dir, and a mode (see below),
 * and returns "Did something besides 'Escape' happen?".  The dir is loaded
 * with -1 (on abort) or 0 (for target) or 1-9 (for a "keypad" direction)
 * including 5 (for "here").
 *
 * The mode indicates whether "5" is a legal direction, and also
 * how to handle confusion (always, most times, or never).  There
 * is an extra bit, perhaps for "treat stun as confusion".
 *
 * The mode allows indication of whether target mode is enforced,
 * or if not, if it is optional, and if so, if it can be interactively
 * re-oriented, and how confusion should affect targetting.
 *
 * Note that if (command_dir > 0), it will pre-empt user interaction
 * Note that "Force Target", if set, will pre-empt user interaction
 *
 * So, currently, there is no account taken of "remember that we
 * are currently shooting at the target" (except via "Force Target").
 *
 * Note that confusion (if requested) over-rides any other choice.
 * "Force Target" + "Always Confused" yields ugly situation for user...
 *
 * Note that use of the "Force Target" plus "Repeated Commands"
 * and/or "Confusion" + "Repeated Commands" could be really messy
 *
 * Note: We change neither command_rep nor energy_use
 * Note: We assume our caller correctly handles the "abort" case
 *
 * Although we seem to correctly handle the (dir == &command_dir) case,
 * it is not recommended to use this with the "Confusion" flags,
 * since the users "preferred" direction is then lost.
 *
 * However, note that if we are given "&command_dir" as our "dir" arg,
 * we will correctly reset it to -1 on cancellation.  However, if we
 * are also requested to handle confusion, things may get ugly.
 *
 * Modes are composed of the or-ing of these bit flags:
 *   0x01 = Apply total Confusion
 *   0x02 = Apply partial Confusion (75%)
 *   0x04 = Allow the "here" direction ('5')
 *   0x08 = ??? Handle stun like confused
 *   0x10 = Allow use of Use-Target command ('t','0','5')
 *   0x20 = Allow use of Set-Target command ('*')
 *   0x40 = ???
 *   0x80 = ???
 */
int get_a_dir(cptr prompt, int *dir, int mode)
{
    char        command;

    char	out_val[160];
    

    /* Use global command_dir (if set) */
    if (command_dir > 0) {
        *dir = command_dir;
    }

    /* Use "old target" if possible */
    else if (use_old_target && (mode & 0x10) && target_okay()) {
        *dir = 0;
    }

    /* Ask user for a direction */
    else {

        /* Start with a non-direction */
        *dir = -1;

        /* Ask until satisfied */
        while (1) {

            if (prompt) {
                strcpy(out_val, prompt);
            }
            else if (!(mode & 0x10) || !target_okay()) {
                sprintf(out_val, "Direction (%sEscape to cancel)? ",
                        (mode & 0x20) ? "'*' to choose a target, " : "");
            }
            else {
                sprintf(out_val, "Direction (%s%sEscape to cancel)? ",
                        (mode & 0x10) ? "'5' for target, " : "",
                        (mode & 0x20) ? "'*' to re-target, " : "");
            }

            /* Get a command (or Cancel) */
            if (!get_com(out_val, &command)) return (FALSE);

            /* Convert various keys to "standard" keys */
            switch (command) {

                /* Convert roguelike directions */
                case 'B': case 'b': command = '1'; break;
                case 'J': case 'j': command = '2'; break;
                case 'N': case 'n': command = '3'; break;
                case 'H': case 'h': command = '4'; break;
                case 'L': case 'l': command = '6'; break;
                case 'Y': case 'y': command = '7'; break;
                case 'K': case 'k': command = '8'; break;
                case 'U': case 'u': command = '9'; break;

                /* Roguelike uses "." for "center" */
                case '.': command = '5'; break;

                /* Accept "t" for "target" */
                case 'T': case 't': command = '0'; break;
            }

            /* Hack -- Perhaps accept '5' as itself */
            if ((mode & 0x04) && (command == '5')) {
                *dir = 5; break;
            }

            /* If not accepting '5' as itself, convert it */
            if (command == '5') command = '0';

            /* Perhaps allow "use-target" */
            if ((mode & 0x10) && (command == '0')) {
                if (target_okay()) {
                    *dir = 0; break;
                }
            }

            /* Perhaps allow "set-target" */
            if ((mode & 0x20) && (command == '*')) {
                if (target_set() && target_okay()) {
                    *dir = 0; break;
                }
            }

            /* Always accept actual directions (command != '5') */	
            if (command >= '1' && command <= '9') {
                *dir = command - '0'; break;
            }

            /* Optional "help" */
            if (command == '?') {
                /* do_cmd_help("help.hlp"); */
            }

            /* Errors */
            bell();
        }
    }

    /* Confuse the direction */
    confuse_dir(dir, mode);

    /* A "valid" direction was entered */
    return (TRUE);
}





/*
 * See "get_a_dir" above
 */
int get_dir(cptr prompt, int *dir)
{
    /* Allow use of "target" commands.  Forbid the "5" direction. */
    if (get_a_dir(prompt, dir, 0x30)) return (TRUE);

    /* XXX XXX XXX Mega-Hack -- allow commands to be careless */
    energy_use = 0;

    /* Command aborted */
    return FALSE;
}


/*
 * Like get_dir(), but if "confused", pick randomly
 */

int get_dir_c(cptr prompt, int *dir)
{
    /* Allow "Target".  Forbid "5".  Handle "total" confusion */
    if (get_a_dir(prompt, dir, 0x31)) return (TRUE);

    /* Hack -- allow commands to be careless */
    energy_use = 0;

    /* Command aborted */
    return FALSE;
}





/*
 * Search Mode.  Note that nobody should use "status" for anything
 * but "visual reflection of current status", that is, the "searching"
 * condition of the player needs an actual flag.
 */
void search_on()
{
    if (p_ptr->searching) return;

    /* Set the searching flag */
    p_ptr->searching = TRUE;

    /* Update stuff */
    p_ptr->update |= (PU_BONUS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_STATE | PR_SPEED);
}

void search_off(void)
{
    if (!p_ptr->searching) return;

    /* Clear the searching flag */
    p_ptr->searching = FALSE;

    /* Update stuff */
    p_ptr->update |= (PU_BONUS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_STATE | PR_SPEED);
}


/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 * The second arg allows this function to flush all pending output.
 */
void disturb(int stop_search, int flush_output)
{
    /* Cancel auto-commands */
    /* command_new = 0; */
    
    /* Cancel repeated commands */
    if (command_rep) command_rep = 0;

    /* Cancel resting */
    if (p_ptr->rest) rest_off();

    /* Cancal running if possible */
    if (find_flag) end_find();

    /* Cancel searching if requested */
    if (stop_search) search_off();

    /* Hack -- redraw the "state" later */
    p_ptr->redraw |= (PR_STATE);

    /* Hack -- sometimes flush the input */
    if (flush_disturb) flush();

    /* Hack -- handle stuff if requested */
    if (flush_output) handle_stuff();
    
    /* Hack -- always hilite the player */
    move_cursor_relative(py, px);

    /* Hack -- flush output if requested */
    if (flush_output) Term_fresh();
}


/*
 * Searches for hidden things.			-RAK-	
 */
void search(void)
{
    int           y, x, chance;

    cave_type    *c_ptr;
    inven_type   *i_ptr;

    char	i_name[80];


    /* Start with base search ability */
    chance = p_ptr->skill_srh;
    
    /* Penalize various conditions */
    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused || p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (y = (py - 1); y <= (py + 1); y++) {
        for (x = (px - 1); x <= (px + 1); x++) {

            /* Sometimes, notice things */
            if (randint(100) < chance) {

                c_ptr = &cave[y][x];
                i_ptr = &i_list[c_ptr->i_idx];

                /* Nothing there */
                if (!(c_ptr->i_idx)) {
                    /* Nothing */
                }

                /* Invisible trap? */
                else if (i_ptr->tval == TV_INVIS_TRAP) {
                    objdes(i_name, i_ptr, TRUE, 3);
                    msg_format("You have found %s.", i_name);
                    i_ptr->tval = TV_VIS_TRAP;
                    lite_spot(y, x);
                    disturb(0, 0);
                }

                /* Secret door?	*/
                else if (i_ptr->tval == TV_SECRET_DOOR) {

                    msg_print("You have found a secret door.");

                    /* Hack -- drop on top */
                    invcopy(i_ptr, OBJ_CLOSED_DOOR);

                    /* Place it in the dungeon */
                    i_ptr->iy = y;
                    i_ptr->ix = x;

                    /* Redraw the door */
                    lite_spot(y, x);

                    /* Notice it */
                    disturb(0, 0);
                }

                /* Search chests */
                else if (i_ptr->tval == TV_CHEST) {

                    /* Examine chests for traps */
                    if (!inven_known_p(i_ptr) && (chest_traps[i_ptr->pval])) {

                        msg_print("You have discovered a trap on the chest!");

                        /* Know the trap */
                        inven_known(i_ptr);

                        /* Notice it */
                        disturb(0, 0);
                    }
                }
            }
        }
    }
}



/*
 * Stop resting
 */
void rest_off()
{
    p_ptr->rest = 0;

    /* Redraw the state */
    p_ptr->redraw |= (PR_STATE);

    /* flush last message, or delete "press any key" message */
    msg_print(NULL);
}



/*
 * Calculates current boundaries
 * Called below and from "do_cmd_locate()".
 */
void panel_bounds()
{
    panel_row_min = panel_row * (SCREEN_HGT / 2);
    panel_row_max = panel_row_min + SCREEN_HGT - 1;
    panel_row_prt = panel_row_min - 1;
    panel_col_min = panel_col * (SCREEN_WID / 2);
    panel_col_max = panel_col_min + SCREEN_WID - 1;
    panel_col_prt = panel_col_min - 13;
}



/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
void verify_panel(void)
{
    int y = py;
    int x = px;

    int prow = panel_row;
    int pcol = panel_col;

    /* Scroll screen when 2 grids from top/bottom edge */
    if ((y < panel_row_min + 2) || (y > panel_row_max - 2)) {
        prow = ((y - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
        if (prow > max_panel_rows) prow = max_panel_rows;
        else if (prow < 0) prow = 0;
    }

    /* Scroll screen when 4 grids from left/right edge */
    if ((x < panel_col_min + 4) || (x > panel_col_max - 4)) {
        pcol = ((x - SCREEN_WID / 4) / (SCREEN_WID / 2));
        if (pcol > max_panel_cols) pcol = max_panel_cols;
        else if (pcol < 0) pcol = 0;
    }

    /* Check for "no change" */
    if ((prow == panel_row) && (pcol == panel_col)) return;

    /* Hack -- optional disturb at "edge of panel" */
    if (find_bound) disturb(0, 0);

    /* Save the new panel info */
    panel_row = prow;
    panel_col = pcol;

    /* Recalculate the boundaries */
    panel_bounds();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
}



/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(int pickup)
{
    cave_type  *c_ptr = &cave[py][px];
    inven_type *i_ptr = &i_list[c_ptr->i_idx];

    char	i_name[80];
    

    /* Hack -- nothing here to pick up */
    if (!(c_ptr->i_idx)) return;


    /* Describe the object */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Pick up gold */
    if (i_ptr->tval == TV_GOLD) {

        /* Disturb */
        disturb(0, 0);

        /* Message */
        msg_format("You have found %ld gold pieces worth of %s.",
                   (long)i_ptr->pval, i_name);

        /* Collect the gold */
        p_ptr->au += i_ptr->pval;

        /* Redraw gold */
        p_ptr->redraw |= (PR_GOLD);

        /* Delete gold */
        delete_object(py, px);
    }

    /* Can it be picked up? */
    else if (i_ptr->tval <= TV_MAX_PICK_UP) {

        /* Hack -- disturb */
        disturb(0, 0);

        /* Describe the object */
        if (!pickup) {
            msg_format("You see %s.", i_name);
        }

        /* Note that the pack is too full */
        else if (!inven_check_num(i_ptr)) {
            msg_format("You have no room for %s.", i_name);
        }

        /* Pick up the item (if requested and allowed) */
        else {

            int okay = TRUE;

            /* Hack -- query every item */
            if (carry_query_flag) {	
                char out_val[160];
                sprintf(out_val, "Pick up %s? ", i_name);
                okay = get_check(out_val);
            }

            /* Attempt to pick up an object. */
            if (okay) {

                int locn = inven_carry(i_ptr);

                /* Message */
                msg_format("You have %s. (%c)", i_name, index_to_label(locn));

                /* Delete original */
                delete_object(py, px);
            }
        }
    }
}





/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int check_hit(int power)
{
    int k, ac;

    /* Hack -- 1 in 20 always miss, 1 in 20 always hit */
    k = randint(20);
    if (k == 1) return (FALSE);
    if (k == 20) return (TRUE);

    /* Paranoia -- No power */
    if (power <= 0) return (FALSE);
    
    /* Player armor */
    ac = p_ptr->pac + p_ptr->ptoac;

    /* Power competes against Armor */
    if (randint(power) > ((3 * ac) / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}


/*
 * Handle player hitting a real trap
 *
 * We use the old location to back away from rubble traps
 */
static void hit_trap(int oy, int ox)
{
    int			i, num, dam;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    char		i_name[80];


    /* Disturb the player */
    disturb(0, 0);

    /* Get the cave grid */
    c_ptr = &cave[py][px];

    /* Get the trap */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Make the trap "visible" */
    i_ptr->tval = TV_VIS_TRAP;

    /* Paranoia -- redraw the grid */
    lite_spot(py, px);

    /* Describe the trap */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Examine the trap sub-val */
    switch (i_ptr->sval) {

      case SV_TRAP_PIT:
        msg_print("You fell into a pit!");
        if (p_ptr->ffall) {
            msg_print("You float gently to the bottom of the pit.");
        }
        else {
            dam = damroll(2, 6);
            take_hit(dam, i_name);
        }
        break;

      case SV_TRAP_SPIKED_PIT:

        /* Now here is a nasty trap indeed */
        /* It really makes feather falling important */
        msg_print("You fall into a spiked pit!");

        if (p_ptr->ffall) {
            msg_print("You float gently to the floor of the pit.");
            msg_print("You carefully avoid touching the spikes.");
        }

        else {

            /* Base damage */
            dam = damroll(2,6);

            /* Extra spike damage */
            if (rand_int(2) == 0) {

                msg_print("You are impaled!");
                dam = dam * 2;
                cut_player(randint(dam));
            }

            /* Poisonous spikes */
            if (rand_int(3) == 0) {

                msg_print("The spikes are poisoned!");

                if (p_ptr->immune_pois ||
                    p_ptr->resist_pois ||
                    p_ptr->oppose_pois) {
                    msg_print("The poison does not affect you!");
                }

                else {
                    dam = dam * 2;
                    add_poisoned(randint(dam));
                }
            }

            /* Take the damage */
            take_hit(dam, i_name);
        }
        break;

      case SV_TRAP_TRAP_DOOR:
        msg_print("You fell through a trap door!");
        if (p_ptr->ffall) {
            msg_print("You float gently down to the next level.");
        }
        else {
            dam = damroll(2,8);
            take_hit(dam, i_name);
        }
        new_level_flag = TRUE;
        dun_level++;
        break;

      case SV_TRAP_ARROW:
        if (check_hit(125)) {
            msg_print("An arrow hits you.");
            dam = damroll(1,8);
            take_hit(dam, i_name);
        }
        else {
            msg_print("An arrow barely misses you.");
        }
        break;

      case SV_TRAP_DART_SLOW:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, i_name);
            if (!p_ptr->free_act) {
                add_slow(rand_int(20) + 10);
                msg_print("You feel sluggish!");
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_DART_DEX:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, i_name);
            if (!p_ptr->sustain_dex) {
                (void)dec_stat(A_DEX, 10, FALSE);
                msg_print("You feel clumsy!");
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_DART_STR:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, i_name);
            if (!p_ptr->sustain_str) {
                msg_print("You feel weaker!");
                (void)dec_stat(A_STR, 10, FALSE);
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_DART_CON:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, i_name);
            if (!p_ptr->sustain_con) {
                (void)dec_stat(A_CON, 10, FALSE);
                msg_print("You feel less healthy!");
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_GAS_POISON:
        msg_print("A pungent green gas surrounds you!");
        add_poisoned(rand_int(20) + 10);
        break;

      case SV_TRAP_GAS_BLIND:
        msg_print("A black gas surrounds you!");
        add_blind(rand_int(50) + 50);
        break;

      case SV_TRAP_GAS_CONFUSE:
        msg_print("A gas of scintillating colors surrounds you!");
        add_confused(rand_int(15) + 15);
        break;

      case SV_TRAP_GAS_SLEEP:
        msg_print("A strange white mist surrounds you!");
        add_paralysis(rand_int(10) + 5);
        break;

      case SV_TRAP_FIRE:
        msg_print("You are enveloped in flames!");
        dam = damroll(4, 6);
        fire_dam(dam, "a fire trap");
        break;

      case SV_TRAP_ACID:
        msg_print("You are splashed with acid!");
        dam = damroll(4, 6);
        acid_dam(dam, "an acid trap");
        break;

      case SV_TRAP_TELEPORT:
        msg_print("You hit a teleport trap!");
        teleport_flag = TRUE;
        teleport_dist = 100;
        break;

      case SV_TRAP_SUMMON:
        msg_print("You are enveloped in a cloud of smoke!");
        delete_object(py, px); /* Rune disappears.    */
        num = 2 + randint(3);
        for (i = 0; i < num; i++) {
            (void)summon_monster(py, px, dun_level + MON_SUMMON_ADJ);
        }
        break;

      case SV_TRAP_FALLING_ROCK:

        /* Message */
        msg_print("You are hit by falling rock.");
        
        /* Damage */
        dam = damroll(2, 6);
        take_hit(dam, "a falling rock");

        /* Hack -- Turn the trap into rubble */
        invcopy(i_ptr, OBJ_RUBBLE);
        i_ptr->iy = py;
        i_ptr->ix = px;

        /* Hack -- Back the player up */
        move_rec(py, px, oy, ox);

        /* Verify the panel */
        verify_panel();

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
        p_ptr->update |= (PU_DISTANCE);

        break;

      case SV_TRAP_LOOSE_ROCK:
        msg_print("Hmmm, there was something under this rock.");
        delete_object(py, px);
        place_object(py, px, FALSE, FALSE);
        break;

      case SV_TRAP_GLYPH:
        break;

      default:
        msg_print("Oops. Undefined trap.");
        break;
    }
}


/*
 * Moves player in the given direction, with the given "pickup" flag.
 *
 * Note that "moving" into a wall is a free move, and will NOT hit any
 * invisible monster which is "hiding" in the walls.  The player must
 * tunnel into the wall to hit invisible monsters.  However, to make
 * life easier, visible monsters can be attacked by moving into walls.
 *
 * Note that every "result" of moving that should cancel running or
 * repeated walking must explicitly call the "disturb()" function.
 */
void move_player(int dir, int do_pickup)
{
    int                 y, x;
    cave_type *c_ptr;
    inven_type	*i_ptr;
    monster_type *m_ptr;

    /* Remember if the player was running */
    bool was_running = find_flag;

    /* Save info for dealing with traps and such */
    int old_row = py;
    int old_col = px;


    /* Find the result of moving */
    y = py + ddy[dir];
    x = px + ddx[dir];

    /* Examine the destination */
    c_ptr = &cave[y][x];

    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];


    /* Hack -- always attack visible monsters */
    if ((c_ptr->m_idx > 1) && (m_ptr->ml)) {

        /* Hitting a monster is disturbing */
        disturb(0, 0);

        /* Handle fear */
        if (!p_ptr->fear) {
            py_attack(y, x);
        }
        else {
            msg_print("You are too afraid!");
        }
    }

    /* Player can not walk through "walls" */
    else if (!floor_grid_bold(y,x)) {

        /* Disturb the player */
        disturb(0, 0);

        /* XXX XXX Hack -- allow "mapping" in the dark */
        if (!(c_ptr->info & GRID_MARK) &&
            (p_ptr->blind || !(c_ptr->info & GRID_LITE))) {

            /* Rubble */
            if (i_ptr->tval == TV_RUBBLE) {
                msg_print("You feel some rubble blocking your way.");
                c_ptr->info |= GRID_MARK;
                lite_spot(y, x);
            }

            /* Closed door */
            else if (i_ptr->tval == TV_CLOSED_DOOR) {
                msg_print("You feel a closed door blocking your way.");
                c_ptr->info |= GRID_MARK;
                lite_spot(y, x);
            }

            /* Wall (or secret door) */
            else {
                msg_print("You feel a wall blocking your way.");
                c_ptr->info |= GRID_MARK;
                lite_spot(y, x);
            }
        }

        /* Notice non-walls unless "running" */
        else if (!was_running && (c_ptr->i_idx)) {

            /* Rubble */
            if (i_ptr->tval == TV_RUBBLE) {
                msg_print("There is rubble blocking your way.");
            }

            /* Closed doors */
            else if (i_ptr->tval == TV_CLOSED_DOOR) {
                msg_print("There is a closed door blocking your way.");
            }
        }

        /* Free move */
        energy_use = 0;
    }

    /* Hack -- attack invisible monsters on floors */
    else if (c_ptr->m_idx > 1) {

        /* Hitting a monster is disturbing */
        disturb(0, 0);

        /* Handle fear */
        if (!p_ptr->fear) {
            py_attack(y, x);
        }
        else {
            msg_print("You are too afraid!");
        }
    }

    /* Normal movement */
    else {

        /* Move "player" record to the new location */
        move_rec(py, px, y, x);

        /* Check for new panel (redraw map) */
        verify_panel();

#ifdef WDT_TRACK_OPTIONS
        /* Update "tracking" code */
        if (c_ptr->track < 10) c_ptr->track += 3;
#endif
        
        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
        p_ptr->update |= (PU_DISTANCE);


        /* Spontaneous Searching */
        if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos))) {
            search();
        }
    
        /* Continuous Searching */
        if (p_ptr->searching) {
            search();
        }


        /* Handle "objects" */
        if (c_ptr->i_idx) {

            /* Enter a store */
            if (i_ptr->tval == TV_STORE_DOOR) {
                disturb(0, 0);
                store_enter(i_ptr->sval - 1);
            }

            /* Set off a trap */
            else if ((i_ptr->tval == TV_VIS_TRAP) ||
                     (i_ptr->tval == TV_INVIS_TRAP)) {
                disturb(0, 0);
                hit_trap(old_row, old_col);
            }

            /* Pick up (or note) gold and objects */
            else {
                carry(do_pickup);
            }
        }
    }
}



/*
 * Hack -- Do we see a wall?  Used in running.		-CJS-
 */
static int see_wall(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grids are blank */
    if (!in_bounds2(y, x)) return (FALSE);

    /* Non-wall means non-wall */
    if (!(cave[y][x].info & GRID_WALL_MASK)) return (FALSE);

    /* Unknown grids are blank, and thus not walls */
    if (!test_lite_bold(y, x)) return (FALSE);

    /* Default */
    return (TRUE);
}

/*
 * Aux routine -- Do we see anything "interesting"
 */
static int see_nothing(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* Illegal grid are blank */
    if (!in_bounds2(y, x)) return (TRUE);

    /* Unknown grids are blank */
    if (!test_lite_bold(y, x)) return (TRUE);

    /* Default */
    return (FALSE);
}





/* The running algorithm:			-CJS- */


/*
   Overview: You keep moving until something interesting happens.
   If you are in an enclosed space, you follow corners. This is
   the usual corridor scheme. If you are in an open space, you go
   straight, but stop before entering enclosed space. This is
   analogous to reaching doorways. If you have enclosed space on
   one side only (that is, running along side a wall) stop if
   your wall opens out, or your open space closes in. Either case
   corresponds to a doorway.

   What happens depends on what you can really SEE. (i.e. if you
   have no light, then running along a dark corridor is JUST like
   running in a dark room.) The algorithm works equally well in
   corridors, rooms, mine tailings, earthquake rubble, etc, etc.

   These conditions are kept in static memory:
        find_openarea	 You are in the open on at least one
                         side.
        find_breakleft	 You have a wall on the left, and will
                         stop if it opens
        find_breakright	 You have a wall on the right, and will
                         stop if it opens

   To initialize these conditions is the task of x.
   If moving from the square marked @ to the square marked x (in
   the diagrams below), then two adjacent sqares on the left and
   the right (L and R) are considered. If either one is seen to
   be closed, then that side is considered to be closed. If both
   sides are closed, then it is an enclosed (corridor) run.

         LL		L
        @x	       LxR
         RR	       @R

   Looking at more than just the immediate squares is
   significant. Consider the following case. A run along the
   corridor will stop just before entering the center point,
   because a choice is clearly established. Running in any of
   three available directions will be defined as a corridor run.
   Note that a minor hack is inserted to make the angled corridor
   entry (with one side blocked near and the other side blocked
   further away from the runner) work correctly. The runner moves
   diagonally, but then saves the previous direction as being
   straight into the gap. Otherwise, the tail end of the other
   entry would be perceived as an alternative on the next move.

           #.#
          ##.##
          .@...
          ##.##
           #.#

   Likewise, a run along a wall, and then into a doorway (two
   runs) will work correctly. A single run rightwards from @ will
   stop at 1. Another run right and down will enter the corridor
   and make the corner, stopping at the 2.

        #@	  1
        ########### ######
        2	    #
        #############
        #

   After any move, the function area_affect is called to
   determine the new surroundings, and the direction of
   subsequent moves. It takes a location (at which the runner has
   just arrived) and the previous direction (from which the
   runner is considered to have come). Moving one square in some
   direction places you adjacent to three or five new squares
   (for straight and diagonal moves) to which you were not
   previously adjacent.

       ...!	  ...	       EG Moving from 1 to 2.
       .12!	  .1.!		  . means previously adjacent
       ...!	  ..2!		  ! means newly adjacent
                   !!!

   You STOP if you can't even make the move in the chosen
   direction. You STOP if any of the new squares are interesting
   in any way: usually containing monsters or treasure. You STOP
   if any of the newly adjacent squares seem to be open, and you
   are also looking for a break on that side. (i.e. find_openarea
   AND find_break) You STOP if any of the newly adjacent squares
   do NOT seem to be open and you are in an open area, and that
   side was previously entirely open.

   Corners: If you are not in the open (i.e. you are in a
   corridor) and there is only one way to go in the new squares,
   then turn in that direction. If there are more than two new
   ways to go, STOP. If there are two ways to go, and those ways
   are separated by a square which does not seem to be open, then
   STOP.

   Otherwise, we have a potential corner. There are two new open
   squares, which are also adjacent. One of the new squares is
   diagonally located, the other is straight on (as in the
   diagram). We consider two more squares further out (marked
   below as ?).

          .X
         @.?
          #?

   If they are both seen to be closed, then it is seen that no
   benefit is gained from moving straight. It is a known corner.
   To cut the corner, go diagonally, otherwise go straight, but
   pretend you stepped diagonally into that next location for a
   full view next time. Conversely, if one of the ? squares is
   not seen to be closed, then there is a potential choice. We check
   to see whether it is a potential corner or an intersection/room entrance.
   If the square two spaces straight ahead, and the space marked with 'X'
   are both blank, then it is a potential corner and enter if find_examine
   is set, otherwise must stop because it is not a corner.
*/




/*
 * The cycle lists the directions in anticlockwise order, for over  -CJS-
 * two complete cycles.
 */
static int cycle[] = {1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1};

/*
 * The chome array maps a direction on to its position in the cycle.
 */
static int chome[] = {-1, 8, 9, 10, 7, -1, 11, 6, 5, 4};

/*
 * Some global variables
 */
static int find_openarea, find_breakright, find_breakleft, find_prevdir;




/*
 * Determine the next direction for a run, or if we should stop.  -CJS-
 */
static void area_affect(int dir, int y, int x)
{
    int                  newdir = 0, t, inv, check_dir = 0, row, col;
    int         i, max, option, option2;

    cave_type		*c_ptr;
    monster_type	*m_ptr;


    /* Hack -- blind yields disturb */
    if (p_ptr->blind) {

        disturb(0,0);
    }
    
    /* Look around */
    else {
    
        option = 0;
        option2 = 0;
        dir = find_prevdir;
        max = (dir & 0x01) + 1;


        /* Look at every newly adjacent square. */
        for (i = -max; i <= max; i++) {

            newdir = cycle[chome[dir] + i];

            row = y + ddy[newdir];
            col = x + ddx[newdir];

            c_ptr = &cave[row][col];
            m_ptr = &m_list[c_ptr->m_idx];

            /* Hack -- notice visible monsters */
            if ((c_ptr->m_idx > 1) && (m_ptr->ml)) {
                disturb(0,0);
                return;
            }


            /* Assume the new grid cannot be seen */
            inv = TRUE;

            /* Can we "see" (or "remember") the adjacent grid? */
            if (test_lite_bold(row, col)) {

                /* Most (visible) objects stop the running */
                if (c_ptr->i_idx) {

                    /* Examine the object */
                    t = i_list[c_ptr->i_idx].tval;
                    if ((t != TV_INVIS_TRAP) &&
                        (t != TV_SECRET_DOOR) &&
                        (t != TV_UP_STAIR || !find_ignore_stairs) &&
                        (t != TV_DOWN_STAIR || !find_ignore_stairs) &&
                        (t != TV_OPEN_DOOR || !find_ignore_doors)) {

                        disturb(0,0);
                        return;
                    }
                }

                /* The grid is "visible" */
                inv = FALSE;
            }


            /* If cannot see the grid, assume it is clear */
            if (inv || floor_grid_bold(row, col)) {

                /* Certain somethings */
                if (find_openarea) {
                    if (i < 0) {
                        if (find_breakright) {
                            disturb(0,0);
                            return;
                        }
                    }
                    else if (i > 0) {
                        if (find_breakleft) {
                            disturb(0,0);
                            return;
                        }
                    }
                }

                /* The first new direction. */
                else if (!option) {
                    option = newdir;
                }

                /* Three new directions. Stop running. */
                else if (option2) {
                    disturb(0,0);
                    return;
                }

                /* If not adjacent to prev, STOP */
                else if (option != cycle[chome[dir] + i - 1]) {
                    disturb(0,0);
                    return;
                }

                /* Two adjacent choices. Make option2 the diagonal, */
                /* and remember the other diagonal adjacent to the  */
                /* first option. */
                else {
                    if ((newdir & 0x01) == 1) {
                        check_dir = cycle[chome[dir] + i - 2];
                        option2 = newdir;
                    }
                    else {
                        check_dir = cycle[chome[dir] + i + 1];
                        option2 = option;
                        option = newdir;
                    }
                }
            }

            /* We see an obstacle.  Break to one side. */
            /* In open area, STOP if on a side previously open. */
            else if (find_openarea) {
                if (i < 0) {
                    if (find_breakleft) {
                        disturb(0,0);
                        return;
                    }
                    find_breakright = TRUE;
                }
                else if (i > 0) {
                    if (find_breakright) {
                        disturb(0,0);
                        return;
                    }
                    find_breakleft = TRUE;
                }
            }
        }


        /* choose a direction. */
        if (find_openarea == FALSE) {

            /* There is only one option, or if two, then we always examine */
            /* potential corners and never cut known corners, so you step */
            /* into the straight option. */
            if (option2 == 0 || (find_examine && !find_cut)) {
                if (option != 0) command_dir = option;
                if (option2 == 0) find_prevdir = option;
                else find_prevdir = option2;
            }

            /* Two options! */
            else {

                /* Get next location */
                row = y + ddy[option];
                col = x + ddx[option];

                /* Don't see that it is closed off. */
                /* This could be a potential corner or an intersection. */
                if (!see_wall(option, row, col) ||
                    !see_wall(check_dir, row, col)) {

                    /* Can not see anything ahead and in the direction we */
                    /* are turning, assume that it is a potential corner. */
                    if (find_examine &&
                        see_nothing(option, row, col) &&
                        see_nothing(option2, row, col)) {
                        command_dir = option;
                        find_prevdir = option2;
                    }

                    /* STOP: we are next to an intersection or a room */
                    else {
                        disturb(0,0);
                        return;
                    }
                }

                /* This corner is seen to be enclosed; we cut the corner. */
                else if (find_cut) {
                    command_dir = option2;
                    find_prevdir = option2;
                }

                /* This corner is seen to be enclosed, and we */
                /* deliberately go the long way. */
                else {
                    command_dir = option;
                    find_prevdir = option2;
                }
            }
        }
    }
}



/*
 * Walk one step along the current running direction
 * Apply confusion, stop running if "fully" confused
 * Update the current "find" info via "area_affect()"
 */
void find_step(void)
{
    int dir;

    /* Get the desired direction */
    dir = command_dir;

    /* Apply confusion */
    confuse_dir(&dir, 0x02);

    /* Confusion cancels running */
    if (dir != command_dir) disturb(0,0);

    /* Move the player, using the "pickup" flag */
    move_player(dir, always_pickup);

    /* Important -- Handle stuff */
    handle_stuff();

    /* Check to see if he should stop running */
    if (find_flag) area_affect(dir, py, px);

    /* Hack -- run out of breath */
    if (find_flag && (--command_arg <= 0)) {
        msg_print("You stop running to catch your breath.");
        disturb(0,0);
    }

    /* Hack -- refresh */
    if (fresh_find) {

        /* Hack -- Hilite the player */
        move_cursor_relative(py, px);
            
        /* Refresh */
        Term_fresh();
    }
}


/*
 * Initialize the running algorithm, do NOT take any steps.
 *
 * Note that we use "command_arg" as a "limit" on running time.
 * If "command_arg" is zero (as usual) then impose no limit.
 */
void find_init()
{
    int          dir, row, col, deepleft, deepright;
    int		i, shortleft, shortright;


    /* Start running */
    find_flag = 1;


    /* Extract the desired direction */
    dir = command_dir;

    /* Find the destination grid */
    row = py + ddy[dir];
    col = px + ddx[dir];

    /* XXX Un-indent */
    if (TRUE) {

        find_breakright = find_breakleft = FALSE;
        find_prevdir = dir;

        /* Look around unless blind */
        if (!p_ptr->blind) {

            i = chome[dir];
            deepleft = deepright = FALSE;
            shortright = shortleft = FALSE;

            if (see_wall(cycle[i + 1], py, px)) {
                find_breakleft = TRUE;
                shortleft = TRUE;
            }
            else if (see_wall(cycle[i + 1], row, col)) {
                find_breakleft = TRUE;
                deepleft = TRUE;
            }

            if (see_wall(cycle[i - 1], py, px)) {
                find_breakright = TRUE;
                shortright = TRUE;
            }
            else if (see_wall(cycle[i - 1], row, col)) {
                find_breakright = TRUE;
                deepright = TRUE;
            }

            if (find_breakleft && find_breakright) {

                find_openarea = FALSE;

                /* a hack to allow angled corridor entry */
                if (dir & 0x01) {
                    if (deepleft && !deepright) {
                        find_prevdir = cycle[i - 1];
                    }
                    else if (deepright && !deepleft) {
                        find_prevdir = cycle[i + 1];
                    }
                }

                /* else if there is a wall two spaces ahead and seem to be in a */
                /* corridor, then force a turn into the side corridor, must be */
                /* moving straight into a corridor here */

                else if (see_wall(cycle[i], row, col)) {
                    if (shortleft && !shortright) {
                        find_prevdir = cycle[i - 2];
                    }
                    else if (shortright && !shortleft) {
                        find_prevdir = cycle[i + 2];
                    }
                }
            }
            else {
                find_openarea = TRUE;
            }
        }
    }
}


/*
 * Stop running.  Fix the lights and monsters.
 */
void end_find()
{
    /* Were we running? */
    if (find_flag) {

        /* Cancel the running */
        find_flag = 0;

        /* Hack -- Redraw the player */
        lite_spot(py, px);

        /* Fix the lite radius */
        if (view_reduce_lite) extract_cur_lite();
        
        /* Update the view/lite */
        if (view_reduce_view) p_ptr->update |= (PU_VIEW | PU_MONSTERS);
        if (view_reduce_lite) p_ptr->update |= (PU_LITE | PU_MONSTERS);
    }
}



