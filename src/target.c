/*
 * File: target.c
 * Purpose: Targetting code
 *
 * Copyright (c) 1997-2007 Angband contributors
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


/*** File-wide variables ***/

/* Is the target set? */
bool target_set;

/* Current monster being tracked, or 0 */
u16b target_who;

/* Target location */
s16b target_x, target_y;



/*** Functions ***/

/*
 * Monster health description
 */
static void look_mon_desc(char *buf, size_t max, int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool living = TRUE;


	/* Determine if the monster is "living" (vs "undead") */
	if (r_ptr->flags[2] & (RF3_UNDEAD | RF3_DEMON)) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		my_strcpy(buf, (living ? "unhurt" : "undamaged"), max);
	}
	else
	{
		/* Calculate a health "percentage" */
		int perc = 100L * m_ptr->hp / m_ptr->maxhp;

		if (perc >= 60)
			my_strcpy(buf, (living ? "somewhat wounded" : "somewhat damaged"), max);
		else if (perc >= 25)
			my_strcpy(buf, (living ? "wounded" : "damaged"), max);
		else if (perc >= 10)
			my_strcpy(buf, (living ? "badly wounded" : "badly damaged"), max);
		else
			my_strcpy(buf, (living ? "almost dead" : "almost destroyed"), max);
	}

	if (m_ptr->csleep) my_strcat(buf, ", asleep", max);
	if (m_ptr->confused) my_strcat(buf, ", confused", max);
	if (m_ptr->monfear) my_strcat(buf, ", afraid", max);
	if (m_ptr->stunned) my_strcat(buf, ", stunned", max);
}



/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
bool target_able(int m_idx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	monster_type *m_ptr;

	/* No monster */
	if (m_idx <= 0) return (FALSE);

	/* Get monster */
	m_ptr = &mon_list[m_idx];

	/* Monster must be alive */
	if (!m_ptr->r_idx) return (FALSE);

	/* Monster must be visible */
	if (!m_ptr->ml) return (FALSE);

	/* Monster must be projectable */
	if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) return (FALSE);

	/* Hack -- no targeting hallucinations */
	if (p_ptr->timed[TMD_IMAGE]) return (FALSE);

	/* Hack -- Never target trappers XXX XXX XXX */
	/* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

	/* Assume okay */
	return (TRUE);
}




/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool target_okay(void)
{
	/* No target */
	if (!target_set) return (FALSE);

	/* Accept "location" targets */
	if (target_who == 0) return (TRUE);

	/* Check "monster" targets */
	if (target_who > 0)
	{
		int m_idx = target_who;

		/* Accept reasonable targets */
		if (target_able(m_idx))
		{
			monster_type *m_ptr = &mon_list[m_idx];

			/* Get the monster location */
			target_y = m_ptr->fy;
			target_x = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}


/*
 * Set the target to a monster (or nobody)
 */
void target_set_monster(int m_idx)
{
	/* Acceptable target */
	if ((m_idx > 0) && target_able(m_idx))
	{
		monster_type *m_ptr = &mon_list[m_idx];

		/* Save target info */
		target_set = TRUE;
		target_who = m_idx;
		target_y = m_ptr->fy;
		target_x = m_ptr->fx;
	}

	/* Clear target */
	else
	{
		/* Reset target info */
		target_set = FALSE;
		target_who = 0;
		target_y = 0;
		target_x = 0;
	}
}


/*
 * Set the target to a location
 */
void target_set_location(int y, int x)
{
	/* Legal target */
	if (in_bounds_fully(y, x))
	{
		/* Save target info */
		target_set = TRUE;
		target_who = 0;
		target_y = y;
		target_x = x;
	}

	/* Clear target */
	else
	{
		/* Reset target info */
		target_set = FALSE;
		target_who = 0;
		target_y = 0;
		target_x = 0;
	}
}



/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
static bool ang_sort_comp_distance(const void *u, const void *v, int a, int b)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = x[a]; kx -= px; kx = ABS(kx);
	ky = y[a]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = x[b]; kx -= px; kx = ABS(kx);
	ky = y[b]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	return (da <= db);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static void ang_sort_swap_distance(void *u, void *v, int a, int b)
{
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	byte temp;

	/* Swap "x" */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;

	/* Swap "y" */
	temp = y[a];
	y[a] = y[b];
	y[b] = temp;
}



/*
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(int y1, int x1, int dy, int dx)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < temp_n; i++)
	{
		/* Point 2 */
		x2 = temp_x[i];
		y2 = temp_y[i];

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* Penalize location XXX XXX XXX */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i; b_v = v;
	}

	/* Result */
	return (b_i);
}


