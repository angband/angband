/*
 * File: dungeon.c
 * Purpose: The game core bits, shared across platforms.
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
#include "button.h"
#include "cave.h"
#include "cmds.h"
#include "files.h"
#include "game-event.h"
#include "generate.h"
#include "init.h"
#include "monster/monster.h"
#include "monster/mon-spell.h"
#include "object/tvalsval.h"
#include "prefs.h"
#include "savefile.h"
#include "spells.h"
#include "target.h"

/*
 * Change dungeon level - e.g. by going up stairs or with WoR.
 */
void dungeon_change_level(int dlev)
{
	/* New depth */
	p_ptr->depth = dlev;

	/* If we're returning to town, update the store contents
	   according to how long we've been away */
	if (!dlev && daycount)
	{
		if (OPT(cheat_xtra)) msg("Updating Shops...");
		while (daycount--)
		{
			int n;

			/* Maintain each shop (except home) */
			for (n = 0; n < MAX_STORES; n++)
			{
				/* Skip the home */
				if (n == STORE_HOME) continue;

				/* Maintain */
				store_maint(&stores[n]);
			}

			/* Sometimes, shuffle the shop-keepers */
			if (one_in_(STORE_SHUFFLE))
			{
				/* Message */
				if (OPT(cheat_xtra)) msg("Shuffling a Shopkeeper...");

				/* Pick a random shop (except home) */
				while (1)
				{
					n = randint0(MAX_STORES);
					if (n != STORE_HOME) break;
				}

				/* Shuffle it */
				store_shuffle(&stores[n]);
			}
		}
		daycount = 0;
		if (OPT(cheat_xtra)) msg("Done.");
	}

	/* Leaving */
	p_ptr->leaving = TRUE;

	/* Save the game when we arrive on the new level. */
	p_ptr->autosave = TRUE;
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
		wieldeds_notice_flag(p_ptr, OF_REGEN);
		wieldeds_notice_flag(p_ptr, OF_IMPAIR_HP);
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
		wieldeds_notice_flag(p_ptr, OF_REGEN);
		wieldeds_notice_flag(p_ptr, OF_IMPAIR_MANA);
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
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		/* Check the i'th monster */
		monster_type *m_ptr = cave_monster(cave, i);
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
			if (rf_has(r_ptr->flags, RF_REGENERATE)) frac *= 2;

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
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 * Also inform player when first item of a stack has recharged. -HK-
 * Notify all recharges w/o inscription if notify_recharge option set -WP-
 */
static void recharged_notice(const object_type *o_ptr, bool all)
{
	char o_name[120];

	const char *s;

	bool notify = FALSE;

	if (OPT(notify_recharge))
	{
		notify = TRUE;
	}
	else if (o_ptr->note)
	{
		/* Find a '!' */
		s = strchr(quark_str(o_ptr->note), '!');

		/* Process notification request */
		while (s)
		{
			/* Find another '!' */
			if (s[1] == '!')
			{
				notify = TRUE;
				break;
			}

			/* Keep looking for '!'s */
			s = strchr(s + 1, '!');
		}
	}

	if (!notify) return;


	/* Describe (briefly) */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Disturb the player */
	disturb(p_ptr, 0, 0);

	/* Notify the player */
	if (o_ptr->number > 1)
	{
		if (all) msg("Your %s have recharged.", o_name);
		else msg("One of your %s has recharged.", o_name);
	}

	/* Artifacts */
	else if (o_ptr->artifact)
	{
		msg("The %s has recharged.", o_name);
	}

	/* Single, non-artifact items */
	else msg("Your %s has recharged.", o_name);
}


/*
 * Recharge activatable objects in the player's equipment
 * and rods in the inventory and on the ground.
 */
static void recharge_objects(void)
{
	int i;

	bool charged = FALSE, discharged_stack;

	object_type *o_ptr;

	/*** Recharge equipment ***/
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Get the object */
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Recharge activatable objects */
		if (recharge_timeout(o_ptr))
		{
			charged = TRUE;

			/* Message if an item recharged */
			recharged_notice(o_ptr, TRUE);
		}
	}

	/* Notice changes */
	if (charged)
	{
		/* Window stuff */
		p_ptr->redraw |= (PR_EQUIP);
	}

	charged = FALSE;

	/*** Recharge the inventory ***/
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		discharged_stack = (number_charging(o_ptr) == o_ptr->number) ? TRUE : FALSE;

		/* Recharge rods, and update if any rods are recharged */
		if (o_ptr->tval == TV_ROD && recharge_timeout(o_ptr))
		{
			charged = TRUE;

			/* Entire stack is recharged */
			if (o_ptr->timeout == 0)
				recharged_notice(o_ptr, TRUE);

			/* Previously exhausted stack has acquired a charge */
			else if (discharged_stack)
				recharged_notice(o_ptr, FALSE);
		}
	}

	/* Notice changes */
	if (charged)
	{
		/* Combine pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_INVEN);
	}

	/*** Recharge the ground ***/
	for (i = 1; i < o_max; i++)
	{
		/* Get the object */
		o_ptr = object_byid(i);

		/* Skip dead objects */
		if (!o_ptr->kind) continue;

		/* Recharge rods on the ground */
		if (o_ptr->tval == TV_ROD)
			recharge_timeout(o_ptr);
	}
}


