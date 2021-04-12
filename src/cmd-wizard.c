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
#include "generate.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-util.h"
#include "obj-curse.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"
#include "trap.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-output.h"
#include "ui-target.h"
#include "wizard.h"


/*
 * Used by do_cmd_wiz_edit_player_*() functions to track the state of the
 * editing process.
 */
enum EditPlayerState {
	EDIT_PLAYER_UNKNOWN, EDIT_PLAYER_STARTED, EDIT_PLAYER_BREAK
} edit_player_state = EDIT_PLAYER_UNKNOWN;


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
 * Extract a long decimal integer from a string while ensuring that only the
 * decimal value and white space are present in the string.
 * \param s is the string to parse.
 * \param val is dereferenced and set to the extracted value.
 * \return true if val was dereferenced and set; false if the extraction
 * failed
 */
static bool get_long_from_string(const char *s, long *val)
{
	char *endptr;
	long lval = strtol(s, &endptr, 10);

	/*
	 * Reject LONG_MIN and LONG_MAX so it isn't necessary to check errno
	 * to see if the value in the string was out of range.
	 */
	if (s[0] == '\0' ||
			(*endptr != '\0' && !contains_only_spaces(endptr)) ||
			lval <= LONG_MIN || lval >= LONG_MAX) {
		return false;
	}
	*val = lval;
	return true;
}


/**
 * Output part of a bitflag set in binary format.
 */
static void prt_binary(const bitflag *flags, int offset, int row, int col,
	wchar_t ch, int num)
{
	int flag;

	/* Scan the flags. */
	for (flag = FLAG_START + offset; flag < FLAG_START + offset + num; flag++) {
		if (of_has(flags, flag)) {
			Term_putch(col++, row, COLOUR_BLUE, ch);
		} else {
			Term_putch(col++, row, COLOUR_WHITE, L'-');
		}
	}
}


/**
 * Create an instance of an artifact.
 *
 * \param art The artifact to instantiate.
 * \return An object that represents the artifact.
 */
static struct object *wiz_create_object_from_artifact(struct artifact *art)
{
	struct object_kind *kind;
	struct object *obj;

	/* Ignore "empty" artifacts */
	if (!art->name) return NULL;

	/* Acquire the "kind" index */
	kind = lookup_kind(art->tval, art->sval);
	if (!kind) return NULL;

	/* Create the artifact */
	obj = object_new();
	object_prep(obj, kind, art->alloc_min, RANDOMISE);
	obj->artifact = art;
	copy_artifact_data(obj, art);

	/* Mark that the artifact has been created. */
	art->created = true;

	return obj;
}


/**
 * Create an instance of an object of a given kind.
 *
 * \param kind The base type of object to instantiate.
 * \return An object of the provided object type.
 */
static struct object *wiz_create_object_from_kind(struct object_kind *kind)
{
	struct object *obj;

	if (tval_is_money_k(kind)) {
		obj = make_gold(player->depth, kind->name);
	} else {
		obj = object_new();
		object_prep(obj, kind, player->depth, RANDOMISE);

		/* Apply magic (no messages, no artifacts) */
		apply_magic(obj, player->depth, false, false, false, false);
	}

	return obj;
}


/**
 * Display an item's properties.
 */
static void wiz_display_item(const struct object *obj, bool all)
{
	static const char *flagLabels[] = {
		#define OF(a, b) b,
		#include "list-object-flags.h"
		#undef OF
	};
	int j = 0, i, k, nflg;
	bitflag f[OF_SIZE];
	char buf[256];
	bool *labelsDone;

	/* Extract the flags. */
	if (all) {
		object_flags(obj, f);
	} else {
		object_flags_known(obj, f);
	}

	/* Clear screen. */
	Term_clear();

	/* Describe fully */
	object_desc(buf, sizeof(buf), obj,
		ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

	prt(buf, 2, j);

	prt(format("combat = (%dd%d) (%+d,%+d) [%d,%+d]",
		obj->dd, obj->ds, obj->to_h, obj->to_d, obj->ac, obj->to_a),
		4, j);

	prt(format("kind = %-5d  tval = %-5d  sval = %-5d  wgt = %-3d     timeout = %-d",
		obj->kind->kidx, obj->tval, obj->sval, obj->weight,
		obj->timeout), 5, j);

	prt(format("number = %-3d  pval = %-5d  name1 = %-4d  egoidx = %-4d  cost = %ld",
		obj->number, obj->pval,
		obj->artifact ? obj->artifact->aidx : 0,
		obj->ego ? (int) obj->ego->eidx : -1,
		(long)object_value(obj, 1)), 6, j);

	nflg = MIN(OF_MAX - FLAG_START, 80);

	/* Set up header line. */
	if (nflg >= 6) {
		buf[0] = '+';
		k = (nflg - 6) / 2;
		for (i = 1; i < k; ++i) {
			buf[i] = '-';
		}
	} else {
		k = 0;
	}
	buf[k] = 'F';
	buf[k + 1] = 'L';
	buf[k + 2] = 'A';
	buf[k + 3] = 'G';
	buf[k + 4] = 'S';
	for (i = k + 5; i < nflg - 1; ++i) {
		buf[i] = '-';
	}
	if (nflg >= 7) {
		buf[nflg - 1] = '+';
		buf[nflg] = '\0';
	} else {
		buf[k + 5] = '\0';
	}
	prt(buf, 16, j);

	/* Display first five letters of flag labels vertically. */
	labelsDone = mem_zalloc(nflg * sizeof(*labelsDone));
	for (k = 0; k < 5; ++k) {
		for (i = 0; i < nflg; ++i) {
			if (labelsDone[i]) {
				buf[i] = ' ';
			} else if (flagLabels[i][k] == '\0') {
				labelsDone[i] = true;
				buf[i] = ' ';
			} else {
				buf[i] = flagLabels[i][k];
			}
		}
		buf[nflg] = '\0';
		prt(buf, 17 + k, j);
	}
	mem_free(labelsDone);

	prt_binary(f, 0, 22, j, L'*', nflg);
	if (obj->known) {
		prt_binary(obj->known->flags, 0, 23, j, L'+', nflg);
	}
}


/**
 * Drop an object near the player in a manner suitable for debugging.
 *
 * \param obj The object to drop.
 */
static void wiz_drop_object(struct object *obj)
{
	if (obj == NULL) return;

	/* Mark as cheat and where it was created */
	obj->origin = ORIGIN_CHEAT;
	obj->origin_depth = player->depth;

	/* Drop the object from heaven. */
	drop_near(cave, &obj, 0, player->grid, true, true);
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
}


/**
 * Update the queued command object for do_cmd_wiz_play_item() to reflect
 * that the item has changed.
 *
 * Hack:  this assumes that there's only one command queued.
 */
static void wiz_play_item_notify_changed(void)
{
	struct command *cmd = cmdq_peek();

	if (cmd) {
		assert(cmd->code == CMD_WIZ_PLAY_ITEM);
		cmd_set_arg_choice(cmd, "changed", 1);
	}
}


/**
 * Handle the typical updates needed to upkeep flags after playing with an item.
 */
static void wiz_play_item_standard_upkeep(struct player *p, struct object *obj)
{
	if (object_is_carried(p, obj)) {
		p->upkeep->update |= (PU_BONUS | PU_INVEN);
		p->upkeep->notice |= (PN_COMBINE);
		p->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	} else {
		p->upkeep->redraw |= (PR_ITEMLIST);
	}
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
 * Banish nearby monsters (CMD_WIZ_BANISH).  Can take the range of the effect
 * from the argument, "range", of type number in cmd.
 */
void do_cmd_wiz_banish(struct command *cmd)
{
	int d, i;

	if (cmd_get_arg_number(cmd, "range", &d) != CMD_OK) {
		d = get_quantity("Zap within what distance? ",
			z_info->max_sight);
		cmd_set_arg_number(cmd, "range", d);
	}

	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Skip distant monsters */
		if (mon->cdis > d) continue;

		/* Delete the monster */
		delete_monster_idx(i);
	}

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;
}


/**
 * Change the quantity of an item (CMD_WIZ_CHANGE_ITEM_QUANTITY).  Can take
 * the item to modify from the argument, "item", of type item in cmd.  Can
 * take the new amount from the argument, "quantity", of type number in cmd.
 * Takes whether to update player information from the argument, "update",
 * of type choice in cmd.
 */
void do_cmd_wiz_change_item_quantity(struct command *cmd)
{
	struct object *obj;
	int n, nmax;
	int update = 0;

	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK) {
		if (!get_item(&obj, "Change quantity of which item? ",
				"You have nothing to change.", cmd->code,
				NULL, (USE_INVEN | USE_QUIVER | USE_FLOOR))) {
			return;
		}
		cmd_set_arg_item(cmd, "item", obj);
	} else if (object_is_equipped(player->body, obj)) {
		msg("Can not change the quantity of an equipped item.");
		return;
	}

	/* Don't allow multiple artifacts. */
	if (obj->artifact) {
		msg("Can not modify the quantity of an artifact.");
		return;
	}

	/* Find limit on stack size. */
	nmax = obj->kind->base->max_stack;
	if (tval_can_have_charges(obj) && obj->pval > 0 && obj->number > 0) {
		/* Items with charges have a limit imposed by MAX_PVAL. */
		nmax = MIN((MAX_PVAL * obj->number) / obj->pval, nmax);
	}
	if (object_is_in_quiver(player, obj)) {
		/* The quiver may have stricter limits. */
		nmax = MIN(z_info->quiver_slot_size /
			(tval_is_ammo(obj) ? 1 : z_info->thrown_quiver_mult),
			nmax);
	}

	/* Get the new quantity. */
	if (cmd_get_arg_number(cmd, "quantity", &n) != CMD_OK) {
		char prompt[80], s[80];

		strnfmt(prompt, sizeof(prompt), "Quantity (1-%d): ", nmax);

		/* Set the default value. */
		strnfmt(s, sizeof(s), "%d", obj->number);

		if (!get_string(prompt, s, sizeof(s)) ||
				!get_int_from_string(s, &n) ||
				n < 1 || n > obj->kind->base->max_stack) {
			return;
		}

		cmd_set_arg_number(cmd, "quantity", n);
	}

	/* Impose limits. */
	n = MAX(1, MIN(nmax, n));

	if (n != obj->number) {
		/* Adjust charges or timeouts for devices. */
		if (tval_can_have_charges(obj) && obj->number > 0) {
			obj->pval = (obj->pval * n) / obj->number;
		}
		if (tval_can_have_timeout(obj) && obj->number > 0) {
			obj->timeout = (obj->timeout * n) / obj->number;
		}

		/* Accept change. */
		if (cmd_get_arg_choice(cmd, "update", &update) != CMD_OK ||
				update) {
			if (object_is_carried(player, obj)) {
				/*
				 * Remove the weight of the old number of
				 * objects.
				 */
				player->upkeep->total_weight -=
					obj->number * obj->weight;

				/*
				 * Add the weight of the new number of objects.
				 */
				player->upkeep->total_weight += n * obj->weight;
			}
			wiz_play_item_standard_upkeep(player, obj);
		} else {
			wiz_play_item_notify_changed();
		}
		obj->number = n;
	}
}


