/* File: birth.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cmds.h"



/*
 * Forward declare
 */
typedef struct birther /*lovely*/ birther; /*sometimes we think she's a dream*/

/*
 * A structure to hold "rolled" information, and any
 * other useful state for the birth process.
 */
struct birther
{
	s16b age;
	s16b wt;
	s16b ht;
	s16b sc;

	s32b au;

	s16b stat[A_MAX];

	char *name;

	char history[250];
};



/*
 * The last character displayed
 */
static birther prev;


/*
 * Current stats (when rolling a character).
 */
static s16b stat_use[A_MAX];



/*
 * Save the currently rolled data for later.
 */
static void save_prev_data(void)
{
	int i;


	/*** Save the current data ***/

	/* Save the data */
	prev.age = p_ptr->age;
	prev.wt = p_ptr->wt;
	prev.ht = p_ptr->ht;
	prev.sc = p_ptr->sc;
	prev.au = p_ptr->au;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		prev.stat[i] = p_ptr->stat_max[i];
	}

	/* Save the history */
	my_strcpy(prev.history, p_ptr->history, sizeof(prev.history));
}


/*
 * Load the previously rolled data.
 */
static void load_prev_data(void)
{
	int i;

	birther temp;


	/*** Save the current data ***/

	/* Save the data */
	temp.age = p_ptr->age;
	temp.wt = p_ptr->wt;
	temp.ht = p_ptr->ht;
	temp.sc = p_ptr->sc;
	temp.au = p_ptr->au;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		temp.stat[i] = p_ptr->stat_max[i];
	}

	/* Save the history */
	my_strcpy(temp.history, p_ptr->history, sizeof(temp.history));


	/*** Load the previous data ***/

	/* Load the data */
	p_ptr->age = prev.age;
	p_ptr->wt = p_ptr->wt_birth = prev.wt;
	p_ptr->ht = p_ptr->ht_birth = prev.ht;
	p_ptr->sc = prev.sc;
	p_ptr->au = p_ptr->au_birth = prev.au;

	/* Load the stats */
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_max[i] = p_ptr->stat_cur[i] = p_ptr->stat_birth[i] = prev.stat[i];
	}

	/* Load the history */
	my_strcpy(p_ptr->history, prev.history, sizeof(p_ptr->history));


	/*** Save the current data ***/

	/* Save the data */
	prev.age = temp.age;
	prev.wt = temp.wt;
	prev.ht = temp.ht;
	prev.sc = temp.sc;
	prev.au = temp.au;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		prev.stat[i] = temp.stat[i];
	}

	/* Save the history */
	my_strcpy(prev.history, temp.history, sizeof(prev.history));
}




/*
 * Adjust a stat by an amount.
 *
 * This just uses "modify_stat_value()" unless "maximize" mode is false,
 * and a positive bonus is being applied, in which case, a special hack
 * is used, with the "auto_roll" flag affecting the result.
 *
 * The "auto_roll" flag selects "maximal" changes for use with the
 * auto-roller initialization code.  Otherwise, if "maximize" mode
 * is being used, the changes are fixed.  Otherwise, semi-random
 * changes will occur, with larger changes at lower values.
 */
static int adjust_stat(int value, int amount, int auto_roll)
{
	/* Negative amounts or maximize mode */
	if ((amount < 0) || adult_maximize)
	{
		return (modify_stat_value(value, amount));
	}

	/* Special hack */
	else
	{
		int i;

		/* Apply reward */
		for (i = 0; i < amount; i++)
		{
			if (value < 18)
			{
				value++;
			}
			else if (value < 18+70)
			{
				value += ((auto_roll ? 15 : randint(15)) + 5);
			}
			else if (value < 18+90)
			{
				value += ((auto_roll ? 6 : randint(6)) + 2);
			}
			else if (value < 18+100)
			{
				value++;
			}
		}
	}

	/* Return the result */
	return (value);
}




