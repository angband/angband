/* File: cmd3.c */

/* Purpose: high level command processing */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Look at a monster
 */
static cptr look_mon_desc(int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];

    bool          living = TRUE;
    int           perc;


    /* Determine if the monster is "living" (vs "undead") */
    if (r_ptr->flags3 & RF3_UNDEAD) living = FALSE;
    if (r_ptr->flags3 & RF3_DEMON) living = FALSE;
    if (strchr("EgvX", r_ptr->r_char)) living = FALSE;


    /* Healthy monsters */
    if (m_ptr->hp >= m_ptr->maxhp) {

        /* No damage */
        return (living ? "unhurt" : "undamaged");
    }


    /* Calculate a health "percentage" */
    perc = 100L * m_ptr->hp / m_ptr->maxhp;

    if (perc >= 60) {
        return (living ? "somewhat wounded" : "somewhat damaged");
    }

    if (perc >= 25) {
        return (living ? "wounded" : "damaged");
    }

    if (perc >= 10) {
        return (living ? "badly wounded" : "badly damaged");
    }

    return (living ? "almost dead" : "almost destroyed");
}





/*
 * Examine a grid, return a keypress.
 *
 * Assume the player is not blind or hallucinating.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * XXX XXX XXX We should allow the use of a "set target" command.
 *
 * XXX XXX XXX Note new terrain features for 2.8.0
 *
 * We may assume that the grid is supposed to be "interesting".
 */
static int do_cmd_look_aux(int y, int x)
{
    cave_type		*c_ptr = &cave[y][x];

    inven_type		*i_ptr = &i_list[c_ptr->i_idx];
    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_info[m_ptr->r_idx];

    cptr		s1 = "You see ", s2 = "";

    bool		prep = FALSE;

    int			f = (c_ptr->feat & 0x3F);
    
    int			query = ' ';

    char		m_name[80];
    char		i_name[80];

    char		out_val[160];


    /* Hack -- looking under the player */
    if ((y == py) && (x == px)) s1 = "You are on ";


    /* Convert secret doors to walls */
    if (f == 0x30) f = 0x38;

    /* Ignore invisible traps */
    if (f == 0x02) f = 0x01;


    /* Hack -- Invisible monsters */
    if (!m_ptr->ml) m_ptr = &m_list[0];


    /* Actual monsters */
    if (m_ptr->r_idx) {

        /* Get the monster name ("a kobold") */
        monster_desc(m_name, m_ptr, 0x08);

        /* Hack -- track this monster race */
        recent_track(m_ptr->r_idx);

        /* Hack -- health bar for this monster */
        health_track(c_ptr->m_idx);

        /* Hack -- handle stuff */
        handle_stuff();
        
        /* Describe, and prompt for recall */
        sprintf(out_val, "%s%s%s (%s) [(r)ecall]",
                s1, s2, m_name, look_mon_desc(c_ptr->m_idx));
        prt(out_val, 0, 0);

        /* Get a command */
        move_cursor_relative(y, x);
        query = inkey();

        /* Recall as needed */
        while (query == 'r') {

            /* Recall */
            Term_save();
            screen_roff(m_ptr->r_idx);
            Term_addstr(-1, TERM_WHITE, "  --pause--");
            query = inkey();
            Term_load();

            /* Continue if desired */
            if (query != ' ') return (query);
            
            /* Get a new command */
            move_cursor_relative(y, x);
            query = inkey();
        }

        /* Continue if allowed */
        if (query != ' ') return (query);

        /* Change the intro */
        s1 = "It is ";

        /* Hack -- take account of gender */
        if (r_ptr->flags1 & RF1_FEMALE) s1 = "She is ";
        else if (r_ptr->flags1 & RF1_MALE) s1 = "He is ";

        /* Use a preposition with objects */
        s2 = "on ";
        
        /* Use a preposition with terrain */
        prep = TRUE;

        /* Ignore floors */
        if (f == 0x01) f = 0;
    }


    /* Skip "dark" grids */
    if (!test_lite_bold(y, x)) return (query);


    /* Actual items */
    if (i_ptr->k_idx) {

        /* Obtain an object description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Describe the object */
        sprintf(out_val, "%s%s%s.  --pause--", s1, s2, i_name);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Use "space" to advance */
        if (query != ' ') return (query);

        /* Change the intro */
        s1 = "It is ";

        /* Plurals */
        if (i_ptr->number > 1) s1 = "They are ";

        /* Use a preposition with terrain */
        prep = TRUE;

        /* Ignore floors */
        if (f == 0x01) f = 0;
    }


    /* Describe terrain */
    if (f) {

        cptr p1 = "";

        cptr p2 = "a ";
        
        cptr name = f_name + f_info[f].name;

        /* Pick a prefix */
        if (prep) p2 = ((f >= 0x20) ? "in " : "on ");

        /* Note leading vowel */
        if (is_a_vowel(name[0])) p2 = "an ";

        /* Hack -- store doors */
        if ((f >= 0x08) && (f <= 0x0F)) p2 = "the entrance to the ";

        /* Display a message */
        sprintf(out_val, "%s%s%s%s.  --pause--", s1, p1, p2, name);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Continue if allowed */
        if (query != ' ') return (query);
    }

    /* Keep going */
    return (query);
}




