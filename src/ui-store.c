/**
 * \file ui-store.c
 * \brief Store UI
 *
 * Copyright (c) 1997 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 1998-2014 Angband developers
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
#include "cave.h"
#include "cmds.h"
#include "game-event.h"
#include "game-input.h"
#include "hint.h"
#include "init.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-util.h"
#include "store.h"
#include "target.h"
#include "ui-display.h"
#include "ui-input.h"
#include "ui-menu.h"
#include "ui-object.h"
#include "ui-options.h"
#include "ui-knowledge.h"
#include "ui-object.h"
#include "ui-player.h"
#include "ui-spell.h"
#include "ui-command.h"
#include "ui-store.h"
#include "z-debug.h"


/**
 * Shopkeeper welcome messages.
 *
 * The shopkeeper's name must come first, then the character's name.
 */
static const char *comment_welcome[] =
{
	"",
	"%s nods to you.",
	"%s says hello.",
	"%s: \"See anything you like, adventurer?\"",
	"%s: \"How may I help you, %s?\"",
	"%s: \"Welcome back, %s.\"",
	"%s: \"A pleasure to see you again, %s.\"",
	"%s: \"How may I be of assistance, good %s?\"",
	"%s: \"You do honour to my humble store, noble %s.\"",
	"%s: \"I and my family are entirely at your service, %s.\""
};

static const char *comment_hint[] =
{
/*	"%s tells you soberly: \"%s\".",
	"(%s) There's a saying round here, \"%s\".",
	"%s offers to tell you a secret next time you're about."*/
	"\"%s\""
};


/**
 * Easy names for the elements of the 'scr_places' arrays.
 */
enum
{
	LOC_PRICE = 0,
	LOC_OWNER,
	LOC_HEADER,
	LOC_MORE,
	LOC_HELP_CLEAR,
	LOC_HELP_PROMPT,
	LOC_AU,
	LOC_WEIGHT,

	LOC_MAX
};

/* State flags */
#define STORE_GOLD_CHANGE      0x01
#define STORE_FRAME_CHANGE     0x02
#define STORE_SHOW_HELP        0x04

/* Compound flag for the initial display of a store */
#define STORE_INIT_CHANGE		(STORE_FRAME_CHANGE | STORE_GOLD_CHANGE)

struct store_context {
	struct menu menu;			/* Menu instance */
	struct store *store;	/* Pointer to store */
	struct object **list;	/* List of objects (unused) */
	int flags;				/* Display flags */
	bool inspect_only;		/* Only allow looking */

	/* Places for the various things displayed onscreen */
	unsigned int scr_places_x[LOC_MAX];
	unsigned int scr_places_y[LOC_MAX];
};

/* Return a random hint from the global hints list */
static const char *random_hint(void)
{
	struct hint *v, *r = NULL;
	int n;
	for (v = hints, n = 1; v; v = v->next, n++)
		if (one_in_(n))
			r = v;
	return r->hint;
}

/**
 * The greeting a shopkeeper gives the character says a lot about his
 * general attitude.
 *
 * Taken and modified from Sangband 1.0.
 *
 * Note that each comment_hint should have exactly one %s
 */
static void prt_welcome(const struct owner *proprietor)
{
	char short_name[20];
	const char *owner_name = proprietor->name;

	int j;

	if (one_in_(2))
		return;

	/* Get the first name of the store owner (stop before the first space) */
	for (j = 0; owner_name[j] && owner_name[j] != ' '; j++)
		short_name[j] = owner_name[j];

	/* Truncate the name */
	short_name[j] = '\0';

	if (one_in_(3)) {
		size_t i = randint0(N_ELEMENTS(comment_hint));
		msg(comment_hint[i], random_hint());
	} else if (player->lev > 5) {
		const char *player_name;

		/* We go from level 1 - 50  */
		size_t i = ((unsigned)player->lev - 1) / 5;
		i = MIN(i, N_ELEMENTS(comment_welcome) - 1);

		/* Get a title for the character */
		if ((i % 2) && randint0(2))
			player_name = player->class->title[(player->lev - 1) / 5];
		else if (randint0(2))
			player_name = player->full_name;
		else
			player_name = "valued customer";

		/* Balthazar says "Welcome" */
		prt(format(comment_welcome[i], short_name, player_name), 0, 0);
	}
}


