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
#include "files.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "ui-event.h"
#include "ui-menu.h"
#include "spells.h"
#include "target.h"
#include "wizard.h"
#include "z-term.h"


#ifdef ALLOW_DEBUG
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
s16b get_idx_from_name(char *s)
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
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x;

	struct keypress kp;


	for (i = 0; i < MONSTER_FLOW_DEPTH; ++i)
	{
		/* Update map */
		for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
		{
			for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
			{
				byte a = TERM_RED;

				if (!in_bounds_fully(y, x)) continue;

				/* Display proper cost */
				if (cave->cost[y][x] != i) continue;

				/* Reliability in yellow */
				if (cave->when[y][x] == cave->when[py][px])
					a = TERM_YELLOW;

				/* Display player/floors/walls */
				if ((y == py) && (x == px))
					print_rel('@', a, y, x);
				else if (cave_floor_bold(y, x))
					print_rel('*', a, y, x);
				else
					print_rel('#', a, y, x);
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

	screen_save();

	prt("Previous keypresses (top most recent):", 0, 0);

	for (i = 0; i < KEYLOG_SIZE; i++) {
		if (i < log_size) {
			/* find the keypress from our log */
			int j = (log_i + i) % KEYLOG_SIZE;
			struct keypress k = keylog[j];

			/* ugh. it would be nice if there was a verion of keypress_to_text
			 * which took only one keypress. */
			struct keypress keys[2] = {k, {EVT_NONE, 0}};
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
 * Hack -- Teleport to the target
 */
static void do_cmd_wiz_bamf(void)
{
	s16b x, y;

	/* Must have a target */
	if (!target_okay()) return;

	/* Teleport to the target */
	target_get(&x, &y);
	teleport_player_to(y, x);
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
	for (i = 0; i < A_MAX; i++)
	{
		/* Prompt */
		strnfmt(ppp, sizeof(ppp), "%s (3-118): ", stat_names[i]);

		/* Default */
		strnfmt(tmp_val, sizeof(tmp_val), "%d", p_ptr->stat_max[i]);

		/* Query */
		if (!get_string(ppp, tmp_val, 4)) return;

		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Verify */
		if (tmp_int > 18+100) tmp_int = 18+100;
		else if (tmp_int < 3) tmp_int = 3;

		/* Save it */
		p_ptr->stat_cur[i] = p_ptr->stat_max[i] = tmp_int;
	}


	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%ld", (long)(p_ptr->au));

	/* Query */
	if (!get_string("Gold: ", tmp_val, 10)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->au = tmp_long;


	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%ld", (long)(p_ptr->exp));

	/* Query */
	if (!get_string("Experience: ", tmp_val, 10)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	if (tmp_long > p_ptr->exp)
		player_exp_gain(p_ptr, tmp_long - p_ptr->exp);
	else
		player_exp_lose(p_ptr, p_ptr->exp - tmp_long, FALSE);
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
			o_ptr->number, o_ptr->pval[DEFAULT_PVAL],
			o_ptr->artifact ? o_ptr->artifact->aidx : 0,
			o_ptr->ego ? o_ptr->ego->eidx : 0,
			(long)object_value(o_ptr, 1, FALSE)), 6, j);

	prt("+------------FLAGS0------------+", 8, j);
	prt("AFFECT..........SLAY.......BRAND", 9, j);
	prt("                ae      xxxpaefc", 10, j);
	prt("siwdcc  ssidsasmnvudotgddduoclio", 11, j);
	prt("tnieoh  trnipthgiinmrrnrrmniierl", 12, j);
	prt("rtsxna..lcfgdkttmldncltggndsdced", 13, j);
	prt_binary(f, 0, 14, j, '*', 32);
	prt_binary(o_ptr->known_flags, 0, 15, j, '+', 32);

	prt("+------------FLAGS1------------+", 16, j);
	prt("SUST........IMM.RESIST.........", 17, j);
	prt("            afecaefcpfldbc s n  ", 18, j);
	prt("siwdcc      cilocliooeialoshnecd", 19, j);
	prt("tnieoh      irelierliatrnnnrethi", 20, j);
	prt("rtsxna......decddcedsrekdfddxhss", 21, j);
	prt_binary(f, 32, 22, j, '*', 32);
	prt_binary(o_ptr->known_flags, 32, 23, j, '+', 32);

	prt("+------------FLAGS2------------+", 8, j+34);
	prt("s   ts hn    tadiiii   aiehs  hp", 9, j+34);
	prt("lf  eefoo    egrgggg  bcnaih  vr", 10, j+34);
	prt("we  lerlf   ilgannnn  ltssdo  ym", 11, j+34);	prt("da reiedu   merirrrr  eityew ccc", 12, j+34);
	prt("itlepnele   ppanaefc  svaktm uuu", 13, j+34);
	prt("ghigavail   aoveclio  saanyo rrr", 14, j+34);
	prt("seteticf    craxierl  etropd sss", 15, j+34);
	prt("trenhste    tttpdced  detwes eee", 16, j+34);
	prt_binary(f, 64, 17, j + 34, '*', 32);
	prt_binary(o_ptr->known_flags, 64, 18, j + 34, '+', 32);

	prt("o_ptr->ident:", 20, j+34);
	prt(format("sense  %c  worn   %c  empty   %c  known   %c",
		(o_ptr->ident & IDENT_SENSE) ? '+' : ' ',
		(o_ptr->ident & IDENT_WORN) ? '+' : ' ',
		(o_ptr->ident & IDENT_EMPTY) ? '+' : ' ',
		(o_ptr->ident & IDENT_KNOWN) ? '+' : ' '), 21, j+34);
	prt(format("store  %c  attack %c  defence %c  effect  %c",
		(o_ptr->ident & IDENT_STORE) ? '+' : ' ',
		(o_ptr->ident & IDENT_ATTACK) ? '+' : ' ',
		(o_ptr->ident & IDENT_DEFENCE) ? '+' : ' ',
		(o_ptr->ident & IDENT_EFFECT) ? '+' : ' '), 22, j+34);
	prt(format("indest %c  ego    %c",
		(o_ptr->ident & IDENT_INDESTRUCT) ? '+' : ' ',
		(o_ptr->ident & IDENT_NAME) ? '+' : ' '), 23, j+34);
}



static const region wiz_create_item_area = { 0, 0, 0, 0 };

/** Object kind selection */
void wiz_create_item_subdisplay(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	object_kind **choices = menu_priv(m);
	char buf[80];

	object_kind_name(buf, sizeof buf, choices[oid], TRUE);
	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

bool wiz_create_item_subaction(menu_type *m, const ui_event *e, int oid)
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
	object_prep(i_ptr, kind, p_ptr->depth, RANDOMISE);

	/* Apply magic (no messages, no artifacts) */
	apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);

	/* Mark as cheat, and where created */
	i_ptr->origin = ORIGIN_CHEAT;
	i_ptr->origin_depth = p_ptr->depth;

	if (kind->tval == TV_GOLD)
		make_gold(i_ptr, p_ptr->depth, kind->sval);

	/* Drop the object from heaven */
	drop_near(cave, i_ptr, 0, p_ptr->py, p_ptr->px, TRUE);

	return FALSE;
}

menu_iter wiz_create_item_submenu =
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
				of_has(kind->flags, OF_INSTA_ART))
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
	p_ptr->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(p_ptr);

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

#define WIZ_TWEAK(attribute) do {\
	p = "Enter new '" #attribute "' setting: ";\
	strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->attribute);\
	if (!get_string(p, tmp_val, 6)) return;\
	o_ptr->attribute = atoi(tmp_val);\
	wiz_display_item(o_ptr, TRUE);\
} while (0)
	for (i = 0; i < MAX_PVALS; i++) {
		WIZ_TWEAK(pval[i]);
		if (o_ptr->pval[i])
			o_ptr->num_pvals = (i + 1);
	}
	WIZ_TWEAK(to_a);
	WIZ_TWEAK(to_h);
	WIZ_TWEAK(to_d);

	p = "Enter new ego item index: ";
	strnfmt(tmp_val, sizeof(tmp_val), "0");
	if (o_ptr->ego)
		strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->ego->eidx);
	if (!get_string(p, tmp_val, 6)) return;
	val = atoi(tmp_val);
	if (val) {
		o_ptr->ego = &e_info[val];
		ego_apply_magic(o_ptr, p_ptr->depth);
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
			object_prep(i_ptr, o_ptr->kind, p_ptr->depth, RANDOMISE);
			apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);
		}

		/* Apply good magic, but first clear object */
		else if (ch.code == 'g' || ch.code == 'G')
		{
			object_prep(i_ptr, o_ptr->kind, p_ptr->depth, RANDOMISE);
			apply_magic(i_ptr, p_ptr->depth, FALSE, TRUE, FALSE);
		}

		/* Apply great magic, but first clear object */
		else if (ch.code == 'e' || ch.code == 'E')
		{
			object_prep(i_ptr, o_ptr->kind, p_ptr->depth, RANDOMISE);
			apply_magic(i_ptr, p_ptr->depth, FALSE, TRUE, TRUE);
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

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

		/* Window stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP );
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
		           quality, p_ptr->depth);
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
			make_object(cave, i_ptr, level, good, great);

			/* Allow multiple artifacts, because breaking the game is fine here */
			if (o_ptr->artifact) o_ptr->artifact->created = FALSE;

			/* Test for the same tval and sval. */
			if ((o_ptr->tval) != (i_ptr->tval)) continue;
			if ((o_ptr->sval) != (i_ptr->sval)) continue;

			/* Check pvals */
			ismatch = TRUE;
			for (j = 0; j < MAX_PVALS; j++)
				if (i_ptr->pval[j] != o_ptr->pval[j])
					ismatch = FALSE;

			isbetter = TRUE;
			for (j = 0; j < MAX_PVALS; j++)
				if (i_ptr->pval[j] < o_ptr->pval[j])
					isbetter = FALSE;

			isworse = TRUE;
			for (j = 0; j < MAX_PVALS; j++)
				if (i_ptr->pval[j] > o_ptr->pval[j])
					isworse = FALSE;

			/* Check for match */
			if (ismatch && (i_ptr->to_a == o_ptr->to_a) &&
				(i_ptr->to_h == o_ptr->to_h) &&
				(i_ptr->to_d == o_ptr->to_d) &&
				(i_ptr->num_pvals == o_ptr->num_pvals))
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
			p_ptr->total_weight -= (o_ptr->number * o_ptr->weight);

			/* Add the weight of the new number of objects */
			p_ptr->total_weight += (tmp_int * o_ptr->weight);
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
	if (!get_item(&item, q, s, 0, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

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
			wiz_statistics(i_ptr, p_ptr->depth);
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

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP );
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
	drop_near(cave, i_ptr, 0, p_ptr->py, p_ptr->px, TRUE);

	/* All done */
	msg("Allocated.");
	
	/* Redraw map */
	p_ptr->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(p_ptr);
}


/*
 * Cure everything instantly
 */
static void do_cmd_wiz_cure_all(void)
{
	/* Remove curses */
	(void)remove_all_curse();

	/* Restore stats */
	(void)res_stat(A_STR);
	(void)res_stat(A_INT);
	(void)res_stat(A_WIS);
	(void)res_stat(A_CON);
	(void)res_stat(A_DEX);
	(void)res_stat(A_CHR);

	/* Restore the level */
	(void)restore_level();

	/* Heal the player */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Restore mana */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* Cure stuff */
	(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
	(void)player_clear_timed(p_ptr, TMD_PARALYZED, TRUE);
	(void)player_clear_timed(p_ptr, TMD_IMAGE, TRUE);
	(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
	(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
	(void)player_clear_timed(p_ptr, TMD_SLOW, TRUE);
	(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);

	/* No longer hungry */
	player_set_food(p_ptr, PY_FOOD_MAX - 1);

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
	/* Ask for level */
	if (p_ptr->command_arg <= 0)
	{
		char ppp[80];

		char tmp_val[160];

		/* Prompt */
		strnfmt(ppp, sizeof(ppp), "Jump to level (0-%d): ", MAX_DEPTH-1);

		/* Default */
		strnfmt(tmp_val, sizeof(tmp_val), "%d", p_ptr->depth);

		/* Ask for a level */
		if (!get_string(ppp, tmp_val, 11)) return;

		/* Extract request */
		p_ptr->command_arg = atoi(tmp_val);
	}

	/* Paranoia */
	if (p_ptr->command_arg < 0) p_ptr->command_arg = 0;

	/* Paranoia */
	if (p_ptr->command_arg > MAX_DEPTH - 1) p_ptr->command_arg = MAX_DEPTH - 1;

	/* Accept request */
	msg("You jump to dungeon level %d.", p_ptr->command_arg);

	/* New depth */
	p_ptr->depth = p_ptr->command_arg;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Become aware of a lot of objects
 */
static void do_cmd_wiz_learn(void)
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
		if (k_ptr->level <= p_ptr->command_arg)
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

	min_value = (PY_MAX_LEVEL * 3 * (p_ptr->hitdie - 1)) / 8;
	min_value += PY_MAX_LEVEL;

	max_value = (PY_MAX_LEVEL * 5 * (p_ptr->hitdie - 1)) / 8;
	max_value += PY_MAX_LEVEL;

	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Rerate */
	while (1)
	{
		/* Collect values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			p_ptr->player_hp[i] = randint1(p_ptr->hitdie);
			p_ptr->player_hp[i] += p_ptr->player_hp[i - 1];
		}

		/* Legal values */
		if ((p_ptr->player_hp[PY_MAX_LEVEL - 1] >= min_value) &&
		    (p_ptr->player_hp[PY_MAX_LEVEL - 1] <= max_value)) break;
	}

	percent = (int)(((long)p_ptr->player_hp[PY_MAX_LEVEL - 1] * 200L) /
	                (p_ptr->hitdie + ((PY_MAX_LEVEL - 1) * p_ptr->hitdie)));

	/* Update and redraw hitpoints */
	p_ptr->update |= (PU_HP);
	p_ptr->redraw |= (PR_HP);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Message */
	msg("Current Life Rating is %d/100.", percent);
}


/*
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i;

	for (i = 0; i < num; i++)
	{
		(void)summon_specific(py, px, p_ptr->depth, 0, 1);
	}
}


/*
 * Summon a creature of the specified type
 *
 * This function is rather dangerous XXX XXX XXX
 */
static void do_cmd_wiz_named(int r_idx, bool slp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, x, y;

	/* Paranoia */
	if (!r_idx) return;
	if (r_idx >= z_info->r_max-1) return;

	/* Try 10 times */
	for (i = 0; i < 10; i++) {
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, py, px, d, 0);

		/* Require empty grids */
		if (!cave_empty_bold(y, x)) continue;

		/* Place it (allow groups) */
		if (place_monster_aux(cave, y, x, r_idx, slp, TRUE, ORIGIN_DROP_WIZARD)) break;
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
		if (!m_ptr->r_idx) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > d) continue;

		/* Delete the monster */
		delete_monster_idx(i);
	}

	/* Update monster list window */
	p_ptr->redraw |= PR_MONLIST;
}


/*
 * Un-hide all monsters
 */
static void do_cmd_wiz_unhide(int d)
{
	int i;

	/* Process monsters */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > d) continue;

		/* Detect the monster */
		m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

		/* Update the monster */
		update_mon(i, FALSE);
	}
}


