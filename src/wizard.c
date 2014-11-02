/*
 * File: wizard2.c
 * Purpose: Debug mode commands
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "effects.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-make.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "project.h"
#include "target.h"
#include "ui-event.h"
#include "ui-game.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-menu.h"
#include "wizard.h"


static void gf_display(menu_type *m, int type, bool cursor,
		int row, int col, int wid)
{
	size_t i;

	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	const char *gf_name = gf_idx_to_name(type);

	if (type % 2)
		c_prt(attr, ".........................", row, col);
	c_put_str(attr, gf_name, row, col);

	col += 25;

	if (tile_height == 1) {
		for (i = 0; i < BOLT_MAX; i++) {
			col += big_pad(col, row, gf_to_attr[type][i], gf_to_char[type][i]);
		}
	} else {
		prt("Change tile_height to 1 to see graphics.", row, col);
	}
}

static const menu_iter gf_iter = {
	NULL, /* get_tag */
	NULL, /* validity */
	gf_display,
	NULL, /* action */
	NULL /* resize */
};

static void wiz_gf_demo(void)
{
	menu_type *m = menu_new(MN_SKIN_SCROLL, &gf_iter);
	region loc = { 0, 0, 0, 0 };

	menu_setpriv(m, GF_MAX, NULL);

	m->title = "GF_ types display";
	menu_layout(m, &loc);

	screen_save();
	clear_from(0);
	menu_select(m, 0, FALSE);
	screen_load();
}








/*
 * This is a nice utility function; it determines if a (NULL-terminated)
 * string consists of only digits (starting with a non-zero digit).
 */
static s16b get_idx_from_name(char *s)
{
	char *endptr = NULL;
	long l = strtol(s, &endptr, 10);
	return *endptr == '\0' ? (s16b)l : 0;
}

/*
 * Hack -- quick debugging hook
 */
static void do_cmd_wiz_hack_ben(void)
{
	int py = player->py;
	int px = player->px;

	int i, y, x;

	struct keypress kp;


	for (i = 0; i < z_info->max_flow_depth; ++i)
	{
		/* Update map */
		for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
		{
			for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
			{
				byte a = TERM_RED;

				if (!square_in_bounds_fully(cave, y, x)) continue;

				/* Display proper cost */
				if (cave->squares[y][x].cost != i) continue;

				/* Reliability in yellow */
				if (cave->squares[y][x].when == cave->squares[py][px].when)
					a = TERM_YELLOW;

				/* Display player/floors/walls */
				if ((y == py) && (x == px))
					print_rel(L'@', a, y, x);
				else if (square_ispassable(cave, y, x))
					print_rel(L'*', a, y, x);
				else
					print_rel(L'#', a, y, x);
			}
		}

		/* Prompt */
		prt(format("Depth %d: ", i), 0, 0);

		/* Get key */
		kp = inkey();
		if (kp.code == ESCAPE) break;

		/* Redraw map */
		prt_map();
	}

	/* Done */
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}



/*
 * Output part of a bitflag set in binary format.
 */
static void prt_binary(const bitflag *flags, int offset, int row, int col, char ch, int num)
{
	int flag;

	/* Scan the flags */
	for (flag = FLAG_START + offset; flag < FLAG_START + offset + num; flag++)
	{
		if (of_has(flags, flag))
			Term_putch(col++, row, TERM_BLUE, ch);
		else
			Term_putch(col++, row, TERM_WHITE, '-');
	}
}

/**
 * This ugly piece of code exists to figure out what keycodes the user has
 * been generating.
 */
static void do_cmd_keylog(void) {
	int i;
	char buf[50];
	char buf2[12];
	struct keypress keys[2] = {{EVT_NONE, 0}, {EVT_NONE, 0}};

	screen_save();

	prt("Previous keypresses (top most recent):", 0, 0);

	for (i = 0; i < KEYLOG_SIZE; i++) {
		if (i < log_size) {
			/* find the keypress from our log */
			int j = (log_i + i) % KEYLOG_SIZE;
			struct keypress k = keylog[j];

			/* ugh. it would be nice if there was a verion of keypress_to_text
			 * which took only one keypress. */
			keys[0] = k;
			keypress_to_text(buf2, sizeof(buf2), keys, TRUE);

			/* format this line of output */
			strnfmt(buf, sizeof(buf), "    %-12s (code=%u mods=%u)", buf2, k.code, k.mods);
		} else {
			/* create a blank line of output */
			strnfmt(buf, sizeof(buf), "%40s", "");
		}

		prt(buf, i + 1, 0);
	}

	prt("Press any key to continue.", KEYLOG_SIZE + 1, 0);
	inkey();
	screen_load();
}


/*
 * Teleport to the requested target
 */
static void do_cmd_wiz_bamf(void)
{
	s16b x = 0, y = 0;

	/* Use the targeting function. */
	if (!target_set_interactive(TARGET_LOOK, -1, -1))
		return;

	/* grab the target coords. */
	target_get(&x, &y);

	/* Test for passable terrain. */
	if (!square_ispassable(cave, y, x))
		msg("The square you are aiming for is impassable.");

	/* Teleport to the target */
	else
		effect_simple(EF_TELEPORT_TO, "0", y, x, 0, NULL);
}



/*
 * Aux function for "do_cmd_wiz_change()"
 */