/*
 * Hack -- determine if a given location is "interesting"
 */
static bool do_cmd_look_accept(int y, int x)
{
    cave_type *c_ptr;
    
    /* Examine the grid */
    c_ptr = &cave[y][x];

    /* Visible monsters */
    if (c_ptr->m_idx) {

        monster_type *m_ptr = &m_list[c_ptr->m_idx];
        
        /* Visible monsters */
        if (m_ptr->ml) return (TRUE);
    }

    /* Visible objects */
    if (c_ptr->i_idx) {

        /* inven_type *i_ptr = &i_list[c_ptr->i_idx]; */

        /* Visible objects */
        if (test_lite_bold(y, x)) return (TRUE);
    }

    /* Interesting features */
    if (c_ptr->feat & CAVE_MARK) {

        /* Ignore floors and invisible traps */
        if ((c_ptr->feat & 0x3F) <= 0x02) return (FALSE);

        /* Notice doors, traps, stores, etc */
        if ((c_ptr->feat & 0x3F) <= 0x2F) return (TRUE);

        /* Ignore secret doors */
        if ((c_ptr->feat & 0x3F) <= 0x30) return (FALSE);

        /* Notice rubble */
        if ((c_ptr->feat & 0x3F) <= 0x31) return (TRUE);

        /* Ignore veins or Notice veins as appropriate */
        if ((c_ptr->feat & 0x3F) <= 0x35) return (notice_seams);

        /* Notice treasure veins */
        if ((c_ptr->feat & 0x3F) <= 0x37) return (TRUE);

        /* Ignore granite */
        return (FALSE);
    }
    
    /* Nope */
    return (FALSE);
}


/*
 * A new "look" command, similar to the "target" command.
 */
void do_cmd_look(void)
{
    int		i, d, m;

    bool	done = FALSE;

    char	query;


    /* Blind */
    if (p_ptr->blind) {
        msg_print("You can't see a damn thing!");
        return;
    }

    /* Hallucinating */
    if (p_ptr->image) {
        msg_print("You can't believe what you are seeing!");
        return;
    }


    /* Reset "temp" array */
    temp_n = 0;
    
    /* Collect viewable grids */
    for (i = 0; i < view_n; i++) {
        
        int x = view_x[i];
        int y = view_y[i];

        /* Skip off-screen locations */
        if (!panel_contains(y,x)) continue;
        
        /* Skip invalid locations */
        if (!do_cmd_look_accept(y,x)) continue;

        /* Save the location */
        temp_x[temp_n] = x;
        temp_y[temp_n] = y;
        temp_n++;
    }


    /* Nothing to see */
    if (!temp_n) {
        msg_print("You see nothing special.");
        return;
    }
    

    /* Start on (or near) the player */
    m = target_pick(py, px, 0, 0);

    /* Interact */
    while (!done) {
    
        /* Describe and Prompt */
        query = do_cmd_look_aux(temp_y[m], temp_x[m]);

        /* Assume no "direction" */
        d = 0;
            
        /* Analyze (non "recall") command */
        switch (query) {

            case ESCAPE:
            case 'q':
                done = TRUE;
                break;

            case ' ':
                if (++m == temp_n) m = 0;
                break;

            case '-':
                if (m-- == 0) m = temp_n - 1;
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

            /* Find a new grid if possible */
            i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

            /* Use that grid */
            if (i >= 0) m = i;
        }
    }

    /* Clear the prompt */
    prt("", 0, 0);
}




