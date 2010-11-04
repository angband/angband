/*
 * File: cmd3.c
 * Purpose: Miscellaneous queries
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
#include "monster/monster.h"
#include "object/inventory.h"
#include "object/tvalsval.h"
#include "squelch.h"

/*
 * Display inventory
 */
void do_cmd_inven(void)
{
	int diff = weight_remaining();

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_INVEN);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the inventory */
	show_inven(OLIST_WEIGHT | OLIST_QUIVER);

	/* Hack -- hide empty slots */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Command: ",
		        p_ptr->total_weight / 10, p_ptr->total_weight % 10,
		        abs(diff) / 10, abs(diff) % 10,
		        (diff < 0 ? "overweight" : "remaining")),
	    0, 0);

	/* Hack -- Get a new command */
	p_ptr->command_new = inkey();

	/* Load screen */
	screen_load();


	/* Hack -- Process "Escape" */
	if (p_ptr->command_new == ESCAPE)
	{
		/* Reset stuff */
		p_ptr->command_new = 0;
	}
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
	/* Hack -- Start in "equipment" mode */
	p_ptr->command_wrk = (USE_EQUIP);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the equipment */
	show_equip(OLIST_WEIGHT);

	/* Hack -- undo the hack above */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt("(Equipment) Command: ", 0, 0);

	/* Hack -- Get a new command */
	p_ptr->command_new = inkey();

	/* Load screen */
	screen_load();


	/* Hack -- Process "Escape" */
	if (p_ptr->command_new == ESCAPE)
	{
		/* Reset stuff */
		p_ptr->command_new = 0;
	}
}


/*
 * Wield or wear a single item from the pack or floor
 */
void wield_item(object_type *o_ptr, int item, int slot)
{
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	cptr fmt;
	char o_name[80];

	bool combined_ammo = FALSE;
	int num = 1;

	/* If we are stacking ammo in the quiver */
	if (obj_is_ammo(o_ptr))
	{
		num = o_ptr->number;
		combined_ammo = object_similar(o_ptr, &p_ptr->inventory[slot]);
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Obtain local object */
	object_copy(i_ptr, o_ptr);

	/* Modify quantity */
	i_ptr->number = num;

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -num);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -num);
		floor_item_optimize(0 - item);
	}

	/* Get the wield slot */
	o_ptr = &p_ptr->inventory[slot];

	if (combined_ammo)
	{
		/* Add the new ammo to the already-quiver-ed ammo */
		object_absorb(o_ptr, i_ptr);
	}
	else 
	{
		/* Take off existing item */
		if (o_ptr->k_idx)
			(void)inven_takeoff(slot, 255);

		/* If we are wielding ammo we may need to "open" the slot by shifting
		 * later ammo up the quiver; this is because we already called the
		 * inven_item_optimize() function. */
		if (slot >= QUIVER_START)
			open_quiver_slot(slot);
	
		/* Wear the new stuff */
		object_copy(o_ptr, i_ptr);

		/* Increment the equip counter by hand */
		p_ptr->equip_cnt++;
	}

	/* Increase the weight */
	p_ptr->total_weight += i_ptr->weight * num;

	/* Do any ID-on-wield */
	object_notice_on_wield(o_ptr);

	/* Where is the item now */
	if (slot == INVEN_WIELD)
		fmt = "You are wielding %s (%c).";
	else if (slot == INVEN_BOW)
		fmt = "You are shooting with %s (%c).";
	else if (slot == INVEN_LIGHT)
		fmt = "Your light source is %s (%c).";
	else if (combined_ammo)
		fmt = "You combine %s in your quiver (%c).";
	else if (slot >= QUIVER_START && slot < QUIVER_END)
		fmt = "You add %s to your quiver (%c).";
	else
		fmt = "You are wearing %s (%c).";

	/* Describe the result */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	message_format(MSG_WIELD, 0, fmt, o_name, index_to_label(slot));

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		message_format(MSG_CURSED, 0, "Oops! It feels deathly cold!");

		/* Sense the object */
		object_notice_curses(o_ptr);
	}

	/* Save quiver size */
	save_quiver_size(p_ptr);

	/* See if we have to overflow the pack */
	pack_overflow();

	/* Recalculate bonuses, torch, mana */
	p_ptr->notice |= PN_SORT_QUIVER;
	p_ptr->update |= (PU_BONUS | PU_TORCH | PU_MANA);
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
}



