/*
 * File: files.c
 * Purpose: Various file-related activities, poorly organised
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
#include "buildid.h"
#include "cave.h"
#include "cmds.h"
#include "files.h"
#include "game-cmd.h"
#include "history.h"
#include "object/tvalsval.h"
#include "object/pval.h"
#include "option.h"
#include "savefile.h"
#include "ui-menu.h"

#define MAX_PANEL 12

/*
 * Returns a "rating" of x depending on y, and sets "attr" to the
 * corresponding "attribute".
 */
static const char *likert(int x, int y, byte *attr)
{
	/* Paranoia */
	if (y <= 0) y = 1;

	/* Negative value */
	if (x < 0)
	{
		*attr = TERM_RED;
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
		case 0:
		case 1:
		{
			*attr = TERM_RED;
			return ("Bad");
		}
		case 2:
		{
			*attr = TERM_RED;
			return ("Poor");
		}
		case 3:
		case 4:
		{
			*attr = TERM_YELLOW;
			return ("Fair");
		}
		case 5:
		{
			*attr = TERM_YELLOW;
			return ("Good");
		}
		case 6:
		{
			*attr = TERM_YELLOW;
			return ("Very Good");
		}
		case 7:
		case 8:
		{
			*attr = TERM_L_GREEN;
			return ("Excellent");
		}
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			*attr = TERM_L_GREEN;
			return ("Superb");
		}
		case 14:
		case 15:
		case 16:
		case 17:
		{
			*attr = TERM_L_GREEN;
			return ("Heroic");
		}
		default:
		{
			*attr = TERM_L_GREEN;
			return ("Legendary");
		}
	}
}


/*
 * Obtain the "flags" for the player as if he was an item
 */
void player_flags(bitflag f[OF_SIZE])
{
	/* Add racial flags */
	memcpy(f, p_ptr->race->flags, sizeof(p_ptr->race->flags));

	/* Some classes become immune to fear at a certain plevel */
	if (player_has(PF_BRAVERY_30) && p_ptr->lev >= 30)
		of_on(f, OF_RES_FEAR);
}


/*
 * Equippy chars
 */
static void display_player_equippy(int y, int x)
{
	int i;

	byte a;
	char c;

	object_type *o_ptr;


	/* Dump equippy chars */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; ++i)
	{
		/* Object */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty objects */
		if (!o_ptr->kind) continue;

		/* Get attr/char for display */
		a = object_attr(o_ptr);
		c = object_char(o_ptr);

		/* Dump */
		if ((tile_width == 1) && (tile_height == 1))
		{
		        Term_putch(x+i-INVEN_WIELD, y, a, c);
		}
	}
}

/*
 * List of resistances and abilities to display
 */
#define RES_ROWS 9
struct player_flag_record
{
	const char name[7];		/* Name of resistance/ability */
	int res_flag;			/* resistance flag bit */
	int im_flag;			/* corresponding immunity bit, if any */
	int vuln_flag;			/* corresponding vulnerability flag, if any */
};

static const struct player_flag_record player_flag_table[RES_ROWS*4] =
{
	{ "rAcid",	OF_RES_ACID,    OF_IM_ACID, OF_VULN_ACID },
	{ "rElec",	OF_RES_ELEC,    OF_IM_ELEC, OF_VULN_ELEC },
	{ "rFire",	OF_RES_FIRE,    OF_IM_FIRE, OF_VULN_FIRE },
	{ "rCold",	OF_RES_COLD,    OF_IM_COLD, OF_VULN_COLD },
	{ "rPois",	OF_RES_POIS,    FLAG_END,   FLAG_END },
	{ "rLite",	OF_RES_LIGHT,   FLAG_END,   FLAG_END },
	{ "rDark",	OF_RES_DARK,    FLAG_END,   FLAG_END },
	{ "Sound",	OF_RES_SOUND,   FLAG_END,   FLAG_END },
	{ "Shard",	OF_RES_SHARD,   FLAG_END,   FLAG_END },

	{ "Nexus",	OF_RES_NEXUS,   FLAG_END,   FLAG_END },
	{ "Nethr",	OF_RES_NETHR,   FLAG_END,   FLAG_END },
	{ "Chaos",	OF_RES_CHAOS,   FLAG_END,   FLAG_END },
	{ "Disen",	OF_RES_DISEN,   FLAG_END,   FLAG_END },
	{ "Feath",	OF_FEATHER,     FLAG_END,   FLAG_END },
	{ "pFear",	OF_RES_FEAR,    FLAG_END,   FLAG_END },
	{ "pBlnd",	OF_RES_BLIND,   FLAG_END,   FLAG_END },
	{ "pConf",	OF_RES_CONFU,   FLAG_END,   FLAG_END },
	{ "pStun",	OF_RES_STUN,	FLAG_END,   FLAG_END },

	{ "Light",	OF_LIGHT,       FLAG_END,   FLAG_END },
	{ "Regen",	OF_REGEN,       FLAG_END,   FLAG_END },
	{ "  ESP",	OF_TELEPATHY,   FLAG_END,   FLAG_END },
	{ "Invis",	OF_SEE_INVIS,   FLAG_END,   FLAG_END },
	{ "FrAct",	OF_FREE_ACT,    FLAG_END,   FLAG_END },
	{ "HLife",	OF_HOLD_LIFE,   FLAG_END,   FLAG_END },
	{ "Stea.",	OF_STEALTH,     FLAG_END,   FLAG_END },
	{ "Sear.",	OF_SEARCH,      FLAG_END,   FLAG_END },
	{ "Infra",	OF_INFRA,       FLAG_END,   FLAG_END },

	{ "Tunn.",	OF_TUNNEL,      FLAG_END,   FLAG_END },
	{ "Speed",	OF_SPEED,       FLAG_END,   FLAG_END },
	{ "Blows",	OF_BLOWS,       FLAG_END,   FLAG_END },
	{ "Shots",	OF_SHOTS,       FLAG_END,   FLAG_END },
	{ "Might",	OF_MIGHT,       FLAG_END,   FLAG_END },
	{ "S.Dig",	OF_SLOW_DIGEST, FLAG_END,   FLAG_END },
	{ "ImpHP",	OF_IMPAIR_HP,   FLAG_END,   FLAG_END },
	{ " Fear",	OF_AFRAID,      FLAG_END,   FLAG_END },
	{ "Aggrv",	OF_AGGRAVATE,   FLAG_END,   FLAG_END },
};

