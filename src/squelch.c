/*
 * File: squelch.c
 * Purpose: Automatic item destruction ("squelching")
 * Authors: David T. Blackston, Iain McFall, DarkGod, Jeff Greene, David Vestal,
 *          Pete Mack, Andrew Sidwell.
 * Licence: Angband licence, GPL in parts
 *
 * Please read the "COPYING" file in the Angband distribution for licence
 * terms.  If you do not, you have no right to use this file.
 */
#include "angband.h"

/* Structure to describe tval/description pairings. */
typedef struct
{
	int tval;
	cptr desc;
	int squelch_bits;
} tval_desc;

/* Structure to describe ego items */
typedef struct ego_desc
{
	s16b e_idx;
	ego_item_type *e_ptr;
	const char *short_name;
} ego_desc;


/*
 * Stores the various squelch levels for the secondary squelching.
 * It is currently hardcoded at 24 bytes, but since there are only 20
 * applicable tvals, there shouldn't be a problem.
 */
byte squelch_level[SQUELCH_BYTES];

/*
 * These are the base types for automatic squelching on creation.
 * These differ from the tvals, but each one can be seen as a set
 * representing one or more tvals.
 */
#define TYPE_AMMO    1
#define TYPE_BOW     2
#define TYPE_WEAPON1 3
#define TYPE_WEAPON2 4
#define TYPE_BODY    5
#define TYPE_CLOAK   6
#define TYPE_SHIELD  7
#define TYPE_HELM    8
#define TYPE_GLOVES  9
#define TYPE_BOOTS   10
#define TYPE_RING    11
#define TYPE_STAFF   12
#define TYPE_WAND    13
#define TYPE_ROD     14
#define TYPE_SCROLL  15
#define TYPE_POTION  16
#define TYPE_AMULET  17
#define TYPE_BOOK    18
#define TYPE_FOOD    19
#define TYPE_MISC    20

/* Lifted from wizard2.c */
static char head[4] = { 'a', 'A', '0', ':' };

/* Hack -- stores mappings from tval to typeval */
static int tv_to_type[100];

/* Categories for squelch-on-creation. */
static tval_desc typevals[] =
{
	{ TYPE_AMMO,    "Missiles", 0 },
	{ TYPE_BOW,     "Missile Launchers", 0 },
	{ TYPE_WEAPON1, "Weapons (Swords)", 0 },
	{ TYPE_WEAPON2, "Weapons (Non Swords)", 0 },
	{ TYPE_BODY,    "Body Armor", 0 },
	{ TYPE_CLOAK,   "Cloaks", 0 },
	{ TYPE_SHIELD,  "Shields", 0 },
	{ TYPE_HELM,    "Helmets", 0 },
	{ TYPE_GLOVES,  "Gloves", 0 },
	{ TYPE_BOOTS,   "Boots", 0 },
	{ TYPE_AMULET,  "Amulets", 0 },
	{ TYPE_RING,    "Rings", 0 },
	{ TYPE_STAFF,   "Staves", 0 },
	{ TYPE_WAND,    "Wands", 0 },
	{ TYPE_ROD,     "Rods", 0 },
	{ TYPE_SCROLL,  "Scrolls", 0 },
	{ TYPE_POTION,  "Potions", 0 },
	{ TYPE_BOOK,    "Magic Books", 0 },
	{ TYPE_FOOD,    "Food Items", 0 },
	{ TYPE_MISC,    "Miscellaneous", 0 }
};

#define NONE_BITS (1 << SQUELCH_NONE)
#define CHEST_BITS (NONE_BITS | (1 << SQUELCH_OPENED_CHESTS))
#define ORDINARY_BITS (NONE_BITS | (~CHEST_BITS))
#define JEWELRY_BITS (NONE_BITS | (1 << SQUELCH_CURSED))
#define ART_ONLY_BITS (NONE_BITS | (1 << SQUELCH_ALL))

/* Display categories for different kinds of ego-item */
static tval_desc typevals_to_ego[] =
{
	{ TYPE_AMMO,    "Ammo", 0 },
	{ TYPE_BOW,     "Launchers", 0 },
	{ TYPE_WEAPON1, "Weapons", 0 },
	{ TYPE_WEAPON2, "Weapons", 0 },
	{ TYPE_BODY,    "Body Armor", 0 },
	{ TYPE_CLOAK,   "Cloaks", 0 },
	{ TYPE_SHIELD,  "Shields", 0 },
	{ TYPE_HELM,    "Helmets", 0 },
	{ TYPE_GLOVES,  "Gloves", 0 },
	{ TYPE_BOOTS,   "Boots", 0 },
	{ TYPE_AMULET,  "Amulets", 0 },
	{ TYPE_RING,    "Rings", 0 },
	{ TYPE_STAFF,   "Staves", 0 },
	{ TYPE_WAND,    "Wands", 0 },
	{ TYPE_ROD,     "Rods", 0 },
	{ TYPE_SCROLL,  "Scrolls", 0 },
	{ TYPE_POTION,  "Potions", 0 },
	{ TYPE_BOOK,    "Magic Books", 0 },
	{ TYPE_FOOD,    "Food Items", 0 },
	{ TYPE_MISC,    "Miscellaneous", 0 }
};