/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate()
{
    int		dir, y1, x1, y2, x2;

    char	tmp_val[80];

    char	out_val[160];


    /* Start at current panel */
    y2 = y1 = panel_row;
    x2 = x1 = panel_col;

    /* Show panels until done */
    while (1) {

        /* Describe the location */
        if ((y2 == y1) && (x2 == x1)) {
            tmp_val[0] = '\0';
        }
        else {
            sprintf(tmp_val, "%s%s of",
                    ((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
                    ((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
        }

        /* Prepare to ask which way to look */
        sprintf(out_val,
                "Map sector [%d,%d], which is%s your sector.  Direction?",
                y2, x2, tmp_val);

        /* Assume no direction */
        dir = 0;

        /* Get a direction */
        while (!dir) {

            char command;

            /* Get a command (or Cancel) */
            if (!get_com(out_val, &command)) break;

            /* Analyze the keypress */
            switch (command) {

                /* Convert roguelike directions */
                case 'B': case 'b': case '1': dir = 1; break;
                case 'J': case 'j': case '2': dir = 2; break;
                case 'N': case 'n': case '3': dir = 3; break;
                case 'H': case 'h': case '4': dir = 4; break;
                case 'L': case 'l': case '6': dir = 6; break;
                case 'Y': case 'y': case '7': dir = 7; break;
                case 'K': case 'k': case '8': dir = 8; break;
                case 'U': case 'u': case '9': dir = 9; break;
            }

            /* Error */
            if (!dir) bell();
        }

        /* No direction */
        if (!dir) break;

        /* Apply the motion */
        y2 += ddy[dir];
        x2 += ddx[dir];
        
        /* Verify the row */
        if (y2 > max_panel_rows) y2 = max_panel_rows;
        else if (y2 < 0) y2 = 0;

        /* Verify the col */
        if (x2 > max_panel_cols) x2 = max_panel_cols;
        else if (x2 < 0) x2 = 0;

        /* Handle "changes" */
        if ((y2 != panel_row) || (x2 != panel_col)) {

            /* Save the new panel info */
            panel_row = y2;
            panel_col = x2;

            /* Recalculate the boundaries */
            panel_bounds();

            /* Update stuff */
            p_ptr->update |= (PU_MONSTERS);

            /* Redraw stuff */
            p_ptr->redraw |= (PR_MAP);

            /* Handle stuff */
            handle_stuff();
        }
    }


    /* Recenter the map around the player */
    verify_panel();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);

    /* Handle stuff */
    handle_stuff();
}



/*
 * Allocates objects upon opening a chest    -BEN-
 * Disperse treasures from the chest "i_ptr", centered at (x,y).
 */
static void chest_death(int y, int x, inven_type *i_ptr)
{
    int		i, d, ny, nx;
    int		number, small;


    /* Must be a chest */
    if (i_ptr->tval != TV_CHEST) return;

    /* Determine if the chest is small */
    small = (i_ptr->sval < SV_CHEST_MIN_LARGE);

    /* Determine how many items to drop */
    number = (i_ptr->sval % SV_CHEST_MIN_LARGE);

    /* Generate some treasure */
    if (i_ptr->pval && (number > 0)) {

        /* Drop some objects (non-chests) */
        for ( ; number > 0; --number) {

            /* Try 20 times per item */
            for (i = 0; i < 20; ++i) {

                /* Pick a distance */
                d = ((i + 15) / 15);

                /* Pick a location */
                scatter(&ny, &nx, y, x, d, 0);

                /* Must be a clean floor grid */
                if (!clean_grid_bold(ny, nx)) continue;

                /* Opening a chest */
                opening_chest = TRUE;

                /* The "pval" of a chest is how "good" it is */
                object_level = ABS(i_ptr->pval);

                /* Small chests often drop gold */
                if (small && (rand_int(100) < 75)) {
                    place_gold(ny, nx);
                }

                /* Otherwise drop an item */
                else {
                    place_object(ny, nx, FALSE, FALSE);
                }

                /* Reset the object level */
                object_level = dun_level;

                /* No longer opening a chest */
                opening_chest = FALSE;

                /* Actually display the object's grid */
                lite_spot(ny, nx);

                /* Successful placement */
                break;
            }
        }
    }

    /* Empty */
    i_ptr->pval = 0;

    /* Known */
    inven_known(i_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, inven_type *i_ptr)
{
    int  i, trap;


    /* Only analyze chests */
    if (i_ptr->tval != TV_CHEST) return;

    /* Ignore disarmed chests */
    if (i_ptr->pval <= 0) return;

    /* Obtain the traps */
    trap = chest_traps[i_ptr->pval];

    /* Lose strength */
    if (trap & CHEST_LOSE_STR) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 4), "a poison needle");
        (void)do_dec_stat(A_STR);
    }

    /* Lose constitution */
    if (trap & CHEST_LOSE_CON) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 4), "a poison needle");
        (void)do_dec_stat(A_CON);
    }

    /* Poison */
    if (trap & CHEST_POISON) {
        msg_print("A puff of green gas surrounds you!");
        if (add_poisoned(10 + randint(20))) {
            msg_print("You are poisoned!");
        }
        else {
            msg_print("You are unaffected.");
        }
    }

    /* Paralyze */
    if (trap & CHEST_PARALYZE) {
        msg_print("A puff of yellow gas surrounds you!");
        if (add_paralysis(10 + randint(20))) {
            msg_print("You choke and pass out.");
        }
        else {
            msg_print("You are unaffected.");
        }
    }

    /* Summon monsters */
    if (trap & CHEST_SUMMON) {
        int num = 2 + randint(3);
        for (i = 0; i < num; i++) {
            (void)summon_specific(y, x, dun_level, 0);
        }
    }

    /* Explode */
    if (trap & CHEST_EXPLODE) {
        msg_print("There is a sudden explosion!");
        msg_print("Everything inside the chest is destroyed!");
        i_ptr->pval = 0;
        take_hit(damroll(5, 8), "an exploding chest");
    }
}





