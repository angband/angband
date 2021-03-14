/**
 * \file cmd-wizard.c
 * \brief Implements debug commands in Angband 4's command system.
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
#include "cmds.h"
#include "effects.h"
#include "game-input.h"
#include "init.h"
#include "mon-make.h"
#include "mon-util.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-output.h"
#include "ui-target.h"
#include "ui-term.h"


/**
 * Extract a decimal integer from a string while ensuring that only the
 * decimal value and white space are present in the string.
 * \param s is the string to parse.
 * \param val is dereferenced and set to the extracted value.
 * \return true if val was dereferenced and set; false if the extraction
 * failed
 */
static bool get_int_from_string(const char *s, int *val)
{
	char *endptr;
	long lval = strtol(s, &endptr, 10);

	/*
	 * Reject INT_MIN and INT_MAX so, for systems where
	 * sizeof(long) == sizeof(int), it isn't necessary to check errno to
	 * see if the value in the string was out of range.
	 */
	if (s[0] == '\0' ||
			(*endptr != '\0' && !contains_only_spaces(endptr)) ||
			lval <= INT_MIN || lval >= INT_MAX) {
		return false;
	}
	*val = (int)lval;
	return true;
}


/**
 * Redraw the visible portion of the map to accentuate some chosen
 * characteristic.
 *
 * \param c is the chunk to use as the source for data.
 * \param p is the player to use.
 * \param func is a pointer to a function which will set the value pointed
 * to by its fourth argument to whether or not to display the given grid and,
 * if displaying that grid, set its fifth argument to the color to use for the
 * grid.
 * \param closure is passed as the second argument to func.
 *
 * Assumes the active terminal displays a map.
 */
static void wiz_hack_map(struct chunk *c, struct player *p,
	void (*func)(struct chunk *, void *, struct loc, bool *, byte *),
	void *closure)
{
	int y;

	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++) {
		int x;

		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++) {
			struct loc grid = loc(x, y);
			bool show;
			byte color;

			if (!square_in_bounds_fully(c, grid)) continue;

			(*func)(c, closure, grid, &show, &color);
			if (!show) continue;

			if (loc_eq(grid, p->grid)) {
				print_rel(L'@', color, y, x);
			} else if (square_ispassable(c, grid)) {
				print_rel(L'*', color, y, x);
			} else {
				print_rel(L'#', color, y, x);
			}
		}
	}

	Term_redraw();
}


/**
 * Acquire some good or great objects and drop them near the player
 * (CMD_WIZ_ACQUIRE).  Can take the number of objects to acquire from the
 * argument, "quantity", of type number in cmd.  Can take whether to get good
 * or great objects from the argument, "choice", of type choice in cmd:  a
 * non-zero value for that will select great objects.
 */
void do_cmd_wiz_acquire(struct command *cmd)
{
	int n, great;

	if (cmd_get_arg_choice(cmd, "choice", &great) != CMD_OK) {
		great = (get_check("Acquire great objects? ")) ? 1 : 0;
		cmd_set_arg_choice(cmd, "choice", great);
	}

	if (cmd_get_arg_number(cmd, "quantity", &n) != CMD_OK) {
		n = get_quantity((great) ?
			"How many great objects? " : "How many good objects? ",
			40);
		if (n < 1) return;
		cmd_set_arg_number(cmd, "quantity", n);
	}

	acquirement(player->grid, player->depth, n, great);
}


/**
 * Advance the player to level 50 with max stats and other bonuses
 * (CMD_WIZ_ADVANCE).  Takes no arguments from cmd.
 */
void do_cmd_wiz_advance(struct command *cmd)
{
	int i;

	/* Max stats */
	for (i = 0; i < STAT_MAX; i++) {
		player->stat_cur[i] = player->stat_max[i] = 118;
	}

	/* Lots of money */
	player->au = 1000000L;

	/* Level 50 */
	player_exp_gain(player, PY_MAX_EXP);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Get some awesome equipment */
	/* Artifacts: 3, 5, 12, ... */

	/* Flag update and redraw for things not handled in player_exp_gain() */
	player->upkeep->redraw |= PR_GOLD | PR_HP | PR_MANA;
}


/**
 * Instantly cure the player of everything (CMD_WIZ_CURE_ALL).  Takes no
 * arguments from cmd.
 */
