/*
 * File: cmd-obj.c
 * Purpose: Handle objects in various ways
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andrew Sidwell, Chris Carr, Ed Graham, Erik Osheim
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
#include "object/object.h"
#include "game-cmd.h"
#include "cmds.h"
#include "effects.h"

/*** Utility bits and bobs ***/

/*
 * Check to see if the player can use a rod/wand/staff/activatable object.
 */
static int check_devices(object_type *o_ptr)
{
	int fail;
	const char *msg;
	const char *what = NULL;

	/* Get the right string */
	switch (o_ptr->tval)
	{
		case TV_ROD:   msg = "zap the rod";   break;
		case TV_WAND:  msg = "use the wand";  what = "wand";  break;
		case TV_STAFF: msg = "use the staff"; what = "staff"; break;
		default:       msg = "activate it";  break;
	}

	/* Figure out how hard the item is to use */
	fail = get_use_device_chance(o_ptr);

	/* Roll for usage */
	if (randint1(1000) < fail)
	{
		if (OPT(flush_failure)) flush();
		msg_format("You failed to %s properly.", msg);
		return FALSE;
	}

	/* Notice empty staffs */
	if (what && o_ptr->pval <= 0)
	{
		if (OPT(flush_failure)) flush();
		msg_format("The %s has no charges left.", what);
		o_ptr->ident |= (IDENT_EMPTY);
		return FALSE;
	}

	return TRUE;
}

/*
 *Returns the number of times in 1000 that @ will FAIL
 * - thanks to Ed Graham for the formula
 */
int get_use_device_chance(const object_type *o_ptr)
{
	int lev, skill, fail;

	/* these could be globals if desired, calculated rather than stated */
	int skill_min = 10;
	int skill_max = 141;
	int diff_min = 1;
	int diff_max = 100;

	/* Extract the item level, which is the difficulty rating */
	if (artifact_p(o_ptr))
		lev = a_info[o_ptr->name1].level;
	else
		lev = k_info[o_ptr->k_idx].level;

	/* Chance of failure */
	skill = p_ptr->state.skills[SKILL_DEVICE];

	fail = 100 * ((skill - lev) - (skill_max - diff_min))
		/ ((lev - skill) - (diff_max - skill_min));

	/* Limit range */
	if (fail > 950) fail = 950;
	if (fail < 10) fail = 10;

	return fail;
}


/*
 * Return the chance of an effect beaming, given a tval.
 */
static int beam_chance(int tval)
{
	switch (tval)
	{
		case TV_WAND: return 20;
		case TV_ROD:  return 10;
	}

	return 0;
}


typedef enum {
	ART_TAG_NONE,
	ART_TAG_NAME,
	ART_TAG_KIND,
	ART_TAG_VERB,
	ART_TAG_VERB_IS
} art_tag_t;

static art_tag_t art_tag_lookup(const char *tag)
{
	if (strncmp(tag, "name", 4) == 0)
		return ART_TAG_NAME;
	else if (strncmp(tag, "kind", 4) == 0)
		return ART_TAG_KIND;
	else if (strncmp(tag, "s", 1) == 0)
		return ART_TAG_VERB;
	else if (strncmp(tag, "is", 2) == 0)
		return ART_TAG_VERB_IS;
	else
		return ART_TAG_NONE;
}

/*
 * Print an artifact activation message.
 *
 * In order to support randarts, with scrambled names, we re-write
 * the message to replace instances of {name} with the artifact name
 * and instances of {kind} with the type of object.
 *
 * This code deals with plural and singular forms of verbs correctly
 * when encountering {s}, though in fact both names and kinds are
 * always singular in the current code (gloves are "Set of" and boots
 * are "Pair of")
 */