/*** Display code ***/


/**
 * This function sets up screen locations based on the current term size.
 *
 * Current screen layout:
 *  line 0: reserved for messages
 *  line 1: shopkeeper and their purse / item buying price
 *  line 2: empty
 *  line 3: table headers
 *
 *  line 4: Start of items
 *
 * If help is turned off, then the rest of the display goes as:
 *
 *  line (height - 4): end of items
 *  line (height - 3): "more" prompt
 *  line (height - 2): empty
 *  line (height - 1): Help prompt and remaining gold
 *
 * If help is turned on, then the rest of the display goes as:
 *
 *  line (height - 7): end of items
 *  line (height - 6): "more" prompt
 *  line (height - 4): gold remaining
 *  line (height - 3): command help 
 */
static void store_display_recalc(struct store_context *ctx)
{
	int wid, hgt;
	region loc;

	struct menu *m = &ctx->menu;
	struct store *store = ctx->store;

	Term_get_size(&wid, &hgt);

	/* Clip the width at a max of 104 (enough room for an 80-char item name) */
	if (wid > 104) wid = 104;

	/* Clip the text_out function at two smaller than the screen width */
	text_out_wrap = wid - 2;


	/* X co-ords first */
	ctx->scr_places_x[LOC_PRICE] = wid - 14;
	ctx->scr_places_x[LOC_AU] = wid - 26;
	ctx->scr_places_x[LOC_OWNER] = wid - 2;
	ctx->scr_places_x[LOC_WEIGHT] = wid - 14;

	/* Add space for for prices */
	if (store->sidx != STORE_HOME)
		ctx->scr_places_x[LOC_WEIGHT] -= 10;

	/* Then Y */
	ctx->scr_places_y[LOC_OWNER] = 1;
	ctx->scr_places_y[LOC_HEADER] = 3;

	/* If we are displaying help, make the height smaller */
	if (ctx->flags & (STORE_SHOW_HELP))
		hgt -= 3;

	ctx->scr_places_y[LOC_MORE] = hgt - 3;
	ctx->scr_places_y[LOC_AU] = hgt - 1;

	loc = m->boundary;

	/* If we're displaying the help, then put it with a line of padding */
	if (ctx->flags & (STORE_SHOW_HELP)) {
		ctx->scr_places_y[LOC_HELP_CLEAR] = hgt - 1;
		ctx->scr_places_y[LOC_HELP_PROMPT] = hgt;
		loc.page_rows = -5;
	} else {
		ctx->scr_places_y[LOC_HELP_CLEAR] = hgt - 2;
		ctx->scr_places_y[LOC_HELP_PROMPT] = hgt - 1;
		loc.page_rows = -2;
	}

	menu_layout(m, &loc);
}


/**
 * Redisplay a single store entry
 */
static void store_display_entry(struct menu *menu, int oid, bool cursor, int row,
								int col, int width)
{
	struct object *obj;
	s32b x;
	int desc = ODESC_PREFIX;

	char o_name[80];
	char out_val[160];
	byte colour;

	struct store_context *ctx = menu_priv(menu);
	struct store *store = ctx->store;
	assert(store);

	/* Get the object */
	obj = ctx->list[oid];

	/* Describe the object - preserving insriptions in the home */
	if (store->sidx == STORE_HOME) {
		desc |= ODESC_FULL;
	} else {
		desc |= ODESC_FULL | ODESC_STORE;
	}
	object_desc(o_name, sizeof(o_name), obj, desc);

	/* Display the object */
	c_put_str(obj->kind->base->attr, o_name, row, col);

	/* Show weights */
	colour = curs_attrs[CURS_KNOWN][(int)cursor];
	strnfmt(out_val, sizeof out_val, "%3d.%d lb", obj->weight / 10,
			obj->weight % 10);
	c_put_str(colour, out_val, row, ctx->scr_places_x[LOC_WEIGHT]);

	/* Describe an object (fully) in a store */
	if (store->sidx != STORE_HOME) {
		/* Extract the "minimum" price */
		x = price_item(store, obj, false, 1);

		/* Make sure the player can afford it */
		if ((int) player->au < (int) x)
			colour = curs_attrs[CURS_UNKNOWN][(int)cursor];

		/* Actually draw the price */
		if (tval_can_have_charges(obj) && (obj->number > 1))
			strnfmt(out_val, sizeof out_val, "%9d avg", x);
		else
			strnfmt(out_val, sizeof out_val, "%9d    ", x);

		c_put_str(colour, out_val, row, ctx->scr_places_x[LOC_PRICE]);
	}
}