void do_cmd_wiz_cure_all(struct command *cmd)
{
	int i;

	/* Remove curses */
	for (i = 0; i < player->body.count; i++) {
		if (player->body.slots[i].obj &&
				player->body.slots[i].obj->curses) {
			mem_free(player->body.slots[i].obj->curses);
			player->body.slots[i].obj->curses = NULL;
		}
	}

	/* Restore stats */
	for (i = 0; i < STAT_MAX; i++) {
		effect_simple(EF_RESTORE_STAT, source_player(), "0", i,
			0, 0, 0, 0, NULL);
	}

	/* Restore the level */
	effect_simple(EF_RESTORE_EXP, source_none(), "0", 0, 0, 0, 0, 0, NULL);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Cure stuff */
	(void) player_clear_timed(player, TMD_BLIND, true);
	(void) player_clear_timed(player, TMD_CONFUSED, true);
	(void) player_clear_timed(player, TMD_POISONED, true);
	(void) player_clear_timed(player, TMD_AFRAID, true);
	(void) player_clear_timed(player, TMD_PARALYZED, true);
	(void) player_clear_timed(player, TMD_IMAGE, true);
	(void) player_clear_timed(player, TMD_STUN, true);
	(void) player_clear_timed(player, TMD_CUT, true);
	(void) player_clear_timed(player, TMD_SLOW, true);
	(void) player_clear_timed(player, TMD_AMNESIA, true);

	/* No longer hungry */
	player_set_timed(player, TMD_FOOD, PY_FOOD_FULL - 1, false);

	/* Flag what needs to be updated or redrawn */
	player->upkeep->update |= PU_TORCH | PU_UPDATE_VIEW | PU_MONSTERS;
	player->upkeep->redraw |= PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP;

	/* Give the player some feedback */
	msg("You feel *much* better!");
}


/**
 * Detect everything nearby (CMD_WIZ_DETECT_ALL_LOCAL).  Takes no arguments
 * from cmd.
 */
void do_cmd_wiz_detect_all_local(struct command *cmd)
{
	effect_simple(EF_DETECT_TRAPS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_DOORS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_STAIRS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_GOLD, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_OBJECTS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_VISIBLE_MONSTERS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
	effect_simple(EF_DETECT_INVISIBLE_MONSTERS, source_player(), "0",
		0, 0, 0, 22, 40, NULL);
}


/**
 * Hit all monsters in the player's line of sight (CMD_WIZ_HIT_ALL_LOS).  Takes
 * no arguments from cmd.
 */
void do_cmd_wiz_hit_all_los(struct command *cmd)
{
	effect_simple(EF_PROJECT_LOS, source_player(), "10000", PROJ_DISP_ALL,
		0, 0, 0, 0, NULL);
}


/**
 * Go to any level, optionally choosing the level generation algorithm
 * (CMD_WIZ_JUMP_LEVEL).  Can take the level to jump to from the argument,
 * "level", of type number in cmd.  Can take whether to choose the generation
 * algorithm from the argument, "choice", of type choice in cmd.
 *
 * Bugs:
 * Because choose_profile() prompts for the generation algorithm, it is not
 * stored in the command and the prompt will be repeated if the command is
 * repeated.
 */
void do_cmd_wiz_jump_level(struct command *cmd)
{
	int level, choose_gen;

	if (cmd_get_arg_number(cmd, "level", &level) != CMD_OK) {
		char prompt[80], s[80];

		strnfmt(prompt, sizeof(prompt), "Jump to level (0-%d): ",
			z_info->max_depth - 1);

		/* Set default */
		strnfmt(s, sizeof(s), "%d", player->depth);

		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &level)) return;
		cmd_set_arg_number(cmd, "level", level);
	}

	/* Paranoia */
	if (level < 0 || level >= z_info->max_depth) return;

	if (cmd_get_arg_choice(cmd, "choice", &choose_gen) != CMD_OK) {
		choose_gen = (get_check("Choose cave profile? ")) ? 1 : 0;
		cmd_set_arg_choice(cmd, "choice", choose_gen);
	}

	if (choose_gen) {
		player->noscore |= NOSCORE_JUMPING;
	}

	msg("You jump to dungeon level %d.", level);
	dungeon_change_level(player, level);

	/*
	 * Because of the structure of the game loop, need to take some energy,
	 * or the change level request will not be processed until after
	 * performing another action that takes energy.
	 */
	player->upkeep->energy_use = z_info->move_energy;
}


/**
 * Make the player aware of all object kinds up to a given level
 * (CMD_WIZ_LEARN_OBJECT_KINDS).  Can take the level from the argument,
 * "level", of type number in cmd.
 */