/*
 * Query the dungeon
 */
static void do_cmd_wiz_query(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;

	struct keypress cmd;

	u16b mask = 0x00;


	/* Get a "debug command" */
	if (!get_com("Debug Command Query: ", &cmd)) return;

	/* Extract a flag */
	switch (cmd.code)
	{
		case '0': mask = (1 << 0); break;
		case '1': mask = (1 << 1); break;
		case '2': mask = (1 << 2); break;
		case '3': mask = (1 << 3); break;
		case '4': mask = (1 << 4); break;
		case '5': mask = (1 << 5); break;
		case '6': mask = (1 << 6); break;
		case '7': mask = (1 << 7); break;

		case 'm': mask |= (CAVE_MARK); break;
		case 'g': mask |= (CAVE_GLOW); break;
		case 'r': mask |= (CAVE_ROOM); break;
		case 'i': mask |= (CAVE_ICKY); break;
		case 's': mask |= (CAVE_SEEN); break;
		case 'v': mask |= (CAVE_VIEW); break;
		case 't': mask |= (CAVE_TEMP); break;
		case 'w': mask |= (CAVE_WALL); break;
	}

	/* Scan map */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
	{
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
		{
			byte a = TERM_RED;

			if (!in_bounds_fully(y, x)) continue;

			/* Given mask, show only those grids */
			if (mask && !(cave->info[y][x] & mask)) continue;

			/* Given no mask, show unknown grids */
			if (!mask && (cave->info[y][x] & (CAVE_MARK))) continue;

			/* Color */
			if (cave_floor_bold(y, x)) a = TERM_YELLOW;

			/* Display player/floors/walls */
			if ((y == py) && (x == px))
				print_rel('@', a, y, x);
			else if (cave_floor_bold(y, x))
				print_rel('*', a, y, x);
			else
				print_rel('#', a, y, x);
		}
	}

	/* Get keypress */
	msg("Press any key.");
	message_flush();

	/* Redraw map */
	prt_map();
}