/**
 * Display store (after clearing screen)
 */
static void store_display_frame(struct store_context *ctx)
{
	char buf[80];
	struct store *store = ctx->store;
	struct owner *proprietor = store->owner;

	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (store->sidx == STORE_HOME) {
		/* Put the owner name */
		put_str("Your Home", ctx->scr_places_y[LOC_OWNER], 1);

		/* Label the object descriptions */
		put_str("Home Inventory", ctx->scr_places_y[LOC_HEADER], 1);

		/* Show weight header */
		put_str("Weight", ctx->scr_places_y[LOC_HEADER],
				ctx->scr_places_x[LOC_WEIGHT] + 2);
	} else {
		/* Normal stores */
		const char *store_name = store->name;
		const char *owner_name = proprietor->name;

		/* Put the owner name */
		put_str(owner_name, ctx->scr_places_y[LOC_OWNER], 1);

		/* Show the max price in the store (above prices) */
		strnfmt(buf, sizeof(buf), "%s (%d)", store_name,
				proprietor->max_cost);
		prt(buf, ctx->scr_places_y[LOC_OWNER],
			ctx->scr_places_x[LOC_OWNER] - strlen(buf));

		/* Label the object descriptions */
		put_str("Store Inventory", ctx->scr_places_y[LOC_HEADER], 1);

		/* Showing weight label */
		put_str("Weight", ctx->scr_places_y[LOC_HEADER],
				ctx->scr_places_x[LOC_WEIGHT] + 2);

		/* Label the asking price (in stores) */
		put_str("Price", ctx->scr_places_y[LOC_HEADER], ctx->scr_places_x[LOC_PRICE] + 4);
	}
}


/**
 * Display help.
 */
static void store_display_help(struct store_context *ctx)
{
	struct store *store = ctx->store;
	int help_loc = ctx->scr_places_y[LOC_HELP_PROMPT];
	bool is_home = (store->sidx == STORE_HOME) ? true : false;

	/* Clear */
	clear_from(ctx->scr_places_y[LOC_HELP_CLEAR]);

	/* Prepare help hooks */
	text_out_hook = text_out_to_screen;
	text_out_indent = 1;
	Term_gotoxy(1, help_loc);

	if (OPT(player, rogue_like_commands))
		text_out_c(COLOUR_L_GREEN, "x");
	else
		text_out_c(COLOUR_L_GREEN, "l");

	text_out(" examines");
	if (!ctx->inspect_only) {
		text_out(" and ");
		text_out_c(COLOUR_L_GREEN, "p");

		if (is_home) text_out(" picks up");
		else text_out(" purchases");
	}
	text_out(" the selected item. ");

	if (!ctx->inspect_only) {
		if (OPT(player, birth_no_selling)) {
			text_out_c(COLOUR_L_GREEN, "d");
			text_out(" gives an item to the store in return for its identification. Some wands and staves will also be recharged. ");
		} else {
			text_out_c(COLOUR_L_GREEN, "d");
			if (is_home) text_out(" drops");
			else text_out(" sells");
			text_out(" an item from your inventory. ");
		}
	} else {
		text_out_c(COLOUR_L_GREEN, "I");
		text_out(" inspects an item from your inventory. ");
	}

	text_out_c(COLOUR_L_GREEN, "ESC");
	if (!ctx->inspect_only)
		text_out(" exits the building.");
	else
		text_out(" exits this screen.");

	text_out_indent = 0;
}

/**
 * Decides what parts of the store display to redraw.  Called on terminal
 * resizings and the redraw command.
 */
static void store_redraw(struct store_context *ctx)
{
	if (ctx->flags & (STORE_FRAME_CHANGE)) {
		store_display_frame(ctx);

		if (ctx->flags & STORE_SHOW_HELP)
			store_display_help(ctx);
		else
			prt("Press '?' for help.", ctx->scr_places_y[LOC_HELP_PROMPT], 1);

		ctx->flags &= ~(STORE_FRAME_CHANGE);
	}

	if (ctx->flags & (STORE_GOLD_CHANGE)) {
		prt(format("Gold Remaining: %9d", player->au),
				ctx->scr_places_y[LOC_AU], ctx->scr_places_x[LOC_AU]);
		ctx->flags &= ~(STORE_GOLD_CHANGE);
	}
}