void do_cmd_wiz_learn_object_kinds(struct command *cmd)
{
	int level, i;

	if (cmd_get_arg_number(cmd, "level", &level) != CMD_OK) {
		char s[80] = "100";

		if (!get_string("Learn object kinds up to level (0-100)? ",
			s, sizeof(s))) return;
		if (!get_int_from_string(s, &level)) return;
		cmd_set_arg_number(cmd, "level", level);
	}

	/* Scan all object kinds */
	for (i = 1; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		/* Induce awareness */
		if (kind && kind->name && kind->level <= level) {
			kind->aware = true;
		}
	}

	update_player_object_knowledge(player);
	msg("You now know about many items!");
}


struct wiz_query_feature_closure {
	const int *features;
	int n;
};


/**
 * Is a helper function passed by do_cmd_wiz_query_feature() to wiz_hack_map().
 *
 * \param c is the chunk to access for data.
 * \param closure is a pointer to a struct wiz_query_feature_closure cast to a
 * pointer to void.  Selects the terrain shown.
 * \param grid is the location in the chunk.
 * \param show is dereferenced and set to true if grid contains one of the
 * types of terrain selected by closure.  Otherwise, it is dereferenced and
 * set to false.
 * \param color is dereferenced and set to the color to use if *show is set
 * to true.  Otherwise, it is not dereferenced.
 */
static void wiz_hack_map_query_feature(struct chunk *c, void *closure,
	struct loc grid, bool *show, byte *color)
{
	const struct wiz_query_feature_closure *sel_feats = closure;
	int i = 0;
	int sq_feat = square(c, grid)->feat;

	while (1) {
		if (i >= sel_feats->n) {
			*show = false;
			return;
		}
		if (sq_feat == sel_feats->features[i]) {
			*show = true;
			*color = (square_ispassable(c, grid)) ?
				COLOUR_YELLOW : COLOUR_RED;
			return;
		}
		++i;
	}
}


/**
 * Redraw the visible portion of the map to highlight certain terrain
 * (CMD_WIZ_QUERY_FEATURE).  Can take the terrain to highlight from the
 * argument, "choice", of type choice in cmd.  This function will need
 * to be changed if the terrain types change.
 */
void do_cmd_wiz_query_feature(struct command *cmd)
{
	int feature_class;
	struct wiz_query_feature_closure selected;
	/* OMG hax */
	const int featf[] = { FEAT_FLOOR };
	const int feato[] = { FEAT_OPEN };
	const int featb[] = { FEAT_BROKEN };
	const int featu[] = { FEAT_LESS };
	const int featz[] = { FEAT_MORE };
	const int featt[] = { FEAT_LESS, FEAT_MORE };
	const int featc[] = { FEAT_CLOSED };
	const int featd[] = { FEAT_CLOSED, FEAT_OPEN, FEAT_BROKEN,
		FEAT_SECRET };
	const int feath[] = { FEAT_SECRET };
	const int featm[] = { FEAT_MAGMA, FEAT_MAGMA_K };
	const int featq[] = { FEAT_QUARTZ, FEAT_QUARTZ_K };
	const int featg[] = { FEAT_GRANITE };
	const int featp[] = { FEAT_PERM };
	const int featr[] = { FEAT_RUBBLE };
	const int feata[] = { FEAT_PASS_RUBBLE };

	if (cmd_get_arg_choice(cmd, "choice", &feature_class) != CMD_OK) {
		char choice;

		if (!get_com("Debug Command Feature Query: ", &choice)) return;
		feature_class = choice;
		cmd_set_arg_choice(cmd, "choice", feature_class);
	}

	switch (feature_class) {
		/* Floors */
		case 'f':
			selected.features = featf;
			selected.n = (int) N_ELEMENTS(featf);
			break;

		/* Open doors */
		case 'o':
			selected.features = feato;
			selected.n = (int) N_ELEMENTS(feato);
			break;

		/* Broken doors */
		case 'b':
			selected.features = featb;
			selected.n = (int) N_ELEMENTS(featb);
			break;

		/* Upstairs */
		case 'u':
			selected.features = featu;
			selected.n = (int) N_ELEMENTS(featu);
			break;

		/* Downstairs */
		case 'z':
			selected.features = featz;
			selected.n = (int) N_ELEMENTS(featz);
			break;

		/* Stairs */
		case 't':
			selected.features = featt;
			selected.n = (int) N_ELEMENTS(featt);
			break;

		/* Closed doors */
		case 'c':
			selected.features = featc;
			selected.n = (int) N_ELEMENTS(featc);
			break;

		/* Doors */
		case 'd':
			selected.features = featd;
			selected.n = (int) N_ELEMENTS(featd);
			break;

		/* Secret doors */
		case 'h':
			selected.features = feath;
			selected.n = (int) N_ELEMENTS(feath);
			break;

		/* Magma */
		case 'm':
			selected.features = featm;
			selected.n = (int) N_ELEMENTS(featm);
			break;

		/* Quartz */
		case 'q':
			selected.features = featq;
			selected.n = (int) N_ELEMENTS(featq);
			break;

		/* Granite */
		case 'g':
			selected.features = featg;
			selected.n = (int) N_ELEMENTS(featg);
			break;

		/* Permanent wall */
		case 'p':
			selected.features = featp;
			selected.n = (int) N_ELEMENTS(featp);
			break;

		/* Rubble */
		case 'r':
			selected.features = featr;
			selected.n = (int) N_ELEMENTS(featr);
			break;

		/* Passable rubble */
		case 'a':
			selected.features = feata;
			selected.n = (int) N_ELEMENTS(feata);
			break;

		/* Invalid entry */
		default:
			msg("That was an invalid selection.  Use one of fobuztcdhmqgpra .");
			return;
	}

	wiz_hack_map(cave, player, wiz_hack_map_query_feature, &selected);

	msg("Press any key.");
	inkey_ex();
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}


