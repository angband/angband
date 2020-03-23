/**
 * \file ui-player.c
 * \brief character screens and dumps
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
#include "game-world.h"
#include "init.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-util.h"
#include "player.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "ui-display.h"
#include "ui-history.h"
#include "ui-input.h"
#include "ui-menu.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-player.h"


/**
 * ------------------------------------------------------------------------
 * Panel utilities
 * ------------------------------------------------------------------------ */

/**
 * Panel line type
 */
struct panel_line {
	byte attr;
	const char *label;
	char value[20];
};

/**
 * Panel holder type
 */
struct panel {
	size_t len;
	size_t max;
	struct panel_line *lines;
};

/**
 * Allocate some panel lines
 */
static struct panel *panel_allocate(int n) {
	struct panel *p = mem_zalloc(sizeof *p);

	p->len = 0;
	p->max = n;
	p->lines = mem_zalloc(p->max * sizeof *p->lines);

	return p;
}

/**
 * Free up panel lines
 */
static void panel_free(struct panel *p) {
	assert(p);
	mem_free(p->lines);
	mem_free(p);
}

/**
 * Add a new line to the panel
 */
static void panel_line(struct panel *p, byte attr, const char *label,
		const char *fmt, ...) {
	va_list vp;

	struct panel_line *pl;

	/* Get the next panel line */
	assert(p);
	assert(p->len != p->max);
	pl = &p->lines[p->len++];

	/* Set the basics */
	pl->attr = attr;
	pl->label = label;

	/* Set the value */
	va_start(vp, fmt);
	vstrnfmt(pl->value, sizeof pl->value, fmt, vp);
	va_end(vp);
}

/**
 * Add a spacer line in a panel
 */
static void panel_space(struct panel *p) {
	assert(p);
	assert(p->len != p->max);
	p->len++;
}


/**
 * Returns a "rating" of x depending on y, and sets "attr" to the
 * corresponding "attribute".
 */
static const char *likert(int x, int y, byte *attr)
{
	/* Paranoia */
	if (y <= 0) y = 1;

	/* Negative value */
	if (x < 0) {
		*attr = COLOUR_RED;
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
		case 0:
		case 1:
		{
			*attr = COLOUR_RED;
			return ("Bad");
		}
		case 2:
		{
			*attr = COLOUR_RED;
			return ("Poor");
		}
		case 3:
		case 4:
		{
			*attr = COLOUR_YELLOW;
			return ("Fair");
		}
		case 5:
		{
			*attr = COLOUR_YELLOW;
			return ("Good");
		}
		case 6:
		{
			*attr = COLOUR_YELLOW;
			return ("Very Good");
		}
		case 7:
		case 8:
		{
			*attr = COLOUR_L_GREEN;
			return ("Excellent");
		}
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		{
			*attr = COLOUR_L_GREEN;
			return ("Superb");
		}
		case 14:
		case 15:
		case 16:
		case 17:
		{
			*attr = COLOUR_L_GREEN;
			return ("Heroic");
		}
		default:
		{
			*attr = COLOUR_L_GREEN;
			return ("Legendary");
		}
	}
}


/**
 * Equippy chars
 */
static void display_player_equippy(int y, int x)
{
	int i;

	byte a;
	wchar_t c;

	struct object *obj;

	/* Dump equippy chars */
	for (i = 0; i < player->body.count; ++i) {
		/* Object */
		obj = slot_object(player, i);

		/* Skip empty objects */
		if (!obj) continue;

		/* Get attr/char for display */
		a = object_attr(obj);
		c = object_char(obj);

		/* Dump */
		if ((tile_width == 1) && (tile_height == 1))
		        Term_putch(x + i, y, a, c);
	}
}

/**
 * List of resistances and abilities to display
 */
#define RES_ROWS 13
struct player_flag_record
{
	const char name[7];	/* Name of resistance/ability */
	int mod;			/* Modifier */
	int flag;			/* Flag bit */
	int element;		/* Element */
	int tmd_flag;		/* corresponding timed flag */
};