/*
 * Destroy an item
 */
void do_cmd_destroy(cmd_code code, cmd_arg args[])
{
	int item, amt;

	object_type *o_ptr;

	object_type destroyed_obj;

	char o_name[120];

	item = args[0].item;
	amt = args[1].number;

	/* Destroying squelched items is easy. */
	if (item == ALL_SQUELCHED)
	{
		squelch_items();
		return;
	}

	if (!item_is_available(item, NULL, USE_INVEN | USE_EQUIP | USE_FLOOR))
	{
		msg_print("You do not have that item to destroy it.");
		return;
	}

	o_ptr = object_from_item_idx(item);

	/* Can't destroy cursed items we're wielding. */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		msg_print("You cannot destroy the cursed item.");
		return;
	}	

	/* Describe the destroyed object by taking a copy with the right "amt" */
	object_copy_amt(&destroyed_obj, o_ptr, amt);
	object_desc(o_name, sizeof(o_name), &destroyed_obj,
				ODESC_PREFIX | ODESC_FULL);

	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr))
	{
		/* Message */
		msg_format("You cannot destroy %s.", o_name);
		object_notice_indestructible(o_ptr);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP);

		/* Done */
		return;
	}

	/* Message */
	message_format(MSG_DESTROY, 0, "You destroy %s.", o_name);

	/* Reduce the charges of rods/wands/staves */
	reduce_charges(o_ptr, amt);

	/* Eliminate the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Eliminate the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -amt);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}


void textui_cmd_destroy(void)
{
	int item, amt;

	object_type *o_ptr;

	object_type obj_to_destroy;

	char result;
	char o_name[120];
	char out_val[160];

	cptr q, s;

	/* Get an item */
	q = "Destroy which item? ";
	s = "You have nothing to destroy.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP | USE_FLOOR | CAN_SQUELCH))) return;

	/* Deal with squelched items */
	if (item == ALL_SQUELCHED)
	{
		cmd_insert(CMD_DESTROY, item, 0);
		return;
	}
	
	o_ptr = object_from_item_idx(item);

	/* Ask if player would prefer squelching instead of destruction */

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);
	if (amt <= 0) return;

	/* Describe the destroyed object by taking a copy with the right "amt" */
	object_copy_amt(&obj_to_destroy, o_ptr, amt);
	object_desc(o_name, sizeof(o_name), &obj_to_destroy,
				ODESC_PREFIX | ODESC_FULL);

	/* Verify destruction */
	strnfmt(out_val, sizeof(out_val), "Really destroy %s? ", o_name);
	
	result = get_char(out_val, "yns", 3, 'n');

	if (result == 'y')
		cmd_insert(CMD_DESTROY, item, amt);
	else if (result == 's' && squelch_interactive(o_ptr))
	{
		p_ptr->notice |= PN_SQUELCH;

		/* If the item is not equipped, we can rely on it being dropped and */
		/* ignored, otherwise we should continue on to check if we should */
		/* still destroy it. */
		if (item < INVEN_WIELD) return;
	}
}

void refill_lamp(object_type *j_ptr, object_type *o_ptr, int item)
{
	/* Refuel */
	j_ptr->timeout += o_ptr->timeout ? o_ptr->timeout : o_ptr->pval;

	/* Message */
	msg_print("You fuel your lamp.");

	/* Comment */
	if (j_ptr->timeout >= FUEL_LAMP)
	{
		j_ptr->timeout = FUEL_LAMP;
		msg_print("Your lamp is full.");
	}

	/* Refilled from a lantern */
	if (o_ptr->sval == SV_LIGHT_LANTERN)
	{
		/* Unstack if necessary */
		if (o_ptr->number > 1)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Obtain a local object */
			object_copy(i_ptr, o_ptr);

			/* Modify quantity */
			i_ptr->number = 1;

			/* Remove fuel */
			i_ptr->timeout = 0;

			/* Unstack the used item */
			o_ptr->number--;
			p_ptr->total_weight -= i_ptr->weight;

			/* Carry or drop */
			if (item >= 0)
				item = inven_carry(p_ptr, i_ptr);
			else
				drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, FALSE);
		}

		/* Empty a single lantern */
		else
		{
			/* No more fuel */
			o_ptr->timeout = 0;
		}

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN);
	}

	/* Refilled from a flask */
	else
	{
		/* Decrease the item (from the pack) */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		}

		/* Decrease the item (from the floor) */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Redraw stuff */
	p_ptr->redraw |= (PR_EQUIP);
}