/* Categories for squelch-on-identification (lifted/edited from wizard2.c). */
static tval_desc tvals[] =
{
	{ TV_SWORD,      "Sword", 			ORDINARY_BITS },
	{ TV_POLEARM,    "Polearm", 		ORDINARY_BITS },
	{ TV_HAFTED,     "Hafted Weapon",	ORDINARY_BITS },
	{ TV_BOW,        "Bow",				ORDINARY_BITS },
	{ TV_ARROW,      "Arrows",			ORDINARY_BITS },
	{ TV_BOLT,       "Bolts",			ORDINARY_BITS },
	{ TV_SHOT,       "Shots",			ORDINARY_BITS },
	{ TV_SHIELD,     "Shield",			ORDINARY_BITS },
	{ TV_CROWN,      "Crown",			ORDINARY_BITS },
	{ TV_HELM,       "Helm",			ORDINARY_BITS },
	{ TV_GLOVES,     "Gloves",			ORDINARY_BITS },
	{ TV_BOOTS,      "Boots",			ORDINARY_BITS },
	{ TV_CLOAK,      "Cloak",			ORDINARY_BITS },
	{ TV_DRAG_ARMOR, "Dragon Scale Mail", ORDINARY_BITS },
	{ TV_HARD_ARMOR, "Hard Armor",		ORDINARY_BITS },
	{ TV_SOFT_ARMOR, "Soft Armor",		ORDINARY_BITS },
	{ TV_DIGGING,    "Diggers",			ORDINARY_BITS },
	{ TV_RING,       "Rings",			JEWELRY_BITS },
	{ TV_AMULET,     "Amulets",			JEWELRY_BITS },
	{ TV_CHEST,      "Open Chests",		CHEST_BITS  },
	{ TV_LITE,       "Lite Sources",	ART_ONLY_BITS },
	{ 0,            NULL , 				0 }
};


#define LINES_PER_COLUMN   19

#define SQUELCH_HEAD   0
#define SQUELCH_TAIL   3

/* Index into squelch_level for different item types */
#define RING_INDEX  17
#define AMULET_INDEX  18


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
static cptr get_autoinscription(s16b kind_idx)
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
	if (!note || !object_aware_p(o_ptr))
		return 0;

	/* Don't re-inscribe if it's already correctly inscribed */
	if (existing_inscription && streq(note, existing_inscription))
		return 0;

	/* Get an object description */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 3);

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
	for (i = INVEN_PACK; i > 0; i--)
	{
		/* Skip empty items */
		if (!inventory[i].k_idx) continue;

		/* Apply the inscription */
		apply_autoinscription(&inventory[i]);
	}

	return;
}





/*** Squelch code ***/

/*
 * The squelch code is split up into two parts; UI and game workings.  The UI
 * section needs a lot more doing than the game workings, which work.
 *
 * Additionally, when screen_save() and screen_load() work multiple times, 
 * lots of this code can be cleaned up to be less refresh-happy (a big win
 * for doing things over SSH).
 *
 * The following should be considered:
 *  - use the same menu implementation for all the different menus, rather
 *    then reimplmenting it three(?) times -- perhaps steal Zangband code
 *  - making sure we only use "safe" string functions, from z-form.c
 *  - Make dump_autoins_info() and dump_squelch_info() more similar for
 *    consistency.
 *  - Simplify do_qual_squelch().
 *  - Add find-as-you-type for the various menus, a la Firefox (nice UI touch)
 */

/*
 * Utility function used for sorting an array of ego-item indices by
 * ego-item name.
 */
static int ego_comp_func(const void *a_ptr, const void *b_ptr)
{
	const ego_desc *a = a_ptr;
	const ego_desc *b = b_ptr;

	/* Compare real names (minus prefixes) */
	return (strcmp(a->short_name, b->short_name));
}



/*
 * Determines if an object is going to be squelched on identification.
 *
 * `o_ptr` should be the object type being identified.  `feel` should be the
 * feeling of the object (on pseudo-id) or 0.  `fullid` is TRUE if the object
 * is being identified.
 *
 * Returns either SQUELCH_NO, SQUELCH_YES, or SQUELCH_FAILED.
 */
int squelch_item_ok(object_type *o_ptr, byte feel, bool fullid)
{
	int i;
	int result = SQUELCH_NO;
	int num = -1;

	/* Squelch some ego items if known */
	if (fullid && (ego_item_p(o_ptr)) && (e_info[o_ptr->name2].squelch))
	{
		if (o_ptr->note) return (SQUELCH_FAILED);
		else return (SQUELCH_YES);
	}

	/* Check to see if the object is eligible for squelch-on-ID */

	/* Find the appropriate squelch group */
	for (i = 0; i < (int)N_ELEMENTS(tvals); i++)
	{
		if (tvals[i].tval == o_ptr->tval)
			num = i;
	}

	/* Never squelched */
	if (num == -1) return SQUELCH_NO;

	/* If the object is being identified get the feeling returned by a heavy pseudoid. */
	if (fullid) feel = value_check_aux1(o_ptr);

	/* Get result based on the feeling and the squelch_level */
	switch (squelch_level[num])
	{
		case SQUELCH_CURSED:
		{
			if ((feel == INSCRIP_BROKEN) || (feel == INSCRIP_TERRIBLE) ||
	 	      (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED))
			{
				result = SQUELCH_YES;
			}

      		break;
		}

		case SQUELCH_AVERAGE:
		{
			if ((feel == INSCRIP_BROKEN) || (feel == INSCRIP_TERRIBLE) ||
		 	    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE))
			{
				result = SQUELCH_YES;
			}

      		break;
		}

		case SQUELCH_GOOD_WEAK:
		{
			if ((feel == INSCRIP_BROKEN) || (feel == INSCRIP_TERRIBLE) ||
		 	    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE) || (feel == INSCRIP_GOOD))
			{
				result = SQUELCH_YES;
			}

			break;
		}

		case SQUELCH_GOOD_STRONG:
		{
			if ((feel == INSCRIP_BROKEN) || (feel == INSCRIP_TERRIBLE) ||
		 	    (feel == INSCRIP_WORTHLESS) || (feel == INSCRIP_CURSED) ||
			    (feel == INSCRIP_AVERAGE) ||
			    ((feel == INSCRIP_GOOD) &&
					((fullid) || (cp_ptr->flags & CF_PSEUDO_ID_HEAVY))))
			{
				result = SQUELCH_YES;
			}

			break;
		}

		case SQUELCH_ALL:
		{
      		result = SQUELCH_YES;
      		break;
		}
	}

	/* Make sure we don't squelch artifacts */
	if (result == SQUELCH_YES)
	{
		/* Squelching will fail on an artifact */
		if ((artifact_p(o_ptr)) || (o_ptr->note)) result = SQUELCH_FAILED;
	}

	return (result);
}

