/**
 * \file borg-flow-kill.c
 * \brief Flow (move) toward a kill (monster)
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

#include "borg-flow-kill.h"

#ifdef ALLOW_BORG

#include "../game-world.h"
#include "../monster.h"
#include "../ui-term.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-fight-attack.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow-take.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg-util.h"
#include "borg.h"

/*
 * The monster list.  This list is used to "track" monsters.
 */

int16_t borg_kills_cnt;
int16_t borg_kills_summoner; /* index of a summoner */
int16_t borg_kills_nxt;

borg_kill *borg_kills;

/*
 * Hack -- count racial appearances per level
 */
int16_t *borg_race_count;

/*
 * Hack -- count racial kills (for uniques)
 */

int16_t *borg_race_death;

/*
 * Hack -- help identify "unique" monster names
 */
static int           borg_unique_size; /* Number of uniques */
static unsigned int *borg_unique_what; /* Indexes of uniques */
static const char  **borg_unique_text; /* Names of uniques */

/*
 * Hack -- help identify "normal" monster names
 */
static int           borg_normal_size; /* Number of normals */
static unsigned int *borg_normal_what; /* Indexes of normals */
static const char  **borg_normal_text; /* Names of normals */

/*
 * Monsters or Uniques on this level
 */
unsigned int borg_morgoth_id   = 0;
unsigned int borg_sauron_id    = 0;
unsigned int borg_tarrasque_id = 0;
unsigned int borg_t_id         = 0;
unsigned int unique_on_level;
bool         scaryguy_on_level; /* flee from certain guys */
bool         morgoth_on_level;
bool         borg_morgoth_position;
bool         breeder_level = false; /* Borg will shut door */

uint8_t borg_nasties_num   = 7; /* Current size of the list */
uint8_t borg_nasties_count[7];
char    borg_nasties[7]       = { 'Z', 'A', 'V', 'U', 'L', 'W',
             'D' }; /* Order of Nastiness.  Hounds < Demons < Wyrms */
uint8_t borg_nasties_limit[7] = { 20, 20, 10, 10, 10, 10, 10 };

int morgy_panel_y;
int morgy_panel_x;

/* am I fighting a unique? */
int  borg_fighting_unique;
bool borg_fighting_evil_unique; /* Need to know if evil for Priest Banishment */

/* am I fighting a summoner? */
bool borg_fighting_summoner;

/*
 * Hack -- Update a "new" monster
 */
static void borg_update_kill_new(int i)
{
    int k   = 0;
    int j   = 0;
    int num = 0;
    int pct;

    borg_kill *kill            = &borg_kills[i];

    struct monster      *m_ptr = &cave->monsters[kill->m_idx];
    struct monster_race *r_ptr = &r_info[kill->r_idx];

    /* Extract the monster speed */
    kill->speed = (m_ptr->mspeed);

    /* Extract max hitpoints */
    /* This is a cheat.  Borg does not look
     * at the bar at the bottom and frankly that would take a lot of code.
     * It would involve targeting every monster to read their individual bar.
     * then keeping track of it.  When the borg has telepathy this would
     * cripple him down and be tremendously slow.
     *
     * This cheat is not too bad.  A human could draw the same info from
     * from the screen.
     *
     * Basically the borg is cheating the real hit points of the monster then
     * using that information to calculate the estimated hp of the monster.
     * Its the same basic tactic that we would use.
     *
     * Kill->power is used a lot in borg_danger,
     * for calculating damage from breath attacks.
     */
    if (m_ptr->maxhp) {
        /* Cheat the "percent" of health */
        pct = 100L * m_ptr->hp / ((m_ptr->maxhp > 1) ? m_ptr->maxhp : 1);
    } else {
        pct = 100;
    }

    /* Compute estimated HP based on number of * in monster health bar */
    kill->power  = (m_ptr->maxhp * pct) / 100;
    kill->injury = 100 - pct;

    /* Extract the Level*/
    kill->level = r_ptr->level;

    /* Some monsters never move */
    if (rf_has(r_ptr->flags, RF_NEVER_MOVE))
        kill->awake = true;

    /* Cheat in the game's index of the monster.
     * Used in tracking monsters
     */
    kill->m_idx = square_monster(cave, loc(kill->pos.x, kill->pos.y))->midx;

    /* Is it sleeping */
    if (m_ptr->m_timed[MON_TMD_SLEEP] == 0)
        kill->awake = true;
    else
        kill->awake = false;

    /* Is it afraid */
    if (m_ptr->m_timed[MON_TMD_FEAR] == 0)
        kill->afraid = false;
    else
        kill->afraid = true;

    /* Is it confused */
    if (m_ptr->m_timed[MON_TMD_CONF] == 0)
        kill->confused = false;
    else
        kill->confused = true;

    /* Is it stunned*/
    if (m_ptr->m_timed[MON_TMD_STUN] == 0)
        kill->stunned = false;
    else
        kill->stunned = true;

    /* Preload the spells from the race into this individual monster */
    kill->spell_flags[0] = r_ptr->spell_flags[0];
    kill->spell_flags[1] = r_ptr->spell_flags[1];
    kill->spell_flags[2] = r_ptr->spell_flags[2];

    /* Extract the spells */
    for (k = RSF_NONE + 1; k < RSF_MAX; k++) {
        if (rsf_has(r_ptr->spell_flags, k))
            kill->spell[num++] = k;
    }

    /* Store the number of ranged attacks */
    kill->ranged_attack = num;

    /* We want to remember Morgy's panel */
    if (kill->r_idx == borg_morgoth_id) {
        j = ((kill->pos.y - borg_panel_hgt() / 2) / borg_panel_hgt())
            * borg_panel_hgt();
        if (j < 0)
            j = 0;
        if (j > DUNGEON_HGT - SCREEN_HGT)
            j = DUNGEON_HGT - SCREEN_HGT;
        morgy_panel_y = j;

        j = ((kill->pos.x - borg_panel_wid() / 2) / borg_panel_wid())
            * borg_panel_wid();
        if (j < 0)
            j = 0;
        if (j > DUNGEON_WID - SCREEN_WID)
            j = DUNGEON_WID - SCREEN_WID;
        morgy_panel_x = j;
    }

    /* Hack -- Force the monster to be sitting on a floor
     * grid unless that monster can pass through walls
     */
    if (!rf_has(r_ptr->flags, RF_PASS_WALL)) {
        borg_grids[kill->pos.y][kill->pos.x].feat = FEAT_FLOOR;
    }

    /* Hack -- Force the ghostly monster to be in a wall
     * grid until the grid is proven to be something else
     */
    if (borg_grids[kill->pos.y][kill->pos.x].feat != FEAT_FLOOR
        && rf_has(r_ptr->flags, RF_PASS_WALL)) {
        borg_grids[kill->pos.y][kill->pos.x].feat = FEAT_GRANITE;
    }
}

/*
 * Hack -- Update a "old" monster
 *
 * We round the player speed down, and the monster speed up,
 * and we assume maximum racial speed for each monster.
 */
static void borg_update_kill_old(int i)
{
    int t, e;
    int k   = 0;
    int num = 0;
    int j   = 0;
    int pct;

    borg_kill *kill       = &borg_kills[i];

    struct monster *m_ptr = square_monster(cave, loc(kill->pos.x, kill->pos.y));
    struct monster_race *r_ptr = &r_info[kill->r_idx];

    /* Extract max hitpoints */
    /* Extract actual Hitpoints, this is a cheat.  Borg does not look
     * at the bar at the bottom and frankly that would take a lot of code.
     * It would involve targeting every monster to read their individual bar.
     * then keeping track of it.  When the borg has telepathy this would
     * cripple him down and be tremendously slow.
     *
     * This cheat is not too bad.  A human could draw the same info from
     * from the screen.
     *
     * Basically the borg is cheating the real hit points of the monster then
     * using that information to calculate the estimated hp of the monster.
     * It's the same basic tactics that we would use.
     *
     * Kill->power is used a lot in borg_danger,
     * for calculating damage from breath attacks.
     */

    if (m_ptr->maxhp) {
        /* Cheat the "percent" of health */
        pct = 100L * m_ptr->hp / ((m_ptr->maxhp > 1) ? m_ptr->maxhp : 1);
    } else {
        pct = 100;
    }

    /* Compute estimated HP based on number of * in monster health bar */
    kill->power  = (m_ptr->maxhp * pct) / 100;
    kill->injury = 100 - pct;

    /* Is it sleeping */
    if (m_ptr->m_timed[MON_TMD_SLEEP] == 0)
        kill->awake = true;
    else
        kill->awake = false;

    /* Is it afraid */
    if (m_ptr->m_timed[MON_TMD_FEAR] == 0)
        kill->afraid = false;
    else
        kill->afraid = true;

    /* Is it confused */
    if (m_ptr->m_timed[MON_TMD_CONF] == 0)
        kill->confused = false;
    else
        kill->confused = true;

    /* Is it stunned*/
    if (m_ptr->m_timed[MON_TMD_STUN] == 0)
        kill->stunned = false;
    else
        kill->stunned = true;

    /* Cheat in the game's index of the monster.
     * Used in tracking monsters
     */
    kill->m_idx = square_monster(cave, loc(kill->pos.x, kill->pos.y))->midx;

    /* Extract the monster speed */
    kill->speed = (m_ptr->mspeed);

    /* Player energy per game turn */
    e = extract_energy[borg.trait[BI_SPEED]];

    /* Game turns per player move */
    t = (100 + (e - 1)) / e;

    /* Monster energy per game turn */
    e = extract_energy[kill->speed];

    /* Monster moves (times ten) */
    kill->moves = (t * e) / 10;

    /* Preload the spells from the race into this individual monster */
    kill->spell_flags[0] = r_ptr->spell_flags[0];
    kill->spell_flags[1] = r_ptr->spell_flags[1];
    kill->spell_flags[2] = r_ptr->spell_flags[2];

    /* Extract the spells */
    for (k = RSF_NONE + 1; k < RSF_MAX; k++) {
        if (rsf_has(r_ptr->spell_flags, k))
            kill->spell[num++] = k;
    }

    /* Store the number of ranged attacks */
    kill->ranged_attack = num;

    /* Special check on uniques, sometimes the death
     * is not caught so he thinks they are still running
     * around
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && r_ptr->max_num == 0)
        borg_race_death[i] = 1;

    /* We want to remember Morgy's panel */
    if (streq(r_ptr->base->name, "Morgoth")) {
        j = ((kill->pos.y - borg_panel_hgt() / 2) / borg_panel_hgt())
            * borg_panel_hgt();
        if (j < 0)
            j = 0;
        if (j > DUNGEON_HGT - SCREEN_HGT)
            j = DUNGEON_HGT - SCREEN_HGT;
        morgy_panel_y = j;

        j = ((kill->pos.x - borg_panel_wid() / 2) / borg_panel_wid())
            * borg_panel_wid();
        if (j < 0)
            j = 0;
        if (j > DUNGEON_WID - SCREEN_WID)
            j = DUNGEON_WID - SCREEN_WID;
        morgy_panel_x = j;
    }

    /* Hack -- Force the monster to be sitting on a floor
     * grid unless that monster can pass through walls
     */
    if (!rf_has(r_ptr->flags, RF_PASS_WALL)) {
        borg_grids[kill->pos.y][kill->pos.x].feat = FEAT_FLOOR;
    }

    /* Hack -- Force the ghostly monster to be in a wall
     * grid until the grid is proven to be something else
     */
    if (borg_grids[kill->pos.y][kill->pos.x].feat != FEAT_FLOOR
        && rf_has(r_ptr->flags, RF_PASS_WALL)) {
        borg_grids[kill->pos.y][kill->pos.x].feat = FEAT_GRANITE;
    }
}

