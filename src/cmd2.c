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
 * Hack -- Check if a level is a "quest" level
 */
bool is_quest(int level)
{
    int i;

    /* Town is never a quest */
    if (!level) return (FALSE);

    /* Check quests */
    for (i = 0; i < MAX_Q_IDX; i++) {
    
        /* Check for quest */
        if (q_list[i].level == level) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}




/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
static int get_coin_type(monster_race *r_ptr)
{
    cptr name = (r_name + r_ptr->name);

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
void monster_death(int m_idx)
{
    int			i, j, y, x, ny, nx;

    int			dump_item = 0;
    int			dump_gold = 0;

    int			number = 0;
    int			total = 0;

    cave_type		*c_ptr;

    monster_type	*m_ptr = &m_list[m_idx];

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    bool visible = (m_ptr->ml || (r_ptr->flags1 & RF1_UNIQUE));

    bool good = (r_ptr->flags1 & RF1_DROP_GOOD) ? TRUE : FALSE;
    bool great = (r_ptr->flags1 & RF1_DROP_GREAT) ? TRUE : FALSE;

    bool do_gold = (!(r_ptr->flags1 & RF1_ONLY_ITEM));
    bool do_item = (!(r_ptr->flags1 & RF1_ONLY_GOLD));

    int force_coin = get_coin_type(r_ptr);


    /* Get the location */
    y = m_ptr->fy;
    x = m_ptr->fx;

    /* Determine how much we can drop */
    if ((r_ptr->flags1 & RF1_DROP_60) && (rand_int(100) < 60)) number++;
    if ((r_ptr->flags1 & RF1_DROP_90) && (rand_int(100) < 90)) number++;
    if (r_ptr->flags1 & RF1_DROP_1D2) number += damroll(1, 2);
    if (r_ptr->flags1 & RF1_DROP_2D2) number += damroll(2, 2);
    if (r_ptr->flags1 & RF1_DROP_3D2) number += damroll(3, 2);
    if (r_ptr->flags1 & RF1_DROP_4D2) number += damroll(4, 2);

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
                if (player_can_see_bold(ny, nx)) dump_gold++;
            }

            /* Place Object */
            else {
                place_object(ny, nx, good, great);
                if (player_can_see_bold(ny, nx)) dump_item++;
            }

            /* Reset the object level */
            object_level = dun_level;

            /* Reset "coin" type */
            coin_type = 0;

            /* Notice */
            note_spot(ny, nx);
            
            /* Display */
            lite_spot(ny, nx);

            /* Under the player */
            if ((ny == py) && (nx == px)) {
                msg_print("You feel something roll beneath your feet.");
            }

            break;
        }
    }


    /* Take note of any dropped treasure */
    if (visible && (dump_item || dump_gold)) {

        /* Take notes on treasure */
        lore_treasure(m_idx, dump_item, dump_gold);
    }


    /* Mega-Hack -- drop "winner" treasures */
    if (r_ptr->flags1 & RF1_DROP_CHOSEN) {

        /* Hack -- an "object holder" */
        inven_type prize;


        /* Mega-Hack -- Prepare to make "Grond" */
        invcopy(&prize, lookup_kind(TV_HAFTED, SV_GROND));

        /* Mega-Hack -- Mark this item as "Grond" */
        prize.name1 = ART_GROND;

        /* Mega-Hack -- Actually create "Grond" */
        apply_magic(&prize, -1, TRUE, TRUE, TRUE);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);


        /* Mega-Hack -- Prepare to make "Morgoth" */
        invcopy(&prize, lookup_kind(TV_CROWN, SV_MORGOTH));

        /* Mega-Hack -- Mark this item as "Morgoth" */
        prize.name1 = ART_MORGOTH;

        /* Mega-Hack -- Actually create "Morgoth" */
        apply_magic(&prize, -1, TRUE, TRUE, TRUE);

        /* Drop it in the dungeon */
        drop_near(&prize, -1, y, x);
    }


    /* Only process "Quest Monsters" */
    if (!(r_ptr->flags1 & RF1_QUESTOR)) return;


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

        /* Delete any old object XXX XXX XXX */
        delete_object(y, x);

        /* Explain the stairway */
        msg_print("A magical stairway appears...");

        /* Access the grid */
        c_ptr = &cave[y][x];
        
        /* Create stairs down */
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x07);

        /* Note the spot */
        note_spot(y, x);

        /* Draw the spot */
        lite_spot(y, x);

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
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 */
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note)
{
    monster_type	*m_ptr = &m_list[m_idx];

    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    s32b		new_exp, new_exp_frac;


    /* Redraw (later) if needed */
    if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);


    /* Wake it up */
    m_ptr->csleep = 0;

    /* Hurt it */
    m_ptr->hp -= dam;

    /* It is dead now */
    if (m_ptr->hp < 0) {

        char m_name[80];

        /* Extract monster name */
        monster_desc(m_name, m_ptr, 0);

        /* Make a sound */
        sound(SOUND_KILL);

        /* Death by Missile/Spell attack */
        if (note) {
            msg_format("%^s%s", m_name, note);
        }

        /* Death by physical attack -- invisible monster */
        else if (!m_ptr->ml) {
            msg_format("You have killed %s.", m_name);
        }

        /* Death by Physical attack -- non-living monster */
        else if ((r_ptr->flags3 & RF3_DEMON) ||
                 (r_ptr->flags3 & RF3_UNDEAD) ||
                 (r_ptr->flags2 & RF2_STUPID) ||
                 (strchr("Evg", r_ptr->r_char))) {
            msg_format("You have destroyed %s.", m_name);
        }

        /* Death by Physical attack -- living monster */
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
        monster_death(m_idx);

        /* When the player kills a Unique, it stays dead */
        if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 0;

        /* XXX XXX XXX Mega-Hack -- allow another ghost later */
        if (m_ptr->r_idx == MAX_R_IDX-1) r_ptr->max_num = 1;

        /* Recall even invisible uniques or winners */
        if (m_ptr->ml || (r_ptr->flags1 & RF1_UNIQUE)) {

            /* Count kills this life */
            if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

            /* Count kills in all lives */
            if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

            /* Hack -- Auto-recall */
            recent_track(m_ptr->r_idx);
        }

        /* Delete the monster */
        delete_monster_idx(m_idx);

        /* Not afraid */
        (*fear) = FALSE;

        /* Monster is dead */
        return (TRUE);
    }