/*
 * This performs the squelch, actually removing the item from the
 * game.  It returns 1 if the item was squelched, and 0 otherwise.
 * This return value is never actually used.
 */
int squelch_item(int squelch, int item, object_type *o_ptr)
{
	if (squelch != SQUELCH_YES) return 0;

	if (item >= 0)
	{
		inven_item_increase(item, -o_ptr->number);
		inven_item_optimize(item);
	}

	else
	{
		floor_item_increase(0 - item, -o_ptr->number);
		floor_item_optimize(0 - item);
	}

	return 1;
}

/*
 * Puts all items to be squelched at the bottom of a given pile/stack in the
 * dungeon so that squelch_pile() has an easier job.
 *
 * XXX Needs better documentation.
 */
static void rearrange_stack(int y, int x)
{
	s16b o_idx, next_o_idx;
	s16b first_bad_idx, first_good_idx, cur_bad_idx, cur_good_idx;

	object_type *o_ptr;

	bool sq_flag = FALSE;

	/* Initialize */
	first_bad_idx = 0;
	first_good_idx = 0;
	cur_bad_idx = 0;
	cur_good_idx = 0;

	/* go through all the objects */
	for (o_idx = cave_o_idx[y][x]; o_idx; o_idx = next_o_idx)
	{
		o_ptr = &(o_list[o_idx]);
		next_o_idx = o_ptr->next_o_idx;

		/*is it marked for squelching*/
		sq_flag = ((k_info[o_ptr->k_idx].squelch == SQUELCH_ALWAYS) &&
		(k_info[o_ptr->k_idx].aware));

		if (sq_flag)
		{
			if (first_bad_idx == 0)
			{
				first_bad_idx = o_idx;
				cur_bad_idx = o_idx;
			}
			else
			{
				o_list[cur_bad_idx].next_o_idx = o_idx;
				cur_bad_idx = o_idx;
			}
		}
		else
		{
			if (first_good_idx == 0)
			{
				first_good_idx = o_idx;
				cur_good_idx = o_idx;
			}
			else
			{
				o_list[cur_good_idx].next_o_idx = o_idx;
				cur_good_idx = o_idx;
			}
		}
	}

	if (first_good_idx != 0)
	{
		cave_o_idx[y][x] = first_good_idx;
		o_list[cur_good_idx].next_o_idx = first_bad_idx;
		o_list[cur_bad_idx].next_o_idx = 0;
	}

	else
	{
		cave_o_idx[y][x] = first_bad_idx;
	}
}

/*
 * Given a stack of objects in the dungeon, squelch those relevant.
 */
void squelch_pile(int y, int x)
{
	s16b o_idx, next_o_idx;
	object_type *o_ptr;
	bool squelch = FALSE;

	/* Rearrange the stack before squelching it. */
	rearrange_stack(y, x);

	/* Iterate over all objects on the given grid */
	for (o_idx = cave_o_idx[y][x]; o_idx; o_idx = next_o_idx)
	{
		o_ptr = &(o_list[o_idx]);
		next_o_idx = o_ptr->next_o_idx;

		/* Only squelch things that should be squelched */
		if (k_info[o_ptr->k_idx].squelch == SQUELCH_ALWAYS) squelch = TRUE;
		if (!k_info[o_ptr->k_idx].aware) squelch = FALSE;
		if (artifact_p(o_ptr)) squelch = FALSE;

		/* Always squelch "nothing" XXX? */
		if (!o_ptr->k_idx) squelch = TRUE;

		/* Actually do the squelching */
		if (squelch) delete_object_idx(o_idx);
	}
}


/*
 * Convert the values returned by squelch_item_ok to string.
 */
const char *squelch_to_label(int squelch)
{
	if (squelch == SQUELCH_YES) return ("(squelched)");
	if (squelch == SQUELCH_FAILED) return ("(squelch failed)");

	return "";
}


/*
 * Save squelch data to a pref file.
 */
static void dump_squelch_info(void)
{
	int i, tval, sval, squelch;
	FILE *fff;
	char buf[1024];
	char fname[80];

	/* Prompt */
	prt("Command: Dump Squelch Info", 17, 30);
	prt("File: ", 18, 30);

	/* Default filename */
	strnfmt(fname, sizeof(fname), "%s.squ", op_ptr->base_name);

	/* Get a filename */
	if (askfor_aux(fname, sizeof fname, NULL))
	{
		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

		/* Append to the file */
		safe_setuid_drop();
		fff = my_fopen(buf, "a");
		safe_setuid_grab();

      	/* Test for success */
		if (!fff) return;

		/* Start dumping */
		fprintf(fff, "\n\n");
		fprintf(fff, "# Squelch bits\n\n");

		/* Dump squelch bits */
		for (i = 1; i < z_info->k_max; i++)
		{
			tval = k_info[i].tval;
			sval = k_info[i].sval;
			squelch = k_info[i].squelch;

			/* Dump the squelch info */
			if (tval || sval)
				fprintf(fff, "Q:%d:%d:%d:%d\n", i, tval, sval, squelch);
		}

		fprintf(fff, "\n\n# squelch_level array\n\n");

		for (i = 0; i < SQUELCH_BYTES; i++)
			fprintf(fff, "Q:%d:%d\n", i, squelch_level[i]);

		/* All done */
		fprintf(fff, "\n\n\n\n");

		/* Close */
		my_fclose(fff);

		/* Ending message */
		prt("Squelch file saved successfully.  Hit any key to continue.", 17, 30);
		inkey();
	}

	return;
}


/*
 * Save autoinscription data to a pref file.
 */
