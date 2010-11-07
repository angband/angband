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
#include "object/tvalsval.h"



/*
 * Squelch flags
 */
#define SQUELCH_IF_AWARE	0x01
#define SQUELCH_IF_UNAWARE	0x02


/*
 * List of kinds of item, for pseudo-id squelch.
 */
typedef enum
{
	TYPE_WEAPON_POINTY,
	TYPE_WEAPON_BLUNT,
	TYPE_SHOOTER,
	TYPE_MISSILE_SLING,
	TYPE_MISSILE_BOW,
	TYPE_MISSILE_XBOW,
	TYPE_ARMOR_ROBE,
	TYPE_ARMOR_BODY,
	TYPE_ARMOR_CLOAK,
	TYPE_ARMOR_ELVEN_CLOAK,
	TYPE_ARMOR_SHIELD,
	TYPE_ARMOR_HEAD,
	TYPE_ARMOR_HANDS,
	TYPE_ARMOR_FEET,
	TYPE_DIGGER,
	TYPE_RING,
	TYPE_AMULET,
	TYPE_LIGHT,

	TYPE_MAX
} squelch_type_t;

typedef struct
{
	squelch_type_t squelch_type;
	int tval;
	int min_sval;
	int max_sval;
} quality_squelch_struct;

static quality_squelch_struct quality_mapping[] =
{
	{ TYPE_WEAPON_POINTY,	TV_SWORD,	0,		SV_UNKNOWN },
	{ TYPE_WEAPON_POINTY,	TV_POLEARM,	0,		SV_UNKNOWN },
	{ TYPE_WEAPON_BLUNT,	TV_HAFTED,	0,		SV_UNKNOWN },
	{ TYPE_SHOOTER,		TV_BOW,		0,		SV_UNKNOWN },
	{ TYPE_MISSILE_SLING,	TV_SHOT,	0,		SV_UNKNOWN },
	{ TYPE_MISSILE_BOW,	TV_ARROW,	0,		SV_UNKNOWN },
	{ TYPE_MISSILE_XBOW,	TV_BOLT,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_ROBE,	TV_SOFT_ARMOR,	SV_ROBE,	SV_ROBE },
	{ TYPE_ARMOR_BODY,	TV_DRAG_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,	TV_HARD_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_BODY,	TV_SOFT_ARMOR,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_CLOAK,	TV_CLOAK,	SV_CLOAK, 	SV_FUR_CLOAK },
	{ TYPE_ARMOR_CLOAK,	TV_CLOAK,	SV_ETHEREAL_CLOAK, 	SV_ETHEREAL_CLOAK },
/* XXX Eddie need to assert SV_CLOAK < SV_FUR_CLOAK < SV_ELVEN_CLOAK */
	{ TYPE_ARMOR_ELVEN_CLOAK,	TV_CLOAK,	SV_ELVEN_CLOAK, 	SV_ELVEN_CLOAK },
	{ TYPE_ARMOR_SHIELD,	TV_SHIELD,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,	TV_HELM,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HEAD,	TV_CROWN,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_HANDS,	TV_GLOVES,	0,		SV_UNKNOWN },
	{ TYPE_ARMOR_FEET,	TV_BOOTS,	0,		SV_UNKNOWN },
	{ TYPE_DIGGER,		TV_DIGGING,	0,		SV_UNKNOWN },
	{ TYPE_RING,		TV_RING,	0,		SV_UNKNOWN },
	{ TYPE_AMULET,		TV_AMULET,	0,		SV_UNKNOWN },
	{ TYPE_LIGHT, 		TV_LIGHT, 	0,		SV_UNKNOWN },
};


typedef struct
{
	int enum_val;
	const char *name;
} quality_name_struct;

