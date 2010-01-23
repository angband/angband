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
#include "object/tvalsval.h"
#include "monster/constants.h"


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

static void output_list_dam(const char *list[], int num, int col[], int dam[])
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

		text_out_c(col[i], list[i]);

		if(dam[i])
		{
			text_out_c(col[i], format(" (%d)", dam[i]));
		}
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


/* Struct for coloring attacks in monster memory.
 * Initialized in get_attack_colors()
 * RBE_ flags index the melee array directly
 * RSF0_ bit flags use the get/set_spell_color functions
 */
typedef struct
{
	int melee[RBE_MAX+1];
	int spell[3][32];
} atk_colors;

int get_spell_color(const atk_colors *col, int byte, u32b flag)
{
	int i;

	/* Convert the flag into an index "i" */

	for (i = 0; flag > 1; i++)
		flag >>= 1;

	return col->spell[byte][i];
}

void set_spell_color(atk_colors *col, int byte, u32b flag, int color)
{
	int i;

	/* Convert the flag into an index "i" */

	for (i = 0; flag > 1; i++)
		flag >>= 1;

	col->spell[byte][i] = color;
}

void get_attack_colors(atk_colors *col)
{
	int m, n;
	bool known;
	u32b f[OBJ_FLAG_N];
	player_state st;
	int tmp_col;

	calc_bonuses(inventory, &st, TRUE);

	/* Initialize the colors to green */
	for (m = 0; m < RBE_MAX+1; m++)
		col->melee[m] = TERM_L_GREEN;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 32; n++)
			col->spell[m][n] = TERM_L_GREEN;

	/* Scan the inventory for potentially vulnerable items */
	for (m = 0; m < INVEN_TOTAL; m++)
	{
		object_type *o_ptr = &inventory[m];

		/* Only occupied slots */
		if (!o_ptr->k_idx) continue;

		object_flags_known(o_ptr, f);

		/* Don't reveal the nature of an object.
		 * Assume the player is conservative with unknown items.
		 */
		known = object_is_known(o_ptr);

		/* Drain charges - requires a charged item */
		if (m < INVEN_PACK && (!known || o_ptr->pval > 0) &&
				(o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND))
			col->melee[RBE_UN_POWER] = TERM_L_RED;

		/* Steal item - requires non-artifacts */
		if (m < INVEN_PACK && (!known || !artifact_p(o_ptr)) &&
				p_ptr->lev + adj_dex_safe[st.stat_ind[A_DEX]] < 100)
			col->melee[RBE_EAT_ITEM] = TERM_L_RED;

		/* Eat food - requries food */
		if (m < INVEN_PACK && o_ptr->tval == TV_FOOD)
			col->melee[RBE_EAT_FOOD] = TERM_YELLOW;

		/* Eat light - requires a fuelled light */
		if (m == INVEN_LIGHT && !(f[2] & TR2_NO_FUEL) &&
				o_ptr->timeout > 0)
			col->melee[RBE_EAT_LIGHT] = TERM_YELLOW;

		/* Disenchantment - requires an enchanted item */
		if (m >= INVEN_WIELD && (!known || o_ptr->to_a > 0 ||
				o_ptr->to_h > 0 || o_ptr->to_d > 0) && !st.resist_disen)
		{
			col->melee[RBE_UN_BONUS] = TERM_L_RED;
			set_spell_color(col, 0, RSF0_BR_DISE, TERM_L_RED);
		}
	}

	/* Acid */
	if (st.immune_acid)
		tmp_col = TERM_L_GREEN;
	else if (st.resist_acid)
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	col->melee[RBE_ACID] = tmp_col;
	set_spell_color(col, 0, RSF0_BR_ACID, tmp_col);
	set_spell_color(col, 1, RSF1_BO_ACID, tmp_col);
	set_spell_color(col, 1, RSF1_BA_ACID, tmp_col);

	/* Cold and ice */
	if (st.immune_cold)
		tmp_col = TERM_L_GREEN;
	else if (st.resist_cold)
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	col->melee[RBE_COLD] = tmp_col;
	set_spell_color(col, 0, RSF0_BR_COLD, tmp_col);
	set_spell_color(col, 1, RSF1_BO_COLD, tmp_col);
	set_spell_color(col, 1, RSF1_BA_COLD, tmp_col);
	set_spell_color(col, 1, RSF1_BO_ICEE, tmp_col);

	/* Elec */
	if (st.immune_elec)
		tmp_col = TERM_L_GREEN;
	else if (st.resist_elec)
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	col->melee[RBE_ELEC] = tmp_col;
	set_spell_color(col, 0, RSF0_BR_ELEC, tmp_col);
	set_spell_color(col, 1, RSF1_BO_ELEC, tmp_col);
	set_spell_color(col, 1, RSF1_BA_ELEC, tmp_col);

	/* Fire */
	if (st.immune_fire)
		tmp_col = TERM_L_GREEN;
	else if (st.resist_fire)
		tmp_col = TERM_YELLOW;
	else
		tmp_col = TERM_ORANGE;

	col->melee[RBE_FIRE] = tmp_col;
	set_spell_color(col, 0, RSF0_BR_FIRE, tmp_col);
	set_spell_color(col, 1, RSF1_BO_FIRE, tmp_col);
	set_spell_color(col, 1, RSF1_BA_FIRE, tmp_col);

	/* Poison */
	if (!st.resist_pois)
	{
		col->melee[RBE_POISON] = TERM_ORANGE;
		set_spell_color(col, 0, RSF0_BR_POIS, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BA_POIS, TERM_ORANGE);
	}

	/* Nexus  */
	if (!st.resist_nexus)
	{
		if(st.skills[SKILL_SAVE] < 100)
			set_spell_color(col, 0, RSF0_BR_NEXU, TERM_L_RED);
		else
			set_spell_color(col, 0, RSF0_BR_NEXU, TERM_YELLOW);
	}

	/* Nether */
	if (!st.resist_nethr)
	{
		set_spell_color(col, 0, RSF0_BR_NETH, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BA_NETH, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BO_NETH, TERM_ORANGE);
	}

	/* Inertia, gravity, and time */
	set_spell_color(col, 0, RSF0_BR_INER, TERM_ORANGE);
	set_spell_color(col, 0, RSF0_BR_GRAV, TERM_L_RED);
	set_spell_color(col, 0, RSF0_BR_TIME, TERM_L_RED);

	/* Sound, force, and plasma */
	if (!st.resist_sound)
	{
		set_spell_color(col, 0, RSF0_BR_SOUN, TERM_ORANGE);
		set_spell_color(col, 0, RSF0_BR_WALL, TERM_YELLOW);

		set_spell_color(col, 0, RSF0_BR_PLAS, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BO_PLAS, TERM_ORANGE);
	}
	else
	{
		set_spell_color(col, 0, RSF0_BR_PLAS, TERM_YELLOW);
		set_spell_color(col, 1, RSF1_BO_PLAS, TERM_YELLOW);
	}

 	/* Shards */
 	if(!st.resist_shard)
 		set_spell_color(col, 0, RSF0_BR_SHAR, TERM_ORANGE);

	/* Confusion */
	if (!st.resist_confu)
	{
		col->melee[RBE_CONFUSE] = TERM_ORANGE;
		set_spell_color(col, 0, RSF0_BR_CONF, TERM_ORANGE);
	}

	/* Chaos */
	if (!st.resist_chaos)
		set_spell_color(col, 0, RSF0_BR_CHAO, TERM_ORANGE);

	/* Light */
	if (!st.resist_light)
		set_spell_color(col, 0, RSF0_BR_LIGHT, TERM_ORANGE);

	/* Darkness */
	if (!st.resist_dark)
	{
		set_spell_color(col, 0, RSF0_BR_DARK, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BA_DARK, TERM_L_RED);
	}

	/* Water */
	if (!st.resist_confu || !st.resist_sound)
	{
		set_spell_color(col, 1, RSF1_BA_WATE, TERM_L_RED);
		set_spell_color(col, 1, RSF1_BO_WATE, TERM_L_RED);
	}
	else
	{
		set_spell_color(col, 1, RSF1_BA_WATE, TERM_ORANGE);
		set_spell_color(col, 1, RSF1_BO_WATE, TERM_ORANGE);
	}

	/* Mana */
	set_spell_color(col, 0, RSF0_BR_MANA, TERM_L_RED);
	set_spell_color(col, 1, RSF1_BA_MANA, TERM_L_RED);
	set_spell_color(col, 1, RSF1_BO_MANA, TERM_L_RED);

	/* These attacks only apply without a perfect save */
	if (st.skills[SKILL_SAVE] < 100)
	{
		/* Amnesia */
		set_spell_color(col, 2, RSF2_FORGET, TERM_YELLOW);

		/* Fear */
		if (!st.resist_fear)
		{
			col->melee[RBE_TERRIFY] = TERM_YELLOW;
			set_spell_color(col, 1, RSF1_SCARE, TERM_YELLOW);
		}

		/* Paralysis and slow */
		if (!st.free_act)
		{
			col->melee[RBE_PARALYZE] = TERM_L_RED;
			set_spell_color(col, 1, RSF1_HOLD, TERM_L_RED);
			set_spell_color(col, 1, RSF1_SLOW, TERM_ORANGE);
		}

		/* Blind */
		if (!st.resist_blind)
			set_spell_color(col, 1, RSF1_BLIND, TERM_ORANGE);

		/* Confusion */
		if (!st.resist_confu)
			set_spell_color(col, 1, RSF1_CONF, TERM_ORANGE);

		/* Cause wounds */
		set_spell_color(col, 1, RSF1_CAUSE_1, TERM_YELLOW);
		set_spell_color(col, 1, RSF1_CAUSE_2, TERM_YELLOW);
		set_spell_color(col, 1, RSF1_CAUSE_3, TERM_YELLOW);
		set_spell_color(col, 1, RSF1_CAUSE_4, TERM_YELLOW);

		/* Mind blast */
		set_spell_color(col, 1, RSF1_MIND_BLAST,
			(st.resist_confu ? TERM_YELLOW : TERM_ORANGE));

		/* Brain smash slows even when conf/blind resisted */
		set_spell_color(col, 1, RSF1_BRAIN_SMASH, (st.resist_blind &&
			st.free_act && st.resist_confu ? TERM_ORANGE : TERM_L_RED));
	}

	/* Gold theft */
	if (p_ptr->lev + adj_dex_safe[st.stat_ind[A_DEX]] < 100 && p_ptr->au)
		col->melee[RBE_EAT_GOLD] = TERM_YELLOW;

	/* Melee blindness and hallucinations */
	if (!st.resist_blind)
		col->melee[RBE_BLIND] = TERM_YELLOW;
	if (!st.resist_chaos)
		col->melee[RBE_HALLU] = TERM_YELLOW;

	/* Stat draining is bad */
	if (!st.sustain_str) col->melee[RBE_LOSE_STR] = TERM_ORANGE;
	if (!st.sustain_int) col->melee[RBE_LOSE_INT] = TERM_ORANGE;
	if (!st.sustain_wis) col->melee[RBE_LOSE_WIS] = TERM_ORANGE;
	if (!st.sustain_dex) col->melee[RBE_LOSE_DEX] = TERM_ORANGE;
	if (!st.sustain_con) col->melee[RBE_LOSE_CON] = TERM_ORANGE;
	if (!st.sustain_chr) col->melee[RBE_LOSE_CHR] = TERM_ORANGE;

	/* Drain all gets a red warning */
	if (!st.sustain_str || !st.sustain_int || !st.sustain_wis ||
			!st.sustain_dex || !st.sustain_con || !st.sustain_chr)
		col->melee[RBE_LOSE_ALL] = TERM_L_RED;

	/* Hold life isn't 100% effective */
	col->melee[RBE_EXP_10] = col->melee[RBE_EXP_20] =
		col->melee[RBE_EXP_40] = col->melee[RBE_EXP_80] =
		st.hold_life ? TERM_YELLOW : TERM_ORANGE;

	/* Shatter is always noteworthy */
	col->melee[RBE_SHATTER] = TERM_YELLOW;

	/* Heal (and drain mana) and haste are always noteworthy */
	set_spell_color(col, 2, RSF2_HEAL, TERM_YELLOW);
	set_spell_color(col, 1, RSF1_DRAIN_MANA, TERM_YELLOW);
	set_spell_color(col, 2, RSF2_HASTE, TERM_YELLOW);

	/* Player teleports and traps are annoying */
	set_spell_color(col, 2, RSF2_TELE_TO, TERM_YELLOW);
	set_spell_color(col, 2, RSF2_TELE_AWAY, TERM_YELLOW);
	if (!st.resist_nexus && st.skills[SKILL_SAVE] < 100)
		set_spell_color(col, 2, RSF2_TELE_LEVEL, TERM_YELLOW);
	set_spell_color(col, 2, RSF2_TRAPS, TERM_YELLOW);

	/* Summons are potentially dangerous */
	set_spell_color(col, 2, RSF2_S_MONSTER, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_MONSTERS, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_KIN, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_ANIMAL, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_SPIDER, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_HOUND, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_HYDRA, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_ANGEL, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_DEMON, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_DRAGON, TERM_ORANGE);
	set_spell_color(col, 2, RSF2_S_UNDEAD, TERM_ORANGE);

	/* High level summons are very dangerous */
	set_spell_color(col, 2, RSF2_S_HI_DEMON, TERM_L_RED);
	set_spell_color(col, 2, RSF2_S_HI_DRAGON, TERM_L_RED);
	set_spell_color(col, 2, RSF2_S_HI_UNDEAD, TERM_L_RED);
	set_spell_color(col, 2, RSF2_S_UNIQUE, TERM_L_RED);
	set_spell_color(col, 2, RSF2_S_WRAITH, TERM_L_RED);

	/* Shrieking can lead to bad combos */
	set_spell_color(col, 0, RSF0_SHRIEK, TERM_ORANGE);

	/* Ranged attacks can't be resisted (only mitigated by accuracy)
	 * They are colored yellow to indicate the damage is a hard value
	 */
	set_spell_color(col, 0, RSF0_ARROW_1, TERM_YELLOW);
	set_spell_color(col, 0, RSF0_ARROW_2, TERM_YELLOW);
	set_spell_color(col, 0, RSF0_ARROW_3, TERM_YELLOW);
	set_spell_color(col, 0, RSF0_ARROW_4, TERM_YELLOW);
	set_spell_color(col, 0, RSF0_BOULDER, TERM_YELLOW);
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
	if ((4 + level) * a >= 80 * d) return (TRUE);

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