/*
 * Opens a closed door or closed chest.		-RAK-
 * Note that failed opens take time, or ghosts could be found
 * Note unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open()
{
    int				y, x, i, j, dir;
    int				flag;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    bool more = FALSE;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get requested grid */
        c_ptr = &cave[y][x];

        /* Get the object (if any) */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Nothing useful */
        if ((((c_ptr->feat & 0x3F) < 0x20) ||
             ((c_ptr->feat & 0x3F) > 0x2F)) &&
            (i_ptr->tval != TV_CHEST)) {

            /* Message */
            msg_print("You see nothing there to open.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Open a closed chest. */
        else if (i_ptr->tval == TV_CHEST) {

            /* Take a turn */
            energy_use = 100;
            
            /* Assume opened successfully */
            flag = TRUE;

            /* Attempt to unlock it */
            if (i_ptr->pval > 0) {

                /* Assume locked, and thus not open */
                flag = FALSE;

                /* Get the "disarm" factor */
                i = p_ptr->skill_dis;

                /* Penalize some conditions */
                if (p_ptr->blind || no_lite()) i = i / 10;
                if (p_ptr->confused || p_ptr->image) i = i / 10;

                /* Extract the difficulty */
                j = i - i_ptr->pval;

                /* Always have a small chance of success */
                if (j < 2) j = 2;

                /* Success -- May still have traps */
                if (rand_int(100) < j) {
                    msg_print("You have picked the lock.");
                    gain_exp(1);
                    flag = TRUE;
                }

                /* Failure -- Keep trying */
                else {
                    /* We may continue repeating */
                    more = TRUE;
                    if (flush_failure) flush();
                    msg_print("You failed to pick the lock.");
                }
            }

            /* Allowed to open */
            if (flag) {

                /* Apply chest traps, if any */
                chest_trap(y, x, i_ptr);

                /* Let the Chest drop items */
                chest_death(y, x, i_ptr);
            }
        }

        /* Jammed door */
        else if ((c_ptr->feat & 0x3F) >= 0x28) {

            /* Take a turn */
            energy_use = 100;
            
            /* Stuck */
            msg_print("The door appears to be stuck.");
        }

        /* Locked door */
        else if ((c_ptr->feat & 0x3F) >= 0x21) {

            /* Take a turn */
            energy_use = 100;
            
            /* Disarm factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* Extract the difficulty */
            j = i - ((int)(c_ptr->feat & 0x07)) * 4;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Success */
            if (rand_int(100) < j) {

                /* Message */
                msg_print("You have picked the lock.");

                /* Experience */
                gain_exp(1);

                /* Open the door */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
            
                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
            }

            /* Failure */
            else {

                /* Failure */
                if (flush_failure) flush();

                /* Message */
                msg_print("You failed to pick the lock.");

                /* We may keep trying */
                more = TRUE;
            }
        }

        /* Closed door */
        else {

            /* Open the door */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
            
            /* Notice */
            note_spot(y, x);

            /* Redraw */
            lite_spot(y, x);

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
        }
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}


/*
 * Close an open door.
 */
void do_cmd_close()
{
    int			y, x, dir;
    cave_type		*c_ptr;

    bool more = FALSE;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Broken door */
        if ((c_ptr->feat & 0x3F) == 0x05) {
        
            /* Message */            
            msg_print("The door appears to be broken.");
        }
        
        /* Require open door */
        else if ((c_ptr->feat & 0x3F) != 0x04) {

            /* Message */
            msg_print("You see nothing there to close.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Close the door */
        else {

            /* Take a turn */
            energy_use = 100;

            /* Close the door */
            c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x20);

            /* Notice */
            note_spot(y, x);

            /* Redraw */
            lite_spot(y, x);

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
        }
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}


/*
 * Tunnels through "walls" (including rubble and closed doors)
 *
 * Note that tunneling almost always takes time, since otherwise
 * you can use tunnelling to find monsters.  Also note that you
 * must tunnel in order to hit invisible monsters in walls (etc).
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel()
{
    int			y, x, dir;

    cave_type		*c_ptr;

    bool old_floor = FALSE;

    bool more = FALSE;


    /* Get a direction to tunnel, or Abort */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];

        /* Check the floor-hood */
        old_floor = floor_grid_bold(y, x);

        /* No tunnelling through emptiness */
        if ((c_ptr->feat & 0x3F) < 0x20) {

            /* Message */
            msg_print("You see nothing there to tunnel through.");
        }

        /* No tunnelling through doors */
        else if ((c_ptr->feat & 0x3F) < 0x30) {

            /* Message */
            msg_print("You cannot tunnel through doors.");
        }

        /* A monster is in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Okay, try digging */
        else {

            /* Take a turn */
            energy_use = 100;
            
            /* Titanium */
            if ((c_ptr->feat & 0x3F) >= 0x3C) {

                msg_print("This seems to be permanent rock.");
            }

            /* Granite */
            else if ((c_ptr->feat & 0x3F) >= 0x38) {

                /* Tunnel */
                if ((p_ptr->skill_dig > 40 + rand_int(1600)) && twall(y, x)) {
                    msg_print("You have finished the tunnel.");
                }

                /* Keep trying */
                else {
                    /* We may continue tunelling */
                    msg_print("You tunnel into the granite wall.");
                    more = TRUE;
                }
            }

            /* Quartz / Magma */
            else if ((c_ptr->feat & 0x3F) >= 0x32) {

                bool okay = FALSE;
                bool gold = FALSE;
                bool hard = FALSE;
                
                /* Found gold */
                if ((c_ptr->feat & 0x3F) >= 0x34) gold = TRUE;
                
                /* Quartz */
                if (c_ptr->feat & 0x01) hard = TRUE;

                /* Quartz */
                if (hard) {

                    okay = (p_ptr->skill_dig > 20 + rand_int(800));
                }
                
                /* Magma */
                else {

                    okay = (p_ptr->skill_dig > 10 + rand_int(400));
                }

                /* Success */
                if (okay && twall(y,x)) {

                    /* Found treasure */
                    if (gold) {

                        /* Place some gold */
                        place_gold(y, x);

                        /* Display it */
                        lite_spot(y, x);

                        /* Message */
                        msg_print("You have found something!");
                    }

                    /* Found nothing */
                    else {

                        /* Message */
                        msg_print("You have finished the tunnel.");
                    }
                }

                /* Failure (quartz) */
                else if (hard) {

                    /* Message, continue digging */
                    msg_print("You tunnel into the quartz vein.");
                    more = TRUE;
                }

                /* Failure (magma) */
                else {

                    /* Message, continue digging */
                    msg_print("You tunnel into the magma vein.");
                    more = TRUE;
                }
            }

            /* Rubble */
            else if ((c_ptr->feat & 0x3F) == 0x31) {

                /* Remove the rubble */
                if ((p_ptr->skill_dig > rand_int(200)) && twall(y,x)) {

                    /* Message */
                    msg_print("You have removed the rubble.");

                    /* Hack -- place an object */
                    if (rand_int(100) < 10) {
                        place_object(y, x, FALSE, FALSE);
                        if (test_lite_bold(y, x)) {
                             msg_print("You have found something!");
                        }
                    }

                    /* Display */
                    lite_spot(y, x);
                }

                else {

                    /* Message, keep digging */
                    msg_print("You dig in the rubble.");
                    more = TRUE;
                }
            }

            /* Default to secret doors */
            else {

                /* Message, keep digging */
                msg_print("You tunnel into the granite wall.");
                more = TRUE;

                /* Hack -- Search */
                search();
            }
        }

        /* Notice "blockage" changes */
        if (old_floor != floor_grid_bold(y, x)) {

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
        }
    }

    /* Cancel repetition unless we can continue */
    if (!more) disturb(0, 0);
}