static const struct player_flag_record player_flag_table[RES_ROWS * 4] = {
	{ " Acid",	-1,					-1,				ELEM_ACID,	TMD_OPP_ACID },
	{ " Elec",	-1,					-1,				ELEM_ELEC,	TMD_OPP_ELEC },
	{ " Fire",	-1,					-1,				ELEM_FIRE,	TMD_OPP_FIRE },
	{ " Cold",	-1,					-1,				ELEM_COLD,	TMD_OPP_COLD },
	{ " Pois",	-1,					-1,				ELEM_POIS,	TMD_OPP_POIS },
	{ "Light",	-1,					-1,				ELEM_LIGHT,	-1 },
	{ " Dark",	-1,					-1,				ELEM_DARK,	-1 },	
	{ "Sound",	-1,					-1,				ELEM_SOUND,	-1 },
	{ "Shard",	-1,					-1,				ELEM_SHARD,	-1 },
	{ "Nexus",	-1,					-1,				ELEM_NEXUS,	-1 },
	{ "Nethr",	-1,					-1,				ELEM_NETHER,-1 },
	{ "Chaos",	-1,					-1,				ELEM_CHAOS,	-1 },
	{ "Disen",	-1,					-1,				ELEM_DISEN,	-1 },

	{ "pFear",	-1,					OF_PROT_FEAR,	-1,			TMD_BOLD },
	{ "pBlnd",	-1,					OF_PROT_BLIND,	-1,			-1 },
	{ "pConf",	-1,					OF_PROT_CONF,	-1,			TMD_OPP_CONF },
	{ "pStun",	-1,					OF_PROT_STUN,	-1,			-1 },
	{ "HLife",	-1,					OF_HOLD_LIFE,	-1, 		-1 },
	{ "Regen",	-1,					OF_REGEN,		-1, 		-1 },
	{ "  ESP",	-1,					OF_TELEPATHY,	-1,			TMD_TELEPATHY },
	{ "S.Inv",	-1,					OF_SEE_INVIS,	-1,			TMD_SINVIS },
	{ "FrAct",	-1,					OF_FREE_ACT,	-1, 		TMD_FREE_ACT },
	{ "Feath",	-1,					OF_FEATHER,		-1,			-1 },
	{ "S.Dig",	-1,					OF_SLOW_DIGEST,	-1, 		-1 },
	{ "TrpIm",	-1,					OF_TRAP_IMMUNE, -1,			-1 },
	{ "Bless",	-1,					OF_BLESSED,		-1, 		-1 },

	{ "ImpHP",	-1,					OF_IMPAIR_HP,	-1, 		-1 },
	{ "ImpSP",	-1,					OF_IMPAIR_MANA,	-1, 		-1 },
	{ " Fear",	-1,					OF_AFRAID,		-1,			TMD_AFRAID },
	{ "Aggrv",	-1,					OF_AGGRAVATE,	-1, 		-1 },
	{ "NoTel",	-1,					OF_NO_TELEPORT,	-1, 		-1 },
	{ "DrExp",	-1,					OF_DRAIN_EXP,	-1, 		-1 },
	{ "Stick",	-1,					OF_STICKY,		-1, 		-1 },
	{ "Fragl",	-1,					OF_FRAGILE,		-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },

	{ "Stea.",	OBJ_MOD_STEALTH,	-1,				-1, 		TMD_STEALTH },
	{ "Sear.",	OBJ_MOD_SEARCH,		-1,				-1, 		-1 },
	{ "Infra",	OBJ_MOD_INFRA,		-1,				-1,			TMD_SINFRA },
	{ "Tunn.",	OBJ_MOD_TUNNEL,		-1,				-1, 		-1 },
	{ "Speed",	OBJ_MOD_SPEED,		-1,				-1,			TMD_FAST },
	{ "Blows",	OBJ_MOD_BLOWS,		-1,				-1, 		-1 },
	{ "Shots",	OBJ_MOD_SHOTS,		-1,				-1, 		-1 },
	{ "Might",	OBJ_MOD_MIGHT,		-1,				-1, 		-1 },
	{ "Light",	OBJ_MOD_LIGHT,		-1,				-1, 		-1 },
	{ "D.Red",	OBJ_MOD_DAM_RED,	-1,				-1, 		-1 },
	{ "Moves",	OBJ_MOD_MOVES,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
	{ "",	-1,		-1,				-1, 		-1 },
};

static void display_resistance_panel(const struct player_flag_record *rec,
									size_t size, const region *bounds) 
{
	size_t i;
	int j;
	int col = bounds->col;
	int row = bounds->row;
	int res_cols = 5 + 2 + player->body.count;

	/* Equippy */
	display_player_equippy(row++, col + 6);

	Term_putstr(col, row++, res_cols, COLOUR_WHITE, "      abcdefghijkl@");
	for (i = 0; i < size - 3; i++, row++) {
		byte name_attr = COLOUR_WHITE;
		Term_gotoxy(col + 6, row);

		/* Repeated extraction of flags is inefficient but more natural */
		for (j = 0; j <= player->body.count; j++) {
			bitflag f[OF_SIZE];
			byte attr = COLOUR_WHITE | (j % 2) * 8; /* alternating columns */
			char sym = strlen(rec[i].name) ? '.' : ' ';
			bool res = false, imm = false, vul = false, rune = false;
			bool timed = false;
			bool known = false;

			/* Object or player info? */
			if (j < player->body.count) {
				int index = 0;
				struct object *obj = slot_object(player, j);
				struct curse_data *curse = obj ? obj->curses : NULL;

				while (obj) {
					/* Wipe flagset */
					of_wipe(f);

					/* Get known properties */
					object_flags_known(obj, f);
					if (rec[i].element != -1) {
						known = object_element_is_known(obj, rec[i].element);
					} else if (rec[i].flag != -1) {
						known = object_flag_is_known(obj, rec[i].flag);
					} else {
						known = true;
					}

					/* Get resistance, immunity and vulnerability info */
					if (rec[i].mod != -1) {
						if (obj->modifiers[rec[i].mod] != 0) {
							res = true;
						}
						rune = (player->obj_k->modifiers[rec[i].mod] == 1);
					} else if (rec[i].flag != -1) {
						if (of_has(f, rec[i].flag)) {
							res = true;
						}
						rune = of_has(player->obj_k->flags, rec[i].flag);
					} else if (rec[i].element != -1) {
						if (known) {
							if (obj->el_info[rec[i].element].res_level == 3) {
								imm = true;
							}
							if (obj->el_info[rec[i].element].res_level == 1) {
								res = true;
							}
							if (obj->el_info[rec[i].element].res_level == -1) {
								vul = true;
							}
						}
						rune = (player->obj_k->el_info[rec[i].element].res_level == 1);
					}

					/* Move to any unprocessed curse object */
					if (curse) {
						index++;
						obj = NULL;
						while (index < z_info->curse_max) {
							if (curse[index].power) {
								obj = curses[index].obj;
								break;
							} else {
								index++;
							}
						}
					} else {
						obj = NULL;
					}
				}
			} else {
				player_flags(player, f);
				known = true;

				/* Timed flags only in the player column */
				if (rec[i].tmd_flag >= 0) {
	 				timed = player->timed[rec[i].tmd_flag] ? true : false;
					/* There has to be one special case... */
					if ((rec[i].tmd_flag == TMD_AFRAID) &&
						(player->timed[TMD_TERROR]))
						timed = true;
					/* ..and a couple more... */
					if ((rec[i].tmd_flag == TMD_BOLD) &&
						(player->timed[TMD_HERO] || player->timed[TMD_SHERO]))
						timed = true;
				}

				/* Set which (if any) symbol and color are used */
				if (rec[i].mod != -1) {
					int k;

					/* Shape modifiers */
					for (k = 0; k < OBJ_MOD_MAX; k++) {
						res = (player->shape->modifiers[i] > 0);
						vul = (player->shape->modifiers[i] > 0);
					}

					/* Messy special cases */
					if (rec[i].mod == OBJ_MOD_INFRA)
						res |= (player->race->infra > 0);
					if (rec[i].mod == OBJ_MOD_TUNNEL)
						res |= (player->race->r_skills[SKILL_DIGGING] > 0);
				} else if (rec[i].flag != -1) {
					res = of_has(f, rec[i].flag);
					res |= (of_has(player->shape->flags, rec[i].flag) &&
							of_has(player->obj_k->flags, rec[i].flag));
				} else if (rec[i].element != -1) {
					int el = rec[i].element;
					imm = (player->race->el_info[el].res_level == 3) ||
						((player->shape->el_info[el].res_level == 3) &&
						 (player->obj_k->el_info[el].res_level));
					res = (player->race->el_info[el].res_level == 1) ||
						((player->shape->el_info[el].res_level == 1) &&
						 (player->obj_k->el_info[el].res_level));
					vul = (player->race->el_info[el].res_level == -1) ||
						((player->shape->el_info[el].res_level == -1) &&
						 (player->obj_k->el_info[el].res_level));
				}
			}

			/* Colour the name appropriately */
			if (imm) {
				name_attr = COLOUR_GREEN;
			} else if (res && (name_attr != COLOUR_GREEN)) {
				name_attr = COLOUR_L_BLUE;
			} else if (vul && (name_attr != COLOUR_GREEN)) {
				name_attr = COLOUR_RED;
			}

			/* Set the symbols and print them */
			if (vul) {
				sym = '-';
			} else if (imm) {
				sym = '*';
			} else if (res) {
				sym = '+';
			} else if (timed) {
				sym = '!';
				attr = COLOUR_L_GREEN;
			} else if ((j < player->body.count) && slot_object(player, j) &&
					   !known && !rune) {
				sym = '?';
			}

			Term_addch(attr, sym);
		}

		/* Check if the rune is known */
		if (((rec[i].mod >= 0) &&
			 (player->obj_k->modifiers[rec[i].mod] == 0))
			|| ((rec[i].flag >= 0) &&
				!of_has(player->obj_k->flags, rec[i].flag))
			|| ((rec[i].element >= 0) &&
				(player->obj_k->el_info[rec[i].element].res_level == 0))) {
			name_attr = COLOUR_SLATE;
		}

		if (strlen(rec[i].name)) {
			Term_putstr(col, row, 6, name_attr, format("%5s:", rec[i].name));
		}
	}
	//Term_putstr(col, row++, res_cols, COLOUR_WHITE, "      abcdefghijkl@");
}

static void display_player_flag_info(void)
{
	int i;
	int res_cols = 5 + 2 + player->body.count;
	region resist_region[] = {
		{  0 * (res_cols + 1), 7, res_cols, RES_ROWS + 2 },
		{  1 * (res_cols + 1), 7, res_cols, RES_ROWS + 2 },
		{  2 * (res_cols + 1), 7, res_cols, RES_ROWS + 2 },
		{  3 * (res_cols + 1), 7, res_cols, RES_ROWS + 2 },
	};

	for (i = 0; i < 4; i++)

		display_resistance_panel(player_flag_table + (i * RES_ROWS),
								 RES_ROWS + 3, &resist_region[i]);
}


/**
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
	c_put_str(COLOUR_WHITE, "  Self", row-1, col+5);
	c_put_str(COLOUR_WHITE, " RB", row-1, col+12);
	c_put_str(COLOUR_WHITE, " CB", row-1, col+16);
	c_put_str(COLOUR_WHITE, " EB", row-1, col+20);
	c_put_str(COLOUR_WHITE, "  Best", row-1, col+24);

	/* Display the stats */
	for (i = 0; i < STAT_MAX; i++) {
		/* Reduced or normal */
		if (player->stat_cur[i] < player->stat_max[i])
			/* Use lowercase stat name */
			put_str(stat_names_reduced[i], row+i, col);
		else
			/* Assume uppercase stat name */
			put_str(stat_names[i], row+i, col);

		/* Indicate natural maximum */
		if (player->stat_max[i] == 18+100)
			put_str("!", row+i, col+3);

		/* Internal "natural" maximum value */
		cnv_stat(player->stat_max[i], buf, sizeof(buf));
		c_put_str(COLOUR_L_GREEN, buf, row+i, col+5);

		/* Race Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", player->race->r_adj[i]);
		c_put_str(COLOUR_L_BLUE, buf, row+i, col+12);

		/* Class Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", player->class->c_adj[i]);
		c_put_str(COLOUR_L_BLUE, buf, row+i, col+16);

		/* Equipment Bonus */
		strnfmt(buf, sizeof(buf), "%+3d", player->state.stat_add[i]);
		c_put_str(COLOUR_L_BLUE, buf, row+i, col+20);

		/* Resulting "modified" maximum value */
		cnv_stat(player->state.stat_top[i], buf, sizeof(buf));
		c_put_str(COLOUR_L_GREEN, buf, row+i, col+24);

		/* Only display stat_use if there has been draining */
		if (player->stat_cur[i] < player->stat_max[i]) {
			cnv_stat(player->state.stat_use[i], buf, sizeof(buf));
			c_put_str(COLOUR_YELLOW, buf, row+i, col+31);
		}
	}
}


