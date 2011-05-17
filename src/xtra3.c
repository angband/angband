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
#include "buildid.h"
#include "button.h"
#include "cave.h"
#include "files.h"
#include "game-event.h"
#include "game-cmd.h"
#include "monster/monster.h"
#include "object/tvalsval.h"
#include "textui.h"
#include "ui-birth.h"

/* 
 * There are a few functions installed to be triggered by several 
 * of the basic player events.  For convenience, these have been grouped 
 * in this list.
 */
game_event_type player_events[] =
{
	EVENT_RACE_CLASS,
	EVENT_PLAYERTITLE,
	EVENT_EXPERIENCE,
	EVENT_PLAYERLEVEL,
	EVENT_GOLD,
	EVENT_EQUIPMENT,  /* For equippy chars */
	EVENT_STATS,
	EVENT_HP,
	EVENT_MANA,
	EVENT_AC,

	EVENT_MONSTERHEALTH,

	EVENT_PLAYERSPEED,
	EVENT_DUNGEONLEVEL,
};

game_event_type statusline_events[] =
{
	EVENT_STUDYSTATUS,
	EVENT_STATUS,
	EVENT_DETECTIONSTATUS,
	EVENT_STATE,
	EVENT_MOUSEBUTTONS
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

		if (bonus >= 220)
			strnfmt(out_val, out_len, "18/***");
		else if (bonus >= 100)
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
static void prt_field(const char *info, int row, int col)
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
		cnv_stat(p_ptr->state.stat_use[stat], tmp, sizeof(tmp));
		c_put_str(TERM_YELLOW, tmp, row, col + 6);
	}

	/* Display "healthy" stat */
	else
	{
		put_str(stat_names[stat], row, col);
		cnv_stat(p_ptr->state.stat_use[stat], tmp, sizeof(tmp));
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
	const char *p;

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
		p = p_ptr->class->title[(p_ptr->lev - 1) / 5];
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
	if (tile_width > 1 || tile_height > 1) return;

	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
		/* Object */
		o_ptr = &p_ptr->inventory[i];

		if (o_ptr->kind) {
			c = object_char(o_ptr);
			a = object_attr(o_ptr);
		} else {
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
	strnfmt(tmp, sizeof(tmp), "%5d", p_ptr->state.dis_ac + p_ptr->state.dis_to_a);
	c_put_str(TERM_L_GREEN, tmp, row, col + 7);
}

/*
 * Prints Cur hit points
 */
static void prt_hp(int row, int col)
{
	char cur_hp[32], max_hp[32];
	byte color = player_hp_attr(p_ptr);

	put_str("HP ", row, col);

	strnfmt(max_hp, sizeof(max_hp), "%4d", p_ptr->mhp);
	strnfmt(cur_hp, sizeof(cur_hp), "%4d", p_ptr->chp);
	
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
	byte color = player_sp_attr(p_ptr);

	/* Do not show mana unless we have some */
	if (!p_ptr->msp) return;

	put_str("SP ", row, col);

	strnfmt(max_sp, sizeof(max_sp), "%4d", p_ptr->msp);
	strnfmt(cur_sp, sizeof(cur_sp), "%4d", p_ptr->csp);

	/* Show mana */
	c_put_str(color, cur_sp, row, col + 3);
	c_put_str(TERM_WHITE, "/", row, col + 7);
	c_put_str(TERM_L_GREEN, max_sp, row, col + 8);
}

/*
 * Calculate the monster bar color separately, for ports.
 */
byte monster_health_attr(void)
{
	byte attr = TERM_WHITE;
	
	/* Not tracking */
	if (!p_ptr->health_who)
		attr = TERM_DARK;

	/* Tracking an unseen, hallucinatory, or dead monster */
	else if ((!cave_monster(cave, p_ptr->health_who)->ml) ||
			(p_ptr->timed[TMD_IMAGE]) ||
			(cave_monster(cave, p_ptr->health_who)->hp < 0))
	{
		/* The monster health is "unknown" */
		attr = TERM_WHITE;
	}
	
	else
	{
		struct monster *mon = cave_monster(cave, p_ptr->health_who);
		int pct;

		/* Default to almost dead */
		attr = TERM_RED;

		/* Extract the "percent" of health */
		pct = 100L * mon->hp / mon->maxhp;

		/* Badly wounded */
		if (pct >= 10) attr = TERM_L_RED;

		/* Wounded */
		if (pct >= 25) attr = TERM_ORANGE;

		/* Somewhat Wounded */
		if (pct >= 60) attr = TERM_YELLOW;

		/* Healthy */
		if (pct >= 100) attr = TERM_L_GREEN;

		/* Afraid */
		if (mon->m_timed[MON_TMD_FEAR]) attr = TERM_VIOLET;

		/* Confused */
		if (mon->m_timed[MON_TMD_CONF]) attr = TERM_UMBER;

		/* Stunned */
		if (mon->m_timed[MON_TMD_STUN]) attr = TERM_L_BLUE;

		/* Asleep */
		if (mon->m_timed[MON_TMD_SLEEP]) attr = TERM_BLUE;
	}
	
	return attr;
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
	byte attr = monster_health_attr();
	struct monster *mon;
	
	/* Not tracking */
	if (!p_ptr->health_who)
	{
		/* Erase the health bar */
		Term_erase(col, row, 12);
		return;
	}

	mon = cave_monster(cave, p_ptr->health_who);

	/* Tracking an unseen, hallucinatory, or dead monster */
	if ((!mon->ml) || /* Unseen */
			(p_ptr->timed[TMD_IMAGE]) || /* Hallucination */
			(mon->hp < 0)) /* Dead (?) */
	{
		/* The monster health is "unknown" */
		Term_putstr(col, row, 12, attr, "[----------]");
	}

	/* Tracking a visible monster */
	else
	{
		int pct, len;

		monster_type *m_ptr = cave_monster(cave, p_ptr->health_who);

		/* Extract the "percent" of health */
		pct = 100L * m_ptr->hp / m_ptr->maxhp;

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
	int i = p_ptr->state.speed;

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
	else
	{
		strnfmt(depths, sizeof(depths), "%d' (L%d)",
		        p_ptr->depth * 50, p_ptr->depth);
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
static void prt_race(int row, int col) { prt_field(p_ptr->race->name, row, col); }
static void prt_class(int row, int col) { prt_field(p_ptr->class->name, row, col); }


/*
 * Struct of sidebar handlers.
 */
static const struct side_handler_t
{
	void (*hook)(int, int);	 /* int row, int col */
	int priority;		 /* 1 is most important (always displayed) */
	game_event_type type;	 /* PR_* flag this corresponds to */
} side_handlers[] =
{
	{ prt_race,    19, EVENT_RACE_CLASS },
	{ prt_title,   18, EVENT_PLAYERTITLE },
	{ prt_class,   22, EVENT_RACE_CLASS },
	{ prt_level,   10, EVENT_PLAYERLEVEL },
	{ prt_exp,     16, EVENT_EXPERIENCE },
	{ prt_gold,    11, EVENT_GOLD },
	{ prt_equippy, 17, EVENT_EQUIPMENT },
	{ prt_str,      6, EVENT_STATS },
	{ prt_int,      5, EVENT_STATS },
	{ prt_wis,      4, EVENT_STATS },
	{ prt_dex,      3, EVENT_STATS },
	{ prt_con,      2, EVENT_STATS },
	{ prt_chr,      1, EVENT_STATS },
	{ NULL,        15, 0 },
	{ prt_ac,       7, EVENT_AC },
	{ prt_hp,       8, EVENT_HP },
	{ prt_sp,       9, EVENT_MANA },
	{ NULL,        21, 0 },
	{ prt_health,  12, EVENT_MONSTERHEALTH },
	{ NULL,        20, 0 },
	{ NULL,        22, 0 },
	{ prt_speed,   13, EVENT_PLAYERSPEED }, /* Slow (-NN) / Fast (+NN) */
	{ prt_depth,   14, EVENT_DUNGEONLEVEL }, /* Lev NNN / NNNN ft */
};


/*
 * This prints the sidebar, using a clever method which means that it will only
 * print as much as can be displayed on <24-line screens.
 *
 * Each row is given a priority; the least important higher numbers and the most
 * important lower numbers.  As the screen gets smaller, the rows start to
 * disappear in the order of lowest to highest importance.
 */
static void update_sidebar(game_event_type type, game_event_data *data, void *user)
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

static void hp_colour_change(game_event_type type, game_event_data *data, void *user)
{
	/*
	 * hack:  redraw player, since the player's color
	 * now indicates approximate health.  Note that
	 * using this command when graphics mode is on
	 * causes the character to be a black square.
	 */
	if ((OPT(hp_changes_color)) && (arg_graphics == GRAPHICS_NONE))
	{
		cave_light_spot(cave, p_ptr->py, p_ptr->px);
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
	{ TMD_TERROR,    S("Terror"),     TERM_RED },
	{ TMD_IMAGE,     S("Halluc"),     TERM_ORANGE },
	{ TMD_POISONED,  S("Poisoned"),   TERM_ORANGE },
	{ TMD_PROTEVIL,  S("ProtEvil"),   TERM_L_GREEN },
	{ TMD_SPRINT,    S("Sprint"),     TERM_L_GREEN },
	{ TMD_TELEPATHY, S("ESP"),        TERM_L_BLUE },
	{ TMD_INVULN,    S("Invuln"),     TERM_L_GREEN },
	{ TMD_HERO,      S("Hero"),       TERM_L_GREEN },
	{ TMD_SHERO,     S("Berserk"),    TERM_L_GREEN },
	{ TMD_BOLD,      S("Bold"),       TERM_L_GREEN },
	{ TMD_STONESKIN, S("Stone"),      TERM_L_GREEN },
	{ TMD_SHIELD,    S("Shield"),     TERM_L_GREEN },
	{ TMD_BLESSED,   S("Blssd"),      TERM_L_GREEN },
	{ TMD_SINVIS,    S("SInvis"),     TERM_L_GREEN },
	{ TMD_SINFRA,    S("Infra"),      TERM_L_GREEN },
	{ TMD_OPP_ACID,  S("RAcid"),      TERM_SLATE },
	{ TMD_OPP_ELEC,  S("RElec"),      TERM_BLUE },
	{ TMD_OPP_FIRE,  S("RFire"),      TERM_RED },
	{ TMD_OPP_COLD,  S("RCold"),      TERM_WHITE },
	{ TMD_OPP_POIS,  S("RPois"),      TERM_GREEN },
	{ TMD_OPP_CONF,  S("RConf"),      TERM_VIOLET },
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
 * Print recall status.
 */
static size_t prt_recall(int row, int col)
{
	if (p_ptr->word_recall)
	{
		c_put_str(TERM_WHITE, "Recall", row, col);
		return sizeof "Recall";
	}

	return 0;
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
		
		/* Rest until HP or SP filled */
		else if (n == -3)
		{
			text[5] = text[6] = text[7] = text[8] = text[9] = '!';
		}

	}

	/* Repeating */
	else if (cmd_get_nrepeats())
	{
		int nrepeats = cmd_get_nrepeats();

		if (nrepeats > 999)
			strnfmt(text, sizeof(text), "Rep. %3d00", nrepeats / 100);
		else
			strnfmt(text, sizeof(text), "Repeat %3d", nrepeats);
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
	byte info = cave->info2[p_ptr->py][p_ptr->px];

	/* The player is in a trap-detected grid */
	if (info & (CAVE2_DTRAP))
	{
		/* The player is on the border */
		if (dtrap_edge(p_ptr->py, p_ptr->px))
			c_put_str(TERM_YELLOW, "DTrap", row, col);
		else
			c_put_str(TERM_L_GREEN, "DTrap", row, col);

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

/**
 * Print "unignoring" status
 */
static size_t prt_unignore(int row, int col)
{
	if (p_ptr->unignoring) {
		const char *str = "Unignoring";
		put_str(str, row, col);
		return strlen(str);
	}

	return 0;
}

/*
 * Print mouse buttons
 */
static size_t prt_buttons(int row, int col)
{
	if (OPT(mouse_buttons))
		return button_print(row, col);

	return 0;
}


/* Useful typedef */
typedef size_t status_f(int row, int col);

status_f *status_handlers[] =
{ prt_buttons, prt_unignore, prt_recall, prt_state, prt_cut, prt_stun,
  prt_hunger, prt_study, prt_tmd, prt_dtrap };


/*
 * Print the status line.
 */
static void update_statusline(game_event_type type, game_event_data *data, void *user)
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
 * Map redraw.
 * ------------------------------------------------------------------------ */
#if 0
static void trace_map_updates(game_event_type type, game_event_data *data, void *user)
{
	if (data->point.x == -1 && data->point.y == -1)
	{
		printf("Redraw whole map\n");
	}
	else
	{
		printf("Redraw (%i, %i)\n", data->point.x, data->point.y);
	}
}
#endif

static void update_maps(game_event_type type, game_event_data *data, void *user)
{
	term *t = user;

	/* This signals a whole-map redraw. */
	if (data->point.x == -1 && data->point.y == -1)
	{
		prt_map();
	}
	/* Single point to be redrawn */
	else
	{
		grid_data g;
		byte a, ta;
		char c, tc;
		
		int ky, kx;
		int vy, vx;
		
		/* Location relative to panel */
		ky = data->point.y - t->offset_y;
		kx = data->point.x - t->offset_x;

		if (t == angband_term[0])
		{
			/* Verify location */
			if ((ky < 0) || (ky >= SCREEN_HGT)) return;
			
			/* Verify location */
			if ((kx < 0) || (kx >= SCREEN_WID)) return;
			
			/* Location in window */
			vy = ky + ROW_MAP;
			vx = kx + COL_MAP;

		      if (tile_width > 1)
		      {
			      vx += (tile_width - 1) * kx;
		      }
		      if (tile_height > 1)
		      {
			      vy += (tile_height - 1) * ky;
		      }
		}
		else
		{
			if (tile_width > 1)
			{
			        kx += (tile_width - 1) * kx;
			}
			if (tile_height > 1)
			{
			        ky += (tile_height - 1) * ky;
			}
			
			/* Verify location */
			if ((ky < 0) || (ky >= t->hgt)) return;
			if ((kx < 0) || (kx >= t->wid)) return;
			
			/* Location in window */
			vy = ky;
			vx = kx;
		}

		
		/* Redraw the grid spot */
		map_info(data->point.y, data->point.x, &g);
		grid_data_as_text(&g, &a, &c, &ta, &tc);
		Term_queue_char(t, vx, vy, a, c, ta, tc);
#if 0
		/* Plot 'spot' updates in light green to make them visible */
		Term_queue_char(t, vx, vy, TERM_L_GREEN, c, ta, tc);
#endif
		
		if ((tile_width > 1) || (tile_height > 1))
		{
		        Term_big_queue_char(t, vx, vy, a, c, TERM_WHITE, ' ');
		}
	}
}

/* ------------------------------------------------------------------------
 * Subwindow displays
 * ------------------------------------------------------------------------ */

/* 
 * TRUE when we're supposed to display the equipment in the inventory 
 * window, or vice-versa.
 */
static bool flip_inven;

static void update_inven_subwindow(game_event_type type, game_event_data *data,
				       void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	if (!flip_inven)
		show_inven(OLIST_WINDOW | OLIST_WEIGHT | OLIST_QUIVER);
	else
		show_equip(OLIST_WINDOW | OLIST_WEIGHT);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

static void update_equip_subwindow(game_event_type type, game_event_data *data,
				   void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	if (!flip_inven)
		show_equip(OLIST_WINDOW | OLIST_WEIGHT);
	else
		show_inven(OLIST_WINDOW | OLIST_WEIGHT | OLIST_QUIVER);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
	term *old = Term;
	int i;

	/* Change the actual setting */
	flip_inven = !flip_inven;

	/* Redraw any subwindows showing the inventory/equipment lists */
	for (i = 0; i < ANGBAND_TERM_MAX; i++)
	{
		Term_activate(angband_term[i]); 

		if (op_ptr->window_flag[i] & PW_INVEN)
		{
			if (!flip_inven)
				show_inven(OLIST_WINDOW | OLIST_WEIGHT | OLIST_QUIVER);
			else
				show_equip(OLIST_WINDOW | OLIST_WEIGHT);
			
			Term_fresh();
		}
		else if (op_ptr->window_flag[i] & PW_EQUIP)
		{
			if (!flip_inven)
				show_equip(OLIST_WINDOW | OLIST_WEIGHT);
			else
				show_inven(OLIST_WINDOW | OLIST_WEIGHT | OLIST_QUIVER);
			
			Term_fresh();
		}
	}

	Term_activate(old);
}

static void update_itemlist_subwindow(game_event_type type, game_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	display_itemlist();
	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}

static void update_monlist_subwindow(game_event_type type, game_event_data *data, void *user)
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


static void update_monster_subwindow(game_event_type type, game_event_data *data, void *user)
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


static void update_object_subwindow(game_event_type type, game_event_data *data, void *user)
{
	term *old = Term;
	term *inv_term = user;
	
	/* Activate */
	Term_activate(inv_term);
	
	if (p_ptr->object_idx != NO_OBJECT)
		display_object_idx_recall(p_ptr->object_idx);
	else if(p_ptr->object_kind_idx != NO_OBJECT)
		display_object_kind_recall(p_ptr->object_kind_idx);
	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


static void update_messages_subwindow(game_event_type type, game_event_data *data, void *user)
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

static struct minimap_flags
{
	int win_idx;
	bool needs_redraw;
} minimap_data[ANGBAND_TERM_MAX];

static void update_minimap_subwindow(game_event_type type, game_event_data *data, void *user)
{
	struct minimap_flags *flags = user;

	if (type == EVENT_MAP)
	{
		/* Set flag if whole-map redraw. */
		if (data->point.x == -1 && data->point.y == -1)
			flags->needs_redraw = TRUE;
	}
	else if (type == EVENT_END)
	{
		term *old = Term;
		term *t = angband_term[flags->win_idx];
		
		/* Activate */
		Term_activate(t);

		/* If whole-map redraw, clear window first. */
		if (flags->needs_redraw)
			Term_clear();

		/* Redraw map */
		display_map(NULL, NULL);
		Term_fresh();
		
		/* Restore */
		Term_activate(old);

		flags->needs_redraw = FALSE;
	}
}


/*
 * Hack -- display player in sub-windows (mode 0)
 */
static void update_player0_subwindow(game_event_type type, game_event_data *data, void *user)
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
static void update_player1_subwindow(game_event_type type, game_event_data *data, void *user)
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
 * Display the left-hand-side of the main term, in more compact fashion.
 */
static void update_player_compact_subwindow(game_event_type type, game_event_data *data, void *user)
{
	int row = 0;
	int col = 0;
	int i;

	term *old = Term;
	term *inv_term = user;

	/* Activate */
	Term_activate(inv_term);

	/* Race and Class */
	prt_field(p_ptr->race->name, row++, col);
	prt_field(p_ptr->class->name, row++, col);

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


static void flush_subwindow(game_event_type type, game_event_data *data, void *user)
{
	term *old = Term;
	term *t = user;

	/* Activate */
	Term_activate(t);

	Term_fresh();
	
	/* Restore */
	Term_activate(old);
}


static void subwindow_flag_changed(int win_idx, u32b flag, bool new_state)
{
	void (*register_or_deregister)(game_event_type type, game_event_handler *fn, void *user);
	void (*set_register_or_deregister)(game_event_type *type, size_t n_events, game_event_handler *fn, void *user);

	/* Decide whether to register or deregister an evenrt handler */
	if (new_state == FALSE)
	{
		register_or_deregister = event_remove_handler;
		set_register_or_deregister = event_remove_handler_set;
	}
	else
	{
		register_or_deregister = event_add_handler;
		set_register_or_deregister = event_add_handler_set;
	}

	switch (flag)
	{
		case PW_INVEN:
		{
			register_or_deregister(EVENT_INVENTORY,
					       update_inven_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_EQUIP:
		{
			register_or_deregister(EVENT_EQUIPMENT,
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
		{
			register_or_deregister(EVENT_MAP,
					       update_maps,
					       angband_term[win_idx]);

			register_or_deregister(EVENT_END,
					       flush_subwindow,
					       angband_term[win_idx]);
			break;
		}


		case PW_MESSAGE:
		{
			register_or_deregister(EVENT_MESSAGE,
					       update_messages_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_OVERHEAD:
		{
			minimap_data[win_idx].win_idx = win_idx;

			register_or_deregister(EVENT_MAP,
					       update_minimap_subwindow,
					       &minimap_data[win_idx]);

			register_or_deregister(EVENT_END,
					       update_minimap_subwindow,
					       &minimap_data[win_idx]);
			break;
		}

		case PW_MONSTER:
		{
			register_or_deregister(EVENT_MONSTERTARGET,
					       update_monster_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_OBJECT:
		{
			register_or_deregister(EVENT_OBJECTTARGET,
						   update_object_subwindow,
						   angband_term[win_idx]);
			break;
		}

		case PW_MONLIST:
		{
			register_or_deregister(EVENT_MONSTERLIST,
					       update_monlist_subwindow,
					       angband_term[win_idx]);
			break;
		}

		case PW_ITEMLIST:
		{
			register_or_deregister(EVENT_ITEMLIST,
						   update_itemlist_subwindow,
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
	size_t j;

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
 * Showing and updating the splash screen.
 * ------------------------------------------------------------------------ */
/*
 * Hack -- Explain a broken "lib" folder and quit (see below).
 */
static void init_angband_aux(const char *why)
{
	quit_fmt("%s\n\n%s", why,
	         "The 'lib' directory is probably missing or broken.\n"
	         "Perhaps the archive was not extracted correctly.\n"
	         "See the 'readme.txt' file for more information.");
}

/*
 * Hack -- take notes on line 23
 */
static void splashscreen_note(game_event_type type, game_event_data *data, void *user)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, format("[%s]", data->string));
	Term_fresh();
}

static void show_splashscreen(game_event_type type, game_event_data *data, void *user)
{
	ang_file *fp;

	char buf[1024];

	/*** Verify the "news" file ***/

	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "news.txt");
	if (!file_exists(buf))
	{
		char why[1024];

		/* Crash and burn */
		strnfmt(why, sizeof(why), "Cannot access the '%s' file!", buf);
		init_angband_aux(why);
	}


	/*** Display the "news" file ***/

	Term_clear();

	/* Open the News file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "news.txt");
	fp = file_open(buf, MODE_READ, -1);

	text_out_hook = text_out_to_screen;

	/* Dump */
	if (fp)
	{
		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
		{
			char *version_marker = strstr(buf, "$VERSION");
			if (version_marker)
			{
				ptrdiff_t pos = version_marker - buf;
				strnfmt(version_marker, sizeof(buf) - pos, "%-8s", buildver);
			}

			text_out_e("%s", buf);
			text_out("\n");
		}

		file_close(fp);
	}

	/* Flush it */
	Term_fresh();
}


/* ------------------------------------------------------------------------
 * Temporary (hopefully) hackish solutions.
 * ------------------------------------------------------------------------ */
static void check_panel(game_event_type type, game_event_data *data, void *user)
{
	verify_panel();
}

static void see_floor_items(game_event_type type, game_event_data *data, void *user)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	size_t floor_num = 0;
	int floor_list[MAX_FLOOR_STACK + 1];
	bool blind = ((p_ptr->timed[TMD_BLIND]) || (no_light()));

	const char *p = "see";
	int can_pickup = 0;
	size_t i;

	/* Scan all marked objects in the grid */
	floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), py, px, 0x03);
	if (floor_num == 0) return;

	for (i = 0; i < floor_num; i++)
	    can_pickup += inven_carry_okay(object_byid(floor_list[i]));
	
	/* One object */
	if (floor_num == 1)
	{
		/* Get the object */
		object_type *o_ptr = object_byid(floor_list[0]);
		char o_name[80];

		if (!can_pickup)
			p = "have no room for";
		else if (blind)
			p = "feel";

		/* Describe the object.  Less detail if blind. */
		if (blind)
			object_desc(o_name, sizeof(o_name), o_ptr,
					ODESC_PREFIX | ODESC_BASE);
		else
			object_desc(o_name, sizeof(o_name), o_ptr,
					ODESC_PREFIX | ODESC_FULL);

		/* Message */
		message_flush();
		msg("You %s %s.", p, o_name);
	}
	else
	{
		ui_event e;

		if (!can_pickup)	p = "have no room for the following objects";
		else if (blind)     p = "feel something on the floor";

		/* Display objects on the floor */
		screen_save();
		show_floor(floor_list, floor_num, (OLIST_WEIGHT));
		prt(format("You %s: ", p), 0, 0);

		/* Wait for it.  Use key as next command. */
		e = inkey_ex();
		Term_event_push(&e);

		/* Restore screen */
		screen_load();
	}
}

extern game_event_handler ui_enter_birthscreen;

/* ------------------------------------------------------------------------
 * Initialising
 * ------------------------------------------------------------------------ */
static void ui_enter_init(game_event_type type, game_event_data *data, void *user)
{
	show_splashscreen(type, data, user);

	/* Set up our splashscreen handlers */
	event_add_handler(EVENT_INITSTATUS, splashscreen_note, NULL);
}

static void ui_leave_init(game_event_type type, game_event_data *data, void *user)
{
	/* Remove our splashscreen handlers */
	event_remove_handler(EVENT_INITSTATUS, splashscreen_note, NULL);
}

static void ui_enter_game(game_event_type type, game_event_data *data, void *user)
{
	/* Because of the "flexible" sidebar, all these things trigger
	   the same function. */
	event_add_handler_set(player_events, N_ELEMENTS(player_events),
			      update_sidebar, NULL);

	/* The flexible statusbar has similar requirements, so is
	   also trigger by a large set of events. */
	event_add_handler_set(statusline_events, N_ELEMENTS(statusline_events),
			      update_statusline, NULL);

	/* Player HP can optionally change the colour of the '@' now. */
	event_add_handler(EVENT_HP, hp_colour_change, NULL);

	/* Simplest way to keep the map up to date - will do for now */
	event_add_handler(EVENT_MAP, update_maps, angband_term[0]);
#if 0
	event_add_handler(EVENT_MAP, trace_map_updates, angband_term[0]);
#endif
	/* Check if the panel should shift when the player's moved */
	event_add_handler(EVENT_PLAYERMOVED, check_panel, NULL);
	event_add_handler(EVENT_SEEFLOOR, see_floor_items, NULL);
}

static void ui_leave_game(game_event_type type, game_event_data *data, void *user)
{
	/* Because of the "flexible" sidebar, all these things trigger
	   the same function. */
	event_remove_handler_set(player_events, N_ELEMENTS(player_events),
			      update_sidebar, NULL);

	/* The flexible statusbar has similar requirements, so is
	   also trigger by a large set of events. */
	event_remove_handler_set(statusline_events, N_ELEMENTS(statusline_events),
			      update_statusline, NULL);

	/* Player HP can optionally change the colour of the '@' now. */
	event_remove_handler(EVENT_HP, hp_colour_change, NULL);

	/* Simplest way to keep the map up to date - will do for now */
	event_remove_handler(EVENT_MAP, update_maps, angband_term[0]);
#if 0
	event_remove_handler(EVENT_MAP, trace_map_updates, angband_term[0]);
#endif
	/* Check if the panel should shift when the player's moved */
	event_remove_handler(EVENT_PLAYERMOVED, check_panel, NULL);
	event_remove_handler(EVENT_SEEFLOOR, see_floor_items, NULL);
}

errr textui_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_BIRTH)
		return get_birth_command(wait);
	else if (context == CMD_GAME)
		textui_process_command(!wait);

	/* If we've reached here, we haven't got a command. */
	return 1;
}


void init_display(void)
{
	event_add_handler(EVENT_ENTER_INIT, ui_enter_init, NULL);
	event_add_handler(EVENT_LEAVE_INIT, ui_leave_init, NULL);

	event_add_handler(EVENT_ENTER_GAME, ui_enter_game, NULL);
	event_add_handler(EVENT_LEAVE_GAME, ui_leave_game, NULL);

	ui_init_birthstate_handlers();
}


/* Return a random hint from the global hints list */
char* random_hint(void)
{
	struct hint *v, *r = NULL;
	int n;
	for (v = hints, n = 1; v; v = v->next, n++)
		if (one_in_(n))
			r = v;
	return r->hint;
}