static void activation_message(object_type *o_ptr, const char *message)
{
	char buf[1024] = "\0";
	const char *next;
	const char *s;
	const char *tag;
	const char *in_cursor;
	size_t end = 0;

	in_cursor = message;

	next = strchr(in_cursor, '{');
	while (next)
	{
		/* Copy the text leading up to this { */
		strnfcat(buf, 1024, &end, "%.*s", next - in_cursor, in_cursor); 

		s = next + 1;
		while (*s && isalpha((unsigned char) *s)) s++;

		if (*s == '}')		/* Valid tag */
		{
			tag = next + 1; /* Start the tag after the { */
			in_cursor = s + 1;

			switch(art_tag_lookup(tag))
			{
			case ART_TAG_NAME:
				end += object_desc(buf, 1024, o_ptr, ODESC_PREFIX | ODESC_BASE); 
				break;
			case ART_TAG_KIND:
				object_kind_name(&buf[end], 1024-end, o_ptr->k_idx, TRUE);
				end += strlen(&buf[end]);
				break;
			case ART_TAG_VERB:
				strnfcat(buf, 1024, &end, "s");
				break;
			case ART_TAG_VERB_IS:
				if((end > 2) && (buf[end-2] == 's'))
					strnfcat(buf, 1024, &end, "are");
				else
					strnfcat(buf, 1024, &end, "is");
			default:
				break;
			}
		}
		else    /* An invalid tag, skip it */
		{
			in_cursor = next + 1;
		}

		next = strchr(in_cursor, '{');
	}
	strnfcat(buf, 1024, &end, in_cursor);

	msg_print(buf);
}



/*** Inscriptions ***/

/* Remove inscription */
void do_cmd_uninscribe(cmd_code code, cmd_arg args[])
{
	object_type *o_ptr = object_from_item_idx(args[0].item);

	if (obj_has_inscrip(o_ptr))
		msg_print("Inscription removed.");

	o_ptr->note = 0;

	p_ptr->notice |= (PN_COMBINE | PN_SQUELCH | PN_SORT_QUIVER);
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
}

/* Add inscription */
void do_cmd_inscribe(cmd_code code, cmd_arg args[])
{
	object_type *o_ptr = object_from_item_idx(args[0].item);

	o_ptr->note = quark_add(args[1].string);

	p_ptr->notice |= (PN_COMBINE | PN_SQUELCH | PN_SORT_QUIVER);
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
}

static void obj_inscribe(object_type *o_ptr, int item)
{
	char o_name[80];
	char tmp[80] = "";

	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);
	msg_format("Inscribing %s.", o_name);
	message_flush();

	/* Use old inscription */
	if (o_ptr->note)
		strnfmt(tmp, sizeof(tmp), "%s", quark_str(o_ptr->note));

	/* Get a new inscription (possibly empty) */
	if (get_string("Inscription: ", tmp, sizeof(tmp)))
	{
		cmd_insert(CMD_INSCRIBE, item, tmp);
	}
}


/*** Examination ***/
static void obj_examine(object_type *o_ptr, int item)
{
	track_object(item);

	text_out_hook = text_out_to_screen;
	screen_save();

	object_info_header(o_ptr);
	if (!object_info(o_ptr, OINFO_NONE))
		text_out("\n\nThis item does not seem to possess any special abilities.");

	text_out_c(TERM_L_BLUE, "\n\n[Press any key to continue]\n");
	(void)anykey();

	screen_load();
}



/*** Taking off/putting on ***/

/* Take off an item */
void do_cmd_takeoff(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;

	if (!item_is_available(item, NULL, USE_EQUIP))
	{
		msg_print("You are not wielding that item.");
		return;
	}

	if (!obj_can_takeoff(object_from_item_idx(item)))
	{
		msg_print("You cannot take off that item.");
		return;
	}

	(void)inven_takeoff(item, 255);
	pack_overflow();
	p_ptr->energy_use = 50;
}

