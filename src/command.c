/* File: command.c */ 

/* Purpose: process player commands */

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
static char original_commands(char);
static char roguelike_commands(char);
static int command_takes_rep(char);
static int command_takes_arg(char);
static void do_cmd_help(void);
static void do_cmd_messages(void);
static void do_cmd_browse(void);
static void do_cmd_go_up(void);
static void do_cmd_go_down(void);
static void do_cmd_refill(void);
static void do_cmd_target(void);
static void do_cmd_options(void);
static int do_std_command(void);
#endif
#endif




/*
 * Give the player some help
 * XXX These files are out of date
 */
static void do_cmd_help(void)
{	    
    /* Help is always free */
    free_turn_flag = TRUE;

    /* Dump a file */
    if (rogue_like_commands) {
	helpfile(ANGBAND_R_HELP);
    }
    else {
	helpfile(ANGBAND_O_HELP);
    }
}




/*
 * Examine a Book					-RAK-	
 */
static void do_cmd_browse(void)
{
    int32u               j1, j2, tmp;
    int                  i, k, item_val, flag;
    int                  spell_index[64];
    register inven_type *i_ptr;
    register spell_type *s_ptr;
    int                  first_spell;

    /* This command is free */
    free_turn_flag = TRUE;

    if (!find_range(TV_MAGIC_BOOK, TV_PRAYER_BOOK, &i, &k)) {
	msg_print("You are not carrying any books.");
	return;
    }

    if (p_ptr->blind > 0) {
	msg_print("You can't see to read your spell book!");
	return;
    }

    if (no_lite()) {
	msg_print("You have no light to read by.");
	return;
    }

    if (p_ptr->confused > 0) {
	msg_print("You are too confused.");
	return;
    }

    if (!get_item(&item_val, "Which Book?", i, k, 0)) {
	return;
    }

    flag = FALSE;

    i_ptr = &inventory[item_val];

    /* Check the language */
    if (class[p_ptr->pclass].spell == MAGE) {
	if (i_ptr->tval == TV_MAGIC_BOOK) flag = TRUE;
    }
    else if (class[p_ptr->pclass].spell == PRIEST) {
	if (i_ptr->tval == TV_PRAYER_BOOK) flag = TRUE;
    }

    if (!flag) {
	msg_print("You do not understand the language.");
	return;
    }

    i = 0;

    j1 = i_ptr->flags1;

    /* check which spell was first */
    tmp = j1;
    first_spell = bit_pos(&tmp);

    while (j1) {
	k = bit_pos(&j1);
	s_ptr = &magic_spell[p_ptr->pclass - 1][k];
	if (s_ptr->slevel < 99) {
	    spell_index[i] = k;
	    i++;
	}
    }

    j2 = i_ptr->flags2;

    /* if none from other set of flags */
    if (first_spell == -1) {
	tmp = j2;
	first_spell = 32 + bit_pos(&tmp);
    }

    while (j2) {
	k = bit_pos(&j2);
	s_ptr = &magic_spell[p_ptr->pclass - 1][k + 32];
	if (s_ptr->slevel < 99) {
	    spell_index[i] = (k + 32);
	    i++;
	}
    }

    /* Display the spells */
    save_screen();
    print_spells(spell_index, i, TRUE, first_spell);
    pause_line(0);
    restore_screen();
}




/*
 * Go up one level					-RAK-	
 */
static void do_cmd_go_up()
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = &cave[char_row][char_col];
    i_ptr = &i_list[c_ptr->i_idx];

    /* Verify stairs */
    if (i_ptr->tval != TV_UP_STAIR) {
	msg_print("I see no up staircase here.");
	free_turn_flag = TRUE;
	return;
    }

    /* Success */    
    msg_print("You enter a maze of up staircases. ");

    /* Go up the stairs */
    dun_level--;
    new_level_flag = TRUE;

    /* Prepare to create a reverse-stairway */
    if (dun_level > 0) create_down_stair = TRUE;
}


/*
 * Go down one level
 */
static void do_cmd_go_down()
{
    cave_type *c_ptr;
    inven_type *i_ptr;

    c_ptr = &cave[char_row][char_col];
    i_ptr = &i_list[c_ptr->i_idx];

    if (i_ptr->tval != TV_DOWN_STAIR) {
	msg_print("I see no down staircase here.");
	free_turn_flag = TRUE;
	return;
    }

    /* Success */
    msg_print("You enter a maze of down staircases. ");

    /* Go down */
    dun_level++;
    new_level_flag = TRUE;

    /* Create a way back */
    create_up_stair = TRUE;
}



/*
 * Refill the players lamp	-RAK-
 */
static void do_cmd_refill_lamp()
{
    int                  i, j;

    register inven_type *i_ptr;


    /* Find some oil or complain */
    if (!find_range(TV_FLASK, TV_NEVER, &i, &j)) {
	msg_print("You have no oil.");
    }

    else {

	free_turn_flag = FALSE;
	i_ptr = &inventory[INVEN_LITE];

	/* Use the oil from the bottom to the top */
	i_ptr->pval += inventory[j].pval;

	if (i_ptr->pval < FUEL_LAMP / 2) {
	    msg_print("Your lamp is less than half full.");
	}
	else if (i_ptr->pval < FUEL_LAMP / 2 + FUEL_LAMP / 16) {
	    msg_print("Your lamp is about half full.");
	}
	else if (i_ptr->pval < FUEL_LAMP) {
	    msg_print("Your lamp is more than half full.");
	}
	else {
	    i_ptr->pval = FUEL_LAMP;
	    msg_print("Your lamp overflows, spilling oil on the ground.");
	    msg_print("Your lamp is full.");
	}

	/* Now point at the oil */
	i_ptr = &inventory[j];

	/* Destroy a single flask */
	inven_item_increase(j, -1);
	inven_item_describe(j);
	inven_item_optimize(j);
    }
}


/*
 * Semi-Hack -- Restock the players torch
 */
