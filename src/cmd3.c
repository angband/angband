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
    monster_race *r_ptr = &r_list[m_ptr->r_idx];

    bool          living = TRUE;
    int           perc;


    /* Determine if the monster is "living" (vs "undead") */
    if (r_ptr->rflags3 & RF3_UNDEAD) living = FALSE;
    if (r_ptr->rflags3 & RF3_DEMON) living = FALSE;
    if (strchr("EgvX", r_ptr->r_char)) living = FALSE;


    /* Healthy monsters */
    if (m_ptr->hp >= m_ptr->maxhp) {

        /* Paranoia */
        m_ptr->hp = m_ptr->maxhp;

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
 * Determine if the given grid should be "examined"
 *
 * Note that "axis" directions accept a full 180 degrees
 * and "diagonal" directions accept a full 90 degrees.
 * This is much more "lenient" than the old method.
 */
static bool do_cmd_look_accept(int y, int x, int d, int r)
{
    /* Skip illegal grids */
    if (!in_bounds(y, x)) return (FALSE);

    /* XXX XXX Hack -- Skip unseen grids */
    if (!player_has_los_bold(y, x)) return (FALSE);

    /* Skip grids of the incorrect distance */
    if (distance(py, px, y, x) != r) return (FALSE);

    /* Accept all grids sometimes */
    /* if (d == 5) return (TRUE); */

    /* Hack -- Verify the "Y" offset */
    if (ddy[d] && (SGN(y-py) != ddy[d])) return (FALSE);

    /* Hack -- Verify the "X" offset */
    if (ddx[d] && (SGN(x-px) != ddx[d])) return (FALSE);

    /* Assume okay */
    return (TRUE);
}


/*
 * Examine a grid.  Could limit observations of walls.
 * The "full" parameter is TRUE for "look", FALSE for "examine".
 */
static bool do_cmd_look_examine(int y, int x, int full, int *seen)
{
    cave_type		*c_ptr = &cave[y][x];

    inven_type		*i_ptr = &i_list[c_ptr->i_idx];
    monster_type	*m_ptr = &m_list[c_ptr->m_idx];
    monster_race	*r_ptr = &r_list[m_ptr->r_idx];

    int			wall = (c_ptr->info & GRID_WALL_MASK);

    cptr		s1 = "You see ", s2 = "", s3 = "";

    bool		do_wall = FALSE;

    int			query;

    char		m_name[80];
    char		i_name[160];

    char		out_val[160];


    /* Hack -- Place walls under secret doors */
    if (i_ptr->tval == TV_SECRET_DOOR) wall = GRID_WALL_GRANITE;

    /* Hack -- Ignore secret doors */
    if (i_ptr->tval == TV_SECRET_DOOR) i_ptr = &i_list[0];

    /* Hack -- Ignore invisible traps */
    if (i_ptr->tval == TV_INVIS_TRAP) i_ptr = &i_list[0];


    /* Hack -- looking under the player */
    if (c_ptr->m_idx == 1) s1 = "You are on ";

    /* Hack -- Player is not a monster */
    if (c_ptr->m_idx == 1) m_ptr = &m_list[0];


    /* Hack -- Invisible monsters */
    if (!m_ptr->ml) m_ptr = &m_list[0];


    /* Convert seams to walls if requested */
    if (wall && !notice_seams) wall = GRID_WALL_GRANITE;

    /* Hack -- describe the first few seams */
    if (wall && (wall != GRID_WALL_GRANITE) && (*seen < 3)) do_wall = TRUE;


    /* Actual monsters */
    if (m_ptr->r_idx) {

        /* Saw something */
        (*seen)++;

        /* Get the monster name ("a kobold") */
        monster_desc(m_name, m_ptr, 0x08);

        /* Auto-recall */
        if (use_recall_win && term_recall) {

            /* Auto-recall */
            roff_recall(m_ptr->r_idx);

            /* Describe, and wait for a response */
            sprintf(out_val, "%s%s%s (%s).  --pause--",
                    s1, s2, m_name, look_mon_desc(c_ptr->m_idx));
            prt(out_val, 0, 0);
            move_cursor_relative(y, x);
            query = inkey();

            /* Abort if needed */
            if (query == ESCAPE) return (FALSE);
        }

        /* Prompt for recall */
        else {

            /* Describe, and prompt for recall */
            sprintf(out_val, "%s%s%s (%s) [(r)ecall]",
                    s1, s2, m_name, look_mon_desc(c_ptr->m_idx));
            prt(out_val, 0, 0);
            move_cursor_relative(y, x);
            query = inkey();

            /* Recall as needed */
            if (query == 'r' || query == 'R') {

                /* Recall */
                prt("",0,0);
                save_screen();
                query = roff_recall(m_ptr->r_idx);
                restore_screen();
            }

            /* Abort if needed */
            if (query == ESCAPE) return (FALSE);
        }

        /* Hack -- one chance */
        if (!full) return (TRUE);

        /* Change the intro */
        s1 = "It is ";

        /* Hack -- take account of gender */
        if (r_ptr->rflags1 & RF1_FEMALE) s1 = "She is ";
        else if (r_ptr->rflags1 & RF1_MALE) s1 = "He is ";

        /* Choose a preposition */
        s2 = (i_ptr->k_idx ? "on " : "in ");

        /* Do walls */
        do_wall = TRUE;
    }


    /* XXX XXX Hack -- Skip "dark" grids */
    if (!test_lite_bold(y, x)) return (TRUE);


    /* Actual items */
    if (i_ptr->k_idx) {

        /* Saw something */
        (*seen)++;

        /* Obtain an object description */
        objdes(i_name, i_ptr, TRUE);

        /* Describe the object */
        sprintf(out_val, "%s%s%s.  ---pause---", s1, s2, i_name);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Abort if needed */
        if (query == ESCAPE) return (FALSE);

        /* Hack -- one chance */
        if (!full) return (TRUE);

        /* Change the intro */
        s1 = "It is ";
        s2 = "in ";

        /* Do walls */
        do_wall = TRUE;
    }


    /* Important walls */
    if (wall && do_wall) {

        /* Saw something */
        (*seen)++;

        /* Assume granite wall */
        s3 = "a granite wall";

        /* Notice seams */
        if (wall == GRID_WALL_MAGMA) s3 = "some dark rock";
        if (wall == GRID_WALL_QUARTZ) s3 = "a quartz vein";

        /* Describe walls */
        sprintf(out_val, "%s%s%s.  ---pause---", s1, s2, s3);
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();

        /* Abort if needed */
        if (query == ESCAPE) return (FALSE);
    }


    /* Keep going */
    return (TRUE);
}


/*
 * A new "look" command.
 *
 * We examine grids based on "range", from close up to far away
 * We allow the grids to be restricted by general direction
 *
 * Grids at the appropriate range are processed from north to south
 * and then west to east.  Thus looking "north/south" has a slightly
 * different "feel" from looking "east/west", and looking "everywhere"
 * will look very strange.  We should attempt a rotary pattern.
 */
void do_cmd_look(void)
{
    int		dir, r, y, x;
    int		seen = 0;
    bool	okay = TRUE;


    /* This is a free move */
    energy_use = 0;

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

    /* Get a direction (or "5"), ignoring target and confusion */
    if (!get_a_dir("Look which direction? ", &dir, 0x04)) return;

    /* Look outwards (by distance) */
    for (r = 0; okay && (r < MAX_SIGHT); r++) {

        /* Scan the map */
        for (y = py - r; okay && (y <= py + r); y++) {
            for (x = px - r; okay && (x <= px + r); x++) {

                /* Check the grid */
                if (do_cmd_look_accept(y, x, dir, r)) {

                    /* Examine the grid (fully) */
                    if (!do_cmd_look_examine(y, x, TRUE, &seen)) okay = FALSE;
                }
            }
        }
    }


    /* Aborted look */
    if (!okay) {
        msg_print("Aborting look.");
    }

    /* Nothing to see */
    else if (!seen) {
        if (dir == 5) {
            msg_print("You see nothing of interest.");
        }
        else {
            msg_print("You see nothing of interest in that direction.");
        }
    }

    /* All done */
    else {
        if (dir == 5) {
            msg_print("That's all you see.");
        }
        else {
            msg_print("That's all you see in that direction.");
        }
    }
}




/*
 * Examine the current target location
 */
void do_cmd_examine(void)
{
    int seen = 0;

    bool full = FALSE;


    /* Free move */
    energy_use = 0;

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

    /* No target to examine */
    if (!target_okay()) {
        msg_print("You have no target to examine.");
        return;
    }

    /* Use actual "look" when possible */
    if (player_can_see_bold(target_row, target_col)) full = TRUE;

    /* Examine the grid */
    if (!do_cmd_look_examine(target_row, target_col, full, &seen)) return;

    /* Nothing to see here */
    if (!seen) msg_print("You see nothing special about the current target.");
}


/*
 * Display a compressed map
 */
void do_cmd_view_map()
{
    /* Free move */
    energy_use = 0;

    /* Look at the map */
    screen_map();
}


/*
 * Given an row (y) and col (x), recenter the "panel".
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
static bool do_cmd_locate_aux(int y, int x)
{
    int prow = panel_row;
    int pcol = panel_col;

    prow = ((y - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
    if (prow > max_panel_rows) prow = max_panel_rows;
    else if (prow < 0) prow = 0;

    pcol = ((x - SCREEN_WID / 4) / (SCREEN_WID / 2));
    if (pcol > max_panel_cols) pcol = max_panel_cols;
    else if (pcol < 0) pcol = 0;

    /* Check for "no change" */
    if ((prow == panel_row) && (pcol == panel_col)) return (FALSE);

    /* Save the new panel info */
    panel_row = prow;
    panel_col = pcol;

    /* Recalculate the boundaries */
    panel_bounds();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
    
    /* Handle stuff */
    handle_stuff(TRUE);
    
    /* The map was redrawn */
    return (TRUE);
}



