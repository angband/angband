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
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-map.h"
#include "ui-mon-lore.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-target.h"
#include "ui-term.h"

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
void target_display_help(bool monster, bool free)
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
	text_out(" moves to the selection. '");
	text_out_c(COLOUR_L_GREEN, "p");
	text_out("' selects the player. '");
	text_out_c(COLOUR_L_GREEN, "q");
	text_out("' exits. '");
	text_out_c(COLOUR_L_GREEN, "r");
	text_out("' displays details. '");

	if (free)
	{
		text_out_c(COLOUR_L_GREEN, "m");
		text_out("' restricts to interesting places. ");
	}
	else
	{
		text_out_c(COLOUR_L_GREEN, "+");
		text_out("' and '");
		text_out_c(COLOUR_L_GREEN, "-");
		text_out("' cycle through interesting places. '");
		text_out_c(COLOUR_L_GREEN, "o");
		text_out("' allows free selection. ");
	}
	
	if (monster || free)
	{
		text_out("'");
		text_out_c(COLOUR_L_GREEN, "t");
		text_out("' targets the current selection.");
	}

	/* Reset */
	text_out_indent = 0;
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

	int screen_hgt_main = help ? (Term->hgt - ROW_MAP - 3) 
			 : (Term->hgt - ROW_MAP - 1);

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		int wx, wy;
		int screen_hgt, screen_wid;

		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(window_flag[j] & PW_MAPS)) continue;

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
										  const char *s1, const char *s2,
										  const char *s3, char *coords)
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
			object_desc(o_name, sizeof(o_name), cave->objects[obj->oidx],
						ODESC_PREFIX | ODESC_FULL);

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
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text.  This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * This function correctly handles multiple objects per grid, and objects
 * and terrain features in the same grid, though the latter never happens.
 *
 * This function must handle blindness/hallucination.
 */
