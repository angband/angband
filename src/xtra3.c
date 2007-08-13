/*
 * File: xtra3.c
 * Purpose: Handles the setting up updating, and cleaning up of the various 
 *          things that are displayed by the game.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007 Antony Sidwell
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
#include "ui-event.h"


/* 
 * There are a few functions installed to be triggered by several 
 * of the basic player events.  For convenience, these have been grouped 
 * in this list.
 */
ui_event_type player_events[] =
{
	ui_RACE_CLASS_CHANGED,
	ui_TITLE_CHANGED,
	ui_EXPERIENCE_CHANGED,
	ui_LEVEL_CHANGED,
	ui_GOLD_CHANGED,
	ui_EXPERIENCE_CHANGED,
	ui_EQUIPMENT_CHANGED,
	ui_STATS_CHANGED,
	ui_AC_CHANGED,
	ui_MANA_CHANGED,
	ui_HP_CHANGED,
	ui_HEALTH_CHANGED,
	ui_SPEED_CHANGED,
	ui_DEPTH_CHANGED,
};

ui_event_type statusline_events[] =
{
	ui_STATE_CHANGED,
	ui_STATUS_CHANGED,
	ui_DETECT_TRAPS_CHANGED,
	ui_STUDY_CHANGED
};


/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val, size_t out_len)
{
	/* Above 18 */
	if (val > 18)
	{
		int bonus = (val - 18);

		if (bonus >= 100)
			strnfmt(out_val, out_len, "18/%03d", bonus);
		else
			strnfmt(out_val, out_len, " 18/%02d", bonus);
	}

	/* From 3 to 18 */
	else
	{
		strnfmt(out_val, out_len, "    %2d", val);
	}
}

/* ------------------------------------------------------------------------
 * Sidebar display functions
 * ------------------------------------------------------------------------ */

/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(cptr info, int row, int col)
{
	/* Dump 13 spaces to clear */
	c_put_str(TERM_WHITE, "             ", row, col);

	/* Dump the info itself */
	c_put_str(TERM_L_BLUE, info, row, col);
}


/*
 * Print character stat in given row, column
 */
static void prt_stat(int stat, int row, int col)
{
	char tmp[32];

	/* Display "injured" stat */
	if (p_ptr->stat_cur[stat] < p_ptr->stat_max[stat])
	{
		put_str(stat_names_reduced[stat], row, col);
		cnv_stat(p_ptr->stat_use[stat], tmp, sizeof(tmp));
		c_put_str(TERM_YELLOW, tmp, row, col + 6);
	}

	/* Display "healthy" stat */
	else
	{
		put_str(stat_names[stat], row, col);
		cnv_stat(p_ptr->stat_use[stat], tmp, sizeof(tmp));
		c_put_str(TERM_L_GREEN, tmp, row, col + 6);
	}

	/* Indicate natural maximum */
	if (p_ptr->stat_max[stat] == 18+100)
	{
		put_str("!", row, col + 3);
	}
}


/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title(int row, int col)
{
	cptr p;

	/* Wizard */
	if (p_ptr->wizard)
	{
		p = "[=-WIZARD-=]";
	}

	/* Winner */
	else if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "***WINNER***";
	}

	/* Normal */
	else
	{
		p = c_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
	}

	prt_field(p, row, col);
}


/*
 * Prints level
 */
static void prt_level(int row, int col)
{
	char tmp[32];

	strnfmt(tmp, sizeof(tmp), "%6d", p_ptr->lev);

	if (p_ptr->lev >= p_ptr->max_lev)
	{
		put_str("LEVEL ", row, col);
		c_put_str(TERM_L_GREEN, tmp, row, col + 6);
	}
	else
	{
		put_str("Level ", row, col);
		c_put_str(TERM_YELLOW, tmp, row, col + 6);
	}
}


/*
 * Display the experience
 */