/*
 * Disarms a trap, or chest	-RAK-	
 */
void do_cmd_disarm()
{
    int                 y, x, i, j, dir, power;

    cave_type		*c_ptr;
    inven_type		*i_ptr;

    bool		more = FALSE;


    /* Get a direction (or abort) */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Access the item */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Nothing useful */
        if ((i_ptr->tval != TV_CHEST) &&
            (((c_ptr->feat & 0x3F) < 0x10) ||
             ((c_ptr->feat & 0x3F) > 0x1F))) {

            /* Message */
            msg_print("You see nothing there to disarm.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Normal disarm */
        else if (i_ptr->tval == TV_CHEST) {

            /* Take a turn */
            energy_use = 100;
            
            /* Get the "disarm" factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* Extract the difficulty */
            j = i - i_ptr->pval;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Must find the trap first. */
            if (!inven_known_p(i_ptr)) {
                msg_print("I don't see any traps.");
            }

            /* Already disarmed/unlocked */
            else if (i_ptr->pval <= 0) {
                msg_print("The chest is not trapped.");
            }

            /* No traps to find. */
            else if (!chest_traps[i_ptr->pval]) {
                msg_print("The chest is not trapped.");
            }

            /* Success (get a lot of experience) */
            else if (rand_int(100) < j) {
                msg_print("You have disarmed the chest.");
                gain_exp(i_ptr->pval);
                i_ptr->pval = (0 - i_ptr->pval);
            }

            /* Failure -- Keep trying */
            else if ((i > 5) && (randint(i) > 5)) {
                /* We may keep trying */
                more = TRUE;
                if (flush_failure) flush();
                msg_print("You failed to disarm the chest.");
            }

            /* Failure -- Set off the trap */
            else {
                msg_print("You set off a trap!");
                chest_trap(y, x, i_ptr);
            }
        }

        /* Disarm a trap */
        else {

            /* Take a turn */
            energy_use = 100;
            
            /* Get the "disarm" factor */
            i = p_ptr->skill_dis;

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;

            /* XXX XXX XXX XXX XXX */
            /* Extract trap "power" */
            power = 5;
            
            /* Extract the difficulty */
            j = i - power;

            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Success */
            if (rand_int(100) < j) {
            
                /* Message */
                msg_print("You have disarmed the trap.");

                /* Reward */
                gain_exp(power);

                /* Remove the trap */
                c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x01);
                
                /* move the player onto the trap grid */
                move_player(dir, FALSE);
            }

            /* Failure -- Keep trying */
            else if ((i > 5) && (randint(i) > 5)) {

                /* Failure */
                if (flush_failure) flush();

                /* Message */
                msg_print("You failed to disarm the trap.");

                /* We may keep trying */
                more = TRUE;
            }

            /* Failure -- Set off the trap */
            else {

                /* Message */
                msg_print("You set off the trap!");

                /* Move the player onto the trap */
                move_player(dir, FALSE);
            }
        }
    }

    /* Cancel repeat unless told not to */
    if (!more) disturb(0, 0);
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 *
 * We need to use character body weight for something, or else we need
 * to no longer give female characters extra starting gold.
 */
void do_cmd_bash()
{
    int                 y, x, dir;
    
    int			bash, temp;

    cave_type		*c_ptr;

    bool		more = FALSE;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Bash location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];

        /* Nothing useful */
        if (((c_ptr->feat & 0x3F) < 0x20) ||
            ((c_ptr->feat & 0x3F) > 0x2F)) {

            /* Message */
            msg_print("You see nothing there to bash.");
        }

        /* Monster in the way */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Bash a closed door */
        else {

            /* Take a turn */
            energy_use = 100;
            
            /* Message */
            msg_print("You smash into the door!");

            /* Hack -- Bash power based on strength */
            /* (Ranges from 3 to 20 to 100 to 200) */
            bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

            /* Compare the bash power to the door power */
            temp = (bash - ((int)(c_ptr->feat & 0x07)) * 10);

            /* Hack -- always have a chance */
            if (temp < 1) temp = 1;
            
            /* Hack -- attempt to bash down the door */
            if (rand_int(100) < temp) {

                /* Message */
                msg_print("The door crashes open!");

                /* Break down the door */
                if (rand_int(100) < 50) {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x05);
                }
                
                /* Open the door */
                else {
                    c_ptr->feat = ((c_ptr->feat & ~0x3F) | 0x04);
                }

                /* Notice */
                note_spot(y, x);

                /* Redraw */
                lite_spot(y, x);

                /* Hack -- Fall through the door */
                move_player(dir, FALSE);

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE);
                p_ptr->update |= (PU_DISTANCE);
            }

            /* Saving throw against stun */
            else if (rand_int(100) < adj_dex_safe[p_ptr->stat_ind[A_DEX]]) {

                /* Message */
                msg_print("The door holds firm.");

                /* Allow repeated bashing */
                more = TRUE;
            }

            /* High dexterity yields coolness */
            else {

                /* Message */
                msg_print("You are off-balance.");

                /* Hack -- Bypass "free action" */
                p_ptr->paralysis = 2 + rand_int(2);
            }
        }
    }

    /* Unless valid action taken, cancel bash */
    if (!more) disturb(0, 0);
}