static int dump_autoins_info(void)
{
	int i;
	FILE *fff;
	char buf[1024];
	char fname[80];

	/* Forget it now if there are no inscriptions. */
	if (!inscriptions)
	{
		prt("No inscriptions to save.  Press any key to continue.", 16, 30);
		inkey();
	}

	/* Prompt */
	prt("Command: Dump Autoinscribe Info", 16, 30);
	prt("File: ", 17, 30);

	/* Default filename */
	strcpy(fname, op_ptr->base_name);

	/* Get a filename */
	if (askfor_aux(fname, sizeof fname, NULL))
	{
		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

		/* Overwrite the file */
		safe_setuid_drop();
		fff = my_fopen(buf, "w");
		safe_setuid_grab();

		/* Test for success */
		if (!fff) return 0;

		/* Start dumping */
		fprintf(fff, "# Format: B:[Item Kind]:[Inscription]\n\n");

		for (i = 0; i < inscriptions_count; i++)
		{
			object_kind *k_ptr = &k_info[inscriptions[i].kind_idx];

			/* Write a comment for the autoinscription*/
			fprintf(fff, "# Autoinscription for %s\n", k_name + k_ptr->name);

			/* Dump the autoinscribe info */
			fprintf(fff, "B:%d:%s\n\n", inscriptions[i].kind_idx,
			        quark_str(inscriptions[i].inscription_idx));
		}

		/* Close */
		my_fclose(fff);

		/* Ending message */
		prt("Autoinscribe file saved successfully.  (Hit a key.)", 16, 30);
		inkey();
	}

	return 1;
}


/*
 * Load squelch info from a pref file.
 */
static void load_squelch_info(void)
{
	char fname[80];

 	/* Prompt */
 	prt("Command: Load squelch info from file", 16, 30);
 	prt("File: ", 17, 30);

	/* Default filename */
	strnfmt(fname, sizeof(fname), "%s.squ", op_ptr->base_name);

 	/* Ask for a file (or cancel) */
 	if (askfor_aux(fname, sizeof fname, NULL))
	{
		/* Process the given filename, note success or failure */
		if (process_pref_file(fname))
			prt("Failed to load squelch data!  (Hit a key.)", 17, 30);
		else
			prt("Squelch squelch squelch!  (Hit a key.)", 17, 30);

		inkey();
	}

	return;
}

/*  ===================== QUALITY MENU ======================= */

static menu_type squelch_q_menu;
static menu_type squelch_q_items;

static struct {
	signed char quality;
	int item_pos;
} q_rendez;

static void handle_q_squelch(void *arg, const char *xxx)
{
	int sqlev = (int) arg;
	int i;
	event_type ke = {EVT_REFRESH, 0, 0, 0, 0};

	for (i = 0; i < SQUELCH_BYTES; i++)
	{
		if (tvals[i].squelch_bits & (1 << sqlev))
			squelch_level[i] = sqlev;
	}

	Term_event_push(&ke);

	/* Invalidate q_rendez */
	q_rendez.quality = sqlev;
}

static void handle_q_squelch_one(void *arg, const char *xxx)
{
	int sqlev = (int) arg;
	event_type ke = {EVT_REFRESH, 0, 0, 0, 0};
	bool refresh = FALSE;

	if ((squelch_level[q_rendez.item_pos] != sqlev) &&
	   (tvals[q_rendez.item_pos].squelch_bits & (1 << sqlev)))
	{
		refresh = TRUE;
		squelch_level[q_rendez.item_pos] = sqlev;
	}

	if (refresh) Term_event_push(&ke);

	q_rendez.quality = sqlev;
}

static menu_item squelch_q_kinds [] =
{
	{{ 0, "Affect all groups", handle_q_squelch, 0 }, 0, MN_DISABLED },
	{{ 0, "Squelch Nothing", handle_q_squelch, (void *)SQUELCH_NONE }, 'N', 0 },
	{{ 0, "Squelch Cursed Items", handle_q_squelch, (void *)SQUELCH_CURSED }, 'C', 0 },
	{{ 0, "Squelch Average and Below", handle_q_squelch, (void *)SQUELCH_AVERAGE }, 'V', 0 },
	{{ 0, "Squelch Good (Strong Pseudo-ID and Identify)", handle_q_squelch, (void *)SQUELCH_GOOD_STRONG }, 'G', 0 },
	{{ 0, "Squelch Good (Weak Pseudo-ID)", handle_q_squelch, (void *)SQUELCH_GOOD_WEAK }, 'W', 0 },
	{{ 0, "Squelch All but Artifacts", handle_q_squelch, (void *)SQUELCH_ALL }, 'A', 0 },
	{{ 0, "Squelch Chests after Opening", handle_q_squelch, (void *)SQUELCH_OPENED_CHESTS }, 'O', 0 },
	{{ 0, "Affect selected group", 0, 0 }, 0, MN_DISABLED },
	{{ 0, "Squelch Nothing", handle_q_squelch_one, (void *)SQUELCH_NONE }, 'n', 0 },
	{{ 0, "Squelch Cursed Items", handle_q_squelch_one, (void *)SQUELCH_CURSED }, 'c', 0 },
	{{ 0, "Squelch Average and Below", handle_q_squelch_one, (void *)SQUELCH_AVERAGE }, 'v', 0 },
	{{ 0, "Squelch Good (Strong Pseudo-ID and Identify)", handle_q_squelch_one, (void *)SQUELCH_GOOD_STRONG }, 'g', 0 },
	{{ 0, "Squelch Good (Weak Pseudo-ID)", handle_q_squelch_one, (void *)SQUELCH_GOOD_WEAK }, 'w', 0 },
	{{ 0, "Squelch All but Artifacts", handle_q_squelch_one, (void *)SQUELCH_ALL }, 'a', 0 },
	/* Invisible entry */
	{{ 0, 0, handle_q_squelch, (void *)SQUELCH_OPENED_CHESTS}, 'o', MN_HIDDEN }, 
};


static bool handle_squelch_item(char cmd, void *db, int oid)
{
	if (oid < 0) return FALSE;
	if (q_rendez.item_pos == oid)
	{
		int i = (squelch_level[oid] + 1) % (SQUELCH_OPENED_CHESTS + 1);

		while (!(tvals[oid].squelch_bits & (1 << i)))
			i = (i+1) % (SQUELCH_OPENED_CHESTS+1);

		squelch_level[oid] = i;

		return TRUE;
	}
		
	q_rendez.item_pos = oid;

	if ((q_rendez.quality >= 0 && oid >= 0) &&
	    (tvals[oid].squelch_bits & (1 << q_rendez.quality)))
	{
		squelch_level[oid] = q_rendez.quality;
	}

	return TRUE;
}

