/* File: spells2.c */

/* Purpose: player and creature spells, breaths, wands, scrolls, etc. */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(void)
{
    int i;

    bool sleep = FALSE;
    bool speed = FALSE;
    
    /* Aggravate everyone nearby */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Wake up sleeping monsters */
        if (m_ptr->csleep) {
            /* XXX XXX XXX Message */
            m_ptr->csleep = 0;
            sleep = TRUE;
        }
        
        /* Speed up monsters in line of sight */	
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) {
            /* XXX XXX XXX Message */
            if (m_ptr->mspeed < 110) m_ptr->mspeed += 10;
            else if (m_ptr->mspeed < 120) m_ptr->mspeed += 5;
            speed = TRUE;
        }
    }

    /* XXX XXX XXX Message */
    if (sleep) msg_print("You hear a sudden stirring in the distance!");
}



/*
 * Helper function -- return a "nearby" race for polymorphing
 */
int poly_r_idx(int r_idx)
{
    monster_race *r_ptr = &r_list[r_idx];

    int i, r, lev1, lev2;


    /* Uniques never polymorph */
    if (r_ptr->rflags1 & RF1_UNIQUE) return (r_idx);


    /* Allowable range of "levels" for resulting monster */
    lev1 = r_ptr->level - ((randint(20)/randint(9))+1);
    lev2 = r_ptr->level + ((randint(20)/randint(9))+1);

    /* Pick a (possibly new) non-unique race */
    for (i = 0; i < 1000; i++) {

        /* Hack -- Pick a new race */
        monster_level = (dun_level + r_ptr->level) / 2 + 5;
        r = get_mon_num(monster_level);
        monster_level = dun_level;

        /* Extract that monster */
        r_ptr = &r_list[r];

        /* Skip uniques */
        if (r_ptr->rflags1 & RF1_UNIQUE) continue;

        /* Accept valid monsters */
        if ((r_ptr->level >= lev1) && (r_ptr->level <= lev2)) break;
    }

    /* Use that answer */
    if (i < 1000) return (r);

    /* Use the original */
    return (r_idx);
}


/*
 * Move the creature record to a new location		-RAK-	
 */
void teleport_away(int m_idx, int dis)
{
    int           yn, xn, ctr;

    monster_type *m_ptr = &m_list[m_idx];

    /* Paranoia */
    if (m_ptr->dead) return;
    
    /* Find a location */
    for (ctr = 1; TRUE; ctr++) {

        /* Pick a nearby "legal" location */
        while (1) {
            yn = rand_spread(m_ptr->fy, dis);
            xn = rand_spread(m_ptr->fx, dis);
            if (in_bounds(yn, xn)) break;
        }

        /* Accept "empty" floor grids */
        if (empty_grid_bold(yn,xn)) break;

        /* Occasionally try further away */
        if (ctr % 10 == 0) dis += 5;
    }


    /* Move the monster */
    move_rec(m_ptr->fy, m_ptr->fx, yn, xn);

    /* Update the monster */
    update_mon(m_idx, TRUE);
}




/*
 * Delete (not kill) all creatures of a given "type" from level.
 * Quest monsters (Morgoth and Sauron) are not affected.
 * Note that currently all uses of genocide count as a spell (?)
 */
int genocide(int spell)
{
    int		i;
    char			typ;


    /* Get a monster symbol (or abort) */
    if (!get_com("Which type of creature do you wish exterminated? ", &typ)) {

        /* Hack -- return FALSE to mean "aborted" */
        return (FALSE);
    }

    /* Delete the monsters of that "type" */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip wrong-type monsters */
        if (r_ptr->r_char != typ) continue;

        /* XXX XXX XXX Skip Quest Monsters (?) */
        if (r_ptr->rflags1 & RF1_QUESTOR) continue;

        /* Delete the monster */
        delete_monster_idx(i);

        /* Hurt the player */
        if (spell) {
            take_hit(randint(4), "the strain of casting Genocide");
            move_cursor_relative(py, px);
            p_ptr->redraw |= PR_HP;
            handle_stuff(TRUE);
            Term_fresh();
            delay(20 * delay_spd);
        }
    }

    /* Success */
    return (TRUE);
}


/*
 * Delete all nearby (non-unique) creatures
 * Note that currently all uses of genocide count as a spell (?)
 */
int mass_genocide(int spell)
{
    int        i, result = FALSE;

    /* Delete the (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* XXX XXX XXX Skip unique monsters (?) */
        if (r_ptr->rflags1 & RF1_UNIQUE) continue;

        /* Kill nearby monsters */	
        if (m_ptr->cdis <= MAX_SIGHT) {

            /* Delete the monster */
            delete_monster_idx(i);

            /* Cute visual feedback as the player slowly dies */
            if (spell) {
                take_hit(randint(3), "the strain of casting Mass Genocide");
                move_cursor_relative(py, px);
                p_ptr->redraw |= PR_HP;
                handle_stuff(TRUE);
                Term_fresh();
                delay(20 * delay_spd);
            }

            /* Something happened */
            result = TRUE;
        }
    }

    /* Note Effect */
    return (result);
}


/*
 * Speed nearby creatures.
 */
int speed_monsters(void)
{
    int        i, speed = FALSE;


    /* Speed all (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Wake it up */
        m_ptr->csleep = 0;

        /* Increase the speed */
        m_ptr->mspeed += 10;

        /* Note visible results */
        if (m_ptr->ml) {
            char m_name[80];
            monster_desc(m_name, m_ptr, 0);
            message(m_name, 0x03);
            message(" starts moving faster.", 0);
            speed = TRUE;
        }
    }

    return (speed);
}


/*
 * Slow all nearby (non-unique) creatures.
 *
 * XXX XXX Note that casting three or four mass-slow spells will
 * effectively paralyze every nearby monster (better than sleep!)
 */
int slow_monsters(void)
{
    int        i, speed = FALSE;

    int p_lev = ((p_ptr->lev > 10) ? (p_ptr->lev - 10) : 1);


    /* Slow all (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        char		m_name[80];

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Get the monster name */
        monster_desc(m_name, m_ptr, 0);

        /* Wake him up */
        m_ptr->csleep = 0;

        /* Require non-unique and low level */
        if ((r_ptr->level < randint(p_lev) + 10) &&
            !(r_ptr->rflags1 & RF1_UNIQUE)) {

            /* Slow him down */
            m_ptr->mspeed -= 10;

            /* Note effect */
            if (m_ptr->ml) {
                message(m_name, 0x03);
                message(" starts moving slower.", 0);
                speed = TRUE;
            }
        }

        /* Note lack of effect */
        else if (m_ptr->ml) {
            monster_desc(m_name, m_ptr, 0);
            message(m_name, 0x03);
            message(" is unaffected.", 0);
        }
    }

    return (speed);
}


/*
 * Sleep any creature.		-RAK-	
 */
int sleep_monsters2(void)
{
    int        i, sleep = FALSE;

    int p_lev = ((p_ptr->lev > 10) ? (p_ptr->lev - 10) : 1);


    /* Sleep all (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        char		m_name[80];

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Get the monster name */
        monster_desc(m_name, m_ptr, 0);

        /* Hack -- memorize NO_SLEEP */
        if (m_ptr->ml && (r_ptr->rflags3 & RF3_NO_SLEEP)) {
            l_ptr->flags3 |= RF3_NO_SLEEP;
        }

        /* Resist */
        if ((r_ptr->level > randint(p_lev) + 10) ||
            (r_ptr->rflags1 & RF1_UNIQUE) ||
            (r_ptr->rflags3 & RF3_NO_SLEEP)) {

            if (m_ptr->ml) {
                message(m_name, 0x03);
                message(" is unaffected.", 0);
            }
        }

        /* Fall asleep */
        else {
            m_ptr->csleep = 500;
            if (m_ptr->ml) {
                message(m_name, 0x03);
                message(" falls asleep.", 0);
                sleep = TRUE;
            }
        }
    }

    return (sleep);
}



/*
 * Change players hit points in some manner		-RAK-	
 */
int hp_player(int num)
{
    int          res;

    res = FALSE;

    if (p_ptr->chp < p_ptr->mhp) {

        p_ptr->chp += num;

        if (p_ptr->chp > p_ptr->mhp) {
            p_ptr->chp = p_ptr->mhp;
            p_ptr->chp_frac = 0;
        }

        p_ptr->redraw |= PR_HP;

        num = num / 5;
        if (num < 3) {
            if (num == 0) {
                msg_print("You feel a little better.");
            }
            else {
                msg_print("You feel better.");
            }
        }
        else {
            if (num < 7) {
                msg_print("You feel much better.");
            }
            else {
                msg_print("You feel very good.");
            }
        }
        res = TRUE;
    }

    return (res);
}


/*
 * Cure players confusion (next turn)
 */