#define RES_COLS (5 + 2 + INVEN_TOTAL - INVEN_WIELD)
static const region resist_region[] =
{
	{  0*(RES_COLS+1), 10, RES_COLS, RES_ROWS+2 },
	{  1*(RES_COLS+1), 10, RES_COLS, RES_ROWS+2 },
	{  2*(RES_COLS+1), 10, RES_COLS, RES_ROWS+2 },
	{  3*(RES_COLS+1), 10, RES_COLS, RES_ROWS+2 },
};

static void display_resistance_panel(const struct player_flag_record *resists,
									size_t size, const region *bounds) 
{
	size_t i, j;
	int col = bounds->col;
	int row = bounds->row;
	Term_putstr(col, row++, RES_COLS, TERM_WHITE, "      abcdefghijkl@");
	for (i = 0; i < size-3; i++, row++)
	{
		byte name_attr = TERM_WHITE;
		Term_gotoxy(col+6, row);
		/* repeated extraction of flags is inefficient but more natural */
		for (j = INVEN_WIELD; j <= INVEN_TOTAL; j++)
		{
			object_type *o_ptr = &p_ptr->inventory[j];
			bitflag f[OF_SIZE];

			byte attr = TERM_WHITE | (j % 2) * 8; /* alternating columns */
			char sym = '.';

			bool res, imm, vuln;

			/* Wipe flagset */
			of_wipe(f);

			if (j < INVEN_TOTAL && o_ptr->kind)
			{
				object_flags_known(o_ptr, f);
			}
			else if (j == INVEN_TOTAL)
			{
				player_flags(f);

				/* If the race has innate infravision/digging, force the corresponding flag
				   here.  If we set it in player_flags(), then all callers of that
				   function will think the infravision is caused by equipment. */
				if (p_ptr->race->infra > 0)
					of_on(f, OF_INFRA);
				if (p_ptr->race->r_skills[SKILL_DIGGING] > 0)
					of_on(f, OF_TUNNEL);
			}

			res = of_has(f, resists[i].res_flag);
			imm = of_has(f, resists[i].im_flag);
			vuln = of_has(f, resists[i].vuln_flag);

			if (imm) name_attr = TERM_GREEN;
			else if (res && name_attr == TERM_WHITE) name_attr = TERM_L_BLUE;

			if (vuln) sym = '-';
			else if (imm) sym = '*';
			else if (res) sym = '+';
			else if ((j < INVEN_TOTAL) && o_ptr->kind && 
				!object_flag_is_known(o_ptr, resists[i].res_flag)) sym = '?';
			Term_addch(attr, sym);
		}
		Term_putstr(col, row, 6, name_attr, format("%5s:", resists[i].name));
	}
	Term_putstr(col, row++, RES_COLS, TERM_WHITE, "      abcdefghijkl@");
	/* Equippy */
	display_player_equippy(row++, col+6);
}

static void display_player_flag_info(void)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		display_resistance_panel(player_flag_table+(i*RES_ROWS), RES_ROWS+3,
								&resist_region[i]);
	}
}


/*
 * Special display, part 2b
 */
void display_player_stat_info(void)
{
	int i, row, col;

	char buf[80];


	/* Row */
	row = 2;

	/* Column */
	col = 42;

	/* Print out the labels for the columns */
	c_put_str(TERM_WHITE, "  Self", row-1, col+5);
	c_put_str(TERM_WHITE, " RB", row-1, col+12);
	c_put_str(TERM_WHITE, " CB", row-1, col+16);
	c_put_str(TERM_WHITE, " EB", row-1, col+20);
	c_put_str(TERM_WHITE, "  Best", row-1, col+24);

	/* Display the stats */
	for (i = 0; i < A_MAX; i++)
	{
		/* Reduced */
		if (p_ptr->state.stat_use[i] < p_ptr->state.stat_top[i])
		{
			/* Use lowercase stat name */
			put_str(stat_names_reduced[i], row+i, col);
		}

		/* Normal */
		else
		{
			/* Assume uppercase stat name */
			put_str(stat_names[i], row+i, col);
		}

		/* Indicate natural maximum */
		if (p_ptr->stat_max[i] == 18+100)
		{
			put_str("!", row+i, col+3);
		}

		/* Internal "natural" maximum value */
		cnv_stat(p_ptr->stat_max[i], buf, sizeof(buf));
		c_put_str(TERM_L_GREEN, buf, row+i, col+5);

		/* Race Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", p_ptr->race->r_adj[i]);
		c_put_str(TERM_L_BLUE, buf, row+i, col+12);

		/* Class Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", p_ptr->class->c_adj[i]);
		c_put_str(TERM_L_BLUE, buf, row+i, col+16);

		/* Equipment Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", p_ptr->state.stat_add[i]);
		c_put_str(TERM_L_BLUE, buf, row+i, col+20);

		/* Resulting "modified" maximum value */
		cnv_stat(p_ptr->state.stat_top[i], buf, sizeof(buf));
		c_put_str(TERM_L_GREEN, buf, row+i, col+24);

		/* Only display stat_use if not maximal */
		if (p_ptr->state.stat_use[i] < p_ptr->state.stat_top[i])
		{
			cnv_stat(p_ptr->state.stat_use[i], buf, sizeof(buf));
			c_put_str(TERM_YELLOW, buf, row+i, col+31);
		}
	}
}


/*
 * Special display, part 2c
 *
 * How to print out the modifications and sustains.
 * Positive mods with no sustain will be light green.
 * Positive mods with a sustain will be dark green.
 * Sustains (with no modification) will be a dark green 's'.
 * Negative mods (from a curse) will be red.
 * Huge mods (>9), like from MICoMorgoth, will be a '*'
 * No mod, no sustain, will be a slate '.'
 */
