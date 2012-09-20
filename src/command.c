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



/*
 * Hack -- make sure we have a good "ANSI" definition for "CTRL()"
 */
#undef CTRL
#define CTRL(C) ((C)&037)



/*
 * Give the player some help (files may be out of date)
 */
static void do_cmd_help(cptr fname)
{	    
    /* Help is always free */
    free_turn_flag = TRUE;

    /* Default help files */
    if (!fname) fname = rogue_like_commands ? ANGBAND_R_HELP : ANGBAND_O_HELP;

    /* Dump the help file file */
    helpfile(fname);
}




/*
 * Examine a Book					-RAK-	
 */
static void do_cmd_browse(void)
{
    u32b               j1, j2, tmp;
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

    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Get a book or stop checking */
    if (!get_item(&item_val, "Browse which Book?", i, k)) return;

    /* Cancel auto-see */
    command_see = FALSE;
    

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

    /* Create a way back */
    create_down_stair = TRUE;
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
 * Hack -- commit suicide
 */
static void do_cmd_suicide(void)
{
    free_turn_flag = TRUE;

    flush();

    if (total_winner) {
	if (!get_check("Do you want to retire?")) return;
    }
    else {
	int i;
	if (!get_check("Do you really want to quit?")) return;
	prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
	i = inkey();
	prt("", 0, 0);
	if (i != '@') return;
    }

    death = TRUE;
    (void)strcpy(died_from, "Quitting");
}


/*
 * Hack -- redraw the screen
 */
static void do_cmd_redraw(void)
{
    free_turn_flag = TRUE;

    if (p_ptr->image > 0) {
	msg_print("You cannot be sure what is real and what is not!");
    }

    /* Redraw the screen */
    draw_cave();
    Term_redraw();
}


/*
 * Hack -- change name
 */
static void do_cmd_change_name(void)
{
    free_turn_flag = TRUE;

    save_screen();
    change_name();
    restore_screen();
}


/*
 * Hack -- toggle search mode
 */
static void do_cmd_toggle_search(void)
{
    free_turn_flag = TRUE;

    if (p_ptr->searching) {
	search_off();
    }
    else {
	search_on();
    }
}


/*
 * Hack -- pick up objects
 */
static void do_cmd_pick_up(void)
{
    free_turn_flag = TRUE;

    if (!prompt_carry_flag) return;
    
    if (cave[char_row][char_col].i_idx == 0) return;

    free_turn_flag = FALSE;
    carry(char_row, char_col, TRUE);
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

	/* Use the cheapest oil first */
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


    /* Refuel the wielded torch */
    j_ptr = &inventory[INVEN_LITE];

    /* No lites in the pack */
    if (!find_range(TV_LITE, TV_NEVER, &i1, &i2)) {
	msg_print("You have no torches in your pack.");
	return;
    }

    /* Hack -- Scan the Lite's for a usable torch (bottom up) */
    for (j = i2 ; j >= i1; j--) {
	i_ptr = &inventory[j];
	if (i_ptr->sval == SV_LITE_TORCH) break;
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

    /* Add the fuel (empty torches yield 5 fuel) */
    j_ptr->pval += i_ptr->pval + 5;

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
    { "Quick messages",		                &quick_messages },
    { "Require (g)et-key to pickup", 		&prompt_carry_flag },
    { "Prompt before pickup", 			&carry_query_flag },
    { "Use old target by default",		&use_old_target },
    { "Accept all pickup commands", 		&always_pickup },
    { "Accept all throw commands",		&always_throw },
    { "Repeat obvious commands",		&always_repeat },

    { "Use new screen layout",                  &new_screen_layout },
    { "Display Equippy chars",		        &equippy_chars },
    { "Show dungeon level in feet",		&depth_in_feet },
    { "Notice mineral seams", 			&notice_seams },

    { "Use color",				&use_color },
    { "Recall window",				&use_recall_win },
    { "Choice window",				&use_choice_win },

    { "Compress savefiles",			&compress_savefile },
    
    { "Hilite the player",			&hilite_player },

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
    
    { "Flush input whenever disturbed",		&flush_disturb },
    { "Flush input on various failures",	&flush_failure },
    { "Flush input before every command",	&flush_command },
    { "Flush input (not used yet)",		&flush_unused },

    { "Draw Torch-Lite in yellow",		&view_yellow_lite },
    { "Draw Viewable Lite brightly",		&view_bright_lite },
    { "Optimize Yellow-Lite when running",	&view_yellow_fast },
    { "Optimize Bright-Lite when running",	&view_bright_fast },

    { NULL,					NULL }
};


/*
 * Gameplay Options -- these actually affect game-play
 */
static opt_desc options_gameplay[] = {

    { "Pre-compute viewable region",		&view_pre_compute },
    { "Pre-compute something else",		&view_xxx_compute },

    { "Reduce view-radius when running",	&view_reduce_view },
    { "Reduce lite-radius when running",	&view_reduce_lite },
    
    { "Map remembers all illuminated walls",	&view_wall_memory },
    { "Map remembers all important stuff",	&view_xtra_memory },
    { "Map remembers all perma-lit grids",	&view_perma_grids },
    { "Map remembers all torch-lit grids",	&view_torch_grids },

    { "Monsters chase current location",	&flow_by_sound },
    { "Monsters chase recent locations",	&flow_by_smell },
    
    { "Disable haggling in stores",		&no_haggle_flag },
    { "Shuffle store owners",                   &shuffle_owners },

    { "Show weights in inventory", 		&show_inven_weight },
    { "Show weights in equipment list",		&show_equip_weight },
    { "Show weights in stores",			&show_store_weight },
    { "Plain object descriptions",		&plain_descriptions },

    { "Allow weapons and armor to stack",	&stack_allow_items },
    { "Allow wands/staffs/rods to stack",	&stack_allow_wands },
    { "Over-ride inscriptions when stacking",	&stack_force_notes },
    { "Over-ride discounts when stacking",	&stack_force_costs },

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
	prt("(8) Dump a macro file", 9, 5);

	/* Ask for a choice */        
	prt("Angband Options (1-8 or ESC to exit) ", 0, 0);
	i = inkey();
	if (i == ESCAPE) break;

	/* General Options */
	if (i == '1') {

	    /* Process the general options */
	    do_cmd_options_aux(options_interface, "User Interface Options");
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

	/* Hack -- Redefine object attr/chars */
	else if (i == '6') {

	    /* Hack -- start on the "floor" */
	    static int k_idx = OBJ_GRANITE_WALL;

	    /* Clear the screen */
	    clear_screen();

	    /* Hack -- query until done */
	    while (1) {

		int k = k_idx;
		
		int da = (byte)k_list[k].k_attr;
		int dc = (byte)k_list[k].k_char;
		int ca = (byte)k_attr[k];
		int cc = (byte)k_char[k];
		
		/* Label the object */
		Term_putstr(5, 10, -1, TERM_WHITE,
			    format("Object = %d, Name = %-40.40s",
				   k, k_list[k].name));

		/* Label the Default values */
		Term_putstr(10, 12, -1, TERM_WHITE,
			    format("Default attr/char = %3d / %3d", da, dc));
		Term_putstr(40, 12, -1, TERM_WHITE, "<< ? >>");
		Term_putch(43, 12, da, dc);

		/* Label the Current values */
		Term_putstr(10, 14, -1, TERM_WHITE,
			    format("Current attr/char = %3d / %3d", ca, cc));
		Term_putstr(40, 14, -1, TERM_WHITE, "<< ? >>");
		Term_putch(43, 14, ca, cc);

		/* Directions */
		Term_putstr(5, 2, -1, TERM_WHITE, "n/N = change index");
		Term_putstr(5, 3, -1, TERM_WHITE, "a/A = change attr");
		Term_putstr(5, 4, -1, TERM_WHITE, "c/C = change char");
		
		/* Prompt */
		Term_putstr(0, 0, -1, TERM_WHITE,
			    "Object attr/char (n/N/a/A/c/C): ");

		/* Get a command */
		i = inkey();

		/* All done */
		if (i == ESCAPE) break;

		/* Analyze */
		if (i == 'n') k_idx = (k_idx + MAX_K_IDX + 1) % MAX_K_IDX;
		if (i == 'N') k_idx = (k_idx + MAX_K_IDX - 1) % MAX_K_IDX;
		if (i == 'a') k_attr[k] = ca + 1;
		if (i == 'A') k_attr[k] = ca - 1;
		if (i == 'c') k_char[k] = cc + 1;
		if (i == 'C') k_char[k] = cc - 1;
	    }
	}
	
	/* Hack -- monsters */
	else if (i == '7') {

	    /* Hack -- start on the "urchin" */
	    static int r_idx = 0;

	    /* Clear the screen */
	    clear_screen();

	    /* Hack -- query until done */
	    while (1) {

		int r = r_idx;

		int da = (byte)r_list[r].r_attr;
		int dc = (byte)r_list[r].r_char;
		int ca = (byte)r_attr[r];
		int cc = (byte)r_char[r];

		/* Label the object */
		Term_putstr(5, 10, -1, TERM_WHITE,
			    format("Object = %d, Name = %-40.40s",
				   r, r_list[r].name));

		/* Label the Default values */
		Term_putstr(10, 12, -1, TERM_WHITE,
			    format("Default attr/char = %3u / %3u", da, dc));
		Term_putstr(40, 12, -1, TERM_WHITE, "<< ? >>");
		Term_putch(43, 12, da, dc);

		/* Label the Current values */
		Term_putstr(10, 14, -1, TERM_WHITE,
			    format("Current attr/char = %3u / %3u", ca, cc));
		Term_putstr(40, 14, -1, TERM_WHITE, "<< ? >>");
		Term_putch(43, 14, ca, cc);

		/* Directions */
		Term_putstr(5, 2, -1, TERM_WHITE, "n/N = change index");
		Term_putstr(5, 3, -1, TERM_WHITE, "a/A = change attr");
		Term_putstr(5, 4, -1, TERM_WHITE, "c/C = change char");
		
		/* Prompt */
		Term_putstr(0, 0, -1, TERM_WHITE,
			    "Object attr/char (n/N/a/A/c/C): ");

		/* Get a command */
		i = inkey();

		/* All done */
		if (i == ESCAPE) break;

		/* Analyze */
		if (i == 'n') r_idx = (r_idx + MAX_K_IDX + 1) % MAX_K_IDX;
		if (i == 'N') r_idx = (r_idx + MAX_K_IDX - 1) % MAX_K_IDX;
		if (i == 'a') r_attr[r] = ca + 1;
		if (i == 'A') r_attr[r] = ca - 1;
		if (i == 'c') r_char[r] = cc + 1;
		if (i == 'C') r_char[r] = cc - 1;
	    }
	}
	
	/* Macro interaction */
	else if (i == '8') {

	    char buf[80];

	    /* Hack -- dump macros to a file */
	    clear_screen();

	    /* Info */
	    prt("This option allows all current macros to be appended to", 0, 0);
	    prt("the file of your choice.  Return uses the startup file.", 1, 0);
	    
	    /* Get a filename, dump the macros to it */
	    prt("File: ", 5, 0);
	    if (!askfor(buf, 70)) continue;
	    
	    /* Empty filename yields "default" */
	    if (!buf[0]) macro_dump(ANGBAND_A_LIST);
	    
	    /* Dump the macros */
	    else macro_dump(buf);
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
 * Note something in the message recall
 */
static void do_cmd_note(void)
{
    char buf[128];
    free_turn_flag = TRUE;
    prt("Note: ", 0, 0);
    if (askfor(buf, 60)) msg_print(format("Note: %s", buf));
}






#ifdef ALLOW_KEYMAP

/*
 * Define a new "keymap" entry
 */
static void do_cmd_keymap(void)
{
    int k, r, d;
    char i1, i2, i3;
    
    /* Free turn */
    free_turn_flag = TRUE;
    
    /* Flush the input */
    flush();

    /* Get the trigger */
    if (get_com("Type the trigger keypress: ", &i1) &&
	get_com("Type the resulting keypress: ", &i2) &&
	get_com("Type a direction (or space): ", &i3)) {

	/* Acquire the array index */
	k = (byte)(i1);
	r = (byte)(i2);
	d = (byte)(i3);
	
	/* Hack -- ignore icky keys */
	if (k >= 128) k = 0;
	
	/* Analyze the result key */
	keymap_cmds[k] = (r < 128) ? i2 : ESCAPE;

	/* Analyze the "direction" */
	keymap_dirs[k] = keymap_dirs[d];

	/* Success */
	prt(format("Keypress 0x%02x mapped to 0x%02x/%d",
		   k, keymap_cmds[k], keymap_dirs[k]), 0, 0);
    }

    /* Forget it */
    else {
	prt("Cancelled.", 0, 0);
    }
}

#endif



#ifdef ALLOW_MACROS

/*
 * Define a new macro
 *
 * Note that this function resembles "inkey_aux()" in "io.c"
 *
 * The "trigger" must be entered with less than 10 milliseconds
 * between keypresses, as in "inkey_aux()".
 */
static void do_cmd_macro(void)
{
    int i, n = 0, w, fix;
    
    char pat[1024] = "";
    char act[1024] = "";

    char tmp[1024];


    /* Free turn */
    free_turn_flag = TRUE;
    
    
    /* Flush the input */
    flush();

    /* Show the cursor */
    fix = (0 == Term_show_cursor());
    
    /* Get the first key */
    prt("Press the macro trigger key: ", 0, 0);
    Term_fresh();

    /* Wait for the first key */
    i = Term_inkey();
    pat[n++] = i;
    pat[n] = '\0';

    /* Read the rest of the pattern */
    for (w = 0; TRUE; ) {

	/* If a key is ready, acquire it */
	if (Term_kbhit()) {
	    i = Term_inkey();
	    pat[n++] = i;
	    pat[n] = '\0';
	    w = 0;
	}
	
	/* Wait for it */
	else {
	    if (w > 30) break;
	    delay(++w);
	}	
    }

    /* Hide cursor if needed */    
    if (fix) Term_hide_cursor();


    /* Hack -- verify "normal" characters */
    if ((n == 1) && (pat[0] >= 32) && (pat[0] < 127)) {
	if (!get_check("Are you sure you want to redefine that?")) {
	    prt("Macro cancelled.", 0, 0);
	    return;
	}
    }
    

    /* Convert the trigger to text */
    ascii_to_text(tmp, pat);

    /* Flush the input */
    flush();

    /* Verify */
    msg_print(format("Macro trigger '%s' accepted.", tmp));
    msg_print("Enter an encoded action.");
    msg_print(NULL);


    /* Prompt for the action */
    prt("Action: ", 0, 0);

    /* Get the encoded action */
    if (askfor(tmp, 70)) {

	/* Analyze the string */
	text_to_ascii(act, tmp);
	
	/* Add the macro */
	macro(pat, act);
    
	/* Clear the line */
	prt("Macro accepted.", 0, 0);    
    }

    /* Cancel */
    else {

	/* Cancelled */
	prt("Macro cancelled.", 0, 0);
    }
}

#endif


/*
 * Hack -- allow the system to "strip" special undefined macros
 *
 * These macros are all introduced by "control-underscore" and
 * followed by a series of "normal" keys, and terminated by return.
 */
static void do_cmd_strip(void)
{
    int i;

    /* Strip "normal" keys, plus a "terminator" */
    for (i = 32; i >= 32; i = inkey());
}


/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void process_command(void)
{
    /* Parse the command */
    switch (command_cmd) {

	/* (ESC) do nothing. */
	case ESCAPE:
	    free_turn_flag = TRUE; break;

	/* (SPACE) do nothing */
	case ' ':
	    free_turn_flag = TRUE; break;


	 
	/*** Wizard Commands ***/
	    
	/* Toggle Wizard Mode */
	case CTRL('W'):
	    free_turn_flag = TRUE;
	    if (wizard) {
		wizard = FALSE;
		msg_print("Wizard mode off.");
	    }
	    else if (enter_wiz_mode()) {
		msg_print("Wizard mode on.");
	    }
	    else {
		msg_print("You are not allowed to do that.");
	    }
	    prt_winner();
	    break;

	/* Special Wizard Command */
	case CTRL('A'):
	    if (wizard) {
		do_wiz_command();
	    }
	    else {
		msg_print("You are not allowed to do that.");
	    }
	    break;

	/* Interact with the Borg */
	case CTRL('Z'):
	    free_turn_flag = TRUE;
#ifdef AUTO_PLAY
	    Borg_mode();
#endif
	    break;


	/*** Extra commands ***/

	case ':':
	    free_turn_flag = TRUE;
	    do_cmd_note();
	    break;	    

	case '@':
	    free_turn_flag = TRUE;
#ifdef ALLOW_MACROS
	    do_cmd_macro();
#endif
	    break;

	case '!':
	    free_turn_flag = TRUE;
#ifdef ALLOW_KEYMAP
	    do_cmd_keymap();
#endif
	    break;

	case CTRL('_'):
	    free_turn_flag = TRUE;
	    do_cmd_strip();
	    break;


	/*** Inventory Commands ***/

	/* Wear or wield something */
	case '[':
	    inven_command('w'); break;

	/* Take something off */
	case ']':
	    inven_command('t'); break;

#if 0
	/* Exchange primary and aux weapons */
	case 'x':
	    inven_command('x'); break;
#endif

	/* Drop something */
	case 'd':
	    inven_command('d'); break;

	/* Equipment list */
	case 'e':
	    inven_command('e'); break;

	/* Inventory */
	case 'i':
	    inven_command('i'); break;


	/*** Standard "Movement" Commands ***/

	/* Dig a tunnel */
	case '+':
	    do_cmd_tunnel(); break;

	/* Move without picking anything up */
	case '-':
	    do_cmd_walk(FALSE); break;

	/* Move (picking up) */
	case ';':
	    do_cmd_walk(TRUE); break;


	/*** Commands that "re-interpret" the repeat count ***/

	/* Begin Running -- Arg is Max Distance */
	case '.':
	    do_cmd_run(); break;

	/* Stay still, and, if repeated, Begin Resting -- Arg is time */
	case ',':
	    do_cmd_stay(TRUE);
	    if (command_arg > 1) do_cmd_rest();
	    break;

	/* Begin Resting -- Arg is time */
	case 'R':
	    do_cmd_rest(); break;



	/*** Searching, Resting ***/

	/* Pick up an object */
	case 'g':
	    do_cmd_pick_up(); break;

	/* Toggle search status */
	case '#':
	    do_cmd_toggle_search(); break;

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

	/* Inscribe an object */
	case '{':
	    do_cmd_inscribe(); break;

	/* Inscribe an object (in a different way) */
	case '}':
	    do_cmd_inscribe(); break;

	/* Activate an artifact */
	case 'A':
	    do_cmd_activate(); break;

	/* Eat some food */
	case 'E':
	    do_cmd_eat_food(); break;

	/* Fill the lamp */
	case 'F':
	    do_cmd_refill(); break;

	/* Throw something */
	case 'f':
	    do_cmd_fire(); break;

	/* Zap a wand */
	case 'a':
	    do_cmd_aim_wand(); break;

	/* Activate a rod */
	case 'z':
	    do_cmd_zap_rod(); break;

	/* Quaff a potion */
	case 'q':
	    do_cmd_quaff_potion(); break;

	/* Read a scroll */
	case 'r':
	    do_cmd_read_scroll(); break;

	/* Zap a staff */
	case 'u':
	    do_cmd_use_staff(); break;


	/*** Looking at Things (nearby or on map) ***/

	/* Full screen Map */
	case 'M':
	    do_cmd_view_map(); break;

	/* Locate player on the map */	
	case 'W':
	    do_cmd_locate(); break;

	/* Examine surroundings */
	case 'l':
	    do_cmd_look(); break;

	/* Attempt to select a new target, if compiled */
	case '*':
	    do_cmd_target(); break;



	/*** Help and Such ***/

	/* Help */
	case '?':
	    do_cmd_help(NULL); break;

	/* Identify Symbol */
	case '/':
	    do_cmd_query_symbol(); break;

	/* Character Description */
	case 'C':
	    do_cmd_change_name(); break;


	/*** System Commands ***/

	/* Game Version */
	case 'V':
	    do_cmd_help(ANGBAND_VERSION); break;

	/* Repeat Feeling */
	case CTRL('F'):
	    do_cmd_feeling(); break;

	/* Previous message(s). */
	case CTRL('P'):
	    do_cmd_messages(); break;

	/* Commit Suicide and Quit */
	case 'Q':
	    do_cmd_suicide(); break;

	/* Save and Quit */
	case CTRL('X'):
	    exit_game(); break;

	/* Redraw the screen */
	case CTRL('R'):
	    do_cmd_redraw(); break;

	/* Set options */
	case '=':
	    do_cmd_options(); break;

	/* Check artifacts */
	case '~':
	    do_cmd_check_artifacts(); break;

	/* Check uniques */
	case '|':
	    do_cmd_check_uniques(); break;


	/* Hack -- Unknown command */
	default:
	    free_turn_flag = TRUE;
	    prt("Type '?' for help.", 0, 0);
	    return;
    }


    /* Save the command */
    command_old = command_cmd;
}





/*
 * XXX An explanation of the "Angband Keypress" concept. XXX
 *
 * Inherently, many Angband commands consist not only of a basic action
 * (such as "tunnel"), but also a direction (such as "north-east"), and
 * even a "repeat" count or similar argument (such as a "max run distance").
 *
 * These components are thus explicitly represented, with globals.
 *
 * The "base command" (see below) is stored in "command_cmd"
 * The "desired direction" is stored in "command_dir".
 * The "repeat count" is stored in "command_rep"
 * The "numerical argument" is stored in "command_arg"
 *
 * When "command_dir" is set, it overrides all calls to "get*dir()"
 * So we always reset command_dir to -1 before getting a new command.
 * Hack -- a "command_dir" of "zero" means "the current target".
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
 * The last command successfully executed is stored in "command_old".
 *
 * Eventually, "command_esc" will allow one to track low level "Escapes".
 *
 * The functions below this comment encode conversions from the traditional
 * "roguelike" and "original" command sets into the new "Angband Commands"
 * (command_cmd + command_dir, etc).  Note that the "Angband Chars" bear a
 * striking resemblance to the "original commands", with special keys for
 * "walk", "jump", "run", "tunnel", "stay", and a few others.
 *
 *   "Walk" is encoded as ";" plus a direction.
 *   "Jump" is encoded as "-" plus a direction
 *   "Tunnel" is encoded as "+" plus a direction
 *   "Run" is encoded as "." plus a direction
 *   "Stay" is encoded as "," (with no direction)
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
 * mapping from keypress to command.  Hmmm...  Also note that there are
 * now a few "extra" commands, such as "[" and "]" for wear/remove so that
 * we can eliminate some annoying problems from "inven_command()".
 */



/*
 * Check whether this command can be "repeated".
 *
 * Note -- this routine applies ONLY to "Angband Commands".
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
	case '+': /* Tunnel */
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
	case '.': /* Run */
	case ',': /* Stay */
	case 'R': /* Rest */

	/* Hack -- All Wizard Commands */
	case CTRL('A'): /* Special Wizard Command */
	    return TRUE;
    }

    /* Assume no count allowed */
    return (FALSE);
}



/*
 * Apply a keypress to extract "command_cmd" and "command_dir"
 */
static void keymap_apply(char cmd)
{
    int k = (byte)(cmd);
    
    /* Hack -- prevent illegal commands */
    if (k >= 128) k = 0;
    
    /* Access the array info */
    command_cmd = keymap_cmds[k];
    command_dir = keymap_dirs[k];

    /* Hack -- notice "undefined" commands */
    if (!command_cmd) command_cmd = ESCAPE;
    
    /* Hack -- extract "non-directions" if needed */
    if (command_dir > 9) command_dir = -1;
}


/*
 * Request a command from the user.
 *
 * Sets command_cmd, command_dir, command_rep, command_arg.
 *
 * Note that "caret" ("^") is treated special, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 */
void request_command(void)
{
    int i = 0;
    char cmd;


    /* Notice changes in the "rogue_like_commands" flag */
    static old_rogue_like = -1;
    
    /* Hack -- notice changes in "rogue_like_commands" */
    if (old_rogue_like != rogue_like_commands) keymap_init();
    
    /* Save the "rogue_like_commands" setting */
    old_rogue_like = rogue_like_commands;
    

    /* Hack -- Assume no abortion yet */
    command_esc = 0;


    /* Hack -- Illuminate the player */
    move_cursor_relative(char_row, char_col);


    /* Hack -- process "repeated" commands */
    if (command_rep > 0) {

	/* Count this execution */
	command_rep--;

	/* Hack -- Flush the output */
	Term_fresh();

	/* Hack -- Assume messages were seen */
	msg_flag = FALSE;

	/* All done */
	return;
    }


    /* No command yet */
    command_cmd = 0;

    /* No "argument" yet */
    command_arg = 0;

    /* Hack -- no direction yet */
    command_dir = -1;



    /* Hack -- auto-commands */
    if (command_new) {
	prt("", 0, 0);
	cmd = command_new;
	command_new = 0;
    }
    
    /* Get a keypress in "command" mode */
    else {
	msg_flag = FALSE;
	inkey_flag = TRUE;
	cmd = inkey();
	inkey_flag = FALSE;
    }


    /* Analyze the keypress */
    keymap_apply(cmd);

    /* Special command -- Get a "count" for another command */
    if (command_cmd == '0') {

	/* Begin the input */
	prt("Repeat count:", 0, 0);

	/* Get a command count */
	while (1) {

	    /* Get a new keypress */
	    cmd = inkey();

	    /* Simple editing */
	    if (cmd == DELETE || cmd == CTRL('H')) {
		i = i / 10;
		prt(format("Repeat count: %d", i), 0, 0);
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
		    prt(format("Repeat count: %d", i), 0, 0);
		}
	    }

	    /* Exit on "unusable" input */
	    else {
		break;
	    }
	}

	/* Let a "non-count" default to 99 repetitions */
	if (i == 0) {
	    i = 99;
	    prt(format("Repeat count: %d", i), 0, 0);
	}

	/* Hack -- white-space means "enter command now" */
	if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) {
	    if (!get_com("Command:", &cmd)) cmd = ESCAPE;
	}

	/* Analyze the keypress */
	keymap_apply(cmd);
    }

    /* Hack -- allow "control chars" to be entered */
    if (command_cmd == '^') {

	/* Get a char to "cast" into a control char */
	if (!get_com("Control-", &cmd)) cmd = ESCAPE;

	/* Hack -- create a control char if legal */
	else if (CTRL(cmd)) cmd = CTRL(cmd);

	/* Analyze the keypress */
	keymap_apply(cmd);
    }


    /* Move cursor to player char again, in case it moved */
    move_cursor_relative(char_row, char_col);

    /* Hack -- let some commands get repeated by default */
    if (always_repeat && (i <= 0)) {

	/* Hack -- Tunnel gets 99 tries */
	if (command_cmd == '+') i = 99;
	
	/* Bash, Disarm, Open, Search get 99 tries */
	else if (strchr("BDos", command_cmd)) i = 99;
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
    
    /* Hack -- erase the message line. */
    prt("", 0, 0);

    /* Hack -- Re-Illuminate the player */
    move_cursor_relative(char_row, char_col);
}