/**
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
	int i, row, col, stat;

	struct object *obj;
	bitflag f[OF_SIZE];

	byte a;
	char c;


	/* Row */
	row = 2;

	/* Column */
	col = 26;

	/* Header */
	c_put_str(COLOUR_WHITE, "abcdefghijkl@", row - 1, col);

	/* Process equipment */
	for (i = 0; i < player->body.count; ++i) {
		/* Get the object */
		obj = slot_object(player, i);

		if (!obj) {
			col++;
			continue;
		}

		/* Get the "known" flags */
		object_flags_known(obj, f);

		/* Initialize color based on sign of modifier. */
		for (stat = OBJ_MOD_MIN_STAT; stat < OBJ_MOD_MIN_STAT + STAT_MAX;
			 stat++) {
			/* Default */
			a = COLOUR_SLATE;
			c = '.';

			/* Boosted or reduced */
			if (obj->modifiers[stat] > 0) {
				/* Good */
				a = COLOUR_L_GREEN;

				/* Label boost */
				if (obj->modifiers[stat] < 10)
						c = I2D(obj->modifiers[stat]);
			} else if (obj->modifiers[stat] < 0) {
				/* Bad */
				a = COLOUR_RED;

				/* Label boost */
				if (obj->modifiers[stat] > -10)
					c = I2D(-(obj->modifiers[stat]));
			}

			/* Sustain */
			if (of_has(f, sustain_flag(stat))) {
				/* Dark green */
				a = COLOUR_GREEN;

				/* Convert '.' to 's' */
				if (c == '.') c = 's';
			}

			if ((c == '.') && obj && 
				!object_flag_is_known(obj, sustain_flag(stat)))
				c = '?';

			/* Dump proper character */
			Term_putch(col, row + stat, a, c);
		}

		/* Advance */
		col++;
	}

	/* Player flags */
	player_flags(player, f);

	/* Check stats */
	for (stat = 0; stat < STAT_MAX; ++stat) {
		/* Default */
		a = COLOUR_SLATE;
		c = '.';

		/* Sustain */
		if (of_has(f, sustain_flag(stat))) {
			/* Dark green "s" */
			a = COLOUR_GREEN;
			c = 's';
		}

		/* Dump */
		Term_putch(col, row + stat, a, c);
	}
}