static void do_cmd_refill_torch()
{
    int                  j, i1, i2;

    register inven_type *i_ptr = NULL;
    register inven_type *j_ptr;


    /* Find a wielded torch */
    j_ptr = &inventory[INVEN_LITE];

    /* No lites in the pack */
    if (!find_range(TV_LITE, TV_NEVER, &i1, &i2)) {
	msg_print("You have no torches in your pack.");
	return;
    }

    /* Hack -- Scan the Lite's for a usable torch (bottom first) */
    for (j = i2 ; j >= i1; j--) {
	i_ptr = &inventory[j];
	if (i_ptr->sval != SV_LITE_TORCH) continue;
	if (i_ptr->pval > 0) break;
    }

    /* None found */
    if (j < i1) {
	msg_print("You have no torch fuel in your pack.");
	return;
    }

    /* No free turn */
    free_turn_flag = FALSE;

    /* Message */
    msg_print("You refuel your torch using one from your pack.");


    /* Get the torch (again) */
    i_ptr = &inventory[j];

    /* Add the fuel */
    j_ptr->pval += i_ptr->pval;

    /* Over-fuel */
    if (j_ptr->pval > FUEL_TORCH) {
	j_ptr->pval = FUEL_TORCH;
	msg_print("Your torch is now fully fueled.");
    }

    /* Destroy a single torch */
    inven_item_increase(j, -1);
    inven_item_describe(j);
    inven_item_optimize(j);
}


/*
 * Refill the players lamp, or restock his torches
 */
static void do_cmd_refill(void)
{
    register inven_type *i_ptr;

    /* Assume a free turn */
    free_turn_flag = TRUE;

    /* Get the light */
    i_ptr = &inventory[INVEN_LITE];

    /* It is nothing */
    if (i_ptr->tval != TV_LITE) {
	message("You are not wielding a light", 0);
    }

    /* It's a lamp */
    else if (i_ptr->sval == SV_LITE_LANTERN) {
	do_cmd_refill_lamp();
    }

    /* It's a torch */
    else if (i_ptr->sval == SV_LITE_TORCH) {
	do_cmd_refill_torch();
    }

    /* No torch to refill */
    else {
	message("Your light cannot be refilled.", 0);
    }
}



/*
 * Support code for the "CTRL('P')" recall command
 *
 * If "command_arg" is set, recall that many messages.
 * Otherwise, recall until there are no more messages.
 *
 * The screen format uses line 0 for prompts, skips line 1,
 * uses line 2 and 23 for "[continued]" messages, and uses
 * lines 3 to 22 for the display of old messages.
 */

static void do_cmd_messages(void)
{
    int i = 0, j = 0, n = 0, k = 0;

    cptr pmt1 = "[... older messages continued above, use ^P to scroll ...]";
    cptr pmt2 = "[... newer messages continued below, use ^N to scroll ...]";

    /* Free move */
    free_turn_flag = TRUE;

    /* Assume "full recall" */
    n = message_num();

    /* Hack -- no messages */
    if (n <= 0) {
	prt("There are no messages to recall.", 0, 0);
	return;
    }

    /* Hack -- allow "quick recall" of a single message */
    if ((!command_arg) && (command_old != CTRL('P'))) n = 1;

    /* Hack -- quick recall of simple messages */
    if (n <= 1) {
	put_str(">", 0, 0);
	prt(message_str(0), 0, 1);
	return;
    }

    /* Save the screen */
    save_screen();

    /* Hack -- force "clear" */
    k = ' ';

    /* Process requests until done */
    while (1) {

	/* Only redraw if needed */
	if (k) {

	    /* Clear the screen */
	    clear_screen();

	    /* Dump the current batch of messages */
	    for (j = 0; j < 20 && (i+j < n); j++) {

		/* Dump the messages, bottom to top */
		put_str(message_str(i+j), 22-j, 0);
	    }

	    /* Indicate extra messages */
	    if (i + 20 < n) prt(pmt1, 2, 0);

	    /* Indicate extra messages */
	    if (i > 0) prt(pmt2, 23, 0);
	}

	/* Display a simple prompt */
	prt("Message Recall [press ESCAPE to exit] ", 0, 0);

	/* Get a command */
	k = inkey();

	/* Exit on Escape */
	if (k == ESCAPE) break;

	/* Save the old index */
	j = i;

	/* Recall more older messages */
	if (k == CTRL('P')) {
	    if (i+20 < n) i += 20;
	}

	/* Recall more newer messages */
	if (k == CTRL('N')) {
	    if (i >= 20) i -= 20;
	}

	/* Error of some kind, do not redraw */
	if (i == j) {
	    k = 0;
	    bell();
	}
    }

    /* Restore the screen */
    restore_screen();
}


/*
 * Target command
 */
static void do_cmd_target()
{
    /* Free move */
    free_turn_flag = TRUE;

#ifdef TARGET

    /* Be sure we can see */
    if (p_ptr->blind > 0) {
	msg_print("You can't see anything to target!");
    }
    else if (!target_set()) {
	msg_print("Target Aborted");
    }
    else {
	msg_print("Target Selected");
    }

#else

    msg_print("Target code not compiled in this version.");

#endif

}





/*
 * A simple structure to hold some options
 */
typedef struct _opt_desc {
    cptr	o_prompt;
    int         *o_var;
} opt_desc;


/*
 * General User-Interface Options
 */
static opt_desc options_interface[] = {

    { "Rogue-like commands", 			&rogue_like_commands },
    { "Require (g)et-key to pickup", 		&prompt_carry_flag },
    { "Prompt before pickup", 			&carry_query_flag },
    { "Accept all throw commands",		&always_throw },
    { "Always repeat commands",			&always_repeat },
    { "Plain object descriptions",		&plain_descriptions },
    { "Quick messages",		                &quick_messages },

    { "Hilite player",				&hilite_player },
    { "Dungeon level in feet",			&depth_in_feet },
    { "Notice mineral seams", 			&notice_seams },
    { "Use new screen layout",                  &new_screen_layout },
    { "Equippy chars",		                &equippy_chars },

    { "Use color",				&use_color },
    { "Recall window",				&use_recall_win },
    { "Choice window",				&use_choice_win },

    { "Ring bell on error",			&ring_bell },

    { NULL,					NULL }
};