int cure_confusion()
{
    if (p_ptr->confused) {
        p_ptr->confused = 1;
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Cure blindness (next turn)
 */
int cure_blindness(void)
{
    if (p_ptr->blind) {
        p_ptr->blind = 1;
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Cure poison (next turn)
 */
int cure_poison(void)
{
    if (p_ptr->poisoned) {
        p_ptr->poisoned = 1;
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Cure fear (next turn)
 */
int remove_fear()
{
    if (p_ptr->afraid) {
        p_ptr->afraid = 1;
        return (TRUE);
    }

    return (FALSE);
}




/*
 * This is a fun one.  In a given block, pick some walls and
 * turn them into open spots.  Pick some open spots and turn
 * them into walls.  An "Earthquake" effect.	       -RAK-
 */
void earthquake(void)
{
    int        i, j;
    int                 kill, damage, tmp, y, x;


    /* Shatter the terrain around the player */
    for (i = py - 10; i <= py + 10; i++) {
        for (j = px - 10; j <= px + 10; j++) {

            /* Skip the epicenter (the player) */
            if ((i == py) && (j == px)) continue;

            /* Verify distance */
            if (distance(py, px, i, j) > 10) continue;

            /* Verify legality */
            if (!in_bounds(i, j)) continue;

            /* Artifacts and Stairs and Stores resist */
            if (!valid_grid(i,j)) continue;

            /* Shatter some of the terrain */	
            if (rand_int(8) == 0) {

                cave_type *c_ptr = &cave[i][j];

                /* Delete the object */
                delete_object(i, j);

                /* Hurt the monster, if any */
                if (c_ptr->m_idx > 1) {

                    char	m_name[80];

                    monster_type *m_ptr = &m_list[c_ptr->m_idx];
                    monster_race *r_ptr = &r_list[m_ptr->r_idx];

                    /* Some monsters ignore rocks */
                    if (!(r_ptr->rflags2 & RF2_PASS_WALL) &&
                        !(r_ptr->rflags2 & RF2_KILL_WALL)) {

                        /* monster can not move to escape the wall */
                        if (r_ptr->rflags1 & RF1_NEVER_MOVE) {
                            kill = TRUE;
                        }

                        /* Monster MAY have somewhere to flee */
                        else {
                            kill = TRUE;
                            for (y = i - 1; y <= i + 1; y++) {
                                for (x = j - 1; x <= j + 1; x++) {
                                    if (!in_bounds(y,x)) continue;
                                    if (empty_grid_bold(y, x)) kill = FALSE;
                                }
                            }
                        }

                        if (kill) {
                            damage = 320;
                        }
                        else {
                            damage = damroll(3 + randint(3), 8+randint(5));
                        }

                        /* Acquire monster name */
                        monster_desc(m_name, m_ptr, 0);

                        /* Pain message */
                        message(m_name, 0x03);
                        message(" wails out in pain!", 0);

                        /* Do damage, get experience (?) */
                        if (mon_take_hit(c_ptr->m_idx, damage, TRUE)) {
                            message(m_name, 0x03);
                            message(" is embedded in the rock.", 0);
                            check_experience();
                        }
                    }
                }

                /* Destroy walls */
                if (c_ptr->info & GRID_WALL_MASK) {
                    c_ptr->info &= ~GRID_WALL_MASK;
                }

                /* Make walls, if none there */
                else {
                    tmp = randint(10);
                    if (tmp < 6) c_ptr->info |= GRID_WALL_QUARTZ;
                    else if (tmp < 9) c_ptr->info |= GRID_WALL_MAGMA;
                    else c_ptr->info |= GRID_WALL_GRANITE;
                }

                /* Not in a room */
                c_ptr->info &= ~GRID_ROOM;
                c_ptr->info &= ~GRID_GLOW;
                c_ptr->info &= ~GRID_MARK;
            }
        }
    }


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP | PR_BLOCK);
}


/*
 * Evil creatures don't like this.		       -RAK-
 */
int protect_evil()
{
    int res = FALSE;
    if (!p_ptr->protevil) res = TRUE;
    p_ptr->protevil += randint(25) + 3 * p_ptr->lev;
    if (p_ptr->protevil > 30000) p_ptr->protevil = 30000;
    return (res);
}


/*
 * Make the player no longer hungry
 */
void satisfy_hunger(void)
{
    msg_print("You feel full!");
    msg_print(NULL);

    /* No longer hungry */
    p_ptr->food = PLAYER_FOOD_MAX;
}


/*
 * Banish evil monsters
 */
int banish_evil(int dist)
{
    int           i, result = FALSE;

    /* Banish all (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Teleport evil monsters away */
        if (r_ptr->rflags3 & RF3_EVIL) {

            /* Memorize the flag */
            l_ptr->flags3 |= RF3_EVIL;

            /* Teleport it away */
            (void)teleport_away(i, dist);

            /* Note the result */
            result = TRUE;
        }
    }

    return (result);
}

int probing(void)
{
    int            i, probe = FALSE;


    /* Probe all (nearby) monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Probe visible monsters */
        if (m_ptr->ml) {

            char m_name[80];

            /* Start the message */
            if (!probe) msg_print("Probing...");

            /* Get "the monster" or "something" */
            monster_desc(m_name, m_ptr, 0x04);

            /* Describe the monster */
            message(m_name, 0x03);
            message(format(" has %d hit points.", m_ptr->hp), 0);

            /* Learn all of the non-spell, non-treasure flags */
            lore_do_probe(m_ptr);

            /* Probe worked */
            probe = TRUE;
        }
    }

    if (probe) {
        msg_print("That's all.");
    }

    /* Result */
    return (probe);
}


/*
 * Dispel a monster
 */
int dispel_monster(int m_idx, int dam)
{
    monster_type *m_ptr = &m_list[m_idx];

    char		m_name[80];

    /* Get the name */
    monster_desc(m_name, m_ptr, 0);

    /* Apply the damage */
    if (mon_take_hit(m_idx, randint(dam), TRUE)) {
        message(m_name, 0x03);
        message(" dissolves!", 0);
        check_experience();
    }
    else {
        message(m_name, 0x03);
        message(" shudders.", 0);
    }

    /* Note the effect */
    return (TRUE);
}


/*
 * Apply damage to all nearby monsters.
 */
int dispel_monsters(int damage, bool only_evil, bool only_undead)
{
    int	i, dispel = FALSE;

    /* Affect all nearby monsters within line of sight */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type	*m_ptr = &m_list[i];
        monster_race	*r_ptr = &r_list[m_ptr->r_idx];
        monster_lore	*l_ptr = &l_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require appropriate type */
        if (only_evil && !(r_ptr->rflags3 & RF3_EVIL)) continue;
        if (only_undead && !(r_ptr->rflags3 & RF3_UNDEAD)) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Memorize the susceptibility */
        if (r_ptr->rflags3 & RF3_EVIL) l_ptr->flags3 |= RF3_EVIL;
        if (r_ptr->rflags3 & RF3_UNDEAD) l_ptr->flags3 |= RF3_UNDEAD;

        /* Dispel the monster, note effects */
        if (dispel_monster(i, damage)) dispel = TRUE;
    }

    /* Note effect */
    return (dispel);
}


/*
 * Attempt to turn (scare) undead creatures.	-RAK-	
 */
int turn_undead(void)
{
    int            i, result = FALSE;

    /* Turn undead monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Require line of sight */
        if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

        /* Only affect undead */
        if (r_ptr->rflags3 & RF3_UNDEAD) {

            char m_name[80];

            monster_desc(m_name, m_ptr, 0);

            if (((p_ptr->lev + 1) > r_ptr->level) || (rand_int(5) == 0)) {

                if (m_ptr->ml) {
                    l_ptr->flags3 |= RF3_UNDEAD;
                    message(m_name, 0x03);
                    message(" runs frantically!", 0);
                    result = TRUE;
                }

                /* Increase fear */
                m_ptr->monfear += randint(p_ptr->lev) * 2;
            }

            else if (m_ptr->ml) {
                message(m_name, 0x03);
                message(" is unaffected.", 0);
            }
        }
    }

    return (result);
}


/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph(void)
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    int y = py;
    int x = px;

    if (!clean_grid_bold(y, x)) return;

    c_ptr = &cave[y][x];
    c_ptr->i_idx = i_pop();
    i_ptr = &i_list[c_ptr->i_idx];
    invcopy(i_ptr, OBJ_GLYPH);
    i_ptr->iy = y;
    i_ptr->ix = x;
}




/*
 * Lose experience					-RAK-	
 */
void lose_exp(s32b amount)
{
    int          i;

    /* Lose some experience */
    if (amount > p_ptr->exp) {
        p_ptr->exp = 0;
    }
    else {
        p_ptr->exp -= amount;
    }

    /* Gain levels as needed */
    check_experience();

    /* Start at level zero */
    i = 0;

    /* Find max legal level */
    while ((i < MAX_PLAYER_LEVEL) &&
           ((player_exp[i] * p_ptr->expfact / 100L) <= p_ptr->exp)) {
        i++;
    }

    /* Assume a level */
    i++;

    /* Cannot go over maximum */
    if (i > MAX_PLAYER_LEVEL) i = MAX_PLAYER_LEVEL;

    /* New level */
    if (p_ptr->lev != i) {

        /* Save the level */
        p_ptr->lev = i;

        /* Update hitpoints */
        p_ptr->update |= PU_HP;

        /* Update spells and mana */
        p_ptr->update |= (PU_MANA | PU_SPELLS);

        /* Redraw stuff */
        p_ptr->redraw |= PR_BLOCK;
    }
}


/*
 * Slow Poison						-RAK-	
 */
