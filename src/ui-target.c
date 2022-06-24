/**
 * \file ui-target.c
 * \brief UI for targetting code
 *
 * Copyright (c) 1997-2014 Angband contributors
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
#include "cave.h"
#include "game-input.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-predicate.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "project.h"
#include "target.h"
#include "trap.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-map.h"
#include "ui-mon-lore.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-target.h"
#include "ui-term.h"

/*
 * Holds state passed between target_set_interactive_aux() and the handlers
 * that help it handle different types of grids or situations.  In general,
 * the handlers should only modify press (passed back from
 * target_set_interactive_aux() to target_set_interactive()) and boring
 * (modulates how later handlers act).
 */
struct target_aux_state {
	char coord_desc[20];
	const char *phrase1;
	const char *phrase2;
	struct loc grid;
	ui_event press;
	int mode;
	bool boring;
};

typedef bool (*target_aux_handler)(struct chunk *c, struct player *p,
	struct target_aux_state *auxst);

/**
 * Extract a direction (or zero) from a character
 */
int target_dir(struct keypress ch)
{
	return target_dir_allow(ch, false);
}

int target_dir_allow(struct keypress ch, bool allow_5)
{
	int d = 0;

	/* Already a direction? */
	if (isdigit((unsigned char)ch.code)) {
		d = D2I(ch.code);
	} else if (isarrow(ch.code)) {
		switch (ch.code) {
			case ARROW_DOWN:  d = 2; break;
			case ARROW_LEFT:  d = 4; break;
			case ARROW_RIGHT: d = 6; break;
			case ARROW_UP:    d = 8; break;
		}
	} else {
		int mode;
		const struct keypress *act;

		if (OPT(player, rogue_like_commands))
			mode = KEYMAP_MODE_ROGUE;
		else
			mode = KEYMAP_MODE_ORIG;

		/* XXX see if this key has a digit in the keymap we can use */
		act = keymap_find(mode, ch);
		if (act) {
			const struct keypress *cur;
			for (cur = act; cur->type == EVT_KBRD; cur++) {
				if (isdigit((unsigned char) cur->code))
					d = D2I(cur->code);
			}
		}
	}

	/* Paranoia */
	if (d == 5 && !allow_5) d = 0;

	/* Return direction */
	return (d);
}

/**
 * Display targeting help at the bottom of the screen.
 */
void target_display_help(bool monster, bool object, bool free)
{
	/* Determine help location */
	int wid, hgt, help_loc;
	Term_get_size(&wid, &hgt);
	help_loc = hgt - HELP_HEIGHT;
	
	/* Clear */
	clear_from(help_loc);

	/* Prepare help hooks */
	text_out_hook = text_out_to_screen;
	text_out_indent = 1;
	Term_gotoxy(1, help_loc);

	/* Display help */
	text_out_c(COLOUR_L_GREEN, "<dir>");
	text_out(" and ");
	text_out_c(COLOUR_L_GREEN, "<click>");
	text_out(" look around. '");
	text_out_c(COLOUR_L_GREEN, "g");
	text_out("' moves to selection. '");
	text_out_c(COLOUR_L_GREEN, "p");
	text_out("' selects player. '");
	text_out_c(COLOUR_L_GREEN, "q");
	text_out("' exits. '");
	text_out_c(COLOUR_L_GREEN, "r");
	text_out("' displays details. '");

	if (free) {
		text_out_c(COLOUR_L_GREEN, "m");
		text_out("' restricts to interesting places.");
	} else {
		text_out_c(COLOUR_L_GREEN, "+");
		text_out("' and '");
		text_out_c(COLOUR_L_GREEN, "-");
		text_out("' cycle through places. '");
		text_out_c(COLOUR_L_GREEN, "o");
		text_out("' allows free selection.");
	}
	
	if (monster || free) {
		text_out(" '");
		text_out_c(COLOUR_L_GREEN, "t");
		text_out("' targets selection.");
	}

	if (object) {
		unsigned char key = cmd_lookup_key(CMD_IGNORE,
			(OPT(player, rogue_like_commands)) ?
			KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG);
		char label[3];

		if (KTRL(key) == key) {
			label[0] = '^';
			label[1] = UN_KTRL(key);
			label[2] = '\0';
		} else {
			label[0] = key;
			label[1] = '\0';
		}
		text_out(" '");
		text_out_c(COLOUR_L_GREEN, "%s", label);
		text_out("' ignores selection.");
	}

	/* Reset */
	text_out_indent = 0;
}


/**
 * Return whether a key triggers a running action.
 */
static bool is_running_keymap(struct keypress ch)
{
	int mode = (OPT(player, rogue_like_commands)) ?
		KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	const struct keypress *act = keymap_find(mode, ch);

	if (act) {
		unsigned char run_key = cmd_lookup_key(CMD_RUN, mode);
		const struct keypress *cur;

		for (cur = act; cur->type == EVT_KBRD; cur++) {
			if ((unsigned char)cur->code == run_key) {
				return true;
			}
		}
	}
	return false;
}


/**
 * Perform the minimum "whole panel" adjustment to ensure that the given
 * location is contained inside the current panel, and return true if any
 * such adjustment was performed. Optionally accounts for the targeting
 * help window.
 */