static quality_name_struct quality_choices[TYPE_MAX] =
{
	{ TYPE_WEAPON_POINTY,	"Pointy Melee Weapons" },
	{ TYPE_WEAPON_BLUNT,	"Blunt Melee Weapons" },
	{ TYPE_SHOOTER,		"Missile weapons" },
	{ TYPE_MISSILE_SLING,	"Shots and Pebbles" },
	{ TYPE_MISSILE_BOW,	"Arrows" },
	{ TYPE_MISSILE_XBOW,	"Bolts" },
	{ TYPE_ARMOR_ROBE,	"Robes" },
	{ TYPE_ARMOR_BODY,	"Body Armor" },
	{ TYPE_ARMOR_CLOAK,	"Cloaks" },
	{ TYPE_ARMOR_ELVEN_CLOAK,	"Elven Cloaks" },
	{ TYPE_ARMOR_SHIELD,	"Shields" },
	{ TYPE_ARMOR_HEAD,	"Headgear" },
	{ TYPE_ARMOR_HANDS,	"Handgear" },
	{ TYPE_ARMOR_FEET,	"Footgear" },
	{ TYPE_DIGGER,		"Diggers" },
	{ TYPE_RING,		"Rings" },
	{ TYPE_AMULET,		"Amulets" },
	{ TYPE_LIGHT, 		"Lights" },
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
	{ TV_LIGHT,			"Lights" },
	{ TV_FLASK,			"Flasks of oil" },
/*	{ TV_DRAG_ARMOR,	"Dragon Mail Armor" }, */
	{ TV_GOLD,			"Money" },
};

byte squelch_level[TYPE_MAX];
const size_t squelch_size = TYPE_MAX;


/*
 * The different kinds of quality squelch
 */
enum
{
	SQUELCH_NONE,
	SQUELCH_BAD,
	SQUELCH_AVERAGE,
	SQUELCH_GOOD,
	SQUELCH_EXCELLENT_NO_HI,
	SQUELCH_EXCELLENT_NO_SPL,
	SQUELCH_ALL,

	SQUELCH_MAX
};

/*
 * The names for the various kinds of quality
 */
static quality_name_struct quality_values[SQUELCH_MAX] =
{
	{ SQUELCH_NONE,		"none" },
	{ SQUELCH_BAD,		"bad" },
	{ SQUELCH_AVERAGE,	"average" },
	{ SQUELCH_GOOD,		"good" },
	{ SQUELCH_EXCELLENT_NO_HI,	"excellent with no high resists" },
	{ SQUELCH_EXCELLENT_NO_SPL,	"excellent but not splendid" },
	{ SQUELCH_ALL,		"everything except artifacts" },
};

/*
 * menu struct for differentiating aware from unaware squelch
 */
typedef struct
{
	s16b idx;
	bool aware;
} squelch_choice;

/*
 * Ordering function for squelch choices.
 * Aware comes before unaware, and then sort alphabetically.
 */
static int cmp_squelch(const void *a, const void *b)
{
	char bufa[80];
	char bufb[80];
	const squelch_choice *x = (squelch_choice *)a;
	const squelch_choice *y = (squelch_choice *)b;

	if (x->aware && !y->aware)
		return TRUE;
	if (!x->aware && y->aware)
		return FALSE;

	object_kind_name(bufa, sizeof(bufa), x->idx, x->aware);
	object_kind_name(bufb, sizeof(bufb), y->idx, y->aware);

	/* the = is crucial, inf loop in sort if use < rather than <= */
	return strcmp(bufa, bufb) <= 0;
}


/*
 * Initialise the squelch package (currently just asserts).
 */
void squelch_init(void)
{
	int i;

	for (i = 0; i < TYPE_MAX; i++)
		assert(quality_choices[i].enum_val == i);
	for (i = 0; i < SQUELCH_MAX; i++)
		assert(quality_values[i].enum_val == i);
}


/*
 * Reset the player's squelch choices for a new game.
 */
void squelch_birth_init(void)
{
	int i;

	/* Reset squelch bits */
	for (i = 0; i < z_info->k_max; i++)
		k_info[i].squelch = FALSE;

	/* Clear the squelch bytes */
	for (i = 0; i < TYPE_MAX; i++)
		squelch_level[i] = 0;
}



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
	if (!note || !object_flavor_is_aware(o_ptr))
		return 0;

	/* Don't re-inscribe if it's already inscribed */
	if (existing_inscription)
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

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
	for (i = INVEN_PACK; i >= 0; i--)
	{
		/* Skip empty items */
		if (!p_ptr->inventory[i].k_idx) continue;

		/* Apply the inscription */
		apply_autoinscription(&p_ptr->inventory[i]);
	}

	return;
}