static void display_tval(menu_type *menu, int oid,
								bool cursor, int row, int col, int width)
{
	const char *squelch_str = "NCVGWAO";
	const byte squelch_attr[] = {TERM_WHITE, TERM_L_GREEN, TERM_YELLOW, TERM_YELLOW, TERM_ORANGE, TERM_ORANGE, TERM_L_GREEN};

	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	c_put_str(attr, format("[%c] %s", squelch_str[squelch_level[oid]], tvals[oid].desc), row, col);
	Term_putch(col+1, row, squelch_attr[squelch_level[oid]], squelch_str[squelch_level[oid]]);
}

static const menu_iter tval_iter = {
	0, 0, 0,
	display_tval,
	handle_squelch_item
};


/*
 * Display and handle the quality-based squelching menu.
 */
static void do_qual_squelch(void)
{
	region item_region = { 0, 0, 20, 0 };
	region command_region = { 25, 0, 0, 0 };
	event_type ke = { EVT_NONE, 0, 0, 0, 0 };

	menu_layout(&squelch_q_items, &item_region);
	menu_layout(&squelch_q_menu, &command_region);

	Term_save();
	Term_clear();

	q_rendez.item_pos = 0;
	q_rendez.quality = -1;

	while (ke.key != ESCAPE)
		ke = menu_select(&squelch_q_items, &q_rendez.item_pos, 0);

	Term_load();

	return;
}



#define MAX_EGO_ROWS       19

/*
 * Display and handle menu for squelching of ego-items.
 *
 * Be wary when modifying this function to always FREE(choice) before
 * returning.  Also, the return value is significant.  -1 will give a
 * "no egos known" message on the main menu; 0 will not.
 */
static int do_cmd_squelch_egos(void)
{
	int i, j, idx;
	int max_num = 0;
	ego_item_type *e_ptr;

	int first = 0;
	int active = 0;
	int old_active = -1;
	int last;
	bool display_all = TRUE;
	char ch;
	ego_desc *choice;
	const char *name;

	int selected_y = 0;

 	/* Allocate the array of ego item choices */
 	C_MAKE(choice, alloc_ego_size, ego_desc);

	/* Get the valid ego-items */
	for (i = 0; i < alloc_ego_size; i++)
	{
		/* Save some things for later */
		idx = alloc_ego_table[i].index;
		e_ptr = &e_info[idx];
		name = e_name + e_ptr->name;

		/* Only existent, known ego-items allowed */
		if (!e_ptr->name || !e_ptr->everseen) continue;

		/* Append the index and name to the array */
		choice[max_num].e_idx = idx;
		choice[max_num].e_ptr = &e_info[idx];
		choice[max_num].short_name = name;

		/* Strip prefix of the ego name */
		if (prefix(name, "of the "))  choice[max_num].short_name = (name + 7);
		else if (prefix(name, "of ")) choice[max_num].short_name = (name + 3);

		/* Next record */
		max_num++;
	}

	/* No ego-items, just return */
	if (max_num == 0)
	{
		FREE(choice);
		return -1;
	}

	/* Quickly sort the array by ego-item name */
	qsort(choice, max_num, sizeof(choice[0]), ego_comp_func);

	/* Determine the last ego-item to display on the screen */
	last = MIN(MAX_EGO_ROWS, max_num) - 1;

	/* Loop infinitely */
	while (1)
	{
		if (display_all)
		{
			/* Clear the screen */
			Term_clear();

			/* Hack -- add paging */
			if (first > 0) c_put_str(TERM_WHITE, "-more-", 2, 4);
			if (last < (max_num - 1)) c_put_str(TERM_WHITE, "-more-", 22, 4);

			/* Page header */
			Term_gotoxy(1, 0);

			text_out_hook = text_out_to_screen;
			text_out_indent = 1;

			text_out("Use the ");
			text_out_c(TERM_L_GREEN, "movement keys");
			text_out(" to navigate, or ");
			text_out_c(TERM_L_GREEN, "space");
			text_out(" to advance a page.  ");
			text_out_c(TERM_L_GREEN, "Enter");
			text_out(" toggles squelch status.  ");
			text_out_c(TERM_L_RED, "*");
			text_out(" marks an item to be squelched.");
		}

		/* Only show a portion of the list */
		for (i = first; i <= last; i++)
		{
			const char *tval_name = NULL;
			byte attr;
			int tval = 0, y, typeval;

			/* Only redraw those things which need redrawing */
			if (!display_all && (i != active) && (i != old_active))
				continue;

			/* Grab the ego-item */
			e_ptr = choice[i].e_ptr;

			/* Choose a colour for the line of text, depending on its status */
			if (i == active) attr = TERM_L_BLUE;
			else attr = TERM_WHITE;

			/* Get the first valid tval for this ego-item */
			for (j = 0; j < EGO_TVALS_MAX; j++)
			{
				/* Ignore "empty" entries */
				if (e_ptr->tval[j] < 1) continue;

				/* Set this as our tval-to-find */
				tval = e_ptr->tval[j];
				break;
			}

			/*
			 * Get the name of the kind of this ego-item, by finding the
			 * typeval coresponding to its first tval.
			 * 
			 * Note that this will mess up if it's ever possible to have, say,
			 * both gloves and arrows of the same ego-type, because it assumes
			 * that ego-items can only be of one kind of item.
			 */
			typeval = tv_to_type[tval];
			for (j = 0; j < (int)N_ELEMENTS(typevals_to_ego); j++)
			{
				if (typevals_to_ego[j].tval == typeval)
					tval_name = typevals_to_ego[j].desc;
			}

			/* In the rare case that there's no name for a tval, panic */
			if (tval_name == NULL)
				tval_name = "Fish";

			/* Work out y-position of the list item, and remember if selected */
			y = i - first + 3;
			if (i == active) selected_y = y;

		 	/* Display on screen, and add squelch mark if appropriate */
		 	c_put_str(attr, format("[ ] %s %s", tval_name, e_name + e_ptr->name), y, 1);
			if (e_ptr->squelch) c_put_str(TERM_L_RED, "*", y, 2);
		}

		/* Move the cursor to between the brackets, for visual consistency */
		Term_gotoxy(2, selected_y);

		/* Reset some flags */
		display_all = FALSE;
		old_active = -1;

		/* Get a command */
		ch = inkey();

		/* Get the selected ego-item type */
		e_ptr = choice[active].e_ptr;

		/* Process the command */
		switch (ch)
		{
			case ESCAPE:
			{
				return 0;
			}

			case '\r':
			case '\n':
			{
				/* Toggle the "squelch" flag */
				e_ptr->squelch = !e_ptr->squelch;
				break;
			}

			case '2':
			case ARROW_DOWN:
			{
				/* Proceed a position */
				old_active = active;
				active++;

				if (active > max_num - 1)
				{
					if ((first != 0) && (last != MIN(MAX_EGO_ROWS, max_num) - 1))
						display_all = 1;

					first = 0;
					active = 0;

					last = MIN(MAX_EGO_ROWS, max_num) - 1;
				}
				else if (active > last)
				{
					first++;
					last++;

					/* Redraw all */
					display_all = 1;
				}

				break;
			}

			case '8':
			case ARROW_UP:
			{
				/* Retrocede a position */
				old_active = active;
				active--;

				if (active == -1)
				{
					active = last = max_num - 1;
					first = last - MAX_EGO_ROWS;

					display_all = 1;
					if (first < 0) first = 0;
					if (active < 0) active = 0;
				}
				else if (active < first)
				{
					first--;
					last--;

					/* Redraw all */
					display_all = 1;
				}

				break;
			}

			case ' ':
			{
				/* Check for loopy loopy looping */
				if (last < (max_num - 1))
				{
					/* Advance one "screen" */
					first = first + MAX_EGO_ROWS;
					active = first;
					last = last + MAX_EGO_ROWS;
					if (last >= max_num - 1) last = max_num - 1;
				}
				else
				{
					last = MIN(MAX_EGO_ROWS, max_num) - 1;
					active = 0;
					first = 0;
				}

				/* Redraw all */
				display_all = 1;

				break;
			}

#if 0
			/* Compare with the first letter of ego-item names */
			default:
			{
				/* Ignore strange characters */
				if (!isgraph((unsigned char)ch)) break;

				/* Check for seen ego-items */
				for (i = 0; i < max_num; i++)
				{
					/* Compare first letter, case insensitively */
					if (toupper((unsigned char)choice[i].short_name[0]) ==
					    toupper((unsigned char)ch)) break;
				}

				/* Found one? */
				if (i >= max_num) break;

				/* Jump there */
				active = i;

				/* Adjust visual bounds */
				/* Try to put the found ego in the first row */
				last = MIN(active + MAX_EGO_ROWS - 1, max_num - 1);
				first = MAX(last - MAX_EGO_ROWS + 1, 0);

				/* Redraw all */
				display_all = 1;
				break;
			}
#endif
		}
	}

	/* Free resources */
	FREE(choice);

	return 0;
}



