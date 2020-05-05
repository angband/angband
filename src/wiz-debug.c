/**
 * \file wiz-debug.c
 * \brief Debug mode commands
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
#include "game-input.h"
#include "grafmode.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"
#include "trap.h"
#include "ui-command.h"
#include "ui-event.h"
#include "ui-display.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-menu.h"
#include "ui-prefs.h"
#include "ui-target.h"
#include "wizard.h"


static void proj_display(struct menu *m, int type, bool cursor,
		int row, int col, int wid)
{
	size_t i;

	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	const char *proj_name = proj_idx_to_name(type);

	if (type % 2)
		c_prt(attr, ".........................", row, col);
	c_put_str(attr, proj_name, row, col);

	col += 25;

	if (tile_height == 1) {
		for (i = 0; i < BOLT_MAX; i++) {
			if (use_graphics == GRAPHICS_NONE) {
				/* ASCII is simple */
				wchar_t chars[] = L"*|/-\\";

				col += big_pad(col, row, projections[type].color, chars[i]);
			} else {
				col += big_pad(col, row, proj_to_attr[type][i],
							   proj_to_char[type][i]);
			}
		}
	} else {
		prt("Change tile_height to 1 to see graphics.", row, col);
	}
}

static const menu_iter proj_iter = {
	NULL, /* get_tag */
	NULL, /* validity */
	proj_display,
	NULL, /* action */
	NULL /* resize */
};

static void wiz_proj_demo(void)
{
	struct menu *m = menu_new(MN_SKIN_SCROLL, &proj_iter);
	region loc = { 0, 0, 0, 0 };

	menu_setpriv(m, PROJ_MAX, NULL);

	m->title = "PROJ_ types display";
	menu_layout(m, &loc);

	screen_save();
	clear_from(0);
	menu_select(m, 0, false);
	screen_load();
	mem_free(m);
}








/**
 * This is a nice utility function; it determines if a (NULL-terminated)
 * string consists of only digits (starting with a non-zero digit).
 */
static s16b get_idx_from_name(char *s)
{
	char *endptr = NULL;
	long l = strtol(s, &endptr, 10);
	return *endptr == '\0' ? (s16b)l : 0;
}

/**
 * Display in sequence the squares at n grids from the player, as measured by
 * the noise and scent algorithms; n goes from 1 to max flow depth
 */
static void do_cmd_wiz_hack_nick(void)
{
	int i, y, x;

	char kp;

	/* Noise */
	for (i = 0; i < 100; i++) {
		/* Update map */
		for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
			for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++) {
				byte a = COLOUR_RED;
				struct loc grid = loc(x, y);

				if (!square_in_bounds_fully(cave, grid)) continue;

				/* Display proper noise */
				if (cave->noise.grids[y][x] != i) continue;

				/* Display player/floors/walls */
				if (loc_eq(grid, player->grid))
					print_rel(L'@', a, y, x);
				else if (square_ispassable(cave, grid))
					print_rel(L'*', a, y, x);
				else
					print_rel(L'#', a, y, x);
			}

		/* Get key */
		if (!get_com(format("Depth %d: ", i), &kp))
			break;

		/* Redraw map */
		prt_map();
	}

	/* Smell */
	for (i = 0; i < 50; i++) {
		/* Update map */
		for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
			for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++) {
				byte a = COLOUR_YELLOW;
				struct loc grid = loc(x, y);

				if (!square_in_bounds_fully(cave, grid)) continue;

				/* Display proper smell */
				if (cave->scent.grids[y][x] != i) continue;

				/* Display player/floors/walls */
				if (loc_eq(grid, player->grid))
					print_rel(L'@', a, y, x);
				else if (square_ispassable(cave, grid))
					print_rel(L'*', a, y, x);
				else
					print_rel(L'#', a, y, x);
			}

		/* Get key */
		if (!get_com(format("Depth %d: ", i), &kp))
			break;

		/* Redraw map */
		prt_map();
	}

	/* Done */
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}



/**
 * Output part of a bitflag set in binary format.
 */
static void prt_binary(const bitflag *flags, int offset, int row, int col,
					   char ch, int num)
{
	int flag;

	/* Scan the flags */
	for (flag = FLAG_START + offset; flag < FLAG_START + offset + num; flag++)
		if (of_has(flags, flag))
			Term_putch(col++, row, COLOUR_BLUE, ch);
		else
			Term_putch(col++, row, COLOUR_WHITE, '-');
}

/**
 * This ugly piece of code exists to figure out what keycodes the user has
 * been generating.
 */
static void do_cmd_keylog(void) {
	int i;
	char buf[50];
	char buf2[12];
	struct keypress keys[2] = {KEYPRESS_NULL, KEYPRESS_NULL};

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
			keypress_to_text(buf2, sizeof(buf2), keys, true);

			/* format this line of output */
			strnfmt(buf, sizeof(buf), "    %-12s (code=%u mods=%u)", buf2,
					k.code, k.mods);
		} else {
			/* create a blank line of output */
			strnfmt(buf, sizeof(buf), "%40s", "");
		}

		prt(buf, i + 1, 0);
	}

	prt("Press any key to continue.", KEYLOG_SIZE + 1, 0);
	anykey();
	screen_load();
}


/**
 * Teleport to the requested target
 */
static void do_cmd_wiz_bamf(void)
{
	struct loc grid = loc(0, 0);

	/* Use the targeting function. */
	if (!target_set_interactive(TARGET_LOOK, -1, -1))
		return;

	/* grab the target coords. */
	target_get(&grid);

	/* Test for passable terrain. */
	if (!square_ispassable(cave, grid)) {
		msg("The square you are aiming for is impassable.");

	} else {
		/* Teleport to the target */
		effect_simple(EF_TELEPORT_TO, source_player(), "0", 0, 0, 0, grid.y,
					  grid.x, NULL);
	}
}



/**
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
	for (i = 0; i < STAT_MAX; i++) {
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
		player_exp_lose(player, player->exp - tmp_long, false);
}


/**
 * Change player stats, gold and experience.
 */
static void do_cmd_wiz_change(void)
{
	/* Interact */
	do_cmd_wiz_change_aux();

	/* Redraw everything */
	do_cmd_redraw();
}