static bool store_get_check(const char *prompt)
{
	struct keypress ch;

	/* Prompt for it */
	prt(prompt, 0, 0);

	/* Get an answer */
	ch = inkey();

	/* Erase the prompt */
	prt("", 0, 0);

	if (ch.code == ESCAPE) return (false);
	if (strchr("Nn", ch.code)) return (false);

	/* Success */
	return (true);
}

/*
 * Sell an object, or drop if it we're in the home.
 */
static bool store_sell(struct store_context *ctx)
{
	int amt;
	int get_mode = USE_EQUIP | USE_INVEN | USE_FLOOR | USE_QUIVER;

	struct store *store = ctx->store;

	struct object *obj;
	struct object object_type_body = OBJECT_NULL;
	struct object *temp_obj = &object_type_body;

	char o_name[120];

	item_tester tester = NULL;

	const char *reject = "You have nothing that I want. ";
	const char *prompt = OPT(player, birth_no_selling) ? "Give which item? " : "Sell which item? ";

	assert(store);

	/* Clear all current messages */
	msg_flag = false;
	prt("", 0, 0);

	if (store->sidx == STORE_HOME) {
		prompt = "Drop which item? ";
	} else {
		tester = store_will_buy_tester;
		get_mode |= SHOW_PRICES;
	}

	/* Get an item */
	player->upkeep->command_wrk = USE_INVEN;

	if (!get_item(&obj, prompt, reject, CMD_DROP, tester, get_mode))
		return false;

	/* Cannot remove stickied objects */
	if (object_is_equipped(player->body, obj) && !obj_can_takeoff(obj)) {
		/* Oops */
		msg("Hmmm, it seems to be stuck.");

		/* Nope */
		return false;
	}

	/* Get a quantity */
	amt = get_quantity(NULL, obj->number);

	/* Allow user abort */
	if (amt <= 0) return false;

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(temp_obj, obj, amt);

	if (!store_check_num(store, temp_obj)) {
		object_wipe(temp_obj);
		if (store->sidx == STORE_HOME)
			msg("Your home is full.");
		else
			msg("I have not the room in my store to keep it.");

		return false;
	}

	/* Get a full description */
	object_desc(o_name, sizeof(o_name), temp_obj, ODESC_PREFIX | ODESC_FULL);

	/* Real store */
	if (store->sidx != STORE_HOME) {
		/* Extract the value of the items */
		u32b price = price_item(store, temp_obj, true, amt);

		object_wipe(temp_obj);
		screen_save();

		/* Show price */
		if (!OPT(player, birth_no_selling))
			prt(format("Price: %d", price), 1, 0);

		/* Confirm sale */
		if (!store_get_check(format("%s %s? [ESC, any other key to accept]",
				OPT(player, birth_no_selling) ? "Give" : "Sell", o_name))) {
			screen_load();
			return false;
		}

		screen_load();

		cmdq_push(CMD_SELL);
		cmd_set_arg_item(cmdq_peek(), "item", obj);
		cmd_set_arg_number(cmdq_peek(), "quantity", amt);
	} else { /* Player is at home */
		object_wipe(temp_obj);
		cmdq_push(CMD_STASH);
		cmd_set_arg_item(cmdq_peek(), "item", obj);
		cmd_set_arg_number(cmdq_peek(), "quantity", amt);
	}

	/* Update the display */
	ctx->flags |= STORE_GOLD_CHANGE;

	return true;
}



/**
 * Buy an object from a store
 */
