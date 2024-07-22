/**
 * \file borg-escape.c
 * \brief Various routines to help the borg run away
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

#include "borg-escape.h"

#ifdef ALLOW_BORG

#include "../ui-event.h"

#include "borg-cave-util.h"
#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow-stairs.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Determine "twice" the distance between two points
 * This results in "diagonals" being "correctly" ranged,
 * that is, a diagonal appears "further" than an adjacent.
 */
#define double_distance(Y1, X1, Y2, X2)                                        \
    (borg_distance(((int)(Y1)) << 1, ((int)(X1)) << 1, ((int)(Y2)) << 1,       \
        ((int)(X2)) << 1))

/*
 * Attempt to induce WORD_OF_RECALL
 * artifact activations added throughout this code
 */
bool borg_recall(void)
{
    /* Multiple "recall" fails */
    if (!borg.goal.recalling) {
        /* Try to "recall" */
        if (borg_zap_rod(sv_rod_recall) || borg_activate_item(act_recall)
            || borg_spell_fail(WORD_OF_RECALL, 60)
            || borg_read_scroll(sv_scroll_word_of_recall)) {
            /* Do reset depth at certain times. */
            if (borg.trait[BI_CDEPTH] < borg.trait[BI_MAXDEPTH]
                && ((borg.trait[BI_MAXDEPTH] >= 60
                        && borg.trait[BI_CDEPTH] >= 40)
                    || (borg.trait[BI_CLEVEL] < 48
                        && borg.trait[BI_CDEPTH] >= borg.trait[BI_MAXDEPTH] - 3)
                    || (borg.trait[BI_CLEVEL] < 48
                        && borg.trait[BI_CDEPTH] >= 15
                        && borg.trait[BI_MAXDEPTH] - borg.trait[BI_CDEPTH]
                               > 10))) {
                /* Special check on deep levels */
                if (borg.trait[BI_CDEPTH] >= 80 && borg.trait[BI_CDEPTH] < 100
                    && /* Deep */
                    borg_race_death[borg_sauron_id] != 0) /* Sauron is Dead */
                {
                    /* Do reset Depth */
                    borg_note("# Resetting recall depth.");
                    borg_keypress('y');
                } else if (borg.goal.fleeing_munchkin == true) {
                    /* Do not reset Depth */
                    borg_note("# Resetting recall depth during munchkin mode.");
                    borg_keypress('y');
                } else if (borg.trait[BI_CDEPTH] >= 100
                           && !borg.trait[BI_KING]) {
                    /* Do reset Depth */
                    borg_note("# Not Resetting recall depth.");
                    borg_keypress('n');
                } else {
                    /* Do reset Depth */
                    borg_note("# Resetting recall depth.");
                    borg_keypress('y');
                }
            }

            /* reset recall depth in dungeon? */
            else if (borg.trait[BI_CDEPTH] < borg.trait[BI_MAXDEPTH]
                     && borg.trait[BI_CDEPTH] != 0) {
                /* Do not reset Depth */
                borg_note("# Not resetting recall depth.");
                borg_keypress('n');
            }

            borg_keypress(ESCAPE);

            /* Success */
            return true;
        }
    }

    /* Nothing */
    return false;
}

/*
 * Hack -- evaluate the likelihood of the borg getting surrounded
 * by a bunch of monsters.  This is called from borg_danger() when
 * he looking for a strategic retreat.  It is hopeful that the borg
 * will see that several monsters are approaching him and he may
 * become surrounded then die.  This routine looks at near-by monsters
 * and determines the likelihood of him getting surrounded.
 */
