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
 * list[] and mult[] must be > 11 in size
 */
static int collect_slays(cptr desc[], int mult[], u32b f1)
{
	int cnt = 0;

	/* Collect slays */
	if (f1 & TR1_SLAY_ANIMAL) { mult[cnt] = 2; desc[cnt++] = "animals"; }
	if (f1 & TR1_SLAY_EVIL)   { mult[cnt] = 2; desc[cnt++] = "evil creatures"; }

	if (f1 & TR1_SLAY_ORC)    { mult[cnt] = 3; desc[cnt++] = "orcs"; }
	if (f1 & TR1_SLAY_TROLL)  { mult[cnt] = 3; desc[cnt++] = "trolls"; }
	if (f1 & TR1_SLAY_GIANT)  { mult[cnt] = 3; desc[cnt++] = "giants"; }
	if (f1 & TR1_SLAY_DRAGON) { mult[cnt] = 3; desc[cnt++] = "dragons"; }
	if (f1 & TR1_SLAY_DEMON)  { mult[cnt] = 3; desc[cnt++] = "demons"; }
	if (f1 & TR1_SLAY_UNDEAD) { mult[cnt] = 3; desc[cnt++] = "undead"; }

	if (f1 & TR1_BRAND_ACID)  { mult[cnt] = 3; desc[cnt++] = "acid-vulnerable creatures"; }
	if (f1 & TR1_BRAND_ELEC)  { mult[cnt] = 3; desc[cnt++] = "electricity-vulnerable creatures"; }
	if (f1 & TR1_BRAND_FIRE)  { mult[cnt] = 3; desc[cnt++] = "fire-vulnerable creatures"; }
	if (f1 & TR1_BRAND_COLD)  { mult[cnt] = 3; desc[cnt++] = "frost-vulnerable creatures"; }
	if (f1 & TR1_BRAND_POIS)  { mult[cnt] = 3; desc[cnt++] = "poison-vulnerable creatures"; }

	if (f1 & TR1_KILL_DRAGON) { mult[cnt] = 5; desc[cnt++] = "dragons"; }
	if (f1 & TR1_KILL_DEMON)  { mult[cnt] = 5; desc[cnt++] = "demons"; }
	if (f1 & TR1_KILL_UNDEAD) { mult[cnt] = 5; desc[cnt++] = "undead"; }

	return cnt;
}


/*
 * Describe combat advantages.
 */