/* Wield or wear an item */
void do_cmd_wield(cmd_code code, cmd_arg args[])
{
	object_type *equip_o_ptr;
	char o_name[80];

	unsigned n;

	int item = args[0].item;
	int slot = args[1].number;
	object_type *o_ptr = object_from_item_idx(item);

	if (!item_is_available(item, NULL, USE_INVEN | USE_FLOOR))
	{
		msg_print("You do not have that item to wield.");
		return;
	}

	/* Check the slot */
	if (!slot_can_wield_item(slot, o_ptr))
	{
		msg_print("You cannot wield that item there.");
		return;
	}

	equip_o_ptr = &inventory[slot];

	/* If the slot is open, wield and be done */
	if (!equip_o_ptr->k_idx) {
		wield_item(o_ptr, item, slot);
		return;
	}

	/* If the slot is in the quiver and objects can be combined */
	if (obj_is_ammo(equip_o_ptr) && object_similar(equip_o_ptr, o_ptr))
	{
		wield_item(o_ptr, item, slot);
		return;
	}

	/* Prevent wielding into a cursed slot */
	if (cursed_p(equip_o_ptr))
	{
		object_desc(o_name, sizeof(o_name), equip_o_ptr, ODESC_BASE);
		msg_format("The %s you are %s appears to be cursed.", o_name,
				   describe_use(slot));
		return;
	}

		/* "!t" checks for taking off */
		n = check_for_inscrip(equip_o_ptr, "!t");
		while (n--)
		{
			/* Prompt */
			object_desc(o_name, sizeof(o_name), equip_o_ptr,
						ODESC_PREFIX | ODESC_FULL);

		/* Forget it */
		if (!get_check(format("Really take off %s? ", o_name))) return;
	}

	wield_item(o_ptr, item, slot);
}

/* Drop an item */
void do_cmd_drop(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	object_type *o_ptr = object_from_item_idx(item);
	int amt = args[1].number;

	if (!item_is_available(item, NULL, USE_INVEN | USE_EQUIP))
	{
		msg_print("You do not have that item to drop it.");
		return;
	}

	/* Hack -- Cannot remove cursed items */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		msg_print("Hmmm, it seems to be cursed.");
		return;
	}

	inven_drop(item, amt);
	p_ptr->energy_use = 50;
}

static void obj_drop(object_type *o_ptr, int item)
{
	int amt;

	amt = get_quantity(NULL, o_ptr->number);
	if (amt <= 0) return;

	cmd_insert(CMD_DROP, item, amt);
}

static void obj_wield(object_type *o_ptr, int item)
{
	int slot = wield_slot(o_ptr);

	/* Usually if the slot is taken we'll just replace the item in the slot,
	 * but in some cases we need to ask the user which slot they actually
	 * want to replace */
	if (inventory[slot].k_idx)
	{
		if (o_ptr->tval == TV_RING)
		{
			cptr q = "Replace which ring? ";
			cptr s = "Error in obj_wield, please report";
			item_tester_hook = obj_is_ring;
			if (!get_item(&slot, q, s, USE_EQUIP)) return;
		}

		if (obj_is_ammo(o_ptr) && !object_similar(&inventory[slot], o_ptr))
		{
			cptr q = "Replace which ammunition? ";
			cptr s = "Error in obj_wield, please report";
			item_tester_hook = obj_is_ammo;
			if (!get_item(&slot, q, s, USE_EQUIP)) return;
		}
	}

	cmd_insert(CMD_WIELD, item, slot);
}


/*** Casting and browsing ***/

/* Peruse spells in a book */
static void obj_browse(object_type *o_ptr, int item)
{
	do_cmd_browse_aux(o_ptr, item);
}

/* Study a book to gain a new spell */
static void obj_study(object_type *o_ptr, int item)
{
	/* Track the object kind */
	track_object(item);

	/* Mage -- Choose a spell to study */
	if (cp_ptr->flags & CF_CHOOSE_SPELLS)
	{
		int spell = get_spell(o_ptr, "study", FALSE, FALSE);
		if (spell >= 0)
			cmd_insert(CMD_STUDY_SPELL, spell);
		else if (spell == -2)
			msg_print("You cannot learn any spells from that book.");
	}

	/* Priest -- Choose a book to study */
	else
	{
		cmd_insert(CMD_STUDY_BOOK, item);
	}
}

