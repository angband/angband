/* File: cmd4.c */

/* Purpose: more low level code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"




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

    bool	offset = FALSE;

    char	query;

    char	desc[160];


    /* Go ahead and turn off target mode */
    target_who = 0;

    /* Turn off health tracking */
    health_track(0);


    /* Check monsters first */
    for (m_idx = MIN_M_IDX; m_idx < m_max; m_idx++) {

        monster_type *m_ptr = &m_list[m_idx];
        monster_race *r_ptr = &r_list[m_ptr->r_idx];

        /* Paranoia -- Skip "dead" monsters */
        if (m_ptr->dead) continue;

        /* Ignore "unreasonable" monsters */
        if (!target_able(m_idx)) continue;

        /* Access monster location */
        row = m_ptr->fy;
        col = m_ptr->fx;

        /* Hack -- Track that monster */
        health_track(m_idx);

        /* Auto-recall */
        if (use_recall_win && term_recall) {

            /* Describe the monster */
            roff_recall(m_ptr->r_idx);

            /* Describe */
            sprintf(desc,
                    "%s [(t)arget, (o)ffset, (p)osition, or (q)uit]",
                    r_ptr->name);
            prt(desc,0,0);

            /* Get a command */
            move_cursor_relative(row,col);
            query = inkey();
        }

        /* Optional recall */
        else {

            /* Describe, prompt for recall */
            sprintf(desc,
                    "%s [(t)arget, (o)ffset, (p)osition, (r)ecall, or (q)uit]",
                    r_ptr->name);
            prt(desc,0,0);

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
                target_who = m_idx;
                target_row = row;
                target_col = col;
                return (TRUE);

            case 'O': case 'o':
                offset = TRUE;

            case 'P': case 'p':
                m_idx = m_max;
        }
    }


    /* Now try a location */
    prt("Use cursor to designate target. [(t)arget]",0,0);

    /* Usually start on the player */
    if (!offset) {
        row = py;
        col = px;
    }

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
    char	pbuf[80];

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
                strcpy(pbuf, prompt);
            }
            else if (!(mode & 0x10) || !target_okay()) {
                sprintf(pbuf, "Direction (%sEscape to cancel)? ",
                        (mode & 0x20) ? "'*' to choose a target, " : "");
            }
            else {
                sprintf(pbuf, "Direction (%s%sEscape to cancel)? ",
                        (mode & 0x10) ? "'5' for target, " : "",
                        (mode & 0x20) ? "'*' to re-target, " : "");
            }

            /* Get a command (or Cancel) */
            if (!get_com(pbuf, &command)) return (FALSE);

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

    /* XXX Mega-Hack -- allow commands to be careless */
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
    /* Disturb "auto-commands" */
    screen_change = TRUE;

    /* Cancel repeated commands */
    if (command_rep) command_rep = 0;

    /* Cancel resting */
    if (p_ptr->rest) rest_off();

    /* Cancal running if possible */
    if (find_flag) end_find();

    /* Cancel searching if requested */
    if (stop_search) search_off();

    /* Hack -- redraw the "state" later */
    p_ptr->redraw |= PR_STATE;

    /* Hack -- sometimes flush the input */
    if (flush_disturb) flush();

    /* Hack -- handle stuff if requested */
    if (flush_output) handle_stuff(TRUE);
    
    /* Hack -- always hilite the player */
    move_cursor_relative(py, px);

    /* Hack -- flush output if requested */
    if (flush_output) Term_fresh();
}


/*
 * Searches for hidden things.			-RAK-	
 */
