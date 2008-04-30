/*
 * File: squelch.c
 * Purpose: Item destruction
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * David Vestal, Pete Mack, Andrew Sidwell.
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
#include "ui-menu.h"

/*
 * The squelch code has a long history.  Originally it started out as a simple
 * sval-dependent item destroyer, but then ego-item and quality squelch was
 * added too, and then squelched items on the dungeon floor were marked by
 * purple dots, and by this time the code was quite unmaintainable and pretty
 * much impossible to work with.
 *
 * Luckily, though, it's been cleaned up.  There is now only sval-dependent
 * squelch and quality-based squelch, and the two don't interact -- quality-based
 * is for items that get pseudo-id'd and sval-dependent is for potions and the
 * like.
 *
 * The squelch code figures most things out itself.  Simply do:
 *     p_ptr->notice |= PN_SQUELCH;
 * whenever you want to make the game check for squelched items.
 *
 * The quality-dependent squelch is much reduced in scope from how it used to
 * be.  If less "general" settings are desired, they can be added easily enough
 * by changing entries in type_tvals[][], adding more TYPE_* constants, and
 * updating type_names.  Savefile compatibility is automatically ensured.
 *
 *
 * The UI code is much cleaner than it was before, but the interface itself
 * still needs some design.  XXX
 */


/*
 * List of kinds of item, for pseudo-id squelch.
 */
enum
{
	TYPE_WEAPON,
	TYPE_SHOOTER,
	TYPE_MISSILE,
	TYPE_ARMOR,
	TYPE_JEWELRY,
	TYPE_DIGGER,

	TYPE_MAX
};

/*
 * Names of categories.
 */
static const char *type_names[TYPE_MAX] =
{
	"Melee weapons",
	"Missile weapons",
	"Ammunition",
	"Armor",
	"Jewelry",
	"Diggers",
};

/* Mapping of tval -> type */
static int type_tvals[][2] =
{
	{ TYPE_WEAPON,	TV_SWORD },
	{ TYPE_WEAPON,	TV_POLEARM },
	{ TYPE_WEAPON,	TV_HAFTED },
	{ TYPE_SHOOTER,	TV_BOW },
	{ TYPE_MISSILE, TV_ARROW },
	{ TYPE_MISSILE,	TV_BOLT },
	{ TYPE_MISSILE,	TV_SHOT },
	{ TYPE_ARMOR,	TV_SHIELD },
	{ TYPE_ARMOR,	TV_HELM },
	{ TYPE_ARMOR,	TV_GLOVES },
	{ TYPE_ARMOR,	TV_BOOTS },
	{ TYPE_ARMOR,	TV_HARD_ARMOR },
	{ TYPE_ARMOR,	TV_SOFT_ARMOR },
	{ TYPE_ARMOR,	TV_CLOAK },
	{ TYPE_ARMOR,	TV_CROWN },
	{ TYPE_JEWELRY,	TV_RING },
	{ TYPE_JEWELRY,	TV_AMULET },
	{ TYPE_DIGGER,	TV_DIGGING },
};

byte squelch_level[TYPE_MAX];
size_t squelch_size = TYPE_MAX;


/*
 * The different kinds of quality squelch
 */
enum
{
	SQUELCH_NONE,
	SQUELCH_BAD,
	SQUELCH_AVERAGE,
	SQUELCH_GOOD,
	SQUELCH_EXCELLENT,
	SQUELCH_ALL,

	SQUELCH_MAX
};

/*
 * The names for the various kinds of quality
 */
static const char *quality_names[SQUELCH_MAX] =
{
	"none",                        /* SQUELCH_NONE */
	"bad",                         /* SQUELCH_BAD */
	"average",                     /* SQUELCH_AVERAGE */
	"good",                        /* SQUELCH_GOOD */
	"excellent",                   /* SQUELCH_EXCELLENT */
	"everything except artifacts", /* SQUELCH_ALL */
};