static void obj_cast(object_type *o_ptr, int item)
{
	int spell, dir = DIR_UNKNOWN;

	cptr verb = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "cast" : "recite");
	cptr noun = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");

	/* Track the object kind */
	track_object(item);

	/* Ask for a spell */
	spell = get_spell(o_ptr, verb, TRUE, FALSE);
	if (spell < 0)
	{
		if (spell == -2) msg_format("You don't know any %ss in that book.", noun);
		return;
	}

	if (spell_needs_aim(cp_ptr->spell_book, spell) && !get_aim_dir(&dir))
		return;

	cmd_insert(CMD_CAST, spell, dir);
}


/*** Using items the traditional way ***/

/*
 * Use an object the right way.
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "o_ptr" to
 * no longer point at the correct item, with horrifying results.
 */
void do_cmd_use(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	object_type *o_ptr = object_from_item_idx(item);
	int effect;
	bool ident = FALSE, used = FALSE;
	bool was_aware = object_flavor_is_aware(o_ptr);
	int dir = 5;
	int px = p_ptr->px, py = p_ptr->py;
	int snd, boost, level;
	use_type use;
	int items_allowed = 0;

	/* Determine how this item is used. */
	if (obj_is_rod(o_ptr))
	{
		if (!obj_can_zap(o_ptr))
		{
			msg_print("That rod is still charging.");
			return;
		}

		use = USE_TIMEOUT;
		snd = MSG_ZAP_ROD;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_wand(o_ptr))
	{
		if (!obj_has_charges(o_ptr))
		{
			msg_print("That wand has no charges.");
			return;
		}

		use = USE_CHARGE;
		snd = MSG_ZAP_ROD;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_staff(o_ptr))
	{
		if (!obj_has_charges(o_ptr))
		{
			msg_print("That staff has no charges.");
			return;
		}

		use = USE_CHARGE;
		snd = MSG_ZAP_ROD;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_food(o_ptr))
	{
		use = USE_SINGLE;
		snd = MSG_EAT;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_potion(o_ptr))
	{
		use = USE_SINGLE;
		snd = MSG_QUAFF;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_scroll(o_ptr))
	{
		/* Check player can use scroll */
		if (!player_can_read())
			return;

		use = USE_SINGLE;
		snd = MSG_GENERIC;
		items_allowed = USE_INVEN | USE_FLOOR;
	}
	else if (obj_is_activatable(o_ptr))
	{
		if (!obj_can_activate(o_ptr))
		{
			msg_print("That item is still charging.");
			return;
		}

		use = USE_TIMEOUT;
		snd = MSG_ACT_ARTIFACT;
		items_allowed = USE_EQUIP;
	}
	else
	{
		msg_print("The item cannot be used at the moment");
	}

	/* Check if item is within player's reach. */
	if (items_allowed == 0 || !item_is_available(item, NULL, items_allowed))
	{
		msg_print("You cannot use that item from its current location.");
		return;
	}

	/* track the object used */
	track_object(item);

	/* Figure out effect to use */
	effect = object_effect(o_ptr);

	/* If the item requires a direction, get one (allow cancelling) */
	if (obj_needs_aim(o_ptr))
		dir = args[1].direction;

	/* Check for use if necessary, and execute the effect */
	if ((use != USE_CHARGE && use != USE_TIMEOUT) ||
	    check_devices(o_ptr))
	{
		/* Special message for artifacts */
		if (artifact_p(o_ptr))
		{
			message(snd, 0, "You activate it.");
			activation_message(o_ptr, a_text + a_info[o_ptr->name1].effect_msg);
			level = a_info[o_ptr->name1].level;
		}
		else
		{
			/* Make a noise! */
			sound(snd);
			level = k_info[o_ptr->k_idx].level;
		}

		/* A bit of a hack to make ID work better.
			-- Check for "obvious" effects beforehand. */
		if (effect_obvious(effect)) object_flavor_aware(o_ptr);

		/* Boost damage effects if skill > difficulty */
		boost = p_ptr->state.skills[SKILL_DEVICE] - level;
		if (boost < 0) boost = 0;

		/* Do effect */
		used = effect_do(effect, &ident, was_aware, dir,
			beam_chance(o_ptr->tval), boost);

		/* Quit if the item wasn't used and no knowledge was gained */
		if (!used && (was_aware || !ident)) return;
	}

	/* If the item is a null pointer or has been wiped, be done now */
	if (!o_ptr || o_ptr->k_idx <= 1) return;

	if (ident) object_notice_effect(o_ptr);

	/* Food feeds the player */
	if (o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION)
		(void)set_food(p_ptr->food + o_ptr->pval);

	/* Use the turn */
	p_ptr->energy_use = 100;

	/* Mark as tried and redisplay */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_OBJECT);

	/*
	 * If the player becomes aware of the item's function, then mark it as
	 * aware and reward the player with some experience.  Otherwise, mark
	 * it as "tried".
	 */
	if (ident && !was_aware)
	{
		/* Object level */
		int lev = k_info[o_ptr->k_idx].level;

		object_flavor_aware(o_ptr);
		if (o_ptr->tval == TV_ROD) object_notice_everything(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}
	else
	{
		object_flavor_tried(o_ptr);
	}

	/* If there are no more of the item left, then we're done. */
	if (!o_ptr->number) return;

	/* Chargeables act differently to single-used items when not used up */
	if (used && use == USE_CHARGE)
	{
		/* Use a single charge */
		o_ptr->pval--;

		/* Describe charges */
		if (item >= 0)
			inven_item_charges(item);
		else
			floor_item_charges(0 - item);
	}
	else if (used && use == USE_TIMEOUT)
	{
		/* Artifacts use their own special field */
		if (o_ptr->name1)
		{
			const artifact_type *a_ptr = &a_info[o_ptr->name1];
			o_ptr->timeout = randcalc(a_ptr->time, 0, RANDOMISE);
		}
		else
		{
			const object_kind *k_ptr = &k_info[o_ptr->k_idx];
			o_ptr->timeout += randcalc(k_ptr->time, 0, RANDOMISE);
		}
	}
	else if (used && use == USE_SINGLE)
	{
		/* Destroy a potion in the pack */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		}

		/* Destroy a potion on the floor */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	}
	
	/* Hack to make Glyph of Warding work properly */
	if (cave_feat[py][px] == FEAT_GLYPH)
	{
		/* Shift any objects to further away */
		for (o_ptr = get_first_object(py, px); o_ptr; o_ptr = get_next_object(o_ptr))
		{
			drop_near(o_ptr, 0, py, px);
		}
		
		/* Delete the "moved" objects from their original position */
		delete_object(py, px);
	}

	
}