/*
 * Disturbance Options -- for running/resting
 */
static opt_desc options_disturb[] = {

    { "Cut known corners",	 		&find_cut },
    { "Examine potential corners",		&find_examine },
    { "Print self during run",			&find_prself },
    { "Stop when map sector changes",		&find_bound },
    { "Run through open doors", 		&find_ignore_doors },
    { "Run past stairs", 			&find_ignore_stairs },

    { "Monster moving nearby disturbs me",	&disturb_near },
    { "Monster moving anywhere disturbs me",	&disturb_move },
    { "Monster appearance disturbs me",		&disturb_enter },
    { "Monster disappearance disturbs me",	&disturb_leave },
    
    { NULL,					NULL }
};


/*
 * Gameplay Options -- these actually affect game-play
 */
static opt_desc options_gameplay[] = {

    { "Disable haggling in stores",		&no_haggle_flag },
    { "Shuffle store owners",                   &shuffle_owners },

    { "Show weights in inventory", 		&show_inven_weight },
    { "Show weights in equipment list",		&show_equip_weight },
    { "Show weights in stores",			&show_store_weight },

    { "Pre-compute viewable region",		&view_pre_compute },
    { "Reduce view-radius when running",	&view_reduce_view },
    { "Reduce lite-radius when running",	&view_reduce_lite },

    { "Draw Torch-Lite in yellow",		&view_yellow_lite },
    { "Optimize Yellow-Lite when running",	&view_yellow_fast },
    { "Draw Viewable Lite brightly",		&view_bright_lite },
    { "Optimize Bright-Lite when running",	&view_bright_fast },

    { "Map remembers all perma-lit grids",	&view_perma_grids },
    { "Map remembers all torch-lit grids",	&view_torch_grids },

    { "Compress savefile (lose messages)",	&compress_savefile },
    
    { NULL,					NULL}
};


/*
 * Set or unset various boolean options.
 */
static void do_cmd_options_aux(opt_desc *options, cptr info)
{
    register int i, max, ch;
    vtype        pmt, string;

    clear_screen();

    /* Prompt */    
    sprintf(pmt, "%s (RET to advance, y/n to set, ESC to accept) ", info);
    prt(pmt, 0, 0);

    /* Prepare the screen, Count the options. */
    for (max = 0; options[max].o_prompt; max++) {
	sprintf(string, "%-38s: %s ", options[max].o_prompt,
		(*options[max].o_var ? "yes" : "no "));
	prt(string, max + 2, 0);
    }

    /* Start at the first option */
    i = 0;

    /* Interact with the player */
    for (;;) {
	move_cursor(i + 2, 40);
	ch = inkey();
	switch (ch) {
	  case ESCAPE:	  
	    return;
	  case '-':
	    i = (max + i - 1) % max;
	    break;
	  case ' ':
	  case '\n':
	  case '\r':
	    i = (i + 1) % max;
	    break;
	  case 'y':
	  case 'Y':
	    put_str("yes ", i + 2, 40);
	    *options[i].o_var = TRUE;
	    i = (i + 1) % max;
	    break;
	  case 'n':
	  case 'N':
	    put_str("no  ", i + 2, 40);
	    *options[i].o_var = FALSE;
	    i = (i + 1) % max;
	    break;
	  default:
	    bell();
	    break;
	}
    }
}


/*
 * Set or unset various options.  Redraw screen when done.
 */
