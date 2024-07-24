/**
 * \file borg-update.c
 * \brief Update the borgs values based on the current frame
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

#include "borg-update.h"

#ifdef ALLOW_BORG

#include "../cave.h"
#include "../trap.h"
#include "../ui-term.h"

#include "borg-cave-light.h"
#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-fight-attack.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow-take.h"
#include "borg-flow.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-wear.h"
#include "borg-junk.h"
#include "borg-messages.h"
#include "borg-prepared.h"
#include "borg-projection.h"
#include "borg-store-buy.h"
#include "borg-store-sell.h"
#include "borg-store.h"
#include "borg-think-dungeon.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Hack -- monster/object tracking grids
 */
typedef struct borg_wank borg_wank;

struct borg_wank {
    uint8_t x;
    uint8_t y;

    uint8_t t_a;
    wchar_t t_c;

    bool is_take;
    bool is_kill;
};

/*
 * Hack -- object/monster tracking array
 */
static int        borg_wank_num = 0;
static borg_wank *borg_wanks;

bool borg_failure; /* Notice failure */

/*
 * Hack -- the detection arrays
 */
bool borg_detect_wall[6][18];
bool borg_detect_trap[6][18];
bool borg_detect_door[6][18];
bool borg_detect_evil[6][18];
bool borg_detect_obj[6][18];

#if 0
/*
 * Maintain a temporary set of grids
 * Used to store lit grid info
 */
int16_t borg_temp_lit_n = 0;
uint8_t borg_temp_lit_x[AUTO_TEMP_MAX];
uint8_t borg_temp_lit_y[AUTO_TEMP_MAX];
#endif

/*
 * Strategy flags -- recalculate things
 */
bool borg_do_update_view = false; /* Recalculate view */
bool borg_do_update_lite = false; /* Recalculate lite */

/*
 * Old values
 */
/* Old panel */
static int o_w_x = -1;
static int o_w_y = -1;

/* Old location */
static struct loc old_c = { -1, -1 };

/*
 * Update the Borg based on the current "map"
 */
static void borg_forget_map(void)
{
    int x, y;

    borg_grid *ag;

    /* Clean up the grids */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {
            /* Access the grid */
            ag = &borg_grids[y][x];

            /* Wipe it */
            memset(ag, 0, sizeof(borg_grid));

            /* Lay down the outer walls */
            ag->feat = FEAT_PERM;
        }
    }

    /* Clean up the grids */
    for (y = 1; y < AUTO_MAX_Y - 1; y++) {
        for (x = 1; x < AUTO_MAX_X - 1; x++) {
            /* Access the grid */
            ag = &borg_grids[y][x];

            /* Forget the contents */
            ag->feat = FEAT_NONE;

            /* Hack -- prepare the town */
            if (!borg.trait[BI_CDEPTH])
                ag->feat = FEAT_FLOOR;
        }
    }

    /* Reset "borg_data_cost" */
    memcpy(borg_data_cost, borg_data_hard, sizeof(borg_data));

    /* Reset "borg_data_flow" */
    memcpy(borg_data_flow, borg_data_hard, sizeof(borg_data));

    /* Clear "borg_data_know" */
    memset(borg_data_know, 0, sizeof(borg_data));

    /* Clear "borg_data_icky" */
    memset(borg_data_icky, 0, sizeof(borg_data));

    /* Forget the view */
    borg_forget_view();
}

/*
 * Update the "map" based on visual info on the screen
 *
 * Note that we make assumptions about the grid under the player,
 * to prevent painful situations such as seeming to be standing
 * in a wall, or on a trap, etc.
 *
 * In general, we use the same "feat" codes as the game itself, but
 * sometimes we are just guessing (as with "visible traps"), and we
 * use some special codes, explained below.
 *
 * Note that we use the "feat" code of "FEAT_NONE" for grids which
 * have never been seen, or which, when seen, have always contained
 * an object or monster.  These grids are probably walls, unless
 * they contain a monster or object, in which case they are probably
 * floors, unless they contain a monster which passes through walls,
 * in which case they are probably walls.
 *
 * Note that we use the "feat" code of "FEAT_FLOOR" for grids which
 * were a normal floor last time we checked.  These grids may have
 * changed into non-floor grids recently (via earthquake?), unless
 * the grid is on the current panel, and is currently "lit" in some
 * manner, and does not contain a monster.
 *
 * Note that we use the other "feat" codes for grids which probably
 * contain the given feature type, unless several feature types use
 * the same symbol, in which case we use some "default" code, changing
 * our guess when messages provide us with more information.  This is
 * especially necessary for distinguishing magma from quartz, and for
 * distinguishing normal doors from locked doors from jammed doors.
 * Note that most "feat" codes, even if they are not "guesses", may
 * not be valid unless the grid is on the current panel.
 *
 * We use the "BORG_MARK" flag to mark a grid as having been "observed",
 * though this may or may not indicate that the "feature" code is known,
 * since observations of monsters or objects via telepathy and/or remote
 * detection may trigger this flag.
 *
 * We use the "BORG_OKAY" flag to mark a grid as being on the current
 * panel, which is used for various things, including verifying that
 * a grid can be used as the destination of a target, and to allow us
 * to assume that off-panel monsters are not "visible".
 *
 * Note the "interesting" code used to learn which floor grids are "dark"
 * and which are "perma-lit", by tracking those floor grids which appear
 * to be "lit", and then marking all of these grids which do not appear
 * to be lit by the torch as "known" to be illuminated, and by marking
 * any grids which "disappear" or which are displayed as "dark floors"
 * as "known" to be "dark".  This leaves many grids, especially those
 * lit by the torch, as being neither lit nor dark.
 *
 * The basic problem is that, especially with no special options set,
 * the player has very little direct information about which grids
 * are perma-lit, since all non-floor grids are memorized when they
 * are seen, and torch-lit floor grids look just like perma-lit
 * floor grids.  Also, monsters hide any object or feature in their
 * grid, and objects hide any feature in their grid, and objects are
 * memorized when they are seen, and monsters can be detected by a
 * variety of methods, including infravision and telepathy.
 *
 * So we ignore most non-floor grids, and we mark any floor grids which
 * are "known" to be perma-lit as "BORG_GLOW", and any which are "known"
 * to be dark as "BORG_DARK".  These flags are used for many purposes,
 * most importantly, to determine when "call lite" would be useful, and
 * to help determine when a monster is standing in a viewable perma-lit
 * grid, and should thus be "visible", and to determine when the player
 * has "lite", even though his torch has gone out.
 *
 * When a "call lite" spell succeeds, we mark the grids around the
 * player as "BORG_GLOW" and not "BORG_DARK", but we do not attempt
 * to "spread" the lite to the entire room, since, in general, it is
 * not possible to know what grids are in the "room".
 *
 * Note that we assume that normally, when the player steps onto
 * something, it disappears, and turns into a normal floor, unless
 * the player is stepping onto a grid which is normally "permanent"
 * (floors, stairs, store doors), in which case it does not change.
 *
 * Note that when we encounter a grid which blocks motion, but which
 * was previously thought to not block motion, we must be sure to
 * remove it from any "flow" which might be in progress, to prevent
 * nasty situations in which we attempt to flow into a wall grid
 * which was thought to be something else, like an unknown grid.
 *
 */