static void describe_monster_spells(int r_idx, const monster_lore *l_ptr, const atk_colors *colors)
{
	const monster_race *r_ptr = &r_info[r_idx];
	u32b f[RACE_FLAG_STRICT_UB];
	int m, n;
	int msex = 0;
	bool breath = FALSE;
	bool magic = FALSE;
	int vn; /* list size */
	cptr vp[64]; /* list item names */
	int vc[64]; /* list colors */
	int vd[64]; /* list avg damage values */
	int known_hp;
	
	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Collect innate attacks */
	vn = 0;
	for(m = 0; m < 64; m++) { vd[m] = 0; vc[m] = TERM_WHITE; }

	if (l_ptr->spell_flags[0] & RSF0_SHRIEK)
	{
		vp[vn] = "shriek for help";
		vc[vn++] = get_spell_color(colors, 0, RSF0_SHRIEK);
	}
	if (l_ptr->spell_flags[0] & RSF0_ARROW_1)
	{
		vp[vn] = "fire an arrow";
		vc[vn] = get_spell_color(colors, 0, RSF0_ARROW_1);
		vd[vn++] = ARROW1_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[0] & RSF0_ARROW_2)
	{
		vp[vn] = "fire arrows";
		vc[vn] = get_spell_color(colors, 0, RSF0_ARROW_2);
		vd[vn++] = ARROW2_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[0] & RSF0_ARROW_3)
	{
		vp[vn] = "fire a missile";
		vc[vn] = get_spell_color(colors, 0, RSF0_ARROW_3);
		vd[vn++] = ARROW3_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[0] & RSF0_ARROW_4)
	{
		vp[vn] = "fire missiles";
		vc[vn] = get_spell_color(colors, 0, RSF0_ARROW_4);
		vd[vn++] = ARROW4_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[0] & RSF0_BOULDER)
	{
		vp[vn] = "throw boulders";
		vc[vn] = get_spell_color(colors, 0, RSF0_BOULDER);
		vd[vn++] = BOULDER_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[0] & RSF0_XXX2) vp[vn++] = "do something";
	if (l_ptr->spell_flags[0] & RSF0_XXX3) vp[vn++] = "do something";
	if (l_ptr->spell_flags[0] & RSF0_XXX4) vp[vn++] = "do something";

	/* Describe innate attacks */
	if(vn)
	{
		text_out("%^s may ", wd_he[msex]);
		output_list_dam(vp, -vn, vc, vd);
		text_out(".  ");
	}

	/* Collect breaths */
	vn = 0;
	for(m = 0; m < 64; m++) { vd[m] = 0; vc[m] = TERM_WHITE; }

	known_hp = know_armour(r_idx, l_ptr) ? r_ptr->avg_hp : 0;

	if (l_ptr->spell_flags[0] & RSF0_BR_ACID)
	{
		vp[vn] = "acid";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_ACID);
		vd[vn++] = MIN(known_hp / BR_ACID_DIVISOR, BR_ACID_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_ELEC)
	{
		vp[vn] = "lightning";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_ELEC);
		vd[vn++] = MIN(known_hp / BR_ELEC_DIVISOR, BR_ELEC_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_FIRE)
	{
		vp[vn] = "fire";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_FIRE);
		vd[vn++] = MIN(known_hp / BR_FIRE_DIVISOR, BR_FIRE_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_COLD)
	{
		vp[vn] = "frost";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_COLD);
		vd[vn++] = MIN(known_hp / BR_COLD_DIVISOR, BR_COLD_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_POIS)
	{
		vp[vn] = "poison";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_POIS);
		vd[vn++] = MIN(known_hp / BR_POIS_DIVISOR, BR_POIS_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_NETH)
	{
		vp[vn] = "nether";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_NETH);
		vd[vn++] = MIN(known_hp / BR_NETH_DIVISOR, BR_NETH_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_LIGHT)
	{
		vp[vn] = "light";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_LIGHT);
		vd[vn++] = MIN(known_hp / BR_LIGHT_DIVISOR, BR_LIGHT_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_DARK)
	{
		vp[vn] = "darkness";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_DARK);
		vd[vn++] = MIN(known_hp / BR_DARK_DIVISOR, BR_DARK_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_CONF)
	{
		vp[vn] = "confusion";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_CONF);
		vd[vn++] = MIN(known_hp / BR_CONF_DIVISOR, BR_CONF_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_SOUN)
	{
		vp[vn] = "sound";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_SOUN);
		vd[vn++] = MIN(known_hp / BR_SOUN_DIVISOR, BR_SOUN_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_CHAO)
	{
		vp[vn] = "chaos";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_CHAO);
		vd[vn++] = MIN(known_hp / BR_CHAO_DIVISOR, BR_CHAO_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_DISE)
	{
		vp[vn] = "disenchantment";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_DISE);
		vd[vn++] = MIN(known_hp / BR_DISE_DIVISOR, BR_DISE_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_NEXU)
	{
		vp[vn] = "nexus";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_NEXU);
		vd[vn++] = MIN(known_hp / BR_NEXU_DIVISOR, BR_NEXU_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_TIME)
	{
		vp[vn] = "time";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_TIME);
		vd[vn++] = MIN(known_hp / BR_TIME_DIVISOR, BR_TIME_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_INER)
	{
		vp[vn] = "inertia";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_INER);
		vd[vn++] = MIN(known_hp / BR_INER_DIVISOR, BR_INER_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_GRAV)
	{
		vp[vn] = "gravity";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_GRAV);
		vd[vn++] = MIN(known_hp / BR_GRAV_DIVISOR, BR_GRAV_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_SHAR)
	{
		vp[vn] = "shards";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_SHAR);
		vd[vn++] = MIN(known_hp / BR_SHAR_DIVISOR, BR_SHAR_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_PLAS)
	{
		vp[vn] = "plasma";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_PLAS);
		vd[vn++] = MIN(known_hp / BR_PLAS_DIVISOR, BR_PLAS_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_WALL)
	{
		vp[vn] = "force";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_WALL);
		vd[vn++] = MIN(known_hp / BR_FORC_DIVISOR, BR_FORC_MAX);
	}
	if (l_ptr->spell_flags[0] & RSF0_BR_MANA)
	{
		vp[vn] = "mana";
		vc[vn] = get_spell_color(colors, 0, RSF0_BR_MANA);
		vd[vn++] = 0;
	}
	if (l_ptr->spell_flags[0] & RSF0_XXX5) vp[vn++] = "something";
	if (l_ptr->spell_flags[0] & RSF0_XXX6) vp[vn++] = "something";
	if (l_ptr->spell_flags[0] & RSF0_XXX7) vp[vn++] = "something";

	/* Describe breaths */
	if (vn)
	{
		/* Note breath */
		breath = TRUE;

		/* Display */
		text_out("%^s may ", wd_he[msex]);
		text_out_c(TERM_L_RED, "breathe ");
		output_list_dam(vp, -vn, vc, vd);
	}


	/* Collect spell information */
	vn = 0;
	for(m = 0; m < 64; m++) { vd[m] = 0; vc[m] = TERM_WHITE; }

	/* Ball spells */
	if (l_ptr->spell_flags[1] & RSF1_BA_MANA)
	{
		vp[vn] = "invoke mana storms";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_MANA);
		vd[vn++] = BA_MANA_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_DARK)
	{
		vp[vn] = "invoke darkness storms";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_DARK);
		vd[vn++] = BA_DARK_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_WATE)
	{
		vp[vn] = "produce water balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_WATE);
		vd[vn++] = BA_WATE_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_NETH)
	{
		vp[vn] = "produce nether balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_NETH);
		vd[vn++] = BA_NETH_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_FIRE)
	{
		vp[vn] = "produce fire balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_FIRE);
		vd[vn++] = BA_FIRE_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_ACID)
	{
		vp[vn] = "produce acid balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_ACID);
		vd[vn++] = BA_ACID_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_COLD)
	{
		vp[vn] = "produce frost balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_COLD);
		vd[vn++] = BA_COLD_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_ELEC)
	{
		vp[vn] = "produce lightning balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_ELEC);
		vd[vn++] = BA_ELEC_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BA_POIS)
	{
		vp[vn] = "produce poison balls";
		vc[vn] = get_spell_color(colors, 1, RSF1_BA_POIS);
		vd[vn++] = BA_POIS_DMG(r_ptr->level, AVERAGE);
	}

	/* Bolt spells */
	if (l_ptr->spell_flags[1] & RSF1_BO_MANA)
	{
		vp[vn] = "produce mana bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_MANA);
		vd[vn++] = BO_MANA_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_PLAS)
	{
		vp[vn] = "produce plasma bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_PLAS);
		vd[vn++] = BO_PLAS_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_ICEE)
	{
		vp[vn] = "produce ice bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_ICEE);
		vd[vn++] = BO_ICEE_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_WATE)
	{
		vp[vn] = "produce water bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_WATE);
		vd[vn++] = BO_WATE_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_NETH)
	{
		vp[vn] = "produce nether bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_NETH);
		vd[vn++] = BO_NETH_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_FIRE)
	{
		vp[vn] = "produce fire bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_FIRE);
		vd[vn++] = BO_FIRE_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_ACID)
	{
		vp[vn] = "produce acid bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_ACID);
		vd[vn++] = BO_ACID_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_COLD)
	{
		vp[vn] = "produce frost bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_COLD);
		vd[vn++] = BO_COLD_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_ELEC)
	{
		vp[vn] = "produce lightning bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_ELEC);
		vd[vn++] = BO_ELEC_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BO_POIS)
	{
		vp[vn] = "produce poison bolts";
		vc[vn] = get_spell_color(colors, 1, RSF1_BO_POIS);
		vd[vn++] = 0;
	}
	if (l_ptr->spell_flags[1] & RSF1_MISSILE)
	{
		vp[vn] = "produce magic missiles";
		vc[vn] = get_spell_color(colors, 1, RSF1_MISSILE);
		vd[vn++] = MISSILE_DMG(r_ptr->level, AVERAGE);
	}

	/* Curses */
	if (l_ptr->spell_flags[1] & RSF1_BRAIN_SMASH)
	{
		vp[vn] = "cause brain smashing";
		vc[vn] = get_spell_color(colors, 1, RSF1_BRAIN_SMASH);
		vd[vn++] = BRAIN_SMASH_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_MIND_BLAST)
	{
		vp[vn] = "cause mind blasting";
		vc[vn] = get_spell_color(colors, 1, RSF1_MIND_BLAST);
		vd[vn++] = MIND_BLAST_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_4)
	{
		vp[vn] = "cause mortal wounds";
		vc[vn] = get_spell_color(colors, 1, RSF1_CAUSE_4);
		vd[vn++] = CAUSE4_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_3)
	{
		vp[vn] = "cause critical wounds";
		vc[vn] = get_spell_color(colors, 1, RSF1_CAUSE_3);
		vd[vn++] = CAUSE3_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_2)
	{
		vp[vn] = "cause serious wounds";
		vc[vn] = get_spell_color(colors, 1, RSF1_CAUSE_2);
		vd[vn++] = CAUSE2_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[1] & RSF1_CAUSE_1)
	{
		vp[vn] = "cause light wounds";
		vc[vn] = get_spell_color(colors, 1, RSF1_CAUSE_1);
		vd[vn++] = CAUSE1_DMG(r_ptr->level, AVERAGE);
	}
	if (l_ptr->spell_flags[2] & RSF2_FORGET)
	{
		vp[vn] = "cause amnesia";
		vc[vn++] = get_spell_color(colors, 2, RSF2_FORGET);
	}
	if (l_ptr->spell_flags[1] & RSF1_SCARE)
	{
		vp[vn] = "terrify";
		vc[vn++] = get_spell_color(colors, 1, RSF1_SCARE);
	}
	if (l_ptr->spell_flags[1] & RSF1_BLIND)
	{
		vp[vn] = "blind";
		vc[vn++] = get_spell_color(colors, 1, RSF1_BLIND);
	}
	if (l_ptr->spell_flags[1] & RSF1_CONF)
	{
		vp[vn] = "confuse";
		vc[vn++] = get_spell_color(colors, 1, RSF1_CONF);
	}
	if (l_ptr->spell_flags[1] & RSF1_SLOW)
	{
		vp[vn] = "slow";
		vc[vn++] = get_spell_color(colors, 1, RSF1_SLOW);
	}
	if (l_ptr->spell_flags[1] & RSF1_HOLD)
	{
		vp[vn] = "paralyze";
		vc[vn++] = get_spell_color(colors, 1, RSF1_HOLD);
	}

	/* Healing and haste */
	if (l_ptr->spell_flags[1] & RSF1_DRAIN_MANA)
	{
		vp[vn] = "drain mana";
		vc[vn++] = get_spell_color(colors, 1, RSF1_DRAIN_MANA);
	}
	if (l_ptr->spell_flags[2] & RSF2_HEAL)
	{
		vp[vn] = "heal-self";
		vc[vn++] = get_spell_color(colors, 2, RSF2_HEAL);
	}
	if (l_ptr->spell_flags[2] & RSF2_HASTE)
	{
		vp[vn] = "haste-self";
		vc[vn++] = get_spell_color(colors, 2, RSF2_HASTE);
	}

	/* Teleports */
	if (l_ptr->spell_flags[2] & RSF2_BLINK)
	{
		vp[vn] = "blink-self";
		vc[vn++] = get_spell_color(colors, 2, RSF2_BLINK);
	}
	if (l_ptr->spell_flags[2] & RSF2_TPORT)
	{
		vp[vn] = "teleport-self";
		vc[vn++] = get_spell_color(colors, 2, RSF2_TPORT);
	}
	if (l_ptr->spell_flags[2] & RSF2_TELE_TO)
	{
		vp[vn] = "teleport to";
		vc[vn++] = get_spell_color(colors, 2, RSF2_TELE_TO);
	}
	if (l_ptr->spell_flags[2] & RSF2_TELE_AWAY)
	{
		vp[vn] = "teleport away";
		vc[vn++] = get_spell_color(colors, 2, RSF2_TELE_AWAY);
	}
	if (l_ptr->spell_flags[2] & RSF2_TELE_LEVEL)
	{
		vp[vn] = "teleport level";
		vc[vn++] = get_spell_color(colors, 2, RSF2_TELE_LEVEL);
	}

	/* Annoyances */
	if (l_ptr->spell_flags[2] & RSF2_DARKNESS)
	{
		vp[vn] = "create darkness";
		vc[vn++] = get_spell_color(colors, 2, RSF2_DARKNESS);
	}
	if (l_ptr->spell_flags[2] & RSF2_TRAPS)
	{
		vp[vn] = "create traps";
		vc[vn++] = get_spell_color(colors, 2, RSF2_TRAPS);
	}

	/* Summoning */
	if (l_ptr->spell_flags[2] & RSF2_S_KIN)
	{
		vp[vn] = "summon similar monsters";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_KIN);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_MONSTER)
	{
		vp[vn] = "summon a monster";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_MONSTER);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_MONSTERS)
	{
		vp[vn] = "summon monsters";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_MONSTERS);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_ANIMAL)
	{
		vp[vn] = "summon animals";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_ANIMAL);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_SPIDER)
	{
		vp[vn] = "summon spiders";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_SPIDER);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_HOUND)
	{
		vp[vn] = "summon hounds";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_HOUND);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_HYDRA)
	{
		vp[vn] = "summon hydras";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_HYDRA);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_ANGEL)
	{
		vp[vn] = "summon an angel";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_ANGEL);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_DEMON)
	{
		vp[vn] = "summon a demon";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_DEMON);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_UNDEAD)
	{
		vp[vn] = "summon an undead";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_UNDEAD);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_DRAGON)
	{
		vp[vn] = "summon a dragon";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_DRAGON);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_HI_UNDEAD)
	{
		vp[vn] = "summon greater undead";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_HI_UNDEAD);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_HI_DRAGON)
	{
		vp[vn] = "summon ancient dragons";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_HI_DRAGON);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_HI_DEMON)
	{
		vp[vn] = "summon greater demons";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_HI_DEMON);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_WRAITH)
	{
		vp[vn] = "summon ringwraiths";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_WRAITH);
	}
	if (l_ptr->spell_flags[2] & RSF2_S_UNIQUE)
	{
		vp[vn] = "summon uniques";
		vc[vn++] = get_spell_color(colors, 2, RSF2_S_UNIQUE);
	}

	/* For expansion */
	if (l_ptr->spell_flags[2] & RSF2_XXX1) vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX2) vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX3) vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX4) vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX5) vp[vn++] = "do something";
	if (l_ptr->spell_flags[2] & RSF2_XXX6) vp[vn++] = "do something";

	/* Describe spells */
	if (vn)
	{
		/* Note magic */
		magic = TRUE;

		/* Intro */
		if (breath)
			text_out(", and may ");
		else
			text_out("%^s may ", wd_he[msex]);

		/* Verb Phrase */
		text_out_c(TERM_L_RED, "cast spells");

		/* Adverb */
		if (f[1] & RF1_SMART) text_out(" intelligently");

		/* List */
		text_out(" which ");
		output_list_dam(vp, -vn, vc, vd);
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
	u32b f[RACE_FLAG_STRICT_UB];

	int n;
	int msex = 0;

	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

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
		if (n == 1) text_out_c(TERM_BLUE, " a single ");
		else if (n == 2) text_out_c(TERM_BLUE, " one or two ");
		else
		{
			text_out(" up to ");
			text_out_c(TERM_BLUE, format("%d ", n));
		}

		/* Quality */
		if (f[0] & RF0_DROP_GREAT) text_out_c(TERM_BLUE, "exceptional ");
		else if (f[0] & RF0_DROP_GOOD) text_out_c(TERM_BLUE, "good ");

		/* Objects */
		if (l_ptr->drop_item)
		{
			/* Dump "object(s)" */
			text_out_c(TERM_BLUE, "object%s", PLURAL(n));

			/* Add conjunction if also dropping gold */
			if (l_ptr->drop_gold) text_out_c(TERM_BLUE, " or ");
		}

		/* Treasures */
		if (l_ptr->drop_gold)
		{
			/* Dump "treasure(s)" */
			text_out_c(TERM_BLUE, "treasure%s", PLURAL(n));
		}

		/* End this sentence */
		text_out(".  ");
	}
}