static void do_cmd_options()
{
    register int i;

    while (1) {

	clear_screen();

	/* Give some choices */
	prt("(1) User Interface Options", 2, 5);
	prt("(2) Disturbance Options", 3, 5);
	prt("(3) Game-Play Options", 4, 5);
	prt("(4) Hitpoint Warning", 5, 5);
	prt("(5) Delay Speed", 6, 5);
	prt("(6) Object attr/chars", 7, 5);
	prt("(7) Monster attr/chars", 8, 5);
	prt("(8) Undefined options", 9, 5);

	/* Ask for a choice */        
	prt("Angband Options (1-8 or ESC to exit) ", 0, 0);
	i = inkey();
	if (i == ESCAPE) break;

	/* General Options */
	if (i == '1') {

	    /* Process the general options */
	    do_cmd_options_aux(options_interface, "User Interface Options");
	    
	    /* Hack -- turn "choices" window back off, it's broken */
	    use_choice_win = FALSE;
	}

	/* Disturbance Options */
	else if (i == '2') {

	    /* Process the running options */
	    do_cmd_options_aux(options_disturb, "Disturbance Options");
	}

	/* Gameplay Options */
	else if (i == '3') {

	    /* Process the running options */
	    do_cmd_options_aux(options_gameplay, "Game-Play Options");
	}

	/* Hack -- hitpoint warning factor */
	else if (i == '4') {

	    clear_screen();

	    /* Get a new value */
	    while (1) {
		char buf[128];
		sprintf(buf, "Current hitpoint warning: %d0%%", hitpoint_warn);
		prt(buf, 5, 5);
		prt("Hitpoint Warning (0-9 or ESC to accept) ", 0, 0);                    
		i = inkey();
		if (i == ESCAPE) break;
		if (isdigit(i)) hitpoint_warn = (i - '0');
		else bell();
	    }
	}

	/* Hack -- Delay Speed */
	else if (i == '5') {

	    clear_screen();

	    /* Get a new value */
	    while (1) {
		char buf[128];
		sprintf(buf, "Current delay speed: %d milliseconds", delay_spd);
		prt(buf, 5, 5);
		prt("Delay Speed (0-9 or ESC to accept) ", 0, 0);                    
		i = inkey();
		if (i == ESCAPE) break;
		if (isdigit(i)) delay_spd = (i - '0');
		else bell();
	    }
	}

	/* Redefine object attr/chars */
	else if (i == '6') {

	    /* Hack -- start on the "floor" */
	    static int k_idx = OBJ_GRANITE_WALL;

	    /* Clear the screen */
	    clear_screen();

	    /* Hack -- query until done */
	    while (1) {

		int k = k_idx;
		int k_a = (byte)k_list[k].i_attr;
		int k_c = (byte)k_list[k].i_char;
		int x_a = (byte)x_list[k].x_attr;
		int x_c = (byte)x_list[k].x_char;

		/* Label the object */
		Term_putstr(5, 10, -1, COLOR_WHITE,
			    format("Object = %d, Name = %-40.40s",
				   k, k_list[k].name));

		/* Label the Default values */
		Term_putstr(10, 12, -1, COLOR_WHITE,
			    format("Default attr/char = %3d / %3d",
			           k_a, k_c));
		Term_putstr(40, 12, -1, COLOR_WHITE, "<< ? >>");
		Term_putch(43, 12, k_a, k_c);

		/* Label the Current values */
		Term_putstr(10, 14, -1, COLOR_WHITE,
			    format("Current attr/char = %3d / %3d",
		                   x_a, x_c));
		Term_putstr(40, 14, -1, COLOR_WHITE, "<< ? >>");
		Term_putch(43, 14, x_a, x_c);

		/* Directions */
		Term_putstr(5, 2, -1, COLOR_WHITE, "n/N = change index");
		Term_putstr(5, 3, -1, COLOR_WHITE, "a/A = change attr");
		Term_putstr(5, 4, -1, COLOR_WHITE, "c/C = change char");
		
		/* Prompt */
		Term_putstr(0, 0, -1, COLOR_WHITE,
			    "Object attr/char (n/N/a/A/c/C): ");

		/* Get a command */
		i = inkey();

		/* All done */
		if (i == ESCAPE) break;

		/* Analyze */
		if (i == 'n') k_idx = (k_idx + 1) % MAX_K_IDX;
		if (i == 'N') k_idx = (k_idx + MAX_K_IDX - 1) % MAX_K_IDX;
		if (i == 'a') x_a = (x_a + 1) % 256;
		if (i == 'A') x_a = (x_a + 256 - 1) % 256;
		if (i == 'c') x_c = (x_c + 1) % 256;
		if (i == 'C') x_c = (x_c + 256 - 1) % 256;
		
		/* Hack -- Save the values */
		x_list[k].x_attr = x_a;
		x_list[k].x_char = x_c;
	    }
	}
	
	/* Hack -- edit monsters */
	else if (i == '7') {

	    /* Hack -- start on the "urchin" */
	    static int r_idx = 0;

	    /* Clear the screen */
	    clear_screen();

	    /* Hack -- query until done */
	    while (1) {

		int k = r_idx;
		int r_a = (byte)r_list[k].r_attr;
		int r_c = (byte)r_list[k].r_char;
		int l_a = (byte)l_list[k].l_attr;
		int l_c = (byte)l_list[k].l_char;

		/* Label the object */
		Term_putstr(5, 10, -1, COLOR_WHITE,
			    format("Monster = %d, Name = %-40.40s",
				   k, r_list[k].name));

		/* Label the Default values */
		Term_putstr(10, 12, -1, COLOR_WHITE,
			    format("Default attr/char = %3d / %3d",
			           r_a, r_c));
		Term_putstr(40, 12, -1, COLOR_WHITE, "<< ? >>");
		Term_putch(43, 12, r_a, r_c);

		/* Label the Current values */
		Term_putstr(10, 14, -1, COLOR_WHITE,
			    format("Current attr/char = %3d / %3d",
		                   l_a, l_c));
		Term_putstr(40, 14, -1, COLOR_WHITE, "<< ? >>");
		Term_putch(43, 14, l_a, l_c);

		/* Directions */
		Term_putstr(5, 2, -1, COLOR_WHITE, "n/N = change index");
		Term_putstr(5, 3, -1, COLOR_WHITE, "a/A = change attr");
		Term_putstr(5, 4, -1, COLOR_WHITE, "c/C = change char");
		
		/* Prompt */
		Term_putstr(0, 0, -1, COLOR_WHITE,
			    "Object attr/char (n/N/a/A/c/C): ");

		/* Get a command */
		i = inkey();

		/* All done */
		if (i == ESCAPE) break;

		/* Analyze */
		if (i == 'n') r_idx = (r_idx + 1) % MAX_K_IDX;
		if (i == 'N') r_idx = (r_idx + MAX_K_IDX - 1) % MAX_K_IDX;
		if (i == 'a') l_a = (l_a + 1) % 256;
		if (i == 'A') l_a = (l_a + 256 - 1) % 256;
		if (i == 'c') l_c = (l_c + 1) % 256;
		if (i == 'C') l_c = (l_c + 256 - 1) % 256;
		
		/* Hack -- Save the values */
		l_list[k].l_attr = l_a;
		l_list[k].l_char = l_c;
	    }
	}
	
	/* Unknown option */
	else {
	    bell();
	}
    }


    /* Free turn */
    free_turn_flag = TRUE;

    /* Redraw the screen */
    draw_cave();
    Term_redraw();
}



/*
 * Attempt to parse and execute the command 'command' in normal mode.
 * Return 'TRUE' iff we successfully parsed the message.
 */