/**
 * Is a helper function passed by do_cmd_wiz_query_square_flag() to
 * wiz_hack_map().
 *
 * \param c is the chunk to access for data.
 * \param closure is a pointer to the int with the flag to highlight.
 * \param grid is the location in the chunk.
 * \param show is dereferenced and set to true if the grid has the flag.
 * Otherwise, it is dereferenced and set to false.
 * \param color is dereferenced and set to the color to use if *show is set
 * to true.  Otherwise, it is not dereferenced.
 */
static void wiz_hack_map_query_square_flag(struct chunk *c, void *closure,
	struct loc grid, bool *show, byte *color)
{
	int flag = *((int*)closure);

	/* With a flag, test for that.  Otherwise, test if grid is known. */
	if ((flag && sqinfo_has(square(c, grid)->info, flag)) ||
			(!flag && square_isknown(c, grid))) {
		*show = true;
		*color = (square_ispassable(c, grid)) ?
			COLOUR_YELLOW : COLOUR_RED;
	} else {
		*show = false;
	}
}


/**
 * Redraw the visible portion of the map to highlight squares with a given
 * flag (CMD_WIZ_QUERY_SQUARE_FLAG).  Can take the flag to highlight from the
 * argument, "choice", of type choice in cmd.  This function will need to
 * be changed if list-square-flags.h changes.
 */
void do_cmd_wiz_query_square_flag(struct command *cmd)
{
	int flag = 0;

	if (cmd_get_arg_choice(cmd, "choice", &flag) != CMD_OK) {
		char c;

		if (!get_com("Debug Command Query [grasvwdftniolx]: ", &c))
			return;
		switch (c) {
			case 'g': flag = SQUARE_GLOW; break;
			case 'r': flag = SQUARE_ROOM; break;
			case 'a': flag = SQUARE_VAULT; break;
			case 's': flag = SQUARE_SEEN; break;
			case 'v': flag = SQUARE_VIEW; break;
			case 'w': flag = SQUARE_WASSEEN; break;
			case 'd': flag = SQUARE_DTRAP; break;
			case 'f': flag = SQUARE_FEEL; break;
			case 't': flag = SQUARE_TRAP; break;
			case 'n': flag = SQUARE_INVIS; break;
			case 'i': flag = SQUARE_WALL_INNER; break;
			case 'o': flag = SQUARE_WALL_OUTER; break;
			case 'l': flag = SQUARE_WALL_SOLID; break;
			case 'x': flag = SQUARE_MON_RESTRICT; break;
		}
		cmd_set_arg_choice(cmd, "choice", flag);
	}

	wiz_hack_map(cave, player, wiz_hack_map_query_square_flag, &flag);

	msg("Press any key.");
	inkey_ex();
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}


/**
 * Rerate the player's hit points (CMD_WIZ_RERATE).  Takes no arguments from
 * cmd.
 */