/*
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_interactive_accept(int y, int x)
{
	object_type *o_ptr;


	/* Player grids are always interesting */
	if (cave_m_idx[y][x] < 0) return (TRUE);


	/* Handle hallucination */
	if (p_ptr->timed[TMD_IMAGE]) return (FALSE);


	/* Visible monsters */
	if (cave_m_idx[y][x] > 0)
	{
		monster_type *m_ptr = &mon_list[cave_m_idx[y][x]];

		/* Visible monsters */
		if (m_ptr->ml) return (TRUE);
	}

	/* Scan all objects in the grid */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
	{
		/* Memorized object */
		if (o_ptr->marked && !squelch_hide_item(o_ptr)) return (TRUE);
	}

	/* Interesting memorized features */
	if (cave_info[y][x] & (CAVE_MARK))
	{
		/* Notice glyphs */
		if (cave_feat[y][x] == FEAT_GLYPH) return (TRUE);

		/* Notice doors */
		if (cave_feat[y][x] == FEAT_OPEN) return (TRUE);
		if (cave_feat[y][x] == FEAT_BROKEN) return (TRUE);

		/* Notice stairs */
		if (cave_feat[y][x] == FEAT_LESS) return (TRUE);
		if (cave_feat[y][x] == FEAT_MORE) return (TRUE);

		/* Notice shops */
		if ((cave_feat[y][x] >= FEAT_SHOP_HEAD) &&
		    (cave_feat[y][x] <= FEAT_SHOP_TAIL)) return (TRUE);

		/* Notice traps */
		if ((cave_feat[y][x] >= FEAT_TRAP_HEAD) &&
		    (cave_feat[y][x] <= FEAT_TRAP_TAIL)) return (TRUE);

		/* Notice doors */
		if ((cave_feat[y][x] >= FEAT_DOOR_HEAD) &&
		    (cave_feat[y][x] <= FEAT_DOOR_TAIL)) return (TRUE);

		/* Notice rubble */
		if (cave_feat[y][x] == FEAT_RUBBLE) return (TRUE);

		/* Notice veins with treasure */
		if (cave_feat[y][x] == FEAT_MAGMA_K) return (TRUE);
		if (cave_feat[y][x] == FEAT_QUARTZ_K) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}


/*
 * Prepare the "temp" array for "target_interactive_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_interactive_prepare(int mode)
{
	int y, x;

	/* Reset "temp" array */
	temp_n = 0;

	/* Scan the current panel */
	for (y = Term->offset_y; y < Term->offset_y + SCREEN_HGT; y++)
	{
		for (x = Term->offset_x; x < Term->offset_x + SCREEN_WID; x++)
		{
			/* Check bounds */
			if (!in_bounds_fully(y, x)) continue;

			/* Require "interesting" contents */
			if (!target_set_interactive_accept(y, x)) continue;

			/* Special mode */
			if (mode & (TARGET_KILL))
			{
				/* Must contain a monster */
				if (!(cave_m_idx[y][x] > 0)) continue;

				/* Must be a targettable monster */
			 	if (!target_able(cave_m_idx[y][x])) continue;
			}

			/* Save the location */
			temp_x[temp_n] = x;
			temp_y[temp_n] = y;
			temp_n++;
		}
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_distance;
	ang_sort_swap = ang_sort_swap_distance;

	/* Sort the positions */
	ang_sort(temp_x, temp_y, temp_n);
}