static void prt_exp(int row, int col)
{
	char out_val[32];
	bool lev50 = (p_ptr->lev == 50);

	long xp = (long)p_ptr->exp;


	/* Calculate XP for next level */
	if (!lev50)
		xp = (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L) - p_ptr->exp;

	/* Format XP */
	strnfmt(out_val, sizeof(out_val), "%8ld", (long)xp);


	if (p_ptr->exp >= p_ptr->max_exp)
	{
		put_str((lev50 ? "EXP" : "NXT"), row, col);
		c_put_str(TERM_L_GREEN, out_val, row, col + 4);
	}
	else
	{
		put_str((lev50 ? "Exp" : "Nxt"), row, col);
		c_put_str(TERM_YELLOW, out_val, row, col + 4);
	}
}


/*
 * Prints current gold
 */
static void prt_gold(int row, int col)
{
	char tmp[32];

	put_str("AU ", row, col);
	strnfmt(tmp, sizeof(tmp), "%9ld", (long)p_ptr->au);
	c_put_str(TERM_L_GREEN, tmp, row, col + 3);
}


/*
 * Equippy chars
 */
static void prt_equippy(int row, int col)
{
	int i;

	byte a;
	char c;

	object_type *o_ptr;

	/* No equippy chars in bigtile mode */
	if (use_bigtile) return;

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Object */
		o_ptr = &inventory[i];

		a = object_attr(o_ptr);
		c = object_char(o_ptr);

		/* Clear the part of the screen */
		if (!o_ptr->k_idx)
		{
			c = ' ';
			a = TERM_WHITE;
		}

		/* Dump */
		Term_putch(col + i - INVEN_WIELD, row, a, c);
	}
}


/*
 * Prints current AC
 */
static void prt_ac(int row, int col)
{
	char tmp[32];

	put_str("Cur AC ", row, col);
	strnfmt(tmp, sizeof(tmp), "%5d", p_ptr->dis_ac + p_ptr->dis_to_a);
	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}


/*
 * Prints Cur hit points
 */
static void prt_hp(int row, int col)
{
	char cur_hp[32], max_hp[32];
	byte color;

	put_str("HP ", row, col);

	strnfmt(max_hp, sizeof(max_hp), "%4d", p_ptr->mhp);
	strnfmt(cur_hp, sizeof(cur_hp), "%4d", p_ptr->chp);

	if (p_ptr->chp >= p_ptr->mhp)
		color = TERM_L_GREEN;
	else if (p_ptr->chp > (p_ptr->mhp * op_ptr->hitpoint_warn) / 10)
		color = TERM_YELLOW;
	else
		color = TERM_RED;

	c_put_str(color, cur_hp, row, col + 3);
	c_put_str(TERM_WHITE, "/", row, col + 7);
	c_put_str(TERM_L_GREEN, max_hp, row, col + 8);
}


/*
 * Prints players max/cur spell points
 */
static void prt_sp(int row, int col)
{
	char cur_sp[32], max_sp[32];
	byte color;

	/* Do not show mana unless it matters */
	if (!cp_ptr->spell_book) return;

	put_str("SP ", row, col);

	strnfmt(max_sp, sizeof(max_sp), "%4d", p_ptr->msp);
	strnfmt(cur_sp, sizeof(cur_sp), "%4d", p_ptr->csp);

	if (p_ptr->csp >= p_ptr->msp)
		color = TERM_L_GREEN;
	else if (p_ptr->csp > (p_ptr->msp * op_ptr->hitpoint_warn) / 10)
		color = TERM_YELLOW;
	else
		color = TERM_RED;

	/* Show mana */
	c_put_str(color, cur_sp, row, col + 3);
	c_put_str(TERM_WHITE, "/", row, col + 7);
	c_put_str(TERM_L_GREEN, max_sp, row, col + 8);
}


/*
 * Redraw the "monster health bar"
 *
 * The "monster health bar" provides visual feedback on the "health"
 * of the monster currently being "tracked".  There are several ways
 * to "track" a monster, including targetting it, attacking it, and
 * affecting it (and nobody else) with a ranged attack.  When nothing
 * is being tracked, we clear the health bar.  If the monster being
 * tracked is not currently visible, a special health bar is shown.
 */