static bool describe_combat(const object_type *o_ptr, u32b f1)
{
	cptr desc[15];
	int mult[15];
	int cnt, dam;
	int xtra_dam = 0;
	object_type *j_ptr = &inventory[INVEN_BOW];

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->ammo_tval == o_ptr->tval) &&
	              (j_ptr->k_idx);

	/* Abort if we've nothing to say */
	if (!weapon && !ammo) return FALSE;



	if (weapon)
	{
		int blows = calc_blows(o_ptr);

		dam = (o_ptr->ds * o_ptr->dd * 5);

		xtra_dam = (p_ptr->to_d * 10);
		if (object_known_p(o_ptr))
			xtra_dam += (o_ptr->to_d * 10);

		/* Warn about heavy weapons */
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			if (new_paragraph) { text_out("\n\n"); new_paragraph = FALSE; }
			text_out_c(TERM_L_RED, "You are too weak to use this weapon effectively!  ");
			blows = 1;
		}
	
	    p_text_out("Using this weapon, in your current condition, you are able to score ");
	    text_out_c(TERM_L_GREEN, format("%d ", blows));
	    if (blows > 1)
			text_out("blows per round.  Each blow will do an average damage of ");
	    else
			text_out("blow per round, averaging a damage of ");
	}
	else
	{
		int tdis = 10 + 5 * p_ptr->ammo_mult;

		/* Calculate damage */
		dam = (o_ptr->ds * o_ptr->dd * 5);
		if (object_known_p(o_ptr)) dam += (o_ptr->to_d * 10);
		if (object_known_p(j_ptr)) dam += (j_ptr->to_d * 10);
		dam *= p_ptr->ammo_mult;

		p_text_out("Fired from your current bow, this arrow will hit targets up to ");
		text_out_c(TERM_L_GREEN, format("%d", tdis * 10));
		text_out(" feet away, inflicting an average damage of ");
	}

	/* Collect slays */
	cnt = collect_slays(desc, mult, f1);
	if (object_known_p(o_ptr) && cnt)
	{
		size_t i;

		for (i = 0; i < cnt; i++)
		{
			text_out_c(TERM_L_GREEN, "%d", ((dam * mult[i]) + xtra_dam) / 10);
			text_out(" against %s, ", desc[i]);
		}

		text_out("and ");
	}

    if (dam % 10)
		text_out_c(TERM_L_GREEN, "%d.%d", dam / 10, dam % 10);
    else
		text_out_c(TERM_L_GREEN, "%d", dam / 10);

	text_out(" against normal creatures.  ");

	/* Add breakage chance */
	if (ammo)
	{
		text_out("It has a ");
		text_out_c(TERM_L_GREEN, "%d%%", breakage_chance(o_ptr));
		text_out(" chance of breaking upon contact.");
	}

	/* You always have something to say... */
	return TRUE;
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

	/* Describe lights */
	if (o_ptr->tval == TV_LITE || (f3 & TR3_LITE))
	{
		bool artifact = artifact_p(o_ptr);
		bool no_fuel = (f3 & TR3_NO_FUEL) ? TRUE : FALSE;
		int rad = 0;

		if (artifact)
			rad = 3;
		else if (o_ptr->tval == TV_LITE)
			rad = 2;

		if (f3 & TR3_LITE) rad++;

		p_text_out("It usually provides light of radius ");
		text_out_c(TERM_L_GREEN, format("%d", rad));
		if (no_fuel && !artifact)
			text_out(", and never needs refuelling");
		else if (o_ptr->tval == TV_LITE && o_ptr->sval == SV_LITE_TORCH)
			text_out(", though this is reduced when running of out fuel");
		text_out(".  ");

		something = TRUE;
	}

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
 * Describe origin of an item
 */
static bool describe_origin(const object_type *o_ptr)
{
	/* Abort now if undisplayable origin */
	if (o_ptr->origin == ORIGIN_NONE ||
	    o_ptr->origin == ORIGIN_MIXED)
		return FALSE;

	if (o_ptr->number > 1)
		p_text_out("They were ");
	else
		p_text_out("It was ");

	/* Display the right thing */
	switch (o_ptr->origin)
	{
		case ORIGIN_BIRTH:
			text_out("an inheritance from your family");
			break;

		case ORIGIN_STORE:
			text_out("bought in a store");
			break;

		case ORIGIN_FLOOR:
			text_out("lying on the floor");
 			break;

		case ORIGIN_DROP:
		{
			const char *name = r_name + r_info[o_ptr->origin_xtra].name;
			bool unique = (r_info[o_ptr->origin_xtra].flags1 & RF1_UNIQUE) ? TRUE : FALSE;

			text_out("dropped by %s%s", is_a_vowel(name[0]) ? "an " : "a ", name);

 			break;
		}

		case ORIGIN_DROP_UNKNOWN:
			text_out("dropped by an unknown monster");
			break;

		case ORIGIN_ACQUIRE:
			text_out("conjured forth by magic");
 			break;

		case ORIGIN_CHEAT:
			text_out("created by a debug option");
 			break;

		case ORIGIN_CHEST:
			text_out("found in a chest");
			break;
	}

	if (o_ptr->origin_depth)
	{
		if (depth_in_feet)
			text_out(" at a depth of %d feet", o_ptr->origin_depth * 50);
		else
			text_out(" on dungeon level %d", o_ptr->origin_depth);
	}

	text_out(".  ");
	return TRUE;
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


	/* Describe combat bits */
	new_paragraph = TRUE;
	if (describe_combat(o_ptr, f1)) something = TRUE;

	/* Describe other bits */
	new_paragraph = TRUE;
	if (describe_stats(o_ptr, f1)) something = TRUE;
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

	/* Dump origin info */
	describe_origin(o_ptr);

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