/* Structure to describe tval/description pairings. */
typedef struct
{
	int tval;
	const char *desc;
} tval_desc;

/* Categories for sval-dependent squelch. */
static tval_desc sval_dependent[] =
{
	{ TV_STAFF,			"Staffs" },
	{ TV_WAND,			"Wands" },
	{ TV_ROD,			"Rods" },
	{ TV_SCROLL,		"Scrolls" },
	{ TV_POTION,		"Potions" },
	{ TV_RING,			"Rings" },
	{ TV_AMULET,		"Amulets" },
	{ TV_FOOD,			"Food" },
	{ TV_MAGIC_BOOK,	"Magic books" },
	{ TV_PRAYER_BOOK,	"Prayer books" },
	{ TV_SPIKE,			"Spikes" },
	{ TV_LITE,			"Lights" },
	{ TV_FLASK,			"Flasks of oil" },
	{ TV_DRAG_ARMOR,	"Dragon Mail Armor" },
};


/*** Autoinscription stuff ***/

/*
 * This code needs documenting.
 */
int get_autoinscription_index(s16b k_idx)
{
	int i;

	for (i = 0; i < inscriptions_count; i++)
	{
		if (k_idx == inscriptions[i].kind_idx)
			return i;
	}

	return -1;
}

/*
 * DOCUMENT ME!
 */
const char *get_autoinscription(s16b kind_idx)
{
	int i;

	for (i = 0; i < inscriptions_count; i++)
	{
		if (kind_idx == inscriptions[i].kind_idx)
			return quark_str(inscriptions[i].inscription_idx);
	}

	return 0;
}

/* Put the autoinscription on an object */
int apply_autoinscription(object_type *o_ptr)
{
	char o_name[80];
	cptr note = get_autoinscription(o_ptr->k_idx);
	cptr existing_inscription = quark_str(o_ptr->note);

	/* Don't inscribe unaware objects */
	if (!note || !object_aware_p(o_ptr))
		return 0;

	/* Don't re-inscribe if it's already correctly inscribed */
	if (existing_inscription && streq(note, existing_inscription))
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, ODESC_FULL);

	if (note[0] != 0)
		o_ptr->note = quark_add(note);
	else
		o_ptr->note = 0;

	msg_format("You autoinscribe %s.", o_name);

	return 1;
}


int remove_autoinscription(s16b kind)
{
	int i = get_autoinscription_index(kind);

	/* It's not here. */
	if (i == -1) return 0;

	while (i < inscriptions_count - 1)
	{
		inscriptions[i] = inscriptions[i+1];
		i++;
	}

	inscriptions_count--;

	return 1;
}


int add_autoinscription(s16b kind, cptr inscription)
{
	int index;

	/* Paranoia */
	if (kind == 0) return 0;

	/* If there's no inscription, remove it */
	if (!inscription || (inscription[0] == 0))
		return remove_autoinscription(kind);

	index = get_autoinscription_index(kind);

	if (index == -1)
		index = inscriptions_count;

	if (index >= AUTOINSCRIPTIONS_MAX)
	{
		msg_format("This inscription (%s) cannot be added because the inscription array is full!", inscription);
		return 0;
	}

	inscriptions[index].kind_idx = kind;
	inscriptions[index].inscription_idx = quark_add(inscription);

	/* Only increment count if inscription added to end of array */
	if (index == inscriptions_count)
		inscriptions_count++;

	return 1;
}


void autoinscribe_ground(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	s16b this_o_idx, next_o_idx = 0;

	/* Scan the pile of objects */
	for (this_o_idx = cave_o_idx[py][px]; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Get the next object */
		next_o_idx = o_list[this_o_idx].next_o_idx;

		/* Apply an autoinscription */
		apply_autoinscription(&o_list[this_o_idx]);
	}
}

