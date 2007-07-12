/*
 * File: obj-info.c
 * Purpose: Object description code.
 *
 * Copyright (c) 2002-2007 Andrew Sidwell, Robert Ruehlmann
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
#include "effects.h"
#include "cmds.h"


/* TRUE if a paragraph break should be output before next p_text_out() */
static bool new_paragraph = FALSE;


static void p_text_out(cptr str)
{
	if (new_paragraph)
	{
		text_out("\n\n");
		new_paragraph = FALSE;
	}

	text_out(str);
}


static void output_list(const char *list[], int num)
{
	int i;
	const char *conjunction = "and ";

	if (num < 0)
	{
		num = -num;
		conjunction = "or ";
	}

	for (i = 0; i < num; i++)
	{
        if (i)
		{
			if (num > 2)
				text_out(", ");
			else
				text_out(" ");

			if (i == num - 1)
				text_out(conjunction);
		}

		text_out(list[i]);
	}
}


static void output_desc_list(cptr intro, cptr list[], int n)
{
	if (n != 0)
	{
		/* Output intro */
		p_text_out(intro);

		/* Output list */
		output_list(list, n);

		/* Output end */
		text_out(".  ");
	}
}



/*
 * Describe stat modifications.
 */
static bool describe_stats(const object_type *o_ptr, u32b f1)
{
	cptr descs[16];
	int cnt = 0;
	bool prev = FALSE;

	int pval = (o_ptr->pval > 0 ? o_ptr->pval : -o_ptr->pval);
	const char *what = (o_ptr->pval > 0) ? "increases" : "decreases";
	byte col = (o_ptr->pval > 0) ? TERM_L_GREEN : TERM_RED;


	/* No pval?  Forget it! */
	if (!o_ptr->pval) return FALSE;

	/* Collect stat bonuses */
	cnt = 0;
	if (f1 & (TR1_STR)) descs[cnt++] = stat_names_full[A_STR];
	if (f1 & (TR1_INT)) descs[cnt++] = stat_names_full[A_INT];
	if (f1 & (TR1_WIS)) descs[cnt++] = stat_names_full[A_WIS];
	if (f1 & (TR1_DEX)) descs[cnt++] = stat_names_full[A_DEX];
	if (f1 & (TR1_CON)) descs[cnt++] = stat_names_full[A_CON];
	if (f1 & (TR1_CHR)) descs[cnt++] = stat_names_full[A_CHR];

    /* Shorten to "all stats" if appropriate */
	if (cnt == A_MAX)
	{
		p_text_out(format("It %s all your stats", what));
		cnt = 0;
		prev = TRUE;
	}

	/* Collect secondry bonuses */
	if (f1 & (TR1_STEALTH)) descs[cnt++] = "stealth";
	if (f1 & (TR1_SEARCH))  descs[cnt++] = "searching";
	if (f1 & (TR1_INFRA))   descs[cnt++] = "infravision";
	if (f1 & (TR1_TUNNEL))  descs[cnt++] = "tunneling";
	if (f1 & (TR1_SPEED))   descs[cnt++] = "speed";
	if (f1 & (TR1_BLOWS))   descs[cnt++] = "attack speed";
	if (f1 & (TR1_SHOTS))   descs[cnt++] = "shooting speed";
	if (f1 & (TR1_MIGHT))   descs[cnt++] = "shooting power";

	if (cnt)
	{
		if (prev)
			text_out(" and your ");
		else
			p_text_out(format("It %s your ", what));

		/* Output list */
		output_list(descs, cnt);
		prev = TRUE;
	}

	/* Output end */
	if (prev)
	{
		text_out(" by ");
		text_out_c(col, "%i", pval);
		text_out(".  ");
	}

	return prev;
}


/*
 * Describe the special slays and executes of an item.
 */
