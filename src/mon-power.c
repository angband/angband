/**
 * \file mon-power.c
 * \brief functions for monster power evaluation
 *
 * Copyright (c) 2000-11 Chris Carr, Chris Robertson, Andrew Doull
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
#include "game-world.h"
#include "init.h"
#include "mon-init.h"
#include "mon-power.h"
#include "mon-spell.h"
#include "mon-blow-methods.h"
#include "mon-blow-effects.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "z-textblock.h"

bool arg_power;				/* Command arg -- Generate monster power */
bool arg_rebalance;			/* Command arg -- Rebalance monsters */

long *power, *scaled_power, *final_hp, *final_melee_dam, *final_spell_dam;
int *highest_threat;
s32b tot_mon_power;

static long eval_blow_effect(int effect, random_value atk_dam, int rlev)
{
	int adjustment = monster_blow_effect_eval(effect);
	int power = randcalc(atk_dam, rlev, MAXIMISE);

	if (effect == RBE_POISON) {
		power *= 5;
		power /= 4;
		power += rlev;
	} else {
		power += adjustment;
	}

	return power;
}

static byte adj_energy(struct monster_race *race)
{
	unsigned i = race->speed + (rsf_has(race->spell_flags,RSF_HASTE) ? 5 : 0);

	/* Fastest monster in the game is currently +30, but bounds check anyway */
	return turn_energy(MIN(i, N_ELEMENTS(extract_energy) - 1));
}

static long eval_max_dam(struct monster_race *race, int ridx)
{
	int rlev, i;
	int melee_dam = 0, atk_dam = 0, spell_dam = 0;
	int dam = 1;

	/* Extract the monster level, force 1 for town monsters */
	rlev = ((race->level >= 1) ? race->level : 1);

	/* Assume single resist for the elemental attacks */
	spell_dam = best_spell_power(race, 1);

	/* Hack - Apply over 10 rounds */
	spell_dam *= 10;

	/* Scale for frequency and availability of mana / ammo */
	if (spell_dam) {
		int freq = race->freq_spell;

		/* Hack -- always get 1 shot */
		if (freq < 10) freq = 10;

		/* Adjust for frequency */
		spell_dam = spell_dam * freq / 100;
	}

	/* Check attacks */
	for (i = 0; i < z_info->mon_blows_max; i++) {
		int effect, method;
		random_value dice;

		if (!race->blow) break;

		/* Extract the attack infomation */
		effect = race->blow[i].effect;
		method = race->blow[i].method;
		dice = race->blow[i].dice;

		/* Assume maximum damage */
		atk_dam = eval_blow_effect(effect, dice, race->level);

		/* Factor for dangerous side effects */
		if (monster_blow_method_stun(method)) {
			/* Stun definitely most dangerous*/
			atk_dam *= 4;
			atk_dam /= 3;
		} else if (monster_blow_method_stun(method)) {
			/* Cut */
			atk_dam *= 7;
			atk_dam /= 5;
		}

		/* Normal melee attack */
		if (!rf_has(race->flags, RF_NEVER_BLOW)) {
			/* Keep a running total */
			melee_dam += atk_dam;
		}
	}

	/* Apply damage over 10 rounds. We assume that the monster has to make
	 * contact first.
	 * Hack - speed has more impact on melee as has to stay in contact with
	 * player.
	 * Hack - this is except for pass wall and kill wall monsters which can
	 * always get to the player.
	 * Hack - use different values for huge monsters as they strike out to
	 * range 2. */
		if (flags_test(race->flags, RF_SIZE, RF_KILL_WALL, RF_PASS_WALL,
					   FLAG_END))
			melee_dam *= 10;
		else
			melee_dam = melee_dam * 3 + melee_dam * adj_energy(race) / 7;

		/* Scale based on attack accuracy. We make a massive number of
		 * assumptions here and just use monster level. */
		melee_dam = melee_dam * MIN(45 + rlev * 3, 95) / 100;

		/* Hack -- Monsters that multiply ignore the following reductions */
		if (!rf_has(race->flags, RF_MULTIPLY)) {
			/*Reduce damamge potential for monsters that move randomly */
			if (flags_test(race->flags, RF_SIZE, RF_RAND_25, RF_RAND_50,
						   FLAG_END)) {
				int reduce = 100;

				if (rf_has(race->flags, RF_RAND_25)) reduce -= 25;
				if (rf_has(race->flags, RF_RAND_50)) reduce -= 50;

				/* Even moving randomly one in 8 times will hit the player */
				reduce += (100 - reduce) / 8;

				/* Adjust the melee damage */
				melee_dam = (melee_dam * reduce) / 100;
			}

			/* Monsters who can't move are much less of a combat threat */
			if (rf_has(race->flags, RF_NEVER_MOVE)) {
				if (rsf_has(race->spell_flags, RSF_TELE_TO) ||
				    rsf_has(race->spell_flags, RSF_BLINK)) {
					/* Scale for frequency */
					melee_dam = melee_dam / 5 + 4 * melee_dam *
						race->freq_spell / 500;

					/* Incorporate spell failure chance */
					if (!rf_has(race->flags, RF_STUPID))
						melee_dam = melee_dam / 5 + 4 * melee_dam *
							MIN(75 + (rlev + 3) / 4, 100) / 500;
				}
				else if (rf_has(race->flags, RF_INVISIBLE))
					melee_dam /= 3;
				else
					melee_dam /= 5;
			}
		}

		/* But keep at a minimum */
		if (melee_dam < 1) melee_dam = 1;

	/* Combine spell and melee damage */
	dam = (spell_dam + melee_dam);

	highest_threat[ridx] = dam;
	final_spell_dam[ridx] = spell_dam;
	final_melee_dam[ridx] = melee_dam;

	/* Adjust for speed - monster at speed 120 will do double damage, monster
	 * at speed 100 will do half, etc.  Bonus for monsters who can haste self */
	dam = (dam * adj_energy(race)) / 10;

	/* Adjust threat for speed -- multipliers are more threatening. */
	if (rf_has(race->flags, RF_MULTIPLY))
		highest_threat[ridx] = (highest_threat[ridx] * adj_energy(race)) / 5;

	/* Adjust threat for friends, this can be improved, but is probably good
	 * enough for now. */
	if (race->friends)
		highest_threat[ridx] *= 2;
	else if (race->friends_base)
		/* Friends base is weaker, because they are <= monster level */
		highest_threat[ridx] = highest_threat[ridx] * 3 / 2;
		
	/* But keep at a minimum */
	if (dam < 1) dam  = 1;

	/* We're done */
	return (dam);
}