/*** Refuelling ***/
void do_cmd_refill(cmd_code code, cmd_arg args[])
{
	object_type *j_ptr = &inventory[INVEN_LITE];
	u32b f[OBJ_FLAG_N];

	int item = args[0].item;
	object_type *o_ptr = object_from_item_idx(item);

	if (!item_is_available(item, NULL, USE_INVEN | USE_FLOOR))
	{
		msg_print("You do not have that item to refill with it.");
		return;
	}

	/* Check what we're wielding. */
	object_flags(j_ptr, f);

	if (j_ptr->tval != TV_LITE)
	{
		msg_print("You are not wielding a light.");
		return;
	}

	else if (f[2] & TR2_NO_FUEL)
	{
		msg_print("Your light cannot be refilled.");
		return;
	}

	/* It's a lamp */
	if (j_ptr->sval == SV_LITE_LANTERN)
		refill_lamp(j_ptr, o_ptr, item);

	/* It's a torch */
	else if (j_ptr->sval == SV_LITE_TORCH)
		refuel_torch(j_ptr, o_ptr, item);

	p_ptr->energy_use = 50;
}



/*** Handling bits ***/

/* Item "action" type */
typedef struct
{
	void (*action)(object_type *, int);
	cmd_code command;
	const char *desc;

	const char *prompt;
	const char *noop;

	bool (*filter)(const object_type *o_ptr);
	int mode;
	bool (*prereq)(void);
} item_act_t;