/*
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Choose spikes, perhaps, and maybe no repeating
 *
 * We should probably consider allowing the user to "choose" spikes.
 */
static bool get_spike(int *ip)
{
    int i;

    /* Check every item in the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        inven_type *i_ptr = &inventory[i];

        /* Check the "tval" code */
        if (i_ptr->tval == TV_SPIKE) {

            /* Save the spike index */
            (*ip) = i;

            /* Success */
            return (TRUE);
        }
    }

    /* Oops */
    return (FALSE);
}



/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike()
{
    int                  y, x, dir, item;

    cave_type		*c_ptr;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];

        /* Require closed door */
        if (((c_ptr->feat & 0x3F) < 0x20) ||
            ((c_ptr->feat & 0x3F) > 0x2F)) {

            /* Message */
            msg_print("You see nothing there to spike.");
        }

        /* Get a spike */
        else if (!get_spike(&item)) {

            /* Message */
            msg_print("You have no spikes!");
        }

        /* Is a monster in the way? */
        else if (c_ptr->m_idx) {

            /* Take a turn */
            energy_use = 100;

            /* Message */
            msg_print("There is a monster in the way!");

            /* Attack */
            py_attack(y, x);
        }

        /* Go for it */
        else {

            /* Take a turn */
            energy_use = 100;

            /* Successful jamming */
            msg_print("You jam the door with a spike.");

            /* Convert "locked" to "stuck" */
            if ((c_ptr->feat & 0x3F) < 0x28) c_ptr->feat |= 0x08;
            
            /* Add one spike to the door */
            if ((c_ptr->feat & 0x07) < 0x07) c_ptr->feat++;
            
            /* Use up, and describe, a single spike, from the bottom */
            inven_item_increase(item, -1);
            inven_item_describe(item);
            inven_item_optimize(item);
        }
    }
}



