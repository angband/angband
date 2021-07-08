/**
 * \file ui-equip-cmp.c
 * \brief Supply a "resistance grid for home" in the knowledge menu
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

#include "cave.h"
#include "game-input.h"
#include "init.h"
#include "object.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-tval.h"
#include "player.h"
#include "store.h"
#include "ui-entry.h"
#include "ui-entry-renderers.h"
#include "ui-equip-cmp.h"
#include "ui-event.h"
#include "ui-input.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-term.h"
#include "z-color.h"
#include "z-file.h"
#include "z-textblock.h"
#include "z-util.h"
#include "z-virt.h"

enum equipable_source {
	EQUIP_SOURCE_WORN,
	EQUIP_SOURCE_PACK,
	EQUIP_SOURCE_FLOOR,
	EQUIP_SOURCE_HOME,
	EQUIP_SOURCE_STORE
};

enum equipable_quality {
	EQUIP_QUAL_ARTIFACT,
	EQUIP_QUAL_EGO,
	EQUIP_QUAL_GOOD,
	EQUIP_QUAL_AVERAGE,
	EQUIP_QUAL_BAD
};

struct equipable {
	char *short_name;
	int *vals;
	int *auxvals;
	const struct object *obj;
	enum equipable_source src;
	enum equipable_quality qual;
	int slot;
	int nmlen;
	wchar_t ch;
	byte at;
};

union equipable_selfunc_extra {
	enum equipable_source src;
	enum equipable_quality qual;
	int slot;
	int propind;
};
typedef bool (*equipable_selfunc)(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
struct equipable_selector {
	equipable_selfunc func;
	union equipable_selfunc_extra ex;
};
enum equipable_expr_class {
	EQUIP_EXPR_SELECTOR,
	EQUIP_EXPR_AND,
	EQUIP_EXPR_OR,
	EQUIP_EXPR_TERMINATOR
};
struct equipable_expr {
	struct equipable_selector s;
	enum equipable_expr_class c;
};
struct equipable_filter {
	/*
	 * Has nv + 1 used elements with the last a sentinel (c ==
	 * EQUIP_EXPR_TERMINATOR).
	 */
	struct equipable_expr *v;
	/*
	 * Use EQUIP_EXPR_SELECTOR to indicate the simple evaluation is not
	 * possible.  Use EQUIP_EXPR_TERMINATOR when nv is zero:  there's no
	 * filtering.  The others indicate the way the terms are combined
	 * during simple evaluation.
	 */
	enum equipable_expr_class simple;
	int nv;
	int nalloc;
};

typedef int (*equipable_cmpfunc)(const struct equipable *left,
	const struct equipable *right, int propind);
struct equipable_arranger {
	equipable_cmpfunc func;
	int propind;
};
struct equipable_sorter {
	/* Has nv + 1 used elements with the last a sentinel (funct == 0). */
	struct equipable_arranger *v;
	int nv;
	int nalloc;
};

enum store_inclusion {
	EQUIPABLE_NO_STORE,
	EQUIPABLE_ONLY_STORE,
	EQUIPABLE_YES_STORE
};

struct prop_category {
	struct ui_entry **entries;
	wchar_t **labels;
	wchar_t *label_buffer;
	int n, off, nvw[3], ivw[3];
};

struct equipable_summary {
	/* Has space for nalloc items; nitems are currently used */
	struct equipable *items;
	/*
	 * Has space for nalloc + 1 items; nfilt + 1 are currently used with
	 * the last a sentinel (-1).
	 */
	int *sorted_indices;
	int *p_and_eq_vals;
	int *p_and_eq_auxvals;
	const char *dlg_trans_msg;
	struct prop_category propcats[5];
	struct equipable_filter easy_filt;
	/*
	 * Was intended to be an alternative to easy_filt where the player
	 * could configure a filter on more than one attribute.  Backend
	 * support is, in principle there, but there's currently in place to
	 * allow the player to set it up.  Leaving it here for now just in
	 * case.  config_filt was to be what the player specified compiled to
	 * what the backend wants.  config_mod_filt would be config_filt
	 * modified to handle the filtering of items from the stores as set
	 * by 'c'.
	 */
	struct equipable_filter config_filt;
	struct equipable_filter config_mod_filt;
	struct equipable_sorter default_sort;
	/*
	 * Was intended to be an alternative to the default sort that the
	 * player could configure.  While backend support should mostly be
	 * there, there's nothing that allows the player to do the
	 * configuration.  Leaving it here for now, just in case.
	 */
	struct equipable_sorter config_sort;
	enum store_inclusion stores;
	/* Is the index, in sorted_indices, for first shown on page. */
	int ifirst;
	int indinc;
	int iview;
	/* Are indices into items for selection. */
	int isel0, isel1;
	/* Is the index, into sorted_indices, for choosing selection. */
	int work_sel;
	/* Is the number shown on current page. */
	int npage;
	int nfilt;
	int nitems;
	int nalloc;
	int nprop;
	/* Is the maximum number that can be shown on any page. */
	int maxpage;
	/* Is the maximum number of characters for a short object name. */
	int nshortnm;
	/* Is the number of character for propery labels. */
	int nproplab;
	/*
	 * Is the number of views to use for displaying the properties, either
	 * two or three.
	 */
	int nview;
	/*
	 * Is the row used for display of the combined player and current
	 * equipment properties.
	 */
	int irow_combined_equip;
	/* Is the column where the object names start. */
	int icol_name;
	/*
	 * These two are the terminal dimensions used when configuring the
	 * layout.
	 */
	int term_ncol, term_nrow;
	bool config_filt_is_on;
	bool config_sort_is_on;
};

struct indirect_sort_data {
	const struct equipable *items;
	const struct equipable_arranger *arr;
};


/* These have to match up with the state array in equip_cmp_display(). */
enum {
	EQUIP_CMP_MENU_DONE,
	EQUIP_CMP_MENU_BAIL,
	EQUIP_CMP_MENU_NEW_PAGE,
	EQUIP_CMP_MENU_SAME_PAGE,
	EQUIP_CMP_MENU_SEL0,
	EQUIP_CMP_MENU_SEL1,
};
struct menu_display_state {
	const char *prompt;
	int (*keyfunc)(struct keypress ch, int istate,
		struct equipable_summary *s, struct player *p);
	bool clear;
	bool refresh;
};


static int initialize_summary(struct player *p,
	struct equipable_summary **s);
static void cleanup_summary(struct equipable_summary *s);
static void cleanup_summary_items(struct equipable_summary *s);
static int display_page(struct equipable_summary *s, const struct player *p,
	bool allow_reconfig);
static void display_equip_cmp_help(void);
static void display_equip_cmp_sel_help(void);
static int handle_key_bail(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p);
static int handle_key_equip_cmp_general(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p);
static int handle_key_equip_cmp_select(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p);
static int prompt_for_easy_filter(struct equipable_summary *s, bool apply_not);
static void display_object_comparison(const struct equipable_summary *s);
static bool dump_to_file(const char *path);
static void append_to_file(ang_file *fff);
static void filter_items(struct equipable_summary *s);
/*
 * Not used at the moment; left here in case more configurable filtering
 * is implemented.
 */
