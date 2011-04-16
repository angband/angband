/*
 * File: melee1.c
 * Purpose: Monster attacking code
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
#include "attack.h"
#include "cave.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "spells.h"

/*
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (randint0(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
	{
		while (randint0(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}





/*
 * Determine if a monster attack against the player succeeds.
 */
bool check_hit(int power, int level)
{
	int chance, ac;

	/* Calculate the "attack quality" */
	chance = (power + (level * 3));

	/* Total armor */
	ac = p_ptr->state.ac + p_ptr->state.to_a;

	/* if the monster checks vs ac, the player learns ac bonuses */
	/* XXX Eddie should you only learn +ac on miss, -ac on hit?  who knows */
	object_notice_on_defend();

	/* Check if the player was hit */
	return test_hit(chance, ac, TRUE);
}


#define MAX_DESC_INSULT 8


/*
 * Hack -- possible "insult" messages
 */
static const char *desc_insult[MAX_DESC_INSULT] =
{
	"insults you!",
	"insults your mother!",
	"gives you the finger!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obscene gestures!",
	"moons you!!!"
};


#define MAX_DESC_MOAN 8


/*
 * Hack -- possible "insult" messages
 */
static const char *desc_moan[MAX_DESC_MOAN] =
{
	"wants his mushrooms back.",
	"tells you to get off his land.",
	"looks for his dogs. ",
	"says 'Did you kill my Fang?' ",
	"asks 'Do you want to buy any mushrooms?' ",
	"seems sad about something.",
	"asks if you have seen his dogs.",
	"mumbles something about mushrooms."
};