/*
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
static ui_event_data target_set_interactive_aux(int y, int x, int mode, cptr info)
{
	s16b this_o_idx = 0, next_o_idx = 0;

	cptr s1, s2, s3;

	bool boring;

	bool floored;

	int feat;

	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

	ui_event_data query;

	char out_val[256];


	/* Repeat forever */
	while (1)
	{
		/* Paranoia */
		query.key = ' ';

		/* Assume boring */
		boring = TRUE;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";


		/* The player */
		if (cave_m_idx[y][x] < 0)
		{
			/* Description */
			s1 = "You are ";

			/* Preposition */
			s2 = "on ";
		}


		/* Hack -- hallucination */
		if (p_ptr->timed[TMD_IMAGE])
		{
			cptr name = "something strange";

			/* Display a message */
			if (p_ptr->wizard)
			{
				strnfmt(out_val, sizeof(out_val),
				        "%s%s%s%s [%s] (%d:%d)", s1, s2, s3, name, info, y, x);
			}
			else
			{
				strnfmt(out_val, sizeof(out_val),
				        "%s%s%s%s [%s]", s1, s2, s3, name, info);
			}

			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey_ex();

			/* Stop on everything but "return" */
			if ((query.key != '\n') && (query.key != '\r')) break;

			/* Repeat forever */
			continue;
		}


		/* Actual monsters */
		if (cave_m_idx[y][x] > 0)
		{
			monster_type *m_ptr = &mon_list[cave_m_idx[y][x]];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Visible */
			if (m_ptr->ml)
			{
				bool recall = FALSE;

				char m_name[80];

				/* Not boring */
				boring = FALSE;

				/* Get the monster name ("a kobold") */
				monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_IND2);

				/* Hack -- track this monster race */
				monster_race_track(m_ptr->r_idx);

				/* Hack -- health bar for this monster */
				health_track(cave_m_idx[y][x]);

				/* Hack -- handle stuff */
				handle_stuff();

				/* Interact */
				while (1)
				{
					/* Recall */
					if (recall)
					{
						/* Save screen */
						screen_save();

						/* Recall on screen */
						screen_roff(m_ptr->r_idx);

						/* Hack -- Complete the prompt (again) */
						Term_addstr(-1, TERM_WHITE, format("  [r,%s]", info));

						/* Command */
						query = inkey_ex();

						/* Load screen */
						screen_load();
					}

					/* Normal */
					else
					{
						char buf[80];

						/* Describe the monster */
						look_mon_desc(buf, sizeof(buf), cave_m_idx[y][x]);

						/* Describe, and prompt for recall */
						if (p_ptr->wizard)
						{
							strnfmt(out_val, sizeof(out_val),
							        "%s%s%s%s (%s) [r,%s] (%d:%d)",
						            s1, s2, s3, m_name, buf, info, y, x);
						}
						else
						{
							strnfmt(out_val, sizeof(out_val),
							        "%s%s%s%s (%s) [r,%s]",
							        s1, s2, s3, m_name, buf, info);
						}

						prt(out_val, 0, 0);

						/* Place cursor */
						move_cursor_relative(y, x);

						/* Command */
						query = inkey_ex();
					}

					/* Normal commands */
					if (query.key != 'r') break;

					/* Toggle recall */
					recall = !recall;
				}

				/* Stop on everything but "return"/"space" */
				if ((query.key != '\n') && (query.key != '\r') && (query.key != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query.key == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Hack -- take account of gender */
				if (r_ptr->flags[0] & (RF1_FEMALE)) s1 = "She is ";
				else if (r_ptr->flags[0] & (RF1_MALE)) s1 = "He is ";

				/* Use a preposition */
				s2 = "carrying ";

				/* Scan all objects being carried */
				for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
				{
					char o_name[80];

					object_type *o_ptr;

					/* Get the object */
					o_ptr = &o_list[this_o_idx];

					/* Get the next object */
					next_o_idx = o_ptr->next_o_idx;

					/* Obtain an object description */
					object_desc(o_name, sizeof(o_name), o_ptr, TRUE, ODESC_FULL);

					/* Describe the object */
					if (p_ptr->wizard)
					{
						strnfmt(out_val, sizeof(out_val),
						        "%s%s%s%s [%s] (%d:%d)",
						        s1, s2, s3, o_name, info, y, x);
					}
					else
					{
						strnfmt(out_val, sizeof(out_val),
						        "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
					}

					prt(out_val, 0, 0);
					move_cursor_relative(y, x);
					query = inkey_ex();

					/* Stop on everything but "return"/"space" */
					if ((query.key != '\n') && (query.key != '\r') && (query.key != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query.key == ' ') && !(mode & (TARGET_LOOK))) break;

					/* Change the intro */
					s2 = "also carrying ";
				}

				/* Double break */
				if (this_o_idx) break;

				/* Use a preposition */
				s2 = "on ";
			}
		}


		/* Assume not floored */
		floored = FALSE;

		floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), y, x, 0x02);

		/* Scan all marked objects in the grid */
		if ((floor_num > 0) &&
		    (!(p_ptr->timed[TMD_BLIND]) || (y == p_ptr->py && x == p_ptr->px)))
		{
			/* Not boring */
			boring = FALSE;

			/* If there is more than one item... */
			if (floor_num > 1) while (1)
			{
				floored = TRUE;

				/* Describe the pile */
				if (p_ptr->wizard)
				{
					strnfmt(out_val, sizeof(out_val),
					        "%s%s%sa pile of %d objects [r,%s] (%d:%d)",
					        s1, s2, s3, floor_num, info, y, x);
				}
				else
				{
					strnfmt(out_val, sizeof(out_val),
					        "%s%s%sa pile of %d objects [r,%s]",
					        s1, s2, s3, floor_num, info);
				}

				prt(out_val, 0, 0);
				move_cursor_relative(y, x);
				query = inkey_ex();

				/* Display objects */
				if (query.key == 'r')
				{
					/* Save screen */
					screen_save();

					/* Display */
					show_floor(floor_list, floor_num, TRUE);

					/* Describe the pile */
					prt(out_val, 0, 0);
					query = inkey_ex();

					/* Load screen */
					screen_load();

					/* Continue on 'r' only */
					if (query.key == 'r') continue;
				}

				/* Done */
				break;
			}
			/* Only one object to display */
			else
			{

				char o_name[80];

				/* Get the single object in the list */
				object_type *o_ptr = &o_list[floor_list[0]];

				/* Not boring */
				boring = FALSE;

				/* Obtain an object description */
				object_desc(o_name, sizeof(o_name), o_ptr, TRUE, ODESC_FULL);

				/* Describe the object */
				if (p_ptr->wizard)
				{
					strnfmt(out_val, sizeof(out_val),
					        "%s%s%s%s [%s] (%d:%d)",
					        s1, s2, s3, o_name, info, y, x);
				}
				else
				{
					strnfmt(out_val, sizeof(out_val),
					        "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
				}

				prt(out_val, 0, 0);
				move_cursor_relative(y, x);
				query = inkey_ex();

				/* Stop on everything but "return"/"space" */
				if ((query.key != '\n') && (query.key != '\r') && (query.key != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query.key == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Plurals */
				if (o_ptr->number != 1) s1 = "They are ";

				/* Preposition */
				s2 = "on ";
			}
		}

		/* Double break */
		if (this_o_idx) break;


		/* Feature (apply "mimic") */
		feat = f_info[cave_feat[y][x]].mimic;

		/* Require knowledge about grid, or ability to see grid */
		if (!(cave_info[y][x] & (CAVE_MARK)) && !player_can_see_bold(y,x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}

		/* Terrain feature if needed */
		if (boring || (feat > FEAT_INVIS))
		{
			cptr name = f_name + f_info[feat].name;

			/* Hack -- handle unknown grids */
			if (feat == FEAT_NONE) name = "unknown grid";

			/* Pick a prefix */
			if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";

			/* Pick proper indefinite article */
			s3 = (is_a_vowel(name[0])) ? "an " : "a ";

			/* Hack -- special introduction for store doors */
			if ((feat >= FEAT_SHOP_HEAD) && (feat <= FEAT_SHOP_TAIL))
			{
				s3 = "the entrance to the ";
			}

			/* Display a message */
			if (p_ptr->wizard)
			{
				strnfmt(out_val, sizeof(out_val),
				        "%s%s%s%s [%s] (%d:%d)", s1, s2, s3, name, info, y, x);
			}
			else
			{
				strnfmt(out_val, sizeof(out_val),
				        "%s%s%s%s [%s]", s1, s2, s3, name, info);
			}

			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey_ex();

			/* Stop on everything but "return"/"space" */
			if ((query.key != '\n') && (query.key != '\r') && (query.key != ' ')) break;
		}

		/* Stop on everything but "return" */
		if ((query.key != '\n') && (query.key != '\r')) break;
	}

	/* Keep going */
	return (query);
}




/*
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
 */
bool target_set_interactive(int mode)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, d, m, t, bd;

	int y = py;
	int x = px;

	bool done = FALSE;

	bool flag = TRUE;

	ui_event_data query;

	char info[80];


	/* Cancel target */
	target_set_monster(0);


	/* Cancel tracking */
	/* health_track(0); */


	/* Prepare the "temp" array */
	target_set_interactive_prepare(mode);

	/* Start near the player */
	m = 0;

	/* Interact */
	while (!done)
	{
		/* Interesting grids */
		if (flag && temp_n)
		{
			y = temp_y[m];
			x = temp_x[m];

			/* Allow target */
			if ((cave_m_idx[y][x] > 0) && target_able(cave_m_idx[y][x]))
			{
				my_strcpy(info, "g,q,t,p,o,+,-,<dir>, <click>", sizeof(info));
			}

			/* Dis-allow target */
			else
			{
				my_strcpy(info, "g,q,p,o,+,-,<dir>, <click>", sizeof(info));
			}

			/* Adjust panel if needed */
			if (adjust_panel(y, x))
			{
				/* Handle stuff */
				handle_stuff();
			}

			/* Describe and Prompt */
			query = target_set_interactive_aux(y, x, mode, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no "direction" */
			d = 0;

			/* Analyze */
			switch (query.key)
			{
				case ESCAPE:
				case 'q':
				{
					done = TRUE;
					break;
				}

				case ' ':
				case '*':
				case '+':
				{
					if (++m == temp_n)
						m = 0;

					break;
				}

				case '-':
				{
					if (m-- == 0)
						m = temp_n - 1;

					break;
				}

				case 'p':
				{
					/* Recenter around player */
					verify_panel();

					/* Handle stuff */
					handle_stuff();

					y = py;
					x = px;
				}

				case 'o':
				{
					flag = FALSE;
					break;
				}

				case 'm':
				{
					break;
				}

				case '\xff':
				{
					x = query.mousex + Term->offset_x;
					y = query.mousey + Term->offset_y;
					target_set_location(y, x);
					done = TRUE;
					break;
				}
				case 't':
				case '5':
				case '0':
				case '.':
				{
					int m_idx = cave_m_idx[y][x];

					if ((m_idx > 0) && target_able(m_idx))
					{
						health_track(m_idx);
						target_set_monster(m_idx);
						done = TRUE;
					}
					else
					{
						bell("Illegal target!");
					}
					break;
				}

				case 'g':
				{
					do_cmd_pathfind(y, x);
					done = TRUE;
					break;
				}

				default:
				{
					/* Extract direction */
					d = target_dir(query.key);

					/* Oops */
					if (!d) bell("Illegal command for target mode!");

					break;
				}
			}

			/* Hack -- move around */
			if (d)
			{
				int old_y = temp_y[m];
				int old_x = temp_x[m];

				/* Find a new monster */
				i = target_pick(old_y, old_x, ddy[d], ddx[d]);

				/* Scroll to find interesting grid */
				if (i < 0)
				{
					int old_wy = Term->offset_y;
					int old_wx = Term->offset_x;

					/* Change if legal */
					if (change_panel(d))
					{
						/* Recalculate interesting grids */
						target_set_interactive_prepare(mode);

						/* Find a new monster */
						i = target_pick(old_y, old_x, ddy[d], ddx[d]);

						/* Restore panel if needed */
						if ((i < 0) && modify_panel(Term, old_wy, old_wx))
						{
							/* Recalculate interesting grids */
							target_set_interactive_prepare(mode);
						}

						/* Handle stuff */
						handle_stuff();
					}
				}

				/* Use interesting grid if found */
				if (i >= 0) m = i;
			}
		}

		/* Arbitrary grids */
		else
		{
			/* Default prompt */
			my_strcpy(info, "g,q,t,p,m,+,-,<dir>, <click>", sizeof(info));

			/* Describe and Prompt (enable "TARGET_LOOK") */
			query = target_set_interactive_aux(y, x, mode | TARGET_LOOK, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no direction */
			d = 0;

			/* Analyze the keypress */
			switch (query.key)
			{
				case ESCAPE:
				case 'q':
				{
					done = TRUE;
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
					handle_stuff();

					y = py;
					x = px;
				}

				case 'o':
				{
					break;
				}

				case 'm':
				{
					flag = TRUE;

					m = 0;
					bd = 999;

					/* Pick a nearby monster */
					for (i = 0; i < temp_n; i++)
					{
						t = distance(y, x, temp_y[i], temp_x[i]);

						/* Pick closest */
						if (t < bd)
						{
							m = i;
							bd = t;
						}
					}

					/* Nothing interesting */
					if (bd == 999) flag = FALSE;

					break;
				}

				case '\xff':
				{
					x = query.mousex + Term->offset_x;
					y = query.mousey + Term->offset_y;
				}
				case 't':
				case '5':
				case '0':
				case '.':
				{
					target_set_location(y, x);
					done = TRUE;
					break;
				}

				case 'g':
				{
					do_cmd_pathfind(y,x);
					done = TRUE;
					break;
				}

				default:
				{
					/* Extract a direction */
					d = target_dir(query.key);

					/* Oops */
					if (!d) bell("Illegal command for target mode!");

					break;
				}
			}

			/* Handle "direction" */
			if (d)
			{
				int dungeon_hgt = (p_ptr->depth == 0) ? TOWN_HGT : DUNGEON_HGT;
				int dungeon_wid = (p_ptr->depth == 0) ? TOWN_WID : DUNGEON_WID;

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
				if (adjust_panel(y, x))
				{
					/* Handle stuff */
					handle_stuff();

					/* Recalculate interesting grids */
					target_set_interactive_prepare(mode);
				}
			}
		}
	}

	/* Forget */
	temp_n = 0;

	/* Clear the top line */
	prt("", 0, 0);

	/* Recenter around player */
	verify_panel();

	/* Handle stuff */
	handle_stuff();

	/* Failure to set target */
	if (!target_set) return (FALSE);

	/* Success */
	return (TRUE);
}


/**
 * Obtains the location the player currently targets.
 *
 * Both `col` and `row` must point somewhere, and on function termination,
 * contain the X and Y locations respectively.
 */
void target_get(s16b *col, s16b *row)
{
	assert(col);
	assert(row);

	*col = target_x;
	*row = target_y;
}


/**
 * Returns the currently targeted monster index.
 */
s16b target_get_monster(void)
{
	return target_who;
}