static bool describe_slay(const object_type *o_ptr, u32b f1)
{
	cptr slays[8], execs[3];
	int slcnt = 0, excnt = 0;
	bool prev = FALSE;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect brands */
	if (f1 & (TR1_SLAY_ANIMAL)) slays[slcnt++] = "animals";
	if (f1 & (TR1_SLAY_ORC))    slays[slcnt++] = "orcs";
	if (f1 & (TR1_SLAY_TROLL))  slays[slcnt++] = "trolls";
	if (f1 & (TR1_SLAY_GIANT))  slays[slcnt++] = "giants";

	/* Dragon slay/execute */
	if (f1 & TR1_KILL_DRAGON)
		execs[excnt++] = "dragons";
	else if (f1 & TR1_SLAY_DRAGON)
		slays[slcnt++] = "dragons";

	/* Demon slay/execute */
	if (f1 & TR1_KILL_DEMON)
		execs[excnt++] = "demons";
	else if (f1 & TR1_SLAY_DEMON)
		slays[slcnt++] = "demons";

	/* Undead slay/execute */
	if (f1 & TR1_KILL_UNDEAD)
		execs[excnt++] = "undead";
	else if (f1 & TR1_SLAY_UNDEAD)
		slays[slcnt++] = "undead";

	if (f1 & (TR1_SLAY_EVIL)) slays[slcnt++] = "all evil creatures";

	if (slcnt)
	{
		p_text_out("It slays ");
		output_list(slays, slcnt);
		prev = TRUE;
	}

	if (excnt)
	{
		/* Intro */
		if (prev) text_out(", and is especially deadly against ");
		else p_text_out("It is especially deadly against ");

		/* List */
		output_list(execs, excnt);
		prev = TRUE;
	}

	/* Output end */
	if (prev) text_out(".  ");

	/* We are done here */
	return prev;
}


/*
 * Describe elemental brands.
 */
static bool describe_brand(const object_type *o_ptr, u32b f1)
{
	cptr descs[5];
	int cnt = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect brands */
	if (f1 & (TR1_BRAND_ACID)) descs[cnt++] = "acid";
	if (f1 & (TR1_BRAND_ELEC)) descs[cnt++] = "electricity";
	if (f1 & (TR1_BRAND_FIRE)) descs[cnt++] = "fire";
	if (f1 & (TR1_BRAND_COLD)) descs[cnt++] = "frost";
	if (f1 & (TR1_BRAND_POIS)) descs[cnt++] = "poison";

	/* Describe brands */
	output_desc_list("It is branded with ", descs, cnt);

	/* We are done here */
	return (cnt ? TRUE : FALSE);
}


/*
 * Describe immunities granted by an object.
 */
static bool describe_immune(const object_type *o_ptr, u32b f2, u32b f3)
{
	cptr vp[26];
	int vn;
	bool prev = FALSE;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect immunities */
	vn = 0;
	if (f2 & (TR2_IM_ACID))  vp[vn++] = "acid";
	if (f2 & (TR2_IM_ELEC))  vp[vn++] = "lightning";
	if (f2 & (TR2_IM_FIRE))  vp[vn++] = "fire";
	if (f2 & (TR2_IM_COLD))  vp[vn++] = "cold";
	if (f3 & (TR3_FREE_ACT)) vp[vn++] = "paralysis";

	/* Describe immunities */
	if (vn)
	{
		text_out("It provides immunity to ");

		output_list(vp, vn);
		prev = TRUE;
	}

	/* Collect resistances */
	vn = 0;
	if ((f2 & (TR2_RES_ACID)) && !(f2 & (TR2_IM_ACID)))
		vp[vn++] = "acid";
	if ((f2 & (TR2_RES_ELEC)) && !(f2 & (TR2_IM_ELEC)))
		vp[vn++] = "lightning";
	if ((f2 & (TR2_RES_FIRE)) && !(f2 & (TR2_IM_FIRE)))
		vp[vn++] = "fire";
	if ((f2 & (TR2_RES_COLD)) && !(f2 & (TR2_IM_COLD)))
		vp[vn++] = "cold";

	if (f2 & (TR2_RES_POIS))  vp[vn++] = "poison";
	if (f2 & (TR2_RES_FEAR))  vp[vn++] = "fear";
	if (f2 & (TR2_RES_LITE))  vp[vn++] = "light";
	if (f2 & (TR2_RES_DARK))  vp[vn++] = "dark";
	if (f2 & (TR2_RES_BLIND)) vp[vn++] = "blindness";
	if (f2 & (TR2_RES_CONFU)) vp[vn++] = "confusion";
	if (f2 & (TR2_RES_SOUND)) vp[vn++] = "sound";
	if (f2 & (TR2_RES_SHARD)) vp[vn++] = "shards";
	if (f2 & (TR2_RES_NEXUS)) vp[vn++] = "nexus" ;
	if (f2 & (TR2_RES_NETHR)) vp[vn++] = "nether";
	if (f2 & (TR2_RES_CHAOS)) vp[vn++] = "chaos";
	if (f2 & (TR2_RES_DISEN)) vp[vn++] = "disenchantment";
	if (f3 & (TR3_HOLD_LIFE)) vp[vn++] = "life draining";

	if (vn)
	{
		if (prev)
			text_out(", and provides resistance to ");
		else
			p_text_out("It provides resistance to ");

		/* Output list */
		output_list(vp, vn);
		prev = TRUE;
	}

	/* Parting words */
	if (prev) text_out(".  ");

	return prev;
}