static void play_ambient_sound(void)
{
	/* Town sound */
	if (p_ptr->depth == 0) 
	{
		/* Hack - is it daytime or nighttime? */
		if (turn % (10L * TOWN_DAWN) < TOWN_DAWN / 2)
		{
			/* It's day. */
			sound(MSG_AMBIENT_DAY);
		} 
		else 
		{
			/* It's night. */
			sound(MSG_AMBIENT_NITE);
		}
		
	}

	/* Dungeon level 1-20 */
	else if (p_ptr->depth <= 20) 
	{
		sound(MSG_AMBIENT_DNG1);
	}

	/* Dungeon level 21-40 */
	else if (p_ptr->depth <= 40) 
	{
		sound(MSG_AMBIENT_DNG2);
	}

	/* Dungeon level 41-60 */
	else if (p_ptr->depth <= 60) 
	{
		sound(MSG_AMBIENT_DNG3);
	}

	/* Dungeon level 61-80 */
	else if (p_ptr->depth <= 80)
	{
		sound(MSG_AMBIENT_DNG4);
	}

	/* Dungeon level 80- */
	else
	{
		sound(MSG_AMBIENT_DNG5);
	}
}

/*
 * Helper for process_world -- decrement p_ptr->timed[] fields.
 */
static void decrease_timeouts(void)
{
	int adjust = (adj_con_fix[p_ptr->state.stat_ind[A_CON]] + 1);
	int i;

	/* Decrement all effects that can be done simply */
	for (i = 0; i < TMD_MAX; i++)
	{
		int decr = 1;
		if (!p_ptr->timed[i])
			continue;

		switch (i)
		{
			case TMD_CUT:
			{
				/* Hack -- check for truly "mortal" wound */
				decr = (p_ptr->timed[i] > 1000) ? 0 : adjust;
				break;
			}

			case TMD_POISONED:
			case TMD_STUN:
			{
				decr = adjust;
				break;
			}
		}
		/* Decrement the effect */
		player_dec_timed(p_ptr, i, decr, FALSE);
	}

	return;
}


/*
 * Handle certain things once every 10 game turns
 */