/*
 * Delete an old "kill" record
 */
void borg_delete_kill(int i)
{
    borg_kill *kill = &borg_kills[i];

    /* Paranoia -- Already wiped */
    if (!kill->r_idx)
        return;

    /* Note */
    borg_note(format("# Forgetting a monster '%s' at (%d,%d)",
        (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x));

    /* Clear goals if I am flowing to this monster.*/
    if (borg.goal.type == GOAL_KILL && borg_flow_y[0] == kill->pos.y
        && borg_flow_x[0] == kill->pos.x)
        borg.goal.type = 0;

    /* Update the grids */
    borg_grids[kill->pos.y][kill->pos.x].kill = 0;

    /* save a time stamp of when the last multiplier was killed */
    if (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))
        borg.when_last_kill_mult = borg_t;

    /* Kill the monster */
    memset(kill, 0, sizeof(borg_kill));

    /* One less monster */
    borg_kills_cnt--;

    /* Recalculate danger */
    borg_danger_wipe = true;
}

/*
 * Force sleep onto a "kill" record
 * ??? Since this check is done at update_kill should I have it here?
 */
void borg_sleep_kill(int i)
{
    borg_kill *kill = &borg_kills[i];

    /* Paranoia -- Already wiped */
    if (!kill->r_idx)
        return;

    /* Note */
    borg_note(format("# Noting sleep on a monster '%s' at (%d,%d)",
        (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x));

    /* note sleep */
    kill->awake = false;

    /* Wipe flow goals */
    borg.goal.type = 0;

    /* Recalculate danger */
    borg_danger_wipe = true;
}

/*
 * Determine if a monster should be "viewable"
 */
static bool borg_follow_kill_aux(int i, int y, int x)
{
    int d;

    borg_grid *ag;

    borg_kill *kill            = &borg_kills[i];

    struct monster_race *r_ptr = &r_info[kill->r_idx];

    /* Distance to player */
    d = distance(borg.c, loc(x, y));

    /* Too far away */
    if (d > z_info->max_sight)
        return false;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Not on-screen */
    if (!(ag->info & BORG_OKAY))
        return false;

    /* Line of sight */
    if (ag->info & BORG_VIEW) {
        /* Use "illumination" */
        if (ag->info & (BORG_LIGHT | BORG_GLOW)) {
            /* We can see invisible */
            if (borg.trait[BI_SINV] || borg.see_inv)
                return true;

            /* Monster is not invisible */
            if (!(rf_has(r_ptr->flags, RF_INVISIBLE)))
                return true;
        }

        /* Use "infravision" */
        if (d <= borg.trait[BI_INFRA]) {
            /* Infravision works on "warm" creatures */
            if (!(rf_has(r_info->flags, RF_COLD_BLOOD)))
                return true;
        }
    }

    /* Telepathy requires "telepathy" */
    if (borg.trait[BI_ESP]) {
        /* Telepathy fails on "strange" monsters */
        if (rf_has(r_info->flags, RF_EMPTY_MIND))
            return false;
        if (rf_has(r_info->flags, RF_WEIRD_MIND))
            return false;

        /* Success */
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Attempt to "follow" a missing monster
 *
 * This routine is not called when the player is blind or hallucinating.
 *
 * Currently this function is a total hack, but handles the case of only
 * one possible location (taking it), two adjacent possible locations
 * (taking the diagonal one), and three adjacent locations with the
 * central one being a diagonal (taking the diagonal one).
 *
 * We should perhaps handle the case of three adjacent locations with
 * the central one being a non-diagonal (taking the non-diagonal one).
 *
 * We should perhaps attempt to take into account "last known direction",
 * which would allow us to "predict" motion up to walls, and we should
 * perhaps attempt to take into account the "standard" flee algorithm,
 * though that feels a little like cheating.
 */
void borg_follow_kill(int i)
{
    int j;
    int x, y;
    int ox, oy;

    int dx, b_dx = 0;
    int dy, b_dy = 0;

    borg_grid *ag;

    borg_kill *kill = &borg_kills[i];

    /* Paranoia */
    if (!kill->r_idx)
        return;

    /* Old location */
    ox = kill->pos.x;
    oy = kill->pos.y;

    /* Out of sight */
    if (!borg_follow_kill_aux(i, oy, ox))
        return;

    /* Note */
    borg_note(format("# There was a monster '%s' at (%d,%d)",
        (r_info[kill->r_idx].name), oy, ox));

    /* Prevent silliness */
    if (!borg_cave_floor_bold(oy, ox)) {
        /* Delete the monster */
        borg_delete_kill(i);

        /* Done */
        return;
    }

    /* Prevent loops */
    if (randint1(100) < 1) {
        /* Just delete the monster */
        borg_delete_kill(i);

        /* Done */
        return;
    }

    /* prevent overflows */
    if (borg_t > 20000) {
        /* Just delete the monster */
        borg_delete_kill(i);

        /* Done */
        return;
    }

    /* Some never move, no reason to follow them */
    if ((rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE)) ||
        /* Some are sleeping and don't move, no reason to follow them */
        (kill->awake == false)) {
        /* delete them if they are under me */
        if (kill->pos.y == borg.c.y && kill->pos.x == borg.c.x) {
            borg_delete_kill(i);
        }
        /* Don't 'forget' certain ones */
        return;
    }

    /* Scan locations */
    for (j = 0; j < 8; j++) {
        /* Access offset */
        dx = ddx_ddd[j];
        dy = ddy_ddd[j];

        /* Access location */
        x = ox + dx;
        y = oy + dy;

        /* legal */
        if (!square_in_bounds_fully(cave, loc(x, y)))
            continue;

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Skip known walls and doors */
        if (!borg_cave_floor_grid(ag))
            continue;

        /* Skip known monsters */
        if (ag->kill)
            continue;

        /* Skip visible grids */
        if (borg_follow_kill_aux(i, y, x))
            continue;

        /* Collect the offsets */
        b_dx += dx;
        b_dy += dy;
    }

    /* Don't go too far */
    if (b_dx < -1)
        b_dx = -1;
    else if (b_dx > 1)
        b_dx = 1;

    /* Don't go too far */
    if (b_dy < -1)
        b_dy = -1;
    else if (b_dy > 1)
        b_dy = 1;

    /* Access location */
    x = ox + b_dx;
    y = oy + b_dy;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Avoid walls and doors */
    if (!borg_cave_floor_grid(ag)) {
        /* Just delete the monster */
        borg_delete_kill(i);

        /* Done */
        return;
    }

    /* Avoid monsters */
    if (ag->kill != i) {
        /* Just delete the monster */
        borg_delete_kill(i);

        /* Done */
        return;
    }

    /* Delete monsters that did not really move */
    if (kill->pos.y == oy && kill->pos.x == ox) {
        borg_delete_kill(i);
        return;
    }

    /* Update the grids */
    borg_grids[kill->pos.y][kill->pos.x].kill = 0;

    /* Save the old Location */
    kill->ox = ox;
    kill->oy = oy;

    /* Save the Location */
    kill->pos.x = ox + b_dx;
    kill->pos.y = oy + b_dy;

    /* Update the grids */
    borg_grids[kill->pos.y][kill->pos.x].kill = i;

    /* Note */
    borg_note(format("# Following a monster '%s' to (%d,%d) from (%d,%d)",
        (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x, oy, ox));

    /* Recalculate danger */
    borg_danger_wipe = true;

    /* Clear goals */
    if ((!borg.trait[BI_ESP] && borg.goal.type == GOAL_KILL
            && (borg_flow_y[0] == kill->pos.y && borg_flow_x[0] == kill->pos.x))
        || (borg.goal.type == GOAL_TAKE && borg.munchkin_mode))
        borg.goal.type = 0;
}

/*
 * Obtain a new "kill" index
 */
static int borg_new_kill(unsigned int r_idx, int y, int x)
{
    int i, n = -1;

    borg_kill           *kill;
    borg_grid           *ag;
    struct monster_race *r_ptr;
    struct monster      *m_ptr;

    /* Look for a "dead" monster */
    for (i = 1; (n < 0) && (i < borg_kills_nxt); i++) {
        /* Skip real entries */
        if (!borg_kills[i].r_idx)
            n = i;
    }

    /* Allocate a new monster */
    if ((n < 0) && (borg_kills_nxt < 255)) {
        /* Acquire the entry, advance */
        n = borg_kills_nxt++;
    }

    /* Hack -- steal an old monster */
    if (n < 0) {
        /* Note */
        borg_note("# Too many monsters");

        /* Hack -- Pick a random monster */
        n = randint1(borg_kills_nxt - 1) + 1;

        /* Kill it */
        borg_delete_kill(n);
    }

    /* it might be that it can't be found */
    m_ptr = square_monster(cave, loc(x, y));
    if (!m_ptr)
        return -1;

    /* Count the monsters */
    borg_kills_cnt++;

    /* Access the monster */
    kill  = &borg_kills[n];
    r_ptr = &r_info[kill->r_idx];
    ag    = &borg_grids[y][x];

    /* Save the race */
    kill->r_idx = r_idx;

    /* Location */
    kill->ox = kill->pos.x = x;
    kill->oy = kill->pos.y = y;

    /* Games Index of the monster */
    kill->m_idx = m_ptr->midx;

    /* Update the grids */
    borg_grids[kill->pos.y][kill->pos.x].kill = n;

    /* Timestamp */
    kill->when = borg_t;

    /* Mark the Morgoth time stamp if needed */
    if (kill->r_idx == borg_morgoth_id)
        borg_t_morgoth = borg_t;

    /* Update the monster */
    borg_update_kill_new(n);

    /* Update the monster */
    borg_update_kill_old(n);

    /* Note (r_info[kill->r_idx].name)*/
    borg_note(format(
        "# Creating a monster '%s' at (%d,%d), HP: %d, Time: %d, Index: %d",
        (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x, kill->power,
        kill->when, kill->r_idx));

    /* Recalculate danger */
    borg_danger_wipe = true;

    /* Remove Regional Fear which may have been induced from a non-LOS monster.
     * We assume this newly created monster is the one which induced our
     * Regional Fear.  If it wasn't, then the borg will create new Regional Fear
     * next time the unseen monster attacks.  There is no harm done by clearing
     * these. At most, he may end up resting in an area for 1 turn */
    if (borg_t < borg.need_see_invis + 5) {
        int y0, x0, y1, x1, y2, x2;

        y0 = (borg.c.y / 11);
        x0 = (borg.c.x / 11);

        /* Nearby regions */
        y1 = (y0 > 0) ? (y0 - 1) : 0;
        x1 = (x0 > 0) ? (x0 - 1) : 0;
        y2 = (x0 < 5) ? (x0 + 1) : 5;
        x2 = (x0 < 17) ? (x0 + 1) : 17;

        /* Remove "fear", spread around */
        borg_fear_region[y0][x0] = 0;
        borg_fear_region[y0][x1] = 0;
        borg_fear_region[y0][x2] = 0;
        borg_fear_region[y1][x0] = 0;
        borg_fear_region[y2][x0] = 0;
        borg_fear_region[y1][x1] = 0;
        borg_fear_region[y1][x2] = 0;
        borg_fear_region[y2][x1] = 0;
        borg_fear_region[y2][x2] = 0;
        borg_note(format("# Removing Regional Fear (%d,%d) because of a LOS %s",
            y, x, r_info[kill->r_idx].name));
    }

    /* Wipe goals only if I have some light source */
    if (borg.trait[BI_CURLITE]
        && borg_los(kill->pos.y, kill->pos.x, borg.c.y, borg.c.x))
        borg.goal.type = 0;

    /* Hack -- Force the monster to be sitting on a floor
     * grid unless that monster can pass through walls
     */
    if (!(rf_has(r_ptr->flags, RF_PASS_WALL))) {
        ag->feat = FEAT_FLOOR;
    }

    /* Hack -- Force the ghostly monster to be in a wall
     * grid until the grid is proven to be something else
     */
    if (rf_has(r_ptr->flags, RF_PASS_WALL)) {
        ag->feat = FEAT_GRANITE;
    }

    /* Count up out list of Nasties */
    for (i = 0; i < borg_nasties_num; i++) {
        /* Count them up */
        if (r_info[kill->r_idx].d_char == borg_nasties[i])
            borg_nasties_count[i]++;
    }

    /* Return the monster */
    return n;
}

/*
 * Attempt to guess what type of monster is at the given location.
 *
 * If we are unable to think of any monster that could be at the
 * given location, we will assume the monster is a player ghost.
 * This is a total hack, but may prevent crashes.
 *
 * The guess can be improved by the judicious use of a specialized
 * "attr/char" mapping, especially for unique monsters.  Currently,
 * the Borg does not stoop to such redefinitions.
 *
 * We will probably fail to identify "trapper" and "lurker" monsters,
 * since they look like whatever they are standing on.  Now we will
 * probably just assume they are player ghosts.  XXX XXX XXX
 *
 * Note that "town" monsters may only appear in town, and in "town",
 * only "town" monsters may appear, unless we summon or polymorph
 * a monster while in town, which should never happen.
 *
 * To guess which monster is at the given location, we consider every
 * possible race, keeping the race (if any) with the best "score".
 *
 * Certain monster races are "impossible", including town monsters
 * in the dungeon, dungeon monsters in the town, unique monsters
 * known to be dead, monsters more than 50 levels out of depth,
 * and monsters with an impossible char, or an impossible attr.
 *
 * Certain aspects of a monster race are penalized, including extreme
 * out of depth, minor out of depth, clear/multihued attrs.
 *
 * Certain aspects of a monster race are rewarded, including monsters
 * that appear in groups, monsters that reproduce, monsters that have
 * been seen on this level a lot.
 *
 * We are never called for "trapper", "lurker", or "mimic" monsters.
 *
 * The actual rewards and penalties probably need some tweaking.
 *
 * Hack -- try not to choose "unique" monsters, or we will flee a lot.
 */
static unsigned int borg_guess_race(
    uint8_t a, wchar_t c, bool multi, int y, int x)
{
    /*  ok, this is an real cheat.  he ought to use the look command
     * in order to correctly id the monster.  but i am passing that up for
     * the sake of speed
     */
    struct monster *m_ptr;
    m_ptr = square_monster(cave, loc(x, y));

    if (!m_ptr)
        return 0;

    /* Actual monsters */
    return (m_ptr->race->ridx);
}

/*
 * Attempt to notice a changing "kill"
 */
bool observe_kill_diff(int y, int x, uint8_t a, wchar_t c)
{
    int          i;
    unsigned int r_idx;

    borg_kill *kill;

    /* Guess the race */
    r_idx = borg_guess_race(a, c, false, y, x);

    /* Oops */
    if (!r_idx)
        return false;

    /* no new monsters if hallucinations */
    if (borg.trait[BI_ISIMAGE])
        return false;

    /* Create a new monster */
    i = borg_new_kill(r_idx, y, x);

    /* Get the object */
    kill = &borg_kills[i];

    /* Timestamp */
    kill->when = borg_t;

    /* Mark the Morgoth time stamp if needed */
    if (kill->r_idx == borg_morgoth_id)
        borg_t_morgoth = borg_t;

    /* Done */
    return true;
}

/*
 * Attempt to "track" a "kill" at the given location
 * Assume that the monster moved at most 'd' grids.
 * If "flag" is true, allow monster "conversion"
 */
bool observe_kill_move(int y, int x, int d, uint8_t a, wchar_t c, bool flag)
{
    int                  i, z, ox, oy;
    unsigned int         r_idx;
    borg_kill           *kill;
    struct monster_race *r_ptr;

    bool flicker = false;

    /* Look at the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Skip assigned monsters */
        if (kill->seen)
            continue;

        /* Old location */
        ox = kill->pos.x;
        oy = kill->pos.y;

        /* Calculate distance */
        z = borg_distance(oy, ox, y, x);

        /* Access the monster race */
        r_ptr = &r_info[kill->r_idx];

        /* Double the distance for uniques so the borg does not create 2 of the
         * same unique monster on the level */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            d = d * 2;

        /* Verify distance */
        if (z > d)
            continue;

        /* Verify that we are looking at the right one */
        if (kill->m_idx != square_monster(cave, loc(x, y))->midx)
            continue;

        /* Verify "reasonable" motion, if allowed */
        if (!flag && (z > (kill->moves / 10) + 1))
            continue;

        /* Verify matching char so long as not hallucinating */
        if (!borg.trait[BI_ISIMAGE] && c != r_ptr->d_char)
            continue;

        /* Verify matching attr so long as not hallucinating */
        if (a != r_ptr->d_attr || borg.trait[BI_ISIMAGE]) {
            /* Require matching attr (for normal monsters) */
            if (!rf_has(r_ptr->flags, RF_ATTR_MULTI)
                && !rf_has(r_ptr->flags, RF_ATTR_CLEAR)) {
                /* Require flag */
                if (!flag)
                    continue;

                /* Never flicker known monsters */
                if (kill->known)
                    continue;

                /* Find a multi-hued monster */
                r_idx = borg_guess_race(a, c, true, y, x);

                /* Handle failure */
                if (!r_idx)
                    continue;

                /* Note */
                borg_note(format("# Flickering monster '%s' at (%d,%d)",
                    (r_info[r_idx].name), y, x));

                /* Note */
                borg_note(format("# Converting a monster '%s' at (%d,%d)",
                    (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x));

                /* Change the race */
                kill->r_idx = r_idx;

                /* Monster flickers */
                flicker = true;

                /* Recalculate danger */
                borg_danger_wipe = true;

                /* Clear monster flow goals */
                borg.goal.type = 0;
            }
        }

        /* Actual movement */
        if (z) {

            /* Update the grids */
            borg_grids[kill->pos.y][kill->pos.x].kill = 0;

            /* Save the old Location */
            kill->ox = kill->pos.x;
            kill->oy = kill->pos.y;

            /* Save the Location */
            kill->pos.x = x;
            kill->pos.y = y;

            /* Update the grids */
            borg_grids[kill->pos.y][kill->pos.x].kill = i;

            /* Note */
            borg_note(
                format("# Tracking a monster '%s' at (%d,%d) from (%d,%d)",
                    (r_ptr->name), kill->pos.y, kill->pos.x, oy, ox));

            /* Recalculate danger */
            borg_danger_wipe = true;

            /* Clear goals */
            if ((!borg.trait[BI_ESP] && borg.goal.type == GOAL_KILL
                    && (borg_flow_y[0] == kill->pos.y
                        && borg_flow_x[0] == kill->pos.x))
                || (borg.goal.type == GOAL_TAKE && borg.munchkin_mode))
                borg.goal.type = 0;
        }

        /* Note when last seen */
        kill->when = borg_t;

        /* Mark the Morgoth time stamp if needed */
        if (kill->r_idx == borg_morgoth_id)
            borg_t_morgoth = borg_t;

        /* Monster flickered */
        if (flicker) {
            /* Update the monster */
            borg_update_kill_new(i);
        }

        /* Update the monster */
        borg_update_kill_old(i);

        /* Mark as seen */
        kill->seen = true;

        /* Done */
        return true;
    }

    /* Oops */
    return false;
}

/*
 * Attempt to convert a monster name into a race index
 *
 * First we check for all possible "unique" monsters, including
 * ones we have killed, and even if the monster name is "prefixed"
 * (as in "The Tarrasque" and "The Lernean Hydra").  Since we use
 * a fast binary search, this is acceptable.
 *
 * Otherwise, if the monster is NOT named "The xxx", we assume it
 * must be a "player ghost" (which is impossible).
 *
 * Then, we do a binary search on all "normal" monster names, using
 * a search which is known to find the last matching entry, if one
 * exists, and otherwise to find an entry which would follow the
 * matching entry if there was one, unless the matching entry would
 * follow all the existing entries, in which case it will find the
 * final entry in the list.  Thus, we can search *backwards* from
 * the result of the search, and know that we will access all of
 * the matching entries, as long as we stop once we find an entry
 * which does not match, since this will catch all cases above.
 *
 * Finally, we assume the monster must be a "player ghost" (which
 * as noted above is impossible), which is a hack, but may prevent
 * crashes, even if it does induce strange behavior.
 */
static unsigned int borg_guess_race_name(char *who)
{
    int k, m, n;

    int i, b_i = 0;
    int s, b_s = 0;

    struct monster_race *r_ptr;

    char partial[160];
    int  len = strlen(who);

    /* Start the search */
    m = 0;
    n = borg_unique_size;

    /* Binary search */
    while (m < n - 1) {
        /* Pick a "middle" entry */
        i = (m + n) / 2;

        /* Search to the right (or here) */
        if (strcmp(borg_unique_text[i], who) <= 0) {
            m = i;
        }

        /* Search to the left */
        else {
            n = i;
        }
    }

    /* Check for equality */
    if (streq(who, borg_unique_text[m])) {
        /* Use this monster */
        return (borg_unique_what[m]);
    }

    /* Hack -- handle "offscreen" */
    if (suffix(who, " (offscreen)")) {
        /* Remove the suffix */
        my_strcpy(partial, who, sizeof(partial));
        partial[len - 12] = '\0';
        who               = partial;

        /* Message */
        borg_note(format("# Handling offscreen monster (%s)", who));
    }

    /* Assume player ghost */
    if (!prefix(who, "The ") && !prefix(who, "the ")) {
        /* Message */
        borg_note(format("# Assuming player ghost (%s) (a)", who));

        /* Oops */
        return (z_info->r_max - 1);
    }

    /* Skip the prefix */
    who += 4;

    /* Start the search */
    m = 0;
    n = borg_normal_size;

    /* Binary search */
    while (m < n - 1) {
        /* Pick a "middle" entry */
        i = (m + n) / 2;

        /* Search to the right (or here) */
        if (strcmp(borg_normal_text[i], who) <= 0) {
            m = i;
        }

        /* Search to the left */
        else {
            n = i;
        }
    }

    /* Scan possibilities */
    for (k = m; k >= 0; k--) {
        /* Stop when done */
        if (!streq(who, borg_normal_text[k]))
            break;

        /* Extract the monster */
        i = (int)borg_normal_what[k];

        /* Access the monster */
        r_ptr = &r_info[i];

        /* Basic score */
        s = 1000;

        /* Penalize "depth miss" */
        s = s - ABS(r_ptr->level - borg.trait[BI_CDEPTH]);

        /* Track best */
        if (b_i && (s < b_s))
            continue;

        /* Track it */
        b_i = i;
        b_s = s;
    }

    /* Success */
    if (b_i)
        return ((unsigned int)b_i);

    /* No match found */
    borg_note(format("# Assuming player ghost (%s)(b)", who));

    /* Oops */
    return (z_info->r_max - 1);
}

/*
 * Attempt to locate a monster which could explain a message involving
 * the given monster name, near the given location, up to the given
 * distance from the given location.
 *
 * Invisible monsters, bizarre monsters, and unexplainable monsters are
 * all treated the same way, and should eventually contribute some amount
 * of basic "fear" to the current region.
 *
 * First, we attempt to convert "similar" objects into the desired monster,
 * then we attempt to convert "similar" monsters into the desired monster,
 * then we attempt to match an existing monster, and finally, we give up.
 *
 * XXX XXX XXX Hack -- To prevent fatal situations, every time we think
 * there may be a monster nearby, we look for a nearby object which could
 * be the indicated monster, and convert it into that monster.  This allows
 * us to correctly handle a room full of multiplying clear mushrooms.
 *
 * XXX XXX XXX When surrounded by multiple monsters of the same type,
 * we will ascribe most messages to one of those monsters, and ignore
 * the existence of all the other similar monsters.
 *
 * XXX XXX XXX Currently, confusion may cause messages to be ignored.
 */
int borg_locate_kill(char *who, struct loc c, int r)
{
    int          i, d;
    unsigned int r_idx;

    int b_i, b_d;

    borg_take *take;
    borg_kill *kill;

    struct object_kind *k_ptr;

    struct monster_race *r_ptr;

    /* Handle invisible monsters */
    if (my_strnicmp(who, "Something", 9) == 0
        || my_strnicmp(who, "It", 2) == 0) {
        /* Note */
        borg_note("# Invisible monster nearby.");
        /* if I can, cast detect inviso--time stamp it
         * We stamp it now if we can, or later if we just did the spell
         * That way we dont loop casting the spell.
         */
        /* detect invis spell not working right, for now just shift panel
         * and cast a light beam if in a hallway and we have see_inv*/
        if (borg.need_see_invis < (borg_t)) {
            borg.need_see_invis = (borg_t);
        }

        /* Ignore */
        return 0;
    }

    /* Handle offsreen monsters */
    if (suffix(who, " (offscreen)")) {
        /* Note */
        borg_note("# Offscreen monster nearby");

        /* Shift the panel */
        borg.need_shift_panel = true;

        /* Ignore */
        return 0;
    }

    /* Guess the monster race */
    r_idx = borg_guess_race_name(who);

    /* Access the monster race */
    r_ptr = &r_info[r_idx];

    /* Note */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("# There is a monster '%s' within %d grids of %d,%d",
            (r_ptr->name), r, c.y, c.x));

    /* Hack -- count racial appearances */
    if (borg_race_count[r_idx] < SHRT_MAX)
        borg_race_count[r_idx]++;

    /* Handle trappers and lurkers and mimics */
    if (rf_has(r_ptr->flags, RF_CHAR_CLEAR)) {
        /* Note */
        borg_note("# Bizarre monster nearby");
    }

    /*** Hack -- Find a similar object ***/

    /* Nothing yet */
    b_i = -1;
    b_d = 999;

    /* Scan the objects */
    for (i = 1; i < borg_takes_nxt; i++) {
        take = &borg_takes[i];

        /* Skip "dead" objects */
        if (!take->kind)
            continue;

        /* Access kind */
        k_ptr = take->kind;

        /* Verify char */
        if (k_ptr->d_char != r_ptr->d_char)
            continue;

        /* Verify attr (unless clear or multi-hued) */
        if (!rf_has(r_ptr->flags, RF_ATTR_MULTI)
            && !rf_has(r_ptr->flags, RF_ATTR_CLEAR)) {
            /* Verify attr (unless flavored) */
            if (!(k_ptr->flavor)) {
                /* Verify attr */
                if (k_ptr->d_attr != r_ptr->d_attr)
                    continue;
            }
        }

        /* Calculate distance */
        d = distance(loc(take->x, take->y), c);

        /* Skip "wrong" objects */
        if (d > r)
            continue;

        /* Track closest one */
        if (d > b_d)
            continue;

        /* Track it */
        b_i = i;
        b_d = d;
    }

    /* Found one */
    if (b_i >= 0) {
        take = &borg_takes[b_i];

        /* Note */
        borg_note(format("# Converting an object '%s' at (%d,%d)",
            (take->kind->name), take->y, take->x));

        /* Save location */
        c.x = take->x;
        c.y = take->y;

        /* Delete the object */
        borg_delete_take(b_i);

        /* Make a new monster */
        b_i = borg_new_kill(r_idx, c.y, c.x);
        if (b_i < 0)
            return b_i;

        /* Get the monster */
        kill = &borg_kills[b_i];

        /* Timestamp */
        kill->when = borg_t;

        /* Mark the Morgoth time stamp if needed */
        if (kill->r_idx == borg_morgoth_id)
            borg_t_morgoth = borg_t;

        /* Known identity */
        if (!r)
            kill->known = true;

        /* Return the index */
        return b_i;
    }

    /*** Hack -- Find a similar monster ***/

    /* Nothing yet */
    b_i = -1;
    b_d = 999;

    /* Scan the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        kill = &borg_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx)
            continue;

        /* Skip "matching" monsters */
        if (kill->r_idx == r_idx)
            continue;

        /* check the position is the same */
        struct monster *m_ptr = &cave->monsters[kill->m_idx];
        if (m_ptr->grid.x != kill->pos.x || m_ptr->grid.y != kill->pos.y)
            continue;

        /* Verify char */
        if (r_info[kill->r_idx].d_char != r_ptr->d_char)
            continue;

        /* Verify attr (unless clear or multi-hued) */
        if (!rf_has(r_ptr->flags, RF_ATTR_MULTI)
            && !rf_has(r_ptr->flags, RF_ATTR_CLEAR)) {
            /* Verify attr */
            if (r_info[kill->r_idx].d_attr != r_ptr->d_attr)
                continue;
        }

        /* Distance away */
        d = distance(kill->pos, c);

        /* Check distance */
        if (d > r)
            continue;

        /* Track closest one */
        if (d > b_d)
            continue;

        /* Track it */
        b_i = i;
        b_d = d;
    }

    /* Found one */
    if (b_i >= 0) {
        kill = &borg_kills[b_i];

        /* Note */
        borg_note(format("# Converting a monster '%s' at (%d,%d)",
            (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x));

        /* Change the race */
        kill->r_idx = r_idx;

        /* Update the monster */
        borg_update_kill_new(b_i);

        /* Update the monster */
        borg_update_kill_old(b_i);

        /* Known identity */
        if (!r)
            kill->known = true;

        /* Recalculate danger */
        borg_danger_wipe = true;

        /* Clear goals */
        borg.goal.type = 0;

        /* Index */
        return b_i;
    }

    /*** Hack -- Find an existing monster ***/

    /* Nothing yet */
    b_i = -1;
    b_d = 999;

    /* Scan the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        kill = &borg_kills[i];

        /* Skip "dead" monsters */
        if (!kill->r_idx)
            continue;

        /* Skip "different" monsters */
        if (kill->r_idx != r_idx)
            continue;

        /* Distance away */
        d = distance(kill->pos, c);

        /* Check distance */
        if (d > r + 3)
            continue;

        /* Hopefully this will add fear to our grid */
        if (!borg_projectable(kill->pos.y, kill->pos.x, c.y, c.x))
            continue;

        /* Track closest one */
        if (d > b_d)
            continue;

        /* Track it */
        b_i = i;
        b_d = d;
    }

    /*** Hack -- Find an existing monster Last Chance ***/
    /* Note:
     * There can be some problems with monsters that use melee
     * attack.  The range (r) will be 1.  But the known monster
     * may be a few paces further and moved closely and attacked
     * in the same round.  If this is the case, we miss them the
     * first pass then end up Ignoring them.
     */
    if (b_i == -1) {
        /* Nothing yet */
        b_i = -1;
        b_d = 999;

        /* Scan the monsters */
        for (i = 1; i < borg_kills_nxt; i++) {
            kill = &borg_kills[i];

            /* Skip "dead" monsters */
            if (!kill->r_idx)
                continue;

            /* Skip "different" monsters */
            if (kill->r_idx != r_idx)
                continue;

            /* Distance away */
            d = distance(kill->pos, c);

            /* Check distance */
            /* Note:
             * There can be some problems with monsters that use melee
             * attack.  The range (r) will be 1.  But the known monster
             * may be a few paces further and moved closely and attacked
             * in the same round.  If this is the case, we miss them the
             * first pass then end up Ignoring them.
             * We extend the range of the search*/
            if (d > r + 20)
                continue;

            /* Track closest one */
            if (d > b_d)
                continue;

            /* Track it */
            b_i = i;
            b_d = d;
        }
    }

    /* Found one */
    if (b_i >= 0) {
        kill = &borg_kills[b_i];

        /* Note */
        if (borg_cfg[BORG_VERBOSE])
            borg_note(format(
                "# Matched a monster '%s' at (%d,%d) for the parsed msg.",
                (r_info[kill->r_idx].name), kill->pos.y, kill->pos.x));

        /* Known identity */
        if (!r)
            kill->known = true;

        /* Index */
        return b_i;
    }

    /*** Oops ***/

    /* Note */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("# Unable to locate monster '%s' near (%d,%d), which "
                         "generated the msg (%s).",
            (r_ptr->name), c.y, c.x, who));

    /* Oops */
    /* this is the case where we know the name of the monster */
    /* but cannot locate it on the monster list. */
    return (-1);
}

/*
 * Notice the "death" of a monster
 */
void borg_count_death(int i)
{
    borg_kill *kill = &borg_kills[i];

    if (kill->r_idx) {
        /* Hack -- count racial deaths */
        if (borg_race_death[kill->r_idx] < SHRT_MAX)
            borg_race_death[kill->r_idx]++;

        /* if it was a unique then remove the unique_on_level flag */
        if (rf_has(r_info[kill->r_idx].flags, RF_UNIQUE))
            unique_on_level = 0;
    }
}

/*
 * Prepare to "flow" towards monsters to "kill"
 * But in a few phases, viewable, near and far.
 * Note that monsters under the player are always deleted
 */
bool borg_flow_kill(bool viewable, int nearness)
{
    int i, x, y, p, j, b_j = -1;
    int b_stair       = -1;

    bool borg_in_hall = false;
    int  hall_y, hall_x, hall_walls = 0;
    bool skip_monster = false;

    borg_grid *ag;

    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt)
        return false;

    /* Don't chase down town monsters when you are just starting out */
    if (borg.trait[BI_CDEPTH] == 0 && borg.trait[BI_CLEVEL] < 20)
        return false;

    /* YOU ARE NOT A WARRIOR!! DON'T ACT LIKE ONE!! */
    if ((borg.trait[BI_CLASS] == CLASS_MAGE
            || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        && borg.trait[BI_CLEVEL] < (borg.trait[BI_CDEPTH] ? 35 : 25))
        return false;

    /* Not if Weak from hunger or no food */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK]
        || borg.trait[BI_FOOD] == 0)
        return false;

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position)
        return false;

    /* Nothing found */
    borg_temp_n = 0;

    /* check to see if in a hall, used later */
    for (hall_x = -1; hall_x <= 1; hall_x++) {
        for (hall_y = -1; hall_y <= 1; hall_y++) {
            /* Acquire location */
            x  = hall_x + borg.c.x;
            y  = hall_y + borg.c.y;

            ag = &borg_grids[y][x];

            /* track walls */
            if ((ag->glyph)
                || ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_PERM))) {
                hall_walls++;
            }

            /* addem up */
            if (hall_walls >= 5)
                borg_in_hall = true;
        }
    }

    /* Check distance away from stairs, used later */

    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++) {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = distance(borg.c, loc(x, y));

        /* skip the closer ones */
        if (b_j >= j)
            continue;

        /* track it */
        b_j     = j;
        b_stair = i;
    }

    /* Scan the monster list */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];
        int        x9   = kill->pos.x;
        int        y9   = kill->pos.y;
        int        ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* don't bother flowing to an adjacent monster when I am afraid */
        if (d == 1 && (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR]))
            continue;

        /* Ignore multiplying monsters */
        if (borg.goal.ignoring && !borg.trait[BI_ISAFRAID]
            && (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY)))
            continue;

        /* Ignore molds when low level */
        if (borg.trait[BI_MAXCLEVEL] < 10
            && (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE)))
            continue;

        /* Avoid flowing to a fight if a scary guy is on the level */
        if (scaryguy_on_level)
            continue;

        /* Avoid multiplying monsters when low level */
        if (borg.trait[BI_CLEVEL] < 10
            && (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY)))
            continue;

        /* Hack -- ignore Maggot until later.  Player will chase Maggot
         * down all across the screen waking up all the monsters.  Then
         * he is stuck in a compromised situation.
         */
        if ((rf_has(r_info[kill->r_idx].flags, RF_UNIQUE))
            && borg.trait[BI_CDEPTH] == 0 && borg.trait[BI_CLEVEL] < 5)
            continue;

        /* Access the location */
        x = kill->pos.x;
        y = kill->pos.y;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW))
            continue;

        /* Calculate danger */
        p = borg_danger(y, x, 1, true, false);

        /* Hack -- Skip "deadly" monsters unless uniques*/
        if (borg.trait[BI_CLEVEL] > 25 && (!rf_has(r_info->flags, RF_UNIQUE))
            && p > avoidance / 2)
            continue;
        if (borg.trait[BI_CLEVEL] <= 15 && p > avoidance / 3)
            continue;

        /* Skip ones that make me wander too far */
        if (b_stair != -1 && borg.trait[BI_CLEVEL] < 10) {
            /* Check the distance of this monster to the stair */
            j = borg_distance(
                track_less.y[b_stair], track_less.x[b_stair], y, x);
            /* skip far away monsters while I am close to stair */
            if (b_j <= borg.trait[BI_CLEVEL] * 5 + 9
                && j >= borg.trait[BI_CLEVEL] * 5 + 9)
                continue;
        }

        /* Hack -- Avoid getting surrounded */
        if (borg_in_hall && (rf_has(r_info[kill->r_idx].flags, RF_GROUP_AI))) {
            /* check to see if monster is in a hall, */
            for (hall_x = -1; hall_x <= 1; hall_x++) {
                for (hall_y = -1; hall_y <= 1; hall_y++) {
                    if (!square_in_bounds_fully(
                            cave, loc(hall_x + x, hall_y + y)))
                        continue;
                    ag = &borg_grids[hall_y + y][hall_x + x];

                    /* track walls */
                    if ((ag->glyph)
                        || ((ag->feat >= FEAT_MAGMA)
                            && (ag->feat <= FEAT_PERM))) {
                        hall_walls++;
                    }

                    /* we want the monster to be in a hall also
                     *
                     *  ########################
                     *  ############      S  ###
                     *  #         @'     SSS ###
                     *  # ##########       SS###
                     *  # #        #       Ss###
                     *  # #        ###### ######
                     *  # #             # #
                     * Currently, we would like the borg to avoid
                     * flowing to a situation like the one above.
                     * We would like him to stay in the hall and
                     * attack from a distance.  One problem is the
                     * lower case 's' in the corner, He will show
                     * up as being in a corner, and the borg may
                     * flow to it.  Let's hope that is a rare case.
                     *
                     * The borg might flow to the 'dark' south exit
                     * of the room.  This would be dangerous for
                     * him as well.
                     */
                    /* add 'em up */
                    if (hall_walls < 4) {
                        /* This monster is not in a hallway.
                         * It may not be safe to fight.
                         */
                        skip_monster = true;
                    }
                }
            }
        }

        /* Skip this one if it is just 2 grids from me and it can attack me as
         * soon as I move 1 grid closer to it.  Note that some monsters are
         * faster than me and it could still cover the 1 grid and hit me. I'll
         * fix it (based on my speed) later XXX
         */
        if (d == 2 && /* Spacing is important */
            (!(kill->ranged_attack)) && /* Ranged Attacks, don't rest. */
            (!(rf_has(r_info[kill->r_idx].flags,
                RF_NEVER_MOVE)))) /* Skip monsters that dont chase */
        {
            skip_monster = true;
        }

        /* skip certain ones */
        if (skip_monster)
            continue;

        /* don't go too far from the stairs */
        if (borg_flow_far_from_stairs(x, y, b_stair))
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to kill */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward monsters that we can see, make sure they */
    /* are at least easily reachable.  The second flag is whether or not */
    /* to avoid unknown squares.  This was for performance when we have ESP. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("kill", GOAL_KILL))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_KILL))
        return false;

    /* Success */
    return true;
}

