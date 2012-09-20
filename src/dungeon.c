/* File: dungeon.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * Return a "feeling" (or NULL) about an item.  Method 1 (Heavy).
 */
static int value_check_aux1(const object_type *o_ptr)
{
	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_TERRIBLE);

		/* Normal */
		return (INSCRIP_SPECIAL);
	}

	/* Ego-Items */
	if (ego_item_p(o_ptr))
	{
		/* Cursed/Broken */
		if (cursed_p(o_ptr) || broken_p(o_ptr)) return (INSCRIP_WORTHLESS);

		/* Normal */
		return (INSCRIP_EXCELLENT);
	}

	/* Cursed items */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items */
	if (broken_p(o_ptr)) return (INSCRIP_BROKEN);

	/* Good "armor" bonus */
	if (o_ptr->to_a > 0) return (INSCRIP_GOOD);

	/* Good "weapon" bonus */
	if (o_ptr->to_h + o_ptr->to_d > 0) return (INSCRIP_GOOD);

	/* Default to "average" */
	return (INSCRIP_AVERAGE);
}


/*
 * Return a "feeling" (or NULL) about an item.  Method 2 (Light).
 */
static int value_check_aux2(const object_type *o_ptr)
{
	/* Cursed items (all of them) */
	if (cursed_p(o_ptr)) return (INSCRIP_CURSED);

	/* Broken items (all of them) */
	if (broken_p(o_ptr)) return (INSCRIP_BROKEN);

	/* Artifacts -- except cursed/broken ones */
	if (artifact_p(o_ptr)) return (INSCRIP_GOOD);

	/* Ego-Items -- except cursed/broken ones */
	if (ego_item_p(o_ptr)) return (INSCRIP_GOOD);

	/* Good armor bonus */
	if (o_ptr->to_a > 0) return (INSCRIP_GOOD);

	/* Good weapon bonuses */
	if (o_ptr->to_h + o_ptr->to_d > 0) return (INSCRIP_GOOD);

	/* No feeling */
	return (0);
}



/*
 * Sense the inventory
 */
static void sense_inventory(void)
{
	int i;

	int plev = p_ptr->lev;

	bool heavy = ((cp_ptr->flags & CF_PSEUDO_ID_HEAVY) ? TRUE : FALSE);

	int feel;

	object_type *o_ptr;

	char o_name[80];


	/*** Check for "sensing" ***/

	/* No sensing when confused */
	if (p_ptr->confused) return;

	if (cp_ptr->flags & CF_PSEUDO_ID_IMPROV)
	{
		if (0 != rand_int(cp_ptr->sense_base / (plev * plev + cp_ptr->sense_div)))
			return;
	}
	else
	{
		if (0 != rand_int(cp_ptr->sense_base / (plev + cp_ptr->sense_div)))
			return;
	}


	/*** Sense everything ***/

	/* Check everything */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		bool okay = FALSE;

		o_ptr = &inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Valid "tval" codes */
		switch (o_ptr->tval)
		{
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			{
				okay = TRUE;
				break;
			}
		}

		/* Skip non-sense machines */
		if (!okay) continue;

		/* It already has a discount or special inscription */
		if ((o_ptr->discount > 0) &&
		    (o_ptr->discount != INSCRIP_INDESTRUCTIBLE)) continue;

		/* It has already been sensed, do not sense it again */
		if (o_ptr->ident & (IDENT_SENSE)) continue;

		/* It is fully known, no information needed */
		if (object_known_p(o_ptr)) continue;

		/* Occasional failure on inventory items */
		if ((i < INVEN_WIELD) && (0 != rand_int(5))) continue;

		/* Indestructible objects are either excellent or terrible */
		if (o_ptr->discount == INSCRIP_INDESTRUCTIBLE)
			heavy = TRUE;

		/* Check for a feeling */
		feel = (heavy ? value_check_aux1(o_ptr) : value_check_aux2(o_ptr));

		/* Skip non-feelings */
		if (!feel) continue;

		/* Stop everything */
		if (disturb_minor) disturb(0, 0);

		/* Get an object description */
		object_desc(o_name, o_ptr, FALSE, 0);

		/* Message (equipment) */
		if (i >= INVEN_WIELD)
		{
			msg_format("You feel the %s (%c) you are %s %s %s...",
			           o_name, index_to_label(i), describe_use(i),
			           ((o_ptr->number == 1) ? "is" : "are"),
			           inscrip_text[feel - INSCRIP_NULL]);
		}

		/* Message (inventory) */
		else
		{
			msg_format("You feel the %s (%c) in your pack %s %s...",
			           o_name, index_to_label(i),
			           ((o_ptr->number == 1) ? "is" : "are"),
			           inscrip_text[feel - INSCRIP_NULL]);
		}

		/* Sense the object */
		o_ptr->discount = feel;

		/* The object has been "sensed" */
		o_ptr->ident |= (IDENT_SENSE);


		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * Regenerate hit points
 */
static void regenhp(int percent)
{
	s32b new_chp, new_chp_frac;
	int old_chp;

	/* Save the old hitpoints */
	old_chp = p_ptr->chp;

	/* Extract the new hitpoints */
	new_chp = ((long)p_ptr->mhp) * percent + PY_REGEN_HPBASE;
	p_ptr->chp += (s16b)(new_chp >> 16);   /* div 65536 */

	/* check for overflow */
	if ((p_ptr->chp < 0) && (old_chp > 0)) p_ptr->chp = MAX_SHORT;
	new_chp_frac = (new_chp & 0xFFFF) + p_ptr->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L)
	{
		p_ptr->chp_frac = (u16b)(new_chp_frac - 0x10000L);
		p_ptr->chp++;
	}
	else
	{
		p_ptr->chp_frac = (u16b)new_chp_frac;
	}

	/* Fully healed */
	if (p_ptr->chp >= p_ptr->mhp)
	{
		p_ptr->chp = p_ptr->mhp;
		p_ptr->chp_frac = 0;
	}

	/* Notice changes */
	if (old_chp != p_ptr->chp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
	}
}


/*
 * Regenerate mana points
 */
static void regenmana(int percent)
{
	s32b new_mana, new_mana_frac;
	int old_csp;

	old_csp = p_ptr->csp;
	new_mana = ((long)p_ptr->msp) * percent + PY_REGEN_MNBASE;
	p_ptr->csp += (s16b)(new_mana >> 16);	/* div 65536 */
	/* check for overflow */
	if ((p_ptr->csp < 0) && (old_csp > 0))
	{
		p_ptr->csp = MAX_SHORT;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p_ptr->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L)
	{
		p_ptr->csp_frac = (u16b)(new_mana_frac - 0x10000L);
		p_ptr->csp++;
	}
	else
	{
		p_ptr->csp_frac = (u16b)new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p_ptr->csp >= p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
		p_ptr->csp_frac = 0;
	}

	/* Redraw mana */
	if (old_csp != p_ptr->csp)
	{
		/* Redraw */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
	}
}