/*
 * Support code for the "Locate ourself on the Map" command
 */
void do_cmd_locate()
{
    int		dir_val, y, x, cy, cx, p_y, p_x;

    char	out_val[80];
    char	tmp_str[80];


    /* Free move */
    energy_use = 0;


    /* Save character location */
    y = py;
    x = px;

    /* Move to a new panel */
    (void)do_cmd_locate_aux(y, x);

    /* Extract (original) panel info */
    cy = panel_row;
    cx = panel_col;


    /* Show panels until done */
    while (1) {

        /* Save panel info */
        p_y = panel_row;
        p_x = panel_col;

        /* Describe the location */
        if ((p_y == cy) && (p_x == cx)) {
            tmp_str[0] = '\0';
        }
        else {
            (void)sprintf(tmp_str, "%s%s of",
                (p_y < cy) ? " North" : (p_y > cy) ? " South" : "",
                (p_x < cx) ? " West" : (p_x > cx) ? " East" : "");
        }


        /* Prepare to ask which way to look */
        (void)sprintf(out_val,
            "Map sector [%d,%d], which is%s your sector. Look which direction?",
            p_y, p_x, tmp_str);

        /* Get a direction (or Escape) */
        if (!get_a_dir(out_val, &dir_val, 0)) break;


        /* Keep "moving" until the panel changes */
        while (1) {

            /* Apply the direction */
            x += ((dir_val - 1) % 3 - 1) * SCREEN_WID / 2;
            y -= ((dir_val - 1) / 3 - 1) * SCREEN_HGT / 2;

            /* No motion off map */
            if (x < 0 || y < 0 || x >= cur_wid || y >= cur_wid) {
                msg_print("You've gone past the end of your map.");
                x -= ((dir_val - 1) % 3 - 1) * SCREEN_WID / 2;
                y += ((dir_val - 1) / 3 - 1) * SCREEN_HGT / 2;
                break;
            }

            /* Hack -- keep sliding until done (?) */
            if (do_cmd_locate_aux(y, x)) break;
        }
    }


    /* Recenter the map around the player */
    verify_panel();

    /* Update stuff */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_MAP);
    
    /* Handle stuff */
    handle_stuff(TRUE);
}