static bool store_purchase(struct store_context *ctx, int item, bool single)
{
	struct store *store = ctx->store;

	struct object *obj = ctx->list[item];
	struct object *dummy = NULL;

	char o_name[80];

	int amt, num;

	s32b price;

	/* Clear all current messages */
	msg_flag = false;
	prt("", 0, 0);


	/*** Check the player can get any at all ***/

	/* Get an amount if we weren't given one */
	if (single) {
		amt = 1;

		/* Check if the player can afford any at all */
		if (store->sidx != STORE_HOME &&
				(int)player->au < (int)price_item(store, obj, false, 1)) {
			msg("You do not have enough gold for this item.");
			return false;
		}
	} else {
		if (store->sidx == STORE_HOME) {
			amt = obj->number;
		} else {
			/* Price of one */
			price = price_item(store, obj, false, 1);

			/* Check if the player can afford any at all */
			if ((u32b)player->au < (u32b)price) {
				msg("You do not have enough gold for this item.");
				return false;
			}

			/* Work out how many the player can afford */
			if (price == 0)
				amt = obj->number; /* Prevent division by zero */
			else
				amt = player->au / price;

			if (amt > obj->number) amt = obj->number;

			/* Double check for wands/staves */
			if ((player->au >= price_item(store, obj, false, amt+1)) &&
				(amt < obj->number))
				amt++;
		}

		/* Limit to the number that can be carried */
		amt = MIN(amt, inven_carry_num(obj, false));

		/* Fail if there is no room */
		if ((amt <= 0) || (!object_flavor_is_aware(obj) && pack_is_full())) {
			msg("You cannot carry that many items.");
			return false;
		}

		/* Find the number of this item in the inventory */
		if (!object_flavor_is_aware(obj))
			num = 0;
		else
			num = find_inven(obj);

		strnfmt(o_name, sizeof o_name, "%s how many%s? (max %d) ",
				(store->sidx == STORE_HOME) ? "Take" : "Buy",
				num ? format(" (you have %d)", num) : "", amt);

		/* Get a quantity */
		amt = get_quantity(o_name, amt);

		/* Allow user abort */
		if (amt <= 0) return false;
	}

	/* Get desired object */
	dummy = object_new();
	object_copy_amt(dummy, obj, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(dummy)) {
		msg("You cannot carry that many items.");
		object_delete(&dummy);
		return false;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), dummy, ODESC_PREFIX | ODESC_FULL |
		ODESC_STORE);

	/* Attempt to buy it */
	if (store->sidx != STORE_HOME) {
		bool response;

		/* Extract the price for the entire stack */
		price = price_item(store, dummy, false, dummy->number);

		screen_save();

		/* Show price */
		prt(format("Price: %d", price), 1, 0);

		/* Confirm purchase */
		response = store_get_check(format("Buy %s? [ESC, any other key to accept]", o_name));
		screen_load();

		/* Negative response, so give up */
		if (!response) return false;

		cmdq_push(CMD_BUY);
		cmd_set_arg_item(cmdq_peek(), "item", obj);
		cmd_set_arg_number(cmdq_peek(), "quantity", amt);
	} else {
		/* Home is much easier */
		cmdq_push(CMD_RETRIEVE);
		cmd_set_arg_item(cmdq_peek(), "item", obj);
		cmd_set_arg_number(cmdq_peek(), "quantity", amt);
	}

	/* Update the display */
	ctx->flags |= STORE_GOLD_CHANGE;

	object_delete(&dummy);

	/* Not kicked out */
	return true;
}


/**
 * Examine an item in a store
 */
static void store_examine(struct store_context *ctx, int item)
{
	struct object *obj;
	char header[120];
	textblock *tb;
	region area = { 0, 0, 0, 0 };
	int odesc_flags = ODESC_PREFIX | ODESC_FULL;

	if (item < 0) return;

	/* Get the actual object */
	obj = ctx->list[item];

	/* Items in the home get less description */
	if (ctx->store->sidx == STORE_HOME) {
		odesc_flags |= ODESC_CAPITAL;
	} else {
		odesc_flags |= ODESC_STORE;
	}

	/* Hack -- no flush needed */
	msg_flag = false;

	/* Show full info in most stores, but normal info in player home */
	tb = object_info(obj, OINFO_NONE);
	object_desc(header, sizeof(header), obj, odesc_flags);

	textui_textblock_show(tb, area, header);
	textblock_free(tb);

	/* Hack -- Browse book, then prompt for a command */
	if (obj_can_browse(obj))
		textui_book_browse(obj);
}