static void display_player_sust_info(void)
{
	int i, j, row, col, stat;

	object_type *o_ptr;
	bitflag f[OF_SIZE];

	int stat_flags[A_MAX];
	int sustain_flags[A_MAX];

	byte a;
	char c;


	/* Row */
	row = 2;

	/* Column */
	col = 26;

	/* Build the stat flags tables */
	stat_flags[A_STR] = OF_STR;
	stat_flags[A_INT] = OF_INT;
	stat_flags[A_WIS] = OF_WIS;
	stat_flags[A_DEX] = OF_DEX;
	stat_flags[A_CON] = OF_CON;
	stat_flags[A_CHR] = OF_CHR;
	sustain_flags[A_STR] = OF_SUST_STR;
	sustain_flags[A_INT] = OF_SUST_INT;
	sustain_flags[A_WIS] = OF_SUST_WIS;
	sustain_flags[A_DEX] = OF_SUST_DEX;
	sustain_flags[A_CON] = OF_SUST_CON;
	sustain_flags[A_CHR] = OF_SUST_CHR;

	/* Header */
	c_put_str(TERM_WHITE, "abcdefghijkl@", row-1, col);

	/* Process equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; ++i)
	{
		/* Get the object */
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->kind) {
			col++;
			continue;
		}

		/* Get the "known" flags */
		object_flags_known(o_ptr, f);

		/* Initialize color based of sign of pval. */
		for (stat = 0; stat < A_MAX; stat++)
		{
			/* Default */
			a = TERM_SLATE;
			c = '.';

			/* Boost */
			if (of_has(f, stat_flags[stat]))
			{
				/* Default */
				c = '*';

				/* Work out which pval we're talking about */
				j = which_pval(o_ptr, stat_flags[stat]);

				/* Good */
				if (o_ptr->pval[j] > 0)
				{
					/* Good */
					a = TERM_L_GREEN;

					/* Label boost */
					if (o_ptr->pval[j] < 10)
						c = I2D(o_ptr->pval[j]);
				}

				/* Bad */
				if (o_ptr->pval[j] < 0)
				{
					/* Bad */
					a = TERM_RED;

					/* Label boost */
					if (o_ptr->pval[j] > -10)
						c = I2D(-(o_ptr->pval[j]));
				}
			}

			/* Sustain */
			if (of_has(f, sustain_flags[stat]))
			{
				/* Dark green */
				a = TERM_GREEN;

				/* Convert '.' to 's' */
				if (c == '.') c = 's';
			}

			if ((c == '.') && o_ptr->kind && !object_flag_is_known(o_ptr, sustain_flags[stat]))
				c = '?';

			/* Dump proper character */
			Term_putch(col, row+stat, a, c);
		}

		/* Advance */
		col++;
	}

	/* Player flags */
	player_flags(f);

	/* Check stats */
	for (stat = 0; stat < A_MAX; ++stat)
	{
		/* Default */
		a = TERM_SLATE;
		c = '.';

		/* Sustain */
		if (of_has(f, sustain_flags[stat]))
		{
			/* Dark green "s" */
			a = TERM_GREEN;
			c = 's';
		}

		/* Dump */
		Term_putch(col, row+stat, a, c);
	}

	/* Column */
	col = 26;

	/* Footer */
	c_put_str(TERM_WHITE, "abcdefghijkl@", row+6, col);

	/* Equippy */
	display_player_equippy(row+7, col);
}



static void display_panel(const data_panel *panel, int count, bool left_adj, const region *bounds)
{
	int i;
	char buffer[50];
	int col = bounds->col;
	int row = bounds->row;
	int w = bounds->width;
	int offset = 0;

	region_erase(bounds);

	if (left_adj)
	{
		for (i = 0; i < count; i++)
		{
			int len = panel[i].label ? strlen(panel[i].label) : 0;
			if (offset < len) offset = len;
		}
		offset += 2;
	}

	for (i = 0; i < count; i++, row++)
	{
		int len;
		if (!panel[i].label) continue;
		Term_putstr(col, row, strlen(panel[i].label), TERM_WHITE, panel[i].label);

		strnfmt(buffer, sizeof(buffer), panel[i].fmt, panel[i].value[0], panel[i].value[1]);

		len = strlen(buffer);
		len = len < w - offset ? len : w - offset - 1;
		if (left_adj)
			Term_putstr(col+offset, row, len, panel[i].color, buffer);
		else
			Term_putstr(col+w-len, row, len, panel[i].color, buffer);
	}
}


static const region boundaries [] =
{
	/* x   y     width, rows */
	{ 0,   0,		0,		0 },
	{ 1,   1,		40,		8 }, /* Name, Class, ... */
	{ 1,   9,		22,		9 }, /* Cur Exp, Max Exp, ... */
	{ 26,  9,		17,		9 }, /* AC, melee, ... */
	{ 48,  9,		24,		8 }, /* skills */
	{ 21,  2,		18,		5 }, /* Age, ht, wt, ... */
};


static const char *show_title(void)
{
	if (p_ptr->wizard)
		return "[=-WIZARD-=]";
	else if (p_ptr->total_winner || p_ptr->lev > PY_MAX_LEVEL)
		return "***WINNER***";
	else
		return p_ptr->class->title[(p_ptr->lev - 1) / 5];
}

static const char *show_adv_exp(void)
{
	if (p_ptr->lev < PY_MAX_LEVEL)
	{
		static char buffer[30];
		s32b advance = (player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L);
		strnfmt(buffer, sizeof(buffer), "%d", advance);
		return buffer;
	}
	else {
		return "********";
	}
}

static const char *show_depth(void)
{
	static char buffer[13];

	if (p_ptr->max_depth == 0) return "Town";

	strnfmt(buffer, sizeof(buffer), "%d' (L%d)",
	        p_ptr->max_depth * 50, p_ptr->max_depth);
	return buffer;
}

static const char *show_speed(void)
{
	static char buffer[10];
	int tmp = p_ptr->state.speed;
	if (p_ptr->timed[TMD_FAST]) tmp -= 10;
	if (p_ptr->timed[TMD_SLOW]) tmp += 10;
	if (p_ptr->searching) tmp += 10;
	if (tmp == 110) return "Normal";
	strnfmt(buffer, sizeof(buffer), "%d", tmp - 110);
	return buffer;
}

static const char *show_melee_weapon(const object_type *o_ptr)
{
	static char buffer[12];
	int hit = p_ptr->state.dis_to_h;
	int dam = p_ptr->state.dis_to_d;

	if (object_attack_plusses_are_visible(o_ptr))
	{
		hit += o_ptr->to_h;
		dam += o_ptr->to_d;
	}

	strnfmt(buffer, sizeof(buffer), "(%+d,%+d)", hit, dam);
	return buffer;
}

static const char *show_missile_weapon(const object_type *o_ptr)
{
	static char buffer[12];
	int hit = p_ptr->state.dis_to_h;
	int dam = 0;

	if (object_attack_plusses_are_visible(o_ptr))
	{
		hit += o_ptr->to_h;
		dam += o_ptr->to_d;
	}

	strnfmt(buffer, sizeof(buffer), "(%+d,%+d)", hit, dam);
	return buffer;
}