static int do_std_command ()
{
    char prt1[80];


    /* Parse the command */
    switch (command_cmd) {

	/* (ESC) do nothing. */
	case ESCAPE:
	    free_turn_flag = TRUE; break;

	/* (SPACE) do nothing */
	case ' ':
	    free_turn_flag = TRUE; break;


	/*** Inventory Commands ***/

	/* Wear or weild something */
	case 'w':
	    inven_command('w'); break;

	/* Exchange primary and aux weapons */
	case 'x':
	    inven_command('x'); break;

	/* Drop something */
	case 'd':
	    inven_command('d'); break;

	/* Equipment list */
	case 'e':
	    inven_command('e'); break;

	/* Inventory */
	case 'i':
	    inven_command('i'); break;

	/* Take something off */
	case 't':
	    inven_command('t'); break;


	/*** Standard "Movement" Commands ***/

	/* Tunnel digging */
	case '}':
	    do_cmd_tunnel(); break;

	/* Move without picking anything up */
	case '-':
	    do_cmd_walk(FALSE); break;

	/* Move (picking up) */
	case ';':
	    do_cmd_walk(TRUE); break;


	/*** Commands that "re-interpret" the repeat count ***/

	/* Begin Running (done "off-line") -- Arg is Max Distance */
	case ',':
	    do_cmd_run();
	    break;

	/* Stay still, and, if repeated, Begin Resting -- Arg is time */
	case '\'':
	    do_cmd_stay(TRUE);
	    if (command_arg > 1) do_cmd_rest();
	    break;

	/* Begin Resting (rest done "off-line") -- Arg is time */
	case 'R':
	    do_cmd_rest();
	    break;


	/*** Searching, Resting ***/

	/* Get an object (Should it always be a free move?) */
	case 'g':
	    free_turn_flag = TRUE;
	    if (prompt_carry_flag) {
		if (cave[char_row][char_col].i_idx != 0) {
		    free_turn_flag = FALSE;
		    carry(char_row, char_col, TRUE);
		}
	    }
	    break;

	/* Toggle search status */
	case '#':
	    free_turn_flag = TRUE;
	    if (p_ptr->searching) {
		search_off();
	    }
	    else {
		search_on();
	    }
	    break;

	/* Search the adjoining grids */
	case 's':
	    do_cmd_search(); break;


	/*** Stairs and Doors and Chests and Traps ***/

	/* Go up staircases */
	case '<':
	    do_cmd_go_up(); break;

	/* Go down staircases */
	case '>':
	    do_cmd_go_down(); break;

	/* Open something */
	case 'o':
	    do_cmd_open(); break;

	/* Close something */
	case 'c':
	    do_cmd_close(); break;

	/* Spike a door */
	case 'S':
	    do_cmd_spike(); break;

	/* Force a door or Bash a monster. */
	case 'B':
	    do_cmd_bash(); break;

	/* Disarm a trap */
	case 'D':
	    do_cmd_disarm(); break;


	/*** Magic and Prayers ***/

	/* Peruse a Book */
	case 'P':
	    do_cmd_browse(); break;

	/* Gain some spells */
	case 'G':
	    gain_spells(); break;

	/* Cast a magic spell */
	case 'm':
	    cast(); break;

	/* Pray a prayer */
	case 'p':
	    pray(); break;


	/*** Use various objects ***/

	/* ({) inscribe an object    */
	case '{':
	    scribe_object(); free_turn_flag = TRUE; break;

	/* Activate an artifact */
	case 'A':
	    do_cmd_activate(); break;

	/* Eat some food */
	case 'E':
	    eat_food(); break;

	/* Fill the lamp */
	case 'F':
	    do_cmd_refill(); break;

	/* Throw something */
	case 'f':
	    do_cmd_fire(); break;

	/* Zap a wand */
	case 'a':
	    aim_wand(); break;

	/* Activate a rod */
	case 'z':
	    zap_rod(); break;

	/* Quaff a potion */
	case 'q':
	    quaff_potion(); break;

	/* Read a scroll */
	case 'r':
	    read_scroll(); break;

	/* Zap a staff */
	case 'u':
	    use_staff(); break;


	/*** Looking at Things (nearby or on map) ***/

	/* Full screen Map */
	case 'M':
	    do_cmd_view_map(); break;

	/* (W)here are we on the map	(L)ocate on map */	
	case 'W':
	    do_cmd_locate(); break;

	/* Examine surroundings (lowercase "L") */
	case 'l':
	    do_cmd_look(); break;

	/* Attempt to select a new target, if compiled */
	case '*':
	    do_cmd_target(); break;



	/*** Help and Such ***/

	/* Help */
	case '?':
	    do_cmd_help(); break;

	/* Identify Symbol */
	case '/':
	    ident_char(); free_turn_flag = TRUE; break;

	/* Character Description */
	case 'C':
	    save_screen();
	    change_name();
	    restore_screen();
	    free_turn_flag = TRUE;
	    break;



	/*** System Commands ***/

	/* Game Version */
	case 'V':
	    helpfile(ANGBAND_VERSION); free_turn_flag = TRUE; break;

	/* Repeat (^F)eeling */
	case CTRL('F'):
	    do_cmd_feeling(); break;

	/* (^P)revious message. */
	case CTRL('P'):
	    do_cmd_messages(); break;

	/* (^W)izard mode */
	case CTRL('W'):
	    if (!wizard && enter_wiz_mode()) {
		msg_print("Wizard mode on.");
	    }
	    prt_winner();
	    free_turn_flag = TRUE;
	    break;

	/* Commit Suicide and Quit */
	case 'Q':
	    flush();
	    if (get_check(total_winner ?
			  "Do you want to retire?" :
			  "Do you really want to quit?")) {
		new_level_flag = TRUE;
		death = TRUE;
		(void)strcpy(died_from, "Quitting");
	    }
	    free_turn_flag = TRUE;
	    break;

	/* Save and Quit */
	case CTRL('X'):
	    if (total_winner) {
		msg_print("You are a Total Winner, your character must be retired.");
		if (rogue_like_commands) {
		    msg_print("Use 'Q' to when you are ready to retire.");
		}
		else {
		    msg_print("Use <Control>-K when you are ready to retire.");
		}
	    }
	    else {

		/* Attempt to save */
		(void)strcpy(died_from, "(saved)");
		msg_print("Saving game...");
		if (save_player()) {
		    msg_print("done.");
		    quit(NULL);
		}

		/* Oops. */
		msg_print("Save failed...");
		(void)strcpy(died_from, "(alive and well)");
	    }
	    free_turn_flag = TRUE;
	    break;

	/* Redraw the screen (i.e. when hallucinations run out */
	case CTRL('R'):

	    /* Hallucination, redraw with a "warning" :-) */
	    if (p_ptr->image > 0) {
		msg_print("You cannot be sure what is real and what is not!");
	    }

	    /* Redraw the screen */
	    draw_cave();
	    Term_redraw();

	    free_turn_flag = TRUE;
	    break;

	/* (=) set options */
	case '=':
	    do_cmd_options();
	    break;

#ifdef ALLOW_SCORE
	/* score patch originally by Mike Welsh mikewe@acacia.cs.pdx.edu */
	case 'v':
	    sprintf(prt1,"Your current score is: %ld", total_points());
	    message(prt1, 0);
	    free_turn_flag = TRUE;
	    break;
#endif

#ifdef ALLOW_ARTIFACT_CHECK /* -CWS */
	/* Check artifacts -- NOT as a wizard */
	case '~':
	    /* Hack -- no checking in the dungeon */
	    if (dun_level) {
		msg_print("You need to be in town to check artifacts!");
	    }
	    else {
		artifact_check();
	    }
	    free_turn_flag = TRUE;
	    break;
#endif

#ifdef ALLOW_CHECK_UNIQUES /* -CWS */
	/* Check uniques -- NOT as a wizard */
	case '|':
	    check_uniques();
	    free_turn_flag = TRUE;
	    break;
#endif

	default:
	    return (FALSE);
    }

    /* Successful Parse */
    return (TRUE);
}


