/*
 * File: player/calcs.c
 * Purpose: Player status calculation, signalling ui events based on status
 *          changes.
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
#include "game-event.h"
#include "object/tvalsval.h"



/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
	int i, j, k, levels;
	int num_allowed, num_known;
	int percent_spells;

	const magic_type *s_ptr;

	s16b old_spells;

	cptr p = ((cp_ptr->spell_book == TV_MAGIC_BOOK) ? "spell" : "prayer");


	/* Hack -- must be literate */
	if (!cp_ptr->spell_book) return;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Save the new_spells value */
	old_spells = p_ptr->new_spells;


	/* Determine the number of spells allowed */
	levels = p_ptr->lev - cp_ptr->spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Number of 1/100 spells per level */
	percent_spells = adj_mag_study[p_ptr->state.stat_ind[cp_ptr->spell_stat]];

	/* Extract total allowed spells (rounded up) */
	num_allowed = (((percent_spells * levels) + 50) / 100);

	/* Assume none known */
	num_known = 0;

	/* Count the number of spells we know */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Count known spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			num_known++;
		}
	}

	/* See how many spells we must forget or may learn */
	p_ptr->new_spells = num_allowed - num_known;



	/* Forget spells which are too hard */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Get the spell */
		j = p_ptr->spell_order[i];

		/* Skip non-spells */
		if (j >= 99) continue;

		/* Get the spell */
		s_ptr = &mp_ptr->info[j];

		/* Skip spells we are allowed to know */
		if (s_ptr->slevel <= p_ptr->lev) continue;

		/* Is it known? */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg_format("You have forgotten the %s of %s.", p,
			           get_spell_name(cp_ptr->spell_book, j));

			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}


	/* Forget spells if we know too many spells */
	for (i = PY_MAX_SPELLS - 1; i >= 0; i--)
	{
		/* Stop when possible */
		if (p_ptr->new_spells >= 0) break;

		/* Get the (i+1)th spell learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) continue;

		/* Forget it (if learned) */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			/* Mark as forgotten */
			p_ptr->spell_flags[j] |= PY_SPELL_FORGOTTEN;

			/* No longer known */
			p_ptr->spell_flags[j] &= ~PY_SPELL_LEARNED;

			/* Message */
			msg_format("You have forgotten the %s of %s.", p,
			           get_spell_name(cp_ptr->spell_book, j));

			/* One more can be learned */
			p_ptr->new_spells++;
		}
	}


	/* Check for spells to remember */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		/* None left to remember */
		if (p_ptr->new_spells <= 0) break;

		/* Get the next spell we learned */
		j = p_ptr->spell_order[i];

		/* Skip unknown spells */
		if (j >= 99) break;

		/* Get the spell */
		s_ptr = &mp_ptr->info[j];

		/* Skip spells we cannot remember */
		if (s_ptr->slevel > p_ptr->lev) continue;

		/* First set of spells */
		if (p_ptr->spell_flags[j] & PY_SPELL_FORGOTTEN)
		{
			/* No longer forgotten */
			p_ptr->spell_flags[j] &= ~PY_SPELL_FORGOTTEN;

			/* Known once more */
			p_ptr->spell_flags[j] |= PY_SPELL_LEARNED;

			/* Message */
			msg_format("You have remembered the %s of %s.",
			           p, get_spell_name(cp_ptr->spell_book, j));

			/* One less can be learned */
			p_ptr->new_spells--;
		}
	}


	/* Assume no spells available */
	k = 0;

	/* Count spells that can be learned */
	for (j = 0; j < PY_MAX_SPELLS; j++)
	{
		/* Get the spell */
		s_ptr = &mp_ptr->info[j];

		/* Skip spells we cannot remember or don't exist */
		if (s_ptr->slevel > p_ptr->lev || s_ptr->slevel == 0) continue;

		/* Skip spells we already know */
		if (p_ptr->spell_flags[j] & PY_SPELL_LEARNED)
		{
			continue;
		}

		/* Count it */
		k++;
	}

	/* Cannot learn more spells than exist */
	if (p_ptr->new_spells > k) p_ptr->new_spells = k;

	/* Spell count changed */
	if (old_spells != p_ptr->new_spells)
	{
		/* Message if needed */
		if (p_ptr->new_spells)
		{
			/* Message */
			msg_format("You can learn %d more %s%s.",
			           p_ptr->new_spells, p,
			           (p_ptr->new_spells != 1) ? "s" : "");
		}

		/* Redraw Study Status */
		p_ptr->redraw |= (PR_STUDY | PR_OBJECT);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
	int msp, levels, cur_wgt, max_wgt;

	object_type *o_ptr;

	bool old_cumber_glove = p_ptr->cumber_glove;
	bool old_cumber_armor = p_ptr->cumber_armor;

	/* Hack -- Must be literate */
	if (!cp_ptr->spell_book) return;


	/* Extract "effective" player level */
	levels = (p_ptr->lev - cp_ptr->spell_first) + 1;
	if (levels > 0)
	{
		msp = 1;
		msp += adj_mag_mana[p_ptr->state.stat_ind[cp_ptr->spell_stat]] * levels / 100;
	}
	else
	{
		levels = 0;
		msp = 0;
	}

	/* Process gloves for those disturbed by them */
	if (cp_ptr->flags & CF_CUMBER_GLOVE)
	{
		u32b f[OBJ_FLAG_N];

		/* Assume player is not encumbered by gloves */
		p_ptr->cumber_glove = FALSE;

		/* Get the gloves */
		o_ptr = &inventory[INVEN_HANDS];

		/* Examine the gloves */
		object_flags(o_ptr, f);

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->k_idx &&
		    !(f[2] & TR2_FREE_ACT) &&
		    !((f[0] & TR0_DEX) && (o_ptr->pval > 0)) &&
		    !(o_ptr->sval == SV_SET_OF_ALCHEMISTS_GLOVES))
		{
			/* Encumbered */
			p_ptr->cumber_glove = TRUE;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}

		/* XXX Eddie this will have to change with alchemist's gloves */
		if (!(f[0] & TR0_DEX))
		{
			/* If no dex bonus, know whether gloves provide FA */
			object_notice_flags(o_ptr, 2, TR2_FREE_ACT);
		}
	}


	/* Assume player not encumbered by armor */
	p_ptr->cumber_armor = FALSE;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += inventory[INVEN_BODY].weight;
	cur_wgt += inventory[INVEN_HEAD].weight;
	cur_wgt += inventory[INVEN_ARM].weight;
	cur_wgt += inventory[INVEN_OUTER].weight;
	cur_wgt += inventory[INVEN_HANDS].weight;
	cur_wgt += inventory[INVEN_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = cp_ptr->spell_weight;

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		p_ptr->cumber_armor = TRUE;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}


	/* Mana can never be negative */
	if (msp < 0) msp = 0;


	/* Maximum mana has changed */
	if (p_ptr->msp != msp)
	{
		/* Save new limit */
		p_ptr->msp = msp;

		/* Enforce new limit */
		if (p_ptr->csp >= msp)
		{
			p_ptr->csp = msp;
			p_ptr->csp_frac = 0;
		}

		/* Display mana later */
		p_ptr->redraw |= (PR_MANA);
	}


	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "glove state" changes */
	if (old_cumber_glove != p_ptr->cumber_glove)
	{
		/* Message */
		if (p_ptr->cumber_glove)
		{
			msg_print("Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msg_print("Your hands feel more suitable for spellcasting.");
		}
	}


	/* Take note when "armor state" changes */
	if (old_cumber_armor != p_ptr->cumber_armor)
	{
		/* Message */
		if (p_ptr->cumber_armor)
		{
			msg_print("The weight of your armor encumbers your movement.");
		}
		else
		{
			msg_print("You feel able to move more freely.");
		}
	}
}