/*
 * Regenerate the monsters (once per 100 game turns)
 *
 * XXX XXX XXX Should probably be done during monster turns.
 */
static void regen_monsters(void)
{
	int i, frac;

	/* Regenerate everyone */
	for (i = 1; i < m_max; i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Allow regeneration (if needed) */
		if (m_ptr->hp < m_ptr->maxhp)
		{
			/* Hack -- Base regeneration */
			frac = m_ptr->maxhp / 100;

			/* Hack -- Minimal regeneration rate */
			if (!frac) frac = 1;

			/* Hack -- Some monsters regenerate quickly */
			if (r_ptr->flags2 & (RF2_REGENERATE)) frac *= 2;

			/* Hack -- Regenerate */
			m_ptr->hp += frac;

			/* Do not over-regenerate */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == i) p_ptr->redraw |= (PR_HEALTH);
		}
	}
}



/*
 * Handle certain things once every 10 game turns
 */
static void process_world(void)
{
	int i, j;

	int regen_amount;

	object_type *o_ptr;



	/* Every 10 game turns */
	if (turn % 10) return;


	/*** Check the Time and Load ***/

	if (!(turn % 1000))
	{
		/* Check time and load */
		if ((0 != check_time()) || (0 != check_load()))
		{
			/* Warning */
			if (closing_flag <= 2)
			{
				/* Disturb */
				disturb(0, 0);

				/* Count warnings */
				closing_flag++;

				/* Message */
				msg_print("The gates to ANGBAND are closing...");
				msg_print("Please finish up and/or save your game.");
			}

			/* Slam the gate */
			else
			{
				/* Message */
				msg_print("The gates to ANGBAND are now closed.");

				/* Stop playing */
				p_ptr->playing = FALSE;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}
	}


	/*** Handle the "town" (stores and sunshine) ***/

	/* While in town */
	if (!p_ptr->depth)
	{
		/* Hack -- Daybreak/Nighfall in town */
		if (!(turn % ((10L * TOWN_DAWN) / 2)))
		{
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn % (10L * TOWN_DAWN)));

			/* Day breaks */
			if (dawn)
			{
				/* Message */
				msg_print("The sun has risen.");
			}

			/* Night falls */
			else
			{
				/* Message */
				msg_print("The sun has fallen.");
			}

			/* Illuminate */
			town_illuminate(dawn);
		}
	}


	/* While in the dungeon */
	else
	{
		/*** Update the Stores ***/

		/* Update the stores once a day (while in dungeon) */
		if (!(turn % (10L * STORE_TURNS)))
		{
			int n;

			/* Message */
			if (cheat_xtra) msg_print("Updating Shops...");

			/* Maintain each shop (except home) */
			for (n = 0; n < MAX_STORES; n++)
			{
				/* Skip the home */
				if (n == STORE_HOME) continue;

				/* Maintain */
				store_maint(n);
			}

			/* Sometimes, shuffle the shop-keepers */
			if (rand_int(STORE_SHUFFLE) == 0)
			{
				/* Message */
				if (cheat_xtra) msg_print("Shuffling a Shopkeeper...");

				/* Pick a random shop (except home) */
				while (1)
				{
					n = rand_int(MAX_STORES);
					if (n != STORE_HOME) break;
				}

				/* Shuffle it */
				store_shuffle(n);
			}

			/* Message */
			if (cheat_xtra) msg_print("Done.");
		}
	}


	/*** Process the monsters ***/

	/* Check for creature generation */
	if (rand_int(MAX_M_ALLOC_CHANCE) == 0)
	{
		/* Make a new monster */
		(void)alloc_monster(MAX_SIGHT + 5, FALSE);
	}

	/* Hack -- Check for creature regeneration */
	if (!(turn % 100)) regen_monsters();


	/*** Damage over Time ***/

	/* Take damage from poison */
	if (p_ptr->poisoned)
	{
		/* Take damage */
		take_hit(1, "poison");
	}

	/* Take damage from cuts */
	if (p_ptr->cut)
	{
		/* Mortal wound or Deep Gash */
		if (p_ptr->cut > 200)
		{
			i = 3;
		}

		/* Severe cut */
		else if (p_ptr->cut > 100)
		{
			i = 2;
		}

		/* Other cuts */
		else
		{
			i = 1;
		}

		/* Take damage */
		take_hit(i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if (!(turn % 100))
		{
			/* Basic digestion rate based on speed */
			i = extract_energy[p_ptr->pspeed] * 2;

			/* Regeneration takes more food */
			if (p_ptr->regenerate) i += 30;

			/* Slow digestion takes less food */
			if (p_ptr->slow_digest) i -= 10;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			(void)set_food(p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		(void)set_food(p_ptr->food - 100);
	}

	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		take_hit(i, "starvation");
	}

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Getting Weak */
	if (p_ptr->food < PY_FOOD_WEAK)
	{
		/* Lower regeneration */
		if (p_ptr->food < PY_FOOD_STARVE)
		{
			regen_amount = 0;
		}
		else if (p_ptr->food < PY_FOOD_FAINT)
		{
			regen_amount = PY_REGEN_FAINT;
		}
		else
		{
			regen_amount = PY_REGEN_WEAK;
		}

		/* Getting Faint */
		if (p_ptr->food < PY_FOOD_FAINT)
		{
			/* Faint occasionally */
			if (!p_ptr->paralyzed && (rand_int(100) < 10))
			{
				/* Message */
				msg_print("You faint from the lack of food.");
				disturb(1, 0);

				/* Hack -- faint (bypass free action) */
				(void)set_paralyzed(p_ptr->paralyzed + 1 + rand_int(5));
			}
		}
	}

	/* Regeneration ability */
	if (p_ptr->regenerate)
	{
		regen_amount = regen_amount * 2;
	}

	/* Searching or Resting */
	if (p_ptr->searching || p_ptr->resting)
	{
		regen_amount = regen_amount * 2;
	}

	/* Regenerate the mana */
	if (p_ptr->csp < p_ptr->msp)
	{
		regenmana(regen_amount);
	}

	/* Various things interfere with healing */
	if (p_ptr->paralyzed) regen_amount = 0;
	if (p_ptr->poisoned) regen_amount = 0;
	if (p_ptr->stun) regen_amount = 0;
	if (p_ptr->cut) regen_amount = 0;

	/* Regenerate Hit Points if needed */
	if (p_ptr->chp < p_ptr->mhp)
	{
		regenhp(regen_amount);
	}


	/*** Timeout Various Things ***/

	/* Hack -- Hallucinating */
	if (p_ptr->image)
	{
		(void)set_image(p_ptr->image - 1);
	}

	/* Blindness */
	if (p_ptr->blind)
	{
		(void)set_blind(p_ptr->blind - 1);
	}

	/* Times see-invisible */
	if (p_ptr->tim_invis)
	{
		(void)set_tim_invis(p_ptr->tim_invis - 1);
	}

	/* Timed infra-vision */
	if (p_ptr->tim_infra)
	{
		(void)set_tim_infra(p_ptr->tim_infra - 1);
	}

	/* Paralysis */
	if (p_ptr->paralyzed)
	{
		(void)set_paralyzed(p_ptr->paralyzed - 1);
	}

	/* Confusion */
	if (p_ptr->confused)
	{
		(void)set_confused(p_ptr->confused - 1);
	}

	/* Afraid */
	if (p_ptr->afraid)
	{
		(void)set_afraid(p_ptr->afraid - 1);
	}

	/* Fast */
	if (p_ptr->fast)
	{
		(void)set_fast(p_ptr->fast - 1);
	}

	/* Slow */
	if (p_ptr->slow)
	{
		(void)set_slow(p_ptr->slow - 1);
	}

	/* Protection from evil */
	if (p_ptr->protevil)
	{
		(void)set_protevil(p_ptr->protevil - 1);
	}

	/* Invulnerability */
	if (p_ptr->invuln)
	{
		(void)set_invuln(p_ptr->invuln - 1);
	}

	/* Heroism */
	if (p_ptr->hero)
	{
		(void)set_hero(p_ptr->hero - 1);
	}

	/* Super Heroism */
	if (p_ptr->shero)
	{
		(void)set_shero(p_ptr->shero - 1);
	}

	/* Blessed */
	if (p_ptr->blessed)
	{
		(void)set_blessed(p_ptr->blessed - 1);
	}

	/* Shield */
	if (p_ptr->shield)
	{
		(void)set_shield(p_ptr->shield - 1);
	}

	/* Oppose Acid */
	if (p_ptr->oppose_acid)
	{
		(void)set_oppose_acid(p_ptr->oppose_acid - 1);
	}

	/* Oppose Lightning */
	if (p_ptr->oppose_elec)
	{
		(void)set_oppose_elec(p_ptr->oppose_elec - 1);
	}

	/* Oppose Fire */
	if (p_ptr->oppose_fire)
	{
		(void)set_oppose_fire(p_ptr->oppose_fire - 1);
	}

	/* Oppose Cold */
	if (p_ptr->oppose_cold)
	{
		(void)set_oppose_cold(p_ptr->oppose_cold - 1);
	}

	/* Oppose Poison */
	if (p_ptr->oppose_pois)
	{
		(void)set_oppose_pois(p_ptr->oppose_pois - 1);
	}


	/*** Poison and Stun and Cut ***/

	/* Poison */
	if (p_ptr->poisoned)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Apply some healing */
		(void)set_poisoned(p_ptr->poisoned - adjust);
	}

	/* Stun */
	if (p_ptr->stun)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Apply some healing */
		(void)set_stun(p_ptr->stun - adjust);
	}

	/* Cut */
	if (p_ptr->cut)
	{
		int adjust = (adj_con_fix[p_ptr->stat_ind[A_CON]] + 1);

		/* Hack -- Truly "mortal" wound */
		if (p_ptr->cut > 1000) adjust = 0;

		/* Apply some healing */
		(void)set_cut(p_ptr->cut - adjust);
	}



	/*** Process Light ***/

	/* Check for light being wielded */
	o_ptr = &inventory[INVEN_LITE];

	/* Burn some fuel in the current lite */
	if (o_ptr->tval == TV_LITE)
	{
		/* Hack -- Use some fuel (except on artifacts) */
		if (!artifact_p(o_ptr) && (o_ptr->pval > 0))
		{
			/* Decrease life-span */
			o_ptr->pval--;

			/* Hack -- notice interesting fuel steps */
			if ((o_ptr->pval < 100) || (!(o_ptr->pval % 100)))
			{
				/* Window stuff */
				p_ptr->window |= (PW_EQUIP);
			}

			/* Hack -- Special treatment when blind */
			if (p_ptr->blind)
			{
				/* Hack -- save some light for later */
				if (o_ptr->pval == 0) o_ptr->pval++;
			}

			/* The light is now out */
			else if (o_ptr->pval == 0)
			{
				disturb(0, 0);
				msg_print("Your light has gone out!");
			}

			/* The light is getting dim */
			else if ((o_ptr->pval < 100) && (!(o_ptr->pval % 10)))
			{
				if (disturb_minor) disturb(0, 0);
				msg_print("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);


	/*** Process Inventory ***/

	/* Handle experience draining */
	if (p_ptr->exp_drain)
	{
		if ((rand_int(100) < 10) && (p_ptr->exp > 0))
		{
			p_ptr->exp--;
			p_ptr->max_exp--;
			check_experience();
		}
	}

	/* Process equipment */
	for (j = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Get the object */
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Recharge activatable objects */
		if (o_ptr->timeout > 0)
		{
			/* Recharge */
			o_ptr->timeout--;

			/* Notice changes */
			if (!(o_ptr->timeout)) j++;
		}
	}

	/* Notice changes */
	if (j)
	{
		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}

	/* Recharge rods */
	for (j = 0, i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Examine all charging rods */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->pval))
		{
			/* Charge it */
			o_ptr->pval--;

			/* Notice changes */
			if (!(o_ptr->pval)) j++;
		}
	}

	/* Notice changes */
	if (j)
	{
		/* Combine pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/* Feel the inventory */
	sense_inventory();


	/*** Process Objects ***/

	/* Process objects */
	for (i = 1; i < o_max; i++)
	{
		/* Get the object */
		o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Recharge rods on the ground */
		if ((o_ptr->tval == TV_ROD) && (o_ptr->pval)) o_ptr->pval--;
	}


	/*** Involuntary Movement ***/

	/* Mega-Hack -- Random teleportation XXX XXX XXX */
	if ((p_ptr->teleport) && (rand_int(100) < 1))
	{
		/* Teleport player */
		teleport_player(40);
	}

	/* Delayed Word-of-Recall */
	if (p_ptr->word_recall)
	{
		/* Count down towards recall */
		p_ptr->word_recall--;

		/* Activate the recall */
		if (!p_ptr->word_recall)
		{
			/* Disturbing! */
			disturb(0, 0);

			/* Sound */
			sound(MSG_TPLEVEL);

			/* Determine the level */
			if (p_ptr->depth)
			{
				msg_print("You feel yourself yanked upwards!");

				/* New depth */
				p_ptr->depth = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
			else
			{
				msg_print("You feel yourself yanked downwards!");

				/* New depth */
				p_ptr->depth = p_ptr->max_depth;
				if (p_ptr->depth < 1) p_ptr->depth = 1;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}
	}
}



/*
 * Verify use of "wizard" mode
 */
static bool enter_wizard_mode(void)
{
	/* Ask first time */
	if (verify_special && !(p_ptr->noscore & 0x0002))
	{
		/* Mention effects */
		msg_print("You are about to enter 'wizard' mode for the very first time!");
		msg_print("This is a form of cheating, and your game will not be scored!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
		{
			return (FALSE);
		}
	}

	/* Mark savefile */
	p_ptr->noscore |= 0x0002;

	/* Success */
	return (TRUE);
}



#ifdef ALLOW_DEBUG

/*
 * Verify use of "debug" mode
 */
static bool verify_debug_mode(void)
{
	/* Ask first time */
	if (verify_special && !(p_ptr->noscore & 0x0008))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, debug commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the debug commands? "))
		{
			return (FALSE);
		}
	}

	/* Mark savefile */
	p_ptr->noscore |= 0x0008;

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- Declare the Debug Routines
 */
extern void do_cmd_debug(void);

#endif



#ifdef ALLOW_BORG

/*
 * Verify use of "borg" mode
 */
static bool verify_borg_mode(void)
{
	/* Ask first time */
	if (verify_special && !(p_ptr->noscore & 0x0010))
	{
		/* Mention effects */
		msg_print("You are about to use the dangerous, unsupported, borg commands!");
		msg_print("Your machine may crash, and your savefile may become corrupted!");
		message_flush();

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
		{
			return (FALSE);
		}
	}

	/* Mark savefile */
	p_ptr->noscore |= 0x0010;

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- Declare the Borg Routines
 */
extern void do_cmd_borg(void);

#endif



/*
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
static void process_command(void)
{

#ifdef ALLOW_REPEAT

	/* Handle repeating the last command */
	repeat_check();

#endif /* ALLOW_REPEAT */

	/* Parse the command */
	switch (p_ptr->command_cmd)
	{
		/* Ignore */
		case ESCAPE:
		case ' ':
		case '\n':
		case '\r':
		case '\a':
		{
			break;
		}


		/*** Cheating Commands ***/

		/* Toggle Wizard Mode */
		case KTRL('W'):
		{
			if (p_ptr->wizard)
			{
				p_ptr->wizard = FALSE;
				msg_print("Wizard mode off.");
			}
			else if (enter_wizard_mode())
			{
				p_ptr->wizard = TRUE;
				msg_print("Wizard mode on.");
			}

			/* Update monsters */
			p_ptr->update |= (PU_MONSTERS);

			/* Redraw "title" */
			p_ptr->redraw |= (PR_TITLE);

			break;
		}


#ifdef ALLOW_DEBUG

		/* Special "debug" commands */
		case KTRL('A'):
		{
			if (verify_debug_mode()) do_cmd_debug();
			break;
		}

#endif


#ifdef ALLOW_BORG

		/* Special "borg" commands */
		case KTRL('Z'):
		{
			if (verify_borg_mode()) do_cmd_borg();
			break;
		}

#endif



		/*** Inventory Commands ***/

		/* Wear/wield equipment */
		case 'w':
		{
			do_cmd_wield();
			break;
		}

		/* Take off equipment */
		case 't':
		{
			do_cmd_takeoff();
			break;
		}

		/* Drop an item */
		case 'd':
		{
			do_cmd_drop();
			break;
		}

		/* Destroy an item */
		case 'k':
		{
			do_cmd_destroy();
			break;
		}

		/* Equipment list */
		case 'e':
		{
			do_cmd_equip();
			break;
		}

		/* Inventory list */
		case 'i':
		{
			do_cmd_inven();
			break;
		}


		/*** Various commands ***/

		/* Identify an object */
		case 'I':
		{
			do_cmd_observe();
			break;
		}

		/* Hack -- toggle windows */
		case KTRL('E'):
		{
			toggle_inven_equip();
			break;
		}


		/*** Standard "Movement" Commands ***/

		/* Alter a grid */
		case '+':
		{
			do_cmd_alter();
			break;
		}

		/* Dig a tunnel */
		case 'T':
		{
			do_cmd_tunnel();
			break;
		}

		/* Walk */
		case ';':
		{
			do_cmd_walk();
			break;
		}

		/* Jump */
		case '-':
		{
			do_cmd_jump();
			break;
		}


		/*** Running, Resting, Searching, Staying */

		/* Begin Running -- Arg is Max Distance */
		case '.':
		{
			do_cmd_run();
			break;
		}

		/* Hold still */
		case ',':
		{
			do_cmd_hold();
			break;
		}

		/* Stay still */
		case 'g':
		{
			do_cmd_stay();
			break;
		}

		/* Rest -- Arg is time */
		case 'R':
		{
			do_cmd_rest();
			break;
		}

		/* Search for traps/doors */
		case 's':
		{
			do_cmd_search();
			break;
		}

		/* Toggle search mode */
		case 'S':
		{
			do_cmd_toggle_search();
			break;
		}


		/*** Stairs and Doors and Chests and Traps ***/

		/* Enter store */
		case '_':
		{
			do_cmd_store();
			break;
		}

		/* Go up staircase */
		case '<':
		{
			do_cmd_go_up();
			break;
		}

		/* Go down staircase */
		case '>':
		{
			do_cmd_go_down();
			break;
		}

		/* Open a door or chest */
		case 'o':
		{
			do_cmd_open();
			break;
		}

		/* Close a door */
		case 'c':
		{
			do_cmd_close();
			break;
		}

		/* Jam a door with spikes */
		case 'j':
		{
			do_cmd_spike();
			break;
		}

		/* Bash a door */
		case 'B':
		{
			do_cmd_bash();
			break;
		}

		/* Disarm a trap or chest */
		case 'D':
		{
			do_cmd_disarm();
			break;
		}


		/*** Magic and Prayers ***/

		/* Gain new spells/prayers */
		case 'G':
		{
			do_cmd_study();
			break;
		}

		/* Browse a book */
		case 'b':
		{
			do_cmd_browse();
			break;
		}

		/* Cast a spell */
		case 'm':
		{
			do_cmd_cast();
			break;
		}

		/* Pray a prayer */
		case 'p':
		{
			do_cmd_pray();
			break;
		}


		/*** Use various objects ***/

		/* Inscribe an object */
		case '{':
		{
			do_cmd_inscribe();
			break;
		}

		/* Uninscribe an object */
		case '}':
		{
			do_cmd_uninscribe();
			break;
		}

		/* Activate an artifact */
		case 'A':
		{
			do_cmd_activate();
			break;
		}

		/* Eat some food */
		case 'E':
		{
			do_cmd_eat_food();
			break;
		}

		/* Fuel your lantern/torch */
		case 'F':
		{
			do_cmd_refill();
			break;
		}

		/* Fire an item */
		case 'f':
		{
			do_cmd_fire();
			break;
		}

		/* Throw an item */
		case 'v':
		{
			do_cmd_throw();
			break;
		}

		/* Aim a wand */
		case 'a':
		{
			do_cmd_aim_wand();
			break;
		}

		/* Zap a rod */
		case 'z':
		{
			do_cmd_zap_rod();
			break;
		}

		/* Quaff a potion */
		case 'q':
		{
			do_cmd_quaff_potion();
			break;
		}

		/* Read a scroll */
		case 'r':
		{
			do_cmd_read_scroll();
			break;
		}

		/* Use a staff */
		case 'u':
		{
			do_cmd_use_staff();
			break;
		}


		/*** Looking at Things (nearby or on map) ***/

		/* Full dungeon map */
		case 'M':
		{
			do_cmd_view_map();
			break;
		}

		/* Locate player on map */
		case 'L':
		{
			do_cmd_locate();
			break;
		}

		/* Look around */
		case 'l':
		{
			do_cmd_look();
			break;
		}

		/* Target monster or location */
		case '*':
		{
			do_cmd_target();
			break;
		}



		/*** Help and Such ***/

		/* Help */
		case '?':
		{
			do_cmd_help();
			break;
		}

		/* Identify symbol */
		case '/':
		{
			do_cmd_query_symbol();
			break;
		}

		/* Character description */
		case 'C':
		{
			do_cmd_change_name();
			break;
		}


		/*** System Commands ***/

		/* Hack -- User interface */
		case '!':
		{
			(void)Term_user(0);
			break;
		}

		/* Single line from a pref file */
		case '"':
		{
			do_cmd_pref();
			break;
		}

		/* Interact with macros */
		case '@':
		{
			do_cmd_macros();
			break;
		}

		/* Interact with visuals */
		case '%':
		{
			do_cmd_visuals();
			break;
		}

		/* Interact with colors */
		case '&':
		{
			do_cmd_colors();
			break;
		}

		/* Interact with options */
		case '=':
		{
			do_cmd_options();
			do_cmd_redraw();
			break;
		}


		/*** Misc Commands ***/

		/* Take notes */
		case ':':
		{
			do_cmd_note();
			break;
		}

		/* Version info */
		case 'V':
		{
			do_cmd_version();
			break;
		}

		/* Repeat level feeling */
		case KTRL('F'):
		{
			do_cmd_feeling();
			break;
		}

		/* Show previous message */
		case KTRL('O'):
		{
			do_cmd_message_one();
			break;
		}

		/* Show previous messages */
		case KTRL('P'):
		{
			do_cmd_messages();
			break;
		}

		/* Redraw the screen */
		case KTRL('R'):
		{
			do_cmd_redraw();
			break;
		}

#ifndef VERIFY_SAVEFILE

		/* Hack -- Save and don't quit */
		case KTRL('S'):
		{
			do_cmd_save_game();
			break;
		}

#endif

		/* Save and quit */
		case KTRL('X'):
		{
			/* Stop playing */
			p_ptr->playing = FALSE;

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}

		/* Quit (commit suicide) */
		case 'Q':
		{
			do_cmd_suicide();
			break;
		}

		/* Check knowledge */
		case '~':
		case '|':
		{
			do_cmd_knowledge();
			break;
		}

		/* Load "screen dump" */
		case '(':
		{
			do_cmd_load_screen();
			break;
		}

		/* Save "screen dump" */
		case ')':
		{
			do_cmd_save_screen();
			break;
		}

		/* Hack -- Unknown command */
		default:
		{
			prt("Type '?' for help.", 0, 0);
			break;
		}
	}
}



/*
 * Hack -- helper function for "process_player()"
 *
 * Check for changes in the "monster memory"
 */
static void process_player_aux(void)
{
	static int old_monster_race_idx = 0;

	static u32b	old_flags1 = 0L;
	static u32b	old_flags2 = 0L;
	static u32b	old_flags3 = 0L;
	static u32b	old_flags4 = 0L;
	static u32b	old_flags5 = 0L;
	static u32b	old_flags6 = 0L;

	static byte	old_blows0 = 0;
	static byte	old_blows1 = 0;
	static byte	old_blows2 = 0;
	static byte	old_blows3 = 0;

	static byte	old_cast_innate = 0;
	static byte	old_cast_spell = 0;


	/* Tracking a monster */
	if (p_ptr->monster_race_idx)
	{
		/* Get the monster lore */
		monster_lore *l_ptr = &l_list[p_ptr->monster_race_idx];

		/* Check for change of any kind */
		if ((old_monster_race_idx != p_ptr->monster_race_idx) ||
		    (old_flags1 != l_ptr->flags1) ||
		    (old_flags2 != l_ptr->flags2) ||
		    (old_flags3 != l_ptr->flags3) ||
		    (old_flags4 != l_ptr->flags4) ||
		    (old_flags5 != l_ptr->flags5) ||
		    (old_flags6 != l_ptr->flags6) ||
		    (old_blows0 != l_ptr->blows[0]) ||
		    (old_blows1 != l_ptr->blows[1]) ||
		    (old_blows2 != l_ptr->blows[2]) ||
		    (old_blows3 != l_ptr->blows[3]) ||
		    (old_cast_innate != l_ptr->cast_innate) ||
		    (old_cast_spell != l_ptr->cast_spell))
		{
			/* Memorize old race */
			old_monster_race_idx = p_ptr->monster_race_idx;

			/* Memorize flags */
			old_flags1 = l_ptr->flags1;
			old_flags2 = l_ptr->flags2;
			old_flags3 = l_ptr->flags3;
			old_flags4 = l_ptr->flags4;
			old_flags5 = l_ptr->flags5;
			old_flags6 = l_ptr->flags6;

			/* Memorize blows */
			old_blows0 = l_ptr->blows[0];
			old_blows1 = l_ptr->blows[1];
			old_blows2 = l_ptr->blows[2];
			old_blows3 = l_ptr->blows[3];

			/* Memorize castings */
			old_cast_innate = l_ptr->cast_innate;
			old_cast_spell = l_ptr->cast_spell;

			/* Window stuff */
			p_ptr->window |= (PW_MONSTER);

			/* Window stuff */
			window_stuff();
		}
	}
}


/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 *
 * Notice the annoying code to handle "monster memory" changes,
 * which allows us to avoid having to update the window flags
 * every time we change any internal monster memory field, and
 * also reduces the number of times that the recall window must
 * be redrawn.
 *
 * Note that the code to check for user abort during repeated commands
 * and running and resting can be disabled entirely with an option, and
 * even if not disabled, it will only check during every 128th game turn
 * while resting, for efficiency.
 */
static void process_player(void)
{
	int i;


	/*** Check for interrupts ***/

	/* Complete resting */
	if (p_ptr->resting < 0)
	{
		/* Basic resting */
		if (p_ptr->resting == -1)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
			    (p_ptr->csp == p_ptr->msp))
			{
				disturb(0, 0);
			}
		}

		/* Complete resting */
		else if (p_ptr->resting == -2)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
			    (p_ptr->csp == p_ptr->msp) &&
			    !p_ptr->blind && !p_ptr->confused &&
			    !p_ptr->poisoned && !p_ptr->afraid &&
			    !p_ptr->stun && !p_ptr->cut &&
			    !p_ptr->slow && !p_ptr->paralyzed &&
			    !p_ptr->image && !p_ptr->word_recall)
			{
				disturb(0, 0);
			}
		}
	}

	/* Handle "abort" */
	if (!avoid_abort)
	{
		/* Check for "player abort" */
		if (p_ptr->running ||
		    p_ptr->command_rep ||
		    (p_ptr->resting && !(turn & 0x7F)))
		{
			/* Do not wait */
			inkey_scan = TRUE;

			/* Check for a key */
			if (inkey())
			{
				/* Flush input */
				flush();

				/* Disturb */
				disturb(0, 0);

				/* Hack -- Show a Message */
				msg_print("Cancelled.");
			}
		}
	}


	/*** Handle actual user input ***/

	/* Repeat until energy is reduced */
	do
	{
		/* Notice stuff (if needed) */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff (if needed) */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff (if needed) */
		if (p_ptr->window) window_stuff();


		/* Place the cursor on the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Refresh (optional) */
		if (fresh_before) Term_fresh();


		/* Hack -- Pack Overflow */
		if (inventory[INVEN_PACK].k_idx)
		{
			int item = INVEN_PACK;

			char o_name[80];

			object_type *o_ptr;

			/* Get the slot to be dropped */
			o_ptr = &inventory[item];

			/* Disturbing */
			disturb(0, 0);

			/* Warning */
			msg_print("Your pack overflows!");

			/* Describe */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Message */
			msg_format("You drop %s (%c).", o_name, index_to_label(item));

			/* Drop it (carefully) near the player */
			drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);

			/* Modify, Describe, Optimize */
			inven_item_increase(item, -255);
			inven_item_describe(item);
			inven_item_optimize(item);

			/* Notice stuff (if needed) */
			if (p_ptr->notice) notice_stuff();

			/* Update stuff (if needed) */
			if (p_ptr->update) update_stuff();

			/* Redraw stuff (if needed) */
			if (p_ptr->redraw) redraw_stuff();

			/* Window stuff (if needed) */
			if (p_ptr->window) window_stuff();
		}


		/* Hack -- cancel "lurking browse mode" */
		if (!p_ptr->command_new) p_ptr->command_see = FALSE;


		/* Assume free turn */
		p_ptr->energy_use = 0;


		/* Paralyzed or Knocked Out */
		if ((p_ptr->paralyzed) || (p_ptr->stun >= 100))
		{
			/* Take a turn */
			p_ptr->energy_use = 100;
		}

		/* Resting */
		else if (p_ptr->resting)
		{
			/* Timed rest */
			if (p_ptr->resting > 0)
			{
				/* Reduce rest count */
				p_ptr->resting--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);
			}

			/* Take a turn */
			p_ptr->energy_use = 100;
		}

		/* Running */
		else if (p_ptr->running)
		{
			/* Take a step */
			run_step(0);
		}

		/* Repeated command */
		else if (p_ptr->command_rep)
		{
			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command();

			/* Count this execution */
			if (p_ptr->command_rep)
			{
				/* Count this execution */
				p_ptr->command_rep--;

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATE);

				/* Redraw stuff */
				/* redraw_stuff(); */
			}
		}

		/* Normal command */
		else
		{
			/* Check monster recall */
			process_player_aux();

			/* Place the cursor on the player */
			move_cursor_relative(p_ptr->py, p_ptr->px);

			/* Get a command (normal) */
			request_command(FALSE);

			/* Process the command */
			process_command();
		}


		/*** Clean up ***/

		/* Significant */
		if (p_ptr->energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= p_ptr->energy_use;


			/* Hack -- constant hallucination */
			if (p_ptr->image) p_ptr->redraw |= (PR_MAP);


			/* Shimmer monsters if needed */
			if (!avoid_other && shimmer_monsters)
			{
				/* Clear the flag */
				shimmer_monsters = FALSE;

				/* Shimmer multi-hued monsters */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;
					monster_race *r_ptr;

					/* Get the monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					if (!m_ptr->r_idx) continue;

					/* Get the monster race */
					r_ptr = &r_info[m_ptr->r_idx];

					/* Skip non-multi-hued monsters */
					if (!(r_ptr->flags1 & (RF1_ATTR_MULTI))) continue;

					/* Reset the flag */
					shimmer_monsters = TRUE;

					/* Redraw regardless */
					lite_spot(m_ptr->fy, m_ptr->fx);
				}
			}

			/* Repair "nice" flags */
			if (repair_mflag_nice)
			{
				/* Clear flag */
				repair_mflag_nice = FALSE;

				/* Process monsters */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;

					/* Get the monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					/* if (!m_ptr->r_idx) continue; */

					/* Clear "nice" flag */
					m_ptr->mflag &= ~(MFLAG_NICE);
				}
			}

			/* Repair "mark" flags */
			if (repair_mflag_mark)
			{
				/* Reset the flag */
				repair_mflag_mark = FALSE;

				/* Process the monsters */
				for (i = 1; i < m_max; i++)
				{
					monster_type *m_ptr;

					/* Get the monster */
					m_ptr = &m_list[i];

					/* Skip dead monsters */
					/* if (!m_ptr->r_idx) continue; */

					/* Repair "mark" flag */
					if (m_ptr->mflag & (MFLAG_MARK))
					{
						/* Skip "show" monsters */
						if (m_ptr->mflag & (MFLAG_SHOW))
						{
							/* Repair "mark" flag */
							repair_mflag_mark = TRUE;

							/* Skip */
							continue;
						}

						/* Forget flag */
						m_ptr->mflag &= ~(MFLAG_MARK);

						/* Update the monster */
						update_mon(i, FALSE);
					}
				}
			}
		}

		/* Repair "show" flags */
		if (repair_mflag_show)
		{
			/* Reset the flag */
			repair_mflag_show = FALSE;

			/* Process the monsters */
			for (i = 1; i < m_max; i++)
			{
				monster_type *m_ptr;

				/* Get the monster */
				m_ptr = &m_list[i];

				/* Skip dead monsters */
				/* if (!m_ptr->r_idx) continue; */

				/* Clear "show" flag */
				m_ptr->mflag &= ~(MFLAG_SHOW);
			}
		}
	}
	while (!p_ptr->energy_use && !p_ptr->leaving);
}