/*** Squelch code ***/

/*
 * Determines whether a tval is eligible for sval-squelch.
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
 * Squelch the flavor of an object
 */
static void object_squelch_flavor_of(const object_type *o_ptr)
{
	assert(squelch_tval(o_ptr->tval));
	if (object_flavor_is_aware(o_ptr))
		k_info[o_ptr->k_idx].squelch |= SQUELCH_IF_AWARE;
	else
		k_info[o_ptr->k_idx].squelch |= SQUELCH_IF_UNAWARE;
}


/*
 * Find the squelch type of the object, or TYPE_MAX if none
 */
static squelch_type_t squelch_type_of(const object_type *o_ptr)
{
	size_t i;

	/* Find the appropriate squelch group */
	for (i = 0; i < N_ELEMENTS(quality_mapping); i++)
	{
		if ((quality_mapping[i].tval == o_ptr->tval) &&
			(quality_mapping[i].min_sval <= o_ptr->sval) &&
			(quality_mapping[i].max_sval >= o_ptr->sval))
			return quality_mapping[i].squelch_type;
	}

	return TYPE_MAX;
}


/*
 * Determine the squelch level of an object, which is similar to its pseudo.
 *
 * The main point is when the value is undetermined given current info,
 * return the maximum possible value.
 */
static byte squelch_level_of(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	byte value;
	bitflag f[OF_SIZE];

	object_flags_known(o_ptr, f);

	if ((object_pval_is_visible(o_ptr)) && (o_ptr->pval < 0))
		return SQUELCH_BAD;

	/* Deal with jewelry specially. */
	if (object_is_jewelry(o_ptr))
	{
		if ((object_pval_is_visible(o_ptr)) && (o_ptr->pval > 0))
			return SQUELCH_AVERAGE;
		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_AVERAGE;
		if ((o_ptr->to_h < 0) || (o_ptr->to_d < 0) || (o_ptr->to_a < 0))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
	}

	/* And lights */
	if (o_ptr->tval == TV_LIGHT)
	{
		if (flags_test(f, OF_SIZE, OF_OBVIOUS_MASK, FLAG_END))
			return SQUELCH_ALL;
		if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
			return SQUELCH_GOOD;
		if ((o_ptr->to_h < 0) || (o_ptr->to_d < 0) || (o_ptr->to_a < 0))
			return SQUELCH_BAD;

		return SQUELCH_AVERAGE;
	}

	if (object_was_sensed(o_ptr))
	{
		obj_pseudo_t pseudo = object_pseudo(o_ptr);

		switch (pseudo)
		{
			case INSCRIP_AVERAGE:
				value = SQUELCH_AVERAGE;
				break;

			case INSCRIP_EXCELLENT:
				/* have to assume splendid until you have tested it */
				if (object_was_worn(o_ptr))
				{
					if (object_high_resist_is_possible(o_ptr))
						value = SQUELCH_EXCELLENT_NO_SPL;
					else
						value = SQUELCH_EXCELLENT_NO_HI;
				}
				else
				{
					value = SQUELCH_ALL;
				}
				break;

			case INSCRIP_STRANGE: /* XXX Eddie perhaps some strange count as something else */
			case INSCRIP_SPLENDID:
				value = SQUELCH_ALL;
				break;
			case INSCRIP_NULL:
			case INSCRIP_SPECIAL:
				value = SQUELCH_MAX;
				break;

			/* This is the interesting case */
			case INSCRIP_MAGICAL:
				value = SQUELCH_GOOD;
				if ((object_attack_plusses_are_visible(o_ptr) || (randcalc_valid(k_ptr->to_h, o_ptr->to_h) && randcalc_valid(k_ptr->to_d, o_ptr->to_d))) &&
				    (object_defence_plusses_are_visible(o_ptr) || (randcalc_valid(k_ptr->to_a, o_ptr->to_a))) &&
				    (o_ptr->to_h <= randcalc(k_ptr->to_h, 0, MINIMISE)) &&
				    (o_ptr->to_d <= randcalc(k_ptr->to_d, 0, MINIMISE)) &&
				    (o_ptr->to_a <= randcalc(k_ptr->to_a, 0, MINIMISE)))
					value = SQUELCH_BAD;
				break;


			default:
				/* do not handle any other possible pseudo values */
				assert(0);
		}
	}
	else
	{
		if (object_was_worn(o_ptr))
			value = SQUELCH_EXCELLENT_NO_SPL; /* object would be sensed if it were splendid */
		else if (object_is_known_not_artifact(o_ptr))
			value = SQUELCH_ALL;
		else
			value = SQUELCH_MAX;
	}

	return value;
}