#ifdef ALLOW_FEAR

    /* Mega-Hack -- Pain cancels fear */
    if (m_ptr->monfear && (dam > 0)) {

        int tmp = randint(dam);

        /* Cure a little fear */        
        if (tmp < m_ptr->monfear) {
        
            /* Reduce fear */
            m_ptr->monfear -= tmp;
        }

        /* Cure all the fear */
        else {

            /* Cure fear */
            m_ptr->monfear = 0;

            /* No more fear */
            (*fear) = FALSE;
        }
    }

    /* Sometimes a monster gets scared by damage */
    if (!m_ptr->monfear && !(r_ptr->flags3 & RF3_NO_FEAR)) {

        int		percentage;

        /* Percentage of fully healthy */
        percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

        /*
         * Run (sometimes) if at 10% or less of max hit points,
         * or (usually) when hit for half its current hit points
         */
        if (((percentage <= 10) && (rand_int(10) < percentage)) ||
            ((dam >= m_ptr->hp) && (rand_int(100) < 80))) {

            /* Hack -- note fear */
            (*fear) = TRUE;

            /* XXX XXX XXX Hack -- Add some timed fear */
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
 * Factor in weapon weight, total plusses, player level.
 */
static int critical_norm(int weight, int plus, int dam)
{
    int i, k;

    /* Extract "blow" power */
    i = (weight + ((p_ptr->to_h + plus) * 5) + (p_ptr->lev * 3));

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
    i = (weight + ((p_ptr->to_h + plus) * 4) + (p_ptr->lev * 2));

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
 *
 * Hack -- this function uses "chance" to determine if it should produce
 * some form of "description" of the drop event (under the player).
 *
 * This function should probably be broken up into a function to determine
 * a "drop location", and several functions to actually "drop" an object.
 *
 * XXX XXX XXX Consider allowing objects to combine on the ground.
 */
void drop_near(inven_type *i_ptr, int chance, int y, int x)
{
    int		k, d, ny, nx, y1, x1, i_idx;

    cave_type	*c_ptr;

    bool flag = FALSE;


    /* Start at the drop point */
    ny = y1 = y;  nx = x1 = x;

    /* See if the object "survives" the fall */
    if (artifact_p(i_ptr) || (rand_int(100) >= chance)) {

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
            y1 = ny; x1 = nx;

            /* Get the cave grid */
            c_ptr = &cave[ny][nx];

            /* XXX XXX XXX */
            
            /* Nothing here?  Use it */
            if (!(c_ptr->i_idx)) flag = TRUE;

            /* After trying 99 places, crush any (normal) object */
            else if ((k>99) && valid_grid(ny,nx)) flag = TRUE;
        }

        /* Hack -- Artifacts will destroy ANYTHING to stay alive */
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

        /* Assume fails */
        flag = FALSE;

        /* XXX XXX XXX */
        
        /* Crush anything under us (for artifacts) */
        delete_object(ny,nx);

        /* Make a new object */
        i_idx = i_pop();

        /* Success */
        if (i_idx) {

            /* Structure copy */
            i_list[i_idx] = *i_ptr;

            /* Access */
            i_ptr = &i_list[i_idx];

            /* Locate */
            i_ptr->iy = ny;
            i_ptr->ix = nx;

            /* Place */
            c_ptr = &cave[ny][nx];
            c_ptr->i_idx = i_idx;
            
            /* Note the spot */
            note_spot(ny, nx);

            /* Draw the spot */
            lite_spot(ny, nx);

            /* Sound */
            sound(SOUND_DROP);

            /* Mega-Hack -- no message if "dropped" by player */
            /* Message when an object falls under the player */
            if (chance && (ny == py) && (nx == px)) {
                msg_print("You feel something roll beneath your feet.");
            }

            /* Success */
            flag = TRUE;
        }
    }


    /* Poor little object */
    if (!flag) {

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
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
static int test_hit_norm(int chance, int ac, int vis)
{
    int k;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Wimpy attack never hits */
    if (chance <= 0) return (FALSE);

    /* Penalize invisible targets */
    if (!vis) chance = (chance + 1) / 2;

    /* Power must defeat armor */
    if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}


/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
static int test_hit_fire(int chance, int ac, int vis)
{
    int k;

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- Instant miss or hit */
    if (k < 10) return (k < 5);

    /* Never hit */
    if (chance <= 0) return (FALSE);

    /* Invisible monsters are harder to hit */
    if (!vis) chance = (chance + 1) / 2;

    /* Power competes against armor */
    if (rand_int(chance) < (ac * 3 / 4)) return (FALSE);

    /* Assume hit */
    return (TRUE);
}


/*
 * Decreases players hit points and sets death flag if necessary
 */
void take_hit(int damage, cptr hit_from)
{
    int old_chp = p_ptr->chp;

    int warning = (p_ptr->mhp * hitpoint_warn / 10);

    
    /* Mega-Hack -- Apply "invulnerability" */
    if (p_ptr->invuln && (damage < 9000)) return;

    /* Hurt the player */
    p_ptr->chp -= damage;

    /* Display the hitpoints */
    p_ptr->redraw |= (PR_HP);

    /* Dead player */
    if (p_ptr->chp < 0) {

        /* Cheat -- avoid death */
        if (wizard || cheat_live) {
        
            /* Allow cheating */
            if (!get_check("Die? ")) {

                /* Mark savefile */
                noscore |= 0x0001;

                /* Message */
                msg_print("You invoke wizard mode and cheat death.");

                /* Restore hitpoints */
                p_ptr->chp = p_ptr->mhp;
                p_ptr->chp_frac = 0;
                
                /* Done */
                return;
            }
        }

        /* New death */
        if (!death) {

            /* No longer a winner */
            total_winner = FALSE;

            /* Stop playing */
            alive = FALSE;
            
            /* Note death */
            death = TRUE;

            /* Note cause of death */
            (void)strcpy(died_from, hit_from);

            /* Sound */
            sound(SOUND_DEATH);

            /* Hack -- Note death */
            msg_print("You die.");
            msg_print(NULL);
        }

        /* Dead */
        return;
    }

    /* Hack -- hitpoint warning */
    if (warning && (p_ptr->chp <= warning)) {

        /* Hack -- bell on first notice */
        if (alert_hitpoint && (old_chp > warning)) bell();
        
        /* Message */
        msg_print("*** LOW HITPOINT WARNING! ***");
        msg_print(NULL);
    }
}





/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands are x3, except Acid (x2) and Elec (x5).
 *
 * Note that most slays are x3, except Animal (x2) and Evil (x2).
 *
 * Note that kill dragon ("execute dragon") is x5.
 */
static int tot_dam_aux(inven_type *i_ptr, int tdam, monster_type *m_ptr)
{
    int mult = 1;

    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    u32b f1, f2, f3;
    
    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);

    /* Some "weapons" and "ammo" do extra damage */
    switch (i_ptr->tval) {

      case TV_SHOT:
      case TV_ARROW:
      case TV_BOLT:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
      case TV_DIGGING:

        /* Slay Animal */
        if ((f1 & TR1_SLAY_ANIMAL) &&
            (r_ptr->flags3 & RF3_ANIMAL)) {

            if (mult < 2) mult = 2;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_ANIMAL;
        }

        /* Slay Evil */
        if ((f1 & TR1_SLAY_EVIL) &&
            (r_ptr->flags3 & RF3_EVIL)) {

            if (mult < 2) mult = 2;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_EVIL;
        }

        /* Slay Undead */
        if ((f1 & TR1_SLAY_UNDEAD) &&
            (r_ptr->flags3 & RF3_UNDEAD)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_UNDEAD;
        }

        /* Slay Demon */
        if ((f1 & TR1_SLAY_DEMON) &&
            (r_ptr->flags3 & RF3_DEMON)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DEMON;
        }

        /* Slay Orc */
        if ((f1 & TR1_SLAY_ORC) &&
            (r_ptr->flags3 & RF3_ORC)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_ORC;
        }

        /* Slay Troll */
        if ((f1 & TR1_SLAY_TROLL) &&
            (r_ptr->flags3 & RF3_TROLL)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_TROLL;
        }

        /* Slay Giant */
        if ((f1 & TR1_SLAY_GIANT) &&
            (r_ptr->flags3 & RF3_GIANT)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_GIANT;
        }

        /* Slay Dragon  */
        if ((f1 & TR1_SLAY_DRAGON) &&
            (r_ptr->flags3 & RF3_DRAGON)) {

            if (mult < 3) mult = 3;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DRAGON;
        }

        /* Execute Dragon */
        if ((f1 & TR1_KILL_DRAGON) &&
            (r_ptr->flags3 & RF3_DRAGON)) {

            if (mult < 5) mult = 5;
            if (m_ptr->ml) r_ptr->r_flags3 |= RF3_DRAGON;
        }


        /* Brand (Acid) */
        if (f1 & TR1_BRAND_ACID) {

            /* Notice immunity */
            if (r_ptr->flags3 & RF3_IM_ACID) {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_ACID;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 2) mult = 2;
            }
        }

        /* Brand (Elec) */
        if (f1 & TR1_BRAND_ELEC) {

            /* Notice immunity */
            if (r_ptr->flags3 & RF3_IM_ELEC) {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_ELEC;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 5) mult = 5;
            }
        }

        /* Brand (Fire) */
        if (f1 & TR1_BRAND_FIRE) {

            /* Notice immunity */
            if (r_ptr->flags3 & RF3_IM_FIRE) {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_FIRE;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 3) mult = 3;
            }
        }

        /* Brand (Cold) */
        if (f1 & TR1_BRAND_COLD) {

            /* Notice immunity */
            if (r_ptr->flags3 & RF3_IM_COLD) {
                if (m_ptr->ml) r_ptr->r_flags3 |= RF3_IM_COLD;
            }

            /* Otherwise, take the damage */
            else {
                if (mult < 3) mult = 3;
            }
        }
    }


    /* Return the total damage */
    return (tdam * mult);
}