/**
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


/**
 * Display an item's properties
 */
static void wiz_display_item(const struct object *obj, bool all)
{
	int j = 0;
	bitflag f[OF_SIZE];
	char buf[256];


	/* Extract the flags */
	if (all)
		object_flags(obj, f);
	else
		object_flags_known(obj, f);

	/* Clear screen */
	Term_clear();

	/* Describe fully */
	object_desc(buf, sizeof(buf), obj,
				ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

	prt(buf, 2, j);

	prt(format("combat = (%dd%d) (%+d,%+d) [%d,%+d]",
	           obj->dd, obj->ds, obj->to_h, obj->to_d, obj->ac,
			   obj->to_a), 4, j);

	prt(format("kind = %-5d  tval = %-5d  sval = %-5d  wgt = %-3d     timeout = %-d",
	           obj->kind->kidx, obj->tval, obj->sval, obj->weight,
			   obj->timeout), 5, j);

	prt(format("number = %-3d  pval = %-5d  name1 = %-4d  egoidx = %-4d  cost = %ld",
			   obj->number, obj->pval,
			   obj->artifact ? obj->artifact->aidx : 0,
			   obj->ego ? obj->ego->eidx : 0,
			   (long)object_value(obj, 1)), 6, j);

	prt("+------------FLAGS-------------+", 16, j);
	prt("SUST.PROT<-OTHER--><BAD->CUR....", 17, j);
	prt("     fbcssf  s  ibniiatadlhp....", 18, j);
	prt("siwdcelotdfrei  plommfegrccc....", 19, j);
	prt("tnieoannuiaesnfhcefhsrlgxuuu....", 20, j);
	prt("rtsxnrdfnglgpvaltsuppderprrr....", 21, j);
	prt_binary(f, 0, 22, j, '*', 28);
	if (obj->known) {
		prt_binary(obj->known->flags, 0, 23, j, '+', 28);
	}
}



/** Object creation code **/
static bool choose_artifact = false;

static const region wiz_create_item_area = { 0, 0, 0, 0 };

/**
 * Build an "artifact name" and transfer it into a buffer.
 */
static void get_art_name(char *buf, int max, int a_idx)
{
	struct object *obj, *known_obj;
	struct object_kind *kind;
	struct artifact *art = &a_info[a_idx];

	/* Get object */
	obj = object_new();

	/* Acquire the "kind" index */
	kind = lookup_kind(art->tval, art->sval);

	/* Oops */
	if (!kind)
		return;

	/* Create the base object */
	object_prep(obj, kind, 0, RANDOMISE);

	/* Mark it as an artifact */
	obj->artifact = art;

	/* Make it known to us */
	known_obj = object_new();
	obj->known = known_obj;
	object_copy(known_obj, obj);
	known_obj->notice |= OBJ_NOTICE_IMAGINED;

	/* Create the artifact description */
	object_desc(buf, max, obj, ODESC_SINGULAR | ODESC_SPOIL);

	object_delete(&known_obj);
	obj->known = NULL;
	object_delete(&obj);
}

#define WIZ_CREATE_ALL_MENU_ITEM -9999

/**
 * Create an instance of an object of a given kind.
 *
 * \param kind The base type of object to instantiate.
 * \return An object of the provided object type.
 */
static struct object *wiz_create_item_object_from_kind(struct object_kind *kind)
{
	struct object *obj;

	/* Create the item */
	if (tval_is_money_k(kind))
		obj = make_gold(player->depth, kind->name);
	else {
		/* Get object */
		obj = object_new();
		object_prep(obj, kind, player->depth, RANDOMISE);

		/* Apply magic (no messages, no artifacts) */
		apply_magic(obj, player->depth, false, false, false, false);
	}

	return obj;
}

/**
 * Create an instance of an artifact.
 *
 * \param art The artifact to instantiate.
 * \return An object that represents the artifact.
 */
static struct object *wiz_create_item_object_from_artifact(struct artifact *art)
{
	struct object_kind *kind;
	struct object *obj;

	/* Ignore "empty" artifacts */
	if (!art->name) return NULL;

	/* Acquire the "kind" index */
	kind = lookup_kind(art->tval, art->sval);
	if (!kind)
		return NULL;

	/* Get object */
	obj = object_new();

	/* Create the artifact */
	object_prep(obj, kind, art->alloc_min, RANDOMISE);
	obj->artifact = art;
	copy_artifact_data(obj, art);

	/* Mark that the artifact has been created. */
	art->created = true;

	return obj;
}

/**
 * Drop an object near the player in a manner suitable for debugging.
 *
 * \param obj The object to drop.
 */
static void wiz_create_item_drop_object(struct object *obj)
{
	if (obj == NULL)
		return;

	/* Mark as cheat, and where created */
	obj->origin = ORIGIN_CHEAT;
	obj->origin_depth = player->depth;

	/* Drop the object from heaven */
	drop_near(cave, &obj, 0, player->grid, true, true);
}

/**
 * Drop all possible artifacts or objects by the player.
 *
 * \param create_artifacts When true, all artifacts will be created; when false,
 *		  all regular objects will be created.
 */
static void wiz_create_item_all_items(bool create_artifacts)
{
	int i;
	struct object *obj;
	struct object_kind *kind;
	struct artifact *art;

	if (create_artifacts) {
		for (i = 1; i < z_info->a_max; i++) {
			art = &a_info[i];
			obj = wiz_create_item_object_from_artifact(art);
			wiz_create_item_drop_object(obj);
		}
	}
	else {
		for (i = 0; i < z_info->k_max; i++) {
			kind = &k_info[i];

			if (kind->base == NULL || kind->base->name == NULL)
				continue;

			if (kf_has(kind->kind_flags, KF_INSTA_ART))
				continue;

			obj = wiz_create_item_object_from_kind(kind);
			wiz_create_item_drop_object(obj);
		}
	}
}

/**
 * Artifact or object kind selection
 */
static void wiz_create_item_subdisplay(struct menu *m, int oid, bool cursor,
		int row, int col, int width)
{
	int *choices = menu_priv(m);
	int selected = choices[oid];
	char buf[70];

	if (selected == WIZ_CREATE_ALL_MENU_ITEM) {
		/* Super big hack: the special flag should be the last menu item, with
		 * the selected tval stored in the next element. */
		int current_tval = choices[oid + 1];
		char name[70];

		object_base_name(name, sizeof(name), current_tval, true);
		if (choose_artifact)
			strnfmt(buf, sizeof(buf), "All artifact %s", name);
		else
			strnfmt(buf, sizeof(buf), "All %s", name);
	}
	else {
		if (choose_artifact)
			get_art_name(buf, sizeof(buf), selected);
		else
			object_kind_name(buf, sizeof(buf), &k_info[selected], true);
	}

	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_subaction(struct menu *m, const ui_event *e, int oid)
{
	int *choices = menu_priv(m);
	int selected = choices[oid];
	struct object_kind *kind;
	struct object *obj;
	struct artifact *art;

	if (e->type != EVT_SELECT)
		return true;

	if (selected == WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		int cur;
		for (cur = 0; cur < oid; cur++) {
			kind = &k_info[choices[cur]];
			obj = wiz_create_item_object_from_kind(kind);
			wiz_create_item_drop_object(obj);
		}
	}
	else if (selected == WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		int cur;
		for (cur = 0; cur < oid; cur++) {
			art = &a_info[choices[cur]];
			obj = wiz_create_item_object_from_artifact(art);
			wiz_create_item_drop_object(obj);
		}
	}
	else if (selected != WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		kind = &k_info[choices[oid]];
		obj = wiz_create_item_object_from_kind(kind);
		wiz_create_item_drop_object(obj);
	}
	else if (selected != WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		art = &a_info[choices[oid]];
		obj = wiz_create_item_object_from_artifact(art);
		wiz_create_item_drop_object(obj);
	}

	return false;
}

static menu_iter wiz_create_item_submenu =
{
	NULL,
	NULL,
	wiz_create_item_subdisplay,
	wiz_create_item_subaction,
	NULL
};

/**
 * Object base kind selection
 */

static void wiz_create_item_display(struct menu *m, int oid, bool cursor,
		int row, int col, int width)
{
	char buf[80];

	if (oid == WIZ_CREATE_ALL_MENU_ITEM) {
		if (choose_artifact)
			my_strcpy(buf, "All artifacts", sizeof(buf));
		else
			my_strcpy(buf, "All objects", sizeof(buf));
	}
	else {
		object_base_name(buf, sizeof(buf), oid, true);
	}

	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_action(struct menu *m, const ui_event *e, int oid)
{
	ui_event ret;
	struct menu *menu;

	char buf[80];
	char title[80];

	int choice[70];
	int num;

	int i;

	if (e->type != EVT_SELECT)
		return true;

	if (oid == WIZ_CREATE_ALL_MENU_ITEM) {
		wiz_create_item_all_items(choose_artifact);
		return false;
	}

	/* Artifacts */
	if (choose_artifact) {
		/* ...We have to search the whole artifact list. */
		for (num = 0, i = 1; (num < 60) && (i < z_info->a_max); i++) {
			struct artifact *art = &a_info[i];

			if (art->tval != oid) continue;

			choice[num++] = i;
		}
	} else {
		/* Regular objects */
		for (num = 0, i = 1; (num < 60) && (i < z_info->k_max); i++) {
			struct object_kind *kind = &k_info[i];

			if (kind->tval != oid || kf_has(kind->kind_flags, KF_INSTA_ART))
				continue;

			choice[num++] = i;
		}
	}

	/* Add a flag for an "All <tval>" item to create all svals of that tval. The
	 * tval is stored (in a super hacky way) beyond the end of the valid menu
	 * items. The menu won't render it, but we can still get to it without 
	 * doing a bunch of work. */
	choice[num++] = WIZ_CREATE_ALL_MENU_ITEM;
	choice[num] = oid;

	screen_save();
	clear_from(0);

	menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_submenu);
	menu->selections = all_letters;

	object_base_name(buf, sizeof(buf), oid, true);
	if (choose_artifact)
		strnfmt(title, sizeof(title), "Which artifact %s? ", buf);
	else
		strnfmt(title, sizeof(title), "What kind of %s?", buf);
	menu->title = title;

	menu_setpriv(menu, num, choice);
	menu_layout(menu, &wiz_create_item_area);
	ret = menu_select(menu, 0, false);

	screen_load();
	mem_free(menu);

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


/**
 * Choose and create an instance of an artifact or object kind
 */
static void wiz_create_item(bool art)
{
	int tvals[TV_MAX];
	int i, n;

	struct menu *menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_menu);

	choose_artifact = art;

	menu->selections = all_letters;
	menu->title = art ? "What kind of artifact?" : "What kind of object?";

	/* Make a list of all tvals for the filter */
	for (i = 0, n = 0; i < TV_MAX; i++) {
		/* Only real object bases */
		if (!kb_info[i].name) continue;

		/* For artifact creation, only include tvals which have an artifact */
		if (art) {
			int j;
			for (j = 1; j < z_info->a_max; j++) {
				struct artifact *art_local = &a_info[j];
				if (art_local->tval == i) break;
			}
			if (j == z_info->a_max) continue;
		}

		tvals[n++] = i;
	}

	tvals[n++] = WIZ_CREATE_ALL_MENU_ITEM;

	screen_save();
	clear_from(0);

	menu_setpriv(menu, TV_MAX, kb_info);
	menu_set_filter(menu, tvals, n);
	menu_layout(menu, &wiz_create_item_area);
	menu_select(menu, 0, false);

	screen_load();
	mem_free(menu);
	
	/* Redraw map */
	player->upkeep->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(player);

}

/**
 * Tweak an item - make it ego or artifact, give values for modifiers, to_a,
 * to_h or to_d
 */
static void wiz_tweak_item(struct object *obj)
{
	const char *p;
	char tmp_val[80];
	int i, val;

	/* Hack -- leave artifacts alone */
	if (obj->artifact) return;

	/* Get ego name */
	p = "Enter new ego item: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (obj->ego) {
		strnfmt(tmp_val, sizeof(tmp_val), "%s", obj->ego->name);
	}
	if (!get_string(p, tmp_val, 30)) return;

	/* Accept index or name */
	val = get_idx_from_name(tmp_val);
	if (val) {
		obj->ego = &e_info[val];
	} else {
		obj->ego = lookup_ego_item(tmp_val, obj->tval, obj->sval);
	}
	if (obj->ego) {
		struct ego_item *e = obj->ego;
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known = obj->known;
		object_prep(obj, obj->kind, player->depth, RANDOMISE);
		obj->ego = e;
		obj->prev = prev;
		obj->next = next;
		obj->known = known;
		ego_apply_magic(obj, player->depth);
	}
	wiz_display_item(obj, true);

	/* Get artifact name */
	p = "Enter new artifact: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (obj->artifact)
		strnfmt(tmp_val, sizeof(tmp_val), "%s", obj->artifact->name);
	if (!get_string(p, tmp_val, 30)) return;

	/* Accept index or name */
	val = get_idx_from_name(tmp_val);
	if (val) {
		obj->artifact = &a_info[val];
	} else {
		obj->artifact = lookup_artifact_name(tmp_val);
	}
	if (obj->artifact) {
		struct artifact *a = obj->artifact;
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known = obj->known;
		obj->ego = NULL;
		object_prep(obj, obj->kind, obj->artifact->alloc_min, RANDOMISE);
		obj->artifact = a;
		obj->prev = prev;
		obj->next = next;
		obj->known = known;
		copy_artifact_data(obj, obj->artifact);
	}
	wiz_display_item(obj, true);

#define WIZ_TWEAK(attribute) do {\
	p = "Enter new '" #attribute "' setting: ";\
	strnfmt(tmp_val, sizeof(tmp_val), "%d", obj->attribute);\
	if (!get_string(p, tmp_val, 6)) return;\
	obj->attribute = atoi(tmp_val);\
	wiz_display_item(obj, true);\
} while (0)
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		WIZ_TWEAK(modifiers[i]);
	}
	WIZ_TWEAK(to_a);
	WIZ_TWEAK(to_h);
	WIZ_TWEAK(to_d);
}