static void borg_update_map(void)
{
    int i, x, y, dx, dy;

    borg_grid       *ag;
    struct grid_data g;

    /* Analyze the current map panel */
    for (dy = 0; dy < SCREEN_HGT; dy++) {
        /* Scan the row */
        for (dx = 0; dx < SCREEN_WID; dx++) {
            bool old_wall;
            bool new_wall;

            /* Obtain the map location */
            x = w_x + dx;
            y = w_y + dy;

            /* Cheat the exact information from the screen */
            struct loc l = loc(x, y);
            /* since the map is now dynamically sized, double check we are in
             * bounds */
            if (!square_in_bounds(cave, l))
                continue;
            map_info(l, &g);

            /* Get the borg_grid */
            ag = &borg_grids[y][x];

            /* Notice "on-screen" */
            ag->info |= BORG_OKAY;

            /* Notice "knowledge" */
            /* if this square is not in view and the borg previously */
            /* cast stone to mud here, ignore the map info so repeated */
            /* stone to mud aren't cast */
            if (g.f_idx != FEAT_NONE
                && (g.in_view || !(ag->info & BORG_IGNORE_MAP))) {
                if (g.in_view) {
                    ag->info &= ~BORG_IGNORE_MAP;
                }
                ag->info |= BORG_MARK;
                ag->feat = g.f_idx;
            }

            /* default store to - 1 */
            ag->store = -1;

            /* Notice the player */
            if (g.is_player) {
                /* Memorize player location */
                borg.c.x = x;
                borg.c.y = y;
            }

            /* Save the old "wall" or "door" */
            old_wall = !borg_cave_floor_grid(ag);

            /* Analyze know information about grid */
            /* Shop Doors */
            if (feat_is_shop(g.f_idx)) {
                /* Shop type */
                ag->feat  = g.f_idx;

                i         = square_shopnum(cave, l);
                ag->store = i;

                /* Save new information */
                track_shop_x[i] = x;
                track_shop_y[i] = y;

            } else if (square_isdisarmabletrap(cave, l)) {
                /* Minor cheat for the borg.  If the borg is running
                 * in the graphics mode (not the AdamBolt Tiles) he will
                 * mis-id the glyph of warding as a trap
                 */
                ag->trap      = true;
                uint8_t t_idx = square(cave, l)->trap->t_idx;
                if (trf_has(trap_info[t_idx].flags, TRF_GLYPH)) {
                    ag->glyph = true;
                    /* Check for an existing glyph */
                    for (i = 0; i < track_glyph.num; i++) {
                        /* Stop if we already new about this glyph */
                        if ((track_glyph.x[i] == x) && (track_glyph.y[i] == y))
                            break;
                    }

                    /* Track the newly discovered glyph */
                    if ((i == track_glyph.num) && (i < track_glyph.size)) {
                        track_glyph.x[i] = x;
                        track_glyph.y[i] = y;
                        track_glyph.num++;
                    }
                }
            }
            /* Darkness */
            else if (g.f_idx == FEAT_NONE) {
                /* The grid is not lit */
                ag->info &= ~BORG_GLOW;

                /* Known grids must be dark floors */
                if (ag->feat != FEAT_NONE)
                    ag->info |= BORG_DARK;
            }
            /* Floors */
            else if (g.f_idx == FEAT_NONE) {
                /* Handle "blind" */
                if (borg.trait[BI_ISBLIND]) {
                    /* Nothing */
                }

                /* Handle "dark" floors */
                if (g.lighting == LIGHTING_DARK) {
                    /* Dark floor grid */
                    ag->info |= BORG_DARK;
                    ag->info &= ~BORG_GLOW;
                }

                /* Handle Glowing floors */
                else if (g.lighting == LIGHTING_LIT) {
                    /* Perma Glowing Grid */
                    ag->info |= BORG_GLOW;

                    /* Assume not dark */
                    ag->info &= ~BORG_DARK;
                }

                /* torch-lit or line of sight grids */
                else {
                    ag->info |= BORG_LIGHT;

                    /* Assume not dark */
                    ag->info &= ~BORG_DARK;
                }
            }
            /* Open doors */
            else if (g.f_idx == FEAT_OPEN || g.f_idx == FEAT_BROKEN) {
            }
            /* Walls */
            else if (g.f_idx == FEAT_GRANITE || g.f_idx == FEAT_PERM) {
                /* ok this is a humongo cheat.  He is pulling the
                 * grid information from the game rather than from
                 * his memory.  He is going to see if the wall is perm.
                 * This is a cheat. May the Lord have mercy on my soul.
                 *
                 * The only other option is to have him "dig" on each
                 * and every granite wall to see if it is perm.  Then he
                 * can mark it as a non-perm.  However, he would only have
                 * to dig once and only in a range of spaces near the
                 * center of the map.  Since perma-walls are located in
                 * vaults and vaults have a minimum size.  So he can avoid
                 * digging on walls that are, say, 10 spaces from the edge
                 * of the map.  He can also limit the dig by his depth.
                 * Vaults are found below certain levels and with certain
                 * "feelings."  Can be told not to dig on boring levels
                 * and not before level 50 or whatever.
                 *
                 * Since the code to dig slows the borg down a lot.
                 * (Found in borg6.c in _flow_dark_interesting()) We will
                 * limit his capacity to search.  We will set a flag on
                 * the level is perma grids are found.
                 */
                /* is it a perma grid?  Only counts for being a vault if not in
                 * town */
                /* and not on edge of map */
                if (ag->feat == FEAT_PERM && borg.trait[BI_CDEPTH] && x && y
                    && x != (cave->width - 1) && y != (cave->height - 1)) {
                    vault_on_level = true;
                }
            }
            /* lava */
            else if (g.f_idx == FEAT_LAVA) {
            }
            /* Seams */
            else if (g.f_idx == FEAT_MAGMA || g.f_idx == FEAT_QUARTZ) {
                /* Done */
            }
            /* Hidden */
            else if (g.f_idx == FEAT_MAGMA_K || g.f_idx == FEAT_QUARTZ_K) {
                /* Check for an existing vein */
                for (i = 0; i < track_vein.num; i++) {
                    /* Stop if we already new about this */
                    if ((track_vein.x[i] == x) && (track_vein.y[i] == y))
                        break;
                }

                /* Track the newly discovered vein */
                if ((i == track_vein.num) && (i < track_vein.size)) {
                    track_vein.x[i] = x;
                    track_vein.y[i] = y;
                    track_vein.num++;

                    /* do not overflow */
                    if (track_vein.num > 99)
                        track_vein.num = 99;
                }
            }
            /* Rubble */
            else if (g.f_idx == FEAT_RUBBLE) {
                /* Done */
            }
            /* Doors */
            else if (g.f_idx == FEAT_CLOSED) {
                /* Only while low level */
                if (borg.trait[BI_CLEVEL] <= 5) {
                    /* Check for an existing door */
                    for (i = 0; i < track_closed.num; i++) {
                        /* Stop if we already new about this door */
                        if ((track_closed.x[i] == x)
                            && (track_closed.y[i] == y))
                            break;
                    }

                    /* Track the newly discovered door */
                    if ((i == track_closed.num) && (i < track_closed.size)) {
                        track_closed.x[i] = x;
                        track_closed.y[i] = y;
                        track_closed.num++;

                        /* do not overflow */
                        if (track_closed.num > 254)
                            track_closed.num = 254;
                    }
                }
            }
            /* Up stairs */
            else if (g.f_idx == FEAT_LESS) {
                /* Check for an existing "up stairs" */
                for (i = 0; i < track_less.num; i++) {
                    /* Stop if we already new about these stairs */
                    if ((track_less.x[i] == x) && (track_less.y[i] == y))
                        break;
                }

                /* Track the newly discovered "up stairs" */
                if ((i == track_less.num) && (i < track_less.size)) {
                    track_less.x[i] = x;
                    track_less.y[i] = y;
                    track_less.num++;
                }
            }
            /* Down stairs */
            else if (g.f_idx == FEAT_MORE) {
                /* Check for an existing "down stairs" */
                for (i = 0; i < track_more.num; i++) {
                    /* We already knew about that one */
                    if ((track_more.x[i] == x) && (track_more.y[i] == y))
                        break;
                }

                /* Track the newly discovered "down stairs" */
                if ((i == track_more.num) && (i < track_more.size)) {
                    track_more.x[i] = x;
                    track_more.y[i] = y;
                    track_more.num++;
                }
            }

            if (ag->feat == FEAT_FLOOR && square_iswebbed(cave, l)) {
                ag->web = true;
            } else
                ag->web = false;          

            /* Now do non-feature stuff */
            if ((g.first_kind || g.m_idx) && !borg.trait[BI_ISIMAGE]) {
                /* Monsters/Objects */
                borg_wank *wank;

                /* Check for memory overflow */
                if (borg_wank_num == AUTO_VIEW_MAX) {
                    borg_note(format("# Wank problem at grid (%d,%d) m:%d "
                                     "o:%d, borg at (%d,%d)",
                        y, x, g.m_idx, g.first_kind ? g.first_kind->kidx : 0,
                        borg.c.y, borg.c.x));
                    borg_oops("too many objects...");
                }

                /* Access next wank, advance */
                wank = &borg_wanks[borg_wank_num++];

                /* Save some information */
                wank->x = x;
                wank->y = y;
                /* monster symbol takes priority */
                /* TODO: Store known information about monster/object, instead
                 * of just the screen character */
                if (g.m_idx) {
                    struct monster *m_ptr = cave_monster(cave, g.m_idx);
                    wank->t_a             = m_ptr->attr;
                    wank->t_c             = r_info[m_ptr->race->ridx].d_char;
                } else {
                    wank->t_a = g.first_kind->d_attr;
                    wank->t_c = g.first_kind->d_char;
                }
                wank->is_take = (g.first_kind != NULL);
                wank->is_kill = (g.m_idx != 0);
            }

            /* Save the new "wall" or "door" */
            new_wall = !borg_cave_floor_grid(ag);

            /* Notice wall changes */
            if (old_wall != new_wall) {
                /* Remove this grid from any flow */
                if (new_wall)
                    borg_data_flow->data[y][x] = 255;

                /* Remove this grid from any flow */
                borg_data_know->data[y][x] = false;

                /* Remove this grid from any flow */
                borg_data_icky->data[y][x] = false;

                /* Recalculate the view (if needed) */
                if (ag->info & BORG_VIEW)
                    borg_do_update_view = true;

                /* Recalculate the lite (if needed) */
                if (ag->info & BORG_LIGHT)
                    borg_do_update_lite = true;
            }
        }
    }
}

/*
 * Increase the "grid danger" from lots of monsters
 *   ###################
 *   #54433333333333445#  Each monster gives some danger.
 *   #54433222222233445#  The danger decreases as you move out.
 *   #54433222222233445#  There is no danger if the monster
 *   #54433221112233445#  does not have LOS to the grid.
 *   #54433221@12233445#
 *   #54433221112233445#
 *   #54433222222233445#
 *   #54433222222233445#
 *   #54433333333333445#
 *   ###################
 */