static void do_cmd_wiz_change_aux(void)
{
	int i;

	int tmp_int;

	long tmp_long;

	char tmp_val[160];

	char ppp[80];


	/* Query the stats */
	for (i = 0; i < STAT_MAX; i++)
	{
		/* Prompt */
		strnfmt(ppp, sizeof(ppp), "%s (3-118): ", stat_names[i]);

		/* Default */
		strnfmt(tmp_val, sizeof(tmp_val), "%d", player->stat_max[i]);

		/* Query */
		if (!get_string(ppp, tmp_val, 4)) return;

		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Verify */
		if (tmp_int > 18+100) tmp_int = 18+100;
		else if (tmp_int < 3) tmp_int = 3;

		/* Save it */
		player->stat_cur[i] = player->stat_max[i] = tmp_int;
	}


	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%ld", (long)(player->au));

	/* Query */
	if (!get_string("Gold: ", tmp_val, 10)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	player->au = tmp_long;


	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%ld", (long)(player->exp));

	/* Query */
	if (!get_string("Experience: ", tmp_val, 10)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	if (tmp_long > player->exp)
		player_exp_gain(player, tmp_long - player->exp);
	else
		player_exp_lose(player, player->exp - tmp_long, FALSE);
}


/*
 * Change various "permanent" player variables.
 */
static void do_cmd_wiz_change(void)
{
	/* Interact */
	do_cmd_wiz_change_aux();

	/* Redraw everything */
	do_cmd_redraw();
}


/*
 * Wizard routines for creating objects and modifying them
 *
 * This has been rewritten to make the whole procedure
 * of debugging objects much easier and more comfortable.
 *
 * Here are the low-level functions
 *
 * - wiz_display_item()
 *     display an item's debug-info
 * - wiz_create_itemtype()
 *     specify tval and sval (type and subtype of object)
 * - wiz_tweak_item()
 *     specify pval, +AC, +tohit, +todam
 *     Note that the wizard can leave this function anytime,
 *     thus accepting the default-values for the remaining values.
 *     pval comes first now, since it is most important.
 * - wiz_reroll_item()
 *     apply some magic to the item or turn it into an artifact.
 * - wiz_roll_item()
 *     Get some statistics about the rarity of an item:
 *     We create a lot of fake items and see if they are of the
 *     same type (tval and sval), then we compare pval and +AC.
 *     If the fake-item is better or equal it is counted.
 *     Note that cursed items that are better or equal (absolute values)
 *     are counted, too.
 *     HINT: This is *very* useful for balancing the game!
 * - wiz_quantity_item()
 *     change the quantity of an item, but be sane about it.
 *
 * And now the high-level functions
 * - do_cmd_wiz_play()
 *     play with an existing object
 * - wiz_create_item()
 *     create a new object
 *
 * Note -- You do not have to specify "pval" and other item-properties
 * directly. Just apply magic until you are satisfied with the item.
 *
 * Note -- For some items (such as wands, staffs, some rings, etc), you
 * must apply magic, or you will get "broken" or "uncharged" objects.
 *
 * Note -- Redefining artifacts via "do_cmd_wiz_play()" may destroy
 * the artifact.  Be careful.
 *
 * Hack -- this function will allow you to create multiple artifacts.
 * This "feature" may induce crashes or other nasty effects.
 */


/*
 * Display an item's properties
 */
static void wiz_display_item(const object_type *o_ptr, bool all)
{
	int j = 0;

	bitflag f[OF_SIZE];

	char buf[256];


	/* Extract the flags */
	if (all)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	/* Clear screen */
	Term_clear();

	/* Describe fully */
	object_desc(buf, sizeof(buf), o_ptr,
				ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

	prt(buf, 2, j);

	prt(format("combat = (%dd%d) (%+d,%+d) [%d,%+d]",
	           o_ptr->dd, o_ptr->ds, o_ptr->to_h, o_ptr->to_d, o_ptr->ac, o_ptr->to_a), 4, j);

	prt(format("kind = %-5d  tval = %-5d  sval = %-5d  wgt = %-3d     timeout = %-d",
	           o_ptr->kind->kidx, o_ptr->tval, o_ptr->sval, o_ptr->weight, o_ptr->timeout), 5, j);

	/* CC: multiple pvals not shown, pending #1290 */
	prt(format("number = %-3d  pval = %-5d  name1 = %-4d  egoidx = %-4d  cost = %ld",
			o_ptr->number, o_ptr->pval,
			o_ptr->artifact ? o_ptr->artifact->aidx : 0,
			o_ptr->ego ? o_ptr->ego->eidx : 0,
			(long)object_value(o_ptr, 1, FALSE)), 6, j);

	prt("+------------FLAGS-------------+", 16, j);
	prt("SUST.PROT<-OTHER--><BAD->CUR....", 17, j);
	prt("     fbcssf  s  ibniiatadlhp....", 18, j);
	prt("siwdcelotdfrei  plommfegrccc....", 19, j);
	prt("tnieoannuiaesnfhcefhsrlgxuuu....", 20, j);
	prt("rtsxnrdfnglgpvaltsuppderprrr....", 21, j);
	prt_binary(f, 0, 22, j, '*', 28);
	prt_binary(o_ptr->known_flags, 0, 23, j, '+', 28);
}



static const region wiz_create_item_area = { 0, 0, 0, 0 };

/** Object kind selection */
static void wiz_create_item_subdisplay(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	object_kind **choices = menu_priv(m);
	char buf[80];

	object_kind_name(buf, sizeof buf, choices[oid], TRUE);
	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_subaction(menu_type *m, const ui_event *e, int oid)
{
	object_kind **choices = menu_priv(m);
	object_kind *kind = choices[oid];

	object_type *i_ptr;
	object_type object_type_body;

	if (e->type != EVT_SELECT)
		return TRUE;


	/* Get local object */
	i_ptr = &object_type_body;

	/* Create the item */
	object_prep(i_ptr, kind, player->depth, RANDOMISE);

	/* Apply magic (no messages, no artifacts) */
	apply_magic(i_ptr, player->depth, FALSE, FALSE, FALSE, FALSE);

	/* Mark as cheat, and where created */
	i_ptr->origin = ORIGIN_CHEAT;
	i_ptr->origin_depth = player->depth;

	if (tval_is_money_k(kind))
		make_gold(i_ptr, player->depth, lookup_kind(TV_GOLD, kind->sval)->name);

	/* Drop the object from heaven */
	drop_near(cave, i_ptr, 0, player->py, player->px, TRUE);

	return FALSE;
}

static menu_iter wiz_create_item_submenu =
{
	NULL,
	NULL,
	wiz_create_item_subdisplay,
	wiz_create_item_subaction,
	NULL
};

/** Object base kind selection **/

static void wiz_create_item_display(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	char buf[80];
	object_base_name(buf, sizeof buf, oid, TRUE);
	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_action(menu_type *m, const ui_event *e, int oid)
{
	ui_event ret;
	menu_type *menu;

	char buf[80];

	object_kind *choice[60];
	int n_choices;

	int i;

	if (e->type != EVT_SELECT)
		return TRUE;

	for (n_choices = 0, i = 1; (n_choices < 60) && (i < z_info->k_max); i++)
	{
		object_kind *kind = &k_info[i];

		if (kind->tval != oid ||
				kf_has(kind->kind_flags, KF_INSTA_ART))
			continue;

		choice[n_choices++] = kind;
	}

	screen_save();
	clear_from(0);

	menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_submenu);
	menu->selections = all_letters;

	object_base_name(buf, sizeof buf, oid, TRUE);
	menu->title = string_make(format("What kind of %s?", buf));

	menu_setpriv(menu, n_choices, choice);
	menu_layout(menu, &wiz_create_item_area);
	ret = menu_select(menu, 0, FALSE);

	screen_load();
	string_free((char *)menu->title);

	return (ret.type == EVT_ESCAPE);
}

static const menu_iter wiz_create_item_menu =
{
	NULL,
	NULL,
	wiz_create_item_display,
	wiz_create_item_action,
	NULL
};


/*
 * Choose and create an instance of an object kind
 */
static void wiz_create_item(void)
{
	int tvals[TV_MAX];
	size_t i, n;

	menu_type *menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_menu);

	menu->selections = all_letters;
	menu->title = "What kind of object?";

	/* Make a list of all tvals for the filter */
	for (i = 0, n = 0; i < TV_MAX; i++) {
		if (!kb_info[i].name)
			continue;

		tvals[n++] = i;
	}

	screen_save();
	clear_from(0);

	menu_setpriv(menu, TV_MAX, kb_info);
	menu_set_filter(menu, tvals, n);
	menu_layout(menu, &wiz_create_item_area);
	menu_select(menu, 0, FALSE);

	screen_load();
	
	/* Redraw map */
	player->upkeep->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(player->upkeep);

}



/*
 * Tweak an item
 */
static void wiz_tweak_item(object_type *o_ptr)
{
	const char *p;
	char tmp_val[80];
	int i, val;

	/* Hack -- leave artifacts alone */
	if (o_ptr->artifact) return;

	p = "Enter new ego item index: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (o_ptr->ego)
		strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->ego->eidx);
	if (!get_string(p, tmp_val, 6)) return;
	val = atoi(tmp_val);
	if (val) {
		o_ptr->ego = &e_info[val];
		ego_apply_magic(o_ptr, player->depth);
	} else
		o_ptr->ego = 0;
	wiz_display_item(o_ptr, TRUE);

	p = "Enter new artifact index: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (o_ptr->artifact)
		strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->artifact->aidx);
	if (!get_string(p, tmp_val, 6)) return;
	val = atoi(tmp_val);
	if (val) {
		o_ptr->artifact = &a_info[val];
		copy_artifact_data(o_ptr, o_ptr->artifact);
	} else
		o_ptr->artifact = 0;
	wiz_display_item(o_ptr, TRUE);

#define WIZ_TWEAK(attribute) do {\
	p = "Enter new '" #attribute "' setting: ";\
	strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->attribute);\
	if (!get_string(p, tmp_val, 6)) return;\
	o_ptr->attribute = atoi(tmp_val);\
	wiz_display_item(o_ptr, TRUE);\
} while (0)
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		WIZ_TWEAK(modifiers[i]);
	}
	WIZ_TWEAK(to_a);
	WIZ_TWEAK(to_h);
	WIZ_TWEAK(to_d);
}