#if 0
static bool sel_better_than(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
#endif
static bool sel_at_least_resists(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_does_not_resist(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_has_flag(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_does_not_have_flag(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_has_pos_mod(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_has_nonpos_mod(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
/*
 * Not used at the moment; left here in case more configurable filtering
 * is implemented.
 */
#if 0
static bool sel_exclude_slot(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_only_slot(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
#endif
static bool sel_exclude_src(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static bool sel_only_src(const struct equipable *eq,
	const union equipable_selfunc_extra *ex);
static void sort_items(struct equipable_summary *s);
static wchar_t source_to_char(enum equipable_source src);


static struct equipable_summary *the_summary = NULL;
/* Used by append_to_file() (and therefore for dump_to_file()) */
static struct equipable_summary *s_for_file = NULL;
static struct indirect_sort_data sort_dat;



void equip_cmp_display(void)
{
	const struct menu_display_state states[] = {
		/* EQUIP_CMP_MENU_DONE */
		{ "", 0, false, false },
		/* EQUIP_CMP_MENU_BAIL */
		{ "Sorry, could not display.  Press any key.",
			handle_key_bail, true, false },
		/* EQUIP_CMP_MENU_NEW_PAGE */
		{ "[Up/Down arrow, p/PgUp, n/PgDn to move; ? for help; ESC to exit]", handle_key_equip_cmp_general, true, true },
		/* EQUIP_CMP_MENU_SAME_PAGE */
		{ "[Up/Down arrow, p/PgUp, n/PgDn to move; ? for help; ESC to exit]", handle_key_equip_cmp_general, false, false },
		/* EQUIP_CMP_MENU_SEL0 */
		{ "[Up/Down arrow, p/PgUp, n/PgDn to move; return to accept]", handle_key_equip_cmp_select, true, true },
		/* EQUIP_CMP_MENU_SEL1 */
		{ "[Up/Down arrow, p/PgUp, n/PgDn to move; return to accept]", handle_key_equip_cmp_select, true, true },
	};
	int istate;

	screen_save();

	if (initialize_summary(player, &the_summary)) {
		istate = EQUIP_CMP_MENU_BAIL;
	} else {
		istate = EQUIP_CMP_MENU_NEW_PAGE;
	}

	while (istate != EQUIP_CMP_MENU_DONE) {
		struct keypress ch;
		int wid, hgt;

		assert(istate >= 0 && istate < (int)N_ELEMENTS(states));
		if (states[istate].clear) {
			Term_clear();
		}
		if (states[istate].refresh) {
			if (display_page(the_summary, player, true)) {
				istate = EQUIP_CMP_MENU_BAIL;
			}
		}

		/*
		 * Use row 0 for state transition or other informational
		 * messages
		 */
		if (the_summary->dlg_trans_msg != NULL &&
			the_summary->dlg_trans_msg[0] != '\0') {
			prt(the_summary->dlg_trans_msg, 0, 0);
			the_summary->dlg_trans_msg = NULL;
		} else {
			if (the_summary->nfilt == 0) {
				prt("No items; use q, !, c, or R to change filter", 0, 0);
			} else if (! states[istate].clear) {
				prt("", 0, 0);
			}
		}

		/* Use last row for prompt. */
		Term_get_size(&wid, &hgt);
		prt(states[istate].prompt, hgt - 1, 0);

		ch = inkey();
		istate = (*states[istate].keyfunc)(ch, istate, the_summary,
			player);
	}

	screen_load();
}


static void display_equip_cmp_help(void)
{
	int irow, wid, hgt;

	Term_clear();
	irow = 1;
	prt("Movement/scrolling ---------------------------------", irow, 0);
	++irow;
	prt("Down arrow  one line down    Up arrow    one line up", irow, 0);
	++irow;
	prt("n, PgDn     one page down    p, PgUp     one page up", irow, 0);
	++irow;
	prt("space       one page down", irow, 0);
	++irow;
	prt("Filtering/searching/sorting ------------------------", irow, 0);
	++irow;
	prt("q           quick filter     !           use opposite quick", irow, 0);
	++irow;
	prt("c           cycle inclusion of stores' goods", irow, 0);
	++irow;
	prt("r           reverse", irow, 0);
	++irow;
	prt("Information ----------------------------------------", irow, 0);
	++irow;
	prt("v           cycle through attribute views", irow, 0);
	++irow;
	prt("I, x        select one or two items for details", irow, 0);
	++irow;
	prt("Other ----------------------------------------------", irow, 0);
	++irow;
	prt("d           dump to file     R           reset display", irow, 0);
	++irow;
	prt("ESC         exit", irow, 0);
	++irow;

	Term_get_size(&wid, &hgt);
	prt("Press any key to continue", hgt - 1, 0);
	(void) inkey();
}


static int handle_key_bail(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p)
{
	return EQUIP_CMP_MENU_DONE;
}


static int handle_key_equip_cmp_general(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p)
{
	static const char *trans_msg_unknown_key =
		"Unknown key pressed; ? will list available keys";
	static const char *trans_msg_view =
		"Showing alternate attributes; press v to cycle";
	static const char *trans_msg_onlystore =
		"Only showing goods from stores; press c to change";
	static const char *trans_msg_withstore =
		"Showing possesions and goods from stores; press c to change";
	static const char *trans_msg_save_ok = "Successfully saved to file";
	static const char *trans_msg_save_bad = "Failed to save to file!";
	static const char *trans_msg_sel0 = "Select first item to examine";
	int result;
	int ilast;

	switch (ch.code) {
	case 'n':
	case ' ':
	case KC_PGDOWN:
		if (s->npage == s->maxpage) {
			ilast = s->ifirst + 2 * s->indinc * s->npage;
			if (ilast > s->nfilt) {
				assert(s->indinc > 0);
				s->ifirst = s->nfilt + 1 - s->npage;
				--s->npage;
			} else if (ilast < 0) {
				assert(s->indinc < 0);
				s->ifirst = s->npage - 2;
				--s->npage;
			} else {
				s->ifirst += s->indinc * s->npage;
			}
			result = EQUIP_CMP_MENU_NEW_PAGE;
		} else {
			/* Page already includes the end, so don't move. */
			result = EQUIP_CMP_MENU_SAME_PAGE;
		}
		break;

	case 'p':
	case KC_PGUP:
		if (s->indinc > 0) {
			if (s->ifirst > 0) {
				s->ifirst = (s->ifirst < s->maxpage) ?
					0 : s->ifirst - s->maxpage;
				s->npage = (s->ifirst + s->maxpage <= s->nfilt) ?
					s->maxpage : s->nfilt - s->ifirst;
				result = EQUIP_CMP_MENU_NEW_PAGE;
			} else {
				result = EQUIP_CMP_MENU_SAME_PAGE;
			}
		} else {
			if (s->ifirst < s->nfilt - 1) {
				s->ifirst = (s->ifirst >= s->nfilt - s->maxpage) ?
					s->nfilt - 1 : s->ifirst + s->maxpage;
				s->npage = (s->ifirst - s->maxpage >= -1) ?
					s->maxpage : s->ifirst + 1;
				result = EQUIP_CMP_MENU_NEW_PAGE;
			} else {
				result = EQUIP_CMP_MENU_SAME_PAGE;
			}
		}
		break;

	case ARROW_DOWN:
		if (s->npage == s->maxpage) {
			s->ifirst += s->indinc;
			ilast = s->ifirst + s->npage * s->indinc;
			if (ilast < -1 || ilast > s->nfilt) {
				--s->npage;
			}
			result = EQUIP_CMP_MENU_NEW_PAGE;
		} else {
			/* Page already includes the end, so don't move. */
			result = EQUIP_CMP_MENU_SAME_PAGE;
		}
		break;

	case ARROW_UP:
		if (s->indinc > 0) {
			if (s->ifirst > 0) {
				--s->ifirst;
				if (s->npage < s->maxpage &&
					s->ifirst + s->npage < s->nfilt) {
					++s->npage;
				}
				result = EQUIP_CMP_MENU_NEW_PAGE;
			} else {
				result = EQUIP_CMP_MENU_SAME_PAGE;
			}
		} else {
			if (s->ifirst < s->nfilt - 1) {
				++s->ifirst;
				if (s->npage < s->maxpage &&
					s->ifirst - s->npage > -1) {
					++s->npage;
				}
				result = EQUIP_CMP_MENU_NEW_PAGE;
			} else {
				result = EQUIP_CMP_MENU_SAME_PAGE;
			}
		}
		break;

	case 'c':
		/*
		 * Cycle through no goods from stores(default), only goods
		 * from stores, and both possesoions and gopds from stores.
		 */
		switch (s->stores) {
		case EQUIPABLE_NO_STORE:
			assert(s->easy_filt.simple == EQUIP_EXPR_AND &&
				s->easy_filt.nv >= 1 &&
				s->easy_filt.v[0].s.func == sel_exclude_src &&
				s->easy_filt.v[0].s.ex.src == EQUIP_SOURCE_STORE);
			s->easy_filt.v[0].s.func = sel_only_src;
			s->stores = EQUIPABLE_ONLY_STORE;
			s->dlg_trans_msg = trans_msg_onlystore;
			break;

		case EQUIPABLE_ONLY_STORE:
			assert(s->easy_filt.simple == EQUIP_EXPR_AND &&
				s->easy_filt.nv >= 1 &&
				s->easy_filt.v[0].s.func == sel_only_src &&
				s->easy_filt.v[0].s.ex.src == EQUIP_SOURCE_STORE);
			s->easy_filt.v[0] = s->easy_filt.v[1];
			if (s->easy_filt.nv > 1) {
				s->easy_filt.v[1] = s->easy_filt.v[2];
			}
			--s->easy_filt.nv;
			s->stores = EQUIPABLE_YES_STORE;
			s->dlg_trans_msg = trans_msg_withstore;
			break;

		case EQUIPABLE_YES_STORE:
			assert(s->easy_filt.nv == 0 || s->easy_filt.nv == 1);
			s->easy_filt.v[2] = s->easy_filt.v[1];
			s->easy_filt.v[1] = s->easy_filt.v[0];
			s->easy_filt.v[0].s.func = sel_exclude_src;
			s->easy_filt.v[0].s.ex.src = EQUIP_SOURCE_STORE;
			s->easy_filt.v[0].c = EQUIP_EXPR_SELECTOR;
			++s->easy_filt.nv;
			s->stores = EQUIPABLE_NO_STORE;
			break;
		}
		filter_items(s);
		sort_items(s);
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case 'd':
		/* Dump to a file. */
		{
			char buf[1024];
			char fname[80];

			player_safe_name(fname, sizeof(fname),
				p->full_name, false);
			my_strcat(fname, "_equip.txt", sizeof(fname));
			if (get_file(fname, buf, sizeof(buf))) {
				s_for_file = s;
				if (dump_to_file(buf)) {
					s->dlg_trans_msg = trans_msg_save_ok;
				} else {
					s->dlg_trans_msg = trans_msg_save_bad;
				}
			}
		}
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case 'q':
		/* Choose a quick filter - one based on a single attribute. */
		result = prompt_for_easy_filter(s, false);
		break;

	case 'r':
		/* Reverse the order of display; keep in the same page. */
		if (s->npage > 0) {
			s->ifirst += s->npage * s->indinc - 1;
		}
		s->indinc *= -1;
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case 'v':
		/* Cycle through which attributes are shown. */
		++s->iview;
		if (s->iview >= s->nview) {
			s->iview = 0;
		} else {
			s->dlg_trans_msg = trans_msg_view;
		}
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case 'x':
	case 'I':
		/* Select an item or two to examine. */
		s->work_sel = s->ifirst;
		s->dlg_trans_msg = trans_msg_sel0;
		result = EQUIP_CMP_MENU_SEL0;
		break;

	case 'R':
		/* Reset view to defaults. */
		s->indinc = 1;
		s->iview = 0;
		s->config_filt_is_on = false;
		s->stores = EQUIPABLE_NO_STORE;
		s->easy_filt.simple = EQUIP_EXPR_AND;
		s->easy_filt.nv = 1;
		s->easy_filt.v[0].s.func = sel_exclude_src;
		s->easy_filt.v[0].s.ex.src = EQUIP_SOURCE_STORE;
		s->easy_filt.v[0].c = EQUIP_EXPR_SELECTOR;
		s->easy_filt.v[1].c = EQUIP_EXPR_TERMINATOR;
		filter_items(s);
		s->config_sort_is_on = false;
		sort_items(s);
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case '!':
		/*
		 * If using a quick filter, use not for the criteria.
		 * Otherwise, set up a quick filter to do that.
		 */
		if (! s->config_filt_is_on && (s->easy_filt.nv == 2 ||
			(s->easy_filt.nv == 1 &&
			s->stores == EQUIPABLE_YES_STORE))) {
			int ind = (s->easy_filt.nv == 2) ? 1 : 0;

			assert(s->easy_filt.v[ind].c == EQUIP_EXPR_SELECTOR);
			if (s->easy_filt.v[ind].s.func ==
				sel_at_least_resists) {
				s->easy_filt.v[ind].s.func =
					sel_does_not_resist;
			} else if (s->easy_filt.v[ind].s.func ==
				sel_has_flag) {
				s->easy_filt.v[ind].s.func =
					sel_does_not_have_flag;
			} else if (s->easy_filt.v[ind].s.func ==
				sel_has_pos_mod) {
				s->easy_filt.v[ind].s.func =
					sel_has_nonpos_mod;
			} else if (s->easy_filt.v[ind].s.func ==
				sel_does_not_resist) {
				s->easy_filt.v[ind].s.func =
					sel_at_least_resists;
			} else if (s->easy_filt.v[ind].s.func ==
				sel_does_not_have_flag) {
				s->easy_filt.v[ind].s.func = sel_has_flag;
			} else if (s->easy_filt.v[ind].s.func ==
				sel_has_nonpos_mod) {
				s->easy_filt.v[ind].s.func = sel_has_pos_mod;
			} else {
				assert(0);
			}
			filter_items(s);
			sort_items(s);
			result = EQUIP_CMP_MENU_NEW_PAGE;
		} else {
			result = prompt_for_easy_filter(s, true);
		}
		break;

	case '?':
		display_equip_cmp_help();
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case ESCAPE:
		result = EQUIP_CMP_MENU_DONE;
		break;

	default:
		s->dlg_trans_msg = trans_msg_unknown_key;
		result = EQUIP_CMP_MENU_SAME_PAGE;
		break;
	}

	return result;
}


static void display_equip_cmp_sel_help(void)
{
	int irow, wid, hgt;

	Term_clear();
	irow = 1;
	prt("Down arrow  move selection one line down", irow, 0);
	++irow;
	prt("Up arrow    move selection one line up", irow, 0);
	++irow;
	prt("n, PgDn     move selection one page up", irow, 0);
	++irow;
	prt("p, PgUp     move selection one page up", irow, 0);
	++irow;
	prt("x           stop selection; if first item, escapes", irow, 0);
	++irow;
	prt("return      select current item", irow, 0);
	++irow;
	prt("ESC         leave selection process", irow, 0);
	++irow;

	Term_get_size(&wid, &hgt);
	prt("Press any key to continue", hgt - 1, 0);
	(void) inkey();
}


static int handle_key_equip_cmp_select(struct keypress ch, int istate,
	struct equipable_summary *s, struct player *p)
{
	static const char *trans_msg_unknown_key =
		"Unknown key pressed; ? will list available keys";
	static const char *trans_msg_sel1 = "Select second item; x to skip";
	int result;
	int ilast;

	switch (ch.code) {
	case 'n':
	case KC_PGDOWN:
		/* Move selection by one page. */
		s->work_sel += s->indinc * s->npage;
		if (s->work_sel < 0) {
			s->work_sel = 0;
		} else if (s->work_sel >= s->nfilt) {
			s->work_sel = s->nfilt - 1;
		}
		/* Shift view. */
		s->ifirst += s->indinc * s->npage;
		if (s->indinc > 0) {
			if (s->ifirst >= s->nfilt + 1 - s->maxpage) {
				s->ifirst = s->nfilt + 1 - s->maxpage;
				if (s->ifirst < 0) {
					s->ifirst = 0;
					s->npage = s->nfilt;
				} else {
					s->npage = s->maxpage - 1;
				}
			}
		} else {
			if (s->ifirst < s->maxpage - 2) {
				s->ifirst = s->maxpage - 2;
				if (s->ifirst >= s->nfilt) {
					s->ifirst = s->nfilt - 1;
					s->npage = s->nfilt;
				} else {
					s->npage = s->maxpage - 1;
				}
				--s->npage;
			}
		}
		result = istate;
		break;

	case 'p':
	case KC_PGUP:
		/* Move selection by one page. */
		s->work_sel -= s->indinc * s->npage;
		if (s->work_sel < 0) {
			s->work_sel = 0;
		} else if (s->work_sel >= s->nfilt) {
			s->work_sel = s->nfilt - 1;
		}
		/* Shift view. */
		s->ifirst -= s->indinc * s->npage;
		if (s->ifirst < 0) {
			s->ifirst = 0;
		} else if (s->ifirst >= s->nfilt) {
			s->ifirst = s->nfilt - 1;
		}
		ilast = s->ifirst + s->indinc * s->maxpage;
		if (s->indinc > 0) {
			s->npage = (ilast <= s->nfilt) ?
				s->maxpage : s->nfilt - s->ifirst;
		} else {
			s->npage = (ilast >= -1) ?
				s->maxpage : s->ifirst + 1;
		}
		result = istate;
		break;

	case ARROW_DOWN:
		/* Move selection by one line. */
		s->work_sel += s->indinc;
		if ((s->work_sel - s->ifirst) * s->indinc >= s->npage) {
			if (s->work_sel < 0) {
				s->work_sel = 0;
			} else if (s->work_sel >= s->nfilt) {
				s->work_sel = s->nfilt - 1;
			} else {
				/* Shift view. */
				s->ifirst = s->work_sel - s->indinc;
				if (s->indinc > 0) {
					if (s->ifirst + s->npage > s->nfilt) {
						s->ifirst = s->nfilt + 1 -
							s->npage;
						--s->npage;
					}
				} else {
					if (s->ifirst - s->npage < -1) {
						s->ifirst = s->npage - 2;
						--s->npage;
					}
				}
			}
		}
		result = istate;
		break;

	case ARROW_UP:
		/* Move selection by one line. */
		s->work_sel -= s->indinc;
		if ((s->work_sel - s->ifirst) * s->indinc < 0) {
			if (s->work_sel < 0) {
				s->work_sel = 0;
			} else if (s->work_sel >= s->nfilt) {
				s->work_sel = s->nfilt - 1;
			} else {
				/* Shift view. */
				s->ifirst = s->work_sel + s->indinc *
					(2 - s->npage);
				if (s->indinc > 0) {
					if (s->ifirst < 0) {
						s->ifirst = 0;
					}
					ilast = s->ifirst + s->maxpage;
					if (ilast <= s->nfilt) {
						s->npage = s->maxpage;
					} else {
						s->npage =
							s->nfilt - s->ifirst;
					}
				} else {
					if (s->ifirst >= s->nfilt) {
						s->ifirst = s->nfilt - 1;
					}
					ilast = s->ifirst - s->maxpage;
					if (ilast >= -1) {
						s->npage = s->maxpage;
					} else {
						s->npage = s->ifirst + 1;
					}
				}
			}
		}
		result = istate;
		break;

	case 'x':
		/* Skip the selection. For the first, acts like ESC. */
		if (istate == EQUIP_CMP_MENU_SEL1) {
			display_object_comparison(s);
		}
		s->isel0 = -1;
		s->isel1 = -1;
		s->work_sel = -1;
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	case KC_ENTER:
		assert(s->work_sel >= 0 && s->work_sel < s->nfilt);
		if (istate == EQUIP_CMP_MENU_SEL0) {
			s->isel0 = s->sorted_indices[s->work_sel];
			s->dlg_trans_msg = trans_msg_sel1;
			result = EQUIP_CMP_MENU_SEL1;
		} else {
			s->isel1 = s->sorted_indices[s->work_sel];
			display_object_comparison(s);
			s->isel0 = -1;
			s->isel1 = -1;
			s->work_sel = -1;
			result = EQUIP_CMP_MENU_NEW_PAGE;
		}
		break;

	case '?':
		display_equip_cmp_sel_help();
		result = istate;
		break;

	case ESCAPE:
		s->isel0 = -1;
		s->isel1 = -1;
		s->work_sel = -1;
		result = EQUIP_CMP_MENU_NEW_PAGE;
		break;

	default:
		s->dlg_trans_msg = trans_msg_unknown_key;
		result = istate;
		break;
	}

	return result;
}


static int prompt_for_easy_filter(struct equipable_summary *s, bool apply_not)
{
	static const char *no_matching_attribute_msg =
		"Did not find attribute with that name; filter unchanged";
	char c[4] = "";
	int itry;
	bool threec;

	if (! get_string("Enter 2 or 3 (for stat) character code and return or return to clear ", c,
		N_ELEMENTS(c))) {
		return EQUIP_CMP_MENU_NEW_PAGE;
	}

	/* Clear the current filter. */
	if (c[0] == '\0') {
		s->config_filt_is_on = false;
		if (s->easy_filt.nv == 2 ||
			(s->easy_filt.nv == 1 &&
			s->stores == EQUIPABLE_YES_STORE)) {
			s->easy_filt.v[s->easy_filt.nv - 1] =
				s->easy_filt.v[s->easy_filt.nv];
			--s->easy_filt.nv;
			filter_items(s);
			sort_items(s);
		}
		return EQUIP_CMP_MENU_NEW_PAGE;
	}

	/*
	 * Try different combinations of capitalization to match the
	 * entered string to one of the column labels.
	 */
	itry = 0;
	threec = false;
	while (1) {
		char ctry[4];
		wchar_t wc[4];

		if (itry >= 4 || (threec && itry >= 3)) {
			s->dlg_trans_msg = no_matching_attribute_msg;
			break;
		}
		switch (itry) {
		case 0:
			ctry[0] = toupper(c[0]);
			if (c[1] != '\0') {
				ctry[1] = tolower(c[1]);
				if (c[2] != '\0') {
					ctry[2] = tolower(c[2]);
					ctry[3] = '\0';
					threec = true;
				} else {
					ctry[2] = '\0';
				}
			} else {
				ctry[1] = '\0';
			}
			break;

		case 1:
			ctry[0] = toupper(c[0]);
			if (c[1] != '\0') {
				ctry[1] = toupper(c[1]);
				if (c[2] != '\0') {
					ctry[2] = toupper(c[2]);
					ctry[3] = '\0';
					threec = true;
				} else {
					ctry[2] = '\0';
				}
			} else {
				ctry[1] = '\0';
			}
			break;

		case 2:
			ctry[0] = tolower(c[0]);
			if (c[1] != '\0') {
				ctry[1] = tolower(c[1]);
				if (c[2] != '\0') {
					ctry[2] = tolower(c[2]);
					ctry[3] = '\0';
					threec = true;
				} else {
					ctry[2] = '\0';
				}
			} else {
				ctry[1] = '\0';
			}
			break;

		case 3:
			ctry[0] = tolower(c[0]);
			if (c[1] != '\0') {
				ctry[1] = toupper(c[1]);
				ctry[2] = '\0';
			} else {
				ctry[1] = '\0';
			}
			break;
		}

		if (text_mbstowcs(wc, ctry, N_ELEMENTS(wc)) != (size_t)-1) {
			int j = 0, k = 0;
			bool search = true;

			while (search) {
				if (j < (int)N_ELEMENTS(s->propcats)) {
					if (k < s->propcats[j].n) {
						if (threec) {
							wchar_t wce[4];

							get_ui_entry_label(s->propcats[j].entries[k], 4, false, wce);
							if (wc[0] == wce[0] &&
							    wc[1] == wce[1] &&
							    wc[2] == wce[2]) {
								search = false;
							} else {
								++k;
							}
						} else if (wc[0] == s->propcats[j].labels[k][0] &&
							wc[1] == s->propcats[j].labels[k][1]) {
							search = false;
						} else {
							++k;
						}
					} else {
						k = 0;
						++j;
					}
				} else {
					search = false;
				}
			}

			/* Configure the new filter and apply it. */
			if (j < (int)N_ELEMENTS(s->propcats)) {
				int ind;

				s->config_filt_is_on = false;
				if (s->easy_filt.nv == 0 ||
					(s->easy_filt.nv == 1 &&
					s->stores != EQUIPABLE_YES_STORE)) {
					s->easy_filt.v[s->easy_filt.nv].c =
						EQUIP_EXPR_SELECTOR;
					++s->easy_filt.nv;
				}
				ind = s->easy_filt.nv - 1;
				s->easy_filt.v[ind].s.ex.propind =
					s->propcats[j].off + k;
				switch (j) {
				case 0:
					/* Resistance */
					s->easy_filt.v[ind].s.func =
						(apply_not) ?
						sel_does_not_resist :
						sel_at_least_resists;
					break;

				case 1:
					/*
					 * A boolean flag where on is usually
					 * preferred.
					 */
					s->easy_filt.v[ind].s.func =
						(apply_not) ?
						sel_does_not_have_flag :
						sel_has_flag;
					break;

				case 2:
					/*
					 * A boolean flag where off is usually
					 * preferred.
					 */
					s->easy_filt.v[ind].s.func =
						(apply_not) ?
						sel_has_flag :
						sel_does_not_have_flag;
					break;

				case 3:
				case 4:
					/* Integer modifier. */
					s->easy_filt.v[ind].s.func =
						(apply_not) ?
						sel_has_nonpos_mod :
						sel_has_pos_mod;
					break;

				default:
					assert(0);
				}

				filter_items(s);
				sort_items(s);
				break;
			}
		}

		++itry;
	}
	return EQUIP_CMP_MENU_NEW_PAGE;
}


static void display_object_comparison(const struct equipable_summary *s)
{
	char hbuf[120];
	textblock *tb0;
	region local_area = { 0, 0, 0, 0 };

	assert(s->isel0 >= 0 && s->isel0 < s->nitems);
	tb0 = object_info(s->items[s->isel0].obj, OINFO_NONE);
	object_desc(hbuf, sizeof(hbuf), s->items[s->isel0].obj,
		ODESC_PREFIX | ODESC_FULL | ODESC_CAPITAL);
	if (s->isel1 != -1 && s->isel1 != s->isel0) {
		textblock *tb1 = textblock_new();
		textblock *tb2;

		assert(s->isel1 >= 0 && s->isel1 < s->nitems);
		textblock_append(tb1, "%s\n", hbuf);
		textblock_append_textblock(tb1, tb0);
		object_desc(hbuf, sizeof(hbuf), s->items[s->isel1].obj,
			ODESC_PREFIX | ODESC_FULL | ODESC_CAPITAL);
		textblock_append(tb1, "\n%s\n", hbuf);
		tb2 = object_info(s->items[s->isel1].obj, OINFO_NONE);
		textblock_append_textblock(tb1, tb2);
		textblock_free(tb2);

		textui_textblock_show(tb1, local_area, "Object comparison");
		textblock_free(tb1);
	} else {
		textui_textblock_show(tb0, local_area, hbuf);
	}
	textblock_free(tb0);
}


static bool dump_to_file(const char *path)
{
	return (text_lines_to_file(path, append_to_file)) ? false : true;
}


/**
 * Write all the pages with the current selection of items and properties to
 * display.
 */
static void append_to_file(ang_file *fff)
{
	int cached_ifirst, cached_npage;
	int wid, hgt, y;
	char *buf;

	if (!s_for_file) {
		if (initialize_summary(player, &the_summary)) {
			return;
		}
		s_for_file = the_summary;
	}

	cached_ifirst = s_for_file->ifirst;
	cached_npage = s_for_file->npage;
	s_for_file->ifirst =
		(s_for_file->indinc > 0 || s_for_file->nfilt == 0) ?
		0 : s_for_file->nfilt - 1;
	s_for_file->npage =
		(s_for_file->maxpage <= s_for_file->nfilt) ?
		s_for_file->maxpage : s_for_file->nfilt;

	Term_get_size(&wid, &hgt);
	buf = mem_alloc(5 * wid);

	(void) display_page(s_for_file, player, false);

	/* Dump part of the screen. */
	for (y = s_for_file->irow_combined_equip - s_for_file->nproplab;
		y < s_for_file->irow_combined_equip + 1 + s_for_file->npage;
		++y) {
		char *p = buf;
		int x, a;
		wchar_t c;

		/* Dump a row. */
		for (x = 0; x < wid; ++x) {
			(void) Term_what(x, y, &a, &c);
			p += wctomb(p, c);
		}
		/* Back up over spaces */
		while ((p > buf) && (p[-1] == ' ')) {
			--p;
		}
		/* Terminate */
		*p = '\0';

		file_putf(fff, "%s\n", buf);
	}

	while (1) {
		s_for_file->ifirst += s_for_file->npage * s_for_file->indinc;
		if (s_for_file->indinc > 0) {
			if (s_for_file->ifirst >= s_for_file->nfilt) {
				break;
			}
			if (s_for_file->ifirst + s_for_file->npage >
				s_for_file->nfilt) {
				s_for_file->npage = s_for_file->nfilt -
					s_for_file->ifirst;
			}
		} else {
			if (s_for_file->ifirst < 0) {
				break;
			}
			if (s_for_file->ifirst - s_for_file->npage < -1) {
				s_for_file->npage = s_for_file->ifirst + 1;
			}
		}

		(void) display_page(s_for_file, player, false);

		/*
		 * Dump part of the screen.  Exclude the header that appeared
		 * on the first page.
		 */
		for (y = s_for_file->irow_combined_equip + 1;
			y < s_for_file->irow_combined_equip + 1 +
			s_for_file->npage; ++y) {
			char *p = buf;
			int x, a;
			wchar_t c;

			/* Dump a row. */
			for (x = 0; x < wid; ++x) {
				(void) Term_what(x, y, &a, &c);
				p += wctomb(p, c);
			}
			/* Back up over spaces */
			while ((p > buf) && (p[-1] == ' ')) {
				--p;
			}
			/* Terminate */
			*p = '\0';

			file_putf(fff, "%s\n", buf);
		}
	}

	mem_free(buf);

	s_for_file->ifirst = cached_ifirst;
	s_for_file->npage = cached_npage;
	s_for_file = NULL;
}


static char *set_short_name(const struct object *obj, size_t length)
{
	char buf[80];
	const char *nmsrc;
	size_t nmlen;
	bool tail;
	char *result;

	if (obj->known && obj->known->artifact) {
		nmsrc = obj->known->artifact->name;
		tail = true;
	} else if (obj->known && obj->known->ego) {
		nmsrc = obj->known->ego->name;
		tail = true;
	} else {
		object_desc(buf, N_ELEMENTS(buf), obj, ODESC_COMBAT |
			ODESC_SINGULAR | ODESC_TERSE);
		nmsrc = buf;
		tail = false;
	}
	nmlen = strlen(nmsrc);
	if (nmlen <= length) {
		result = string_make(nmsrc);
	} else if (tail) {
		result = string_make(nmsrc + nmlen - length);
	} else {
		result = string_make(format("%.*s", (int) length, nmsrc));
	}
	return result;
}


#if 0
static bool sel_better_than(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->qual < ex->qual;
}
#endif


static bool sel_at_least_resists(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] >= 1;
}


static bool sel_does_not_resist(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] < 1;
}


static bool sel_has_flag(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] != 0;
}


static bool sel_does_not_have_flag(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] == 0;
}


static bool sel_has_pos_mod(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] > 0;
}


static bool sel_has_nonpos_mod(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->vals[ex->propind] <= 0;
}


#if 0
static bool sel_exclude_slot(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->slot != ex->slot;
}


static bool sel_only_slot(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->slot == ex->slot;
}
#endif


static bool sel_exclude_src(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->src != ex->src;
}


static bool sel_only_src(const struct equipable *eq,
	const union equipable_selfunc_extra *ex)
{
	return eq->src == ex->src;
}


static void apply_simple_filter(const struct equipable_filter *f,
	struct equipable_summary *s)
{
	int i;

	s->nfilt = 0;
	switch (f->simple) {
	case EQUIP_EXPR_AND:
		for (i = 0; i < s->nitems; ++i) {
			int j = 0;

			while (1) {
				if (j >= f->nv) {
					assert(s->nfilt < s->nitems &&
						s->nfilt < s->nalloc);
					s->sorted_indices[s->nfilt] = i;
					++s->nfilt;
					break;
				}
				assert(f->v[j].c == EQUIP_EXPR_SELECTOR);
				if (! (*f->v[j].s.func)(s->items + i,
					&f->v[j].s.ex)) {
					break;
				}
				++j;
			}
		}
		break;

	case EQUIP_EXPR_OR:
		for (i = 0; i < s->nitems; ++i) {
			int j = 0;

			while (1) {
				if (j >= f->nv) {
					break;
				}
				assert(f->v[j].c == EQUIP_EXPR_SELECTOR);
				if ((*f->v[j].s.func)(s->items + i, 
					&f->v[j].s.ex)) {
					assert(s->nfilt < s->nitems &&
						s->nfilt < s->nalloc);
					s->sorted_indices[s->nfilt] = i;
					++s->nfilt;
					break;
				}
				++j;
			}
		}
		break;

	default:
		assert(0);
	}

	s->sorted_indices[s->nfilt] = -1;
}


static void apply_complex_filter(const struct equipable_filter *f,
	struct equipable_summary *s)
{
	bool *stack;
	int nst, i;

	assert(f->nv > 0);
	stack = mem_alloc(f->nv * sizeof(*stack));
	nst = 0;
	s->nfilt = 0;
	for (i = 0; i < s->nitems; ++i) {
		int j = 0;

		while (1) {
			assert(j <= f->nv);

			if (f->v[j].c == EQUIP_EXPR_TERMINATOR) {
				assert(nst == 1);
				if (stack[0]) {
					assert(s->nfilt < s->nitems &&
						s->nfilt < s->nalloc);
					s->sorted_indices[s->nfilt] = i;
					++s->nfilt;
				}
				break;
			}

			switch (f->v[j].c) {
			case EQUIP_EXPR_SELECTOR:
				assert(nst < f->nv);
				stack[nst] = (*f->v[j].s.func)(s->items + i,
					&f->v[j].s.ex);
				++nst;
				break;

			case EQUIP_EXPR_AND:
				assert(nst >= 2);
				stack[nst - 2] =
					(stack[nst - 1] && stack[nst - 2]);
				--nst;
				break;

			case EQUIP_EXPR_OR:
				assert(nst >= 2);
				stack[nst - 2] =
					(stack[nst - 1] || stack[nst - 2]);
				--nst;
				break;

			case EQUIP_EXPR_TERMINATOR:
				assert(0);
			}
			++j;
		}
	}

	s->sorted_indices[s->nfilt] = -1;

	mem_free(stack);
}


static void filter_items(struct equipable_summary *s)
{
	if (s->config_filt_is_on) {
		if (s->config_mod_filt.simple == EQUIP_EXPR_SELECTOR) {
			apply_complex_filter(&s->config_mod_filt, s);
		} else {
			apply_simple_filter(&s->config_mod_filt, s);
		}
	} else if (s->easy_filt.simple == EQUIP_EXPR_SELECTOR) {
		apply_complex_filter(&s->easy_filt, s);
	} else if (s->easy_filt.simple != EQUIP_EXPR_TERMINATOR) {
		apply_simple_filter(&s->easy_filt, s);
	} else {
		/*
		 * Tnere's no filtering; set up the sorted indices to include
		 * everything.
		 */
		int i;

		s->nfilt = s->nitems;
		for (i = 0; i < s->nitems; ++i) {
			s->sorted_indices[i] = i;
		}
		s->sorted_indices[s->nitems] = -1;
	}

	/* Reset the first item shown and the number on that page. */
	s->ifirst = (s->indinc > 0) ? 0 : s->nfilt - 1;
	s->npage = (s->maxpage <= s->nfilt) ? s->maxpage : s->nfilt;
}


static int cmp_by_location(const struct equipable *left,
	const struct equipable *right, int propind)
{
	return (left->src < right->src) ?
		-1 : ((left->src > right->src) ? 1 : 0);
}


static int cmp_by_quality(const struct equipable *left,
	const struct equipable *right, int propind)
{
	return (left->qual < right->qual) ?
		-1 : ((left->qual > right->qual) ? 1 : 0);
}


static int cmp_by_short_name(const struct equipable *left,
	const struct equipable *right, int propind)
{

	return strcmp(left->short_name, right->short_name);
}


static int cmp_by_slot(const struct equipable *left,
	const struct equipable *right, int propind)
{
	return (left->slot < right->slot) ?
		-1 : ((left->slot > right->slot) ? 1 : 0);
}


static int cmp_for_sort_items(const void *left, const void *right)
{
	int ileft = *((int*) left);
	int iright = *((int*) right);
	int i = 0;

	while (1) {
		int cres;

		if (sort_dat.arr[i].func == 0) {
			return 0;
		}
		cres = (*sort_dat.arr[i].func)(sort_dat.items + ileft,
			sort_dat.items + iright, sort_dat.arr[i].propind);
		if (cres != 0) {
			return cres;
		}
		++i;
	}
}


static void sort_items(struct equipable_summary *s)
{
	sort_dat.items = s->items;
	sort_dat.arr = (s->config_sort_is_on) ?
		s->config_sort.v : s->default_sort.v;
	sort(s->sorted_indices, s->nfilt, sizeof(*s->sorted_indices),
		cmp_for_sort_items);
}


typedef bool (*objselfunc)(const struct object *obj, const void *closure);
typedef void (*objusefunc)(const struct object *obj, void *closure);
struct obj_visitor_data {
	objselfunc selfunc;
	objusefunc usefunc;
	const void *selfunc_closure;
	void *usefunc_closure;
};


/**
 * Intended for use with apply_visitor_to_pile() or
 * apply_visitor_to_equipped().
 */
static bool select_any(const struct object *obj, const void *closure)
{
	return true;
}


/**
 * Test for wearable objects in the pack.  Intended for use with
 * apply_visitor_to_pile().  Assumes obj is in the player's gear.
 */
static bool select_nonequipped_wearable(const struct object *obj,
	const void *closure)
{
	const struct player *p = closure;

	return tval_is_wearable(obj) && !object_is_equipped(p->body, obj);
}


/**
 * Test for wearable objects.  Intended for use with apply_visitor_to_pile().
 */
static bool select_wearable(const struct object *obj, const void *closure)
{
	return tval_is_wearable(obj);
}


/**
 * Test for wearable objects that are not ignored and have been seen (have a
 * known object).  Intended for use with apply_visitor_to_pile().
 */
static bool select_seen_wearable(const struct object *obj, const void *closure)
{
	return tval_is_wearable(obj) && obj->known && !ignore_item_ok(obj);
}


/**
 * Increment a counter; intended for use with apply_visitor_to_pile() or
 * apply_visitor_to_equipped().
 */
static void count_objects(const struct object *obj, void *closure)
{
	int *n = closure;

	++*n;
}


/**
 * Add an object to the summary of equipable items; intended for use with
 * apply_visitor_to_pile() or apply_visitor_to_equipped().
 */
struct add_obj_to_summary_closure {
	const struct player *p;
	struct equipable_summary *summary;
	enum equipable_source src;
};
static void add_obj_to_summary(const struct object *obj, void *closure)
{
	struct add_obj_to_summary_closure *c = closure;
	struct equipable *e;
	struct cached_object_data *cache;
	int i;

	assert(c->summary->nitems < c->summary->nalloc);
	e = c->summary->items + c->summary->nitems;
	++c->summary->nitems;

	if (!e->vals) {
		e->vals = mem_alloc(c->summary->nprop * sizeof(*e->vals));
	}
	if (!e->auxvals) {
		e->auxvals = mem_alloc(c->summary->nprop *
			sizeof(*e->auxvals));
	}
	cache = NULL;
	for (i = 0; i < (int)N_ELEMENTS(c->summary->propcats); ++i) {
		int j;

		for (j = 0; j < c->summary->propcats[i].n; ++j) {
			compute_ui_entry_values_for_object(
				c->summary->propcats[i].entries[j], obj,
				c->p, &cache,
				e->vals + j + c->summary->propcats[i].off,
				e->auxvals + j + c->summary->propcats[i].off);
		}
	}
	release_cached_object_data(cache);

	if (c->summary->nshortnm > 0) {
		string_free(e->short_name);
		e->short_name = set_short_name(obj, c->summary->nshortnm);
		e->nmlen = (int)strlen(e->short_name);
	}

	e->obj = obj;
	e->src = c->src;

	switch (ignore_level_of(obj)) {
	case IGNORE_GOOD:
		e->qual = EQUIP_QUAL_GOOD;
		break;

	case IGNORE_AVERAGE:
		e->qual = EQUIP_QUAL_AVERAGE;
		break;

	case IGNORE_BAD:
		e->qual = EQUIP_QUAL_BAD;
		break;

	default:
		/* Try to get some finer distinctions. */
		if (obj->known && obj->known->artifact) {
			e->qual = EQUIP_QUAL_ARTIFACT;
		} else if (obj->known && obj->known->ego) {
			e->qual = EQUIP_QUAL_EGO;
		} else {
			/* Treat unknown items as average. */
			e->qual = EQUIP_QUAL_AVERAGE;
		}
		break;
	}

	e->slot = wield_slot(obj);
	e->ch = object_char(obj);
	e->at = object_attr(obj);
}


/**
 * Given the function pointers in visitor, apply them to each object in the
 * pile pointed to by obj.  Assumes that the functions do not modify the
 * structure of the pile.
 */
static void apply_visitor_to_pile(const struct object *obj,
	struct obj_visitor_data *visitor)
{
	while (obj) {
		if ((*visitor->selfunc)(obj, visitor->selfunc_closure)) {
			(*visitor->usefunc)(obj, visitor->usefunc_closure);
		}
		obj = obj->next;
	}
}


/**
 * Given the function pointers in visitor, apply them to each object in the
 * given player's inventory.
 */
static void apply_visitor_to_equipped(struct player *p,
	struct obj_visitor_data *visitor)
{
	int i;

	for (i = 0; i < p->body.count; ++i) {
		const struct object *obj = slot_object(p, i);

		if (obj && (*visitor->selfunc)(obj,
			visitor->selfunc_closure)) {
			(*visitor->usefunc)(obj, visitor->usefunc_closure);
		}
	}
}


static int reconfigure_for_term_if_necessary(bool update_names,
	struct equipable_summary *s)
{
	int result = 0;
	int min_length = 16;
	int ncol, nrow, length, i;

	Term_get_size(&ncol, &nrow);
	if (s->term_ncol == ncol && s->term_nrow == nrow) {
		return result;
	}

	/*
	 * Have s->irow_combined_equip + 1 rows of a header and one row, for
	 * prompts, of a footer.
	 */
	s->maxpage = nrow - s->irow_combined_equip - 2;
	if (s->maxpage < 1) {
		result = 1;
	} else if (s->npage > s->maxpage) {
		s->npage = s->maxpage;
		if (s->work_sel != -1 &&
			(s->work_sel - s->ifirst) * s->indinc >= s->npage) {
			s->work_sel = s->ifirst + (s->npage - 1) * s->indinc;
		}
	}

	/*
	 * Leave a space between the name and the properties.  Don't include
	 * the core stat modifiers in the first view.
	 */
	length = ncol - s->nprop + s->propcats[N_ELEMENTS(s->propcats) - 1].n -
		1 - s->icol_name;
	if (length < min_length) {
		/* Try shifting the other modifiers to the second view. */
		length += s->propcats[N_ELEMENTS(s->propcats) - 2].n;
		if (length < min_length) {
			/* Try a three view layout. */
			length += s->propcats[N_ELEMENTS(s->propcats) - 3].n;
			if (length < min_length) {
				/* Give up. */
				result = 1;
			}
			s->nview = 3;
			for (i = 0; i < (int)N_ELEMENTS(s->propcats) - 3; ++i) {
				s->propcats[i].nvw[0] = s->propcats[i].n;
				s->propcats[i].nvw[1] = 0;
				s->propcats[i].nvw[2] = 0;
				s->propcats[i].ivw[0] = 0;
				s->propcats[i].ivw[1] = 0;
				s->propcats[i].ivw[2] = 0;
			}
			for (i = (int)N_ELEMENTS(s->propcats) - 3;
				i < (int)N_ELEMENTS(s->propcats) - 1; ++i) {
				s->propcats[i].nvw[0] = 0;
				s->propcats[i].nvw[1] = s->propcats[i].n;
				s->propcats[i].nvw[2] = 0;
				s->propcats[i].ivw[0] = 0;
				s->propcats[i].ivw[1] = 0;
				s->propcats[i].ivw[2] = 0;
			}
			s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[0] = 0;
			s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[1] = 0;
			s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[2] =
				s->propcats[N_ELEMENTS(s->propcats) - 1].n;
			s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[0] = 0;
			s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[1] = 0;
			s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[2] = 0;
		} else {
			s->nview = 2;
			for (i = 0; i < (int)N_ELEMENTS(s->propcats) - 2; ++i) {
				s->propcats[i].nvw[0] = s->propcats[i].n;
				s->propcats[i].nvw[1] = 0;
				s->propcats[i].nvw[2] = 0;
				s->propcats[i].ivw[0] = 0;
				s->propcats[i].ivw[1] = 0;
				s->propcats[i].ivw[2] = 0;
			}
			for (i = (int)N_ELEMENTS(s->propcats) - 2;
				i < (int)N_ELEMENTS(s->propcats); ++i) {
				s->propcats[i].nvw[0] = 0;
				s->propcats[i].nvw[1] = s->propcats[i].n;
				s->propcats[i].nvw[2] = 0;
				s->propcats[i].ivw[0] = 0;
				s->propcats[i].ivw[1] = 0;
				s->propcats[i].ivw[2] = 0;
			}
		}
	} else {
		s->nview = 2;
		for (i = 0; i < (int)N_ELEMENTS(s->propcats) - 1; ++i) {
			s->propcats[i].nvw[0] = s->propcats[i].n;
			s->propcats[i].nvw[1] = 0;
			s->propcats[i].nvw[2] = 0;
			s->propcats[i].ivw[0] = 0;
			s->propcats[i].ivw[1] = 0;
			s->propcats[i].ivw[2] = 0;
		}
		s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[0] = 0;
		s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[1] =
			s->propcats[N_ELEMENTS(s->propcats) - 1].n;
		s->propcats[N_ELEMENTS(s->propcats) - 1].nvw[2] = 0;
		s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[0] = 0;
		s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[1] = 0;
		s->propcats[N_ELEMENTS(s->propcats) - 1].ivw[2] = 0;
	}
	if (s->iview >= s->nview) {
		s->iview = s->nview - 1;
	}
	if (length > 20) {
		length = 20;
	}
	if (length != s->nshortnm && update_names) {
		for (i = 0; i < s->nitems; ++i) {
			string_free(s->items[i].short_name);
			s->items[i].short_name =
				set_short_name(s->items[i].obj, length);
			s->items[i].nmlen =
				(int)strlen(s->items[i].short_name);
		}
	}
	s->nshortnm = length;

	s->term_ncol = ncol;
	s->term_nrow = nrow;

	return result;
}


static void compute_player_and_equipment_values(struct player *p,
	struct equipable_summary *s)
{
	struct cached_player_data *pcache;
	struct ui_entry_combiner_state *cstates;
	struct ui_entry_combiner_funcs cfuncs;
	int i;

	if (! s->p_and_eq_vals) {
		s->p_and_eq_vals = mem_alloc(s->nprop *
			sizeof(*s->p_and_eq_vals));
	}
	if (! s->p_and_eq_auxvals) {
		s->p_and_eq_auxvals = mem_alloc(s->nprop *
			sizeof(*s->p_and_eq_auxvals));
	}

	pcache = NULL;
	cstates = mem_alloc(s->nprop * sizeof(*cstates));
	for (i = 0; i < (int)N_ELEMENTS(s->propcats); ++i) {
		int j;

		for (j = 0; j < s->propcats[i].n; ++j) {
			const struct ui_entry *entry =
				s->propcats[i].entries[j];
			int rendind = get_ui_entry_renderer_index(entry);
			int combind =
				ui_entry_renderer_query_combiner(rendind);
			int v, a;

			assert(combind > 0);
			(void) ui_entry_combiner_get_funcs(combind, &cfuncs);
			compute_ui_entry_values_for_player(
				entry, p, &pcache, &v, &a);
			(*cfuncs.init_func)(v, a,
				cstates + j + s->propcats[i].off);
		}
	}
	release_cached_player_data(pcache);

	/* Combine with the values from the equipment. */
	for (i = 0; i < p->body.count; ++i) {
		const struct object *obj = slot_object(p, i);
		struct cached_object_data *cache = NULL;
		int j;

		if (!obj) {
			continue;
		}
		for (j = 0; j < (int)N_ELEMENTS(s->propcats); ++j) {
			int k;

			for (k = 0; k < s->propcats[j].n; ++k) {
				const struct ui_entry *entry =
					s->propcats[j].entries[k];
				int rendind = get_ui_entry_renderer_index(entry);
				int combind = ui_entry_renderer_query_combiner(rendind);
				int v, a;

				assert(combind > 0);
				(void) ui_entry_combiner_get_funcs(
					combind, &cfuncs);
				compute_ui_entry_values_for_object(
					s->propcats[j].entries[k], obj, p,
					&cache, &v, &a);
				(*cfuncs.accum_func)(v, a,
					cstates + k + s->propcats[j].off);
			}
		}
		release_cached_object_data(cache);
	}

	for (i = 0; i < (int)N_ELEMENTS(s->propcats); ++i) {
		int j;

		for (j = 0; j < s->propcats[i].n; ++j) {
			const struct ui_entry *entry =
				s->propcats[i].entries[j];
			int rendind = get_ui_entry_renderer_index(entry);
			int combind =
				ui_entry_renderer_query_combiner(rendind);

			assert(combind > 0);
			(void) ui_entry_combiner_get_funcs(combind, &cfuncs);
			(*cfuncs.finish_func)(
				cstates + j + s->propcats[i].off);
			s->p_and_eq_vals[j + s->propcats[i].off] =
				cstates[j + s->propcats[i].off].accum;
			s->p_and_eq_auxvals[j + s->propcats[i].off] =
				cstates[j + s->propcats[i].off].accum_aux;
		}
	}

	mem_free(cstates);
}


static bool check_for_two_categories(const struct ui_entry *entry,
	void *closure)
{
	char **categories = closure;

	return ui_entry_has_category(entry, categories[0]) &&
		ui_entry_has_category(entry, categories[1]);
}


static int initialize_summary(struct player *p,
	struct equipable_summary **s)
{
	struct obj_visitor_data visitor;
	struct add_obj_to_summary_closure add_obj_data;
	int count, i;

	if (*s == NULL) {
		char *categories[] = {
			"resistances",
			"abilities",
			"hindrances",
			"modifiers",
			"stat_modifiers"
		};
		char *test_categories[2];

		*s = mem_alloc(sizeof(**s));
		(*s)->items = NULL;
		(*s)->sorted_indices = NULL;
		(*s)->p_and_eq_vals = NULL;
		(*s)->p_and_eq_auxvals = NULL;
		(*s)->dlg_trans_msg = NULL;
		(*s)->nitems = 0;
		(*s)->nalloc = 0;
		(*s)->stores = EQUIPABLE_NO_STORE;
		(*s)->ifirst = 0;
		(*s)->indinc = 1;
		(*s)->iview = 0;
		(*s)->npage = 0;
		(*s)->nshortnm = 0;
		(*s)->term_ncol = -1;
		(*s)->term_nrow = -1;

		/* These are currently hardwired layout choices. */
		(*s)->nproplab = 2;
		(*s)->irow_combined_equip = (*s)->nproplab + 1;
		/*
		 * Leave room for a character (item character), a space,
		 * a character (item location code), and a space.
		 */
		(*s)->icol_name = 4;

		/*
		 * These need to be done once after the game configuration is
		 * read.
		 */
		(*s)->nprop = 0;
		test_categories[0] = "EQUIPCMP_SCREEN";
		for (i = 0; i < (int)N_ELEMENTS((*s)->propcats); ++i) {
			struct ui_entry_iterator *ui_iter;
			int n, j;

			test_categories[1] = categories[i];
			ui_iter = initialize_ui_entry_iterator(
				check_for_two_categories, test_categories,
				test_categories[1]);
			n = count_ui_entry_iterator(ui_iter);
			(*s)->nprop += n;
			(*s)->propcats[i].n = n;
			(*s)->propcats[i].off = (i == 0) ?
				0 : (*s)->propcats[i - 1].off +
				(*s)->propcats[i - 1].n;
			(*s)->propcats[i].entries = mem_alloc(n *
				sizeof(*(*s)->propcats[i].entries));
			(*s)->propcats[i].labels = mem_alloc(n *
				sizeof(*(*s)->propcats[i].labels));
			(*s)->propcats[i].label_buffer = mem_alloc(n *
				((*s)->nproplab + 1) *
				sizeof(*(*s)->propcats[i].label_buffer));
			for (j = 0; j < n; ++j) {
				struct ui_entry *entry =
					advance_ui_entry_iterator(ui_iter);

				(*s)->propcats[i].entries[j] = entry;
				(*s)->propcats[i].labels[j] =
					(*s)->propcats[i].label_buffer +
					j * ((*s)->nproplab + 1);
				get_ui_entry_label(entry,
					(*s)->nproplab + 1, true,
					(*s)->propcats[i].labels[j]);
			}
			release_ui_entry_iterator(ui_iter);
		}

		/*
		 * Start with nothing for the easy filter but set up space
		 * so it is trivial to add one term filtering on an attribute
		 * and another that includes/excludes the stores' inventories.
		 */
		(*s)->easy_filt.nalloc = 3;
		(*s)->easy_filt.v = mem_alloc((*s)->easy_filt.nalloc *
			sizeof(*(*s)->easy_filt.v));
		switch ((*s)->stores) {
		case EQUIPABLE_NO_STORE:
			(*s)->easy_filt.simple = EQUIP_EXPR_AND;
			(*s)->easy_filt.nv = 1;
			(*s)->easy_filt.v[0].s.func = sel_exclude_src;
			(*s)->easy_filt.v[0].s.ex.src = EQUIP_SOURCE_STORE;
			(*s)->easy_filt.v[0].c = EQUIP_EXPR_SELECTOR;
			break;

		case EQUIPABLE_ONLY_STORE:
			(*s)->easy_filt.simple = EQUIP_EXPR_AND;
			(*s)->easy_filt.nv = 1;
			(*s)->easy_filt.v[0].s.func = sel_only_src;
			(*s)->easy_filt.v[0].s.ex.src = EQUIP_SOURCE_STORE;
			(*s)->easy_filt.v[0].c = EQUIP_EXPR_SELECTOR;
			break;

		case EQUIPABLE_YES_STORE:
			/* There's no filtering to be done. */
			(*s)->easy_filt.simple = EQUIP_EXPR_TERMINATOR;
			(*s)->easy_filt.nv = 0;
			break;
		}
		(*s)->easy_filt.v[(*s)->easy_filt.nv].c =
			EQUIP_EXPR_TERMINATOR;
		if ((*s)->easy_filt.nv + 1 < (*s)->easy_filt.nalloc) {
			(*s)->easy_filt.v[(*s)->easy_filt.nv + 1].c =
				EQUIP_EXPR_TERMINATOR;
		}

		/* Start with nothing for the specially configured filter. */
		(*s)->config_filt.v = NULL;
		(*s)->config_filt.simple = EQUIP_EXPR_TERMINATOR;
		(*s)->config_filt.nv = 0;
		(*s)->config_filt.nalloc = 0;
		(*s)->config_mod_filt.v = NULL;
		(*s)->config_mod_filt.simple = EQUIP_EXPR_TERMINATOR;
		(*s)->config_mod_filt.nv = 0;
		(*s)->config_mod_filt.nalloc = 0;
		(*s)->config_filt_is_on = false;

		/*
		 * The default sort is by equipment slot.  Any ties are
		 * resolved first by the location of the object, rough object
		 * quality, and then alphabetical order.
		 */
		(*s)->default_sort.nalloc = 5;
		(*s)->default_sort.v = mem_alloc((*s)->default_sort.nalloc *
			sizeof(*(*s)->default_sort.v));
		(*s)->default_sort.v[0].func = cmp_by_slot;
		(*s)->default_sort.v[0].propind = 0;
		(*s)->default_sort.v[1].func = cmp_by_location;
		(*s)->default_sort.v[1].propind = 0;
		(*s)->default_sort.v[2].func = cmp_by_quality;
		(*s)->default_sort.v[2].propind = 0;
		(*s)->default_sort.v[3].func = cmp_by_short_name;
		(*s)->default_sort.v[3].propind = 0;
		(*s)->default_sort.v[4].func = 0;
		(*s)->default_sort.v[4].propind = 0;
		(*s)->default_sort.nv = 4;

		/* Start with nothing for the specially configured sort. */
		(*s)->config_sort.v = NULL;
		(*s)->config_sort.nv = 0;
		(*s)->config_sort.nalloc = 0;
		(*s)->config_sort_is_on = false;
	}

	/* These need to be redone on a change to the terminal size. */
	if (reconfigure_for_term_if_necessary(false, *s)) {
		return 1;
	}

	/*
	 * These need to be redone for any change in the equipped items, pack,
	 * home, or stores.
	 */

	/* Count the available items to include. */
	count = 0;
	visitor.usefunc = count_objects;
	visitor.usefunc_closure = &count;
	visitor.selfunc = select_any;
	visitor.selfunc_closure = NULL;
	apply_visitor_to_equipped(p, &visitor);
	visitor.selfunc = select_nonequipped_wearable;
	visitor.selfunc_closure = p;
	apply_visitor_to_pile(p->gear, &visitor);
	if (cave) {
		visitor.selfunc = select_seen_wearable;
		visitor.selfunc_closure = NULL;
		apply_visitor_to_pile(square_object(cave, p->grid), &visitor);
	}
	visitor.selfunc = select_wearable;
	visitor.selfunc_closure = NULL;
	apply_visitor_to_pile(stores[STORE_HOME].stock, &visitor);
	for (i = 0; i < MAX_STORES; ++i) {
		if (i == STORE_HOME) {
			continue;
		}
		apply_visitor_to_pile(stores[i].stock, &visitor);
	}

	/* Allocate storage and add the available items. */
	if (count > (*s)->nalloc) {
		mem_free((*s)->sorted_indices);
		cleanup_summary_items(*s);
		mem_free((*s)->items);
		(*s)->items = mem_zalloc(count * sizeof(*(*s)->items));
		(*s)->sorted_indices =
			mem_alloc((count + 1) * sizeof(*(*s)->sorted_indices));
		(*s)->nalloc = count;
	}
	(*s)->nitems = 0;
	visitor.usefunc = add_obj_to_summary;
	visitor.usefunc_closure = &add_obj_data;
	add_obj_data.p = p;
	add_obj_data.summary = *s;
	add_obj_data.src = EQUIP_SOURCE_WORN;
	visitor.selfunc = select_any;
	visitor.selfunc_closure = NULL;
	apply_visitor_to_equipped(p, &visitor);
	add_obj_data.src = EQUIP_SOURCE_PACK;
	visitor.selfunc = select_nonequipped_wearable;
	visitor.selfunc_closure = p;
	apply_visitor_to_pile(p->gear, &visitor);
	if (cave) {
		add_obj_data.src = EQUIP_SOURCE_FLOOR;
		visitor.selfunc = select_seen_wearable;
		visitor.selfunc_closure = NULL;
		apply_visitor_to_pile(square_object(cave, p->grid), &visitor);
	}
	add_obj_data.src = EQUIP_SOURCE_HOME;
	visitor.selfunc = select_wearable;
	visitor.selfunc_closure = NULL;
	apply_visitor_to_pile(stores[STORE_HOME].stock, &visitor);
	add_obj_data.src = EQUIP_SOURCE_STORE;
	for (i = 0; i < MAX_STORES; ++i) {
		if (i == STORE_HOME) {
			continue;
		}
		apply_visitor_to_pile(stores[i].stock, &visitor);
	}

	compute_player_and_equipment_values(p, *s);

	/*
	 * Do an initial filtering and sorting of the items so they're ready
	 * for display.
	 */
	filter_items(*s);
	sort_items(*s);

	/* Reset selection state. */
	(*s)->isel0 = -1;
	(*s)->isel1 = -1;
	(*s)->work_sel = -1;

	return 0;
}


static void cleanup_summary(struct equipable_summary *s)
{
	int i;

	if (s == NULL) {
		return;
	}
	cleanup_summary_items(s);
	mem_free(s->p_and_eq_auxvals);
	mem_free(s->p_and_eq_vals);
	mem_free(s->sorted_indices);
	mem_free(s->items);
	mem_free(s->config_sort.v);
	mem_free(s->default_sort.v);
	mem_free(s->config_mod_filt.v);
	mem_free(s->config_filt.v);
	mem_free(s->easy_filt.v);
	for (i = 0; i < (int)N_ELEMENTS(s->propcats); ++i) {
		mem_free(s->propcats[i].label_buffer);
		mem_free(s->propcats[i].labels);
		mem_free(s->propcats[i].entries);
	}
	mem_free(s);
}


static void cleanup_summary_items(struct equipable_summary *s)
{
	int i;

	for (i = 0; i < s->nalloc; ++i) {
		string_free(s->items[i].short_name);
		mem_free(s->items[i].auxvals);
		mem_free(s->items[i].vals);
	}
}


static wchar_t source_to_char(enum equipable_source src)
{
	static bool first_call = true;
	static wchar_t wchars[7];
	wchar_t result;

	if (first_call) {
		if (text_mbstowcs(wchars, "epfhs ", N_ELEMENTS(wchars)) == (size_t)-1) {
			quit("Invalid encoding for item location codes");
		}
		first_call = false;
	}

	switch (src) {
	case EQUIP_SOURCE_WORN:
		result = wchars[0];
		break;

	case EQUIP_SOURCE_PACK:
		result = wchars[1];
		break;

	case EQUIP_SOURCE_FLOOR:
		result = wchars[2];
		break;

	case EQUIP_SOURCE_HOME:
		result = wchars[3];
		break;

	case EQUIP_SOURCE_STORE:
		result = wchars[4];
		break;

	default:
		result = wchars[5];
		break;
	}

	return result;
}


static int display_page(struct equipable_summary *s, const struct player *p,
	bool allow_reconfig)
{
	struct ui_entry_details rdetails;
	int color = (COLOUR_WHITE);
	int i;

	/* Try to handle terminal size changes while displaying the summary. */
	if (allow_reconfig) {
		if (reconfigure_for_term_if_necessary(true, s)) {
			return 1;
		}
	}

	/*
	 * Display the column labels and the combined values for @ and the
	 * the current equipment.
	 */
	rdetails.label_position.x = s->icol_name + s->nshortnm + 1;
	rdetails.label_position.y = s->irow_combined_equip - s->nproplab;
	rdetails.value_position.x = rdetails.label_position.x;
	rdetails.value_position.y = s->irow_combined_equip;
	rdetails.position_step = loc(1, 0);
	rdetails.combined_position = loc(0, 0);
	rdetails.vertical_label = true;
	rdetails.alternate_color_first = false;
	rdetails.show_combined = false;
	Term_putch(s->icol_name - 4, rdetails.value_position.y, color, '@');
	for (i = 0; i < (int)N_ELEMENTS(s->propcats); ++i) {
		int j;

		if (!s->propcats[i].nvw[s->iview]) {
			continue;
		}
		for (j = 0; j < s->propcats[i].nvw[s->iview]; ++j) {
			int joff = j + s->propcats[i].ivw[s->iview];

			rdetails.known_rune = is_ui_entry_for_known_rune(
				s->propcats[i].entries[joff], p);
			ui_entry_renderer_apply(get_ui_entry_renderer_index(
				s->propcats[i].entries[joff]),
				s->propcats[i].labels[joff], s->nproplab,
				s->p_and_eq_vals + joff + s->propcats[i].off,
				s->p_and_eq_auxvals + joff +
				s->propcats[i].off, 1, &rdetails);
			++rdetails.label_position.x;
			++rdetails.value_position.x;
		}
	}

	/* Display the items available on the current page. */
	++rdetails.value_position.y;
	for (i = 0; i < s->npage; ++i) {
		const struct equipable *e;
		int icnv, isort, j, nmcolor;

		icnv = s->ifirst + s->indinc * i;
		assert(icnv >= 0 && icnv < s->nfilt);
		isort = s->sorted_indices[icnv];
		assert(isort >= 0 && isort < s->nitems);
		e = s->items + isort;

		Term_putch(s->icol_name - 4, rdetails.value_position.y,
			e->at, e->ch);
		Term_putch(s->icol_name - 2, rdetails.value_position.y, color,
			source_to_char(e->src));
		if (isort == s->isel0 || isort == s->isel1 ||
			icnv == s->work_sel) {
			nmcolor = (COLOUR_L_BLUE);
		} else {
			nmcolor = color;
		}
		Term_putstr(s->icol_name, rdetails.value_position.y,
			e->nmlen, nmcolor, e->short_name);
		rdetails.value_position.x = s->icol_name + s->nshortnm + 1;
		for (j = 0; j < (int)N_ELEMENTS(s->propcats); ++j) {
			int k;

			if (!s->propcats[j].nvw[s->iview]) {
				continue;
			}
			for (k = 0; k < s->propcats[j].nvw[s->iview]; ++k) {
				int koff = k + s->propcats[j].ivw[s->iview];

				ui_entry_renderer_apply(
					get_ui_entry_renderer_index(
					s->propcats[j].entries[koff]), NULL, 0,
					e->vals + koff + s->propcats[j].off,
					e->auxvals + koff + s->propcats[j].off,
					1, &rdetails);
				++rdetails.value_position.x;
			}
		}
		++rdetails.value_position.y;
	}

	return 0;
}


static void init_ui_equip_cmp(void)
{
	/* There's nothing to do; initialize lazily. */
}


static void cleanup_ui_equip_cmp(void)
{
	cleanup_summary(the_summary);
	the_summary = NULL;
}


struct init_module ui_equip_cmp_module = {
	.name = "ui-equip-cmp",
	.init = init_ui_equip_cmp,
	.cleanup = cleanup_ui_equip_cmp
};