/*
 * The borg can take a shot from a distance
 */
static bool borg_has_distance_attack(void)
{
    /* line up Magic Missile shots (covers Mages) */
    if (borg_attack_aux_spell_bolt(
            MAGIC_MISSILE, 0, 10, BORG_ATTACK_MISSILE, z_info->max_range, false)
        > 0)
        return true;

    /* line up Nether Bolt shots (covers Necromancers) */
    if (borg_attack_aux_spell_bolt(
            NETHER_BOLT, 0, 10, BORG_ATTACK_NETHER, z_info->max_range, false)
        > 0)
        return true;

    /* or arrows (covers warrior/ranger/paladins/rogues) */
    if (borg_attack_aux_launch() > 0)
        return true;

    /* not lining up Priests (OOD has area of effect, will line up more
     * naturally) */
    /* or Druids (Stinking cloud is area of effect again) */
    /* Blackguards should be doing HTH */

    return false;
}

/*
 * Take a couple of steps to line up a shot
 */
bool borg_flow_kill_aim(bool viewable)
{
    int o_y, o_x;
    int s_c_y = borg.c.y;
    int s_c_x = borg.c.x;
    int i;

    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt)
        return false;

    /* Sometimes we loop on this if we back  up to a point where */
    /* the monster is out of site */
    if (borg.time_this_panel > 500)
        return false;

    /* Not if Weak from hunger or no food */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK]
        || borg.trait[BI_FOOD] == 0)
        return false;

    /* If you can shoot from where you are, don't bother reaiming */
    if (borg_has_distance_attack())
        return false;

    /* Consider each adjacent spot */
    for (o_x = -2; o_x <= 2; o_x++) {
        for (o_y = -2; o_y <= 2; o_y++) {
            /* borg_attack would have already checked
               for a shot from where I currently am */
            if (o_x == 0 && o_y == 0)
                continue;

            /* XXX  Mess with where the program thinks the
               player is */
            borg.c.x = s_c_x + o_x;
            borg.c.y = s_c_y + o_y;

            /* avoid screen edges */
            if (borg.c.x > AUTO_MAX_X - 2 || borg.c.x < 2
                || borg.c.y > AUTO_MAX_Y - 2 || borg.c.y < 2)
                continue;

            /* Make sure we do not end up next to a monster */
            for (i = 0; i < borg_kills_nxt; i++) {
                if (distance(borg.c, borg_kills[i].pos) == 1)
                    break;
            }
            if (i != borg_kills_nxt)
                continue;

            /* Check for a distance attack from here */
            if (borg_has_distance_attack()) {
                /* Clear the flow codes */
                borg_flow_clear();

                /* Enqueue the grid */
                borg_flow_enqueue_grid(borg.c.y, borg.c.x);

                /* restore the saved player position */
                borg.c.x = s_c_x;
                borg.c.y = s_c_y;

                /* Spread the flow */
                borg_flow_spread(5, true, !viewable, false, -1, false);

                /* Attempt to Commit the flow */
                if (!borg_flow_commit("targetable position", GOAL_KILL))
                    return false;

                /* Take one step */
                if (!borg_flow_old(GOAL_KILL))
                    return false;

                return true;
            }
        }
    }

    /* restore the saved player position */
    borg.c.x = s_c_x;
    borg.c.y = s_c_y;

    return false;
}