/*
 * Describe 'ignores' of an object.
 */
static bool describe_ignores(const object_type *o_ptr, u32b f3)
{
	cptr list[4];
	int n = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect the ignores */
	if (f3 & (TR3_IGNORE_ACID)) list[n++] = "acid";
	if (f3 & (TR3_IGNORE_ELEC)) list[n++] = "electricity";
	if (f3 & (TR3_IGNORE_FIRE)) list[n++] = "fire";
	if (f3 & (TR3_IGNORE_COLD)) list[n++] = "cold";

	/* Describe ignores */
	if (n == 4)
		p_text_out("It cannot be harmed by the elements.  ");
	else
		output_desc_list("It cannot be harmed by ", list, -n);

	return (n ? TRUE : FALSE);
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(const object_type *o_ptr, u32b f2)
{
	cptr list[A_MAX];
	int n = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect the sustains */
	if (f2 & (TR2_SUST_STR)) list[n++] = stat_names_full[A_STR];
	if (f2 & (TR2_SUST_INT)) list[n++] = stat_names_full[A_INT];
	if (f2 & (TR2_SUST_WIS)) list[n++] = stat_names_full[A_WIS];
	if (f2 & (TR2_SUST_DEX)) list[n++] = stat_names_full[A_DEX];
	if (f2 & (TR2_SUST_CON)) list[n++] = stat_names_full[A_CON];
	if (f2 & (TR2_SUST_CHR)) list[n++] = stat_names_full[A_CHR];

	/* Describe immunities */
	if (n == A_MAX)
		p_text_out("It sustains all your stats.  ");
	else
		output_desc_list("It sustains your ", list, n);

	/* We are done here */
	return (n ? TRUE : FALSE);
}


/*
 * Describe miscellaneous powers such as see invisible, free action,
 * permanent light, etc; also note curses and penalties.
 */
static bool describe_misc_magic(const object_type *o_ptr, u32b f3)
{
	cptr good[6], bad[4];
	int gc = 0, bc = 0;
	bool something = FALSE;

	/* Collect stuff which can't be categorized */
	if (f3 & (TR3_BLESSED))     good[gc++] = "is blessed by the gods";
	if (f3 & (TR3_IMPACT))      good[gc++] = "creates earthquakes on impact";
	if (f3 & (TR3_SLOW_DIGEST)) good[gc++] = "slows your metabolism";
	if (f3 & (TR3_FEATHER))     good[gc++] = "makes you fall like a feather";
	if (f3 & (TR3_REGEN))       good[gc++] = "speeds your regeneration";

	if ((o_ptr->tval == TV_LITE) || (f3 & TR3_LITE))
		good[gc++] = "lights the dungeon around you";
	if (f3 & TR3_NO_FUEL)       good[gc++] = "does not require fuel";

	/* Describe */
	if (gc)
	{
		output_desc_list("It ", good, gc);
		something = TRUE;
	}


	/* Collect granted powers */
	gc = 0;
	if (f3 & (TR3_TELEPATHY)) good[gc++] = "the power of telepathy";
	if (f3 & (TR3_SEE_INVIS)) good[gc++] = "the ability to see invisible things";

	/* Collect penalties */
	if (f3 & (TR3_AGGRAVATE)) bad[bc++] = "aggravates creatures around you";
	if (f3 & (TR3_DRAIN_EXP)) bad[bc++] = "drains experience";
	if (f3 & (TR3_TELEPORT))  bad[bc++] = "induces random teleportation";

	/* Deal with cursed stuff */
	if (cursed_p(o_ptr))
	{
		if (f3 & (TR3_PERMA_CURSE)) bad[bc++] = "is permanently cursed";
		else if (f3 & (TR3_HEAVY_CURSE)) bad[bc++] = "is heavily cursed";
		else if (object_known_p(o_ptr)) bad[bc++] = "is cursed";
	}

	/* Describe */
	if (gc)
	{
		/* Output intro */
		p_text_out("It grants you ");

		/* Output list */
		output_list(good, gc);

		/* Output end (if needed) */
		if (!bc) p_text_out(".  ");
	}

	if (bc)
	{
		/* Output intro */
		if (gc) p_text_out(", but it also ");
		else p_text_out("It ");

		/* Output list */
		output_list(bad, bc);

		/* Output end */
		p_text_out(".  ");
	}

	/* Return "something" */
	return (gc || bc) ? TRUE : FALSE;
}


/*
 * Describe an object's activation, if any.
 */
static bool describe_activation(const object_type *o_ptr, u32b f3)
{
	const object_kind *k_ptr = &k_info[o_ptr->k_idx];
	const char *desc;

	int effect, base, dice, sides;
	char temp[] = "x";

	if (o_ptr->name1)
	{
		const artifact_type *a_ptr = &a_info[o_ptr->name1];
		if (!object_known_p(o_ptr)) return FALSE;

		effect = a_ptr->effect;
		base = a_ptr->time_base;
		dice = a_ptr->time_dice;
		sides = a_ptr->time_sides;
	}
	else
	{
        if (!object_aware_p(o_ptr)) return FALSE;
        
		effect = k_ptr->effect;
		base = k_ptr->time_base;
		dice = k_ptr->time_dice;
		sides = k_ptr->time_sides;
	}

	/* Forget it without an effect */
	if (!effect) return FALSE;

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	p_text_out("When ");

	if (f3 & TR3_ACTIVATE)
		text_out("activated");
	else if (effect_aim(effect))
		text_out("aimed");
	else if (o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION)
		text_out("ingested");
	else if (o_ptr->tval == TV_SCROLL)
	    text_out("read");
	else
	    text_out("used");

	text_out(", it ");

	/* Print a colourised description */
	do
	{
		temp[0] = *desc;

		if (isdigit((unsigned char) *desc) || isdigit((unsigned char) *(desc + 1)))
			text_out_c(TERM_L_GREEN, temp);
		else
			text_out(temp);
	} while (*desc++);

	text_out(".  ");

	if (base || dice || sides)
	{
		/* Some artifacts can be activated */
		text_out("When it is used, it takes ");

		/* Output the number of turns */
		if (dice && dice != 1)
		    text_out_c(TERM_L_GREEN, "%d", dice);

		if (sides)
		    text_out_c(TERM_L_GREEN, "d%d", sides);

		if (base)
		{
			if (sides) text_out_c(TERM_L_GREEN, "+");
		    text_out_c(TERM_L_GREEN, "%d", base);
		}

		text_out(" turns to recharge.  ");
	}

	/* No activation */
	return (FALSE);
}


/*
 * Output object information
 */
bool object_info_out(const object_type *o_ptr)
{
	u32b f1, f2, f3;
	bool something = FALSE;

	/* Grab the object flags */
	object_info_out_flags(o_ptr, &f1, &f2, &f3);


	/* New para */
	new_paragraph = TRUE;

	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval)
	{
		p_text_out("It provides nourishment for about ");
		text_out_c(TERM_L_GREEN, "%d", o_ptr->pval / 2);
		text_out(" turns under normal conditions.  ");
	}


	/* Describe the object */
	new_paragraph = TRUE;
	if (describe_stats(o_ptr, f1)) something = TRUE;
	if (describe_slay(o_ptr, f1)) something = TRUE;
	if (describe_brand(o_ptr, f1)) something = TRUE;
	if (describe_immune(o_ptr, f2, f3)) something = TRUE;
	if (describe_sustains(o_ptr, f2)) something = TRUE;
	if (describe_misc_magic(o_ptr, f3)) something = TRUE;
	if (describe_activation(o_ptr, f3)) something = TRUE;
	if (describe_ignores(o_ptr, f3)) something = TRUE;

	/* Unknown extra powers (ego-item with random extras or artifact) */
	if (object_known_p(o_ptr) && (!(o_ptr->ident & IDENT_MENTAL)) &&
	    ((o_ptr->xtra1) || artifact_p(o_ptr)))
	{
		/* Hack -- Put this in a separate paragraph if screen dump */
		if (text_out_hook == text_out_to_screen)
			new_paragraph = TRUE;

		p_text_out("It might have hidden powers.");
		something = TRUE;
	}

	/* We are done. */
	return something;
}


