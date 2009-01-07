/*
 * File: monster1.c
 * Purpose: Monster description code.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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


/*
 * Pronoun arrays, by gender.
 */
static cptr wd_he[3] = { "it", "he", "she" };
static cptr wd_his[3] = { "its", "his", "her" };


/*
 * Pluralizer.  Args(count, singular, plural)
 */
#define plural(c, s, p)    (((c) == 1) ? (s) : (p))



static void output_list(const char *list[], int num, byte attr)
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

		text_out_c(attr, list[i]);
	}
}


static void output_desc_list(int msex, cptr intro, cptr list[], int n, byte attr)
{
	if (n != 0)
	{
		/* Output intro */
		text_out(format("%^s %s ", wd_he[msex], intro));

		/* Output list */
		output_list(list, n, attr);

		/* Output end */
		text_out(".  ");
	}
}






/*
 * Determine if the "armor" is known
 * The higher the level, the fewer kills needed.
 */
static bool know_armour(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b kills = l_ptr->tkills;

	/* Normal monsters */
	if (kills > 304 / (4 + level)) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags[0] & RF0_UNIQUE)) return (FALSE);

	/* Unique monsters */
	if (kills > 304 / (38 + (5 * level) / 4)) return (TRUE);

	/* Assume false */
	return (FALSE);
}


/*
 * Determine if the "damage" of the given attack is known
 * the higher the level of the monster, the fewer the attacks you need,
 * the more damage an attack does, the more attacks you need
 */
static bool know_damage(int r_idx, const monster_lore *l_ptr, int i)
{
	const monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b a = l_ptr->blows[i];

	s32b d1 = r_ptr->blow[i].d_dice;
	s32b d2 = r_ptr->blow[i].d_side;

	s32b d = d1 * d2;

	/* Normal monsters */
	if ((4 + level) * a > 80 * d) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags[0] & RF0_UNIQUE)) return (FALSE);

	/* Unique monsters */
	if ((4 + level) * (2 * a) > 80 * d) return (TRUE);

	/* Assume false */
	return (FALSE);
}

/*
 * Dump flavour text
 */
static void describe_monster_desc(int r_idx)
{
	const monster_race *r_ptr = &r_info[r_idx];
	text_out("%s\n", r_text + r_ptr->text);
}