static void display_panel(const struct panel *p, bool left_adj,
		const region *bounds)
{
	size_t i;
	int col = bounds->col;
	int row = bounds->row;
	int w = bounds->width;
	int offset = 0;

	region_erase(bounds);

	if (left_adj) {
		for (i = 0; i < p->len; i++) {
			struct panel_line *pl = &p->lines[i];

			int len = pl->label ? strlen(pl->label) : 0;
			if (offset < len) offset = len;
		}
		offset += 2;
	}

	for (i = 0; i < p->len; i++, row++) {
		int len;
		struct panel_line *pl = &p->lines[i];

		if (!pl->label)
			continue;

		Term_putstr(col, row, strlen(pl->label), COLOUR_WHITE, pl->label);

		len = strlen(pl->value);
		len = len < w - offset ? len : w - offset - 1;

		if (left_adj)
			Term_putstr(col+offset, row, len, pl->attr, pl->value);
		else
			Term_putstr(col+w-len, row, len, pl->attr, pl->value);
	}
}

static const char *show_title(void)
{
	if (player->wizard)
		return "[=-WIZARD-=]";
	else if (player->total_winner || player->lev > PY_MAX_LEVEL)
		return "***WINNER***";
	else
		return player->class->title[(player->lev - 1) / 5];
}