int slow_poison()
{
    if (p_ptr->poisoned) {
        p_ptr->poisoned = p_ptr->poisoned / 2;
        if (p_ptr->poisoned < 1) p_ptr->poisoned = 1;
        msg_print("The effect of the poison has been reduced.");
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Bless						-RAK-	
 */
void bless(int amount)
{
    if (p_ptr->blessed < 30000) p_ptr->blessed += amount;
}


/*
 * Detect Invisible for period of time			-RAK-	
 */
void detect_inv2(int amount)
{
    if (p_ptr->detect_inv < 30000) p_ptr->detect_inv += amount;
}


/*
 * The spell of destruction.				-RAK-
 *
 * Note that this spell "deletes" (not "kills") monsters
 */
void destroy_area(int y1, int x1)
{
    int y, x, k, t;

    /* Message */
    msg_print("There is a searing blast of light!");

    /* Blind the player */
    if (!p_ptr->resist_blind && !p_ptr->resist_lite) {
        p_ptr->blind += 10 + randint(10);
    }

    /* No destroying the town */
    if (dun_level) {

        /* Big area of affect */
        for (y = (y1 - 15); y <= (y1 + 15); y++) {
            for (x = (x1 - 15); x <= (x1 + 15); x++) {

                /* Extract the distance */
                k = distance(y1, x1, y, x);

                /* Stay in the circle of death */
                if (k >= 16) continue;

                /* Skip the epicenter */
                if ((y == y1) && (x == x1)) continue;

		/* Skip illegal grids */
		if (!in_bounds(y, x)) continue;

                /* Delete the monster (if any) */
                delete_monster(y, x);
		
                /* Destroy "valid" grids */
                if (valid_grid(y, x)) {

                    cave_type *c_ptr = &cave[y][x];

                    /* Replace the ground */
                    t = ((k < 13) ? randint(6) : randint(9));

                    /* Clear the walls */
                    c_ptr->info &= ~GRID_WALL_MASK;

                    /* Make new walls */
                    switch (t) {
                      case 1: case 2: case 3:
                        break;
                      case 4: case 7: case 10:
                        c_ptr->info |= GRID_WALL_GRANITE;
                        break;
                      case 5: case 8: case 11:
                        c_ptr->info |= GRID_WALL_MAGMA;
                        break;
                      case 6: case 9: case 12:
                        c_ptr->info |= GRID_WALL_QUARTZ;
                        break;
                    }

                    /* No longer part of a room */
                    c_ptr->info &= ~GRID_ROOM;
                    c_ptr->info &= ~GRID_MARK;
                    c_ptr->info &= ~GRID_GLOW;

                    /* Delete the object (if any) */
                    delete_object(y, x);
                }
            }
        }
    }


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP | PR_BLOCK);
}




/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] = {
   0, 10,  50, 100, 200,
   300, 400, 500, 700, 950,
   990, 992, 995, 997, 999,
   1000
};


/*
 * Dump a message describing a monster's reaction to damage
 *
 * Technically should attempt to treat "Beholder"'s as jelly's
 */
void message_pain(int m_idx, int dam)
{
    long			oldhp, newhp, tmp;
    int				percentage;

    monster_type		*m_ptr = &m_list[m_idx];
    monster_race		*r_ptr = &r_list[m_ptr->r_idx];

    char			m_name[80];


    /* Get the monster name */
    monster_desc(m_name, m_ptr, 0);

    /* Start the message */
    message(m_name, 0x03);

    /* Notice non-damage */
    if (dam == 0) {
        message(" is unharmed.", 0);
        return;
    }

    /* Note -- subtle fix -CFT */
    newhp = (long)(m_ptr->hp);
    oldhp = newhp + (long)(dam);
    tmp = (newhp * 100L) / oldhp;
    percentage = (int)(tmp);


    /* Jelly's and Mold's and Quthl's */
    if (strchr("jmvQ", r_ptr->r_char)) {

        if (percentage > 95)
            message(" barely notices.", 0);
        else if (percentage > 75)
            message(" flinches.", 0);
        else if (percentage > 50)
            message(" squelches.", 0);
        else if (percentage > 35)
            message(" quivers in pain.", 0);
        else if (percentage > 20)
            message(" writhes about.", 0);
        else if (percentage > 10)
            message(" writhes in agony.", 0);
        else
            message(" jerks limply.", 0);
    }

    /* Dogs and Hounds */
    else if (strchr("CZ", r_ptr->r_char)) {

        if (percentage > 95)
            message(" shrugs off the attack.", 0);
        else if (percentage > 75)
            message(" snarls with pain.", 0);
        else if (percentage > 50)
            message(" yelps in pain.", 0);
        else if (percentage > 35)
            message(" howls in pain.", 0);
        else if (percentage > 20)
            message(" howls in agony.", 0);
        else if (percentage > 10)
            message(" writhes in agony.", 0);
        else
            message(" yelps feebly.", 0);
    }

    /* One type of monsters (ignore,squeal,shriek) */
    else if (strchr("KcaUqRXbFJlrsSt", r_ptr->r_char)) {

        if (percentage > 95)
            message(" ignores the attack.", 0);
        else if (percentage > 75)
            message(" grunts with pain.", 0);
        else if (percentage > 50)
            message(" squeals in pain.", 0);
        else if (percentage > 35)
            message(" shrieks in pain.", 0);
        else if (percentage > 20)
            message(" shrieks in agony.", 0);
        else if (percentage > 10)
            message(" writhes in agony.", 0);
        else
            message(" cries out feebly.", 0);
    }

    /* Another type of monsters (shrug,cry,scream) */
    else {

        if (percentage > 95)
            message(" shrugs off the attack.", 0);
        else if (percentage > 75)
            message(" grunts with pain.", 0);
        else if (percentage > 50)
            message(" cries out in pain.", 0);
        else if (percentage > 35)
            message(" screams in pain.", 0);
        else if (percentage > 20)
            message(" screams in agony.", 0);
        else if (percentage > 10)
            message(" writhes in agony.", 0);
        else
            message(" cries out feebly.", 0);
    }
}



/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
    int		i, cnt;

    /* Attempt to uncurse items being worn */
    for (cnt = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        inven_type *i_ptr = &inventory[i];

        /* Not wearable */
        if (!wearable_p(i_ptr)) continue;

        /* Uncursed already */
        if (!cursed_p(i_ptr)) continue;

        /* Heavily Cursed Items need a special spell */
        if (!all && (i_ptr->flags3 & TR3_HEAVY_CURSE)) continue;

        /* Perma-Cursed Items can NEVER be uncursed */
        if (i_ptr->flags3 & TR3_PERMA_CURSE) continue;

        /* Uncurse it, and take note */
        i_ptr->flags3 &= ~TR3_CURSED;
        i_ptr->flags3 &= ~TR3_HEAVY_CURSE;

        /* Hack -- assume felt */
        i_ptr->ident |= ID_FELT;

        /* Take note */
        inscribe(i_ptr, "uncursed");

        /* Count the uncursings */
        cnt++;
    }

    /* Recalculate the bonuses if needed */
    if (cnt) p_ptr->update |= PU_BONUS;

    /* Return "something uncursed" */
    return (cnt);
}


/*
 * Remove most curses
 */