static void borg_fear_grid(
    char *who, int y, int x, int k) /* 8-8, this was uint */
{
    int        x1 = 0, y1 = 0;
    borg_kill *kill;
    borg_grid *ag;

    /* Not in town */
    if (borg.trait[BI_CDEPTH] == 0)
        return;

    /* In a Sea of Runes, no worry */
    if (borg_morgoth_position || borg_as_position)
        return;

    /* Do not add fear in a vault -- Cheating the cave info */
    if (square_isvault(cave, loc(x, y)))
        return;

    /* Access the grid info */
    ag   = &borg_grids[y][x];
    kill = &borg_kills[ag->kill];

    /* Level 50 borgs have greatly reduced Monster Fear */
    if (borg.trait[BI_CLEVEL] == 50)
        k = k * 5 / 10;

    /* Collect "fear", spread around */
    for (x1 = -6; x1 <= 6; x1++) {
        for (y1 = -6; y1 <= 6; y1++) {
            /* careful */
            if (x + x1 <= 0 || x1 + x >= AUTO_MAX_X)
                continue;
            if (y + y1 <= 0 || y1 + y >= AUTO_MAX_Y)
                continue;

            /* Very Weak Fear at this range */
            if (borg_los(kill->pos.y, kill->pos.x, y + y1, x + x1))
                borg_fear_monsters[y + y1][x + x1] += (k / 8);

            /* Next range set */
            if (x1 <= -5 || x1 >= 5)
                continue;
            if (y1 <= -5 || y1 >= 5)
                continue;

            /* Weak Fear at this range */
            if (borg_los(kill->pos.y, kill->pos.x, y + y1, x + x1))
                borg_fear_monsters[y + y1][x + x1] += (k / 5);

            /* Next range set */
            if (x1 <= -3 || x1 >= 3)
                continue;
            if (y1 <= -3 || y1 >= 3)
                continue;

            /* Fear at this range */
            if (borg_los(kill->pos.y, kill->pos.x, y + y1, x + x1))
                borg_fear_monsters[y + y1][x + x1] += (k / 3);

            /* Next range set */
            if (x1 <= -2 || x1 >= 2)
                continue;
            if (y1 <= -2 || y1 >= 2)
                continue;

            /* Mild Fear at this range */
            if (borg_los(kill->pos.y, kill->pos.x, y + y1, x + x1))
                borg_fear_monsters[y + y1][x + x1] += (k / 2);

            /* Next range set */
            if (x1 <= -1 || x1 >= 1)
                continue;
            if (y1 <= -1 || y1 >= 1)
                continue;

            /* Full fear close to this monster */
            if (borg_los(kill->pos.y, kill->pos.x, y + y1, x + x1))
                borg_fear_monsters[y + y1][x + x1] += k;
        }
    }
}

/*
 * Increase the "region danger"
 * This is applied when the borg cannot find the source of a message.  He
 * assumes it is an invisible monster.  This will keep him from resting while
 * unseen guys attack him.
 */
static void borg_fear_regional(
    char *who, int y, int x, int k, bool seen_guy) /* 8-8 , had been uint */
{
    int x0, y0, x1, x2, y1, y2;

    /* Do not add fear in a vault -- Cheating the cave info */
    if (square_isvault(cave, loc(x, y)))
        return;

    /* Messages */
    if (seen_guy) {
        borg_note(format("# Fearing region value %d.", k));
    } else {
        borg_note(
            format("# Fearing region (%d,%d) value %d because of a non-LOS %s",
                y, x, k, who));
        borg.need_shift_panel = true;
    }

    /* Current region */
    y0 = (y / 11);
    x0 = (x / 11);

    /* Nearby regions */
    y1 = (y0 > 0) ? (y0 - 1) : 0;
    x1 = (x0 > 0) ? (x0 - 1) : 0;
    y2 = (x0 < 5) ? (x0 + 1) : 5;
    x2 = (x0 < 17) ? (x0 + 1) : 17;

    /* Collect "fear", spread around */
    borg_fear_region[y0][x0] += k;
    borg_fear_region[y0][x1] += k;
    borg_fear_region[y0][x2] += k;
    borg_fear_region[y1][x0] += k / 2;
    borg_fear_region[y2][x0] += k / 2;
    borg_fear_region[y1][x1] += k / 2;
    borg_fear_region[y1][x2] += k / 3;
    borg_fear_region[y2][x1] += k / 3;
    borg_fear_region[y2][x2] += k / 3;
}

/*
 * Calculate base danger from a spell attack by an invisible monster
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 */
static int borg_fear_spell(int i)
{
    int z    = 0;
    int p    = 0;

    int ouch = 0;

    /* Damage taken */
    if (borg.oldchp > borg.trait[BI_CURHP])
        ouch = (borg.oldchp - borg.trait[BI_CURHP]) * 2;

    /* Check the spell */
    switch (i) {
    case RSF_SHRIEK:
        p += 10;

        /* If low level borg.  Get off the level now. */
        if (borg.trait[BI_CLEVEL] <= 5)
            borg.goal.fleeing = borg.goal.leaving = true;
        break;

    case RSF_WHIP:
        p = 100;
        break;

    case RSF_SPIT:
        break;

    case RSF_SHOT:
        z = ouch;
        break;

    case RSF_ARROW:
        z = ouch;
        break;

    case RSF_BOLT:
        z = ouch;
        break;

    case RSF_BR_ACID:
        if (borg.trait[BI_IACID])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BR_ELEC:
        if (borg.trait[BI_IELEC])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BR_FIRE:
        if (borg.trait[BI_IFIRE])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BR_COLD:
        if (borg.trait[BI_ICOLD])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BR_POIS:
        z = ouch;
        if (borg.trait[BI_RPOIS])
            break;
        if (borg.temp.res_pois)
            break;
        p += 20;
        break;

    case RSF_BR_NETH:
        z = ouch + 100;
        if (borg.trait[BI_RNTHR])
            break;
        p += 50;
        if (borg.trait[BI_HLIFE])
            break;
        /* do not worry about drain exp after level 50 */
        if (borg.trait[BI_CLEVEL] >= 50)
            break;
        p += 150;
        break;

    case RSF_BR_LIGHT:
        z = ouch;
        if (borg.trait[BI_RLITE])
            break;
        if (borg.trait[BI_RBLIND])
            break;
        p += 20;
        break;

    case RSF_BR_DARK:
        z = ouch;
        if (borg.trait[BI_RDARK])
            break;
        if (borg.trait[BI_RBLIND])
            break;
        p += 20;
        break;

    case RSF_BR_SOUN:
        z = ouch;
        if (borg.trait[BI_RSND])
            break;
        p += 50;
        break;

    case RSF_BR_CHAO:
        z = ouch;
        if (borg.trait[BI_RKAOS])
            break;
        p += 200;
        if (!borg.trait[BI_RNTHR])
            p += 50;
        if (!borg.trait[BI_HLIFE])
            p += 50;
        if (!borg.trait[BI_RCONF])
            p += 50;
        if (borg.trait[BI_CLEVEL] == 50)
            break;
        p += 100;
        break;

    case RSF_BR_DISE:
        z = ouch;
        if (borg.trait[BI_RDIS])
            break;
        p += 500;
        break;

    case RSF_BR_NEXU:
        z = ouch;
        if (borg.trait[BI_RNXUS])
            break;
        p += 100;
        break;

    case RSF_BR_TIME:
        z = ouch;
        p += 200;
        break;

    case RSF_BR_INER:
        z = ouch;
        p += 50;
        break;

    case RSF_BR_GRAV:
        z = ouch;
        p += 50;
        if (borg.trait[BI_RSND])
            break;
        p += 50;
        break;

    case RSF_BR_SHAR:
        z = ouch;
        if (borg.trait[BI_RSHRD])
            break;
        p += 50;
        break;

    case RSF_BR_PLAS:
        z = ouch;
        if (borg.trait[BI_RSND])
            break;
        p += 50;
        break;

    case RSF_BR_WALL:
        z = ouch;
        if (borg.trait[BI_RSND])
            break;
        p += 50;
        break;

    case RSF_BR_MANA:
        /* Nothing currently breaths mana but best to have a check */
        p += 50;
        break;

    case RSF_BOULDER:
        z = ouch;
        p += 40;
        break;

    case RSF_WEAVE:
        /* annoying */
        break;

    case RSF_BA_ACID:
        if (borg.trait[BI_IACID])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BA_ELEC:
        if (borg.trait[BI_IELEC])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BA_FIRE:
        if (borg.trait[BI_IFIRE])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BA_COLD:
        if (borg.trait[BI_ICOLD])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BA_POIS:
        z = ouch;
        if (borg.trait[BI_RPOIS])
            break;
        p += 20;
        break;

    case RSF_BA_SHAR:
        z = ouch;
        if (borg.trait[BI_RSHRD])
            break;
        p += 20;
        break;

    case RSF_BA_NETH:
        z = ouch + 100;
        if (borg.trait[BI_RNTHR])
            break;
        p += 300;
        break;

    case RSF_BA_WATE:
        z = ouch;
        p += 50;
        break;

    case RSF_BA_MANA:
        z = ouch;
        break;

    case RSF_BA_HOLY:
        z = ouch;
        p += 50;
        break;

    case RSF_BA_DARK:
        z = ouch;
        if (borg.trait[BI_RDARK])
            break;
        if (borg.trait[BI_RBLIND])
            break;
        p += 20;
        break;

    case RSF_BA_LIGHT:
        z = ouch;
        if (borg.trait[BI_RLITE])
            break;
        if (borg.trait[BI_RBLIND])
            break;
        p += 20;
        break;

    case RSF_STORM:
        z = ouch;
        if (borg.trait[BI_RSND])
            break;
        if (borg.trait[BI_ISSTUN])
            p += 50;
        if (borg.trait[BI_ISHEAVYSTUN])
            p += 100;
        if (borg.trait[BI_RCONF])
            break;
        p += 100;
        break;

    case RSF_DRAIN_MANA:
        if (borg.trait[BI_MAXSP])
            p += 10;
        break;

    case RSF_MIND_BLAST:
        z = 20;
        break;

    case RSF_BRAIN_SMASH:
        z = (12 * 15);
        p += 100;
        break;

    case RSF_WOUND:
        z = (12 * 15);
        p += 100;
        break;

    case RSF_BO_ACID:
        if (borg.trait[BI_IACID])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BO_ELEC:
        if (borg.trait[BI_IELEC])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BO_FIRE:
        if (borg.trait[BI_IFIRE])
            break;
        z = ouch;
        p += 40;
        break;

    case RSF_BO_COLD:
        if (borg.trait[BI_ICOLD])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BO_POIS:
        if (borg.trait[BI_IPOIS])
            break;
        z = ouch;
        p += 20;
        break;

    case RSF_BO_NETH:
        z = ouch + 100;
        if (borg.trait[BI_RNTHR])
            break;
        p += 200;
        break;

    case RSF_BO_WATE:
        z = ouch;
        p += 20;
        break;

    case RSF_BO_MANA:
        z = ouch;
        break;

    case RSF_BO_PLAS:
        z = ouch;
        p += 20;
        break;

    case RSF_BO_ICE:
        z = ouch;
        p += 20;
        break;

    case RSF_MISSILE:
        z = ouch;
        break;

    case RSF_BE_ELEC:
        z = ouch;
        p += 20;
        break;

    case RSF_BE_NETH:
        z = ouch;
        p += 50;
        break;

    case RSF_SCARE:
        p += 10;
        break;

    case RSF_BLIND:
        p += 10;
        break;

    case RSF_CONF:
        p += 10;
        break;

    case RSF_SLOW:
        p += 5;
        break;

    case RSF_HOLD:
        p += 20;
        break;

    case RSF_HASTE:
        p += 10 + borg.trait[BI_CDEPTH];
        break;

    case RSF_HEAL:
        p += 10;
        break;

    case RSF_HEAL_KIN:
        break;

    case RSF_BLINK:
        break;

    case RSF_TPORT:
        p += 10;
        break;

    case RSF_TELE_TO:
        p += 20;
        break;

    case RSF_TELE_SELF_TO:
        p += 20;
        break;

    case RSF_TELE_AWAY:
        p += 10;
        break;

    case RSF_TELE_LEVEL:
        p += 25;
        break;

    case RSF_DARKNESS:
        break;

    case RSF_TRAPS:
        p += 50;
        break;

    case RSF_FORGET:
        /* if you are a spell caster, this is very scary.*/
        if (borg.trait[BI_CURSP] > 10)
            p += 500;
        else
            p += 30;
        break;

    case RSF_SHAPECHANGE:
        p += 200;
        break;

        /* Summoning is only as dangerous as the monster that is */
        /* actually summoned.  This helps borgs kill summoners */

    case RSF_S_KIN:
        p += 55;
        break;

    case RSF_S_HI_DEMON:
        p += 95;
        break;

    case RSF_S_MONSTER:
        p += 55;
        break;

    case RSF_S_MONSTERS:
        p += 30;
        break;

    case RSF_S_ANIMAL:
        p += 15;
        break;

    case RSF_S_SPIDER:
        p += 25;
        break;

    case RSF_S_HOUND:
        p += 45;
        break;

    case RSF_S_HYDRA:
        p += 70;
        break;

    case RSF_S_AINU:
        p += 80;
        break;

    case RSF_S_DEMON:
        p += 80;
        break;

    case RSF_S_UNDEAD:
        p += 80;
        break;

    case RSF_S_DRAGON:
        p += 80;
        break;

    case RSF_S_HI_UNDEAD:
        p += 95;
        break;

    case RSF_S_HI_DRAGON:
        p += 95;
        break;

    case RSF_S_WRAITH:
        p += 95;
        break;

    case RSF_S_UNIQUE:
        p += 50;
        break;

    default:
        p += 1000;
        borg_note("# Mystery attack.  BUG.");
        break;
    }

    /* Things which hurt us alot need to be a concern */
    if (ouch >= borg.trait[BI_CURHP] / 2)
        ouch = ouch * 2;

    /* Notice damage */
    return (p + z + (borg.trait[BI_CDEPTH] * 2));
}