/*
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(void)
{
	int i, j;

	int bonus;

	int dice[18];


	/* Roll and verify some stats */
	while (TRUE)
	{
		/* Roll some dice */
		for (j = i = 0; i < 18; i++)
		{
			/* Roll the dice */
			dice[i] = randint(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/* Verify totals */
		if ((j > 42) && (j < 54)) break;
	}

	/* Roll the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

		/* Save that value */
		p_ptr->stat_max[i] = j;

		/* Obtain a "bonus" for "race" and "class" */
		bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

		/* Variable stat maxes */
		if (adult_maximize)
		{
			/* Start fully healed */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i];

			/* Efficiency -- Apply the racial/class bonuses */
			stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
		}

		/* Fixed stat maxes */
		else
		{
			/* Apply the bonus to the stat (somewhat randomly) */
			stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus, FALSE);

			/* Save the resulting stat maximum */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
		}

		p_ptr->stat_birth[i] = p_ptr->stat_max[i];
	}
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra(void)
{
	int i, j, min_value, max_value;


	/* Level one */
	p_ptr->max_lev = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

	/* Initial hitpoints */
	p_ptr->mhp = p_ptr->hitdie;

	/* Minimum hitpoints at highest level */
	min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
	min_value += PY_MAX_LEVEL;

	/* Maximum hitpoints at highest level */
	max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
	max_value += PY_MAX_LEVEL;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Roll out the hitpoints */
	while (TRUE)
	{
		/* Roll the hitpoint values */
		for (i = 1; i < PY_MAX_LEVEL; i++)
		{
			j = randint(p_ptr->hitdie);
			p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
		}

		/* XXX Could also require acceptable "mid-level" hitpoints */

		/* Require "valid" hitpoints at highest level */
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] < min_value) continue;
		if (p_ptr->player_hp[PY_MAX_LEVEL-1] > max_value) continue;

		/* Acceptable */
		break;
	}
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(void)
{
	int i, chart, roll, social_class;


	/* Clear the previous history strings */
	p_ptr->history[0] = '\0';


	/* Initial social class */
	social_class = randint(4);

	/* Starting place */
	chart = rp_ptr->hist;


	/* Process the history */
	while (chart)
	{
		/* Start over */
		i = 0;

		/* Roll for nobility */
		roll = randint(100);

		/* Get the proper entry in the table */
		while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

		/* Get the textual history */
		my_strcat(p_ptr->history, (h_text + h_info[i].text), sizeof(p_ptr->history));

		/* Add in the social class */
		social_class += (int)(h_info[i].bonus) - 50;

		/* Enter the next chart */
		chart = h_info[i].next;
	}



	/* Verify social class */
	if (social_class > 100) social_class = 100;
	else if (social_class < 1) social_class = 1;

	/* Save the social class */
	p_ptr->sc = social_class;
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
	/* Calculate the age */
	p_ptr->age = rp_ptr->b_age + randint(rp_ptr->m_age);

	/* Calculate the height/weight for males */
	if (p_ptr->psex == SEX_MALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->m_b_wt, rp_ptr->m_m_wt);
	}

	/* Calculate the height/weight for females */
	else if (p_ptr->psex == SEX_FEMALE)
	{
		p_ptr->ht = p_ptr->ht_birth = Rand_normal(rp_ptr->f_b_ht, rp_ptr->f_m_ht);
		p_ptr->wt = p_ptr->wt_birth = Rand_normal(rp_ptr->f_b_wt, rp_ptr->f_m_wt);
	}
}




/*
 * Get the player's starting money
 */
static void get_money(void)
{
	int i;

	int gold;

	/* Social Class determines starting gold */
	gold = (p_ptr->sc * 6) + randint(100) + 300;

	/* Process the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Mega-Hack -- reduce gold for high stats */
		if (stat_use[i] >= 18+50) gold -= 300;
		else if (stat_use[i] >= 18+20) gold -= 200;
		else if (stat_use[i] > 18) gold -= 150;
		else gold -= (stat_use[i] - 8) * 10;
	}

	/* Minimum 100 gold */
	if (gold < 100) gold = 100;

	/* Save the gold */
	p_ptr->au = p_ptr->au_birth = gold;
}



/*
 * Clear all the global "character" data
 */
static void player_wipe(void)
{
	int i;

	/* Backup the player choices */
	byte psex = p_ptr->psex;
	byte prace = p_ptr->prace;
	byte pclass = p_ptr->pclass;

	/* Wipe the player */
	(void)WIPE(p_ptr, player_type);

	/* Restore the choices */
	p_ptr->psex = psex;
	p_ptr->prace = prace;
	p_ptr->pclass = pclass;

	/* Clear the inventory */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_wipe(&inventory[i]);
	}


	/* Start with no artifacts made yet */
	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		a_ptr->cur_num = 0;
	}


	/* Start with no quests */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		q_list[i].level = 0;
	}

	/* Add a special quest */
	q_list[0].level = 99;

	/* Add a second quest */
	q_list[1].level = 100;


	/* Reset the "objects" */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Reset "tried" */
		k_ptr->tried = FALSE;

		/* Reset "aware" */
		k_ptr->aware = FALSE;
	}


	/* Reset the "monsters" */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Hack -- Reset the counter */
		r_ptr->cur_num = 0;

		/* Hack -- Reset the max counter */
		r_ptr->max_num = 100;

		/* Hack -- Reset the max counter */
		if (r_ptr->flags1 & (RF1_UNIQUE)) r_ptr->max_num = 1;

		/* Clear player kills */
		l_ptr->pkills = 0;
	}


	/* Hack -- no ghosts */
	r_info[z_info->r_max-1].max_num = 0;


	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;


	/* None of the spells have been learned yet */
	for (i = 0; i < PY_MAX_SPELLS; i++) p_ptr->spell_order[i] = 99;
}

/*
 * Try to wield everything wieldable in the inventory.
 */
static void wield_all(void)
{
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	int slot;
	int item;

	/* Scan through the slots backwards */
	for (item = INVEN_PACK - 1; item >= 0; item--)
	{
		o_ptr = &inventory[item];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Make sure we can wield it and that there's nothing else in that slot */
		slot = wield_slot(o_ptr);
		if (slot < INVEN_WIELD) continue;
		if (inventory[slot].k_idx) continue;

		/* Get local object */
		i_ptr = &object_type_body;
		object_copy(i_ptr, o_ptr);

		/* Modify quantity */
		i_ptr->number = 1;

		/* Decrease the item (from the pack) */
		if (item >= 0)
		{
			inven_item_increase(item, -1);
			inven_item_optimize(item);
		}

		/* Decrease the item (from the floor) */
		else
		{
			floor_item_increase(0 - item, -1);
			floor_item_optimize(0 - item);
		}

		/* Get the wield slot */
		o_ptr = &inventory[slot];

		/* Wear the new stuff */
		object_copy(o_ptr, i_ptr);

		/* Increase the weight */
		p_ptr->total_weight += i_ptr->weight;

		/* Increment the equip counter by hand */
		p_ptr->equip_cnt++;
	}

	return;
}


/*
 * Init players with some belongings
 *
 * Having an item identifies it and makes the player "aware" of its purpose.
 */