static byte max_color(int val, int max)
{
	return val < max ? TERM_YELLOW : TERM_L_GREEN;
}


static const char *show_status(void)
{
	int sc = p_ptr->sc;
	sc /= 10;

	switch (sc)
	{
		case 0:
		case 1:
			return "Pariah";

		case 2:
			return "Outcast";

		case 3:
		case 4:
			return "Unknown";

		case 5:
			return "Known";

		case 6:
		/* Maximum status by birth 75 = 7 */
		case 7:
			return "Liked";

		case 8:
			return "Well-liked";

		case 9:
		case 10:
			return "Respected";

		case 11:
		case 12:
			return "Role model";

		case 13:
			return "Feared";

		case 14:
		case 15:
			return "Lordly";
	}

	return format("%d", sc);
}

/* data_panel array element initializer, for ansi compliance */
#define P_I(col, lab, format, val1, val2) \
	{ panel[i].color = col; panel[i].label = lab; panel[i].fmt = format; \
	 panel[i].value[0] = val1; panel[i].value[1] = val2; \
	 i++; }

/* colours for table items */
static const byte colour_table[] =
{
	TERM_RED, TERM_RED, TERM_RED, TERM_L_RED, TERM_ORANGE,
	TERM_YELLOW, TERM_YELLOW, TERM_GREEN, TERM_GREEN, TERM_L_GREEN,
	TERM_L_BLUE
};

static int get_panel(int oid, data_panel *panel, size_t size)
{
	int ret = (s32b) size;
	switch(oid)
	{
  case 1:
  {
	int i = 0;
	assert( size >= (u32b) boundaries[1].page_rows);
	ret = boundaries[1].page_rows;
	P_I(TERM_L_BLUE, "Name",	"%y",	s2u(op_ptr->full_name), END  );
	P_I(TERM_L_BLUE, "Sex",		"%y",	s2u(p_ptr->sex->title), END  );
	P_I(TERM_L_BLUE, "Race",	"%y",	s2u(p_ptr->race->name), END  );
	P_I(TERM_L_BLUE, "Class",	"%y",	s2u(p_ptr->class->name), END  );
	P_I(TERM_L_BLUE, "Title",	"%y",	s2u(show_title()), END  );
	P_I(TERM_L_BLUE, "HP",	"%y/%y",	i2u(p_ptr->chp), i2u(p_ptr->mhp)  );
	P_I(TERM_L_BLUE, "SP",	"%y/%y",	i2u(p_ptr->csp), i2u(p_ptr->msp)  );
	P_I(TERM_L_BLUE, "Level",	"%y",	i2u(p_ptr->lev), END  );
	assert(i == boundaries[1].page_rows);
	return ret;
  }
  case 2:
  {
	int i = 0;
	assert( ret >= boundaries[2].page_rows);
	ret = boundaries[2].page_rows;
	P_I(max_color(p_ptr->lev, p_ptr->max_lev), "Level", "%y", i2u(p_ptr->lev), END  );
	P_I(max_color(p_ptr->exp, p_ptr->max_exp), "Cur Exp", "%y", i2u(p_ptr->exp), END  );
	P_I(TERM_L_GREEN, "Max Exp",	"%y",	i2u(p_ptr->max_exp), END  );
	P_I(TERM_L_GREEN, "Adv Exp",	"%y",	s2u(show_adv_exp()), END  );
	P_I(TERM_L_GREEN, "MaxDepth",	"%y",	s2u(show_depth()), END  );
	P_I(TERM_L_GREEN, "Game Turns",	"%y",	i2u(turn), END  );
	P_I(TERM_L_GREEN, "Standard Turns","%y", i2u(p_ptr->total_energy / 100), END  );
	P_I(TERM_L_GREEN, "Resting Turns","%y",	i2u(p_ptr->resting_turn), END  );
	P_I(TERM_L_GREEN, "Gold",		"%y",	i2u(p_ptr->au), END  );
	assert(i == boundaries[2].page_rows);
	return ret;
  }
  case 3:
  {
	int i = 0;
	assert(ret >= boundaries[3].page_rows);
	ret = boundaries[3].page_rows;
	P_I(TERM_L_BLUE, "Armor", "[%y,%+y]",	i2u(p_ptr->state.dis_ac), i2u(p_ptr->state.dis_to_a)  );
	P_I(TERM_L_BLUE, "Fight", "(%+y,%+y)",	i2u(p_ptr->state.dis_to_h), i2u(p_ptr->state.dis_to_d)  );
	P_I(TERM_L_BLUE, "Melee", "%y",		s2u(show_melee_weapon(&p_ptr->inventory[INVEN_WIELD])), END  );
	P_I(TERM_L_BLUE, "Shoot", "%y",		s2u(show_missile_weapon(&p_ptr->inventory[INVEN_BOW])), END  );
	P_I(TERM_L_BLUE, "Blows", "%y.%y/turn",	i2u(p_ptr->state.num_blows / 100), i2u((p_ptr->state.num_blows / 10) % 10) );
	P_I(TERM_L_BLUE, "Shots", "%y/turn",	i2u(p_ptr->state.num_shots), END  );
	P_I(TERM_L_BLUE, "Infra", "%y ft",	i2u(p_ptr->state.see_infra * 10), END  );
	P_I(TERM_L_BLUE, "Speed", "%y",		s2u(show_speed()), END );
	P_I(TERM_L_BLUE, "Burden","%.1y lbs",	f2u(p_ptr->total_weight/10.0), END  );
	assert(i == boundaries[3].page_rows);
	return ret;
  }
  case 4:
  {
	static struct {
		const char *name;
		int skill;
		int div;
	} skills[] =
	{
		{ "Saving Throw", SKILL_SAVE, 6 },
		{ "Stealth", SKILL_STEALTH, 1 },
		{ "Fighting", SKILL_TO_HIT_MELEE, 12 },
		{ "Shooting", SKILL_TO_HIT_BOW, 12 },
		{ "Disarming", SKILL_DISARM, 8 },
		{ "Magic Device", SKILL_DEVICE, 6 },
		{ "Perception", SKILL_SEARCH_FREQUENCY, 6 },
		{ "Searching", SKILL_SEARCH, 6 }
	};
	int i;
	assert(N_ELEMENTS(skills) == boundaries[4].page_rows);
	ret = N_ELEMENTS(skills);
	if ((u32b) ret > size) ret = size;
	for (i = 0; i < ret; i++)
	{
		s16b skill = p_ptr->state.skills[skills[i].skill];
		panel[i].color = TERM_L_BLUE;
		panel[i].label = skills[i].name;
		if (skills[i].skill == SKILL_SAVE ||
				skills[i].skill == SKILL_SEARCH)
		{
			if (skill < 0) skill = 0;
			if (skill > 100) skill = 100;
			panel[i].fmt = "%y%%";
			panel[i].value[0] = i2u(skill);
			panel[i].color = colour_table[skill / 10];
		}
		else if (skills[i].skill == SKILL_DEVICE)
		{
			panel[i].fmt = "%y";
			panel[i].value[0] = i2u(skill);
			panel[i].color = colour_table[skill / 13];
		}
		else if (skills[i].skill == SKILL_SEARCH_FREQUENCY)
		{
			if (skill <= 0) skill = 1;
			if (skill >= 50)
			{
				panel[i].fmt = "1 in 1";
				panel[i].color = colour_table[10];
			}
			else
			{
				/* convert to % chance of searching */
				skill = 50 - skill;
				panel[i].fmt = "1 in %y";
				panel[i].value[0] = i2u(skill);
				panel[i].color =
					colour_table[(100 - skill*2) / 10];
			}
		}
		else if (skills[i].skill == SKILL_DISARM)
		{
			/* assume disarming a dungeon trap */
			skill -= 5;
			if (skill > 100) skill = 100;
			if (skill < 2) skill = 2;
			panel[i].fmt = "%y%%";
			panel[i].value[0] = i2u(skill);
			panel[i].color = colour_table[skill / 10];
		}
		else
		{
			panel[i].fmt = "%y";
			panel[i].value[0] = s2u(likert(skill, skills[i].div, &panel[i].color));
		}
	}
	return ret;
  }
  case 5:
  {
	int i = 0;
	assert(ret >= boundaries[5].page_rows);
	ret = boundaries[5].page_rows;
	P_I(TERM_L_BLUE, "Age",			"%y",	i2u(p_ptr->age), END );
	P_I(TERM_L_BLUE, "Height",		"%y",	i2u(p_ptr->ht), END  );
	P_I(TERM_L_BLUE, "Weight",		"%y",	i2u(p_ptr->wt), END  );
	P_I(TERM_L_BLUE, "Social",		"%y",	s2u(show_status()), END  );
	P_I(TERM_L_BLUE, "Maximize",	"%y",	c2u(OPT(birth_maximize) ? 'Y' : 'N'), END);
#if 0
	/* Preserve mode deleted */
	P_I(TERM_L_BLUE, "Preserve",	"%y",	c2u(birth_preserve ? 'Y' : 'N'), END);
#endif
	assert(i == boundaries[5].page_rows);
	return ret;
  }
 }
	/* hopefully not reached */
	return 0;
}

