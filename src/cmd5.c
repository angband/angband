/*
 * File: cmd5.c
 * Purpose: Spell and prayer casting/praying
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "object/tvalsval.h"
#include "game-cmd.h"

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
	const magic_type *s_ptr = &mp_ptr->info[spell];

	char help[30];
	char out[80];

	int attr;
	const char *name;
	const char *comment = NULL;

	if (s_ptr->slevel >= 99) {
		name = "(illegible)";
		attr = TERM_L_DARK;
	} else if (p_ptr->spell_flags[spell] & PY_SPELL_FORGOTTEN) {
		comment = " forgotten";
		attr = TERM_YELLOW;
	} else if (p_ptr->spell_flags[spell] & PY_SPELL_LEARNED) {
		if (p_ptr->spell_flags[spell] & PY_SPELL_WORKED) {
			/* Get extra info */
			get_spell_info(cp_ptr->spell_book, spell, help, sizeof(help));
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
			get_spell_name(cp_ptr->spell_book, spell),
			s_ptr->slevel, s_ptr->smana, spell_chance(spell), comment);
	c_prt(attr, out, row, col);
}

/**
 * Handle an event on a menu row.
 */
static bool spell_menu_handler(menu_type *m, const ui_event_data *e, int oid)
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

	screen_load();
	screen_save();

	Term_gotoxy(loc->col, loc->row + loc->page_rows);
	text_out("\n%s\n", s_text + s_info[(cp_ptr->spell_book == TV_MAGIC_BOOK) ? spell : spell + PY_MAX_SPELLS].text);

	/* XXX */
	text_out_pad = 0;
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

	screen_save();
	menu_select(m, 0);
	screen_load();

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

	screen_save();
	d->browse = TRUE;
	menu_select(m, 0);
	screen_load();

	screen_load();
}


/**
 * Interactively select a spell.
 *
 * Returns the spell selected, or -1.
 */