static void player_outfit(void)
{
	int i;
	const start_item *e_ptr;
	object_type *i_ptr;
	object_type object_type_body;


	/* Hack -- Give the player his equipment */
	for (i = 0; i < MAX_START_ITEMS; i++)
	{
		/* Access the item */
		e_ptr = &(cp_ptr->start_items[i]);

		/* Get local object */
		i_ptr = &object_type_body;

		/* Hack	-- Give the player an object */
		if (e_ptr->tval > 0)
		{
			/* Get the object_kind */
			int k_idx = lookup_kind(e_ptr->tval, e_ptr->sval);

			/* Valid item? */
			if (!k_idx) continue;

			/* Prepare the item */
			object_prep(i_ptr, k_idx);
			i_ptr->number = (byte)rand_range(e_ptr->min, e_ptr->max);
			i_ptr->origin = ORIGIN_BIRTH;

			object_aware(i_ptr);
			object_known(i_ptr);
			(void)inven_carry(i_ptr);
			k_info[k_idx].everseen = TRUE;
		}
	}


	/* Hack -- give the player hardcoded equipment XXX */

	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some food */
	object_prep(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
	i_ptr->number = (byte)rand_range(3, 7);
	i_ptr->origin = ORIGIN_BIRTH;
	object_aware(i_ptr);
	object_known(i_ptr);
	k_info[i_ptr->k_idx].everseen = TRUE;
	(void)inven_carry(i_ptr);


	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Give the player some torches */
	object_prep(i_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
	i_ptr->number = (byte)rand_range(3, 7);
	i_ptr->timeout = rand_range(3, 7) * 500;
	i_ptr->origin = ORIGIN_BIRTH;
	object_aware(i_ptr);
	object_known(i_ptr);
        k_info[i_ptr->k_idx].everseen = TRUE;
	(void)inven_carry(i_ptr);


	/* Now try wielding everything */
	wield_all();
}


/* Locations of the tables on the screen */
#define HEADER_ROW       1
#define QUESTION_ROW     7
#define TABLE_ROW       10

#define QUESTION_COL     2
#define SEX_COL          2
#define RACE_COL        14
#define RACE_AUX_COL    29
#define CLASS_COL       29
#define CLASS_AUX_COL   50

/*
 * Clear the previous question
 */
static void clear_question(void)
{
	int i;

	for (i = QUESTION_ROW; i < TABLE_ROW; i++)
	{
		/* Clear line, position cursor */
		Term_erase(0, i, 255);
	}
}
/* =================================================== */

/* gender/race/classs menu selector */

/*
 * Display additional information about each race during the selection.
 */
static void race_aux_hook(int race, void *db, const region *reg)
{
	int i;
	char s[50];

	if (race == z_info->p_max) return;

	/* Display relevant details. */
	for (i = 0; i < A_MAX; i++)
	{
		strnfmt(s, sizeof(s), "%s%+d", stat_names_reduced[i],
		        p_info[race].r_adj[i]);
		Term_putstr(RACE_AUX_COL, TABLE_ROW + i, -1, TERM_WHITE, s);
	}

	strnfmt(s, sizeof(s), "Hit die: %d ", p_info[race].r_mhp);
	Term_putstr(RACE_AUX_COL, TABLE_ROW + A_MAX, -1, TERM_WHITE, s);
	strnfmt(s, sizeof(s), "Experience: %d%% ", p_info[race].r_exp);
	Term_putstr(RACE_AUX_COL, TABLE_ROW + A_MAX + 1, -1, TERM_WHITE, s);
	strnfmt(s, sizeof(s), "Infravision: %d ft ", p_info[race].infra * 10);
	Term_putstr(RACE_AUX_COL, TABLE_ROW + A_MAX + 2, -1, TERM_WHITE, s);
}


/*
 * Display additional information about each class during the selection.
 */
static void class_aux_hook(int class_idx, void *db, const region *loc)
{
	int i;
	char s[128];

	if (class_idx == z_info->c_max) return;

	/* Display relevant details. */
	for (i = 0; i < A_MAX; i++)
	{
		strnfmt(s, sizeof(s), "%s%+d", stat_names_reduced[i],
		        c_info[class_idx].c_adj[i]);
		Term_putstr(CLASS_AUX_COL, TABLE_ROW + i, -1, TERM_WHITE, s);
	}

	strnfmt(s, sizeof(s), "Hit die: %d ", c_info[class_idx].c_mhp);
	Term_putstr(CLASS_AUX_COL, TABLE_ROW + A_MAX, -1, TERM_WHITE, s);
	strnfmt(s, sizeof(s), "Experience: %d%% ", c_info[class_idx].c_exp);
	Term_putstr(CLASS_AUX_COL, TABLE_ROW + A_MAX + 1, -1, TERM_WHITE, s);
}


static region gender_region = {SEX_COL, TABLE_ROW, 15, -2};
static region race_region = {RACE_COL, TABLE_ROW, 15, -2};
static region class_region = {CLASS_COL, TABLE_ROW, 19, -2};
static region roller_region = {44, TABLE_ROW, 21, -2};


/* Event handler implementation */
static bool handler_aux(char cmd, int oid, byte *val, int max, int mask, cptr topic)
{
	if (cmd == '\xff' || cmd == '\r') {
		*val = oid;
	}
	else if (cmd == '*') {
		for(;;) 
		{
			oid = rand_int(max);
			*val = oid;
			if(mask & (1L << oid)) break;
		}
	}
	else if (cmd == '=') 
	{
		do_cmd_options();
		return FALSE;
	}
	else if (cmd == KTRL('X')) 
	{
		quit(NULL);
	}
	else if (cmd == '?') {
		char buf[80];
		strnfmt(buf, sizeof(buf), "%s#%s", "birth.txt", topic);
		screen_save();
		show_file(buf, NULL, 0, 0);
		screen_load();
		return FALSE;
	}
	else return FALSE;

	sp_ptr = &sex_info[p_ptr->psex];
	rp_ptr = &p_info[p_ptr->prace];
	cp_ptr = &c_info[p_ptr->pclass];
	mp_ptr = &cp_ptr->spells;
	return TRUE;
}

/* GENDER */
/* Display a gender */
static void display_gender(menu_type *menu, int oid, bool cursor,
							int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	c_put_str(attr, sex_info[oid].title, row, col);
}

static bool gender_handler(char cmd, void *db, int oid)
{
	return handler_aux(cmd, oid, &p_ptr->psex, SEX_MALE+1,
							0xffffffff, sex_info[oid].title);
}

/* RACE */
static void display_race(menu_type *menu, int oid, bool cursor,
						int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	c_put_str(attr, p_name + p_info[oid].name, row, col);
}

static bool race_handler(char cmd, void *db, int oid)
{
	return handler_aux(cmd, oid, &p_ptr->prace, z_info->p_max,
							0xffffffff, p_name + p_info[oid].name);
}

/* CLASS */
static void display_class(menu_type *menu, int oid, bool cursor,
							int row, int col, int width)
{
	byte attr = curs_attrs[0 != (rp_ptr->choice & (1L << oid))][0 != cursor];
	c_put_str(attr, c_name + c_info[oid].name, row, col);
}

static bool class_handler(char cmd, void *db, int oid)
{
	return handler_aux(cmd, oid, &p_ptr->pclass, z_info->c_max,
							(rp_ptr->choice), c_name + c_info[oid].name);
}

/* ROLLER */
static void display_roller(menu_type *menu, int oid, bool cursor,
							int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][0 != cursor];
	const char *str;

	if (oid == 0)
		str = "Point-based";
	else if (oid == 1)
		str = "Autoroller";
	else
		str = "Standard roller";

	c_prt(attr, str, row, col);
}


static byte roller_type = 0;
#define ROLLER_POINT    0
#define ROLLER_AUTO     1
#define ROLLER_STD      2

static bool roller_handler(char cmd, void *db, int oid)
{
	if (cmd == '\xff' || cmd == '\r')
	{
		roller_type = oid;
		return TRUE;
	}
	else if (cmd == '*')
	{
		roller_type = 2;
		return TRUE;
	}
	else if(cmd == '=')
		do_cmd_options();
	else if(cmd == KTRL('X'))
		quit(NULL);
	else if(cmd == '?') {
		char buf[80];
		char *str;

		if (oid == 0)
			str = "Point-based";
		else if (oid == 1)
			str = "Autoroller";
		else
			str = "Standard roller";

		strnfmt(buf, sizeof(buf), "%s#%s", "birth.txt", str);
		screen_save();
		show_file(buf, NULL, 0, 0);
		screen_load();
	}

	return FALSE;
}


static const menu_iter menu_defs[] = {
	{ 0, 0, 0, display_gender, gender_handler },
	{ 0, 0, 0, display_race, race_handler },
	{ 0, 0, 0, display_class, class_handler },
	{ 0, 0, 0, display_roller, roller_handler },
};

/* Menu display and selector */

#define ASEX 0
#define ARACE 1
#define ACLASS 2
#define AROLL 3



static bool choose_character(bool start_at_end)
{
	int i = 0;

	const region *regions[] = { &gender_region, &race_region, &class_region, &roller_region };
	byte *values[4]; /* { &p_ptr->psex, &p_ptr->prace, &p_ptr->pclass }; */
	int limits[4]; /* { SEX_MALE +1, z_info->p_max, z_info->c_max }; */

	menu_type menu;

	const char *hints[] =
	{
		"Your 'sex' does not have any significant gameplay effects.",
		"Your 'race' determines various intrinsic factors and bonuses.",
		"Your 'class' determines various intrinsic abilities and bonuses",
		"Your choice of character generation.  Point-based is recommended."
	};
	
	typedef void (*browse_f) (int oid, void *, const region *loc);
	browse_f browse[] = {NULL, race_aux_hook, class_aux_hook, NULL };

	/* Stupid ISO C array initialization. */
	values[ASEX] = &p_ptr->psex;
	values[ARACE] = &p_ptr->prace;
	values[ACLASS] = &p_ptr->pclass;
	values[AROLL] = &roller_type;
	limits[ASEX] = SEX_MALE + 1;
	limits[ARACE] = z_info->p_max;
	limits[ACLASS] = z_info->c_max;
	limits[AROLL] = 3;

	WIPE(&menu, menu);
	menu.cmd_keys = "?=*\r\n\x18";		 /* ?, ,= *, \n, <ctl-X> */
	menu.selections = lower_case;

	while (i < (int)N_ELEMENTS(menu_defs))
	{
		event_type cx;
		int cursor = *values[i];

		menu.flags = MN_DBL_TAP;
		menu.count = limits[i];
		menu.browse_hook = browse[i];
		menu_init2(&menu, find_menu_skin(MN_SCROLL), &menu_defs[i], regions[i]);

		clear_question();
		Term_putstr(QUESTION_COL, QUESTION_ROW, -1, TERM_YELLOW, hints[i]);

		if (start_at_end)
		{
			menu_refresh(&menu);
			i++;
			if (i == N_ELEMENTS(menu_defs) - 1)
			{
				start_at_end = FALSE;
			}
		}
		else
		{
			cx = menu_select(&menu, &cursor, 0);

			if (cx.key == ESCAPE || cx.type == EVT_BACK)
			{
				if (i > 0) 
				{
					/* Move back one menu */
					*values[i] = cursor;
					region_erase(regions[i]);
					i--;
				}
			}
			else if (cx.key == '*')
			{
				/* Force refresh */
				Term_key_push('6');
				continue;
			}

			/* Selection! */
			else if(cx.key == '\r' || cx.key == '\n' || cx.key == '\xff')
				i++;
		}
	}

	return TRUE;
}


/*
 * Helper function for 'player_birth()'.
 *
 * This function allows the player to select a sex, race, and class, and
 * modify options (including the birth options).
 */
static bool player_birth_aux_1(bool start_at_end)
{
	int i;

	/*** Instructions ***/

	/* Clear screen */
	Term_clear();

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Indent output */
	text_out_indent = QUESTION_COL;
	Term_gotoxy(QUESTION_COL, HEADER_ROW);

	/* Display some helpful information */
	text_out_c(TERM_L_BLUE,
	           "Please select your character from the menu below:\n\n");
	text_out("Use the ");
	text_out_c(TERM_L_GREEN, "movement keys");
	text_out(" to scroll the menu, ");
	text_out_c(TERM_L_GREEN, "Enter");
	text_out(" to select the current menu item, '");
	text_out_c(TERM_L_GREEN, "*");
	text_out("' for a random menu item, '");
	text_out_c(TERM_L_GREEN, "ESC");
	text_out("' to step back through the birth process, '");
	text_out_c(TERM_L_GREEN, "=");
	text_out("' for the birth options, '");
	text_out_c(TERM_L_GREEN, "?");
	text_out("' for help, or '");
	text_out_c(TERM_L_GREEN, "Ctrl-X");
	text_out("' to quit.");

	/* Reset text_out() indentation */
	text_out_indent = 0;

	if (!choose_character(start_at_end)) return FALSE;


	/* Set adult options from birth options */
	for (i = OPT_BIRTH; i < OPT_CHEAT; i++)
		op_ptr->opt[OPT_ADULT + (i - OPT_BIRTH)] = op_ptr->opt[i];

	/* Reset score options from cheat options */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
		op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = op_ptr->opt[i];

	/* Reset squelch bits */
	for (i = 0; i < z_info->k_max; i++)
		k_info[i].squelch = FALSE;

	/* Clear the squelch bytes */
	for (i = 0; i < SQUELCH_BYTES; i++)
		squelch_level[i] = 0;


	/* Done */
	return (TRUE);
}

/* =================================================== */

/*
 * Initial stat costs (initial stats always range from 10 to 18 inclusive).
 */
static const int birth_stat_costs[(18-10)+1] = { 0, 1, 2, 4, 7, 11, 16, 22, 30 };


/*
 * Helper function for 'player_birth()'.
 *
 * This function handles "point-based" character creation.
 *
 * The player selects, for each stat, a value from 10 to 18 (inclusive),
 * each costing a certain amount of points (as above), from a pool of 48
 * available points, to which race/class modifiers are then applied.
 *
 * Each unused point is converted into 100 gold pieces.
 */
static int player_birth_aux_2(bool start_at_end)
{
	int i;

	int row = 3;
	int col = 42;

	int stat = 0;

	static int stats[A_MAX];

	int cost;

	char ch;

	char buf[80];

	bool first_time = FALSE;

	/* Clear */
	Term_clear();

	if (!start_at_end)
	{
		first_time = TRUE;

		/* Initialize stats */
		for (i = 0; i < A_MAX; i++)
		{
			/* Initial stats */
			stats[i] = 10;
		}


		/* Roll for base hitpoints */
		get_extra();

		/* Roll for age/height/weight */
		get_ahw();

		/* Roll for social class */
		get_history();
	}

	/* Interact */
	while (1)
	{
		/* Reset cost */
		cost = 0;

		/* Process stats */
		for (i = 0; i < A_MAX; i++)
		{
			/* Variable stat maxes */
			if (adult_maximize)
			{
				/* Reset stats */
				p_ptr->stat_cur[i] = p_ptr->stat_max[i] = p_ptr->stat_birth[i] = stats[i];
			}

			/* Fixed stat maxes */
			else
			{
				/* Obtain a "bonus" for "race" and "class" */
				int bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

				/* Apply the racial/class bonuses */
				p_ptr->stat_cur[i] = p_ptr->stat_max[i] =
					modify_stat_value(stats[i], bonus);
			}

			/* Total cost */
			cost += birth_stat_costs[stats[i] - 10];
		}

		/* Restrict cost */
		if (cost > 48)
		{
			/* Warning */
			bell("Excessive stats!");

			/* Reduce stat */
			stats[stat]--;

			/* Recompute costs */
			continue;
		}

		/* Gold is inversely proportional to cost */
		p_ptr->au = p_ptr->au_birth = (100 * (48 - cost)) + 100;

		/* Calculate the bonuses and hitpoints */
		p_ptr->update |= (PU_BONUS | PU_HP);

		/* Update stuff */
		update_stuff();

		/* Fully healed */
		p_ptr->chp = p_ptr->mhp;

		/* Fully rested */
		p_ptr->csp = p_ptr->msp;

		/* Display the player */
		display_player(0);

		/* Display the costs header */
		put_str("Cost", row - 1, col + 32);

		/* Display the costs */
		for (i = 0; i < A_MAX; i++)
		{
			/* Display cost */
			strnfmt(buf, sizeof(buf), "%4d", birth_stat_costs[stats[i] - 10]);
			put_str(buf, row + i, col + 32);
		}


		/* Prompt XXX XXX XXX */
		strnfmt(buf, sizeof(buf), "Total Cost %2d/48.  Use 2/8 to move, 4/6 to modify, 'Enter' to accept.", cost);
		prt(buf, 0, 0);

		/* Place cursor just after cost of current stat */
		Term_gotoxy(col + 36, row + stat);

		/* Get key */
		ch = inkey();

		if (ch == KTRL('X')) 
			quit(NULL);

		/* Go back a step, or back to the start of this step */
		if (ch == ESCAPE) 
		{
			if (first_time) 
				return -1;
			else 
				return 0;
		}

		first_time = FALSE;

		/* Done */
		if ((ch == '\r') || (ch == '\n')) break;

		ch = target_dir(ch);

		/* Prev stat */
		if (ch == 8)
		{
			stat = (stat + A_MAX - 1) % A_MAX;
		}

		/* Next stat */
		if (ch == 2)
		{
			stat = (stat + 1) % A_MAX;
		}

		/* Decrease stat */
		if ((ch == 4) && (stats[stat] > 10))
		{
			stats[stat]--;
		}

		/* Increase stat */
		if ((ch == 6) && (stats[stat] < 18))
		{
			stats[stat]++;
		}
	}


	/* Done - advance a step*/
	return +1;
}


bool minstat_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	if (keypress == KTRL('x'))
		quit(NULL);

	return askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
}


