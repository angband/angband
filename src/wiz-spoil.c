/*
 * File: wiz-spoil.c
 * Purpose: Spoiler generation
 *
 * Copyright (c) 1997 Ben Harrison, and others
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
#include "buildid.h"
#include "cmds.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "ui-menu.h"
#include "wizard.h"
#include "z-file.h"


#ifdef ALLOW_SPOILERS


/*
 * The spoiler file being created
 */
static ang_file *fh = NULL;


/*
 * Write out `n' of the character `c' to the spoiler file
 */
static void spoiler_out_n_chars(int n, char c)
{
	while (--n >= 0) file_writec(fh, c);
}

/*
 * Write out `n' blank lines to the spoiler file
 */
static void spoiler_blanklines(int n)
{
	spoiler_out_n_chars(n, '\n');
}

/*
 * Write a line to the spoiler file and then "underline" it with hypens
 */
static void spoiler_underline(const char *str, char c)
{
	text_out("%s", str);
	text_out("\n");
	spoiler_out_n_chars(strlen(str), c);
	text_out("\n");
}



/*
 * Item Spoilers by Ben Harrison (benh@phial.com)
 */


/*
 * The basic items categorized by type
 */
static const grouper group_item[] =
{
	{ TV_SHOT,		"Ammo" },
	{ TV_ARROW,		  NULL },
	{ TV_BOLT,		  NULL },

	{ TV_BOW,		"Bows" },

	{ TV_SWORD,		"Weapons" },
	{ TV_POLEARM,	  NULL },
	{ TV_HAFTED,	  NULL },
	{ TV_DIGGING,	  NULL },

	{ TV_SOFT_ARMOR,	"Armour (Body)" },
	{ TV_HARD_ARMOR,	  NULL },
	{ TV_DRAG_ARMOR,	  NULL },

	{ TV_CLOAK,		"Armour (Misc)" },
	{ TV_SHIELD,	  NULL },
	{ TV_HELM,		  NULL },
	{ TV_CROWN,		  NULL },
	{ TV_GLOVES,	  NULL },
	{ TV_BOOTS,		  NULL },

	{ TV_AMULET,	"Amulets" },
	{ TV_RING,		"Rings" },

	{ TV_SCROLL,	"Scrolls" },
	{ TV_POTION,	"Potions" },
	{ TV_FOOD,		"Food" },

	{ TV_ROD,		"Rods" },
	{ TV_WAND,		"Wands" },
	{ TV_STAFF,		"Staffs" },

	{ TV_MAGIC_BOOK,	"Books (Mage)" },
	{ TV_PRAYER_BOOK,	"Books (Priest)" },

	{ TV_CHEST,		"Chests" },

	{ TV_SPIKE,		"Various" },
	{ TV_LIGHT,		  NULL },
	{ TV_FLASK,		  NULL },
	{ TV_JUNK,		  NULL },
	{ TV_BOTTLE,	  NULL },
	{ TV_SKELETON,	  NULL },

	{ 0, "" }
};





/*
 * Describe the kind
 */
static void kind_info(char *buf, size_t buf_len,
                      char *dam, size_t dam_len,
                      char *wgt, size_t wgt_len,
                      int *lev, s32b *val, int k)
{
	object_kind *k_ptr;

	object_type *i_ptr;
	object_type object_type_body;
	int i;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Prepare a fake item */
	object_prep(i_ptr, &k_info[k], 0, MAXIMISE);

	/* Obtain the "kind" info */
	k_ptr = i_ptr->kind;

	/* Cancel bonuses */
	for (i = 0; i < MAX_PVALS; i++)
		i_ptr->pval[i] = 0;
	i_ptr->to_a = 0;
	i_ptr->to_h = 0;
	i_ptr->to_d = 0;


	/* Level */
	(*lev) = k_ptr->level;

	/* Make known */
	object_notice_everything(i_ptr);

	/* Value */
	(*val) = object_value(i_ptr, 1, FALSE);

	/* Description (too brief) */
	if (buf)
		object_desc(buf, buf_len, i_ptr, ODESC_BASE | ODESC_SPOIL);

	/* Weight */
	if (wgt)
		strnfmt(wgt, wgt_len, "%3d.%d",
				i_ptr->weight / 10, i_ptr->weight % 10);

	/* Hack */
	if (!dam)
		return;

	/* Misc info */
	dam[0] = '\0';

	/* Damage */
	switch (i_ptr->tval)
	{
		/* Bows */
		case TV_BOW:
		{
			break;
		}

		/* Ammo */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			strnfmt(dam, dam_len, "%dd%d", i_ptr->dd, i_ptr->ds);
			break;
		}

		/* Weapons */
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			strnfmt(dam, dam_len, "%dd%d", i_ptr->dd, i_ptr->ds);
			break;
		}

		/* Armour */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			strnfmt(dam, dam_len, "%d", i_ptr->ac);
			break;
		}
	}
}