/*
 * Describe all of a monster's attacks.
 */
static void describe_monster_attack(int r_idx, const monster_lore *l_ptr, const atk_colors *colors)
{
	const monster_race *r_ptr = &r_info[r_idx];
	u32b f[RACE_FLAG_STRICT_UB];
	int m, n, r;
	int msex = 0;
	
	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);
	
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
			case RBE_EAT_LIGHT:  q = "absorb light"; break;
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
			text_out(" to ");
			text_out_c(colors->melee[effect], "%s", q);

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
	else if (f[0] & RF0_NEVER_BLOW)
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
	u32b f[RACE_FLAG_STRICT_UB];

	int vn;
	cptr vp[64];
	bool prev = FALSE;

	int msex = 0;
	
	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Collect special abilities. */
	vn = 0;
	if (f[1] & RF1_OPEN_DOOR) vp[vn++] = "open doors";
	if (f[1] & RF1_BASH_DOOR) vp[vn++] = "bash down doors";
	if (f[1] & RF1_PASS_WALL) vp[vn++] = "pass through walls";
	if (f[1] & RF1_KILL_WALL) vp[vn++] = "bore through walls";
	if (f[1] & RF1_MOVE_BODY) vp[vn++] = "push past weaker monsters";
	if (f[1] & RF1_KILL_BODY) vp[vn++] = "destroy weaker monsters";
	if (f[1] & RF1_TAKE_ITEM) vp[vn++] = "pick up objects";
	if (f[1] & RF1_KILL_ITEM) vp[vn++] = "destroy objects";

	/* Describe special abilities. */
	output_desc_list(msex, "can", vp, vn, TERM_WHITE);


	/* Describe detection traits */
	vn = 0;
	if (f[1] & RF1_INVISIBLE)  vp[vn++] = "invisible";
	if (f[1] & RF1_COLD_BLOOD) vp[vn++] = "cold blooded";
	if (f[1] & RF1_EMPTY_MIND) vp[vn++] = "not detected by telepathy";
	if (f[1] & RF1_WEIRD_MIND) vp[vn++] = "rarely detected by telepathy";

	output_desc_list(msex, "is", vp, vn, TERM_WHITE);


	/* Describe special things */
	if (f[1] & RF1_MULTIPLY)
		text_out("%^s breeds explosively.  ", wd_he[msex]);
	if (f[1] & RF1_REGENERATE)
		text_out("%^s regenerates quickly.  ", wd_he[msex]);



	/* Collect susceptibilities */
	vn = 0;
	if (f[2] & RF2_HURT_ROCK) vp[vn++] = "rock remover";
	if (f[2] & RF2_HURT_LIGHT) vp[vn++] = "bright light";
	if (f[2] & RF2_HURT_FIRE) vp[vn++] = "fire";
	if (f[2] & RF2_HURT_COLD) vp[vn++] = "cold";

	if (vn)
	{
		/* Output connecting text */
		text_out("%^s is hurt by ", wd_he[msex]);
		output_list(vp, vn, TERM_VIOLET);
		prev = TRUE;
	}

	/* Collect immunities and resistances */
	vn = 0;
	if (f[2] & RF2_IM_ACID)   vp[vn++] = "acid";
	if (f[2] & RF2_IM_ELEC)   vp[vn++] = "lightning";
	if (f[2] & RF2_IM_FIRE)   vp[vn++] = "fire";
	if (f[2] & RF2_IM_COLD)   vp[vn++] = "cold";
	if (f[2] & RF2_IM_POIS)   vp[vn++] = "poison";
	if (f[2] & RF2_IM_WATER)  vp[vn++] = "water";
	if (f[2] & RF2_RES_NETH)  vp[vn++] = "nether";
	if (f[2] & RF2_RES_PLAS)  vp[vn++] = "plasma";
	if (f[2] & RF2_RES_NEXUS) vp[vn++] = "nexus";
	if (f[2] & RF2_RES_DISE)  vp[vn++] = "disenchantment";
	/* Note lack of vulnerability as a resistance */
	if ((l_ptr->flags[2] & RF2_HURT_LIGHT) && !(f[2] & RF2_HURT_LIGHT)) vp[vn++] = "bright light";
	if ((l_ptr->flags[2] & RF2_HURT_ROCK) && !(f[2] & RF2_HURT_ROCK)) vp[vn++] = "rock remover";

	if (vn)
	{
		/* Output connecting text */
		if (prev)
			text_out(", but resists ");
		else
			text_out("%^s resists ", wd_he[msex]);

		/* Write the text */
		output_list(vp, vn, TERM_L_UMBER);
		prev = TRUE;
	}

	/* Collect non-effects */
	vn = 0;
	if (f[2] & RF2_NO_STUN) vp[vn++] = "stunned";
	if (f[2] & RF2_NO_FEAR) vp[vn++] = "frightened";
	if (f[2] & RF2_NO_CONF) vp[vn++] = "confused";
	if (f[2] & RF2_NO_SLEEP) vp[vn++] = "slept";

	if (vn)
	{
		/* Output connecting text */
		if (prev)
			text_out(", and cannot be ");
		else
			text_out("%^s cannot be ", wd_he[msex]);

		output_list(vp, -vn, TERM_L_UMBER);
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
		text_out_c(TERM_L_BLUE, "%d", 10 * r_ptr->aaf);
		text_out(" feet.  ");
	}

	/* Describe escorts */
	if ((f[0] & (RF0_ESCORT | RF0_ESCORTS)))
	{
		text_out("%^s usually appears with escorts.  ", wd_he[msex]);
	}

	/* Describe friends */
	else if ((f[0] & (RF0_FRIEND | RF0_FRIENDS)))
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
	u32b f[RACE_FLAG_STRICT_UB];

	int msex = 0;

	bool out = TRUE;
	
	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Treat uniques differently */
	if (f[0] & RF0_UNIQUE)
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
	u32b f[RACE_FLAG_STRICT_UB];

	int msex = 0;

	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	/* Extract a gender (if applicable) */
	if (r_ptr->flags[0] & RF0_FEMALE) msex = 2;
	else if (r_ptr->flags[0] & RF0_MALE) msex = 1;

	/* Describe monster "toughness" */
	if (know_armour(r_idx, l_ptr))
	{
		/* Armor */
		text_out("%^s has an armor rating of ", wd_he[msex]);
		text_out_c(TERM_L_BLUE, "%d", r_ptr->ac);
		text_out(", and a");

		if (!(f[0] & RF0_UNIQUE))
			text_out("n average");

		text_out(" life rating of ");
		text_out_c(TERM_L_BLUE, "%d", r_ptr->avg_hp);
		text_out(".  ");
	}
}