/*
 * Helper function for 'player_birth()'.
 *
 * This function handles "auto-rolling" and "random-rolling".
 */
static int player_birth_aux_3(bool start_at_end, bool autoroll)
{
	int i, j, m, v;

	bool flag;
	static bool prev = FALSE;

	char ch;

	char b1 = '[';
	char b2 = ']';

	char buf[80];


	/* We'll keep these for when we step "back" into the autoroller */
	static s16b stat_limit[A_MAX];

	s32b stat_match[A_MAX];

	s32b auto_round = 0L;

	s32b last_round;


	/* Clear */
	Term_clear();

	/*** Autoroll ***/

	/* Initialize */
	if (!start_at_end && autoroll)
	{
		int mval[A_MAX];

		char inp[80];

		prev = FALSE;

		/* Extra info */
		Term_putstr(5, 10, -1, TERM_WHITE,
		            "The auto-roller will automatically ignore characters which do");
		Term_putstr(5, 11, -1, TERM_WHITE,
		            "not meet the minimum values for any stats specified below.");
		Term_putstr(5, 12, -1, TERM_WHITE,
		            "Note that stats are not independant, so it is not possible to");
		Term_putstr(5, 13, -1, TERM_WHITE,
		            "get perfect (or even high) values for all your stats.");

		/* Prompt for the minimum stats */
		put_str("Enter minimum value for: ", 15, 2);

		/* Output the maximum stats */
		for (i = 0; i < A_MAX; i++)
		{
			/* Reset the "success" counter */
			stat_match[i] = 0;

			/* Race/Class bonus */
			j = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

			/* Obtain the "maximal" stat */
			m = adjust_stat(17, j, TRUE);

			/* Save the maximum */
			mval[i] = m;

			/* Extract a textual format */
			/* cnv_stat(m, inp, sizeof(buf); */

			/* Above 18 */
			if (m > 18)
			{
				strnfmt(inp, sizeof(inp), "(Max of 18/%02d):", (m - 18));
			}

			/* From 3 to 18 */
			else
			{
				strnfmt(inp, sizeof(inp), "(Max of %2d):", m);
			}

			/* Prepare a prompt */
			strnfmt(buf, sizeof(buf), "%-5s%-20s", stat_names[i], inp);

			/* Dump the prompt */
			put_str(buf, 16 + i, 5);
		}

		/* Input the minimum stats */
		for (i = 0; i < A_MAX; i++)
		{
			/* Get a minimum stat */
			while (TRUE)
			{
				char *s;

				/* Move the cursor */
				put_str("", 16 + i, 30);

				/* Default */
				inp[0] = '\0';

				/* Get a response (or escape) */
				if (!askfor_aux(inp, 9, minstat_keypress)) 
				{
					if (i == 0) 
						/* Back a step */
						return -1;
					else 
						/* Repeat this step */
						return 0;
				}

				/* Hack -- add a fake slash */
				my_strcat(inp, "/", sizeof(inp));

				/* Hack -- look for the "slash" */
				s = strchr(inp, '/');

				/* Hack -- Nuke the slash */
				*s++ = '\0';

				/* Hack -- Extract an input */
				v = atoi(inp) + atoi(s);

				/* Break on valid input */
				if (v <= mval[i]) break;
			}

			/* Save the minimum stat */
			stat_limit[i] = (v > 0) ? v : 0;
		}
	}


	/* Clean up */
	if (!start_at_end)
		clear_from(10);

	/*** Generate ***/

	/* Roll */
	while (TRUE)
	{
		if (!start_at_end)
		{
			int col = 42;

			/* Feedback */
			if (autoroll)
			{
				Term_clear();

				/* Label */
				put_str(" Limit", 2, col+5);

				/* Label */
				put_str("  Freq", 2, col+13);

				/* Label */
				put_str("  Roll", 2, col+24);

				/* Put the minimal stats */
				for (i = 0; i < A_MAX; i++)
				{
					/* Label stats */
					put_str(stat_names[i], 3+i, col);

					/* Put the stat */
					cnv_stat(stat_limit[i], buf, sizeof(buf));
					c_put_str(TERM_L_BLUE, buf, 3+i, col+5);
				}

				/* Note when we started */
				last_round = auto_round;

				/* Label count */
				put_str("Round:", 10, col+13);

				/* Indicate the state */
				put_str("(Hit ESC to stop)", 12, col+13);

				/* Auto-roll */
				while (1)
				{
					bool accept = TRUE;

					/* Get a new character */
					get_stats();

					/* Advance the round */
					auto_round++;

					/* Hack -- Prevent overflow */
					if (auto_round >= 1000000L) break;

					/* Check and count acceptable stats */
					for (i = 0; i < A_MAX; i++)
					{
						/* This stat is okay */
						if (stat_use[i] >= stat_limit[i])
						{
							stat_match[i]++;
						}

						/* This stat is not okay */
						else
						{
							accept = FALSE;
						}
					}

					/* Break if "happy" */
					if (accept) break;

					/* Take note every 25 rolls */
					flag = (!(auto_round % 25L));

					/* Update display occasionally */
					if (flag || (auto_round < last_round + 100))
					{
						/* Put the stats (and percents) */
						for (i = 0; i < A_MAX; i++)
						{
							/* Put the stat */
							cnv_stat(stat_use[i], buf, sizeof(buf));
							c_put_str(TERM_L_GREEN, buf, 3+i, col+24);

							/* Put the percent */
							if (stat_match[i])
							{
								int p = 1000L * stat_match[i] / auto_round;
								byte attr = (p < 100) ? TERM_YELLOW : TERM_L_GREEN;
								strnfmt(buf, sizeof(buf), "%3d.%d%%", p/10, p%10);
								c_put_str(attr, buf, 3+i, col+13);
							}

							/* Never happened */
							else
							{
								c_put_str(TERM_RED, "(NONE)", 3+i, col+13);
							}
						}

						/* Dump round */
						put_str(format("%10ld", auto_round), 10, col+20);

						/* Make sure they see everything */
						Term_fresh();

						/* Do not wait for a key */
						inkey_scan = TRUE;

						/* Check for a keypress */
						if (inkey()) break;
					}
				}
			}

			/* Otherwise just get a character */
			else
			{
				/* Get a new character */
				get_stats();
			}

			/* Flush input */
			flush();


			/*** Display ***/

			/* Roll for base hitpoints */
			get_extra();

			/* Roll for age/height/weight */
			get_ahw();

			/* Roll for social class */
			get_history();

			/* Roll for gold */
			get_money();
		}

		start_at_end = FALSE;

		/* Input loop */
		while (TRUE)
		{
			/* Calculate the bonuses and hitpoints */
			p_ptr->update |= (PU_BONUS | PU_HP);

			/* Update stuff */
			update_stuff();

			/* Fully healed */
			p_ptr->chp = p_ptr->mhp;

			/* Fully rested */
			p_ptr->csp = p_ptr->msp;

			/* Display the player */
			display_player(0);

			/* Prepare a prompt (must squeeze everything in) */
			Term_gotoxy(2, 23);
			Term_addch(TERM_WHITE, b1);
			Term_addstr(-1, TERM_WHITE, "'r' to reroll");
			if (prev) Term_addstr(-1, TERM_WHITE, ", 'p' for prev");
			Term_addstr(-1, TERM_WHITE, ", or 'Enter' to accept");
			Term_addch(TERM_WHITE, b2);

			/* Prompt and get a command */
			ch = inkey();

			/* Go back to the start of the step, or the previous step */
			/* if we're not autorolling. */
			if (ch == ESCAPE) 
			{
				if (autoroll) 
					return 0;
				else 
					return -1;
			}

			/* 'Enter' accepts the roll */
			if ((ch == '\r') || (ch == '\n')) break;

			/* Reroll this character */
			if ((ch == ' ') || (ch == 'r')) break;

			/* Previous character */
			if (prev && (ch == 'p'))
			{
				load_prev_data();
				continue;
			}

			/* Help */
			if (ch == '?')
			{
				do_cmd_help();
				continue;
			}

			if (ch == KTRL('X')) 
				quit(NULL);

			/* Warning */
			bell("Illegal auto-roller command!");
		}

		/* Are we done? */
		if ((ch == '\r') || (ch == '\n')) break;

		/* Save this for the "previous" character */
		save_prev_data();

		/* Note that a previous roll exists */
		prev = TRUE;
	}

	/* Clear prompt */
	clear_from(23);

	/* Done - move on a stage */
	return +1;
}

