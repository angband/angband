/* File: creature.c */

/* Purpose: process the monsters */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




/*
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player, checking
 * for visibility (natural, infravision, see-invis, telepathy),
 * updating the monster visibility flag, redrawing or erasing the
 * monster when the visibility changes, and taking note of any
 * "visual" features of the monster (cold-blooded, invisible, etc).
 *
 * The only monster fields that are changed here are "cdis" and "ml".
 *
 * There are a few cases where the calling routine knows that the
 * distance from the player to the monster has not changed, and so
 * we have a special parameter to request distance computation.
 * This lets many calls to this function run very quickly.
 *
 * Note that it is important to call this function every time a monster
 * moves, for that monster, and every time the player moves, for all
 * monsters, and every time the player state changes, for all monsters.
 * But we only need to recalculate distance for the first two situations.
 *
 * Note in particular that any change in the player's "state" that
 * affects visibility *must* call the update_monsters() function.
 * This includes blindness, infravision, and see invisibile.
 *
 * The routines that actually move the monsters call this routine
 * directly, and the ones that move the player call update_monsters().
 *
 * Routines that light/darken rooms, and cause player blindness, do
 * it too.  Keep it in mind if a monster ever acquires the ability
 * to make itself invisible without moving.
 *
 * Note that this function is called once per monster every time the
 * player moves, so it is important to optimize it for monsters which
 * are far away.  Note the optimization which skips monsters which
 * are far away and were invisible last turn.
 *
 * Total Hack -- allow wizards to "remember" where monsters are once
 * they have been seen once, or "peeked" via "unblock all monsters".
 * This is convenient for examining a dungeon level after creation.
 */
void update_mon(int m_idx, bool dist)
{
    monster_type  *m_ptr = &m_list[m_idx];
    monster_lore *l_ptr = &l_list[m_ptr->r_idx];

    /* The current monster location */
    int fy = m_ptr->fy;
    int fx = m_ptr->fx;

    /* Can the monster be sensed in any way? */
    bool flag = FALSE;

    /* Various extra flags */
    bool do_viewable = FALSE;
    bool do_telepathy = FALSE;
    bool do_empty_mind = FALSE;
    bool do_weird_mind = FALSE;
    bool do_invisible = FALSE;
    bool do_cold_blood = FALSE;


    /* Calculate distance */
    if (dist) {

        /* Calculate the approximate distance from player */
        int dy = (py > fy) ? (py - fy) : (fy - py);
        int dx = (px > fx) ? (px - fx) : (fx - px);
        int d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

        /* Save the distance (in a byte) */
        m_ptr->cdis = (d < 255) ? d : 255;
    }


    /* Efficiency -- Ignore distant unseen monsters */
    if ((m_ptr->cdis > MAX_SIGHT) && !m_ptr->ml) return;


    /* The monster is on the current "panel", and the player can see */
    if (panel_contains(fy, fx) && (!(p_ptr->blind))) {

        /* Get the monster race (to check flags) */
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        /* Telepathy can see all "nearby" monsters with "minds" */
        if (p_ptr->telepathy && (m_ptr->cdis <= MAX_SIGHT)) {

            /* Empty mind, no telepathy */
            if (r_ptr->rflags2 & RF2_EMPTY_MIND) {
                do_empty_mind = TRUE;
            }
            
            /* Weird mind, occasional telepathy */
            else if (r_ptr->rflags2 & RF2_WEIRD_MIND) {
                do_weird_mind = TRUE;
                if (rand_int(10) == 0) do_telepathy = TRUE;
            }
            
            /* Normal mind, allow telepathy */
            else {
                do_telepathy = TRUE;
            }
            
            /* Apply telepathy */
            if (do_telepathy) {

                /* Apply telepathy */
                flag = TRUE;

                /* Hack -- Memorize mental flags */
                if (r_ptr->rflags2 & RF2_SMART) l_ptr->flags2 |= RF2_SMART;
                if (r_ptr->rflags2 & RF2_STUPID) l_ptr->flags2 |= RF2_STUPID;
            }
        }

        /* Normal line of sight */
        if (player_has_los_bold(fy, fx)) {

            /* Remember line of sight */
            do_viewable = TRUE;
            
            /* Infravision is able to see "nearby" monsters */
            if (p_ptr->see_infra &&
                (m_ptr->cdis <= (unsigned)(p_ptr->see_infra))) {

                /* Infravision only works on "warm" creatures */
                /* Below, we will need to know that infravision failed */
                if (r_ptr->rflags2 & RF2_COLD_BLOOD) do_cold_blood = TRUE;

                /* Infravision works */
                if (!do_cold_blood) flag = TRUE;
            }

            /* Check for "illumination" of the monster grid */
            if (player_can_see_bold(fy, fx)) {

                /* Take note of invisibility */
                if (r_ptr->rflags2 & RF2_INVISIBLE) do_invisible = TRUE;

                /* Visible, or detectable, monsters get seen */
                if (!do_invisible || p_ptr->see_inv) flag = TRUE;
            }
        }
    }


    /* The monster is now visible */
    if (flag || wizard) {

        /* It was previously unseen */
        if (!m_ptr->ml) {

            /* Appearing monsters can disturb the player a lot */
            if (disturb_enter) disturb(1, 0);

            /* Count "fresh" sightings */
            if (l_ptr->sights < MAX_SHORT) l_ptr->sights++;

            /* Mark Monster as visible */
            m_ptr->ml = TRUE;

            /* Draw the monster */
            lite_spot(fy, fx);
        }

        /* Memorize various observable flags */
        if (do_empty_mind) l_ptr->flags2 |= RF2_EMPTY_MIND;
        if (do_weird_mind) l_ptr->flags2 |= RF2_WEIRD_MIND;
        if (do_cold_blood) l_ptr->flags2 |= RF2_COLD_BLOOD;
        if (do_invisible) l_ptr->flags2 |= RF2_INVISIBLE;
    }

    /* The monster has disappeared */
    else {

        /* It was previously seen */
        if (m_ptr->ml) {

            /* Mark monster as hidden */
            m_ptr->ml = FALSE;

            /* Disappearing monsters can "disturb" player. */
            if (disturb_leave) disturb(0, 0);

            /* Erase the monster */
            lite_spot(fy, fx);
        }
    }
}