void refuel_torch(object_type *j_ptr, object_type *o_ptr, int item)
{
	bitflag f[OF_SIZE];
	bitflag g[OF_SIZE];

	/* Refuel */
	j_ptr->timeout += o_ptr->timeout + 5;

	/* Message */
	msg_print("You combine the torches.");

	/* Transfer the LIGHT flag if refuelling from a torch with it to one
	   without it */
	object_flags(o_ptr, f);
	object_flags(j_ptr, g);
	if (of_has(f, OF_LIGHT) && !of_has(g, OF_LIGHT))
	{
		of_on(j_ptr->flags, OF_LIGHT);
		if (!j_ptr->name2) j_ptr->name2 = EGO_BRIGHTNESS;
		msg_print("Your torch shines further!");
	}

	/* Over-fuel message */
	if (j_ptr->timeout >= FUEL_TORCH)
	{
		j_ptr->timeout = FUEL_TORCH;
		msg_print("Your torch is fully fueled.");
	}

	/* Refuel message */
	else
	{
		msg_print("Your torch glows more brightly.");
	}

	/* Decrease the item (from the pack) */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Decrease the item (from the floor) */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Redraw stuff */
	p_ptr->redraw |= (PR_EQUIP);
}






/*
 * Target command
 */
void do_cmd_target(void)
{
	if (target_set_interactive(TARGET_KILL, -1, -1))
		msg_print("Target Selected.");
	else
		msg_print("Target Aborted.");
}


void do_cmd_target_closest(void)
{
	target_set_closest(TARGET_KILL);
}


/*
 * Look command
 */
void do_cmd_look(void)
{
	/* Look around */
	if (target_set_interactive(TARGET_LOOK, -1, -1))
	{
		msg_print("Target Selected.");
	}
}



/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate(void)
{
	int dir, y1, x1, y2, x2;

	char tmp_val[80];

	char out_val[160];


	/* Start at current panel */
	y1 = Term->offset_y;
	x1 = Term->offset_x;

	/* Show panels until done */
	while (1)
	{
		/* Get the current panel */
		y2 = Term->offset_y;
		x2 = Term->offset_x;
		
		/* Describe the location */
		if ((y2 == y1) && (x2 == x1))
		{
			tmp_val[0] = '\0';
		}
		else
		{
			strnfmt(tmp_val, sizeof(tmp_val), "%s%s of",
			        ((y2 < y1) ? " north" : (y2 > y1) ? " south" : ""),
			        ((x2 < x1) ? " west" : (x2 > x1) ? " east" : ""));
		}

		/* Prepare to ask which way to look */
		strnfmt(out_val, sizeof(out_val),
		        "Map sector [%d,%d], which is%s your sector.  Direction?",
		        (y2 / PANEL_HGT), (x2 / PANEL_WID), tmp_val);

		/* More detail */
		if (OPT(center_player))
		{
			strnfmt(out_val, sizeof(out_val),
		        	"Map sector [%d(%02d),%d(%02d)], which is%s your sector.  Direction?",
		        	(y2 / PANEL_HGT), (y2 % PANEL_HGT),
		        	(x2 / PANEL_WID), (x2 % PANEL_WID), tmp_val);
		}

		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			char command;

			/* Get a command (or Cancel) */
			if (!get_com(out_val, &command)) break;

			/* Extract direction */
			dir = target_dir(command);

			/* Error */
			if (!dir) bell("Illegal direction for locate!");
		}

		/* No direction */
		if (!dir) break;

		/* Apply the motion */
		change_panel(dir);

		/* Handle stuff */
		handle_stuff();
	}

	/* Verify panel */
	verify_panel();
}