bool borg_surrounded(void)
{
    borg_kill           *kill;
    struct monster_race *r_ptr;

    int safe_grids        = 8;
    int non_safe_grids    = 0;
    int monsters          = 0;
    int adjacent_monsters = 0;

    int x9, y9, ax, ay, d;
    int i;

    /* Evaluate the local monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        x9 = kill->pos.x;
        y9 = kill->pos.y;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* if the monster is too far then skip it. */
        if (d > 3)
            continue;

        /* if he cant see me then forget it.*/
        if (!borg_los(borg.c.y, borg.c.x, y9, x9))
            continue;

        /* if asleep, don't consider this one */
        if (!kill->awake)
            continue;

        /* Monsters with Pass Wall are dangerous, no escape from them */
        if (rf_has(r_ptr->flags, RF_PASS_WALL))
            continue;
        if (rf_has(r_ptr->flags, RF_KILL_WALL))
            continue;

        /* Cant really run away from Breeders very well */
        /* if (rf_has(r_ptr->flags, RF_MULTIPLY)) continue; */

        /* Monsters who never move cant surround */
        /* if (rf_has(r_ptr->flags, RF_NEVER_MOVE)) continue; */

        /* keep track of monsters touching me */
        if (d == 1)
            adjacent_monsters++;

        /* Add them up. */
        monsters++;
    }

    /* Evaluate the Non Safe Grids, (walls, closed doors, traps, monsters) */
    for (i = 0; i < 8; i++) {
        int x = borg.c.x + ddx_ddd[i];
        int y = borg.c.y + ddy_ddd[i];

        /* Access the grid */
        borg_grid *ag = &borg_grids[y][x];

        /* Bound check */
        if (!square_in_bounds_fully(cave, loc(x, y)))
            continue;

        /* Skip walls/doors */
        if (!borg_cave_floor_grid(ag))
            non_safe_grids++;

        /* Skip unknown grids */
        else if (ag->feat == FEAT_NONE)
            non_safe_grids++;

        /* Skip monster grids */
        else if (ag->kill)
            non_safe_grids++;

        /* Mega-Hack -- skip stores XXX XXX XXX */
        else if (feat_is_shop(ag->feat))
            non_safe_grids++;

        /* Mega-Hack -- skip traps XXX XXX XXX */
        if (ag->trap && !ag->glyph)
            non_safe_grids++;
    }

    /* Safe grids are decreased */
    safe_grids = safe_grids - non_safe_grids;

    /* Am I in hallway? If so don't worry about it */
    if (safe_grids == 1 && adjacent_monsters == 1)
        return false;

    /* I am likely to get surrounded */
    if (monsters > safe_grids) {
        borg_note(format(
            "# Possibility of being surrounded (monsters/safegrids)(%d/%d)",
            monsters, safe_grids));

        /* The borg can get trapped by continuing to flee
         * into a dead-end.  So he needs to be able to trump this
         * routine.
         */
        if (borg.goal.ignoring) {
            /* borg_note("# Ignoring the fact that I am surrounded.");
             * return false;
             */
        } else
            return true;
    }

    /* Probably will not be surrounded */
    return false;
}

/*
 * Mega-Hack -- evaluate the "freedom" of the given location
 *
 * The theory is that often, two grids will have equal "danger",
 * but one will be "safer" than the other, perhaps because it
 * is closer to stairs, or because it is in a corridor, or has
 * some other characteristic that makes it "safer".
 *
 * Then, if the Borg is in danger, say, from a normal speed monster
 * which is standing next to him, he will know that walking away from
 * the monster is "pointless", because the monster will follow him,
 * but if the resulting location is "safer" for some reason, then
 * he will consider it.  This will allow him to flee towards stairs
 * in the town, and perhaps towards corridors in the dungeon.
 *
 * This method is used in town to chase the stairs.
 *
 * XXX XXX XXX We should attempt to walk "around" buildings.
 */