/* All possible item actions */
static item_act_t item_actions[] =
{
	/* Not setting IS_HARMLESS for this one because it could cause a true
	 * dangerous command to not be prompted, later.
	 */
	{ NULL, CMD_UNINSCRIBE, "uninscribe",
	  "Un-inscribe which item? ", "You have nothing to un-inscribe.",
	  obj_has_inscrip, (USE_EQUIP | USE_INVEN | USE_FLOOR), NULL },

	{ obj_inscribe, CMD_NULL, "inscribe",
	  "Inscribe which item? ", "You have nothing to inscribe.",
	  NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS), NULL },

	{ obj_examine, CMD_NULL, "examine",
	  "Examine which item? ", "You have nothing to examine.",
	  NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS), NULL },

	/*** Takeoff/drop/wear ***/
	{ NULL, CMD_TAKEOFF, "takeoff",
	  "Take off which item? ", "You are not wearing anything you can take off.",
	  obj_can_takeoff, USE_EQUIP, NULL },

	{ obj_wield, CMD_WIELD, "wield",
	  "Wear/Wield which item? ", "You have nothing you can wear or wield.",
	  obj_can_wear, (USE_INVEN | USE_FLOOR), NULL },

	{ obj_drop, CMD_NULL, "drop",
	  "Drop which item? ", "You have nothing to drop.",
	  NULL, (USE_EQUIP | USE_INVEN), NULL },

	/*** Spellbooks ***/
	{ obj_browse, CMD_NULL, "browse",
	  "Browse which book? ", "You have no books that you can read.",
	  obj_can_browse, (USE_INVEN | USE_FLOOR | IS_HARMLESS), NULL },

	{ obj_study, CMD_NULL, "study",
	  "Study which book? ", "You have no books that you can read.",
	  obj_can_browse, (USE_INVEN | USE_FLOOR), player_can_study },

	{ obj_cast, CMD_NULL, "cast",
	  "Use which book? ", "You have no books that you can read.",
	  obj_can_browse, (USE_INVEN | USE_FLOOR), player_can_cast },

	/*** Item usage ***/
	{ NULL, CMD_USE_STAFF, "use",
	  "Use which staff? ", "You have no staff to use.",
	  obj_is_staff, (USE_INVEN | USE_FLOOR), NULL },

	{ NULL, CMD_USE_WAND, "aim",
      "Aim which wand? ", "You have no wand to aim.",
	  obj_is_wand, (USE_INVEN | USE_FLOOR), NULL },

	{ NULL, CMD_USE_ROD, "zap",
      "Zap which rod? ", "You have no charged rods to zap.",
	  obj_is_rod, (USE_INVEN | USE_FLOOR), NULL },

	{ NULL, CMD_ACTIVATE, "activate",
      "Activate which item? ", "You have nothing to activate.",
	  obj_is_activatable, USE_EQUIP, NULL },

	{ NULL, CMD_EAT, "eat",
      "Eat which item? ", "You have nothing to eat.",
	  obj_is_food, (USE_INVEN | USE_FLOOR), NULL },

	{ NULL, CMD_QUAFF, "quaff",
      "Quaff which potion? ", "You have no potions to quaff.",
	  obj_is_potion, (USE_INVEN | USE_FLOOR), NULL },

	{ NULL, CMD_READ_SCROLL, "read",
      "Read which scroll? ", "You have no scrolls to read.",
	  obj_is_scroll, (USE_INVEN | USE_FLOOR), player_can_read },

	{ NULL, CMD_REFILL, "refill",
      "Refuel with what fuel source? ", "You have nothing to refuel with.",
	  obj_can_refill, (USE_INVEN | USE_FLOOR), NULL },
};


