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
#include "object/object.h"
#include "ui-menu.h"
#include "spells.h"
#include "target.h"
#include "wizard.h"


#ifdef ALLOW_DEBUG

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
				if (cave_cost[y][x] != i) continue;

				/* Reliability in yellow */
				if (cave_when[y][x] == cave_when[py][px])
				{
					a = TERM_YELLOW;
				}

				/* Display player/floors/walls */
				if ((y == py) && (x == px))
				{
					print_rel('@', a, y, x);
				}
				else if (cave_floor_bold(y, x))
				{
					print_rel('*', a, y, x);
				}
				else
				{
					print_rel('#', a, y, x);
				}
			}
		}

		/* Prompt */
		prt(format("Depth %d: ", i), 0, 0);

		/* Get key */
		if (inkey() == ESCAPE) break;

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

	/* Save */
	p_ptr->exp = tmp_long;

	/* Update */
	check_experience();

	/* Default */
	strnfmt(tmp_val, sizeof(tmp_val), "%ld", (long)(p_ptr->max_exp));

	/* Query */
	if (!get_string("Max Exp: ", tmp_val, 10)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->max_exp = tmp_long;

	/* Update */
	check_experience();
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
	           o_ptr->k_idx, o_ptr->tval, o_ptr->sval, o_ptr->weight, o_ptr->timeout), 5, j);

	prt(format("number = %-3d  pval = %-5d  name1 = %-4d  name2 = %-4d  cost = %ld",
	           o_ptr->number, o_ptr->pval, o_ptr->name1, o_ptr->name2, (long)object_value(o_ptr, 1, FALSE)), 6, j);

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
	prt("we  lerlf   ilgannnn  ltssdo  ym", 11, j+34);
	prt("da reiedu   merirrrr  eityew ccc", 12, j+34);
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
	int *choices = menu_priv(m);
	char buf[80];

	object_kind_name(buf, sizeof buf, choices[oid], TRUE);
	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

bool wiz_create_item_subaction(menu_type *m, const ui_event_data *e, int oid)
{
	int *choices = menu_priv(m);

	object_kind *kind = &k_info[choices[oid]];

	object_type *i_ptr;
	object_type object_type_body;

	if (e->type != EVT_SELECT)
		return TRUE;


	/* Get local object */
	i_ptr = &object_type_body;

	/* Create the item */
	object_prep(i_ptr, &k_info[choices[oid]], p_ptr->depth, RANDOMISE);

	/* Apply magic (no messages, no artifacts) */
	apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);

	/* Mark as cheat, and where created */
	i_ptr->origin = ORIGIN_CHEAT;
	i_ptr->origin_depth = p_ptr->depth;

	if (kind->tval == TV_GOLD)
		make_gold(i_ptr, p_ptr->depth, kind->sval);

	/* Drop the object from heaven */
	drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, TRUE);

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

/*
 * A structure to hold a tval and its description
 */
struct tval_desc
{
	int tval;
	const char *desc;
};

/*
 * A list of tvals and their textual names
 */
static struct tval_desc tvals[] =
{
	{ TV_SWORD,             "Sword"                },
	{ TV_POLEARM,           "Polearm"              },
	{ TV_HAFTED,            "Hafted Weapon"        },
	{ TV_BOW,               "Bow"                  },
	{ TV_ARROW,             "Arrows"               },
	{ TV_BOLT,              "Bolts"                },
	{ TV_SHOT,              "Shots"                },
	{ TV_SHIELD,            "Shield"               },
	{ TV_CROWN,             "Crown"                },
	{ TV_HELM,              "Helm"                 },
	{ TV_GLOVES,            "Gloves"               },
	{ TV_BOOTS,             "Boots"                },
	{ TV_CLOAK,             "Cloak"                },
	{ TV_DRAG_ARMOR,        "Dragon Scale Mail"    },
	{ TV_HARD_ARMOR,        "Hard Armor"           },
	{ TV_SOFT_ARMOR,        "Soft Armor"           },
	{ TV_RING,              "Ring"                 },
	{ TV_AMULET,            "Amulet"               },
	{ TV_LIGHT,             "Light"                },
	{ TV_POTION,            "Potion"               },
	{ TV_SCROLL,            "Scroll"               },
	{ TV_WAND,              "Wand"                 },
	{ TV_STAFF,             "Staff"                },
	{ TV_ROD,               "Rod"                  },
	{ TV_PRAYER_BOOK,       "Priest Book"          },
	{ TV_MAGIC_BOOK,        "Magic Book"           },
	{ TV_SPIKE,             "Spikes"               },
	{ TV_DIGGING,           "Digger"               },
	{ TV_CHEST,             "Chest"                },
	{ TV_FOOD,              "Food"                 },
	{ TV_FLASK,             "Flask"                },
	{ TV_SKELETON,          "Skeletons"            },
	{ TV_BOTTLE,            "Empty bottle"         },
	{ TV_JUNK,              "Junk"                 },
	{ TV_GOLD,              "Gold"                 }
};