int remove_curse()
{
    return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
int remove_all_curse()
{
    return (remove_curse_aux(TRUE));
}



/*
 * Restores any drained experience			-RAK-	
 */
int restore_level()
{
    /* Nothing to do */
    if (p_ptr->exp >= p_ptr->max_exp) return (FALSE);

    msg_print("You feel your life energies returning.");

    /* XXX XXX this while loop is not redundant */
    /* check_experience() may reduce the exp level */
    while (p_ptr->exp < p_ptr->max_exp) {
        p_ptr->exp = p_ptr->max_exp;
        check_experience();
    }

    return (TRUE);
}


/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 */
void self_knowledge()
{
    int    i = 0, j, k;
    u32b f1 = 0L, f2 = 0L, f3 = 0L;
    inven_type *i_ptr;
    cptr info[128];


    /* Acquire item flags (from worn items) */
    for (k = INVEN_WIELD; k < INVEN_TOTAL; k++) {
        i_ptr = &inventory[k];

        /* Only examine real items */
        if (i_ptr->tval) {

            /* Certain fields depend on a positive "pval" */
            if (i_ptr->pval > 0) {
                f1 |= i_ptr->flags1;
            }
            else {
                /* Mask out the "inactive" flags */
                f1 |= (i_ptr->flags1 & ~TR1_PVAL_MASK);
            }
            f2 |= i_ptr->flags2;
            f3 |= i_ptr->flags3;
        }
    }


    if (p_ptr->blind) {
        info[i++] = "You cannot see.";
    }
    if (p_ptr->confused) {
        info[i++] = "You are confused.";
    }
    if (p_ptr->afraid) {
        info[i++] = "You are terrified.";
    }
    if (p_ptr->cut) {
        info[i++] = "You are bleeding.";
    }
    if (p_ptr->stun) {
        info[i++] = "You are stunned and reeling.";
    }
    if (p_ptr->poisoned) {
        info[i++] = "You are poisoned.";
    }
    if (p_ptr->image) {
        info[i++] = "You are hallucinating.";
    }
    if (p_ptr->aggravate) {
        info[i++] = "You aggravate monsters.";
    }
    if (p_ptr->teleport) {
        info[i++] = "Your position is very uncertain.";
    }
    if (p_ptr->blessed) {
        info[i++] = "You feel rightous.";
    }
    if (p_ptr->hero) {
        info[i++] = "You feel heroic.";
    }
    if (p_ptr->shero) {
        info[i++] = "You are in a battle rage.";
    }
    if (p_ptr->protevil) {
        info[i++] = "You are protected from evil.";
    }
    if (p_ptr->shield) {
        info[i++] = "You are protected by a mystic shield.";
    }
    if (p_ptr->invuln) {
        info[i++] = "You are temporarily invulnerable.";
    }
    if (p_ptr->confusing) {
        info[i++] = "Your hands are glowing dull red.";
    }
    if (p_ptr->searching) {
        info[i++] = "You are looking around very carefully.";
    }
    if (p_ptr->new_spells) {
        info[i++] = "You can learn some more spells.";
    }
    if (p_ptr->word_recall) {
        info[i++] = "You will soon be recalled.";
    }
    if ((p_ptr->see_infra) || (p_ptr->tim_infra)) {
        info[i++] = "Your eyes are sensitive to infrared light.";
    }
    if ((p_ptr->see_inv) || (p_ptr->detect_inv)) {
        info[i++] = "You can see invisible creatures.";
    }
    if (p_ptr->ffall) {
        info[i++] = "You land gently.";
    }
    if (p_ptr->free_act) {
        info[i++] = "You have free action.";
    }
    if (p_ptr->regenerate) {
        info[i++] = "You regenerate quickly.";
    }
    if (p_ptr->slow_digest) {
        info[i++] = "Your appetite is small.";
    }
    if (p_ptr->telepathy) {
        info[i++] = "You have ESP.";
    }
    if (p_ptr->hold_life) {
        info[i++] = "You have a firm hold on your life force.";
    }
    if (p_ptr->lite) {
        info[i++] = "You are carrying a permanent light.";
    }

    if (p_ptr->immune_acid) {
        info[i++] = "You are completely immune to acid.";
    }
    else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid)) {
        info[i++] = "You resist acid exceptionally well.";
    }
    else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid)) {
        info[i++] = "You are resistant to acid.";
    }

    if (p_ptr->immune_elec) {
        info[i++] = "You are completely immune to lightning.";
    }
    else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec)) {
        info[i++] = "You resist lightning exceptionally well.";
    }
    else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec)) {
        info[i++] = "You are resistant to lightning.";
    }

    if (p_ptr->immune_fire) {
        info[i++] = "You are completely immune to fire.";
    }
    else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire)) {
        info[i++] = "You resist fire exceptionally well.";
    }
    else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire)) {
        info[i++] = "You are resistant to fire.";
    }

    if (p_ptr->immune_cold) {
        info[i++] = "You are completely immune to cold.";
    }
    else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold)) {
        info[i++] = "You resist cold exceptionally well.";
    }
    else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold)) {
        info[i++] = "You are resistant to cold.";
    }

    if (p_ptr->immune_pois) {
        info[i++] = "You are completely immune to poison.";
    }
    else if ((p_ptr->resist_pois) && (p_ptr->oppose_pois)) {
        info[i++] = "You resist poison exceptionally well.";
    }
    else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois)) {
        info[i++] = "You are resistant to poison.";
    }

    if (p_ptr->resist_lite) {
        info[i++] = "You are resistant to bright light.";
    }
    if (p_ptr->resist_dark) {
        info[i++] = "You are resistant to darkness.";
    }
    if (p_ptr->resist_conf) {
        info[i++] = "You are resistant to confusion.";
    }
    if (p_ptr->resist_sound) {
        info[i++] = "You are resistant to sonic attacks.";
    }
    if (p_ptr->resist_disen) {
        info[i++] = "You are resistant to disenchantment.";
    }
    if (p_ptr->resist_chaos) {
        info[i++] = "You are resistant to chaos.";
    }
    if (p_ptr->resist_shard) {
        info[i++] = "You are resistant to blasts of shards.";
    }
    if (p_ptr->resist_nexus) {
        info[i++] = "You are resistant to nexus attacks.";
    }
    if (p_ptr->resist_neth) {
        info[i++] = "You are resistant to nether forces.";
    }
    if (p_ptr->resist_fear) {
        info[i++] = "You are completely fearless.";
    }
    if (p_ptr->resist_blind) {
        info[i++] = "Your eyes are resistant to blindness.";
    }

    if (p_ptr->sustain_str) {
        info[i++] = "You will not become weaker.";
    }
    if (p_ptr->sustain_int) {
        info[i++] = "You will not become dumber.";
    }
    if (p_ptr->sustain_wis) {
        info[i++] = "You will not become less wise.";
    }
    if (p_ptr->sustain_con) {
        info[i++] = "You will not become out of shape.";
    }
    if (p_ptr->sustain_dex) {
        info[i++] = "You will not become clumsy.";
    }
    if (p_ptr->sustain_chr) {
        info[i++] = "You will not become less popular.";
    }


    if (f1 & TR1_STR) {
        info[i++] = "You are magically strong.";
    }
    if (f1 & TR1_INT) {
        info[i++] = "You are magically intelligent.";
    }
    if (f1 & TR1_WIS) {
        info[i++] = "You are magically wise.";
    }
    if (f1 & TR1_DEX) {
        info[i++] = "You are magically agile.";
    }
    if (f1 & TR1_CON) {
        info[i++] = "You are magically tough.";
    }
    if (f1 & TR1_CHR) {
        info[i++] = "You are magically popular.";
    }

    if (f1 & TR1_STEALTH) {
        info[i++] = "You are magically stealthy.";
    }
    if (f1 & TR1_SEARCH) {
        info[i++] = "You are magically perceptive.";
    }
    if (f1 & TR1_ATTACK_SPD) {
        info[i++] = "You can strike at your foes with uncommon speed.";
    }


    /* Access the current weapon */
    i_ptr = &inventory[INVEN_WIELD];

    /* Analyze the weapon */
    if (i_ptr->tval) {

        /* Items with a "good" pval */
        if (i_ptr->pval >= 0) {
            f1 |= i_ptr->flags1;
            f2 |= i_ptr->flags2;
            f3 |= i_ptr->flags3;
        }

        /* Items with a "bad" pval */
        else {
            f1 |= (i_ptr->flags1 & ~TR1_PVAL_MASK);
            f2 |= i_ptr->flags2;
            f3 |= i_ptr->flags3;
        }


        /* Indicate various curses */
        if (f3 & TR3_HEAVY_CURSE) {
            info[i++] = "Your weapon is truly foul.";
        }
        else if (f3 & TR3_CURSED) {
            info[i++] = "Your weapon is accursed.";
        }

        /* Indicate Blessing */
        if (f3 & TR3_BLESSED) {
            info[i++] = "Your weapon has been blessed by the gods.";
        }

        /* Special "Attack Bonuses" */
        if (f1 & TR1_TUNNEL) {
            info[i++] = "Your weapon is an effective digging tool.";
        }
        if (f1 & TR1_ATTACK_SPD) {
            info[i++] = "Your weapon strikes with uncommon speed.";
        }
        if (f1 & TR1_BRAND_COLD) {
            info[i++] = "Your frigid weapon freezes your foes.";
        }
        if (f1 & TR1_BRAND_FIRE) {
            info[i++] = "Your flaming weapon burns your foes.";
        }
        if (f1 & TR1_BRAND_ELEC) {
            info[i++] = "Your weapon electrocutes your foes.";
        }
        if (f1 & TR1_IMPACT) {
            info[i++] = "The unbelievable impact of your weapon can cause earthquakes.";
        }

        if (f1 & TR1_KILL_DRAGON) {
            info[i++] = "Your weapon is a great bane of dragons.";
        }
        else if (f1 & TR1_SLAY_DRAGON) {
            info[i++] = "Your weapon is especially deadly against dragons.";
        }
        if (f1 & TR1_SLAY_ORC) {
            info[i++] = "Your weapon is especially deadly against orcs.";
        }
        if (f1 & TR1_SLAY_TROLL) {
            info[i++] = "Your weapon is especially deadly against trolls.";
        }
        if (f1 & TR1_SLAY_GIANT) {
            info[i++] = "Your weapon is especially deadly against giants.";
        }
        if (f1 & TR1_SLAY_ANIMAL) {
            info[i++] = "Your weapon is especially deadly against natural creatures.";
        }
        if (f1 & TR1_SLAY_DEMON) {
            info[i++] = "Your weapon strikes at demons with holy wrath.";
        }
        if (f1 & TR1_SLAY_UNDEAD) {
            info[i++] = "Your weapon strikes at undead with holy wrath.";
        }
        if (f1 & TR1_SLAY_EVIL) {
            info[i++] = "Your weapon fights against evil with holy fury.";
        }
    }


    /* Save the screen */
    save_screen();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

    /* Label the information */
    prt("     Your Attributes:", 1, 15);

    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++) {

        /* Show the info */
        prt(info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j+1 < i)) {
            prt("-- more --", k, 15);
            inkey();
            for ( ; k > 2; k--) prt("", k, 15);
        }
    }

    /* Pause */
    prt("[Press any key to continue]", k, 13);
    inkey();

    /* Restore the screen */
    restore_screen();
}





/*
 * Teleport the player one level up or down (random when legal)
 */
void tele_level()
{
    if (!dun_level) {
        dun_level++;
        msg_print("You sink through the floor.");
    }
    else if (is_quest(dun_level)) {
        dun_level--;
        msg_print("You rise up through the ceiling.");
    }
    else if (rand_int(2) == 0) {
        dun_level--;
        msg_print("You rise up through the ceiling.");
    }
    else {
        dun_level++;
        msg_print("You sink through the floor.");
    }

    /* New level */
    new_level_flag = TRUE;
}



/*
 * Sleep creatures adjacent to player			-RAK-	
 * Could be done as a "radius one" ball attack via "project()".
 */