/*
 * Handle DETECTION spells and "call lite"
 *
 * Note that we must use the "old" player location
 */
static void borg_handle_self(char *str)
{
    int i;

    int q_x, q_y;
    int y, x;

    /* Extract panel */
    q_x = o_w_x / borg_panel_wid();
    q_y = o_w_y / borg_panel_hgt();

    /* Handle failure */
    if (borg_failure) {
        borg_note("# Something failed");
    }

    /* Handle "call lite" */
    else if (prefix(str, "lite")) {
        /* Message */
        borg_note(format("# Called lite at (%d,%d)", old_c.y, old_c.x));

        /* If not holding a lite, then glow adjacent grids */
        if (!borg.trait[BI_CURLITE]) {
            /* Scan the "local" grids (5x5) 2 same as torch grid
             * The spells do some goofy radius thing.
             */
            for (y = borg.c.y - 1; y <= borg.c.y + 1; y++) {
                /* Scan the "local" grids (5x5) */
                for (x = borg.c.x - 1; x <= borg.c.x + 1; x++) {
                    /* Get the grid */
                    borg_grid *ag = &borg_grids[y][x];

                    /* Mark as perma-lit */
                    ag->info |= BORG_GLOW;

                    /* Mark as not dark */
                    ag->info &= ~BORG_DARK;
                }
            }
        }

        /* Hack -- convert torch-lit grids to perma-lit grids */
        for (i = 0; i < borg_light_n; i++) {
            x = borg_light_x[i];
            y = borg_light_y[i];

            /* Get the grid */
            borg_grid *ag = &borg_grids[y][x];

            /* Mark as perma-lit */
            ag->info |= BORG_GLOW;

            /* Mark as not dark */
            ag->info &= ~BORG_DARK;
        }

    }

    /* Handle "detect walls" */
    else if (prefix(str, "wall")) {
        /* Message */
        borg_note(format(
            "# Detected walls (%d,%d to %d,%d)", q_y, q_x, q_y + 1, q_x + 1));

        /* Mark detected walls */
        borg_detect_wall[q_y + 0][q_x + 0] = true;
        borg_detect_wall[q_y + 0][q_x + 1] = true;
        borg_detect_wall[q_y + 1][q_x + 0] = true;
        borg_detect_wall[q_y + 1][q_x + 1] = true;
    }

    /* Handle "detect traps" */
    else if (prefix(str, "trap")) {
        /* Message */
        borg_note(format(
            "# Detected traps (%d,%d to %d,%d)", q_y, q_x, q_y + 1, q_x + 1));

        /* Mark detected traps */
        borg_detect_trap[q_y + 0][q_x + 0] = true;
        borg_detect_trap[q_y + 0][q_x + 1] = true;
        borg_detect_trap[q_y + 1][q_x + 0] = true;
        borg_detect_trap[q_y + 1][q_x + 1] = true;
    }

    /* Handle "detect doors" */
    else if (prefix(str, "door")) {
        /* Message */
        borg_note(format(
            "# Detected doors (%d,%d to %d,%d)", q_y, q_x, q_y + 1, q_x + 1));

        /* Mark detected doors */
        borg_detect_door[q_y + 0][q_x + 0] = true;
        borg_detect_door[q_y + 0][q_x + 1] = true;
        borg_detect_door[q_y + 1][q_x + 0] = true;
        borg_detect_door[q_y + 1][q_x + 1] = true;
    }

    /* Handle "detect traps and doors" */
    else if (prefix(str, "both")) {
        /* Message */
        borg_note(format("# Detected traps and doors (%d,%d to %d,%d)", q_y,
            q_x, q_y + 1, q_x + 1));

        /* Mark detected traps */
        borg_detect_trap[q_y + 0][q_x + 0] = true;
        borg_detect_trap[q_y + 0][q_x + 1] = true;
        borg_detect_trap[q_y + 1][q_x + 0] = true;
        borg_detect_trap[q_y + 1][q_x + 1] = true;

        /* Mark detected doors */
        borg_detect_door[q_y + 0][q_x + 0] = true;
        borg_detect_door[q_y + 0][q_x + 1] = true;
        borg_detect_door[q_y + 1][q_x + 0] = true;
        borg_detect_door[q_y + 1][q_x + 1] = true;
    }

    /* Handle "detect traps and doors and evil" */
    else if (prefix(str, "TDE")) {
        /* Message */
        borg_note(format("# Detected traps, doors & evil (%d,%d to %d,%d)", q_y,
            q_x, q_y + 1, q_x + 1));

        /* Mark detected traps */
        borg_detect_trap[q_y + 0][q_x + 0] = true;
        borg_detect_trap[q_y + 0][q_x + 1] = true;
        borg_detect_trap[q_y + 1][q_x + 0] = true;
        borg_detect_trap[q_y + 1][q_x + 1] = true;

        /* Mark detected doors */
        borg_detect_door[q_y + 0][q_x + 0] = true;
        borg_detect_door[q_y + 0][q_x + 1] = true;
        borg_detect_door[q_y + 1][q_x + 0] = true;
        borg_detect_door[q_y + 1][q_x + 1] = true;

        /* Mark detected evil */
        borg_detect_evil[q_y + 0][q_x + 0] = true;
        borg_detect_evil[q_y + 0][q_x + 1] = true;
        borg_detect_evil[q_y + 1][q_x + 0] = true;
        borg_detect_evil[q_y + 1][q_x + 1] = true;
    }

    /* Handle DETECT_EVIL */
    else if (prefix(str, "evil")) {
        /* Message */
        borg_note(format(
            "# Detected evil (%d,%d to %d,%d)", q_y, q_x, q_y + 1, q_x + 1));

        /* Mark detected evil */
        borg_detect_evil[q_y + 0][q_x + 0] = true;
        borg_detect_evil[q_y + 0][q_x + 1] = true;
        borg_detect_evil[q_y + 1][q_x + 0] = true;
        borg_detect_evil[q_y + 1][q_x + 1] = true;
    }

    /* Handle "detect objects" */
    else if (prefix(str, "obj")) {
        /* Message */
        borg_note(format(
            "# Detected objects (%d,%d to %d,%d)", q_y, q_x, q_y + 1, q_x + 1));

        /* Mark detected evil */
        borg_detect_obj[q_y + 0][q_x + 0] = true;
        borg_detect_obj[q_y + 0][q_x + 1] = true;
        borg_detect_obj[q_y + 1][q_x + 0] = true;
        borg_detect_obj[q_y + 1][q_x + 1] = true;
    }
}