typedef enum 
{
	BIRTH_RESTART = 0,
	BIRTH_QUESTIONS,
	BIRTH_STATS,
	BIRTH_NAME,
	BIRTH_FINAL_APPROVAL,
	BIRTH_ACCEPTED
} birth_stages;


/*
 * Helper function for 'player_birth()'.
 *
 * See "display_player" for screen layout code.
 */
static void player_birth_aux(void)
{
	char ch;
	cptr prompt = "['ESC' to step back, 'S' to start over, or any other key to continue]";
	birth_stages state = BIRTH_QUESTIONS;
	birth_stages last_state = BIRTH_RESTART;

	while (1)
	{
		bool start_at_end = (last_state > state);
		last_state = state;

		switch (state)
		{
			case BIRTH_RESTART:
			{
				state++;
				break;
			}

			case BIRTH_QUESTIONS:
			{
				/* Race, class, etc. choices */
				if (player_birth_aux_1(start_at_end)) 
					state++;
				break;
			}

			case BIRTH_STATS:
			{
				if (roller_type == ROLLER_POINT)
				{
					/* Fill stats using point-based methods */
					state += player_birth_aux_2(start_at_end);
				}
				else
				{
					/* Fills stats using the standard- or auto-roller */
					state += player_birth_aux_3(start_at_end, roller_type == ROLLER_AUTO);
				}
				break;
			}

			case BIRTH_NAME:
			{
				/* Get a name, prepare savefile */
				if (get_name(FALSE)) 
					state++;
				else 
					state--;

				break;
			}
			
			case BIRTH_FINAL_APPROVAL:
			{
				/* Display the player */
				display_player(0);

				/* Prompt for it */
				prt(prompt, Term->hgt - 1, Term->wid / 2 - strlen(prompt) / 2);

				/* Get a key */
				ch = inkey();

				/* Start over */
				if (ch == 'S') 
					state = BIRTH_RESTART;

				if (ch == KTRL('X')) 
					quit(NULL);

				if (ch == ESCAPE) 
					state--;
				else
					state++;

				/* Clear prompt */
				clear_from(23);

				break;
			}
			
			case BIRTH_ACCEPTED:
			{
				return;
			}

		}
	}
}