/*
 * Allocates objects upon opening a chest    -BEN-
 *
 * Disperse treasures from the chest "i_ptr", centered at (x,y).
 * Adapted from "monster_death()", but much simpler...
 */
static void chest_death(int y, int x, inven_type *i_ptr)
{
    int			i, d, y1, x1, number;

    bool	do_item = (i_ptr->flags1 & CH1_CARRY_OBJ) ? TRUE : FALSE;
    bool	do_gold = (i_ptr->flags1 & CH1_CARRY_GOLD) ? TRUE : FALSE;


    /* Must be a chest */
    if (i_ptr->tval != TV_CHEST) return;

    /* Count how many objects */
    number = 0;
    if ((i_ptr->flags1 & CH1_HAS_60) && (randint(100) < 60)) number++;
    if ((i_ptr->flags1 & CH1_HAS_90) && (randint(100) < 90)) number++;
    if (i_ptr->flags1 & CH1_HAS_1D2) number += randint(2);
    if (i_ptr->flags1 & CH1_HAS_2D2) number += damroll(2, 2);
    if (i_ptr->flags1 & CH1_HAS_4D2) number += damroll(4, 2);

    /* Summon some objects */
    if (number > 0) {

        /* Drop some objects (non-chests) */
        for ( ; number > 0; --number) {

            /* Try 20 times per item */
            for (i = 0; i < 20; ++i) {

                /* Pick a distance */
                d = 2;

                /* Pick a location */
                while (1) {
                    y1 = rand_spread(y, d);
                    x1 = rand_spread(x, d);
                    if (!in_bounds(y1, x1)) continue;
                    if (distance(y, x, y1, x1) > d) continue;
                    if (los(y, x, y1, x1)) break;
                }

                /* Must be a clean floor grid */
                if (!clean_grid_bold(y1, x1)) continue;

                /* Opening a chest */
                opening_chest = TRUE;

                /* The "pval" of a chest is how "good" it is */
                object_level = i_ptr->pval;

                /* Place an Item or Gold */
                if (do_gold && (rand_int(2) == 0)) {
                    place_gold(y1, x1);
                }
                else if (do_item) {
                    place_object(y1, x1);
                }
                else if (do_gold) {
                    place_gold(y1, x1);
                }

                /* Reset the object level */
                object_level = dun_level;

                /* No longer opening a chest */
                opening_chest = FALSE;

                /* Actually display the object's grid */
                lite_spot(y1, x1);

                /* Successful placement */
                break;
            }
        }
    }

    /* The chest is now identified */
    inven_known(i_ptr);

    /* The chest is "dead" */
    i_ptr->cost = 0L;
    i_ptr->flags1 = 0L;
    i_ptr->flags2 = 0L;
    i_ptr->pval = 0;
}


/*
 * Chests have traps too.
 * Note: Chests now use "flags2" for their traps
 * Exploding chest destroys contents, and traps.
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, inven_type *i_ptr)
{
    int        i;

    if (i_ptr->tval != TV_CHEST) return;

    if (i_ptr->flags2 & CH2_LOSE_STR) {
        msg_print("A small needle has pricked you!");
        if (!p_ptr->sustain_str) {
            (void)dec_stat(A_STR, 10, FALSE);
            take_hit(damroll(1, 4), "a poison needle");
            msg_print("You feel weaker!");
        }
        else {
            msg_print("You are unaffected.");
        }
    }

    if (i_ptr->flags2 & CH2_POISON) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 6), "a poison needle");
        if (!(p_ptr->resist_pois ||
              p_ptr->oppose_pois ||
              p_ptr->immune_pois)) {
            p_ptr->poisoned += 10 + randint(20);
        }
    }

    if (i_ptr->flags2 & CH2_PARALYSED) {
        msg_print("A puff of yellow gas surrounds you!");
        if (p_ptr->free_act) {
            msg_print("You are unaffected.");
        }
        else {
            msg_print("You choke and pass out.");
            p_ptr->paralysis = 10 + randint(20);
        }
    }

    if (i_ptr->flags2 & CH2_SUMMON) {
        for (i = 0; i < 3; i++) {
            (void)summon_monster(y, x, dun_level + MON_SUMMON_ADJ);
        }
    }

    if (i_ptr->flags2 & CH2_EXPLODE) {
        msg_print("There is a sudden explosion!");
        msg_print("Everything inside the chest is destroyed!");
        i_ptr->flags1 = 0L;
        i_ptr->flags2 = 0L;
        take_hit(damroll(5, 8), "an exploding chest");
    }
}





/*
 * Opens a closed door or closed chest.		-RAK-
 * Note that failed opens take time, or ghosts could be found
 * Note unlocking a door is worth one XP, and unlocking a chest
 * is worth as many XP as the chest had "levels".
 */