/*
 * Header for additional information when printing to screen.
 *
 * Return TRUE if an object description was displayed.
 */
static bool screen_out_head(const object_type *o_ptr)
{
	char *o_name;
	int name_size = Term->wid;

	/* Allocate memory to the size of the screen */
	o_name = C_RNEW(name_size, char);

	/* Description */
	object_desc(o_name, name_size, o_ptr, TRUE, 3);

	/* Print, in colour */
	text_out_c(TERM_YELLOW, format("%^s", o_name));

	/* Free up the memory */
	FREE(o_name);

	/* Display the known artifact description */
	if (!adult_randarts && o_ptr->name1 &&
	    object_known_p(o_ptr) && a_info[o_ptr->name1].text)
	{
		p_text_out(a_text + a_info[o_ptr->name1].text);
	}

	/* Display the known object description */
	else if (object_aware_p(o_ptr) || object_known_p(o_ptr))
	{
		if (k_info[o_ptr->k_idx].text)
			p_text_out(k_text + k_info[o_ptr->k_idx].text);

		/* Display an additional ego-item description */
		if (o_ptr->name2 && object_known_p(o_ptr) && e_info[o_ptr->name2].text)
			p_text_out(e_text + e_info[o_ptr->name2].text);
	}

	else
	{
		return FALSE;
	}

	return TRUE;
}


/*
 * Place an item description on the screen.
 */
void object_info_screen(const object_type *o_ptr)
{
	bool has_description, has_info;

	/* Redirect output to the screen */
	text_out_hook = text_out_to_screen;

	/* Save the screen */
	screen_save();

	new_paragraph = TRUE;
	has_description = screen_out_head(o_ptr);
	object_info_out_flags = object_flags_known;

	/* Dump the info */
	has_info = object_info_out(o_ptr);

	new_paragraph = TRUE;
	if (!object_known_p(o_ptr))
		p_text_out("This item has not been identified.");
	else if (!has_description && !has_info)
		p_text_out("This item does not seem to possess any special abilities.");

	new_paragraph = TRUE;
	text_out_c(TERM_L_BLUE, "\n\n[Press any key to continue]\n");

	/* Wait for input */
	(void)anykey();

	/* Load the screen */
	screen_load();

	/* Hack -- Browse book, then prompt for a command */
	if (o_ptr->tval == cp_ptr->spell_book)
	{
		/* Call the aux function */
		do_cmd_browse_aux(o_ptr);
	}
}
