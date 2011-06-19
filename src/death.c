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
#include "cmds.h"
#include "files.h"
#include "history.h"
#include "savefile.h"
#include "ui-menu.h"
#include "wizard.h"

/*
 * Hack - save the time of death
 */
static time_t death_time = (time_t)0;



/*
 * Write formatted string `fmt` on line `y`, centred between points x1 and x2.
 */
static void put_str_centred(int y, int x1, int x2, const char *fmt, ...)
{
	va_list vp;
	char *tmp;
	size_t len;
	int x;

	/* Format into the (growable) tmp */
	va_start(vp, fmt);
	tmp = vformat(fmt, vp);
	va_end(vp);

	/* Centre now */
	len = strlen(tmp);
	x = x1 + ((x2-x1)/2 - len/2);

	put_str(tmp, y, x);
}


/*
 * Display the tombstone
 */
static void print_tomb(void)
{
	ang_file *fp;
	char buf[1024];
	int line = 0;


	Term_clear();

	/* Open the death file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "dead.txt");
	fp = file_open(buf, MODE_READ, -1);

	if (fp)
	{
		while (file_getl(fp, buf, sizeof(buf)))
			put_str(buf, line++, 0);

		file_close(fp);
	}

	line = 7;

	put_str_centred(line++, 8, 8+31, "%s", op_ptr->full_name);
	put_str_centred(line++, 8, 8+31, "the");
	if (p_ptr->total_winner)
		put_str_centred(line++, 8, 8+31, "Magnificent");
	else
		put_str_centred(line++, 8, 8+31, "%s", p_ptr->class->title[(p_ptr->lev - 1) / 5]);

	line++;

	put_str_centred(line++, 8, 8+31, "%s", p_ptr->class->name);
	put_str_centred(line++, 8, 8+31, "Level: %d", (int)p_ptr->lev);
	put_str_centred(line++, 8, 8+31, "Exp: %d", (int)p_ptr->exp);
	put_str_centred(line++, 8, 8+31, "AU: %d", (int)p_ptr->au);
	put_str_centred(line++, 8, 8+31, "Killed on Level %d", p_ptr->depth);
	put_str_centred(line++, 8, 8+31, "by %s.", p_ptr->died_from);

	line++;

	put_str_centred(line++, 8, 8+31, "by %-.24s", ctime(&death_time));
}


/*
 * Know inventory and home items upon death
 */
static void death_knowledge(void)
{
	struct store *st_ptr = &stores[STORE_HOME];
	object_type *o_ptr;

	int i;

	for (i = 0; i < ALL_INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (!o_ptr->kind) continue;

		object_flavor_aware(o_ptr);
		object_notice_everything(o_ptr);
	}

	for (i = 0; i < st_ptr->stock_num; i++)
	{
		o_ptr = &st_ptr->stock[i];
		if (!o_ptr->kind) continue;

		object_flavor_aware(o_ptr);
		object_notice_everything(o_ptr);
	}

	history_unmask_unknown();

	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
	handle_stuff(p_ptr);
}



/*
 * Display the winner crown
 */
static void display_winner(void)
{
	char buf[1024];
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

	put_str_centred(i, 0, wid, "All Hail the Mighty %s!", p_ptr->sex->winner);

	flush();
	pause_line(Term);
}


/*
 * Menu command: dump character dump to file.
 */
static void death_file(const char *title, int row)
{
	char buf[1024];
	char ftmp[80];

	strnfmt(ftmp, sizeof(ftmp), "%s.txt", op_ptr->base_name);

	if (get_file(ftmp, buf, sizeof buf))
	{
		errr err;

		/* Dump a character file */
		screen_save();
		err = file_character(buf, FALSE);
		screen_load();

		/* Check result */
		if (err)
			msg("Character dump failed!");
		else
			msg("Character dump successful.");

		/* Flush messages */
		message_flush();
	}
}

/*
 * Menu command: view character dump and inventory.
 */
static void death_info(const char *title, int row)
{
	int i, j, k;
	object_type *o_ptr;
	struct store *st_ptr = &stores[STORE_HOME];


	screen_save();

	/* Display player */
	display_player(0);

	/* Prompt for inventory */
	prt("Hit any key to see more information: ", 0, 0);

	/* Allow abort at this point */
	(void)anykey();


	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (p_ptr->equip_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_equip(OLIST_WEIGHT);
		prt("You are using: -more-", 0, 0);
		(void)anykey();
	}

	/* Inventory -- if any */
	if (p_ptr->inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven(OLIST_WEIGHT);
		prt("You are carrying: -more-", 0, 0);
		(void)anykey();
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
				object_desc(o_name, sizeof(o_name), o_ptr,
							ODESC_PREFIX | ODESC_FULL);

				/* Get the inventory color */
				attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

				/* Display the object */
				c_put_str(attr, o_name, j+2, 7);
			}

			/* Caption */
			prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

			/* Wait for it */
			(void)anykey();
		}
	}

	screen_load();
}