void autoinscribe_pack(void)
{
	int i;

	/* Cycle through the inventory */
	for (i = INVEN_PACK; i > 0; i--)
	{
		/* Skip empty items */
		if (!inventory[i].k_idx) continue;

		/* Apply the inscription */
		apply_autoinscription(&inventory[i]);
	}

	return;
}




/*** Squelch code ***/

/*
 * Determines whether a tval is eligable for sval-squelch.
 */
bool squelch_tval(int tval)
{
	size_t i;

	/* Only squelch if the tval's allowed */
	for (i = 0; i < N_ELEMENTS(sval_dependent); i++)
	{
		if (tval == sval_dependent[i].tval)
			return TRUE;
	}

	return FALSE;
}


/*
 * Determines if an object is eligable for squelching.
 */
bool squelch_item_ok(const object_type *o_ptr)
{
	size_t i;
	int num = -1;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	bool fullid = object_known_p(o_ptr);
	bool sensed = (o_ptr->ident & IDENT_SENSE) || fullid;
	byte feel   = fullid ? object_pseudo_heavy(o_ptr) : o_ptr->pseudo;


	/* Don't squelch artifacts */
	if (artifact_p(o_ptr)) return FALSE;

	/* Don't squelch stuff inscribed not to be destroyed (!k) */
	if (check_for_inscrip(o_ptr, "!k") || check_for_inscrip(o_ptr, "!*"))
	{
		return FALSE;
	}

	/* Auto-squelch dead chests */
	if (o_ptr->tval == TV_CHEST && o_ptr->pval == 0)
		return TRUE;

	/* Do squelching by sval, if we 'know' the flavour. */
	if (k_ptr->squelch && (k_ptr->flavor == 0 || k_ptr->aware))
	{
		if (squelch_tval(k_info[o_ptr->k_idx].tval))
			return TRUE;
	}


	/* Don't check pseudo-ID for nonsensed things */
	if (!sensed) return FALSE;



	/* Find the appropriate squelch group */
	for (i = 0; i < N_ELEMENTS(type_tvals); i++)
	{
		if (type_tvals[i][1] == o_ptr->tval)
		{
			num = type_tvals[i][0];
			break;
		}
	}

	/* Never squelched */
	if (num == -1)
		return FALSE;


	/* Get result based on the feeling and the squelch_level */
	switch (squelch_level[num])
	{
		case SQUELCH_BAD:
		{
			if ((feel == INSCRIP_TERRIBLE) ||
			    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED))
			{
				return TRUE;
			}

			if ((feel != INSCRIP_AVERAGE) && fullid &&
				 (o_ptr->to_a <= 0 && o_ptr->to_h <= 0 && o_ptr->to_d <= 0))
				return TRUE;

			break;
		}

		case SQUELCH_AVERAGE:
		{
			if ((feel == INSCRIP_TERRIBLE) ||
			    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE))
			{
				return TRUE;
			}

			if ((feel != INSCRIP_AVERAGE) && fullid &&
				 (o_ptr->to_a <= 0 && o_ptr->to_h <= 0 && o_ptr->to_d <= 0))
				return TRUE;

			break;
		}

		case SQUELCH_GOOD:
		{
			if ((feel == INSCRIP_TERRIBLE) ||
			    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE) || (feel == INSCRIP_MAGICAL))
			{
				return TRUE;
			}

			if (fullid && !o_ptr->name2 && !o_ptr->name1 &&
				 (o_ptr->to_a >= 0 && o_ptr->to_h >= 0 && o_ptr->to_d >= 0))
				return TRUE;

			break;
		}

		case SQUELCH_EXCELLENT:
		{
			if ((feel == INSCRIP_TERRIBLE) ||
			    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE) || (feel == INSCRIP_EXCELLENT))
			{
				return TRUE;
			}
		}

		case SQUELCH_ALL:
		{
			return TRUE;
			break;
		}
	}

	/* Failure */
	return FALSE;
}