/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup)
{
    int dir;

    bool more = FALSE;


    /* Get a "repeated" direction */
    if (get_rep_dir(&dir)) {

        /* Take a turn */
        energy_use = 100;

        /* Actually move the character */
        move_player(dir, pickup);

        /* Allow more walking */
        more = TRUE;
    }

    /* Cancel repeat unless we may continue */
    if (!more) disturb(0, 0);
}


/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_stay(int pickup)
{
    cave_type *c_ptr = &cave[py][px];


    /* Take a turn */
    energy_use = 100;
    

    /* Spontaneous Searching */
    if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos))) {
        search();
    }

    /* Continuous Searching */
    if (p_ptr->searching) {
        search();
    }


    /* Hack -- enter a store if we are on one */
    if (((c_ptr->feat & 0x3F) >= 0x08) &&
        ((c_ptr->feat & 0x3F) <= 0x0F)) {
        disturb(0, 0);
        store_enter(c_ptr->feat & 0x07);
    }


    /* Try to Pick up anything under us */
    carry(pickup);
}






/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{
    /* Take a turn */
    energy_use = 100;
    
    /* Search */
    search();
}




/*
 * Resting allows a player to safely restore his hp	-RAK-	
 */
void do_cmd_rest(void)
{
    char out_val[80];


    /* Prompt for time if needed */
    if (command_arg <= 0) {

        /* Assume no rest */
        command_arg = 0;

        /* Ask the question (perhaps a "prompt" routine would be good) */
        prt("Rest for how long? ('*' for HP/mana, '&' as needed): ", 0, 0);

        /* Default to "until done" */
        strcpy(out_val, "&");
        
        /* Ask for how long */
        if (askfor_aux(out_val, 4)) {

            if (out_val[0] == '*') {
                command_arg = (-1);
            }
            else if (out_val[0] == '&') {
                command_arg = (-2);
            }
            else if (out_val[0]) {
                command_arg = atoi(out_val);
                if (command_arg < 0) command_arg = 0;
            }
        }

        /* Nuke prompt */
        prt("", 0, 0);

        /* Handle "cancel" */
        if (!command_arg) return;
    }

    /* Hack -- maximum rest time */
    if (command_arg > 9999) command_arg = 9999;


    /* Hack -- take a turn */
    energy_use = 100;
    
    /* Hack -- Stop searching */
    search_off();

    /* Save the rest code */
    resting = command_arg;

    /* Redraw the state */
    p_ptr->redraw |= (PR_STATE);

    /* Handle stuff */
    handle_stuff();

    /* Refresh */
    Term_fresh();
}



/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling()
{
    /* No useful feeling in town */
    if (!dun_level) {
        msg_print("Looks like a typical town.");
        return;
    }

    /* Analyze the feeling */
    switch (feeling) {
      case 0:
        msg_print("Looks like any other level.");
        break;
      case 1:
        msg_print("You feel there is something special about this level.");
        break;
      case 2:
        msg_print("You have a superb feeling about this level.");
        break;
      case 3:
        msg_print("You have an excellent feeling...");
        break;
      case 4:
        msg_print("You have a very good feeling...");
        break;
      case 5:
        msg_print("You have a good feeling...");
        break;
      case 6:
        msg_print("You feel strangely lucky...");
        break;
      case 7:
        msg_print("You feel your luck is turning...");
        break;
      case 8:
        msg_print("You like the look of this place...");
        break;
      case 9:
        msg_print("This level can't be all bad...");
        break;
      default:
        msg_print("What a boring place...");
        break;
    }
}