/*
 * Helper function for 'player_birth_quick()'.
 *
 * See "display_player" for screen layout code.
 */
static bool player_birth_quick(void)
{
	char ch;
	int i;
	birther old_char;
	byte old_hitdie;
	u16b old_expfact;
	s16b old_hp[PY_MAX_LEVEL];

	old_hitdie = p_ptr->hitdie;
	old_expfact = p_ptr->expfact;

	old_char.age = p_ptr->age;
	old_char.au = p_ptr->au_birth;
	old_char.ht = p_ptr->ht_birth;
	old_char.wt = p_ptr->wt_birth;
	old_char.sc = p_ptr->sc;

	/* Save the stats */
	for (i = 0; i < A_MAX; i++)
	{
		old_char.stat[i] = p_ptr->stat_birth[i];
	}

	/* Save the history */
	my_strcpy(old_char.history, p_ptr->history, sizeof(old_char.history));

	/* Save the hp */
	for (i = 0; i < PY_MAX_LEVEL; i++)
	{
		old_hp[i] = p_ptr->player_hp[i];
	}

	/* Wipe the player */
	player_wipe();

	/* Level one */
	p_ptr->max_lev = p_ptr->lev = 1;

	p_ptr->hitdie = old_hitdie;
	p_ptr->expfact = old_expfact;

	p_ptr->age = old_char.age;
	p_ptr->au_birth = p_ptr->au = old_char.au;
	p_ptr->ht_birth = p_ptr->ht = old_char.ht;
	p_ptr->wt_birth = p_ptr->wt = old_char.wt;
	p_ptr->sc = old_char.sc;

	/* Load the stats */
	for (i = 0; i < A_MAX; i++)
	{
		p_ptr->stat_birth[i] = p_ptr->stat_cur[i] = p_ptr->stat_max[i] = old_char.stat[i];
	}

	/* Load the history */
	my_strcpy(p_ptr->history, old_char.history, sizeof(p_ptr->history));

	/* Load the hp */
	for (i = 0; i < PY_MAX_LEVEL; i++)
	{
		p_ptr->player_hp[i] = old_hp[i];
	}

	/* Set adult options from birth options */
	for (i = OPT_BIRTH; i < OPT_CHEAT; i++)
	{
		op_ptr->opt[OPT_ADULT + (i - OPT_BIRTH)] = op_ptr->opt[i];
	}

	/* Reset score options from cheat options */
	for (i = OPT_CHEAT; i < OPT_ADULT; i++)
	{
		op_ptr->opt[OPT_SCORE + (i - OPT_CHEAT)] = op_ptr->opt[i];
	}

	/* Calculate the bonuses and hitpoints */
	p_ptr->update |= (PU_BONUS | PU_HP);

	/* Update stuff */
	update_stuff();

	/* Fully healed */
	p_ptr->chp = p_ptr->mhp;

	/* Fully rested */
	p_ptr->csp = p_ptr->msp;

	/* Display the player */
	display_player(0);

	/* Get a name, prepare savefile */
	get_name(FALSE);

	/* Prompt for it */
	prt("['CTRL-X' to quit, 'ESC' to start over, or any other key to continue]", 23, 5);

	/* Get a key */
	ch = inkey();

	/* Quit */
	if (ch == KTRL('X')) quit(NULL);

	/* Start over */
	if (ch == ESCAPE) return (FALSE);

	/* Accept */
	return (TRUE);
}