/*
 * Returns TRUE if an item should be hidden due to the player's
 * current settings.
 */
bool squelch_hide_item(object_type *o_ptr)
{
	return (hide_squelchable ? squelch_item_ok(o_ptr) : FALSE);
}


/*
 * Destroy all {squelch}able items.
 *
 * Imported, with thanks, from Ey... much cleaner than the original.
 */
void squelch_items(void)
{
	int floor_list[MAX_FLOOR_STACK];
	int floor_num, n;
	int count = 0;

	object_type *o_ptr;

	/* Set the hook and scan the floor */
	item_tester_hook = squelch_item_ok;
	floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), p_ptr->py, p_ptr->px, 0x01);

	if (floor_num)
	{
		for (n = 0; n < floor_num; n++)
		{
			o_ptr = &o_list[floor_list[n]];

			/* Avoid artifacts */
			if (artifact_p(o_ptr)) continue;

			if (item_tester_okay(o_ptr))
			{
				/* Destroy item */
				floor_item_increase(floor_list[n], -o_ptr->number);
				floor_item_optimize(floor_list[n]);
				count++;
			}
		}
	}

	/* Scan through the slots backwards */
	for (n = INVEN_PACK - 1; n >= 0; n--)
	{
		o_ptr = &inventory[n];

		/* Skip non-objects and artifacts */
		if (!o_ptr->k_idx) continue;
		if (artifact_p(o_ptr)) continue;

		if (item_tester_okay(o_ptr))
		{
			/* Destroy item */
			inven_item_increase(n, -o_ptr->number);
			inven_item_optimize(n);
			count++;
		}
	}

	item_tester_hook = NULL;

	/* Mention casualties */
	if (count > 0)
	{
		message_format(MSG_DESTROY, 0, "%d item%s squelched.",
		               count, ((count > 1) ? "s" : ""));

		/* Combine/reorder the pack */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	}
}


/*
 * Drop all {squelch}able items.
 */
void squelch_drop(void)
{
	int n;

	/* Scan through the slots backwards */
	for (n = INVEN_PACK - 1; n >= 0; n--)
	{
		object_type *o_ptr = &inventory[n];

		/* Skip non-objects and unsquelchable objects */
		if (!o_ptr->k_idx) continue;
		if (!squelch_item_ok(o_ptr)) continue;

		/* Check for !d (no drop) inscription */
		if (!check_for_inscrip(o_ptr, "!d") && !check_for_inscrip(o_ptr, "!*"))
		{
			/* We're allowed to drop it. */
			inven_drop(n, o_ptr->number);
		}
	}

	/* Combine/reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
}



/*** Quality-squelch menu ***/

/*
 * Display an entry in the menu.
 */
static void quality_display(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	const char *name = type_names[oid];

	byte level = squelch_level[oid];
	const char *level_name = quality_names[level];

	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);


	c_put_str(attr, format("%-20s : %s", name, level_name), row, col);
}


/*
 * Display the quality squelch subtypes.
 */
static void quality_subdisplay(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	const char *name = quality_names[oid];
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);

	c_put_str(attr, name, row, col);
}

/*
 * Handle "Enter".  :(
 */
static bool quality_subaction(char cmd, void *db, int oid)
{
	return TRUE;
}


/*
 * Handle keypresses.
 */
static bool quality_action(char cmd, void *db, int oid)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, quality_subdisplay, quality_subaction };
	region area = { 24, 5, 26, SQUELCH_MAX };
	ui_event_data evt;
	int cursor;

	/* Display at the right point */
	area.row += oid;
	cursor = squelch_level[oid];

	/* Save */
	screen_save();

	/* Run menu */
	WIPE(&menu, menu);
	menu.cmd_keys = "\n\r";
	menu.count = SQUELCH_MAX;
	if (oid == TYPE_JEWELRY)
		menu.count = area.page_rows = SQUELCH_BAD + 1;

	menu_init(&menu, MN_SKIN_SCROLL, &menu_f, &area);
	window_make(area.col - 2, area.row - 1, area.col + area.width + 2, area.row + area.page_rows);

	evt = menu_select(&menu, &cursor, 0);

	/* Set the new value appropriately */
	if (evt.key != ESCAPE && evt.type != EVT_BACK)
		squelch_level[oid] = cursor;

	/* Load and finish */
	screen_load();
	return TRUE;
}