/*
 * Remove any squelching of a particular flavor
 */
void kind_squelch_clear(object_kind *k_ptr)
{
	k_ptr->squelch = 0;
	p_ptr->notice |= PN_SQUELCH;
}

bool kind_is_squelched_aware(const object_kind *k_ptr)
{
	return (k_ptr->squelch & SQUELCH_IF_AWARE) ? TRUE : FALSE;
}

bool kind_is_squelched_unaware(const object_kind *k_ptr)
{
	return (k_ptr->squelch & SQUELCH_IF_UNAWARE) ? TRUE : FALSE;
}

void kind_squelch_when_aware(object_kind *k_ptr)
{
	k_ptr->squelch |= SQUELCH_IF_AWARE;
	p_ptr->notice |= PN_SQUELCH;
}

void kind_squelch_when_unaware(object_kind *k_ptr)
{
	k_ptr->squelch |= SQUELCH_IF_UNAWARE;
	p_ptr->notice |= PN_SQUELCH;
}



/*
 * Determines if an object is eligible for squelching.
 */
bool squelch_item_ok(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	byte type;

	/* Don't squelch artifacts unless marked to be squelched */
	if (artifact_p(o_ptr))
		return FALSE;

	/* Don't squelch stuff inscribed not to be destroyed (!k) */
	if (check_for_inscrip(o_ptr, "!k") || check_for_inscrip(o_ptr, "!*"))
		return FALSE;

	/* Auto-squelch dead chests */
	if (o_ptr->tval == TV_CHEST && o_ptr->pval == 0)
		return TRUE;

	/* check option for worthless kinds */
	if (OPT(squelch_worthless) && o_ptr->tval != TV_GOLD)
	{
		if (object_flavor_is_aware(o_ptr) && k_ptr->cost == 0)
			return TRUE;
		if (object_is_known_cursed(o_ptr))
			return TRUE;
	}

	/* Do squelching by kind */
	if (object_flavor_is_aware(o_ptr) ?
		 kind_is_squelched_aware(k_ptr) :
		 kind_is_squelched_unaware(k_ptr))
		return TRUE;

	type = squelch_type_of(o_ptr);
	if (type == TYPE_MAX)
		return FALSE;

	/* Squelch items known not to be special */
	if (object_is_known_not_artifact(o_ptr) && squelch_level[type] == SQUELCH_ALL)
		return TRUE;

	/* Get result based on the feeling and the squelch_level */
	if (squelch_level_of(o_ptr) <= squelch_level[type])
		return TRUE;
	else
		return FALSE;
}


/*
 * Returns TRUE if an item should be hidden due to the player's
 * current settings.
 */
bool squelch_hide_item(object_type *o_ptr)
{
	return (OPT(hide_squelchable) ? squelch_item_ok(o_ptr) : FALSE);
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
		o_ptr = &p_ptr->inventory[n];

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
		p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);
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
		object_type *o_ptr = &p_ptr->inventory[n];

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
	const char *name = quality_choices[oid].name;

	byte level = squelch_level[oid];
	const char *level_name = quality_values[level].name;

	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);


	c_put_str(attr, format("%-20s : %s", name, level_name), row, col);
}