void do_cmd_open()
{
    int				y, x, i, j, dir;
    int				flag;
    cave_type		*c_ptr;
    monster_type	*m_ptr;
    inven_type		*i_ptr;


    /* Assume we will not continue repeating this command */
    int more = FALSE;

    /* Get a direction (or Escape) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        /* Graceful exit */
        energy_use = 0;
    }

    else {

        /* Apply partial confusion */
        dir = command_dir;
        confuse_dir(&dir, 0x02);

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get requested grid */
        c_ptr = &cave[y][x];

        /* Get the monster (if any) */
        m_ptr = &m_list[c_ptr->m_idx];

        /* Get the object (if any) */
        i_ptr = &i_list[c_ptr->i_idx];

        /* Nothing is there */
        if ((i_ptr->tval != TV_CLOSED_DOOR) &&
            (i_ptr->tval != TV_CHEST)) {
            msg_print("I do not see anything you can open there.");
            energy_use = 0;
        }

        /* Monster in the way */
        else if (c_ptr->m_idx > 1) {

            char m_name[80];

            /* Acquire "Monster" (or "Something") */
            monster_desc(m_name, m_ptr, 0x04);
            message(m_name, 0x03);
            message(" is in your way!", 0);
        }

        /* Closed door */
        else if (i_ptr->tval == TV_CLOSED_DOOR) {

            /* Stuck */
            if (i_ptr->pval < 0) {
                msg_print("It appears to be stuck.");
            }

            /* Locked */
            else if (i_ptr->pval > 0) {

                /* Disarm factor */
                i = (p_ptr->disarm + todis_adj() + stat_adj(A_INT) +
                     (class_level_adj[p_ptr->pclass][CLA_DISARM] * 
                      p_ptr->lev / 3));

                /* Penalize some conditions */
                if (p_ptr->blind || no_lite()) i = i / 10;
                if (p_ptr->confused || p_ptr->image) i = i / 10;
                
                /* Extract the difficulty */
                j = i - i_ptr->pval;
                
                /* Always have a small chance of success */
                if (j < 2) j = 2;

                /* Success */
                if (rand_int(100) < j) {
                    msg_print("You have picked the lock.");
                    p_ptr->exp++;
                    check_experience();
                    i_ptr->pval = 0;
                }

                /* Failure */
                else {
                    /* We may keep trying */
                    more = TRUE;
                    if (flush_failure) flush();
                    msg_print("You failed to pick the lock.");
                }
            }

            /* In any case, if the door is unlocked, open it */
            if (i_ptr->pval == 0) {

                invcopy(i_ptr, OBJ_OPEN_DOOR);
                i_ptr->iy = y;
                i_ptr->ix = x;

                /* Hack -- nuke any walls */
                c_ptr->info &= ~GRID_WALL_MASK;

                /* Draw the door */
                lite_spot(y, x);

                /* Update some things */
                p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
            }
        }

        /* Open a closed chest. */
        else if (i_ptr->tval == TV_CHEST) {

            /* Assume opened successfully */
            flag = TRUE;

            /* Attempt to unlock it */
            if (i_ptr->flags2 & CH2_LOCKED) {

                /* Assume locked, and thus not open */
                flag = FALSE;

                /* Get the "disarm" factor */
                i = (p_ptr->disarm + todis_adj() + stat_adj(A_INT) +
                     (class_level_adj[p_ptr->pclass][CLA_DISARM] *
                      p_ptr->lev / 3));

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
                    i_ptr->flags2 &= ~CH2_LOCKED;
                    p_ptr->exp += i_ptr->pval;
                    check_experience();
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
                if (i_ptr->flags2) chest_trap(y, x, i_ptr);

                /* Let the Chest drop items */
                chest_death(y, x, i_ptr);
            }
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
    inven_type		*i_ptr;
    monster_type	*m_ptr;


    /* Get a "desired" direction, or Abort */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        /* Abort gracefully */
        energy_use = 0;
    }

    else {

        /* Apply partial confusion */
        dir = command_dir;
        confuse_dir(&dir, 0x02);

        /* Get requested location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];
        i_ptr = &i_list[c_ptr->i_idx];
        m_ptr = &m_list[c_ptr->m_idx];

        /* Require open door */
        if (i_ptr->tval != TV_OPEN_DOOR) {

            msg_print("I do not see anything you can close there.");
            energy_use = 0;
        }

        /* Handle broken doors */
        else if (i_ptr->pval) {
            msg_print("The door appears to be broken.");
            energy_use = 0;
        }

        /* Monster in the way */
        else if (c_ptr->m_idx > 1) {

            char m_name[80];

            /* Acquire "Monster" (or "Something") */
            monster_desc(m_name, m_ptr, 0x04);

            message(m_name, 0x03);
            message(" is in your way!", 0);
        }

        /* Close it */
        else {

            /* Hack -- kill the old object */
            i_ptr = &i_list[c_ptr->i_idx];
            invcopy(i_ptr, OBJ_CLOSED_DOOR);

            /* Place it in the dungeon */
            i_ptr->iy = y;
            i_ptr->ix = x;

            /* Redisplay */
            lite_spot(y, x);

            /* Update some things */
            p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
        }
    }
}


/*
 * Tunnels through rubble and walls			-RAK-
 * Must take into account: secret doors,  special tools
 *
 * Note that tunneling almost always takes time, since otherwise
 * you can use tunnelling to find monsters.  Also note that you
 * must tunnel in order to hit monsters in walls or on closed doors.
 */