/*
 * Player attacks a (poor, defenseless) creature	-RAK-	
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x)
{
    int			num = 0, k, bonus, chance;

    cave_type		*c_ptr = &cave[y][x];
    
    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    inven_type		*i_ptr;

    u32b		f1, f2, f3;

    char		m_name[80];

    bool		fear = FALSE;
    
    bool		can_quake = FALSE;
    bool		do_quake = FALSE;


    /* Disturb the player */
    disturb(0, 0);


    /* Disturb the monster */
    m_ptr->csleep = 0;


    /* Extract monster name (or "it") */
    monster_desc(m_name, m_ptr, 0);


    /* Auto-Recall if possible and visible */
    if (m_ptr->ml) recent_track(m_ptr->r_idx);

    /* Track a new monster */
    if (m_ptr->ml) health_track(c_ptr->m_idx);


    /* Handle player fear */
    if (p_ptr->afraid) {

        /* Message */
        msg_format("You are too afraid to attack %s!", m_name);

        /* Done */
        return;
    }


    /* Access the weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Extract the flags XXX XXX XXX */
    inven_flags(i_ptr, &f1, &f2, &f3);

    /* Check for quake potential */
    if (f1 & TR1_IMPACT) can_quake = TRUE;

    /* Calculate the "attack quality" */
    bonus = p_ptr->to_h + i_ptr->to_h;
    chance = (p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ));


    /* Attack once for each legal blow */
    while (num++ < p_ptr->num_blow) {

        /* Test for hit */
        if (test_hit_norm(chance, r_ptr->ac, m_ptr->ml)) {

            /* Sound */
            sound(SOUND_HIT);

            /* Message */
            msg_format("You hit %s.", m_name);

            /* Hack -- bare hands do one damage */
            k = 1;

            /* Handle normal weapon */
            if (i_ptr->k_idx) {
                k = damroll(i_ptr->dd, i_ptr->ds);
                k = tot_dam_aux(i_ptr, k, m_ptr);
                if (can_quake && (k > 50)) do_quake = TRUE;
                k = critical_norm(i_ptr->weight, i_ptr->to_h, k);
                k += i_ptr->to_d;
            }

            /* Apply the player damage bonuses */
            k += p_ptr->to_d;

            /* No negative damage */
            if (k < 0) k = 0;

            /* Complex message */
            if (wizard) {
                msg_format("You do %d (out of %d) damage.", k, m_ptr->hp);
            }

            /* Damage, check for fear and death */
            if (mon_take_hit(c_ptr->m_idx, k, &fear, NULL)) break;

            /* Confusion attack */
            if (p_ptr->confusing) {

                /* Cancel glowing hands */
                p_ptr->confusing = FALSE;

                /* Message */
                msg_print("Your hands stop glowing.");

                /* Confuse the monster */
                if (r_ptr->flags3 & RF3_NO_CONF) {
                    if (m_ptr->ml) r_ptr->r_flags3 |= RF3_NO_CONF;
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
        }

        /* Player misses */
        else {

            /* Sound */
            sound(SOUND_MISS);

            /* Message */
            msg_format("You miss %s.", m_name);
        }
    }


    /* Hack -- delay fear messages */
    if (fear && m_ptr->ml) {

        /* Sound */
        sound(SOUND_FLEE);

        /* Message */
        msg_format("%^s flees in terror!", m_name);
    }


    /* Mega-Hack -- apply earthquake brand */
    if (do_quake) earthquake(py, px, 10);
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
bool twall(int y, int x)
{
    cave_type	*c_ptr = &cave[y][x];

    /* Paranoia -- Require a wall or door or some such */
    if ((c_ptr->feat & 0x3F) < 0x20) return (FALSE);

    /* Remove the feature */
    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);

    /* Forget the "field mark" */
    c_ptr->feat &= ~CAVE_MARK;

    /* Notice */
    note_spot(y, x);

    /* Redisplay the grid */
    lite_spot(y, x);

    /* Update some things */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

    /* Result */
    return (TRUE);
}




/*** Targetting Code ***/


/*
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and known
 * not to be a hallucination.  It does NOT have to be "projectable()".
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */



/*
 * Determine is a monster makes a reasonable target
 */
bool target_able(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

    /* Monster MUST be visible */
    if (!m_ptr->ml) return (FALSE);

    /* Hack -- no targeting hallucinations */
    if (p_ptr->image) return (FALSE);

    /* XXX XXX XXX Hack -- Never target trappers */
    /* if (CLEAR_ATTR && CLEAR_CHAR) return (FALSE); */

    /* Assume okay */
    return (TRUE);
}




/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
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
 * Sorting hook -- comp function -- see below
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
    byte *x = (byte*)(u);
    byte *y = (byte*)(v);

    int da, db;

    /* Distance to first point */
    da = distance(px, py, x[a], y[a]);
        
    /* Distance to second point */
    db = distance(px, py, x[b], y[b]);

    /* Compare the distances */
    return (da <= db);
}


/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
    byte *x = (byte*)(u);
    byte *y = (byte*)(v);

    byte temp;
    
    /* Swap "x" */
    temp = x[a];
    x[a] = x[b];
    x[b] = temp;
    
    /* Swap "y" */
    temp = y[a];
    y[a] = y[b];
    y[b] = temp;
}




/*
 * Hack -- help "select" a location (see below)
 */
s16b target_pick(int y1, int x1, int dy, int dx)
{
    int i, v;

    int x2, y2, x3, y3, x4, y4;

    int b_i = -1, b_v = 9999;


    /* Scan the locations */
    for (i = 0; i < temp_n; i++) {

        /* Point 2 */
        x2 = temp_x[i];
        y2 = temp_y[i];

        /* Directed distance */
        x3 = (x2 - x1);
        y3 = (y2 - y1);

        /* Verify quadrant */
        if (dx && (x3 * dx <= 0)) continue;
        if (dy && (y3 * dy <= 0)) continue;

        /* Absolute distance */
        x4 = ABS(x3);
        y4 = ABS(y3);

        /* Verify quadrant */
        if (dy && !dx && (x4 > y4)) continue;
        if (dx && !dy && (y4 > x4)) continue;

        /* Approximate Double Distance */
        v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

        /* XXX XXX XXX Penalize location */
        
        /* Track best */
        if ((b_i >= 0) && (v >= b_v)) continue;
        
        /* Track best */
        b_i = i; b_v = v;
    }

    /* Result */
    return (b_i);
}


/*
 * Set a new target.  This code can be called from "get_aim_dir()"
 *
 * The target must be on the current panel.  Consider the use of
 * "panel_bounds()" to allow "off-panel" targets.  XXX XXX XXX
 *
 * That is, consider the possibility of "auto-scrolling" the screen
 * while the cursor moves around.  This may require changes in the
 * "update_mon()" code to allow "visibility" even if off panel.
 *
 * Hack -- targetting an "outer border grid" may be dangerous,
 * so this is not currently allowed.
 *
 * You can now use the direction keys to move among legal monsters,
 * just like the new "look" function allows the use of direction
 * keys to move amongst interesting locations.
 */