/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int ap_cnt;

	int i, k, tmp, ac, rlev;
	int do_cut, do_stun;

	s32b gold;

	object_type *o_ptr;

	char o_name[80];

	char m_name[80];

	char ddesc[80];

	bool blinked;

	int sound_msg;


	/* Not allowed to attack */
	if (rf_has(r_ptr->flags, RF_NEVER_BLOW)) return (FALSE);

	/* Become aware of monster
	if (m_ptr->unaware)
	{
		m_ptr->unaware = FALSE;
		update_mon(m_idx, FALSE);
	} */

	/* Total armor */
	ac = p_ptr->state.ac + p_ptr->state.to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Assume no blink */
	blinked = FALSE;

	/* Scan through all blows */
	for (ap_cnt = 0; ap_cnt < MONSTER_BLOW_MAX; ap_cnt++)
	{
		bool visible = FALSE;
		bool obvious = FALSE;
		bool do_break = FALSE;

		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Extract visibility (before blink) */
		if (m_ptr->ml) visible = TRUE;

		/* Extract visibility from carrying lite */
		if (rf_has(r_ptr->flags, RF_HAS_LITE)) visible = TRUE;

		/* Extract the attack "power" */
		switch (effect)
		{
			case RBE_HURT:      power = 60; break;
			case RBE_POISON:    power =  5; break;
			case RBE_UN_BONUS:  power = 20; break;
			case RBE_UN_POWER:  power = 15; break;
			case RBE_EAT_GOLD:  power =  5; break;
			case RBE_EAT_ITEM:  power =  5; break;
			case RBE_EAT_FOOD:  power =  5; break;
			case RBE_EAT_LIGHT: power =  5; break;
			case RBE_ACID:      power =  0; break;
			case RBE_ELEC:      power = 10; break;
			case RBE_FIRE:      power = 10; break;
			case RBE_COLD:      power = 10; break;
			case RBE_BLIND:     power =  2; break;
			case RBE_CONFUSE:   power = 10; break;
			case RBE_TERRIFY:   power = 10; break;
			case RBE_PARALYZE:  power =  2; break;
			case RBE_LOSE_STR:  power =  0; break;
			case RBE_LOSE_DEX:  power =  0; break;
			case RBE_LOSE_CON:  power =  0; break;
			case RBE_LOSE_INT:  power =  0; break;
			case RBE_LOSE_WIS:  power =  0; break;
			case RBE_LOSE_CHR:  power =  0; break;
			case RBE_LOSE_ALL:  power =  2; break;
			case RBE_SHATTER:   power = 60; break;
			case RBE_EXP_10:    power =  5; break;
			case RBE_EXP_20:    power =  5; break;
			case RBE_EXP_40:    power =  5; break;
			case RBE_EXP_80:    power =  5; break;
			case RBE_HALLU:     power = 10; break;
		}


		/* Monster hits player */
		if (!effect || check_hit(power, rlev))
		{
			/* Always disturbing */
			disturb(1, 0);


			/* Hack -- Apply "protection from evil" */
			if (p_ptr->timed[TMD_PROTEVIL] > 0)
			{
				/* Learn about the evil flag */
				if (m_ptr->ml)
				{
					rf_on(l_ptr->flags, RF_EVIL);
				}

				if (rf_has(r_ptr->flags, RF_EVIL) &&
				    p_ptr->lev >= rlev &&
				    randint0(100) + p_ptr->lev > 50)
				{
					/* Message */
					msg("%^s is repelled.", m_name);

					/* Hack -- Next attack */
					continue;
				}
			}


			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Assume no sound */
			sound_msg = MSG_GENERIC;

			/* Describe the attack method */
			switch (method)
			{
				case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					sound_msg = MSG_MON_HIT;
					break;
				}

				case RBM_TOUCH:
				{
					act = "touches you.";
					sound_msg = MSG_MON_TOUCH;
					break;
				}

				case RBM_PUNCH:
				{
					act = "punches you.";
					do_stun = 1;
					sound_msg = MSG_MON_PUNCH;
					break;
				}

				case RBM_KICK:
				{
					act = "kicks you.";
					do_stun = 1;
					sound_msg = MSG_MON_KICK;
					break;
				}

				case RBM_CLAW:
				{
					act = "claws you.";
					do_cut = 1;
					sound_msg = MSG_MON_CLAW;
					break;
				}

				case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					sound_msg = MSG_MON_BITE;
					break;
				}

				case RBM_STING:
				{
					act = "stings you.";
					sound_msg = MSG_MON_STING;
					break;
				}

				case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					sound_msg = MSG_MON_BUTT;
					break;
				}

				case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					sound_msg = MSG_MON_CRUSH;
					break;
				}

				case RBM_ENGULF:
				{
					act = "engulfs you.";
					sound_msg = MSG_MON_ENGULF;
					break;
				}

				case RBM_CRAWL:
				{
					act = "crawls on you.";
					sound_msg = MSG_MON_CRAWL;
					break;
				}

				case RBM_DROOL:
				{
					act = "drools on you.";
					sound_msg = MSG_MON_DROOL;
					break;
				}

				case RBM_SPIT:
				{
					act = "spits on you.";
					sound_msg = MSG_MON_SPIT;
					break;
				}

				case RBM_GAZE:
				{
					act = "gazes at you.";
					sound_msg = MSG_MON_GAZE;
					break;
				}

				case RBM_WAIL:
				{
					act = "wails at you.";
					sound_msg = MSG_MON_WAIL;
					break;
				}

				case RBM_SPORE:
				{
					act = "releases spores at you.";
					sound_msg = MSG_MON_SPORE;
					break;
				}

				case RBM_BEG:
				{
					act = "begs you for money.";
					sound_msg = MSG_MON_BEG;
					break;
				}

				case RBM_INSULT:
				{
					act = desc_insult[randint0(MAX_DESC_INSULT)];
					sound_msg = MSG_MON_INSULT;
					break;
				}

				case RBM_MOAN:
				{
					act = desc_moan[randint0(MAX_DESC_MOAN)];
					sound_msg = MSG_MON_MOAN;
					break;
				}
			}

			/* Message */
			if (act)
				msgt(sound_msg, "%^s %s", m_name, act);


			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			if (d_dice > 0 && d_side > 0)
				damage = damroll(d_dice, d_side);
			else
				damage = 0;

			/* Apply appropriate damage */
			switch (effect)
			{
				case 0:
				{
					/* Hack -- Assume obvious */
					obvious = TRUE;

					/* Hack -- No damage */
					damage = 0;

					break;
				}

				case RBE_HURT:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 240) ? ac : 240) / 400);

					/* Take damage */
					take_hit(damage, ddesc);

					break;
				}

				case RBE_POISON:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->state.flags[OF_RES_POIS] ||
							p_ptr->timed[TMD_OPP_POIS]))
					{
						if (inc_timed(TMD_POISONED, randint1(rlev) + 5, TRUE))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_POIS);

					break;
				}

				case RBE_UN_BONUS:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Allow complete resist */
					if (!p_ptr->state.flags[OF_RES_DISEN])
					{
						/* Apply disenchantment */
						if (apply_disenchant(0)) obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_DISEN);

					break;
				}

				case RBE_UN_POWER:
				{
					int unpower = 0, newcharge;

					/* Take damage */
					take_hit(damage, ddesc);

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = randint0(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Drain charged wands/staves */
						if ((o_ptr->tval == TV_STAFF) ||
						    (o_ptr->tval == TV_WAND))
						{
							/* Charged? */
							if (o_ptr->pval[DEFAULT_PVAL])
							{
								/* Get number of charge to drain */
								unpower = (rlev / (o_ptr->kind->level + 2)) + 1;

								/* Get new charge value, don't allow negative */
								newcharge = MAX((o_ptr->pval[DEFAULT_PVAL]
										- unpower),0);
								
								/* Remove the charges */
								o_ptr->pval[DEFAULT_PVAL] = newcharge;
							}
						}

						if (unpower)
						{
							int heal = rlev * unpower;

							msg("Energy drains from your pack!");

							obvious = TRUE;

							/* Don't heal more than max hp */
							heal = MIN(heal, m_ptr->maxhp - m_ptr->hp);

							/* Heal */
							m_ptr->hp += heal;

							/* Redraw (later) if needed */
							if (p_ptr->health_who == m_idx)
								p_ptr->redraw |= (PR_HEALTH);

							/* Combine / Reorder the pack */
							p_ptr->notice |= (PN_COMBINE | PN_REORDER);

							/* Redraw stuff */
							p_ptr->redraw |= (PR_INVEN);

							/* Affect only a single inventory slot */
							break;
						}
					}

					break;
				}

				case RBE_EAT_GOLD:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Obvious */
					obvious = TRUE;

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->timed[TMD_PARALYZED] &&
					    (randint0(100) < (adj_dex_safe[p_ptr->state.stat_ind[A_DEX]] +
					                      p_ptr->lev)))
					{
						/* Saving throw message */
						msg("You quickly protect your money pouch!");

						/* Occasional blink anyway */
						if (randint0(3)) blinked = TRUE;
					}

					/* Eat gold */
					else
					{
						gold = (p_ptr->au / 10) + randint1(25);
						if (gold < 2) gold = 2;
						if (gold > 5000) gold = (p_ptr->au / 20) + randint1(3000);
						if (gold > p_ptr->au) gold = p_ptr->au;
						p_ptr->au -= gold;
						if (gold <= 0)
						{
							msg("Nothing was stolen.");
							break;
						}

						/* Let the player know they were robbed */
						msg("Your purse feels lighter.");
						if (p_ptr->au)
							msg("%ld coins were stolen!", (long)gold);
						else
							msg("All of your coins were stolen!");

						/* While we have gold, put it in objects */
						while (gold > 0)
						{
							int amt;

							/* Create a new temporary object */
							object_type o;
							object_wipe(&o);
							object_prep(&o, objkind_get(TV_GOLD, SV_GOLD), 0, MINIMISE);

							/* Amount of gold to put in this object */
							amt = gold > MAX_PVAL ? MAX_PVAL : gold;
							o.pval[DEFAULT_PVAL] = amt;
							gold -= amt;

							/* Give the gold to the monster */
							monster_carry(m_idx, &o);
						}

						/* Redraw gold */
						p_ptr->redraw |= (PR_GOLD);

						/* Blink away */
						blinked = TRUE;
					}

					break;
				}

				case RBE_EAT_ITEM:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->timed[TMD_PARALYZED] &&
					    (randint0(100) < (adj_dex_safe[p_ptr->state.stat_ind[A_DEX]] +
					                      p_ptr->lev)))
					{
						/* Saving throw message */
						msg("You grab hold of your backpack!");

						/* Occasional "blink" anyway */
						blinked = TRUE;

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Pick an item */
						i = randint0(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Skip artifacts */
						if (o_ptr->artifact) continue;

						/* Get a description */
						object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

						/* Message */
						msg("%sour %s (%c) was stolen!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain local object */
						object_copy(i_ptr, o_ptr);

						/* Modify number */
						i_ptr->number = 1;

						/* Hack -- If a rod, staff, or wand, allocate total
						 * maximum timeouts or charges between those
						 * stolen and those missed. -LM-
						 */
						distribute_charges(o_ptr, i_ptr, 1);

						/* Carry the object */
						(void)monster_carry(m_idx, i_ptr);

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Blink away */
						blinked = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_FOOD:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Steal some food */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item from the pack */
						i = randint0(INVEN_PACK);

						/* Get the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Skip non-food objects */
						if (o_ptr->tval != TV_FOOD) continue;

						/* Get a description */
						object_desc(o_name, sizeof(o_name), o_ptr,
									ODESC_PREFIX | ODESC_BASE);

						/* Message */
						msg("%sour %s (%c) was eaten!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_LIGHT:
				{
					bitflag f[OF_SIZE];

					/* Take damage */
					take_hit(damage, ddesc);

					/* Get the light, and its flags */
					o_ptr = &p_ptr->inventory[INVEN_LIGHT];
					object_flags(o_ptr, f);

					/* Drain fuel where applicable */
					if (!of_has(f, OF_NO_FUEL) && (o_ptr->timeout > 0))
					{
						/* Reduce fuel */
						o_ptr->timeout -= (250 + randint1(250));
						if (o_ptr->timeout < 1) o_ptr->timeout = 1;

						/* Notice */
						if (!p_ptr->timed[TMD_BLIND])
						{
							msg("Your light dims.");
							obvious = TRUE;
						}

						/* Redraw stuff */
						p_ptr->redraw |= (PR_EQUIP);
					}

					break;
				}

				case RBE_ACID:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are covered in acid!");

					/* Special damage */
					damage = adjust_dam(GF_ACID, damage, RANDOMISE, 
						check_for_resist(GF_ACID));
					if (damage) {
						take_hit(damage, ddesc);
						inven_damage(GF_ACID, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_ACID);

					break;
				}

				case RBE_ELEC:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are struck by electricity!");

					/* Take damage (special) */
					damage = adjust_dam(GF_ELEC, damage, RANDOMISE,
						check_for_resist(GF_ELEC));
					if (damage) {
						take_hit(damage, ddesc);
						inven_damage(GF_ELEC, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_ELEC);

					break;
				}

				case RBE_FIRE:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are enveloped in flames!");

					/* Take damage (special) */
					damage = adjust_dam(GF_FIRE, damage, RANDOMISE,
						check_for_resist(GF_FIRE));
					if (damage) {
						take_hit(damage, ddesc);
						inven_damage(GF_FIRE, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_FIRE);

					break;
				}

				case RBE_COLD:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are covered with frost!");

					/* Take damage (special) */
					damage = adjust_dam(GF_COLD, damage, RANDOMISE,
						check_for_resist(GF_COLD));
					if (damage) {
						take_hit(damage, ddesc);
						inven_damage(GF_COLD, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_COLD);

					break;
				}

				case RBE_BLIND:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "blind" */
					if (!p_ptr->state.flags[OF_RES_BLIND])
					{
						if (inc_timed(TMD_BLIND, 10 + randint1(rlev), TRUE))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_BLIND);

					break;
				}

				case RBE_CONFUSE:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "confused" */
					if (!p_ptr->state.flags[OF_RES_CONFU])
					{
						if (inc_timed(TMD_CONFUSED, 3 + randint1(rlev), TRUE))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_CONFU);

					break;
				}

				case RBE_TERRIFY:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "afraid" */
					if (p_ptr->state.flags[OF_RES_FEAR])
					{
						msg("You stand your ground!");
						obvious = TRUE;
					}
					else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
					{
						msg("You stand your ground!");
						obvious = TRUE;
					}
					else
					{
						if (inc_timed(TMD_AFRAID, 3 + randint1(rlev), TRUE))
							obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_FEAR);

					break;
				}

				case RBE_PARALYZE:
				{
					/* Hack -- Prevent perma-paralysis via damage */
					if (p_ptr->timed[TMD_PARALYZED] && (damage < 1)) damage = 1;

					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "paralyzed" */
					if (p_ptr->state.flags[OF_FREE_ACT])
					{
						msg("You are unaffected!");
						obvious = TRUE;
					}
					else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
					{
						msg("You resist the effects!");
						obvious = TRUE;
					}
					else
					{
						if (inc_timed(TMD_PARALYZED, 3 + randint1(rlev), TRUE))
							obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FREE);

					break;
				}

				case RBE_LOSE_STR:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_STR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_INT:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_INT, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_WIS:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_WIS, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_DEX:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_DEX, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CON:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CON, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CHR:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CHR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_ALL:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Damage (stats) */
					if (do_dec_stat(A_STR, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_DEX, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_CON, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_INT, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_WIS, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_CHR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_SHATTER:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 240) ? ac : 240) / 400);

					/* Take damage */
					take_hit(damage, ddesc);

					/* Radius 8 earthquake centered at the monster */
					if (damage > 23)
					{
						int px_old = p_ptr->px;
						int py_old = p_ptr->py;

						earthquake(m_ptr->fy, m_ptr->fx, 8);

						/* Stop the blows if the player is pushed away */
						if ((px_old != p_ptr->px) ||
						    (py_old != p_ptr->py))
						    do_break = TRUE;
					}
					break;
				}

				case RBE_EXP_10:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					/* XXX Eddie need a DRS for HOLD_LIFE */
					wieldeds_notice_flag(OF_HOLD_LIFE);

					if (p_ptr->state.flags[OF_HOLD_LIFE] && (randint0(100) < 95))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->state.flags[OF_HOLD_LIFE])
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p_ptr, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p_ptr, d, FALSE);
						}
					}
					break;
				}

				case RBE_EXP_20:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					wieldeds_notice_flag(OF_HOLD_LIFE);

					if (p_ptr->state.flags[OF_HOLD_LIFE] && (randint0(100) < 90))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->state.flags[OF_HOLD_LIFE])
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p_ptr, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p_ptr, d, FALSE);
						}
					}
					break;
				}

				case RBE_EXP_40:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					wieldeds_notice_flag(OF_HOLD_LIFE);

					if (p_ptr->state.flags[OF_HOLD_LIFE] && (randint0(100) < 75))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->state.flags[OF_HOLD_LIFE])
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p_ptr, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p_ptr, d, FALSE);
						}
					}
					break;
				}

				case RBE_EXP_80:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(damage, ddesc);

					wieldeds_notice_flag(OF_HOLD_LIFE);

					if (p_ptr->state.flags[OF_HOLD_LIFE] && (randint0(100) < 50))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->state.flags[OF_HOLD_LIFE])
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p_ptr, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p_ptr, d, FALSE);
						}
					}
					break;
				}

				case RBE_HALLU:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "image" */
					if (!p_ptr->state.flags[OF_RES_CHAOS])
					{
						if (inc_timed(TMD_IMAGE, 3 + randint1(rlev / 2), TRUE))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_RES_CHAOS);

					break;
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (randint0(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(5) + 5; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(50) + 50; break;
					case 5: k = randint1(100) + 100; break;
					case 6: k = 300; break;
					default: k = 500; break;
				}

				/* Apply the cut */
				if (k) (void)inc_timed(TMD_CUT, k, TRUE);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(10) + 10; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(30) + 30; break;
					case 5: k = randint1(40) + 40; break;
					case 6: k = 100; break;
					default: k = 200; break;
				}

				/* Apply the stun */
				if (k) (void)inc_timed(TMD_STUN, k, TRUE);
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
				case RBM_HIT:
				case RBM_TOUCH:
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_STING:
				case RBM_BUTT:
				case RBM_CRUSH:
				case RBM_ENGULF:

				/* Visible monsters */
				if (m_ptr->ml)
				{
					/* Disturbing */
					disturb(1, 0);

					/* Message */
					msg("%^s misses you.", m_name);
				}

				break;
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (l_ptr->blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (l_ptr->blows[ap_cnt] < MAX_UCHAR)
				{
					l_ptr->blows[ap_cnt]++;
				}
			}
		}

		/* Skip the other blows if necessary */
		if (do_break) break;
	}


	/* Blink away */
	if (blinked)
	{
		msg("There is a puff of smoke!");
		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}


	/* Always notice cause of death */
	if (p_ptr->is_dead && (l_ptr->deaths < MAX_SHORT))
	{
		l_ptr->deaths++;
	}


	/* Assume we attacked */
	return (TRUE);
}