static const char *show_adv_exp(void)
{
	if (player->lev < PY_MAX_LEVEL) {
		static char buffer[30];
		s32b advance = (player_exp[player->lev - 1] * player->expfact / 100L);
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

	if (player->max_depth == 0) return "Town";

	strnfmt(buffer, sizeof(buffer), "%d' (L%d)",
	        player->max_depth * 50, player->max_depth);
	return buffer;
}

static const char *show_speed(void)
{
	static char buffer[10];
	int tmp = player->state.speed;
	if (player->timed[TMD_FAST]) tmp -= 10;
	if (player->timed[TMD_SLOW]) tmp += 10;
	if (tmp == 110) return "Normal";
	int multiplier = 10 * extract_energy[tmp] / extract_energy[110];
	int int_mul = multiplier / 10;
	int dec_mul = multiplier % 10;
	if (OPT(player, effective_speed))
		strnfmt(buffer, sizeof(buffer), "%d.%dx (%d)", int_mul, dec_mul, tmp - 110);
	else
		strnfmt(buffer, sizeof(buffer), "%d (%d.%dx)", tmp - 110, int_mul, dec_mul);
	return buffer;
}

static byte max_color(int val, int max)
{
	return val < max ? COLOUR_YELLOW : COLOUR_L_GREEN;
}

/**
 * Colours for table items
 */
static const byte colour_table[] =
{
	COLOUR_RED, COLOUR_RED, COLOUR_RED, COLOUR_L_RED, COLOUR_ORANGE,
	COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_GREEN, COLOUR_GREEN, COLOUR_L_GREEN,
	COLOUR_L_BLUE
};


static struct panel *get_panel_topleft(void) {
	struct panel *p = panel_allocate(6);

	panel_line(p, COLOUR_L_BLUE, "Name", "%s", player->full_name);
	panel_line(p, COLOUR_L_BLUE, "Race",	"%s", player->race->name);
	panel_line(p, COLOUR_L_BLUE, "Class", "%s", player->class->name);
	panel_line(p, COLOUR_L_BLUE, "Title", "%s", show_title());
	panel_line(p, COLOUR_L_BLUE, "HP", "%d/%d", player->chp, player->mhp);
	panel_line(p, COLOUR_L_BLUE, "SP", "%d/%d", player->csp, player->msp);

	return p;
}

static struct panel *get_panel_midleft(void) {
	struct panel *p = panel_allocate(9);
	int diff = weight_remaining(player);
	byte attr = diff < 0 ? COLOUR_L_RED : COLOUR_L_GREEN;

	panel_line(p, max_color(player->lev, player->max_lev),
			"Level", "%d", player->lev);
	panel_line(p, max_color(player->exp, player->max_exp),
			"Cur Exp", "%d", player->exp);
	panel_line(p, COLOUR_L_GREEN, "Max Exp", "%d", player->max_exp);
	panel_line(p, COLOUR_L_GREEN, "Adv Exp", "%s", show_adv_exp());
	panel_space(p);
	panel_line(p, COLOUR_L_GREEN, "Gold", "%d", player->au);
	panel_line(p, attr, "Burden", "%.1f lb",
			   player->upkeep->total_weight / 10.0F);
	panel_line(p, attr, "Overweight", "%d.%d lb", -diff / 10, abs(diff) % 10);
	panel_line(p, COLOUR_L_GREEN, "Max Depth", "%s", show_depth());

	return p;
}

static struct panel *get_panel_combat(void) {
	struct panel *p = panel_allocate(9);
	struct object *obj;
	int bth, dam, hit;
	int melee_dice = 1, melee_sides = 1;

	/* AC */
	panel_line(p, COLOUR_L_BLUE, "Armor", "[%d,%+d]",
			player->known_state.ac, player->known_state.to_a);

	/* Melee */
	obj = equipped_item_by_slot_name(player, "weapon");
	bth = (player->state.skills[SKILL_TO_HIT_MELEE] * 10) / BTH_PLUS_ADJ;
	dam = player->known_state.to_d + (obj ? obj->known->to_d : 0);
	hit = player->known_state.to_h + (obj ? obj->known->to_h : 0);

	panel_space(p);

	if (obj) {
		melee_dice = obj->dd;
		melee_sides = obj->ds;
	}

	panel_line(p, COLOUR_L_BLUE, "Melee", "%dd%d,%+d", melee_dice, melee_sides, dam);
	panel_line(p, COLOUR_L_BLUE, "To-hit", "%d,%+d", bth / 10, hit);
	panel_line(p, COLOUR_L_BLUE, "Blows", "%d.%d/turn",
			player->state.num_blows / 100, (player->state.num_blows / 10 % 10));

	/* Ranged */
	obj = equipped_item_by_slot_name(player, "shooting");
	bth = (player->state.skills[SKILL_TO_HIT_BOW] * 10) / BTH_PLUS_ADJ;
	hit = player->known_state.to_h + (obj ? obj->known->to_h : 0);
	dam = obj ? obj->known->to_d : 0;

	panel_space(p);
	panel_line(p, COLOUR_L_BLUE, "Shoot to-dam", "%+d", dam);
	panel_line(p, COLOUR_L_BLUE, "To-hit", "%d,%+d", bth / 10, hit);
	panel_line(p, COLOUR_L_BLUE, "Shots", "%d.%d/turn",
			   player->state.num_shots / 10, player->state.num_shots % 10);

	return p;
}

static struct panel *get_panel_skills(void) {
	struct panel *p = panel_allocate(8);

	int skill;
	byte attr;
	const char *desc;
	int depth = cave ? cave->depth : 0;

#define BOUND(x, min, max)		MIN(max, MAX(min, x))

	/* Saving throw */
	skill = BOUND(player->state.skills[SKILL_SAVE], 0, 100);
	panel_line(p, colour_table[skill / 10], "Saving Throw", "%d%%", skill);

	/* Stealth */
	desc = likert(player->state.skills[SKILL_STEALTH], 1, &attr);
	panel_line(p, attr, "Stealth", "%s", desc);

	/* Physical disarming: assume we're disarming a dungeon trap */
	skill = BOUND(player->state.skills[SKILL_DISARM_PHYS] - depth / 5, 2, 100);
	panel_line(p, colour_table[skill / 10], "Disarm - phys.", "%d%%", skill);

	/* Magical disarming */
	skill = BOUND(player->state.skills[SKILL_DISARM_MAGIC] - depth / 5, 2, 100);
	panel_line(p, colour_table[skill / 10], "Disarm - magic", "%d%%", skill);

	/* Magic devices */
	skill = player->state.skills[SKILL_DEVICE];
	panel_line(p, colour_table[skill / 13], "Magic Devices", "%d", skill);

	/* Searching ability */
	skill = BOUND(player->state.skills[SKILL_SEARCH], 0, 100);
	panel_line(p, colour_table[skill / 10], "Searching", "%d%%", skill);

	/* Infravision */
	panel_line(p, COLOUR_L_GREEN, "Infravision", "%d ft",
			player->state.see_infra * 10);

	/* Speed */
	skill = player->state.speed;
	if (player->timed[TMD_FAST]) skill -= 10;
	if (player->timed[TMD_SLOW]) skill += 10;
	attr = skill < 110 ? COLOUR_L_UMBER : COLOUR_L_GREEN;
	panel_line(p, attr, "Speed", "%s", show_speed());

	return p;
}

static struct panel *get_panel_misc(void) {
	struct panel *p = panel_allocate(7);
	byte attr = COLOUR_L_BLUE;

	panel_line(p, attr, "Age", "%d", player->age);
	panel_line(p, attr, "Height", "%d'%d\"", player->ht / 12, player->ht % 12);
	panel_line(p, attr, "Weight", "%dst %dlb", player->wt / 14, player->wt % 14);
	panel_line(p, attr, "Turns used:", "");
	panel_line(p, attr, "Game", "%d", turn);
	panel_line(p, attr, "Standard", "%d", player->total_energy / 100);
	panel_line(p, attr, "Resting", "%d", player->resting_turn);

	return p;
}

/**
 * Panels for main character screen
 */
static const struct {
	region bounds;
	bool align_left;
	struct panel *(*panel)(void);
} panels[] =
{
	/*   x  y wid rows */
	{ {  1, 1, 40, 7 }, true,  get_panel_topleft },	/* Name, Class, ... */
	{ { 21, 1, 18, 3 }, false, get_panel_misc },	/* Age, ht, wt, ... */
	{ {  1, 9, 24, 9 }, false, get_panel_midleft },	/* Cur Exp, Max Exp, ... */
	{ { 29, 9, 19, 9 }, false, get_panel_combat },
	{ { 52, 9, 20, 8 }, false, get_panel_skills },
};

void display_player_xtra_info(void)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(panels); i++) {
		struct panel *p = panels[i].panel();
		display_panel(p, panels[i].align_left, &panels[i].bounds);
		panel_free(p);
	}

	/* Indent output by 1 character, and wrap at column 72 */
	text_out_wrap = 72;
	text_out_indent = 1;

	/* History */
	Term_gotoxy(text_out_indent, 19);
	text_out_to_screen(COLOUR_WHITE, player->history);

	/* Reset text_out() vars */
	text_out_wrap = 0;
	text_out_indent = 0;

	return;
}