static void describe_monster_spells(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];
	int m, n;
	int msex = 0;
	bool breath = FALSE;
	bool magic = FALSE;
	int vn;
	cptr vp[64];


	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Collect innate attacks */
	vn = 0;
	if (l_ptr->spell_flags[0] & RSF0_SHRIEK)  vp[vn++] = "shriek for help";
	if (l_ptr->spell_flags[0] & RSF0_XXX2)    vp[vn++] = "do something";
	if (l_ptr->spell_flags[0] & RSF0_XXX3)    vp[vn++] = "do something";
	if (l_ptr->spell_flags[0] & RSF0_XXX4)    vp[vn++] = "do something";
	if (l_ptr->spell_flags[0] & RSF0_ARROW_1) vp[vn++] = "fire an arrow";
	if (l_ptr->spell_flags[0] & RSF0_ARROW_2) vp[vn++] = "fire arrows";
	if (l_ptr->spell_flags[0] & RSF0_ARROW_3) vp[vn++] = "fire a missile";
	if (l_ptr->spell_flags[0] & RSF0_ARROW_4) vp[vn++] = "fire missiles";
	if (l_ptr->spell_flags[0] & RSF0_BOULDER) vp[vn++] = "throw boulders";

	/* Describe innate attacks */
	output_desc_list(msex, "may", vp, -vn, TERM_WHITE);


	/* Collect breaths */
	vn = 0;
	if (l_ptr->spell_flags[0] & RSF0_BR_ACID) vp[vn++] = "acid";
	if (l_ptr->spell_flags[0] & RSF0_BR_ELEC) vp[vn++] = "lightning";
	if (l_ptr->spell_flags[0] & RSF0_BR_FIRE) vp[vn++] = "fire";
	if (l_ptr->spell_flags[0] & RSF0_BR_COLD) vp[vn++] = "frost";
	if (l_ptr->spell_flags[0] & RSF0_BR_POIS) vp[vn++] = "poison";
	if (l_ptr->spell_flags[0] & RSF0_BR_NETH) vp[vn++] = "nether";
	if (l_ptr->spell_flags[0] & RSF0_BR_LITE) vp[vn++] = "light";
	if (l_ptr->spell_flags[0] & RSF0_BR_DARK) vp[vn++] = "darkness";
	if (l_ptr->spell_flags[0] & RSF0_BR_CONF) vp[vn++] = "confusion";
	if (l_ptr->spell_flags[0] & RSF0_BR_SOUN) vp[vn++] = "sound";
	if (l_ptr->spell_flags[0] & RSF0_BR_CHAO) vp[vn++] = "chaos";
	if (l_ptr->spell_flags[0] & RSF0_BR_DISE) vp[vn++] = "disenchantment";
	if (l_ptr->spell_flags[0] & RSF0_BR_NEXU) vp[vn++] = "nexus";
	if (l_ptr->spell_flags[0] & RSF0_BR_TIME) vp[vn++] = "time";
	if (l_ptr->spell_flags[0] & RSF0_BR_INER) vp[vn++] = "inertia";
	if (l_ptr->spell_flags[0] & RSF0_BR_GRAV) vp[vn++] = "gravity";
	if (l_ptr->spell_flags[0] & RSF0_BR_SHAR) vp[vn++] = "shards";
	if (l_ptr->spell_flags[0] & RSF0_BR_PLAS) vp[vn++] = "plasma";
	if (l_ptr->spell_flags[0] & RSF0_BR_WALL) vp[vn++] = "force";
	if (l_ptr->spell_flags[0] & RSF0_BR_MANA) vp[vn++] = "mana";
	if (l_ptr->spell_flags[0] & RSF0_XXX5)    vp[vn++] = "something";
	if (l_ptr->spell_flags[0] & RSF0_XXX6)    vp[vn++] = "something";
	if (l_ptr->spell_flags[0] & RSF0_XXX7)    vp[vn++] = "something";

	/* Describe breaths */
	if (vn)
	{
		/* Note breath */
		breath = TRUE;

		/* Display */
		text_out("%^s may ", wd_he[msex]);
		text_out_c(TERM_L_RED, "breathe ");
		output_list(vp, -vn, TERM_WHITE);
	}


	/* Collect spells */
	vn = 0;
	if (l_ptr->spell_flags[1] & RSF1_BA_ACID)     vp[vn++] = "produce acid balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_ELEC)     vp[vn++] = "produce lightning balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_FIRE)     vp[vn++] = "produce fire balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_COLD)     vp[vn++] = "produce frost balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_POIS)     vp[vn++] = "produce poison balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_NETH)     vp[vn++] = "produce nether balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_WATE)     vp[vn++] = "produce water balls";
	if (l_ptr->spell_flags[1] & RSF1_BA_MANA)     vp[vn++] = "invoke mana storms";
	if (l_ptr->spell_flags[1] & RSF1_BA_DARK)     vp[vn++] = "invoke darkness storms";
	if (l_ptr->spell_flags[1] & RSF1_DRAIN_MANA)  vp[vn++] = "drain mana";
	if (l_ptr->spell_flags[1] & RSF1_MIND_BLAST)  vp[vn++] = "cause mind blasting";
	if (l_ptr->spell_flags[1] & RSF1_BRAIN_SMASH) vp[vn++] = "cause brain smashing";
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_1)     vp[vn++] = "cause light wounds";
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_2)     vp[vn++] = "cause serious wounds";
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_3)     vp[vn++] = "cause critical wounds";
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_4)     vp[vn++] = "cause mortal wounds";
	if (l_ptr->spell_flags[1] & RSF1_BO_ACID)     vp[vn++] = "produce acid bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_ELEC)     vp[vn++] = "produce lightning bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_FIRE)     vp[vn++] = "produce fire bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_COLD)     vp[vn++] = "produce frost bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_POIS)     vp[vn++] = "produce poison bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_NETH)     vp[vn++] = "produce nether bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_WATE)     vp[vn++] = "produce water bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_MANA)     vp[vn++] = "produce mana bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_PLAS)     vp[vn++] = "produce plasma bolts";
	if (l_ptr->spell_flags[1] & RSF1_BO_ICEE)     vp[vn++] = "produce ice bolts";
	if (l_ptr->spell_flags[1] & RSF1_MISSILE)     vp[vn++] = "produce magic missiles";
	if (l_ptr->spell_flags[1] & RSF1_SCARE)       vp[vn++] = "terrify";
	if (l_ptr->spell_flags[1] & RSF1_BLIND)       vp[vn++] = "blind";
	if (l_ptr->spell_flags[1] & RSF1_CONF)        vp[vn++] = "confuse";
	if (l_ptr->spell_flags[1] & RSF1_SLOW)        vp[vn++] = "slow";
	if (l_ptr->spell_flags[1] & RSF1_HOLD)        vp[vn++] = "paralyze";
	if (l_ptr->spell_flags[2] & RSF2_HASTE)       vp[vn++] = "haste-self";
	if (l_ptr->spell_flags[2] & RSF2_XXX1)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_HEAL)        vp[vn++] = "heal-self";
	if (l_ptr->spell_flags[2] & RSF2_XXX2)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_BLINK)       vp[vn++] = "blink-self";
	if (l_ptr->spell_flags[2] & RSF2_TPORT)       vp[vn++] = "teleport-self";
	if (l_ptr->spell_flags[2] & RSF2_XXX3)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX4)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_TELE_TO)     vp[vn++] = "teleport to";
	if (l_ptr->spell_flags[2] & RSF2_TELE_AWAY)   vp[vn++] = "teleport away";
	if (l_ptr->spell_flags[2] & RSF2_TELE_LEVEL)  vp[vn++] = "teleport level";
	if (l_ptr->spell_flags[2] & RSF2_XXX5)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_DARKNESS)    vp[vn++] = "create darkness";
	if (l_ptr->spell_flags[2] & RSF2_TRAPS)       vp[vn++] = "create traps";
	if (l_ptr->spell_flags[2] & RSF2_FORGET)      vp[vn++] = "cause amnesia";
	if (l_ptr->spell_flags[2] & RSF2_XXX6)        vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_S_KIN)       vp[vn++] = "summon similar monsters";
	if (l_ptr->spell_flags[2] & RSF2_S_MONSTER)   vp[vn++] = "summon a monster";
	if (l_ptr->spell_flags[2] & RSF2_S_MONSTERS)  vp[vn++] = "summon monsters";
	if (l_ptr->spell_flags[2] & RSF2_S_ANIMAL)    vp[vn++] = "summon animals";
	if (l_ptr->spell_flags[2] & RSF2_S_SPIDER)    vp[vn++] = "summon spiders";
	if (l_ptr->spell_flags[2] & RSF2_S_HOUND)     vp[vn++] = "summon hounds";
	if (l_ptr->spell_flags[2] & RSF2_S_HYDRA)     vp[vn++] = "summon hydras";
	if (l_ptr->spell_flags[2] & RSF2_S_ANGEL)     vp[vn++] = "summon an angel";
	if (l_ptr->spell_flags[2] & RSF2_S_DEMON)     vp[vn++] = "summon a demon";
	if (l_ptr->spell_flags[2] & RSF2_S_UNDEAD)    vp[vn++] = "summon an undead";
	if (l_ptr->spell_flags[2] & RSF2_S_DRAGON)    vp[vn++] = "summon a dragon";
	if (l_ptr->spell_flags[2] & RSF2_S_HI_UNDEAD) vp[vn++] = "summon greater undead";
	if (l_ptr->spell_flags[2] & RSF2_S_HI_DRAGON) vp[vn++] = "summon ancient dragons";
	if (l_ptr->spell_flags[2] & RSF2_S_HI_DEMON)  vp[vn++] = "summon greater demons";
	if (l_ptr->spell_flags[2] & RSF2_S_WRAITH)    vp[vn++] = "summon ringwraiths";
	if (l_ptr->spell_flags[2] & RSF2_S_UNIQUE)    vp[vn++] = "summon uniques";

	/* Describe spells */
	if (vn)
	{
		/* Note magic */
		magic = TRUE;

		/* Intro */
		if (breath)
			text_out(", and is also ");
		else
			text_out("%^s is ", wd_he[msex]);

		/* Verb Phrase */
		text_out_c(TERM_L_RED, "magical");
		text_out(", casting spells");

		/* Adverb */
		if (l_ptr->flags[1] & RF1_SMART) text_out(" intelligently");

		/* List */
		text_out(" which ");
		output_list(vp, -vn, TERM_WHITE);
	}


	/* End the sentence about innate/other spells */
	if (breath || magic)
	{
		/* Total casting */
		m = l_ptr->cast_innate + l_ptr->cast_spell;

		/* Average frequency */
		n = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;

		/* Describe the spell frequency */
		if (m > 100)
		{
			text_out("; ");
			text_out_c(TERM_L_GREEN, "1");
			text_out(" time in ");
			text_out_c(TERM_L_GREEN, "%d", 100 / n);
		}

		/* Guess at the frequency */
		else if (m)
		{
			n = ((n + 9) / 10) * 10;
			text_out("; about ");
			text_out_c(TERM_L_GREEN, "1");
			text_out(" time in ");
			text_out_c(TERM_L_GREEN, "%d", 100 / n);
		}

		/* End this sentence */
		text_out(".  ");
	}
}