static void store_menu_set_selections(struct menu *menu, bool knowledge_menu)
{
	if (knowledge_menu) {
		if (OPT(player, rogue_like_commands)) {
			/* These two can't intersect! */
			menu->cmd_keys = "?|Ieilx";
			menu->selections = "abcdfghjkmnopqrstuvwyz134567";
		} else {
			/* These two can't intersect! */
			menu->cmd_keys = "?|Ieil";
			menu->selections = "abcdfghjkmnopqrstuvwxyz13456";
		}
	} else {
		if (OPT(player, rogue_like_commands)) {
			/* These two can't intersect! */
			menu->cmd_keys = "\x04\x05\x10?={|}~CEIPTdegilpswx"; /* \x10 = ^p , \x04 = ^D, \x05 = ^E */
			menu->selections = "abcfmnoqrtuvyz13456790ABDFGH";
		} else {
			/* These two can't intersect! */
			menu->cmd_keys = "\x05\x010?={|}~CEIbdegiklpstwx"; /* \x05 = ^E, \x10 = ^p */
			menu->selections = "acfhjmnoqruvyz13456790ABDFGH";
		}
	}
}

static void store_menu_recalc(struct menu *m)
{
	struct store_context *ctx = menu_priv(m);
	menu_setpriv(m, ctx->store->stock_num, ctx);
}

/**
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands in the stores
 * which are not allowed in the dungeon, and we must disable some commands
 * which are allowed in the dungeon but not in the stores, to prevent chaos.
 */
static bool store_process_command_key(struct keypress kp)
{
	int cmd = 0;

	/* Hack -- no flush needed */
	prt("", 0, 0);
	msg_flag = false;

	/* Process the keycode */
	switch (kp.code) {
		case 'T': /* roguelike */
		case 't': cmd = CMD_TAKEOFF; break;

		case KTRL('D'): /* roguelike */
		case 'k': textui_cmd_ignore(); break;

		case 'P': /* roguelike */
		case 'b': textui_spell_browse(); break;

		case '~': textui_browse_knowledge(); break;
		case 'I': textui_obj_examine(); break;
		case 'w': cmd = CMD_WIELD; break;
		case '{': cmd = CMD_INSCRIBE; break;
		case '}': cmd = CMD_UNINSCRIBE; break;

		case 'e': do_cmd_equip(); break;
		case 'i': do_cmd_inven(); break;
		case '|': do_cmd_quiver(); break;
		case KTRL('E'): toggle_inven_equip(); break;
		case 'C': do_cmd_change_name(); break;
		case KTRL('P'): do_cmd_messages(); break;
		case ')': do_cmd_save_screen(); break;

		default: return false;
	}

	if (cmd)
		cmdq_push_repeat(cmd, 0);

	return true;
}

/**
 * Select an item from the store's stock, and return the stock index
 */
static int store_get_stock(struct menu *m, int oid)
{
	ui_event e;
	int no_act = m->flags & MN_NO_ACTION;

	/* Set a flag to make sure that we get the selection or escape
	 * without running the menu handler */
	m->flags |= MN_NO_ACTION;
	e = menu_select(m, 0, true);
	if (!no_act) {
		m->flags &= ~MN_NO_ACTION;
	}

	if (e.type == EVT_SELECT) {
		return m->cursor;
	} else if (e.type == EVT_ESCAPE) {
		return -1;
	}

	/* if we do not have a new selection, just return the original item */
	return oid;
}

/** Enum for context menu entries */
enum {
	ACT_INSPECT_INVEN,
	ACT_SELL,
	ACT_EXAMINE,
	ACT_BUY,
	ACT_BUY_ONE,
	ACT_EXIT
};

/* pick the context menu options appropiate for a store */
static int context_menu_store(struct store_context *ctx, const int oid, int mx, int my)
{
	struct store *store = ctx->store;
	bool home = (store->sidx == STORE_HOME) ? true : false;

	struct menu *m = menu_dynamic_new();

	int selected;
	char *labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Inspect inventory", 'I', ACT_INSPECT_INVEN, labels);
	menu_dynamic_add_label(m, home ? "Stash" : "Sell", 'd', ACT_SELL, labels);
	menu_dynamic_add_label(m, "Exit", '`', ACT_EXIT, labels);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch (selected) {
		case ACT_SELL:
			store_sell(ctx);
			break;
		case ACT_INSPECT_INVEN:
			textui_obj_examine();
			break;
		case ACT_EXIT:
			return false;
	}

	return true;
}