void display_player_xtra_info(void)
{
	int i;
	int panels [] = { 1, 2, 3, 4, 5};
	bool left_adj [] = { 1, 0, 0, 0, 0 };
	data_panel data[MAX_PANEL];

	for (i = 0; i < (int)N_ELEMENTS(panels); i++)
	{
		int oid = panels[i];
		int rows = get_panel(oid, data, N_ELEMENTS(data));

		/* Hack:  Don't show 'Level' in the name, class ...  panel */
		if (oid == 1) rows -= 1;

		display_panel(data, rows, left_adj[i], &boundaries[oid]);
	}

	/* Indent output by 1 character, and wrap at column 72 */
	text_out_wrap = 72;
	text_out_indent = 1;

	/* History */
	Term_gotoxy(text_out_indent, 19);
	text_out_to_screen(TERM_WHITE, p_ptr->history);

	/* Reset text_out() vars */
	text_out_wrap = 0;
	text_out_indent = 0;

	return;
}

/*
 * Display the character on the screen (two different modes)
 *
 * The top two lines, and the bottom line (or two) are left blank.
 *
 * Mode 0 = standard display with skills/history
 * Mode 1 = special display with equipment flags
 */
void display_player(int mode)
{
	/* Erase screen */
	clear_from(0);

	/* When not playing, do not display in subwindows */
	if (Term != angband_term[0] && !p_ptr->playing) return;

	/* Stat info */
	display_player_stat_info();

	if (mode)
	{
		data_panel data[MAX_PANEL];
		int rows = get_panel(1, data, N_ELEMENTS(data));

		display_panel(data, rows, 1, &boundaries[1]);

		/* Stat/Sustain flags */
		display_player_sust_info();

		/* Other flags */
		display_player_flag_info();
	}

	/* Standard */
	else
	{
		/* Extra info */
		display_player_xtra_info();
	}
}