/**
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
	if (Term != angband_term[0] && !player->upkeep->playing) return;

	/* Stat info */
	display_player_stat_info();

	if (mode) {
		struct panel *p = panels[0].panel();
		display_panel(p, panels[0].align_left, &panels[0].bounds);
		panel_free(p);

		/* Stat/Sustain flags */
		display_player_sust_info();

		/* Other flags */
		display_player_flag_info();
	} else {
		/* Extra info */
		display_player_xtra_info();
	}
}


/**
 * Write a character dump
 */
void write_character_dump(ang_file *fff)
{
	int i, x, y;

	int a;
	wchar_t c;

	struct store *home = &stores[STORE_HOME];
	struct object **home_list = mem_zalloc(sizeof(struct object *) *
										   z_info->store_inven_max);
	char o_name[80];

	char buf[1024];
	char *p;

	/* Begin dump */
	file_putf(fff, "  [%s Character Dump]\n\n", buildid);

	/* Display player basics */
	display_player(0);

	/* Dump part of the screen */
	for (y = 1; y < 23; y++) {
		p = buf;
		/* Dump each row */
		for (x = 0; x < 79; x++) {
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			p += wctomb(p, c);
		}

		/* Back up over spaces */
		while ((p > buf) && (p[-1] == ' ')) --p;

		/* Terminate */
		*p = '\0';

		/* End the row */
		file_putf(fff, "%s\n", buf);
	}

	/* Display player resistances etc */
	display_player(1);

	/* Print a header */
	file_putf(fff, format("%-20s%s\n", "Resistances", "Abilities"));

	/* Dump part of the screen */
	for (y = 9; y < 9 + RES_ROWS; y++) {
		p = buf;
		/* Dump each row */
		for (x = 0; x < 39; x++) {
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			p += wctomb(p, c);
		}

		/* Back up over spaces */
		while ((p > buf) && (p[-1] == ' ')) --p;

		/* Terminate */
		*p = '\0';

		/* End the row */
		file_putf(fff, "%s\n", buf);
	}

	/* Skip a line */
	file_putf(fff, "\n");

	/* Print a header */
	file_putf(fff, format("%-20s%s\n", "Hindrances", "Modifiers"));

	/* Dump part of the screen */
	for (y = 9; y < 20; y++) {
		p = buf;
		/* Dump each row */
		for (x = 0; x < 39; x++) {
			/* Get the attr/char */
			(void)(Term_what(x + 40, y, &a, &c));

			/* Dump it */
			p += wctomb(p, c);
		}

		/* Back up over spaces */
		while ((p > buf) && (p[-1] == ' ')) --p;

		/* Terminate */
		*p = '\0';

		/* End the row */
		file_putf(fff, "%s\n", buf);
	}

	/* Skip some lines */
	file_putf(fff, "\n\n");


	/* If dead, dump last messages -- Prfnoff */
	if (player->is_dead) {
		i = messages_num();
		if (i > 15) i = 15;
		file_putf(fff, "  [Last Messages]\n\n");
		while (i-- > 0)
		{
			file_putf(fff, "> %s\n", message_str((s16b)i));
		}
		file_putf(fff, "\nKilled by %s.\n\n", player->died_from);
	}


	/* Dump the equipment */
	file_putf(fff, "  [Character Equipment]\n\n");
	for (i = 0; i < player->body.count; i++) {
		struct object *obj = slot_object(player, i);
		if (!obj) continue;

		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
		file_putf(fff, "%c) %s\n", gear_to_label(obj), o_name);
		object_info_chardump(fff, obj, 5, 72);
	}
	file_putf(fff, "\n\n");

	/* Dump the inventory */
	file_putf(fff, "\n\n  [Character Inventory]\n\n");
	for (i = 0; i < z_info->pack_size; i++) {
		struct object *obj = player->upkeep->inven[i];
		if (!obj) break;

		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
		file_putf(fff, "%c) %s\n", gear_to_label(obj), o_name);
		object_info_chardump(fff, obj, 5, 72);
	}
	file_putf(fff, "\n\n");

	/* Dump the quiver */
	file_putf(fff, "\n\n  [Character Quiver]\n\n");
	for (i = 0; i < z_info->quiver_size; i++) {
		struct object *obj = player->upkeep->quiver[i];
		if (!obj) continue;

		object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
		file_putf(fff, "%c) %s\n", gear_to_label(obj), o_name);
		object_info_chardump(fff, obj, 5, 72);
	}
	file_putf(fff, "\n\n");

	/* Dump the Home -- if anything there */
	store_stock_list(home, home_list, z_info->store_inven_max);
	if (home->stock_num) {
		/* Header */
		file_putf(fff, "  [Home Inventory]\n\n");

		/* Dump all available items */
		for (i = 0; i < z_info->store_inven_max; i++) {
			struct object *obj = home_list[i];
			if (!obj) break;
			object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);
			file_putf(fff, "%c) %s\n", I2A(i), o_name);

			object_info_chardump(fff, obj, 5, 72);
		}

		/* Add an empty line */
		file_putf(fff, "\n\n");
	}

	/* Dump character history */
	dump_history(fff);
	file_putf(fff, "\n\n");

	/* Dump options */
	file_putf(fff, "  [Options]\n\n");

	/* Dump options */
	for (i = 0; i < OP_MAX; i++) {
		int opt;
		const char *title = "";
		switch (i) {
			case OP_INTERFACE: title = "User interface"; break;
			case OP_BIRTH: title = "Birth"; break;
		    default: continue;
		}

		file_putf(fff, "  [%s]\n\n", title);
		for (opt = 0; opt < OPT_MAX; opt++) {
			if (option_type(opt) != i) continue;

			file_putf(fff, "%-45s: %s (%s)\n",
			        option_desc(opt),
			        player->opts.opt[opt] ? "yes" : "no ",
			        option_name(opt));
		}

		/* Skip some lines */
		file_putf(fff, "\n");
	}

	mem_free(home_list);
}

