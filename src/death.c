/*
 * File: death.c
 * Purpose: Handle the UI bits that happen after the character dies.
 *
 * Copyright (c) 1987 - 2007 Angband contributors
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "ui-menu.h"



/*
 * Centers a string within a 31 character string
 */
static void center_string(char *buf, size_t len, cptr str)
{
	int i, j;

	/* Total length */
	i = strlen(str);

	/* Necessary border */
	j = 15 - i / 2;

	/* Mega-Hack */
	strnfmt(buf, len, "%*s%s%*s", j, "", str, 31 - i - j, "");
}





/*
 * Hack - save the time of death
 */
static time_t death_time = (time_t)0;


/*
 * Display a "tomb-stone"
 */
static void print_tomb(void)
{
	cptr p;

	char tmp[160];
	char buf[1024];

	ang_file *fp;

	int line = 7;


	/* Clear screen */
	Term_clear();

	/* Open the death file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "dead.txt");
	fp = file_open(buf, MODE_READ, -1);

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
			put_str(buf, i++, 0);

		file_close(fp);
	}


	/* Get the right total */
	if (p_ptr->total_winner)
		p = "Magnificent";
	else
		p = c_text + cp_ptr->title[(p_ptr->lev - 1) / 5];

	center_string(buf, sizeof(buf), op_ptr->full_name);
	put_str(buf, line++, 8);

	center_string(buf, sizeof(buf), "the");
	put_str(buf, line++, 8);

	center_string(buf, sizeof(buf), p);
	put_str(buf, line++, 8);

	line++;

	center_string(buf, sizeof(buf), c_name + cp_ptr->name);
	put_str(buf, line++, 8);

	strnfmt(tmp, sizeof(tmp), "Level: %d", (int)p_ptr->lev);
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);

	strnfmt(tmp, sizeof(tmp), "Exp: %ld", (long)p_ptr->exp);
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);

	strnfmt(tmp, sizeof(tmp), "AU: %ld", (long)p_ptr->au);
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);

	strnfmt(tmp, sizeof(tmp), "Killed on Level %d", p_ptr->depth);
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);

	strnfmt(tmp, sizeof(tmp), "by %s.", p_ptr->died_from);
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);

	line++;

	strnfmt(tmp, sizeof(tmp), "%-.24s", ctime(&death_time));
	center_string(buf, sizeof(buf), tmp);
	put_str(buf, line++, 8);
}


/*
 * Hack - Know inventory and home items upon death
 */
static void death_knowledge(void)
{
	int i;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];


	/* Hack -- Know everything in the inven/equip */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_KNOWN);
	}

	/* Hack -- Know everything in the home */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		o_ptr = &st_ptr->stock[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_KNOWN);
	}

	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();
}



/*
 * Display the winner file
 */
static void display_winner(void)
{
	char buf[1024];
	char tmp[1024];
	ang_file *fp;

	int wid, hgt;
	int i = 2;
	int width = 0;


	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "crown.txt");
	fp = file_open(buf, MODE_READ, -1);

	Term_clear();
	Term_get_size(&wid, &hgt);

	if (fp)
	{
		/* Get us the first line of file, which tells us how long the */
		/* longest line is */
		file_getl(fp, buf, sizeof(buf));
		sscanf(buf, "%d", &width);
		if (!width) width = 25;

		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
			put_str(buf, i++, (wid/2) - (width/2));

		file_close(fp);
	}

	strnfmt(buf, sizeof(buf), "All Hail the Mighty %s!", sp_ptr->winner);
	center_string(tmp, sizeof(tmp), buf);
	put_str(tmp, i++, (wid/2) - (31/2));

	flush();
	pause_line(Term->hgt - 1);
}







static void death_file(void *unused, const char *title)
{
	char ftmp[80];
	strnfmt(ftmp, sizeof(ftmp), "%s.txt", op_ptr->base_name);

	if (!get_string("File name: ", ftmp, sizeof(ftmp)))
		return;

	if (ftmp[0] && (ftmp[0] != ' '))
	{
		errr err;

		/* Dump a character file */
		screen_save();
		err = file_character(ftmp, FALSE);
		screen_load();

		/* Check result */
		if (err)
			msg_print("Character dump failed!");
		else
			msg_print("Character dump successful.");

		/* Flush messages */
		message_flush();
	}
}