/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static cptr ident_info[] =
{
	" :A dark grid",
	"!:A potion (or oil)",
	"\":An amulet (or necklace)",
	"#:A wall (or secret door)",
	"$:Treasure (gold or gems)",
	"%:A vein (magma or quartz)",
	/* "&:unused", */
	"':An open door",
	"(:Soft armor",
	"):A shield",
	"*:A vein with treasure",
	"+:A closed door",
	",:Food (or mushroom patch)",
	"-:A wand (or rod)",
	".:Floor",
	"/:A polearm (Axe/Pike/etc)",
	/* "0:unused", */
	"1:Entrance to General Store",
	"2:Entrance to Armory",
	"3:Entrance to Weaponsmith",
	"4:Entrance to Temple",
	"5:Entrance to Alchemy shop",
	"6:Entrance to Magic store",
	"7:Entrance to Black Market",
	"8:Entrance to your home",
	/* "9:unused", */
	"::Rubble",
	";:A glyph of warding",
	"<:An up staircase",
	"=:A ring",
	">:A down staircase",
	"?:A scroll",
	"@:You",
	"A:Angel",
	"B:Bird",
	"C:Canine",
	"D:Ancient Dragon/Wyrm",
	"E:Elemental",
	"F:Dragon Fly",
	"G:Ghost",
	"H:Hybrid",
	"I:Insect",
	"J:Snake",
	"K:Killer Beetle",
	"L:Lich",
	"M:Multi-Headed Reptile",
	/* "N:unused", */
	"O:Ogre",
	"P:Giant Humanoid",
	"Q:Quylthulg (Pulsing Flesh Mound)",
	"R:Reptile/Amphibian",
	"S:Spider/Scorpion/Tick",
	"T:Troll",
	"U:Major Demon",
	"V:Vampire",
	"W:Wight/Wraith/etc",
	"X:Xorn/Xaren/etc",
	"Y:Yeti",
	"Z:Zephyr Hound",
	"[:Hard armor",
	"\\:A hafted weapon (mace/whip/etc)",
	"]:Misc. armor",
	"^:A trap",
	"_:A staff",
	/* "`:unused", */
	"a:Ant",
	"b:Bat",
	"c:Centipede",
	"d:Dragon",
	"e:Floating Eye",
	"f:Feline",
	"g:Golem",
	"h:Hobbit/Elf/Dwarf",
	"i:Icky Thing",
	"j:Jelly",
	"k:Kobold",
	"l:Louse",
	"m:Mold",
	"n:Naga",
	"o:Orc",
	"p:Person/Human",
	"q:Quadruped",
	"r:Rodent",
	"s:Skeleton",
	"t:Townsperson",
	"u:Minor Demon",
	"v:Vortex",
	"w:Worm/Worm-Mass",
	/* "x:unused", */
	"y:Yeek",
	"z:Zombie/Mummy",
	"{:A missile (arrow/bolt/shot)",
	"|:An edged weapon (sword/dagger/etc)",
	"}:A launcher (bow/crossbow/sling)",
	"~:A tool (or miscellaneous item)",
	NULL
};



/*
 * Sorting hook -- Comp function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform on "u".
 */
