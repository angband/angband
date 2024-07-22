/**
 * \file borg-flow-glyph.c
 * \brief Flow (move) to glyphs
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "borg-flow-glyph.h"

#ifdef ALLOW_BORG

#include "borg-cave.h"
#include "borg-flow-misc.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"

/*
 * Hack -- Glyph creating
 */

static uint8_t glyph_x;
static uint8_t glyph_y;
static uint8_t glyph_y_center = 0;
static uint8_t glyph_x_center = 0;

/*
 * Track glyphs
 */
struct borg_track track_glyph;

/*
 * Environment changed.  Need to make a new Sea of Runes for Morgy
 */
bool borg_needs_new_sea;

/*
 * Prepare to flow towards a location and create a
 * special glyph of warding pattern.
 *
 * The borg will look for a room that is at least 7x7.
 * ##########
 * #3.......#
 * #2.xxxxx.#
 * #1.xxxxx.#
 * #0.xx@xx.#
 * #1.xxxxx.#
 * #2.xxxxx.#
 * #3.......#
 * # 3210123#
 * ##########
 * and when he locates one, he will attempt to:
 * 1. flow to a central location and
 * 2. begin planting Runes in a pattern. When complete,
 * 3. move to the center of it.
 */
/*
 * ghijk  The borg will use the following ddx and ddy to search
 * d827a  for a suitable grid in an open room.
 * e4@3b
 * f615c
 * lmnop  24 grids
 *
 */