/*
 * Create lots of items.
 */
static void wiz_test_kind(int tval)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	int sval;

	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	for (sval = 0; sval < 255; sval++)
	{
		object_kind *kind = lookup_kind(tval, sval);
		if (!kind) continue;

		/* Create the item */
		object_prep(i_ptr, kind, p_ptr->depth, RANDOMISE);

		/* Apply magic (no messages, no artifacts) */
		apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);

		/* Mark as cheat, and where created */
		i_ptr->origin = ORIGIN_CHEAT;
		i_ptr->origin_depth = p_ptr->depth;

		if (tval == TV_GOLD)
			make_gold(i_ptr, p_ptr->depth, sval);

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
	for (i = 0; i < A_MAX; i++)
		p_ptr->stat_cur[i] = p_ptr->stat_max[i] = 118;

	/* Lots of money */
	p_ptr->au = 1000000L;

	/* Level 50 */
	player_exp_gain(p_ptr, PY_MAX_EXP);

	/* Heal the player */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Restore mana */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* Get some awesome equipment */
	/* Artifacts: 3, 5, 12, ...*/
	
	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN | PR_EQUIP |
	                  PR_MESSAGE | PR_MONSTER | PR_OBJECT |
					  PR_MONLIST | PR_ITEMLIST);

	/* Hack -- update */
	handle_stuff(p_ptr);

}