/*
 * Apply magic to an item or turn it into an artifact. -Bernd-
 */
static void wiz_reroll_item(object_type *o_ptr)
{
	object_type *i_ptr;
	object_type object_type_body;

	struct keypress ch;

	bool changed = FALSE;


	/* Hack -- leave artifacts alone */
	if (o_ptr->artifact) return;


	/* Get local object */
	i_ptr = &object_type_body;

	/* Copy the object */
	object_copy(i_ptr, o_ptr);


	/* Main loop. Ask for magification and artifactification */
	while (TRUE)
	{
		/* Display full item debug information */
		wiz_display_item(i_ptr, TRUE);

		/* Ask wizard what to do. */
		if (!get_com("[a]ccept, [n]ormal, [g]ood, [e]xcellent? ", &ch))
			break;

		/* Create/change it! */
		if (ch.code == 'A' || ch.code == 'a')
		{
			changed = TRUE;
			break;
		}

		/* Apply normal magic, but first clear object */
		else if (ch.code == 'n' || ch.code == 'N')
		{
			object_prep(i_ptr, o_ptr->kind, player->depth, RANDOMISE);
			apply_magic(i_ptr, player->depth, FALSE, FALSE, FALSE, FALSE);
		}

		/* Apply good magic, but first clear object */
		else if (ch.code == 'g' || ch.code == 'G')
		{
			object_prep(i_ptr, o_ptr->kind, player->depth, RANDOMISE);
			apply_magic(i_ptr, player->depth, FALSE, TRUE, FALSE, FALSE);
		}

		/* Apply great magic, but first clear object */
		else if (ch.code == 'e' || ch.code == 'E')
		{
			object_prep(i_ptr, o_ptr->kind, player->depth, RANDOMISE);
			apply_magic(i_ptr, player->depth, FALSE, TRUE, TRUE, FALSE);
		}
	}


	/* Notice change */
	if (changed)
	{
		/* Mark as cheat */
		i_ptr->origin = ORIGIN_CHEAT;

		/* Restore the position information */
		i_ptr->iy = o_ptr->iy;
		i_ptr->ix = o_ptr->ix;
		i_ptr->next_o_idx = o_ptr->next_o_idx;
		i_ptr->marked = o_ptr->marked;

		/* Apply changes */
		object_copy(o_ptr, i_ptr);

		/* Recalculate bonuses, gear */
		player->upkeep->update |= (PU_BONUS | PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );
	}
}



/*
 * Maximum number of rolls
 */
#define TEST_ROLL 100000


/*
 * Try to create an item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now.  We acquire a clean grid, and then
 * repeatedly place an object in this grid, copying it into an item
 * holder, and then deleting the object.  We fiddle with the artifact
 * counter flags to prevent weirdness.  We use the items to collect
 * statistics on item creation relative to the initial item.
 */