/*
 * Dig an anti-summon corridor. Type I
 *
 *            ############## We want the borg to dig a tunnel which
 *            #............# limits the LOS of summoned monsters.
 *          ###............# It works better in hallways.
 *         ##@#............#
 *         #p##............# The borg will build an array of grids
 * ########## #######+###### near him.  Then look at specific patterns
 * #                  #      to find the good grids to excavate.
 * # ################ #
 *   #              # #
 * ###              # #
 *
 * Look at wall array to see if it is acceptable
 * We want to find this in the array:
 *
 * #####  ..@..  ####.  .####
 * ##.##  ##.##	 ##.#.  .#.##
 * #.#.#  #.#.#  #.#.@  @.#.#
 * ##.##  ##.##  ##.#.  .#.##
 * ..@..  #####  ####.  .####
 *
 * NORTH  SOUTH  WEST   East
 *
 */
bool borg_flow_kill_corridor(bool viewable)
{
    int o_y = 0;
    int o_x = 0;
    int m_x = 0;
    int m_y = 0;
    int b_y = 0, b_x = 0;
    int b_distance = 99;

    int  i;
    bool b_n        = false;
    bool b_s        = false;
    bool b_e        = false;
    bool b_w        = false;

    int n_array[25] = { 1, 0, 0, 0, 1, 
                        1, 0, 1, 0, 1, 
                        0, 1, 0, 1, 0, 
                        0, 0, 1, 0, 0, 
                        1, 1, 1, 1, 1 };
    int ny[25] = { -4, -4, -4, -4, -4, 
                   -3, -3, -3, -3, -3, 
                   -2, -2, -2, -2, -2,
                   -1, -1, -1, -1, -1, 
                    0,  0,  0,  0,  0 };
    int nx[25] = { -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2 };

    int s_array[25] = { 1, 1, 1, 1, 1, 
                        0, 0, 1, 0, 0, 
                        0, 1, 0, 1, 0, 
                        1, 0, 1, 0, 1, 
                        1, 0, 0, 0, 1 };
    int sy[25] = { 0, 0, 0, 0, 0, 
                   1, 1, 1, 1, 1, 
                   2, 2, 2, 2, 2, 
                   3, 3, 3, 3, 3,
                   4, 4, 4, 4, 4 };
    int sx[25] = { -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2, 
                   -2, -1,  0,  1,  2 };

    int e_array[25] = { 1, 0, 0, 1, 1, 
                        1, 0, 1, 0, 0, 
                        1, 1, 0, 1, 0, 
                        1, 0, 1, 0, 0, 
                        1, 0, 0, 1, 1 };
    int ey[25] = { -2, -2, -2, -2, -2, 
                   -1, -1, -1, -1, -1, 
                    0,  0,  0,  0,  0, 
                    1,  1,  1,  1,  1, 
                    2,  2,  2,  2,  2 };
    int ex[25] = { 0, 1, 2, 3, 4, 
                   0, 1, 2, 3, 4, 
                   0, 1, 2, 3, 4, 
                   0, 1, 2, 3, 4,
                   0, 1, 2, 3, 4 };

    int w_array[25] = { 1, 1, 0, 0, 1, 
                        0, 0, 1, 0, 1, 
                        0, 1, 0, 1, 1, 
                        0, 0, 1, 0, 1, 
                        1, 1, 0, 0, 1 };
    int wy[25] = { -2, -2, -2, -2, -2, 
                   -1, -1, -1, -1, -1, 
                    0,  0,  0,  0,  0, 
                    1,  1,  1,  1,  1, 
                    2,  2,  2,  2,  2 };
    int wx[25] = { -4, -3, -2, -1, 0, 
                   -4, -3, -2, -1, 0, 
                   -4, -3, -2, -1, 0, 
                   -4, -3, -2, -1, 0, 
                   -4, -3, -2, -1, 0 };

    int wall_north = 0;
    int wall_south = 0;
    int wall_east  = 0;
    int wall_west  = 0;
    int q_x;
    int q_y;

    borg_kill *kill;

    borg_digging = false;

    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt)
        return false;

    /* Only do this to summoners when they are close*/
    if (borg_kills_summoner == -1)
        return false;

    /* Hungry,starving */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* Sometimes we loop on this */
    if (borg.time_this_panel > 500)
        return false;

    /* Do not dig when confused */
    if (borg.trait[BI_ISCONFUSED])
        return false;

    /* Not when darkened */
    if (borg.trait[BI_CURLITE] == 0)
        return false;

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position)
        return false;
    if (borg_as_position)
        return false;

    /* get the summoning monster */
    kill = &borg_kills[borg_kills_summoner];

    /* Summoner must be mobile */
    if (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE))
        return false;
    /* Summoner must be able to pass through walls */
    if (rf_has(r_info[kill->r_idx].flags, RF_PASS_WALL))
        return false;
    if (rf_has(r_info[kill->r_idx].flags, RF_KILL_WALL))
        return false;

    /* Summoner has to be awake (so he will chase me */
    if (!kill->awake)
        return false;

    /* Must have Stone to Mud spell */
    if (!borg_spell_okay(TURN_STONE_TO_MUD) && !borg_spell_okay(SHATTER_STONE)
        && !borg_equips_ring(sv_ring_digging)
        && !borg_equips_item(act_stone_to_mud, true))
        return false;

    /* Summoner needs to be able to follow me.
     * So I either need to be able to
     * 1) have LOS on him or
     * 2) this panel needs to have had Magic Map or Wizard light cast on it.
     * If Mapped, then the flow codes needs to be used.
     */
    if (!borg_los(kill->pos.y, kill->pos.x, borg.c.y, borg.c.x)) {
        /* Extract panel */
        q_x = w_x / borg_panel_wid();
        q_y = w_y / borg_panel_hgt();

        if (borg_detect_wall[q_y + 0][q_x + 0] == true
            && borg_detect_wall[q_y + 0][q_x + 1] == true
            && borg_detect_wall[q_y + 1][q_x + 0] == true
            && borg_detect_wall[q_y + 1][q_x + 1] == true) {
            borg_flow_clear();
            borg_digging = true;
            borg_flow_enqueue_grid(kill->pos.y, kill->pos.x);
            borg_flow_spread(10, true, false, false, -1, false);
            if (!borg_flow_commit("Monster Path", GOAL_KILL))
                return false;
        } else {
            borg_flow_clear();
            borg_digging = true;
            borg_flow_enqueue_grid(kill->pos.y, kill->pos.x);
            borg_flow_spread(10, true, true, false, -1, false);
            if (!borg_flow_commit("Monster Path", GOAL_KILL))
                return false;
        }
    }

    /* NORTH -- Consider each area near the borg, looking for a good spot to
     * hide */
    for (o_y = -2; o_y < 1; o_y++) {
        /* Reset Wall count */
        wall_north = 0;

        /* No E-W offset when looking North-South */
        o_x = 0;

        for (i = 0; i < 25; i++) {
            borg_grid *ag;

            /* Check grids near borg */
            m_y = borg.c.y + o_y + ny[i];
            m_x = borg.c.x + o_x + nx[i];

            /* avoid screen edges */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y))) {
                continue;
            }

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (n_array[i] == 0
                && ((ag->feat == FEAT_NONE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* This is a good grid */
                wall_north++;
            }
            if (n_array[i] == 1
                && ((ag->feat <= FEAT_MORE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* A good wall would score 25. */
                wall_north++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_north == 25) {
            if (distance(
                    borg.c, loc(borg.c.x + o_x + nx[7], borg.c.y + o_y + ny[7]))
                < b_distance) {
                b_y        = o_y;
                b_x        = o_x;
                b_n        = true;
                b_distance = distance(borg.c,
                    loc(borg.c.x + o_x + nx[7], borg.c.y + o_y + ny[7]));
            }
        }
    }

    /* SOUTH -- Consider each area near the borg, looking for a good spot to
     * hide */
    for (o_y = -1; o_y < 2; o_y++) {
        /* Reset Wall count */
        wall_south = 0;

        for (i = 0; i < 25; i++) {
            borg_grid *ag;

            /* No lateral offset on South check */
            o_x = 0;

            /* Check grids near borg */
            m_y = borg.c.y + o_y + sy[i];
            m_x = borg.c.x + o_x + sx[i];

            /* avoid screen edges */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y)))
                continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (s_array[i] == 0
                && ((ag->feat == FEAT_NONE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* This is a good grid */
                wall_south++;
            }
            if (s_array[i] == 1
                && ((ag->feat <= FEAT_MORE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* A good wall would score 25. */
                wall_south++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_south == 25) {
            if (borg_distance(borg.c.y, borg.c.x, borg.c.y + o_y + sy[17],
                    borg.c.x + o_x + sx[17])
                < b_distance) {
                b_y        = o_y;
                b_x        = o_x;
                b_s        = true;
                b_n        = false;
                b_distance = borg_distance(borg.c.y, borg.c.x,
                    borg.c.y + b_y + sy[17], borg.c.x + b_x + sx[17]);
            }
        }
    }

    /* EAST -- Consider each area near the borg, looking for a good spot to hide
     */
    for (o_x = -1; o_x < 2; o_x++) {
        /* Reset Wall count */
        wall_east = 0;

        /* No N-S offset check when looking E-W */
        o_y = 0;

        for (i = 0; i < 25; i++) {
            borg_grid *ag;

            /* Check grids near borg */
            m_y = borg.c.y + o_y + ey[i];
            m_x = borg.c.x + o_x + ex[i];

            /* avoid screen edges */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y)))
                continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (e_array[i] == 0
                && ((ag->feat == FEAT_NONE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* This is a good grid */
                wall_east++;
            }
            if (e_array[i] == 1
                && ((ag->feat <= FEAT_MORE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* A good wall would score 25. */
                wall_east++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_east == 25) {
            if (borg_distance(borg.c.y, borg.c.x, borg.c.y + o_y + ey[13],
                    borg.c.x + o_x + ex[13])
                < b_distance) {
                b_y        = o_y;
                b_x        = o_x;
                b_e        = true;
                b_s        = false;
                b_n        = false;
                b_distance = borg_distance(borg.c.y, borg.c.x,
                    borg.c.y + b_y + ey[13], borg.c.x + b_x + ex[13]);
            }
        }
    }

    /* WEST -- Consider each area near the borg, looking for a good spot to hide
     */
    for (o_x = -2; o_x < 1; o_x++) {
        /* Reset Wall count */
        wall_west = 0;

        /* No N-S offset check when looking E-W */
        o_y = 0;

        for (i = 0; i < 25; i++) {
            borg_grid *ag;

            /* Check grids near borg */
            m_y = borg.c.y + o_y + wy[i];
            m_x = borg.c.x + o_x + wx[i];

            /* avoid screen edges */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y)))
                continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (w_array[i] == 0
                && ((ag->feat == FEAT_NONE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* This is a good grid */
                wall_west++;
            }
            if (w_array[i] == 1
                && ((ag->feat <= FEAT_MORE)
                    || (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K)
                    || ag->feat == FEAT_GRANITE)) {
                /* A good wall would score 25. */
                wall_west++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_west == 25) {
            if (borg_distance(borg.c.y, borg.c.x, borg.c.y + o_y + wy[11],
                    borg.c.x + o_x + wx[11])
                < b_distance) {
                b_y        = o_y;
                b_x        = o_x;
                b_w        = true;
                b_e        = false;
                b_s        = false;
                b_n        = false;
                b_distance = borg_distance(borg.c.y, borg.c.x,
                    borg.c.y + o_y + wy[11], borg.c.x + o_x + wx[11]);
            }
        }
    }

    /* Attempt to enqueue the grids that should be floor grids and have the
     * borg move onto those grids
     */
    if (b_n == true) {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(borg.c.y + b_y + ny[7], borg.c.x + b_x + nx[7]);

        /* Spread the flow */
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit(
                "anti-summon corridor north type 1", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }
    if (b_s == true) {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(
            borg.c.y + b_y + sy[17], borg.c.x + b_x + sx[17]);

        /* Spread the flow */
        borg_flow_spread(6, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit(
                "anti-summon corridor south type 1", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }
    if (b_e == true) {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(
            borg.c.y + b_y + ey[13], borg.c.x + b_x + ex[13]);

        /* Spread the flow */
        borg_digging = true;
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor east type 1", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }
    if (b_w == true) {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(
            borg.c.y + b_y + wy[11], borg.c.x + b_x + wx[11]);

        /* Spread the flow */
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor west type 1", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }

    return false;
}

/* Dig a straight Tunnel to a close monster */
bool borg_flow_kill_direct(bool viewable, bool twitchy)
{
    int i;
    int b_i = -1;
    int d;
    int b_d = z_info->max_sight;

    borg_kill *kill;

    /* Assume we need to dig granite */
    if (!borg_can_dig(false, FEAT_GRANITE))
        return false;

    /* Not if Weak from hunger or no food */
    if (!twitchy
        && (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK]
            || borg.trait[BI_FOOD] == 0))
        return false;

    /* Only when sitting for too long or twitchy */
    if (!twitchy && borg_t - borg_began < 3000 && borg.times_twitch < 5)
        return false;

    /* Do not dig when confused */
    if (borg.trait[BI_ISCONFUSED])
        return false;

    /* Not when darkened */
    if (borg.trait[BI_CURLITE] == 0)
        return false;

    /* Efficiency -- Nothing to kill */
    if (borg_kills_cnt) {
        /* Scan the monsters */
        for (i = 1; i < borg_kills_nxt; i++) {
            kill = &borg_kills[i];

            /* Skip "dead" monsters */
            if (!kill->r_idx)
                continue;

            /* Distance away */
            d = distance(kill->pos, borg.c);

            /* Track closest one */
            if (d > b_d)
                continue;

            /* Track it */
            b_i = i;
            b_d = d;
        }
    }

    /* If no Kill, then pick the center of the map */
    if (b_i == -1) {

        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid */
        borg_flow_enqueue_grid(AUTO_MAX_Y / 2, AUTO_MAX_X / 2);

        /* Spread the flow */
        borg_flow_spread(150, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("center direct", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }

    if (b_i) /* don't want it near permawall */
    {
        /* get the closest monster */
        kill = &borg_kills[b_i];

        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid */
        borg_flow_enqueue_grid(kill->pos.y, kill->pos.x);

        /* Spread the flow */
        borg_flow_spread(15, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("kill direct", GOAL_DIGGING))
            return false;

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING))
            return false;

        return true;
    }

    return false;
}

/*
 * Scan the monster lists for certain types of monster that we
 * should be concerned over.
 * This only works for monsters we know about.  If one of the
 * monsters around is misidentified then it may be a unique
 * and we wouldn't know.  Special consideration is given to Morgoth
 */
void borg_near_monster_type(int dist)
{
    borg_kill           *kill;
    struct monster_race *r_ptr;

    int x9, y9, ax, ay, d;
    int i;
    int breeder_count = 0;

    /* reset the borg flags */
    borg_fighting_summoner    = false;
    borg_fighting_unique      = 0;
    borg_fighting_evil_unique = false;
    borg_kills_summoner       = -1;

    /* Scan the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Count breeders */
        if (rf_has(r_ptr->flags, RF_MULTIPLY))
            breeder_count++;

        /*** Scan for Scary Guys ***/

        /* Do ScaryGuys now, before distance checks.  We are
         * Looking for scary guys on level, not scary guys
         * near me
         */

        /* run from certain scaries */
        if (borg.trait[BI_CLEVEL] <= 5 && (strstr(r_ptr->name, "Squint")))
            scaryguy_on_level = true;

        /* Mage and priest are extra fearful */
        if (borg.trait[BI_CLEVEL] <= 6
            && (borg.trait[BI_CLASS] == CLASS_MAGE
                || borg.trait[BI_CLASS] == CLASS_PRIEST)
            && (strstr(r_ptr->name, "Squint")))
            scaryguy_on_level = true;

        /* run from certain dungeon scaries */
        if (borg.trait[BI_CLEVEL] <= 5
            && (strstr(r_ptr->name, "Grip") || strstr(r_ptr->name, "Fang")
                || strstr(r_ptr->name, "Small kobold")))
            scaryguy_on_level = true;

        /* run from certain scaries */
        if (borg.trait[BI_CLEVEL] <= 8
            && (strstr(r_ptr->name, "Novice") || strstr(r_ptr->name, "Kobold")
                || strstr(r_ptr->name, "Kobold archer")
                || strstr(r_ptr->name, "Jackal")
                || strstr(r_ptr->name, "Shrieker")
                || strstr(r_ptr->name, "Farmer Maggot")
                || strstr(r_ptr->name, "Filthy street urchin")
                || strstr(r_ptr->name, "Battle-scarred veteran")
                || strstr(r_ptr->name, "Mean-looking mercenary")))
            scaryguy_on_level = true;

        if (borg.trait[BI_CLEVEL] <= 15
            && (strstr(r_ptr->name, "Bullr")
                || ((strstr(r_ptr->name, "Giant white mouse")
                        || strstr(r_ptr->name, "White worm mass")
                        || strstr(r_ptr->name, "Green worm mass"))
                    && breeder_count >= borg.trait[BI_CLEVEL])))
            scaryguy_on_level = true;

        if (borg.trait[BI_CLEVEL] <= 20
            && (strstr(r_ptr->name, "Cave spider")
                || strstr(r_ptr->name, "Pink naga")
                || strstr(r_ptr->name, "Giant pink frog")
                || strstr(r_ptr->name, "Radiation eye")
                || (strstr(r_ptr->name, "Yellow worm mass")
                    && breeder_count >= borg.trait[BI_CLEVEL])))
            scaryguy_on_level = true;

        if (borg.trait[BI_CLEVEL] < 45
            && (strstr(r_ptr->name, "Gravity") || strstr(r_ptr->name, "Inertia")
                || strstr(r_ptr->name, "Ancient")
                || strstr(r_ptr->name, "Beorn")
                || strstr(r_ptr->name, "Dread") /* Appear in Groups */))
            scaryguy_on_level = true;

        /* Nether breath is bad */
        if (!borg.trait[BI_SRNTHR]
            && (strstr(r_ptr->name, "Ossë, Herald of Ulmo")
                || strstr(r_ptr->name, "Dracolich")
                || strstr(r_ptr->name, "Dracolisk")))
            scaryguy_on_level = true;

        /* Blindness is really bad */
        if ((!borg.trait[BI_SRBLIND])
            && ((strstr(r_ptr->name, "Light hound") && !borg.trait[BI_SRLITE])
                || (strstr(r_ptr->name, "Dark hound")
                    && !borg.trait[BI_SRDARK])))
            scaryguy_on_level = true;

        /* Chaos and Confusion are really bad */
        if ((!borg.trait[BI_SRKAOS] && !borg.trait[BI_SRCONF])
            && (strstr(r_ptr->name, "Chaos")))
            scaryguy_on_level = true;
        if (!borg.trait[BI_SRCONF]
            && (strstr(r_ptr->name, "Pukelman")
                || strstr(r_ptr->name, "Nightmare")))
            scaryguy_on_level = true;

        /* Poison is really Bad */
        if (!borg.trait[BI_RPOIS] && /* Note the RPois not SRPois */
            (strstr(r_ptr->name, "Drolem")))
            scaryguy_on_level = true;

        /* Now do distance considerations */
        x9 = kill->pos.x;
        y9 = kill->pos.y;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* if the guy is too far then skip it unless in town. */
        if (d > dist && borg.trait[BI_CDEPTH])
            continue;

        /*** Scan for Uniques ***/

        /* this is a unique. */
        if (rf_has(r_ptr->flags, RF_UNIQUE))

        {
            /* Set a flag for use with certain types of spells */
            unique_on_level = kill->r_idx;

            /* return 1 if not Morgy, +10 if it is Morgy or Sauron */
            if (rf_has(r_ptr->flags, RF_QUESTOR)) {
                borg_fighting_unique += 10;
            }

            /* regular unique */
            borg_fighting_unique++;

            /* Note that fighting a Questor would result in a 11 value */
            if (rf_has(r_ptr->flags, RF_EVIL))
                borg_fighting_evil_unique = true;
        }

        /*** Scan for Summoners ***/
        if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN))
            || (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON))
            || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER))
            || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS))
            || (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL))
            || (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER))
            || (rsf_has(r_ptr->spell_flags, RSF_S_HOUND))
            || (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA))
            || (rsf_has(r_ptr->spell_flags, RSF_S_AINU))
            || (rsf_has(r_ptr->spell_flags, RSF_S_DEMON))
            || (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD))
            || (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON))
            || (rsf_has(r_ptr->spell_flags, RSF_S_HI_DRAGON))
            || (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD))
            || (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH))
            || (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE))) {
            /* mark the flag */
            borg_fighting_summoner = true;

            /* recheck the distance to see if close
             * and mark the index for as-corridor
             */
            if (d < 8) {
                borg_kills_summoner = i;
            }
        }
    }
}