/*
 * Display the quality squelch subtypes.
 */
static void quality_subdisplay(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	const char *name = quality_values[oid].name;
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);

	c_put_str(attr, name, row, col);
}


/*
 * Handle keypresses.
 */
static bool quality_action(menu_type *m, const ui_event_data *event, int oid)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, quality_subdisplay, NULL, NULL };
	region area = { 24, 5, 29, SQUELCH_MAX };
	ui_event_data evt;
	int cursor;
	int count;

	/* Display at the right point */
	area.row += oid;
	cursor = squelch_level[oid];

	/* Save */
	screen_save();

	/* Work out how many options we have */
	count = SQUELCH_MAX;
	if ((oid == TYPE_RING) || (oid == TYPE_AMULET))
		count = area.page_rows = SQUELCH_BAD + 1;

	/* Run menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, count, quality_values);

	/* Stop menus from going off the bottom of the screen */
	if (area.row + menu.count > Term->hgt - 1)
		area.row += Term->hgt - 1 - area.row - menu.count;

	menu_layout(&menu, &area);

	window_make(area.col - 2, area.row - 1, area.col + area.width + 2, area.row + area.page_rows);

	evt = menu_select(&menu, 0);

	/* Set the new value appropriately */
	if (evt.type == EVT_SELECT)
		squelch_level[oid] = menu.cursor;

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
	menu_iter menu_f = { NULL, NULL, quality_display, quality_action, NULL };
	region area = { 1, 5, -1, -1 };

	/* Save screen */
	screen_save();
	clear_from(0);

	/* Help text */
	prt("Quality squelch menu", 0, 0);

	Term_gotoxy(1, 1);
	text_out_to_screen(TERM_L_RED, "Use the movement keys to navigate, and Enter to change settings.");

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, TYPE_MAX, quality_values);
	menu_layout(&menu, &area);

	/* Select an entry */
	menu_select(&menu, 0);

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
	const squelch_choice *choice = menu_priv(menu);
	int idx = choice[oid].idx;

	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);


	/* Acquire the "name" of object "i" */
	object_kind_name(buf, sizeof(buf), idx, choice[oid].aware);

	/* Print it */
	c_put_str(attr, format("[ ] %s", buf), row, col);
	if ((choice[oid].aware && (k_info[idx].squelch & SQUELCH_IF_AWARE)) ||
	    ((!choice[oid].aware) && (k_info[idx].squelch & SQUELCH_IF_UNAWARE)))
		c_put_str(TERM_L_RED, "*", row, col + 1);
}

/*
 * Deal with events on the sval menu
 */