static void describe_monster_exp(int r_idx, const monster_lore *l_ptr)
{
	const monster_race *r_ptr = &r_info[r_idx];
	u32b f[RACE_FLAG_STRICT_UB];

	cptr p, q;

	long i, j;

	char buf[20] = "";

	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	/* Introduction */
	if (f[0] & RF0_UNIQUE)
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
	text_out_c(TERM_BLUE, format("%s point%s", buf, PLURAL((i == 1) && (j == 0))));

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
	u32b f[RACE_FLAG_STRICT_UB];

	bool old = FALSE;

	/* Get the known monster flags */
	monster_flags_known(r_ptr, l_ptr, f);

	text_out("This");

	if (r_ptr->flags[2] & RF2_ANIMAL) text_out_c(TERM_L_BLUE, " natural");
	if (r_ptr->flags[2] & RF2_EVIL) text_out_c(TERM_L_BLUE, " evil");
	if (r_ptr->flags[2] & RF2_UNDEAD) text_out_c(TERM_L_BLUE, " undead");
	if (r_ptr->flags[2] & RF2_METAL) text_out_c(TERM_L_BLUE, " metal");

	if (r_ptr->flags[2] & RF2_DRAGON) text_out_c(TERM_L_BLUE, " dragon");
	else if (r_ptr->flags[2] & RF2_DEMON) text_out_c(TERM_L_BLUE, " demon");
	else if (r_ptr->flags[2] & RF2_GIANT) text_out_c(TERM_L_BLUE, " giant");
	else if (r_ptr->flags[2] & RF2_TROLL) text_out_c(TERM_L_BLUE, " troll");
	else if (r_ptr->flags[2] & RF2_ORC) text_out_c(TERM_L_BLUE, " orc");
	else text_out_c(TERM_L_BLUE, " creature");

	/* Describe location */
	if (r_ptr->level == 0)
	{
		text_out(" lives in the town");
		old = TRUE;
	}
	else
	{
		byte colour = (r_ptr->level > p_ptr->max_depth) ? TERM_RED : TERM_L_BLUE;

		if (f[0] & RF0_FORCE_DEPTH)
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
	if ((f[0] & (RF0_RAND_50 | RF0_RAND_25)))
	{
		/* Adverb */
		if ((f[0] & RF0_RAND_50) && (f[0] & RF0_RAND_25))
			text_out(" extremely");
		else if (f[0] & RF0_RAND_50)
			text_out(" somewhat");
		else if (f[0] & RF0_RAND_25)
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
	if (f[0] & RF0_NEVER_MOVE)
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
	memset(l_ptr->flags, 255, sizeof(l_ptr->flags));
	race_flags_assign_spell(l_ptr->spell_flags, r_ptr->spell_flags);
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
	u32b f[RACE_FLAG_STRICT_UB];
	atk_colors colors;

	/* Get the race and lore */
	const monster_race *r_ptr = &r_info[r_idx];
	monster_lore *l_ptr = &l_list[r_idx];

	/* Determine the special attack colors */
	get_attack_colors(&colors);

	/* Hack -- create a copy of the monster-memory */
	COPY(&lore, l_ptr, monster_lore);

	/* Assume some "obvious" flags */
	lore.flags[0] |= (RF0_OBVIOUS_MASK);

	/* Killing a monster reveals some properties */
	if (lore.tkills)
	{
		/* Know "race" flags */
		lore.flags[2] |= (RF2_RACE_MASK);

		/* Know "forced" flags */
		lore.flags[0] |= (RF0_FORCE_DEPTH);
	}
	
	/* Now get the known monster flags */
	monster_flags_known(r_ptr, &lore, f);

	/* Cheat -- know everything */
	if (OPT(cheat_know) || spoilers) cheat_monster_lore(r_idx, &lore);

	/* Show kills of monster vs. player(s) */
	if (!spoilers) describe_monster_kills(r_idx, &lore);

	/* Monster description */
	describe_monster_desc(r_idx);

	/* Describe the monster type, speed, life, and armor */
	describe_monster_movement(r_idx, &lore);
   if (!spoilers) describe_monster_toughness(r_idx, &lore);

   /* Describe the experience and item reward when killed */
	if (!spoilers) describe_monster_exp(r_idx, &lore);
   describe_monster_drop(r_idx, &lore);

	/* Describe the special properties of the monster */
	describe_monster_abilities(r_idx, &lore);

   /* Describe the spells, spell-like abilities and melee attacks */
   describe_monster_spells(r_idx, &lore, &colors);
	describe_monster_attack(r_idx, &lore, &colors);

	/* Notice "Quest" monsters */
	if (f[0] & RF0_QUESTOR)
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


/*
 * Return the r_idx of the monster with the given name.
 *
 * Note: on the rare occasions where two different entries have
 * the same name (such as novice mages), we return the index of the
 * first entry that matches. It would probably be a good idea in the
 * future to make sure all monsters have distinct names.
 */
int lookup_monster(const char *name)
{
	int i;

	/* Look for it */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		const char *nm = r_name + r_ptr->name;

		/* Found a match */
		if (streq(name, nm))
			return i;

	}

	return -1;
}