/* pick the context menu options appropiate for an item available in a store */
static void context_menu_store_item(struct store_context *ctx, const int oid, int mx, int my)
{
	struct store *store = ctx->store;
	bool home = (store->sidx == STORE_HOME) ? true : false;

	struct menu *m = menu_dynamic_new();
	struct object *obj = ctx->list[oid];

	int selected;
	char *labels;
	char header[120];

	object_desc(header, sizeof(header), obj,
				ODESC_PREFIX | ODESC_FULL | ODESC_STORE);

	labels = string_make(lower_case);
	m->selections = labels;

	menu_dynamic_add_label(m, "Examine", 'x', ACT_EXAMINE, labels);
	menu_dynamic_add_label(m, home ? "Take" : "Buy", 'd', ACT_SELL, labels);
	if (obj->number > 1)
		menu_dynamic_add_label(m, home ? "Take one" : "Buy one", 'o', ACT_BUY_ONE, labels);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

	prt(format("(Enter to select, ESC) Command for %s:", header), 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	switch (selected) {
		case ACT_EXAMINE:
			store_examine(ctx, oid);
			break;
		case ACT_BUY:
			store_purchase(ctx, oid, false);
			break;
		case ACT_BUY_ONE:
			store_purchase(ctx, oid, true);
			break;
	}
}

/**
 * Handle store menu input
 */
static bool store_menu_handle(struct menu *m, const ui_event *event, int oid)
{
	bool processed = true;
	struct store_context *ctx = menu_priv(m);
	struct store *store = ctx->store;
	
	if (event->type == EVT_SELECT) {
		/* Nothing for now, except "handle" the event */
		return true;
		/* In future, maybe we want a display a list of what you can do. */
	} else if (event->type == EVT_MOUSE) {
		if (event->mouse.button == 2) {
			/* exit the store? what already does this? menu_handle_mouse
			 * so exit this so that menu_handle_mouse will be called */
			return false;
		} else if (event->mouse.button == 1) {
			bool action = false;
			if ((event->mouse.y == 0) || (event->mouse.y == 1)) {
				/* show the store context menu */
				if (context_menu_store(ctx, oid, event->mouse.x, event->mouse.y) == false)
					return false;

				action = true;
			} else if ((oid >= 0) && (event->mouse.y == m->active.row + oid)) {
				/* if press is on a list item, so store item context */
				context_menu_store_item(ctx, oid, event->mouse.x,
										event->mouse.y);
				action = true;
			}

			if (action) {
				ctx->flags |= (STORE_FRAME_CHANGE | STORE_GOLD_CHANGE);

				/* Let the game handle any core commands (equipping, etc) */
				cmdq_pop(CTX_STORE);

				/* Notice and handle stuff */
				notice_stuff(player);
				handle_stuff(player);

				/* Display the store */
				store_display_recalc(ctx);
				store_menu_recalc(m);
				store_redraw(ctx);

				return true;
			}
		}
	} else if (event->type == EVT_KBRD) {
		switch (event->key.code) {
			case 's':
			case 'd': store_sell(ctx); break;

			case 'p':
			case 'g':
				/* use the old way of purchasing items */
				msg_flag = false;
				if (store->sidx != STORE_HOME) {
					prt("Purchase which item? (ESC to cancel, Enter to select)",
						0, 0);
				} else {
					prt("Get which item? (Esc to cancel, Enter to select)",
						0, 0);
				}
				oid = store_get_stock(m, oid);
				prt("", 0, 0);
				if (oid >= 0) {
					store_purchase(ctx, oid, false);
				}
				break;
			case 'l':
			case 'x':
				/* use the old way of examining items */
				msg_flag = false;
				prt("Examine which item? (ESC to cancel, Enter to select)",
					0, 0);
				oid = store_get_stock(m, oid);
				prt("", 0, 0);
				if (oid >= 0) {
					store_examine(ctx, oid);
				}
				break;

			case '?': {
				/* Toggle help */
				if (ctx->flags & STORE_SHOW_HELP)
					ctx->flags &= ~(STORE_SHOW_HELP);
				else
					ctx->flags |= STORE_SHOW_HELP;

				/* Redisplay */
				ctx->flags |= STORE_INIT_CHANGE;

				store_display_recalc(ctx);
				store_redraw(ctx);

				break;
			}

			case '=': {
				do_cmd_options();
				store_menu_set_selections(m, false);
				break;
			}

			default:
				processed = store_process_command_key(event->key);
		}

		/* Let the game handle any core commands (equipping, etc) */
		cmdq_pop(CTX_STORE);

		if (processed) {
			event_signal(EVENT_INVENTORY);
			event_signal(EVENT_EQUIPMENT);
		}

		/* Notice and handle stuff */
		notice_stuff(player);
		handle_stuff(player);

		return processed;
	}

	return false;
}

static region store_menu_region = { 1, 4, -1, -2 };
static const menu_iter store_menu =
{
	NULL,
	NULL,
	store_display_entry,
	store_menu_handle,
	NULL
};

/**
 * Init the store menu
 */
static void store_menu_init(struct store_context *ctx, struct store *store, bool inspect_only)
{
	struct menu *menu = &ctx->menu;

	ctx->store = store;
	ctx->flags = STORE_INIT_CHANGE;
	ctx->inspect_only = inspect_only;
	ctx->list = mem_zalloc(sizeof(struct object *) * z_info->store_inven_max);

	store_stock_list(ctx->store, ctx->list, z_info->store_inven_max);

	/* Init the menu structure */
	menu_init(menu, MN_SKIN_SCROLL, &store_menu);
	menu_setpriv(menu, 0, ctx);

	/* Calculate the positions of things and draw */
	menu_layout(menu, &store_menu_region);
	store_menu_set_selections(menu, inspect_only);
	store_display_recalc(ctx);
	store_menu_recalc(menu);
	store_redraw(ctx);
}

/**
 * Display contents of a store from knowledge menu
 *
 * The only allowed actions are 'I' to inspect an item
 */
void textui_store_knowledge(int n)
{
	struct store_context ctx;

	screen_save();
	clear_from(0);

	store_menu_init(&ctx, &stores[n], true);
	menu_select(&ctx.menu, 0, false);

	/* Flush messages XXX XXX XXX */
	event_signal(EVENT_MESSAGE_FLUSH);

	screen_load();

	mem_free(ctx.list);
}


/**
 * Handle stock change.
 */
static void refresh_stock(game_event_type type, game_event_data *unused, void *user)
{
	struct store_context *ctx = user;
	struct menu *menu = &ctx->menu;

	store_stock_list(ctx->store, ctx->list, z_info->store_inven_max);

	/* Display the store */
	store_display_recalc(ctx);
	store_menu_recalc(menu);
	store_redraw(ctx);
}

/**
 * Enter a store.
 */
void enter_store(game_event_type type, game_event_data *data, void *user)
{
	/* Check that we're on a store */
	if (!square_isshop(cave, player->grid)) {
		msg("You see no store here.");
		return;
	}

	/* Shut down the normal game view */
	event_signal(EVENT_LEAVE_WORLD);
}

/**
 * Interact with a store.
 */
void use_store(game_event_type type, game_event_data *data, void *user)
{
	struct store *store = store_at(cave, player->grid);
	struct store_context ctx;

	/* Check that we're on a store */
	if (!store) return;

	/*** Display ***/

	/* Save current screen (ie. dungeon) */
	screen_save();
	msg_flag = false;

	/* Get a array version of the store stock, register handler for changes */
	event_add_handler(EVENT_STORECHANGED, refresh_stock, &ctx);
	store_menu_init(&ctx, store, false);

	/* Say a friendly hello. */
	if (store->sidx != STORE_HOME)
		prt_welcome(store->owner);

	/* Shopping */
	menu_select(&ctx.menu, 0, false);

	/* Shopping's done */
	event_remove_handler(EVENT_STORECHANGED, refresh_stock, &ctx);
	msg_flag = false;
	mem_free(ctx.list);

	/* Take a turn */
	player->upkeep->energy_use = z_info->move_energy;

	/* Flush messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Load the screen */
	screen_load();
}

void leave_store(game_event_type type, game_event_data *data, void *user)
{
	/* Disable repeats */
	cmd_disable_repeat();

	/* Switch back to the normal game view. */
	event_signal(EVENT_ENTER_WORLD);

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw entire screen */
	player->upkeep->redraw |= (PR_BASIC | PR_EXTRA);

	/* Redraw map */
	player->upkeep->redraw |= (PR_MAP);
}