/**
 * Apply magic to an item or turn it into an artifact. -Bernd-
 * Actually just regenerate it optionally with the good or great flag set - NRM
 */
static void wiz_reroll_item(struct object *obj)
{
	struct object *new;

	char ch;

	bool changed = false;

	/* Hack -- leave artifacts alone */
	if (obj->artifact) return;

	/* Get new copy, hack off slays and brands */
	new = mem_zalloc(sizeof(*new));
	object_copy(new, obj);
	mem_free(new->slays);
	new->slays = NULL;
	mem_free(new->brands);
	new->brands = NULL;

	/* Main loop. Ask for magification and artifactification */
	while (true) {
		/* Display full item debug information */
		wiz_display_item(new, true);

		/* Ask wizard what to do. */
		if (!get_com("[a]ccept, [n]ormal, [g]ood, [e]xcellent? ", &ch))
			break;

		/* Create/change it! */
		if (ch == 'A' || ch == 'a') {
			break;
		} else if (ch == 'n' || ch == 'N') {
			/* Apply normal magic, but first clear object */
			changed = true;
			object_wipe(new);
			object_prep(new, obj->kind, player->depth, RANDOMISE);
			apply_magic(new, player->depth, false, false, false, false);
		} else if (ch == 'g' || ch == 'G') {
			/* Apply good magic, but first clear object */
			changed = true;
			object_wipe(new);
			object_prep(new, obj->kind, player->depth, RANDOMISE);
			apply_magic(new, player->depth, false, true, false, false);
		} else if (ch == 'e' || ch == 'E') {
			/* Apply great magic, but first clear object */
			changed = true;
			object_wipe(new);
			object_prep(new, obj->kind, player->depth, RANDOMISE);
			apply_magic(new, player->depth, false, true, true, false);
		}
	}

	/* Notice change */
	if (changed) {
		/* Record the old pile info */
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known_obj = obj->known;

		/* Free slays and brands on the old object by hand */
		mem_free(obj->slays);
		obj->slays = NULL;
		mem_free(obj->brands);
		obj->brands = NULL;
		mem_free(obj->curses);
		obj->curses = NULL;

		/* Copy over - slays and brands OK, pile info needs restoring */
		object_copy(obj, new);
		obj->prev = prev;
		obj->next = next;
		obj->known = known_obj;

		/* Mark as cheat */
		obj->origin = ORIGIN_CHEAT;

		/* Recalculate bonuses, gear */
		player->upkeep->update |= (PU_BONUS | PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );
	}

	/* Free the copy */
	object_wipe(new);
	mem_free(new);
}