/*
 * Display quality squelch menu.
 */
static void quality_menu(void *unused, const char *also_unused)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, quality_display, quality_action };
	region area = { 1, 5, -1, -1 };
	ui_event_data evt = EVENT_EMPTY;
	int cursor = 0;

	/* Save screen */
	screen_save();
	clear_from(0);

	/* Help text */
	prt("Quality squelch menu", 0, 0);

	Term_gotoxy(1, 1);
	text_out_to_screen(TERM_L_RED, "Use the movement keys to navigate, and Enter to change settings.");

	/* Set up the menu */
	WIPE(&menu, menu);
	menu.cmd_keys = " \n\r";
	menu.count = TYPE_MAX;
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f, &area);

	/* Select an entry */
	while (evt.key != ESCAPE)
		evt = menu_select(&menu, &cursor, 0);

	/* Load screen */
	screen_load();
	return;
}



/*** Sval-dependent menu ***/

/*
 * Display an entry on the sval menu
 */
static void sval_display(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	char buf[80];
	const u16b *choice = menu->menu_data;
	int idx = choice[oid];

	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);


	/* Acquire the "name" of object "i" */
	object_kind_name(buf, sizeof(buf), idx, TRUE);

	/* Print it */
	c_put_str(attr, format("[ ] %s", buf), row, col);
	if (k_info[idx].squelch)
		c_put_str(TERM_L_RED, "*", row, col + 1);
}

/*
 * Deal with events on the sval menu
 */
static bool sval_action(char cmd, void *db, int oid)
{
	u16b *choice = db;

	/* Toggle */
	if (cmd == '\n' || cmd == '\r')
	{
		int idx = choice[oid];
		k_info[idx].squelch = !k_info[idx].squelch;

		return TRUE;
	}

	return FALSE;
}


/*
 * Display list of svals to be squelched.
 */
static bool sval_menu(int tval, const char *desc)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, sval_display, sval_action };
	region area = { 1, 5, -1, -1 };
	ui_event_data evt = { EVT_NONE, 0, 0, 0, 0 };
	int cursor = 0;

	int num = 0;
	size_t i;

	u16b *choice;


	/* Create the array */
	choice = C_ZNEW(z_info->k_max, u16b);

	/* Iterate over all possible object kinds, finding ones which can be squelched */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!k_ptr->name) continue;
		if (!k_ptr->everseen) continue;
		if (k_ptr->tval != tval) continue;

		/* Add this item to our possibles list */
		choice[num++] = i;
	}

	/* Return here if there are no objects */
	if (!num)
	{
		FREE(choice);
		return FALSE;
	}


	/* Save the screen and clear it */
	screen_save();
	clear_from(0);

	/* Help text */

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Indent output */
	text_out_indent = 1;
	text_out_wrap = 79;
	Term_gotoxy(1, 0);

	/* Display some helpful information */
	text_out("Use the ");
	text_out_c(TERM_L_GREEN, "movement keys");
	text_out(" to scroll the list or ");
	text_out_c(TERM_L_GREEN, "ESC");
	text_out(" to return to the previous menu.  ");
	text_out_c(TERM_L_BLUE, "Enter");
	text_out(" toggles the current setting.");

	text_out_indent = 0;

	/* Set up the menu */
	WIPE(&menu, menu);
	menu.cmd_keys = " \n\r";
	menu.count = num;
	menu.menu_data = choice;
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f, &area);

	/* Select an entry */
	while (evt.key != ESCAPE)
		evt = menu_select(&menu, &cursor, 0);

	/* Free memory */
	FREE(choice);

	/* Load screen */
	screen_load();
	return TRUE;
}