/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(void)
{
    int   item;

    inven_type *i_ptr;


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Un-inscribe which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to un-inscribe.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }

    /* Prompt for an inscription */
    if (i_ptr->note) {
        i_ptr->note = 0;
        msg_print("Inscription removed.");
        p_ptr->redraw |= (PR_CHOOSE);
    }
    else {
        msg_print("That item had no inscription to remove.");
    }


    /* Combine the pack */
    p_ptr->update |= (PU_COMBINE);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(void)
{
    int			item;

    inven_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


    /* Get an item (from equip or inven or floor) */
    if (!get_item(&item, "Inscribe which item? ", TRUE, TRUE, TRUE)) {
        if (item == -2) msg_print("You have nothing to inscribe.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }

    /* Describe the activity */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Message */
    msg_format("Inscribing %s.", i_name);
    msg_print(NULL);

    /* Prompt for an inscription */
    prt("Inscription: ", 0, 0);

    /* Replace an old inscription */
    if (i_ptr->note) {

        /* Prepare the default inscription */
        strcpy(out_val, quark_str(i_ptr->note));

        /* Get a new inscription and apply it */
        if (askfor_aux(out_val, 64) && out_val[0]) {
            i_ptr->note = quark_add(out_val);
            p_ptr->redraw |= (PR_CHOOSE);
        }
    }

    /* Get a new inscription */
    else {

        /* Get a new inscription and apply it */
        if (askfor(out_val, 64) && out_val[0]) {
            i_ptr->note = quark_add(out_val);
            p_ptr->redraw |= (PR_CHOOSE);
        }
    }

    /* Combine the pack */
    p_ptr->update |= (PU_COMBINE);
}



/*
 * Print out the artifacts seen.
 * This can be used to notice "missed" artifacts.
 *
 * XXX Perhaps this routine induces a blank final screen.
 *
 * XXX Consider use of "term_mirror"
 */
void do_cmd_check_artifacts(void)
{
    int i, j, k, t;

    char out_val[160];


    /* Hack -- no checking in the dungeon */
    if (dun_level && !wizard) {
        msg_print("You need to be in town to check artifacts!");
        return;
    }


    /* Save the screen */
    Term_save();

    /* Use column 15 */
    j = 15;

    /* Erase some lines */
    for (i = 1; i < 23; i++) prt("", i, j - 2);

    /* Start in line 1 */
    i = 1;

    /* Title the screen */
    prt("Artifacts Seen:", i++, j + 5);

    /* Scan the artifacts */
    for (k = 0; k < MAX_A_IDX; k++) {

        artifact_type *a_ptr = &a_info[k];
        
        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Has that artifact been created? */
        if (a_ptr->cur_num) {

            int z;
            char base_name[80];

            /* Paranoia */
            strcpy(base_name, "Unknown Artifact");

            /* Obtain the base object type */
            z = lookup_kind(a_ptr->tval, a_ptr->sval);

            /* Found it */
            if (z) {

                inven_type forge;

                /* Create the artifact */
                invcopy(&forge, z);
                forge.name1 = k;

                /* Describe the artifact */
                objdes_store(base_name, &forge, FALSE, 0);
            }

            /* Hack -- Build the artifact name */
            sprintf(out_val, "The %s", base_name);

            /* Dump a line */
            prt(out_val, i++, j);

            /* is screen full? */
            if (i == 22) {
                prt("-- more --", i, j);
                if (inkey() == ESCAPE) break;
                for (t = 2; t < 23; t++) prt("", t, j);
                prt("Artifacts seen: (continued)", 1, j + 5);
                i = 2;
            }
        }
    }

    /* Pause */
    prt("[Press any key to continue]", i, j);
    inkey();


    /* Restore the screen */
    Term_load();
}


/*
 * Display the "status" of uniques
 *
 * XXX XXX This routine may induce a blank final screen.
 */
void do_cmd_check_uniques()
{
    int		i, j, k, t;

    char	out_val[160];


    /* Save the screen */
    Term_save();

    /* Column */
    j = 15;

    /* Clear (part of) the screen */
    for (i = 1; i < 23; i++) prt("", i, j - 2);

    /* Row */
    i = 1;

    /* Header */
    prt("Uniques:", i++, j + 5);

    /* Note -- skip the ghost */
    for (k = 1; k < MAX_R_IDX-1; k++) {

        monster_race *r_ptr = &r_info[k];

        /* Only print Uniques */
        if (r_ptr->flags1 & RF1_UNIQUE) {

            bool dead = (r_ptr->max_num == 0);

            /* Only display "known" uniques */
            if (dead || cheat_know || r_ptr->r_sights) {

                /* Print a message */
                sprintf(out_val, "%s is %s.", (r_name + r_ptr->name),
                        dead ? "dead" : "alive");
                prt(out_val, i++, j);

                /* is screen full? */
                if (i == 22) {
                    prt("-- more --", i, j);
                    if (inkey() == ESCAPE) break;
                    for (t = 2; t < 23; t++) prt("", t, j);
                    prt("Uniques: (continued)", 1, j + 5);
                    i = 2;
                }
            }
        }
    }

    /* Pause */
    prt("[Press any key to continue]", i, j);
    inkey();

    /* Restore the screen */
    Term_load();
}