/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth(void)
{
	bool quickstart = FALSE;
	char ch;

	/*
	 * If the birth height value is set, we can do a quick-start character.
	 */
	if (p_ptr->ht_birth)
	{
		/* Prompt */
		while (TRUE)
		{
			Term_clear();

			put_str("Quick-start character based on previous one (y/n)? ", 2, 2);
			ch = inkey();

			if (ch == KTRL('X'))
				quit(NULL);
			else if ((ch == ESCAPE) || strchr("YyNn\r\n", ch))
				break;
			else if (ch == '?')
				(void)show_file("birth.hlp", NULL, 0, 0);
			else
				bell("Illegal answer!");
		}

		/* Quick generation */
		if ((ch == 'y') || (ch == 'Y'))
		{
			if (player_birth_quick()) quickstart = TRUE;
		}
	}


	/* Quickstart trumps normal creation */
	if (!quickstart)
	{
		/* Wipe the player properly */
		player_wipe();

		/* Create a new character */
		player_birth_aux();
	}

	/* Note player birth in the message recall */
	message_add(" ", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add("====================", MSG_GENERIC);
	message_add("  ", MSG_GENERIC);
	message_add(" ", MSG_GENERIC);


	/* Hack -- outfit the player */
	player_outfit();


	/* Initialise the stores */
	store_init();
}