bool target_set()
{
    int		i, d, m;

    int		y = py;
    int		x = px;

    bool	done = FALSE;

    bool	flag = TRUE;

    char	query;

    char	out_val[160];

    cave_type		*c_ptr;
    
    monster_type	*m_ptr;
    monster_race	*r_ptr;


    /* Go ahead and turn off target mode */
    target_who = 0;

    /* Turn off health tracking */
    health_track(0);


    /* Reset "temp" array */
    temp_n = 0;

    /* Collect "target-able" monsters */
    for (i = 1; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Skip "dead" monsters */
        if (!m_ptr->r_idx) continue;

        /* Ignore "unreasonable" monsters */
        if (!target_able(i)) continue;

        /* Save this monster index */
        temp_x[temp_n] = m_ptr->fx;
        temp_y[temp_n] = m_ptr->fy;
        temp_n++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;
    
    /* Sort the positions */
    ang_sort(temp_x, temp_y, temp_n);

    
    /* Choose "closest" monster */
    m = target_pick(py, px, 0, 0);


    /* Interact */
    while (!done) {

        /* Target monsters */
        if (flag && temp_n) {

            y = temp_y[m];
            x = temp_x[m];

            c_ptr = &cave[y][x];

            m_ptr = &m_list[c_ptr->m_idx];
            r_ptr = &r_info[m_ptr->r_idx];

            /* Hack -- Track that monster race */
            recent_track(m_ptr->r_idx);

            /* Hack -- Track that monster */
            health_track(c_ptr->m_idx);

            /* Hack -- handle stuff */
            handle_stuff();

            /* Describe, prompt for recall */
            sprintf(out_val,
                    "%s [(t)arget, (o)ffset, (p)osition, (r)ecall, or (q)uit]",
                    (r_name + r_ptr->name));
            prt(out_val,0,0);

            /* Get a command */
            move_cursor_relative(y,x);
            query = inkey();

            /* Optional recall */
            while (query == 'r') {

                /* Recall on screen */
                Term_save();
                screen_roff(m_ptr->r_idx);
                Term_addstr(-1, TERM_WHITE, "  --pause--");
                query = inkey();
                Term_load();

                /* Hack -- ask again */
                if (query == ' ') {

                    /* Get a new command */
                    move_cursor_relative(y,x);
                    query = inkey();
                }
            }

            /* Hack -- cancel tracking */
            health_track(0);

            /* Assume no "direction" */
            d = 0;
            
            /* Analyze (non "recall") command */
            switch (query) {

                case ESCAPE:
                case 'q':
                    done = TRUE;
                    break;

                case 't':
                case '.':
                case '5': 
                case '0':
                    target_who = c_ptr->m_idx;
                    target_row = y;
                    target_col = x;
                    done = TRUE;
                    break;

                case '*':
                case ' ':
                    if (++m == temp_n) m = 0;
                    break;

                case '-':
                    if (m-- == 0) m = temp_n - 1;
                    break;

                case 'p':
                    y = py;
                    x = px;

                case 'o':
                    flag = !flag;
                    break;

                case 'm':
                    break;

                case '1': case 'b': d = 1; break;
                case '2': case 'j': d = 2; break;
                case '3': case 'n': d = 3; break;
                case '4': case 'h': d = 4; break;
                case '6': case 'l': d = 6; break;
                case '7': case 'y': d = 7; break;
                case '8': case 'k': d = 8; break;
                case '9': case 'u': d = 9; break;

                default:
                    bell();
            }

            /* Hack -- move around */
            if (d) {

                /* Find a new monster */
                i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

                /* Use that monster */
                if (i >= 0) m = i;
            }
        }
    
        /* Target locations */
        else {
        
            /* Now try a location */
            prt("Use cursor to designate target. [(t)arget]",0,0);

            /* Light up the current location */
            move_cursor_relative(y, x);

            /* Get a command, and convert it to standard form */
            query = inkey();

            /* Assume no direction */
            d = 0;
            
            /* Analyze the keypress */
            switch (query) {

                case ESCAPE:
                case 'q':
                    done = TRUE;
                    break;

                case '5':
                case '.':
                case 't':
                case '0':
                    target_who = -1;
                    target_row = y;
                    target_col = x;
                    done = TRUE;
                    break;

                case 'm':
                    flag = !flag;
                    break;

                case 'p':
                    y = py;
                    x = px;

                case 'o':
                    break;

                case '1': case 'b': d = 1; break;
                case '2': case 'j': d = 2; break;
                case '3': case 'n': d = 3; break;
                case '4': case 'h': d = 4; break;
                case '6': case 'l': d = 6; break;
                case '7': case 'y': d = 7; break;
                case '8': case 'k': d = 8; break;
                case '9': case 'u': d = 9; break;

                default:
                    bell();
            }

            /* Handle "direction" */
            if (d) x += ddx[d];
            if (d) y += ddy[d];
            
            /* Hack -- Verify x */
            if ((x>=cur_wid-1) || (x>panel_col_max)) x--;
            else if ((x<=0) || (x<panel_col_min)) x++;

            /* Hack -- Verify y */
            if ((y>=cur_hgt-1) || (y>panel_row_max)) y--;
            else if ((y<=0) || (y<panel_row_min)) y++;
        }
    }

    /* Forget */
    temp_n = 0;

    /* Clear the top line */
    prt("", 0, 0);
    
    /* Success */
    if (target_who) return (TRUE);

    /* Nope */
    return (FALSE);
}



/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 */
bool get_aim_dir(int *dp)
{
    int		dir;

    char        command;

    cptr	p;


    /* Global direction */
    dir = command_dir;

    /* Hack -- auto-target */
    if (use_old_target && target_okay()) dir = 5;

    /* Ask until satisfied */
    while (!dir) {

        /* Choose a prompt */
        if (!target_okay()) {
            p = "Direction ('*' to choose a target, Escape to cancel)? ";
        }
        else {
            p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
        }

        /* Get a command (or Cancel) */
        if (!get_com(p, &command)) break;

        /* Convert various keys to "standard" keys */
        switch (command) {

            /* Various directions */
            case 'B': case 'b': case '1': dir = 1; break;
            case 'J': case 'j': case '2': dir = 2; break;
            case 'N': case 'n': case '3': dir = 3; break;
            case 'H': case 'h': case '4': dir = 4; break;
            case 'L': case 'l': case '6': dir = 6; break;
            case 'Y': case 'y': case '7': dir = 7; break;
            case 'K': case 'k': case '8': dir = 8; break;
            case 'U': case 'u': case '9': dir = 9; break;

            /* Use current target */
            case 'T': case 't': case '.': case '5': dir = 5; break;

            /* Set new target */
            case '*': if (target_set()) dir = 5; break;
        }

        /* Verify requested targets */
        if ((dir == 5) && !target_okay()) dir = 0;

        /* Error */
        if (!dir) bell();
    }

    /* Save the direction */
    *dp = dir;

    /* No direction */
    if (!dir) return (FALSE);

    /* Save the direction */
    command_dir = dir;

    /* Check for confusion */
    if (p_ptr->confused) {

        /* Warn the user */
        msg_print("You are confused.");

        /* Hack -- Random direction */
        *dp = ddd[rand_int(8)];
    }

    /* A "valid" direction was entered */
    return (TRUE);
}



/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "command_dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
bool get_rep_dir(int *dp)
{
    int dir;


    /* Global direction */
    dir = command_dir;

    /* Get a direction */
    while (!dir) {

        char ch;

        /* Get a command (or Cancel) */
        if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

        /* Convert various keys to "standard" keys */
        switch (ch) {

            /* Convert roguelike directions */
            case 'B': case 'b': case '1': dir = 1; break;
            case 'J': case 'j': case '2': dir = 2; break;
            case 'N': case 'n': case '3': dir = 3; break;
            case 'H': case 'h': case '4': dir = 4; break;
            case 'L': case 'l': case '6': dir = 6; break;
            case 'Y': case 'y': case '7': dir = 7; break;
            case 'K': case 'k': case '8': dir = 8; break;
            case 'U': case 'u': case '9': dir = 9; break;

            /* Error */
            default: bell();
        }
    }

    /* Keep the given direction */
    *dp = dir;

    /* Aborted */
    if (!dir) return (FALSE);

    /* Save the direction */
    command_dir = dir;

    /* Apply "confusion" */
    if (p_ptr->confused) {

        /* Warn the user XXX XXX XXX */
        /* msg_print("You are confused."); */

        /* Standard confusion */
        if (rand_int(100) < 75) {

            /* Random direction */
            *dp = ddd[rand_int(8)];
        }
    }

    /* A "valid" direction was entered */
    return (TRUE);
}






/*
 * Turn on Search Mode.
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

/*
 * Turn off Search Mode.
 */
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
 * Searches for hidden things.			-RAK-	
 */
void search(void)
{
    int           y, x, chance;

    cave_type    *c_ptr;
    inven_type   *i_ptr;


    /* Start with base search ability */
    chance = p_ptr->skill_srh;

    /* Penalize various conditions */
    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused || p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (y = (py - 1); y <= (py + 1); y++) {
        for (x = (px - 1); x <= (px + 1); x++) {

            /* Sometimes, notice things */
            if (rand_int(100) < chance) {

                /* Access the grid */
                c_ptr = &cave[y][x];

                /* Access the object */
                i_ptr = &i_list[c_ptr->i_idx];

                /* Invisible trap */
                if ((c_ptr->feat & 0x3F) == 0x02) {

                    /* Message */
                    msg_print("You have found a trap.");

                    /* Pick a trap XXX XXX XXX */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x10) + rand_int(16);

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Secret door */
                else if ((c_ptr->feat & 0x3F) == 0x30) {

                    /* Message */
                    msg_print("You have found a secret door.");

                    /* Pick a door XXX XXX XXX */
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

                    /* Notice */
                    note_spot(y, x);

                    /* Redraw */
                    lite_spot(y, x);

                    /* Disturb */
                    disturb(0, 0);
                }

                /* Search chests */
                else if (i_ptr->tval == TV_CHEST) {

                    /* Examine chests for traps */
                    if (!inven_known_p(i_ptr) && (chest_traps[i_ptr->pval])) {

                        /* Message */
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

    /* Hack -- optional disturb on "panel change" */
    if (disturb_panel) disturb(0, 0);

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

    inven_type *i_ptr;

    char	i_name[80];


    /* Hack -- nothing here to pick up */
    if (!(c_ptr->i_idx)) return;

    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

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

    /* Pick it up */
    else {

        /* Hack -- disturb */
        disturb(0, 0);

        /* Describe the object */
        if (!pickup) {
            msg_format("You see %s.", i_name);
        }

        /* Note that the pack is too full */
        else if (!inven_carry_okay(i_ptr)) {
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

                int slot;

                /* Carry the item */
                slot = inven_carry(i_ptr);

                /* Get the item again */
                i_ptr = &inventory[slot];

                /* Describe the object */
                objdes(i_name, i_ptr, TRUE, 3);

                /* Message */
                msg_format("You have %s (%c).", i_name, index_to_label(slot));

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

    /* Percentile dice */
    k = rand_int(100);

    /* Hack -- 5% hit, 5% miss */
    if (k < 10) return (k < 5);

    /* Paranoia -- No power */
    if (power <= 0) return (FALSE);

    /* Total armor */
    ac = p_ptr->ac + p_ptr->to_a;
    
    /* Power competes against Armor */
    if (randint(power) > ((ac * 3) / 4)) return (TRUE);

    /* Assume miss */
    return (FALSE);
}


/*
 * Handle player hitting a real trap
 *
 * We use the old location to back away from rubble traps
 */
static void hit_trap(void)
{
    int			i, num, dam;

    cave_type		*c_ptr;

    cptr		name = "a trap";

    
    /* Disturb the player */
    disturb(0, 0);

    /* Get the cave grid */
    c_ptr = &cave[py][px];

    /* Pick a trap if needed */
    if ((c_ptr->feat & 0x3F) == 0x02) {

        /* Pick a trap XXX XXX XXX */
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x10) + rand_int(16);

        /* Notice */
        note_spot(py, px);

        /* Redraw */
        lite_spot(py, px);
    }

    /* Examine the trap sub-val */
    switch (c_ptr->feat & 0x3F) {

      case 0x10:
        msg_print("You fell through a trap door!");
        if (p_ptr->ffall) {
            msg_print("You float gently down to the next level.");
        }
        else {
            dam = damroll(2,8);
            take_hit(dam, name);
        }
        new_level_flag = TRUE;
        dun_level++;
        break;
      
      case 0x11:
        msg_print("You fell into a pit!");
        if (p_ptr->ffall) {
            msg_print("You float gently to the bottom of the pit.");
        }
        else {
            dam = damroll(2, 6);
            take_hit(dam, name);
        }
        break;

      case 0x12:
      
        msg_print("You fall into a spiked pit!");
        
        if (p_ptr->ffall) {
            msg_print("You float gently to the floor of the pit.");
            msg_print("You carefully avoid touching the spikes.");
        }
        
        else {

            /* Base damage */
            dam = damroll(2,6);

            /* Extra spike damage */
            if (rand_int(100) < 50) {

                msg_print("You are impaled!");
                dam = dam * 2;
                cut_player(randint(dam));
            }

            /* Take the damage */
            take_hit(dam, name);
        }
        break;

      case 0x13:

        msg_print("You fall into a spiked pit!");

        if (p_ptr->ffall) {
            msg_print("You float gently to the floor of the pit.");
            msg_print("You carefully avoid touching the spikes.");
        }

        else {

            /* Base damage */
            dam = damroll(2,6);

            /* Extra spike damage */
            if (rand_int(100) < 50) {

                msg_print("You are impaled on poisonous spikes!");

                dam = dam * 2;
                cut_player(randint(dam));

                if (p_ptr->immune_pois ||
                    p_ptr->resist_pois ||
                    p_ptr->oppose_pois) {
                    msg_print("The poison does not affect you!");
                }

                else {
                    dam = dam * 2;
                    (void)set_poisoned(p_ptr->poisoned + randint(dam));
                }
            }

            /* Take the damage */
            take_hit(dam, name);
        }

        break;

      case 0x14:
        msg_print("You are enveloped in a cloud of smoke!");
        c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
        c_ptr->feat &= ~CAVE_MARK;
        note_spot(py,px);
        lite_spot(py,px);
        num = 2 + randint(3);
        for (i = 0; i < num; i++) {
            (void)summon_specific(py, px, dun_level, 0);
        }
        break;

      case 0x15:
        msg_print("You hit a teleport trap!");
        teleport_flag = TRUE;
        teleport_dist = 100;
        break;

      case 0x16:
        msg_print("You are enveloped in flames!");
        dam = damroll(4, 6);
        fire_dam(dam, "a fire trap");
        break;

      case 0x17:
        msg_print("You are splashed with acid!");
        dam = damroll(4, 6);
        acid_dam(dam, "an acid trap");
        break;

      case 0x18:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, name);
            (void)set_slow(p_ptr->slow + rand_int(20) + 20);
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case 0x19:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, name);
            (void)do_dec_stat(A_STR);
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case 0x1A:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, name);
            (void)do_dec_stat(A_DEX);
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case 0x1B:
        if (check_hit(125)) {
            msg_print("A small dart hits you!");
            dam = damroll(1,4);
            take_hit(dam, name);
            (void)do_dec_stat(A_CON);
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case 0x1C:
        msg_print("A black gas surrounds you!");
        if (!p_ptr->resist_blind) {
            (void)set_blind(p_ptr->blind + rand_int(50) + 25);
        }
        break;

      case 0x1D:
        msg_print("A gas of scintillating colors surrounds you!");
        if (!p_ptr->resist_conf) {
            (void)set_confused(p_ptr->confused + rand_int(20) + 10);
        }
        break;

      case 0x1E:
        msg_print("A pungent green gas surrounds you!");
        if (!p_ptr->resist_pois &&
            !p_ptr->oppose_pois &&
            !p_ptr->immune_pois) {
            (void)set_poisoned(p_ptr->poisoned + rand_int(20) + 10);
        }
        break;

      case 0x1F:
        msg_print("A strange white mist surrounds you!");
        if (!p_ptr->free_act) {
            (void)set_paralyzed(p_ptr->paralyzed + rand_int(10) + 5);
        }
        break;
    }
}


/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player(int dir, int do_pickup)
{
    int			y, x;

    cave_type		*c_ptr;
    inven_type		*i_ptr;
    monster_type	*m_ptr;


    /* Find the result of moving */
    y = py + ddy[dir];
    x = px + ddx[dir];

    /* Examine the destination */
    c_ptr = &cave[y][x];

    /* Get the object */
    i_ptr = &i_list[c_ptr->i_idx];

    /* Get the monster */
    m_ptr = &m_list[c_ptr->m_idx];


    /* Hack -- attack monsters */
    if (c_ptr->m_idx) {

        /* Attack */
        py_attack(y, x);
    }

    /* Player can not walk through "walls" */
    else if (!floor_grid_bold(y,x)) {

        /* Disturb the player */
        disturb(0, 0);

        /* Notice things in the dark */
        if (!(c_ptr->feat & CAVE_MARK) &&
            (p_ptr->blind || !(c_ptr->feat & CAVE_LITE))) {

            /* Rubble */
            if ((c_ptr->feat & 0x3F) == 0x31) {
                msg_print("You feel some rubble blocking your way.");
                c_ptr->feat |= CAVE_MARK;
                lite_spot(y, x);
            }

            /* Closed door */
            else if ((c_ptr->feat & 0x3F) < 0x30) {
                msg_print("You feel a closed door blocking your way.");
                c_ptr->feat |= CAVE_MARK;
                lite_spot(y, x);
            }

            /* Wall (or secret door) */
            else {
                msg_print("You feel a wall blocking your way.");
                c_ptr->feat |= CAVE_MARK;
                lite_spot(y, x);
            }
        }

        /* Notice things */
        else {

            /* Rubble */
            if ((c_ptr->feat & 0x3F) == 0x31) {
                msg_print("There is rubble blocking your way.");
            }

            /* Closed doors */
            else if ((c_ptr->feat & 0x3F) < 0x30) {
                msg_print("There is a closed door blocking your way.");
            }

            /* Wall (or secret door) */
            else {
                msg_print("There is a wall blocking your way.");
            }
        }
    }

    /* Normal movement */
    else {

        int oy, ox;
        
        /* Save old location */
        oy = py;
        ox = px;
        
        /* Move the player */
        py = y;
        px = x;
        
        /* Redraw new spot */
        lite_spot(py, px);
        
        /* Redraw old spot */
        lite_spot(oy, ox);

        /* Check for new panel (redraw map) */
        verify_panel();

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
        p_ptr->update |= (PU_DISTANCE);


        /* Spontaneous Searching */
        if ((p_ptr->skill_fos >= 50) ||
            (0 == rand_int(50 - p_ptr->skill_fos))) {
            search();
        }

        /* Continuous Searching */
        if (p_ptr->searching) {
            search();
        }

        /* Handle "objects" */
        if (c_ptr->i_idx) carry(do_pickup);

        /* Handle "store doors" */
        if (((c_ptr->feat & 0x3F) >= 0x08) &&
            ((c_ptr->feat & 0x3F) <= 0x0F)) {

            /* Disturb */           
            disturb(0, 0);
            
            /* Enter the store */
            store_enter(c_ptr->feat & 0x07);
        }

        /* Set off an invisible trap */
        else if ((c_ptr->feat & 0x3F) == 0x02) {
        
            /* Disturb */
            disturb(0, 0);

            /* Hit the trap XXX XXX XXX */            
            hit_trap();
        }
        
        /* Set off an visible trap */
        else if (((c_ptr->feat & 0x3F) >= 0x10) &&
                 ((c_ptr->feat & 0x3F) <= 0x1F)) {
        
            /* Disturb */
            disturb(0, 0);

            /* Hit the trap XXX XXX XXX */            
            hit_trap();
        }        
    }
}



/*
 * Determine if a player "knows" about a grid
 *
 * Line 1 -- player has memorized the grid
 * Line 2 -- player can see the grid
 *
 * XXX XXX XXX This function may be "incorrect"
 *
 * The "running algorythm" needs to be verified and optimized.
 */
#define test_lite_bold(Y,X) \
    ((cave[Y][X].feat & CAVE_MARK) || \
     (player_can_see_bold(Y,X)))


/*
 * Hack -- Do we see a wall?  Used in running.		-CJS-
 */
static int see_wall(int dir, int y, int x)
{
    /* Get the new location */
    y += ddy[dir];
    x += ddx[dir];

    /* XXX XXX XXX Illegal grids are blank */
    if (!in_bounds2(y, x)) return (FALSE);

    /* Only Secret doors, veins, and walls are "walls" */
    if ((cave[y][x].feat & 0x3F) < 0x30) return (FALSE);

    /* XXX XXX XXX Only memorized walls count */
    if (!(cave[y][x].feat & CAVE_MARK)) return (FALSE);

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

    /* XXX XXX XXX Illegal grid are blank */
    if (!in_bounds2(y, x)) return (TRUE);

    /* XXX XXX XXX Unknown grids are blank */
    if (!test_lite_bold(y, x)) return (TRUE);

    /* Default */
    return (FALSE);
}





/* The running algorithm:			-CJS- */


/*
   In the diagrams below, the player has just arrived in the
   grid marked as '@', and he has just come from a grid marked
   as 'o', and he is about to enter the grid marked as 'x'.
   
   Of course, if the "requested" move was impossible, then you
   will of course be blocked, and will stop.
   
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

   To initialize these conditions, we examine the grids adjacent
   to the grid marked 'x', two on each side (marked 'L' and 'R').
   If either one of the two grids on a given side is seen to be
   closed, then that side is considered to be closed. If both
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
          .@x..
          ##.##
           #.#

   Likewise, a run along a wall, and then into a doorway (two
   runs) will work correctly. A single run rightwards from @ will
   stop at 1. Another run right and down will enter the corridor
   and make the corner, stopping at the 2.

        #@x	  1
        ########### ######
        2	    #
        #############
        #

   After any move, the function area_affect is called to
   determine the new surroundings, and the direction of
   subsequent moves. It examines the current player location
   (at which the runner has just arrived) and the previous
   direction (from which the runner is considered to have come).
   
   Moving one square in some direction places you adjacent to
   three or five new squares (for straight and diagonal moves
   respectively) to which you were not previously adjacent,
   marked as '!' in the diagrams below.

       ...!	  ...	
       .o@!	  .o.!	
       ...!	  ..@!	
                   !!!

   You STOP if any of the new squares are interesting in any way:
   for example, if they contain visible monsters or treasure.
   
   You STOP if any of the newly adjacent squares seem to be open,
   and you are also looking for a break on that side. (that is,
   find_openarea AND find_break).
   
   You STOP if any of the newly adjacent squares do NOT seem to be
   open and you are in an open area, and that side was previously
   entirely open.

   Corners: If you are not in the open (i.e. you are in a corridor)
   and there is only one way to go in the new squares, then turn in
   that direction. If there are more than two new ways to go, STOP.
   If there are two ways to go, and those ways are separated by a
   square which does not seem to be open, then STOP.

   Otherwise, we have a potential corner. There are two new open
   squares, which are also adjacent. One of the new squares is
   diagonally located, the other is straight on (as in the diagram).
   We consider two more squares further out (marked below as ?).

   We assign "option" to the straight-on grid, and "option2" to the
   diagonal grid, and "check_dir" to the grid marked 's'.

          .s
         @x?
          #?

   If they are both seen to be closed, then it is seen that no
   benefit is gained from moving straight. It is a known corner.
   To cut the corner, go diagonally, otherwise go straight, but
   pretend you stepped diagonally into that next location for a
   full view next time. Conversely, if one of the ? squares is
   not seen to be closed, then there is a potential choice. We check
   to see whether it is a potential corner or an intersection/room entrance.
   If the square two spaces straight ahead, and the space marked with 's'
   are both blank, then it is a potential corner and enter if find_examine
   is set, otherwise must stop because it is not a corner.
*/




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] = { 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] = { 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;




/*
 * Update the current "running" information, check for stopping.
 */
static void area_affect(void)
{
    int			prev_dir, new_dir, check_dir = 0;
    
    int			row, col;
    int			i, max, inv;
    int			option, option2;

    cave_type		*c_ptr;


    /* No options yet */
    option = 0;
    option2 = 0;

    /* Where we came from */
    prev_dir = find_prevdir;


    /* Range of newly adjacent grids */
    max = (prev_dir & 0x01) + 1;


    /* Look at every newly adjacent square. */
    for (i = -max; i <= max; i++) {

        new_dir = cycle[chome[prev_dir] + i];

        row = py + ddy[new_dir];
        col = px + ddx[new_dir];

        c_ptr = &cave[row][col];

        
        /* Visible monsters abort running */
        if (c_ptr->m_idx) {
        
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            
            if (m_ptr->ml) {
                disturb(0,0);
                return;
            }
        }

        /* Visible objects abort running */
        if (c_ptr->i_idx) {
        
            inven_type *i_ptr = &i_list[c_ptr->i_idx];

            if (i_ptr->marked) {
                disturb(0,0);
                return;
            }
        }


        /* Assume the new grid cannot be seen */
        inv = TRUE;

        /* Can we "see" (or "remember") the adjacent grid? */
        if (test_lite_bold(row, col)) {

            int f = (c_ptr->feat & 0x3F);
            
            /* Hack -- ignore floors */
            if (f == 0x01) f = 0;
            
            /* Hack -- ignore invis traps */
            if (f == 0x02) f = 0;
            
            /* Hack -- Option -- Ignore stairs */
            if (((f == 0x06) || (f == 0x07)) && find_ignore_stairs) f = 0;
                        
            /* Hack -- Option -- ignore doors */
            if (((f == 0x04) || (f == 0x05)) && find_ignore_doors) f = 0;

            /* Hack -- ignore walls and secret doors and rubble */
            if (f >= 0x30) f = 0;

            /* Notice remaining features */
            if (f) {

                disturb(0,0);
                return;
            }

            /* The grid is "visible" */
            inv = FALSE;
        }

        /* Analyze floors (and unknowns) */
        if (inv || floor_grid_bold(row, col)) {

            /* Looking for open area */
            if (find_openarea) ;

            /* The first new direction. */
            else if (!option) {
                option = new_dir;
            }

            /* Three new directions. Stop running. */
            else if (option2) {
                disturb(0,0);
                return;
            }

            /* Two non-adjacent new directions.  Stop running. */
            else if (option != cycle[chome[prev_dir] + i - 1]) {
                disturb(0,0);
                return;
            }

            /* Two new (adjacent) directions (case 1) */
            else if (new_dir & 0x01) {
                check_dir = cycle[chome[prev_dir] + i - 2];
                option2 = new_dir;
            }

            /* Two new (adjacent) directions (case 2) */
            else {
                check_dir = cycle[chome[prev_dir] + i + 1];
                option2 = option;
                option = new_dir;
            }
        }

        /* Obstacle, while looking for open area */
        else {
        
            if (find_openarea) {

                if (i < 0) {

                    /* Break to the right */
                    find_breakright = TRUE;
                }

                else if (i > 0) {

                    /* Break to the left */
                    find_breakleft = TRUE;
                }
            }
        }
    }


    /* Looking for open area */
    if (find_openarea) {

        /* Hack -- look again */
        for (i = -max; i < 0; i++) {

            new_dir = cycle[chome[prev_dir] + i];

            row = py + ddy[new_dir];
            col = px + ddx[new_dir];

            /* XXX XXX XXX */
            
            /* Unknown grid or floor */
            if (!test_lite_bold(row, col) || floor_grid_bold(row, col)) {

                /* Looking to break right */
                if (find_breakright) {
                    disturb(0,0);
                    return;
                }
            }

            /* Obstacle */
            if (test_lite_bold(row, col) && !floor_grid_bold(row, col)) {

                /* Looking to break left */
                if (find_breakleft) {
                    disturb(0,0);
                    return;
                }
            }
        }

        /* Hack -- look again */
        for (i = max; i > 0; i--) {

            new_dir = cycle[chome[prev_dir] + i];

            row = py + ddy[new_dir];
            col = px + ddx[new_dir];

            /* Unknown grid or floor */
            if (!test_lite_bold(row, col) || floor_grid_bold(row, col)) {

                /* Looking to break left */
                if (find_breakleft) {
                    disturb(0,0);
                    return;
                }
            }

            /* Obstacle */
            else {

                /* Looking to break right */
                if (find_breakright) {
                    disturb(0,0);
                    return;
                }
            }
        }
    }
    
    
    /* Not looking for open area */
    else {

        /* No options */
        if (!option) {
            disturb(0, 0);
            return;
        }

        /* One option */
        else if (!option2) {

            /* Primary option */
            command_dir = option;

            /* No other options */
            find_prevdir = option;
        }
                
        /* Two options, examining corners */
        else if (find_examine && !find_cut) {

            /* Primary option */
            command_dir = option;

            /* Hack -- allow curving */
            find_prevdir = option2;
        }

        /* Two options, pick one */
        else {

            /* Get next location */
            row = py + ddy[option];
            col = px + ddx[option];

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


    /* About to hit a known wall, stop */
    if (see_wall(command_dir, py, px)) {
        disturb(0, 0);
        return;
    }
}



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  # 
 *       #x#                 @x#
 *       @p.                  p 
 */
static void run_init(int dir)
{
    int		row, col, deepleft, deepright;
    int		i, shortleft, shortright;


    /* Save the direction */
    command_dir = dir;

    /* Assume running straight */
    find_prevdir = dir;

    /* Assume looking for open area */
    find_openarea = TRUE;
    
    /* Assume not looking for breaks */
    find_breakright = find_breakleft = FALSE;

    /* Assume no nearby walls */
    deepleft = deepright = FALSE;
    shortright = shortleft = FALSE;

    /* Find the destination grid */
    row = py + ddy[dir];
    col = px + ddx[dir];

    /* Extract cycle index */
    i = chome[dir];

    /* Check for walls */
    if (see_wall(cycle[i+1], py, px)) {
        find_breakleft = TRUE;
        shortleft = TRUE;
    }
    else if (see_wall(cycle[i+1], row, col)) {
        find_breakleft = TRUE;
        deepleft = TRUE;
    }

    /* Check for walls */
    if (see_wall(cycle[i-1], py, px)) {
        find_breakright = TRUE;
        shortright = TRUE;
    }
    else if (see_wall(cycle[i-1], row, col)) {
        find_breakright = TRUE;
        deepright = TRUE;
    }

    /* Looking for a break */
    if (find_breakleft && find_breakright) {

        /* Not looking for open area */
        find_openarea = FALSE;

        /* Hack -- allow angled corridor entry */
        if (dir & 0x01) {
            if (deepleft && !deepright) {
                find_prevdir = cycle[i - 1];
            }
            else if (deepright && !deepleft) {
                find_prevdir = cycle[i + 1];
            }
        }

        /* Hack -- allow blunt corridor entry */
        else if (see_wall(cycle[i], row, col)) {
            if (shortleft && !shortright) {
                find_prevdir = cycle[i - 2];
            }
            else if (shortright && !shortleft) {
                find_prevdir = cycle[i + 2];
            }
        }
    }
}


/*
 * Do the first (or next) step of a "run"
 */
void do_cmd_run(void)
{
    int dir;

    int old_dir;

    bool more = FALSE;


    /* Note old direction */
    old_dir = (running ? command_dir : 0);

    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Take time */
        energy_use = 100;

        /* Count the steps */
        running++;

        /* Hack -- Verify the lite radius */
        extract_cur_lite();

        /* Hack -- Verify the view radius */
        extract_cur_view();

        /* Hack -- prepare a new direction */
        if (old_dir != dir) run_init(dir);

        /* Move the player, using the "pickup" flag */
        move_player(dir, always_pickup);

        /* Important -- Handle stuff */
        handle_stuff();

        /* Check to see if he should stop running */
        if (running) area_affect();

        /* Hack -- refresh */
        if (fresh_find) {

            /* Hack -- Hilite the player */
            move_cursor_relative(py, px);

            /* Refresh */
            Term_fresh();
        }

        /* Run some more */
        more = TRUE;
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
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
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * You must use slings + pebbles/shots, bows + arrows, xbows + bolts.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
void do_cmd_fire(void)
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			tdam, tdis, thits, tmul;
    int			bonus, chance;
    int			cur_dis, visible;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    inven_type		*j_ptr;

    bool		hit_body = FALSE;

    int			missile_attr;
    int			missile_char;

    char		i_name[80];


    /* Get the "bow" (if any) */
    j_ptr = &inventory[INVEN_BOW];

    /* Require a launcher */
    if (!j_ptr->tval) {
        msg_print("You have nothing to fire with.");
        return;
    }


    /* Require proper missile */
    item_tester_tval = p_ptr->tval_ammo;

    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Fire which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to fire.");
        return;
    }

    /* Access the item (if in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Get a direction (or cancel) */
    if (!get_aim_dir(&dir)) return;


    /* Create a "local missile object" */
    throw_obj = *i_ptr;
    throw_obj.number = 1;

    /* Reduce and describe inventory */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Reduce and describe floor item */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);
    }

    /* Use the missile object */
    i_ptr = &throw_obj;

    /* Describe the object */
    objdes(i_name, i_ptr, FALSE, 3);

    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);


    /* Use the proper number of shots */
    thits = p_ptr->num_fire;

    /* Use a base distance */
    tdis = 10;

    /* Base damage from thrown object plus launcher bonus */
    tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->to_d + j_ptr->to_d;

    /* Actually "fire" the object */
    bonus = (p_ptr->to_h + i_ptr->to_h + j_ptr->to_h);
    chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));

    /* Assume a base multiplier */
    tmul = 1;

    /* Analyze the launcher */
    switch (j_ptr->sval) {

        /* Sling and ammo */
        case SV_SLING:
            tmul = 2;
            break;

        /* Short Bow and Arrow */
        case SV_SHORT_BOW:
            tmul = 2;
            break;

        /* Long Bow and Arrow */
        case SV_LONG_BOW:
            tmul = 3;
            break;

        /* Light Crossbow and Bolt */
        case SV_LIGHT_XBOW:
            tmul = 3;
            break;

        /* Heavy Crossbow and Bolt */
        case SV_HEAVY_XBOW:
            tmul = 4;
            break;
    }

    /* Get extra "power" from "extra might" */
    if (p_ptr->xtra_might) tmul++;
    
    /* Boost the damage */
    tdam *= tmul;

    /* Base range */
    tdis = 10 + 5 * tmul;


    /* Take a (partial) turn */
    energy_use = (100 / thits);


    /* Start at the player */
    y = py;
    x = px;

    /* Predict the "target" location */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Check for "target request" */
    if ((dir == 5) && target_okay()) {
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
        if (!floor_grid_bold(ny,nx)) break;
	
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
        if (cave[y][x].m_idx) {

            cave_type *c_ptr = &cave[y][x];
            
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Check the visibility */
            visible = m_ptr->ml;

            /* Did we hit it (penalize range) */
            if (test_hit_fire(chance - cur_dis, r_ptr->ac, m_ptr->ml)) {

                bool fear = FALSE;
                
                /* Assume a default death */
                cptr note_dies = " dies.";

                /* Some monsters get "destroyed" */
                if ((r_ptr->flags3 & RF3_DEMON) ||
                    (r_ptr->flags3 & RF3_UNDEAD) ||
                    (r_ptr->flags2 & RF2_STUPID) ||
                    (strchr("Evg", r_ptr->r_char))) {

                    /* Special note at death */
                    note_dies = " is destroyed.";
                }


                /* Note the collision */
                hit_body = TRUE;


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

                    /* Hack -- Track this monster race */
                    if (m_ptr->ml) recent_track(m_ptr->r_idx);

                    /* Hack -- Track this monster */
                    if (m_ptr->ml) health_track(c_ptr->m_idx);
                }

                /* Apply special damage XXX XXX XXX */
                tdam = tot_dam_aux(i_ptr, tdam, m_ptr);
                tdam = critical_shot(i_ptr->weight, i_ptr->to_h, tdam);

                /* No negative damage */
                if (tdam < 0) tdam = 0;

                /* Complex message */
                if (wizard) {
                    msg_format("You do %d (out of %d) damage.",
                               tdam, m_ptr->hp);
                }

                /* Hit the monster, check for death */
                if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies)) {

                    /* Dead monster */
                }

                /* No death */
                else {

                    /* Message */
                    message_pain(c_ptr->m_idx, tdam);

                    /* Take note */
                    if (fear && m_ptr->ml) {

                        char m_name[80];

                        /* Sound */
                        sound(SOUND_FLEE);

                        /* Get the monster name (or "it") */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        msg_format("%^s flees in terror!", m_name);
                    }
                }
            }

            /* Stop looking */
            break;
        }
    }

    /* Chance of breakage */
    j = breakage_chance(i_ptr);

    /* Double the chance if we hit a monster */
    if (hit_body) j = j * 2;

    /* Paranoia -- maximum breakage chance */
    if (j > 100) j = 100;

    /* Drop (or break) near that location */
    drop_near(i_ptr, j, y, x);
}