static bool adjust_panel_help(int y, int x, bool help)
{
	bool changed = false;

	int j;

	int screen_hgt_main = help ? (Term->hgt - ROW_MAP - ROW_BOTTOM_MAP - 2)
			 : (Term->hgt - ROW_MAP - ROW_BOTTOM_MAP);

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		int wx, wy;
		int screen_hgt, screen_wid;

		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(window_flag[j] & PW_OVERHEAD)) continue;

		wy = t->offset_y;
		wx = t->offset_x;

		screen_hgt = (j == 0) ? screen_hgt_main : t->hgt;
		screen_wid = (j == 0) ? (Term->wid - COL_MAP - 1) : t->wid;

		/* Bigtile panels need adjustment */
		screen_wid = screen_wid / tile_width;
		screen_hgt = screen_hgt / tile_height;

		/* Adjust as needed */
		while (y >= wy + screen_hgt) wy += screen_hgt / 2;
		while (y < wy) wy -= screen_hgt / 2;

		/* Adjust as needed */
		while (x >= wx + screen_wid) wx += screen_wid / 2;
		while (x < wx) wx -= screen_wid / 2;

		/* Use "modify_panel" */
		if (modify_panel(t, wy, wx)) changed = true;
	}

	return (changed);
}


/**
 * Display the object name of the selected object and allow for full object
 * recall. Returns an event that occurred display.
 *
 * This will only work for a single object on the ground and not a pile. This
 * loop is similar to the monster recall loop in target_set_interactive_aux().
 * The out_val array size needs to match the size that is passed in (since
 * this code was extracted from there).
 *
 * \param obj is the object to describe.
 * \param y is the cave row of the object.
 * \param x is the cave column of the object.
 * \param out_val is the string that holds the name of the object and is
 * returned to the caller.
 * \param s1 is part of the output string.
 * \param s2 is part of the output string.
 * \param s3 is part of the output string.
 * \param coords is part of the output string
 */
static ui_event target_recall_loop_object(struct object *obj, int y, int x,
		char out_val[TARGET_OUT_VAL_SIZE],
		const char *s1,
		const char *s2,
		const char *s3,
		const char *coords,
		const struct player *p)
{
	bool recall = false;
	ui_event press;

	while (1) {
		if (recall) {
			display_object_recall_interactive(cave->objects[obj->oidx]);
			press = inkey_m();
		} else {
			char o_name[80];

			/* Obtain an object description */
			object_desc(o_name, sizeof(o_name),
				cave->objects[obj->oidx],
				ODESC_PREFIX | ODESC_FULL, p);

			/* Describe the object */
			if (player->wizard) {
				strnfmt(out_val, TARGET_OUT_VAL_SIZE,
						"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).", s1, s2, s3,
						o_name, coords, y, x, (int)cave->noise.grids[y][x],
						(int)cave->scent.grids[y][x]);
			} else {
				strnfmt(out_val, TARGET_OUT_VAL_SIZE,
						"%s%s%s%s, %s.", s1, s2, s3, o_name, coords);
			}

			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			press = inkey_m();
		}

		if ((press.type == EVT_MOUSE) && (press.mouse.button == 1) &&
			(KEY_GRID_X(press) == x) && (KEY_GRID_Y(press) == y))
			recall = !recall;
		else if ((press.type == EVT_KBRD) && (press.key.code == 'r'))
			recall = !recall;
		else
			break;
	}

	return press;
}

/**
 * Help target_set_interactive_aux():  reset the state for another pass
 * through the handlers.
 */
static bool aux_reinit(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	struct monster *mon;

	/* Set the default event to focus on the player. */
	auxst->press.type = EVT_KBRD;
	auxst->press.key.code = 'p';
	auxst->press.key.mods = 0;

	/* Bail if looking at a forbidden grid.  Don't run any more handlers. */
	if (!square_in_bounds(c, auxst->grid)) return true;

	/* Assume boring. */
	auxst->boring = true;

	if (square(c, auxst->grid)->mon < 0) {
		/* Looking at the player's grid */
		auxst->phrase1 = "You are ";
		auxst->phrase2 = "on ";
	} else {
		/* Default */
		if (square_isseen(c, auxst->grid)) {
			auxst->phrase1 = "You see ";
		} else {
			mon = square_monster(c, auxst->grid);
			if (mon && monster_is_obvious(mon)) {
				/* Monster is visible because of detection or telepathy */
				auxst->phrase1 = "You sense ";
			} else {
				auxst->phrase1 = "You recall ";
			}
		}
		auxst->phrase2 = "";
	}

	return false;
}

/**
 * Help target_set_interactive_aux():  handle hallucination.
 */