bool ang_sort_comp_hook(const void *u, const void *v, int a, int b)
{
	const u16b *who = (const u16b *)(u);
	const u16b *why = (const u16b *)(v);

	int w1 = who[a];
	int w2 = who[b];

	int z1, z2;


	/* Sort by player kills */
	if (*why >= 4)
	{
		/* Extract player kills */
		z1 = l_list[w1].pkills;
		z2 = l_list[w2].pkills;

		/* Compare player kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by total kills */
	if (*why >= 3)
	{
		/* Extract total kills */
		z1 = l_list[w1].tkills;
		z2 = l_list[w2].tkills;

		/* Compare total kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster level */
	if (*why >= 2)
	{
		/* Extract levels */
		z1 = r_info[w1].level;
		z2 = r_info[w2].level;

		/* Compare levels */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster experience */
	if (*why >= 1)
	{
		/* Extract experience */
		z1 = r_info[w1].mexp;
		z2 = r_info[w2].mexp;

		/* Compare experience */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Compare indexes */
	return (w1 <= w2);
}


/*
 * Sorting hook -- Swap function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform.
 */
void ang_sort_swap_hook(void *u, void *v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b holder;

	/* Unused parameter */
	(void)v;

	/* Swap */
	holder = who[a];
	who[a] = who[b];
	who[b] = holder;
}


/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "multiple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 *
 * The responses may be sorted in several ways, see below.
 *
 * Note that the player ghosts are ignored, since they do not exist.
 */
void do_cmd_query_symbol(void)
{
	int i, n, r_idx;
	char sym;
	char buf[128];

	ui_event_data query;

	bool all = FALSE;
	bool uniq = FALSE;
	bool norm = FALSE;

	bool recall = FALSE;

	u16b why = 0;
	u16b *who;


	/* Get a character, or abort */
	if (!get_com("Enter character to be identified, or control+[ANU]: ", &sym)) return;

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (sym == KTRL('A'))
	{
		all = TRUE;
		my_strcpy(buf, "Full monster list.", sizeof(buf));
	}
	else if (sym == KTRL('U'))
	{
		all = uniq = TRUE;
		my_strcpy(buf, "Unique monster list.", sizeof(buf));
	}
	else if (sym == KTRL('N'))
	{
		all = norm = TRUE;
		my_strcpy(buf, "Non-unique monster list.", sizeof(buf));
	}
	else if (ident_info[i])
	{
		strnfmt(buf, sizeof(buf), "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		strnfmt(buf, sizeof(buf), "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	prt(buf, 0, 0);


	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Collect matching monsters */
	for (n = 0, i = 1; i < z_info->r_max - 1; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Nothing to recall */
		if (!OPT(cheat_know) && !l_ptr->sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && rf_has(r_ptr->flags, RF_UNIQUE)) continue;

		/* Require unique monsters if needed */
		if (uniq && !rf_has(r_ptr->flags, RF_UNIQUE)) continue;

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n)
	{
		/* XXX XXX Free the "who" array */
		FREE(who);

		return;
	}

	/* Buttons */
	button_add("[y]", 'y');
	button_add("[k]", 'k');
	/* Don't collide with the repeat button */
	button_add("[n]", 'q'); 
	redraw_stuff();

	/* Prompt */
	put_str("Recall details? (y/k/n): ", 0, 40);

	/* Query */
	query = inkey_ex();

	/* Restore */
	prt(buf, 0, 0);

	/* Buttons */
	button_kill('y');
	button_kill('k');
	button_kill('q');
	redraw_stuff();

	/* Interpret the response */
	if (query.key == 'k')
	{
		/* Sort by kills (and level) */
		why = 4;
	}
	else if (query.key == 'y' || query.key == 'p')
	{
		/* Sort by level; accept 'p' as legacy */
		why = 2;
	}
	else
	{
		/* Any unsupported response is "nope, no history please" */
	
		/* XXX XXX Free the "who" array */
		FREE(who);

		return;
	}


	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_hook;
	ang_sort_swap = ang_sort_swap_hook;

	/* Sort the array */
	ang_sort(who, &why, n);

	/* Start at the end */
	i = n - 1;

	/* Button */
	button_add("[r]", 'r');
	button_add("[-]", '-');
	button_add("[+]", '+');
	redraw_stuff();

	/* Scan the monster memory */
	while (1)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save screen */
				screen_save();

				/* Recall on screen */
				screen_roff(who[i]);

				/* Hack -- Complete the prompt (again) */
				Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey_ex();

			/* Unrecall */
			if (recall)
			{
				/* Load screen */
				screen_load();
			}

			/* Normal commands */
			if (query.key != 'r') break;

			/* Toggle recall */
			recall = !recall;
		}

		/* Stop scanning */
		if (query.key == ESCAPE) break;

		/* Move to "prev" monster */
		if (query.key == '-')
		{
			if (++i == n)
				i = 0;
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
				i = n - 1;
		}
	}

	/* Button */
	button_kill('r');
	button_kill('-');
	button_kill('+');
	redraw_stuff();

	/* Re-display the identity */
	prt(buf, 0, 0);

	/* Free the "who" array */
	FREE(who);
}

/* Centers the map on the player */
void do_cmd_center_map(void)
{
	center_panel();
}