int borg_freedom(int y, int x)
{
    int d, f = 0;

    /* Hack -- chase down stairs in town */
    if (!borg.trait[BI_CDEPTH] && track_more.num) {
        /* Love the stairs! */
        d = double_distance(y, x, track_more.y[0], track_more.x[0]);

        /* Proximity is good */
        f += (1000 - d);

        /* Close proximity is great */
        if (d < 4)
            f += (2000 - (d * 500));
    }

    /* Hack -- chase Up Stairs in dungeon */
    if (borg.trait[BI_CDEPTH] && track_less.num) {
        /* Love the stairs! */
        d = double_distance(y, x, track_less.y[0], track_less.x[0]);

        /* Proximity is good */
        f += (1000 - d);

        /* Close proximity is great */
        if (d < 4)
            f += (2000 - (d * 500));
    }

    /* Freedom */
    return (f);
}

/*
 * Help determine if PHASE_DOOR seems like a good idea
 */
bool borg_caution_phase(int emergency, int turns)
{
    int n, k, i, d, x, y, p;

    int dis       = 10;
    int min       = dis / 2;

    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    /* must have the ability */
    if (!borg.trait[BI_APHASE])
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
            if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
                continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 1))
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

            /* Skip webs */
            if (ag->web)
                continue;

            /* Stop looking */
            break;
        }

        /* If low level, unknown squares are scary */
        if (ag->feat == FEAT_NONE && borg.trait[BI_MAXHP] < 30) {
            n++;
            continue;
        }

        /* No location */
        /* in the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but may be dangerous. */
        if (i >= 100) {
            n++;
            continue;
        }

        /* Examine */
        p = borg_danger(y, x, turns, true, false);

        /* if *very* scary, do not allow jumps at all */
        if (p > borg.trait[BI_CURHP])
            n++;
    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency) {
        borg_note(format("# No Phase. scary squares: %d", n));
        return false;
    } else
        borg_note(format("# Safe to Phase. scary squares: %d", n));

    /* Okay */
    return true;
}

/*
 * Help determine if "Teleport" seems like a good idea
 */
bool borg_caution_teleport(int emergency, int turns)
{
    int n, k, i, d, x, y, p;

    int dis = 100;
    int min = dis / 2;
    int q_x, q_y;

    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    /* Extract panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* must have the ability */
    if (!borg.trait[BI_ATELEPORT] || !borg.trait[BI_AESCAPE])
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
            if ((y <= 0) || (y >= AUTO_MAX_Y - 1))
                continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 1))
                continue;

            /* Access */
            ag = &borg_grids[y][x];

            /* Skip unknown grids if explored, or been on level for a while,
             * otherwise, consider ok*/
            if (ag->feat == FEAT_NONE
                && ((borg_detect_wall[q_y + 0][q_x + 0] == true
                        && borg_detect_wall[q_y + 0][q_x + 1] == true
                        && borg_detect_wall[q_y + 1][q_x + 0] == true
                        && borg_detect_wall[q_y + 1][q_x + 1] == true)
                    || borg_t > 2000))
                continue;

            /* Skip walls */
            if (!borg_cave_floor_bold(y, x))
                continue;

            /* Skip monsters */
            if (ag->kill)
                continue;

            /* Skip webs */
            if (ag->web)
                continue;

            /* Stop looking */
            break;
        }

        /* If low level, unknown squares are scary */
        if (ag->feat == FEAT_NONE && borg.trait[BI_MAXHP] < 30) {
            n++;
            continue;
        }

        /* No location */
        /* in the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but may be dangerous. */
        if (i >= 100) {
            n++;
            continue;
        }

        /* Examine */
        p = borg_danger(y, x, turns, true, false);

        /* if *very* scary, do not allow jumps at all */
        if (p > borg.trait[BI_CURHP])
            n++;
    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency) {
        borg_note(format("# No Teleport. scary squares: %d", n));
        return false;
    }
    /* Okay */
    return true;
}

/*
 * Hack -- If the borg is standing on a stair and is in some danger, just leave
 * the level. No need to hang around on that level, try conserving the teleport
 * scrolls
 */
static bool borg_escape_stair(void)
{
    /* Current grid */
    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    /* Usable stairs */
    if (ag->feat == FEAT_LESS) {
        /* Take the stairs */
        borg_note("# Escaping level via stairs.");
        borg_keypress('<');

        /* Success */
        return true;
    }

    return false;
}