static bool aux_hallucinate(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	const char *name_strange = "something strange";
	char out_val[TARGET_OUT_VAL_SIZE];

	if (!p->timed[TMD_IMAGE]) return false;

	/* Hallucination messes things up */
	/* Display a message */
	if (p->wizard) {
		strnfmt(out_val, sizeof(out_val),
			"%s%s%s, %s (%d:%d, noise=%d, scent=%d).",
			auxst->phrase1,
			auxst->phrase2,
			name_strange,
			auxst->coord_desc,
			auxst->grid.y,
			auxst->grid.x,
			(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
			(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);
	} else {
		strnfmt(out_val, sizeof(out_val), "%s%s%s, %s.",
			auxst->phrase1,
			auxst->phrase2,
			name_strange,
			auxst->coord_desc);
	}
	prt(out_val, 0, 0);
	move_cursor_relative(auxst->grid.y, auxst->grid.x);

	auxst->press.key = inkey();

	/* Stop on everything but "return" */
	return auxst->press.key.code != KC_ENTER;
}

/**
 * Help target_set_interactive_aux():  handle monsters.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 */
static bool aux_monster(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	struct monster *mon;
	const struct monster_lore *lore;
	char m_name[80];
	char out_val[TARGET_OUT_VAL_SIZE];
	bool recall;

	if (square(c, auxst->grid)->mon <= 0) return false;

	mon = square_monster(c, auxst->grid);
	if (!monster_is_obvious(mon)) return false;

	/* Actual visible monsters */
	lore = get_lore(mon->race);

	/* Not boring */
	auxst->boring = false;

	/* Get the monster name ("a kobold") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_VIS);

	/* Track this monster's race and health */
	monster_race_track(p->upkeep, mon->race);
	health_track(p->upkeep, mon);
	handle_stuff(p);

	/* Interact */
	recall = false;
	while (1) {
		/* Recall or target */
		if (recall) {
			lore_show_interactive(mon->race, lore);
			auxst->press = inkey_m();
		} else {
			char buf[80];

			/* Describe the monster */
			look_mon_desc(buf, sizeof(buf),
				square(c, auxst->grid)->mon);

			/* Describe, and prompt for recall */
			if (p->wizard) {
				strnfmt(out_val, sizeof(out_val),
					"%s%s%s (%s), %s (%d:%d, noise=%d, scent=%d).",
					auxst->phrase1,
					auxst->phrase2,
					m_name,
					buf,
					auxst->coord_desc,
					auxst->grid.y,
					auxst->grid.x,
					(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
					(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);
			} else {
				strnfmt(out_val, sizeof(out_val),
					"%s%s%s (%s), %s.",
					auxst->phrase1,
					auxst->phrase2,
					m_name,
					buf,
					auxst->coord_desc);
			}

			prt(out_val, 0, 0);

			/* Place cursor */
			move_cursor_relative(auxst->grid.y, auxst->grid.x);

			/* Command */
			auxst->press = inkey_m();
		}

		/* Normal commands */
		if (auxst->press.type == EVT_MOUSE
				&& auxst->press.mouse.button == 1
				&& KEY_GRID_X(auxst->press) == auxst->grid.x
				&& KEY_GRID_Y(auxst->press) == auxst->grid.y) {
			recall = !recall;
		} else if (auxst->press.type == EVT_KBRD
				&& auxst->press.key.code == 'r') {
			recall = !recall;
		} else {
			break;
		}
	}

	if (auxst->press.type == EVT_MOUSE) {
		/* Stop on right click */
		if (auxst->press.mouse.button == 2) return true;

		/* Sometimes stop at "space" key */
		if (auxst->press.mouse.button
				&& !(auxst->mode & (TARGET_LOOK))) return true;
	} else {
		/* Stop on everything but "return"/"space" */
		if (auxst->press.key.code != KC_ENTER
				&& auxst->press.key.code != ' ') return true;

		/* Sometimes stop at "space" key */
		if (auxst->press.key.code == ' '
				&& !(auxst->mode & (TARGET_LOOK))) return true;
	}

	/* Describe carried objects (wizards only) */
	if (p->wizard) {
		const char *lphrase1;
		const char *lphrase2;
		struct object *obj;

		/* Take account of gender */
		if (rf_has(mon->race->flags, RF_FEMALE)) {
			lphrase1 = "She is ";
		} else if (rf_has(mon->race->flags, RF_MALE)) {
			lphrase1 = "He is ";
		} else {
			lphrase1 = "It is ";
		}

		/* Use a verb */
		lphrase2 = "carrying ";

		/* Scan all objects being carried */
		for (obj = mon->held_obj; obj; obj = obj->next) {
			char o_name[80];

			/* Obtain an object description */
			object_desc(o_name, sizeof(o_name), obj,
				ODESC_PREFIX | ODESC_FULL, p);

			strnfmt(out_val, sizeof(out_val),
				"%s%s%s, %s (%d:%d, noise=%d, scent=%d).",
				lphrase1,
				lphrase2,
				o_name,
				auxst->coord_desc,
				auxst->grid.y,
				auxst->grid.x,
				(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
				(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);

			prt(out_val, 0, 0);
			move_cursor_relative(auxst->grid.y, auxst->grid.x);
			auxst->press = inkey_m();

			if (auxst->press.type == EVT_MOUSE) {
				/* Stop on right click */
				if (auxst->press.mouse.button == 2) break;

				/* Sometimes stop at "space" key */
				if (auxst->press.mouse.button
						&& !(auxst->mode & (TARGET_LOOK)))
					break;
			} else {
				/* Stop on everything but "return"/"space" */
				if (auxst->press.key.code != KC_ENTER
						&& auxst->press.key.code != ' ')
					break;

				/* Sometimes stop at "space" key */
				if (auxst->press.key.code == ' '
						&& !(auxst->mode & (TARGET_LOOK)))
					break;
			}

			/* Change the intro */
			lphrase2 = "also carrying ";
		}

		/* Double break */
		if (obj) return true;
	}

	return false;
}

/**
 * Help target_set_interactive_aux():  handle visible traps.
 */
static bool aux_trap(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	struct trap *trap;
	char out_val[TARGET_OUT_VAL_SIZE];
	const char *lphrase3;

	if (!square_isvisibletrap(p->cave, auxst->grid)) return false;

	/* A trap */
	trap = square(p->cave, auxst->grid)->trap;

	/* Not boring */
	auxst->boring = false;

	/* Pick proper indefinite article */
	lphrase3 = (is_a_vowel(trap->kind->desc[0])) ? "an " : "a ";

	/* Interact */
	while (1) {
		/* Describe, and prompt for recall */
		if (p->wizard) {
			strnfmt(out_val, sizeof(out_val),
				"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).",
				auxst->phrase1,
				auxst->phrase2,
				lphrase3,
				trap->kind->name,
				auxst->coord_desc,
				auxst->grid.y,
				auxst->grid.x,
				(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
				(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);
		} else {
			strnfmt(out_val, sizeof(out_val), "%s%s%s%s, %s.",
				auxst->phrase1,
				auxst->phrase2,
				lphrase3,
				trap->kind->desc,
				auxst->coord_desc);
		}

		prt(out_val, 0, 0);

		/* Place cursor */
		move_cursor_relative(auxst->grid.y, auxst->grid.x);

		/* Command */
		auxst->press = inkey_m();

		/* Stop on everything but "return"/"space" */
		if (auxst->press.key.code != KC_ENTER
				&& auxst->press.key.code != ' ')
			break;

		/* Sometimes stop at "space" key */
		if (auxst->press.key.code == ' '
				&& !(auxst->mode & (TARGET_LOOK)))
			break;
	}

	return true;
}

/**
 * Help target_set_interactive_aux():  handle objects.
 */
static bool aux_object(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	int floor_max = z_info->floor_size;
	struct object **floor_list =
		mem_zalloc(floor_max * sizeof(*floor_list));
	bool result = false;
	char out_val[TARGET_OUT_VAL_SIZE];
	int floor_num;

	/* Scan all sensed objects in the grid */
	floor_num = scan_distant_floor(floor_list, floor_max, p, auxst->grid);
	if (floor_num <= 0) {
		mem_free(floor_list);
		return result;
	}

	/* Not boring */
	auxst->boring = false;

	track_object(p->upkeep, floor_list[0]);
	handle_stuff(p);

	/* If there is more than one item... */
	if (floor_num > 1) {
		while (1) {
			/* Describe the pile */
			if (p->wizard) {
				strnfmt(out_val, sizeof(out_val),
					"%s%sa pile of %d objects, %s (%d:%d, noise=%d, scent=%d).",
					auxst->phrase1,
					auxst->phrase2,
					floor_num,
					auxst->coord_desc,
					auxst->grid.y,
					auxst->grid.x,
					(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
					(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);
			} else {
				strnfmt(out_val, sizeof(out_val),
					"%s%sa pile of %d objects, %s.",
					auxst->phrase1,
					auxst->phrase2,
					floor_num,
					auxst->coord_desc);
			}

			prt(out_val, 0, 0);
			move_cursor_relative(auxst->grid.y, auxst->grid.x);
			auxst->press = inkey_m();

			/* Display objects */
			if ((auxst->press.type == EVT_MOUSE
					&& auxst->press.mouse.button == 1
					&& KEY_GRID_X(auxst->press) ==
					auxst->grid.x
					&& KEY_GRID_Y(auxst->press) ==
					auxst->grid.y)
					|| (auxst->press.type == EVT_KBRD
					&& auxst->press.key.code == 'r')) {
				int pos;
				while (1) {
					/* Save screen */
					screen_save();

					/*
					 * Use OLIST_DEATH to show item labels
					 */
					show_floor(floor_list, floor_num,
						(OLIST_DEATH | OLIST_WEIGHT
						| OLIST_GOLD), NULL);

					/* Describe the pile */
					prt(out_val, 0, 0);
					auxst->press = inkey_m();

					/* Load screen */
					screen_load();

					if (auxst->press.type == EVT_MOUSE) {
						pos = auxst->press.mouse.y - 1;
					} else {
						pos = auxst->press.key.code -
							'a';
					}
					if (0 <= pos && pos < floor_num) {
						track_object(p->upkeep,
							floor_list[pos]);
						handle_stuff(p);
						continue;
					}
					break;
				}

				/*
				 * Now that the user's done with the display
				 * loop, let's do the outer loop over again.
				 */
				continue;
			}

			/* Done */
			break;
		}
	} else {
		/* Only one object to display */
		/* Get the single object in the list */
		struct object *obj_local = floor_list[0];

		/* Allow user to recall an object */
		auxst->press = target_recall_loop_object(obj_local,
			auxst->grid.y, auxst->grid.x, out_val, auxst->phrase1,
			auxst->phrase2, "", auxst->coord_desc, p);

		/* Stop on everything but "return"/"space" */
		if (auxst->press.key.code != KC_ENTER
				&& auxst->press.key.code != ' ') result = true;

		/* Sometimes stop at "space" key */
		if (auxst->press.key.code == ' '
				&& !(auxst->mode & (TARGET_LOOK))) result = true;
	}

	mem_free(floor_list);
	return result;
}

/**
 * Help target_set_interactive_aux():  handle terrain.
 */
static bool aux_terrain(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	const char *name, *lphrase2, *lphrase3;
	char out_val[TARGET_OUT_VAL_SIZE];

	if (!auxst->boring && !square_isinteresting(c, auxst->grid))
		return false;

	/* Terrain feature if needed */
	name = square_apparent_name(p->cave, auxst->grid);

	/* Hack -- handle unknown grids */

	/* Pick a preposition if needed */
	lphrase2 = (*auxst->phrase2) ?
		square_apparent_look_in_preposition(p->cave, auxst->grid) : "";

	/* Pick prefix for the name */
	lphrase3 = square_apparent_look_prefix(p->cave, auxst->grid);

	/* Display a message */
	if (p->wizard) {
		strnfmt(out_val, sizeof(out_val),
			"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).",
			auxst->phrase1,
			lphrase2,
			lphrase3,
			name,
			auxst->coord_desc,
			auxst->grid.y,
			auxst->grid.x,
			(int)c->noise.grids[auxst->grid.y][auxst->grid.x],
			(int)c->scent.grids[auxst->grid.y][auxst->grid.x]);
	} else {
		strnfmt(out_val, sizeof(out_val),
			"%s%s%s%s, %s.",
			auxst->phrase1,
			lphrase2,
			lphrase3,
			name,
			auxst->coord_desc);
	}

	prt(out_val, 0, 0);
	move_cursor_relative(auxst->grid.y, auxst->grid.x);
	auxst->press = inkey_m();

	/*
	 * Stop on right click of mouse or everything but "return"/"space" for
	 * a key.
	 */
	return (auxst->press.type == EVT_MOUSE
			&& auxst->press.mouse.button == 2)
		|| (auxst->press.type != EVT_MOUSE
			&& auxst->press.key.code != KC_ENTER
			&& auxst->press.key.code != ' ');
}

/**
 * Help target_set_interactive_aux():  check what's in press to decide whether
 * to do another pass through the handlers.
 */
static bool aux_wrapup(struct chunk *c, struct player *p,
		struct target_aux_state *auxst)
{
	if (auxst->press.type == EVT_MOUSE) {
		/* Stop on right click. */
		return auxst->press.mouse.button != 2;
	}
	/* Stop on everything but "return". */
	return auxst->press.key.code != KC_ENTER;
}

/**
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * This function correctly handles multiple objects per grid, and objects
 * and terrain features in the same grid, though the latter never happens.
 *
 * This function must handle blindness/hallucination.
 */
static ui_event target_set_interactive_aux(int y, int x, int mode)
{
	/*
	 * If there's other types to be handled, insert a function to do so
	 * between aux_hallucinate and aux_wrapup.  Because each handler
	 * can signal that the sequence be halted, these are ordered in
	 * decreasing order of precedence.
	 */
	target_aux_handler handlers[] = {
		aux_reinit,
		aux_hallucinate,
		aux_monster,
		aux_trap,
		aux_object,
		aux_terrain,
		aux_wrapup
	};
	struct target_aux_state auxst;
	int ihandler;

	auxst.mode = mode;

	/* Describe the square location */
	auxst.grid.x = x;
	auxst.grid.y = y;
	coords_desc(auxst.coord_desc, sizeof(auxst.coord_desc), y, x);

	/* Apply the handlers in order until done */
	ihandler = 0;
	while (1) {
		if ((*handlers[ihandler])(cave, player, &auxst)) break;
		++ihandler;
		if (ihandler >= (int) N_ELEMENTS(handlers)) ihandler = 0;
	}

	/* Keep going */
	return auxst.press;
}

/**
 * Target command
 */
void textui_target(void)
{
	if (target_set_interactive(TARGET_KILL, -1, -1))
		msg("Target Selected.");
	else
		msg("Target Aborted.");
}

/**
 * Target closest monster.
 *
 * XXX: Move to using CMD_TARGET_CLOSEST at some point instead of invoking
 * target_set_closest() directly.
 */
void textui_target_closest(void)
{
	if (target_set_closest(TARGET_KILL, NULL)) {
		bool visibility;
		struct loc target;

		target_get(&target);

		/* Visual cue */
		Term_fresh();
		Term_get_cursor(&visibility);
		(void)Term_set_cursor(true);
		move_cursor_relative(target.y, target.x);
		Term_redraw_section(target.y, target.x, target.y, target.x);

		/* TODO: what's an appropriate amount of time to spend highlighting */
		Term_xtra(TERM_XTRA_DELAY, 150);
		(void)Term_set_cursor(visibility);
	}
}


/**
 * Draw a visible path over the squares between (x1,y1) and (x2,y2).
 *
 * The path consists of "*", which are white except where there is a
 * monster, object or feature in the grid.
 *
 * This routine has (at least) three weaknesses:
 * - remembered objects/walls which are no longer present are not shown,
 * - squares which (e.g.) the player has walked through in the dark are
 *   treated as unknown space.
 * - walls which appear strange due to hallucination aren't treated correctly.
 *
 * The first two result from information being lost from the dungeon arrays,
 * which requires changes elsewhere
 */
static int draw_path(uint16_t path_n, struct loc *path_g, wchar_t *c, int *a,
					 int y1, int x1)
{
	int i;
	bool on_screen;
	bool pastknown = false;

	/* No path, so do nothing. */
	if (path_n < 1) return 0;

	/* The starting square is never drawn, but notice if it is being
     * displayed. In theory, it could be the last such square.
     */
	on_screen = panel_contains(y1, x1);

	/* Draw the path. */
	for (i = 0; i < path_n; i++) {
		uint8_t colour;

		/* Find the co-ordinates on the level. */
		struct loc grid = path_g[i];
		struct monster *mon = square_monster(cave, grid);
		struct object *obj = square_object(player->cave, grid);

		/*
		 * As path[] is a straight line and the screen is oblong,
		 * there is only section of path[] on-screen.
		 * If the square being drawn is visible, this is part of it.
		 * If none of it has been drawn, continue until some of it
		 * is found or the last square is reached.
		 * If some of it has been drawn, finish now as there are no
		 * more visible squares to draw.
		 */
		 if (panel_contains(grid.y, grid.x)) on_screen = true;
		 else if (on_screen) break;
		 else continue;

	 	/* Find the position on-screen */
		move_cursor_relative(grid.y, grid.x);

		/* This square is being overwritten, so save the original. */
		Term_what(Term->scr->cx, Term->scr->cy, a + i, c + i);

		/* Choose a colour. */
		if (pastknown) {
			/* Once we pass an unknown square, we no longer know
			 * if we will reach later squares */
			colour = COLOUR_L_DARK;
		} else if (mon && monster_is_visible(mon)) {
			/* Mimics act as objects */
			if (monster_is_camouflaged(mon)) 
				colour = COLOUR_YELLOW;
			else
				/* Visible monsters are red. */
				colour = COLOUR_L_RED;
		} else if (obj)
			/* Known objects are yellow. */
			colour = COLOUR_YELLOW;

		else if (!square_isprojectable(cave, grid) &&
				 (square_isknown(cave, grid) || square_isseen(cave, grid)))
			/* Known walls are blue. */
			colour = COLOUR_BLUE;

		else if (!square_isknown(cave, grid) && !square_isseen(cave, grid)) {
			/* Unknown squares are grey. */
			pastknown = true;
			colour = COLOUR_L_DARK;

		} else
			/* Unoccupied squares are white. */
			colour = COLOUR_WHITE;

		/* Draw the path segment */
		(void)Term_addch(colour, L'*');
	}
	return i;
}


/**
 * Load the attr/char at each point along "path" which is on screen from
 * "a" and "c". This was saved in draw_path().
 */
static void load_path(uint16_t path_n, struct loc *path_g, wchar_t *c, int *a)
{
	int i;
	for (i = 0; i < path_n; i++) {
		int y = path_g[i].y;
		int x = path_g[i].x;

		if (!panel_contains(y, x)) continue;
		move_cursor_relative(y, x);
		Term_addch(a[i], c[i]);
	}

	Term_fresh();
}

/**
 * Return true if the object pile contains the player's tracked object
 */
static bool pile_is_tracked(const struct object *obj) {
	for (const struct object *o = obj; o != NULL; o = o->next) {
		if (player->upkeep->object == o) {
			return true;
		}
	}
	return false;
}

/**
 * Return true if the object pile contains at least 1 known item
 */
static bool pile_has_known(const struct object *obj) {
	for (const struct object *o = obj; o != NULL; o = o->next) {
		struct object *base_obj = cave->objects[o->oidx];
		if (!is_unknown(base_obj)) {
			return true;
		}
	}
	return false;
}

/**
 * Handle "target" and "look". May be called from commands or "get_aim_dir()".
 *
 * Currently, when "interesting" grids are being used, and a directional key is
 * pressed, we only scroll by a single panel, in the direction requested, and
 * check for any interesting grids on that panel.  The "correct" solution would
 * actually involve scanning a larger set of grids, including ones in panels
 * which are adjacent to the one currently scanned, but this is overkill for
 * this function.
 *
 * Targetting/observing an "outer border grid" may induce problems, so this is
 * not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 *
 *
 * 'mode' is one of TARGET_LOOK or TARGET_KILL.
 * 'x' and 'y' are the initial position of the target to be highlighted,
 * or -1 if no location is specified.
 * Returns true if a target has been successfully set, false otherwise.
 */
bool target_set_interactive(int mode, int x, int y)
{
	int path_n;
	struct loc path_g[256];

	int wid, hgt, help_prompt_loc;

	bool done = false;
	bool show_interesting = true;
	bool help = false;
	keycode_t ignore_key = cmd_lookup_key(CMD_IGNORE,
		(OPT(player, rogue_like_commands)) ?
		KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG);

	/* These are used for displaying the path to the target */
	wchar_t *path_char = mem_zalloc(z_info->max_range * sizeof(wchar_t));
	int *path_attr = mem_zalloc(z_info->max_range * sizeof(int));

	/* If we haven't been given an initial location, start on the
	   player, otherwise  honour it by going into "free targetting" mode. */
	if (x == -1 || y == -1 || !square_in_bounds_fully(cave, loc(x, y))) {
		x = player->grid.x;
		y = player->grid.y;
	} else {
		show_interesting = false;
	}

	/* Cancel target */
	target_set_monster(0);

	/* Prevent animations */
	disallow_animations();

	/* Calculate the window location for the help prompt */
	Term_get_size(&wid, &hgt);
	help_prompt_loc = hgt - 1;
	
	/* Display the help prompt */
	prt("Press '?' for help.", help_prompt_loc, 0);

	/* Prepare the target set */
	struct point_set *targets = target_get_monsters(mode, NULL, true);
	int target_index = 0;

	/* Interact */
	while (!done) {
		bool path_drawn = false;
		bool use_interesting_mode = show_interesting && point_set_size(targets);
		bool use_free_mode = !use_interesting_mode;

		/* Use an interesting grid if requested and there are any */
		if (use_interesting_mode) {
			y = targets->pts[target_index].y;
			x = targets->pts[target_index].x;

			/* Adjust panel if needed */
			if (adjust_panel_help(y, x, help)) handle_stuff(player);
		}

		/* Update help */
		if (help) {
			bool has_target = target_able(square_monster(cave, loc(x, y)));
			bool has_object = !(mode & TARGET_KILL)
					&& pile_has_known(square_object(cave, loc(x, y)));
			target_display_help(has_target, has_object, use_free_mode);
		}

		/* Find the path. */
		path_n = project_path(cave, path_g, z_info->max_range,
			loc(player->grid.x, player->grid.y), loc(x, y),
			PROJECT_THRU | PROJECT_INFO);

		/* Draw the path in "target" mode. If there is one */
		if (mode & (TARGET_KILL))
			path_drawn = draw_path(path_n, path_g, path_char, path_attr,
					player->grid.y, player->grid.x);

		/* Describe and Prompt */
		ui_event press = target_set_interactive_aux(y, x,
				mode | (use_free_mode ? TARGET_LOOK : 0));

		/* Remove the path */
		if (path_drawn) load_path(path_n, path_g, path_char, path_attr);

		/* Handle an input event */
		if (event_is_mouse_m(press, 2, KC_MOD_CONTROL) || event_is_mouse(press, 3)) {
			/* Set a target and done */
			y = KEY_GRID_Y(press);
			x = KEY_GRID_X(press);
			if (use_free_mode) {
				/* Free mode: Target a location */
				target_set_location(y, x);
				done = true;
			} else {
				/* Interesting mode: Try to target a monster and done, or bell */
				struct monster *m_local = square_monster(cave, loc(x, y));

				if (target_able(m_local)) {
					/* Monster race and health tracked by target_set_interactive_aux() */
					target_set_monster(m_local);
					done = true;
				} else {
					bell();
					if (!square_in_bounds(cave, loc(x, y))) {
						x = player->grid.x;
						y = player->grid.y;
					}
				}
			}

		} else if (event_is_mouse_m(press, 2, KC_MOD_ALT)) {
			/* Navigate to location and done */
			y = KEY_GRID_Y(press);
			x = KEY_GRID_X(press);
			cmdq_push(CMD_PATHFIND);
			cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
			done = true;

		} else if (event_is_mouse(press, 2)) {
			/* Cancel and done */
			if (use_free_mode && (mode & TARGET_KILL)
					&& y == KEY_GRID_Y(press) && x == KEY_GRID_X(press)) {
				/* Free/kill mode: Clicked current location, set target */
				target_set_location(y, x);
			}
			done = true;

		} else if (event_is_mouse(press, 1)) {
			/* Relocate cursor */
			y = KEY_GRID_Y(press);
			x = KEY_GRID_X(press);

			/* If they clicked on an edge of the map, drag the cursor further
			   to trigger a panel scroll */
			if (press.mouse.y <= 1) {
				y--;
			} else if (press.mouse.y >= Term->hgt - 2) {
				y++;
			} else if (press.mouse.x <= COL_MAP) {
				x--;
			} else if (press.mouse.x >= Term->wid - 2) {
				x++;
			}

			/* Restrict cursor to inbounds */
			x = MAX(0, MIN(x, cave->width - 1));
			y = MAX(0, MIN(y, cave->height - 1));

			/* Adjust panel if needed */
			if (adjust_panel_help(y, x, help)) {
				handle_stuff(player);

				/* Recalculate interesting grids */
				point_set_dispose(targets);
				targets = target_get_monsters(mode, NULL, true);
			}

			/* Turn interesting mode off if they clicked a boring spot... */
			show_interesting = false;

			/* ...but turn it on if they clicked an interesting spot */
			for (int i = 0; i < point_set_size(targets); i++) {
				if (y == targets->pts[i].y && x == targets->pts[i].x) {
					target_index = i;
					show_interesting = true;
					break;
				}
			}

		} else if (event_is_key(press, ESCAPE) || event_is_key(press, 'q')) {
			/* Cancel */
			done = true;

		} else if (event_is_key(press, ' ') || event_is_key(press, '*')
				|| event_is_key(press, '+')) {
			/* Cycle interesting target forward */
			if (use_interesting_mode && ++target_index == point_set_size(targets)) {
				target_index = 0;
			}

		} else if (event_is_key(press, '-')) {
			/* Cycle interesting target backwards */
			if (use_interesting_mode && target_index-- == 0) {
				target_index = point_set_size(targets) - 1;
			}

		} else if (event_is_key(press, 'p')) {
			/* Focus the player and switch to free mode */
			y = player->grid.y;
			x = player->grid.x;
			show_interesting = false;

			/* Recenter around player */
			verify_panel();
			handle_stuff(player);

		} else if (event_is_key(press, 'o')) {
			/* Switch to free mode */
			show_interesting = false;

		} else if (event_is_key(press, 'm')) {
			/* Switch to interesting mode */
			if (use_free_mode && point_set_size(targets) > 0) {
				show_interesting = true;
				target_index = 0;
				int min_dist = 999;

				/* Pick the nearest interesting target */
				for (int i = 0; i < point_set_size(targets); i++) {
					int dist = distance(loc(x, y), targets->pts[i]);
					if (dist < min_dist) {
						target_index = i;
						min_dist = dist;
					}
				}
			}

		} else if (event_is_key(press, 't') || event_is_key(press, '5')
				|| event_is_key(press, '0') || event_is_key(press, '.')) {
			/* Set a target and done */
			if (use_interesting_mode) {
				struct monster *m_local = square_monster(cave, loc(x, y));

				if (target_able(m_local)) {
					/* Monster race and health tracked by target_set_interactive_aux() */
					target_set_monster(m_local);
					done = true;
				} else {
					bell();
				}
			} else {
				target_set_location(y, x);
				done = true;
			}

		} else if (event_is_key(press, 'g')) {
			/* Navigate to a location and done */
			cmdq_push(CMD_PATHFIND);
			cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
			done = true;

		} else if (event_is_key(press, ignore_key)) {
			/* Ignore the tracked object, set by target_set_interactive_aux() */
			if (!(mode & TARGET_KILL)
					&& pile_is_tracked(square_object(cave, loc(x, y)))) {
				textui_cmd_ignore_menu(player->upkeep->object);
				handle_stuff(player);

				/* Recalculate interesting grids */
				point_set_dispose(targets);
				targets = target_get_monsters(mode, NULL, true);
			}

		} else if (event_is_key(press, '?')) {
			/* Toggle help text */
			help = !help;

			/* Redraw main window */
			player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
			Term_clear();
			handle_stuff(player);
			if (!help)
				prt("Press '?' for help.", help_prompt_loc, 0);

		} else {
			/* Try to extract a direction from the key press */
			int dir = target_dir(press.key);

			if (!dir) {
				bell();
			} else if (use_interesting_mode) {
				/* Interesting mode direction: Pick new interesting grid */
				int old_y = targets->pts[target_index].y;
				int old_x = targets->pts[target_index].x;
				int new_index;

				/* Look for a new interesting grid */
				new_index = target_pick(old_y, old_x, ddy[dir], ddx[dir], targets);

				/* If none found, try in the next panel */
				if (new_index < 0) {
					int old_wy = Term->offset_y;
					int old_wx = Term->offset_x;

					if (change_panel(dir)) {
						/* Recalculate interesting grids */
						point_set_dispose(targets);
						targets = target_get_monsters(mode, NULL, true);

						/* Look for a new interesting grid again */
						new_index = target_pick(old_y, old_x, ddy[dir], ddx[dir], targets);

						/* If none found again, reset the panel and do nothing */
						if (new_index < 0 && modify_panel(Term, old_wy, old_wx)) {
							/* Recalculate interesting grids */
							point_set_dispose(targets);
							targets = target_get_monsters(mode, NULL, true);
						}

						handle_stuff(player);
					}
				}

				/* Use interesting grid if found */
				if (new_index >= 0) target_index = new_index;
			} else {
				int step = (is_running_keymap(press.key)) ?
					10 : 1;

				/* Free mode direction: Move cursor */
				x += step * ddx[dir];
				y += step * ddy[dir];

				/* Keep 1 away from the edge */
				x = MAX(1, MIN(x, cave->width - 2));
				y = MAX(1, MIN(y, cave->height - 2));

				/* Adjust panel if needed */
				if (adjust_panel_help(y, x, help)) {
					handle_stuff(player);

					/* Recalculate interesting grids */
					point_set_dispose(targets);
					targets = target_get_monsters(mode, NULL, true);
				}
			}
		}
		/* End of while finally */
	}

	/* Forget */
	point_set_dispose(targets);

	/* Redraw as necessary */
	if (help) {
		player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
		Term_clear();
	} else {
		prt("", 0, 0);
		prt("", help_prompt_loc, 0);
		player->upkeep->redraw |= (PR_DEPTH | PR_STATUS);
	}

	/* Recenter around player */
	verify_panel();

	handle_stuff(player);

	mem_free(path_attr);
	mem_free(path_char);

	/* Allow animations again */
	allow_animations();

	return target_is_set();
}