/*
 * Parse and execute the command 'command', or warn if unable
 */

void process_command(void)
{
    int okay = FALSE;

    /* Try the Wizard Commands */
    if (!okay && do_wiz_command()) okay = TRUE;

    /* Try the Standard Commands */
    if (!okay && do_std_command()) okay = TRUE;

    /* If the command scanned, save it -- and if not? */
    if (okay) command_old = command_cmd;

    /* Otherwise, give help */
    else {
	/* Wizard Help */
	if (wizard) {
	    prt("Type '?' or '\\' for help.", 0, 0);
	}

	/* Normal Help */
	else {
	    prt("Type '?' for help.", 0, 0);
	}

	/* A free move in either case */
	free_turn_flag = TRUE;
    }
}





/*
 * XXX An explanation of the "Angband Keypress" concept. XXX
 *
 * Inherently, many Angband commands consist not only of a basic action
 * (such as "tunnel"), but also a direction (such as "north-east"), and
 * even a "repeat" count or similar argument (such as a "max run distance").
 *
 * These components are thus explicitly represented, with globals.
 * The "base command" (see below) is stored in "command_cmd"
 * The "desired direction" is stored in "command_dir".
 * The "repeat count" is stored in "command_rep"
 * The "numerical argument" is stored in "command_arg"
 *
 * When "command_dir" is set, it overrides all calls to "get_*dir()"
 * So we always reset command_dir to -1 before getting a new command.
 *
 * Note that "command_arg" is sometimes used to select an argument
 * to a command (whereas "command_rep" actually "repeats" it).
 * Commands using this include "rest", "make treasure", and "run".
 *
 * Note that nothing cancels "command_rep", so it must be explicitly
 * canceled by the repeatable functions on "termination" conditions.
 * The simple way to do this is to force the respective do_cmd routines
 * to actually check explicitly for permission to NOT cancel it.
 *
 * Note that, to correctly handle repeating commands, all commands that
 * wish to be repeatable AND to do so with a specific "direction" had
 * better set "command_dir" on their first call to the user's DESIRED
 * direction.  A local copy can then be used to check confusion, etc.
 * The easiest way to do this is to actually USE "command_dir" as the
 * argument holding the direction (see "do_cmd_walk").
 *
 * Note that, however, to correctly handle confusion + repeated commands,
 * it may be necessary to call "get_a_dir" as above, and then use a temp
 * dir to apply confusion, via "dir = command_dir; confuse_dir(&dir,mode);"
 * where mode is, for example, 0x02 for "partial" confusion.
 *
 * The functions below this comment encode conversions from the traditional
 * "roguelike" and "original" command sets into the new "Angband Commands"
 * (command_cmd + command_dir, etc).  Note that since the "Angband Chars"
 * are never officially entered by the user, they may be modified at will,
 * and can in fact be replaced by integers.  This may first require a rewrite
 * of "inven_command()" and "target()" and such, however.
 *
 * Actually, it looks like a low level "keymapper" will do the trick...
 *
 * To go along with the vars described above, there are two others.
 * The last command is stored in "command_old".
 * Eventually, "command_esc" will allow one to track low level "Escapes".
 *
 * The actual "Angband Chars" should not be relevant to the user, since
 * they are totally internal, but they are used to prepare the keymapper.
 * Currently, they greatly resemble the "original keys" but this will
 * go through several revisions until they are all symbolic constants.
 *
 * For interest's sake, note that:
 *   "Walk" is encoded as ";" plus a direction.
 *   "Walk without picking up" is encoded as "-" plus a direction
 *   "Tunnel" is encoded as "}" plus a direction
 *   "Run until bored" is encoded as "," plus a direction
 *   Stay still is encoded as "'" (with no direction)
 *
 * The separation of the actual keys from the encodings shuold greatly
 * facilitate the use of macros of various kinds...   Note that now the
 * only difference between Roguelike and Original specification of, say,
 * tunnelling, is that Roguelike pre-sets the command direction.  Furthur,
 * now that "inventory tagging" is implemented, it is possible to conceive
 * of a macro such as "weild my shovel ("@3"), tunnel 100 times in the current
 * direction, weild my weapon ("@1"), move (no pickup) in that direction,
 * pick up whatever is there".  Hmmm...  For ease of use, the keymapper may
 * encode directions via use of the "Meta-Keys" in the upper regions.
 * The other solution (a hack) would be to not have a strict "one-to-one"
 * mapping from keypress to command.  Hmmm...
 *
 * Note that targeting mode will soon be changed to do "on-demand" reset
 * of the target (along with requiring the target to be "chosen").  Should
 * this "over-ride" the "specify no-direction" flag?  Hmmm...
 */




/*
 * Convert a "Rogue" keypress into an "Angband" keypress
 * Where necessary, set the global variable "command_dir"
 */