void wiz_create_item_display(menu_type *m, int oid, bool cursor,
		int row, int col, int width)
{
	struct tval_desc *tvals = menu_priv(m);
	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], tvals[oid].desc, row, col);
}

bool wiz_create_item_action(menu_type *m, const ui_event_data *e, int oid)
{
	ui_event_data ret;
	menu_type *menu;

	int choice[60];
	int n_choices;

	int i;

	if (e->type != EVT_SELECT)
		return TRUE;

	for (n_choices = 0, i = 1; (n_choices < 60) && (i < z_info->k_max); i++)
	{
		object_kind *kind = &k_info[i];

		if (kind->tval != tvals[oid].tval ||
				of_has(kind->flags, OF_INSTA_ART))
			continue;

		choice[n_choices++] = i;
	}

	screen_save();
	clear_from(0);

	menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_submenu);
	menu->selections = all_letters;
	menu->title = format("What kind of %s?", tvals[oid].desc);

	menu_setpriv(menu, n_choices, choice);
	menu_layout(menu, &wiz_create_item_area);
	ret = menu_select(menu, 0);

	screen_load();

	return (ret.type == EVT_ESCAPE);
}

menu_iter wiz_create_item_menu =
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
	menu_type *menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_menu);

	menu->selections = all_letters;
	menu->title = "What kind of object?";

	screen_save();
	clear_from(0);

	menu_setpriv(menu, N_ELEMENTS(tvals), tvals);
	menu_layout(menu, &wiz_create_item_area);
	menu_select(menu, 0);

	screen_load();
}



/*
 * Tweak an item
 */
static void wiz_tweak_item(object_type *o_ptr)
{
	cptr p;
	char tmp_val[80];


	/* Hack -- leave artifacts alone */
	if (artifact_p(o_ptr)) return;

#define WIZ_TWEAK(attribute) do {\
	p = "Enter new '" #attribute "' setting: ";\
	strnfmt(tmp_val, sizeof(tmp_val), "%d", o_ptr->attribute);\
	if (!get_string(p, tmp_val, 6)) return;\
	o_ptr->attribute = atoi(tmp_val);\
	wiz_display_item(o_ptr, TRUE);\
} while (0)

	WIZ_TWEAK(pval);
	WIZ_TWEAK(to_a);
	WIZ_TWEAK(to_h);
	WIZ_TWEAK(to_d);
	WIZ_TWEAK(name1);
	WIZ_TWEAK(name2);
}


/*
 * Apply magic to an item or turn it into an artifact. -Bernd-
 */