/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(const char *path, bool full)
{
	int i, x, y;

	byte a;
	char c;

	ang_file *fp;

	struct store *st_ptr = &stores[STORE_HOME];

	char o_name[80];

	byte (*old_xchar_hook)(byte c) = Term->xchar_hook;

	char buf[1024];

	/* We use either ascii or system-specific encoding */
 	int encoding = OPT(xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

	/* Unused parameter */
	(void)full;


	/* Open the file for writing */
	fp = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (!fp) return (-1);

	/* Display the requested encoding -- ASCII or system-specific */
 	if (!OPT(xchars_to_file)) Term->xchar_hook = NULL;

	/* Begin dump */
	file_putf(fp, "  [%s Character Dump]\n\n", buildid);


	/* Display player */
	display_player(0);

	/* Dump part of the screen */
	for (y = 1; y < 23; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Back up over spaces */
		while ((x > 0) && (buf[x-1] == ' ')) --x;

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		x_file_putf(fp, encoding, "%s\n", buf);
	}

	/* Skip a line */
	file_putf(fp, "\n");

	/* Display player */
	display_player(1);

	/* Dump part of the screen */
	for (y = 11; y < 20; y++)
	{
		/* Dump each row */
		for (x = 0; x < 39; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Back up over spaces */
		while ((x > 0) && (buf[x-1] == ' ')) --x;

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		x_file_putf(fp, encoding, "%s\n", buf);
	}

	/* Skip a line */
	file_putf(fp, "\n");

	/* Dump part of the screen */
	for (y = 11; y < 20; y++)
	{
		/* Dump each row */
		for (x = 0; x < 39; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x + 40, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Back up over spaces */
		while ((x > 0) && (buf[x-1] == ' ')) --x;

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		x_file_putf(fp, encoding, "%s\n", buf);
	}

	/* Skip some lines */
	file_putf(fp, "\n\n");


	/* If dead, dump last messages -- Prfnoff */
	if (p_ptr->is_dead)
	{
		i = messages_num();
		if (i > 15) i = 15;
		file_putf(fp, "  [Last Messages]\n\n");
		while (i-- > 0)
		{
			x_file_putf(fp, encoding, "> %s\n", message_str((s16b)i));
		}
		x_file_putf(fp, encoding, "\nKilled by %s.\n\n", p_ptr->died_from);
	}


	/* Dump the equipment */
	file_putf(fp, "  [Character Equipment]\n\n");
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		if (i == INVEN_TOTAL)
		{
			file_putf(fp, "\n\n  [Character Quiver]\n\n");
			continue;
		}
		object_desc(o_name, sizeof(o_name), &p_ptr->inventory[i],
				ODESC_PREFIX | ODESC_FULL);

		x_file_putf(fp, encoding, "%c) %s\n", index_to_label(i), o_name);
		if (p_ptr->inventory[i].kind)
			object_info_chardump(fp, &p_ptr->inventory[i], 5, 72);
	}

	/* Dump the inventory */
	file_putf(fp, "\n\n  [Character Inventory]\n\n");
	for (i = 0; i < INVEN_PACK; i++)
	{
		if (!p_ptr->inventory[i].kind) break;

		object_desc(o_name, sizeof(o_name), &p_ptr->inventory[i],
					ODESC_PREFIX | ODESC_FULL);

		x_file_putf(fp, encoding, "%c) %s\n", index_to_label(i), o_name);
		object_info_chardump(fp, &p_ptr->inventory[i], 5, 72);
	}
	file_putf(fp, "\n\n");


	/* Dump the Home -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Header */
		file_putf(fp, "  [Home Inventory]\n\n");

		/* Dump all available items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			object_desc(o_name, sizeof(o_name), &st_ptr->stock[i],
						ODESC_PREFIX | ODESC_FULL);
			x_file_putf(fp, encoding, "%c) %s\n", I2A(i), o_name);

			object_info_chardump(fp, &st_ptr->stock[i], 5, 72);
		}

		/* Add an empty line */
		file_putf(fp, "\n\n");
	}

	/* Dump character history */
	dump_history(fp);
	file_putf(fp, "\n\n");

	/* Dump options */
	file_putf(fp, "  [Options]\n\n");

	/* Dump options */
	for (i = OPT_BIRTH; i < OPT_BIRTH + N_OPTS_BIRTH; i++)
	{
		if (option_name(i))
		{
			file_putf(fp, "%-45s: %s (%s)\n",
			        option_desc(i),
			        op_ptr->opt[i] ? "yes" : "no ",
			        option_name(i));
		}
	}

	/* Skip some lines */
	file_putf(fp, "\n\n");

	/* Return to standard display */
 	Term->xchar_hook = old_xchar_hook;

	file_close(fp);


	/* Success */
	return (0);
}


/*
 * Make a string lower case.
 */
static void string_lower(char *buf)
{
	char *s;

	/* Lowercase the string */
	for (s = buf; *s != 0; s++) *s = tolower((unsigned char)*s);
}


/*
 * Recursive file perusal.
 *
 * Return FALSE on "?", otherwise TRUE.
 *
 * This function could be made much more efficient with the use of "seek"
 * functionality, especially when moving backwards through a file, or
 * forwards through a file by less than a page at a time.  XXX XXX XXX
 */
bool show_file(const char *name, const char *what, int line, int mode)
{
	int i, k, n;

	struct keypress ch;

	/* Number of "real" lines passed by */
	int next = 0;

	/* Number of "real" lines in the file */
	int size;

	/* Backup value for "line" */
	int back = 0;

	/* This screen has sub-screens */
	bool menu = FALSE;

	/* Case sensitive search */
	bool case_sensitive = FALSE;

	/* Current help file */
	ang_file *fff = NULL;

	/* Find this string (if any) */
	char *find = NULL;

	/* Jump to this tag */
	const char *tag = NULL;

	/* Hold a string to find */
	char finder[80] = "";

	/* Hold a string to show */
	char shower[80] = "";

	/* Filename */
	char filename[1024];

	/* Describe this thing */
	char caption[128] = "";

	/* Path buffer */
	char path[1024];

	/* General buffer */
	char buf[1024];

	/* Lower case version of the buffer, for searching */
	char lc_buf[1024];

	/* Sub-menu information */
	char hook[26][32];

	int wid, hgt;



	/* Wipe the hooks */
	for (i = 0; i < 26; i++) hook[i][0] = '\0';

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Copy the filename */
	my_strcpy(filename, name, sizeof(filename));

	n = strlen(filename);

	/* Extract the tag from the filename */
	for (i = 0; i < n; i++)
	{
		if (filename[i] == '#')
		{
			filename[i] = '\0';
			tag = filename + i + 1;
			break;
		}
	}

	/* Redirect the name */
	name = filename;


	/* Hack XXX XXX XXX */
	if (what)
	{
		my_strcpy(caption, what, sizeof(caption));

		my_strcpy(path, name, sizeof(path));
		fff = file_open(path, MODE_READ, -1);
	}

	/* Look in "help" */
	if (!fff)
	{
		strnfmt(caption, sizeof(caption), "Help file '%s'", name);

		path_build(path, sizeof(path), ANGBAND_DIR_HELP, name);
		fff = file_open(path, MODE_READ, -1);
	}

	/* Look in "info" */
	if (!fff)
	{
		strnfmt(caption, sizeof(caption), "Info file '%s'", name);

		path_build(path, sizeof(path), ANGBAND_DIR_INFO, name);
		fff = file_open(path, MODE_READ, -1);
	}

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg("Cannot open '%s'.", name);
		message_flush();

		/* Oops */
		return (TRUE);
	}


	/* Pre-Parse the file */
	while (TRUE)
	{
		/* Read a line or stop */
		if (!file_getl(fff, buf, sizeof(buf))) break;

		/* XXX Parse "menu" items */
		if (prefix(buf, "***** "))
		{
			char b1 = '[', b2 = ']';

			/* Notice "menu" requests */
			if ((buf[6] == b1) && isalpha((unsigned char)buf[7]) &&
			    (buf[8] == b2) && (buf[9] == ' '))
			{
				/* This is a menu file */
				menu = TRUE;

				/* Extract the menu item */
				k = A2I(buf[7]);

				/* Store the menu item (if valid) */
				if ((k >= 0) && (k < 26))
					my_strcpy(hook[k], buf + 10, sizeof(hook[0]));
			}
			/* Notice "tag" requests */
			else if (buf[6] == '<')
			{
				if (tag)
				{
					/* Remove the closing '>' of the tag */
					buf[strlen(buf) - 1] = '\0';

					/* Compare with the requested tag */
					if (streq(buf + 7, tag))
					{
						/* Remember the tagged line */
						line = next;
					}
				}
			}

			/* Skip this */
			continue;
		}

		/* Count the "real" lines */
		next++;
	}

	/* Save the number of "real" lines */
	size = next;



	/* Display the file */
	while (TRUE)
	{
		/* Clear screen */
		Term_clear();


		/* Restrict the visible range */
		if (line > (size - (hgt - 4))) line = size - (hgt - 4);
		if (line < 0) line = 0;


		/* Re-open the file if needed */
		if (next > line)
		{
			/* Close it */
			file_close(fff);

			/* Hack -- Re-Open the file */
			fff = file_open(path, MODE_READ, -1);
			if (!fff) return (TRUE);

			/* File has been restarted */
			next = 0;
		}


		/* Goto the selected line */
		while (next < line)
		{
			/* Get a line */
			if (!file_getl(fff, buf, sizeof(buf))) break;

			/* Skip tags/links */
			if (prefix(buf, "***** ")) continue;

			/* Count the lines */
			next++;
		}


		/* Dump the next lines of the file */
		for (i = 0; i < hgt - 4; )
		{
			/* Hack -- track the "first" line */
			if (!i) line = next;

			/* Get a line of the file or stop */
			if (!file_getl(fff, buf, sizeof(buf))) break;

			/* Hack -- skip "special" lines */
			if (prefix(buf, "***** ")) continue;

			/* Count the "real" lines */
			next++;

			/* Make a copy of the current line for searching */
			my_strcpy(lc_buf, buf, sizeof(lc_buf));

			/* Make the line lower case */
			if (!case_sensitive) string_lower(lc_buf);

			/* Hack -- keep searching */
			if (find && !i && !strstr(lc_buf, find)) continue;

			/* Hack -- stop searching */
			find = NULL;

			/* Dump the line */
			Term_putstr(0, i+2, -1, TERM_WHITE, buf);

			/* Highlight "shower" */
			if (shower[0])
			{
				const char *str = lc_buf;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-lc_buf, i+2, len, TERM_YELLOW, &buf[str-lc_buf]);

					/* Advance */
					str += len;
				}
			}

			/* Count the printed lines */
			i++;
		}

		/* Hack -- failed search */
		if (find)
		{
			bell("Search string not found!");
			line = back;
			find = NULL;
			continue;
		}


		/* Show a general "title" */
		prt(format("[%s, %s, Line %d-%d/%d]", buildid,
		           caption, line, line + hgt - 4, size), 0, 0);


		/* Prompt -- menu screen */
		if (menu)
		{
			/* Wait for it */
			prt("[Press a Number, or ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- small files */
		else if (size <= hgt - 4)
		{
			/* Wait for it */
			prt("[Press ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- large files */
		else
		{
			/* Wait for it */
			prt("[Press Space to advance, or ESC to exit.]", hgt - 1, 0);
		}

		/* Get a keypress */
		ch = inkey();

		/* Exit the help */
		if (ch.code == '?') break;

		/* Toggle case sensitive on/off */
		if (ch.code == '!')
		{
			case_sensitive = !case_sensitive;
		}

		/* Try showing */
		if (ch.code == '&')
		{
			/* Get "shower" */
			prt("Show: ", hgt - 1, 0);
			(void)askfor_aux(shower, sizeof(shower), NULL);

			/* Make the "shower" lowercase */
			if (!case_sensitive) string_lower(shower);
		}

		/* Try finding */
		if (ch.code == '/')
		{
			/* Get "finder" */
			prt("Find: ", hgt - 1, 0);
			if (askfor_aux(finder, sizeof(finder), NULL))
			{
				/* Find it */
				find = finder;
				back = line;
				line = line + 1;

				/* Make the "finder" lowercase */
				if (!case_sensitive) string_lower(finder);

				/* Show it */
				my_strcpy(shower, finder, sizeof(shower));
			}
		}

		/* Go to a specific line */
		if (ch.code == '#')
		{
			char tmp[80] = "0";

			prt("Goto Line: ", hgt - 1, 0);
			if (askfor_aux(tmp, sizeof(tmp), NULL))
				line = atoi(tmp);
		}

		/* Go to a specific file */
		if (ch.code == '%')
		{
			char ftmp[80] = "help.hlp";

			prt("Goto File: ", hgt - 1, 0);
			if (askfor_aux(ftmp, sizeof(ftmp), NULL))
			{
				if (!show_file(ftmp, NULL, 0, mode))
					ch.code = ESCAPE;
			}
		}

		switch (ch.code) {
			/* up a line */
			case ARROW_UP:
			case '8': line--; break;

			/* up a page */
			case KC_PGUP:
			case '9':
			case '-': line -= (hgt - 4); break;

			/* home */
			case KC_HOME:
			case '7': line = 0; break;

			/* down a line */
			case ARROW_DOWN:
			case '2':
			case '\n':
			case '\r': line++; break;

			/* down a page */
			case KC_PGDOWN:
			case '3':
			case ' ': line += hgt - 4; break;

			/* end */
			case KC_END:
			case '1': line = size; break;
		}

		/* Recurse on letters */
		if (menu && isalpha((unsigned char)ch.code))
		{
			/* Extract the requested menu item */
			k = A2I(ch.code);

			/* Verify the menu item */
			if ((k >= 0) && (k <= 25) && hook[k][0])
			{
				/* Recurse on that file */
				if (!show_file(hook[k], NULL, 0, mode)) ch.code = ESCAPE;
			}
		}

		/* Exit on escape */
		if (ch.code == ESCAPE) break;
	}

	/* Close the file */
	file_close(fff);

	/* Done */
	return (ch.code != '?');
}


/*
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
	/* Save screen */
	screen_save();

	/* Peruse the main help file */
	(void)show_file("help.hlp", NULL, 0, 0);

	/* Load screen */
	screen_load();
}



/*
 * Process the player name and extract a clean "base name".
 *
 * If "sf" is TRUE, then we initialize "savefile" based on player name.
 *
 * Some platforms (Windows, Macintosh, Amiga) leave the "savefile" empty
 * when a new character is created, and then when the character is done
 * being created, they call this function to choose a new savefile name.
 *
 * This also now handles the turning on and off of the automatic
 * sequential numbering of character names with Roman numerals.  
 */
void process_player_name(bool sf)
{
	int i;

	/* Process the player name */
	for (i = 0; op_ptr->full_name[i]; i++)
	{
		char c = op_ptr->full_name[i];

		/* No control characters */
		if (iscntrl((unsigned char)c))
		{
			/* Illegal characters */
			quit_fmt("Illegal control char (0x%02X) in player name", c);
		}

		/* Convert all non-alphanumeric symbols */
		if (!isalpha((unsigned char)c) && !isdigit((unsigned char)c)) c = '_';

		/* Build "base_name" */
		op_ptr->base_name[i] = c;
	}

#if defined(WINDOWS)

	/* Max length */
	if (i > 8) i = 8;

#endif

	/* Terminate */
	op_ptr->base_name[i] = '\0';

	/* Require a "base" name */
	if (!op_ptr->base_name[0])
	{
		my_strcpy(op_ptr->base_name, "PLAYER", sizeof(op_ptr->base_name));
	}


	/* Pick savefile name if needed */
	if (sf)
	{
		char temp[128];

#if defined(SET_UID)
		/* Rename the savefile, using the player_uid and base_name */
		strnfmt(temp, sizeof(temp), "%d.%s", player_uid, op_ptr->base_name);
#else
		/* Rename the savefile, using the base name */
		strnfmt(temp, sizeof(temp), "%s", op_ptr->base_name);
#endif

		/* Build the filename */
		path_build(savefile, sizeof(savefile), ANGBAND_DIR_SAVE, temp);
	}
}


/*
 * Save the game
 */
void save_game(void)
{
	/* Disturb the player */
	disturb(p_ptr, 1, 0);

	/* Clear messages */
	message_flush();

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Message */
	prt("Saving game...", 0, 0);

	/* Refresh */
	Term_fresh();

	/* The player is not dead */
	my_strcpy(p_ptr->died_from, "(saved)", sizeof(p_ptr->died_from));

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Save the player */
	if (savefile_save(savefile))
		prt("Saving game... done.", 0, 0);
	else
		prt("Saving game... failed!", 0, 0);

	/* Allow suspend again */
	signals_handle_tstp();

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	my_strcpy(p_ptr->died_from, "(alive and well)", sizeof(p_ptr->died_from));
}



/*
 * Close up the current game (player may or may not be dead)
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Flush the messages */
	message_flush();

	/* Flush the input */
	flush();


	/* No suspending now */
	signals_ignore_tstp();


	/* Hack -- Increase "icky" depth */
	character_icky++;


	/* Handle death */
	if (p_ptr->is_dead)
	{
		death_screen();
	}

	/* Still alive */
	else
	{
		/* Save the game */
		save_game();

		if (Term->mapped_flag)
		{
			struct keypress ch;

			prt("Press Return (or Escape).", 0, 40);
			ch = inkey();
			if (ch.code != ESCAPE)
				predict_score();
		}
	}


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Allow suspending now */
	signals_handle_tstp();
}