/*
 * Display and handle squelching menu for a given "typeval", consisting
 * of multiple tvals.
 */
static int do_cmd_squelch_type(int index_into_typevals)
{
	const char *tyval_desc;
	int tyval;

	int max_num = 0;
	int i, j;

	int temp, choice[60];

	bool first_time = TRUE, display_help = TRUE;
	byte text_color, color;
	int row, col;
	int active = 0;
	int old_active = -1;
	int selected_x = 0, selected_y = 0;

	char buf[80];
	char sq, ch;


	/* Work out the tval and its description */
	tyval = typevals[index_into_typevals].tval;
	tyval_desc = typevals[index_into_typevals].desc;


	/* Iterate over all possible object kinds, finding ones which can be squelched */
	for (i = 1; (max_num < (int)sizeof(choice)) && (i < z_info->k_max); i++)
	{
		object_kind *k_ptr = &k_info[i];

		if (tv_to_type[k_ptr->tval] == tyval)
		{
			/* Skip empty objects, artifacts, gold */
			if (!k_ptr->name) continue;
			if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;
			if (k_ptr->tval == TV_GOLD) continue;

			/* Skip if we haven't seen the item yet */
			if (!k_ptr->everseen) continue;

			/* Add this item to our possibles list */
			choice[max_num++] = i;
		}
	}

	/* Return here if there are no objects */
	if (!max_num)
		return -1;

	/* Sort into tval, then cost order (consider sval subsorting?) */
	for (i = 0; i < max_num; i++)
	{
		for (j = i; j < max_num; j++)
		{
			if ((k_info[choice[i]].tval > k_info[choice[j]].tval) ||
			    ((k_info[choice[i]].tval == k_info[choice[j]].tval) &&
			     (k_info[choice[i]].cost > k_info[choice[j]].cost)))
			{
				temp = choice[i];
				choice[i] = choice[j];
				choice[j] = temp;
			}
		}
	}

	/* Clear the screen */
	Term_clear();

	/* Display the screen */
	while (TRUE)
	{
		/* Display te actual items */
		for (i = 0; i < max_num; i++)
		{
			object_kind *k_ptr = &k_info[choice[i]];
			cptr cur_str;

			/* Save redraws by only displaying the full list the first time, */
			/* and thereafter only when the active entry changes */
			if (!first_time && (i != active) && (i != old_active))
				continue;

			/* Prepare it */
			row = 5 + (i % LINES_PER_COLUMN);
			col = 30 * (i / LINES_PER_COLUMN);
#if 0
			ch = head[i / 26] + (i % 26);
#endif

			/* Acquire the "name" of object "i" */
			strip_name(buf, choice[i], TRUE);

			/* Deal with the active selection */
			if (i == active)
			{
				/* Get autoinscription & display it */
				cur_str = get_autoinscription(choice[active]);
				c_put_str(TERM_WHITE,
					format("Current autoinscription: %-40s", cur_str ? cur_str : "[None]"), 4,	39);

				/* Set the display colour */
				text_color = TERM_L_BLUE;

				/* Save selected info */
				selected_x = col + 2;
				selected_y = row;
			}
			else
			{
				/* Set display colour */
				text_color = TERM_WHITE;
			}

			/* Get the color and character */
			switch (k_ptr->squelch)
			{
				case SQUELCH_ALWAYS:           sq = 'S'; color = TERM_L_RED; break;
				case NO_SQUELCH_NEVER_PICKUP:  sq = 'L'; color = TERM_L_GREEN; break;
				case NO_SQUELCH_ALWAYS_PICKUP: sq = 'A'; color = TERM_L_UMBER; break;
				default:                       sq = 'N'; color = TERM_L_BLUE; break;
			}

			/* Print it */
			c_put_str(text_color, format(" [ ] %s", buf), row, col);
			c_put_str(color, format("%c", sq, buf), row, col + 2);
		}

		Term_gotoxy(selected_x, selected_y);


		/* Header text */
		if (display_help)
		{
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
			text_out(" to return to the previous menu.  On a given option, ");
			text_out_c(TERM_L_BLUE, "^N");
			text_out(" will mark it as never squelch, ");
			text_out_c(TERM_L_GREEN, "^L");
			text_out(" as never pickup, ");
			text_out_c(TERM_L_UMBER, "^A");
			text_out(" as always pickup, and ");
			text_out_c(TERM_L_RED, "^S");
			text_out(" as always squelch; ");
			text_out_c(TERM_L_GREEN, "space");
			text_out(" will cycle between these.  ");
			text_out_c(TERM_L_GREEN, "Enter");
			text_out(" will prompt you for a new autoinscription.");

			/* No more! */
			display_help = FALSE;
		}
		
		old_active = -1;

		/* Get input */
		ch = inkey();
		if (ch == ESCAPE) return 1;

		/* Switch options */
		if (ch == KTRL('N'))      k_info[choice[active]].squelch = SQUELCH_NEVER;
		else if (ch == KTRL('L')) k_info[choice[active]].squelch = NO_SQUELCH_NEVER_PICKUP;
		else if (ch == KTRL('A')) k_info[choice[active]].squelch = NO_SQUELCH_ALWAYS_PICKUP;
		else if (ch == KTRL('S')) k_info[choice[active]].squelch = SQUELCH_ALWAYS;

		/* Advance by one */
		else if (ch == ' ')
		{
			/* Boundary control */
			if (k_info[choice[active]].squelch >= SQUELCH_TAIL)
				k_info[choice[active]].squelch = SQUELCH_HEAD;
			else
				k_info[choice[active]].squelch++;
		}
		else if (ch == '8' || ch == ARROW_UP)
		{
			old_active = active;    /* Redraw the current active */
			active--;               /* Move up one row */

			/* Handle wrapping round */
			if (active < 0) active = max_num - 1;
		}
		else if (ch == '2' || ch == ARROW_DOWN)
		{
			old_active = active;    /* Redraw the current active */
			active++;               /* Move down one row */

			/* Handle wrapping round */
			if (active > (max_num - 1)) active = 0;
		}
		else if (ch == '6' || ch == ARROW_RIGHT)
		{
			/* Move one column to right, but check first */
			if ((active + LINES_PER_COLUMN) <= max_num - 1)
			{
				/* Redraw the current active */
				old_active = active;
				active += LINES_PER_COLUMN;
			}

			else bell("");
	    }
		else if (ch == '4' || ch == ARROW_LEFT)
		{
			/* Move one column to left, but check first */
			if ((active - LINES_PER_COLUMN) >= 0)
			{
				/* Redraw the current active */
				old_active = active;

				active -= LINES_PER_COLUMN;
			}

			else bell("");
		}
		else if (ch == 13)
		{
			s16b k_idx = choice[active];
			const char *current;

			/* Obtain the current inscription */
			current = get_autoinscription(k_idx);	

			/* Copy of the current inscription, or clear the buffer */
			if (current)
				my_strcpy(buf, current, sizeof(buf));
			else
				buf[0] = '\0';

			/* Get a new inscription (possibly empty) */
			if (get_string("Autoinscription: ", buf, sizeof(buf)))
			{
				/* Save the inscription */
				add_autoinscription(k_idx, buf);

				/* Inscribe stuff */
				p_ptr->notice |= (PN_AUTOINSCRIBE);
				p_ptr->window |= (PW_INVEN | PW_EQUIP);
			}

			display_help = TRUE;
		}

#if 0
		else
		{
			/* Save the old choice */
			int old_num = active;

			/* Analyze choice */
			active = -1;
			if ((ch >= head[0]) && (ch < head[0] + 26)) active = ch - head[0];
			if ((ch >= head[1]) && (ch < head[1] + 26)) active = ch - head[1] + 26;
			if ((ch >= head[2]) && (ch < head[2] + 17)) active = ch - head[2] + 52;

			/* Bail out if choice is "illegal" */
			if ((active < 0) || (active >= max_num)) active = old_num;
			else old_active = old_num;
		}
#endif

	}

	return 1;
}