static long eval_hp_adjust(struct monster_race *race)
{
	long hp;
	int resists = 1;
	int hide_bonus = 0;

	/* Get the monster base hitpoints */
	hp = race->avg_hp;

	/* Never moves with no ranged attacks - high hit points count for less */
	if (rf_has(race->flags, RF_NEVER_MOVE) &&
		!(race->freq_innate || race->freq_spell)) {
		hp /= 2;
		if (hp < 1)
			hp = 1;
	}

	/* Just assume healers have more staying power */
	if (rsf_has(race->spell_flags, RSF_HEAL)) hp = (hp * 6) / 5;

	/* Miscellaneous improvements */
	if (rf_has(race->flags, RF_REGENERATE)) {hp *= 10; hp /= 9;}
	if (rf_has(race->flags, RF_PASS_WALL)) {hp *= 3; hp /= 2;}

	/* Calculate hide bonus */
	if (rf_has(race->flags, RF_EMPTY_MIND))
		hide_bonus += 2;
	else {
		if (rf_has(race->flags, RF_COLD_BLOOD)) hide_bonus += 1;
		if (rf_has(race->flags, RF_WEIRD_MIND)) hide_bonus += 1;
	}

	/* Invisibility */
	if (rf_has(race->flags, RF_INVISIBLE))
		hp = (hp * (race->level + hide_bonus + 1)) / MAX(1, race->level);

	/* Monsters that can teleport are a hassle, and can easily run away */
	if (flags_test(race->spell_flags, RSF_SIZE, RSF_TPORT, RSF_TELE_AWAY,
				   RSF_TELE_LEVEL, FLAG_END))
		hp = (hp * 6) / 5;

	/* Monsters that multiply are tougher to kill */
	if (rf_has(race->flags, RF_MULTIPLY)) hp *= 2;

	/* Monsters with resistances are harder to kill.
	 * Therefore effective slays / brands against them are worth more. */
	if (rf_has(race->flags, RF_IM_ACID))
		resists += 2;
	if (rf_has(race->flags, RF_IM_FIRE))
		resists += 2;
	if (rf_has(race->flags, RF_IM_COLD))
		resists += 2;
	if (rf_has(race->flags, RF_IM_ELEC))
		resists += 2;
	if (rf_has(race->flags, RF_IM_POIS))
		resists += 2;

	/* Bonus for multiple basic resists and weapon resists */
	if (resists >= 12)
		resists *= 6;
	else if (resists >= 10)
		resists *= 4;
	else if (resists >= 8)
		resists *= 3;
	else if (resists >= 6)
		resists *= 2;

	/* If quite resistant, reduce resists by defense holes */
	if (resists >= 6) {
		if (rf_has(race->flags, RF_HURT_ROCK))
			resists -= 1;
		if (rf_has(race->flags, RF_HURT_LIGHT))
			resists -= 1;
		if (!rf_has(race->flags, RF_NO_SLEEP))
			resists -= 3;
		if (!rf_has(race->flags, RF_NO_FEAR))
			resists -= 2;
		if (!rf_has(race->flags, RF_NO_CONF))
			resists -= 2;
		if (!rf_has(race->flags, RF_NO_STUN))
			resists -= 1;

		if (resists < 5)
			resists = 5;
	}

	/* If quite resistant, bonus for high resists */
	if (resists >= 3) {
		if (rf_has(race->flags, RF_IM_WATER))
			resists += 1;
		if (rf_has(race->flags, RF_IM_NETHER))
			resists += 1;
		if (rf_has(race->flags, RF_IM_NEXUS))
			resists += 1;
		if (rf_has(race->flags, RF_IM_DISEN))
			resists += 1;
	}

	/* Scale resists */
	resists = resists * 25;

	/* Monster resistances */
	if (resists < (race->ac + resists) / 3)
		hp += (hp * resists) / (150 + race->level); 	
	else
		hp += (hp * (race->ac + resists) / 3) / (150 + race->level); 			

	/* Boundary control */
	if (hp < 1)
		hp = 1;

	return (hp);
}