void do_cmd_wiz_rerate(struct command *cmd)
{
	int min_value, max_value, percent;

	min_value = (PY_MAX_LEVEL * 3 * (player->hitdie - 1)) / 8;
	min_value += PY_MAX_LEVEL;

	max_value = (PY_MAX_LEVEL * 5 * (player->hitdie - 1)) / 8;
	max_value += PY_MAX_LEVEL;

	player->player_hp[0] = player->hitdie;

	/* Rerate */
	while (1) {
		int i;

		/* Collect values */
		for (i = 1; i < PY_MAX_LEVEL; i++) {
			player->player_hp[i] = randint1(player->hitdie);
			player->player_hp[i] += player->player_hp[i - 1];
		}

		/* Legal values */
		if (player->player_hp[PY_MAX_LEVEL - 1] >= min_value &&
			player->player_hp[PY_MAX_LEVEL - 1] <= max_value) break;
	}

	percent = (int)(((long)player->player_hp[PY_MAX_LEVEL - 1] * 200L) /
		(player->hitdie + ((PY_MAX_LEVEL - 1) * player->hitdie)));

	/* Update and redraw hitpoints */
	player->upkeep->update |= PU_HP;
	player->upkeep->redraw |= PR_HP;

	msg("Current Life Rating is %d/100.", percent);
}


/**
 * Summon a specific monster (CMD_WIZ_SUMMON_NAMED).  Can take the index
 * of the monster to summon from the argument, "index", of type number in cmd.
 */
void do_cmd_wiz_summon_named(struct command *cmd)
{
	int r_idx, i = 0;
	struct monster_race *r = NULL;
	struct monster_group_info info = { 0, 0 };

	if (cmd_get_arg_number(cmd, "index", &r_idx) == CMD_OK) {
		if (r_idx > 0 && r_idx < z_info->r_max) {
			r = &r_info[r_idx];
		}
	} else {
		char s[80] = "";

		if (!get_string("Summon which monster? ", s, sizeof(s))) return;
		/* See if an index was entered */
		if (get_int_from_string(s, &r_idx)) {
			if (r_idx > 0 && r_idx < z_info->r_max) {
				r = &r_info[r_idx];
			}
		} else {
			/* If not, find by name */
			r = lookup_monster(s);
		}
		if (r != NULL) {
			cmd_set_arg_number(cmd, "index", r->ridx);
		}
	}

	if (r == NULL) {
		msg("No monster found.");
		return;
	}

	/* Try 10 times */
	while (1) {
		struct loc grid;

		if (i >= 10) {
			msg("Could not place monster.");
			break;
		}

		/* Pick a location */
		scatter(cave, &grid, player->grid, 1, true);

		/* Try to place (allowing groups) if empty */
		if (square_isempty(cave, grid) &&
				place_new_monster(cave, grid, r, true, true,
				info, ORIGIN_DROP_WIZARD)) {
			player->upkeep->redraw |= PR_MAP | PR_MONLIST;
			break;
		}

		++i;
	}
}


/**
 * Teleport the player randomly with a given approximate range
 * (CMD_WIZ_TELEPORT_RANDOM).  Can take the range from the argument, "range",
 * of type number in cmd.
 */
void do_cmd_wiz_teleport_random(struct command *cmd)
{
	int range;
	char sr[30];

	if (cmd_get_arg_number(cmd, "range", &range) != CMD_OK) {
		char s[80] = "100";

		if (!get_string("Teleport range? ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &range) || range < 1) return;
		cmd_set_arg_number(cmd, "range", range);
	}

	strnfmt(sr, sizeof(sr), "%d", range);
	effect_simple(EF_TELEPORT, source_player(), sr, 0, 0, 0, 0, 0, NULL);
}


/**
 * Teleport to the requested position (CMD_WIZ_TELEPORT_TO).  Can take the
 * position from the argument, "point", of type point in cmd.
 */
void do_cmd_wiz_teleport_to(struct command *cmd)
{
	struct loc grid;

	if (cmd_get_arg_point(cmd, "point", &grid) != CMD_OK) {
		/* Use the targeting function. */
		if (!target_set_interactive(TARGET_LOOK, -1, -1)) return;

		/* Grab the target coordinates. */
		target_get(&grid);

		/* Record in the command to facilitate repetition. */
		cmd_set_arg_point(cmd, "point", grid);
	}

	/* Test for passable terrain. */
	if (square_ispassable(cave, grid)) {
		/* Teleport to the target */
		effect_simple(EF_TELEPORT_TO, source_player(), "0", 0, 0, 0,
			grid.y, grid.x, NULL);
	} else {
		msg("The square you are aiming for is impassable.");
	}
}