static void wiz_statistics(object_type *o_ptr, int level)
{
	long i, matches, better, worse, other;
	int j;

	struct keypress ch;
	const char *quality;

	bool good, great, ismatch, isbetter, isworse;

	object_type *i_ptr;
	object_type object_type_body;

	const char *q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";


	/* Allow multiple artifacts, because breaking the game is fine here */
	if (o_ptr->artifact) o_ptr->artifact->created = FALSE;


	/* Interact */
	while (TRUE)
	{
		const char *pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";

		/* Display item */
		wiz_display_item(o_ptr, TRUE);

		/* Get choices */
		if (!get_com(pmt, &ch)) break;

		if (ch.code == 'n' || ch.code == 'N')
		{
			good = FALSE;
			great = FALSE;
			quality = "normal";
		}
		else if (ch.code == 'g' || ch.code == 'G')
		{
			good = TRUE;
			great = FALSE;
			quality = "good";
		}
		else if (ch.code == 'e' || ch.code == 'E')
		{
			good = TRUE;
			great = TRUE;
			quality = "excellent";
		}
		else
		{
#if 0 /* unused */
			good = FALSE;
			great = FALSE;
#endif /* unused */
			break;
		}

		/* Let us know what we are doing */
		msg("Creating a lot of %s items. Base level = %d.",
		           quality, player->depth);
		message_flush();

		/* Set counters to zero */
		matches = better = worse = other = 0;

		/* Let's rock and roll */
		for (i = 0; i <= TEST_ROLL; i++)
		{
			/* Output every few rolls */
			if ((i < 100) || (i % 100 == 0))
			{
				struct keypress kp;

				/* Do not wait */
				inkey_scan = SCAN_INSTANT;

				/* Allow interupt */
				kp = inkey();
				if (kp.type != EVT_NONE)
				{
					flush();
					break;
				}

				/* Dump the stats */
				prt(format(q, i, matches, better, worse, other), 0, 0);
				Term_fresh();
			}


			/* Get local object */
			i_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(i_ptr);

			/* Create an object */
			make_object(cave, i_ptr, level, good, great, FALSE, NULL, 0);

			/* Allow multiple artifacts, because breaking the game is fine here */
			if (o_ptr->artifact) o_ptr->artifact->created = FALSE;

			/* Test for the same tval and sval. */
			if ((o_ptr->tval) != (i_ptr->tval)) continue;
			if ((o_ptr->sval) != (i_ptr->sval)) continue;

			/* Check modifiers */
			ismatch = TRUE;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (i_ptr->modifiers[j] != o_ptr->modifiers[j])
					ismatch = FALSE;

			isbetter = TRUE;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (i_ptr->modifiers[j] < o_ptr->modifiers[j])
					isbetter = FALSE;

			isworse = TRUE;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (i_ptr->modifiers[j] > o_ptr->modifiers[j])
					isworse = FALSE;

			/* Check for match */
			if (ismatch && (i_ptr->to_a == o_ptr->to_a) &&
				(i_ptr->to_h == o_ptr->to_h) &&
				(i_ptr->to_d == o_ptr->to_d))
			{
				matches++;
			}

			/* Check for better */
			else if (isbetter && (i_ptr->to_a >= o_ptr->to_a) &&
			         (i_ptr->to_h >= o_ptr->to_h) &&
			         (i_ptr->to_d >= o_ptr->to_d))
			{
					better++;
			}

			/* Check for worse */
			else if (isworse && (i_ptr->to_a <= o_ptr->to_a) &&
			         (i_ptr->to_h <= o_ptr->to_h) &&
			         (i_ptr->to_d <= o_ptr->to_d))
			{
				worse++;
			}

			/* Assume different */
			else
			{
				other++;
			}
		}

		/* Final dump */
		msg(q, i, matches, better, worse, other);
		message_flush();
	}


	/* Hack -- Normally only make a single artifact */
	if (o_ptr->artifact) o_ptr->artifact->created = TRUE;
}


/*
 * Change the quantity of an item
 */
static void wiz_quantity_item(object_type *o_ptr, bool carried)
{
	int tmp_int;

	char tmp_val[3];

	/* Never duplicate artifacts */
	if (o_ptr->artifact) return;

	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->number);

	/* Query */
	if (get_string("Quantity: ", tmp_val, 3))
	{
		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Paranoia */
		if (tmp_int < 1) tmp_int = 1;
		if (tmp_int > 99) tmp_int = 99;

		/* Adjust total weight being carried */
		if (carried)
		{
			/* Remove the weight of the old number of objects */
			player->upkeep->total_weight -= (o_ptr->number * o_ptr->weight);

			/* Add the weight of the new number of objects */
			player->upkeep->total_weight += (tmp_int * o_ptr->weight);
		}

		/* Adjust charges/timeouts for devices */
		reduce_charges(o_ptr, (o_ptr->number - tmp_int));

		/* Accept modifications */
		o_ptr->number = tmp_int;
	}
}


/**
 * Tweak the cursed status of an object.
 *
 * \param o_ptr is the object to curse or decurse
 */
static void wiz_tweak_curse(object_type *o_ptr)
{
	if (cursed_p(o_ptr->flags))
	{
		bitflag f[OF_SIZE];
		msg("Resetting existing curses.");

		create_mask(f, FALSE, OFT_CURSE, OFT_MAX);
		of_diff(o_ptr->flags, f);
	}

	if (get_check("Set light curse? "))
		flags_set(o_ptr->flags, OF_SIZE, OF_LIGHT_CURSE, FLAG_END);
	else if (get_check("Set heavy curse? "))
		flags_set(o_ptr->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, FLAG_END);
	else if (get_check("Set permanent curse? "))
		flags_set(o_ptr->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, OF_PERMA_CURSE, FLAG_END);
}




/*
 * Play with an item. Options include:
 *   - Output statistics (via wiz_roll_item)
 *   - Reroll item (via wiz_reroll_item)
 *   - Change properties (via wiz_tweak_item)
 *   - Change the number of items (via wiz_quantity_item)
 */
static void do_cmd_wiz_play(void)
{
	int item;

	object_type *i_ptr;
	object_type object_type_body;

	object_type *o_ptr;

	struct keypress ch;

	const char *q, *s;

	bool changed = FALSE;
	bool all = TRUE;


	/* Get an item */
	q = "Play with which object? ";
	s = "You have nothing to play with.";
	if (!get_item(&item, q, s, 0, NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR))) return;

	o_ptr = object_from_item_idx(item);

	/* Save screen */
	screen_save();


	/* Get local object */
	i_ptr = &object_type_body;

	/* Copy object */
	object_copy(i_ptr, o_ptr);


	/* The main loop */
	while (TRUE)
	{
		/* Display the item */
		wiz_display_item(i_ptr, all);

		/* Get choice */
		if (!get_com("[a]ccept [s]tatistics [r]eroll [t]weak [c]urse [q]uantity [k]nown? ", &ch))
			break;

		if (ch.code == 'A' || ch.code == 'a')
		{
			changed = TRUE;
			break;
		}
		else if (ch.code == 'c' || ch.code == 'C')
			wiz_tweak_curse(i_ptr);
		else if (ch.code == 's' || ch.code == 'S')
			wiz_statistics(i_ptr, player->depth);
		else if (ch.code == 'r' || ch.code == 'R')
			wiz_reroll_item(i_ptr);
		else if (ch.code == 't' || ch.code == 'T')
			wiz_tweak_item(i_ptr);
		else if (ch.code == 'k' || ch.code == 'K')
			all = !all;
		else if (ch.code == 'q' || ch.code == 'Q')
		{
			bool carried = (item >= 0) ? TRUE : FALSE;
			wiz_quantity_item(i_ptr, carried);
		}
	}


	/* Load screen */
	screen_load();


	/* Accept change */
	if (changed)
	{
		/* Message */
		msg("Changes accepted.");

		/* Change */
		object_copy(o_ptr, i_ptr);

		/* Recalculate gear, bonuses */
		player->upkeep->update |= (PU_INVEN | PU_BONUS);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );
	}

	/* Ignore change */
	else
	{
		msg("Changes ignored.");
	}
}