static void process_world(struct cave *c)
{
	int i;

	int regen_amount;

	object_type *o_ptr;

	/* Every 10 game turns */
	if (turn % 10) return;


	/*** Check the Time ***/

	/* Play an ambient sound at regular intervals. */
	if (!(turn % ((10L * TOWN_DAWN) / 4)))
	{
		play_ambient_sound();
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
				msg("The sun has risen.");

			/* Night falls */
			else
				msg("The sun has fallen.");

			/* Illuminate */
			cave_illuminate(c, dawn);
		}
	}


	/* While in the dungeon */
	else
	{
		/* Update the stores once a day (while in the dungeon).
		   The changes are not actually made until return to town,
		   to avoid giving details away in the knowledge menu. */
		if (!(turn % (10L * STORE_TURNS))) daycount++;
	}


	/*** Process the monsters ***/

	/* Check for creature generation */
	if (one_in_(MAX_M_ALLOC_CHANCE))
	{
		/* Make a new monster */
		(void)alloc_monster(cave, loc(p_ptr->px, p_ptr->py), MAX_SIGHT + 5, FALSE, p_ptr->depth);
	}

	/* Hack -- Check for creature regeneration */
	if (!(turn % 100)) regen_monsters();


	/*** Damage over Time ***/

	/* Take damage from poison */
	if (p_ptr->timed[TMD_POISONED])
	{
		/* Take damage */
		take_hit(p_ptr, 1, "poison");
	}

	/* Take damage from cuts */
	if (p_ptr->timed[TMD_CUT])
	{
		/* Mortal wound or Deep Gash */
		if (p_ptr->timed[TMD_CUT] > 200)
			i = 3;

		/* Severe cut */
		else if (p_ptr->timed[TMD_CUT] > 100)
			i = 2;

		/* Other cuts */
		else
			i = 1;

		/* Take damage */
		take_hit(p_ptr, i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (p_ptr->food < PY_FOOD_MAX)
	{
		/* Every 100 game turns */
		if (!(turn % 100))
		{
			/* Basic digestion rate based on speed */
			i = extract_energy[p_ptr->state.speed] * 2;

			/* Regeneration takes more food */
			if (check_state(p_ptr, OF_REGEN, p_ptr->state.flags)) i += 30;

			/* Slow digestion takes less food */
			if (check_state(p_ptr, OF_SLOW_DIGEST, p_ptr->state.flags)) i -= 10;

			/* Minimal digestion */
			if (i < 1) i = 1;

			/* Digest some food */
			player_set_food(p_ptr, p_ptr->food - i);
		}
	}

	/* Digest quickly when gorged */
	else
	{
		/* Digest a lot of food */
		player_set_food(p_ptr, p_ptr->food - 100);
	}

	/* Getting Faint */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		/* Faint occasionally */
		if (!p_ptr->timed[TMD_PARALYZED] && one_in_(10))
		{
			/* Message */
			msg("You faint from the lack of food.");
			disturb(p_ptr, 1, 0);

			/* Faint (bypass free action) */
			(void)player_inc_timed(p_ptr, TMD_PARALYZED, 1 + randint0(5), TRUE, FALSE);
		}
	}


	/* Starve to death (slowly) */
	if (p_ptr->food < PY_FOOD_STARVE)
	{
		/* Calculate damage */
		i = (PY_FOOD_STARVE - p_ptr->food) / 10;

		/* Take damage */
		take_hit(p_ptr, i, "starvation");
	}

	/** Regenerate HP **/

	/* Default regeneration */
	if (p_ptr->food >= PY_FOOD_WEAK)
		regen_amount = PY_REGEN_NORMAL;
	else if (p_ptr->food < PY_FOOD_STARVE)
		regen_amount = 0;
	else if (p_ptr->food < PY_FOOD_FAINT)
		regen_amount = PY_REGEN_FAINT;
	else /* if (p_ptr->food < PY_FOOD_WEAK) */
		regen_amount = PY_REGEN_WEAK;

	/* Various things speed up regeneration */
	if (check_state(p_ptr, OF_REGEN, p_ptr->state.flags))
		regen_amount *= 2;
	if (p_ptr->searching || p_ptr->resting)
		regen_amount *= 2;

	/* Some things slow it down */
	if (check_state(p_ptr, OF_IMPAIR_HP, p_ptr->state.flags))
		regen_amount /= 2;

	/* Various things interfere with physical healing */
	if (p_ptr->timed[TMD_PARALYZED]) regen_amount = 0;
	if (p_ptr->timed[TMD_POISONED]) regen_amount = 0;
	if (p_ptr->timed[TMD_STUN]) regen_amount = 0;
	if (p_ptr->timed[TMD_CUT]) regen_amount = 0;

	/* Regenerate Hit Points if needed */
	if (p_ptr->chp < p_ptr->mhp)
		regenhp(regen_amount);


	/** Regenerate SP **/

	/* Default regeneration */
	regen_amount = PY_REGEN_NORMAL;

	/* Various things speed up regeneration */
	if (check_state(p_ptr, OF_REGEN, p_ptr->state.flags))
		regen_amount *= 2;
	if (p_ptr->searching || p_ptr->resting)
		regen_amount *= 2;

	/* Some things slow it down */
	if (check_state(p_ptr, OF_IMPAIR_MANA, p_ptr->state.flags))
		regen_amount /= 2;

	/* Regenerate mana */
	if (p_ptr->csp < p_ptr->msp)
		regenmana(regen_amount);



	/*** Timeout Various Things ***/

	decrease_timeouts();



	/*** Process Light ***/

	/* Check for light being wielded */
	o_ptr = &p_ptr->inventory[INVEN_LIGHT];

	/* Burn some fuel in the current light */
	if (o_ptr->tval == TV_LIGHT)
	{
		bitflag f[OF_SIZE];
		bool burn_fuel = TRUE;

		/* Get the object flags */
		object_flags(o_ptr, f);

		/* Turn off the wanton burning of light during the day in the town */
		if (!p_ptr->depth && ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)))
			burn_fuel = FALSE;

		/* If the light has the NO_FUEL flag, well... */
		if (of_has(f, OF_NO_FUEL))
		    burn_fuel = FALSE;

		/* Use some fuel (except on artifacts, or during the day) */
		if (burn_fuel && o_ptr->timeout > 0)
		{
			/* Decrease life-span */
			o_ptr->timeout--;

			/* Hack -- notice interesting fuel steps */
			if ((o_ptr->timeout < 100) || (!(o_ptr->timeout % 100)))
			{
				/* Redraw stuff */
				p_ptr->redraw |= (PR_EQUIP);
			}

			/* Hack -- Special treatment when blind */
			if (p_ptr->timed[TMD_BLIND])
			{
				/* Hack -- save some light for later */
				if (o_ptr->timeout == 0) o_ptr->timeout++;
			}

			/* The light is now out */
			else if (o_ptr->timeout == 0)
			{
				disturb(p_ptr, 0, 0);
				msg("Your light has gone out!");
			}

			/* The light is getting dim */
			else if ((o_ptr->timeout < 100) && (!(o_ptr->timeout % 10)))
			{
				disturb(p_ptr, 0, 0);
				msg("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p_ptr->update |= (PU_TORCH);


	/*** Process Inventory ***/

	/* Handle experience draining */
	if (check_state(p_ptr, OF_DRAIN_EXP, p_ptr->state.flags))
	{
		if ((p_ptr->exp > 0) && one_in_(10))
			player_exp_lose(p_ptr, 1, FALSE);

		wieldeds_notice_flag(p_ptr, OF_DRAIN_EXP);
	}

	/* Recharge activatable objects and rods */
	recharge_objects();

	/* Feel the inventory */
	sense_inventory();


	/*** Involuntary Movement ***/

	/* Random teleportation */
	if (check_state(p_ptr, OF_TELEPORT, p_ptr->state.flags) && one_in_(100))
	{
		wieldeds_notice_flag(p_ptr, OF_TELEPORT);
		teleport_player(40);
		disturb(p_ptr, 0, 0);
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
			disturb(p_ptr, 0, 0);

			/* Determine the level */
			if (p_ptr->depth)
			{
				msgt(MSG_TPLEVEL, "You feel yourself yanked upwards!");
				dungeon_change_level(0);
			}
			else
			{
				msgt(MSG_TPLEVEL, "You feel yourself yanked downwards!");

				/* New depth - back to max depth or 1, whichever is deeper */
				dungeon_change_level(p_ptr->max_depth < 1 ? 1: p_ptr->max_depth);
			}
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
	int i;
	bool changed = FALSE;

	static int old_monster_race_idx = 0;
	static bitflag old_flags[RF_SIZE];
	static bitflag old_spell_flags[RSF_SIZE];

	static byte old_blows[MONSTER_BLOW_MAX];

	static byte	old_cast_innate = 0;
	static byte	old_cast_spell = 0;

	/* Tracking a monster */
	if (p_ptr->monster_race_idx)
	{
		/* Get the monster lore */
		monster_lore *l_ptr = &l_list[p_ptr->monster_race_idx];

		for (i = 0; i < MONSTER_BLOW_MAX; i++)
		{
			if (old_blows[i] != l_ptr->blows[i])
			{
				changed = TRUE;
				break;
			}
		}

		/* Check for change of any kind */
		if (changed ||
		    (old_monster_race_idx != p_ptr->monster_race_idx) ||
		    !rf_is_equal(old_flags, l_ptr->flags) ||
		    !rsf_is_equal(old_spell_flags, l_ptr->spell_flags) ||
		    (old_cast_innate != l_ptr->cast_innate) ||
		    (old_cast_spell != l_ptr->cast_spell))
		{
			/* Memorize old race */
			old_monster_race_idx = p_ptr->monster_race_idx;

			/* Memorize flags */
			rf_copy(old_flags, l_ptr->flags);
			rsf_copy(old_spell_flags, l_ptr->spell_flags);

			/* Memorize blows */
			memmove(old_blows, l_ptr->blows, sizeof(byte)*MONSTER_BLOW_MAX);

			/* Memorize castings */
			old_cast_innate = l_ptr->cast_innate;
			old_cast_spell = l_ptr->cast_spell;

			/* Redraw stuff */
			p_ptr->redraw |= (PR_MONSTER);
			redraw_stuff(p_ptr);
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
		if (p_ptr->resting == REST_ALL_POINTS)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
			    (p_ptr->csp == p_ptr->msp))
			{
				disturb(p_ptr, 0, 0);
			}
		}

		/* Complete resting */
		else if (p_ptr->resting == REST_COMPLETE)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) &&
			    (p_ptr->csp == p_ptr->msp) &&
			    !p_ptr->timed[TMD_BLIND] && !p_ptr->timed[TMD_CONFUSED] &&
			    !p_ptr->timed[TMD_POISONED] && !p_ptr->timed[TMD_AFRAID] &&
			    !p_ptr->timed[TMD_TERROR] &&
			    !p_ptr->timed[TMD_STUN] && !p_ptr->timed[TMD_CUT] &&
			    !p_ptr->timed[TMD_SLOW] && !p_ptr->timed[TMD_PARALYZED] &&
			    !p_ptr->timed[TMD_IMAGE] && !p_ptr->word_recall)
			{
				disturb(p_ptr, 0, 0);
			}
		}
		
		/* Rest until HP or SP are filled */
		else if (p_ptr->resting == REST_SOME_POINTS)
		{
			/* Stop resting */
			if ((p_ptr->chp == p_ptr->mhp) ||
			    (p_ptr->csp == p_ptr->msp))
			{
				disturb(p_ptr, 0, 0);
			}
		}
	}

	/* Check for "player abort" */
	if (p_ptr->running ||
	    cmd_get_nrepeats() > 0 ||
	    (p_ptr->resting && !(turn & 0x7F)))
	{
		ui_event e;

		/* Do not wait */
		inkey_scan = SCAN_INSTANT;

		/* Check for a key */
		e = inkey_ex();
		if (e.type != EVT_NONE) {
			/* Flush and disturb */
			flush();
			disturb(p_ptr, 0, 0);
			msg("Cancelled.");
		}
	}


	/*** Handle actual user input ***/

	/* Repeat until energy is reduced */
	do
	{
		/* Notice stuff (if needed) */
		if (p_ptr->notice) notice_stuff(p_ptr);

		/* Update stuff (if needed) */
		if (p_ptr->update) update_stuff(p_ptr);

		/* Redraw stuff (if needed) */
		if (p_ptr->redraw) redraw_stuff(p_ptr);


		/* Place the cursor on the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Refresh (optional) */
		Term_fresh();

		/* Hack -- Pack Overflow */
		pack_overflow();

		/* Assume free turn */
		p_ptr->energy_use = 0;

		/* Dwarves detect treasure */
		if (player_has(PF_SEE_ORE))
		{
			/* Only if they are in good shape */
			if (!p_ptr->timed[TMD_IMAGE] &&
					!p_ptr->timed[TMD_CONFUSED] &&
					!p_ptr->timed[TMD_AMNESIA] &&
					!p_ptr->timed[TMD_STUN] &&
					!p_ptr->timed[TMD_PARALYZED] &&
					!p_ptr->timed[TMD_TERROR] &&
					!p_ptr->timed[TMD_AFRAID])
				detect_close_buried_treasure();
		}

		/* Paralyzed or Knocked Out */
		if ((p_ptr->timed[TMD_PARALYZED]) || (p_ptr->timed[TMD_STUN] >= 100))
		{
			/* Take a turn */
			p_ptr->energy_use = 100;
		}

		/* Picking up objects */
		else if (p_ptr->notice & PN_PICKUP)
		{
			p_ptr->energy_use = do_autopickup() * 10;
			if (p_ptr->energy_use > 100)
				p_ptr->energy_use = 100;
			p_ptr->notice &= ~(PN_PICKUP);
			
			/* Appropriate time for the player to see objects */
			event_signal(EVENT_SEEFLOOR);
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

			/* Increment the resting counter */
			p_ptr->resting_turn++;
		}

		/* Running */
		else if (p_ptr->running)
		{
			/* Take a step */
			run_step(0);
		}

		/* Repeated command */
		else if (cmd_get_nrepeats() > 0)
		{
			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command(CMD_GAME, TRUE);
		}

		/* Normal command */
		else
		{
			/* Check monster recall */
			process_player_aux();

			/* Place the cursor on the player */
			move_cursor_relative(p_ptr->py, p_ptr->px);

			/* Get and process a command */
			process_command(CMD_GAME, FALSE);

			/* Mega hack - redraw if big graphics - sorry NRM */
			if ((tile_width > 1) || (tile_height > 1)) 
			        p_ptr->redraw |= (PR_MAP);
		}


		/*** Clean up ***/

		/* Significant */
		if (p_ptr->energy_use)
		{
			/* Use some energy */
			p_ptr->energy -= p_ptr->energy_use;

			/* Increment the total energy counter */
			p_ptr->total_energy += p_ptr->energy_use;

			/* Hack -- constant hallucination */
			if (p_ptr->timed[TMD_IMAGE])
			{
				p_ptr->redraw |= (PR_MAP);
			}

			/* Shimmer multi-hued monsters */
			for (i = 1; i < cave_monster_max(cave); i++)
			{
				struct monster_race *race;
				struct monster *mon = cave_monster(cave, i);
				if (!mon->r_idx)
					continue;
				race = &r_info[mon->r_idx];
				if (!rf_has(race->flags, RF_ATTR_MULTI))
					continue;
				cave_light_spot(cave, mon->fy, mon->fx);
			}

			/* Clear NICE flag, and show marked monsters */
			for (i = 1; i < cave_monster_max(cave); i++)
			{
				struct monster *mon = cave_monster(cave, i);
				mon->mflag &= ~MFLAG_NICE;
				if (mon->mflag & MFLAG_MARK) {
					if (!(mon->mflag & MFLAG_SHOW)) {
						mon->mflag &= ~MFLAG_MARK;
						update_mon(i, FALSE);
					}
				}
			}
		}

		/* Clear SHOW flag */
		for (i = 1; i < cave_monster_max(cave); i++)
		{
			struct monster *mon = cave_monster(cave, i);
			mon->mflag &= ~MFLAG_SHOW;
		}

		/* HACK: This will redraw the itemlist too frequently, but I'm don't
		   know all the individual places it should go. */
		p_ptr->redraw |= PR_ITEMLIST;
	}

	while (!p_ptr->energy_use && !p_ptr->leaving);

	/* Notice stuff (if needed) */
	if (p_ptr->notice) notice_stuff(p_ptr);
}