void search(int y, int x, int chance)
{
    int           i, j;

    cave_type    *c_ptr;
    inven_type   *i_ptr;

    char	tmp_str[160];
    char	tmp_str2[160];


    if (p_ptr->blind || no_lite()) chance = chance / 10;
    if (p_ptr->confused) chance = chance / 10;
    if (p_ptr->image) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (i = (y - 1); i <= (y + 1); i++) {
        for (j = (x - 1); j <= (x + 1); j++) {

            /* Sometimes, notice things */
            if (randint(100) < chance) {

                c_ptr = &cave[i][j];
                i_ptr = &i_list[c_ptr->i_idx];

                /* Nothing there */
                if (!(c_ptr->i_idx)) {
                    /* Nothing */
                }

                /* Invisible trap? */
                else if (i_ptr->tval == TV_INVIS_TRAP) {
                    objdes(tmp_str2, i_ptr, TRUE);
                    (void)sprintf(tmp_str, "You have found %s.", tmp_str2);
                    msg_print(tmp_str);
                    i_ptr->tval = TV_VIS_TRAP;
                    lite_spot(i, j);
                    disturb(0, 0);
                }

                /* Secret door?	*/
                else if (i_ptr->tval == TV_SECRET_DOOR) {
                    msg_print("You have found a secret door.");

                    /* Hack -- drop on top */
                    invcopy(i_ptr, OBJ_CLOSED_DOOR);

                    /* Place it in the dungeon */
                    i_ptr->iy = i;
                    i_ptr->ix = j;

                    /* Redraw the door */
                    lite_spot(i, j);

                    /* Notice it */
                    disturb(0, 0);
                }

                /* Chest?  Trapped?  Known? */
                else if (i_ptr->tval == TV_CHEST) {
                    if ((i_ptr->flags2 & CH2_TRAP_MASK) && !inven_known_p(i_ptr)) {
                        msg_print("You have discovered a trap on the chest!");
                        inven_known(i_ptr);
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
    p_ptr->redraw |= PR_STATE;

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
void carry(int y, int x, int pickup)
{
    cave_type  *c_ptr = &cave[y][x];
    inven_type *i_ptr = &i_list[c_ptr->i_idx];

    char	out_val[160];
    char	tmp_str[160];


    /* Hack -- nothing here to pick up */
    if (!(c_ptr->i_idx)) return;


    /* Describe the object */
    objdes(tmp_str, i_ptr, TRUE);

    /* Pick up gold */
    if (i_ptr->tval == TV_GOLD) {
        disturb(0, 0);
        p_ptr->au += i_ptr->cost;
        (void)sprintf(out_val,
                      "You have found %ld gold pieces worth of %s.",
                      (long)i_ptr->cost, tmp_str);
        msg_print(out_val);
        p_ptr->redraw |= PR_BLOCK;
        delete_object(y, x);
    }

    /* Can it be picked up? */
    else if (i_ptr->tval <= TV_MAX_PICK_UP) {

        /* Hack -- disturb */
        disturb(0, 0);

        /* Describe the object */
        if (!pickup) {
            sprintf(out_val, "You see %s.", tmp_str);
            msg_print(out_val);
        }

        /* Note that the pack is too full */
        else if (!inven_check_num(i_ptr)) {
            sprintf(out_val, "You have no room for %s.", tmp_str);
            msg_print(out_val);
        }

        /* Pick up the item (if requested and allowed) */
        else {

            int okay = TRUE;

            /* Hack -- query every item */
            if (carry_query_flag) {	
                sprintf(out_val, "Pick up %s? ", tmp_str);
                okay = get_check(out_val);
            }

            /* Attempt to pick up an object. */
            if (okay) {
                int locn = inven_carry(i_ptr);
                objdes(tmp_str, &inventory[locn], TRUE);
                sprintf(out_val, "You have %s. (%c)",
                        tmp_str, index_to_label(locn));
                msg_print(out_val);
                delete_object(y, x);
            }
        }
    }
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

    char		tmp[160];


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

    /* Roll for damage */
    dam = damroll(i_ptr->dd, i_ptr->ds);

    /* Examine the trap sub-val */
    switch (i_ptr->sval) {

      case SV_TRAP_PIT:
        msg_print("You fell into a pit!");
        if (p_ptr->ffall) {
            msg_print("You float gently to the bottom of the pit.");
        }
        else {
            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
        }
        break;

      case SV_TRAP_ARROW:
        if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
            msg_print("An arrow hits you.");
            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
        }
        else {
            msg_print("An arrow barely misses you.");
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
                    p_ptr->poisoned += randint(dam);
                }
            }

            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
        }
        break;

      case SV_TRAP_TRAP_DOOR:
        msg_print("You fell through a trap door!");
        if (p_ptr->ffall) {
            msg_print("You float gently down to the next level.");
        }
        else {
            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
        }
        msg_print(NULL);
        new_level_flag = TRUE;
        dun_level++;
        break;

      case SV_TRAP_GAS_SLEEP:
        if (p_ptr->paralysis == 0) {
            msg_print("A strange white mist surrounds you!");
            if (p_ptr->free_act) {
                msg_print("You are unaffected.");
            }
            else {
                msg_print("You fall asleep.");
                p_ptr->paralysis += rand_int(10) + 5;
            }
        }
        break;

      case SV_TRAP_LOOSE_ROCK:
        msg_print("Hmmm, there was something under this rock.");
        delete_object(py, px);
        place_object(py, px);
        break;

      case SV_TRAP_DART_STR:
        if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
            objdes(tmp, i_ptr, TRUE);
            if (!p_ptr->sustain_str) {
                msg_print("A small dart weakens you!");
                (void)dec_stat(A_STR, 10, FALSE);
            }
            else {
                msg_print("A small dart hits you.");
            }
            take_hit(dam, tmp);
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_TELEPORT:
        msg_print("You hit a teleport trap!");
        teleport_flag = TRUE;
        teleport_dist = 100;
        break;

      case SV_TRAP_FALLING_ROCK:

        /* Message and damage */
        msg_print("You are hit by falling rock.");
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

      case SV_TRAP_GAS_ACID:
        msg_print("A strange red gas surrounds you.");
        acid_dam(randint(8), "corrosion gas");
        break;

      case SV_TRAP_SUMMON:
        msg_print("You are enveloped in a cloud of smoke!");
        delete_object(py, px); /* Rune disappears.    */
        num = 2 + randint(3);
        for (i = 0; i < num; i++) {
            (void)summon_monster(py, px, dun_level + MON_SUMMON_ADJ);
        }
        break;

      case SV_TRAP_FIRE:
        msg_print("You are enveloped in flames!");
        fire_dam(dam, "a fire trap");
        break;

      case SV_TRAP_ACID:
        msg_print("You are splashed with acid!");
        acid_dam(dam, "an acid trap");
        break;

      case SV_TRAP_GAS_POISON:
        msg_print("A pungent green gas surrounds you!");
        poison_gas(dam, "a poison gas trap");
        break;

      case SV_TRAP_GAS_BLIND:
        msg_print("A black gas surrounds you!");
        if (!p_ptr->resist_blind) {
            p_ptr->blind += rand_int(50) + 50;
        }
        break;

      case SV_TRAP_GAS_CONFUSE:
        msg_print("A gas of scintillating colors surrounds you!");
        if ((!p_ptr->resist_conf) && (!p_ptr->resist_chaos)) {
            p_ptr->confused += rand_int(15) + 15;
        }
        break;

      case SV_TRAP_DART_SLOW:
        if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
            if (p_ptr->free_act) {
                msg_print("A small dart hits you!");
            }
            else {
                msg_print("A small dart hits you!");
                p_ptr->slow += rand_int(20) + 10;
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
        break;

      case SV_TRAP_DART_CON:
        if (test_hit(125, 0, 0, p_ptr->pac + p_ptr->ptoac, CLA_MISC_HIT)) {
            objdes(tmp, i_ptr, TRUE);
            take_hit(dam, tmp);
            if (!p_ptr->sustain_con) {
                (void)dec_stat(A_CON, 10, FALSE);
                msg_print("A small dart saps your health!");
            }
            else {
                msg_print("A small dart hits you.");
            }
        }
        else {
            msg_print("A small dart barely misses you.");
        }
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
        if (p_ptr->afraid < 1) {
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
        if (!(c_ptr->info & GRID_MARK) && !(c_ptr->info & GRID_LITE)) {

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
        if (p_ptr->afraid < 1) {
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

        /* Check to see if he notices something  */
        /* "fos" may be negative if have good rings of searching */
        if ((p_ptr->fos <= 1) || (0 == rand_int(p_ptr->fos))) {
            search(py, px, p_ptr->srh);
        }

        /* Allow "search" mode to always get a "bonus" search */
        if (p_ptr->searching) {
            search(py, px, p_ptr->srh);
        }


        /* Handle "objects" */
        if (c_ptr->i_idx) {

            /* Enter a store */
            if (i_ptr->tval == TV_STORE_DOOR) {
                disturb(0, 0);
                enter_store(i_ptr->sval - 1);
            }

            /* Set off a trap */
            else if ((i_ptr->tval == TV_VIS_TRAP) ||
                     (i_ptr->tval == TV_INVIS_TRAP)) {
                disturb(0, 0);
                hit_trap(old_row, old_col);
            }

            /* Pick up (or note) gold and objects */
            else {
                carry(py, px, do_pickup);
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


    /* We must be able to see... */
    if (p_ptr->blind < 1) {

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
 * Apply confusion, and if we become too confused, stop running
 *
 * Note that move_player() may modify command_dir via "area_affect"
 * Note that the "running" routines now use "command_dir" instead
 * of "find_direction"
 *
 * This routine had better not be called if find_flag is off.
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

    /* Check to see if he should stop running */
    if (find_flag) area_affect(dir, py, px);

    /* Hack -- run out of breath */
    if (find_flag && (--command_arg <= 0)) {
        msg_print("You stop running to catch your breath.");
        disturb(0,0);
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
        if (p_ptr->blind < 1) {

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
 * Stop running.  Hack -- fix the lights.
 *
 * My guess is that it is more efficient *not* to use either of
 * the "view_reduce_*" options since they induce an "extra" call
 * to "update_monsters()".  But this may not be important.
 *
 * If you are concerned about speed, turn off the "view_bright_lite"
 * option which makes everything very slow.  Also, at least right now,
 * the "choice window" is very inefficient.  So turn it off as well.
 * For *very* slow machines, turn off "view_yellow_lite" and turn on
 * the "do not print self when running" option.  This will allow a lot
 * of optimization when the player is running.  Oh, and turn off the
 * "use color" option if you are on a monochrome machine.  Or even if
 * you want to sacrifice color for speed.  And compile out the support
 * for the auto-rotation of the "multi-hued" stuff.  And if you turn
 * off the "flush output" options then fewer screen flushes are needed,
 * but the player may "miss" various transient visual phenomena.
 *
 * Remember that the three biggest processing tasks are updating the
 * screen (especially when color is involved), updating the monsters
 * when the player moves, and processing the monsters after the player
 * moves.  Calculating the view when the player moves is not really a
 * complex procedure, and appears to only be "slow" in the town.
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