/*
 * Describe a monster's drop.
 */
static void describe_monster_drop(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	int n;
	int msex = 0;


	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Drops gold and/or items */
	if (l_ptr->drop_gold || l_ptr->drop_item)
	{
		/* Intro */
		text_out("%^s may carry", wd_he[msex]);

		/* Count maximum drop */
		n = MAX(l_ptr->drop_gold, l_ptr->drop_item);

		/* Count drops */
		if (n == 1) text_out(" a single ");
		else if (n == 2) text_out(" one or two ");
		else text_out(format(" up to %d ", n));


		/* Quality */
		if (l_ptr->flags[0] & RF0_DROP_GREAT) text_out("exceptional ");
		else if (l_ptr->flags[0] & RF0_DROP_GOOD) text_out("good ");


		/* Objects */
		if (l_ptr->drop_item)
		{
			/* Dump "object(s)" */
			text_out("object%s", PLURAL(n));

			/* Add conjunction if also dropping gold */
			if (l_ptr->drop_gold) text_out(" or ");
		}

		/* Treasures */
		if (l_ptr->drop_gold)
		{
			/* Dump "treasure(s)" */
			text_out("treasure%s", PLURAL(n));
		}

		/* End this sentence */
		text_out(".  ");
	}
}

/*
 * Describe all of a monster's attacks.
 */