/**
 * Generate levels and collect statistics about those with disconnected areas
 * and those where the player is disconnected from stairs
 * (CMD_WIZ_COLLECT_DISCONNECT_STATS).  Can take the number of simulations
 * from the argument, "quantity", of type number in cmd.  Can take whether to
 * stop if a disconnected level is found from the argument, "choice", of type
 * choice in cmd (a nonzero value means stop).
 */
void do_cmd_wiz_collect_disconnect_stats(struct command *cmd)
{
	/* Record last-used value to be the default in next run. */
	static int default_nsim = 50;
	int nsim, stop_on_disconnect;

	if (!stats_are_enabled()) return;

	if (cmd_get_arg_number(cmd, "quantity", &nsim) != CMD_OK) {
		char s[80];

		/* Set default. */
		strnfmt(s, sizeof(s), "%d", default_nsim);

		if (!get_string("Number of simulations: ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &nsim) || nsim < 1) return;
		cmd_set_arg_number(cmd, "quantity", nsim);
	}
	default_nsim = nsim;

	if (cmd_get_arg_choice(cmd, "choice", &stop_on_disconnect) != CMD_OK) {
		stop_on_disconnect =
			get_check("Stop if disconnected level found? ") ? 1 : 0;
		cmd_set_arg_choice(cmd, "choice", stop_on_disconnect);
	}

	disconnect_stats(nsim, stop_on_disconnect != 0);
}


/**
 * Generate levels and collect statistics about the included objects and
 * monsters (CMD_WIZ_COLLECT_OBJ_MON_STATS).  Can take the number of
 * simulations from the argument, "quantity", of type number in cmd.  Can take
 * the type of simulation (diving (1), clearing (2), or clearing with randart
 * regeneration (3)) from the argument, "choice", of type choice in cmd.
 */
void do_cmd_wiz_collect_obj_mon_stats(struct command *cmd)
{
	/* Record last-used values to be the default in next run. */
	static int default_nsim = 50;
	static int default_simtype = 1;
	int nsim, simtype;
	char s[80];

	if (!stats_are_enabled()) return;

	if (cmd_get_arg_number(cmd, "quantity", &nsim) != CMD_OK) {
		/* Set default. */
		strnfmt(s, sizeof(s), "%d", default_nsim);

		if (!get_string("Number of simulations: ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &nsim) || nsim < 1) return;
		cmd_set_arg_number(cmd, "quantity", nsim);
	}
	default_nsim = nsim;

	if (cmd_get_arg_choice(cmd, "choice", &simtype) != CMD_OK) {
		/* Set default. */
		strnfmt(s, sizeof(s), "%d", default_simtype);

		if (!get_string("Type of Sim: Diving (1) or Clearing (2) ",
			s, sizeof(s))) return;
		if (!get_int_from_string(s, &simtype) || simtype < 1 ||
			simtype > 2) return;
		if (simtype == 2) {
			if (get_check("Regen randarts (warning SLOW)? ")) {
				simtype = 3;
			}
		}
		cmd_set_arg_choice(cmd, "choice", simtype);
	}
	default_simtype = (simtype == 1) ? 1 : 2;

	stats_collect(nsim, simtype);
}


/**
 * Generate several pits and collect statistics about the types of monsters
 * used (CMD_WIZ_COLLECT_PIT_STATS).  Can take the number of simulations from
 * the argument, "quantity", of type number in cmd.  Can take the depth to use
 * for the simulations from the argument, "depth", of type number in cmd.  Can
 * take the type of pit (pit (1), nest (2), or other (3)) from the argument,
 * "choice", of type choice in cmd.
 */
void do_cmd_wiz_collect_pit_stats(struct command *cmd)
{
	int nsim, depth, pittype;
	char s[80];

	if (!stats_are_enabled()) return;

	if (cmd_get_arg_number(cmd, "quantity", &nsim) != CMD_OK) {
		/* Set default. */
		strnfmt(s, sizeof(s), "%d", 1000);

		if (!get_string("Number of simulations: ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &nsim) || nsim < 1) return;
		cmd_set_arg_number(cmd, "quantity", nsim);
	}

	if (cmd_get_arg_choice(cmd, "choice", &pittype) != CMD_OK) {
		/* Set default. */
		strnfmt(s, sizeof(s), "%d", 1);

		if (!get_string("Pit type (1-3): ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &pittype) || pittype < 1 ||
			pittype > 3) return;
		cmd_set_arg_choice(cmd, "choice", pittype);
	}

	if (cmd_get_arg_number(cmd, "depth", &depth) != CMD_OK) {
		/* Set default. */
		strnfmt(s, sizeof(s), "%d", player->depth);

		if (!get_string("Depth: ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &depth) || depth < 1) return;
		cmd_set_arg_number(cmd, "depth", depth);
	}

	pit_stats(nsim, pittype, depth);
}


/**
 * Create all artifacts and drop them near the player
 * (CMD_WIZ_CREATE_ALL_ARTIFACT).  Takes no arguments from cmd.
 */
void do_cmd_wiz_create_all_artifact(struct command *cmd)
{
	int i;

	for (i = 1; i < z_info->a_max; i++) {
		struct artifact *art = &a_info[i];
		struct object *obj = wiz_create_object_from_artifact(art);

		wiz_drop_object(obj);
	}
}


/**
 * Create all artifacts of a given tval and drop them near the player
 * (CMD_WIZ_CREATE_ALL_ARTIFACT_FROM_TVAL).  Can take the tval to use from
 * the argument, "tval", of type number in cmd.
 */
void do_cmd_wiz_create_all_artifact_from_tval(struct command *cmd)
{
	int tval, i;

	if (cmd_get_arg_number(cmd, "tval", &tval) != CMD_OK) {
		char prompt[80];
		char s[80] = "";

		strnfmt(prompt, sizeof(prompt),
			"Create all artifacts of which tval (1-%d)? ",
			TV_MAX - 1);
		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &tval) || tval < 1 ||
			tval >= TV_MAX) return;
		cmd_set_arg_number(cmd, "tval", tval);
	}

	for (i = 1; i < z_info->a_max; i++) {
		struct artifact *art = &a_info[i];

		if (art->tval == tval) {
			struct object *obj =
				wiz_create_object_from_artifact(art);

			wiz_drop_object(obj);
		}
	}
}


/**
 * Create one of each kind of ordinary object and drop them near the player
 * (CMD_WIZ_CREATE_ALL_OBJ).  Takes no arguments from cmd.
 */
void do_cmd_wiz_create_all_obj(struct command *cmd)
{
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];
		struct object *obj;

		if (kind->base == NULL || kind->base->name == NULL) continue;
		if (kf_has(kind->kind_flags, KF_INSTA_ART)) continue;

		obj = wiz_create_object_from_kind(kind);
		wiz_drop_object(obj);
	}
}


/**
 * Create one of each kind of object from a tval and drop them near the player
 * (CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL).  Can take the tval to use from the
 * argument, "tval", of type number in cmd.  Can take whether to create instant
 * artifacts from the argument, "choice", of type choice in cmd.
 */
void do_cmd_wiz_create_all_obj_from_tval(struct command *cmd)
{
	int tval, art;
	int i;

	if (cmd_get_arg_number(cmd, "tval", &tval) != CMD_OK) {
		char prompt[80];
		char s[80] = "";

		strnfmt(prompt, sizeof(prompt),
			"Create all items of which tval (1-%d)? ", TV_MAX - 1);
		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &tval) || tval < 1 ||
			tval >= TV_MAX) return;
		cmd_set_arg_number(cmd, "tval", tval);
	}

	if (cmd_get_arg_choice(cmd, "choice", &art) != CMD_OK) {
		art = get_check("Create instant artifacts? ");
		cmd_set_arg_choice(cmd, "choice", art);
	}

	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];
		struct object *obj;

		if (kind->tval != tval ||
			(!art && kf_has(kind->kind_flags, KF_INSTA_ART))) continue;

		obj = wiz_create_object_from_kind(kind);
		wiz_drop_object(obj);
	}
}


/**
 * Create a specified artifact (CMD_WIZ_CREATE_ARTIFACT).  Can take the index
 * of the artifact to create from the argument, "index", of type number in cmd.
 */
void do_cmd_wiz_create_artifact(struct command *cmd)
{
	int ind;

	if (cmd_get_arg_number(cmd, "index", &ind) != CMD_OK) {
		char prompt[80];
		char s[80] = "";

		strnfmt(prompt, sizeof(prompt),
			"Create which artifact (1-%d)? ", z_info->a_max - 1);
		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &ind)) return;
		cmd_set_arg_number(cmd, "index", ind);
	}

	if (ind >= 1 && ind < z_info->a_max) {
		struct artifact *art = &a_info[ind];
		struct object *obj = wiz_create_object_from_artifact(art);

		wiz_drop_object(obj);
	} else {
		msg("That's not a valid artifact.");
	}
}


