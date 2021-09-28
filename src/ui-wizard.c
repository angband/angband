/**
 * \file ui-wizard.c
 * \brief Implements menus and ui-game.c shims related to debug commands
 *
 * Copyright (c) 2021 Eric Branlund
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
#include "cmds.h"
#include "game-input.h"
#include "grafmode.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "project.h"
#include "ui-input.h"
#include "ui-menu.h"
#include "ui-prefs.h"
#include "ui-wizard.h"


static void proj_display(struct menu *m, int type, bool cursor,
		int row, int col, int wid)
{
	size_t i;

	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	const char *proj_name = proj_idx_to_name(type);

	if (type % 2) c_prt(attr, ".........................", row, col);
	c_put_str(attr, proj_name, row, col);

	col += 25;

	if (tile_height == 1) {
		for (i = 0; i < BOLT_MAX; i++) {
			if (use_graphics == GRAPHICS_NONE) {
				/* ASCII is simple */
				wchar_t chars[] = L"*|/-\\";

				col += big_pad(col, row,
					projections[type].color, chars[i]);
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


void wiz_proj_demo(void)
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


/** Object creation code **/
static bool choose_artifact = false;
static const region wiz_create_item_area = { 0, 0, 0, 0 };
#define WIZ_CREATE_ALL_MENU_ITEM -9999


/**
 * Build an "artifact name" and transfer it into a buffer.
 */
static void get_art_name(char *buf, int max, int a_idx)
{
	struct object *obj, *known_obj;
	struct object_kind *kind;
	const struct artifact *art = &a_info[a_idx];

	/* Get object */
	obj = object_new();

	/* Acquire the "kind" index */
	kind = lookup_kind(art->tval, art->sval);

	/* Oops */
	if (!kind) return;

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
	object_desc(buf, max, obj, ODESC_SINGULAR | ODESC_SPOIL, NULL);

	object_delete(NULL, NULL, &known_obj);
	obj->known = NULL;
	object_delete(NULL, NULL, &obj);
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
		/*
		 * Super big hack: the special flag should be the last menu
		 * item, with the selected tval stored in the next element.
		 */
		int current_tval = choices[oid + 1];
		char name[70];

		object_base_name(name, sizeof(name), current_tval, true);
		if (choose_artifact) {
			strnfmt(buf, sizeof(buf), "All artifact %s", name);
		} else {
			strnfmt(buf, sizeof(buf), "All %s", name);
		}
	} else {
		if (choose_artifact) {
			get_art_name(buf, sizeof(buf), selected);
		} else {
			object_kind_name(buf, sizeof(buf), &k_info[selected],
				true);
		}
	}

	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}


static bool wiz_create_item_subaction(struct menu *m, const ui_event *e,
	int oid)
{
	int *choices = menu_priv(m);
	int selected = choices[oid];

	if (e->type != EVT_SELECT) return true;

	if (selected == WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL);
		/* Same hack as in wiz_create_item_subdisplay() to get tval. */
		cmd_set_arg_number(cmdq_peek(), "tval", choices[oid + 1]);
		cmd_set_arg_choice(cmdq_peek(), "choice", 0);
	} else if (selected == WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ALL_ARTIFACT_FROM_TVAL);
		cmd_set_arg_number(cmdq_peek(), "tval", choices[oid + 1]);
	} else if (selected != WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_OBJ);
		cmd_set_arg_number(cmdq_peek(), "index", choices[oid]);
	} else if (selected != WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ARTIFACT);
		cmd_set_arg_number(cmdq_peek(), "index", choices[oid]);
	}

	return false;
}