/**
 * Write an amended monster.txt file
 */
void write_monster_entries(ang_file *fff)
{
	ang_file *old;
	char buf[1024];

	path_build(buf, sizeof(buf), ANGBAND_DIR_GAMEDATA, "monster.txt");
	old = file_open(buf, MODE_READ, FTYPE_TEXT);

	while (file_getl(old, buf, sizeof(buf))) {
		static struct monster_race *race;
		int i, n;

		/* Change monster record */
		if (1 == sscanf(buf, "name:%d", &n)) {
			race = &r_info[n];
			file_putf(fff, "%s\n", buf);
		}

		/* Rewrite 'power' line */
		else if (1 == sscanf(buf, "power:%d", &i))
			file_putf(fff,"power:%d:%d:%d:%d:%d\n", race->level, race->rarity,
					  power[n], scaled_power[n], race->mexp);

		/* Just copy */
		else
			file_putf(fff, "%s\n", buf);
	}

	file_close(old);
}

/**
 * Evaluate the whole monster list and write a new one.  power and scaled_power
 * are always adjusted, level, rarity and mexp only if requested.
 */
errr eval_monster_power(struct monster_race *racelist)
{
	int i, j, iteration;
	byte lvl;
	struct monster_race *race = NULL;
	ang_file *mon_fp;
	char buf[1024];
	bool dump = FALSE;
	bool wrote = TRUE;

	/* Allocate arrays */
	power = mem_zalloc(z_info->r_max * sizeof(long));
	scaled_power = mem_zalloc(z_info->r_max * sizeof(long));
	final_hp = mem_zalloc(z_info->r_max * sizeof(long));
	final_melee_dam = mem_zalloc(z_info->r_max * sizeof(long));
	final_spell_dam = mem_zalloc(z_info->r_max * sizeof(long));
	highest_threat = mem_zalloc(z_info->r_max * sizeof(int));

	for (iteration = 0; iteration < 3; iteration ++) {
		long hp, av_hp, dam, av_dam;
		long *tot_hp = mem_zalloc(z_info->max_depth * sizeof(long));
		long *tot_dam = mem_zalloc(z_info->max_depth * sizeof(long));
		long *mon_count = mem_zalloc(z_info->max_depth * sizeof(long));

		/* Reset the sum of all monster power values */
		tot_mon_power = 0;

		/* Go through r_info and evaluate power ratings & flows. */
		for (i = 0; i < z_info->r_max; i++)	{

			/* Point at the "info" */
			race = &racelist[i];

			/* Set the current level */
			lvl = race->level;

			/* Maximum damage this monster can do in 10 game turns */
			dam = eval_max_dam(race, i);

			/* Adjust hit points based on resistances */
			hp = eval_hp_adjust(race);

			/* Hack -- set exp */
			if (lvl == 0)
				race->mexp = 0L;
			else {
				/* Compute depths of non-unique monsters */
				if (!rf_has(race->flags, RF_UNIQUE)) {
					long mexp = (hp * dam) / 25;
					long threat = highest_threat[i];

					/* Compute level algorithmically */
					for (j = 1; (mexp > j + 4) || (threat > j + 5);
						 mexp -= j * j, threat -= (j + 4), j++);

					/* Set level */
					lvl = MIN(( j > 250 ? 90 + (j - 250) / 20 : /* Level 90+ */
								(j > 130 ? 70 + (j - 130) / 6 :	/* Level 70+ */
								 (j > 40 ? 40 + (j - 40) / 3 :	/* Level 40+ */
								  j))), 99);

					/* Set level */
					if (arg_rebalance)
						race->level = lvl;
				}

				if (arg_rebalance) {
					/* Hack -- for Ungoliant */
					if (hp > 10000)
						race->mexp = (hp / 25) * (dam / lvl);
					else race->mexp = (hp * dam) / (lvl * 25);

					/* Round to 2 significant figures */
					if (race->mexp > 100) {
						if (race->mexp < 1000) {
							race->mexp = (race->mexp + 5) / 10;
							race->mexp *= 10;
						}
						else if (race->mexp < 10000) {
							race->mexp = (race->mexp + 50) / 100;
							race->mexp *= 100;
						}
						else if (race->mexp < 100000) {
							race->mexp = (race->mexp + 500) / 1000;
							race->mexp *= 1000;
						}
						else if (race->mexp < 1000000) {
							race->mexp = (race->mexp + 5000) / 10000;
							race->mexp *= 10000;
						}
						else if (race->mexp < 10000000) {
							race->mexp = (race->mexp + 50000) / 100000;
							race->mexp *= 100000;
						}
					}
				}
			}

			/* If we're rebalancing, this is a nop, if not, we restore the
			 * orig value */
			lvl = race->level;
			if ((lvl) && (race->mexp < 1L))
				race->mexp = 1L;

			/*
			 * Hack - We have to use an adjustment factor to prevent overflow.
			 * Try to scale evenly across all levels instead of scaling by level
			 */
			hp /= 2;
			if(hp < 1)
				hp = 1;
			final_hp[i] = hp;

			/* Define the power rating */
			power[i] = hp * dam;

			/* Adjust for group monsters, using somewhat arbitrary 
			 * multipliers for now */
			if (!rf_has(race->flags, RF_UNIQUE)) {
				if (race->friends)
					power[i] *= 3;
			}

			/* Adjust for escorts */
			if (race->friends_base) 
				power[i] *= 2;


			/* Adjust for multiplying monsters. This is modified by the speed,
			 * as fast multipliers are much worse than slow ones. We also
			 * adjust for ability to bypass walls or doors. */
			if (rf_has(race->flags, RF_MULTIPLY)) {
				int adj_power;

				if (flags_test(race->flags, RF_SIZE, RF_KILL_WALL,
							   RF_PASS_WALL, FLAG_END))
					adj_power = power[i] * adj_energy(race);
				else if (flags_test(race->flags, RF_SIZE, RF_OPEN_DOOR,
									RF_BASH_DOOR, FLAG_END))
					adj_power = power[i] * adj_energy(race) * 3 / 2;
				else
					adj_power = power[i] * adj_energy(race) / 2;

				power[i] = MAX(power[i], adj_power);
			}

			/* Update the running totals - these will be used as divisors later
			 * Total HP / dam / count for everything up to the current level */
			for (j = lvl; j < (lvl == 0 ? lvl + 1: z_info->max_depth); j++)	{
				int count = 10;

				/* Uniques don't count towards monster power on the level. */
				if (rf_has(race->flags, RF_UNIQUE)) continue;

				/* Specifically placed monsters don't count towards monster
				 * power on the level. */
				if (!(race->rarity)) continue;

				/* Hack -- provide adjustment factor to prevent overflow */
				if ((j == 90) && (race->level < 90)) {
					hp /= 10;
					dam /= 10;
				}

				if ((j == 65) && (race->level < 65)) {
					hp /= 10;
					dam /= 10;
				}

				if ((j == 40) && (race->level < 40)) {
					hp /= 10;
					dam /= 10;
				}

				/* Hack - if it's a group monster or multiplying monster, add
				 * several to the count so the averages don't get thrown off */

				if (race->friends || race->friends_base)
					count = 15;

				if (rf_has(race->flags, RF_MULTIPLY)) {
					int adj_energy_amt;

					if (flags_test(race->flags, RF_SIZE, RF_KILL_WALL,
								   RF_PASS_WALL, FLAG_END))
						adj_energy_amt = adj_energy(race);
					else if (flags_test(race->flags, RF_SIZE, RF_OPEN_DOOR,
										RF_BASH_DOOR, FLAG_END))
						adj_energy_amt = adj_energy(race) * 3 / 2;
					else
						adj_energy_amt = adj_energy(race) / 2;

					count = MAX(1, adj_energy_amt) * count;
				}

				/* Very rare monsters count less towards total monster power
				 * on the level. */
				if (race->rarity > count) {
					hp = hp * count / race->rarity;
					dam = dam * count / race->rarity;

					count = race->rarity;
				}

				tot_hp[j] += hp;
				tot_dam[j] += dam;

				mon_count[j] += count / race->rarity;
			}

		}

		/* Apply divisors now */
		for (i = 0; i < z_info->r_max; i++) {
			int new_power;

			/* Point at the "info" */
			race = &racelist[i];

			/* Extract level */
			lvl = race->level;

			/* Paranoia */
			if (tot_hp[lvl] != 0 && tot_dam[lvl] != 0) {
				scaled_power[i] = power[i];

				/* Divide by av HP and av damage for all in-level monsters */
				/* Note we have factored in the above 'adjustment factor' */
				av_hp = tot_hp[lvl] * 10 / mon_count[lvl];
				av_dam = tot_dam[lvl] * 10 / mon_count[lvl];

				/* Justifiable paranoia - avoid divide by zero errors */
				if (av_hp > 0)
					scaled_power[i] = scaled_power[i] / av_hp;
				if (av_dam > 0)
					scaled_power[i] = scaled_power[i] / av_dam;

				/* Never less than 1 */
				if (power[i] < 1)
					power[i] = 1;

				/* Set powers */
				if (arg_rebalance) {
					race->power = power[i];
					race->scaled_power = scaled_power[i];
				}

				/* Get power */
				new_power = power[i];

				/* Compute rarity algorithmically */
				for (j = 1; new_power > j; new_power -= j * j, j++);

				/* Set rarity */
				if (arg_rebalance)
					race->rarity = j;
			}
		}

		mem_free(mon_count);
		mem_free(tot_dam);
		mem_free(tot_hp);
	}

	/* Determine total monster power */
	for (i = 0; i < z_info->r_max; i++)
		tot_mon_power += r_info[i].scaled_power;

	if (dump) {
		/* Dump the power details */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "mon_power.txt");
		mon_fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);

		file_putf(mon_fp, "ridx|level|rarity|d_char|name|pwr|scaled|melee|spell|hp\n");

		for (i = 0; i < z_info->r_max; i++) {
			char mbstr[MB_LEN_MAX + 1] = { 0 };
			race = &r_info[i];

			/* Don't print anything for nonexistent monsters */
			if (!race->name) continue;

			wctomb(mbstr, race->d_char);
			file_putf(mon_fp, "%d|%d|%d|%s|%s|%d|%d|%d|%d|%d\n", race->ridx,
				race->level, race->rarity, mbstr, race->name,
				power[i], scaled_power[i], final_melee_dam[i],
				final_spell_dam[i], final_hp[i]);
		}

		file_close(mon_fp);
	}

	/* Write to the user directory */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "new_monster.txt");

	if (text_lines_to_file(buf, write_monster_entries)) {
		msg("Failed to create file %s.new", buf);
		wrote = FALSE;
	}

	/* Free power arrays */
	mem_free(highest_threat);
	mem_free(final_spell_dam);
	mem_free(final_melee_dam);
	mem_free(final_hp);
	mem_free(scaled_power);
	mem_free(power);

	/* Success */
	return wrote ? 0 : -1;
}