static void describe_monster_attack(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];
	int m, n, r;

	int msex = 0;

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;


	/* Count the number of "known" attacks */
	for (n = 0, m = 0; m < MONSTER_BLOW_MAX; m++)
	{
		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Count known attacks */
		if (l_ptr->blows[m]) n++;
	}

	/* Examine (and count) the actual attacks */
	for (r = 0, m = 0; m < MONSTER_BLOW_MAX; m++)
	{
		int method, effect, d1, d2;
		const char *p = NULL;
		const char *q = NULL;

		/* Skip unknown and undefined attacks */
		if (!r_ptr->blow[m].method || !l_ptr->blows[m]) continue;


		/* Extract the attack info */
		method = r_ptr->blow[m].method;
		effect = r_ptr->blow[m].effect;
		d1 = r_ptr->blow[m].d_dice;
		d2 = r_ptr->blow[m].d_side;


		/* Get the method */
		switch (method)
		{
			case RBM_HIT:    p = "hit"; break;
			case RBM_TOUCH:  p = "touch"; break;
			case RBM_PUNCH:  p = "punch"; break;
			case RBM_KICK:   p = "kick"; break;
			case RBM_CLAW:   p = "claw"; break;
			case RBM_BITE:   p = "bite"; break;
			case RBM_STING:  p = "sting"; break;
			case RBM_BUTT:   p = "butt"; break;
			case RBM_CRUSH:  p = "crush"; break;
			case RBM_ENGULF: p = "engulf"; break;
			case RBM_CRAWL:  p = "crawl on you"; break;
			case RBM_DROOL:  p = "drool on you"; break;
			case RBM_SPIT:   p = "spit"; break;
			case RBM_GAZE:   p = "gaze"; break;
			case RBM_WAIL:   p = "wail"; break;
			case RBM_SPORE:  p = "release spores"; break;
			case RBM_BEG:    p = "beg"; break;
			case RBM_INSULT: p = "insult"; break;
			case RBM_MOAN:   p = "moan"; break;
			case RBM_XXX1:
			case RBM_XXX2:
			case RBM_XXX3:
			case RBM_XXX4:
			case RBM_XXX5:
			default:		p = "do something weird";
		}


		/* Get the effect */
		switch (effect)
		{
			case RBE_HURT:      q = "attack"; break;
			case RBE_POISON:    q = "poison"; break;
			case RBE_UN_BONUS:  q = "disenchant"; break;
			case RBE_UN_POWER:  q = "drain charges"; break;
			case RBE_EAT_GOLD:  q = "steal gold"; break;
			case RBE_EAT_ITEM:  q = "steal items"; break;
			case RBE_EAT_FOOD:  q = "eat your food"; break;
			case RBE_EAT_LITE:  q = "absorb light"; break;
			case RBE_ACID:      q = "shoot acid"; break;
			case RBE_ELEC:      q = "electrify"; break;
			case RBE_FIRE:      q = "burn"; break;
			case RBE_COLD:      q = "freeze"; break;
			case RBE_BLIND:     q = "blind"; break;
			case RBE_CONFUSE:   q = "confuse"; break;
			case RBE_TERRIFY:   q = "terrify"; break;
			case RBE_PARALYZE:  q = "paralyze"; break;
			case RBE_LOSE_STR:  q = "reduce strength"; break;
			case RBE_LOSE_INT:  q = "reduce intelligence"; break;
			case RBE_LOSE_WIS:  q = "reduce wisdom"; break;
			case RBE_LOSE_DEX:  q = "reduce dexterity"; break;
			case RBE_LOSE_CON:  q = "reduce constitution"; break;
			case RBE_LOSE_CHR:  q = "reduce charisma"; break;
			case RBE_LOSE_ALL:  q = "reduce all stats"; break;
			case RBE_SHATTER:   q = "shatter"; break;
			case RBE_EXP_10:    q = "lower experience"; break;
			case RBE_EXP_20:    q = "lower experience"; break;
			case RBE_EXP_40:    q = "lower experience"; break;
			case RBE_EXP_80:    q = "lower experience"; break;
			case RBE_HALLU:     q = "cause hallucinations"; break;
		}


		/* Introduce the attack description */
		if (!r)
			text_out("%^s can ", wd_he[msex]);
		else if (r < n - 1)
			text_out(", ");
		else
			text_out(", and ");

		/* Describe the method */
		text_out(p);

		/* Describe the effect (if any) */
		if (q)
		{
			/* Describe the attack type */
			text_out(" to %s", q);

			/* Describe damage (if known) */
			if (d1 && d2 && know_damage(r_idx, l_ptr, m))
			{
				text_out(" with damage ");
				text_out_c(TERM_L_GREEN, "%dd%d", d1, d2);
			}
		}

		/* Count the attacks as printed */
		r++;
	}

	/* Finish sentence above */
	if (r)
		text_out(".  ");

	/* Notice lack of attacks */
	else if (l_ptr->flags[0] & RF0_NEVER_BLOW)
		text_out("%^s has no physical attacks.  ", wd_he[msex]);

	/* Or describe the lack of knowledge */
	else
		text_out("Nothing is known about %s attack.  ", wd_his[msex]);
}