/*
 * Is teleport currently allowed
 */
bool borg_allow_teleport(void)
{
    /* No teleporting in arena levels */
    if (player->upkeep->arena_level)
        return false;

    /* Check for a no teleport grid */
    if (square_isno_teleport(cave, borg.c))
        return false;

    /* Check for a no teleport curse */
    if (borg.trait[BI_CRSNOTEL])
        return false;

    return true;
}

/* short range teleport + pain */
bool borg_shadow_shift(int allow_fail)
{
    /* disallow if hp too low */
    if (borg.trait[BI_CURHP] < 12)
        return false;
    return borg_spell_fail(SHADOW_SHIFT, allow_fail);
}

/* medium range teleport */
bool borg_dimension_door(int allow_fail)
{
    int        x_off, y_off;
    int        t_x, t_y;
    struct loc best;
    int        d, best_d = 0;
    struct loc target;

    /* for now keep the range at under 50, for performance */
    int range = 50;

    /* Require ability (right now) */
    if (!borg_spell_okay_fail(DIMENSION_DOOR, allow_fail))
        return 0;

    /* if we are attacking, calculate gains, but if this is just a teleport */
    /* the current danger is the starting point */
    best_d = borg_fear_region[borg.c.y][borg.c.x];

    /* Pick a location */
    for (x_off = range * -1; x_off < range; x_off++) {
        for (y_off = range * -1; y_off < range; y_off++) {
            t_x = borg.c.x + x_off;
            t_y = borg.c.y + y_off;

            if (t_x < 0 || t_y < 0)
                continue;

            target = loc(borg.c.x + x_off, borg.c.y + y_off);

            if (!square_in_bounds_fully(cave, target))
                continue;

            d = borg_danger(t_y, t_x, 2, true, false);
            if (d < best_d) {
                best_d = d;
                best.x = t_x;
                best.y = t_y;
            }
        }
    }

    if (best_d < borg_fear_region[borg.c.y][borg.c.x]) {
        borg_target(best);

        borg_spell(DIMENSION_DOOR);

        /* pick target */
        borg_keypress('5');
        return true;
    }
    return false;
}

/*
 * Try to phase door or teleport
 * b_q is the danger of the least dangerous square around us.
 */