/*
 * Help determine if PHASE_DOOR with Shoot N Scoot seems like
 * a good idea.
 * Good Idea on two levels:
 * 1.  We are the right class, we got some good ranged weapons
 * 2.  The possible landing grids are ok.
 * Almost a copy of the borg_caution_phase above.
 * The emergency is the number of dangerous grids out of 100
 * that we tolerate.  If we have 80, then we accept the risk
 * of landing on a grid that is 80% likely to be bad.  A low
 * number, like 20, means that we are less like to risk the
 * phase door and we require more of the possible grids to be
 * safe.
 *
 * The pattern of ShootN'Scoot works like this:
 * 1. Shoot monster that is far away.
 * 2. Monsters walks closer and closer each turn
 * 3. Borg shoots monster each step it takes as it approaches.
 * 4. Monster gets within 1 grid of the borg.
 * 5. Borg phases away.
 * 6. Go back to #1
 */
bool borg_shoot_scoot_safe(int emergency, int turns, int b_p)
{
    int n, k, i, d, x, y, p, u;

    int dis               = 10;

    int min               = dis / 2;

    bool adjacent_monster = false;

    borg_grid           *ag;
    borg_kill           *kill;
    struct monster_race *r_ptr;

    /* no need if high level in town */
    if (borg.trait[BI_CLEVEL] >= 8 && borg.trait[BI_CDEPTH] == 0)
        return false;

    /* must have the ability */
    if (!borg.trait[BI_APHASE])
        return false;

    /* Not if No Light */
    if (!borg.trait[BI_CURLITE])
        return false;

    /* Cheat the floor grid */
    /* Not if in a vault since it throws us out of the vault */
    if (square_isvault(cave, borg.c))
        return false;

    /*** Need Missiles or cheap spells ***/

    /* classes that are mainly spellcaster */
    if (borg_primarily_caster()) {
        /* Low mana */
        if (borg.trait[BI_CLEVEL] >= 45 && borg.trait[BI_CURSP] < 15)
            return false;

        /* Low mana, low level, generally OK */
        if (borg.trait[BI_CLEVEL] < 45 && borg.trait[BI_CURSP] < 5)
            return false;
    } else /* Other classes need some missiles */
    {
        if (borg.trait[BI_AMISSILES] < 5 || borg.trait[BI_CLEVEL] >= 45)
            return false;
    }

    /* Not if I am in a safe spot for killing special monsters */
    if (borg_morgoth_position || borg_as_position)
        return false;

    /* scan the adjacent grids for an awake monster */
    for (i = 0; i < 8; i++) {
        /* Grid in that direction */
        x = borg.c.x + ddx_ddd[i];
        y = borg.c.y + ddy_ddd[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Obtain the monster */
        kill  = &borg_kills[ag->kill];
        r_ptr = &r_info[kill->r_idx];

        /* If a qualifying monster is adjacent to me. */
        if ((ag->kill && kill->awake) && !(rf_has(r_ptr->flags, RF_NEVER_MOVE))
            && !(rf_has(r_ptr->flags, RF_PASS_WALL))
            && !(rf_has(r_ptr->flags, RF_KILL_WALL))
            && (kill->power >= borg.trait[BI_CLEVEL])) {
            /* Spell casters shoot at everything */
            if (borg_spell_okay(MAGIC_MISSILE)) {
                adjacent_monster = true;
            } else if (borg_spell_okay(ORB_OF_DRAINING)) {
                adjacent_monster = true;
            } else if (borg_spell_okay(NETHER_BOLT)) {
                adjacent_monster = true;
            }

            /* All other borgs need to make sure he would shoot.
             * In an effort to conserve missiles, the borg will
             * not shoot at certain types of monsters.  That list
             * is defined in borg_launch_damage_one().
             *
             * We need this aforementioned list to match the one
             * following.  Otherwise Rogues and Warriors will
             * burn up Phases as he scoots away but never fire
             * the missiles.  That totally defeats the purpose
             * of this routine.
             *
             * The following criteria are exactly the same as the
             * list in borg_launch_damage_one()
             */
            else if ((borg_danger_one_kill(
                          kill->pos.y, kill->pos.x, 1, i, true, false)
                         > avoidance * 3 / 10)
                     || ((r_ptr->friends
                             || r_ptr->friends_base) /* monster has friends*/
                         && kill->level
                                >= borg.trait[BI_CLEVEL] - 5 /* close levels */)
                     || (kill->ranged_attack /* monster has a ranged attack */)
                     || (rf_has(r_ptr->flags, RF_UNIQUE))
                     || (rf_has(r_ptr->flags, RF_MULTIPLY))
                     || (borg.trait[BI_CLEVEL] <= 5 /* still very weak */)) {
                adjacent_monster = true;
            }
        }
    }

    /* if No Adjacent_monster no need for it */
    if (adjacent_monster == false)
        return false;

    /* Simulate 100 attempts */
    for (n = k = 0; k < 100; k++) {
        /* Pick a location */
        for (i = 0; i < 100; i++) {
            /* Pick a (possibly illegal) location */
            while (1) {
                y = rand_spread(borg.c.y, dis);
                x = rand_spread(borg.c.x, dis);
                d = distance(borg.c, loc(x, y));
                if ((d >= min) && (d <= dis))
                    break;
            }

            /* Ignore illegal locations */
            if ((y <= 0) || (y >= AUTO_MAX_Y - 2))
                continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 2))
                continue;

            /* Access */
            ag = &borg_grids[y][x];

            /* Skip unknown grids */
            if (ag->feat == FEAT_NONE)
                continue;

            /* Skip walls */
            if (!borg_cave_floor_bold(y, x))
                continue;

            /* Skip monsters */
            if (ag->kill)
                continue;

            /* Stop looking.  Really, the game would keep
             * looking for a grid.  The borg could check
             * all the known grids but I don't think that
             * is not a good idea, especially if the area is
             * not fully explored.
             */
            break;
        }

        /* No location */
        /* In the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but we define it as dangerous. */
        if (i >= 100) {
            n++;
            continue;
        }

        /* Examine danger of that grid */
        p = borg_danger(y, x, turns, true, false);

        /* if more scary than my current one, do not allow jumps at all */
        if (p > b_p) {
            n++;
            continue;
        }

        /* Should not land next to a monster either.
         * Scan the adjacent grids for a monster.
         * Reuse the adjacent_monster variable.
         */
        for (u = 0; u < 8; u++) {
            /* Access the grid */
            ag = &borg_grids[y + ddy_ddd[u]][x + ddx_ddd[u]];

            /* Obtain the monster */
            kill = &borg_kills[ag->kill];

            /* If monster adjacent to that grid...
             */
            if (ag->kill && kill->awake)
                n++;
        }
    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency) {
        borg_note(format("# No Shoot'N'Scoot. scary squares: %d/100", n));
        return false;
    } else
        borg_note(format("# Safe to Shoot'N'Scoot. scary squares: %d/100", n));

    /* Okay */
    return true;
}