/*
 * Look at the screen and update the borg
 *
 * Uses the "panel" info (w_x, w_y) obtained earlier
 *
 * Note that all the "important" messages that occured after our last
 * action have been "queued" in a usable form.  We must attempt to use
 * these messages to update our knowledge about the world, keeping in
 * mind that the world may have changed in drastic ways.
 *
 * Note that the "borg_t" variable corresponds *roughly* to player turns,
 * except that resting and "repeated" commands count as a single turn,
 * and "free" moves (including "illegal" moves, such as attempted moves
 * into walls, or tunneling into monsters) are counted as turns.
 *
 * Also note that "borg_t" is not incremented until the Borg is about to
 * do something, so nothing ever has a time-stamp of the current time.
 *
 * We rely on the fact that all "perma-lit" grids are memorized when
 * they are seen, so any grid on the current panel that appears "dark"
 * must not be perma-lit.
 *
 * We rely on the fact that all "objects" are memorized when they are
 * seen, so any grid on the current panel that appears "dark" must not
 * have an object in it.  But it could have a monster which looks like
 * an object, but this is very rare.  XXX XXX XXX
 *
 * XXX XXX XXX The basic problem with timestamping the monsters
 * and objects is that we often get a message about a monster, and so
 * we want to timestamp it, but then we cannot use the timestamp to
 * indicate that the monster has not been "checked" yet.  Perhaps
 * we need to do something like give each monster a "moved" flag,
 * and clear all the flags to false each turn before tracking. (?)
 *
 * Note that when two monsters of the same race are standing next to
 * each other, and they both move, such that the second monster ends
 * up where the first one began, we will incorrectly assume that the
 * first monster stayed still, and either the second monster moved
 * two spaces, or the second monster disappeared and a third monster
 * appeared, which is technically possible, if the first monster ate
 * the second, and then cloned the third.
 *
 * There is a problem with monsters which look like objects, namely,
 * they are assumed to be objects, and then if they leave line of
 * sight, they disappear, and the Borg assumes that they are gone,
 * when really they should be identified as monsters.
 *
 * XXX XXX Hack -- note the fast direct access to the screen.
 */