/*
 * This function simply updates all the (non-dead) monsters (see above).
 */
void update_monsters(bool dist)
{
    int          i;

    /* Update each (live) monster */
    for (i = MIN_M_IDX; i < m_max; i++) {

        /* Paranoia -- skip dead monsters */
        if (m_list[i].dead) continue;

        /* Update the monster */
        update_mon(i, dist);
    }
}



/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency.  Like not using "mod" or "div" when possible.  And
 * attempting to check the conditions in an optimal order.  Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];

#ifdef ALLOW_TERROR

    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    u16b p_lev, m_lev;
    u16b p_chp, p_mhp;
    u16b m_chp, m_mhp;
    u32b p_val, m_val;

#endif

    /* Keep monsters from running too far away */
    if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

    /* All "afraid" monsters will run away */
    if (m_ptr->monfear) return (TRUE);

#ifdef ALLOW_TERROR

    /* Nearby monsters will not become terrified */
    if (m_ptr->cdis <= 5) return (FALSE);

    /* Examine player power (level) */
    p_lev = p_ptr->lev;

    /* Examine monster power (level plus morale) */
    m_lev = r_ptr->level + (m_idx & 0x08) + 25;

    /* Optimize extreme cases below */
    if (m_lev > p_lev + 4) return (FALSE);
    if (m_lev + 4 <= p_lev) return (TRUE);

    /* Examine player health */
    p_chp = p_ptr->chp, p_mhp = p_ptr->mhp;

    /* Examine monster health */
    m_chp = m_ptr->hp, m_mhp = m_ptr->maxhp;

    /* Prepare to optimize the calculation */
    p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
    m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

    /* Strong players scare strong monsters */
    if (p_val * m_mhp > m_val * p_mhp) return (TRUE);

#endif

    /* Assume no terror */
    return (FALSE);
}


#ifdef MONSTER_FLOW

/*
 * Choose the "best" direction for "flowing"
 *
 * Note that ghosts and rock-eaters are never allowed to "flow"
 *
 * Prefer "non-diagonal" directions, but twiddle them a little
 * to angle slightly towards the player's actual location.
 *
 * Allow very perceptive monsters to track old "spoor" left by
 * previous locations occupied by the player.  This will tend
 * to have monsters end up either near the player or on a grid
 * recently occupied by the player (and left via "teleport").
 * Note that if "smell" is turned on, all monsters get vicious.
 * Also note that teleporting away from a location will cause
 * the monsters who were chasing you to converge on that location
 * as long as you are still near enough to "annoy" them without
 * being close enough to chase directly.  I have no idea what will
 * happen if you combine "smell" with low "aaf" values.
 */
static bool get_moves_aux(int m_idx, int *yp, int *xp)
{
    int i, y, x, y1, x1, when = 0, cost = 999;

    cave_type *c_ptr;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Monster flowing disabled */
    if (!flow_by_sound) return (FALSE);

    /* Monster can go through rocks */
    if (r_ptr->rflags2 & RF2_PASS_WALL) return (FALSE);
    if (r_ptr->rflags2 & RF2_KILL_WALL) return (FALSE);

    /* Monster location */
    y1 = m_ptr->fy;
    x1 = m_ptr->fx;

    /* Monster grid */
    c_ptr = &cave[y1][x1];

    /* The player is not currently near the monster grid */
    if (c_ptr->when < cave[py][px].when) {

        /* The player has never been near the monster grid */
        if (!c_ptr->when) return (FALSE);

        /* The monster is not allowed to track the player */
        if (!flow_by_smell) return (FALSE);
    }

    /* Monster is too far away to notice the player */
    if (c_ptr->cost > MONSTER_FLOW_DEPTH) return (FALSE);
    if (c_ptr->cost > r_ptr->aaf) return (FALSE);

    /* Hack -- Player can see us, run towards him */
    if (player_has_los_bold(y1, x1)) return (FALSE);

    /* Check nearby grids, diagonals first */
    for (i = 7; i >= 0; i--) {

        /* Get the location */
        y = y1 + ddy[ddd[i]];
        x = x1 + ddx[ddd[i]];

        /* Ignore illegal locations */
        if (!cave[y][x].when) continue;

        /* Ignore ancient locations */
        if (cave[y][x].when < when) continue;

        /* Ignore distant locations */
        if (cave[y][x].cost > cost) continue;

        /* Save the cost and time */
        when = cave[y][x].when;
        cost = cave[y][x].cost;

        /* Hack -- Save the "twiddled" location */
        (*yp) = py + 100 * ddy[ddd[i]];
        (*xp) = px + 100 * ddx[ddd[i]];
    }

    /* No legal move (?) */
    if (!when) return (FALSE);

    /* Success */
    return (TRUE);
}

#endif


#ifdef WDT_TRACK_OPTIONS

/*
 * Set the "track" field for a monster
 */
static void set_t_bit(int m_idx, bool player, bool direct)
{
    monster_type *m_ptr = &m_list[m_idx];
    
    m_ptr->t_bit = 0;
    if (player) m_ptr->t_bit |= MTB_PLAYER;
    if (direct) m_ptr->t_bit |= MTB_DIRECT;
}


/*
 * Update the target
 */