static void write_html_escape_char(ang_file *fp, char c)
{
	switch (c)
	{
		case '<':
			file_putf(fp, "&lt;");
			break;
		case '>':
			file_putf(fp, "&gt;");
			break;
		case '&':
			file_putf(fp, "&amp;");
			break;
		default:
			file_putf(fp, "%c", c);
			break;
	}
}


/* Take an html screenshot */
void html_screenshot(const char *name, int mode)
{
	int y, x;
	int wid, hgt;

	byte a = TERM_WHITE;
	byte oa = TERM_WHITE;
	char c = ' ';

	const char *new_color_fmt = (mode == 0) ?
					"<font color=\"#%02X%02X%02X\">"
				 	: "[COLOR=\"#%02X%02X%02X\"]";
	const char *change_color_fmt = (mode == 0) ?
					"</font><font color=\"#%02X%02X%02X\">"
					: "[/COLOR][COLOR=\"#%02X%02X%02X\"]";
	const char *close_color_fmt = mode ==  0 ? "</font>" : "[/COLOR]";

	ang_file *fp;
	char buf[1024];


	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
	fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Oops */
	if (!fp)
	{
		plog_fmt("Cannot write the '%s' file!", buf);
		return;
	}

	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	if (mode == 0)
	{
		file_putf(fp, "<!DOCTYPE html><html><head>\n");
		file_putf(fp, "  <meta='generator' content='%s'>\n", buildid);
		file_putf(fp, "  <title>%s</title>\n", name);
		file_putf(fp, "</head>\n\n");
		file_putf(fp, "<body style='color: #fff; background: #000;'>\n");
		file_putf(fp, "<pre>\n");
	}
	else 
	{
		file_putf(fp, "[CODE][TT][BC=black][COLOR=white]\n");
	}

	/* Dump the screen */
	for (y = 0; y < hgt; y++)
	{
		for (x = 0; x < wid; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Color change */
			if (oa != a && c != ' ')
			{
				/* From the default white to another color */
				if (oa == TERM_WHITE)
				{
					file_putf(fp, new_color_fmt,
					        angband_color_table[a][1],
					        angband_color_table[a][2],
					        angband_color_table[a][3]);
				}

				/* From another color to the default white */
				else if (a == TERM_WHITE)
				{
					file_putf(fp, close_color_fmt);
				}

				/* Change colors */
				else
				{
					file_putf(fp, change_color_fmt,
					        angband_color_table[a][1],
					        angband_color_table[a][2],
					        angband_color_table[a][3]);
				}

				/* Remember the last color */
				oa = a;
			}

			/* Write the character and escape special HTML characters */
			if (mode == 0) write_html_escape_char(fp, c);
			else file_putf(fp, "%c", c);
		}

		/* End the row */
		file_putf(fp, "\n");
	}

	/* Close the last font-color tag if necessary */
	if (oa != TERM_WHITE) file_putf(fp, close_color_fmt);

	if (mode == 0)
	{
		file_putf(fp, "</pre>\n");
		file_putf(fp, "</body>\n");
		file_putf(fp, "</html>\n");
	}
	else 
	{
		file_putf(fp, "[/COLOR][/BC][/TT][/CODE]\n");
	}

	/* Close it */
	file_close(fp);
}