static void prt_health(int row, int col)
{
	/* Not tracking */
	if (!p_ptr->health_who)
	{
		/* Erase the health bar */
		Term_erase(col, row, 12);
	}

	/* Tracking an unseen monster */
	else if (!mon_list[p_ptr->health_who].ml)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a hallucinatory monster */
	else if (p_ptr->timed[TMD_IMAGE])
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a dead monster (?) */
	else if (!mon_list[p_ptr->health_who].hp < 0)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a visible monster */
	else
	{
		int pct, len;

		monster_type *m_ptr = &mon_list[p_ptr->health_who];

		/* Default to almost dead */
		byte attr = TERM_RED;

		/* Extract the "percent" of health */
		pct = 100L * m_ptr->hp / m_ptr->maxhp;

		/* Badly wounded */
		if (pct >= 10) attr = TERM_L_RED;

		/* Wounded */
		if (pct >= 25) attr = TERM_ORANGE;

		/* Somewhat Wounded */
		if (pct >= 60) attr = TERM_YELLOW;

		/* Healthy */
		if (pct >= 100) attr = TERM_L_GREEN;

		/* Afraid */
		if (m_ptr->monfear) attr = TERM_VIOLET;

		/* Confused */
		if (m_ptr->confused) attr = TERM_UMBER;

		/* Stunned */
		if (m_ptr->stunned) attr = TERM_L_BLUE;

		/* Asleep */
		if (m_ptr->csleep) attr = TERM_BLUE;

		/* Convert percent into "health" */
		len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;

		/* Default to "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		Term_putstr(col + 1, row, len, attr, "**********");
	}
}


/*
 * Prints the speed of a character.
 */
static void prt_speed(int row, int col)
{
	int i = p_ptr->pspeed;

	byte attr = TERM_WHITE;
	const char *type = NULL;
	char buf[32] = "";

	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->searching) i += 10;

	/* Fast */
	if (i > 110)
	{
		attr = TERM_L_GREEN;
		type = "Fast";
	}

	/* Slow */
	else if (i < 110)
	{
		attr = TERM_L_UMBER;
		type = "Slow";
	}

	if (type)
		strnfmt(buf, sizeof(buf), "%s (%+d)", type, (i - 110));

	/* Display the speed */
	c_put_str(attr, format("%-10s", buf), row, col);
}


/*
 * Prints depth in stat area
 */
static void prt_depth(int row, int col)
{
	char depths[32];

	if (!p_ptr->depth)
	{
		my_strcpy(depths, "Town", sizeof(depths));
	}
	else if (depth_in_feet)
	{
		strnfmt(depths, sizeof(depths), "%d ft", p_ptr->depth * 50);
	}
	else
	{
		strnfmt(depths, sizeof(depths), "Lev %d", p_ptr->depth);
	}

	/* Right-Adjust the "depth", and clear old values */
	put_str(format("%-13s", depths), row, col);
}




/* Some simple wrapper functions */
static void prt_str(int row, int col) { prt_stat(A_STR, row, col); }
static void prt_dex(int row, int col) { prt_stat(A_DEX, row, col); }
static void prt_wis(int row, int col) { prt_stat(A_WIS, row, col); }
static void prt_int(int row, int col) { prt_stat(A_INT, row, col); }
static void prt_con(int row, int col) { prt_stat(A_CON, row, col); }
static void prt_chr(int row, int col) { prt_stat(A_CHR, row, col); }
static void prt_race(int row, int col) { prt_field(p_name + rp_ptr->name, row, col); }
static void prt_class(int row, int col) { prt_field(c_name + cp_ptr->name, row, col); }


/*
 * Struct of sidebar handlers.
 */
static const struct side_handler_t
{
	void (*hook)(int, int);	 /* int row, int col */
	int priority;		 /* 1 is most important (always displayed) */
	ui_event_type type;	 /* PR_* flag this corresponds to */
} side_handlers[] =
{
	{ prt_race,    19, ui_RACE_CLASS_CHANGED },
	{ prt_title,   18, ui_TITLE_CHANGED },
	{ prt_class,   22, ui_RACE_CLASS_CHANGED },
	{ prt_level,   10, ui_LEVEL_CHANGED },
	{ prt_exp,     16, ui_EXPERIENCE_CHANGED },
	{ prt_gold,    11, ui_GOLD_CHANGED },
	{ prt_equippy, 17, ui_EQUIPMENT_CHANGED },
	{ prt_str,      6, ui_STATS_CHANGED },
	{ prt_int,      5, ui_STATS_CHANGED },
	{ prt_wis,      4, ui_STATS_CHANGED },
	{ prt_dex,      3, ui_STATS_CHANGED },
	{ prt_con,      2, ui_STATS_CHANGED },
	{ prt_chr,      1, ui_STATS_CHANGED },
	{ NULL,        15, 0 },
	{ prt_ac,       7, ui_AC_CHANGED },
	{ prt_hp,       8, ui_HP_CHANGED },
	{ prt_sp,       9, ui_MANA_CHANGED },
	{ NULL,        21, 0 },
	{ prt_health,  12, ui_HEALTH_CHANGED },
	{ NULL,        20, 0 },
	{ NULL,        22, 0 },
	{ prt_speed,   13, ui_SPEED_CHANGED }, /* Slow (-NN) / Fast (+NN) */
	{ prt_depth,   14, ui_DEPTH_CHANGED }, /* Lev NNN / NNNN ft */
};


/*
 * This prints the sidebar, using a clever method which means that it will only
 * print as much as can be displayed on <24-line screens.
 *
 * Each row is given a priority; the least important higher numbers and the most
 * important lower numbers.  As the screen gets smaller, the rows start to
 * disappear in the order of lowest to highest importance.
 */
static void update_sidebar(ui_event_type type, ui_event_data *data, void *user)
{
	int x, y, row;
	int max_priority;
	size_t i;


	Term_get_size(&x, &y);

	/* Keep the top and bottom lines clear. */
	max_priority = y - 2;

	/* Display list entries */
	for (i = 0, row = 1; i < N_ELEMENTS(side_handlers); i++)
	{
		const struct side_handler_t *hnd = &side_handlers[i];
		int priority = hnd->priority;
		bool from_bottom = FALSE;

		/* Negative means print from bottom */
		if (priority < 0)
		{
			priority = -priority;
			from_bottom = TRUE;
		}

		/* If this is high enough priority, display it */
		if (priority <= max_priority)
		{
			if (hnd->type == type && hnd->hook)
			{
				if (from_bottom)
					hnd->hook(Term->hgt - (N_ELEMENTS(side_handlers) - i), 0);
				else
				    hnd->hook(row, 0);
			}

			/* Increment for next time */
			row++;
		}
	}
}

static void hp_colour_change(ui_event_type type, ui_event_data *data, void *user)
{
	/*
	 * hack:  redraw player, since the player's color
	 * now indicates approximate health.  Note that
	 * using this command when graphics mode is on
	 * causes the character to be a black square.
	 */
	if ((hp_changes_color) && (arg_graphics == GRAPHICS_NONE))
	{
		lite_spot(p_ptr->py, p_ptr->px);
	}
}



/* ------------------------------------------------------------------------
 * Status line display functions
 * ------------------------------------------------------------------------ */

/* Simple macro to initialise structs */
#define S(s)		s, sizeof(s)

/*
 * Struct to describe different timed effects
 */
struct state_info
{
	int value;
	const char *str;
	size_t len;
	byte attr;
};

/* TMD_CUT descriptions */
static const struct state_info cut_data[] =
{
	{ 1000, S("Mortal wound"), TERM_L_RED },
	{  200, S("Deep gash"),    TERM_RED },
	{  100, S("Severe cut"),   TERM_RED },
	{   50, S("Nasty cut"),    TERM_ORANGE },
	{   25, S("Bad cut"),      TERM_ORANGE },
	{   10, S("Light cut"),    TERM_YELLOW },
	{    0, S("Graze"),        TERM_YELLOW },
};

/* TMD_STUN descriptions */
static const struct state_info stun_data[] =
{
	{   100, S("Knocked out"), TERM_RED },
	{    50, S("Heavy stun"),  TERM_ORANGE },
	{     0, S("Stun"),        TERM_ORANGE },
};

/* p_ptr->hunger descriptions */
static const struct state_info hunger_data[] =
{
	{ PY_FOOD_FAINT, S("Faint"),    TERM_RED },
	{ PY_FOOD_WEAK,  S("Weak"),     TERM_ORANGE },
	{ PY_FOOD_ALERT, S("Hungry"),   TERM_YELLOW },
	{ PY_FOOD_FULL,  S(""),         TERM_L_GREEN },
	{ PY_FOOD_MAX,   S("Full"),     TERM_L_GREEN },
	{ PY_FOOD_UPPER, S("Gorged"),   TERM_GREEN },
};

/* For the various TMD_* effects */
static const struct state_info effects[] =
{
	{ TMD_BLIND,     S("Blind"),      TERM_ORANGE },
	{ TMD_PARALYZED, S("Paralyzed!"), TERM_RED },
	{ TMD_CONFUSED,  S("Confused"),   TERM_ORANGE },
	{ TMD_AFRAID,    S("Afraid"),     TERM_ORANGE },
	{ TMD_IMAGE,     S("Halluc"),     TERM_ORANGE },
	{ TMD_POISONED,  S("Poisoned"),   TERM_ORANGE },
	{ TMD_PROTEVIL,  S("ProtEvil"),   TERM_L_GREEN },
	{ TMD_TELEPATHY, S("ESP"),        TERM_L_BLUE },
	{ TMD_INVULN,    S("Invuln"),     TERM_L_GREEN },
	{ TMD_HERO,      S("Hero"),       TERM_L_GREEN },
	{ TMD_SHERO,     S("Berserk"),    TERM_L_GREEN },
	{ TMD_SHIELD,    S("Shield"),     TERM_L_GREEN },
	{ TMD_BLESSED,   S("Blssd"),      TERM_L_GREEN },
	{ TMD_SINVIS,    S("SInvis"),     TERM_L_GREEN },
	{ TMD_SINFRA,    S("Infra"),      TERM_L_GREEN },
	{ TMD_OPP_ACID,  S("RAcid"),      TERM_SLATE },
	{ TMD_OPP_ELEC,  S("RElec"),      TERM_BLUE },
	{ TMD_OPP_FIRE,  S("RFire"),      TERM_RED },
	{ TMD_OPP_COLD,  S("RCold"),      TERM_WHITE },
	{ TMD_OPP_POIS,  S("RPois"),      TERM_GREEN },
	{ TMD_AMNESIA,   S("Amnesiac"),   TERM_ORANGE },
};

#define PRINT_STATE(sym, data, index, row, col) \
{ \
	size_t i; \
	\
	for (i = 0; i < N_ELEMENTS(data); i++) \
	{ \
		if (index sym data[i].value) \
		{ \
			if (data[i].str[0]) \
			{ \
				c_put_str(data[i].attr, data[i].str, row, col); \
				return data[i].len; \
			} \
			else \
			{ \
				return 0; \
			} \
		} \
	} \
}


/*
 * Print cut indicator.
 */
static size_t prt_cut(int row, int col)
{
	PRINT_STATE(>, cut_data, p_ptr->timed[TMD_CUT], row, col);
	return 0;
}


/*
 * Print stun indicator.
 */
static size_t prt_stun(int row, int col)
{
	PRINT_STATE(>, stun_data, p_ptr->timed[TMD_STUN], row, col);
	return 0;
}


/*
 * Prints status of hunger
 */
static size_t prt_hunger(int row, int col)
{
	PRINT_STATE(<, hunger_data, p_ptr->food, row, col);
	return 0;
}



/*
 * Prints Searching, Resting, or 'count' status
 * Display is always exactly 10 characters wide (see below)
 *
 * This function was a major bottleneck when resting, so a lot of
 * the text formatting code was optimized in place below.
 */
static size_t prt_state(int row, int col)
{
	byte attr = TERM_WHITE;

	char text[16] = "";


	/* Resting */
	if (p_ptr->resting)
	{
		int i;
		int n = p_ptr->resting;

		/* Start with "Rest" */
		my_strcpy(text, "Rest      ", sizeof(text));

		/* Extensive (timed) rest */
		if (n >= 1000)
		{
			i = n / 100;
			text[9] = '0';
			text[8] = '0';
			text[7] = I2D(i % 10);
			if (i >= 10)
			{
				i = i / 10;
				text[6] = I2D(i % 10);
				if (i >= 10)
				{
					text[5] = I2D(i / 10);
				}
			}
		}

		/* Long (timed) rest */
		else if (n >= 100)
		{
			i = n;
			text[9] = I2D(i % 10);
			i = i / 10;
			text[8] = I2D(i % 10);
			text[7] = I2D(i / 10);
		}

		/* Medium (timed) rest */
		else if (n >= 10)
		{
			i = n;
			text[9] = I2D(i % 10);
			text[8] = I2D(i / 10);
		}

		/* Short (timed) rest */
		else if (n > 0)
		{
			i = n;
			text[9] = I2D(i);
		}

		/* Rest until healed */
		else if (n == -1)
		{
			text[5] = text[6] = text[7] = text[8] = text[9] = '*';
		}

		/* Rest until done */
		else if (n == -2)
		{
			text[5] = text[6] = text[7] = text[8] = text[9] = '&';
		}
	}

	/* Repeating */
	else if (p_ptr->command_rep)
	{
		if (p_ptr->command_rep > 999)
			strnfmt(text, sizeof(text), "Rep. %3d00", p_ptr->command_rep / 100);
		else
			strnfmt(text, sizeof(text), "Repeat %3d", p_ptr->command_rep);
	}

	/* Searching */
	else if (p_ptr->searching)
	{
		my_strcpy(text, "Searching ", sizeof(text));
	}

	/* Display the info (or blanks) */
	c_put_str(attr, text, row, col);

	return strlen(text);
}


/*
 * Prints trap detection status
 */
static size_t prt_dtrap(int row, int col)
{
	byte info = cave_info2[p_ptr->py][p_ptr->px];

	/* The player is in a trap-detected grid */
	if (info & (CAVE2_DTRAP))
	{
		c_put_str(TERM_GREEN, "DTrap", row, col);
		return 5;
	}

	return 0;
}



/*
 * Print whether a character is studying or not.
 */
static size_t prt_study(int row, int col)
{
	if (p_ptr->new_spells)
	{
		char *text = format("Study (%d)", p_ptr->new_spells);
		put_str(text, row, col);
		return strlen(text) + 1;
	}

	return 0;
}



/*
 * Print all timed effects.
 */
static size_t prt_tmd(int row, int col)
{
	size_t i, len = 0;

	for (i = 0; i < N_ELEMENTS(effects); i++)
	{
		if (p_ptr->timed[effects[i].value])
		{
			c_put_str(effects[i].attr, effects[i].str, row, col + len);
			len += effects[i].len;
		}
	}

	return len;
}


/* Useful typedef */
typedef size_t status_f(int row, int col);

status_f *status_handlers[] =
{ prt_state, prt_cut, prt_stun, prt_hunger, prt_study, prt_tmd, prt_dtrap };


/*
 * Print the status line.
 */
static void update_statusline(ui_event_type type, ui_event_data *data, void *user)
{
	int row = Term->hgt - 1;
	int col = 13;
	size_t i;

	/* Clear the remainder of the line */
	prt("", row, col);

	/* Display those which need redrawing */
	for (i = 0; i < N_ELEMENTS(status_handlers); i++)
		col += status_handlers[i](row, col);
}



/* ------------------------------------------------------------------------
 * Subwindow displays
 * ------------------------------------------------------------------------ */

/* 
 * TRUE when we're supposed to display the equipment in the inventory 
 * window, or vice-versa.
 */
static bool flip_inven;

static void update_inven_subwindow(ui_event_type type, ui_event_data *data,
				       void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	if (!flip_inven)
		display_inven();
	else
		display_equip();


	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

static void update_equip_subwindow(ui_event_type type, ui_event_data *data,
				   void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	if (!flip_inven)
		display_equip();
	else
		display_inven();

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
	flip_inven = !flip_inven;
}

static void update_monlist_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	display_monlist();
	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


static void update_monster_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Display monster race info */
	if (p_ptr->monster_race_idx)
		display_roff(p_ptr->monster_race_idx);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


static void update_messages_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	int i;
	int w, h;
	int x, y;

	const char *msg;

	/* Activate */
	Term_activate(inv_term);

	/* Get size */
	Term_get_size(&w, &h);

	/* Dump messages */
	for (i = 0; i < h; i++)
	{
		byte color = message_color(i);
		u16b count = message_count(i);
		const char *str = message_str(i);

		if (count == 1)
			msg = str;
		else
			msg = format("%s <%dx>", str, count);

		Term_putstr(0, (h - 1) - i, -1, color, msg);


		/* Cursor */
		Term_locate(&x, &y);

		/* Clear to end of line */
		Term_erase(x, y, 255);
	}

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


/*
 * The "display_map()" function handles NULL arguments in a special manner.
 */
static void update_minimap_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Redraw map */
	display_map(NULL, NULL);
	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


/*
 * Hack -- display player in sub-windows (mode 0)
 */
static void update_player0_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Display flags */
	display_player(0);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

/*
 * Hack -- display player in sub-windows (mode 1)
 */
static void update_player1_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Display flags */
	display_player(1);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


/*
/*
 * Display the left-hand-side of the main term, in more compact fashion.
 */
static void update_player_compact_subwindow(ui_event_type type, ui_event_data *data, void *user)
{
	int row = 0;
	int col = 0;
	int i;

	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Race and Class */
	prt_field(p_name + rp_ptr->name, row++, col);
	prt_field(c_name + cp_ptr->name, row++, col);

	/* Title */
	prt_title(row++, col);

	/* Level/Experience */
	prt_level(row++, col);
	prt_exp(row++, col);

	/* Gold */
	prt_gold(row++, col);

	/* Equippy chars */
	prt_equippy(row++, col);

	/* All Stats */
	for (i = 0; i < A_MAX; i++) prt_stat(i, row++, col);

	/* Empty row */
	row++;

	/* Armor */
	prt_ac(row++, col);

	/* Hitpoints */
	prt_hp(row++, col);

	/* Spellpoints */
	prt_sp(row++, col);

	/* Monster health */
	prt_health(row++, col);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}



static void subwindow_flag_changed(int win_idx, u32b flag, bool new_state)
{
	void (*register_or_deregister)(ui_event_type type, ui_event_handler *fn, void *user);
	void (*set_register_or_deregister)(ui_event_type *type, size_t n_events, ui_event_handler *fn, void *user);

	/* Decide whether to register or deregister an evenrt handler */
	if (new_state == FALSE)
	{
		register_or_deregister = ui_event_deregister;
		set_register_or_deregister = ui_event_deregister_set;
	}
	else
	{
		register_or_deregister = ui_event_register;
		set_register_or_deregister = ui_event_register_set;
	}

	switch (flag)
	{
		case PW_INVEN:
		{
			register_or_deregister(ui_INVENTORY_CHANGED,
					       update_inven_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_EQUIP:
		{
			register_or_deregister(ui_EQUIPMENT_CHANGED,
					       update_equip_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_PLAYER_0:
		{
			set_register_or_deregister(player_events, 
						   N_ELEMENTS(player_events),
						   update_player0_subwindow,
						   angband_term[win_idx]);
			break;
		}

		case PW_PLAYER_1:
		{
			set_register_or_deregister(player_events, 
						   N_ELEMENTS(player_events),
						   update_player1_subwindow,
						   angband_term[win_idx]);
			break;
		}

		case PW_PLAYER_2:
		{
			set_register_or_deregister(player_events, 
						   N_ELEMENTS(player_events),
						   update_player_compact_subwindow,
						   angband_term[win_idx]);
			break;
		}

		case PW_MAP:
			/* Doesn't need to do anything atm. */
			break;


		case PW_MESSAGE:
		{
			register_or_deregister(ui_MONSTER_TARGET_CHANGED,
					       update_messages_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_OVERHEAD:
		{
			register_or_deregister(ui_MAP_CHANGED,
					       update_minimap_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_MONSTER:
		{
			register_or_deregister(ui_MONSTER_TARGET_CHANGED,
					       update_monster_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_MONLIST:
		{
			register_or_deregister(ui_EQUIPMENT_CHANGED,
					       update_monlist_subwindow,
					       angband_term[win_idx]);
			break;
		}
	}
}


/*
 * Set the flags for one Term, calling "subwindow_flag_changed" with each flag that
 * has changed setting so that it can do any housekeeping to do with 
 * siaplying hte new thing or no longer displaying the old one.
 */
static void subwindow_set_flags(int win_idx, u32b new_flags)
{
	term *old = Term;
	int i;

	/* Deal with the changed flags by seeing what's changed */
	for (i = 0; i < 32; i++)
	{
		/* Only process valid flags */
		if (window_flag_desc[i])
		{
			if ((new_flags & (1L << i)) != (op_ptr->window_flag[win_idx] & (1L << i)))
			{
				subwindow_flag_changed(win_idx, (1L << i), (new_flags & (1L << i)) != 0);
			}
		}
	}

	/* Store the new flags */
	op_ptr->window_flag[win_idx] = new_flags;
	
	/* Activate */
	Term_activate(angband_term[win_idx]);
	
	/* Erase */
	Term_clear();
	
	/* Refresh */
	Term_fresh();
			
	/* Restore */
	Term_activate(old);
}

/*
 * Called with an array of the new flags for all the subwindows, in order
 * to set them to the new values, with a chance to perform housekeeping.
 */
void subwindows_set_flags(u32b *new_flags, size_t n_subwindows)
{
	int j;

	for (j = 0; j < n_subwindows; j++)
	{
		/* Dead window */
		if (!angband_term[j]) continue;

		/* Ignore non-changes */
		if (op_ptr->window_flag[j] != new_flags[j])
		{
			subwindow_set_flags(j, new_flags[j]);
		}
	}

}

/* ------------------------------------------------------------------------
 * Temporary (hopefully) hackish solutions.
 * ------------------------------------------------------------------------ */
static void update_maps(ui_event_type type, ui_event_data *data, void *user)
{
	prt_map();
}

static void check_panel(ui_event_type type, ui_event_data *data, void *user)
{
	verify_panel();
}

/* ------------------------------------------------------------------------
 * Initialising
 * ------------------------------------------------------------------------ */
void init_display(void)
{
	/* Because of the "flexible" sidebar, all these things trigger
	   the same function. */
	ui_event_register_set(player_events, N_ELEMENTS(player_events),
			      update_sidebar, NULL);

	/* The flexible statusbar has similar requirements, so is
	   also trigger by a large set of events. */
	ui_event_register_set(statusline_events, N_ELEMENTS(statusline_events),
			      update_statusline, NULL);

	/* Player HP can optionally change the colour of the '@' now. */
	ui_event_register(ui_HP_CHANGED, hp_colour_change, NULL);

	/* Simplest way to keep the map up to date - will do for now */
	ui_event_register(ui_MAP_CHANGED, update_maps, NULL);

	/* Check if the panel should shift when the player's moved */
	ui_event_register(ui_PLAYER_MOVED, check_panel, NULL);
}