byte flicker = 0;
byte color_flicker[MAX_COLORS][3] = 
{
	{TERM_DARK, TERM_L_DARK, TERM_L_RED},
	{TERM_WHITE, TERM_L_WHITE, TERM_L_BLUE},
	{TERM_SLATE, TERM_WHITE, TERM_L_DARK},
	{TERM_ORANGE, TERM_YELLOW, TERM_L_RED},
	{TERM_RED, TERM_L_RED, TERM_L_PINK},
	{TERM_GREEN, TERM_L_GREEN, TERM_L_TEAL},
	{TERM_BLUE, TERM_L_BLUE, TERM_SLATE},
	{TERM_UMBER, TERM_L_UMBER, TERM_MUSTARD},
	{TERM_L_DARK, TERM_SLATE, TERM_L_VIOLET},
	{TERM_WHITE, TERM_SLATE, TERM_L_WHITE},
	{TERM_L_PURPLE, TERM_PURPLE, TERM_L_VIOLET},
	{TERM_YELLOW, TERM_L_YELLOW, TERM_MUSTARD},
	{TERM_L_RED, TERM_RED, TERM_L_PINK},
	{TERM_L_GREEN, TERM_L_TEAL, TERM_GREEN},
	{TERM_L_BLUE, TERM_DEEP_L_BLUE, TERM_BLUE_SLATE},
	{TERM_L_UMBER, TERM_UMBER, TERM_MUD},
	{TERM_PURPLE, TERM_VIOLET, TERM_MAGENTA},
	{TERM_VIOLET, TERM_L_VIOLET, TERM_MAGENTA},
	{TERM_TEAL, TERM_L_TEAL, TERM_L_GREEN},
	{TERM_MUD, TERM_YELLOW, TERM_UMBER},
	{TERM_L_YELLOW, TERM_WHITE, TERM_L_UMBER},
	{TERM_MAGENTA, TERM_L_PINK, TERM_L_RED},
	{TERM_L_TEAL, TERM_L_WHITE, TERM_TEAL},
	{TERM_L_VIOLET, TERM_L_PURPLE, TERM_VIOLET},
	{TERM_L_PINK, TERM_L_RED, TERM_L_WHITE},
	{TERM_MUSTARD, TERM_YELLOW, TERM_UMBER},
	{TERM_BLUE_SLATE, TERM_BLUE, TERM_SLATE},
	{TERM_DEEP_L_BLUE, TERM_L_BLUE, TERM_BLUE},
};