static void get_target(int m_idx)
{
    int i, y, x, y1, x1, best_tracks, difference;

    cave_type *c_ptr;

    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    /* Not following */
    if (!track_follow) {

        /* Use the player */
        m_ptr->ty = py;
        m_ptr->tx = px;

        /* Single turn duration */
        m_ptr->t_dur = 1;
        
        /* Follow the player */
        set_t_bit(m_idx, TRUE, TRUE);

        return;
    }

    /* Monster location */
    y1 = m_ptr->fy;
    x1 = m_ptr->fx;

    /* Determine if there is "line of sight" from player to monster */
    if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) {

        /* Target the player */
        m_ptr->ty = py;
        m_ptr->tx = px;

        /* Extended duration */
        m_ptr->t_dur = 10;
        
        /* Target the player */
        set_t_bit(m_idx, TRUE, TRUE);
        return;
    }
    
    /* Determine if player is in range of ESP */
    if ((r_ptr->rflags2 & (RF2_PASS_WALL | RF2_KILL_WALL)) &&
        (m_ptr->cdis < r_ptr->aaf)) {

        m_ptr->ty = py;
        m_ptr->tx = px;

        m_ptr->t_dur = 1;
        
        set_t_bit(m_idx, TRUE, FALSE);

        return;
    }
    
    /* Check to see if we used to know where the player was... */
    if (m_ptr->t_bit & MTB_PLAYER) {

        byte target_x = m_ptr->tx;
        byte target_y = m_ptr->ty;

        int duration = m_ptr->t_dur;

        bool player = TRUE;

        bool direct = (m_ptr->t_bit & MTB_DIRECT) ? TRUE : FALSE;
        
        /* ... and whether it's obviously not there... */
        if ((m_ptr->fx == m_ptr->tx) && (m_ptr->fy == m_ptr->ty)) {
            direct = FALSE;
            player = FALSE;
            duration = 0;
        }
        
        /* ... whether it just vanished ... */
        else if (direct) {
            player = TRUE;
            direct = FALSE;
            duration = r_ptr->aaf / 3;
            target_x = m_ptr->tx;
            target_y = m_ptr->ty;
        }

        /* Or whether we've been tracking it for a while now */
        /* and should decrease our confidence in that location. */
        else {
            if (duration) duration--;
            else player = FALSE;
        }

        m_ptr->ty = target_y;
        m_ptr->tx = target_x;

        m_ptr->t_dur = duration;
        
        set_t_bit(m_idx, player, direct);

        return;
    }


    /* Monster grid (?) */
    c_ptr = &cave[y1][x1];
    best_tracks = c_ptr->track;

    m_ptr->ty = y1;
    m_ptr->tx = x1;

    m_ptr->t_dur = 1;
    
    set_t_bit(m_idx, FALSE, FALSE);

    /* Check adjacent grids, diagonals last (to give them the best chance) */
    for (i = 0; i < 8; i++) {

        /* Get the location */
        y = y1 + ddy[ddd[i]];
        x = x1 + ddx[ddd[i]];

        /* Get the grid */
        c_ptr = &cave[y][x];

        /* Don't even look at walls */
        if (c_ptr->info & GRID_WALL_MASK) continue;

        /* compare this location to the best one so far */
        difference = best_tracks - c_ptr->track;

        /* Ignore less trampled locations */
        if (difference > 0) continue;

        if (!difference) {
            /* XXX XXX later, later. */
        }

        /* Save the new best location */
        best_tracks = c_ptr->track;

        /* Remember the place */
        m_ptr->ty = y;
        m_ptr->tx = x;

        m_ptr->t_dur = 1;
        
        /* Do not think the player is there */
        set_t_bit(m_idx, FALSE, FALSE);
    }

    /* Success */
    return;
}

#endif



/*
 * Choose correct directions for monster movement	-RAK-	
 *
 * Perhaps monster fear should only work when player can be seen?
 *
 * This function does not have to worry about monster mobility
 */