/*
 * Calculate the players (maximal) hit points
 *
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints(void)
{
	long bonus;
	int mhp;

	/* Get "1/100th hitpoint bonus per level" value */
	bonus = adj_con_mhp[p_ptr->state.stat_ind[A_CON]];

	/* Calculate hitpoints */
	mhp = p_ptr->player_hp[p_ptr->lev-1] + (bonus * p_ptr->lev / 100);

	/* Always have at least one hitpoint per level */
	if (mhp < p_ptr->lev + 1) mhp = p_ptr->lev + 1;

	/* New maximum hitpoints */
	if (p_ptr->mhp != mhp)
	{
		/* Save new limit */
		p_ptr->mhp = mhp;

		/* Enforce new limit */
		if (p_ptr->chp >= mhp)
		{
			p_ptr->chp = mhp;
			p_ptr->chp_frac = 0;
		}

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_HP);
	}
}


/*
 * Calculate and set the current light radius.
 *
 * The brightest wielded object counts as the light source; radii do not add
 * up anymore.
 *
 * Note that a cursed light source no longer emits light.
 */
static void calc_torch(void)
{
	int i;

	s16b old_lite = p_ptr->cur_lite;
	bool burn_light = TRUE;

	s16b new_lite = 0;
	int extra_lite = 0;



	/* Ascertain lightness if in the town */
	if (!p_ptr->depth && ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2)))
		burn_light = FALSE;


	/* Examine all wielded objects, use the brightest */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		u32b f[OBJ_FLAG_N];

		int amt = 0;
		object_type *o_ptr = &inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
		object_flags(o_ptr, f);

		/* Cursed objects emit no light */
		if (f[2] & TR2_LIGHT_CURSE)
			amt = 0;

		/* Examine actual lites */
		else if (o_ptr->tval == TV_LITE)
		{
			int flag_inc = (f[2] & TR2_LITE) ? 1 : 0;

			/* Artifact Lites provide permanent bright light */
			if (artifact_p(o_ptr))
				amt = 3 + flag_inc;

			/* Non-artifact lights and those without fuel provide no light */
			else if (!burn_light || o_ptr->timeout == 0)
				amt = 0;

			/* All lit lights provide at least radius 2 light */
			else
			{
				amt = 2 + flag_inc;

				/* Torches below half fuel provide less light */
				if (o_ptr->sval == SV_LITE_TORCH && o_ptr->timeout < (FUEL_TORCH / 4))
				    amt--;
			}
		}

		else
		{
			/* LITE flag on an non-cursed non-lights always increases radius */
			if (f[2] & TR2_LITE) extra_lite++;
		}

		/* Alter p_ptr->cur_lite if reasonable */
		if (new_lite < amt)
		    new_lite = amt;
	}

	/* Add bonus from LITE flags */
	new_lite += extra_lite;

	/* Limit light */
	new_lite = MIN(new_lite, 5);
	new_lite = MAX(new_lite, 0);

	/* Notice changes in the "lite radius" */
	if (old_lite != new_lite)
	{
		/* Update the visuals */
		p_ptr->cur_lite = new_lite;
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/*
 * Calculate the blows a player would get, in current condition, wielding "o_ptr".
 */
int calc_blows(object_type *o_ptr, player_state *state)
{
	int blows;
	int str_index, dex_index;
	int div;

	/* Enforce a minimum "weight" (tenth pounds) */
	div = ((o_ptr->weight < cp_ptr->min_weight) ? cp_ptr->min_weight : o_ptr->weight);

	/* Get the strength vs weight */
	str_index = adj_str_blow[state->stat_ind[A_STR]] *
			cp_ptr->att_multiply / div;

	/* Maximal value */
	if (str_index > 11) str_index = 11;

	/* Index by dexterity */
	dex_index = MIN(adj_dex_blow[state->stat_ind[A_DEX]], 11);

	/* Use the blows table */
	blows = MIN(blows_table[str_index][dex_index], cp_ptr->max_attacks);

	/* Require at least one blow */
	return MAX(blows, 1);
}


/*
 * Computes current weight limit.
 */
static int weight_limit(player_state *state)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[state->stat_ind[A_STR]] * 100;

	/* Return the result */
	return (i);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * If id_only is true, calc_bonuses() will only use the known
 * information of objects; thus it returns what the player _knows_
 * the character state to be.
 */
void calc_bonuses(object_type inventory[], player_state *state, bool id_only)
{
	int i, j, hold;

	int extra_blows = 0;
	int extra_shots = 0;
	int extra_might = 0;

	object_type *o_ptr;

	u32b f[OBJ_FLAG_N];
	u32b collect_f[OBJ_FLAG_N];


	/*** Reset ***/

	memset(state, 0, sizeof *state);

	/* Set various defaults */
	state->speed = 110;
	state->num_blow = 1;


	/*** Extract race/class info ***/

	/* Base infravision (purely racial) */
	state->see_infra = rp_ptr->infra;

	/* Base skills */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] = rp_ptr->r_skills[i] + cp_ptr->c_skills[i];


	/*** Analyze player ***/

	/* Extract the player flags */
	player_flags(collect_f);


	/*** Analyze equipment ***/

	/* Scan the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Extract the item flags */
		if (id_only)
			object_flags_known(o_ptr, f);
		else
			object_flags(o_ptr, f);

		collect_f[0] |= f[0];
		collect_f[1] |= f[1];
		collect_f[2] |= f[2];

		/* Affect stats */
		if (f[0] & TR0_STR) state->stat_add[A_STR] += o_ptr->pval;
		if (f[0] & TR0_INT) state->stat_add[A_INT] += o_ptr->pval;
		if (f[0] & TR0_WIS) state->stat_add[A_WIS] += o_ptr->pval;
		if (f[0] & TR0_DEX) state->stat_add[A_DEX] += o_ptr->pval;
		if (f[0] & TR0_CON) state->stat_add[A_CON] += o_ptr->pval;
		if (f[0] & TR0_CHR) state->stat_add[A_CHR] += o_ptr->pval;

		/* Affect stealth */
		if (f[0] & TR0_STEALTH) state->skills[SKILL_STEALTH] += o_ptr->pval;

		/* Affect searching ability (factor of five) */
		if (f[0] & TR0_SEARCH) state->skills[SKILL_SEARCH] += (o_ptr->pval * 5);

		/* Affect searching frequency (factor of five) */
		if (f[0] & TR0_SEARCH) state->skills[SKILL_SEARCH_FREQUENCY] += (o_ptr->pval * 5);

		/* Affect infravision */
		if (f[0] & TR0_INFRA) state->see_infra += o_ptr->pval;

		/* Affect digging (factor of 20) */
		if (f[0] & TR0_TUNNEL) state->skills[SKILL_DIGGING] += (o_ptr->pval * 20);

		/* Affect speed */
		if (f[0] & TR0_SPEED) state->speed += o_ptr->pval;

		/* Affect blows */
		if (f[0] & TR0_BLOWS) extra_blows += o_ptr->pval;

		/* Affect shots */
		if (f[0] & TR0_SHOTS) extra_shots += o_ptr->pval;

		/* Affect Might */
		if (f[0] & TR0_MIGHT) extra_might += o_ptr->pval;

		/* Modify the base armor class */
		state->ac += o_ptr->ac;

		/* The base armor class is always known */
		state->dis_ac += o_ptr->ac;

		/* Apply the bonuses to armor class */
		if (!id_only || object_is_known(o_ptr))
			state->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		if (object_defence_plusses_are_visible(o_ptr))
			state->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (i == INVEN_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (i == INVEN_BOW) continue;

		/* Apply the bonuses to hit/damage */
		if (!id_only || object_is_known(o_ptr))
		{
			state->to_h += o_ptr->to_h;
			state->to_d += o_ptr->to_d;
		}

		/* Apply the mental bonuses tp hit/damage, if known */
		if (object_attack_plusses_are_visible(o_ptr))
		{
			state->dis_to_h += o_ptr->to_h;
			state->dis_to_d += o_ptr->to_d;
		}
	}


	/*** Update all flags ***/

	/* Good flags */
	if (collect_f[2] & TR2_SLOW_DIGEST) state->slow_digest = TRUE;
	if (collect_f[2] & TR2_FEATHER) state->ffall = TRUE;
	if (collect_f[2] & TR2_REGEN) state->regenerate = TRUE;
	if (collect_f[2] & TR2_TELEPATHY) state->telepathy = TRUE;
	if (collect_f[2] & TR2_SEE_INVIS) state->see_inv = TRUE;
	if (collect_f[2] & TR2_FREE_ACT) state->free_act = TRUE;
	if (collect_f[2] & TR2_HOLD_LIFE) state->hold_life = TRUE;

	/* Weird flags */
	if (collect_f[2] & TR2_BLESSED) state->bless_blade = TRUE;

	/* Bad flags */
	if (collect_f[2] & TR2_IMPACT) state->impact = TRUE;
	if (collect_f[2] & TR2_AGGRAVATE) state->aggravate = TRUE;
	if (collect_f[2] & TR2_TELEPORT) state->teleport = TRUE;
	if (collect_f[2] & TR2_DRAIN_EXP) state->exp_drain = TRUE;
	if (collect_f[2] & TR2_IMPAIR_HP) state->impair_hp = TRUE;
	if (collect_f[2] & TR2_IMPAIR_MANA) state->impair_mana = TRUE;
	if (collect_f[2] & TR2_AFRAID) state->afraid = TRUE;

	/* Vulnerability flags */
	if (collect_f[1] & TR1_VULN_FIRE) state->vuln_fire = TRUE;
	if (collect_f[1] & TR1_VULN_ACID) state->vuln_acid = TRUE;
	if (collect_f[1] & TR1_VULN_COLD) state->vuln_cold = TRUE;
	if (collect_f[1] & TR1_VULN_ELEC) state->vuln_elec = TRUE;

	/* Immunity flags */
	if (collect_f[1] & TR1_IM_FIRE) state->immune_fire = TRUE;
	if (collect_f[1] & TR1_IM_ACID) state->immune_acid = TRUE;
	if (collect_f[1] & TR1_IM_COLD) state->immune_cold = TRUE;
	if (collect_f[1] & TR1_IM_ELEC) state->immune_elec = TRUE;

	/* Resistance flags */
	if (collect_f[1] & TR1_RES_ACID) state->resist_acid = TRUE;
	if (collect_f[1] & TR1_RES_ELEC) state->resist_elec = TRUE;
	if (collect_f[1] & TR1_RES_FIRE) state->resist_fire = TRUE;
	if (collect_f[1] & TR1_RES_COLD) state->resist_cold = TRUE;
	if (collect_f[1] & TR1_RES_POIS) state->resist_pois = TRUE;
	if (collect_f[1] & TR1_RES_FEAR) state->resist_fear = TRUE;
	if (collect_f[1] & TR1_RES_LITE) state->resist_lite = TRUE;
	if (collect_f[1] & TR1_RES_DARK) state->resist_dark = TRUE;
	if (collect_f[1] & TR1_RES_BLIND) state->resist_blind = TRUE;
	if (collect_f[1] & TR1_RES_CONFU) state->resist_confu = TRUE;
	if (collect_f[1] & TR1_RES_SOUND) state->resist_sound = TRUE;
	if (collect_f[1] & TR1_RES_SHARD) state->resist_shard = TRUE;
	if (collect_f[1] & TR1_RES_NEXUS) state->resist_nexus = TRUE;
	if (collect_f[1] & TR1_RES_NETHR) state->resist_nethr = TRUE;
	if (collect_f[1] & TR1_RES_CHAOS) state->resist_chaos = TRUE;
	if (collect_f[1] & TR1_RES_DISEN) state->resist_disen = TRUE;

	/* Sustain flags */
	if (collect_f[1] & TR1_SUST_STR) state->sustain_str = TRUE;
	if (collect_f[1] & TR1_SUST_INT) state->sustain_int = TRUE;
	if (collect_f[1] & TR1_SUST_WIS) state->sustain_wis = TRUE;
	if (collect_f[1] & TR1_SUST_DEX) state->sustain_dex = TRUE;
	if (collect_f[1] & TR1_SUST_CON) state->sustain_con = TRUE;
	if (collect_f[1] & TR1_SUST_CHR) state->sustain_chr = TRUE;



	/*** Handle stats ***/

	/* Calculate stats */
	for (i = 0; i < A_MAX; i++)
	{
		int add, top, use, ind;

		/* Extract modifier */
		add = state->stat_add[i];

		/* Maximize mode */
		if (OPT(adult_maximize))
		{
			/* Modify the stats for race/class */
			add += (rp_ptr->r_adj[i] + cp_ptr->c_adj[i]);
		}

		/* Extract the new "stat_top" value for the stat */
		top = modify_stat_value(p_ptr->stat_max[i], add);

		/* Save the new value */
		state->stat_top[i] = top;

		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(p_ptr->stat_cur[i], add);

		/* Save the new value */
		state->stat_use[i] = use;

		/* Values: 3, 4, ..., 17 */
		if (use <= 18) ind = (use - 3);

		/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
		else if (use <= 18+219) ind = (15 + (use - 18) / 10);

		/* Range: 18/220+ */
		else ind = (37);

		/* Save the new index */
		state->stat_ind[i] = ind;
	}


	/*** Temporary flags ***/

	/* Apply temporary "stun" */
	if (p_ptr->timed[TMD_STUN] > 50)
	{
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_d -= 20;
		state->dis_to_d -= 20;
	}
	else if (p_ptr->timed[TMD_STUN])
	{
		state->to_h -= 5;
		state->dis_to_h -= 5;
		state->to_d -= 5;
		state->dis_to_d -= 5;
	}

	/* Invulnerability */
	if (p_ptr->timed[TMD_INVULN])
	{
		state->to_a += 100;
		state->dis_to_a += 100;
	}

	/* Temporary blessing */
	if (p_ptr->timed[TMD_BLESSED])
	{
		state->to_a += 5;
		state->dis_to_a += 5;
		state->to_h += 10;
		state->dis_to_h += 10;
	}

	/* Temporary shield */
	if (p_ptr->timed[TMD_SHIELD])
	{
		state->to_a += 50;
		state->dis_to_a += 50;
	}

	/* Temporary stoneskin */
	if (p_ptr->timed[TMD_STONESKIN])
	{
		state->to_a += 40;
		state->dis_to_a += 40;
		state->speed -= 5;
	}

	/* Temporary "Hero" */
	if (p_ptr->timed[TMD_HERO])
	{
		state->to_h += 12;
		state->dis_to_h += 12;
		state->resist_fear = TRUE;
	}

	/* Temporary "Berserk" */
	if (p_ptr->timed[TMD_SHERO])
	{
		state->to_h += 24;
		state->dis_to_h += 24;
		state->to_a -= 10;
		state->dis_to_a -= 10;
		state->resist_fear = TRUE;
	}

	/* Temporary "fast" */
	if (p_ptr->timed[TMD_FAST] || p_ptr->timed[TMD_SPRINT])
		state->speed += 10;

	/* Temporary "slow" */
	if (p_ptr->timed[TMD_SLOW])
		state->speed -= 10;


	/* Temporary see invisible */
	if (p_ptr->timed[TMD_SINVIS])
		state->see_inv = TRUE;

	/* Temporary infravision boost */
	if (p_ptr->timed[TMD_SINFRA])
		state->see_infra += 5;

	/* Temporary telepathy */
	if (p_ptr->timed[TMD_TELEPATHY])
		state->telepathy = TRUE;

	/* Temporary resist confusion */
	if (p_ptr->timed[TMD_OPP_CONF])
		state->resist_confu = TRUE;

	/* Fear */
	if (p_ptr->timed[TMD_AFRAID] || p_ptr->timed[TMD_TERROR])
		state->afraid = TRUE;

	if (p_ptr->timed[TMD_TERROR])
		state->speed += 5;


	/* Fear can come from item flags too */
	if (state->afraid)
	{
		state->to_h -= 20;
		state->dis_to_h -= 20;
		state->to_a += 8;
		state->dis_to_a += 8;
	}


	/*** Analyze weight ***/

	/* Extract the current weight (in tenth pounds) */
	j = p_ptr->total_weight;

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit(state);

	/* Apply "encumbrance" from weight */
	if (j > i / 2) state->speed -= ((j - (i / 2)) / (i / 10));

	/* Bloating slows the player down (a little) */
	if (p_ptr->food >= PY_FOOD_MAX) state->speed -= 10;

	/* Searching slows the player down */
	if (p_ptr->searching) state->speed -= 10;

	/* Sanity check on extreme speeds */
	if (state->speed < 0) state->speed = 0;
	if (state->speed > 199) state->speed = 199;

	/*** Apply modifier bonuses ***/

	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	state->to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	state->dis_to_a += ((int)(adj_dex_ta[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_d += ((int)(adj_str_td[state->stat_ind[A_STR]]) - 128);
	state->dis_to_h += ((int)(adj_dex_th[state->stat_ind[A_DEX]]) - 128);
	state->dis_to_h += ((int)(adj_str_th[state->stat_ind[A_STR]]) - 128);


	/*** Modify skills ***/

	/* Affect Skill -- stealth (bonus one) */
	state->skills[SKILL_STEALTH] += 1;

	/* Affect Skill -- disarming (DEX and INT) */
	state->skills[SKILL_DISARM] += adj_dex_dis[state->stat_ind[A_DEX]];
	state->skills[SKILL_DISARM] += adj_int_dis[state->stat_ind[A_INT]];

	/* Affect Skill -- magic devices (INT) */
	state->skills[SKILL_DEVICE] += adj_int_dev[state->stat_ind[A_INT]];

	/* Affect Skill -- saving throw (WIS) */
	state->skills[SKILL_SAVE] += adj_wis_sav[state->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	state->skills[SKILL_DIGGING] += adj_str_dig[state->stat_ind[A_STR]];

	/* Affect Skills (Level, by Class) */
	for (i = 0; i < SKILL_MAX; i++)
		state->skills[i] += (cp_ptr->x_skills[i] * p_ptr->lev / 10);

	/* Limit Skill -- digging from 1 up */
	if (state->skills[SKILL_DIGGING] < 1) state->skills[SKILL_DIGGING] = 1;

	/* Limit Skill -- stealth from 0 to 30 */
	if (state->skills[SKILL_STEALTH] > 30) state->skills[SKILL_STEALTH] = 30;
	if (state->skills[SKILL_STEALTH] < 0) state->skills[SKILL_STEALTH] = 0;

	/* Apply Skill -- Extract noise from stealth */
	state->noise = (1L << (30 - state->skills[SKILL_STEALTH]));

	/* Obtain the "hold" value */
	hold = adj_str_hold[state->stat_ind[A_STR]];


	/*** Analyze current bow ***/

	/* Examine the "current bow" */
	o_ptr = &inventory[INVEN_BOW];

	/* Assume not heavy */
	state->heavy_shoot = FALSE;

	/* It is hard to carholdry a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		state->heavy_shoot = TRUE;
	}

	/* Analyze launcher */
	if (o_ptr->k_idx)
	{
		/* Get to shoot */
		state->num_fire = 1;

		/* Analyze the launcher */
		switch (o_ptr->sval)
		{
			/* Sling and ammo */
			case SV_SLING:
			{
				state->ammo_tval = TV_SHOT;
				state->ammo_mult = 2;
				break;
			}

			/* Short Bow and Arrow */
			case SV_SHORT_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 2;
				break;
			}

			/* Long Bow and Arrow */
			case SV_LONG_BOW:
			{
				state->ammo_tval = TV_ARROW;
				state->ammo_mult = 3;
				break;
			}

			/* Light Crossbow and Bolt */
			case SV_LIGHT_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 3;
				break;
			}

			/* Heavy Crossbow and Bolt */
			case SV_HEAVY_XBOW:
			{
				state->ammo_tval = TV_BOLT;
				state->ammo_mult = 4;
				break;
			}
		}

		/* Apply special flags */
		if (o_ptr->k_idx && !state->heavy_shoot)
		{
			/* Extra shots */
			state->num_fire += extra_shots;

			/* Extra might */
			state->ammo_mult += extra_might;

			/* Hack -- Rangers love Bows */
			if ((cp_ptr->flags & CF_EXTRA_SHOT) &&
			    (state->ammo_tval == TV_ARROW))
			{
				/* Extra shot at level 20 */
				if (p_ptr->lev >= 20) state->num_fire++;

				/* Extra shot at level 40 */
				if (p_ptr->lev >= 40) state->num_fire++;
			}
		}

		/* Require at least one shot */
		if (state->num_fire < 1) state->num_fire = 1;
	}


	/*** Analyze weapon ***/

	/* Examine the "current weapon" */
	o_ptr = &inventory[INVEN_WIELD];

	/* Assume not heavy */
	state->heavy_wield = FALSE;

	/* It is hard to hold a heavy weapon */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy weapon */
		state->to_h += 2 * (hold - o_ptr->weight / 10);
		state->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy weapon */
		state->heavy_wield = TRUE;
	}

	/* Normal weapons */
	if (o_ptr->k_idx && !state->heavy_wield)
	{
		/* Calculate number of blows */
		state->num_blow = calc_blows(o_ptr, state) + extra_blows;

		/* Boost digging skill by weapon weight */
		state->skills[SKILL_DIGGING] += (o_ptr->weight / 10);
	}


	/* Assume okay */
	state->icky_wield = FALSE;

	/* Priest weapon penalty for non-blessed edged weapons */
	if ((cp_ptr->flags & CF_BLESS_WEAPON) && (!state->bless_blade) &&
	    ((o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM)))
	{
		/* Reduce the real bonuses */
		state->to_h -= 2;
		state->to_d -= 2;

		/* Reduce the mental bonuses */
		state->dis_to_h -= 2;
		state->dis_to_d -= 2;

		/* Icky weapon */
		state->icky_wield = TRUE;
	}

	return;
}

/*
 * Calculate bonuses, and print various things on changes.
 */
static void update_bonuses(void)
{
	int i;

	player_state *state = &p_ptr->state;
	player_state old = p_ptr->state;


	/*** Calculate bonuses ***/

	calc_bonuses(inventory, &p_ptr->state, FALSE);


	/*** Notice changes ***/

	/* Analyze stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Notice changes */
		if (state->stat_top[i] != old.stat_top[i])
		{
			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_use[i] != old.stat_use[i])
		{
			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);
		}

		/* Notice changes */
		if (state->stat_ind[i] != old.stat_ind[i])
		{
			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				p_ptr->update |= (PU_HP);
			}

			/* Change in INT may affect Mana/Spells */
			else if (i == A_INT)
			{
				if (cp_ptr->spell_stat == A_INT)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Change in WIS may affect Mana/Spells */
			else if (i == A_WIS)
			{
				if (cp_ptr->spell_stat == A_WIS)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}
		}
	}


	/* Hack -- Telepathy Change */
	if (state->telepathy != old.telepathy)
	{
		/* Update monster visibility */
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Hack -- See Invis Change */
	if (state->see_inv != old.see_inv)
	{
		/* Update monster visibility */
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Redraw speed (if needed) */
	if (state->speed != old.speed)
	{
		/* Redraw speed */
		p_ptr->redraw |= (PR_SPEED);
	}

	/* Redraw armor (if needed) */
	if ((state->dis_ac != old.dis_ac) || (state->dis_to_a != old.dis_to_a))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_ARMOR);
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "heavy bow" changes */
	if (old.heavy_shoot != state->heavy_shoot)
	{
		/* Message */
		if (state->heavy_shoot)
		{
			msg_print("You have trouble wielding such a heavy bow.");
		}
		else if (inventory[INVEN_BOW].k_idx)
		{
			msg_print("You have no trouble wielding your bow.");
		}
		else
		{
			msg_print("You feel relieved to put down your heavy bow.");
		}
	}

	/* Take note when "heavy weapon" changes */
	if (old.heavy_wield != state->heavy_wield)
	{
		/* Message */
		if (state->heavy_wield)
		{
			msg_print("You have trouble wielding such a heavy weapon.");
		}
		else if (inventory[INVEN_WIELD].k_idx)
		{
			msg_print("You have no trouble wielding your weapon.");
		}
		else
		{
			msg_print("You feel relieved to put down your heavy weapon.");	
		}
	}

	/* Take note when "illegal weapon" changes */
	if (old.icky_wield != state->icky_wield)
	{
		/* Message */
		if (state->icky_wield)
		{
			msg_print("You do not feel comfortable with your weapon.");
		}
		else if (inventory[INVEN_WIELD].k_idx)
		{
			msg_print("You feel comfortable with your weapon.");
		}
		else
		{
			msg_print("You feel more comfortable after removing your weapon.");
		}
	}
}