/*
 * Create the artifact with the specified number
 */
static void wiz_create_artifact(int a_idx)
{
	object_type *i_ptr;
	object_type object_type_body;
	object_kind *kind;

	artifact_type *a_ptr = &a_info[a_idx];

	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Wipe the object */
	object_wipe(i_ptr);

	/* Acquire the "kind" index */
	kind = lookup_kind(a_ptr->tval, a_ptr->sval);
	if (!kind)
		return;

	/* Create the artifact */
	object_prep(i_ptr, kind, a_ptr->alloc_min, RANDOMISE);

	/* Save the name */
	i_ptr->artifact = a_ptr;

	/* Extract the fields */
	copy_artifact_data(i_ptr, a_ptr);

	/* Mark that the artifact has been created. */
	a_ptr->created = TRUE;

	/* Mark as cheat */
	i_ptr->origin = ORIGIN_CHEAT;

	/* Drop the artifact from heaven */
	drop_near(cave, i_ptr, 0, player->py, player->px, TRUE);

	/* All done */
	msg("Allocated.");
	
	/* Redraw map */
	player->upkeep->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(player->upkeep);
}


/*
 * Cure everything instantly
 */
static void do_cmd_wiz_cure_all(void)
{
	/* Remove curses */
	(void)remove_all_curse();

	/* Restore stats */
	effect_simple(EF_RESTORE_STAT, "0", STAT_STR, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, "0", STAT_INT, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, "0", STAT_WIS, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, "0", STAT_DEX, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, "0", STAT_CON, 0, 0, NULL);

	/* Restore the level */
	effect_simple(EF_RESTORE_EXP, "0", 1, 0, 0, NULL);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Cure stuff */
	(void)player_clear_timed(player, TMD_BLIND, TRUE);
	(void)player_clear_timed(player, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(player, TMD_POISONED, TRUE);
	(void)player_clear_timed(player, TMD_AFRAID, TRUE);
	(void)player_clear_timed(player, TMD_PARALYZED, TRUE);
	(void)player_clear_timed(player, TMD_IMAGE, TRUE);
	(void)player_clear_timed(player, TMD_STUN, TRUE);
	(void)player_clear_timed(player, TMD_CUT, TRUE);
	(void)player_clear_timed(player, TMD_SLOW, TRUE);
	(void)player_clear_timed(player, TMD_AMNESIA, TRUE);

	/* No longer hungry */
	player_set_food(player, PY_FOOD_MAX - 1);

	/* Redraw everything */
	do_cmd_redraw();
	
	/* Give the player some feedback */
	msg("You feel *much* better!");
}


/*
 * Go to any level
 */
static void do_cmd_wiz_jump(void)
{
	int depth;

	char ppp[80];
	char tmp_val[160];
	char answer;

	/* Prompt */
	strnfmt(ppp, sizeof(ppp), "Jump to level (0-%d): ", MAX_DEPTH-1);

	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", player->depth);

	/* Ask for a level */
	if (!get_string(ppp, tmp_val, 11)) return;

	/* Extract request */
	depth = atoi(tmp_val);

	/* Paranoia */
	if (depth < 0) depth = 0;

	/* Paranoia */
	if (depth > MAX_DEPTH - 1) depth = MAX_DEPTH - 1;

	/* Prompt */
	strnfmt(ppp, sizeof(ppp), "Choose cave_profile?");

	/* Get to choose generation algorithm */
	answer = get_char(ppp, "yn", 2, 'n');
	if ((answer == 'y') || (answer == 'Y'))
		player->noscore |= NOSCORE_JUMPING;

	/* Accept request */
	msg("You jump to dungeon level %d.", depth);

	/* New depth */
	player->depth = depth;

	/* Leaving */
	player->upkeep->leaving = TRUE;
}


/*
 * Become aware of a lot of objects
 */
static void do_cmd_wiz_learn(int lev)
{
	int i;

	object_type *i_ptr;
	object_type object_type_body;

	/* Scan every object */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		if (!k_ptr || !k_ptr->name) continue;

		/* Induce awareness */
		if (k_ptr->level <= lev)
		{
			/* Get local object */
			i_ptr = &object_type_body;

			/* Prepare object */
			object_prep(i_ptr, k_ptr, 0, MAXIMISE);

			/* Awareness */
			object_flavor_aware(i_ptr);
		}
	}
	
	msg("You now know about many items!");
}


/*
 * Hack -- Rerate Hitpoints
 */
static void do_cmd_rerate(void)
{
	int min_value, max_value, i, percent;

	min_value = (PY_MAX_LEVEL * 3 * (player->hitdie - 1)) / 8;
	min_value += PY_MAX_LEVEL;

	max_value = (PY_MAX_LEVEL * 5 * (player->hitdie - 1)) / 8;
	max_value += PY_MAX_LEVEL;

	player->player_hp[0] = player->hitdie;

	/* Rerate */
	while (1)
	{
		/* Collect values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			player->player_hp[i] = randint1(player->hitdie);
			player->player_hp[i] += player->player_hp[i - 1];
		}

		/* Legal values */
		if ((player->player_hp[PY_MAX_LEVEL - 1] >= min_value) &&
		    (player->player_hp[PY_MAX_LEVEL - 1] <= max_value)) break;
	}

	percent = (int)(((long)player->player_hp[PY_MAX_LEVEL - 1] * 200L) /
	                (player->hitdie + ((PY_MAX_LEVEL - 1) * player->hitdie)));

	/* Update and redraw hitpoints */
	player->upkeep->update |= (PU_HP);
	player->upkeep->redraw |= (PR_HP);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Message */
	msg("Current Life Rating is %d/100.", percent);
}


/*
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
	int i;

	for (i = 0; i < num; i++)
		effect_simple(EF_SUMMON, "1", 0, 0, 0, NULL);
}


/*
 * Summon a creature of the specified type
 *
 * This function is rather dangerous XXX XXX XXX
 */
static void do_cmd_wiz_named(monster_race *r, bool slp)
{
	int py = player->py;
	int px = player->px;

	int i, x, y;

	/* Paranoia */
	assert(r);

	/* Try 10 times */
	for (i = 0; i < 10; i++) {
		int d = 1;

		/* Pick a location */
		scatter(cave, &y, &x, py, px, d, TRUE);

		/* Require empty grids */
		if (!square_isempty(cave, y, x)) continue;

		/* Place it (allow groups) */
		if (place_new_monster(cave, y, x, r, slp, TRUE, ORIGIN_DROP_WIZARD)) break;
	}
}