/*
 * Maximum number of rolls
 */
#define TEST_ROLL 100000


/**
 * Try to create an item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now.  We acquire a clean grid, and then
 * repeatedly place an object in this grid, copying it into an item
 * holder, and then deleting the object.  We fiddle with the artifact
 * counter flags to prevent weirdness.  We use the items to collect
 * statistics on item creation relative to the initial item.
 */
static void wiz_statistics(struct object *obj, int level)
{
	long i, matches, better, worse, other;
	int j;

	char ch;
	const char *quality;

	bool good, great, ismatch, isbetter, isworse;

	struct object *test_obj;

	const char *q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";


	/* Allow multiple artifacts, because breaking the game is fine here */
	if (obj->artifact) obj->artifact->created = false;


	/* Interact */
	while (true) {
		const char *pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";

		/* Display item */
		wiz_display_item(obj, true);

		/* Get choices */
		if (!get_com(pmt, &ch)) break;

		if (ch == 'n' || ch == 'N') {
			good = false;
			great = false;
			quality = "normal";
		} else if (ch == 'g' || ch == 'G') {
			good = true;
			great = false;
			quality = "good";
		} else if (ch == 'e' || ch == 'E') {
			good = true;
			great = true;
			quality = "excellent";
		} else {
#if 0 /* unused */
			good = false;
			great = false;
#endif /* unused */
			break;
		}

		/* Let us know what we are doing */
		msg("Creating a lot of %s items. Base level = %d.", quality,
			player->depth);
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Set counters to zero */
		matches = better = worse = other = 0;

		/* Let's rock and roll */
		for (i = 0; i <= TEST_ROLL; i++) {
			/* Output every few rolls */
			if ((i < 100) || (i % 100 == 0)) {
				struct keypress kp;

				/* Do not wait */
				inkey_scan = SCAN_INSTANT;

				/* Allow interupt */
				kp = inkey();
				if (kp.type != EVT_NONE) {
					event_signal(EVENT_INPUT_FLUSH);
					break;
				}

				/* Dump the stats */
				prt(format(q, i, matches, better, worse, other), 0, 0);
				Term_fresh();
			}

			/* Create an object */
			test_obj = make_object(cave, level, good, great, false, NULL, 0);

			/* Allow multiple artifacts, because breaking the game is OK here */
			if (obj->artifact) obj->artifact->created = false;

			/* Test for the same tval and sval. */
			if ((obj->tval) != (test_obj->tval)) continue;
			if ((obj->sval) != (test_obj->sval)) continue;

			/* Check modifiers */
			ismatch = true;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (test_obj->modifiers[j] != obj->modifiers[j])
					ismatch = false;

			isbetter = true;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (test_obj->modifiers[j] < obj->modifiers[j])
					isbetter = false;

			isworse = true;
			for (j = 0; j < OBJ_MOD_MAX; j++)
				if (test_obj->modifiers[j] > obj->modifiers[j])
					isworse = false;

			/* Check for match */
			if (ismatch && (test_obj->to_a == obj->to_a) &&
				(test_obj->to_h == obj->to_h) &&
				(test_obj->to_d == obj->to_d))
				matches++;

			/* Check for better */
			else if (isbetter && (test_obj->to_a >= obj->to_a) &&
			         (test_obj->to_h >= obj->to_h) &&
			         (test_obj->to_d >= obj->to_d))
					better++;

			/* Check for worse */
			else if (isworse && (test_obj->to_a <= obj->to_a) &&
			         (test_obj->to_h <= obj->to_h) &&
			         (test_obj->to_d <= obj->to_d))
				worse++;

			/* Assume different */
			else
				other++;

			/* Nuke the test object */
			object_delete(&test_obj);
		}

		/* Final dump */
		msg(q, i, matches, better, worse, other);
		event_signal(EVENT_MESSAGE_FLUSH);
	}


	/* Hack -- Normally only make a single artifact */
	if (obj->artifact) obj->artifact->created = true;
}