static bool sval_action(menu_type *m, const ui_event_data *event, int oid)
{
	const squelch_choice *choice = menu_priv(m);

	if (event->type == EVT_SELECT)
	{
		int idx = choice[oid].idx;

		/* Toggle the appropriate flag */
		if (choice[oid].aware)
			k_info[idx].squelch ^= SQUELCH_IF_AWARE;
		else
			k_info[idx].squelch ^= SQUELCH_IF_UNAWARE;

		p_ptr->notice |= PN_SQUELCH;
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
	menu_iter menu_f = { NULL, NULL, sval_display, sval_action, NULL };
	region area = { 1, 5, -1, -1 };

	int num = 0;
	size_t i;

	squelch_choice *choice;


	/* Create the array, with entries both for aware and unaware squelch */
	choice = C_ZNEW(2 * z_info->k_max, squelch_choice);

	/* Iterate over all possible object kinds, finding ones which can be squelched */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!k_ptr->name) continue;
		if (k_ptr->tval != tval) continue;

		if (!k_ptr->aware)
		{
			/* can unaware squelch anything */
			/* XXX Eddie should it be required that unaware squelched flavors have been seen this game, if so, how to save that info? */

			choice[num].idx = i;
			choice[num].aware = FALSE;
			num++;
		}

		if (k_ptr->everseen || k_ptr->tval == TV_GOLD)
		{
			/* aware squelch requires everseen */
			/* do not require awareness for aware squelch, so people can set at game start */

			choice[num].idx = i;
			choice[num].aware = TRUE;
			num++;
		}
	}

	/* Return here if there are no objects */
	if (!num)
	{
		FREE(choice);
		return FALSE;
	}

	/* sort by name in squelch menus except for categories of items that are aware from the start */
	switch(tval)
	{
		case TV_LIGHT:
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_DRAG_ARMOR:
		case TV_GOLD:
			/* leave sorted by sval */
			break;

		default:
			/* sort by name */
			sort(choice, num, sizeof(*choice), cmp_squelch);
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

	/* Run menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, num, choice);
	menu_layout(&menu, &area);
	menu_select(&menu, 0);

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
	const char *name;
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

bool handle_options_item(menu_type *menu, const ui_event_data *event, int oid)
{
	if (event->type == EVT_SELECT)
	{
		if ((size_t) oid < N_ELEMENTS(sval_dependent))
		{
			sval_menu(sval_dependent[oid].tval, sval_dependent[oid].desc);
		}
		else
		{
			oid = oid - (int)N_ELEMENTS(sval_dependent) - 1;
			assert((size_t) oid < N_ELEMENTS(extra_item_options));
			extra_item_options[oid].action(NULL, NULL);
		}

		return TRUE;
	}

	return FALSE;
}


static const menu_iter options_item_iter =
{
	tag_options_item,
	valid_options_item,
	display_options_item,
	handle_options_item,
	NULL
};


/*
 * Display and handle the main squelching menu.
 */
void do_cmd_options_item(const char *title, int row)
{
	menu_type menu;

	menu_init(&menu, MN_SKIN_SCROLL, &options_item_iter);
	menu_setpriv(&menu, N_ELEMENTS(sval_dependent) + N_ELEMENTS(extra_item_options) + 1, NULL);

	menu.title = title;
	menu_layout(&menu, &SCREEN_REGION);

	screen_save();
	clear_from(0);
	menu_select(&menu, 0);
	screen_load();

	p_ptr->notice |= PN_SQUELCH;

	return;
}


/*
 * Inquire whether the player wishes to squelch items similar to an object
 *
 * Returns whether the item is now squelched.
 */
bool squelch_interactive(const object_type *o_ptr)
{
	char out_val[70];

	if (squelch_tval(o_ptr->tval))
	{
		char sval_name[50];

		/* Obtain plural form without a quantity */
		object_desc(sval_name, sizeof sval_name, o_ptr,
					ODESC_BASE | ODESC_PLURAL);
		/* XXX Eddie while correct in a sense, to squelch all torches on torch of brightness you get the message "Ignore Wooden Torches of Brightness in future? " */
		strnfmt(out_val, sizeof out_val, "Ignore %s in future? ",
				sval_name);

		if (!artifact_p(o_ptr) || !object_flavor_is_aware(o_ptr))
		{
			if (get_check(out_val))
			{
				object_squelch_flavor_of(o_ptr);
				msg_format("Ignoring %s from now on.", sval_name);
				return TRUE;
			}
		}
		/* XXX Eddie need to add generalized squelching, e.g. con rings with pval < 3 */
		if (!object_is_jewelry(o_ptr) || (squelch_level_of(o_ptr) != SQUELCH_BAD))
			return FALSE;
	}

	if (object_was_sensed(o_ptr) || object_was_worn(o_ptr) || object_is_known_not_artifact(o_ptr))
	{
		byte value = squelch_level_of(o_ptr);
		int type = squelch_type_of(o_ptr);

/* XXX Eddie on pseudoed cursed artifact, only showed {cursed}, asked to ignore artifacts */
		if ((value != SQUELCH_MAX) && ((value == SQUELCH_BAD) || !object_is_jewelry(o_ptr)))
		{

			strnfmt(out_val, sizeof out_val, "Ignore all %s that are %s in future? ",
				quality_choices[type].name, quality_values[value].name);

			if (get_check(out_val))
			{
				squelch_level[type] = value;
				return TRUE;
			}
		}

	}
	return FALSE;
}