/*
 * Hack -- Delete all nearby monsters
 */
static void do_cmd_wiz_zap(int d)
{
	int i;

	/* Banish everyone nearby */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > d) continue;

		/* Delete the monster */
		delete_monster_idx(i);
	}

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;
}


/*
 * Query square flags
 */
static void do_cmd_wiz_query(void)
{
	int py = player->py;
	int px = player->px;

	int y, x;

	struct keypress cmd;

	int flag = 0;


	/* Get a "debug command" */
	if (!get_com("Debug Command Query: ", &cmd)) return;

	/* Extract a flag */
	switch (cmd.code)
	{
		case 'm': flag = (SQUARE_MARK); break;
		case 'g': flag = (SQUARE_GLOW); break;
		case 'r': flag = (SQUARE_ROOM); break;
		case 'a': flag = (SQUARE_VAULT); break;
		case 's': flag = (SQUARE_SEEN); break;
		case 'v': flag = (SQUARE_VIEW); break;
		case 'w': flag = (SQUARE_WASSEEN); break;
		case 'd': flag = (SQUARE_DTRAP); break;
		case 'f': flag = (SQUARE_FEEL); break;
		case 'e': flag = (SQUARE_DEDGE); break;
		case 'z': flag = (SQUARE_VERT); break;
		case 't': flag = (SQUARE_TRAP); break;
		case 'n': flag = (SQUARE_INVIS); break;
		case 'i': flag = (SQUARE_WALL_INNER); break;
		case 'o': flag = (SQUARE_WALL_OUTER); break;
		case 'l': flag = (SQUARE_WALL_SOLID); break;
		case 'x': flag = (SQUARE_MON_RESTRICT); break;
	}

	/* Scan map */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
	{
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
		{
			byte a = TERM_RED;

			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Given flag, show only those grids */
			if (!sqinfo_has(cave->info[y][x], flag)) continue;

			/* Given no flag, show unknown grids */
			if (!flag && (!square_ismark(cave, y, x))) continue;

			/* Color */
			if (square_ispassable(cave, y, x)) a = TERM_YELLOW;

			/* Display player/floors/walls */
			if ((y == py) && (x == px))
				print_rel(L'@', a, y, x);
			else if (square_ispassable(cave, y, x))
				print_rel(L'*', a, y, x);
			else
				print_rel(L'#', a, y, x);
		}
	}

	Term_redraw();

	/* Get keypress */
	msg("Press any key.");
	inkey_ex();
	message_flush();

	/* Redraw map */
	prt_map();
}

/*
 * Query terrain
 */
static void do_cmd_wiz_features(void)
{
	int py = player->py;
	int px = player->px;

	int y, x;

	struct keypress cmd;

	/* OMG hax */
	int *feat = NULL;
	int featf[] = {FEAT_FLOOR};
	int feato[] = {FEAT_OPEN};
	int featb[] = {FEAT_BROKEN};
	int featu[] = {FEAT_LESS};
	int featz[] = {FEAT_MORE};
	int featt[] = {FEAT_LESS, FEAT_MORE};
	int featc[] = {FEAT_CLOSED};
	int featd[] = {FEAT_CLOSED, FEAT_OPEN, FEAT_BROKEN, FEAT_SECRET};
	int feath[] = {FEAT_SECRET};
	int featm[] = {FEAT_MAGMA, FEAT_MAGMA_K};
	int featq[] = {FEAT_QUARTZ, FEAT_QUARTZ_K};
	int featg[] = {FEAT_GRANITE};
	int featp[] = {FEAT_PERM};
	int featr[] = {FEAT_RUBBLE};
	int length = 0;


	/* Get a "debug command" */
	if (!get_com("Debug Command Feature Query: ", &cmd)) return;

	/* Choose a feature (type) */
	switch (cmd.code)
	{
		/* Floors */
		case 'f': feat = featf; length = 1; break;
		/* Open doors */
		case 'o': feat = feato; length = 1; break;
		/* Broken doors */
		case 'b': feat = featb; length = 1; break;
		/* Upstairs */
		case 'u': feat = featu; length = 1; break;
		/* Downstairs */
		case 'z': feat = featz; length = 1; break;
		/* Stairs */
		case 't': feat = featt; length = 2; break;
		/* Closed doors */
		case 'c': feat = featc; length = 8; break;
		/* Doors */
		case 'd': feat = featd; length = 11; break;
		/* Secret doors */
		case 'h': feat = feath; length = 1; break;
		/* Magma */
		case 'm': feat = featm; length = 3; break;
		/* Quartz */
		case 'q': feat = featq; length = 3; break;
		/* Granite */
		case 'g': feat = featg; length = 1; break;
		/* Permanent wall */
		case 'p': feat = featp; length = 1; break;
		/* Rubble */
		case 'r': feat = featr; length = 1; break;
	}

	/* Scan map */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
	{
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
		{
			byte a = TERM_RED;
			bool show = FALSE;
			int i;

			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Given feature, show only those grids */
			for (i = 0; i < length; i++)
				if (cave->feat[y][x] == feat[i]) show = TRUE;

			/* Color */
			if (square_ispassable(cave, y, x)) a = TERM_YELLOW;

			if (!show) continue;

			/* Display player/floors/walls */
			if ((y == py) && (x == px))
				print_rel(L'@', a, y, x);
			else if (square_ispassable(cave, y, x))
				print_rel(L'*', a, y, x);
			else
				print_rel(L'#', a, y, x);
		}
	}

	Term_redraw();

	/* Get keypress */
	msg("Press any key.");
	inkey_ex();
	message_flush();

	/* Redraw map */
	prt_map();
}

/*
 * Create lots of items.
 */
static void wiz_test_kind(int tval)
{
	int py = player->py;
	int px = player->px;
	int sval;

	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	for (sval = 0; sval < 255; sval++)
	{
		object_kind *kind = lookup_kind(tval, sval);
		if (!kind) continue;

		/* Create the item */
		object_prep(i_ptr, kind, player->depth, RANDOMISE);

		/* Apply magic (no messages, no artifacts) */
		apply_magic(i_ptr, player->depth, FALSE, FALSE, FALSE, FALSE);

		/* Mark as cheat, and where created */
		i_ptr->origin = ORIGIN_CHEAT;
		i_ptr->origin_depth = player->depth;

		if (tval == TV_GOLD)
			make_gold(i_ptr, player->depth, lookup_kind(TV_GOLD, sval)->name);

		/* Drop the object from heaven */
		drop_near(cave, i_ptr, 0, py, px, TRUE);
	}

	msg("Done.");
}