/*
 * Describe special abilities of monsters.
 */
static void describe_monster_abilities(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	int vn;
	cptr vp[64];
	bool prev = FALSE;
	
	int msex = 0;


	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;


	/* Collect special abilities. */
	vn = 0;
	if (l_ptr->flags[1] & RF1_OPEN_DOOR) vp[vn++] = "open doors";
	if (l_ptr->flags[1] & RF1_BASH_DOOR) vp[vn++] = "bash down doors";
	if (l_ptr->flags[1] & RF1_PASS_WALL) vp[vn++] = "pass through walls";
	if (l_ptr->flags[1] & RF1_KILL_WALL) vp[vn++] = "bore through walls";
	if (l_ptr->flags[1] & RF1_MOVE_BODY) vp[vn++] = "push past weaker monsters";
	if (l_ptr->flags[1] & RF1_KILL_BODY) vp[vn++] = "destroy weaker monsters";
	if (l_ptr->flags[1] & RF1_TAKE_ITEM) vp[vn++] = "pick up objects";
	if (l_ptr->flags[1] & RF1_KILL_ITEM) vp[vn++] = "destroy objects";

	/* Describe special abilities. */
	output_desc_list(msex, "can", vp, vn, TERM_WHITE);


	/* Describe detection traits */
	vn = 0;
	if (l_ptr->flags[1] & RF1_INVISIBLE)  vp[vn++] = "invisible";
	if (l_ptr->flags[1] & RF1_COLD_BLOOD) vp[vn++] = "cold blooded";
	if (l_ptr->flags[1] & RF1_EMPTY_MIND) vp[vn++] = "not detected by telepathy";
	if (l_ptr->flags[1] & RF1_WEIRD_MIND) vp[vn++] = "rarely detected by telepathy";

	output_desc_list(msex, "is", vp, vn, TERM_WHITE);


	/* Describe special things */
	if (l_ptr->flags[1] & RF1_MULTIPLY)
		text_out("%^s breeds explosively.  ", wd_he[msex]);
	if (l_ptr->flags[1] & RF1_REGENERATE)
		text_out("%^s regenerates quickly.  ", wd_he[msex]);



	/* Collect susceptibilities */
	vn = 0;
	if (l_ptr->flags[2] & RF2_HURT_ROCK) vp[vn++] = "rock remover";
	if (l_ptr->flags[2] & RF2_HURT_LITE) vp[vn++] = "bright light";
	if (l_ptr->flags[2] & RF2_HURT_FIRE) vp[vn++] = "fire";
	if (l_ptr->flags[2] & RF2_HURT_COLD) vp[vn++] = "cold";

	if (vn)
	{
		/* Output connecting text */
		text_out("%^s is hurt by ", wd_he[msex]);
		output_list(vp, vn, TERM_YELLOW);
		prev = TRUE;
	}

	/* Collect immunities and resistances */
	vn = 0;
	if (l_ptr->flags[2] & RF2_IM_ACID)   vp[vn++] = "acid";
	if (l_ptr->flags[2] & RF2_IM_ELEC)   vp[vn++] = "lightning";
	if (l_ptr->flags[2] & RF2_IM_FIRE)   vp[vn++] = "fire";
	if (l_ptr->flags[2] & RF2_IM_COLD)   vp[vn++] = "cold";
	if (l_ptr->flags[2] & RF2_IM_POIS)   vp[vn++] = "poison";
	if (l_ptr->flags[2] & RF2_IM_WATER)  vp[vn++] = "water";
	if (l_ptr->flags[2] & RF2_RES_NETH)  vp[vn++] = "nether";
	if (l_ptr->flags[2] & RF2_RES_PLAS)  vp[vn++] = "plasma";
	if (l_ptr->flags[2] & RF2_RES_NEXUS) vp[vn++] = "nexus";
	if (l_ptr->flags[2] & RF2_RES_DISE)  vp[vn++] = "disenchantment";

	if (vn)
	{
		/* Output connecting text */
		if (prev)
			text_out(", but resists ");
		else
			text_out("%^s resists ", wd_he[msex]);

		/* Write the text */
		output_list(vp, vn, TERM_ORANGE);
		prev = TRUE;
	}

	/* Collect non-effects */
	vn = 0;
	if (l_ptr->flags[2] & RF2_NO_STUN) vp[vn++] = "stunned";
	if (l_ptr->flags[2] & RF2_NO_FEAR) vp[vn++] = "frightened";
	if (l_ptr->flags[2] & RF2_NO_CONF) vp[vn++] = "confused";
	if (l_ptr->flags[2] & RF2_NO_SLEEP) vp[vn++] = "slept";

	if (vn)
	{
		/* Output connecting text */
		if (prev)
			text_out(", and cannot be ");
		else
			text_out("%^s cannot be ", wd_he[msex]);

		output_list(vp, -vn, TERM_ORANGE);
		prev = TRUE;
	}


	/* Full stop. */
	if (prev) text_out(".  ");



	/* Do we know how aware it is? */
	if ((((int)l_ptr->wake * (int)l_ptr->wake) > r_ptr->sleep) ||
	    (l_ptr->ignore == MAX_UCHAR) ||
	    ((r_ptr->sleep == 0) && (l_ptr->tkills >= 10)))
	{
		cptr act;

		if (r_ptr->sleep > 200)     act = "prefers to ignore";
		else if (r_ptr->sleep > 95) act = "pays very little attention to";
		else if (r_ptr->sleep > 75) act = "pays little attention to";
		else if (r_ptr->sleep > 45) act = "tends to overlook";
		else if (r_ptr->sleep > 25) act = "takes quite a while to see";
		else if (r_ptr->sleep > 10) act = "takes a while to see";
		else if (r_ptr->sleep > 5)  act = "is fairly observant of";
		else if (r_ptr->sleep > 3)  act = "is observant of";
		else if (r_ptr->sleep > 1)  act = "is very observant of";
		else if (r_ptr->sleep > 0)  act = "is vigilant for";
		else                        act = "is ever vigilant for";

		text_out("%^s %s intruders, which %s may notice from ", wd_he[msex], act, wd_he[msex]);
		text_out_c(TERM_L_GREEN, "%d", 10 * r_ptr->aaf);
		text_out(" feet.  ");
	}

	/* Describe escorts */
	if ((l_ptr->flags[0] & (RF0_ESCORT | RF0_ESCORTS)))
	{
		text_out("%^s usually appears with escorts.  ", wd_he[msex]);
	}

	/* Describe friends */
	else if ((l_ptr->flags[0] & (RF0_FRIEND | RF0_FRIENDS)))
	{
		text_out("%^s usually appears in groups.  ", wd_he[msex]);
	}
}