/*
 * Display and handle the main squelching menu.
 */
static int do_cmd_squelch_aux(void)
{
	static bool known_objs = TRUE;
	static bool known_egos = TRUE;
	int col, row, num;
	char ch;

	/* Clear screen */
	Term_clear();

	/* Print all typevals and their descriptions */
	for (num = 0; (num < 60) && (num < (int)N_ELEMENTS(typevals)); num++)
	{
		row = 3 + (num % 20);
		col = 30 * (num / 20);
		ch = head[num / 26] + (num % 26);
		prt(format("[%c] %s", ch, typevals[num].desc), row, col);
	}

	prt("Commands:", 3, 30);
	prt("[a-t]: Go to squelching settings for this type", 5, 30);
	prt("Q    : Go to quality squelching sub-menu*", 6, 30);
	prt("E    : Go to ego-item squelching sub_menu", 7, 30);
	prt("S    : Save squelch values to pref file", 8, 30);
	prt("B    : Save autoinscriptions to pref file", 9, 30);
	prt("L    : Load squelch/autoinscription settings", 10, 30);
/*	prt("G    : Load autoinscriptions from pref file", 11, 30); */
	prt("ESC  : Back to options menu.", 12, 30);

	prt(" * includes squelching opened chests.", 14, 30);

	/* From the previous call: show if there are no known objects of that type */
	if (!known_objs)
	{
		c_put_str(TERM_RED, "No known objects of that type.", 16, 30);
		known_objs = TRUE;
	}

	if (!known_egos)
	{
		c_put_str(TERM_RED, "You have not seen any ego items yet.", 16, 30);
		known_egos = TRUE;
	}

	/* Choose! */
	if (!get_com("Item Squelching and Autoinscription Main Menu: ", &ch))
		return (0);

	/* This could equally well be a switch */
	if (ch == 'S') dump_squelch_info();         /* Dump squelch info */
	else if (ch == 'B') dump_autoins_info();    /* Dump autoinscribe info */
	else if (ch == 'L') load_squelch_info();    /* Load squelch info */

	else if (ch == 'Q')
	{
		/* Switch to secondary squelching menu */
		do_qual_squelch();
	}

	else if (ch == 'E')
	{
		/* Switch to ego-item squelching menu */
		if (do_cmd_squelch_egos() == -1) known_egos = FALSE;
	}

	else
 	{
		/* Analyze choice */
		num = A2I(ch);

		/* Bail out if choice is illegal */
		if ((num < 0) || (num >= (int)N_ELEMENTS(typevals))) return (0);
		else
		{
			if (do_cmd_squelch_type(num) == -1)
				known_objs = FALSE;
		}
	}


	/* Redisplay */
	return (1);
}