/*
 *  Create a kill at the given location
 */
int borg_create_kill(char *who, struct loc c)
{
    return borg_new_kill(borg_guess_race_name(who), c.y, c.x);
}


static void borg_init_monster_names(void)
{
    int      i, size;
    uint16_t r_max = z_info->r_max - 1;

    int16_t what[1024];
    char   *text[1024];

    /*** Parse "unique" monster names ***/

    /* Start over */
    size = 0;

    /* Collect "unique" monsters */
    for (i = 1; i < r_max; i++) {
        struct monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name)
            continue;

        /* Skip non-unique monsters */
        if (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            continue;

        text[size] = borg_massage_special_chars(r_ptr->name);
        what[size] = i;

        /* a few special uniques to look out for */
        if (streq(r_ptr->name, "Morgoth, Lord of Darkness"))
            borg_morgoth_id = r_ptr->ridx;
        if (streq(r_ptr->name, "Sauron, the Sorcerer"))
            borg_sauron_id = r_ptr->ridx;
        if (streq(r_ptr->name, "The Tarrasque"))
            borg_tarrasque_id = r_ptr->ridx;

        size++;
    }

    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);

    /* Save the size */
    borg_unique_size = size;

    /* Allocate the arrays */
    borg_unique_text = mem_zalloc(borg_unique_size * sizeof(const char *));
    borg_unique_what = mem_zalloc(borg_unique_size * sizeof(unsigned int));

    /* Save the entries */
    for (i = 0; i < size; i++)
        borg_unique_text[i] = text[i];
    for (i = 0; i < size; i++)
        borg_unique_what[i] = (unsigned int)what[i];

    /*** Parse "normal" monster names ***/

    /* Start over */
    size = 0;

    /* Collect "normal" monsters */
    for (i = 1; i < r_max; i++) {
        struct monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name)
            continue;

        /* Skip unique monsters */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            continue;

        text[size] = borg_massage_special_chars(r_ptr->name);
        what[size] = i;
        size++;
        if (r_ptr->plural) {
            text[size] = borg_massage_special_chars(r_ptr->plural);
            what[size] = i;
            size++;
        }
    }

    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);

    /* Save the size */
    borg_normal_size = size;

    /* Allocate the arrays */
    borg_normal_text = mem_zalloc(borg_normal_size * sizeof(const char *));
    borg_normal_what = mem_zalloc(borg_normal_size * sizeof(unsigned int));

    /* Save the entries */
    for (i = 0; i < size; i++)
        borg_normal_text[i] = text[i];
    for (i = 0; i < size; i++)
        borg_normal_what[i] = (unsigned int)what[i];
}