int sleep_monsters1(int y, int x)
{
    int            i, j, sleep = FALSE;

    int p_lev = ((p_ptr->lev > 10) ? (p_ptr->lev - 10) : 1);


    /* Scan next to the player */
    for (i = y - 1; i <= y + 1; i++) {
        for (j = x - 1; j <= x + 1; j++) {

            cave_type *c_ptr = &cave[i][j];

            /* Affect monsters */
            if (c_ptr->m_idx > 1) {

                char m_name[80];

                monster_type *m_ptr = &m_list[c_ptr->m_idx];
                monster_race *r_ptr = &r_list[m_ptr->r_idx];
                monster_lore *l_ptr = &l_list[m_ptr->r_idx];

                monster_desc(m_name, m_ptr, 0);

                /* Hack -- memorize "NO_SLEEP" */
                if (m_ptr->ml && (r_ptr->rflags3 & RF3_NO_SLEEP)) {
                    l_ptr->flags3 |= RF3_NO_SLEEP;
                }

                /* Success */
                if ((r_ptr->level > randint(p_lev) + 10) ||
                    (r_ptr->rflags3 & RF3_NO_SLEEP) ||
                    (r_ptr->rflags1 & RF1_UNIQUE)) {

                    message(m_name, 0x03);
                    message(" is unaffected.", 0);
                }

                /* Resists */
                else {

                    sleep = TRUE;
                    m_ptr->csleep = 500;
                    message(m_name, 0x03);
                    message(" falls asleep.", 0);
                }
            }
        }
    }

    return (sleep);
}



/*
 * Forget everything
 */
int lose_all_info(void)
{
    int                 i;

    /* Forget info about objects */
    for (i = 0; i < INVEN_TOTAL; i++) {

        inven_type *i_ptr = &inventory[i];

        /* Skip non-items */
        if (!i_ptr->tval) continue;

        /* Now forget about the item */
        if (inven_known_p(i_ptr)) {

            /* Hack -- Clear the "known" flag */
            i_ptr->ident &= ~ID_KNOWN;

            /* Hack -- Clear the "felt" flag */
            i_ptr->ident &= ~ID_FELT;
        }
    }

    /* Mega-Hack -- Forget the map */
    wiz_dark();

    /* It worked */
    return (TRUE);
}


/*
 * Detect any treasure on the current panel		-RAK-	
 */
int detect_treasure(void)
{
    int        i, j, detect = FALSE;
    cave_type *c_ptr;

    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid */
            c_ptr = &cave[i][j];

            /* Notice gold */
            if (i_list[c_ptr->i_idx].tval == TV_GOLD) {

                /* Notice new items */
                if (!test_lite_bold(i, j)) detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->info |= GRID_MARK;

                /* Redraw */
                lite_spot(i, j);
            }
        }
    }

    return (detect);
}



/*
 * Detect magic items.
 *
 * This will light up all spaces with "magic" items, including potions, scrolls,
 * books, rods, wands, staves, amulets, rings, artifacts, and "enchanted" items.
 *
 * It can probably be argued that this function is now too powerful.
 */
int detect_magic()
{
    int i, j, tv, detect = FALSE;

    cave_type *c_ptr;
    inven_type *i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];
            i_ptr = &i_list[c_ptr->i_idx];

            /* Nothing there */
            if (!(c_ptr->i_idx)) continue;

            /* Examine the tval */
            tv = i_ptr->tval;

            /* Artifacts, misc magic items, or enchanted wearables */
            if (artifact_p(i_ptr) ||
                (tv == TV_AMULET) || (tv == TV_RING) ||
                (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
                (tv == TV_SCROLL) || (tv == TV_POTION) ||
                (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
                (wearable_p(i_ptr) &&
                 ((i_ptr->toac>0) || (i_ptr->tohit>0) || (i_ptr->todam>0)))) {

                /* Note new items */
                if (!test_lite_bold(i, j)) detect = TRUE;

                /* Hack -- memorize the item */
                c_ptr->info |= GRID_MARK;

                /* Redraw */
                lite_spot(i, j);
            }
        }
    }

    /* Return result */
    return (detect);
}





/*
 * Locates and displays all invisible creatures on current panel -RAK-
 */