static void wiz_reroll_item(object_type *o_ptr)
{
	object_type *i_ptr;
	object_type object_type_body;

	char ch;

	bool changed = FALSE;


	/* Hack -- leave artifacts alone */
	if (artifact_p(o_ptr)) return;


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
		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}

		/* Apply normal magic, but first clear object */
		else if (ch == 'n' || ch == 'N')
		{
			object_prep(i_ptr, o_ptr->kind, p_ptr->depth, RANDOMISE);
			apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);
		}

		/* Apply good magic, but first clear object */
		else if (ch == 'g' || ch == 'g')
		{
			object_prep(i_ptr, o_ptr->kind, p_ptr->depth, RANDOMISE);
			apply_magic(i_ptr, p_ptr->depth, FALSE, TRUE, FALSE);
		}

		/* Apply great magic, but first clear object */
		else if (ch == 'e' || ch == 'e')
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

	char ch;
	cptr quality;

	bool good, great;

	object_type *i_ptr;
	object_type object_type_body;

	cptr q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";


	artifact_type *a_ptr = artifact_of(o_ptr);

	/* Allow multiple artifacts, because breaking the game is fine here */
	if (a_ptr) a_ptr->created = FALSE;


	/* Interact */
	while (TRUE)
	{
		cptr pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";

		/* Display item */
		wiz_display_item(o_ptr, TRUE);

		/* Get choices */
		if (!get_com(pmt, &ch)) break;

		if (ch == 'n' || ch == 'N')
		{
			good = FALSE;
			great = FALSE;
			quality = "normal";
		}
		else if (ch == 'g' || ch == 'G')
		{
			good = TRUE;
			great = FALSE;
			quality = "good";
		}
		else if (ch == 'e' || ch == 'E')
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
		msg_format("Creating a lot of %s items. Base level = %d.",
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
				/* Do not wait */
				inkey_scan = SCAN_INSTANT;

				/* Allow interupt */
				if (inkey())
				{
					/* Flush */
					flush();

					/* Stop rolling */
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
			make_object(i_ptr, level, good, great);

			/* Allow multiple artifacts, because breaking the game is fine here */
			a_ptr = artifact_of(o_ptr);
			if (a_ptr) a_ptr->created = FALSE;

			/* Test for the same tval and sval. */
			if ((o_ptr->tval) != (i_ptr->tval)) continue;
			if ((o_ptr->sval) != (i_ptr->sval)) continue;

			/* Check for match */
			if ((i_ptr->pval == o_ptr->pval) &&
			    (i_ptr->to_a == o_ptr->to_a) &&
			    (i_ptr->to_h == o_ptr->to_h) &&
			    (i_ptr->to_d == o_ptr->to_d))
			{
				matches++;
			}

			/* Check for better */
			else if ((i_ptr->pval >= o_ptr->pval) &&
			         (i_ptr->to_a >= o_ptr->to_a) &&
			         (i_ptr->to_h >= o_ptr->to_h) &&
			         (i_ptr->to_d >= o_ptr->to_d))
			{
				better++;
			}

			/* Check for worse */
			else if ((i_ptr->pval <= o_ptr->pval) &&
			         (i_ptr->to_a <= o_ptr->to_a) &&
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
		msg_format(q, i, matches, better, worse, other);
		message_flush();
	}


	/* Hack -- Normally only make a single artifact */
	if (artifact_p(o_ptr)) a_info[o_ptr->name1].created = TRUE;
}


/*
 * Change the quantity of a the item
 */
static void wiz_quantity_item(object_type *o_ptr, bool carried)
{
	int tmp_int;

	char tmp_val[3];

	/* Never duplicate artifacts */
	if (artifact_p(o_ptr)) return;

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
	if (cursed_p(o_ptr))
	{
		msg_print("Resetting existing curses.");
		flags_clear(o_ptr->flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
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

	char ch;

	cptr q, s;

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

		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}
		else if (ch == 'c' || ch == 'C')
			wiz_tweak_curse(i_ptr);
		else if (ch == 's' || ch == 'S')
			wiz_statistics(i_ptr, p_ptr->depth);
		else if (ch == 'r' || ch == 'r')
			wiz_reroll_item(i_ptr);
		else if (ch == 't' || ch == 'T')
			wiz_tweak_item(i_ptr);
		else if (ch == 'k' || ch == 'K')
			all = !all;
		else if (ch == 'q' || ch == 'Q')
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
		msg_print("Changes accepted.");

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
		msg_print("Changes ignored.");
	}
}


/*
 * Create the artifact with the specified number
 */
static void wiz_create_artifact(int a_idx)
{
	object_type *i_ptr;
	object_type object_type_body;
	int k_idx;

	artifact_type *a_ptr = &a_info[a_idx];

	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Wipe the object */
	object_wipe(i_ptr);

	/* Acquire the "kind" index */
	k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!k_idx) return;

	/* Create the artifact */
	object_prep(i_ptr, &k_info[k_idx], a_ptr->alloc_min, RANDOMISE);

	/* Save the name */
	i_ptr->name1 = a_idx;

	/* Extract the fields */
	i_ptr->pval = a_ptr->pval;
	i_ptr->ac = a_ptr->ac;
	i_ptr->dd = a_ptr->dd;
	i_ptr->ds = a_ptr->ds;
	i_ptr->to_a = a_ptr->to_a;
	i_ptr->to_h = a_ptr->to_h;
	i_ptr->to_d = a_ptr->to_d;
	i_ptr->weight = a_ptr->weight;

	/* Hack -- extract the "cursed" flags */
	if (cursed_p(a_ptr))
	{
		bitflag curse_flags[OF_SIZE];
		of_copy(curse_flags, a_ptr->flags);
		flags_mask(curse_flags, OF_SIZE, OF_CURSE_MASK, FLAG_END);
		of_union(i_ptr->flags, curse_flags);
	}

	/* Mark that the artifact has been created. */
	a_ptr->created = TRUE;

	/* Mark as cheat */
	i_ptr->origin = ORIGIN_CHEAT;

	/* Drop the artifact from heaven */
	drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, TRUE);

	/* All done */
	msg_print("Allocated.");
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
	(void)clear_timed(TMD_BLIND, TRUE);
	(void)clear_timed(TMD_CONFUSED, TRUE);
	(void)clear_timed(TMD_POISONED, TRUE);
	(void)clear_timed(TMD_AFRAID, TRUE);
	(void)clear_timed(TMD_PARALYZED, TRUE);
	(void)clear_timed(TMD_IMAGE, TRUE);
	(void)clear_timed(TMD_STUN, TRUE);
	(void)clear_timed(TMD_CUT, TRUE);
	(void)clear_timed(TMD_SLOW, TRUE);
	(void)clear_timed(TMD_AMNESIA, TRUE);

	/* No longer hungry */
	(void)set_food(PY_FOOD_MAX - 1);

	/* Redraw everything */
	do_cmd_redraw();
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
	msg_format("You jump to dungeon level %d.", p_ptr->command_arg);

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
	handle_stuff();

	/* Message */
	msg_format("Current Life Rating is %d/100.", percent);
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
	for (i = 0; i < 10; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, py, px, d, 0);

		/* Require empty grids */
		if (!cave_empty_bold(y, x)) continue;

		/* Place it (allow groups) */
		if (place_monster_aux(cave, y, x, r_idx, slp, TRUE)) break;
	}
}