/**
 * Save the lore to a file in the user directory.
 *
 * \param path is the path to the filename
 *
 * \returns true on success, false otherwise.
 */
bool dump_save(const char *path)
{
	if (text_lines_to_file(path, write_character_dump)) {
		msg("Failed to create file %s.new", path);
		return false;
	}

	return true;
}



#define INFO_SCREENS 2 /* Number of screens in character info mode */


/**
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
	ui_event ke;
	int mode = 0;

	const char *p;

	bool more = true;

	/* Prompt */
	p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";

	/* Save screen */
	screen_save();

	/* Forever */
	while (more) {
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, COLOUR_WHITE, p);

		/* Query */
		ke = inkey_ex();

		if ((ke.type == EVT_KBRD)||(ke.type == EVT_BUTTON)) {
			switch (ke.key.code) {
				case ESCAPE: more = false; break;
				case 'c': {
					if(arg_force_name)
						msg("You are not allowed to change your name!");
					else {
					char namebuf[32] = "";

					/* Set player name */
					if (get_character_name(namebuf, sizeof namebuf))
						my_strcpy(player->full_name, namebuf,
								  sizeof(player->full_name));
					}

					break;
				}

				case 'f': {
					char buf[1024];
					char fname[80];

					/* Get the filesystem-safe name and append .txt */
					player_safe_name(fname, sizeof(fname), player->full_name, false);
					my_strcat(fname, ".txt", sizeof(fname));

					if (get_file(fname, buf, sizeof buf)) {
						if (dump_save(buf))
							msg("Character dump successful.");
						else
							msg("Character dump failed!");
					}
					break;
				}
				
				case 'h':
				case ARROW_LEFT:
				case ' ':
					mode = (mode + 1) % INFO_SCREENS;
					break;

				case 'l':
				case ARROW_RIGHT:
					mode = (mode - 1) % INFO_SCREENS;
					break;
			}
		} else if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				/* Flip through the screens */			
				mode = (mode + 1) % INFO_SCREENS;
			} else if (ke.mouse.button == 2) {
				/* exit the screen */
				more = false;
			} else {
				/* Flip backwards through the screens */			
				mode = (mode - 1) % INFO_SCREENS;
			}
		}

		/* Flush messages */
		event_signal(EVENT_MESSAGE_FLUSH);
	}

	/* Load screen */
	screen_load();
}