/**
 * Create an object of a given kind and drop it near the player
 * (CMD_WIZ_CREATE_OBJ).  Can take the index of the kind of object from the
 * argument, "index", of type number in cmd.
 */
void do_cmd_wiz_create_obj(struct command *cmd)
{
	int ind;

	if (cmd_get_arg_number(cmd, "index", &ind) != CMD_OK) {
		char prompt[80];
		char s[80] = "";

		strnfmt(prompt, sizeof(prompt),
			"Create which object (0-%d)? ", z_info->k_max - 1);
		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &ind)) return;
		cmd_set_arg_number(cmd, "index", ind);
	}

	if (ind >= 0 && ind < z_info->k_max) {
		struct object_kind *kind = &k_info[ind];
		struct object *obj = wiz_create_object_from_kind(kind);

		wiz_drop_object(obj);
	} else {
		msg("That's not a valid kind of object.");
	}
}


/**
 * Create a trap at the player's position (CMD_WIZ_CREATE_TRAP).  Can take
 * the type of trap to generate from the argument, "index", of type number
 * in cmd.
 */
void do_cmd_wiz_create_trap(struct command *cmd)
{
	int tidx;

	if (cmd_get_arg_number(cmd, "index", &tidx) != CMD_OK) {
		char s[80] = "";

		if (!get_string("Create which trap? ", s, sizeof(s))) return;
		if (!get_int_from_string(s, &tidx)) {
			const struct trap_kind *trap = lookup_trap(s);

			tidx = (trap) ? trap->tidx : z_info->trap_max;
		}
		cmd_set_arg_number(cmd, "index", tidx);
	}

	if (!square_isfloor(cave, player->grid)) {
		msg("You can't place a trap there!");
	} else if (player->depth == 0) {
		msg("You can't place a trap in the town!");
	} else if (tidx < 1 || tidx >= z_info->trap_max) {
		msg("Trap not found.");
	} else {
		place_trap(cave, player->grid, tidx, 0);
		/* Can not repeat since there's now a trap here. */
		cmd_disable_repeat();
	}
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
 * Change a curse on an item (CMD_WIZ_CURSE_ITEM).  Can take the item to use
 * from the argument, "item", of type item.  Can take the index of the curse
 * to use from the argument, "index", of type number.  Can take the power of
 * the curse from the argument, "power", of type number.  Using a power of
 * zero will remove a curse.  Takes whether to update player information
 * from the argument, "update", of type choice; when update is zero, something
 * else, presumably do_cmd_wiz_play_item(), handles the update.
 */
void do_cmd_wiz_curse_item(struct command *cmd)
{
	struct object *obj;
	int curse_index, power;
	char s[80];
	bool changed;

	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK) {
		if (!get_item(&obj, "Change curse on which item? ",
				"You have nothing to change.", cmd->code,
				NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER |
				USE_FLOOR))) {
			return;
		}
		cmd_set_arg_item(cmd, "item", obj);
	}

	/* Get curse. */
	if (cmd_get_arg_number(cmd, "index", &curse_index) != CMD_OK) {
		strnfmt(s, sizeof(s), "0");
		if (!get_string("Enter curse name or index: ",
				s, sizeof(s))) return;
		if (!get_int_from_string(s, &curse_index)) {
			curse_index = lookup_curse(s);
		}
		cmd_set_arg_number(cmd, "index", curse_index);
	}
	if (curse_index <= 0 || curse_index >= z_info->curse_max) {
		return;
	}

	/* Get power for curse. */
	if (cmd_get_arg_number(cmd, "power", &power) != CMD_OK) {
		strnfmt(s, sizeof(s), "0");
		if (!get_string("Enter curse power (0 removes): ", s,
				sizeof(s)) || !get_int_from_string(s, &power)) {
			return;
		}
		cmd_set_arg_number(cmd, "power", power);
	}
	if (power < 0) {
		return;
	}

	/* Apply. */
	if (power) {
		append_object_curse(obj, curse_index, power);
		changed = true;
	} else if (obj->curses) {
		int i = 0;

		changed = (obj->curses[curse_index].power > 0);
		obj->curses[curse_index].power = 0;

		/* Duplicates logic from non-public check_object_curses(). */
		while (1) {
			if (i >= z_info->curse_max) {
				changed = true;
				mem_free(obj->curses);
				obj->curses = NULL;
				break;
			}
			if (obj->curses[i].power) {
				break;
			}
			++i;
		}
	} else {
		changed = false;
	}

	if (changed) {
		int update = 0;

		if (cmd_get_arg_choice(cmd, "update", &update) != CMD_OK ||
				update) {
			wiz_play_item_standard_upkeep(player, obj);
		} else {
			wiz_play_item_notify_changed();
		}
	}
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
 * Detect all monsters (CMD_WIZ_DETECT_ALL_MONSTERS).  Takes no arguments
 * from cmd.
 */