/*
 * Describe how often the monster has killed/been killed.
 */
static void describe_monster_kills(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	int msex = 0;

	bool out = TRUE;


	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;


	/* Treat uniques differently */
	if (l_ptr->flags[0] & RF0_UNIQUE)
	{
		/* Hack -- Determine if the unique is "dead" */
		bool dead = (r_ptr->max_num == 0) ? TRUE : FALSE;

		/* We've been killed... */
		if (l_ptr->deaths)
		{
			/* Killed ancestors */
			text_out("%^s has slain %d of your ancestors", wd_he[msex], l_ptr->deaths);


			/* But we've also killed it */
			if (dead)
				text_out(", but you have taken revenge!  ");

			/* Unavenged (ever) */
			else
				text_out(", who remain%s unavenged.  ", PLURAL(l_ptr->deaths));
		}

		/* Dead unique who never hurt us */
		else if (dead)
		{
			text_out("You have slain this foe.  ");
		}
		else
		{
			/* Alive and never killed us */
			out = FALSE;
		}
	}

	/* Not unique, but killed us */
	else if (l_ptr->deaths)
	{
		/* Dead ancestors */
		text_out("%d of your ancestors %s been killed by this creature, ",
		            l_ptr->deaths, plural(l_ptr->deaths, "has", "have"));

		/* Some kills this life */
		if (l_ptr->pkills)
		{
			text_out("and you have exterminated at least %d of the creatures.  ",
			            l_ptr->pkills);
		}

		/* Some kills past lives */
		else if (l_ptr->tkills)
		{
			text_out("and %s have exterminated at least %d of the creatures.  ",
			            "your ancestors", l_ptr->tkills);
		}

		/* No kills */
		else
		{
			text_out_c(TERM_RED, "and %s is not ever known to have been defeated.  ",
			            wd_he[msex]);
		}
	}

	/* Normal monsters */
	else
	{
		/* Killed some this life */
		if (l_ptr->pkills)
		{
			text_out("You have killed at least %d of these creatures.  ",
			            l_ptr->pkills);
		}

		/* Killed some last life */
		else if (l_ptr->tkills)
		{
			text_out("Your ancestors have killed at least %d of these creatures.  ",
			            l_ptr->tkills);
		}

		/* Killed none */
		else
		{
			text_out("No battles to the death are recalled.  ");
		}
	}

	/* Separate */
	if (out) text_out("\n");
}


/*
 * Note how tough a monster is.
 */
static void describe_monster_toughness(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	int msex = 0;

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;
	

	/* Describe monster "toughness" */
	if (know_armour(r_idx, l_ptr))
	{
		/* Armor */
		text_out("%^s has an armor rating of ", wd_he[msex]);
		text_out_c(TERM_L_GREEN, "%d", r_ptr->ac);
		text_out(", and a");

		if (!(l_ptr->flags[0] & RF0_UNIQUE))
			text_out("n average");

		text_out(" life rating of ");
		text_out_c(TERM_L_GREEN, "%d", r_ptr->avg_hp);
		text_out(".  ");
	}
}