/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 *
 * Do we really need "verification" (note the "destroy" command)?
 */
void do_cmd_throw(void)
{
    int			dir, item;
    int			j, y, x, ny, nx, ty, tx;
    int			chance, tdam, tdis;
    int			mul, div;
    int			cur_dis, visible;

    inven_type          throw_obj;
    inven_type		*i_ptr;

    bool		ok_throw = FALSE;

    bool		hit_body = FALSE;

    int			missile_attr;
    int			missile_char;

    char		i_name[80];



    /* Get an item (from inven or floor) */
    if (!get_item(&item, "Throw which item? ", FALSE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to throw.");
        return;
    }

    /* Access the item (if in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }
    else {
        i_ptr = &i_list[0 - item];
    }


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


    /* Verify if needed */
    if (!ok_throw) {

        char out_val[160];

        inven_type temp_obj;
        
        /* Create a "local missile object" */
        temp_obj = *i_ptr;
        temp_obj.number = 1;

        /* Description */
        objdes(i_name, &temp_obj, TRUE, 3);

        /* Verify */
        sprintf(out_val, "Really throw %s? ", i_name);
        if (!get_check(out_val)) return;
    }


    /* Get a direction (or cancel) */
    if (!get_aim_dir(&dir)) return;


    /* Create a "local missile object" */
    throw_obj = *i_ptr;
    throw_obj.number = 1;

    /* Reduce and describe inventory */
    if (item >= 0) {
        inven_item_increase(item, -1);
        inven_item_describe(item);
        inven_item_optimize(item);
    }

    /* Reduce and describe floor item */
    else {
        floor_item_increase(0 - item, -1);
        floor_item_optimize(0 - item);
    }

    /* Use the local object */
    i_ptr = &throw_obj;

    /* Description */
    objdes(i_name, i_ptr, FALSE, 3);

    /* Find the color and symbol for the object for throwing */
    missile_attr = inven_attr(i_ptr);
    missile_char = inven_char(i_ptr);


    /* Extract a "distance multiplier" */
    mul = 10;

    /* Enforce a minimum "weight" of one pound */
    div = ((i_ptr->weight > 10) ? i_ptr->weight : 10);

    /* Hack -- Distance -- Reward strength, penalize weight */
    tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

    /* Max distance of 10 */
    if (tdis > 10) tdis = 10;

    /* Hack -- Base damage from thrown object */
    tdam = damroll(i_ptr->dd, i_ptr->ds) + i_ptr->to_d;
    
    /* Chance of hitting */
    chance = (p_ptr->skill_tht + (p_ptr->to_h * BTH_PLUS_ADJ));


    /* Take a turn */
    energy_use = 100;


    /* Start at the player */
    y = py;
    x = px;

    /* Predict the "target" location */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Check for "target request" */
    if ((dir == 5) && target_okay()) {
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
        if (!floor_grid_bold(ny,nx)) break;

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
        if (cave[y][x].m_idx) {

            cave_type *c_ptr = &cave[y][x];
            
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            /* Check the visibility */
            visible = m_ptr->ml;

            /* Did we hit it (penalize range) */
            if (test_hit_fire(chance - cur_dis, r_ptr->ac, m_ptr->ml)) {

                bool fear = FALSE;
                
                /* Assume a default death */
                cptr note_dies = " dies.";

                /* Some monsters get "destroyed" */
                if ((r_ptr->flags3 & RF3_DEMON) ||
                    (r_ptr->flags3 & RF3_UNDEAD) ||
                    (r_ptr->flags2 & RF2_STUPID) ||
                    (strchr("Evg", r_ptr->r_char))) {

                    /* Special note at death */
                    note_dies = " is destroyed.";
                }


                /* Note the collision */
                hit_body = TRUE;


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

                    /* Hack -- Track this monster race */
                    if (m_ptr->ml) recent_track(m_ptr->r_idx);

                    /* Hack -- Track this monster */
                    if (m_ptr->ml) health_track(c_ptr->m_idx);
                }

                /* Apply special damage XXX XXX XXX */
                tdam = tot_dam_aux(i_ptr, tdam, m_ptr);
                tdam = critical_shot(i_ptr->weight, i_ptr->to_h, tdam);

                /* No negative damage */
                if (tdam < 0) tdam = 0;

                /* Complex message */
                if (wizard) {
                    msg_format("You do %d (out of %d) damage.",
                               tdam, m_ptr->hp);
                }

                /* Hit the monster, check for death */
                if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies)) {

                    /* Dead monster */
                }

                /* No death */
                else {

                    /* Message */
                    message_pain(c_ptr->m_idx, tdam);

                    /* Take note */
                    if (fear && m_ptr->ml) {

                        char m_name[80];

                        /* Sound */
                        sound(SOUND_FLEE);

                        /* Get the monster name (or "it") */
                        monster_desc(m_name, m_ptr, 0);

                        /* Message */
                        msg_format("%^s flees in terror!", m_name);
                    }
                }
            }

            /* Stop looking */
            break;
        }
    }

    /* Chance of breakage */
    j = breakage_chance(i_ptr);

    /* Double the chance if we hit a monster */
    if (hit_body) j = j * 2;

    /* Paranoia -- maximum breakage chance */
    if (j > 100) j = 100;

    /* Drop (or break) near that location */
    drop_near(i_ptr, j, y, x);
}