/* Returns TRUE if there's anything to display a menu of */
static bool seen_tval(int tval)
{
	int i;

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!k_ptr->name) continue;
		if (!k_ptr->everseen) continue;
		if (k_ptr->tval != tval) continue;

		 return TRUE;
	}


	return FALSE;
}


/* Extra options on the "item options" menu */
struct
{
	char tag;
	char *name;
	void (*action)(void *unused, const char *also_unused);
} extra_item_options[] =
{
	{ 'Q', "Quality squelching options", quality_menu },
	{ '{', "Autoinscription setup", do_cmd_knowledge_objects },
};

static char tag_options_item(menu_type *menu, int oid)
{
	size_t line = (size_t) oid;

	if (line < N_ELEMENTS(sval_dependent))
		return I2A(oid);

	/* Separator - blank line. */
	if (line == N_ELEMENTS(sval_dependent))
		return 0;

	line = line - N_ELEMENTS(sval_dependent) - 1;

	if (line < N_ELEMENTS(extra_item_options))
		return extra_item_options[line].tag;

	return 0;
}

static int valid_options_item(menu_type *menu, int oid)
{
	size_t line = (size_t) oid;

	if (line < N_ELEMENTS(sval_dependent))
		return 1;

	/* Separator - blank line. */
	if (line == N_ELEMENTS(sval_dependent))
		return 0;

	line = line - N_ELEMENTS(sval_dependent) - 1;

	if (line < N_ELEMENTS(extra_item_options))
		return 1;

	return 0;
}

static void display_options_item(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	size_t line = (size_t) oid;

	/* First section of menu - the svals */
	if (line < N_ELEMENTS(sval_dependent))
	{
		bool known = seen_tval(sval_dependent[line].tval);
		byte attr = curs_attrs[known ? CURS_KNOWN: CURS_UNKNOWN][(int)cursor];

		c_prt(attr, sval_dependent[line].desc, row, col);
	}
	/* Second section - the "extra options" */
	else
	{
		byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

		line = line - N_ELEMENTS(sval_dependent) - 1;

		if (line < N_ELEMENTS(extra_item_options))
			c_prt(attr, extra_item_options[line].name, row, col);
	}
}


static const menu_iter options_item_iter =
{
	tag_options_item,
	valid_options_item,
	display_options_item,
	NULL
};


/*
 * Display and handle the main squelching menu.
 */
void do_cmd_options_item(void *unused, cptr title)
{
	int cursor = 0;
	ui_event_data c = EVENT_EMPTY;
	const char cmd_keys[] = { ARROW_LEFT, ARROW_RIGHT, '\0' };

	menu_type menu;

	WIPE(&menu, menu_type);
	menu.title = title;
        menu.cmd_keys = cmd_keys;
	menu.count = N_ELEMENTS(sval_dependent) + N_ELEMENTS(extra_item_options) + 1;
	menu_init(&menu, MN_SKIN_SCROLL, &options_item_iter, &SCREEN_REGION);

	/* Save and clear screen */
	screen_save();
	clear_from(0);

	while (c.key != ESCAPE)
	{
		clear_from(0);
		c = menu_select(&menu, &cursor, 0);

		if (c.type == EVT_SELECT)
		{
			if ((size_t) cursor < N_ELEMENTS(sval_dependent))
			{
				sval_menu(sval_dependent[cursor].tval, sval_dependent[cursor].desc);
			}
			else
			{
				cursor = cursor - N_ELEMENTS(sval_dependent) - 1;
				if ((size_t) cursor < N_ELEMENTS(extra_item_options))
					extra_item_options[cursor].action(NULL, NULL);
			}
		}
	}

	/* Load screen and finish */
	screen_load();
	return;
}