void do_cmd_wiz_detect_all_monsters(struct command *cmd)
{
	effect_simple(EF_DETECT_VISIBLE_MONSTERS, source_player(), "0",
		0, 0, 0, 500, 500, NULL);
	effect_simple(EF_DETECT_INVISIBLE_MONSTERS, source_player(), "0",
		0, 0, 0, 500, 500, NULL);
}


/**
 * Display the keycodes the user has been generating (CMD_WIZ_DISPLAY_KEYLOG).
 * Takes no arguments from cmd.
 */
void do_cmd_wiz_display_keylog(struct command *cmd)
{
	int i;
	char buf[50];
	char buf2[12];
	struct keypress keys[2] = {KEYPRESS_NULL, KEYPRESS_NULL};

	screen_save();

	prt("Previous keypresses (top most recent):", 0, 0);

	for (i = 0; i < KEYLOG_SIZE; i++) {
		if (i < log_size) {
			/*
			 * Find the keypress from the log; log_i is one past
			 * the most recent.
			 */
			int j = (log_i > i) ?
				log_i - i - 1 : log_i - i - 1 + KEYLOG_SIZE;
			struct keypress k = keylog[j];

			/*
			 * ugh. it would be nice if there was a version of
			 * keypress_to_text which took only one keypress.
			 */
			keys[0] = k;
			keypress_to_text(buf2, sizeof(buf2), keys, true);

			/* format this line of output */
			strnfmt(buf, sizeof(buf), "    %-12s (code=%u mods=%u)",
				buf2, k.code, k.mods);
		} else {
			/* create a blank line of output */
			strnfmt(buf, sizeof(buf), "%40s", "");
		}

		prt(buf, i + 1, 0);
	}

	prt("Press any key to continue.", KEYLOG_SIZE + 1, 0);
	anykey();
	screen_load();
}


/**
 * Dump a map of the current level as an HTML file (CMD_WIZ_DUMP_LEVEL_MAP).
 * Takes no arguments from cmd.
 *
 * Bugs:
 * The path and title could be passed through arguments of type string on the
 * command.  That, however, is problematic because there's nothing to say how
 * the lifetime of the string should be handled (does it need to be freed at
 * all, should it be freed here which would break the option to repeat the
 * command, or should it be freed within cmd-core.c as part of managing the
 * lifecycle for the command).
 */
void do_cmd_wiz_dump_level_map(struct command *cmd)
{
	char path[1024] = "";
	char title[80];
	ang_file *fo;

	strnfmt(title, sizeof(title), "Map of level %d", player->depth);
	if (!get_file("level.html", path, sizeof(path)) ||
			!get_string("Title for map: ", title, sizeof(title))) {
		return;
	}
	fo = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (fo) {
		dump_level(fo, title, cave, NULL);
		if (file_close(fo)) {
			msg(format("Level dumped to %s.", path));
		}
	}
}


/**
 * Edit the player's amount of experience (CMD_WIZ_EDIT_PLAYER_EXP).  Takes
 * no arguments from cmd.
 */