int detect_invisible()
{
    int           i, flag = FALSE;


    /* Detect all invisible monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];
        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all invisible monsters */
        if (panel_contains(fy, fx) && (r_ptr->rflags2 & RF2_INVISIBLE)) {

            /* Take note that they are invisible */
            l_ptr->flags2 |= RF2_INVISIBLE;

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe result, and clean up */
    if (flag) {

        msg_print("You sense the presence of invisible creatures!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}



/*
 * Display evil creatures on current panel		-RAK-	
 */
int detect_evil(void)
{
    int        i, flag = FALSE;


    /* Display all the evil monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect evil monsters */
        if (panel_contains(fy, fx) && (r_ptr->rflags3 & RF3_EVIL)) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Note effects and clean up */
    if (flag) {

        msg_print("You sense the presence of evil!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}



/*
 * Display all non-invisible monsters on the current panel
 */
int detect_monsters(void)
{
    int        i, flag = FALSE;


    /* Detect non-invisible monsters */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all non-invisible monsters */
        if (panel_contains(fy, fx) && (!(r_ptr->rflags2 & RF2_INVISIBLE))) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = TRUE;
        }
    }

    /* Describe and clean up */
    if (flag) {

        /* Describe, and wait for acknowledgement */
        msg_print("You sense the presence of monsters!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (flag);
}


/*
 * Detect everything
 */
int detection(void)
{
    int           i, flag = FALSE, detect = FALSE;


    /* Detect the easy things */
    if (detect_treasure()) detect = TRUE;
    if (detect_object()) detect = TRUE;
    if (detect_trap()) detect = TRUE;
    if (detect_sdoor()) detect = TRUE;


    /* Detect all monsters in the current panel */
    for (i = MIN_M_IDX; i < m_max; i++) {

        monster_type *m_ptr = &m_list[i];

        int fy = m_ptr->fy;
        int fx = m_ptr->fx;

        /* Paranoia -- Skip dead monsters */
        if (m_ptr->dead) continue;

        /* Skip visible monsters */
        if (m_ptr->ml) continue;

        /* Detect all monsters */
        if (panel_contains(fy, fx)) {

            /* Mega-Hack -- Show the monster */
            m_ptr->ml = TRUE;
            lite_spot(fy, fx);
            flag = detect = TRUE;
        }
    }

    /* Describe the result, then fix the monsters */
    if (flag) {

        msg_print("You sense the presence of monsters!");
        msg_print(NULL);

        /* Mega-Hack -- Fix the monsters */
        update_monsters(FALSE);
    }

    /* Result */
    return (detect);
}


/*
 * Detect all objects on the current panel		-RAK-	
 */
int detect_object(void)
{
    int        i, j, detect = FALSE;

    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid */
            cave_type *c_ptr = &cave[i][j];

            /* Nothing here */
            if (!(c_ptr->i_idx)) continue;

	    /* Skip landmark objects */
            if (i_list[c_ptr->i_idx].tval > TV_MAX_OBJECT) continue;

            /* Note new objects */
            if (!test_lite_bold(i,j)) detect = TRUE;

            /* Hack -- memorize it */
            c_ptr->info |= GRID_MARK;

            /* Redraw */
            lite_spot(i, j);
            detect = TRUE;
        }
    }

    return (detect);
}


/*
 * Locates and displays traps on current panel		-RAK-	
 */
int detect_trap(void)
{
    int         i, j, detect = FALSE;

    cave_type  *c_ptr;
    inven_type *i_ptr;


    /* Scan the current panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];

	    /* Nothing here */
            if (!(c_ptr->i_idx)) continue;

	    /* Get the object */
            i_ptr = &i_list[c_ptr->i_idx];

            /* Notice traps */
            if ((i_ptr->tval == TV_INVIS_TRAP) ||
                (i_ptr->tval == TV_VIS_TRAP)) {

                /* Make traps visible */
                i_ptr->tval = TV_VIS_TRAP;

                /* Hack -- memorize it */
                c_ptr->info |= GRID_MARK;

                /* Redraw */
                lite_spot(i, j);
                detect = TRUE;
            }

            /* Hack -- Identify chests */
            else if (i_ptr->tval == TV_CHEST) {
            
                /* Identify the chest */
                inven_known(i_ptr);
            }
        }
    }

    return (detect);
}



/*
 * Create stairs at the player location
 */
void stair_creation()
{
    cave_type		*c_ptr;
    inven_type		*i_ptr;

    /* Do not destroy useful stuff */
    if (valid_grid(py, px)) {

        delete_object(py, px);

        c_ptr = &cave[py][px];
        c_ptr->i_idx = i_pop();
        i_ptr = &i_list[c_ptr->i_idx];

        if (!dun_level) {
            invcopy(i_ptr, OBJ_DOWN_STAIR);
        }
        else if (is_quest(dun_level)) {
            invcopy(i_ptr, OBJ_UP_STAIR);
        }
        else if (rand_int(2) == 0) {
            invcopy(i_ptr, OBJ_UP_STAIR);
        }
        else {
            invcopy(i_ptr, OBJ_DOWN_STAIR);
        }

        /* Save the location */
        i_ptr->iy = py;
        i_ptr->ix = px;

        /* Stairs are permanent */
        c_ptr->info |= GRID_PERM;
    }

    else {
        msg_print("The object resists the spell.");
    }
}


/*
 * Locates and displays all stairs and secret doors on current panel -RAK-	
 */
int detect_sdoor()
{
    int        i, j, detect = FALSE;

    cave_type *c_ptr;
    inven_type *i_ptr;


    /* Scan the panel */
    for (i = panel_row_min; i <= panel_row_max; i++) {
        for (j = panel_col_min; j <= panel_col_max; j++) {

            /* Access the grid and object */
            c_ptr = &cave[i][j];

	    /* Nothing here */
            if (!(c_ptr->i_idx)) continue;

	    /* Get the object */
            i_ptr = &i_list[c_ptr->i_idx];

            /* Secret doors  */
            if (i_ptr->tval == TV_SECRET_DOOR) {

                /* Hack -- make a closed door */
                invcopy(i_ptr, OBJ_CLOSED_DOOR);

                /* Place it in the dungeon */
                i_ptr->iy = i;
                i_ptr->ix = j;

                /* Hack -- memorize it */
                c_ptr->info |= GRID_MARK;

                /* Redraw */
                lite_spot(i, j);
                detect = TRUE;
            }

            /* Staircases */
            else if ((i_ptr->tval == TV_UP_STAIR) ||
                     (i_ptr->tval == TV_DOWN_STAIR)) {

                /* Hack -- memorize it */
                c_ptr->info |= GRID_MARK;

                /* Redraw */
                lite_spot(i, j);
                detect = TRUE;
            }
        }
    }

    return (detect);
}



/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(inven_type *i_ptr)
{
    if (i_ptr->tval == TV_SWORD) return (TRUE);
    if (i_ptr->tval == TV_HAFTED) return (TRUE);
    if (i_ptr->tval == TV_POLEARM) return (TRUE);
    if (i_ptr->tval == TV_DIGGING) return (TRUE);
    if (i_ptr->tval == TV_BOW) return (TRUE);
    if (i_ptr->tval == TV_BOLT) return (TRUE);
    if (i_ptr->tval == TV_ARROW) return (TRUE);
    if (i_ptr->tval == TV_SHOT) return (TRUE);

    return (FALSE);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(inven_type *i_ptr)
{
    if (i_ptr->tval == TV_DRAG_ARMOR) return (TRUE);
    if (i_ptr->tval == TV_HARD_ARMOR) return (TRUE);
    if (i_ptr->tval == TV_SOFT_ARMOR) return (TRUE);
    if (i_ptr->tval == TV_SHIELD) return (TRUE);
    if (i_ptr->tval == TV_CLOAK) return (TRUE);
    if (i_ptr->tval == TV_CROWN) return (TRUE);
    if (i_ptr->tval == TV_HELM) return (TRUE);
    if (i_ptr->tval == TV_BOOTS) return (TRUE);
    if (i_ptr->tval == TV_GLOVES) return (TRUE);

    return (FALSE);
}



/*
 * Enchants a plus onto an item.                        -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(inven_type *i_ptr, int n, int eflag)
{
    int i, chance, prob;

    bool res = FALSE;

    bool a = artifact_p(i_ptr);


    /* Large piles resist enchantment */
    prob = i_ptr->number * 100;

    /* Missiles are easy to enchant */
    if ((i_ptr->tval == TV_BOLT) ||
        (i_ptr->tval == TV_ARROW) ||
        (i_ptr->tval == TV_SHOT)) {
        prob = prob / 20;
    }

    /* Try "n" times */
    for (i=0; i<n; i++) {

        /* Hack -- Roll for pile resistance */
        if (rand_int(prob) >= 100) continue;

        /* Enchant to hit */
        if (eflag & ENCH_TOHIT) {

            if (i_ptr->tohit < 0) chance = 0;
            else if (i_ptr->tohit > 15) chance = 1000;
            else chance = enchant_table[i_ptr->tohit];

            if ((randint(1000) > chance) && (!a || (randint(7) > 3))) {

                i_ptr->tohit++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (i_ptr->tohit >= 0) && (rand_int(4) == 0)) {
                    msg_print("The curse is broken! ");
                    i_ptr->flags3 &= ~TR3_CURSED;
                    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
                    i_ptr->ident &= ~ID_FELT;
                    inscribe(i_ptr, "uncursed");
                }
            }
        }

        /* Enchant to damage */
        if (eflag & ENCH_TODAM) {

            if (i_ptr->todam < 0) chance = 0;
            else if (i_ptr->todam > 15) chance = 1000;
            else chance = enchant_table[i_ptr->todam];

            if ((randint(1000) > chance) && (!a || (randint(7) > 3))) {

                i_ptr->todam++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (i_ptr->todam >= 0) && (rand_int(4) == 0)) {
                    msg_print("The curse is broken! ");
                    i_ptr->flags3 &= ~TR3_CURSED;
                    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
                    i_ptr->ident &= ~ID_FELT;
                    inscribe(i_ptr, "uncursed");
                }
            }
        }

        /* Enchant to armor class */
        if (eflag & ENCH_TOAC) {

            if (i_ptr->toac < 0) chance = 0;
            else if (i_ptr->toac > 15) chance = 1000;
            else chance = enchant_table[i_ptr->toac];

            if ((randint(1000) > chance) && (!a || (randint(7) > 3))) {

                i_ptr->toac++;
                res = TRUE;

                /* only when you get it above -1 -CFT */
                if (cursed_p(i_ptr) &&
                    (i_ptr->toac >= 0) && (rand_int(4) == 0)) {
                    msg_print("The curse is broken! ");
                    i_ptr->flags3 &= ~TR3_CURSED;
                    i_ptr->flags3 &= ~TR3_HEAVY_CURSE;
                    i_ptr->ident &= ~ID_FELT;
                    inscribe(i_ptr, "uncursed");
                }
            }
        }
    }

    /* Recalculate bonuses */
    p_ptr->update |= PU_BONUS;

    /* Return result */
    return (res);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
    int			item_val;
    bool		floor, okay = FALSE;

    cave_type		*c_ptr = &cave[py][px];
    inven_type		*i_ptr = &i_list[c_ptr->i_idx];

    cptr		pmt = "Enchant which item? ";

    char		out_val[160];
    char		tmp_str[160];


    /* Choose the hook */
    if (num_ac) {
        item_tester_hook = item_tester_hook_armour;
    }
    else {
        item_tester_hook = item_tester_hook_weapon;
    }

    /* Hack -- turn off auto-see mode */
    command_see = FALSE;

    /* Verify the item on the ground */
    floor = item_tester_hook(i_ptr);

    /* Get an item to enchant (allow floor) or abort */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, floor)) return (FALSE);

    /* Get the item (if in inven/equip) */
    if (item_val >= 0) i_ptr = &inventory[item_val];

    /* Cancel "auto-see" */
    command_see = FALSE;


    /* Description */
    objdes(tmp_str, i_ptr, FALSE);

    /* Describe */
    sprintf(out_val, "%s %s glow%s brightly!",
            ((item_val >= 0) ? "Your" : "The"), tmp_str,
            ((i_ptr->number > 1) ? "" : "s"));
    message(out_val, 0);

    /* Enchant */
    if (enchant(i_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
    if (enchant(i_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
    if (enchant(i_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

    /* Detect "failure" */
    if (!okay) {
        if (flush_failure) flush();
        msg_print("The enchantment failed.");
    }

    /* Something happened */
    return (TRUE);
}


/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell()
{
    int			item_val;
    bool		floor;

    cave_type		*c_ptr = &cave[py][px];
    inven_type		*i_ptr = &i_list[c_ptr->i_idx];

    cptr		pmt = "Identify which item? ";

    char		out_val[160];
    char		tmp_str[160];


    /* Hack -- turn off auto-see mode */
    command_see = FALSE;

    /* Verify the item on the ground */
    floor = (i_ptr->tval && (i_ptr->tval < TV_MAX_PICK_UP));

    /* Get an item to identify (allow floor) or abort */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, floor)) return (FALSE);

    /* Get the item (if in inven/equip) */
    if (item_val >= 0) i_ptr = &inventory[item_val];

    /* Cancel "auto-see" */
    command_see = FALSE;


    /* Identify it fully */
    inven_aware(i_ptr);
    inven_known(i_ptr);

    /* Recalculate bonuses */
    p_ptr->update |= PU_BONUS;	

    /* Description */
    objdes(tmp_str, i_ptr, TRUE);

    /* Describe */
    if (item_val >= INVEN_WIELD) {
        (void)sprintf(out_val, "%s: %s. ",
                      describe_use(item_val), tmp_str);
        message(out_val, 0);
    }
    else if (item_val >= 0) {
        (void)sprintf(out_val, "(%c) %s. ",
                      index_to_label(item_val), tmp_str);
        message(out_val, 0);
    }
    else {
        (void)sprintf(out_val, "On the ground: %s. ", tmp_str);
        message(out_val, 0);
    }

    /* Something happened */
    return (TRUE);
}




/*
 * Fully "identify" an object in the inventory	-BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully()
{
    int			i = 0, j, k, item_val;
    bool		floor;

    cave_type		*c_ptr = &cave[py][px];
    inven_type		*i_ptr = &i_list[c_ptr->i_idx];

    cptr		pmt = "Fully *identify* which item? ";

    char		out_val[160];
    char		tmp_str[160];

    cptr		info[128];


    /* Hack -- turn off auto-see mode */
    command_see = FALSE;

    /* Verify the item on the ground */
    floor = (i_ptr->tval && (i_ptr->tval < TV_MAX_PICK_UP));

    /* Get an item to identify (allow floor) or abort */
    if (!get_item(&item_val, pmt, 0, INVEN_TOTAL-1, floor)) return (FALSE);

    /* Get the item (if in inven/equip) */
    if (item_val >= 0) i_ptr = &inventory[item_val];

    /* Hack -- turn off auto-see mode */
    command_see = FALSE;


    /* Identify it fully */
    inven_aware(i_ptr);
    inven_known(i_ptr);

    /* Recalculate bonuses */
    p_ptr->update |= PU_BONUS;

    /* Description */
    objdes(tmp_str, i_ptr, TRUE);

    /* Describe */
    if (item_val >= INVEN_WIELD) {
        (void)sprintf(out_val, "%s: %s. ",
                      describe_use(item_val), tmp_str);
        message(out_val, 0);
    }
    else if (item_val >= 0) {
        (void)sprintf(out_val, "(%c) %s. ",
                      index_to_label(item_val), tmp_str);
        message(out_val, 0);
    }
    else {
        (void)sprintf(out_val, "On the ground: %s. ", tmp_str);
        message(out_val, 0);
    }



    /* Hack -- All done if not wearable */
    if (!wearable_p(i_ptr)) return (TRUE);


    /* And then describe it fully */

    if (i_ptr->flags1 & TR1_STR) {
        info[i++] = "It affects your strength.";
    }
    if (i_ptr->flags1 & TR1_INT) {
        info[i++] = "It affects your intelligence.";
    }
    if (i_ptr->flags1 & TR1_WIS) {
        info[i++] = "It affects your wisdom.";
    }
    if (i_ptr->flags1 & TR1_DEX) {
        info[i++] = "It affects your dexterity.";
    }
    if (i_ptr->flags1 & TR1_CON) {
        info[i++] = "It affects your constitution.";
    }
    if (i_ptr->flags1 & TR1_CHR) {
        info[i++] = "It affects your charisma.";
    }

    if (i_ptr->flags1 & TR1_STEALTH) {
        info[i++] = "It affects your stealth.";
    }
    if (i_ptr->flags1 & TR1_SEARCH) {
        info[i++] = "It affects your searching.";
    }
    if (i_ptr->flags1 & TR1_INFRA) {
        info[i++] = "It affects your infravision.";
    }
    if (i_ptr->flags1 & TR1_ATTACK_SPD) {
        info[i++] = "It affects your attack speed.";
    }
    if (i_ptr->flags1 & TR1_SPEED) {
        info[i++] = "It affects your speed.";
    }
    if (i_ptr->flags1 & TR1_TUNNEL) {
        info[i++] = "It affects your ability to tunnel.";
    }

    if (i_ptr->flags1 & TR1_BRAND_ACID) {
        info[i++] = "It does extra damage from acid.";
    }
    if (i_ptr->flags1 & TR1_BRAND_ELEC) {
        info[i++] = "It does extra damage from electricity.";
    }
    if (i_ptr->flags1 & TR1_BRAND_FIRE) {
        info[i++] = "It does extra damage from fire.";
    }
    if (i_ptr->flags1 & TR1_BRAND_COLD) {
        info[i++] = "It does extra damage from frost.";
    }

    if (i_ptr->flags1 & TR1_IMPACT) {
        info[i++] = "It can cause earthquakes.";
    }

    if (i_ptr->flags1 & TR1_KILL_DRAGON) {
        info[i++] = "It is a great bane of dragons.";
    }
    else if (i_ptr->flags1 & TR1_SLAY_DRAGON) {
        info[i++] = "It is especially deadly against dragons.";
    }
    if (i_ptr->flags1 & TR1_SLAY_ORC) {
        info[i++] = "It is especially deadly against orcs.";
    }
    if (i_ptr->flags1 & TR1_SLAY_TROLL) {
        info[i++] = "It is especially deadly against trolls.";
    }
    if (i_ptr->flags1 & TR1_SLAY_GIANT) {
        info[i++] = "It is especially deadly against giants.";
    }
    if (i_ptr->flags1 & TR1_SLAY_DEMON) {
        info[i++] = "It strikes at demons with holy wrath.";
    }
    if (i_ptr->flags1 & TR1_SLAY_UNDEAD) {
        info[i++] = "It strikes at undead with holy wrath.";
    }
    if (i_ptr->flags1 & TR1_SLAY_EVIL) {
        info[i++] = "It fights against evil with holy fury.";
    }
    if (i_ptr->flags1 & TR1_SLAY_ANIMAL) {
        info[i++] = "It is especially deadly against natural creatures.";
    }

    if (i_ptr->flags2 & TR2_SUST_STR) {
        info[i++] = "It sustains your strength.";
    }
    if (i_ptr->flags2 & TR2_SUST_INT) {
        info[i++] = "It sustains your intelligence.";
    }
    if (i_ptr->flags2 & TR2_SUST_WIS) {
        info[i++] = "It sustains your wisdom.";
    }
    if (i_ptr->flags2 & TR2_SUST_DEX) {
        info[i++] = "It sustains your dexterity.";
    }
    if (i_ptr->flags2 & TR2_SUST_CON) {
        info[i++] = "It sustains your constitution.";
    }
    if (i_ptr->flags2 & TR2_SUST_CHR) {
        info[i++] = "It sustains your charisma.";
    }

    if (i_ptr->flags2 & TR2_IM_ACID) {
        info[i++] = "It provides immunity to acid.";
    }
    if (i_ptr->flags2 & TR2_IM_ELEC) {
        info[i++] = "It provides immunity to electricity.";
    }
    if (i_ptr->flags2 & TR2_IM_FIRE) {
        info[i++] = "It provides immunity to fire.";
    }
    if (i_ptr->flags2 & TR2_IM_COLD) {
        info[i++] = "It provides immunity to cold.";
    }
    if (i_ptr->flags2 & TR2_IM_POIS) {
        info[i++] = "It provides immunity to poison.";
    }

    if (i_ptr->flags2 & TR2_FREE_ACT) {
        info[i++] = "It provides immunity to paralysis.";
    }
    if (i_ptr->flags2 & TR2_HOLD_LIFE) {
        info[i++] = "It provides immunity to life draining.";
    }

    if (i_ptr->flags2 & TR2_RES_ACID) {
        info[i++] = "It provides resistance to acid.";
    }
    if (i_ptr->flags2 & TR2_RES_ELEC) {
        info[i++] = "It provides resistance to electricity.";
    }
    if (i_ptr->flags2 & TR2_RES_FIRE) {
        info[i++] = "It provides resistance to fire.";
    }
    if (i_ptr->flags2 & TR2_RES_COLD) {
        info[i++] = "It provides resistance to cold.";
    }
    if (i_ptr->flags2 & TR2_RES_POIS) {
        info[i++] = "It provides resistance to poison.";
    }

    if (i_ptr->flags2 & TR2_RES_LITE) {
        info[i++] = "It provides resistance to light.";
    }
    if (i_ptr->flags2 & TR2_RES_DARK) {
        info[i++] = "It provides resistance to dark.";
    }

    if (i_ptr->flags2 & TR2_RES_BLIND) {
        info[i++] = "It provides resistance to blindness.";
    }
    if (i_ptr->flags2 & TR2_RES_CONF) {
        info[i++] = "It provides resistance to confusion.";
    }
    if (i_ptr->flags2 & TR2_RES_SOUND) {
        info[i++] = "It provides resistance to sound.";
    }
    if (i_ptr->flags2 & TR2_RES_SHARDS) {
        info[i++] = "It provides resistance to shards.";
    }

    if (i_ptr->flags2 & TR2_RES_NETHER) {
        info[i++] = "It provides resistance to nether.";
    }
    if (i_ptr->flags2 & TR2_RES_NEXUS) {
        info[i++] = "It provides resistance to nexus.";
    }
    if (i_ptr->flags2 & TR2_RES_CHAOS) {
        info[i++] = "It provides resistance to chaos.";
    }
    if (i_ptr->flags2 & TR2_RES_DISEN) {
        info[i++] = "It provides resistance to disenchantment.";
    }

    if (i_ptr->flags3 & TR3_FEATHER) {
        info[i++] = "It induces feather falling.";
    }
    if (i_ptr->flags3 & TR3_LITE) {
        info[i++] = "It provides permanent light.";
    }
    if (i_ptr->flags3 & TR3_SEE_INVIS) {
        info[i++] = "It allows you to see invisible monsters.";
    }
    if (i_ptr->flags3 & TR3_TELEPATHY) {
        info[i++] = "It gives telepathic powers.";
    }
    if (i_ptr->flags3 & TR3_SLOW_DIGEST) {
        info[i++] = "It slows your metabolism.";
    }
    if (i_ptr->flags3 & TR3_REGEN) {
        info[i++] = "It speeds your regenerative powers.";
    }

    if (i_ptr->flags3 & TR3_XTRA_MIGHT) {
        info[i++] = "It fires missiles with extra might.";
    }
    if (i_ptr->flags3 & TR3_XTRA_SHOTS) {
        info[i++] = "It fires missiles excessively fast.";
    }

    if (i_ptr->flags3 & TR3_ACTIVATE) {
        info[i++] = "It can be activated.";
    }
    if (i_ptr->flags3 & TR3_DRAIN_EXP) {
        info[i++] = "It drains experience.";
    }
    if (i_ptr->flags3 & TR3_TELEPORT) {
        info[i++] = "It induces random teleportation.";
    }
    if (i_ptr->flags3 & TR3_AGGRAVATE) {
        info[i++] = "It aggravates nearby creatures.";
    }

    if (i_ptr->flags3 & TR3_BLESSED) {
        info[i++] = "It has been blessed by the gods.";
    }

    if (i_ptr->flags3 & TR3_PERMA_CURSE) {
        info[i++] = "It is permanently cursed.";
    }
    else if (i_ptr->flags3 & TR3_HEAVY_CURSE) {
        info[i++] = "It is heavily cursed.";
    }
    else if (i_ptr->flags3 & TR3_CURSED) {
        info[i++] = "It is cursed.";
    }

    if (i_ptr->flags3 & TR3_IGNORE_ACID) {
        info[i++] = "It cannot be harmed by acid.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_ELEC) {
        info[i++] = "It cannot be harmed by electricity.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_FIRE) {
        info[i++] = "It cannot be harmed by fire.";
    }
    if (i_ptr->flags3 & TR3_IGNORE_COLD) {
        info[i++] = "It cannot be harmed by cold.";
    }


    /* No special effects */
    if (!i) return (TRUE);


    /* Save the screen */
    save_screen();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

    /* Label the information */
    prt("     Item Attributes:", 1, 15);

    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++) {

        /* Show the info */
        prt(info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j+1 < i)) {
            prt("-- more --", k, 15);
            inkey();
            for ( ; k > 2; k--) prt("", k, 15);
        }
    }

    /* Wait for it */
    prt("[Press any key to continue]", k, 15);
    inkey();

    /* Restore the screen */
    restore_screen();


    /* Return the index of the item */
    return (TRUE);
}