bool borg_escape(int b_q)
{

    int risky_boost = 0;
    int j;
    int glyphs = 0;

    borg_grid *ag;

    /* only escape with spell if fail is low */
    int allow_fail = 25;
    int sv_mana;

    /* if very healthy, allow extra fail */
    if (((borg.trait[BI_CURHP] * 100) / borg.trait[BI_MAXHP]) > 70)
        allow_fail = 10;

    /* comprimised, get out of the fight */
    if (borg.trait[BI_ISHEAVYSTUN])
        allow_fail = 35;

    /* for emergencies */
    sv_mana = borg.trait[BI_CURSP];

    /* Borgs who are bleeding to death or dying of poison may sometimes
     * phase around the last two hit points right before they enter a
     * shop.  He knows to make a bee-line for the temple but the danger
     * trips this routine.  So we must bypass this routine for some
     * particular circumstances.
     */
    if (!borg.trait[BI_CDEPTH]
        && (borg.trait[BI_ISPOISONED] || borg.trait[BI_ISWEAK]
            || borg.trait[BI_ISCUT]))
        return false;

    /* Borgs who are in a sea of runes or trying to build one
     * and mostly healthy stay put
     */
    if ((borg.trait[BI_CDEPTH] == 100)
        && borg.trait[BI_CURHP] >= (borg.trait[BI_MAXHP] * 5 / 10)) {
        /* In a sea of runes */
        if (borg_morgoth_position)
            return false;

        /* Scan neighbors */
        for (j = 0; j < 8; j++) {
            int y = borg.c.y + ddy_ddd[j];
            int x = borg.c.x + ddx_ddd[j];

            /* Get the grid */
            ag = &borg_grids[y][x];

            /* Skip unknown grids (important) */
            if (ag->glyph)
                glyphs++;
        }
        /* Touching at least 3 glyphs */
        if (glyphs >= 3)
            return false;
    }

    /* Hack -- If the borg is weak (no food, starving) on depth 1 and he has no
     * idea where the stairs may be, run the risk of diving deeper against the
     * benefit of rising to town.
     */
    if (borg.trait[BI_ISWEAK] && borg.trait[BI_CDEPTH] == 1) {
        if (borg_read_scroll(sv_scroll_teleport_level)
            || borg_activate_item(act_tele_level)) {
            borg_note("# Attempting to leave via teleport level");
            return true;
        }
    }

    /* Risky borgs are more likely to stay in a fight */
    if (borg_cfg[BORG_PLAYS_RISKY])
        risky_boost = 3;

    /* 1. really scary, I'm about to die */
    /* Try an emergency teleport, or phase door as last resort */
    if (borg.trait[BI_ISHEAVYSTUN]
        || (b_q > avoidance * (45 + risky_boost) / 10)
        || ((b_q > avoidance * (40 + risky_boost) / 10)
            && borg_fighting_unique >= 10 && borg.trait[BI_CDEPTH] == 100
            && borg.trait[BI_CURHP] < 600)
        || ((b_q > avoidance * (30 + risky_boost) / 10)
            && borg_fighting_unique >= 10 && borg.trait[BI_CDEPTH] == 99
            && borg.trait[BI_CURHP] < 600)
        || ((b_q > avoidance * (25 + risky_boost) / 10)
            && borg_fighting_unique >= 1 && borg_fighting_unique <= 8
            && borg.trait[BI_CDEPTH] >= 95 && borg.trait[BI_CURHP] < 550)
        || ((b_q > avoidance * (17 + risky_boost) / 10)
            && borg_fighting_unique >= 1 && borg_fighting_unique <= 8
            && borg.trait[BI_CDEPTH] < 95)
        || ((b_q > avoidance * (15 + risky_boost) / 10)
            && !borg_fighting_unique)) {

        int tmp_allow_fail = 15;

        if (borg_escape_stair()
            || (borg_allow_teleport()
                && (borg_dimension_door(tmp_allow_fail - 10)
                    || borg_spell_fail(TELEPORT_SELF, tmp_allow_fail - 10)
                    || borg_spell_fail(PORTAL, tmp_allow_fail - 10)
                    || borg_shadow_shift(tmp_allow_fail - 10)
                    || borg_read_scroll(sv_scroll_teleport)
                    || borg_read_scroll(sv_scroll_teleport_level)
                    || borg_use_staff_fail(sv_staff_teleportation)
                    || borg_activate_item(act_tele_long)
                    || borg_activate_item(act_tele_level) ||

                    /* revisit spells, increased fail rate */
                    borg_dimension_door(tmp_allow_fail + 9)
                    || borg_spell_fail(TELEPORT_SELF, tmp_allow_fail + 9)
                    || borg_spell_fail(PORTAL, tmp_allow_fail + 9)
                    || borg_shadow_shift(tmp_allow_fail + 9) ||

                    /* revisit teleport, increased fail rate */
                    borg_use_staff(sv_staff_teleportation) ||

                    /* Attempt Teleport Level */
                    borg_spell_fail(TELEPORT_LEVEL, tmp_allow_fail + 9) ||

                    /* try phase at least, with some hedging of the safety of
                       landing zone */
                    (borg_caution_phase(75, 2)
                        && (borg_read_scroll(sv_scroll_phase_door)
                            || borg_activate_item(act_tele_phase)
                            || borg_spell_fail(PHASE_DOOR, tmp_allow_fail)
                            || borg_spell_fail(PORTAL, tmp_allow_fail)))))) {
            /* Flee! */
            borg_note("# Danger Level 1.");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            return true;
        }

        borg.trait[BI_CURSP] = borg.trait[BI_MAXSP];

        /* try to teleport, get far away from here */
        if (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 10
            && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] * 1 / 10)
            && borg_allow_teleport()
            && (borg_dimension_door(90) || borg_spell(TELEPORT_SELF)
                || borg_spell(PORTAL))) {
            /* verify use of spell */
            /* borg_keypress('y');  */

            /* Flee! */
            borg_note("# Danger Level 1.1  Critical Attempt");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            return true;
        }

        /* emergency phase activation no concern for safety of landing zone. */
        if (borg.trait[BI_CDEPTH]
            && ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] * 1 / 10
                    || b_q > avoidance * (45 + risky_boost) / 10)
                && (borg_activate_item(act_tele_phase)
                    || borg_read_scroll(sv_scroll_phase_door)))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 1.2  Critical Phase");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            return true;
        }

        /* emergency phase spell */
        if (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 10
            && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] * 1 / 10)
            && ((borg_spell_fail(PHASE_DOOR, 15) || borg_spell(PORTAL)))) {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Danger Level 1.3  Critical Attempt");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            return true;
        }

        /* Restore the real mana level */
        borg.trait[BI_CURSP] = sv_mana;
    }

    /* If fighting a unique and at the end of the game try to stay and
     * finish the fight.  Only bail out in extreme danger as above.
     */
    if (b_q < avoidance * (25 + risky_boost) / 10 && borg_fighting_unique >= 1
        && borg_fighting_unique <= 3 && borg.trait[BI_CDEPTH] >= 97)
        return false;

    /* 2 - a bit more scary/
     * Attempt to teleport (usually)
     * do not escape from uniques so quick
     */
    if (borg.trait[BI_ISHEAVYSTUN]
        || ((b_q > avoidance * (3 + risky_boost) / 10)
            && borg.trait[BI_CLASS] == CLASS_MAGE && borg.trait[BI_CURSP] <= 20
            && borg.trait[BI_MAXCLEVEL] >= 45)
        || ((b_q > avoidance * (13 + risky_boost) / 10)
            && borg_fighting_unique >= 1 && borg_fighting_unique <= 8
            && borg.trait[BI_CDEPTH] != 99)
        || ((b_q > avoidance * (11 + risky_boost) / 10)
            && !borg_fighting_unique)) {

        /* Try teleportation */
        if (borg_escape_stair()
            || (borg_allow_teleport()
                && (borg_dimension_door(allow_fail - 10)
                    || borg_spell_fail(TELEPORT_SELF, allow_fail - 10)
                    || borg_spell_fail(PORTAL, allow_fail - 10)
                    || borg_shadow_shift(allow_fail - 10)
                    || borg_use_staff_fail(sv_staff_teleportation)
                    || borg_activate_item(act_tele_long)
                    || borg_read_scroll(sv_scroll_teleport)
                    || borg_read_scroll(sv_scroll_teleport_level)
                    || borg_dimension_door(allow_fail)
                    || borg_activate_item(act_tele_level)
                    || borg_spell_fail(TELEPORT_SELF, allow_fail)
                    || borg_spell_fail(PORTAL, allow_fail)
                    || borg_shadow_shift(allow_fail)
                    || borg_use_staff(sv_staff_teleportation)))) {
            /* Flee! */
            borg_note("# Danger Level 2.1");

            /* Success */
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            return true;
        }
        /* Phase door, if useful */
        if (borg_caution_phase(50, 2) && borg_t - borg_t_antisummon > 50
            && (borg_spell(PHASE_DOOR) || borg_spell(PORTAL)
                || borg_read_scroll(sv_scroll_phase_door)
                || borg_activate_item(act_tele_phase))) {
            /* Flee! */
            borg_note("# Danger Level 2.2");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;
            /* Success */
            return true;
        }
    }

    /* 3- not too bad */
    /* also run if stunned or it is scary here */
    if (borg.trait[BI_ISHEAVYSTUN]
        || ((b_q > avoidance * (13 + risky_boost) / 10)
            && borg_fighting_unique >= 2 && borg_fighting_unique <= 8)
        || ((b_q > avoidance * (10 + risky_boost) / 10)
            && !borg_fighting_unique)
        || ((b_q > avoidance * (10 + risky_boost) / 10)
            && borg.trait[BI_ISAFRAID]
            && (borg.trait[BI_AMISSILES] <= 0
                && borg.trait[BI_CLASS] == CLASS_WARRIOR))) {
        /* Phase door, if useful */
        if ((borg_escape_stair() || borg_caution_phase(25, 2))
            && borg_t - borg_t_antisummon > 50
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 3.1");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* Teleport via spell */
        if (borg_allow_teleport()
            && (borg_dimension_door(allow_fail)
                || borg_spell_fail(TELEPORT_SELF, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_shadow_shift(allow_fail)
                || borg_activate_item(act_tele_long)
                || borg_use_staff_fail(sv_staff_teleportation)
                || borg_read_scroll(sv_scroll_teleport)
                || borg_activate_item(act_tele_phase))) {
            /* Flee! */
            borg_note("# Danger Level 3.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }
        /* Phase door, if useful */
        if (borg_caution_phase(75, 2) && borg_t - borg_t_antisummon > 50
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_shadow_shift(allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 3.3");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* Use Tport Level after the above attempts failed. */
        if (borg_read_scroll(sv_scroll_teleport_level)
            || borg_activate_item(act_tele_level)) {
            /* Flee! */
            borg_note("# Danger Level 3.4");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!borg.goal.fleeing
            && (!borg_fighting_unique || borg.trait[BI_CLEVEL] < 35)
            && !vault_on_level) {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }

        /* Flee now */
        if (!borg.goal.leaving
            && (!borg_fighting_unique || borg.trait[BI_CLEVEL] < 35)
            && !vault_on_level) {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            borg.goal.leaving = true;
        }
    }
    /* 4- not too scary but I'm compromised */
    if ((b_q > avoidance * (8 + risky_boost) / 10
            && (borg.trait[BI_CLEVEL] < 35
                || borg.trait[BI_CURHP] <= borg.trait[BI_MAXHP] / 3))
        || ((b_q > avoidance * (9 + risky_boost) / 10)
            && borg_fighting_unique >= 1 && borg_fighting_unique <= 8
            && (borg.trait[BI_CLEVEL] < 35
                || borg.trait[BI_CURHP] <= borg.trait[BI_MAXHP] / 3))
        || ((b_q > avoidance * (6 + risky_boost) / 10)
            && borg.trait[BI_CLEVEL] <= 20 && !borg_fighting_unique)
        || ((b_q > avoidance * (6 + risky_boost) / 10)
            && borg.trait[BI_CLEVEL] <= 35)) {
        /* Phase door, if useful */
        if ((borg_escape_stair() || borg_caution_phase(20, 2))
            && borg_t - borg_t_antisummon > 50
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_shadow_shift(allow_fail)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 4.1");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* Teleport via spell */
        if (borg_allow_teleport()
            && (borg_dimension_door(allow_fail)
                || borg_spell_fail(TELEPORT_SELF, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_long)
                || borg_shadow_shift(allow_fail)
                || borg_read_scroll(sv_scroll_teleport)
                || borg_use_staff_fail(sv_staff_teleportation))) {
            /* Flee! */
            borg_note("# Danger Level 4.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!borg.goal.fleeing && !borg_fighting_unique
            && borg.trait[BI_CLEVEL] < 25 && !vault_on_level) {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }

        /* Flee now */
        if (!borg.goal.leaving && !borg_fighting_unique && !vault_on_level) {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            borg.goal.leaving = true;
        }
        /* Emergency Phase door if a weak mage */
        if (((borg.trait[BI_CLASS] == CLASS_MAGE
                 || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
                && borg.trait[BI_CLEVEL] <= 35)
            && borg_caution_phase(65, 2) && borg_t - borg_t_antisummon > 50
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_activate_item(act_tele_long)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 4.3");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }
    }

    /* 5- not too scary but I'm very low level  */
    if (borg.trait[BI_CLEVEL] < 10
        && (b_q > avoidance * (5 + risky_boost) / 10
            || (b_q > avoidance * (7 + risky_boost) / 10
                && borg_fighting_unique >= 1 && borg_fighting_unique <= 8))) {
        /* Phase door, if useful */
        if ((borg_escape_stair() || borg_caution_phase(20, 2))
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_shadow_shift(allow_fail)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg_note("# Danger Level 5.1");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* Teleport via spell */
        if (borg_allow_teleport()
            && (borg_dimension_door(allow_fail)
                || borg_spell_fail(TELEPORT_SELF, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_shadow_shift(allow_fail)
                || borg_activate_item(act_tele_long)
                || borg_read_scroll(sv_scroll_teleport)
                || borg_use_staff_fail(sv_staff_teleportation))) {
            /* Flee! */
            borg_note("# Danger Level 5.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!borg.goal.fleeing && !borg_fighting_unique) {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }

        /* Flee now */
        if (!borg.goal.leaving && !borg_fighting_unique) {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            borg.goal.leaving = true;
        }
        /* Emergency Phase door if a weak mage */
        if (((borg.trait[BI_CLASS] == CLASS_MAGE
                 || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
                && borg.trait[BI_CLEVEL] <= 8)
            && borg_caution_phase(65, 2)
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_read_scroll(sv_scroll_phase_door)
                || borg_activate_item(act_tele_long))) {
            /* Flee! */
            borg.escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 5.3");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }
    }

    /* 6- not too scary but I'm out of mana  */
    if ((borg.trait[BI_CLASS] == CLASS_MAGE
            || borg.trait[BI_CLASS] == CLASS_PRIEST
            || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        && (b_q > avoidance * (6 + risky_boost) / 10
            || (b_q > avoidance * (8 + risky_boost) / 10
                && borg_fighting_unique >= 1 && borg_fighting_unique <= 8))
        && (borg.trait[BI_CURSP] <= (borg.trait[BI_MAXSP] * 1 / 10)
            && borg.trait[BI_MAXSP] >= 100)) {
        /* Phase door, if useful */
        if ((borg_escape_stair() || borg_caution_phase(20, 2))
            && borg_t - borg_t_antisummon > 50
            && (borg_spell_fail(PHASE_DOOR, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_phase)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* Flee! */
            borg_note("# Danger Level 6.1");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }

        /* Teleport via spell */
        if (borg_allow_teleport()
            && (borg_dimension_door(allow_fail)
                || borg_spell_fail(TELEPORT_SELF, allow_fail)
                || borg_spell_fail(PORTAL, allow_fail)
                || borg_activate_item(act_tele_long)
                || borg_read_scroll(sv_scroll_teleport)
                || borg_use_staff_fail(sv_staff_teleportation))) {
            /* Flee! */
            borg_note("# Danger Level 6.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }
    }

    /* 7- Shoot N Scoot */
    if ((borg_spell_okay_fail(PHASE_DOOR, allow_fail)
            || borg_spell_okay_fail(PORTAL, allow_fail))
        && borg_shoot_scoot_safe(20, 2, b_q)) {
        /* Phase door */
        if (borg_spell_fail(PHASE_DOOR, allow_fail)
            || borg_spell_fail(PORTAL, allow_fail)) {
            /* Flee! */
            borg_note("# Shoot N Scoot. (Danger Level 7.1)");
            borg.escapes--; /* a phase isn't really an escape */

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50)
                borg_t_antisummon = 0;

            /* Success */
            return true;
        }
    }

    return false;
}

#endif