/*
 * Create a spoiler file for items
 */
static void spoil_obj_desc(const char *fname)
{
	int i, k, s, t, n = 0;

	u16b who[200];

	char buf[1024];

	char wgt[80];
	char dam[80];

	const char *format = "%-51s  %7s%6s%4s%9s\n";

	/* We use either ascii or system-specific encoding */
 	int encoding = (OPT(xchars_to_file)) ? SYSTEM_SPECIFIC : ASCII;

	/* Open the file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg("Cannot create spoiler file.");
		return;
	}


	/* Header */
	file_putf(fh, "Spoiler File -- Basic Items (%s)\n\n\n", buildid);

	/* More Header */
	file_putf(fh, format, "Description", "Dam/AC", "Wgt", "Lev", "Cost");
	file_putf(fh, format, "----------------------------------------",
	        "------", "---", "---", "----");

	/* List the groups */
	for (i = 0; TRUE; i++)
	{
		/* Write out the group title */
		if (group_item[i].name)
		{
			/* Hack -- bubble-sort by cost and then level */
			for (s = 0; s < n - 1; s++)
			{
				for (t = 0; t < n - 1; t++)
				{
					int i1 = t;
					int i2 = t + 1;

					int e1;
					int e2;

					s32b t1;
					s32b t2;

					kind_info(NULL, 0, NULL, 0, NULL, 0, &e1, &t1, who[i1]);
					kind_info(NULL, 0, NULL, 0, NULL, 0, &e2, &t2, who[i2]);

					if ((t1 > t2) || ((t1 == t2) && (e1 > e2)))
					{
						int tmp = who[i1];
						who[i1] = who[i2];
						who[i2] = tmp;
					}
				}
			}

			/* Spoil each item */
			for (s = 0; s < n; s++)
			{
				int e;
				s32b v;

				/* Describe the kind */
				kind_info(buf, sizeof(buf), dam, sizeof(dam), wgt, sizeof(wgt), &e, &v, who[s]);

				/* Dump it */
				x_file_putf(fh, encoding, "  %-51s%7s%6s%4d%9ld\n",
				        buf, dam, wgt, e, (long)(v));
			}

			/* Start a new set */
			n = 0;

			/* Notice the end */
			if (!group_item[i].tval) break;

			/* Start a new set */
			x_file_putf(fh, encoding, "\n\n%s\n\n", group_item[i].name);
		}

		/* Get legal item types */
		for (k = 1; k < z_info->k_max; k++)
		{
			object_kind *k_ptr = &k_info[k];

			/* Skip wrong tval's */
			if (k_ptr->tval != group_item[i].tval) continue;

			/* Hack -- Skip instant-artifacts */
			if (of_has(k_ptr->flags, OF_INSTA_ART)) continue;

			/* Save the index */
			who[n++] = k;
		}
	}


	/* Check for errors */
	if (!file_close(fh))
	{
		msg("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg("Successfully created a spoiler file.");
}



/*
 * Artifact Spoilers by: randy@PICARD.tamu.edu (Randy Hutson)
 *
 * (Mostly) rewritten in 2002 by Andrew Sidwell and Robert Ruehlmann.
 */


/*
 * The artifacts categorized by type
 */
static const grouper group_artifact[] =
{
	{ TV_SWORD,         "Edged Weapons" },
	{ TV_POLEARM,       "Polearms" },
	{ TV_HAFTED,        "Hafted Weapons" },
	{ TV_BOW,           "Bows" },
	{ TV_DIGGING,       "Diggers" },

	{ TV_SOFT_ARMOR,    "Body Armor" },
	{ TV_HARD_ARMOR,    NULL },
	{ TV_DRAG_ARMOR,    NULL },

	{ TV_CLOAK,         "Cloaks" },
	{ TV_SHIELD,        "Shields" },
	{ TV_HELM,          "Helms/Crowns" },
	{ TV_CROWN,         NULL },
	{ TV_GLOVES,        "Gloves" },
	{ TV_BOOTS,         "Boots" },

	{ TV_LIGHT,         "Light Sources" },
	{ TV_AMULET,        "Amulets" },
	{ TV_RING,          "Rings" },

	{ 0, NULL }
};


/*
 * Hack -- Create a "forged" artifact
 */
bool make_fake_artifact(object_type *o_ptr, struct artifact *artifact)
{
	object_kind *kind;

	/* Don't bother with empty artifacts */
	if (!artifact->tval) return FALSE;

	/* Get the "kind" index */
	kind = lookup_kind(artifact->tval, artifact->sval);
	if (!kind) return FALSE;

	/* Create the artifact */
	object_prep(o_ptr, kind, 0, MAXIMISE);

	/* Save the name */
	o_ptr->artifact = artifact;

	/* Extract the fields */
	copy_artifact_data(o_ptr, artifact);

	/* Success */
	o_ptr->ident |= IDENT_FAKE;
	return (TRUE);
}


/*
 * Create a spoiler file for artifacts
 */
static void spoil_artifact(const char *fname)
{
	int i, j;

	object_type *i_ptr;
	object_type object_type_body;

	char buf[1024];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg("Cannot create spoiler file.");
		return;
	}

	/* Dump to the spoiler file */
	text_out_hook = text_out_to_file;
	text_out_file = fh;

	/* Dump the header */
	spoiler_underline(format("Artifact Spoilers for %s", buildid), '=');

	/* List the artifacts by tval */
	for (i = 0; group_artifact[i].tval; i++)
	{
		/* Write out the group title */
		if (group_artifact[i].name)
		{
			spoiler_blanklines(2);
			spoiler_underline(group_artifact[i].name, '=');
			spoiler_blanklines(1);
		}

		/* Now search through all of the artifacts */
		for (j = 1; j < z_info->a_max; ++j)
		{
			artifact_type *a_ptr = &a_info[j];
			char buf[80];

			/* We only want objects in the current group */
			if (a_ptr->tval != group_artifact[i].tval) continue;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Wipe the object */
			object_wipe(i_ptr);

			/* Attempt to "forge" the artifact */
			if (!make_fake_artifact(i_ptr, a_ptr)) continue;

			/* Grab artifact name */
			object_desc(buf, sizeof(buf), i_ptr, ODESC_PREFIX |
				ODESC_COMBAT | ODESC_EXTRA | ODESC_SPOIL);

			/* Print name and underline */
			spoiler_underline(buf, '-');

			/* Write out the artifact description to the spoiler file */
			object_info_spoil(fh, i_ptr, 80);

			/*
			 * Determine the minimum and maximum depths an
			 * artifact can appear, its rarity, its weight, and
			 * its power rating.
			 */
			text_out("\nMin Level %u, Max Level %u, Generation chance %u, Power %d, %d.%d lbs\n",
				a_ptr->alloc_min, a_ptr->alloc_max,
				a_ptr->alloc_prob, object_power(i_ptr, FALSE,
				NULL, TRUE), (a_ptr->weight / 10),
				(a_ptr->weight % 10));

			if (OPT(birth_randarts)) text_out("%s.\n", a_ptr->text);

			/* Terminate the entry */
			spoiler_blanklines(2);
		}
	}

	/* Check for errors */
	if (!file_close(fh))
	{
		msg("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg("Successfully created a spoiler file.");
}





/*
 * Create a spoiler file for monsters
 */
static void spoil_mon_desc(const char *fname)
{
	int i, n = 0;

	char buf[1024];

	char nam[80];
	char lev[80];
	char rar[80];
	char spd[80];
	char ac[80];
	char hp[80];
	char exp[80];

	u16b *who;

	/* We use either ascii or system-specific encoding */
 	int encoding = (OPT(xchars_to_file)) ? SYSTEM_SPECIFIC : ASCII;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */
	x_file_putf(fh, encoding, "Monster Spoilers for %s\n", buildid);
	x_file_putf(fh, encoding, "------------------------------------------\n\n");

	/* Dump the header */
	x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
	        "Name", "Lev", "Rar", "Spd", "Hp", "Ac", "Visual Info");
	x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
	        "----", "---", "---", "---", "--", "--", "-----------");

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monsters (except the ghost) */
	for (i = 1; i < z_info->r_max - 1; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->name) who[n++] = (u16b)i;
	}

	/* Sort the array by dungeon depth of monsters */
	sort(who, n, sizeof(*who), cmp_monsters);

	/* Scan again */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[who[i]];

		const char *name = r_ptr->name;

		/* Get the "name" */
		if (rf_has(r_ptr->flags, RF_QUESTOR))
		{
			strnfmt(nam, sizeof(nam), "[Q] %s", name);
		}
		else if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			strnfmt(nam, sizeof(nam), "[U] %s", name);
		}
		else
		{
			strnfmt(nam, sizeof(nam), "The %s", name);
		}


		/* Level */
		strnfmt(lev, sizeof(lev), "%d", r_ptr->level);

		/* Rarity */
		strnfmt(rar, sizeof(rar), "%d", r_ptr->rarity);

		/* Speed */
		if (r_ptr->speed >= 110)
			strnfmt(spd, sizeof(spd), "+%d", (r_ptr->speed - 110));
		else
			strnfmt(spd, sizeof(spd), "-%d", (110 - r_ptr->speed));

		/* Armor Class */
		strnfmt(ac, sizeof(ac), "%d", r_ptr->ac);

		/* Hitpoints */
		strnfmt(hp, sizeof(hp), "%d", r_ptr->avg_hp);


		/* Experience */
		strnfmt(exp, sizeof(exp), "%ld", (long)(r_ptr->mexp));

		/* Hack -- use visual instead */
		strnfmt(exp, sizeof(exp), "%s '%c'", attr_to_text(r_ptr->d_attr), r_ptr->d_char);

		/* Dump the info */
		x_file_putf(fh, encoding, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
		        nam, lev, rar, spd, hp, ac, exp);
	}

	/* End it */
	file_putf(fh, "\n");

	/* Free the "who" array */
	FREE(who);


	/* Check for errors */
	if (!file_close(fh))
	{
		msg("Cannot close spoiler file.");
		return;
	}

	/* Worked */
	msg("Successfully created a spoiler file.");
}