/*** Generic "deal with" functions ***/

/*
 * Handle "p_ptr->notice"
 */
void notice_stuff(void)
{
	/* Notice stuff */
	if (!p_ptr->notice) return;


	/* Deal with autoinscribe stuff */
	if (p_ptr->notice & PN_AUTOINSCRIBE)
	{
		p_ptr->notice &= ~(PN_AUTOINSCRIBE);
		autoinscribe_pack();
		autoinscribe_ground();
	}

	/* Deal with squelch stuff */
	if (p_ptr->notice & PN_SQUELCH)
	{
		p_ptr->notice &= ~(PN_SQUELCH);
		if (OPT(hide_squelchable)) squelch_drop();
	}

	/* Combine the pack */
	if (p_ptr->notice & PN_COMBINE)
	{
		p_ptr->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Reorder the pack */
	if (p_ptr->notice & PN_REORDER)
	{
		p_ptr->notice &= ~(PN_REORDER);
		reorder_pack();
	}
}

/*
 * Handle "p_ptr->update"
 */
void update_stuff(void)
{
	/* Update stuff */
	if (!p_ptr->update) return;


	if (p_ptr->update & (PU_BONUS))
	{
		p_ptr->update &= ~(PU_BONUS);
		update_bonuses();
	}

	if (p_ptr->update & (PU_TORCH))
	{
		p_ptr->update &= ~(PU_TORCH);
		calc_torch();
	}

	if (p_ptr->update & (PU_HP))
	{
		p_ptr->update &= ~(PU_HP);
		calc_hitpoints();
	}

	if (p_ptr->update & (PU_MANA))
	{
		p_ptr->update &= ~(PU_MANA);
		calc_mana();
	}

	if (p_ptr->update & (PU_SPELLS))
	{
		p_ptr->update &= ~(PU_SPELLS);
		calc_spells();
	}


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	if (p_ptr->update & (PU_FORGET_VIEW))
	{
		p_ptr->update &= ~(PU_FORGET_VIEW);
		forget_view();
	}

	if (p_ptr->update & (PU_UPDATE_VIEW))
	{
		p_ptr->update &= ~(PU_UPDATE_VIEW);
		update_view();
	}


	if (p_ptr->update & (PU_FORGET_FLOW))
	{
		p_ptr->update &= ~(PU_FORGET_FLOW);
		forget_flow();
	}

	if (p_ptr->update & (PU_UPDATE_FLOW))
	{
		p_ptr->update &= ~(PU_UPDATE_FLOW);
		update_flow();
	}


	if (p_ptr->update & (PU_DISTANCE))
	{
		p_ptr->update &= ~(PU_DISTANCE);
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(TRUE);
	}

	if (p_ptr->update & (PU_MONSTERS))
	{
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(FALSE);
	}


	if (p_ptr->update & (PU_PANEL))
	{
		p_ptr->update &= ~(PU_PANEL);
		event_signal(EVENT_PLAYERMOVED);
	}
}



struct flag_event_trigger
{
	u32b flag;
	game_event_type event;
};



/*
 * Events triggered by the various flags.
 */
static const struct flag_event_trigger redraw_events[] =
{
	{ PR_MISC,    EVENT_RACE_CLASS },
	{ PR_TITLE,   EVENT_PLAYERTITLE },
	{ PR_LEV,     EVENT_PLAYERLEVEL },
	{ PR_EXP,     EVENT_EXPERIENCE },
	{ PR_STATS,   EVENT_STATS },
	{ PR_ARMOR,   EVENT_AC },
	{ PR_HP,      EVENT_HP },
	{ PR_MANA,    EVENT_MANA },
	{ PR_GOLD,    EVENT_GOLD },
	{ PR_HEALTH,  EVENT_MONSTERHEALTH },
	{ PR_DEPTH,   EVENT_DUNGEONLEVEL },
	{ PR_SPEED,   EVENT_PLAYERSPEED },
	{ PR_STATE,   EVENT_STATE },
	{ PR_STATUS,  EVENT_STATUS },
	{ PR_STUDY,   EVENT_STUDYSTATUS },
	{ PR_DTRAP,   EVENT_DETECTIONSTATUS },
	{ PR_BUTTONS, EVENT_MOUSEBUTTONS },

	{ PR_INVEN,   EVENT_INVENTORY },
	{ PR_EQUIP,   EVENT_EQUIPMENT },
	{ PR_MONLIST, EVENT_MONSTERLIST },
	{ PR_ITEMLIST, EVENT_ITEMLIST },
	{ PR_MONSTER, EVENT_MONSTERTARGET },
	{ PR_OBJECT, EVENT_OBJECTTARGET },
	{ PR_MESSAGE, EVENT_MESSAGE },
};

/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(void)
{
	size_t i;

	/* Redraw stuff */
	if (!p_ptr->redraw) return;

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;

	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;

	/* For each listed flag, send the appropriate signal to the UI */
	for (i = 0; i < N_ELEMENTS(redraw_events); i++)
	{
		const struct flag_event_trigger *hnd = &redraw_events[i];

		if (p_ptr->redraw & hnd->flag)
			event_signal(hnd->event);
	}

	/* Then the ones that require parameters to be supplied. */
	if (p_ptr->redraw & PR_MAP)
	{
		/* Mark the whole map to be redrawn */
		event_signal_point(EVENT_MAP, -1, -1);
	}

	p_ptr->redraw = 0;

	/*
	 * Do any plotting, etc. delayed from earlier - this set of updates
	 * is over.
	 */
	event_signal(EVENT_END);
}


/*
 * Handle "p_ptr->update" and "p_ptr->redraw"
 */
void handle_stuff(void)
{
	/* Update stuff */
	if (p_ptr->update) update_stuff();

	/* Redraw stuff */
	if (p_ptr->redraw) redraw_stuff();
}