static void describe_monster_exp(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	cptr p, q;

	long i, j;

	char buf[20] = "";

	/* Introduction */
	if (l_ptr->flags[0] & RF0_UNIQUE)
		text_out("Killing");
	else
		text_out("A kill of");

	text_out(" this creature");

	/* calculate the integer exp part */
	i = (long)r_ptr->mexp * r_ptr->level / p_ptr->lev;

	/* calculate the fractional exp part scaled by 100, */
	/* must use long arithmetic to avoid overflow */
	j = ((((long)r_ptr->mexp * r_ptr->level % p_ptr->lev) *
		  (long)1000 / p_ptr->lev + 5) / 10);

	/* Calculate textual representation */
	strnfmt(buf, sizeof(buf), "%ld", (long)i);
	if (j) my_strcat(buf, format(".%02ld", (long)j), sizeof(buf));

	/* Mention the experience */
	text_out(" is worth ");
	text_out_c(TERM_ORANGE, buf);
	text_out(" point%s", PLURAL((i == 1) && (j == 0)));

	/* Take account of annoying English */
	p = "th";
	i = p_ptr->lev % 10;
	if ((p_ptr->lev / 10) == 1) /* nothing */;
	else if (i == 1) p = "st";
	else if (i == 2) p = "nd";
	else if (i == 3) p = "rd";

	/* Take account of "leading vowels" in numbers */
	q = "";
	i = p_ptr->lev;
	if ((i == 8) || (i == 11) || (i == 18)) q = "n";

	/* Mention the dependance on the player's level */
	text_out(" for a%s %lu%s level character.  ", q, (long)i, p);
}


static void describe_monster_movement(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	bool old = FALSE;


	text_out("This");

	if (l_ptr->flags[2] & RF2_ANIMAL) text_out(" natural");
	if (l_ptr->flags[2] & RF2_EVIL) text_out(" evil");
	if (l_ptr->flags[2] & RF2_UNDEAD) text_out(" undead");
	if (l_ptr->flags[2] & RF2_METAL) text_out(" metal");

	if (l_ptr->flags[2] & RF2_DRAGON) text_out(" dragon");
	else if (l_ptr->flags[2] & RF2_DEMON) text_out(" demon");
	else if (l_ptr->flags[2] & RF2_GIANT) text_out(" giant");
	else if (l_ptr->flags[2] & RF2_TROLL) text_out(" troll");
	else if (l_ptr->flags[2] & RF2_ORC) text_out(" orc");
	else text_out(" creature");

	/* Describe location */
	if (r_ptr->level == 0)
	{
		text_out(" lives in the town");
		old = TRUE;
	}
	else
	{
		byte colour = (r_ptr->level > p_ptr->max_depth) ? TERM_RED : TERM_L_GREEN;

		if (l_ptr->flags[0] & RF0_FORCE_DEPTH)
			text_out(" is found ");
		else
			text_out(" is normally found ");

		text_out("at depths of ");
		text_out_c(colour, "%d", r_ptr->level * 50);
		text_out(" feet (level ");
		text_out_c(colour, "%d", r_ptr->level);
		text_out(")");

		old = TRUE;
	}

	if (old) text_out(", and");

	text_out(" moves");

	/* Random-ness */
	if ((l_ptr->flags[0] & (RF0_RAND_50 | RF0_RAND_25)))
	{
		/* Adverb */
		if ((l_ptr->flags[0] & RF0_RAND_50) && (l_ptr->flags[0] & RF0_RAND_25))
			text_out(" extremely");
		else if (l_ptr->flags[0] & RF0_RAND_50)
			text_out(" somewhat");
		else if (l_ptr->flags[0] & RF0_RAND_25)
			text_out(" a bit");

		/* Adjective */
		text_out(" erratically");

		/* Hack -- Occasional conjunction */
		if (r_ptr->speed != 110) text_out(", and");
	}

	/* Speed */
	if (r_ptr->speed > 110)
	{
		if (r_ptr->speed > 130) text_out_c(TERM_GREEN, " incredibly");
		else if (r_ptr->speed > 120) text_out_c(TERM_GREEN, " very");
		text_out_c(TERM_GREEN, " quickly");
	}
	else if (r_ptr->speed < 110)
	{
		if (r_ptr->speed < 90) text_out_c(TERM_GREEN, " incredibly");
		else if (r_ptr->speed < 100) text_out_c(TERM_GREEN, " very");
		text_out_c(TERM_GREEN, " slowly");
	}
	else
	{
		text_out(" at ");
		text_out_c(TERM_GREEN, "normal speed");
	}

	/* The code above includes "attack speed" */
	if (l_ptr->flags[0] & RF0_NEVER_MOVE)
		text_out(", but does not deign to chase intruders");

	/* End this sentence */
	text_out(".  ");
}



/*
 * Learn everything about a monster (by cheating)
 */