/**
 * Change the quantity of an item
 */
static void wiz_quantity_item(struct object *obj, bool carried)
{
	char tmp_val[3];

	/* Never duplicate artifacts */
	if (obj->artifact) return;

	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", obj->number);

	/* Query */
	if (get_string("Quantity: ", tmp_val, 3)) {
		/* Extract */
		int tmp_int = atoi(tmp_val);

		/* Paranoia */
		if (tmp_int < 1) tmp_int = 1;
		if (tmp_int > 99) tmp_int = 99;

		/* Adjust total weight being carried */
		if (carried) {
			/* Remove the weight of the old number of objects */
			player->upkeep->total_weight -= (obj->number * obj->weight);

			/* Add the weight of the new number of objects */
			player->upkeep->total_weight += (tmp_int * obj->weight);
		}

		/* Adjust charges/timeouts for devices */
		if (tval_can_have_charges(obj))
			obj->pval = obj->pval * tmp_int / obj->number;

		if (tval_can_have_timeout(obj))
			obj->timeout = obj->timeout * tmp_int / obj->number;

		/* Accept modifications */
		obj->number = tmp_int;
	}
}


/**
 * Tweak the cursed status of an object.
 *
 * \param obj is the object to curse or decurse
 */
static void wiz_tweak_curse(struct object *obj)
{
	const char *p;
	char tmp_val[80];
	int val, pval;

	/* Get curse name */
	p = "Enter curse name or index: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (! get_string(p, tmp_val, sizeof(tmp_val))) return;

	/* Accept index or name */
	val = get_idx_from_name(tmp_val);
	if (! val) {
		val = lookup_curse(tmp_val);
	}
	if (val <= 0 || val >= z_info->curse_max) {
		return;
	}

	/* Get power */
	p = "Enter curse power (0 removes): ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (! get_string(p, tmp_val, 30)) return;
	pval = get_idx_from_name(tmp_val);
	if (pval < 0) {
		return;
	}

	/* Apply */
	if (pval) {
		append_object_curse(obj, val, pval);
	} else if (obj->curses) {
		obj->curses[val].power = 0;

		/* Duplicates logic from non-public check_object_curses(). */
		int i;

		for (i = 0; i < z_info->curse_max; i++) {
			if (obj->curses[i].power) {
				return;
			}
		}
		mem_free(obj->curses);
		obj->curses = NULL;
	}
}




/**
 * Play with an item. Options include:
 *   - Output statistics (via wiz_roll_item)
 *   - Reroll item (via wiz_reroll_item)
 *   - Change properties (via wiz_tweak_item)
 *   - Change the number of items (via wiz_quantity_item)
 */
static void do_cmd_wiz_play(void)
{
	struct object *obj;

	char ch;

	const char *q, *s;

	bool changed = false;
	bool all = true;


	/* Get an item */
	q = "Play with which object? ";
	s = "You have nothing to play with.";
	if (!get_item(&obj, q, s, 0, NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR))) return;

	/* Save screen */
	screen_save();

	/* The main loop */
	while (true) {
		/* Display the item */
		wiz_display_item(obj, all);

		/* Get choice */
		if (!get_com("[a]ccept [s]tatistics [r]eroll [t]weak [c]urse [q]uantity [k]nown? ", &ch))
			break;

		if (ch == 'A' || ch == 'a') {
			changed = true;
			break;
		} else if (ch == 'c' || ch == 'C')
			wiz_tweak_curse(obj);
		else if (ch == 's' || ch == 'S')
			wiz_statistics(obj, player->depth);
		else if (ch == 'r' || ch == 'R')
			wiz_reroll_item(obj);
		else if (ch == 't' || ch == 'T')
			wiz_tweak_item(obj);
		else if (ch == 'k' || ch == 'K')
			all = !all;
		else if (ch == 'q' || ch == 'Q') {
			bool carried = (object_is_carried(player, obj)) ? true : false;
			wiz_quantity_item(obj, carried);
		}
	}

	/* Load screen */
	screen_load();

	/* Accept change */
	if (changed) {
		/* Message */
		msg("Changes accepted.");

		/* Recalculate gear, bonuses */
		player->upkeep->update |= (PU_INVEN | PU_BONUS);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );
	} else
		msg("Changes ignored.");
}