/*
 * Display the debug commands help file.
 */
static void do_cmd_wiz_help(void) 
{
	char buf[80];
	strnfmt(buf, sizeof(buf), "debug.txt");
	screen_save();
	show_file(buf, NULL, 0, 0);
	screen_load();
}

/*
 * Advance the player to level 50 with max stats and other bonuses.
 */
static void do_cmd_wiz_advance(void)
{
	int i;

	/* Max stats */
	for (i = 0; i < STAT_MAX; i++)
		player->stat_cur[i] = player->stat_max[i] = 118;

	/* Lots of money */
	player->au = 1000000L;

	/* Level 50 */
	player_exp_gain(player, PY_MAX_EXP);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Get some awesome equipment */
	/* Artifacts: 3, 5, 12, ...*/
	
	/* Update stuff */
	player->upkeep->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Redraw everything */
	player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN |
			PR_EQUIP | PR_MESSAGE | PR_MONSTER | PR_OBJECT | PR_MONLIST |
			PR_ITEMLIST);

	/* Hack -- update */
	handle_stuff(player->upkeep);

}

/**
 * Prompt for an effect and perform it.
 */
void do_cmd_wiz_effect(void)
{
	char name[80] = "";
	char dice[80] = "0";
	int index = -1;
	int p1 = 0, p2 = 0, p3 = 0;
	bool ident = FALSE;

	/* Avoid the prompt getting in the way */
	screen_save();

	/* Prompt */
	prt("Do which effect? ", 0, 0);

	/* Get the name */
	if (askfor_aux(name, sizeof(name), NULL)) {
		/* See if an effect index was entered */
		index = get_idx_from_name(name);

		/* If not, find the effect with that name */
		if (index <= EF_NONE || index >= EF_MAX)
			index = effect_lookup(name);
	}

	/* Prompt */
	prt("Enter damage dice (eg 1+2d6M2): ", 0, 0);

	/* Get the dice */
	if (!askfor_aux(dice, sizeof(dice), NULL))
		my_strcpy(dice, "0", sizeof(dice));

	/* Get the parameters */
	prt("Enter name or number for first parameter: ", 0, 0);

	/* Get the name */
	if (askfor_aux(name, sizeof(name), NULL)) {
		/* See if an effect parameter was entered */
		p1 = effect_param(name);
		if (p1 == -1) p1 = 0;
	}

	p2 = get_quantity("Enter second parameter: ", 100);
	p3 = get_quantity("Enter third parameter: ", 100);

	/* Reload the screen */
	screen_load();

	if (index > EF_NONE && index < EF_MAX)
		effect_simple(index, dice, p1, p2, p3, &ident);
	else
		msg("No effect found.");

	if (ident)
		msg("Identified!");
}

/*
 * Ask for and parse a "debug command"
 */