int get_spell(const object_type *o_ptr, const char *verb,
		bool (*spell_test)(int spell))
{
	menu_type *m;
	const char *noun = (cp_ptr->spell_book == TV_MAGIC_BOOK ?
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
 * Browse the given book.
 */
void textui_spell_browse(object_type *o_ptr, int item)
{
	menu_type *m;
	const char *noun = (cp_ptr->spell_book == TV_MAGIC_BOOK ?
			"spell" : "prayer");

	/* Track the object kind */
	track_object(item);
	handle_stuff();

	m = spell_menu_new(o_ptr, spell_okay_to_browse);
	if (m) {
		spell_menu_browse(m, noun);
		spell_menu_destroy(m);
	} else {
		msg_print("You cannot browse that.");
	}
}





/*** Game commands ***/

/**
 * Collect spells from a book into the spells[] array.
 */
int spell_collect_from_book(const object_type *o_ptr, int spells[PY_MAX_SPELLS])
{
	int i;
	int n_spells = 0;

	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		int spell = get_spell_index(o_ptr, i);

		if (spell != -1)
			spells[n_spells++] = spell;
	}

	return n_spells;
}


/**
 * True if at least one spell in spells[] is OK according to spell_test.
 */
bool spell_okay_list(bool (*spell_test)(int spell),
		const int spells[], int n_spells)
{
	int i;
	bool okay = FALSE;

	for (i = 0; i < n_spells; i++)
	{
		if (spell_test(spells[i]))
			okay = TRUE;
	}

	return okay;
}

/**
 * True if the spell is castable.
 */
bool spell_okay_to_cast(int spell)
{
	return (p_ptr->spell_flags[spell] & PY_SPELL_LEARNED);
}

/**
 * True if the spell can be studied.
 */
bool spell_okay_to_study(int spell)
{
	const magic_type *s_ptr = &mp_ptr->info[spell];
	return (s_ptr->slevel <= p_ptr->lev) &&
			!(p_ptr->spell_flags[spell] & PY_SPELL_LEARNED);
}

/**
 * True if the spell is browsable.
 */
bool spell_okay_to_browse(int spell)
{
	const magic_type *s_ptr = &mp_ptr->info[spell];
	return (s_ptr->slevel < 99);
}


/*
 * Returns chance of failure for a spell
 */
s16b spell_chance(int spell)
{
	int chance, minfail;

	const magic_type *s_ptr;


	/* Paranoia -- must be literate */
	if (!cp_ptr->spell_book) return (100);

	/* Get the spell */
	s_ptr = &mp_ptr->info[spell];

	/* Extract the base spell failure rate */
	chance = s_ptr->sfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (p_ptr->lev - s_ptr->slevel);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= adj_mag_stat[p_ptr->state.stat_ind[cp_ptr->spell_stat]];

	/* Not enough mana to cast */
	if (s_ptr->smana > p_ptr->csp)
	{
		chance += 5 * (s_ptr->smana - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->state.stat_ind[cp_ptr->spell_stat]];

	/* Non mage/priest characters never get better than 5 percent */
	if (!player_has(PF_ZERO_FAIL) && minfail < 5)
	{
		minfail = 5;
	}

	/* Priest prayer penalty for "edged" weapons (before minfail) */
	if (p_ptr->state.icky_wield)
	{
		chance += 25;
	}

	/* Fear makes spells harder (before minfail) */
	/* Note that spells that remove fear have a much lower fail rate than
	 * surrounding spells, to make sure this doesn't cause mega fail */
	if (p_ptr->state.afraid) chance += 20;

	/* Minimal and maximal failure rate */
	if (chance < minfail) chance = minfail;
	if (chance > 50) chance = 50;

	/* Stunning makes spells harder (after minfail) */
	if (p_ptr->timed[TMD_STUN] > 50) chance += 25;
	else if (p_ptr->timed[TMD_STUN]) chance += 15;

	/* Amnesia doubles failure change */
	if (p_ptr->timed[TMD_AMNESIA]) chance = 50 + chance / 2;

	/* Always a 5 percent chance of working */
	if (chance > 95) chance = 95;

	/* Return the chance */
	return (chance);
}



/* Check if the given spell is in the given book. */
static bool spell_in_book(int spell, int book)
{
	int i;
	object_type *o_ptr = object_from_item_idx(book);

	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		if (spell == get_spell_index(o_ptr, i))
			return TRUE;
	}

	return FALSE;
}

/* Gain a specific spell, specified by spell number (for mages). */
void do_cmd_study_spell(cmd_code code, cmd_arg args[])
{
	int spell = args[0].choice;

	int item_list[INVEN_TOTAL + MAX_FLOOR_STACK];
	int item_num;
	int i;

	/* Check the player can study at all atm */
	if (!player_can_study())
		return;

	/* Check that the player can actually learn the nominated spell. */
	item_tester_hook = obj_can_browse;
	item_num = scan_items(item_list, N_ELEMENTS(item_list), (USE_INVEN | USE_FLOOR));

	/* Check through all available books */
	for (i = 0; i < item_num; i++)
	{
		if (spell_in_book(spell, item_list[i]))
		{
			if (spell_okay_to_study(spell))
			{
				/* Spell is in an available book, and player is capable. */
				spell_learn(spell);
				p_ptr->energy_use = 100;
			}
			else
			{
				/* Spell is present, but player incapable. */
				msg_format("You cannot learn that spell.");
			}

			return;
		}
	}
}

/* Cast a spell from a book */
void do_cmd_cast(cmd_code code, cmd_arg args[])
{
	int spell = args[0].choice;
	int dir = args[1].direction;

	int item_list[INVEN_TOTAL + MAX_FLOOR_STACK];
	int item_num;
	int i;

	cptr verb = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "cast" : "recite");
	cptr noun = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Check the player can cast spells at all */
	if (!player_can_cast())
		return;

	/* Check spell is in a book they can access */
	item_tester_hook = obj_can_browse;
	item_num = scan_items(item_list, N_ELEMENTS(item_list), (USE_INVEN | USE_FLOOR));

	/* Check through all available books */
	for (i = 0; i < item_num; i++)
	{
		if (spell_in_book(spell, item_list[i]))
		{
			if (spell_okay_to_cast(spell))
			{
				/* Get the spell */
				const magic_type *s_ptr = &mp_ptr->info[spell];	
				
				/* Verify "dangerous" spells */
				if (s_ptr->smana > p_ptr->csp)
				{
					/* Warning */
					msg_format("You do not have enough mana to %s this %s.", verb, noun);
					
					/* Flush input */
					flush();
					
					/* Verify */
					if (!get_check("Attempt it anyway? ")) return;
				}

				/* Cast a spell */
				if (spell_cast(spell, dir))
					p_ptr->energy_use = 100;
			}
			else
			{
				/* Spell is present, but player incapable. */
				msg_format("You cannot %s that %s.", verb, noun);
			}

			return;
		}
	}

}