/**
 * What happens when you cheat death.  Tsk, tsk.
 */
void wiz_cheat_death(void)
{
	/* Mark social class, reset age, if needed */
	player->age = 1;
	player->noscore |= NOSCORE_WIZARD;

	player->is_dead = false;

	/* Restore hit & spell points */
	player->chp = player->mhp;
	player->chp_frac = 0;
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Healing */
	(void)player_clear_timed(player, TMD_BLIND, true);
	(void)player_clear_timed(player, TMD_CONFUSED, true);
	(void)player_clear_timed(player, TMD_POISONED, true);
	(void)player_clear_timed(player, TMD_AFRAID, true);
	(void)player_clear_timed(player, TMD_PARALYZED, true);
	(void)player_clear_timed(player, TMD_IMAGE, true);
	(void)player_clear_timed(player, TMD_STUN, true);
	(void)player_clear_timed(player, TMD_CUT, true);

	/* Prevent starvation */
	player_set_timed(player, TMD_FOOD, PY_FOOD_MAX - 1, false);

	/* Cancel recall */
	if (player->word_recall)
	{
		/* Message */
		msg("A tension leaves the air around you...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->word_recall = 0;
	}

	/* Cancel deep descent */
	if (player->deep_descent)
	{
		/* Message */
		msg("The air around you stops swirling...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->deep_descent = 0;
	}

	/* Note cause of death XXX XXX XXX */
	my_strcpy(player->died_from, "Cheating death", sizeof(player->died_from));

	/* Back to the town */
	dungeon_change_level(player, 0);
}

/**
 * Cure everything instantly
 */
static void do_cmd_wiz_cure_all(void)
{
	int i;

	/* Remove curses */
	for (i = 0; i < player->body.count; i++) {
		if (player->body.slots[i].obj && player->body.slots[i].obj->curses) {
			mem_free(player->body.slots[i].obj->curses);
			player->body.slots[i].obj->curses = NULL;
		}
	}

	/* Restore stats */
	effect_simple(EF_RESTORE_STAT, source_player(), "0", STAT_STR, 0, 0, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, source_player(), "0", STAT_INT, 0, 0, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, source_player(), "0", STAT_WIS, 0, 0, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, source_player(), "0", STAT_DEX, 0, 0, 0, 0, NULL);
	effect_simple(EF_RESTORE_STAT, source_player(), "0", STAT_CON, 0, 0, 0, 0, NULL);

	/* Restore the level */
	effect_simple(EF_RESTORE_EXP, source_none(), "0", 0, 0, 0, 0, 0, NULL);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Cure stuff */
	(void)player_clear_timed(player, TMD_BLIND, true);
	(void)player_clear_timed(player, TMD_CONFUSED, true);
	(void)player_clear_timed(player, TMD_POISONED, true);
	(void)player_clear_timed(player, TMD_AFRAID, true);
	(void)player_clear_timed(player, TMD_PARALYZED, true);
	(void)player_clear_timed(player, TMD_IMAGE, true);
	(void)player_clear_timed(player, TMD_STUN, true);
	(void)player_clear_timed(player, TMD_CUT, true);
	(void)player_clear_timed(player, TMD_SLOW, true);
	(void)player_clear_timed(player, TMD_AMNESIA, true);

	/* No longer hungry */
	player_set_timed(player, TMD_FOOD, PY_FOOD_FULL - 1, false);

	/* Redraw everything */
	do_cmd_redraw();
	
	/* Give the player some feedback */
	msg("You feel *much* better!");
}


/**
 * Go to any level. optionally choosing level generation algorithm
 */
static void do_cmd_wiz_jump(void)
{
	int depth;

	char ppp[80];
	char tmp_val[160];

	/* Prompt */
	strnfmt(ppp, sizeof(ppp), "Jump to level (0-%d): ", z_info->max_depth-1);

	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%d", player->depth);

	/* Ask for a level */
	if (!get_string(ppp, tmp_val, 11)) return;

	/* Extract request */
	depth = atoi(tmp_val);

	/* Paranoia */
	if (depth < 0) depth = 0;

	/* Paranoia */
	if (depth > z_info->max_depth - 1)
		depth = z_info->max_depth - 1;

	/* Prompt */
	strnfmt(ppp, sizeof(ppp), "Choose cave_profile?");

	/* Get to choose generation algorithm */
	if (get_check(ppp))
		player->noscore |= NOSCORE_JUMPING;

	/* Accept request */
	msg("You jump to dungeon level %d.", depth);

	/* New depth */
	dungeon_change_level(player, depth);

	/* Hack - should be handled by redoing how debug commands work - NRM */
	cmdq_push(CMD_HOLD);
}


/**
 * Become aware of all object flavors
 */
static void do_cmd_wiz_learn(int lev)
{
	int i;

	/* Scan every object */
	for (i = 1; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		if (!kind || !kind->name) continue;

		/* Induce awareness */
		if (kind->level <= lev)
			kind->aware = true;
	}
	
	msg("You now know about many items!");
}


/**
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
	handle_stuff(player);

	/* Message */
	msg("Current Life Rating is %d/100.", percent);
}


/**
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
	int i;

	for (i = 0; i < num; i++)
		effect_simple(EF_SUMMON, source_player(), "1", 0, 0, 0, 0, 0, NULL);
}


/**
 * Summon a creature of the specified type
 */
static void do_cmd_wiz_named(struct monster_race *r, bool slp)
{
	int i;
	struct monster_group_info info = { 0, 0 };

	/* Paranoia */
	assert(r);

	/* Try 10 times */
	for (i = 0; i < 10; i++) {
		struct loc grid;
		int d = 1;

		/* Pick a location */
		scatter(cave, &grid, player->grid, d, true);

		/* Require empty grids */
		if (!square_isempty(cave, grid)) continue;

		/* Place it (allow groups) */
		if (place_new_monster(cave, grid, r, slp, true, info,
							  ORIGIN_DROP_WIZARD)) {
			break;
		}
	}
}



/**
 * Delete all nearby monsters
 */
static void do_cmd_wiz_zap(int d)
{
	int i;

	/* Banish everyone nearby */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Skip distant monsters */
		if (mon->cdis > d) continue;

		/* Delete the monster */
		delete_monster_idx(i);
	}

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;
}