/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is currently unused, but could induce output flush.
 *
 * All disturbance cancels repeated commands, resting, and running.
 */
void disturb(int stop_search, int unused_flag)
{
    /* Cancel auto-commands */
    /* command_new = 0; */

    /* Cancel repeated commands */
    if (command_rep) {

        /* Cancel */
        command_rep = 0;
    
        /* Redraw the state (later) */
        p_ptr->redraw |= (PR_STATE);
    }
    
    /* Cancel Resting */
    if (resting) {

        /* Cancel */
        resting = 0;

        /* Redraw the state (later) */
        p_ptr->redraw |= (PR_STATE);
    }

    /* Cancel running */
    if (running) {

        /* Cancel */
        running = 0;

        /* Hack -- redraw the player */
        lite_spot(py, px);

        /* Hack -- Verify the lite radius */
        extract_cur_lite();

        /* Hack -- Verify the view radius */
        extract_cur_view();
    }

    /* Cancel searching if requested */
    if (stop_search && p_ptr->searching) {

        /* Cancel */
        p_ptr->searching = FALSE;

        /* Update stuff (later) */
        p_ptr->update |= (PU_BONUS);

        /* Redraw stuff (later) */
        p_ptr->redraw |= (PR_STATE | PR_SPEED);
    }
    
    /* Flush the input if requested */
    if (flush_disturb) flush();

    /* Flush messages if requested */
    if (filch_disturb) msg_print(NULL);
}