static void death_info(void *unused, const char *title)
{
	int i, j, k;
	object_type *o_ptr;
	store_type *st_ptr = &store[STORE_HOME];


	screen_save();

	/* Display player */
	display_player(0);

	/* Prompt for inventory */
	prt("Hit any key to see more information (ESC to abort): ", 23, 0);

	/* Allow abort at this point */
	if (inkey() == ESCAPE) return;


	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (p_ptr->equip_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_equip();
		prt("You are using: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (p_ptr->inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven();
		prt("You are carrying: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}



	/* Home -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Display contents of the home */
		for (k = 0, i = 0; i < st_ptr->stock_num; k++)
		{
			/* Clear screen */
			Term_clear();

			/* Show 12 items */
			for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
			{
				byte attr;

				char o_name[80];
				char tmp_val[80];

				/* Get the object */
				o_ptr = &st_ptr->stock[i];

				/* Print header, clear line */
				strnfmt(tmp_val, sizeof(tmp_val), "%c) ", I2A(j));
				prt(tmp_val, j+2, 4);

				/* Get the object description */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

				/* Get the inventory color */
				attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

				/* Display the object */
				c_put_str(attr, o_name, j+2, 7);
			}

			/* Caption */
			prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

			/* Wait for it */
			if (inkey() == ESCAPE) return;
		}
	}

	screen_load();
}

static void death_messages(void *unused, const char *title)
{
	screen_save();
	do_cmd_messages();
	screen_load();
}

static void death_scores(void *unused, const char *title)
{
	screen_save();
	top_twenty();
	screen_load();
}

static void death_examine(void *unused, const char *title)
{
	int item;
	cptr q, s;


	screen_save();
	Term_clear();

	/* Start out in "display" mode */
	p_ptr->command_see = TRUE;

	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";

	while (get_item(&item, q, s, (USE_INVEN | USE_EQUIP)))
	{
		object_type *o_ptr = &inventory[item];

		/* "Know" */
		o_ptr->ident |= IDENT_KNOWN;

		/* Describe */
		text_out_hook = text_out_to_screen;
		screen_save();
		Term_gotoxy(0, 0);

		object_info_header(o_ptr);
		if (!object_info_known(o_ptr))
			text_out("This item does not possess any special abilities.");

		text_out_c(TERM_L_BLUE, "\n\n[Press any key to continue]\n");
		(void)anykey();

		screen_load();
	}

	screen_load();
}

menu_type death_menu;

static const menu_action death_actions[] =
{
	{ 'i', "Information",   death_info,     NULL },
	{ 'm', "Messages",      death_messages, NULL },
	{ 'f', "File dump",     death_file,     NULL },
	{ 'v', "View scores",   death_scores,   NULL },
	{ 'x', "Examine items", death_examine,  NULL },
	{ 'q', "Quit",          death_examine,  NULL },
};




static char tag_death_main(menu_type *menu, int oid)
{
	if (death_actions[oid].id)
		return death_actions[oid].id;

	return 0;
}

static int valid_death_main(menu_type *menu, int oid)
{
	if (death_actions[oid].name)
		return 1;

	return 0;
}

static void display_death_main(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

	if (death_actions[oid].name)
		c_prt(attr, death_actions[oid].name, row, col);
}


static const menu_iter death_iter =
{
	0,
	tag_death_main,
	valid_death_main,
	display_death_main,
	NULL
};




/*
 * Handle character death
 */
void death_screen(void)
{
	bool wants_to_quit = FALSE;


	if (p_ptr->total_winner)
	{
		/* Retire in the town in a good state */
		p_ptr->depth = 0;
		my_strcpy(p_ptr->died_from, "Ripe Old Age", sizeof(p_ptr->died_from));
		p_ptr->exp = p_ptr->max_exp;
		p_ptr->lev = p_ptr->max_lev;
		p_ptr->au += 10000000L;

		display_winner();
	}

	/* Save dead player */
	if (new_save ? !save(savefile) : !old_save())
	{
		msg_print("death save failed!");
		message_flush();
	}

	/* Get time of death */
	(void)time(&death_time);

	/* You are dead */
	print_tomb();

	/* Hack - Know everything upon death */
	death_knowledge();

	/* Enter player in high score list */
	enter_score(&death_time);

	/* Flush all input keys */
	flush();

	/* Flush messages */
	message_flush();


	/* Initialize the menus */
	menu_type *menu;
   const char cmd_keys[] = { ARROW_LEFT, ARROW_RIGHT, '\0' };
	const region area = { 51, 2, 0, 6 };

	int cursor = 0;
	ui_event_data c = EVENT_EMPTY;


	/* options screen selection menu */
	menu = &death_menu;
	WIPE(menu, menu_type);
	menu_set_id(menu, 1);
	menu->menu_data = death_actions;
	menu->flags = MN_CASELESS_TAGS;
	menu->cmd_keys = cmd_keys;
	menu->count = N_ELEMENTS(death_actions);

	menu_init2(menu, find_menu_skin(MN_SCROLL), &death_iter, &area);

	while (TRUE)
	{
		c = menu_select(&death_menu, &cursor, 0);

		if (c.key == ESCAPE || cursor == 5)
		{
			if (get_check("Do you want to quit? "))
				break;
		}
		else if (c.type == EVT_SELECT && death_actions[cursor].action)
		{
			death_actions[cursor].action(death_actions[cursor].data, NULL);
		}
	}
}