static void get_moves(int m_idx, int *mm)
{
    monster_type *m_ptr = &m_list[m_idx];

    int y, ay, x, ax;

    int y2 = py;
    int x2 = px;

    int move_val = 0;


#ifdef MONSTER_FLOW
    /* Get a "desired destination" (may be off the map) */
    if (flow_by_sound) (void)get_moves_aux(m_idx, &y2, &x2);
#endif

    /* Extract the "pseudo-direction" */
    y = m_ptr->fy - y2;
    x = m_ptr->fx - x2;

#ifdef WDT_TRACK_OPTIONS
    /* Track the player */
    if (track_follow) {
        y = m_ptr->fy - m_ptr->ty;
        x = m_ptr->fx - m_ptr->tx;
    }
#endif


    /* Apply fear if possible and necessary */
    if (mon_will_run(m_idx)) {

        /* XXX Not very "smart" */
        y = (-y), x = (-x);
    }


    /* Extract the "absolute distances" */
    ax = ABS(x);
    ay = ABS(y);

    /* Do something weird */
    if (y < 0) move_val += 8;
    if (x > 0) move_val += 4;

    /* Prevent the diamond maneuvre */
    if (ay > (ax << 1)) {
        move_val++;
        move_val++;
    }
    else if (ax > (ay << 1)) {
        move_val++;
    }

    /* Extract some directions */
    switch (move_val) {
      case 0:
        mm[0] = 9;
        if (ay > ax) {
            mm[1] = 8;
            mm[2] = 6;
            mm[3] = 7;
            mm[4] = 3;
        }
        else {
            mm[1] = 6;
            mm[2] = 8;
            mm[3] = 3;
            mm[4] = 7;
        }
        break;
      case 1:
      case 9:
        mm[0] = 6;
        if (y < 0) {
            mm[1] = 3;
            mm[2] = 9;
            mm[3] = 2;
            mm[4] = 8;
        }
        else {
            mm[1] = 9;
            mm[2] = 3;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
      case 2:
      case 6:
        mm[0] = 8;
        if (x < 0) {
            mm[1] = 9;
            mm[2] = 7;
            mm[3] = 6;
            mm[4] = 4;
        }
        else {
            mm[1] = 7;
            mm[2] = 9;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
      case 4:
        mm[0] = 7;
        if (ay > ax) {
            mm[1] = 8;
            mm[2] = 4;
            mm[3] = 9;
            mm[4] = 1;
        }
        else {
            mm[1] = 4;
            mm[2] = 8;
            mm[3] = 1;
            mm[4] = 9;
        }
        break;
      case 5:
      case 13:
        mm[0] = 4;
        if (y < 0) {
            mm[1] = 1;
            mm[2] = 7;
            mm[3] = 2;
            mm[4] = 8;
        }
        else {
            mm[1] = 7;
            mm[2] = 1;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
      case 8:
        mm[0] = 3;
        if (ay > ax) {
            mm[1] = 2;
            mm[2] = 6;
            mm[3] = 1;
            mm[4] = 9;
        }
        else {
            mm[1] = 6;
            mm[2] = 2;
            mm[3] = 9;
            mm[4] = 1;
        }
        break;
      case 10:
      case 14:
        mm[0] = 2;
        if (x < 0) {
            mm[1] = 3;
            mm[2] = 1;
            mm[3] = 6;
            mm[4] = 4;
        }
        else {
            mm[1] = 1;
            mm[2] = 3;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
      case 12:
        mm[0] = 1;
        if (ay > ax) {
            mm[1] = 2;
            mm[2] = 4;
            mm[3] = 3;
            mm[4] = 7;
        }
        else {
            mm[1] = 4;
            mm[2] = 2;
            mm[3] = 7;
            mm[4] = 3;
        }
        break;
    }
}





/*
 * Given five choices of moves, make the first legal one. -RAK-
 *
 * Note that we can directly update the monster lore if needed.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door.
 *
 * This routine needs to make sure (it currently does) that immobile
 * monsters do not move (whether from fear or confusion).
 *
 * Note that we allow "staying still" as a legal move, so monsters
 * must be careful not to attempt to "eat" themselves.
 *
 * XXX Technically, need to check for monster in the way
 * combined with that monster being in a wall (or door?)
 *
 * XXX We should terminate the move array with zero
 * Or pass the number of directions as an argument
 */
static void make_move(int m_idx, int *mm)
{
    int			i, ny, nx;

    cave_type    	*c_ptr;
    inven_type  	*i_ptr;

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    bool		do_turn = FALSE;
    bool		do_move = FALSE;
    bool		do_view = FALSE;

    bool		did_open_door = FALSE;
    bool		did_bash_door = FALSE;
    bool		did_take_item = FALSE;
    bool		did_kill_item = FALSE;
    bool		did_move_body = FALSE;
    bool		did_kill_body = FALSE;
    bool		did_pass_wall = FALSE;
    bool		did_kill_wall = FALSE;


    /* Mega-Paranoia */
    if (m_ptr->dead) return;
    
    
    /* Take an array of five directions to try */
    for (i = 0; !do_turn && (i < 5); i++) {

        /* Get the destination */
        ny = m_ptr->fy + ddy[mm[i]];
        nx = m_ptr->fx + ddx[mm[i]];

        /* Access that cave grid */
        c_ptr = &cave[ny][nx];

        /* Access that cave grid's contents */
        i_ptr = &i_list[c_ptr->i_idx];


        /* Floor is open? */
        if (floor_grid_bold(ny, nx)) {

            /* Go ahead and move */
            do_move = TRUE;
        }

        /* Permanent blockage */
        else if (c_ptr->info & GRID_PERM) {

            /* Nothing */
        }

        /* Creature moves through walls? */
        else if (r_ptr->rflags2 & RF2_PASS_WALL) {

            /* Pass through walls/doors/rubble */
            do_move = TRUE;

            /* Monster went through a wall */
            did_pass_wall = TRUE;
        }

        /* Crunch up those Walls Morgoth and Umber Hulks!!!! */
        else if (r_ptr->rflags2 & RF2_KILL_WALL) {

            /* Eat through walls/doors/rubble */
            do_move = TRUE;

            /* Monster destroyed a wall */
            did_kill_wall = TRUE;

            /* Hack -- destroy doors and rubble */
            if ((i_ptr->tval == TV_CLOSED_DOOR) ||
                (i_ptr->tval == TV_SECRET_DOOR) ||
                (i_ptr->tval == TV_RUBBLE)) {

                /* Delete the door/rubble */
                delete_object(ny, nx);
            }

            /* Clear the wall code, if any */
            c_ptr->info &= ~GRID_WALL_MASK;

            /* Forget the "field mark", if any */
            c_ptr->info &= ~GRID_MARK;

            /* Redisplay the grid */
            lite_spot(ny, nx);

            /* Note changes to viewable region */
            if (player_has_los_bold(ny, nx)) do_view = TRUE;
        }

        /* Creature can open doors? */
        else if (c_ptr->i_idx) {

            /* Open closed/secret doors */
            if ((i_ptr->tval == TV_CLOSED_DOOR) ||
                (i_ptr->tval == TV_SECRET_DOOR)) {

                bool may_bash = TRUE;

                /* Take a turn */
                do_turn = TRUE;

                /* Creature can open doors. */
                if (r_ptr->rflags2 & RF2_OPEN_DOOR) {

                    /* Normal doors */
                    if (i_ptr->pval == 0) {

                        /* The door is open */
                        did_open_door = TRUE;

                        /* Do not bash the door */
                        may_bash = FALSE;
                    }

                    /* Locked doors -- take a turn to unlock it */
                    else if (i_ptr->pval > 0) {

                        /* Try to unlock it */
                        if (randint((m_ptr->hp + 1) * (50 + i_ptr->pval)) <
                            40 * (m_ptr->hp - 10 - i_ptr->pval)) {

                            /* The door is unlocked */
                            i_ptr->pval = 0;

                            /* Do not bash the door */
                            may_bash = FALSE;
                        }
                    }
                }

                /* Stuck doors -- attempt to bash them down if allowed */
                if (may_bash && (r_ptr->rflags2 & RF2_BASH_DOOR)) {

                    /* Attempt to Bash */
                    if (randint((m_ptr->hp + 1) * (50 - i_ptr->pval)) <
                        40 * (m_ptr->hp - 10 + i_ptr->pval)) {

                        /* Message */
                        msg_print("You hear a door burst open!");
                        disturb(1, 0);

                        /* The door was bashed open */
                        did_bash_door = TRUE;

                        /* Hack -- fall into doorway */
                        do_move = TRUE;
                    }
                }


                /* Deal with doors in the way */
                if (did_open_door || did_bash_door) {

                    /* XXX Should create a new object XXX */
                    invcopy(i_ptr, OBJ_OPEN_DOOR);

                    /* Place it in the dungeon */
                    i_ptr->iy = ny;
                    i_ptr->ix = nx;

                    /* 50% chance of breaking door during bash */
                    if (did_bash_door) i_ptr->pval = 0 - rand_int(2);

                    /* Redraw door */
                    lite_spot(ny, nx);

                    /* Handle viewable doors */
                    if (player_has_los_bold(ny, nx)) do_view = TRUE;
                }
            }
        }


        /* Hack -- check for Glyph of Warding */
        if (do_move &&
            (c_ptr->i_idx) &&
            (i_ptr->tval == TV_VIS_TRAP) &&
            (i_ptr->sval == SV_TRAP_GLYPH)) {

            /* Assume no move allowed */
            do_move = FALSE;

            /* Break the ward */
            if (randint(BREAK_GLYPH) < r_ptr->level) {

                /* Describe observable breakage */
                if (player_can_see_bold(ny, nx)) {
                    msg_print("The rune of protection is broken!");
                }

                /* Delete the rune */
                delete_object(ny, nx);

                /* Allow movement */
                do_move = TRUE;
            }
        }


        /* The player is in the way */
        if (do_move && (c_ptr->m_idx == 1)) {

            /* Do the attack */
            (void)make_attack_normal(m_idx);

            /* Do not move */
            do_move = FALSE;

            /* Took a turn */
            do_turn = TRUE;
        }


        /* Hack -- prevent bizarre "running molds" */
        if (r_ptr->rflags1 & RF1_NEVER_MOVE) do_move = FALSE;


        /* A monster is in the way (and we are not staying still) */
        if (do_move && (c_ptr->m_idx > 1) && (c_ptr->m_idx != m_idx)) {

            /* Assume no movement */
            do_move = FALSE;

            /* Kill weaker monsters */
            if ((r_ptr->rflags2 & RF2_KILL_BODY) &&
                (r_ptr->mexp > r_list[m_list[c_ptr->m_idx].r_idx].mexp)) {

                /* Allow movement */
                do_move = TRUE;

                /* Monster ate another monster */
                did_kill_body = TRUE;

                /* XXX XXX XXX Message */
                
                /* Kill the monster */
                delete_monster(ny, nx);
            }

            /* Push past weaker monsters */
            if ((r_ptr->rflags2 & RF2_MOVE_BODY) &&
                (r_ptr->mexp > r_list[m_list[c_ptr->m_idx].r_idx].mexp)) {

                /* Allow movement */
                do_move = TRUE;

                /* Monster pushed past another monster */
                did_move_body = TRUE;

                /* XXX XXX XXX Message */                
            }
        }


        /* Creature has been allowed move */
        if (do_move) {

            /* Take a turn */
            do_turn = TRUE;

            /* Move the creature */
            move_rec(m_ptr->fy, m_ptr->fx, ny, nx);

            /* Update the monster */
            update_mon(m_idx, TRUE);

#ifdef WDT_TRACK_OPTIONS
            /* Erase the player footprints */
            if (c_ptr->track > 4) c_ptr->track -= 2;
            if (c_ptr->track > -7) c_ptr->track -= 1;
#endif

            /* Hack -- Moving monsters can disturb the player */
            if (m_ptr->ml &&
                (disturb_move ||
                 (disturb_near && (m_ptr->cdis < 5)))) {
                disturb(0, 0);
            }


            /* Take or Kill "normal" objects on the floor */
            if (i_ptr->tval && (i_ptr->tval <= TV_MAX_OBJECT) &&
                ((r_ptr->rflags2 & RF2_TAKE_ITEM) ||
                 (r_ptr->rflags2 & RF2_KILL_ITEM))) {

                u32b flg3 = 0L;

                char m_name[80];
                char i_name[160];

                /* Check the grid */
                c_ptr = &cave[ny][nx];
                i_ptr = &i_list[c_ptr->i_idx];

                /* Acquire the object name */
                objdes(i_name, i_ptr, TRUE);

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0x04);

                /* Analyze "monster hurting" flags on the object */
                if (wearable_p(i_ptr)) {

                    /* React to objects that hurt the monster */
                    if (i_ptr->flags1 & TR1_KILL_DRAGON) flg3 |= RF3_DRAGON;
                    if (i_ptr->flags1 & TR1_SLAY_DRAGON) flg3 |= RF3_DRAGON;
                    if (i_ptr->flags1 & TR1_SLAY_TROLL) flg3 |= RF3_TROLL;
                    if (i_ptr->flags1 & TR1_SLAY_GIANT) flg3 |= RF3_GIANT;
                    if (i_ptr->flags1 & TR1_SLAY_ORC) flg3 |= RF3_ORC;
                    if (i_ptr->flags1 & TR1_SLAY_DEMON) flg3 |= RF3_DEMON;
                    if (i_ptr->flags1 & TR1_SLAY_UNDEAD) flg3 |= RF3_UNDEAD;
                    if (i_ptr->flags1 & TR1_SLAY_ANIMAL) flg3 |= RF3_ANIMAL;
                    if (i_ptr->flags1 & TR1_SLAY_EVIL) flg3 |= RF3_EVIL;
                }

                /* The object cannot be picked up by the monster */
                if (artifact_p(i_ptr) || (r_ptr->rflags3 & flg3)) {

                    /* Only give a message for "take_item" */
                    if (r_ptr->rflags2 & RF2_TAKE_ITEM) {
                    
                        /* Take note */
                        did_take_item = TRUE;

                        /* Describe observable situations */
                        if (m_ptr->ml && player_has_los_bold(ny, nx)) {

                            /* Dump a message */
                            message(m_name, 0x03);
                            message(" tries to pick up ", 0x02);
                            message(i_name, 0x02);
                            message(", but stops suddenly!", 0x04);
                        }
                    }
                }
                
                /* Pick up the item */
                else if (r_ptr->rflags2 & RF2_TAKE_ITEM) {

                    /* Take note */
                    did_take_item = TRUE;

                    /* Describe observable situations */
                    if (player_has_los_bold(ny, nx)) {
                     
                        /* Dump a message */
                        message(m_name, 0x03);
                        message(" picks up ", 0x02);
                        message(i_name, 0x02);
                        message(".", 0x04);
                    }
                        
                    /* Delete the object */
                    delete_object(ny, nx);
                }

                /* Destroy the item */
                else {

                    /* Take note */
                    did_kill_item = TRUE;

                    /* Describe observable situations */
                    if (player_has_los_bold(ny, nx)) {

                        /* Dump a message */
                        message(m_name, 0x03);
                        message(" crushes ", 0x02);
                        message(i_name, 0x02);
                        message(".", 0x04);
                    }
                        
                    /* Delete the object */
                    delete_object(ny, nx);
                }
            }
        }
    }


    /* Notice changes in view */
    if (do_view) {

        /* Update some things */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
    }


    /* Learn things from observable monster */
    if (m_ptr->ml) {

        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        /* Monster opened a door */
        if (did_open_door) l_ptr->flags2 |= RF2_OPEN_DOOR;

        /* Monster bashed a door */
        if (did_bash_door) l_ptr->flags2 |= RF2_BASH_DOOR;

        /* Monster tried to pick something up */
        if (did_take_item) l_ptr->flags2 |= RF2_TAKE_ITEM;

        /* Monster tried to crush something */
        if (did_kill_item) l_ptr->flags2 |= RF2_KILL_ITEM;

        /* Monster pushed past another monster */
        if (did_move_body) l_ptr->flags2 |= RF2_MOVE_BODY;

        /* Monster ate another monster */
        if (did_kill_body) l_ptr->flags2 |= RF2_KILL_BODY;

        /* Monster passed through a wall */
        if (did_pass_wall) l_ptr->flags2 |= RF2_PASS_WALL;

        /* Monster destroyed a wall */
        if (did_kill_wall) l_ptr->flags2 |= RF2_KILL_WALL;
    }


    /* Hack -- get "bold" if out of options */
    if (!do_turn && !do_move && m_ptr->monfear) {

        /* No longer afraid */
        m_ptr->monfear = 0;

        /* Message if seen */
        if (m_ptr->ml) {

            char m_name[80];

            /* Acquire the monster name */
            monster_desc(m_name, m_ptr, 0);

            /* Dump a message */
            message(m_name, 0x03);
            message(" turns to fight!", 0);
        }
    }
}



/*
 * Hack -- get some random moves
 */
static void get_moves_random(int *mm)
{
    mm[0] = randint(9);
    mm[1] = randint(9);
    mm[2] = randint(9);
    mm[3] = randint(9);
    mm[4] = randint(9);
}


/*
 * Move a critter about the dungeon			-RAK-
 *
 * Note that this routine is called ONLY from "process_monster()".
 */
static void mon_move(int m_idx)
{
    int			i, j, k, dir, mm[9];

    monster_type	*m_ptr = &m_list[m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];
    monster_lore	*l_ptr = &l_list[m_ptr->r_idx];

    /* Get the monster location */
    int fx = m_ptr->fx;
    int fy = m_ptr->fy;

    int stagger = 0;
    

    /* Hack -- if in wall, must immediately escape to a clear area */
    if (!floor_grid_bold(fy, fx) &&
        (!(r_ptr->rflags2 & RF2_PASS_WALL)) &&
        (!(r_ptr->rflags2 & RF2_KILL_WALL))) {

        /* Start with random moves */
        get_moves_random(mm);

        /* Attempt to find an *empty* space to escape into */
        for (k = 0, dir = 1, i = fy + 1; i >= fy - 1; i--) {
            for (j = fx - 1; j <= fx + 1; j++, dir++) {

                /* No staying still */
                if (dir == 5) continue;

                /* Require floor space */
                if (!floor_grid_bold(i,j)) continue;

                /* Do not allow attack against the player */
                if (cave[i][j].m_idx != 1) mm[k++] = dir;
            }
        }


        /* Attempt to "escape" */
        if (k) {

            /* Pick a random direction to prefer */
            j = rand_int(k);

            /* Prefer that direction */
            i = mm[0];
            mm[0] = mm[j];
            mm[j] = i;

            /* Move the monster */
            make_move(m_idx, mm);
        }


        /* Hack -- if still in a wall, apply more damage, and dig out */
        if (cave[m_ptr->fy][m_ptr->fx].info & GRID_WALL_MASK) {

            /* XXX XXX XXX XXX The player may not have caused the rocks */

            /* Apply damage, check for death (no fear messages) */
            if (mon_take_hit(m_idx, damroll(8, 8), FALSE)) {
                msg_print("You hear a scream muffled by rock!");
            }
            else {
                (void)twall((int)m_ptr->fy, (int)m_ptr->fx, 1, 0);
            }
        }


        /* monster movement finished */
        return;
    }


    /* Does the critter multiply?  Are creatures allowed to multiply? */
    /* Efficiency -- pre-empt multiplying if place_monster() will fail */
    /* Mega-Hack -- If the player is resting, only multiply occasionally */
    if ((r_ptr->rflags2 & RF2_MULTIPLY) &&
        (l_ptr->cur_num < l_ptr->max_num) &&
        (l_ptr->pkills < 30000) &&
        (!p_ptr->rest || (!rand_int(MON_MULT_ADJ))) ) {

        /* Count the adjacent monsters */
        for (k = 0, i = fy - 1; i <= fy + 1; i++) {
            for (j = fx - 1; j <= fx + 1; j++) {
                if (cave[i][j].m_idx > 1) k++;
            }
        }

        /* Hack -- multiply slower in crowded rooms */
        if ((k < 4) && (!k || !rand_int(k * MON_MULT_ADJ))) {

            /* Try to multiply, take note if kid is visible */
            if (multiply_monster(m_idx)) {

                /* Take note if mom is visible too */
                if (m_ptr->ml) l_ptr->flags2 |= RF2_MULTIPLY;
            }
        }
    }


    /* Attempt to cast a spell */
    if (make_attack_spell(m_idx)) return;


    /* Extract the stagger values */
    if (r_ptr->rflags1 & RF1_RAND_50) stagger += 50;
    if (r_ptr->rflags1 & RF1_RAND_25) stagger += 25;
    
    
    /* Confused -- 100% random */
    if (m_ptr->confused) {
        get_moves_random(mm);
    }

    /* 75% random movement */
    else if ((stagger == 75) && (rand_int(100) < 75)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_50;
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_25;
        get_moves_random(mm);
    }

    /* 50% random movement */
    else if ((stagger == 50) && (rand_int(100) < 50)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_50;
        get_moves_random(mm);
    }

    /* 25% random movement */
    else if ((stagger == 25) && (rand_int(100) < 25)) {
        if (m_ptr->ml) l_ptr->flags1 |= RF1_RAND_25;
        get_moves_random(mm);
    }

    /* Normal movement */
    else {
        get_moves(m_idx, mm);
    }

    /* Apply the movement */
    make_move(m_idx, mm);
}



/*
 * Hack -- local "player stealth" calculation.
 * Passed "process_monsters()" to "process_monster()"
 */
static u32b noise = 0L;


/*
 * Process a monster (that is, give him a turn)
 * Note that only monsters within 100 grids are processed
 * This limits the "effective" range of "aaf" to 100.
 */
static void process_monster(int i)
{
    monster_type *m_ptr = &m_list[i];
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    monster_lore *l_ptr;

    int fx = m_ptr->fx;
    int fy = m_ptr->fy;

    bool test = FALSE;


    /* Monsters "sense" the player */
    if (m_ptr->cdis <= r_ptr->aaf) {

        /* We can "sense" the player */
        test = TRUE;
    }

    /* For efficiency, pretend line of sight is reflexive */
    else if (player_has_los_bold(fy, fx)) {

        /* We can "see" the player */
        test = TRUE;
    }

    /* Hack -- Aggravation "wakes up" nearby monsters (below) */
    else if (p_ptr->aggravate && (m_ptr->cdis <= MAX_SIGHT)) {

        /* We can "feel" the player */
        test = TRUE;
    }

#ifdef MONSTER_FLOW
    /* Hack -- Monsters can "smell" the player from far away */
    /* Note that most monsters have "aaf" of "20" or so */
    else if (flow_by_sound &&
            (cave[py][px].when == cave[fy][fx].when) &&
            (cave[fy][fx].cost < MONSTER_FLOW_DEPTH) &&
            (cave[fy][fx].cost < r_ptr->aaf)) {

        /* We can "smell" the player */
        test = TRUE;
    }
#endif

#ifdef WDT_TRACK_OPTIONS
    /* Hack -- Monsters can "track" the player */
    else if (track_follow) {

        /* We can "track" the player */
        test = TRUE;
    }
#endif


    /* Not allowed to do anything */	    	
    if (!test) return;


    /* Access the lore */
    l_ptr = &l_list[m_ptr->r_idx];


#ifdef WDT_TRACK_OPTIONS
    /* Re-acquire the target */
    get_target(i);
#endif


    /* Hack -- being embedded in rock wakes a monster up */
    if (!floor_grid_bold(fy, fx) &&
        (!(r_ptr->rflags2 & RF2_PASS_WALL)) &&
        (!(r_ptr->rflags2 & RF2_KILL_WALL))) {

        /* Wake up */
        m_ptr->csleep = 0;

        /* No stun */
        m_ptr->stunned = 0;
    }


    /* Hack -- handle aggravation */
    if (p_ptr->aggravate) m_ptr->csleep = 0;

    /* Handle "sleep" */
    if (m_ptr->csleep) {

        /* Hack -- If the player is resting or paralyzed, then only */
        /* run this block about one turn in 50, since he is "quiet". */
        /* Hack -- nothing can hear the player from 50 grids away */
        if ((!p_ptr->rest && !p_ptr->paralysis) || (!rand_int(50))) {

            u32b notice = rand_int(1024);

            /* XXX See if monster "notices" player */
            if ((notice * notice * notice) <= noise) {

                /* Hack -- amount of "waking" */
                int d = (m_ptr->cdis < 50) ? (100 / m_ptr->cdis) : 1;

                /* Still asleep */
                if (m_ptr->csleep > d) {

                    /* Monster wakes up "a little bit" */
                    m_ptr->csleep -= d;

                    /* Notice the "not waking up" */
                    if (m_ptr->ml) {

                        /* Count the ignores */
                        if (l_ptr->ignore < MAX_UCHAR) l_ptr->ignore++;
                    }
                }

                /* Just woke up */
                else {

                    /* Reset sleep counter */
                    m_ptr->csleep = 0;

                    /* Notice the "waking up" */
                    if (m_ptr->ml) {

                        char m_name[80];

                        /* Acquire the monster name */
                        monster_desc(m_name, m_ptr, 0);

                        /* Dump a message */
                        message(m_name, 0x03);
                        message(" wakes up.", 0);

                        /* Count the wakings */
                        if (l_ptr->wake < MAX_UCHAR) l_ptr->wake++;
                    }
                }
            }
        }

        /* Still sleeping */
        if (m_ptr->csleep) return;
    }


    /* Handle "stun" */
    if (m_ptr->stunned) {

        /* Recover a little bit */
        m_ptr->stunned--;

        /* Make a "saving throw" against stun */
        if (rand_int(5000) <= r_ptr->level * r_ptr->level) {
            m_ptr->stunned = 0;
        }

        /* Hack -- Recover from stun */
        if (!m_ptr->stunned) {

            /* Message if visible */
            if (m_ptr->ml) {

                char m_name[80];

                /* Acquire the monster name */
                monster_desc(m_name, m_ptr, 0);

                /* Dump a message XXX XXX assume "glare" is possible */
                message(m_name, 0x03);
                message(" recovers and glares at you.", 0);
            }
        }

        /* Still stunned */
        if (m_ptr->stunned) return;
    }


    /* Handle "fear" */
    if (m_ptr->monfear) {

        /* Amount of "boldness" */
        int d = randint(1 + r_ptr->level / 10);

        /* Still afraid */
        if (m_ptr->monfear > d) {

            /* Reduce the fear */
            m_ptr->monfear -= d;
        }

        /* Recover from fear, take note if seen */
        else {

            /* No longer afraid */
            m_ptr->monfear = 0;

            /* Visual note */
            if (m_ptr->ml) {

                char m_name[80];
                char m_poss[80];

                /* Acquire the monster name/poss */
                monster_desc(m_name, m_ptr, 0);
                monster_desc(m_poss, m_ptr, 0x22);

                /* Dump a message */
                message(m_name, 0x03);
                message(" recovers ", 0x02);
                message(m_poss, 0x02);
                message(" courage.", 0);
            }
        }
    }


    /* Handle confusion */
    if (m_ptr->confused) {

        /* Amount of "boldness" */
        int d = randint(1 + r_ptr->level / 10);

        /* Still confused */
        if (m_ptr->confused > d) {

            /* Reduce the confusion */
            m_ptr->confused -= d;
        }

        /* Recovered */
        else {

            /* No longer confused */
            m_ptr->confused = 0;
        }
    }


    /* Let it move */
    mon_move(i);
}


/*
 * Process all the monsters.
 *
 * Give monsters some energy, and when allowed, try waking up, moving,
 * attacking, etc.  We also remove "dead" monsters here.
 *
 * Note that a monster can ONLY move in the monster array when someone
 * calls "remove_monster_idx()".  And basically, the only function that
 * does that is this one, and we only do it for monsters that are dead,
 * inducing the "movement" of a monster that was already processed into
 * a slot which will not be processed again until next time.
 *
 * Poly-morph is extremely dangerous, cause it changes the monster race.
 * Actually, it is done by killing the original monster and creating a
 * new monster, so we should be okay, except that the "dead" monster may
 * not be removed until the next time through the loop.
 *
 * Note that this routine ASSUMES that "update_monsters()" has been called
 * every time the player changes his view or lite, and that "update_mon()"
 * has been, and will be, called every time a monster moves.  Likewise, we
 * rely on the lite/view/etc being "verified" EVERY time a door/wall appears
 * or disappears.  This will then require minimal scans of the "monster" array.
 *
 * It is very important to process the array "backwards", as "newly created"
 * monsters should not get a turn until the next round of processing.  Else
 * there is a risk of multiple summons breaking the machine.  As it is, there
 * once the monster array fills up, only the first few "summons" per round
 * will succeed, but luckily, this will (on average) require about 5 * 20
 * or 100 summons during one player turn.
 *
 * Note that (eventually) we should "remove" the "street urchin" monster
 * record, and use "monster race zero" as the "no-race" indicator, that is,
 * the indicator of "free space" in the monster list.  But that is of a very
 * low priority for now.
 *
 * Note also the new "speed code" which allows "better" distribution of events.
 *
 * There is a (hackish) "speed boost" of rand_int(5) energy units applied each
 * time the monster attempt to move.  This means that fast monsters get more
 * "boosts".  On average, for a normal monster, this produces one "extra" move
 * every 50 moves, barely noticable, but also provides some random spread to
 * the monster clumpings.  Calling "random()" so often is very expensive.
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.  Most of this time is in process_monster() and make_move().
 */
void process_monsters(void)
{
    int		i, e;


    /* Hack -- notice player death or departure */
    if (death || new_level_flag) return;


    /* Calculate the "noisiness" of the player */
    noise = (1L << (29 - p_ptr->stl));


    /* Hack -- Require some breathing space */
    tighten_m_list();


    /* Process the monsters (backwards!) */
    for (i = m_max - 1; i >= MIN_M_IDX; i--) {


        /* Get the i'th monster */
        monster_type *m_ptr = &m_list[i];


        /* Remove dead monsters. */
        if (m_ptr->dead) {

            /* Remove the monster */
            remove_monster_idx(i);

            /* Do NOT process it */
            continue;
        }


        /* Obtain the energy boost */
        e = extract_energy[m_ptr->mspeed];

#ifdef WDT_TRACK_OPTIONS
        /* Reduce energy boost if far away and out of sight */
        if (track_follow &&
            (m_ptr->cdis > 20) &&
            !player_has_los_bold(m_ptr->fy, m_ptr->fx)) {

            /* Reduce the energy boost */
            e >>= 2;
        }
#endif

        /* Give this monster some energy */
        m_ptr->energy += e;

        /* Not enough energy to move */
        if (m_ptr->energy < 100) continue;

#ifdef RANDOM_BOOST
        /* Hack -- small "energy boost" (see "dungeon.c") */
        m_ptr->energy += rand_int(5);
#endif

        /* Use up "some" energy */
        m_ptr->energy -= 100;


        /* Process "nearby" monsters */
        if ((m_ptr->cdis < 100) || track_follow) process_monster(i);


        /* Hack -- notice player death or departure */
        if (death || new_level_flag) break;
    }


    /* Hack -- Require some breathing space */
    tighten_m_list();
}