static void cheat_monster_lore(int r_idx, monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];

	int i;


	/* Hack -- Maximal kills */
	l_ptr->tkills = MAX_SHORT;

	/* Hack -- Maximal info */
	l_ptr->wake = l_ptr->ignore = MAX_UCHAR;

	/* Observe "maximal" attacks */
	for (i = 0; i < MONSTER_BLOW_MAX; i++)
	{
		/* Examine "actual" blows */
		if (r_ptr->blow[i].effect || r_ptr->blow[i].method)
		{
			/* Hack -- maximal observations */
			l_ptr->blows[i] = MAX_UCHAR;
		}
	}

	/* Hack -- maximal drops */
	if (r_ptr->flags[0] & RF0_DROP_4)
		l_ptr->drop_item = 6;
	else if (r_ptr->flags[0] & RF0_DROP_3)
		l_ptr->drop_item = 4;
	else if (r_ptr->flags[0] & RF0_DROP_2)
		l_ptr->drop_item = 3;
	else if (r_ptr->flags[0] & RF0_DROP_1)
		l_ptr->drop_item = 3;

	if (r_ptr->flags[0] & RF0_DROP_40)
		l_ptr->drop_item++;
	if (r_ptr->flags[0] & RF0_DROP_60)
		l_ptr->drop_item++;

	l_ptr->drop_gold = l_ptr->drop_item;

	/* Hack -- but only "valid" drops */
	if (r_ptr->flags[0] & RF0_ONLY_GOLD) l_ptr->drop_item = 0;
	if (r_ptr->flags[0] & RF0_ONLY_ITEM) l_ptr->drop_gold = 0;

	/* Hack -- observe many spells */
	l_ptr->cast_innate = MAX_UCHAR;
	l_ptr->cast_spell = MAX_UCHAR;

	/* Hack -- know all the flags */
	race_flags_assign(l_ptr->flags, r_ptr->flags);
}


/*
 * Hack -- display monster information using "roff()"
 *
 * Note that there is now a compiler option to only read the monster
 * descriptions from the raw file when they are actually needed, which
 * saves about 60K of memory at the cost of disk access during monster
 * recall, which is optional to the user.
 *
 * This function should only be called with the cursor placed at the
 * left edge of the screen, on a cleared line, in which the recall is
 * to take place.  One extra blank line is left after the recall.
 */
void describe_monster(int r_idx, bool spoilers)
{
	monster_lore lore;

	/* Get the race and lore */
	const monster_race *r_ptr = &r_info[r_idx];
	monster_lore *l_ptr = &l_list[r_idx];


	/* Hack -- create a copy of the monster-memory */
	COPY(&lore, l_ptr, monster_lore);

	/* Assume some "obvious" flags */
	lore.flags[0] |= (r_ptr->flags[0] & RF0_OBVIOUS_MASK);


	/* Killing a monster reveals some properties */
	if (lore.tkills)
	{
		/* Know "race" flags */
		lore.flags[2] |= (r_ptr->flags[2] & RF2_RACE_MASK);

		/* Know "forced" flags */
		lore.flags[0] |= (r_ptr->flags[0] & (RF0_FORCE_DEPTH));
	}

	/* Cheat -- know everything */
	if (cheat_know || spoilers) cheat_monster_lore(r_idx, &lore);

	/* Show kills of monster vs. player(s) */
	if (!spoilers) describe_monster_kills(r_idx, &lore);

	/* Monster description */
	describe_monster_desc(r_idx);

	/* Describe the movement and level of the monster */
	describe_monster_movement(r_idx, &lore);

	if (!spoilers) describe_monster_exp(r_idx, &lore);
	describe_monster_spells(r_idx, &lore);
	if (!spoilers) describe_monster_toughness(r_idx, &lore);
	/* Describe the abilities of the monster */
	describe_monster_abilities(r_idx, &lore);
	describe_monster_drop(r_idx, &lore);
	describe_monster_attack(r_idx, &lore);

	/* Notice "Quest" monsters */
	if (lore.flags[0] & RF0_QUESTOR)
		text_out("You feel an intense desire to kill this monster...  ");

	/* All done */
	text_out("\n");
}





/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
void roff_top(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	byte a1, a2;
	char c1, c2;


	/* Get the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Get the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;


	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags[0] & RF0_UNIQUE))
	{
		Term_addstr(-1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr(-1, TERM_WHITE, (r_name + r_ptr->name));

	/* Append the "standard" attr/char info */
	Term_addstr(-1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr(-1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr(-1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	if (use_bigtile && (a2 & 0x80)) Term_addch(255, -1);
	Term_addstr(-1, TERM_WHITE, "'):");
}



/*
 * Hack -- describe the given monster race at the top of the screen
 */
void screen_roff(int r_idx)
{
	/* Flush messages */
	message_flush();

	/* Begin recall */
	Term_erase(0, 1, 255);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Recall monster */
	describe_monster(r_idx, FALSE);

	/* Describe monster */
	roff_top(r_idx);
}




/*
 * Hack -- describe the given monster race in the current "term" window
 */
void display_roff(int r_idx)
{
	int y;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Recall monster */
	describe_monster(r_idx, FALSE);

	/* Describe monster */
	roff_top(r_idx);
}