/**
 * Query square flags - needs alteration if list-square-flags.h changes
 */
static void do_cmd_wiz_query(void)
{
	int y, x;
	char cmd;
	int flag = 0;


	/* Get a "debug command" */
	if (!get_com("Debug Command Query: ", &cmd)) return;

	/* Extract a flag */
	switch (cmd)
	{
		case 'g': flag = (SQUARE_GLOW); break;
		case 'r': flag = (SQUARE_ROOM); break;
		case 'a': flag = (SQUARE_VAULT); break;
		case 's': flag = (SQUARE_SEEN); break;
		case 'v': flag = (SQUARE_VIEW); break;
		case 'w': flag = (SQUARE_WASSEEN); break;
		case 'd': flag = (SQUARE_DTRAP); break;
		case 'f': flag = (SQUARE_FEEL); break;
		case 't': flag = (SQUARE_TRAP); break;
		case 'n': flag = (SQUARE_INVIS); break;
		case 'i': flag = (SQUARE_WALL_INNER); break;
		case 'o': flag = (SQUARE_WALL_OUTER); break;
		case 'l': flag = (SQUARE_WALL_SOLID); break;
		case 'x': flag = (SQUARE_MON_RESTRICT); break;
	}

	/* Scan map */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++) {
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++) {
			byte a = COLOUR_RED;
			struct loc grid = loc(x, y);

			if (!square_in_bounds_fully(cave, grid)) continue;

			/* Given flag, show only those grids */
			if (flag && !sqinfo_has(square(cave, grid).info, flag)) continue;

			/* Given no flag, show known grids */
			if (!flag && (!square_isknown(cave, grid))) continue;

			/* Color */
			if (square_ispassable(cave, grid)) a = COLOUR_YELLOW;

			/* Display player/floors/walls */
			if (loc_eq(grid, player->grid))
				print_rel(L'@', a, y, x);
			else if (square_ispassable(cave, grid))
				print_rel(L'*', a, y, x);
			else
				print_rel(L'#', a, y, x);
		}
	}

	Term_redraw();

	/* Get keypress */
	msg("Press any key.");
	inkey_ex();
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Redraw map */
	prt_map();
}

/**
 * Query terrain - needs alteration if terrain types change
 */
static void do_cmd_wiz_features(void)
{
	int y, x;

	char cmd;

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
	int feata[] = {FEAT_PASS_RUBBLE};
	int length = 0;


	/* Get a "debug command" */
	if (!get_com("Debug Command Feature Query: ", &cmd)) return;

	/* Choose a feature (type) */
	switch (cmd)
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
		case 'c': feat = featc; length = 1; break;
		/* Doors */
		case 'd': feat = featd; length = 4; break;
		/* Secret doors */
		case 'h': feat = feath; length = 1; break;
		/* Magma */
		case 'm': feat = featm; length = 2; break;
		/* Quartz */
		case 'q': feat = featq; length = 2; break;
		/* Granite */
		case 'g': feat = featg; length = 1; break;
		/* Permanent wall */
		case 'p': feat = featp; length = 1; break;
		/* Rubble */
		case 'r': feat = featr; length = 1; break;
		/* Passable rubble */
		case 'a': feat = feata; length = 1; break;
		/* Invalid entry */
		default: return;
	}

	/* Scan map */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++) {
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++) {
			struct loc grid = loc(x, y);
			byte a = COLOUR_RED;
			bool show = false;
			int i;

			if (!square_in_bounds_fully(cave, loc(x, y))) continue;

			/* Given feature, show only those grids */
			for (i = 0; i < length; i++)
				if (square(cave, grid).feat == feat[i]) show = true;

			/* Color */
			if (square_ispassable(cave, grid)) a = COLOUR_YELLOW;

			if (!show) continue;

			/* Display player/floors/walls */
			if (loc_eq(grid, player->grid))
				print_rel(L'@', a, y, x);
			else if (square_ispassable(cave, grid))
				print_rel(L'*', a, y, x);
			else
				print_rel(L'#', a, y, x);
		}
	}

	Term_redraw();

	/* Get keypress */
	msg("Press any key.");
	inkey_ex();
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Redraw map */
	prt_map();
}

/**
 * Create lots of items.
 */
static void wiz_test_kind(int tval)
{
	int sval;

	struct object *obj, *known_obj;

	for (sval = 0; sval < 255; sval++) {
		/* This spams failure messages, but that's the downside of wizardry */
		struct object_kind *kind = lookup_kind(tval, sval);
		if (!kind) continue;

		/* Create the item */
		if (tval == TV_GOLD)
			obj = make_gold(player->depth, kind->name);
		else {
			obj = object_new();
			object_prep(obj, kind, player->depth, RANDOMISE);

			/* Apply magic (no messages, no artifacts) */
			apply_magic(obj, player->depth, false, false, false, false);

			/* Mark as cheat, and where created */
			obj->origin = ORIGIN_CHEAT;
			obj->origin_depth = player->depth;
		}

		/* Make a known object */
		known_obj = object_new();
		obj->known = known_obj;

		/* Drop the object from heaven */
		drop_near(cave, &obj, 0, player->grid, true, true);
	}

	msg("Done.");
}

