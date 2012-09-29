/* File: moria4.c */

/* Purpose: more low level code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__
static void area_affect(int,int,int);
static int see_wall(int,int,int);
static int see_nothing(int,int,int);
#endif
#endif





/*** Targetting Code ***/


#ifdef TARGET

/*
 * Be sure that the target_row & target_col vars are correct
 * Also be sure that a targetted creature, if any, is "legal".
 */
void target_update()
{
    /* This bit of targetting code taken from Morgul -CFT */
    /* If we are in targetting mode, with a creature target, */
    /* make the targetted row and column match the creature's. */
    /* Note that with ESP, there is no reason to have ESP */
    /* All that matters is that a "lock" can be acheived */
    /* Thus, all target locations are "valid".  This is not */
    /* so bad, since "aim at target" must be explicitly chosen */

    /* Is there a target monster? */
    if (target_mode && (target_mon > 0)) {

	monster_type *m_ptr = &m_list[target_mon];

	/* Assume invalid target */
	target_mode = FALSE;

	/* Monsters MUST be visible */
	if (m_ptr->ml) {
	    target_mode = TRUE;
	    target_row = m_ptr->fy;
	    target_col = m_ptr->fx;
	}
    }
}

#endif


/*
 * This targetting code stolen from Morgul
 * Returns TRUE if the given position is the target. -CDW 
 */
int target_at(int row,int col)
{

#ifdef TARGET

    /* Update the target */
    target_update();

    /* Compare the locations */
    if (target_mode && (row==target_row) && (col==target_col)) {
	return (TRUE);
    }

#endif

    /* Assume target is not at (row,col) */
    return (FALSE);
}


/*
 * This targetting code stolen from Morgul -CFT
 * Gotta be target mode and either (valid monster and line of sight
 * to monster) or (not valid monster and line of sight to position).  CDW
 * Also, for monster targetting, monster must be lit!  Otherwise player can
 * "lock phasers" on an invis monster while a potion of see inv lasts,
 * and then continue to hit it when the see inv goes away.  Also,
 * targetting mode shouldn't help the player shoot a monster in a
 * dark room.  If he can't see it, he shouldn't be able to aim... -CFT
 */

/*
 * Simple query -- is the "target" okay to use?
 * Obviously, if target mode is disabled, it is not.
 */
int target_okay()
{

#ifdef TARGET

    /* Update the target */
    target_update();

    /* Is the target still okay? */
    if (target_mode) return (TRUE);

#endif

    /* The "target" is invalid */
    return (FALSE);
}






/*
 * This targetting code stolen from Morgul -CFT
 * Targetting routine 					CDW
 * Need to teach it to be smart enough to save/restore screen
 * This will let it be used as a sub-prompt in "get_a_dir();" 
 */