/* Gain a random spell from the given book (for priests) */
void do_cmd_study_book(cmd_code code, cmd_arg args[])
{
	int book = args[0].item;
	object_type *o_ptr = object_from_item_idx(book);

	int spell = -1;
	int i, k = 0;

	cptr p = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Check the player can study at all atm */
	if (!player_can_study())
		return;

	/* Check that the player has access to the nominated spell book. */
	if (!item_is_available(book, obj_can_browse, (USE_INVEN | USE_FLOOR)))
	{
		msg_format("That item is not within your reach.");
		return;
	}

	/* Extract spells */
	for (i = 0; i < SPELLS_PER_BOOK; i++)
	{
		int s = get_spell_index(o_ptr, i);
		
		/* Skip non-OK spells */
		if (s == -1) continue;
		if (!spell_okay_to_study(s)) continue;
		
		/* Apply the randomizer */
		if ((++k > 1) && (randint0(k) != 0)) continue;
		
		/* Track it */
		spell = s;
	}

	if (spell < 0)
	{
		msg_format("You cannot learn any %ss in that book.", p);
	}
	else
	{
		spell_learn(spell);
		p_ptr->energy_use = 100;	
	}
}


/*
 * Learn the specified spell.
 */
void spell_learn(int spell)
{
	int i;
	cptr p = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Learn the spell */
	p_ptr->spell_flags[spell] |= PY_SPELL_LEARNED;

	/* Find the next open entry in "spell_order[]" */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* Stop at the first empty space */
		if (p_ptr->spell_order[i] == 99) break;
	}

	/* Add the spell to the known list */
	p_ptr->spell_order[i] = spell;

	/* Mention the result */
	message_format(MSG_STUDY, 0, "You have learned the %s of %s.",
	           p, get_spell_name(cp_ptr->spell_book, spell));

	/* One less spell available */
	p_ptr->new_spells--;

	/* Message if needed */
	if (p_ptr->new_spells)
	{
		/* Message */
		msg_format("You can learn %d more %s%s.",
		           p_ptr->new_spells, p, PLURAL(p_ptr->new_spells));
	}

	/* Redraw Study Status */
	p_ptr->redraw |= (PR_STUDY | PR_OBJECT);
}


/* Cas the specified spell */
bool spell_cast(int spell, int dir)
{
	int chance;

	/* Get the spell */
	const magic_type *s_ptr = &mp_ptr->info[spell];	

	/* Spell failure chance */
	chance = spell_chance(spell);

	/* Failed spell */
	if (randint0(100) < chance)
	{
		if (OPT(flush_failure)) flush();
		msg_print("You failed to concentrate hard enough!");
	}

	/* Process spell */
	else
	{
		/* Cast the spell */
		if (!cast_spell(cp_ptr->spell_book, spell, dir)) return FALSE;

		/* A spell was cast */
		sound(MSG_SPELL);

		if (!(p_ptr->spell_flags[spell] & PY_SPELL_WORKED))
		{
			int e = s_ptr->sexp;

			/* The spell worked */
			p_ptr->spell_flags[spell] |= PY_SPELL_WORKED;

			/* Gain experience */
			gain_exp(e * s_ptr->slevel);

			/* Redraw object recall */
			p_ptr->redraw |= (PR_OBJECT);
		}
	}

	/* Sufficient mana */
	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)inc_timed(TMD_PARALYZED, randint1(5 * oops + 1), TRUE);

		/* Damage CON (possibly permanently) */
		if (randint0(100) < 50)
		{
			bool perm = (randint0(100) < 25);

			/* Message */
			msg_print("You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(A_CON, perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

	return TRUE;
}