/*
 * UI entry point, called from cmd4.c.
 * XXX Merge with do_cmd_squelch_aux().  This is such a petty function.
 */
void do_cmd_squelch_autoinsc(void)
{
	int flag = 1;
	int x, y;


	/* Simple loop */
	while (flag)
		flag = do_cmd_squelch_aux();


	/* XXX Rearrange all the stacks to reflect squelch settings */
	for (x = 0; x < DUNGEON_WID; x++)
	{
		for (y = 0; y < DUNGEON_HGT; y++)
		{
			rearrange_stack(y, x);
		}
	}

	return;
}



/*
 * Hack -- initialize the mapping from tvals to typevals.
 * Called in init2.c.
 */
void squelch_init(void)
{
	tv_to_type[TV_SKELETON] = TYPE_MISC;
	tv_to_type[TV_BOTTLE] = TYPE_MISC;
	tv_to_type[TV_JUNK] = TYPE_MISC;
	tv_to_type[TV_SPIKE] = TYPE_MISC;
	tv_to_type[TV_CHEST] = TYPE_MISC;
	tv_to_type[TV_SHOT] = TYPE_AMMO;
	tv_to_type[TV_ARROW] = TYPE_AMMO;
	tv_to_type[TV_BOLT] = TYPE_AMMO;
	tv_to_type[TV_BOW] = TYPE_BOW;
	tv_to_type[TV_DIGGING] = TYPE_WEAPON2;
	tv_to_type[TV_HAFTED] = TYPE_WEAPON2;
	tv_to_type[TV_POLEARM] = TYPE_WEAPON2;
	tv_to_type[TV_SWORD] = TYPE_WEAPON1;
	tv_to_type[TV_BOOTS] = TYPE_BOOTS;
	tv_to_type[TV_GLOVES] = TYPE_GLOVES;
	tv_to_type[TV_HELM] = TYPE_HELM;
	tv_to_type[TV_CROWN] = TYPE_HELM;
	tv_to_type[TV_SHIELD] = TYPE_SHIELD;
	tv_to_type[TV_CLOAK] = TYPE_CLOAK;
	tv_to_type[TV_SOFT_ARMOR] = TYPE_BODY;
	tv_to_type[TV_HARD_ARMOR] = TYPE_BODY;
	tv_to_type[TV_DRAG_ARMOR] = TYPE_BODY;
	tv_to_type[TV_LITE] = TYPE_MISC;
	tv_to_type[TV_AMULET] = TYPE_AMULET;
	tv_to_type[TV_RING] = TYPE_RING;
	tv_to_type[TV_STAFF] = TYPE_STAFF;
	tv_to_type[TV_WAND] = TYPE_WAND;
	tv_to_type[TV_ROD] = TYPE_ROD;
	tv_to_type[TV_SCROLL] = TYPE_SCROLL;
	tv_to_type[TV_POTION] = TYPE_POTION;
	tv_to_type[TV_FLASK] = TYPE_MISC;
	tv_to_type[TV_FOOD] = TYPE_FOOD;
	tv_to_type[TV_MAGIC_BOOK] = TYPE_BOOK;
	tv_to_type[TV_PRAYER_BOOK] = TYPE_BOOK;

	squelch_q_menu.title ="Command:";
	squelch_q_menu.flags = MN_NO_CURSOR ;
	squelch_q_menu.count = N_ELEMENTS(squelch_q_kinds);
	squelch_q_menu.menu_data = squelch_q_kinds;
	squelch_q_menu.cursor = -1;
	menu_set_id(&squelch_q_menu, 'qsql');
	menu_init(&squelch_q_menu, MN_SCROLL, MN_ACT, NULL);

	squelch_q_items.title = "Secondary Squelching Menu";
	squelch_q_items.count = N_ELEMENTS(tvals) - 1;

	/* squelch_q_items.menu_data = tvals; -- not used */
	menu_init2(&squelch_q_items, find_menu_skin(MN_SCROLL), &tval_iter, NULL);

	add_listener(&squelch_q_items.target, &squelch_q_menu.target.self);

	return;
}