/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
static void dungeon(void)
{
	monster_type *m_ptr;
	int i;

	int py = p_ptr->py;
	int px = p_ptr->px;


	/* Hack -- enforce illegal panel */
	p_ptr->wy = DUNGEON_HGT;
	p_ptr->wx = DUNGEON_WID;


	/* Not leaving */
	p_ptr->leaving = FALSE;


	/* Reset the "command" vars */
	p_ptr->command_cmd = 0;
	p_ptr->command_new = 0;
	p_ptr->command_rep = 0;
	p_ptr->command_arg = 0;
	p_ptr->command_dir = 0;


	/* Cancel the target */
	target_set_monster(0);

	/* Cancel the health bar */
	health_track(0);


	/* Reset shimmer flags */
	shimmer_monsters = TRUE;
	shimmer_objects = TRUE;

	/* Reset repair flags */
	repair_mflag_born = TRUE;
	repair_mflag_nice = TRUE;
	repair_mflag_show = TRUE;
	repair_mflag_mark = TRUE;


	/* Disturb */
	disturb(1, 0);


	/* Track maximum player level */
	if (p_ptr->max_lev < p_ptr->lev)
	{
		p_ptr->max_lev = p_ptr->lev;
	}


	/* Track maximum dungeon level */
	if (p_ptr->max_depth < p_ptr->depth)
	{
		p_ptr->max_depth = p_ptr->depth;
	}


	/* No stairs down from Quest */
	if (is_quest(p_ptr->depth))
	{
		p_ptr->create_down_stair = FALSE;
	}

	/* No stairs from town or if not allowed */
	if (!p_ptr->depth || !dungeon_stair)
	{
		p_ptr->create_down_stair = p_ptr->create_up_stair = FALSE;
	}

	/* Make a staircase */
	if (p_ptr->create_down_stair || p_ptr->create_up_stair)
	{
		/* Place a staircase */
		if (cave_valid_bold(py, px))
		{
			/* XXX XXX XXX */
			delete_object(py, px);

			/* Make stairs */
			if (p_ptr->create_down_stair)
			{
				cave_set_feat(py, px, FEAT_MORE);
			}
			else
			{
				cave_set_feat(py, px, FEAT_LESS);
			}

			/* Mark the stairs as known */
			cave_info[py][px] |= (CAVE_MARK);
		}

		/* Cancel the stair request */
		p_ptr->create_down_stair = p_ptr->create_up_stair = FALSE;
	}


	/* Choose panel */
	verify_panel();


	/* Flush messages */
	message_flush();


	/* Hack -- Increase "xtra" depth */
	character_xtra++;


	/* Clear */
	Term_clear();


	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	update_stuff();


	/* Fully update the visuals (and monster distances) */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_DISTANCE);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Redraw stuff */
	window_stuff();


	/* Hack -- Decrease "xtra" depth */
	character_xtra--;


	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Combine / Reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Notice stuff */
	notice_stuff();

	/* Update stuff */
	update_stuff();

	/* Redraw stuff */
	redraw_stuff();

	/* Window stuff */
	window_stuff();

	/* Refresh */
	Term_fresh();


	/* Handle delayed death */
	if (p_ptr->is_dead) return;


	/* Announce (or repeat) the feeling */
	if (p_ptr->depth) do_cmd_feeling();


	/*** Process this dungeon level ***/

	/* Reset the monster generation level */
	monster_level = p_ptr->depth;

	/* Reset the object generation level */
	object_level = p_ptr->depth;

	/* Main loop */
	while (TRUE)
	{
		/* Hack -- Compact the monster list occasionally */
		if (m_cnt + 32 > z_info->m_max) compact_monsters(64);

		/* Hack -- Compress the monster list occasionally */
		if (m_cnt + 32 < m_max) compact_monsters(0);


		/* Hack -- Compact the object list occasionally */
		if (o_cnt + 32 > z_info->o_max) compact_objects(64);

		/* Hack -- Compress the object list occasionally */
		if (o_cnt + 32 < o_max) compact_objects(0);


		/*** Apply energy ***/

		/* Give the player some energy */
		p_ptr->energy += extract_energy[p_ptr->pspeed];

		/* Give energy to all monsters */
		for (i = m_max - 1; i >= 1; i--)
		{
			/* Access the monster */
			m_ptr = &m_list[i];

			/* Ignore "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Give this monster some energy */
			m_ptr->energy += extract_energy[m_ptr->mspeed];
		}


		/* Can the player move? */
		while ((p_ptr->energy >= 100) && !p_ptr->leaving)
		{
			/* process monster with even more energy first */
			process_monsters((byte)(p_ptr->energy + 1));

			/* if still alive */
			if (!p_ptr->leaving)
			{
				/* Process the player */
				process_player();
			}
		}

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process all of the monsters */
		process_monsters(100);

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Redraw stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process the world */
		process_world();

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();

		/* Hack -- Hilite the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Optional fresh */
		if (fresh_after) Term_fresh();

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Count game turns */
		turn++;
	}
}