static byte get_flicker(byte a)
{
	switch(flicker % 3)
	{
		case 1: return color_flicker[a][1];
		case 2: return color_flicker[a][2];
	}
	return a;
}

/*
 * This animates monsters and/or items as necessary.
 */
static void do_animation(void)
{
	int i;

	for (i = 1; i < cave_monster_max(cave); i++)
	{
		byte attr;
		monster_type *m_ptr = cave_monster(cave, i);
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		if (!m_ptr || !m_ptr->ml)
			continue;
		else if (rf_has(r_ptr->flags, RF_ATTR_MULTI))
			attr = randint1(BASIC_COLORS - 1);
		else if (rf_has(r_ptr->flags, RF_ATTR_FLICKER))
			attr = get_flicker(r_ptr->x_attr);
		else
			continue;

		m_ptr->attr = attr;
		p_ptr->redraw |= (PR_MAP | PR_MONLIST);
	}
	flicker++;
}


/*
 * This is used when the user is idle to allow for simple animations.
 * Currently the only thing it really does is animate shimmering monsters.
 */
void idle_update(void)
{
	if (!character_dungeon) return;

	if (!OPT(animate_flicker)) return;

	/* Animate and redraw if necessary */
	do_animation();
	redraw_stuff(p_ptr);

	/* Refresh the main screen */
	Term_fresh();
}