/*
 * Menu command: peruse pre-death messages.
 */
static void death_messages(const char *title, int row)
{
	screen_save();
	do_cmd_messages();
	screen_load();
}

/*
 * Menu command: see top twenty scores.
 */
static void death_scores(const char *title, int row)
{
	screen_save();
	show_scores();
	screen_load();
}

/*
 * Menu command: examine items in the inventory.
 */
static void death_examine(const char *title, int row)
{
	int item;
	const char *q, *s;

	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";

	while (get_item(&item, q, s, 0, (USE_INVEN | USE_EQUIP | IS_HARMLESS)))
	{
		char header[120];

		textblock *tb;
		region area = { 0, 0, 0, 0 };

		object_type *o_ptr = &p_ptr->inventory[item];

		tb = object_info(o_ptr, OINFO_FULL);
		object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

		textui_textblock_show(tb, area, format("%^s", header));
		textblock_free(tb);
	}
}


/*
 * Menu command: view character history.
 */
static void death_history(const char *title, int row)
{
	history_display();
}

/*
 * Menu command: allow spoiler generation (mainly for randarts).
 */
static void death_spoilers(const char *title, int row)
{
	do_cmd_spoilers();
}

/* Menu command: toggle birth_keep_randarts option. */
static void death_randarts(const char *title, int row)
{
	if (p_ptr->randarts)
		option_set(option_name(OPT_birth_keep_randarts),
			get_check("Keep randarts for next game? "));
	else
		msg("You are not playing with randarts!");
}


/*
 * Menu structures for the death menu. Note that Quit must always be the
 * last option, due to a hard-coded check in death_screen
 */
static menu_type *death_menu;
static menu_action death_actions[] =
{
	{ 0, 'i', "Information",   death_info      },
	{ 0, 'm', "Messages",      death_messages  },
	{ 0, 'f', "File dump",     death_file      },
	{ 0, 'v', "View scores",   death_scores    },
	{ 0, 'x', "Examine items", death_examine   },
	{ 0, 'h', "History",       death_history   },
	{ 0, 's', "Spoilers",      death_spoilers  },
	{ 0, 'r', "Keep randarts", death_randarts  },
	{ 0, 'q', "Quit",          NULL            },
};



/*
 * Handle character death
 */
void death_screen(void)
{
	bool done = FALSE;
	const region area = { 51, 2, 0, N_ELEMENTS(death_actions) };

	/* Retire in the town in a good state */
	if (p_ptr->total_winner)
	{
		p_ptr->depth = 0;
		my_strcpy(p_ptr->died_from, "Ripe Old Age", sizeof(p_ptr->died_from));
		p_ptr->exp = p_ptr->max_exp;
		p_ptr->lev = p_ptr->max_lev;
		p_ptr->au += 10000000L;

		display_winner();
	}

	/* Get time of death */
	(void)time(&death_time);
	print_tomb();
	death_knowledge();
	enter_score(&death_time);

	/* Flush all input and output */
	flush();
	message_flush();

	/* Display and use the death menu */
	if (!death_menu)
	{
		death_menu = menu_new_action(death_actions,
				N_ELEMENTS(death_actions));

		death_menu->flags = MN_CASELESS_TAGS;
	}

	menu_layout(death_menu, &area);

	while (!done)
	{
		ui_event e = menu_select(death_menu, EVT_KBRD, FALSE);
		if (e.type == EVT_KBRD)
		{
			if (e.key.code == KTRL('X')) break;
		}
		else if (e.type == EVT_SELECT)
		{
			done = get_check("Do you want to quit? ");
		}
	}

	/* Save dead player */
	if (!savefile_save(savefile))
	{
		msg("death save failed!");
		message_flush();
	}
}