static menu_iter wiz_create_item_submenu = {
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
		if (choose_artifact) {
			my_strcpy(buf, "All artifacts", sizeof(buf));
		} else {
			my_strcpy(buf, "All objects", sizeof(buf));
		}
	} else {
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

	if (e->type != EVT_SELECT) return true;

	if (oid == WIZ_CREATE_ALL_MENU_ITEM) {
		cmdq_push((choose_artifact) ? CMD_WIZ_CREATE_ALL_ARTIFACT :
			CMD_WIZ_CREATE_ALL_OBJ);
		return false;
	}

	/* Artifacts */
	if (choose_artifact) {
		/* ...We have to search the whole artifact list. */
		for (num = 0, i = 1; (num < 60) && (i < z_info->a_max); i++) {
			const struct artifact *art = &a_info[i];

			if (art->tval != oid) continue;

			choice[num++] = i;
		}
	} else {
		/* Regular objects */
		for (num = 0, i = 1; (num < 60) && (i < z_info->k_max); i++) {
			struct object_kind *kind = &k_info[i];

			if (kind->tval != oid ||
					kf_has(kind->kind_flags, KF_INSTA_ART))
				continue;

			choice[num++] = i;
		}
	}

	/*
	 * Add a flag for an "All <tval>" item to create all svals of that
	 * tval. The tval is stored (in a super hacky way) beyond the end of
	 * the valid menu items. The menu won't render it, but we can still
	 * get to it without doing a bunch of work.
	 */
	choice[num++] = WIZ_CREATE_ALL_MENU_ITEM;
	choice[num] = oid;

	screen_save();
	clear_from(0);

	menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_submenu);
	menu->selections = all_letters;

	object_base_name(buf, sizeof(buf), oid, true);
	if (choose_artifact) {
		strnfmt(title, sizeof(title), "Which artifact %s? ", buf);
	} else {
		strnfmt(title, sizeof(title), "What kind of %s?", buf);
	}
	menu->title = title;

	menu_setpriv(menu, num, choice);
	menu_layout(menu, &wiz_create_item_area);
	ret = menu_select(menu, 0, false);

	screen_load();
	mem_free(menu);

	return (ret.type == EVT_ESCAPE);
}


static const menu_iter wiz_create_item_menu = {
	NULL,
	NULL,
	wiz_create_item_display,
	wiz_create_item_action,
	NULL
};


/**
 * Choose and create an instance of an artifact or object kind
 */
void wiz_create_item(bool art)
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

		/*
		 * For artifact creation, only include tvals which have an
		 * artifact.
		 */
		if (art) {
			int j;
			for (j = 1; j < z_info->a_max; j++) {
				const struct artifact *art_local = &a_info[j];
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
 * Confirm before quitting without a save.
 */
void wiz_confirm_quit_no_save(void)
{
	if (get_check("Really quit without saving? ")) {
		cmdq_push(CMD_WIZ_QUIT_NO_SAVE);
	}
}


/**
 * Shim for ui-game.c to call wiz_create_item(true).
 */
void wiz_create_artifact(void)
{
	wiz_create_item(true);
}


/**
 * Shim for ui-game.c to call wiz_create_item(false).
 */
void wiz_create_nonartifact(void)
{
	wiz_create_item(false);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_ACQUIRE for good objects.
 */
void wiz_acquire_good(void)
{
	cmdq_push(CMD_WIZ_ACQUIRE);
	cmd_set_arg_choice(cmdq_peek(), "choice", 0);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_ACQUIRE for great objects.
 */
void wiz_acquire_great(void)
{
	cmdq_push(CMD_WIZ_ACQUIRE);
	cmd_set_arg_choice(cmdq_peek(), "choice", 1);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL with
 * instant artifacts included.
 */
void wiz_create_all_for_tval(void)
{
	cmdq_push(CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL);
	cmd_set_arg_choice(cmdq_peek(), "choice", 1);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_LEARN_OBJECT_KINDS for all levels.
 */
void wiz_learn_all_object_kinds(void)
{
	cmdq_push(CMD_WIZ_LEARN_OBJECT_KINDS);
	cmd_set_arg_number(cmdq_peek(), "level", 100);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_TELEPORT_RANDOM for short distances.
 */
void wiz_phase_door(void)
{
	cmdq_push(CMD_WIZ_TELEPORT_RANDOM);
	cmd_set_arg_number(cmdq_peek(), "range", 10);
}


/**
 * Shim for ui-game.c to set up CMD_WIZ_TELEPORT_RANDOM for long distances.
 */
void wiz_teleport(void)
{
	cmdq_push(CMD_WIZ_TELEPORT_RANDOM);
	cmd_set_arg_number(cmdq_peek(), "range", 100);
}