/*
 * Monster spoilers originally by: smchorse@ringer.cs.utsa.edu (Shawn McHorse)
 */


/*
 * Create a spoiler file for monsters (-SHAWN-)
 */
static void spoil_mon_info(const char *fname)
{
	char buf[1024];
	int i, n;
	u16b *who;
	int count = 0;


	/* Open the file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);
	fh = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fh)
	{
		msg("Cannot create spoiler file.");
		return;
	}

	/* Dump to the spoiler file */
	text_out_hook = text_out_to_file;
	text_out_file = fh;

	/* Dump the header */
	text_out("Monster Spoilers for %s\n", buildid);
	text_out("------------------------------------------\n\n");

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Scan the monsters */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->name) who[count++] = (u16b)i;
	}

	sort(who, count, sizeof(*who), cmp_monsters);

	/*
	 * List all monsters in order (except the ghost).
	 */
	for (n = 0; n < count; n++)
	{
		int r_idx = who[n];
		monster_race *r_ptr = &r_info[r_idx];

		/* Prefix */
		if (rf_has(r_ptr->flags, RF_QUESTOR))
		{
			text_out("[Q] ");
		}
		else if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			text_out("[U] ");
		}
		else
		{
			text_out("The ");
		}

		/* Name */
		text_out("%s  (",  r_ptr->name);	/* ---)--- */

		/* Color */
		text_out(attr_to_text(r_ptr->d_attr));

		/* Symbol --(-- */
		text_out(" '%c')\n", r_ptr->d_char);


		/* Indent */
		text_out("=== ");

		/* Number */
		text_out("Num:%d  ", r_idx);

		/* Level */
		text_out("Lev:%d  ", r_ptr->level);

		/* Rarity */
		text_out("Rar:%d  ", r_ptr->rarity);

		/* Speed */
		if (r_ptr->speed >= 110)
		{
			text_out("Spd:+%d  ", (r_ptr->speed - 110));
		}
		else
		{
			text_out("Spd:-%d  ", (110 - r_ptr->speed));
		}

		/* Hitpoints */
		text_out("Hp:%d  ", r_ptr->avg_hp);

		/* Armor Class */
		text_out("Ac:%d  ", r_ptr->ac);

		/* Experience */
		text_out("Exp:%ld\n", (long)(r_ptr->mexp));

		/* Describe */
		describe_monster(r_idx, TRUE);

		/* Terminate the entry */
		text_out("\n");
	}

	/* Free the "who" array */
	FREE(who);

	/* Check for errors */
	if (!file_close(fh))
	{
		msg("Cannot close spoiler file.");
		return;
	}

	msg("Successfully created a spoiler file.");
}