static ui_event target_set_interactive_aux(int y, int x, int mode)
{
	struct object *obj = NULL;

	const char *s1, *s2, *s3;

	bool boring;

	int floor_max = z_info->floor_size;
	struct object **floor_list = mem_zalloc(floor_max * sizeof(*floor_list));
	int floor_num;

	ui_event press;

	char out_val[TARGET_OUT_VAL_SIZE];

	char coords[20];

	const char *name;

	/* Describe the square location */
	coords_desc(coords, sizeof(coords), y, x);

	/* Repeat forever */
	while (1) {
		/* Make the default event to focus on the player */
		press.type = EVT_KBRD;
		press.key.code = 'p';
		press.key.mods = 0;

		/* Assume boring */
		boring = true;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";

		/* Bail if looking at a forbidden grid */
		if (!square_in_bounds(cave, loc(x, y))) {
			break;
		}

		/* The player */
		if (square(cave, loc(x, y)).mon < 0) {
			/* Description */
			s1 = "You are ";

			/* Preposition */
			s2 = "on ";
		}

		/* Hallucination messes things up */
		if (player->timed[TMD_IMAGE]) {
			const char *name_strange = "something strange";

			/* Display a message */
			if (player->wizard)
				strnfmt(out_val, sizeof(out_val),
						"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).", s1, s2, s3,
						name_strange, coords, y, x, (int)cave->noise.grids[y][x],
						(int)cave->scent.grids[y][x]);
			else
				strnfmt(out_val, sizeof(out_val), "%s%s%s%s, %s.",
						s1, s2, s3, name_strange, coords);

			prt(out_val, 0, 0);
			move_cursor_relative(y, x);

			press.key = inkey();

			/* Stop on everything but "return" */
			if (press.key.code == KC_ENTER)
				continue;

			mem_free(floor_list);
			return press;
		}

		/* Actual monsters */
		if (square(cave, loc(x, y)).mon > 0) {
			struct monster *mon = square_monster(cave, loc(x, y));
			const struct monster_lore *lore = get_lore(mon->race);

			/* Visible */
			if (monster_is_obvious(mon)) {
				bool recall = false;

				char m_name[80];

				/* Not boring */
				boring = false;

				/* Get the monster name ("a kobold") */
				monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_VIS);

				/* Hack -- track this monster race */
				monster_race_track(player->upkeep, mon->race);

				/* Hack -- health bar for this monster */
				health_track(player->upkeep, mon);

				/* Hack -- handle stuff */
				handle_stuff(player);

				/* Interact */
				while (1) {
					/* Recall or target */
					if (recall) {
						lore_show_interactive(mon->race, lore);
						press = inkey_m();
					} else {
						char buf[80];

						/* Describe the monster */
						look_mon_desc(buf, sizeof(buf),
									  square(cave, loc(x, y)).mon);

						/* Describe, and prompt for recall */
						if (player->wizard) {
							strnfmt(out_val, sizeof(out_val),
									"%s%s%s%s (%s), %s (%d:%d, noise=%d, scent=%d).",
									s1, s2, s3, m_name, buf, coords, y, x,
									(int)cave->noise.grids[y][x],
									(int)cave->scent.grids[y][x]);
						} else {
							strnfmt(out_val, sizeof(out_val),
									"%s%s%s%s (%s), %s.",
									s1, s2, s3, m_name, buf, coords);
						}

						prt(out_val, 0, 0);

						/* Place cursor */
						move_cursor_relative(y, x);

						/* Command */
						press = inkey_m();
					}

					/* Normal commands */
					if ((press.type == EVT_MOUSE) && (press.mouse.button == 1)
						&& (KEY_GRID_X(press) == x) && (KEY_GRID_Y(press) == y))
						recall = !recall;
					else
					if ((press.type == EVT_KBRD) && (press.key.code == 'r'))
						recall = !recall;
					else
						break;
				}

				if (press.type == EVT_MOUSE) {
					/* Stop on right click */
					if (press.mouse.button == 2)
						break;

					/* Sometimes stop at "space" key */
					if (press.mouse.button && !(mode & (TARGET_LOOK))) break;
				} else {
					/* Stop on everything but "return"/"space" */
					if (press.key.code != KC_ENTER && press.key.code != ' ')
						break;

					/* Sometimes stop at "space" key */
					if ((press.key.code == ' ') && !(mode & (TARGET_LOOK)))
						break;
				}

				/* Take account of gender */
				if (rf_has(mon->race->flags, RF_FEMALE)) s1 = "She is ";
				else if (rf_has(mon->race->flags, RF_MALE)) s1 = "He is ";
				else s1 = "It is ";

				/* Describe carried objects (wizards only) */
				if (player->wizard) {
					/* Use a verb */
					s2 = "carrying ";

					/* Scan all objects being carried */
					for (obj = mon->held_obj; obj; obj = obj->next) {
						char o_name[80];

						/* Obtain an object description */
						object_desc(o_name, sizeof(o_name), obj,
									ODESC_PREFIX | ODESC_FULL);

						strnfmt(out_val, sizeof(out_val),
								"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).",
								s1, s2, s3, o_name, coords, y, x,
								(int)cave->noise.grids[y][x],
								(int)cave->scent.grids[y][x]);

						prt(out_val, 0, 0);
						move_cursor_relative(y, x);
						press = inkey_m();

						if (press.type == EVT_MOUSE) {
							/* Stop on right click */
							if (press.mouse.button == 2)
								break;

							/* Sometimes stop at "space" key */
							if (press.mouse.button && !(mode & (TARGET_LOOK)))
								break;
						} else {
							/* Stop on everything but "return"/"space" */
							if ((press.key.code != KC_ENTER) &&
								(press.key.code != ' '))
								break;

							/* Sometimes stop at "space" key */
							if ((press.key.code == ' ') &&
								!(mode & (TARGET_LOOK)))
								break;
						}

						/* Change the intro */
						s2 = "also carrying ";
					}
				}

				/* Double break */
				if (obj) break;

				/* Use a preposition */
				s2 = "on ";
			}
		}

		/* A trap */
		if (square_isvisibletrap(cave, loc(x, y))) {
			struct trap *trap = square(cave, loc(x, y)).trap;

			/* Not boring */
			boring = false;

			/* Interact */
			while (1) {
				/* Change the intro */
				if (square(cave, loc(x, y)).mon < 0) {
					s1 = "You are ";
					s2 = "on ";
				} else {
					s1 = "You see ";
					s2 = "";
				}

				/* Pick proper indefinite article */
				s3 = (is_a_vowel(trap->kind->desc[0])) ? "an " : "a ";

				/* Describe, and prompt for recall */
				if (player->wizard) {
					strnfmt(out_val, sizeof(out_val),
							"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).", s1, s2,
							s3, trap->kind->name, coords, y, x,
							(int)cave->noise.grids[y][x],
							(int)cave->scent.grids[y][x]);
				} else {
					strnfmt(out_val, sizeof(out_val), "%s%s%s%s, %s.", 
							s1, s2, s3, trap->kind->desc, coords);
				}

				prt(out_val, 0, 0);

				/* Place cursor */
				move_cursor_relative(y, x);

				/* Command */
				press = inkey_m();
		
				/* Stop on everything but "return"/"space" */
				if ((press.key.code != KC_ENTER) && (press.key.code != ' '))
					break;
		
				/* Sometimes stop at "space" key */
				if ((press.key.code == ' ') && !(mode & (TARGET_LOOK)))
					break;
			}
		}
	
		/* Double break */
		if (square_isvisibletrap(cave, loc(x, y)))
			break;
	
		/* Scan all sensed objects in the grid */
		floor_num = scan_distant_floor(floor_list, floor_max, loc(x, y));
		if ((floor_num > 0) &&
		    (!(player->timed[TMD_BLIND]) || loc_eq(loc(x, y), player->grid))) {
			/* Not boring */
			boring = false;

			track_object(player->upkeep, floor_list[0]);
			handle_stuff(player);

			/* If there is more than one item... */
			if (floor_num > 1)
				while (1) {
					/* Describe the pile */
					if (player->wizard) {
						strnfmt(out_val, sizeof(out_val),
								"%s%s%sa pile of %d objects, %s (%d:%d, noise=%d, scent=%d).",
								s1, s2, s3, floor_num, coords, y, x,
								(int)cave->noise.grids[y][x],
								(int)cave->scent.grids[y][x]);
					} else {
						strnfmt(out_val, sizeof(out_val),
								"%s%s%sa pile of %d objects, %s.",
								s1, s2, s3, floor_num, coords);
					}

					prt(out_val, 0, 0);
					move_cursor_relative(y, x);
					press = inkey_m();

					/* Display objects */
					if (((press.type == EVT_MOUSE) && (press.mouse.button == 1)
						 && (KEY_GRID_X(press) == x) && 
						 (KEY_GRID_Y(press) == y)) ||
						((press.type == EVT_KBRD) && (press.key.code == 'r'))) {
						int rdone = 0;
						int pos;
						while (!rdone) {
							/* Save screen */
							screen_save();

							/* Display */
							show_floor(floor_list, floor_num,
									   (OLIST_WEIGHT | OLIST_GOLD), NULL);

							/* Describe the pile */
							prt(out_val, 0, 0);
							press = inkey_m();

							/* Load screen */
							screen_load();

							if (press.type == EVT_MOUSE) {
								pos = press.mouse.y-1;
							} else {
								pos = press.key.code - 'a';
							}
							if (0 <= pos && pos < floor_num) {
								track_object(player->upkeep, floor_list[pos]);
								handle_stuff(player);
								continue;
							}
							rdone = 1;
						}

						/* Now that the user's done with the display loop,
						 * let's do the outer loop over again */
						continue;
					}

					/* Done */
					break;
				}
			/* Only one object to display */
			else {
				/* Get the single object in the list */
				struct object *obj_local = floor_list[0];

				/* Allow user to recall an object */
				press = target_recall_loop_object(obj_local, y, x, out_val, s1, s2,
												  s3, coords);

				/* Stop on everything but "return"/"space" */
				if ((press.key.code != KC_ENTER) && (press.key.code != ' '))
					break;

				/* Sometimes stop at "space" key */
				if ((press.key.code == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Plurals */
				s1 = VERB_AGREEMENT(obj_local->number, "It is ", "They are ");

				/* Preposition */
				s2 = "on ";
			}

		}

		/* Double break */
		if (obj) break;

		name = square_apparent_name(cave, player, loc(x, y));

		/* Terrain feature if needed */
		if (boring || square_isinteresting(cave, loc(x, y))) {
			/* Hack -- handle unknown grids */

			/* Pick a prefix */
			if (*s2 && square_isdoor(cave, loc(x, y))) s2 = "in ";

			/* Pick proper indefinite article */
			s3 = (is_a_vowel(name[0])) ? "an " : "a ";

			/* Hack -- special introduction for store doors */
			if (square_isshop(cave, loc(x, y)))
				s3 = "the entrance to the ";

			/* Display a message */
			if (player->wizard) {
				strnfmt(out_val, sizeof(out_val),
						"%s%s%s%s, %s (%d:%d, noise=%d, scent=%d).", s1, s2, s3,
						name, coords, y, x, (int)cave->noise.grids[y][x],
						(int)cave->scent.grids[y][x]);
			} else {
				strnfmt(out_val, sizeof(out_val),
						"%s%s%s%s, %s.", s1, s2, s3, name, coords);
			}

			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			press = inkey_m();

			if (press.type == EVT_MOUSE) {
				/* Stop on right click */
				if (press.mouse.button == 2)
					break;
			} else {
				/* Stop on everything but "return"/"space" */
				if ((press.key.code != KC_ENTER) && (press.key.code != ' '))
					break;
			}
		}

		/* Stop on everything but "return" */
		if (press.type == EVT_MOUSE) {
				/* Stop on right click */
				if (press.mouse.button != 2)
					break;
		} else {
    			if (press.key.code != KC_ENTER) break;
		}
	}

	mem_free(floor_list);

	/* Keep going */
	return (press);
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
static int draw_path(u16b path_n, struct loc *path_g, wchar_t *c, int *a,
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
		byte colour;

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
static void load_path(u16b path_n, struct loc *path_g, wchar_t *c, int *a)
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
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_aim_dir()".
 *
 * Currently, when "flag" is true, that is, when
 * "interesting" grids are being used, and a directional key is used, we
 * only scroll by a single panel, in the direction requested, and check
 * for any interesting grids on that panel.  The "correct" solution would
 * actually involve scanning a larger set of grids, including ones in
 * panels which are adjacent to the one currently scanned, but this is
 * overkill for this function.  XXX XXX
 *
 * Hack -- targetting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
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
	int py = player->grid.y;
	int px = player->grid.x;

	int path_n;
	struct loc path_g[256];

	int i, d, m, t, bd;
	int wid, hgt, help_prompt_loc;

	bool done = false;
	bool flag = true;
	bool help = false;

	ui_event press;

	/* These are used for displaying the path to the target */
	wchar_t *path_char = mem_zalloc(z_info->max_range * sizeof(wchar_t));
	int *path_attr = mem_zalloc(z_info->max_range * sizeof(int));
	struct point_set *targets;

	/* If we haven't been given an initial location, start on the
	   player, otherwise  honour it by going into "free targetting" mode. */
	if (x == -1 || y == -1 || !square_in_bounds_fully(cave, loc(x, y))) {
		x = player->grid.x;
		y = player->grid.y;
	} else {
		flag = false;
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
	targets = target_get_monsters(mode, NULL);

	/* Start near the player */
	m = 0;

	/* Interact */
	while (!done) {
		bool path_drawn = false;
		
		/* Interesting grids if chosen and there are any, otherwise arbitrary */
		if (flag && point_set_size(targets)) {
			y = targets->pts[m].y;
			x = targets->pts[m].x;

			/* Adjust panel if needed */
			if (adjust_panel_help(y, x, help)) handle_stuff(player);
		
			/* Update help */
			if (help) {
				bool good_target =
					target_able(square_monster(cave, targets->pts[m]));
				target_display_help(good_target,
									!(flag && point_set_size(targets)));
			}

			/* Find the path. */
			path_n = project_path(path_g, z_info->max_range, loc(px, py),
								  loc(x, y), PROJECT_THRU | PROJECT_INFO);

			/* Draw the path in "target" mode. If there is one */
			if (mode & (TARGET_KILL))
				path_drawn = draw_path(path_n, path_g, path_char, path_attr,
									   py, px);

			/* Describe and Prompt */
			press = target_set_interactive_aux(y, x, mode);

			/* Remove the path */
			if (path_drawn) load_path(path_n, path_g, path_char, path_attr);

			/* Assume no "direction" */
			d = 0;


			/* Analyze */
			if (press.type == EVT_MOUSE) {
				if (press.mouse.button == 3) {
					/* give the target selection command */
					press.mouse.button = 2;
					press.mouse.mods = KC_MOD_CONTROL;
				}
				if (press.mouse.button == 2) {
					y = KEY_GRID_Y(press);
					x = KEY_GRID_X(press);
					if (press.mouse.mods & KC_MOD_CONTROL) {
						/* same as keyboard target selection command below */
						struct monster *m_local = square_monster(cave, loc(x, y));

						if (target_able(m_local)) {
							/* Set up target information */
							monster_race_track(player->upkeep, m_local->race);
							health_track(player->upkeep, m_local);
							target_set_monster(m_local);
							done = true;
						} else {
							bell("Illegal target!");
							/*
							 * So there's something
							 * to work with in the
							 * next pass through
							 * the loop.
							 */
							if (! square_in_bounds(cave, loc(x, y))) {
							    x = player->grid.x;
							    y = player->grid.y;
							}
						}
					} else if (press.mouse.mods & KC_MOD_ALT) {
						/* go to spot - same as 'g' command below */
						cmdq_push(CMD_PATHFIND);
						cmd_set_arg_point(cmdq_peek(), "point", y, x);
						done = true;
					} else {
						/* cancel look mode */
						done = true;
					}
				} else {
					y = KEY_GRID_Y(press);
					x = KEY_GRID_X(press);
					if (square_monster(cave, loc(x, y)) ||
						square_object(cave, loc(x, y))) {
							/* reset the flag, to make sure we stay in this
							 * mode if something is actually there */
						flag = false;
						/* scan the interesting list and see if there is
						 * anything here */
						for (i = 0; i < point_set_size(targets); i++) {
							if ((y == targets->pts[i].y) &&
								(x == targets->pts[i].x)) {
								m = i;
								flag = true;
								break;
							}
						}
					} else {
						flag = false;
						if (! square_in_bounds(cave, loc(x, y))) {
						    x = player->grid.x;
						    y = player->grid.y;
						}
					}
				}
			} else
				switch (press.key.code)
				{
					case ESCAPE:
					case 'q':
					{
						done = true;
						break;
					}

					case ' ':
					case '*':
					case '+':
					{
						if (++m == point_set_size(targets))
							m = 0;

						break;
					}

					case '-':
					{
						if (m-- == 0)
							m = point_set_size(targets) - 1;

						break;
					}

					case 'p':
					{
						/* Recenter around player */
						verify_panel();

						/* Handle stuff */
						handle_stuff(player);

						y = player->grid.y;
						x = player->grid.x;
						flag = false;
						break;
					}

					case 'o':
					{
						flag = false;
						break;
					}

					case 'm':
					{
						break;
					}

					case 't':
					case '5':
					case '0':
					case '.':
					{
						struct monster *m_local = square_monster(cave, loc(x, y));

						if (target_able(m_local)) {
							health_track(player->upkeep, m_local);
							target_set_monster(m_local);
							done = true;
						} else {
							bell("Illegal target!");
						}
						break;
					}

					case 'g':
					{
						cmdq_push(CMD_PATHFIND);
						cmd_set_arg_point(cmdq_peek(), "point", y, x);
						done = true;
						break;
					}
				
					case '?':
					{
						help = !help;
					
						/* Redraw main window */
						player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
						Term_clear();
						handle_stuff(player);
						if (!help)
							prt("Press '?' for help.", help_prompt_loc, 0);
					
						break;
					}

					default:
					{
						/* Extract direction */
						d = target_dir(press.key);

						/* Oops */
						if (!d) bell("Illegal command for target mode!");

						break;
					}
				}

			/* Hack -- move around */
			if (d) {
				int old_y = targets->pts[m].y;
				int old_x = targets->pts[m].x;

				/* Find a new monster */
				i = target_pick(old_y, old_x, ddy[d], ddx[d], targets);

				/* Scroll to find interesting grid */
				if (i < 0) {
					int old_wy = Term->offset_y;
					int old_wx = Term->offset_x;

					/* Change if legal */
					if (change_panel(d)) {
						/* Recalculate interesting grids */
						point_set_dispose(targets);
						targets = target_get_monsters(mode, NULL);

						/* Find a new monster */
						i = target_pick(old_y, old_x, ddy[d], ddx[d], targets);

						/* Restore panel if needed */
						if ((i < 0) && modify_panel(Term, old_wy, old_wx)) {
							/* Recalculate interesting grids */
							point_set_dispose(targets);
							targets = target_get_monsters(mode, NULL);
						}

						/* Handle stuff */
						handle_stuff(player);
					}
				}

				/* Use interesting grid if found */
				if (i >= 0) m = i;
			}
		} else {
			/* Update help */
			if (help) {
				bool good_target = target_able(square_monster(cave, loc(x, y)));
				target_display_help(good_target,
									!(flag && point_set_size(targets)));
			}

			/* Find the path. */
			path_n = project_path(path_g, z_info->max_range, loc(px, py),
								  loc(x, y), PROJECT_THRU | PROJECT_INFO);

			/* Draw the path in "target" mode. If there is one */
			if (mode & (TARGET_KILL))
				path_drawn = draw_path (path_n, path_g, path_char, path_attr,
										py, px);

			/* Describe and Prompt (enable "TARGET_LOOK") */
			press = target_set_interactive_aux(y, x, mode | TARGET_LOOK);

			/* Remove the path */
			if (path_drawn)  load_path(path_n, path_g, path_char, path_attr);

			/* Assume no direction */
			d = 0;

			/* Analyze the keypress */
			if (press.type == EVT_MOUSE) {
				if (press.mouse.button == 3) {
					/* give the target selection command */
					press.mouse.button = 2;
					press.mouse.mods = KC_MOD_CONTROL;
				}
				if (press.mouse.button == 2) {
					if (mode & (TARGET_KILL)) {
						if ((y == KEY_GRID_Y(press)) 
								&& (x == KEY_GRID_X(press))) {
							d = -1;
						}
					}
					y = KEY_GRID_Y(press);
					x = KEY_GRID_X(press);
					if (press.mouse.mods & KC_MOD_CONTROL) {
						/* same as keyboard target selection command below */
						target_set_location(y, x);
						done = true;
					} else if (press.mouse.mods & KC_MOD_ALT) {
						/* go to spot - same as 'g' command below */
						cmdq_push(CMD_PATHFIND);
						cmd_set_arg_point(cmdq_peek(), "point", y, x);
						done = true;
					} else {
						/* cancel look mode */
						done = true;
						if (d == -1) {
							target_set_location(y, x);
							d = 0;
						}
					}
				} else {
					int dungeon_hgt = cave->height;
					int dungeon_wid = cave->width;

					y = KEY_GRID_Y(press);
					x = KEY_GRID_X(press);
				  
					if (press.mouse.y <= 1) {
						/* move the screen north */
						y--;
					} else if (press.mouse.y >= (Term->hgt - 2)) {
						/* move the screen south */
						y++;
					} else if (press.mouse.x <= COL_MAP) {
						/* move the screen in west */
						x--;
					} else if (press.mouse.x >= (Term->wid - 2)) {
						/* move the screen east */
						x++;
					}
          
					if (y < 0) y = 0;
					if (x < 0) x = 0;
					if (y >= dungeon_hgt-1) y = dungeon_hgt-1;
					if (x >= dungeon_wid-1) x = dungeon_wid-1;

					/* Adjust panel if needed */
					if (adjust_panel_help(y, x, help)) {
						/* Handle stuff */
						handle_stuff(player);

						/* Recalculate interesting grids */
						point_set_dispose(targets);
						targets = target_get_monsters(mode, NULL);
					}

					if (square_monster(cave, loc(x, y)) ||
						square_object(cave, loc(x, y))) {
						/* scan the interesting list and see if there in
						 * anything here */
						for (i = 0; i < point_set_size(targets); i++) {
							if ((y == targets->pts[i].y) &&
								(x == targets->pts[i].x)) {
								m = i;
								flag = true;
								break;
							}
						}
					} else {
						flag = false;
					}
				}
			} else
				switch (press.key.code)
				{
					case ESCAPE:
					case 'q':
					{
						done = true;
						break;
					}

					case ' ':
					case '*':
					case '+':
					case '-':
					{
						break;
					}

					case 'p':
					{
						/* Recenter around player */
						verify_panel();

						/* Handle stuff */
						handle_stuff(player);

						y = player->grid.y;
						x = player->grid.x;
					}

					case 'o':
					{
						break;
					}

					case 'm':
					{
						flag = true;

						m = 0;
						bd = 999;

						/* Pick a nearby monster */
						for (i = 0; i < point_set_size(targets); i++) {
							t = distance(loc(x, y), targets->pts[i]);

							/* Pick closest */
							if (t < bd) {
								m = i;
								bd = t;
							}
						}

						/* Nothing interesting */
						if (bd == 999) flag = false;

						break;
					}

					case 't':
					case '5':
					case '0':
					case '.':
					{
						target_set_location(y, x);
						done = true;
						break;
					}

					case 'g':
					{
						cmdq_push(CMD_PATHFIND);
						cmd_set_arg_point(cmdq_peek(), "point", y, x);
						done = true;
						break;
					}

					case '?':
					{
						help = !help;

						/* Redraw main window */
						player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP);
						Term_clear();
						handle_stuff(player);
						if (!help)
							prt("Press '?' for help.", help_prompt_loc, 0);
					
						break;
					}

					default:
					{
						/* Extract a direction */
						d = target_dir(press.key);

						/* Oops */
						if (!d) bell("Illegal command for target mode!");

						break;
					}
				}

			/* Handle "direction" */
			if (d) {
				int dungeon_hgt = cave->height;
				int dungeon_wid = cave->width;

				/* Move */
				x += ddx[d];
				y += ddy[d];

				/* Slide into legality */
				if (x >= dungeon_wid - 1) x--;
				else if (x <= 0) x++;

				/* Slide into legality */
				if (y >= dungeon_hgt - 1) y--;
				else if (y <= 0) y++;

				/* Adjust panel if needed */
				if (adjust_panel_help(y, x, help)) {
					/* Handle stuff */
					handle_stuff(player);

					/* Recalculate interesting grids */
					point_set_dispose(targets);
					targets = target_get_monsters(mode, NULL);
				}
			}
		}
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

	/* Handle stuff */
	handle_stuff(player);

	mem_free(path_attr);
	mem_free(path_char);

	/* Allow animations again */
	allow_animations();

	/* Failure to set target */
	if (!target_is_set()) return (false);

	/* Success */
	return (true);
}


