/*
 * File: ui-spell.c
 * Purpose: Spell UI handing
 *
 * Copyright (c) 2010 Andi Sidwell
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
#include "object/tvalsval.h"
#include "game-cmd.h"
#include "spells.h"

#include "ui.h"
#include "ui-menu.h"





/**
 * Spell menu data struct
 */
struct spell_menu_data {
	int spells[PY_MAX_SPELLS];
	int n_spells;

	bool browse;
	bool (*is_valid)(int spell);

	int selected_spell;
};


/**
 * Is item oid valid?
 */
static int spell_menu_valid(menu_type *m, int oid)
{
	struct spell_menu_data *d = menu_priv(m);
	int *spells = d->spells;

	return d->is_valid(spells[oid]);
}

/**
 * Display a row of the spell menu
 */
static void spell_menu_display(menu_type *m, int oid, bool cursor,
		int row, int col, int wid)
{
	struct spell_menu_data *d = menu_priv(m);
	int spell = d->spells[oid];
	const magic_type *s_ptr = &p_ptr->class->spells.info[spell];

	char help[30];
	char out[80];

	int attr;
	const char *illegible = NULL;
	const char *comment = NULL;

	if (s_ptr->slevel >= 99) {
		illegible = "(illegible)";
		attr = TERM_L_DARK;
	} else if (p_ptr->spell_flags[spell] & PY_SPELL_FORGOTTEN) {
		comment = " forgotten";
		attr = TERM_YELLOW;
	} else if (p_ptr->spell_flags[spell] & PY_SPELL_LEARNED) {
		if (p_ptr->spell_flags[spell] & PY_SPELL_WORKED) {
			/* Get extra info */
			get_spell_info(p_ptr->class->spell_book, spell, help, sizeof(help));
			comment = help;
			attr = TERM_WHITE;
		} else {
			comment = " untried";
			attr = TERM_L_GREEN;
		}
	} else if (s_ptr->slevel <= p_ptr->lev) {
		comment = " unknown";
		attr = TERM_L_BLUE;
	} else {
		comment = " difficult";
		attr = TERM_RED;
	}

	/* Dump the spell --(-- */
	strnfmt(out, sizeof(out), "%-30s%2d %4d %3d%%%s",
			get_spell_name(p_ptr->class->spell_book, spell),
			s_ptr->slevel, s_ptr->smana, spell_chance(spell), comment);
	c_prt(attr, illegible ? illegible : out, row, col);
}

/**
 * Handle an event on a menu row.
 */
static bool spell_menu_handler(menu_type *m, const ui_event *e, int oid)
{
	struct spell_menu_data *d = menu_priv(m);

	if (e->type == EVT_SELECT) {
		d->selected_spell = d->spells[oid];
		return d->browse ? TRUE : FALSE;
	}

	return TRUE;
}

/**
 * Show spell long description when browsing
 */
static void spell_menu_browser(int oid, void *data, const region *loc)
{
	struct spell_menu_data *d = data;
	int spell = d->spells[oid];

	/* Redirect output to the screen */
	text_out_hook = text_out_to_screen;
	text_out_wrap = 0;
	text_out_indent = loc->col - 1;
	text_out_pad = 1;

	Term_gotoxy(loc->col, loc->row + loc->page_rows);
	text_out("\n%s\n", s_info[(p_ptr->class->spell_book == TV_MAGIC_BOOK) ? spell : spell + PY_MAX_SPELLS].text);

	/* XXX */
	text_out_pad = 0;
	text_out_indent = 0;
}

static const menu_iter spell_menu_iter = {
	NULL,	/* get_tag = NULL, just use lowercase selections */
	spell_menu_valid,
	spell_menu_display,
	spell_menu_handler,
	NULL	/* no resize hook */
};

/** Create and initialise a spell menu, given an object and a validity hook */
static menu_type *spell_menu_new(const object_type *o_ptr,
		bool (*is_valid)(int spell))
{
	menu_type *m = menu_new(MN_SKIN_SCROLL, &spell_menu_iter);
	struct spell_menu_data *d = mem_alloc(sizeof *d);

	region loc = { -60, 1, 60, -99 };

	/* collect spells from object */
	d->n_spells = spell_collect_from_book(o_ptr, d->spells);
	if (d->n_spells == 0 || !spell_okay_list(is_valid, d->spells, d->n_spells))
	{
		mem_free(m);
		mem_free(d);
		return NULL;
	}

	/* copy across private data */
	d->is_valid = is_valid;
	d->selected_spell = -1;
	d->browse = FALSE;

	menu_setpriv(m, d->n_spells, d);

	/* set flags */
	m->header = "Name                             Lv Mana Fail Info";
	m->flags = MN_CASELESS_TAGS;
	m->selections = lower_case;
	m->browse_hook = spell_menu_browser;

	/* set size */
	loc.page_rows = d->n_spells + 1;
	menu_layout(m, &loc);

	return m;
}