void borg_update(void)
{
    int          i, ii, k, x, y, dx, dy;
    unsigned int u_i;

    int hit_dist;

    char *msg;

    char *what;

    borg_grid *ag;

    bool reset = false;

    int  j;
    int  floor_grid       = 0;
    int  floor_glyphed    = 0;
    bool monster_in_vault = false;
    bool created_traps    = false;

    /*** Process objects/monsters ***/

    /* Scan monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Clear flags */
        kill->seen = false;
        kill->used = false;

        /* Skip recently seen monsters */
        if (borg_t - kill->when < 2000) {
            /* don't skip if hallucinating unless also afraid */
            /* we don't delete kills in this special case so we don't */
            /* get trapped by monsters we are afraid to attack */
            if (!(borg.trait[BI_ISIMAGE] && borg.trait[BI_ISAFRAID]))
                continue;
        }

        /* Note */
        borg_note(format("# Expiring a monster '%s' (%d) at (%d,%d)",
            (r_info[kill->r_idx].name), kill->r_idx, kill->pos.y, kill->pos.x));

        /* Kill the monster */
        borg_delete_kill(i);
    }

    /* Scan objects */
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take *take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind)
            continue;

        /* Clear flags */
        take->seen = false;

        /* delete them if they are under me
         * of if I am Hallucinating
         */
        if ((take->y == borg.c.y && take->x == borg.c.x)
            || (take->y == old_c.y && take->x == old_c.x)
            || borg.trait[BI_ISIMAGE]) {
            borg_delete_take(i);
            continue;
        }

        /* Skip recently seen objects */
        if (borg_t - take->when < 2000)
            continue;

        /* Note */
        borg_note(format("# Expiring an object '%s' (%d) at (%d,%d)",
            (take->kind->name), take->kind->kidx, take->y, take->x));

        /* Kill the object */
        borg_delete_take(i);
    }

    /*** Handle messages ***/

    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Note the message */
        borg_note(format("# %s (+)", msg));
    }

    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Get the arguments */
        what = strchr(msg, ':') + 1;

        /* Hack -- Handle "SELF" info */
        if (prefix(msg, "SELF:")) {
            borg_handle_self(what);
            borg_msg_use[i] = 1;
        }

        /* Handle "You feel..." */
        else if (prefix(msg, "FEELING_DANGER:")) {
            borg_feeling_danger = atoi(what);
            borg_msg_use[i]     = 1;
        }

        /* Handle "You feel..." */
        else if (prefix(msg, "FEELING_STUFF:")) {
            borg_feeling_stuff = atoi(what);
            borg_msg_use[i]    = 1;
        }
    }

    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Get the arguments */
        what = strchr(msg, ':') + 1;

        /* Handle "You hit xxx." */
        if (prefix(msg, "HIT:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_msg_use[i] = 2;
            }
        }

        /* Handle "You miss xxx." */
        else if (prefix(msg, "MISS:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_msg_use[i] = 2;
            }
        }

        /* Handle "You too afraid of xxx." */
        else if (prefix(msg, "AFRAID:")) {
            /* Attempt to find the monster */
            if (borg_grids[borg.goal.g.y][borg.goal.g.x].kill > 0) {
                borg_msg_use[i] = 2;
            } else {
                borg_create_kill(what, borg.goal.g);
            }
        }

        /* Handle "You have killed xxx." */
        else if (prefix(msg, "KILL:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_count_death(k);

                borg_delete_kill(k);
                borg_msg_use[i] = 2;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;

        }

        /* Handle "The xxx disappears!"  via teleport other, and blinks away */
        else if (prefix(msg, "BLINK:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_delete_kill(k);
                borg_msg_use[i] = 2;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "xxx dies." */
        else if (prefix(msg, "DIED:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 3)) > 0) {
                borg_count_death(k);
                borg_delete_kill(k);
                borg_msg_use[i] = 2;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "xxx screams in pain." */
        else if (prefix(msg, "PAIN:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 3)) > 0) {
                borg_msg_use[i] = 2;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__FEAR:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_msg_use[i] = 2;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__BOLD:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 0)) > 0) {
                borg_msg_use[i] = 2;
            }
        } else if (prefix(msg, "STATE_SLEEP:")) {
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        } else if (prefix(msg, "STATE__FEAR:")) {
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        } else if (prefix(msg, "STATE_CONFUSED:")) {
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }
    }

    /* Process messages */
    /* getting distance to allow for 'hit's */
    hit_dist = 1;
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        created_traps = false;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* if you have been teleported then do not count the monsters */
        /* as unknown unless they are very far away */
        if (prefix(msg, "SPELL_")) {
            int spell = atoi(msg + 6);
            if (spell == RSF_TELE_AWAY || spell == RSF_TELE_TO)
                hit_dist = 100;
            break;
        }

        /* monsters move from earthquake */
        if (prefix(msg, "QUAKE:")) {
            hit_dist = 3;
            break;
        }
    }

    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Get the arguments */
        what = strchr(msg, ':') + 1;

        /* Handle "You hit xxx." */
        if (prefix(msg, "HIT:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, hit_dist)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "You miss xxx." */
        else if (prefix(msg, "MISS:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, hit_dist)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "You have killed xxx." */
        else if (prefix(msg, "KILL:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 1)) > 0) {
                borg_count_death(k);
                borg_delete_kill(k);
                borg_msg_use[i] = 3;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "The xxx disappears!"  via teleport other, and blinks away */
        else if (prefix(msg, "BLINK:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.goal.g, 1)) > 0) {
                borg_delete_kill(k);
                borg_msg_use[i] = 3;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target == -1)
                successful_target = 2;
        }

        /* Handle "xxx dies." */
        else if (prefix(msg, "DIED:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_count_death(k);
                borg_delete_kill(k);
                borg_msg_use[i] = 3;
                /* reset the panel.  He's on a roll */
                borg.time_this_panel = 1;
            }
            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "xxx screams in pain." */
        else if (prefix(msg, "PAIN:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_msg_use[i] = 3;
            }

            /* Shooting through darkness worked */
            if (successful_target < 0)
                successful_target = 2;
        }

        /* Handle "xxx hits you." */
        else if (prefix(msg, "HIT_BY:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 1)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "xxx misses you." */
        else if (prefix(msg, "MISS_BY:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 1)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE_SLEEP:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_sleep_kill(k);
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "awake" */
        else if (prefix(msg, "STATE_AWAKE:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__FEAR:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__BOLD:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_msg_use[i] = 3;
            }
        }

        /* Hack -- Handle "spell" */
        else if (prefix(msg, "SPELL_")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, old_c, 20)) > 0) {
                borg_msg_use[i] = 3;
            }
            int spell = atoi(msg + 6);
            if (spell == RSF_TRAPS)
                created_traps = true;
        }

        /* Hack -- Handle "cackles evilly" */
        if (created_traps) {
            /* Remove the flag which tells borg that
             * Trap Detection was done here
             */
            borg_detect_trap[o_w_y / borg_panel_hgt() + 0]
                            [o_w_x / borg_panel_wid() + 0]
                = false;
            borg_detect_trap[o_w_y / borg_panel_hgt() + 0]
                            [o_w_x / borg_panel_wid() + 1]
                = false;
            borg_detect_trap[o_w_y / borg_panel_hgt() + 1]
                            [o_w_x / borg_panel_wid() + 0]
                = false;
            borg_detect_trap[o_w_y / borg_panel_hgt() + 1]
                            [o_w_x / borg_panel_wid() + 1]
                = false;

            /* Leave a note */
            borg_note("# Logging need for use of Detect Traps.");

            /* Mark Adjacent grids to borg as Traps */
            for (ii = 0; ii < 8; ii++) {
                /* Grid in that direction */
                x = borg.c.x + ddx_ddd[ii];
                y = borg.c.y + ddy_ddd[ii];

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* Skip unknown grids (important) */
                if (ag->feat == FEAT_NONE)
                    continue;

                /* Mark known floor grids as trap */
                if (borg_cave_floor_grid(ag)) {
                    ag->trap = true;

                    /* Leave a note */
                    borg_note(format("# Assuming a Traps at (%d,%d).", y, x));
                }
            }
        }
    }

    /* If we didn't successfully hit our target, mark the first unknown grid in
     * the path as a wall.  Then reset our targeting count so the borg can try
     * it again.  If no unknown grid is found, then allow the borg to attempt
     * another shot.  He will not allow a missile to travel along this path
     * since we just marked that unknown grid as a wall.  Perhaps the borg will
     * shoot at another monster (it being on another pathway). If I do not mark
     * the pathway with an unknown grid, then all the grids must be known, and I
     * ought to be allowed to shoot again.  Having a low number on
     * successful_target will prohibit me from using a ranged attack.
     */
    if (successful_target < 0) {
        if (successful_target > -10) {
            successful_target -= 10;
            if (borg_target_unknown_wall(borg.goal.g.y, borg.goal.g.x))
                successful_target = 2;
        }
    }

    /*** Handle new levels ***/

    /* Hack -- note new levels */
    if (old_depth != borg.trait[BI_CDEPTH]) {
        /* if we are not leaving town increment time since town clock */
        if (!old_depth)
            borg_time_town = 0;
        else
            borg_time_town += borg_t - borg_began;

        /* Hack -- Restart the clock */
        borg_t            = 1000;
        borg_t_morgoth    = 1;
        borg_t_antisummon = 0;

        /* reset our panel clock */
        borg.time_this_panel = 1;

        /* reset our vault/unique check */
        vault_on_level    = false;
        unique_on_level   = 0;
        scaryguy_on_level = false;

        /* reset our breeder flag */
        breeder_level = false;

        /* reset our need to see inviso clock */
        borg.need_see_invis = 1;

        /* reset our 'shoot in the dark' flag */
        successful_target = 0;

        /* When level was begun */
        borg_began = borg_t;

        /* New danger thresh-hold */
        avoidance = borg.trait[BI_CURHP];

        /* Wipe the danger */
        borg_danger_wipe = true;

        /* Clear our bought/sold flags */
        sold_item_num   = 0;
        sold_item_nxt   = 1;
        bought_item_nxt = 1;
        bought_item_num = 0;
        for (i = 0; i < 10; i++) {
            sold_item_tval[i]    = 0;
            sold_item_sval[i]    = 0;
            sold_item_pval[i]    = 0;
            sold_item_store[i]   = 0;
            bought_item_tval[i]  = 0;
            bought_item_sval[i]  = 0;
            bought_item_pval[i]  = 0;
            bought_item_store[i] = 0;
        }

        /* Update some stuff */
        borg_do_update_view = true;
        borg_do_update_lite = true;

        /* Examine the world */
        borg_do_inven = true;
        borg_do_equip = true;
        borg_do_spell = true;
        borg_do_panel = true;
        borg_do_frame = true;

        /* Enable some functions */
        borg_do_crush_junk = true;

        /* Mega-Hack -- Clear "call lite" stamp */
        borg.when_call_light = 0;

        /* Mega-Hack -- Clear "wizard lite" stamp */
        borg.when_wizard_light = 0;

        /* Mega-Hack -- Clear "detect traps" stamp */
        borg.when_detect_traps = 0;

        /* Mega-Hack -- Clear "detect doors" stamp */
        borg.when_detect_doors = 0;

        /* Mega-Hack -- Clear "detect walls" stamp */
        borg.when_detect_walls = 0;

        /* Mega-Hack -- Clear DETECT_EVIL stamp */
        borg.when_detect_evil = 0;

        /* Mega-Hack -- Clear "detect obj" stamp */
        borg.when_detect_obj = 0;

        /* Hack -- Clear "panel" flags */
        for (y = 0; y < 6; y++) {
            for (x = 0; x < 18; x++) {
                borg_detect_wall[y][x] = false;
                borg_detect_trap[y][x] = false;
                borg_detect_door[y][x] = false;
                borg_detect_evil[y][x] = false;
            }
        }

        /* Hack -- Clear "fear" */
        for (y = 0; y < 6; y++) {
            for (x = 0; x < 18; x++) {
                borg_fear_region[y][x] = 0;
            }
        }

        /* Remove regional fear from monsters, it gets added back in later. */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {
                borg_fear_monsters[y][x] = 0;
            }
        }
#if 0
        /* Hack -- Clear "shop visit" stamps */
        for (i = 0; i < MAX_STORES; i++) borg_shops[i].when = 0;
#endif
        /* No goal yet */
        borg.goal.type = 0;

        /* Hack -- Clear "shop" goals */
        borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

        /* Reset food&fuel in store */
        borg_food_onsale = -1;
        borg_fuel_onsale = -1;

        /* Do not use any stairs */
        borg.stair_less = borg.stair_more = false;

        /* Hack -- cannot rise past town */
        if (!borg.trait[BI_CDEPTH])
            borg.goal.rising = false;

        /* Assume not leaving the level */
        borg.goal.leaving = false;

        /* Assume not fleeing the level */
        borg.goal.fleeing          = false;
        borg.goal.fleeing_lunal    = false;
        borg.goal.fleeing_munchkin = false;

        /* Assume not ignoring monsters */
        borg.goal.ignoring = false;

        /* clear stuff feeling - danger feeling is automatic */
        borg_feeling_stuff = 0;

        /* Assume not fleeing the level */
        if (!borg.trait[BI_CDEPTH])
            borg.goal.fleeing_to_town = false;
        if (borg.trait[BI_CDEPTH] >= 2)
            borg.goal.fleeing_to_town = false;

        /* No known stairs */
        track_less.num = 0;
        track_more.num = 0;

        /* No known glyph */
        track_glyph.num = 0;

        /* No known steps */
        track_step.num = 0;

        /* No known doors */
        track_door.num = 0;

        /* No known doors */
        track_closed.num = 0;

        /* No known veins*/
        track_vein.num = 0;

        /* No artifacts swapping in and out */
        track_worn_num = 0;

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Forget old objects */
        memset(borg_takes, 0, 256 * sizeof(borg_take));

        /* No monsters here */
        borg_kills_cnt = 0;
        borg_kills_nxt = 1;

        /* Hack- Assume that Morgoth is on Level 100 unless
         * we know he is dead
         */
        morgoth_on_level = false;
        if ((borg.trait[BI_CDEPTH] >= 100 && !borg.trait[BI_KING])
            || (unique_on_level == borg_morgoth_id)) {
            /* We assume Morgoth is on this level */
            morgoth_on_level = true;

            /* Must build a new sea of runes */
            borg_needs_new_sea = true;
        }

        /* Reset nasties to 0 */
        for (i = 0; i < borg_nasties_num; i++) {
            borg_nasties_count[i] = 0;

            /* Assume there are some Hounds on the Level */
            if (borg_nasties[i] == 'Z')
                borg_nasties_count[i] = 25; /* Assume some on level */
        }

        /* Forget old monsters */
        memset(borg_kills, 0, 256 * sizeof(borg_kill));

        /* Hack -- Forget race counters */
        memset(borg_race_count, 0, z_info->r_max * sizeof(int16_t));

        /* Hack -- Rarely, a Unique can die off screen and the borg will miss
         * it. This check will cheat to see if uniques are dead.
         */

        /* Clear our Uniques vars */
        borg_numb_live_unique    = 0;
        borg_living_unique_index = 0;
        borg_unique_depth        = 127;

        /*Extract dead uniques and set some Prep code numbers */
        for (u_i = 1; u_i < (unsigned int)(z_info->r_max - 1); u_i++) {
            struct monster_race *r_ptr = &r_info[u_i];

            /* Skip non-monsters */
            if (!r_ptr->name)
                continue;

            /* Skip non-uniques */
            if (!(rf_has(r_ptr->flags, RF_UNIQUE)))
                continue;

            /* Mega-Hack -- Access "dead unique" list */
            if (r_ptr->max_num == 0)
                borg_race_death[u_i] = 1;

            /* If any have been killed it is not a live unique */
            if (borg_race_death[u_i] != 0)
                continue;

            /* skip if deeper than max dlevel */
            if (r_ptr->level > borg.trait[BI_MAXDEPTH])
                continue;

            /* skip certain questor or seasonal Monsters */
            if (rf_has(r_ptr->flags, RF_QUESTOR)
                || rf_has(r_ptr->flags, RF_SEASONAL))
                continue;

            /* skip things that are just a shape of a different entry */
            if (!r_ptr->rarity)
                continue;

            /* Define some numbers used by Prep code */
            borg_numb_live_unique++;

            /* Its important to know the depth of the most shallow guy */
            if (r_ptr->level < borg_unique_depth)
                borg_unique_depth = r_ptr->level;

            if (u_i < borg_living_unique_index || borg_living_unique_index == 0)
                borg_living_unique_index = u_i;
        }

        /* Forget the map */
        borg_forget_map();

        /* Reset */
        reset = true;

        /* save once per level, but not if Lunal Scumming */
        if (borg_flag_save && !borg.lunal_mode && !borg.munchkin_mode)
            borg_save = true;

        /* Save new depth */
        old_depth         = borg.trait[BI_CDEPTH];

        borg.times_twitch = 0;
        borg.escapes      = 0;

    }

    /* Handle old level */
    else {
        /* reduce Resistance count. NOTE: do not reduce below 1.  That is done
         */
        /* when the spell is cast. */
        if (borg.resistance >= 1) {
            borg.resistance -= borg_game_ratio;
        }

        /* reduce the No-resting-because-I-just-did-a-prep-spell */
        if (borg.no_rest_prep >= 1) {
            borg.no_rest_prep -= borg_game_ratio;
        }

        /* Count down to blast off */
        if (borg.goal.recalling >= 1) {
            borg.goal.recalling -= borg_game_ratio;

            /* dont let it get to 0 or borg will recast the spell */
            if (borg.goal.recalling <= 0)
                borg.goal.recalling = 1;
        }

        /* when we need to cast this spell again */
        if (borg.see_inv >= 1) {
            borg.see_inv -= borg_game_ratio;
        }

        /* Hack- Assume that Morgoth is on Level 100
         */
        morgoth_on_level = false;
        if ((borg.trait[BI_CDEPTH] >= 100 && !borg.trait[BI_KING])
            || (unique_on_level == borg_morgoth_id)) {
            /* We assume Morgoth is on this level */
            morgoth_on_level = true;
        }

        /* If been sitting on level 100 for a long time and Morgoth
         * is a:
         * 1. no show,
         * 2. was here but has not been around in a very long time.
         * then assume he is not here so borg can continue to
         * explore the dungeon.
         */
        if (morgoth_on_level && borg_t - borg_began >= 500) {
            /* Morgoth is a no show */
            if (unique_on_level != borg_morgoth_id)
                morgoth_on_level = false;

            /* Morgoth has not been seen in a long time */
            if (unique_on_level == borg_morgoth_id
                && (borg_t - borg_t_morgoth > 500)) {
                borg_note(format("# Morgoth has not been seen in %d turns.  "
                                 "Going to hunt him.",
                    borg_t - borg_t_morgoth));
                morgoth_on_level = false;
            }

            /* Morgoth has not been seen in a very long time */
            if (borg_t - borg_t_morgoth > 2500) {
                borg_note(
                    format("# Morgoth has not been seen in %d turns.  No show.",
                        borg_t - borg_t_morgoth));
                unique_on_level = 0;
            }
        }

        /* Slight Cheat for the borg.  He would like to keep an
         * eye on the Resist All spell.  The borg will check if
         * all resistances are on and if so, give himself the
         * flag, but not if the flag is already on.
         */
        if (borg.resistance <= 0 && borg.temp.res_fire && borg.temp.res_elec
            && borg.temp.res_cold && borg.temp.res_acid && borg.temp.res_pois) {
            /* Set the flag on with some average count */
            borg.resistance = 20000;
        }

        /* Slight safety check for borg to make sure he really
         * does resist all if he thinks he does.
         */
        if (borg.resistance >= 1 && /* borg thinks it's on */
            (borg.temp.res_fire + borg.temp.res_elec + borg.temp.res_cold
                    + borg.temp.res_acid + borg.temp.res_pois
                != 5)) {
            /* Set the flag on with some average count */
            borg.resistance = 0;
        }

        /* Reduce fear over time */
        if (!(borg_t % 10)) {
            for (y = 0; y < 6; y++) {
                for (x = 0; x < 18; x++) {
                    if (borg_fear_region[y][x])
                        borg_fear_region[y][x]--;
                }
            }
        }

        /* Remove regional fear from monsters, it gets added back in later. */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {
                borg_fear_monsters[y][x] = 0;
            }
        }

        /* Handle changing map panel */
        if ((o_w_x != w_x) || (o_w_y != w_y)) {
            /* Forget the previous map panel */
            for (dy = 0; dy < SCREEN_HGT; dy++) {
                for (dx = 0; dx < SCREEN_WID; dx++) {
                    /* Access the actual location */
                    x = o_w_x + dx;
                    y = o_w_y + dy;

                    if (y >= AUTO_MAX_Y || x >= AUTO_MAX_X)
                        continue;

                    /* Get the borg_grid */
                    ag = &borg_grids[y][x];

                    /* Clear the "okay" field */
                    ag->info &= ~BORG_OKAY;
                }
            }

            /* Time stamp this new panel-- to avoid a repeated motion bug */
            borg.time_this_panel = 1;
        }

        /* Examine the world while in town. */
        if (!borg.trait[BI_CDEPTH])
            borg_do_inven = true;
        if (!borg.trait[BI_CDEPTH])
            borg_do_equip = true;
    }

    /*** Update the map ***/

    /* Track floors and items */
    borg_wank_num = 0;

    /* Update the map */
    borg_update_map();

    /* Mark this grid as having been stepped on */
    track_step.x[track_step.num] = borg.c.x;
    track_step.y[track_step.num] = borg.c.y;
    track_step.num++;

    /* Hack - Clean the steps every so often */
    if (track_step.num >= 75) {
        for (i = 0; i <= 75; i++) {
            /* Move each step down one position */
            track_step.x[i] = track_step.x[i + 1];
            track_step.y[i] = track_step.y[i + 1];
        }
        /* reset the count */
        track_step.num = 75;
    }

    /* Reset */
    if (reset) {
        /* Fake old panel */
        o_w_x = w_x;
        o_w_y = w_y;

        /* Fake old location */
        old_c = borg.c;

        /* Fake goal location */
        borg.goal.g = borg.c;
    }

    /* Player moved */
    if ((old_c.x != borg.c.x) || (old_c.y != borg.c.y)) {
        /* Update view */
        borg_do_update_view = true;

        /* Update lite */
        borg_do_update_lite = true;

        /* Assume I can shoot here */
        successful_target = 0;
    }

    /* Update the view */
    if (borg_do_update_view) {
        /* Update the view */
        borg_update_view();

        /* Take note */
        borg_do_update_view = false;
    }

    /* Update the lite */
    if (borg_do_update_lite) {
        /* Update the lite */
        borg_update_light();

        /* Take note */
        borg_do_update_lite = false;
    }

    /* make sure features that have monsters on them that can't passwall are
     * marked FEAT_FLOOR */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Ignore monsters who can pass through walls  */
        if (rf_has(r_info[kill->r_idx].flags, RF_PASS_WALL))
            continue;

        /* Make sure this grid keeps Floor grid */
        borg_grids[kill->pos.y][kill->pos.x].feat = FEAT_FLOOR;
    }

    /* Let me know if I am correctly positioned for special
     * Morgoth routines; sea of runes
     *
     * ############
     * #3.........#
     * #2..xxxxx..#
     * #1..xxxxx..#
     * #0..xx@xx..#
     * #1..xxxxx..#
     * #2..xxxxx..#
     * #3.........#
     * #4432101234#
     * ############
     */
    borg_morgoth_position = false;
    if (!borg.trait[BI_KING] && morgoth_on_level) {
        /* Must be in a fairly central region */
        if (borg.c.y >= 15 && borg.c.y <= AUTO_MAX_Y - 15 && borg.c.x >= 50
            && borg.c.x <= AUTO_MAX_X - 50) {
            /* Scan neighbors */
            for (j = 0; j < 24; j++) {
                y = borg.c.y + borg_ddy_ddd[j];
                x = borg.c.x + borg_ddx_ddd[j];

                /* Get the grid */
                ag = &borg_grids[y][x];

                /* Skip unknown grids (important) */
                if (ag->glyph)
                    floor_glyphed++;
            }

            /* Number of perfect grids */
            if (floor_glyphed == 24)
                borg_morgoth_position = true;

        } /* Centrally located */
    } /* on depth 100 not King */

    /* Check to see if I am in a correct anti-summon corridor
     *            ############## We want the borg to dig a tunnel which
     *            #............# limits the LOS of summoned monsters.
     *          ###............# It works better in hallways.
     *         ##@#............#
     *         #p##............#
     * ########## #######+######
     * #                  #
     * # ################ #
     *   #              # #
     * ###              # #
     *
     *
     *            ############## Don't Build either of these as an AS-corridor
     *            #............#
     *          ###............#
     *         ##@#............#
     *         #p##............##
     * ########.#########+#####@#
     * #                  #   ###
     * # ################ #
     *   #              # #
     * ###              # #
     *
     */
    for (j = 0; j < 24; j++) {
        y = borg.c.y + borg_ddy_ddd[j];
        x = borg.c.x + borg_ddx_ddd[j];

        /* Stay in the bounds */
        if (!square_in_bounds(cave, loc(x, y))) {
            floor_grid++;
            continue;
        }

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Skip unknown grids (important to make sure next one in line is not
         * LOS floor) */
        if (j <= 7 && ag->feat <= FEAT_MORE)
            floor_grid++;
        if (j >= 8 && ag->feat <= FEAT_MORE
            && borg_los(borg.c.y, borg.c.x, y, x))
            floor_grid++;
    }

    /* Number of perfect grids */
    if (floor_grid == 1)
        borg_as_position = true;
    else
        borg_as_position = false;

    /* Examine changing doors while shallow */
    if (borg.trait[BI_CLEVEL] <= 5 && borg.trait[BI_CDEPTH]
        && track_closed.num) {
        /* Scan all known closed doors */
        for (i = 0; i < track_closed.num; i++) {
            /* Get location */
            x = track_closed.x[i];
            y = track_closed.y[i];

            /* Get the borg_grid */
            ag = &borg_grids[y][x];

            if (ag->feat == FEAT_OPEN || ag->feat == FEAT_BROKEN) {
                /* This door was not opened by me */
                borg_note(format(
                    "# Monster opened door at %d,%d.  That's scary.", y, x));
                scaryguy_on_level = true;
            }
        }
    }

    /*** Track objects and monsters ***/

    /* Pass 1 -- stationary monsters */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track stationary monsters */
        if (wank->is_kill
            && observe_kill_move(
                wank->y, wank->x, 0, wank->t_a, wank->t_c, false)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 2 -- stationary objects */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track stationary objects */
        if (wank->is_take
            && observe_take_move(wank->y, wank->x, 0, wank->t_a, wank->t_c)) {
            /* Hack -- excise the entry */
            wank->is_take = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 3a -- moving monsters (distance 1) */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track moving monsters */
        if (wank->is_kill
            && observe_kill_move(
                wank->y, wank->x, 1, wank->t_a, wank->t_c, false)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 3b -- moving monsters (distance 2) */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track moving monsters */
        if (wank->is_kill
            && observe_kill_move(
                wank->y, wank->x, 2, wank->t_a, wank->t_c, false)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 3c -- moving monsters (distance 3) */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track moving monsters */
        if (wank->is_kill
            && observe_kill_move(
                wank->y, wank->x, 3, wank->t_a, wank->t_c, false)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 3d -- moving monsters (distance 7, allow changes) */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track moving monsters */
        if (wank->is_kill
            && observe_kill_move(
                wank->y, wank->x, 7, wank->t_a, wank->t_c, true)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 4 -- new objects */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track new objects */
        if (wank->is_take
            && observe_take_diff(wank->y, wank->x, wank->t_a, wank->t_c)) {
            /* Hack -- excise the entry (unless it is also a monster) */
            wank->is_take = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }
    /* Pass 5 -- new monsters */
    for (i = borg_wank_num - 1; i >= 0; i--) {
        borg_wank *wank = &borg_wanks[i];

        /* Track new monsters */
        if (wank->is_kill
            && observe_kill_diff(wank->y, wank->x, wank->t_a, wank->t_c)) {
            /* Hack -- excise the entry */
            wank->is_kill = false;
            if (!wank->is_take && !wank->is_kill)
                borg_wanks[i] = borg_wanks[--borg_wank_num];
        }
    }

    /*** Handle messages ***/

    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Get the arguments */
        what = strchr(msg, ':') + 1;

        /* Handle "xxx dies." */
        if (prefix(msg, "DIED:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_count_death(k);
                borg_delete_kill(k);
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "xxx screams in pain." */
        else if (prefix(msg, "PAIN:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "xxx hits you." */
        else if (prefix(msg, "HIT_BY:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, hit_dist)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "xxx misses you." */
        else if (prefix(msg, "MISS_BY:")) {
            /* Attempt to find the monster */

            if ((k = borg_locate_kill(what, borg.c, hit_dist)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE_SLEEP:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_sleep_kill(k);
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "awake" */
        else if (prefix(msg, "STATE_AWAKE:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__FEAR:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Handle "sleep" */
        else if (prefix(msg, "STATE__BOLD:")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_msg_use[i] = 4;
            }
        }

        /* Hack -- Handle "spell" */
        else if (prefix(msg, "SPELL_")) {
            /* Attempt to find the monster */
            if ((k = borg_locate_kill(what, borg.c, 20)) > 0) {
                borg_msg_use[i] = 4;
            }
        }
    }
    /* Process messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Skip parsed messages */
        if (borg_msg_use[i])
            continue;

        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Get the arguments */
        what = strchr(msg, ':') + 1;

        /* Handle "xxx hits you." */
        if (prefix(msg, "HIT_BY:")) {
            borg_fear_regional(what, borg.c.y, borg.c.x,
                4 * ((borg.trait[BI_CDEPTH] / 5) + 1), false);
            borg_msg_use[i] = 5;
        }

        /* Handle "xxx misses you." */
        else if (prefix(msg, "MISS_BY:")) {
            borg_fear_regional(what, borg.c.y, borg.c.x,
                2 * ((borg.trait[BI_CDEPTH] / 5) + 1), false);
            borg_msg_use[i] = 5;
        }

        /* Hack -- Handle "spell" */
        else if (prefix(msg, "SPELL_")) {
            borg_fear_regional(what, borg.c.y, borg.c.x,
                borg_fear_spell(atoi(msg + 6)), false);
            borg_msg_use[i] = 5;
        }
    }
    /* Display messages */
    for (i = 0; i < borg_msg_num; i++) {
        /* Get the message */
        msg = borg_msg_buf + borg_msg_pos[i];

        /* Final message */
        borg_note(format("# %s (%d)", msg, borg_msg_use[i]));
    }

    /*** Notice missing monsters ***/
    /* Scan the monster list */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Skip seen monsters */
        if (kill->when == borg_t)
            continue;

        /* Skip assigned monsters */
        if (kill->seen)
            continue;

        /* Hack -- blind or hallucinating */
        if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISIMAGE])
            continue;

        /* Predict the monster */
        borg_follow_kill(i);
    }

    /* Update the fear_grid_monsters[][] with the monsters danger
     * This will provide a 'regional' fear from the accumulated
     * group of monsters.  One Orc wont be too dangerous, but 20
     * of them can be deadly.
     */
    for (i = 1; i < borg_kills_nxt; i++) {
        int p;

        borg_kill *kill = &borg_kills[i];

        struct monster_race *r_ptr;

        /* Reset the 'vault monsters */
        monster_in_vault = false;

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        r_ptr = &r_info[kill->r_idx];

        /* Skip monsters that dont chase */
        if (rf_has(r_ptr->flags, RF_NEVER_MOVE))
            continue;

        /* Skip monsters that are far away */
        if (distance(kill->pos, borg.c) >= 20)
            continue;

        /* Skip monsters in vaults */
        if (vault_on_level) {
            /* Check adjacent grids to monster */
            for (ii = 0; ii < 8; ii++) {
                /* Grid in that direction */
                x = kill->pos.x + ddx_ddd[ii];
                y = kill->pos.y + ddy_ddd[ii];

                /* Legal grid */
                if (!square_in_bounds_fully(cave, loc(x, y)))
                    continue;

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* Skip unknown grids (important) */
                if (ag->feat == FEAT_NONE)
                    continue;

                /* Mark this as a Vault monster */
                if (ag->feat == FEAT_PERM) {
                    monster_in_vault = true;
                }
            }
        }

        /* Monster is probably in a vault, ignore the regional fear */
        if (monster_in_vault == true)
            continue;

        /* Obtain some danger */
        p = (borg_danger(kill->pos.y, kill->pos.x, 1, false, false) / 10);

        /* Apply the Fear */
        borg_fear_grid(r_info[kill->r_idx].name, kill->pos.y, kill->pos.x, p);
    }

    /*** Notice missing objects ***/

    /* Scan the object list */
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take *take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind)
            continue;

        /* Skip seen objects */
        if (take->when >= borg_t - 2)
            continue;

        /* Hack -- blind or hallucinating */
        if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISIMAGE])
            continue;

        /* Follow the object */
        borg_follow_take(i);
    }

    /*** Various things ***/

    /* Forget goals while "impaired" in any way */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISAFRAID] || borg.trait[BI_ISIMAGE])
        borg.goal.type = 0;

    /* Forget goals while "bleeding" in any way */
    if (borg.trait[BI_ISWEAK] || borg.trait[BI_ISPOISONED]
        || borg.trait[BI_ISCUT] || borg.trait[BI_ISSTUN]
        || borg.trait[BI_ISHEAVYSTUN])
        borg.goal.type = 0;

    /* Save the hit points */
    borg.oldchp = borg.trait[BI_CURHP];

    /* Forget failure */
    borg_failure = false;

    /* Forget the messages */
    borg_msg_len = 0;
    borg_msg_num = 0;

    /*** Save old info ***/

    /* Save the old "location" */
    old_c = borg.c;

    /* Save the old "panel" */
    o_w_x = w_x;
    o_w_y = w_y;

    /*** Defaults ***/

    /* Default "goal" location */
    borg.goal.g = borg.c;
}

void borg_init_update(void)
{
    /* Array of "wanks" */
    borg_wanks = mem_zalloc(AUTO_VIEW_MAX * sizeof(borg_wank));

    /*** Reset the map ***/

    /* Forget the map */
    borg_forget_map();
}

void borg_free_update(void)
{
    mem_free(borg_wanks);
    borg_wanks = NULL;
}

#endif