/*
 * Hack -- Delete all nearby monsters
 */
static void do_cmd_wiz_zap(int d)
{
	int i;

	/* Banish everyone nearby */
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr = &mon_list[i];

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
	for (i = 1; i < mon_max; i++)
	{
		monster_type *m_ptr = &mon_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > d) continue;

		/* Optimize -- Repair flags */
		repair_mflag_mark = repair_mflag_show = TRUE;

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

	char cmd;

	u16b mask = 0x00;


	/* Get a "debug command" */
	if (!get_com("Debug Command Query: ", &cmd)) return;

	/* Extract a flag */
	switch (cmd)
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
			if (mask && !(cave_info[y][x] & mask)) continue;

			/* Given no mask, show unknown grids */
			if (!mask && (cave_info[y][x] & (CAVE_MARK))) continue;

			/* Color */
			if (cave_floor_bold(y, x)) a = TERM_YELLOW;

			/* Display player/floors/walls */
			if ((y == py) && (x == px))
			{
				print_rel('@', a, y, x);
			}
			else if (cave_floor_bold(y, x))
			{
				print_rel('*', a, y, x);
			}
			else
			{
				print_rel('#', a, y, x);
			}
		}
	}

	/* Get keypress */
	msg_print("Press any key.");
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
		int k_idx = lookup_kind(tval, sval);

		if (k_idx)
		{
			/* Create the item */
			object_prep(i_ptr, &k_info[k_idx], p_ptr->depth, RANDOMISE);

			/* Apply magic (no messages, no artifacts) */
			apply_magic(i_ptr, p_ptr->depth, FALSE, FALSE, FALSE);

			/* Mark as cheat, and where created */
			i_ptr->origin = ORIGIN_CHEAT;
			i_ptr->origin_depth = p_ptr->depth;

			if (k_info[k_idx].tval == TV_GOLD)
				make_gold(i_ptr, p_ptr->depth, sval);

			/* Drop the object from heaven */
			drop_near(i_ptr, 0, py, px, TRUE);
		}
	}

	msg_print("Done.");
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
 * Ask for and parse a "debug command"
 *
 * The "p_ptr->command_arg" may have been set.
 */
void do_cmd_debug(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	char cmd;


	/* Get a "debug command" */
	if (!get_com("Debug Command: ", &cmd)) return;

	/* Analyze the command */
	switch (cmd)
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

		/* Magic Mapping */
		case 'm':
		{
			map_area();
			break;
		}

		/* Summon Named Monster */
		case 'n':
		{
			if (p_ptr->command_arg > 0)
			{
				do_cmd_wiz_named(p_ptr->command_arg, TRUE);
			}
			else
			{
				char name[80] = "";
				s16b r_idx;

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
					
					/* Did we find a valid monster? */
					if (r_idx != -1)
						do_cmd_wiz_named(r_idx, TRUE);
				}

				p_ptr->redraw |= (PR_MAP | PR_MONLIST);

				/* Reload the screen */
				screen_load();
			}

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

		/* Query the dungeon */
		case 'q':
		{
			do_cmd_wiz_query();
			break;
		}

		/* Summon Random Monster(s) */
		case 's':
		{
			if (p_ptr->command_arg <= 0) p_ptr->command_arg = 1;
			do_cmd_wiz_summon(p_ptr->command_arg);
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

		/* Increase Experience */
		case 'x':
		{
			if (p_ptr->command_arg)
			{
				gain_exp(p_ptr->command_arg);
			}
			else
			{
				gain_exp(p_ptr->exp + 1);
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
			msg_print("That is not a valid debug command.");
			break;
		}
	}
}

#endif