/** Clean up a spell menu instance */
static void spell_menu_destroy(menu_type *m)
{
	struct spell_menu_data *d = menu_priv(m);
	mem_free(d);
	mem_free(m);
}

/**
 * Run the spell menu to select a spell.
 */
static int spell_menu_select(menu_type *m, const char *noun, const char *verb)
{
	struct spell_menu_data *d = menu_priv(m);

	screen_save();

	region_erase_bordered(&m->active);
	prt(format("%^s which %s? ", verb, noun), 0, 0);

	menu_select(m, 0, TRUE);

	screen_load();

	return d->selected_spell;
}

/**
 * Run the spell menu, without selections.
 */
static void spell_menu_browse(menu_type *m, const char *noun)
{
	struct spell_menu_data *d = menu_priv(m);

	screen_save();

	region_erase_bordered(&m->active);
	prt(format("Browsing %ss.  Press Escape to exit.", noun), 0, 0);

	d->browse = TRUE;
	menu_select(m, 0, TRUE);

	screen_load();
}


/**
 * Interactively select a spell.
 *
 * Returns the spell selected, or -1.
 */
static int get_spell(const object_type *o_ptr, const char *verb,
		bool (*spell_test)(int spell))
{
	menu_type *m;
	const char *noun = (p_ptr->class->spell_book == TV_MAGIC_BOOK ?
			"spell" : "prayer");

	m = spell_menu_new(o_ptr, spell_test);
	if (m) {
		int spell = spell_menu_select(m, noun, verb);
		spell_menu_destroy(m);
		return spell;
	}

	return -1;
}

/**
 * Browse a given book.
 */
void textui_book_browse(const object_type *o_ptr)
{
	menu_type *m;
	const char *noun = (p_ptr->class->spell_book == TV_MAGIC_BOOK ?
			"spell" : "prayer");

	m = spell_menu_new(o_ptr, spell_okay_to_browse);
	if (m) {
		spell_menu_browse(m, noun);
		spell_menu_destroy(m);
	} else {
		msg("You cannot browse that.");
	}
}

/**
 * Browse the given book.
 */
void textui_spell_browse(void)
{
	int item;

	item_tester_hook = obj_can_browse;
	if (!get_item(&item, "Browse which book? ",
			"You have no books that you can read.",
			CMD_BROWSE_SPELL, (USE_INVEN | USE_FLOOR | IS_HARMLESS)))
		return;

	/* Track the object kind */
	track_object(item);
	handle_stuff(p_ptr);

	textui_book_browse(object_from_item_idx(item));
}

/**
 * Study a book to gain a new spell
 */
void textui_obj_study(void)
{
	int item;

	item_tester_hook = obj_can_study;
	if (!get_item(&item, "Study which book? ",
			"You have no books that you can read.",
			CMD_STUDY_BOOK, (USE_INVEN | USE_FLOOR)))
		return;

	track_object(item);
	handle_stuff(p_ptr);

	if (player_has(PF_CHOOSE_SPELLS)) {
		int spell = get_spell(object_from_item_idx(item),
				"study", spell_okay_to_study);
		if (spell >= 0) {
			cmd_insert(CMD_STUDY_SPELL);
			cmd_set_arg_choice(cmd_get_top(), 0, spell);
		}
	} else {
		cmd_insert(CMD_STUDY_BOOK);
		cmd_set_arg_item(cmd_get_top(), 0, item);
	}
}

/**
 * Cast a spell from a book.
 */
void textui_obj_cast(void)
{
	int item;
	int spell;

	const char *verb = ((p_ptr->class->spell_book == TV_MAGIC_BOOK) ? "cast" : "recite");

	item_tester_hook = obj_can_cast_from;
	if (!get_item(&item, "Cast from which book? ",
			"You have no books that you can read.",
			CMD_CAST, (USE_INVEN | USE_FLOOR)))
		return;

	/* Track the object kind */
	track_object(item);

	/* Ask for a spell */
	spell = get_spell(object_from_item_idx(item), verb, spell_okay_to_cast);
	if (spell >= 0) {
		cmd_insert(CMD_CAST);
		cmd_set_arg_choice(cmd_get_top(), 0, spell);
	}
}