void do_cmd_debug(void)
{
	int py = player->py;
	int px = player->px;

	struct keypress cmd;


	/* Get a "debug command" */
	if (!get_com("Debug Command: ", &cmd)) return;

	/* Analyze the command */
	switch (cmd.code)
	{
		/* Ignore */
		case ESCAPE:
		case ' ':
		case KC_ENTER:
		{
			break;
		}

		/* Hack -- Generate Spoilers */
		case '"':
		{
			do_cmd_spoilers();
			break;
		}

		/* Hack -- Help */
		case '?':
		{
			do_cmd_wiz_help();
			break;
		}

		/* Cure all maladies */
		case 'a':
		{
			do_cmd_wiz_cure_all();
			break;
		}

		/* Make the player powerful */
		case 'A':
		{
			do_cmd_wiz_advance();
			break;
		}
		
		/* Teleport to target */
		case 'b':
		{
			do_cmd_wiz_bamf();
			break;
		}

		/* Create any object */
		case 'c':
		{
			wiz_create_item();
			break;
		}

		/* Create an artifact */
		case 'C':
		{
			char name[80] = "";
			int a_idx = -1;

			/* Avoid the prompt getting in the way */
			screen_save();

			/* Prompt */
			prt("Create which artifact? ", 0, 0);

			/* Get the name */
			if (askfor_aux(name, sizeof(name), NULL))
			{
				/* See if an a_idx was entered */
				a_idx = get_idx_from_name(name);

				/* If not, find the artifact with that name */
				if (a_idx < 1)
					a_idx = lookup_artifact_name(name); 

				/* Did we find a valid artifact? */
				if (a_idx != -1 && a_idx < z_info->a_max)
					wiz_create_artifact(a_idx);
				else
					msg("No artifact found.");
			}
				
			/* Reload the screen */
			screen_load();

			break;
		}

		/* Detect everything */
		case 'd':
		{
			effect_simple(EF_DETECT_TRAPS, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_DOORS, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_STAIRS, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_GOLD, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_OBJECTS, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_VISIBLE_MONSTERS, "22d40", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_INVISIBLE_MONSTERS, "22d40", 0, 0, 0, NULL);
			break;
		}
		
		/* Test for disconnected dungeon */
		case 'D':
		{
			disconnect_stats();
			break;
		}

		/* Edit character */
		case 'e':
		{
			do_cmd_wiz_change();
			break;
		}

		/* Perform an effect. */
		case 'E':
		{
			do_cmd_wiz_effect();
			break;
		}

		case 'f':
		{
			stats_collect();
			break;
		}

		case 'F':
		{
			do_cmd_wiz_features();
			break;
		}

		/* Good Objects */
		case 'g':
		{
			int n;
			screen_save();
			n= get_quantity("How many good objects? ", 40);
			screen_load();
			if (n < 1) n = 1;
			acquirement(py, px, player->depth, n, FALSE);
			break;
		}

		/* GF demo */
		case 'G':
		{
			wiz_gf_demo();
			break;
		}

		/* Hitpoint rerating */
		case 'h':
		{
			do_cmd_rerate();
			break;
		}
        
        /* Hit all monsters in LOS */
        case 'H':
        {
			effect_simple(EF_PROJECT_LOS, "10000", GF_DISP_ALL, 0, 0, NULL);
            break;
        }

		/* Identify */
		case 'i':
		{
			effect_simple(EF_IDENTIFY, "0", 0, 0, 0, NULL);
			break;
		}

		/* Go up or down in the dungeon */
		case 'j':
		{
			do_cmd_wiz_jump();
			break;
		}

		/* Learn about objects */
		case 'l':
		{
			do_cmd_wiz_learn(100);
			break;
		}

		case 'L': do_cmd_keylog(); break;

		/* Magic Mapping */
		case 'm':
		{
			effect_simple(EF_MAP_AREA, "22d40", 0, 0, 0, NULL);
			break;
		}

		/* Summon Named Monster */
		case 'n':
		{
			monster_race *r = NULL;
			char name[80] = "";

			/* Avoid the prompt getting in the way */
			screen_save();

			/* Prompt */
			prt("Summon which monster? ", 0, 0);

			/* Get the name */
			if (askfor_aux(name, sizeof(name), NULL))
			{
				/* See if a r_idx was entered */
				int r_idx = get_idx_from_name(name);
				if (r_idx)
					r = &r_info[r_idx];
				else
					/* If not, find the monster with that name */
					r = lookup_monster(name); 
					
				player->upkeep->redraw |= (PR_MAP | PR_MONLIST);
			}

			/* Reload the screen */
			screen_load();

			if (r)
				do_cmd_wiz_named(r, TRUE);
			else
				msg("No monster found.");
			
			break;
		}

		/* Object playing routines */
		case 'o':
		{
			do_cmd_wiz_play();
			break;
		}

		/* Phase Door */
		case 'p':
		{
			const char *near = "10";
			effect_simple(EF_TELEPORT, near, 0, 1, 0, NULL);
			break;
		}

		/* Monster pit stats */
		case 'P':
		{
			pit_stats();
			break;
		}
		
		/* Query the dungeon */
		case 'q':
		{
			do_cmd_wiz_query();
			break;
		}

		/* Get full recall for a monster */
		case 'r':
		{
			const monster_race *r_ptr = NULL;

			struct keypress sym;
			const char *prompt =
				"Full recall for [a]ll monsters or [s]pecific monster? ";

			if (!get_com(prompt, &sym)) return;

			if (sym.code == 'a' || sym.code == 'A')
			{
				int i;
				for (i = 0; i < z_info->r_max; i++)
					cheat_monster_lore(&r_info[i], &l_list[i]);
				msg("Done.");
			}
			else if (sym.code == 's' || sym.code == 'S')
			{
				char name[80] = "";
					
				/* Avoid the prompt getting in the way */
				screen_save();

				/* Prompt */
				prt("Which monster? ", 0, 0);

				/* Get the name */
				if (askfor_aux(name, sizeof(name), NULL))
				{
					/* See if a r_idx was entered */
					int r_idx = get_idx_from_name(name);
					if (r_idx)
						r_ptr = &r_info[r_idx];
					else
						/* If not, find the monster with that name */
						r_ptr = lookup_monster(name); 
				}

				/* Reload the screen */
				screen_load();

				/* Did we find a valid monster? */
				if (r_ptr)
					cheat_monster_lore(r_ptr, get_lore(r_ptr));
				else
					msg("No monster found.");
			}

			break;
		}

		/* Summon Random Monster(s) */
		case 's':
		{
			int n;
			screen_save();
			n = get_quantity("How many monsters? ", 40);
			screen_load();
			if (n < 1) n = 1;
			do_cmd_wiz_summon(n);
			break;
		}
		
		/* Collect stats (S) */
		case 'S':
		{
			stats_collect();
			break;
		}

		/* Teleport */
		case 't':
		{
			const char *far = "100";
			effect_simple(EF_TELEPORT, far, 0, 1, 0, NULL);
			break;
		}

		/* Create a trap */
		case 'T':
		{
			if (!square_isfloor(cave, player->py, player->px))
				msg("You can't place a trap there!");
			else if (player->depth == 0)
				msg("You can't place a trap in the town!");
			else
				square_add_trap(cave, player->py, player->px);
			break;
		}

		/* Un-hide all monsters */
		case 'u':
		{
			effect_simple(EF_DETECT_VISIBLE_MONSTERS, "500d500", 0, 0, 0, NULL);
			effect_simple(EF_DETECT_INVISIBLE_MONSTERS, "500d500", 0, 0, 0, NULL);
			break;
		}

		/* Very Good Objects */
		case 'v':
		{
			int n;
			screen_save();
			n = get_quantity("How many great objects? ", 40);
			screen_load();
			if (n < 1) n = 1;
			acquirement(py, px, player->depth, n, TRUE);
			break;
		}

		case 'V':
		{
			int n;
			screen_save();
			n = get_quantity("Create all items of what tval? ", 255);
			screen_load();
			if (n)
				wiz_test_kind(n);
			break;
		}

		/* Wizard Light the Level */
		case 'w':
		{
			wiz_light(cave, TRUE);
			break;
		}

		/* Wipe recall for a monster */
		case 'W':
		{
			const monster_race *r_ptr = NULL;
			s16b r_idx = 0; 

			struct keypress sym;
			const char *prompt =
				"Wipe recall for [a]ll monsters or [s]pecific monster? ";

			if (!get_com(prompt, &sym)) return;

			if (sym.code == 'a' || sym.code == 'A')
			{
				int i;
				for (i = 0; i < z_info->r_max; i++)
					wipe_monster_lore(&r_info[i], &l_list[i]);
				msg("Done.");
			}
			else if (sym.code == 's' || sym.code == 'S')
			{
				char name[80] = "";

				/* Avoid the prompt getting in the way */
				screen_save();

				/* Prompt */
				prt("Which monster? ", 0, 0);

				/* Get the name */
				if (askfor_aux(name, sizeof(name), NULL))
				{
					/* See if a r_idx was entered */
					r_idx = get_idx_from_name(name);
					if (r_idx)
						r_ptr = &r_info[r_idx];
					else
						/* If not, find the monster with that name */
						r_ptr = lookup_monster(name); 
				}
					
				/* Reload the screen */
				screen_load();

				/* Did we find a valid monster? */
				if (r_ptr)
					wipe_monster_lore(r_ptr, get_lore(r_ptr));
				else
					msg("No monster found.");
			}
	
			break;
		}

		/* Increase Experience */
		case 'x':
		{
			int n;
			screen_save();
			n = get_quantity("Gain how much experience? ", 9999);
			screen_load();
			if (n < 1) n = 1;
			player_exp_gain(player, n);
			break;
		}

		/* Zap Monsters (Banishment) */
		case 'z':
		{
			int n;
			screen_save();
			n = get_quantity("Zap within what distance? ", MAX_SIGHT);
			screen_load();
			do_cmd_wiz_zap(n);
			break;
		}

		/* Hack */
		case '_':
		{
			do_cmd_wiz_hack_ben();
			break;
		}

		/* Oops */
		default:
		{
			msg("That is not a valid debug command.");
			break;
		}
	}
}