int target_set()
{

#ifdef TARGET

    int row, col;
    int m_idx,exit_1;
    char query;
    vtype desc;

    exit_1 = FALSE;

    /* Go ahead and turn off target mode */
    target_mode = FALSE;

    /* Check monsters first */
    for (m_idx = 0; (m_idx < m_max) && (!exit_1); m_idx++) {

	monster_type *m_ptr = &m_list[m_idx];

	/* Ignore "unseen" monsters */
	if (!m_ptr->ml || !m_ptr->los) continue;

	/* Access monster location */
	row = m_ptr->fy;
	col = m_ptr->fx;


	if (use_recall_win) {
	    /* Describe */
	    sprintf(desc, "%s [(t)arget] [(l)ocation] [ESC quits]",
		    r_list[m_list[m_idx].r_idx].name);
	    prt(desc,0,0);
	    move_cursor_relative(row,col);

	    /* Recall, get a command */
	    roff_recall(m_list[m_idx].r_idx);
	    query = inkey();
	}

	else {
	
	    /* Describe, prompt for recall */
	    sprintf(desc, "%s [(t)arget] [(l)ocation] [(r)ecall] [ESC quits]",
		    r_list[m_list[m_idx].r_idx].name);
	    prt(desc,0,0);
	    move_cursor_relative(row,col);

	    /* Get a command, processing recall requests */
	    query = inkey();
	    while ((query == 'r') || (query == 'R')) {

	        save_screen();
	        query = roff_recall(m_list[m_idx].r_idx);
	        restore_screen();

	        /* XXX This is done by "good" restore_screen() */
	        move_cursor_relative(row, col);
	        query = inkey();
	    }
	}


	/* Analyze (non "recall") command */
	switch (query) {

	    case ESCAPE:
		return (FALSE);

	    case 'T': case 't':
	    case '5': case '0': case '.':
		target_mode = TRUE;
		target_mon  = m_idx;
		target_row  = row;
		target_col  = col;
		return (TRUE);

	    case 'L': case 'l':
		exit_1 = TRUE;
	}
    }


    /* Now try a location */
    prt("Use cursor to designate target. [(t)arget]",0,0);

    /* Start on the player */
    row = char_row;
    col = char_col;

    /* Query until done */
    while (TRUE) {

	/* Light up the current location */
	move_cursor_relative(row, col);

	/* Get a command, and convert it to standard form */
	query = inkey();
	if (rogue_like_commands) {
	    switch (query) {
		case ESCAPE: break;
		case 'Q': case 'q': query = ESCAPE; break;
		case 'T': case 't': query = '0'; break;
		case '0': case '5': query = '0'; break;
		case '.': query = '0'; break;
		case 'B': case 'b': query = '1'; break;
		case 'J': case 'j': query = '2'; break;
		case 'N': case 'n': query = '3'; break;
		case 'H': case 'h': query = '4'; break;
		case 'L': case 'l': query = '6'; break;
		case 'Y': case 'y': query = '7'; break;
		case 'K': case 'k': query = '8'; break;
		case 'U': case 'u': query = '9'; break;
		default: query = ' '; break;
	    }
	}
	else {
	    switch (query) {
		case ESCAPE: break;
		case 'Q': case 'q': query = ESCAPE; break;
		case '5': case '0': query = '0'; break;
		case '.': query = '0'; break;
		case 'T': case 't': query = '0'; break;
		case '1': case '2': case '3': case '4': break;
		case '6': case '7': case '8': case '9': break;
		default: query = ' '; break;
	    }
	}

	/* Analyze the command */
	switch (query) {

	    case ESCAPE:
		return (FALSE);

	    case '0':
		target_mode = TRUE;
		target_mon  = 0;
		target_row  = row;
		target_col  = col;
		return (TRUE);

	    case '1':
		col--;
	    case '2':
		row++;
		break;
	    case '3':
		row++;
	    case '6':
		col++;
		break;
	    case '7':
		row--;
	    case '4':
		col--;
		break;
	    case '9':
		col++;
	    case '8':
		row--;
		break;
	    default:
		break;
	}

	/* Verify column */
	if ((col>=cur_width-1) || (col>panel_col_max)) col--;
	else if ((col<=0) || (col<panel_col_min)) col++;

	/* Verify row */
	if ((row>=cur_height-1) || (row>panel_row_max)) row--;
	else if ((row<=0) || (row<panel_row_min)) row++;
    }

#endif

    /* Assume no target */
    return (FALSE);
}



/*
 * A more "correct" movement calculator.
 */
void mmove2(int *y, int *x, int sourcey, int sourcex, int desty, int destx)
{
    int d_y, d_x, k, dist, max_dist, min_dist, shift;

    d_y = (*y < sourcey) ? sourcey - *y : *y - sourcey;
    d_x = (*x < sourcex) ? sourcex - *x : *x - sourcex;

    dist = (d_y > d_x) ? d_y : d_x;
    dist++;

    d_y = (desty < sourcey) ? sourcey - desty : desty - sourcey;
    d_x = (destx < sourcex) ? sourcex - destx : destx - sourcex;

    if (d_y > d_x) {
	max_dist = d_y;
	min_dist = d_x;
    }
    else {
	max_dist = d_x;
	min_dist = d_y;
    }

    for (k = 0, shift = max_dist >> 1; k < dist; k++, shift -= min_dist) {
	shift = (shift > 0) ? shift : shift + max_dist;
    }
    
    if (shift < 0) shift = 0;

    if (d_y > d_x) {
	d_y = (desty < sourcey) ? *y - 1 : *y + 1;
	if (shift)
	    d_x = *x;
	else
	    d_x = (destx < sourcex) ? *x - 1 : *x + 1;
    }
    else {
	d_x = (destx < sourcex) ? *x - 1 : *x + 1;
	if (shift)
	    d_y = *y;
	else
	    d_y = (desty < sourcey) ? *y - 1 : *y + 1;
    }

    *y = d_y;
    *x = d_x;
}