static void spoiler_menu_act(const char *title, int row)
{
	if (row == 0)
		spoil_obj_desc("obj-desc.spo");
	else if (row == 1)
		spoil_artifact("artifact.spo");
	else if (row == 2)
		spoil_mon_desc("mon-desc.spo");
	else if (row == 3)
		spoil_mon_info("mon-info.spo");

	message_flush();
}

static menu_type *spoil_menu;
static menu_action spoil_actions[] =
{
	{ 0, 0, "Brief Object Info (obj-desc.spo)",		spoiler_menu_act },
	{ 0, 0, "Brief Artifact Info (artifact.spo)",	spoiler_menu_act },
	{ 0, 0, "Brief Monster Info (mon-desc.spo)",	spoiler_menu_act },
	{ 0, 0, "Full Monster Info (mon-info.spo)",		spoiler_menu_act },
};


/*
 * Create Spoiler files
 */
void do_cmd_spoilers(void)
{
	if (!spoil_menu)
	{
		spoil_menu = menu_new_action(spoil_actions, N_ELEMENTS(spoil_actions));
		spoil_menu->selections = lower_case;
		spoil_menu->title = "Create spoilers";
	}

	screen_save();
	clear_from(0);
	menu_layout(spoil_menu, &SCREEN_REGION);
	menu_select(spoil_menu, 0, FALSE);
	screen_load();
}


#endif