/**
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

/**
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
	player->upkeep->update |= (PU_BONUS | PU_HP | PU_SPELLS);

	/* Redraw everything */
	player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN |
			PR_EQUIP | PR_MESSAGE | PR_MONSTER | PR_OBJECT | PR_MONLIST |
			PR_ITEMLIST);

	/* Hack -- update */
	handle_stuff(player);

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
	int y = 0, x = 0;
	bool ident = false;

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

		/* Failed */
		if (index <= EF_NONE || index >= EF_MAX) {
			msg("No effect found.");
			return;
		}
	}

	/* Prompt */
	prt("Enter damage dice (eg 1+2d6M2): ", 0, 0);

	/* Get the dice */
	if (!askfor_aux(dice, sizeof(dice), NULL))
		my_strcpy(dice, "0", sizeof(dice));

	/* Get the parameters */
	prt("Enter name or number for effect subtype: ", 0, 0);

	/* Get the name */
	if (askfor_aux(name, sizeof(name), NULL)) {
		/* See if an effect parameter was entered */
		p1 = effect_subtype(index, name);
		if (p1 == -1) p1 = 0;
	}

	p2 = get_quantity("Enter second parameter (radius): ", 100);
	p3 = get_quantity("Enter third parameter (other):", 100);
	y = get_quantity("Enter y parameter:", 100);
	x = get_quantity("Enter x parameter:", 100);

	/* Reload the screen */
	screen_load();

	effect_simple(index, source_player(), dice, p1, p2, p3, y, x, &ident);

	if (ident)
		msg("Identified!");
}

/**
 * Main switch for processing debug commands.  This is a step back in time to
 * how all commands used to be processed
 */
void get_debug_command(void)
{
	char cmd;

	/* Get a "debug command" */
	if (!get_com("Debug Command: ", &cmd)) return;

	/* Analyze the command */
	switch (cmd)
	{
		/* Ignore */
		case ' ':
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
			wiz_create_item(false);
			break;
		}

		/* Create an artifact */
		case 'C':
		{
			wiz_create_item(true);
			break;
		}

		/* Detect everything */
		case 'd':
		{
			effect_simple(EF_DETECT_TRAPS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_DOORS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_STAIRS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_GOLD, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_OBJECTS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_VISIBLE_MONSTERS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			effect_simple(EF_DETECT_INVISIBLE_MONSTERS, source_player(), "0", 0, 0, 0, 22, 40, NULL);
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
			acquirement(player->grid, player->depth, n, false);
			break;
		}

		/* GF demo */
		case 'G':
		{
			wiz_proj_demo();
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
			effect_simple(EF_PROJECT_LOS, source_player(), "10000", PROJ_DISP_ALL, 0, 0, 0, 0, NULL);
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

		/* Work out what the player is typing */
		case 'L': 
		{
			do_cmd_keylog();
			break;
		}

		/* Magic Mapping */
		case 'm':
		{
			effect_simple(EF_MAP_AREA, source_player(), "0", 0, 0, 0, 22, 40, NULL);
			break;
		}

		/* Summon Named Monster */
		case 'n':
		{
			struct monster_race *r = NULL;
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
				do_cmd_wiz_named(r, true);
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
			effect_simple(EF_TELEPORT, source_player(), near, 0, 0, 0, 0, 0, NULL);
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
			const struct monster_race *race = NULL;

			char sym;
			const char *prompt =
				"Full recall for [a]ll monsters or [s]pecific monster? ";

			if (!get_com(prompt, &sym)) return;

			if (sym == 'a' || sym == 'A')
			{
				int i;
				for (i = 0; i < z_info->r_max; i++)
					cheat_monster_lore(&r_info[i], &l_list[i]);
				msg("Done.");
			}
			else if (sym == 's' || sym == 'S')
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
						race = &r_info[r_idx];
					else
						/* If not, find the monster with that name */
						race = lookup_monster(name); 
				}

				/* Reload the screen */
				screen_load();

				/* Did we find a valid monster? */
				if (race)
					cheat_monster_lore(race, get_lore(race));
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
			effect_simple(EF_TELEPORT, source_player(), far, 0, 0, 0, 0, 0, NULL);
			break;
		}

		/* Create a trap */
		case 'T':
		{
			if (!square_isfloor(cave, player->grid)) {
				msg("You can't place a trap there!");
				break;
			} else if (player->depth == 0) {
				msg("You can't place a trap in the town!");
				break;
			}

			char buf[40];
			if (!get_string("Create which trap? ", buf, sizeof(buf)))
				break;

			struct trap_kind *trap = lookup_trap(buf);
			if (trap) {
				place_trap(cave, player->grid, trap->tidx, 0);
			} else {
				msg("Trap not found.");
			}

			break;
		}

		/* Un-hide all monsters */
		case 'u':
		{
			effect_simple(EF_DETECT_VISIBLE_MONSTERS, source_player(), "0", 0, 0, 0, 500, 500, NULL);
			effect_simple(EF_DETECT_INVISIBLE_MONSTERS, source_player(), "0", 0, 0, 0, 500, 500, NULL);
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
			acquirement(player->grid, player->depth, n, true);
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
			wiz_light(cave, player, true);
			break;
		}

		/* Wipe recall for a monster */
		case 'W':
		{
			const struct monster_race *race = NULL;
			s16b r_idx = 0; 

			char sym;
			const char *prompt =
				"Wipe recall for [a]ll monsters or [s]pecific monster? ";

			if (!get_com(prompt, &sym)) return;

			if (sym == 'a' || sym == 'A') {
				int i;
				for (i = 0; i < z_info->r_max; i++)
					wipe_monster_lore(&r_info[i], &l_list[i]);
				msg("Done.");
			} else if (sym == 's' || sym == 'S') {
				char name[80] = "";

				/* Avoid the prompt getting in the way */
				screen_save();

				/* Prompt */
				prt("Which monster? ", 0, 0);

				/* Get the name */
				if (askfor_aux(name, sizeof(name), NULL)) {
					/* See if a r_idx was entered */
					r_idx = get_idx_from_name(name);
					if (r_idx)
						race = &r_info[r_idx];
					else
						/* If not, find the monster with that name */
						race = lookup_monster(name); 
				}
					
				/* Reload the screen */
				screen_load();

				/* Did we find a valid monster? */
				if (race)
					wipe_monster_lore(race, get_lore(race));
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

		/* Quit the game, don't save */
		case 'X':
		{
			if (get_check("Really quit without saving? "))
				quit("user choice");
			break;
		}

		/* Zap Monsters (Banishment) */
		case 'z':
		{
			int n;
			screen_save();
			n = get_quantity("Zap within what distance? ", z_info->max_sight);
			screen_load();
			do_cmd_wiz_zap(n);
			break;
		}

		/* Hack */
		case '_':
		{
			do_cmd_wiz_hack_nick();
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