static char roguelike_commands(char command)
{
    /* Process the command */
    switch (command) {

	/* Wizard Commands */
	case '\\':		/* wizard help */
	case CTRL('A'):		/* cure all */
	case CTRL('D'):		/* up/down */
	case CTRL('E'):		/* wizchar */
	case CTRL('G'):		/* good treasure */
	case CTRL('I'):		/* identify */
	case CTRL('T'):		/* teleport */
	case CTRL('V'):		/* very good treasure */
	case CTRL('Z'):		/* genocide */
	case CTRL('^'):		/* Identify up to level? */
	case ':':		/* map area */
	case '!':		/* rerate hitpoints */
	case '@':		/* create object */
	case '$':		/* wiz. light */
	case '%':		/* self knowledge */
	case '&':		/* summon monster */
	case '+':		/* add experience */
	    return (command);

	/* Movement (rogue keys) */
	case 'b': command_dir = 1; return (';');
	case 'j': command_dir = 2; return (';');
	case 'n': command_dir = 3; return (';');
	case 'h': command_dir = 4; return (';');
	case 'l': command_dir = 6; return (';');
	case 'y': command_dir = 7; return (';');
	case 'k': command_dir = 8; return (';');
	case 'u': command_dir = 9; return (';');

	/* Running (press SHFT(move)) */
	case 'B': command_dir = 1; return (',');
	case 'J': command_dir = 2; return (',');
	case 'N': command_dir = 3; return (',');
	case 'H': command_dir = 4; return (',');
	case 'L': command_dir = 6; return (',');
	case 'Y': command_dir = 7; return (',');
	case 'K': command_dir = 8; return (',');
	case 'U': command_dir = 9; return (',');

	/* Tunnel digging (press CTRL(move)) */
	case CTRL('B'): command_dir = 1; return ('}');
	case CTRL('J'): command_dir = 2; return ('}');
	case CTRL('N'): command_dir = 3; return ('}');
	case CTRL('H'): command_dir = 4; return ('}');
	case CTRL('L'): command_dir = 6; return ('}');
	case CTRL('Y'): command_dir = 7; return ('}');
	case CTRL('K'): command_dir = 8; return ('}');
	case CTRL('U'): command_dir = 9; return ('}');

	/* Hack -- CTRL('M') == return == linefeed == CTRL('J') */
	case CTRL('M'): command_dir = 2; return ('}');

	/* Zap a staff */
	case 'Z':
	    return ('u');

	/* Take off */
	case 'T':
	    return ('t');

	/* Fire */
	case 't':
	    return ('f');

	/* Bash */
	case 'f':
	    return ('B');

	/* Look */
	case 'x':
	    return ('l');

	/* Exchange */
	case 'X':
	    return ('x');

	/* Walking without picking up */
	case '-':
	    return ('-');

	/* Aim a wand */
	case 'z':
	    return ('a');

	/* Zap a rod */
	case 'a':
	    return ('z');

	/* Stand still */
	case '.':
	    return ('\'');




	case ' ':
	case CTRL('F'):
	case CTRL('R'):
	case CTRL('P'):
	case CTRL('W'):
	case CTRL('X'):
	case '#':
	case '/':
	case '<':
	case '>':
	case '=':
	case '{':
	case '?':
	case '~':
	case '|':
	case '*':
	case 'A':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'M':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'V':
	case 'W':
	case 'c':
	case 'd':
	case 'e':
	case 'g':
	case 'i':
	case 'm':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 'v':
	case 'w':
	case '}':
	case ',':
	case ';':
	    return (command);
    }

    /* Hack -- Invalid command */
    return (ESCAPE);
}


/*
 * Convert an "Original" keypress into an "Angband" keypress
 * Where necessary, set the global variable "command_dir"
 */
static char original_commands(char command)
{
    /* Process the command */
    switch (command) {

	case CTRL('J'):
	case CTRL('M'):
	case ' ':
	    return (' ');

	case CTRL('K'):
	    return ('Q');

	case 'L':
	    return ('W');

	case 'S':
	    return ('#');

	case 'b':
	    return ('P');

	case 'h':
	    return ('?');

	case 'j':
	    return ('S');

	case ESCAPE:
	case CTRL('F'):
	case CTRL('R'):
	case CTRL('P'):
	case CTRL('W'):
	case CTRL('X'):
	case '/':
	case '<':
	case '>':
	case '=':
	case '{':
	case '?':
	case '~':
	case '|':
	case '*':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'M':
	case 'R':
	case 'V':
	case 'a':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'i':
	case 'l':
	case 'm':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'z':
	    return (command);

	/* Wizard Commands */
	case '\\':		/* wizard help */
	case CTRL('A'):		/* cure all */
	case CTRL('D'):		/* up/down */
	case CTRL('E'):		/* wizchar */
	case CTRL('G'):		/* good treasure */
	case CTRL('I'):		/* identify */
	case CTRL('T'):		/* teleport */
	case CTRL('V'):		/* very good treasure */
	case CTRL('Z'):		/* genocide */
	case CTRL('^'):		/* Identify up to level? */
	case ':':		/* map area */
	case '!':		/* rerate hitpoints */
	case '@':		/* create object */
	case '$':		/* wiz. light */
	case '%':		/* self knowledge */
	case '&':		/* summon monster */
	case '+':		/* add experience */
	    return (command);

	/* Stand still (That is, Rest one turn) */
	case '5':
	    return ('\'');

	/* Standard walking */
	case '1': command_dir = 1; return (';');
	case '2': command_dir = 2; return (';');
	case '3': command_dir = 3; return (';');
	case '4': command_dir = 4; return (';');
	case '6': command_dir = 6; return (';');
	case '7': command_dir = 7; return (';');
	case '8': command_dir = 8; return (';');
	case '9': command_dir = 9; return (';');

	/* Walking without picking up */
	case '-':
	    return ('-');

	/* Running as far as possible (allow comma as well) */
	case '.':
	    return (',');

	/* Tunnel digging (allow '}' as well) */
	case 'T':
	    return ('}');

	/* Hack -- allow certain "Angband Keys" to be themselves */
	case '}':
	case ';':
	    return (command);
    }

    /* Return an obviously invalid command */
    return (ESCAPE);
}


/*
 * Check whether this command can be "repeated".
 *
 * Note -- this routine applies ONLY to "Angband Commands".
 * Thus, the "Roguelike Movement Keys" are meaningless.
 *
 * Repeated commands must be VERY careful to correctly
 * turn off the repeat when they are done with it!
 */