/*
 * Given direction "dir", returns new row, column location -RAK-
 * targeting code stolen from Morgul -CFT
 * 'dir=0' moves toward target				    CDW  
 */
int mmove(int dir, int *y, int *x)
{
    register int new_row = 0, new_col = 0;
    int          boolflag;

    switch (dir) {

#ifdef TARGET

      case 0:

	target_update();

	new_row = *y;
	new_col = *x;
	mmove2(&new_row, &new_col,
	       char_row, char_col,
	       target_row, target_col);

	break;

#endif /* TARGET */

      case 1:
	new_row = *y + 1;
	new_col = *x - 1;
	break;
      case 2:
	new_row = *y + 1;
	new_col = *x;
	break;
      case 3:
	new_row = *y + 1;
	new_col = *x + 1;
	break;
      case 4:
	new_row = *y;
	new_col = *x - 1;
	break;
      case 5:
	new_row = *y;
	new_col = *x;
	break;
      case 6:
	new_row = *y;
	new_col = *x + 1;
	break;
      case 7:
	new_row = *y - 1;
	new_col = *x - 1;
	break;
      case 8:
	new_row = *y - 1;
	new_col = *x;
	break;
      case 9:
	new_row = *y - 1;
	new_col = *x + 1;
	break;
    }
    boolflag = FALSE;
    if ((new_row >= 0) && (new_row < cur_height)
	&& (new_col >= 0) && (new_col < cur_width)) {
	*y = new_row;
	*x = new_col;
	boolflag = TRUE;
    }
    return (boolflag);
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

void
confuse_dir (dir, mode)
int *dir;
int mode;
{
    /* Check for confusion */
    if (p_ptr->confused > 0) {

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
 * Note: We change neither command_rep nor free_turn_flag
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

    /* Ask user for a direction */
    else {

	/* Start with a non-direction */
	*dir = -1;

	/* Default prompt */
	if (!prompt) {
	    if (!(mode & 0x10) || !(target_mode)) {
		sprintf(pbuf, "Direction (%sEscape to cancel)? ",
			(mode & 0x20) ? "'*' to choose a target, " : "");
	    }
	    else if (rogue_like_commands) {
		sprintf(pbuf, "Direction (%s%sEscape to cancel)? ",
			(mode & 0x10) ? "'.' for target, " : "",
			(mode & 0x20) ? "'*' to re-target, " : "");
	    }
	    else {
		sprintf(pbuf, "Direction (%s%sEscape to cancel)? ",
			(mode & 0x10) ? "'5' for target, " : "", 
			(mode & 0x20) ? "'*' to re-target, " : "");
	    }
	    prompt = pbuf;
	}
	
	
	/* Ask until satisfied */    
	while (1) {

	    /* Get a command (or Escape) */
	    if (!get_com (prompt, &command)) command = ESCAPE;

	    /* Escape yields "cancel" */
	    if (command == ESCAPE) return (FALSE);

	    /* In THIS function, we use numbers as the default */
	    if (rogue_like_commands) {
		switch (command) {
		    case 'H': case 'h': command = '4'; break;
		    case 'Y': case 'y': command = '7'; break;
		    case 'K': case 'k': command = '8'; break;
		    case 'U': case 'u': command = '9'; break;
		    case 'L': case 'l': command = '6'; break;
		    case 'N': case 'n': command = '3'; break;
		    case 'J': case 'j': command = '2'; break;
		    case 'B': case 'b': command = '1'; break;
		    case '.': command = '5'; break;
		    case 'T': case 't': command = '0'; break;
		    case '0': command = '0'; break;
		    case '*': command = '*'; break;
		    case '?': command = '?'; break;
		    default: command = ' '; break;
		}
	    }
	    else {
		switch (command) {
		    case '?':
		    case '*':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '6':
		    case '7':
		    case '8':
		    case '9': break;
		    case '5': break;
		    case '0': command = '0'; break;
		    case 'T': case 't': command = '0'; break;
		    default: command = ' '; break;
		}
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
	    else if (command == '?') {
		/* XXX No help yet */
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

    /* Hack -- allow commands to be careless */
    free_turn_flag = TRUE;

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
    free_turn_flag = TRUE;

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

    p_ptr->searching = TRUE;
    
    change_speed(1);
    p_ptr->food_digested++;

    /* Visual feedback */
    p_ptr->status |= PY_SEARCH;
    prt_state();
    prt_speed();
}

void search_off(void)
{
    if (!p_ptr->searching) return;

    p_ptr->searching = FALSE;
    
    change_speed(-1);
    p_ptr->food_digested--;

    /* Visual feedback */
    p_ptr->status &= ~PY_SEARCH;
    prt_state();
    prt_speed();
}


/*
 * Something happens to disturb the player.		-CJS-
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is disabled.
 *
 * All disturbances cancel inventory, and "repeated" commands.
 */
void disturb(int stop_search, int light_change)
{
    /* Always disturb the inventory commands */
    screen_change = TRUE;

    /* Note that "light_change" is disabled */
    if (light_change) message("XXX Light change", 0);
    
    /* Always cancel repeated commands */
    if (command_rep) command_rep = 0;

    /* Always cancel Rest */
    if (p_ptr->rest) rest_off();

    /* Hack -- Cancel Search Mode if requested */
    if (stop_search) search_off();

    /* If running, cancel it and fix lights */
    if (find_flag) end_find();

    /* Hack -- center the cursor */
    move_cursor_relative(char_row, char_col);

    /* Hack -- Flush the output */
    Term_fresh();
}


/*
 * Searches for hidden things.			-RAK-	
 */
void search(int y, int x, int chance)
{
    register int           i, j;
    register cave_type    *c_ptr;
    register inven_type   *i_ptr;
    bigvtype               tmp_str, tmp_str2;

    if ((p_ptr->blind > 0) || no_lite()) chance = chance / 10;
    if (p_ptr->confused > 0) chance = chance / 10;
    if (p_ptr->image > 0) chance = chance / 10;

    /* Search the nearby grids, which are always in bounds */
    for (i = (y - 1); i <= (y + 1); i++) {
	for (j = (x - 1); j <= (x + 1); j++) {

	    /* Sometimes, notice things */
	    if (randint(100) < chance) {

		c_ptr = &cave[i][j];
		i_ptr = &i_list[c_ptr->i_idx];

		/* Nothing there */
		if (c_ptr->i_idx == 0) {
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
		    if ((i_ptr->flags2 & CH2_TRAP_MASK) && !known2_p(i_ptr)) {
			msg_print("You have discovered a trap on the chest!");
			known2(i_ptr);
			disturb(0, 0);
		    }
		}
	    }
	}
    }
}



void rest_off()
{
    p_ptr->rest = 0;
    
    /* Hack -- update the state */
    p_ptr->status &= ~PY_REST;
    prt_state();

    /* flush last message, or delete "press any key" message */
    msg_print(NULL);

    /* Restore the digestion speed */
    p_ptr->food_digested++;
}





/*
 * Player wants to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(int y, int x, int pickup)
{
    register int         locn, i;
    bigvtype             out_val, tmp_str;
    register cave_type  *c_ptr;
    register inven_type *i_ptr;

    c_ptr = &cave[y][x];
    i_ptr = &i_list[c_ptr->i_idx];
    i = i_ptr->tval;

    /* Disturb the player */
    disturb(0, 0);


    /* Pick up gold */
    if (i_ptr->tval == TV_GOLD) {
	p_ptr->au += i_ptr->cost;
	objdes(tmp_str, i_ptr, TRUE);
	(void)sprintf(out_val,
		      "You have found %ld gold pieces worth of %s.",
		      (long)i_ptr->cost, tmp_str);
	prt_gold();
	delete_object(y, x);
	msg_print(out_val);
    }

    /* Can it be picked up? */
    else if (i <= TV_MAX_PICK_UP) {

	/* Can we pick it up? */
	if (pickup && inven_check_num(i_ptr)) {

	    /* Okay,  pick it up  */
	    if (carry_query_flag) {	
		objdes(tmp_str, i_ptr, TRUE);
		(void)sprintf(out_val, "Pick up %s? ", tmp_str);
		pickup = get_check(out_val);
	    }

	    /* Check to see if it will change the players speed. */
	    if (pickup && !inven_check_weight(i_ptr)) {
		objdes(tmp_str, i_ptr, TRUE);
		(void)sprintf(out_val,
			      "Exceed your weight limit to pick up %s? ",
			      tmp_str);
		pickup = get_check(out_val);
	    }

	    /* Attempt to pick up an object. */
	    if (pickup) {
		locn = inven_carry(i_ptr);
		objdes(tmp_str, &inventory[locn], TRUE);
		(void)sprintf(out_val, "You have %s. (%c)",
			      tmp_str, index_to_label(locn));
		msg_print(out_val);
		delete_object(y, x);
	    }
	}

	/* only if was trying to pick it up. -CFT */
	else if (pickup) {
	    objdes(tmp_str, i_ptr, TRUE);
	    (void)sprintf(out_val, "You can't carry %s.", tmp_str);
	    msg_print(out_val);
	}
    }
}



/*
 * Moves player from one space to another. -RAK-
 * Note: Confusion is NOT checked by this routine
 *
 * This function must be sure to check the validity of "continuing"
 * to repeat a "walk" or "walk without pickup" command, and reset
 * command_rep accordingly.
 *
 * Note that "moving" into a wall is a free move, and will NOT hit any monster
 * which is "hiding" in the walls.  The player must tunnel into the wall.
 * Otherwise, moving into a wall would have to always take a turn.
 * In fact, moving into a wall will not hit ANY monster in the wall.
 * After all, the monster has the entire wall as protection!
 *
 * XXX Someone needs to verify that teleporting onto a rubble trap either will
 * never happen or that it will not cause the player to be trapped in the rubble.
 * I think the "teleport" code is nice enough to require a "clean" destination.
 *
 * The "rubble recovery" routine needs to be moved into the "rubble_trap" code.
 *
 * Note that "moving" into the same square is a "good" way to "update" the screen.
 */
void move_player(int dir, int do_pickup)
{
    int                 y, x;
    register cave_type *c_ptr;
    register inven_type	*i_ptr;

    /* Remember if the player was running */
    bool was_running = find_flag;
    
    /* Save info for dealing with traps and such */
    int old_row = char_row;
    int old_col = char_col;

    /* May we continue walking? */
    int more = FALSE;


    /* Find the result of moving */
    y = char_row;
    x = char_col;
    (void)mmove(dir, &y, &x);

    /* Examine the destination */    
    c_ptr = &cave[y][x];

    /* Player can NEVER move into walls, rubble, closed doors */
    if (!floor_grid(y,x)) {

	/* Disturb the player */
	disturb(0, 0);

	/* Get the "object" if any */
	i_ptr = &i_list[c_ptr->i_idx];

	/* Hack -- allow "mapping" in the dark */
	if (!player_can_see(y, x)) {

	    /* Rubble */
	    if (i_ptr->tval == TV_RUBBLE) {
		msg_print("You feel some rubble blocking your way.");
		c_ptr->info |= CAVE_FM;
		lite_spot(y, x);
	    }

	    /* Closed door */
	    else if (i_ptr->tval == TV_CLOSED_DOOR) {
		msg_print("You feel a closed door blocking your way.");
		c_ptr->info |= CAVE_FM;
		lite_spot(y, x);
	    }

	    /* Wall (or secret door) */
	    else {
		msg_print("You feel a wall blocking your way.");
		c_ptr->info |= CAVE_FM;
		lite_spot(y, x);
	    }
	}
	
	/* Notice non-walls unless starting to "run" */
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
	free_turn_flag = TRUE;
    }

    /* Attacking a creature! */
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
	move_rec(char_row, char_col, y, x);

	/* Check for new panel (redraw map) */
	(void)get_panel(char_row, char_col, FALSE);

	/* Update the view/lite */
	update_view();
	update_lite();
	
	/* Update the monsters */
	update_monsters();
	

	/* Check the view */
	check_view();
	

	/* Check to see if he should stop running */
	if (find_flag) area_affect(dir, char_row, char_col);


	/* Check to see if he notices something  */
	/* "fos" may be negative if have good rings of searching */
	if ((p_ptr->fos <= 1) || (randint(p_ptr->fos) == 1)) {
	    search(char_row, char_col, p_ptr->srh);
	}

	/* Allow "search" mode to always get a "bonus" search */
	if (p_ptr->searching) {
	    search(char_row, char_col, p_ptr->srh);
	}


	/* Nothing interesting here, keep walking */
	if (c_ptr->i_idx == 0) {

	    /* Allow repeated walk to continue */
	    more = TRUE;
	}

	/* An object is beneath him. */
	else {

	    /* Get the object */            
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Pre-handle open doors and stairs */
	    if ((i_ptr->tval == TV_UP_STAIR) ||
		(i_ptr->tval == TV_DOWN_STAIR)) {
		/* Nothing */
	    }

	    /* Pre-handle open doors */
	    else if (i_ptr->tval == TV_OPEN_DOOR) {
		/* Hack -- allow running through doors */
		if (find_ignore_doors) more = TRUE;
	    }

	    /* Set off a trap */
	    else if ((i_ptr->tval == TV_VIS_TRAP) ||
		     (i_ptr->tval == TV_INVIS_TRAP)) {
		disturb(0, 0);
		hit_trap(y, x);
	    }

	    /* Enter a store */
	    else if (i_ptr->tval == TV_STORE_DOOR) {
		 disturb(0, 0);
		 enter_store(i_ptr->sval - 1);
	    }

	    /* Note that we only carry things that can be "carried" */
	    else if (i_ptr->tval == TV_GOLD || !prompt_carry_flag) {
		carry(char_row, char_col, do_pickup);
	    }

	    /* Inform the user he could have carried it */
	    else if (prompt_carry_flag) {
		bigvtype            tmp_str, tmp2_str;
		objdes(tmp_str, i_ptr, TRUE);
		sprintf(tmp2_str, "You see %s.", tmp_str);
		msg_print(tmp2_str);
	    }


	    /* Get the object */            
	    i_ptr = &i_list[c_ptr->i_idx];

	    /* Hack -- if stepped on falling rock trap, the space will */
	    /* now contain rubble, so step back into a clear area */
	    /* XXX There may not BE a "clear area", I suppose */

	    /* Back away from rubble.  Do NOT set off recursive traps */
	    if (i_ptr->tval == TV_RUBBLE) {

		/* Move back to the old location */
		move_rec(char_row, char_col, old_row, old_col);

		/* Check for new panel (redraw map) */
		(void)get_panel(char_row, char_col, FALSE);

		/* Update the view/lite */
		update_view();
		update_lite();
		
		/* Update the monsters */
		update_monsters();
		

		/* Paranoia -- check the view */
		check_view();
	    }
	}
    }


    /* Perhaps Cancel repeated walking */
    if (!more) command_rep = 0;
}






/* The running algorithm:			-CJS-

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

   To initialize these conditions is the task of . If
   moving from the square marked @ to the square marked . (in the
   two diagrams below), then two adjacent sqares on the left and
   the right (L and R) are considered. If either one is seen to
   be closed, then that side is considered to be closed. If both
   sides are closed, then it is an enclosed (corridor) run.

	 LL		L
	@.	       L.R
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
   is set, otherwise must stop because it is not a corner. */

/*
 * The cycle lists the directions in anticlockwise order, for	-CJS- over
 * two complete cycles. The chome array maps a direction on to its position
 * in the cycle. 
 */
static int cycle[] = {1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1};
static int chome[] = {-1, 8, 9, 10, 7, -1, 11, 6, 5, 4};
static int find_openarea, find_breakright, find_breakleft, find_prevdir;


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

    /* Hack -- prevent infinite running */
    if (find_flag++ > 100) {
	msg_print("You stop running to catch your breath.");
	end_find();
    }

    /* Take a step (pick up if necessary) */
    else {

	/* Take account of partial confusion */
	dir = command_dir;
	confuse_dir(&dir, 0x02);

	/* Cancel the running if we have done enough */
	if (command_arg-- == 0) end_find();

	/* Cancel the running if we mis-step */
	if (dir != command_dir) end_find();

	/* Move the player, picking up as you go */
	move_player(dir, TRUE);
    }
}


/*
 * Initialize the running algorithm, do NOT take any steps.
 *
 * Note that we use "command_arg" as a "limit" on running time.
 * If "command_arg" is zero (as usual) then impose no limit.
 *
 * The player is not allowed to run with a full light pattern.
 * This reduces the number of things that can cancel running.
 */
void find_init()
{
    int          dir, row, col, deepleft, deepright;
    register int i, shortleft, shortright;


    /* Start running */
    find_flag = 1;
    

    /* Extract the desired direction */
    dir = command_dir;

    /* Find the destination grid */
    row = char_row;
    col = char_col;
    (void)mmove(dir, &row, &col);

    /* XXX Un-indent */
    if (TRUE) {
	find_flag = 1;
	find_breakright = find_breakleft = FALSE;
	find_prevdir = dir;
	if (p_ptr->blind < 1) {
	    i = chome[dir];
	    deepleft = deepright = FALSE;
	    shortright = shortleft = FALSE;
	    if (see_wall(cycle[i + 1], char_row, char_col)) {
		find_breakleft = TRUE;
		shortleft = TRUE;
	    }
	    else if (see_wall(cycle[i + 1], row, col)) {
		find_breakleft = TRUE;
		deepleft = TRUE;
	    }
	    if (see_wall(cycle[i - 1], char_row, char_col)) {
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
		if (dir & 1) {
		    if (deepleft && !deepright)
			find_prevdir = cycle[i - 1];
		    else if (deepright && !deepleft)
			find_prevdir = cycle[i + 1];
		}
	    /* else if there is a wall two spaces ahead and seem to be in a
	     * corridor, then force a turn into the side corridor, must be
	     * moving straight into a corridor here 
	     */
		else if (see_wall(cycle[i], row, col)) {
		    if (shortleft && !shortright)
			find_prevdir = cycle[i - 2];
		    else if (shortright && !shortleft)
			find_prevdir = cycle[i + 2];
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
 */
void end_find()
{
    /* Were we running? */
    if (find_flag) {

	bool f_lite = (view_yellow_lite && view_yellow_fast);
	bool f_view = (view_bright_lite && view_bright_fast);
	
	/* XXX Hack XXX */
	if (f_lite) forget_lite();
	if (f_view) forget_view();
	
	/* Cancel the running */
	find_flag = 0;

	/* Redraw the player (only needed sometimes) */
	lite_spot(char_row, char_col);

	/* Hack -- Update the view/lite */
	if (view_reduce_view || f_view) update_view();
	if (view_reduce_lite || f_lite) update_view();

	/* Update the monsters */
	update_monsters();

	/* Hack -- Check the view */
	check_view();
    }
}



/*
 * Do we see a wall? Used in running.		-CJS- 
 */
static int see_wall(int dir, int y, int x)
{
    int m;
    int8u a;
    char c;

    /* Attempt to move from (x,y) along dir */
    if (!mmove(dir, &y, &x)) return TRUE;

    /* Check the map (using new x and y) */
    map_info(y, x, &m, &a, &c);

    /* Does it LOOK like a wall? */
    if (c == '#' || c == '%') return TRUE;

    /* Default */
    return (FALSE);
}

/*
 * Do we see anything? Used in running.		-CJS- 
 * Hopefully this is only used for "adjacent" grids,
 * since otherwise, the "torch radius" matters.
 */
static int see_nothing(int dir, int y, int x)
{
    int m;
    int8u a;
    char c;

    /* Attempt to move from (x,y) along dir */
    if (!mmove(dir, &y, &x)) return FALSE;

    /* Check the map (using new x and y) */
    map_info(y, x, &m, &a, &c);

    /* Nothing there */
    if (c == ' ') return TRUE;

    /* Default */
    return FALSE;
}


/*
 * Determine the next direction for a run, or if we should stop.  -CJS- 
 */
void area_affect(int dir, int y, int x)
{
    int                  newdir = 0, t, inv, check_dir = 0, row, col;
    register int         i, max, option, option2;

    register cave_type *c_ptr;

    /* We must be able to see... */
    if (p_ptr->blind < 1) {

	option = 0;
	option2 = 0;
	dir = find_prevdir;
	max = (dir & 1) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++) {

	    newdir = cycle[chome[dir] + i];
	    row = y;
	    col = x;
	    mmove(newdir, &row, &col);

	    c_ptr = &cave[row][col];


	    /* Hack -- notice visible monsters */
	    if ((c_ptr->m_idx > 1) && (m_list[c_ptr->m_idx].ml)) {
		end_find();
		return;
	    }


	    /* Assume the new grid cannot be seen */
	    inv = TRUE;

	    /* Can we "see" (or "remember") the adjacent grid? */
	    if (test_lite(row, col)) {

		/* Most (visible) objects stop the running */
		if (c_ptr->i_idx) {

		    /* Examine the object */
		    t = i_list[c_ptr->i_idx].tval;
		    if ((t != TV_INVIS_TRAP) &&
			(t != TV_SECRET_DOOR) &&
			(t != TV_UP_STAIR || !find_ignore_stairs) &&
			(t != TV_DOWN_STAIR || !find_ignore_stairs) &&
			(t != TV_OPEN_DOOR || !find_ignore_doors)) {

			end_find();
			return;
		    }
		}

		/* The grid is "visible" */
		inv = FALSE;
	    }


	    /* If cannot see the grid, assume it is clear */
	    if (inv || floor_grid(row, col)) {

		/* Certain somethings */
		if (find_openarea) {
		    if (i < 0) {
			if (find_breakright) {
			    end_find();
			    return;
			}
		    }
		    else if (i > 0) {
			if (find_breakleft) {
			    end_find();
			    return;
			}
		    }
		}

		/* The first new direction. */
		else if (option == 0) {
		    option = newdir;
		}

		/* Three new directions. Stop running. */
		else if (option2 != 0) {
		    end_find();             
		    return;
		}

		/* If not adjacent to prev, STOP */
		else if (option != cycle[chome[dir] + i - 1]) {
		    end_find();
		    return;
		}

		/* Two adjacent choices. Make option2 the diagonal, */
		/* and remember the other diagonal adjacent to the  */
		/* first option. */
		else {
		    if ((newdir & 1) == 1) {
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
			end_find();
			return;
		    }
		    find_breakright = TRUE;
		}
		else if (i > 0) {
		    if (find_breakright) {
			end_find();
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

		row = y;
		col = x;
		(void)mmove(option, &row, &col);

		/* Don't see that it is closed off.  This could be a */
		/* potential corner or an intersection.  */
		if (!see_wall(option, row, col) ||
		    !see_wall(check_dir, row, col)) {

		    /* Can not see anything ahead and in the direction we */
		    /* are  turning, assume that it is a potential corner. */
		    if (find_examine && see_nothing(option, row, col) &&
			see_nothing(option2, row, col)) {
			command_dir = option;
			find_prevdir = option2;
		    }

		    /* STOP: we are next to an intersection or a room */
		    else {
			end_find();
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