/*
 * Process some user pref files
 */
static void process_some_user_pref_files(void)
{
	char buf[1024];


	/* Process the "user.prf" file */
	(void)process_pref_file("user.prf");

	/* Get the "PLAYER.prf" filename */
	sprintf(buf, "%s.prf", op_ptr->base_name);

	/* Process the "PLAYER.prf" file */
	(void)process_pref_file(buf);
}


/*
 * Actually play a game.
 *
 * This function is called from a variety of entry points, since both
 * the standard "main.c" file, as well as several platform-specific
 * "main-xxx.c" files, call this function to start a new game with a
 * new savefile, start a new game with an existing savefile, or resume
 * a saved game with an existing savefile.
 *
 * If the "new_game" parameter is true, and the savefile contains a
 * living character, then that character will be killed, so that the
 * player may start a new game with that savefile.  This is only used
 * by the "-n" option in "main.c".
 *
 * If the savefile does not exist, cannot be loaded, or contains a dead
 * (non-wizard-mode) character, then a new game will be started.
 *
 * Several platforms (Windows, Macintosh, Amiga) start brand new games
 * with "savefile" and "op_ptr->base_name" both empty, and initialize
 * them later based on the player name.  To prevent weirdness, we must
 * initialize "op_ptr->base_name" to "PLAYER" if it is empty.
 *
 * Note that we load the RNG state from savefiles (2.8.0 or later) and
 * so we only initialize it if we were unable to load it.  The loading
 * code marks successful loading of the RNG state using the "Rand_quick"
 * flag, which is a hack, but which optimizes loading of savefiles.
 */