static int command_takes_rep(char c)
{
    /* Examine the command */
    switch (c) {

	/* Take a direction */
	case '-': /* Jump */
	case ';': /* Walk */
	case '}': /* Tunnel */
	case 'B': /* Bash/Force */
	case 'D': /* Disarm */
	case 'S': /* Spike */
	case 'o': /* Open */
	    return TRUE;

	/* Take no direction */
	case 's': /* Search */
	    return TRUE;
    }

    /* Assume no count allowed */
    return (FALSE);
}



/*
 * Check whether this command will accept an argument.
 * An "argument" command is one which allows the use of
 * the "repeat" formalism, but does not actually repeat.
 *
 * These commands are supplied an "extra" argument in the global variable
 * "command_arg".  It is (currently) always an integer from 0 to 9999.
 *
 * Note -- this routine applies ONLY to "Angband Commands".
 */
static int command_takes_arg(char c)
{
    /* Examine the command */
    switch (c) {

	/* Normal commands */
	case ',': /* Run */
	case 'R': /* Rest */
	case '\'': /* Stay */

	/* Wizard commands */
	case CTRL('P'): /* Recall */
	case CTRL('D'): /* Teleport to Level */
	case CTRL('G'): /* Good treasure */
	case CTRL('V'): /* Very good treasure */
	case '+':       /* Add experience */
	    return TRUE;
    }

    /* Assume no count allowed */
    return (FALSE);
}



/*
 * Request a command from the user.
 *
 * Sets command_cmd, command_dir, command_rep, command_arg.
 */
void request_command()
{
    int i;
    char cmd;
    char tmp[8];


    /* Receive the command */
    command_cmd = 0;

    /* Commands start without a direction */
    command_dir = -1;

    /* Reset the count (already done) */
    command_rep = 0;

    /* Allow commands to take a "non-count" argument */
    command_arg = 0;

    /* Later -- see if a command gets aborted */
    command_esc = 0;


    /* Assume no "Count" */
    i = 0;

    /* No message pending */
    msg_flag = FALSE;

    /* Usually, we "hilite" the player */
    if (hilite_player) {
    
	/* Get a key */
	cmd = inkey();
    }

    /* Some systems can cleverly "hide" the cursor */
    else {

	int okay;
	
	/* Hack -- Hide the cursor cause it is ugly */
	okay = Term_hide_cursor();

	/* Get a key */
	cmd = Term_inkey();

	/* Hack -- Show the cursor to undo the hack above */
	if (!okay) Term_show_cursor();
    }

    /* Pseudo-Hack -- Get a count for a command -- allow "0" always */
    if ((rogue_like_commands && cmd >= '0' && cmd <= '9') ||
	(!rogue_like_commands && cmd == '#') ||
	(!rogue_like_commands && cmd == '0')) {

	prt("Repeat count:", 0, 0);

	/* Hack to "forget" about the '#' keypress */
	if (cmd == '#') cmd = '0';

	/* Get a command count */
	while (1) {

	    /* Simple editing -- see hack below to repeat "CTRL('H')" */
	    if (cmd == DELETE || cmd == CTRL('H')) {
		i = i / 10;
		(void)sprintf(tmp, "%d ", i);
		prt(tmp, 0, 14);
	    }

	    /* Actual numeric data */
	    else if (cmd >= '0' && cmd <= '9') {

		/* Allow counts up to 9999 */
		if (i > 999) {
		    bell();
		}

		/* Incorporate that digit */
		else {
		    i = i * 10 + cmd - '0';
		    (void)sprintf(tmp, "%d", i);
		    prt(tmp, 0, 14);
		}
	    }

	    /* Exit on "unusable" input */
	    else {
		break;
	    }

	    /* Get the next digit, edit, or command */
	    cmd = inkey();
	}

	/* Let a "non-count" default to 99 repetitions */
	if (i == 0) {
	    i = 99;
	    (void)sprintf(tmp, "%d", i);
	    prt(tmp, 0, 14);
	}

	/* Hack -- allow "0" to "9" and "^H" to be used as commands */
	/* Simply hit space to break out of the "count" specification */
	if (cmd == ' ') {
	    if (!get_com("Command:", &cmd)) cmd = ESCAPE;
	}
    }

    /* Allow control codes to be entered with '^' + a char. [-CJS-] */
    /* This is very useful on the Mac for CTRL('R') and CTRL('^'). */
    if (cmd == '^') {

	/* Get a char to "cast" into a control char */
	if (!get_com("Control-", &cmd)) cmd = ESCAPE;

	/* Do not allow Control-Space */
	else if (cmd == ' ') cmd = ESCAPE;

	/* Special hack to allow "Control-Six" */
	else if (cmd == '6') cmd = CTRL('^');

	/* Special hack to allow "Control-Minus" */
	else if (cmd == '-') cmd = CTRL('_');

	/* Just cast the character to a control char */
	else cmd = CTRL(cmd);
    }

    /* move cursor to player char again, in case it moved */
    move_cursor_relative(char_row, char_col);

    /* Process (and Convert) Roguelike Commands */
    if (rogue_like_commands) {
	command_cmd = roguelike_commands(cmd);
    }

    /* Process (and Convert) Original Commands */
    else {
	command_cmd = original_commands(cmd);
    }

    /* Hack -- let some commands get repeated by default */
    if (always_repeat && (i <= 0)) {

	/* Tunnel, Bash, Disarm, Open, Search get 99 tries */
	if (strchr("}BDos", command_cmd)) i = 99;
    }

    /* Make sure a "Count" is legal for this command */
    if ((i > 0) && (command_cmd != ESCAPE)) {

	/* Commands that can be repeated */
	if (command_takes_rep(command_cmd)) {

	    /* Save and display the count */
	    command_rep = i;
	    prt_state();

	    /* This execution gets counted */
	    command_rep--;
	}

	/* Commands that take arguments */
	else if (command_takes_arg(command_cmd)) {

	    /* Save the argument */
	    command_arg = i;
	}

	/* Invalid combination */
	else {

	    /* Abort gracefully */
	    msg_print("Invalid command with a count.");

	    /* Forget the command */
	    command_cmd = ESCAPE;
	}
    }
}