/* List matching up to item_actions[] */
typedef enum
{
	ACTION_UNINSCRIBE = 0,
	ACTION_INSCRIBE,
	ACTION_EXAMINE,
	ACTION_TAKEOFF,
	ACTION_WIELD,
	ACTION_DROP,

	ACTION_BROWSE,
	ACTION_STUDY,
	ACTION_CAST,

	ACTION_USE_STAFF,
	ACTION_AIM_WAND,
	ACTION_ZAP_ROD,
	ACTION_ACTIVATE,
	ACTION_EAT_FOOD,
	ACTION_QUAFF_POTION,
	ACTION_READ_SCROLL,
	ACTION_REFILL
} item_act;



/*** Old-style noun-verb functions ***/

/* Generic "do item action" function */
static void do_item(item_act act)
{
	int item;
	object_type *o_ptr;
	bool cmd_needs_aim = FALSE;

	cptr q, s;

	if (item_actions[act].prereq)
	{
		if (!item_actions[act].prereq())
			return;
	}

	/* Get item */
	q = item_actions[act].prompt;
	s = item_actions[act].noop;
	item_tester_hook = item_actions[act].filter;
	if (!get_item(&item, q, s, item_actions[act].mode)) return;

	/* Get the item */
	o_ptr = object_from_item_idx(item);

	/* These commands need an aim */
	if (item_actions[act].command == CMD_QUAFF ||
		item_actions[act].command == CMD_ACTIVATE ||
		item_actions[act].command == CMD_USE_WAND ||
		item_actions[act].command == CMD_USE_ROD ||
		item_actions[act].command == CMD_USE_STAFF ||
		item_actions[act].command == CMD_READ_SCROLL)
	{
		cmd_needs_aim = TRUE;
	}

	/* Execute the item command */
	if (item_actions[act].action != NULL)
		item_actions[act].action(o_ptr, item);
	else if (cmd_needs_aim && obj_needs_aim(o_ptr))
	{
		int dir;
		if (!get_aim_dir(&dir))
			return;

		cmd_insert(item_actions[act].command, item, dir);
	}
	else
		cmd_insert(item_actions[act].command, item);
}

/* Wrappers */
void textui_cmd_uninscribe(void) { do_item(ACTION_UNINSCRIBE); }
void textui_cmd_inscribe(void) { do_item(ACTION_INSCRIBE); }
void do_cmd_observe(void) { do_item(ACTION_EXAMINE); }
void textui_cmd_takeoff(void) { do_item(ACTION_TAKEOFF); }
void textui_cmd_wield(void) { do_item(ACTION_WIELD); }
void textui_cmd_drop(void) { do_item(ACTION_DROP); }
void do_cmd_browse(void) { do_item(ACTION_BROWSE); }
void textui_cmd_study(void) { do_item(ACTION_STUDY); }
void textui_cmd_cast(void) { do_item(ACTION_CAST); }
void textui_cmd_pray(void) { do_item(ACTION_CAST); }
void textui_cmd_use_staff(void) { do_item(ACTION_USE_STAFF); }
void textui_cmd_aim_wand(void) { do_item(ACTION_AIM_WAND); }
void textui_cmd_zap_rod(void) { do_item(ACTION_ZAP_ROD); }
void textui_cmd_activate(void) { do_item(ACTION_ACTIVATE); }
void textui_cmd_eat_food(void) { do_item(ACTION_EAT_FOOD); }
void textui_cmd_quaff_potion(void) { do_item(ACTION_QUAFF_POTION); }
void textui_cmd_read_scroll(void) { do_item(ACTION_READ_SCROLL); }
void textui_cmd_refill(void) { do_item(ACTION_REFILL); }