/*
 * Recharge a wand/staff/rod.  XXX XXX Does not work on stacked items.
 *
 * recharge I = recharge(20) = 1/6 failure for empty 10th level wand
 * recharge II = recharge(60) = 1/10 failure for empty 10th level wand
 * make it harder to recharge high level, and highly charged wands
 *
 * XXX XXX XXX Should probably not "destroy" over-charged items, unless
 * we "replace" them by, say, a broken stick or some such.
 */
bool recharge(int num)
{
    int                 i, j, i1, i2, item_val, lev;
    inven_type		*i_ptr;

    /* No range found yet */
    i1 = 999, i2 = -1;

    /* Check for wands */
    if (find_range(TV_WAND, &i, &j)) {
        if (i < i1) i1 = i;
        if (j > i2) i2 = j;
    }

    /* Check for staffs */
    if (find_range(TV_STAFF, &i, &j)) {
        if (i < i1) i1 = i;
        if (j > i2) i2 = j;
    }

    /* Hack -- Check for rods */
    if (find_range(TV_ROD, &i, &j)) {
        if (i < i1) i1 = i;
        if (j > i2) i2 = j;
    }

    /* Quick check */
    if (i1 > i2) {
        msg_print("You have nothing to recharge.");
        return (FALSE);
    }


    /* Hack -- turn off auto-see mode */
    command_see = FALSE;

    /* Ask for it */
    if (!get_item(&item_val, "Recharge which item? ", i1, i2, FALSE)) return (FALSE);

    /* Hack -- turn off auto-see mode */
    command_see = FALSE;

    /* Get the item */
    i_ptr = &inventory[item_val];

    /* Verify item */
    if ((i_ptr->tval != TV_WAND) &&
        (i_ptr->tval != TV_STAFF) &&
        (i_ptr->tval != TV_ROD)) {

        message("Oops.  That item cannot be recharged.", 0);
        return (FALSE);
    }


    /* Hack -- refuse to recharge stacked items */
    if (i_ptr->number > 1) {

        message("Oops.  You cannot recharge stacked items.", 0);
        return (FALSE);
    }


    /* Extract the object "level" */
    lev = k_list[i_ptr->k_idx].level;

    /* Recharge a rod */
    if (i_ptr->tval == TV_ROD) {

        u16b t, t_p = i_ptr->pval;

        /* Back-fire */
        if (randint((100 - lev + num) / 5) == 1) {
            msg_print("The recharge backfires, draining the rod further!");
            /* don't overflow... */
            if (t_p < 10000) i_ptr->pval = (t_p + 100) * 2;
        }

        /* Recharge */
        else {
            /* rechange amount */
            t = (u16b) (num * damroll(2, 4));
            if (t_p < t) i_ptr->pval = 0;
            else i_ptr->pval = t_p - t;
        }
    }

    /* Recharge wand/staff */
    else {

        /* Back-fire */
        if (randint((num + 100 - lev - (10 * i_ptr->pval)) / 15) == 1) {
            msg_print("There is a bright flash of light.");
            inven_item_increase(item_val, -1);
            inven_item_optimize(item_val);
        }

        /* Recharge */
        else {
            num = (num / (lev + 2)) + 1;
            i_ptr->pval += 2 + randint(num);

            /* Hack -- we no longer "know" the item */
            i_ptr->ident &= ~ID_KNOWN;

            /* Hack -- we no longer think the item is empty */
            i_ptr->ident &= ~ID_EMPTY;
        }
    }

    /* Something was done */
    return (TRUE);
}