void play_game(bool new_game)
{
	/* Hack -- Increase "icky" depth */
	character_icky++;


	/* Verify main term */
	if (!term_screen)
	{
		quit("main window does not exist");
	}

	/* Make sure main term is active */
	Term_activate(term_screen);

	/* Verify minimum size */
	if ((Term->hgt < 24) || (Term->wid < 80))
	{
		quit("main window is too small");
	}

	/* Hack -- Turn off the cursor */
	(void)Term_set_cursor(0);


	/* Attempt to load */
	if (!load_player())
	{
		/* Oops */
		quit("broken savefile");
	}

	/* Nothing loaded */
	if (!character_loaded)
	{
		/* Make new player */
		new_game = TRUE;

		/* The dungeon is not ready */
		character_dungeon = FALSE;
	}

	/* Hack -- Default base_name */
	if (!op_ptr->base_name[0])
	{
		strcpy(op_ptr->base_name, "PLAYER");
	}

	/* Init RNG */
	if (Rand_quick)
	{
		u32b seed;

		/* Basic seed */
		seed = (time(NULL));

#ifdef SET_UID

		/* Mutate the seed on Unix machines */
		seed = ((seed >> 3) * (getpid() << 1));

#endif

		/* Use the complex RNG */
		Rand_quick = FALSE;

		/* Seed the "complex" RNG */
		Rand_state_init(seed);
	}

	/* Roll new character */
	if (new_game)
	{
		/* The dungeon is not ready */
		character_dungeon = FALSE;

		/* Start in town */
		p_ptr->depth = 0;

		/* Hack -- seed for flavors */
		seed_flavor = rand_int(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = rand_int(0x10000000);

#ifdef GJW_RANDART

		/* Hack -- seed for random artifacts */
		seed_randart = rand_int(0x10000000);

#endif /* GJW_RANDART */

		/* Roll up a new character */
		player_birth();

#ifdef GJW_RANDART

		/* Randomize the artifacts */
		if (adult_rand_artifacts)
		{
			do_randart(seed_randart, TRUE);
		}

#else /* GJW_RANDART */

		/* Make sure random artifacts are turned off if not available */
		adult_rand_artifacts = FALSE;

#endif /* GJW_RANDART */

		/* Hack -- enter the world */
		turn = 1;
	}

	/* Normal machine (process player name) */
	if (savefile[0])
	{
		process_player_name(FALSE);
	}

	/* Weird machine (process player name, pick savefile name) */
	else
	{
		process_player_name(TRUE);
	}

	/* Flash a message */
	prt("Please wait...", 0, 0);

	/* Flush the message */
	Term_fresh();


	/* Hack -- Enter wizard mode */
	if (arg_wizard && enter_wizard_mode()) p_ptr->wizard = TRUE;


	/* Flavor the objects */
	flavor_init();

	/* Reset visuals */
	reset_visuals(TRUE);


	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER | PW_MESSAGE);

	/* Window stuff */
	window_stuff();


	/* Process some user pref files */
	process_some_user_pref_files();


	/* Set or clear "rogue_like_commands" if requested */
	if (arg_force_original) rogue_like_commands = FALSE;
	if (arg_force_roguelike) rogue_like_commands = TRUE;


	/* React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Generate a dungeon level if needed */
	if (!character_dungeon) generate_cave();


	/* Character is now "complete" */
	character_generated = TRUE;


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Start playing */
	p_ptr->playing = TRUE;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) p_ptr->is_dead = TRUE;

	/* Process */
	while (TRUE)
	{
		/* Process the level */
		dungeon();


		/* Notice stuff */
		if (p_ptr->notice) notice_stuff();

		/* Update stuff */
		if (p_ptr->update) update_stuff();

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff();

		/* Window stuff */
		if (p_ptr->window) window_stuff();


		/* Cancel the target */
		target_set_monster(0);

		/* Cancel the health bar */
		health_track(0);


		/* Forget the view */
		forget_view();


		/* Handle "quit and save" */
		if (!p_ptr->playing && !p_ptr->is_dead) break;


		/* Erase the old cave */
		wipe_o_list();
		wipe_m_list();


		/* XXX XXX XXX */
		message_flush();

		/* Accidental Death */
		if (p_ptr->playing && p_ptr->is_dead)
		{
			/* Mega-Hack -- Allow player to cheat death */
			if ((p_ptr->wizard || cheat_live) && !get_check("Die? "))
			{
				/* Mark social class, reset age, if needed */
				if (p_ptr->sc) p_ptr->sc = p_ptr->age = 0;

				/* Increase age */
				p_ptr->age++;

				/* Mark savefile */
				p_ptr->noscore |= 0x0001;

				/* Message */
				msg_print("You invoke wizard mode and cheat death.");
				message_flush();

				/* Cheat death */
				p_ptr->is_dead = FALSE;

				/* Restore hit points */
				p_ptr->chp = p_ptr->mhp;
				p_ptr->chp_frac = 0;

				/* Restore spell points */
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;

				/* Hack -- Healing */
				(void)set_blind(0);
				(void)set_confused(0);
				(void)set_poisoned(0);
				(void)set_afraid(0);
				(void)set_paralyzed(0);
				(void)set_image(0);
				(void)set_stun(0);
				(void)set_cut(0);

				/* Hack -- Prevent starvation */
				(void)set_food(PY_FOOD_MAX - 1);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg_print("A tension leaves the air around you...");
					message_flush();

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Note cause of death XXX XXX XXX */
				strcpy(p_ptr->died_from, "Cheating death");

				/* New depth */
				p_ptr->depth = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}

		/* Handle "death" */
		if (p_ptr->is_dead) break;

		/* Make a new level */
		generate_cave();
	}

	/* Close stuff */
	close_game();
}