bool borg_flow_glyph(int why)
{
    int i;
    int cost;

    int x, y;
    int v          = 0;

    int b_x        = borg.c.x;
    int b_y        = borg.c.y;
    int b_v        = -1;
    int goal_glyph = 0;
    int glyph      = 0;

    borg_grid *ag;

    if ((glyph_y_center == 0 && glyph_x_center == 0)
        || distance(borg.c, loc(glyph_x_center, glyph_y_center)) >= 50) {
        borg_needs_new_sea = true;
    }

    /* We have arrived */
    if ((glyph_x == borg.c.x) && (glyph_y == borg.c.y)) {
        /* Cancel */
        glyph_x = 0;
        glyph_y = 0;

        /* Store the center of the glyphs */
        if (borg_needs_new_sea) {
            glyph_y_center = borg.c.y;
            glyph_x_center = borg.c.x;
        }

        borg_needs_new_sea = false;

        /* Take note */
        borg_note(format("# Glyph Creating at (%d,%d)", borg.c.x, borg.c.y));

        /* Create the Glyph */
        if (borg_spell_fail(GLYPH_OF_WARDING, 30)
            || borg_read_scroll(sv_scroll_rune_of_protection)
            || borg_activate_item(act_glyph)) {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph.num; i++) {
                /* Stop if we already new about this glyph */
                if ((track_glyph.x[i] == borg.c.x)
                    && (track_glyph.y[i] == borg.c.y))
                    return false;
            }

            /* Track the newly discovered glyph */
            if (track_glyph.num < track_glyph.size) {
                borg_note("# Noting the creation of a glyph.");
                track_glyph.x[track_glyph.num] = borg.c.x;
                track_glyph.y[track_glyph.num] = borg.c.y;
                track_glyph.num++;
            }

            /* Success */
            return true;
        }

        /* Nope */
        return false;
    }

    /* Reverse flow */
    borg_flow_reverse(250, true, false, false, -1, false);

    /* Scan the entire map */
    for (y = 15; y < AUTO_MAX_Y - 15; y++) {
        for (x = 50; x < AUTO_MAX_X - 50; x++) {
            borg_grid *ag_ptr[24];

            int floor     = 0;
            int tmp_glyph = 0;

            /* Acquire the grid */
            ag = &borg_grids[y][x];

            /* Skip every non floor/glyph */
            if (ag->feat != FEAT_FLOOR && ag->glyph)
                continue;

            /* Acquire the cost */
            cost = borg_data_cost->data[y][x];

            /* Skip grids that are really far away.  He probably
             * won't be able to safely get there
             */
            if (cost >= 75)
                continue;

            /* Extract adjacent locations to each considered grid */
            for (i = 0; i < 24; i++) {
                /* Extract the location */
                int xx = x + borg_ddx_ddd[i];
                int yy = y + borg_ddy_ddd[i];

                /* Get the grid contents */
                ag_ptr[i] = &borg_grids[yy][xx];
            }

            /* Center Grid */
            if (borg_needs_new_sea) {
                goal_glyph = 24;

                /* Count Adjacent Flooors */
                for (i = 0; i < 24; i++) {
                    ag = ag_ptr[i];
                    if (ag->feat == FEAT_FLOOR || ag->glyph)
                        floor++;
                }

                /* Not a good location if not the center of the sea */
                if (floor != 24) {
                    continue;
                }

                /* Count floors already glyphed */
                for (i = 0; i < 24; i++) {
                    ag = ag_ptr[i];

                    /* Glyphs */
                    if (ag->glyph) {
                        tmp_glyph++;
                    }
                }

                /* Tweak -- Reward certain floors, punish distance */
                v = 100 + (tmp_glyph * 500) - (cost * 1);
                if (borg_grids[y][x].feat == FEAT_FLOOR)
                    v += 3000;

                /* If this grid is surrounded by glyphs, select it */
                if (tmp_glyph == goal_glyph)
                    v += 5000;

                /* If this grid is already glyphed but not
                 * surrounded by glyphs, then choose another.
                 */
                if (tmp_glyph != goal_glyph && borg_grids[y][x].glyph)
                    v = -1;

                /* The grid is not searchable */
                if (v <= 0)
                    continue;

                /* Track "best" grid */
                if ((b_v >= 0) && (v < b_v))
                    continue;

                /* Save the data */
                b_v = v;
                b_x = x;
                b_y = y;
            }
            /* old center, making outlying glyphs, */
            else {
                /* Count Adjacent Flooors */
                for (i = 0; i < 24; i++) {
                    /* Leave if this grid is not in good array */
                    if (glyph_x_center + borg_ddx_ddd[i] != x)
                        continue;
                    if (glyph_y_center + borg_ddy_ddd[i] != y)
                        continue;

                    /* Already got a glyph on it */
                    if (borg_grids[y][x].glyph)
                        continue;

                    /* Tweak -- Reward certain floors, punish distance */
                    v = 500 + (tmp_glyph * 500) - (cost * 1);

                    /* The grid is not searchable */
                    if (v <= 0)
                        continue;

                    /* Track "best" grid */
                    if ((b_v >= 0) && (v < b_v))
                        continue;

                    /* Save the data */
                    b_v = v;
                    b_x = x;
                    b_y = y;
                }
            }
        }
    }

    /* Extract adjacent locations to each considered grid */
    if (glyph_y_center != 0 && glyph_x_center != 0) {

        for (i = 0; i < 24; i++) {
            /* Extract the location */
            int xx = glyph_x_center + borg_ddx_ddd[i];
            int yy = glyph_y_center + borg_ddy_ddd[i];

            borg_grid *ag_ptr[24];

            /* Get the grid contents */
            ag_ptr[i] = &borg_grids[yy][xx];
            ag        = ag_ptr[i];

            /* If it is not a glyph, skip it */
            if (ag->glyph)
                glyph++;

            /* Save the data */
            if (glyph == 24) {
                b_v = 5000;
                b_x = glyph_x_center;
                b_y = glyph_y_center;
            }
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (b_v < 0)
        return false;

    /* Access grid */
    ag = &borg_grids[b_y][b_x];

    /* Memorize */
    glyph_x = b_x;
    glyph_y = b_y;

    /* Enqueue the grid */
    borg_flow_enqueue_grid(b_y, b_x);

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Glyph", GOAL_MISC))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_MISC))
        return false;

    /* Success */
    return true;
}

void borg_init_flow_glyph(void)
{
    /* Track mineral veins with treasure. */
    borg_init_track(&track_glyph, 200);
}

void borg_free_flow_glyph(void) { borg_free_track(&track_glyph); }

#endif