static void borg_free_monster_names(void)
{
    int i;

    mem_free(borg_normal_what);
    borg_normal_what = NULL;
    if (borg_normal_text) {
        for (i = 0; i < borg_normal_size; ++i) {
            string_free((char *)borg_normal_text[i]);
        }
        mem_free(borg_normal_text);
        borg_normal_text = NULL;
    }
    borg_normal_size = 0;
    mem_free(borg_unique_what);
    borg_unique_what = NULL;
    if (borg_unique_text) {
        for (i = 0; i < borg_unique_size; ++i) {
            string_free((char *)borg_unique_text[i]);
        }
        mem_free(borg_unique_text);
        borg_unique_text = NULL;
    }
    borg_unique_size = 0;
}

void borg_init_flow_kill(void)
{
    /*** Monster tracking ***/

    /* No monsters yet */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

    /* Array of monsters */
    borg_kills = mem_zalloc(256 * sizeof(borg_kill));

    /* Count racial appearances */
    borg_race_count = mem_zalloc(z_info->r_max * sizeof(int16_t));

    /* Count racial deaths */
    borg_race_death = mem_zalloc(z_info->r_max * sizeof(int16_t));

    /*** XXX XXX XXX Hack -- Cheat ***/

    /* Hack -- Extract dead uniques */
    for (int i = 1; i < z_info->r_max - 1; i++) {
        struct monster_race *r_ptr = &r_info[i];

        /* Skip non-monsters */
        if (!r_ptr->name)
            continue;

        /* Skip non-uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            continue;

        /* Mega-Hack -- Access "dead unique" list */
        if (r_ptr->max_num == 0)
            borg_race_death[i] = 1;
    }

    borg_init_monster_names();
}

void borg_free_flow_kill(void)
{
    borg_free_monster_names();

    mem_free(borg_race_death);
    borg_race_death = NULL;

    mem_free(borg_race_count);
    borg_race_count = NULL;

    mem_free(borg_kills);
    borg_kills = NULL;
}

#endif