/*
 * Hooks for the old "player spells"
 */

bool project_hook(int typ, int dir, int dam, int flg)
{
    int tx, ty;

    /* Pass through the target if needed */
    flg |= (PROJECT_THRU);

    /* Use the given direction */
    tx = px + ddx[dir];
    ty = py + ddy[dir];

    /* Use an actual "target" */
    if ((dir == 0) && target_okay()) {
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target", do NOT explode */
    return (project(1, 0, ty, tx, dam, typ, flg));
}

bool fire_bolt(int typ, int dir, int y, int x, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(typ, dir, dam, flg));
}

bool line_spell(int typ, int dir, int y, int x, int dam)
{
    /* Go until we have to stop, do "beam" damage to everyone */
    /* Also, affect all grids (NOT objects) we pass through */
    int flg = PROJECT_BEAM | PROJECT_GRID;
    return (project_hook(typ, dir, dam, flg));
}

bool lite_line(int dir, int y, int x)
{
    return (line_spell(GF_LITE_WEAK, dir, y, x, damroll(6, 8)));
}

bool drain_life(int dir, int y, int x, int dam)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool td_destroy2(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_all(int dir, int y, int x)
{
    int flg = PROJECT_BEAM | PROJECT_ITEM | PROJECT_HIDE;
    return (project_hook(GF_KILL_TRAP, dir, 0, flg));
}

bool td_destroy()
{
    int flg = PROJECT_ITEM | PROJECT_HIDE;
    return (project(1, 1, py, px, 0, GF_KILL_DOOR, flg));
}

bool door_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(1, 1, py, px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation()
{
    int flg = PROJECT_ITEM | PROJECT_GRID | PROJECT_HIDE;
    return (project(1, 1, py, px, 0, GF_MAKE_TRAP, flg));
}

bool heal_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SPEED, dir, 0, flg));
}

bool slow_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int dir, int y, int x, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CONF, dir, plev, flg));
}

bool fear_monster(int dir, int y, int x, int plev)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_SCARE, dir, plev, flg));
}

bool poly_monster(int dir, int y, int x)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int dir, int y, int x)
{
    int flg = PROJECT_STOP;
    return (project_hook(GF_OLD_CLONE, dir, 0, flg));
}

bool teleport_monster(int dir, int y, int x)
{
    int flg = PROJECT_BEAM;
    return (project_hook(GF_OLD_TPORT, dir, MAX_SIGHT * 5, flg));
}



bool lite_area(int y, int x, int dam, int rad)
{
    /* Hack -- Message */
    if ((y == py) && (x == px) && (p_ptr->blind < 1)) {
        msg_print("You are surrounded by a white light.");
    }

    /* Hook into the "project()" function */
    return (project(1, rad, y, x, dam, GF_LITE_WEAK, PROJECT_GRID));
}

bool unlite_area(int y, int x)
{
    /* Hack -- Message */
    if ((y == py) && (x == px) && (p_ptr->blind < 1)) {
        msg_print("Darkness surrounds you.");
    }

    /* Simple "unlite_area" attack centered on player -- hurts lite hounds */
    return (project(1, 3, y, x, 10, GF_DARK_WEAK, PROJECT_GRID));
}



/*
 * Cast a ball spell
 */
bool fire_ball(int typ, int dir, int ppy, int ppx, int dam_hp, int max_dis)
{
    int tx, ty;

    int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_STOP;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Use an actual "target" */
    if ((dir == 0) && target_okay()) {
        flg &= ~PROJECT_STOP;
        tx = target_col;
        ty = target_row;
    }

    /* Analyze the "dir" and the "target".  Hurt items on floor. */
    return (project(1, max_dis, ty, tx, dam_hp, typ, flg));
}





/*
 * Lightning ball in all directions    -SM-
 */
bool starball(int y, int x)
{
    int i;

    for (i = 1; i <= 9; i++) {
        if (i != 5) {
            fire_ball(GF_ELEC, i, y, x, 150, 3);
        }
    }

    return (TRUE);
}




/*
 * Light line in all directions
 */
bool starlite(int y, int x)
{
    int i;

    if (p_ptr->blind < 1) {
        msg_print("The end of the staff bursts into a blue shimmering light.");
    }
    for (i = 1; i <= 9; i++) {
        if (i != 5) lite_line(i, y, x);
    }

    return (TRUE);
}