void do_cmd_tunnel()
{
    int			i, tabil, y, x, dir;

    cave_type		*c_ptr;
    inven_type		*i_ptr;
    monster_type	*m_ptr;

    inven_type		*j_ptr;

    /* Assume we cannot continue */
    int more = FALSE;


    /* Get a direction to tunnel, or Abort */
    if (!get_a_dir (NULL, &command_dir, 0)) {

        /* Abort the tunnel, be graceful */
        energy_use = 0;
    }

    else {

        /* Notice visibility changes */
        bool old_floor = FALSE;

        /* Take partial confusion into account */
        dir = command_dir;
        confuse_dir(&dir, 0x02);

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];
        i_ptr = &i_list[c_ptr->i_idx];
        m_ptr = &m_list[c_ptr->m_idx];

        /* Check the floor-hood */
        old_floor = floor_grid_bold(y, x);

        /* And then apply the current weapon type */
        j_ptr = &inventory[INVEN_WIELD];

        /* Check the validity */
        if (old_floor) {
            energy_use = 0;
            msg_print("You see nothing there to tunnel through.");
        }

        /* A monster is in the way */
        else if (c_ptr->m_idx > 1) {

            char m_name[80];

            /* Acquire "Monster" (or "Something") */
            monster_desc(m_name, m_ptr, 0x04);

            message(m_name, 0x03);
            message(" is in your way!", 0);

            /* Attempt an attack */
            if (p_ptr->afraid < 1) py_attack(y, x);
            else msg_print("You are too afraid!");
        }

        /* Hack -- no tunnelling through doors */
        else if (i_ptr->tval == TV_CLOSED_DOOR) {
            msg_print("You cannot tunnel through doors.");
        }

        /* You cannot dig without a weapon */
        else if (!j_ptr->tval) {
            msg_print("You dig with your hands, making no progress.");
        }

        /* Hack -- Penalize heavy weapon */
        else if (p_ptr->use_stat[A_STR] * 15 < j_ptr->weight) {
            msg_print("Your weapon is too heavy for you to dig with.");
        }

        /* Okay, try digging */
        else {

            /* Compute the digging ability of player based on strength */
            tabil = p_ptr->use_stat[A_STR];

            /* Special diggers (includes all shovels, etc) */
            if (j_ptr->flags1 & TR1_TUNNEL) {

                /* The "pval" is really important */
                tabil += 25 + j_ptr->pval * 50;
            }

            /* Normal weapon */
            else {

                /* Good weapons make digging easier */
                tabil += (j_ptr->dd * j_ptr->ds);

                /* The weapon bonuses help too */
                tabil += (j_ptr->tohit + j_ptr->todam);

                /* But without a shovel, digging is hard */
                tabil = tabil / 2;
            }

            /* Regular walls; Granite, magma intrusion, quartz vein  */
            /* Don't forget the boundary walls, made of titanium (255) */

            if (c_ptr->info & GRID_PERM) {
                msg_print("This seems to be permanent rock.");
            }

            else if ((c_ptr->info & GRID_WALL_MASK) == GRID_WALL_MAGMA) {

                i = randint(600) + 10;
                if (twall(y, x, tabil, i)) {
                    if ((c_ptr->i_idx) && player_can_see_bold(y, x)) {
                        msg_print("You have found something!");
                    }
                    else {
                        msg_print("You have finished the tunnel.");
                    }
                }
                else {
                    /* We may continue tunelling */
                    msg_print("You tunnel into the magma intrusion.");
                    more = TRUE;
                }
            }

            else if ((c_ptr->info & GRID_WALL_MASK) == GRID_WALL_QUARTZ) {

                i = randint(400) + 10;
                if (twall(y, x, tabil, i)) {
                    if ((c_ptr->i_idx) && player_can_see_bold(y, x)) {
                        msg_print("You have found something!");
                    }
                    else {
                        msg_print("You have finished the tunnel.");
                    }
                }
                else {
                    /* We may continue tunelling */
                    msg_print("You tunnel into the quartz vein.");
                    more = TRUE;
                }
            }

            else if ((c_ptr->info & GRID_WALL_MASK) == GRID_WALL_GRANITE) {

                i = randint(1200) + 80;
                if (twall(y, x, tabil, i)) {
                    if ((c_ptr->i_idx) && player_can_see_bold(y, x)) {
                        msg_print("You have found something!");
                    }
                    else {
                        msg_print("You have finished the tunnel.");
                    }
                }
                else {
                    /* We may continue tunelling */
                    msg_print("You tunnel into the granite wall.");
                    more = TRUE;
                }
            }

            /* Secret doors. */
            else if (i_ptr->tval == TV_SECRET_DOOR) {
                /* We may continue tunelling */
                msg_print("You tunnel into the granite wall.");
                search(py, px, p_ptr->srh);
                more = TRUE;
            }

            /* Rubble */
            else if (i_ptr->tval == TV_RUBBLE) {
                if (tabil > randint(180)) {
                    delete_object(y, x);
                    msg_print("You have removed the rubble.");
                    if (rand_int(10) == 0) {
                        place_object(y, x);
                        if (test_lite_bold(y, x)) {
                             msg_print("You have found something!");
                        }
                    }
                    lite_spot(y, x);
                }
                else {
                    /* We may continue tunelling */
                    more = TRUE;
                    msg_print("You dig in the rubble.");
                }
            }

            /* Anything else is illegal */
            else {
                msg_print("You can't tunnel through that.");
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
    int                 y, x, i, j, dir;

    cave_type		*c_ptr;
    inven_type		*i_ptr;
    monster_type	*m_ptr;

    char		o_name[160];

    /* Assume we cannot continue repeating */
    int more = FALSE;


    /* Get a direction (or abort) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        /* Abort Gracefully */
        energy_use = 0;
    }

    else {

        dir = command_dir;
        confuse_dir(&dir, 0x02);

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];
        i_ptr = &i_list[c_ptr->i_idx];
        m_ptr = &m_list[c_ptr->m_idx];

        /* Nothing useful there */
        if ((i_ptr->tval != TV_VIS_TRAP) &&
            (i_ptr->tval != TV_CHEST)) {

            msg_print("I do not see anything there to disarm.");
            energy_use = 0;
        }

        /* Monster in the way */
        else if (c_ptr->m_idx > 1) {

            char m_name[80];

            /* Acquire "Monster" (or "Something") */
            monster_desc(m_name, m_ptr, 0x04);

            /* Message */
            message(m_name, 0x03);
            message(" is in your way!", 0);
        }

        /* Normal disarm */
        else {

            /* Get the "disarm" factor */
            i = (p_ptr->disarm + todis_adj() + stat_adj(A_INT) +
                 (class_level_adj[p_ptr->pclass][CLA_DISARM] *
                  p_ptr->lev / 3));

            /* Penalize some conditions */
            if (p_ptr->blind || no_lite()) i = i / 10;
            if (p_ptr->confused || p_ptr->image) i = i / 10;
            
            /* Extract the difficulty */
            j = i - i_ptr->pval;
                
            /* Always have a small chance of success */
            if (j < 2) j = 2;

            /* Floor trap */
            if (i_ptr->tval == TV_VIS_TRAP) {

                /* Describe the trap (as in "spiked pit") */
                objdes(o_name, i_ptr, FALSE);

                /* Success */
                if (rand_int(100) < j) {
                    msg_print(format("You have disarmed the %s.", o_name));
                    p_ptr->exp += i_ptr->pval;
                    delete_object(y, x);
                    /* move the player onto the trap grid */
                    move_player(dir, FALSE);
                    check_experience();
                }

                /* Failure -- Keep trying */
                else if ((i > 5) && (randint(i) > 5)) {
                    /* We may keep trying */
                    more = TRUE;
                    if (flush_failure) flush();
                    msg_print(format("You failed to disarm the %s.", o_name));
                }

                /* Failure -- Set off the trap */
                else {
                    msg_print("You set off the trap!");
                    /* Move the player onto the trap */
                    move_player(dir, FALSE);
                }
            }

            /* Disarm chest */
            else if (i_ptr->tval == TV_CHEST) {

                /* Must find the trap first. */
                if (!inven_known_p(i_ptr)) {
                    msg_print("I don't see any traps.");
                    energy_use = 0;
                }

                /* No traps to find. */
                else if (!(i_ptr->flags2 & CH2_TRAP_MASK)) {
                    msg_print("The chest is not trapped.");
                    energy_use = 0;
                }

                /* Success */
                else if (rand_int(100) < j) {
                    i_ptr->flags2 &= ~CH2_TRAP_MASK;
                    i_ptr->flags2 |= CH2_DISARMED;
                    msg_print("You have disarmed the chest.");
                    p_ptr->exp += i_ptr->pval;
                    check_experience();
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
        }
    }


    /* Cancel repeat unless told not to */
    if (!more) disturb(0, 0);
}


/*
 * Bash open a door or chest				-RAK-
 *
 * Note: Affected by strength and weight of character
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 * A disarm spell unlocks and unjams doors!
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open doors. A creature with open door ability will
 * (if not in the line of sight) move though a closed or secret door with
 * no changes.  If in the line of sight, closed door are openned, & secret
 * door revealed.  Whether in the line of sight or not, such a creature may
 * unlock or unstick a door.  That is, creatures shut doors behind them,
 * and repair ones they break (oops).  A creature with no such ability
 * will attempt to bash a non-secret door.
 *
 * Note that all forms of bashing now take time, even if silly, so that
 * no information is given away by bashing at invisible creatures.
 */
void do_cmd_bash()
{
    int                 y, x, tmp, dir;
    cave_type  *c_ptr;
    inven_type *i_ptr;

    /* Assume we cannot keep bashing */
    int more = FALSE;

    /* Get a direction (or Escape) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        /* Graceful abort */
        energy_use = 0;
    }

    /* Execute the bash */
    else {

        /* Extract bash direction, apply partial confusion */
        dir = command_dir;
        confuse_dir(&dir, 0x02);

        /* Bash location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid */
        c_ptr = &cave[y][x];

        /* Request to bash a monster */
        if (c_ptr->m_idx > 1) {
            if (p_ptr->afraid) {
                msg_print("You are too afraid!");
            }
            else {
                py_bash(y, x);
            }
        }

        /* Request to bash something */
        else if (c_ptr->i_idx) {

            /* What is there */
            i_ptr = &i_list[c_ptr->i_idx];

            /* Bash a closed door */
            if (i_ptr->tval == TV_CLOSED_DOOR) {

                msg_print("You smash into the door!");

                tmp = p_ptr->use_stat[A_STR] + p_ptr->wt / 2;

                /* Use (roughly) similar method as for monsters. */
                if (randint(tmp * (20 + ABS(i_ptr->pval))) <
                        10 * (tmp - ABS(i_ptr->pval))) {

                    msg_print("The door crashes open!");

                    /* Hack -- drop on the old object */
                    invcopy(i_ptr, OBJ_OPEN_DOOR);

                    /* Place it in the dungeon */
                    i_ptr->iy = y;
                    i_ptr->ix = x;

                    /* 50% chance of breaking door */
                    i_ptr->pval = 1 - randint(2);

                    /* Show the door */
                    lite_spot(y, x);

                    /* Hack -- Fall through the door */
                    move_player(dir, FALSE);

                    /* Update some things */
                    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
                    p_ptr->update |= (PU_DISTANCE);
                }

                else if (randint(150) > p_ptr->use_stat[A_DEX]) {
                    /* Note: this will cancel "repeat" */
                    p_ptr->paralysis = 1 + randint(2);
                    msg_print("You are off-balance.");
                }

                else {
                    /* Allow repeated bashing until dizzy */
                    more = TRUE;
                    msg_print("The door holds firm.");
                }
            }

            /* Semi-Hack -- Bash a Chest */
            else if (i_ptr->tval == TV_CHEST) {
                if (rand_int(10) == 0) {
                    int tmp_iy = i_ptr->iy;
                    int tmp_ix = i_ptr->ix;
                    msg_print("You have destroyed the chest and its contents!");
                    invcopy(i_ptr, OBJ_RUINED_CHEST);
                    i_ptr->iy = tmp_iy;
                    i_ptr->ix = tmp_ix;
                }
                else if ((i_ptr->flags2 & CH2_LOCKED) && (rand_int(10) == 0)) {
                    msg_print("The lock breaks open!");
                    i_ptr->flags2 &= ~CH2_LOCKED;
                }
                else {
                    /* We may continue */
                    more = TRUE;
                    msg_print("The chest holds firm.");
                }
            }

            /* Bash something else (including secret doors) */
            else {
                msg_print("You bash it, but nothing interesting happens.");
            }
        }

        /* Walls (see "secret doors" above) */
        else if (c_ptr->info & GRID_WALL_MASK) {
            msg_print("You bash it, but nothing interesting happens.");
        }

        /* Nothing */
        else {
            msg_print("You bash at empty space.");
        }
    }

    /* Unless valid action taken, cancel bash */
    if (!more) disturb(0, 0);
}


/*
 * Jam a closed door with a spike -RAK-
 *
 * Be sure not to allow the user to "find" ghosts by spiking
 * Thus, anything that could indicate a monster takes a turn
 */
void do_cmd_spike()
{
    int                  y, x, dir, i, j;

    cave_type		*c_ptr;
    inven_type		*i_ptr;
    monster_type	*m_ptr;

    /* Assume we will not continue with the repeating */
    int			more = FALSE;


    /* Get a direction (or cancel) */
    if (!get_a_dir(NULL, &command_dir, 0)) {
        /* Abort gracefully */
        energy_use = 0;
    }

    else {

        /* Confuse the direction (partially) */
        dir = command_dir;
        confuse_dir (&dir, 0x02);

        /* Get location */
        y = py + ddy[dir];
        x = px + ddx[dir];

        /* Get grid and contents */
        c_ptr = &cave[y][x];
        i_ptr = &i_list[c_ptr->i_idx];
        m_ptr = &m_list[c_ptr->m_idx];

        /* Open doors must be closed */
        if (i_ptr->tval == TV_OPEN_DOOR) {
            msg_print("The door must be closed first.");
            energy_use = 0;
        }

        /* Nothing there? */
        else if (i_ptr->tval != TV_CLOSED_DOOR) {
            msg_print("I see no door there.");
            energy_use = 0;
        }

        /* Make sure the player has spikes */
        else if (!find_range(TV_SPIKE, &i, &j)) {
            msg_print("But you have no spikes.");
            energy_use = 0;
        }

        /* Is a monster in the way? */
        else if (c_ptr->m_idx) {

            char m_name[80];

            /* Acquire "Monster" (or "Something") */
            monster_desc(m_name, m_ptr, 0x04);

            message(m_name, 0x03);
            message(" is in your way!", 0);
        }

        /* Go for it */
        else {

            /* We may continue spiking (unless out of spikes) */
            more = TRUE;

            /* Successful jamming */
            msg_print("You jam the door with a spike.");

            /* Make locked to stuck. */
            if (i_ptr->pval > 0) i_ptr->pval = (-i_ptr->pval);

            /* Successive spikes have a progressively smaller effect. */
            /* Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
            i_ptr->pval -= 1 + 190 / (10 - i_ptr->pval);

            /* Use up, and describe, a single spike, from the bottom */
            inven_item_increase(j, -1);
            inven_item_describe(j);
            inven_item_optimize(j);
        }
    }

    /* Cancel repetition unless it worked */
    if (!more) disturb(0, 0);
}


/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{
    /* Use the current location, and ability */
    search(py, px, p_ptr->srh);
}




/*
 * Resting allows a player to safely restore his hp	-RAK-	
 */
void do_cmd_rest(void)
{
    char ch;

    char rest_str[80];


    /* Prompt for time if needed */
    if (command_arg <= 0) {

        /* Assume no rest */
        command_arg = 0;

        /* Ask the question (perhaps a "prompt" routine would be good) */
        prt("Rest for how long? ('*' for HP/mana; '&' as needed): ", 0, 0);

        if (askfor(rest_str, 5)) {
            if (sscanf(rest_str, "%c", &ch) == 1) {
                if (ch == '*') {
                    command_arg = (-1);
                }
                else if (ch == '&') {
                    command_arg = (-2);
                }
                else {
                    command_arg = atoi(rest_str);
                    if (command_arg > 30000) command_arg = 30000;
                    if (command_arg < 0) command_arg = 0;
                }
            }
        }

        /* Handle "cancel" */
        if (!command_arg) {
            energy_use = 0;
            msg_print(NULL);
            return;
        }
    }

    /* Stop searching */
    search_off();

    /* Save the rest code */
    p_ptr->rest = command_arg;

    /* Display the starting rest count */
    p_ptr->redraw |= PR_STATE;

    /* Handle stuff */
    handle_stuff(TRUE);

    /* Describe running */
    prt("Press any key to stop resting...", 0, 0);

    /* Refresh */
    Term_fresh();
}



/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling()
{
    /* Free move */
    energy_use = 0;

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
 * Give an object a textual inscription
 */
void inscribe(inven_type *i_ptr, cptr str)
{
    int i;

    /* Add the desired comment */
    for (i = 0; str[i] && (i < INSCRIP_SIZE - 1); ++i) {
        i_ptr->inscrip[i] = str[i];
    }

    /* Hack -- zero out the extra bytes */
    for ( ; i < INSCRIP_SIZE - 1; i++) {
        i_ptr->inscrip[i] = '\0';
    }

    /* Always terminate the string */
    i_ptr->inscrip[i] = '\0';
}


/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(void)
{
    int   item;
    inven_type *i_ptr;


    /* Free move */
    energy_use = 0;

    /* Require some objects */
    if (!inven_ctr && !equip_ctr) {
        msg_print("You are not carrying anything.");
        return;
    }


    /* Require a choice */
    if (!get_item(&item, "Unscribe which item? ", 0, INVEN_TOTAL-1, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Get the item */
    i_ptr = &inventory[item];

    /* Prompt for an inscription */
    if (i_ptr->inscrip[0]) {
        inscribe(i_ptr, "");
        msg_print("Inscription removed.");
    }
    else {
        msg_print("That item had no inscription to remove.");
    }


    /* Combine the pack */
    combine_pack();
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(void)
{
    int			item;

    inven_type		*i_ptr;

    char		out_val[160];
    char		tmp_str[160];


    /* Free move */
    energy_use = 0;

    /* Require some objects */
    if (!inven_ctr && !equip_ctr) {
        msg_print("You are not carrying anything.");
        return;
    }


    /* Require a choice */
    if (!get_item(&item, "Inscribe which item? ", 0, INVEN_TOTAL-1, FALSE)) return;

    /* Cancel auto-see */
    command_see = FALSE;


    /* Get the item */
    i_ptr = &inventory[item];

    /* Describe the activity */
    objdes(tmp_str, i_ptr, TRUE);
    (void)sprintf(out_val, "Inscribing %s.", tmp_str);
    msg_print(out_val);
    msg_print(NULL);

    /* Prompt for an inscription */
    prt("Inscription: ", 0, 0);

    /* Get a new inscription and apply it */
    strcpy(out_val, i_ptr->inscrip);
    if (askfor_aux(out_val, INSCRIP_SIZE - 1)) inscribe(i_ptr, out_val);


    /* Combine the pack */
    combine_pack();
}



/*
 * Print out the artifacts seen.
 * This can be used to notice "missed" artifacts.
 *
 * XXX Perhaps this routine induces a blank final screen.
 */
void do_cmd_check_artifacts(void)
{
    int i, j, k, t;

    char out_val[256];


    /* Free turn */
    energy_use = 0;


#ifndef ALLOW_CHECK_ARTIFACTS
    if (!wizard) {
        msg_print("That command was not compiled.");
        return;
    }
#endif

    /* Hack -- no checking in the dungeon */
    if (dun_level && !wizard) {
        msg_print("You need to be in town to check artifacts!");
        return;
    }


    /* Save the screen */
    save_screen();

    /* Use column 15 */
    j = 15;

    /* Erase some lines */
    for (i = 1; i < 23; i++) prt("", i, j - 2);

    /* Start in line 1 */
    i = 1;

    /* Title the screen */
    prt("Artifacts Seen:", i++, j + 5);

    /* Scan the artifacts */
    for (k = 0; k < ART_MAX; k++) {

        /* Hack -- Skip "illegal" artifacts */
        if (!v_list[k].name) continue;

        /* Has that artifact been created? */
        if (v_list[k].cur_num) {

            int z;
            char base_name[80];

            /* Paranoia */
            strcpy(base_name, "Unknown Artifact");

            /* Hack -- Track down the "type" name */
            for (z = 0; z < MAX_K_IDX; z++) {

                /* Acquire the correct base type */
                if ((k_list[z].tval == v_list[k].tval) &&
                    (k_list[z].sval == v_list[k].sval)) {

                    inven_type forge;

                    /* Create the artifact */
                    invcopy(&forge, z);
                    forge.name1 = k;

                    /* Describe the artifact */
                    objdes_store(base_name, &forge, FALSE);

                    break;
                }
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
    restore_screen();
}


/*
 * Display the "status" of uniques
 *
 * XXX XXX This routine may induce a blank final screen.
 */
void do_cmd_check_uniques()
{
    int		i, j, k, t;

    char	msg[160];


    energy_use = 0;

#ifndef ALLOW_CHECK_UNIQUES
    if (!wizard) {
        msg_print("That command was not compiled.");
        return;
    }
#endif


    save_screen();

    j = 15;

    for (i = 1; i < 23; i++) prt("", i, j - 2);

    i = 1;
    prt("Uniques:", i++, j + 5);

    /* Note -- skip the ghost */
    for (k = 1; k < MAX_R_IDX-1; k++) {

        monster_race *r_ptr = &r_list[k];
        monster_lore *l_ptr = &l_list[k];

        /* Only print Uniques */
        if (r_ptr->rflags1 & RF1_UNIQUE) {

            bool dead = (l_ptr->max_num == 0);

            /* Only display "known" uniques */
            if (dead || cheat_know || l_ptr->sights) {

                /* Print a message */
                sprintf(msg, "%s is %s.", r_ptr->name,
                        dead ? "dead" : "alive");
                prt(msg, i++, j);

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
    restore_screen();
}