/*
 * Ask for and parse a "debug command"
 *
 * The "p_ptr->command_arg" may have been set.
 */
void do_cmd_debug(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	struct keypress cmd;


	/* Get a "debug command" */
	if (!get_com("Debug Command: ", &cmd)) return;

	/* Analyze the command */
	switch (cmd.code)
	{
		/* Ignore */
		case ESCAPE:
		case ' ':
		case '\n':
		case '\r':
		{
			break;
		}

#ifdef ALLOW_SPOILERS

		/* Hack -- Generate Spoilers */
		case '"':
		{
			do_cmd_spoilers();
			break;
		}

#endif


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
			if (p_ptr->command_arg > 0)
			{
				wiz_create_artifact(p_ptr->command_arg);
			}
			else
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
					if (a_idx != -1)
						wiz_create_artifact(a_idx);
					else
						msg("No artifact found.");
				}
				
				/* Reload the screen */
				screen_load();
			}
			break;
		}

		/* Detect everything */
		case 'd':
		{
			detect_all(TRUE);
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

		case 'f':
		{
			stats_collect();
			break;
		}

		/* Good Objects */
		case 'g':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = 1;
			acquirement(py, px, p_ptr->depth, p_ptr->command_arg, FALSE);
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

		/* Identify */
		case 'i':
		{
			(void)ident_spell();
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
			do_cmd_wiz_learn();
			break;
		}

		case 'L': do_cmd_keylog(); break;

		/* Magic Mapping */
		case 'm':
		{
			map_area();
			break;
		}

		/* Summon Named Monster */
		case 'n':
		{
			s16b r_idx = 0; 

			if (p_ptr->command_arg > 0)
			{
				r_idx = p_ptr->command_arg;
			}
			else
			{
				char name[80] = "";

				/* Avoid the prompt getting in the way */
				screen_save();

				/* Prompt */
				prt("Summon which monster? ", 0, 0);

				/* Get the name */
				if (askfor_aux(name, sizeof(name), NULL))
				{
					/* See if a r_idx was entered */
					r_idx = get_idx_from_name(name);
					
					/* If not, find the monster with that name */
					if (r_idx < 1)
						r_idx = lookup_monster(name); 
					
					p_ptr->redraw |= (PR_MAP | PR_MONLIST);

					/* Reload the screen */
					screen_load();
				}
			}

			if (r_idx > 0)
				do_cmd_wiz_named(r_idx, TRUE);
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
			teleport_player(10);
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
			s16b r_idx = 0; 

			if (p_ptr->command_arg > 0)
			{
				r_idx = p_ptr->command_arg;
			}
			else
			{
				struct keypress sym;
				const char *prompt =
					"Full recall for [a]ll monsters or [s]pecific monster? ";

				if (!get_com(prompt, &sym)) return;

				if (sym.code == 'a' || sym.code == 'A')
				{
					int i;
					for (i = 0; i < z_info->r_max; i++)
						cheat_monster_lore(i, &l_list[i]);
					break;
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
						
						/* If not, find the monster with that name */
						if (r_idx < 1)
							r_idx = lookup_monster(name); 
					}
					
					/* Reload the screen */
					screen_load();
				}
				else
				{
					/* Assume user aborts */
					break;
				}
			}

			/* Did we find a valid monster? */
			if (r_idx > 0)
				cheat_monster_lore(r_idx, &l_list[r_idx]);
			else
				msg("No monster found.");
	
			break;
		}

		/* Summon Random Monster(s) */
		case 's':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = 1;
			do_cmd_wiz_summon(p_ptr->command_arg);
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
			teleport_player(100);
			break;
		}

		/* Create a trap */
		case 'T':
		{
			if (cave->feat[p_ptr->py][p_ptr->px] != FEAT_FLOOR) 
				msg("You can't place a trap there!");
			else if (p_ptr->depth == 0)
				msg("You can't place a trap in the town!");
			else
				cave_set_feat(cave, p_ptr->py, p_ptr->px, FEAT_INVIS);
			break;
		}

		/* Un-hide all monsters */
		case 'u':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = 255;
			do_cmd_wiz_unhide(p_ptr->command_arg);
			break;
		}

		/* Very Good Objects */
		case 'v':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = 1;
			acquirement(py, px, p_ptr->depth, p_ptr->command_arg, TRUE);
			break;
		}

		case 'V':
		{
			wiz_test_kind(p_ptr->command_arg);
			break;
		}

		/* Wizard Light the Level */
		case 'w':
		{
			wiz_light();
			break;
		}

		/* Wipe recall for a monster */
		case 'W':
		{
			s16b r_idx = 0; 

			if (p_ptr->command_arg > 0)
			{
				r_idx = p_ptr->command_arg;
			}
			else
			{
				struct keypress sym;
				const char *prompt =
					"Wipe recall for [a]ll monsters or [s]pecific monster? ";

				if (!get_com(prompt, &sym)) return;

				if (sym.code == 'a' || sym.code == 'A')
				{
					int i;
					for (i = 0; i < z_info->r_max; i++)
						wipe_monster_lore(i, &l_list[i]);
					break;
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
						
						/* If not, find the monster with that name */
						if (r_idx < 1)
							r_idx = lookup_monster(name); 
					}
					
					/* Reload the screen */
					screen_load();
				}
				else
				{
					/* Assume user aborts */
					break;
				}
			}

			/* Did we find a valid monster? */
			if (r_idx > 0)
				wipe_monster_lore(r_idx, &l_list[r_idx]);
			else
				msg("No monster found.");
	
			break;
		}

		/* Increase Experience */
		case 'x':
		{
			if (p_ptr->command_arg)
			{
				player_exp_gain(p_ptr, p_ptr->command_arg);
			}
			else
			{
				player_exp_gain(p_ptr, p_ptr->exp + 1);
			}
			break;
		}

		/* Zap Monsters (Banishment) */
		case 'z':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = MAX_SIGHT;
			do_cmd_wiz_zap(p_ptr->command_arg);
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

#endif