void do_cmd_wiz_edit_player_exp(struct command *cmd)
{
	char s[80];
	long newv;

	if (edit_player_state == EDIT_PLAYER_BREAK) return;

	/* Set default value. */
	strnfmt(s, sizeof(s), "%ld", (long)(player->exp));

	if (!get_string("Experience: ", s, sizeof(s)) ||
			!get_long_from_string(s, &newv)) {
		/* Set next editing stage to break. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}

	/* Keep in the bounds of [0, PY_MAX_EXP]. */
	newv = MIN(PY_MAX_EXP, MAX(0, newv));

	if (newv > player->exp) {
		player_exp_gain(player, newv - player->exp);
	} else {
		player_exp_lose(player, player->exp - newv, false);
	}
}


/**
 * Edit the player's amount of gold (CMD_WIZ_EDIT_PLAYER_GOLD).  Takes no
 * arguments from cmd.
 */
void do_cmd_wiz_edit_player_gold(struct command *cmd)
{
	char s[80];
	long newv;

	if (edit_player_state == EDIT_PLAYER_BREAK) return;

	/* Set default value. */
	strnfmt(s, sizeof(s), "%ld", (long)(player->au));

	if (!get_string("Gold: ", s, sizeof(s)) ||
			!get_long_from_string(s, &newv)) {
		/* Set next editing stage to break. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}

	/*
	 * Keep in the bounds of [0, maximum s32b].  Assumes a two's complement
	 * representation.
	 */
	player->au = MIN((s32b)((1UL << 31) - 1), MAX(0, newv));

	/* Flag what needs to be updated or redrawn. */
	player->upkeep->redraw |= PR_GOLD;
}


/**
 * Start editing the player (CMD_WIZ_EDIT_PLAYER_START).  Takes no arguments
 * from cmd.  Because of the use of static state (edit_player_state), this is
 * not reentrant.
 */
void do_cmd_wiz_edit_player_start(struct command *cmd)
{
	int i;

	if (edit_player_state != EDIT_PLAYER_UNKNOWN) {
		/*
		 * Invoked as the cleanup stage for an edit session to work
		 * nicely with command repetition.
		 */
		edit_player_state = EDIT_PLAYER_UNKNOWN;
		return;
	}
	edit_player_state = EDIT_PLAYER_STARTED;
	for (i = 0; i < STAT_MAX; ++i) {
		if (cmdq_push(CMD_WIZ_EDIT_PLAYER_STAT) != 0) {
			/* Failed.  Skip any queued edit commands. */
			edit_player_state = EDIT_PLAYER_BREAK;
			return;
		}
		cmd_set_arg_choice(cmdq_peek(), "choice", i);
	}
	if (cmdq_push(CMD_WIZ_EDIT_PLAYER_GOLD) != 0) {
		/* Failed.  Skip any queued edit commands. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}
	if (cmdq_push(CMD_WIZ_EDIT_PLAYER_EXP) != 0) {
		/* Failed.  Skip any queued edit commands. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}
	/* Make the last command look like the first so repetition works. */
	if (cmdq_push(CMD_WIZ_EDIT_PLAYER_START) != 0) {
		/* Failed. Skip any queued edit commands. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}
}


/**
 * Edit one of the player's statistics (CMD_WIZ_EDIT_PLAYER_STAT).  Takes
 * the index of the statistic to edit from the argument, "choice", of type
 * choice in the command.
 */
void do_cmd_wiz_edit_player_stat(struct command *cmd)
{
	int stat, newv;
	char prompt[80], s[80];

	if (edit_player_state == EDIT_PLAYER_BREAK) return;

	if (cmd_get_arg_choice(cmd, "choice", &stat) != CMD_OK) {
		strnfmt(prompt, sizeof(prompt),
			"Edit which stat (name or 0-%d): ", STAT_MAX - 1);

		/* Set default value. */
		strnfmt(s, sizeof(s), "%s", stat_idx_to_name(0));

		if (!get_string(prompt, s, sizeof(s))) return;
		if (!get_int_from_string(s, &stat)) {
			stat = stat_name_to_idx(s);
			if (stat < 0) {
				return;
			}
		}

		cmd_set_arg_choice(cmd, "choice", stat);
	}

	if (stat < 0 || stat >= STAT_MAX) {
		return;
	}

	strnfmt(prompt, sizeof(prompt), "%s (3-118): ", stat_idx_to_name(stat));

	/* Set default value. */
	strnfmt(s, sizeof(s), "%d", player->stat_max[stat]);

	if (!get_string(prompt, s, sizeof(s)) ||
			!get_int_from_string(s, &newv)) {
		/* Set next editing stage to break. */
		edit_player_state = EDIT_PLAYER_BREAK;
		return;
	}

	/* Limit to the range of [3, 118]. */
	newv = MIN(118, MAX(3, newv));

	player->stat_cur[stat] = player->stat_max[stat] = newv;

	/* Flag what needs to be updated or redrawn. */
	player->upkeep->update |= (PU_BONUS);
	player->upkeep->redraw |= (PR_STATS);
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
 * Increase the player's experience by a given amount (CMD_WIZ_INCREASE_EXP).
 * Can take the amount from the argument, "quantity", of type number in cmd.
 */
void do_cmd_wiz_increase_exp(struct command *cmd)
{
	int n;

	if (cmd_get_arg_number(cmd, "quantity", &n) != CMD_OK) {
		n = get_quantity("Gain how much experience? ", 9999);
		cmd_set_arg_number(cmd, "quantity", n);
	}

	if (n < 1) n = 1;
	player_exp_gain(player, n);
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


/**
 * Map the area near the player (CMD_WIZ_MAGIC_MAP).  Takes no arguments from
 * cmd.
 */
void do_cmd_wiz_magic_map(struct command *cmd)
{
	effect_simple(EF_MAP_AREA, source_player(), "0", 0, 0, 0, 22, 40, NULL);
}


/**
 * Is a helper function passed by do_cmd_wiz_peek_noise_scent() to
 * wiz_hack_map() in order to peek at the noise.
 *
 * \param c is the chunk to access for data.
 * \param closure is a pointer to an integer with the desired noise level.
 * \param grid is the location in the chunk.
 * \param show is dereferenced and set to true if grid has the desired noise.
 * Otherwise, it is dereferenced and set to false.
 * \param color is dereferenced and set to the color to use if *show is set
 * to true.  Otherwise, it is not dereferenced.
 */
static void wiz_hack_map_peek_noise(struct chunk *c, void *closure,
	struct loc grid, bool *show, byte *color)
{
	if (c->noise.grids[grid.y][grid.x] == *((int*)closure)) {
		*show = true;
		*color = COLOUR_RED;
	} else {
		*show = false;
	}
}


/**
 * Is a helper function passed by do_cmd_wiz_peek_noise_scent() to
 * wiz_hack_map() in order to peek at the scent.
 *
 * \param c is the chunk to access for data.
 * \param closure is a pointer to an integer with the desired scent level.
 * \param grid is the location in the chunk.
 * \param show is dereferenced and set to true if grid has the desired scent.
 * Otherwise, it is dereferenced and set to false.
 * \param color is dereferenced and set to the color to use if *show is set
 * to true.  Otherwise, it is not dereferenced.
 */
static void wiz_hack_map_peek_scent(struct chunk *c, void *closure,
	struct loc grid, bool *show, byte *color)
{
	if (c->scent.grids[grid.y][grid.x] == *((int*)closure)) {
		*show = true;
		*color = COLOUR_YELLOW;
	} else {
		*show = false;
	}
}


/**
 * Display in sequence the squares at n grids from the player, as measured by
 * the noise and scent algorithms; n goes from 1 to the maximum flow depth
 * (CMD_WIZ_PEEK_NOISE_SCENT).  Takes no arguments from cmd.
 */
void do_cmd_wiz_peek_noise_scent(struct command *cmd)
{
	int i;
	char kp;

	/* Noise */
	for (i = 0; i < 100; i++) {
		wiz_hack_map(cave, player, wiz_hack_map_peek_noise, &i);

		/* Get key */
		if (!get_com(format("Depth %d: ", i), &kp)) break;

		/* Redraw map */
		prt_map();
	}

	/* Smell */
	for (i = 0; i < 50; i++) {
		wiz_hack_map(cave, player, wiz_hack_map_peek_scent, &i);

		/* Get key */
		if (!get_com(format("Depth %d: ", i), &kp)) break;

		/* Redraw map */
		prt_map();
	}

	/* Done */
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}


/**
 * Perform an effect (CMD_WIZ_PERFORM_EFFECT).  Takes no arguments from cmd.
 *
 * Bugs:
 * If the command is repeated, it prompts again for all the effect's parameters.
 * The number of parameters currently exceeds CMD_MAX_ARGS so a one-to-one
 * mapping for storing the parameters as arguments in the command would require
 * increasing CMD_MAX_ARGS.  Otherwise, the parameters would have to be
 * multiplexed into the available arguments to store them in the command.  The
 * handling of the lifetime of string arguments for commands is also awkward
 * which would also hamper storing the effect's parameters in the command.
 */
void do_cmd_wiz_perform_effect(struct command *cmd)
{
	char name[80] = "";
	char dice[80] = "0";
	int index = -1;
	int p1 = 0, p2 = 0, p3 = 0;
	int y = 0, x = 0;
	bool ident = false;

	/* Avoid the prompt getting in the way */
	screen_save();

	/* Get the name */
	if (get_string("Do which effect: ", name, sizeof(name))) {
		/* See if an effect index was entered */
		if (!get_int_from_string(name, &index)) {
			/* If not, find the effect with that name */
			index = effect_lookup(name);
		}

		/* Failed */
		if (index <= EF_NONE || index >= EF_MAX) {
			msg("No effect found.");
			return;
		}
	}

	/* Get the dice */
	if (! get_string("Enter damage dice (eg 1+2d6M2): ", dice,
			sizeof(dice))) {
		my_strcpy(dice, "0", sizeof(dice));
	}

	/* Get the effect subtype */
	my_strcpy(name, "0", sizeof(name));
	if (get_string("Enter name or number for effect subtype: ", name,
			sizeof(name))) {
		/* See if an effect parameter was entered */
		p1 = effect_subtype(index, name);
		if (p1 == -1) p1 = 0;
	}

	/* Get the parameters */
	p2 = get_quantity("Enter second parameter (radius): ", 100);
	p3 = get_quantity("Enter third parameter (other): ", 100);
	y = get_quantity("Enter y parameter: ", 100);
	x = get_quantity("Enter x parameter: ", 100);

	/* Reload the screen */
	screen_load();

	effect_simple(index, source_player(), dice, p1, p2, p3, y, x, &ident);

	if (ident) {
		msg("Identified!");
	}
}


/**
 * Play with an item (CMD_WIZ_PLAY_ITEM).  Can take the item to play with
 * from the argument, "item", of type item in cmd.  Uses the arguments,
 * "original_item" (of type item), "all_prop" (of type choice), and "changed"
 * (of type choice), internally for a session of playing with the same object.
 *
 * Things that can be done are:
 * - output statistics via CMD_WIZ_STAT_ITEM
 * - reroll item via CMD_WIZ_REROLL_ITEM
 * - change properties via CMD_WIZ_TWEAK_ITEM
 * - change a curse via CMD_WIZ_CURSE_ITEM
 * - change the number of items via CMD_WIZ_CHANGE_ITEM_QUANTITY
 */
void do_cmd_wiz_play_item(struct command *cmd)
{
	struct object *obj = NULL;
	struct object *orig_obj = NULL;
	int display_all_prop = 1;
	int object_changed = 0;
	bool done = false;
	bool rejected = true;
	char *done_msg = NULL;
	char ch;

	/*
	 * When called to initiate a play session, "item" may be set, but
	 * "original_item" should not be set or should be set to NULL.
	 * Subsequent calls in the same session will have both set to
	 * non-NULL vales along with "all_prop" and "changed".
	 */
	if (cmd_get_arg_item(cmd, "original_item", &orig_obj) == CMD_OK &&
			orig_obj) {
		if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK || !obj) {
			assert(0);
			return;
		}
		if (cmd_get_arg_choice(cmd, "all_prop", &display_all_prop) !=
				CMD_OK) {
			assert(0);
			return;
		}
		if (cmd_get_arg_choice(cmd, "changed", &object_changed) !=
				CMD_OK) {
			assert(0);
			return;
		}
	} else {
		if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK || !obj) {
			if (!get_item(&obj, "Play with which object? ",
					"You have nothing to play with.",
					cmd->code, NULL, (USE_EQUIP |
					USE_INVEN | USE_QUIVER | USE_FLOOR))) {
				return;
			}
		}

		/* Remember the object. */
		cmd_set_arg_item(cmd, "item", obj);

		/* Make a copy so changes can be rejected. */
		orig_obj = object_new();
		object_copy(orig_obj, obj);
		cmd_set_arg_item(cmd, "original_item", orig_obj);

		/*
		 * Store whether all properties are shown or only the known
		 * ones.
		 */
		if (cmd_get_arg_choice(cmd, "all_prop", &display_all_prop) !=
				CMD_OK) {
			display_all_prop = 1;
			cmd_set_arg_choice(cmd, "all_prop", display_all_prop);
		}

		/*
		 * Store whether the working item may have changed from the
		 * original.
		 */
		if (cmd_get_arg_choice(cmd, "changed", &object_changed) !=
				CMD_OK) {
			object_changed = 0;
			cmd_set_arg_choice(cmd, "changed", object_changed);
		}

		/* Save screen. */
		screen_save();
	}

	/* Display the (possibly modified) item. */
	wiz_display_item(obj, display_all_prop != 0);

	/* Get choice. */
	if (get_com("[a]ccept [s]tatistics [r]eroll [t]weak [c]urse [q]uantity [k]nown? ", &ch)) {
		bool queue_failed = false;

		switch (ch) {
			case 'A':
			case 'a':
				/* Accept whatever changes were made. */
				done = true;
				rejected = false;
				if (object_changed) {
					/* Mark for updates. */
					if (object_is_carried(player, obj) &&
							(obj->number !=
							orig_obj->number ||
							obj->weight !=
							orig_obj->weight)) {
						/*
						 * Remove the weight of the old
						 * version.
						 */
						player->upkeep->total_weight -=
							orig_obj->number *
							orig_obj->weight;

						/*
						 * Add the weight of the new
						 * version.
						 */
						player->upkeep->total_weight +=
							obj->number *
							obj->weight;
					}
					wiz_play_item_standard_upkeep(player,
						obj);
				}
				break;

			case 'C':
			case 'c':
				/* Change a curse on the item. */
				if (cmdq_push(CMD_WIZ_CURSE_ITEM) == 0) {
					cmd_set_arg_item(cmdq_peek(), "item",
						obj);
					cmd_set_arg_choice(cmdq_peek(),
						"update", 0);
				} else {
					queue_failed = true;
				}
				break;

			case 'S':
			case 's':
				/* Get statistics about the item. */
				if (cmdq_push(CMD_WIZ_STAT_ITEM) == 0) {
					cmd_set_arg_item(cmdq_peek(), "item",
						obj);
				} else {
					queue_failed = true;
				}
				break;

			case 'R':
			case 'r':
				/* Reroll the item. */
				if (cmdq_push(CMD_WIZ_REROLL_ITEM) == 0) {
					cmd_set_arg_item(cmdq_peek(), "item",
						obj);
					cmd_set_arg_choice(cmdq_peek(),
						"update", 0);
				} else {
					queue_failed = true;
				}
				break;

			case 'T':
			case 't':
				/* Tweak the object's properties. */
				if (cmdq_push(CMD_WIZ_TWEAK_ITEM) == 0) {
					cmd_set_arg_item(cmdq_peek(), "item",
						obj);
					cmd_set_arg_choice(cmdq_peek(),
						"update", 0);
				} else {
					queue_failed = true;
				}
				break;

			case 'K':
			case 'k':
				/* Toggle whether showing all properties. */
				display_all_prop = !display_all_prop;
				cmd_set_arg_choice(cmd, "all_prop",
					display_all_prop);
				break;

			case 'Q':
			case 'q':
				/* Change the number of items in the stack. */
				if (cmdq_push(CMD_WIZ_CHANGE_ITEM_QUANTITY) == 0) {
					cmd_set_arg_item(cmdq_peek(), "item",
						obj);
					cmd_set_arg_choice(cmdq_peek(),
						"update", 0);
				} else {
					queue_failed = true;
				}
				break;

			default:
				/*
				 * Don't have to do anything, next pass through
				 * will ask again what's wanted.
				 */
				break;
		}

		if (queue_failed &&
				get_check("Couldn't proceed.  Stop playing with item and lose all changes? ")) {
			done = true;
			if (object_changed) {
				done_msg = "Bailed out.  Changes to item lost.";
			}
		}
	} else {
		done = true;
		if (object_changed) {
			done_msg = "Changes ignored.";
		}
	}

	if (!done) {
		/* Push the command back on the queue to be reexecuted. */
		if (cmdq_push_copy(cmd) != 0) {
			/* Failed.  Bail out without saving changes. */
			done = true;
			done_msg = "Couldn't queue command.  Changes lost.";
		}
	}

	if (done) {
		if (rejected && object_changed) {
			/*
			 * Restore to the original values.  The pile links
			 * require special handling because object_copy()
			 * resets them.
			 */
			struct object *prev = obj->prev;
			struct object *next = obj->next;

			/* Free slays, brands, and curses by hand. */
			mem_free(obj->slays);
			obj->slays = NULL;
			mem_free(obj->brands);
			obj->brands = NULL;
			mem_free(obj->curses);
			obj->curses = NULL;

			object_copy(obj, orig_obj);
			obj->prev = prev;
			obj->next = next;
		}

		/* Release the preserved copy. */
		object_delete(&orig_obj);

		/*
		 * Reset the original_item and changed arguments so repeating
		 * the command will start a new play session without a
		 * dangling reference to the deleted preserved copy.
		 */
		cmd_set_arg_item(cmd, "original_item", NULL);
		cmd_set_arg_choice(cmd, "changed", 0);

		/* Restore the screen. */
		screen_load();

		/* Provide some feedback. */
		if (done_msg) {
			msg(done_msg);
		}
	}
}


/**
 * Push objects from a selected grid (CMD_WIZ_PUSH_OBJECT).  Can take the
 * location from the argument, "point", of type point in cmd.
 */
void do_cmd_wiz_push_object(struct command *cmd)
{
	struct loc grid;

	if (cmd_get_arg_point(cmd, "point", &grid) != CMD_OK) {
		if (!target_set_interactive(TARGET_KILL, -1, -1)) return;
		target_get(&grid);
		cmd_set_arg_point(cmd, "point", grid);
	}
	push_object(grid);
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

	Term_redraw();

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

	Term_redraw();

	msg("Press any key.");
	inkey_ex();
	prt("", 0, 0);

	/* Redraw map */
	prt_map();
}


/**
 * Quit without saving (CMD_WIZ_QUIT_NO_SAVE).  Takes no arguments from cmd.
 */
void do_cmd_wiz_quit_no_save(struct command *cmd)
{
	quit("user choice");
}


/**
 * Make the player fully aware of a monster race's attributes
 * (CMD_WIZ_RECALL_MONSTER).  Can take the race from the argument, "index", of
 * type number in cmd.  If that index is -1, make the player aware of all
 * races.
 */
void do_cmd_wiz_recall_monster(struct command *cmd)
{
	int r_idx = z_info->r_max;

	if (cmd_get_arg_number(cmd, "index", &r_idx) != CMD_OK) {
		char s[80] = "";
		char c;

		if (!get_com("Full recall for [a]ll monsters or [s]pecific monster? ", &c)) return;
		if (c == 'a' || c == 'A') {
			r_idx = -1;
		} else if (c == 's' || c == 'S') {
			if (!get_string("Which monster? ", s, sizeof(s)))
				return;
			if (!get_int_from_string(s, &r_idx)) {
				const struct monster_race *race =
					lookup_monster(s);

				if (race) {
					r_idx = race->ridx;
				}
			}
		} else {
			return;
		}
		cmd_set_arg_number(cmd, "index", r_idx);
	}

	if (r_idx >= 0 && r_idx < z_info->r_max) {
		const struct monster_race *race = &r_info[r_idx];

		cheat_monster_lore(race, get_lore(race));
	} else if (r_idx == -1) {
		int i;

		for (i = 0; i < z_info->r_max; i++) {
			cheat_monster_lore(&r_info[i], &l_list[i]);
		}
	} else {
		msg("No monster found.");
	}
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
 * Reroll an item (CMD_WIZ_REROLL_ITEM).  Can take the item to use from
 * the argument, "item", of type item in cmd.  Can take the type of roll
 * to use from the argument, "choice", of type choice in cmd:  0 means set
 * good and great to false for the roll, 1 means set good to true and great
 * to false for the roll, and 2 means set good and great to true for the roll.
 */
void do_cmd_wiz_reroll_item(struct command *cmd)
{
	bool good = false;
	bool great = false;
	int roll_choice = 0;
	int update = 0;
	struct object *obj;
	struct object *new;

	/* Get the item to reroll. */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK) {
		if (!get_item(&obj, "Reroll which item? ",
				"You have nothing to reroll.", cmd->code,
				NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER |
				USE_FLOOR))) {
			return;
		}
		cmd_set_arg_item(cmd, "item", obj);
	}

	/* Get the type of roll to use. */
	if (cmd_get_arg_choice(cmd, "choice", &roll_choice) != CMD_OK) {
		char ch;

		if (!get_com("Roll as [n]ormal, [g]ood, or [e]xcellent? ", &ch)) {
			return;
		}
		if (ch == 'n' || ch == 'N') {
			roll_choice = 0;
		} else if (ch == 'g' || ch == 'G') {
			roll_choice = 1;
		} else if (ch == 'e' || ch == 'E') {
			roll_choice = 2;
		} else {
			return;
		}
		cmd_set_arg_choice(cmd, "choice", roll_choice);
	}
	if (roll_choice == 0) {
		good = false;
		great = false;
	} else if (roll_choice == 1) {
		good = true;
		great = false;
	} else if (roll_choice == 2) {
		good = true;
		great = true;
	} else {
		return;
	}

	/* Hack -- leave artifacts alone */
	if (obj->artifact) {
		return;
	}

	/* Get a new object with a clean slate. */
	new = object_new();

	/* Reroll based on old kind and player's depth.  Then apply magic. */
	object_prep(new, obj->kind, player->depth, RANDOMISE);
	apply_magic(new, player->depth, false, good, great, false);

	/* Copy over changes to the original. */
	{
		/* Record old pile information. */
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known_obj = obj->known;
		u16b oidx = obj->oidx;
		struct loc grid = obj->grid;
		bitflag notice = obj->notice;

		/* Free slays, brands, and curses on the old object by hand. */
		mem_free(obj->slays);
		obj->slays = NULL;
		mem_free(obj->brands);
		obj->brands = NULL;
		mem_free(obj->curses);
		obj->curses = NULL;

		/* Copy over; pile information needs to be restored. */
		object_copy(obj, new);
		obj->prev = prev;
		obj->next = next;
		obj->known = known_obj;
		obj->oidx = oidx;
		obj->grid = grid;
		obj->notice = notice;
	}

	/* Mark as cheat */
	obj->origin = ORIGIN_CHEAT;

	/* Flag what needs to be updated. */
	if (cmd_get_arg_choice(cmd, "update", &update) != CMD_OK || update) {
		wiz_play_item_standard_upkeep(player, obj);
	} else {
		wiz_play_item_notify_changed();
	}

	/* Free the copy. */
	object_delete(&new);
}


/**
 * Get some statistics about the rarity of an item (CMD_WIZ_STAT_ITEM).  Can
 * take the item to use from the argument, "item", of type item in cmd.  Can
 * take how items used for comparison are rolled from the argument, "choice",
 * of type choice in cmd:  a value of zero means calling make_object() with
 * good and great set to false, a value of one means calling make_object()
 * with good set to true and great set to false, and a value of two means
 * calling make_object() with good and great set to true.  Can take the
 * depth used when generating items from the argument, "depth", of type
 * number in cmd.
 *
 * Generate a lot of fake items and see if they are of the same type (tval and
 * sval).  Then compare modifiers, AC, to-hit, and to-dam to get whether the
 * fake item matches (all modifiers are the same as are the AC, to-hit, and
 * to-dam), is better (all modifiers, AC, to-hit, and to-dam are not less
 * than on the target item and at least one of those is more than the matching
 * attribute on the target item), is worse (all modifiers, AC, to-hit, and
 * to-dam are not more than on the target item and at least one of those is
 * less than the matching attribute on the target item), or is different.
 * Curses, brands, slays, and other properties not treated as modifiers have
 * no effect on that classification.  A comment left about this procedure in
 * wiz-debug.c was "HINT: This is *very* useful for balancing the game!".
 */
/* This is the maximum number of rolls to use. */
#define TEST_ROLL 100000
void do_cmd_wiz_stat_item(struct command *cmd)
{
	const char *repfmt =
		"Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";
	int level = player->depth;
	int treasure_choice = 0;
	bool good = false, great = false;
	long matches = 0, better = 0, worse = 0, other = 0;
	long n = TEST_ROLL;
	struct object *obj;
	long i;
	const char *quality;

	/* Get the target item for comparison. */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK) {
		if (!get_item(&obj, "Compare with which item? ",
				"You have nothing to compare.", cmd->code,
				NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER |
				USE_FLOOR))) {
			return;
		}
		cmd_set_arg_item(cmd, "item", obj);
	}

	/* Display item. */
	wiz_display_item(obj, true);

	/* Get what kind of treasure to generate. */
	if (cmd_get_arg_choice(cmd, "choice", &treasure_choice) != CMD_OK) {
		char ch;

		if (!get_com("Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ", &ch)) {
			return;
		}
		if (ch == 'n' || ch == 'N') {
			treasure_choice = 0;
		} else if (ch == 'g' || ch == 'G') {
			treasure_choice = 1;
		} else if (ch == 'e' || ch == 'E') {
			treasure_choice = 2;
		} else {
			return;
		}
		cmd_set_arg_choice(cmd, "choice", treasure_choice);
	}
	if (treasure_choice == 0) {
		good = false;
		great = false;
		quality = "normal";
	} else if (treasure_choice == 1) {
		good = true;
		great = false;
		quality = "good";
	} else if (treasure_choice == 2) {
		good = true;
		great = true;
		quality = "excellent";
	} else {
		return;
	}

	/* Get the depth to use when generating treasure. */
	if (cmd_get_arg_number(cmd, "depth", &level) != CMD_OK) {
		char prompt[80], s[80];

		strnfmt(prompt, sizeof(prompt), "Depth for treasure (0-%d): ",
			z_info->max_depth - 1);

		/* Set default. */
		strnfmt(s, sizeof(s), "%d", player->depth);

		if (!get_string(prompt, s, sizeof(s)) ||
				!get_int_from_string(s, &level) || level < 0 ||
				level >= z_info->max_depth) {
			return;
		}
		cmd_set_arg_number(cmd, "depth", level);
	}
	if (level < 0 || level >= z_info->max_depth) {
		return;
	}

	/* Give feedback about what's going to be done. */
	msg("Creating a lot of %s items.  Base level = %d.", quality, level);
	event_signal(EVENT_MESSAGE_FLUSH);

	for (i = 0; i < n; i++) {
		bool ismatch = true, isbetter = true, isworse = true;
		struct object *test_obj;
		int j;

		/* Output every few rolls. */
		if (i < 100 || i % 100 == 0) {
			ui_event e;

			/* Do not wait. */
			inkey_scan = SCAN_INSTANT;

			/* Allow interrupt */
			e = inkey_ex();
			if (e.type != EVT_NONE) {
				event_signal(EVENT_INPUT_FLUSH);
				break;
			}

			/* Dump the stats. */
			prt(format(repfmt, i, matches, better, worse, other),
				0, 0);
			Term_fresh();
		}

		/* Create an object. */
		test_obj = make_object(cave, level, good, great, false, NULL, 0);

		/*
		 * Allow multiple artifacts, because breaking the game is OK
		 * here.
		 */
		if (obj->artifact) {
			obj->artifact->created = false;
		}

		/* Check for failures to generate an object. */
		if (!test_obj) continue;

		/* Test for same tval and sval. */
		if (obj->tval != test_obj->tval ||
				obj->sval != test_obj->sval) {
			object_delete(&test_obj);
			continue;
		}

		/* Check the modifiers. */
		for (j = 0; j < OBJ_MOD_MAX; j++) {
			if (test_obj->modifiers[j] != obj->modifiers[j]) {
				ismatch = false;
				if (test_obj->modifiers[j] < obj->modifiers[j]) {
					isbetter = false;
				} else {
					isworse = false;
				}
			}
		}

		/* Check for match over all the tested properties. */
		if (ismatch && test_obj->to_a == obj->to_a &&
				test_obj->to_h == obj->to_h &&
				test_obj->to_d == obj->to_d) {
			++matches;
		/* Check for same or better over all the tested properties. */
		} else if (isbetter && test_obj->to_a >= obj->to_a &&
				test_obj->to_h >= obj->to_h &&
				test_obj->to_d >= obj->to_d) {
			++better;
		/* Check for same or worse over all the tested properties. */
		} else if (isworse && test_obj->to_a <= obj->to_a &&
				test_obj->to_h <= obj->to_h &&
				test_obj->to_d <= obj->to_d) {
			++worse;
		/* Everything else is merely different. */
		} else {
			++other;
		}

		/* Nuke the test object. */
		object_delete(&test_obj);
	}

	/* Final dump */
	msg(repfmt, i, matches, better, worse, other);
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Hack -- normally only make a single artifact */
	if (obj->artifact) {
		obj->artifact->created = true;
	}
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
 * Summon random monsters near the player (CMD_WIZ_SUMMON_RANDOM).  Can take
 * the number to summon from the argument, "quantity", of type number in cmd.
 */
void do_cmd_wiz_summon_random(struct command *cmd)
{
	int n, i;

	if (cmd_get_arg_number(cmd, "quantity", &n) != CMD_OK) {
		n = get_quantity("How many monsters? ", 40);
		if (n < 1) n = 1;
		cmd_set_arg_number(cmd, "quantity", n);
	}

	for (i = 0; i < n; i++) {
		effect_simple(EF_SUMMON, source_player(), "1",
			0, 0, 0, 0, 0, NULL);
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


/**
 * Tweak an item:  make it ego or artifact, give values for modifiers, to_a,
 * to_h, or to_d.  Can take the item to modify from the argument, "item", of
 * type item in cmd.  Takes whether to update player information from the
 * argument, "update", of type choice in cmd.
 */
void do_cmd_wiz_tweak_item(struct command *cmd)
{
	static const char *obj_mods[] = {
		#define STAT(a) #a,
		#include "list-stats.h"
		#undef STAT
		#define OBJ_MOD(a) #a,
		#include "list-object-modifiers.h"
		#undef OBJ_MOD
		NULL
	};
	struct object *obj;
	char tmp_val[80];
	int i, val;
	int update = 0;

	/* Get the item to tweak. */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK) {
		if (!get_item(&obj, "Tweak which item? ",
				"You have nothing to tweak.", cmd->code,
				NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER |
				USE_FLOOR))) {
			return;
		}
		cmd_set_arg_item(cmd, "item", obj);
	}

	/* Hack -- leave artifacts alone */
	if (obj->artifact) return;

	/*
	 * Check for whether updating the player information or let
	 * do_cmd_wiz_play_item() handle it.
	 */
	if (cmd_get_arg_choice(cmd, "update", &update) != CMD_OK) {
		update = 1;
	}

	/* Get ego name. */
	if (obj->ego) {
		strnfmt(tmp_val, sizeof(tmp_val), "%s", obj->ego->name);
	} else {
		strnfmt(tmp_val, sizeof(tmp_val), "-1");
	}
	if (!get_string("Enter ego item: ", tmp_val, sizeof(tmp_val))) return;

	/* Accept index or name */
	if (get_int_from_string(tmp_val, &val)) {
		if (val >= 0 && val < z_info->e_max) {
			obj->ego = &e_info[val];
		} else {
			obj->ego = NULL;
		}
	} else {
		obj->ego = lookup_ego_item(tmp_val, obj->tval, obj->sval);
	}
	if (obj->ego) {
		struct ego_item *e = obj->ego;
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known = obj->known;
		u16b oidx = obj->oidx;
		struct loc grid = obj->grid;
		bitflag notice = obj->notice;

		object_prep(obj, obj->kind, player->depth, RANDOMISE);
		obj->ego = e;
		obj->prev = prev;
		obj->next = next;
		obj->known = known;
		obj->oidx = oidx;
		obj->grid = grid;
		obj->notice = notice;
		ego_apply_magic(obj, player->depth);
	}
	wiz_display_item(obj, true);

	/* Get artifact name */
	if (obj->artifact) {
		strnfmt(tmp_val, sizeof(tmp_val), "%s", obj->artifact->name);
	} else {
		strnfmt(tmp_val, sizeof(tmp_val), "0");
	}
	if (!get_string("Enter new artifact: ", tmp_val, sizeof(tmp_val))) {
		if (update) {
			wiz_play_item_standard_upkeep(player, obj);
		} else {
			wiz_play_item_notify_changed();
		}
		return;
	}

	/* Accept index or name */
	if (get_int_from_string(tmp_val, &val)) {
		if (val > 0 && val < z_info->a_max) {
			obj->artifact = &a_info[val];
		} else {
			obj->artifact = NULL;
		}
	} else {
		obj->artifact = lookup_artifact_name(tmp_val);
	}
	if (obj->artifact) {
		struct artifact *a = obj->artifact;
		struct object *prev = obj->prev;
		struct object *next = obj->next;
		struct object *known = obj->known;
		u16b oidx = obj->oidx;
		struct loc grid = obj->grid;
		bitflag notice = obj->notice;

		obj->ego = NULL;
		object_prep(obj, obj->kind, obj->artifact->alloc_min, RANDOMISE);
		obj->artifact = a;
		obj->prev = prev;
		obj->next = next;
		obj->known = known;
		obj->oidx = oidx;
		obj->grid = grid;
		obj->notice = notice;
		copy_artifact_data(obj, obj->artifact);
	}
	wiz_display_item(obj, true);

#define WIZ_TWEAK(attribute, name) do {\
		char prompt[80];\
		strnfmt(prompt, sizeof(prompt), "Enter new %s setting: ", name);\
		strnfmt(tmp_val, sizeof(tmp_val), "%d", obj->attribute);\
		if (!get_string(prompt, tmp_val, sizeof(tmp_val))) {\
			if (update) {\
				wiz_play_item_standard_upkeep(player, obj);\
			} else {\
				wiz_play_item_notify_changed();\
			}\
			return;\
		}\
		if (get_int_from_string(tmp_val, &val)) {\
			obj->attribute = val;\
			wiz_display_item(obj, true);\
		}\
} while (0)
	for (i = 0; i < OBJ_MOD_MAX; i++) {
		WIZ_TWEAK(modifiers[i], obj_mods[i]);
	}
	WIZ_TWEAK(to_a, "AC bonus");
	WIZ_TWEAK(to_h, "to-hit");
	WIZ_TWEAK(to_d, "to-dam");

	if (update) {
		wiz_play_item_standard_upkeep(player, obj);
	} else {
		wiz_play_item_notify_changed();
	}
}


/**
 * Make the player ignorant of a monster race's attributes
 * (CMD_WIZ_WIPE_RECALL).  Can take the race from the argument, "index", of
 * type number in cmd.  If that index is -1, make the player ignorant of all
 * races.
 */
void do_cmd_wiz_wipe_recall(struct command *cmd)
{
	int r_idx = z_info->r_max;

	if (cmd_get_arg_number(cmd, "index", &r_idx) != CMD_OK) {
		char s[80] = "";
		char c;

		if (!get_com("Wipe recall for [a]ll monsters or [s]pecific monster? ", &c)) return;
		if (c == 'a' || c == 'A') {
			r_idx = -1;
		} else if (c == 's' || c == 'S') {
			if (!get_string("Which monster? ", s, sizeof(s)))
				return;
			if (!get_int_from_string(s, &r_idx)) {
				const struct monster_race *race =
					lookup_monster(s);

				if (race) {
					r_idx = race->ridx;
				}
			}
		} else {
			return;
		}
		cmd_set_arg_number(cmd, "index", r_idx);
	}

	if (r_idx >= 0 && r_idx < z_info->r_max) {
		const struct monster_race *race = &r_info[r_idx];

		wipe_monster_lore(race, get_lore(race));
	} else if (r_idx == -1) {
		int i;

		for (i = 0; i < z_info->r_max; i++) {
			wipe_monster_lore(&r_info[i], &l_list[i]);
		}
	} else {
		msg("No monster found.");
	}
}


/**
 * Wizard light the level (CMD_WIZ_WIZARD_LIGHT).  Takes no arguments from cmd.
 */
void do_cmd_wiz_wizard_light(struct command *cmd)
{
	wiz_light(cave, player, true);
}