/*
 * Interact with the current dungeon level.
 *
 * This function will not exit until the level is completed,
 * the user dies, or the game is terminated.
 */
static void dungeon(struct cave *c)
{
	monster_type *m_ptr;
	int i;



	/* Hack -- enforce illegal panel */
	Term->offset_y = DUNGEON_HGT;
	Term->offset_x = DUNGEON_WID;


	/* Not leaving */
	p_ptr->leaving = FALSE;


	/* Reset the "command" vars */
	p_ptr->command_arg = 0;


	/* Cancel the target */
	target_set_monster(0);

	/* Cancel the health bar */
	health_track(p_ptr, 0);

	/* Disturb */
	disturb(p_ptr, 1, 0);


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

	/* If autosave is pending, do it now. */
	if (p_ptr->autosave)
	{
/* The borg runs so quickly that this is a bad idea. */
#ifndef ALLOW_BORG 
		save_game();
#endif
		p_ptr->autosave = FALSE;
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
	update_stuff(p_ptr);


	/* Fully update the visuals (and monster distances) */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_DISTANCE);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw dungeon */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP);

	/* Redraw "statusy" things */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_MONSTER | PR_MONLIST | PR_ITEMLIST);

	/* Update stuff */
	update_stuff(p_ptr);

	/* Redraw stuff */
	redraw_stuff(p_ptr);


	/* Hack -- Decrease "xtra" depth */
	character_xtra--;


	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Combine / Reorder the pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

	/* Make basic mouse buttons */
	(void) button_add("[ESC]", ESCAPE);
	(void) button_add("[Ret]", '\r');
	(void) button_add("[Spc]", ' ');
	(void) button_add("[Rpt]", 'n');
	(void) button_add("[Std]", ',');

	/* Redraw buttons */
	p_ptr->redraw |= (PR_BUTTONS);

	/* Notice stuff */
	notice_stuff(p_ptr);

	/* Update stuff */
	update_stuff(p_ptr);

	/* Redraw stuff */
	redraw_stuff(p_ptr);

	/* Refresh */
	Term_fresh();

	/* Handle delayed death */
	if (p_ptr->is_dead) return;

	/* Announce (or repeat) the feeling */
	if (p_ptr->depth) do_cmd_feeling();

	/* Give player minimum energy to start a new level, but do not reduce higher value from savefile for level in progress */
	if (p_ptr->energy < INITIAL_DUNGEON_ENERGY)
		p_ptr->energy = INITIAL_DUNGEON_ENERGY;


	/*** Process this dungeon level ***/

	/* Main loop */
	while (TRUE)
	{
		/* Hack -- Compact the monster list occasionally */
		if (cave_monster_count(cave) + 32 > z_info->m_max) compact_monsters(64);

		/* Hack -- Compress the monster list occasionally */
		if (cave_monster_count(cave) + 32 < cave_monster_max(cave)) compact_monsters(0);

		/* Hack -- Compact the object list occasionally */
		if (o_cnt + 32 > z_info->o_max) compact_objects(64);

		/* Hack -- Compress the object list occasionally */
		if (o_cnt + 32 < o_max) compact_objects(0);

		/* Can the player move? */
		while ((p_ptr->energy >= 100) && !p_ptr->leaving)
		{
    		/* Do any necessary animations */
    		do_animation(); 

			/* process monster with even more energy first */
			process_monsters(c, (byte)(p_ptr->energy + 1));

			/* if still alive */
			if (!p_ptr->leaving)
			{
			        /* Mega hack -redraw big graphics - sorry NRM */
			        if ((tile_width > 1) || (tile_height > 1)) 
				        p_ptr->redraw |= (PR_MAP);

				/* Process the player */
				process_player();
			}
		}

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff(p_ptr);

		/* Update stuff */
		if (p_ptr->update) update_stuff(p_ptr);

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff(p_ptr);

		/* Hack -- Highlight the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process all of the monsters */
		process_monsters(c, 100);

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff(p_ptr);

		/* Update stuff */
		if (p_ptr->update) update_stuff(p_ptr);

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff(p_ptr);

		/* Hack -- Highlight the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Process the world */
		process_world(c);

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff(p_ptr);

		/* Update stuff */
		if (p_ptr->update) update_stuff(p_ptr);

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff(p_ptr);

		/* Hack -- Highlight the player */
		move_cursor_relative(p_ptr->py, p_ptr->px);

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/*** Apply energy ***/

		/* Give the player some energy */
		p_ptr->energy += extract_energy[p_ptr->state.speed];

		/* Give energy to all monsters */
		for (i = cave_monster_max(cave) - 1; i >= 1; i--)
		{
			/* Access the monster */
			m_ptr = cave_monster(cave, i);

			/* Ignore "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Give this monster some energy */
			m_ptr->energy += extract_energy[m_ptr->mspeed];
		}

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
	(void)process_pref_file("user.prf", TRUE, TRUE);

	/* Get the "PLAYER.prf" filename */
	(void)strnfmt(buf, sizeof(buf), "%s.prf", op_ptr->base_name, TRUE);

	/* Process the "PLAYER.prf" file */
	(void)process_pref_file(buf, TRUE, TRUE);
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
 * character, then a new game will be started.
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
void play_game(void)
{
	/* Initialize */
	bool new_game = init_angband();

	/*** Do horrible, hacky things, to start the game off ***/

	/* Hack -- Increase "icky" depth */
	character_icky++;

	/* Verify main term */
	if (!term_screen)
		quit("main window does not exist");

	/* Make sure main term is active */
	Term_activate(term_screen);

	/* Verify minimum size */
	if ((Term->hgt < 24) || (Term->wid < 80))
		quit("main window is too small");

	/* Hack -- Turn off the cursor */
	(void)Term_set_cursor(FALSE);


	/*** Try to load the savefile ***/

	p_ptr->is_dead = TRUE;

	if (savefile[0] && file_exists(savefile)) {
		if (!savefile_load(savefile))
			quit("broken savefile");

		if (p_ptr->is_dead && arg_wizard) {
				p_ptr->is_dead = FALSE;
				p_ptr->chp = p_ptr->mhp;
				p_ptr->noscore |= NOSCORE_WIZARD;
		}
	}

	/* No living character loaded */
	if (p_ptr->is_dead)
	{
		/* Make new player */
		new_game = TRUE;

		/* The dungeon is not ready */
		character_dungeon = FALSE;
	}

	/* Hack -- Default base_name */
	if (!op_ptr->base_name[0])
		my_strcpy(op_ptr->base_name, "PLAYER", sizeof(op_ptr->base_name));


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
		seed_flavor = randint0(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = randint0(0x10000000);

		/* Hack -- seed for random artifacts */
		seed_randart = randint0(0x10000000);

		/* Roll up a new character. Quickstart is allowed if ht_birth is set */
		player_birth(p_ptr->ht_birth ? TRUE : FALSE);

		/* Randomize the artifacts if required */
		if (OPT(birth_randarts) &&
				(!OPT(birth_keep_randarts) || !p_ptr->randarts)) {
			do_randart(seed_randart, TRUE);
			p_ptr->randarts = TRUE;
		}
	}

	/* Initialize temporary fields sensibly */
	p_ptr->object_idx = p_ptr->object_kind_idx = NO_OBJECT;
	p_ptr->monster_race_idx = 0;

	/* Normal machine (process player name) */
	if (savefile[0])
		process_player_name(FALSE);

	/* Weird machine (process player name, pick savefile name) */
	else
		process_player_name(TRUE);

	/* Stop the player being quite so dead */
	p_ptr->is_dead = FALSE;

	/* Flash a message */
	prt("Please wait...", 0, 0);

	/* Allow big cursor */
	smlcurs = FALSE;

	/* Flush the message */
	Term_fresh();

	/* Flavor the objects */
	flavor_init();

	/* Reset visuals */
	reset_visuals(TRUE);

	/* Tell the UI we've started. */
	event_signal(EVENT_ENTER_GAME);

	/* Redraw stuff */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP | PR_MONSTER | PR_MESSAGE);
	redraw_stuff(p_ptr);


	/* Process some user pref files */
	process_some_user_pref_files();


	/* React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Generate a dungeon level if needed */
	if (!character_dungeon)
		cave_generate(cave, p_ptr);


	/* Character is now "complete" */
	character_generated = TRUE;


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Start playing */
	p_ptr->playing = TRUE;

	/* Save not required yet. */
	p_ptr->autosave = FALSE;

	/* Hack -- Enforce "delayed death" */
	if (p_ptr->chp < 0) p_ptr->is_dead = TRUE;

	/* Process */
	while (TRUE)
	{
		/* Play ambient sound on change of level. */
		play_ambient_sound();

		/* Process the level */
		dungeon(cave);

		/* Notice stuff */
		if (p_ptr->notice) notice_stuff(p_ptr);

		/* Update stuff */
		if (p_ptr->update) update_stuff(p_ptr);

		/* Redraw stuff */
		if (p_ptr->redraw) redraw_stuff(p_ptr);


		/* Cancel the target */
		target_set_monster(0);

		/* Cancel the health bar */
		health_track(p_ptr, 0);


		/* Forget the view */
		forget_view();


		/* Handle "quit and save" */
		if (!p_ptr->playing && !p_ptr->is_dead) break;


		/* XXX XXX XXX */
		message_flush();

		/* Accidental Death */
		if (p_ptr->playing && p_ptr->is_dead) {
			/* XXX-elly: this does not belong here. Refactor or
			 * remove. Very similar to do_cmd_wiz_cure_all(). */
			if ((p_ptr->wizard || OPT(cheat_live)) && !get_check("Die? ")) {
				/* Mark social class, reset age, if needed */
				if (p_ptr->sc) p_ptr->sc = p_ptr->age = 0;

				/* Increase age */
				p_ptr->age++;

				/* Mark savefile */
				p_ptr->noscore |= NOSCORE_WIZARD;

				/* Message */
				msg("You invoke wizard mode and cheat death.");
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
				(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
				(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
				(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
				(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
				(void)player_clear_timed(p_ptr, TMD_PARALYZED, TRUE);
				(void)player_clear_timed(p_ptr, TMD_IMAGE, TRUE);
				(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
				(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);

				/* Hack -- Prevent starvation */
				player_set_food(p_ptr, PY_FOOD_MAX - 1);

				/* Hack -- cancel recall */
				if (p_ptr->word_recall)
				{
					/* Message */
					msg("A tension leaves the air around you...");
					message_flush();

					/* Hack -- Prevent recall */
					p_ptr->word_recall = 0;
				}

				/* Note cause of death XXX XXX XXX */
				my_strcpy(p_ptr->died_from, "Cheating death", sizeof(p_ptr->died_from));

				/* New depth */
				p_ptr->depth = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;
			}
		}

		/* Handle "death" */
		if (p_ptr->is_dead) break;

		/* Make a new level */
		cave_generate(cave, p_ptr);
	}

	/* Disallow big cursor */
	smlcurs = TRUE;

	/* Tell the UI we're done with the game state */
	event_signal(EVENT_LEAVE_GAME);

	/* Close stuff */
	close_game();
}
