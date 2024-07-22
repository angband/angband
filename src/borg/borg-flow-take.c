/**
 * \file borg-flow-take.c
 * \brief Flow (move) to an object (take)
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

#include "borg-flow-take.h"

#ifdef ALLOW_BORG

#include "../monster.h"
#include "../obj-knowledge.h"
#include "../obj-tval.h"
#include "../obj-util.h"

#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-io.h"
#include "borg-item-val.h"
#include "borg-item-wear.h"
#include "borg-light.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * The object list.  This list is used to "track" objects.
 */

int16_t    borg_takes_cnt;
int16_t    borg_takes_nxt;
borg_take *borg_takes;

struct object * borg_get_top_object(struct chunk *c, struct loc grid)
{
    /* Cheat the Actual item */
    struct object *o_ptr;
    o_ptr = square_object(cave, grid);
    while (o_ptr) {
        if ((o_ptr->known && o_ptr->known->notice & OBJ_NOTICE_IGNORE))
            o_ptr = o_ptr->next;
        else if (o_ptr->kind->ignore)
            o_ptr = o_ptr->next;
        else
            break;
    }

    return o_ptr;
}

/*
 * Attempt to guess what kind of object is at the given location.
 *
 * This routine should rarely, if ever, return "zero".
 */
static struct object_kind *borg_guess_kind(uint8_t a, wchar_t c, int y, int x)
{
    /* ok, this is an real cheat.  he ought to use the look command
     * in order to correctly id the object.  But I am passing that up for
     * the sake of speed and accuracy
     */

    /* Cheat the Actual item */
    struct object *o_ptr = borg_get_top_object(cave, loc(x, y));
    if (!o_ptr)
        return NULL;
    return (o_ptr->kind);
}

/*
 * Delete an old "object" record
 */
void borg_delete_take(int i)
{
    borg_grid *ag;

    borg_take *take = &borg_takes[i];

    /* Paranoia -- Already wiped */
    if (!take->kind)
        return;

    /* Note */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("# Forgetting an object '%s' at (%d,%d)",
            (take->kind->name), take->y, take->x));

    /* Access the grid */
    ag = &borg_grids[take->y][take->x];

    /* Forget it */
    ag->take = 0;

    /* Kill the object */
    memset(take, 0, sizeof(borg_take));

    /* One less object */
    borg_takes_cnt--;

    /* Wipe goals */
    if (borg.goal.type == GOAL_TAKE)
        borg.goal.type = 0;
}

/*
 * Determine if an object should be "viewable"
 */
static bool borg_follow_take_aux(int i, int y, int x)
{
    borg_grid *ag;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Not on-screen */
    if (!(ag->info & BORG_OKAY))
        return false;

    /* Assume viewable */
    return true;
}

/*
 * Attempt to "follow" a missing object
 *
 * This routine is not called when the player is blind or hallucinating.
 *
 * This function just deletes objects which have disappeared.
 *
 * We assume that a monster walking onto an object destroys the object
 * if it has the appropriate flags.
 */
void borg_follow_take(int i)
{
    int ox, oy;

    borg_take           *take  = &borg_takes[i];
    borg_grid           *ag    = &borg_grids[take->y][take->x];
    borg_kill           *kill  = &borg_kills[ag->kill];
    struct monster_race *r_ptr = &r_info[kill->r_idx];
    struct object_kind  *old_kind;

    /* Paranoia */
    if (!take->kind)
        return;

    /* Old location */
    ox       = take->x;
    oy       = take->y;
    old_kind = take->kind;

    /* delete them if they are under me */
    if (take->y == borg.c.y && take->x == borg.c.x) {
        borg_delete_take(i);
    }

    /* Out of sight */
    if (!borg_follow_take_aux(i, oy, ox))
        return;

    /* Some monsters won't take or crush items */
    if (ag->kill && !rf_has(r_ptr->flags, RF_TAKE_ITEM)
        && !rf_has(r_ptr->flags, RF_KILL_ITEM))
        return;

    /* Note */
    borg_note(format(
        "# There was an object '%s' at (%d,%d)", (old_kind->name), ox, oy));

    /* Kill the object */
    borg_delete_take(i);
}

/*
 * Obtain a new "take" index
 */
static int borg_new_take(struct object_kind *kind, int y, int x)
{
    int i, n = -1;

    borg_take *take;

    borg_grid *ag        = &borg_grids[y][x];

    struct object *o_ptr = borg_get_top_object(cave, loc(x, y));

    /* Look for a "dead" object */
    for (i = 1; (n < 0) && (i < borg_takes_nxt); i++) {
        /* Reuse "dead" objects */
        if (!borg_takes[i].kind)
            n = i;
    }

    /* Allocate a new object */
    if ((n < 0) && (borg_takes_nxt < 256)) {
        /* Acquire the entry, advance */
        n = borg_takes_nxt++;
    }

    /* Hack -- steal an old object */
    if (n < 0) {
        /* Note */
        borg_note("# Too many objects");

        /* Hack -- Pick a random object */
        n = randint0(borg_takes_nxt - 1) + 1;

        /* Delete it */
        borg_delete_take(n);
    }

    /* Count new object */
    borg_takes_cnt++;

    /* Obtain the object */
    take = &borg_takes[n];

    /* Save the kind */
    take->kind = kind;
    take->tval = kind->tval;

    /* Save the location */
    take->x = x;
    take->y = y;

    /* Save the index */
    ag->take = n;

    /* Timestamp */
    take->when = borg_t;

    /* Not had Orb of Draining cast on it */
    take->orbed = false;

    /* Assess a estimated value */
    if (kind->aware) {
        /* Standard Value of an item */
        take->value = kind->cost;

        /* Money needs a value */
        if (take->tval == TV_GOLD)
            take->value = 30;
    } else {
        take->value = 1;
    }

    /* Cheat to see if this item is ID'd or not.  We use this cheat to avoid
     * dumping an item which we know to be bad then turning around and picking
     * it up again.
     */
    if (object_fully_known(o_ptr)
        && (o_ptr->to_a < 0 || o_ptr->to_d < 0 || o_ptr->to_h < 0))
        take->value = -10;

    if (o_ptr->note && prefix(quark_str(o_ptr->note), "borg ignore"))
        take->value = -10;

    /* Note */
    borg_note(format("# Creating an object '%s' at (%d,%d)", (take->kind->name),
        take->x, take->y));

    /* Wipe goals only if I have some light source */
    if (borg.trait[BI_CURLITE])
        borg.goal.type = 0;

    /* Hack -- Force the object to sit on a floor grid */
    ag->feat = FEAT_FLOOR;

    /* Result */
    return n;
}

/*
 * Attempt to notice a changing "take"
 */
bool observe_take_diff(int y, int x, uint8_t a, wchar_t c)
{
    int                 i;
    struct object_kind *kind;

    borg_take *take;

    /* Guess the kind */
    kind = borg_guess_kind(a, c, y, x);

    /* Oops */
    if (!kind)
        return false;

    /* no new takes if hallucinations */
    if (borg.trait[BI_ISIMAGE])
        return false;

    /* Make a new object */
    i = borg_new_take(kind, y, x);

    /* Get the object */
    take = &borg_takes[i];

    /* Timestamp */
    take->when = borg_t;

    /* Okay */
    return true;
}

/*
 * Attempt to "track" a "take" at the given location
 * Assume that the object has not moved more than "d" grids
 * Note that, of course, objects are never supposed to move,
 * but we may want to take account of "falling" missiles later.
 */
bool observe_take_move(int y, int x, int d, uint8_t a, wchar_t c)
{
    int i, z, ox, oy;

    struct object_kind *k_ptr;

    /* Scan the objects */
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take *take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind)
            continue;

        /* Skip assigned objects */
        if (take->seen)
            continue;

        /* Extract old location */
        ox = take->x;
        oy = take->y;

        /* Calculate distance */
        z = borg_distance(oy, ox, y, x);

        /* Possible match */
        if (z > d)
            continue;

        /* Access the kind */
        k_ptr = take->kind;

        /* Require matching char if not hallucinating*/
        if (!borg.trait[BI_ISIMAGE] && c != k_ptr->d_char)
            continue;

        /* Require matching attr if not hallucinating rr9*/
        if (!borg.trait[BI_ISIMAGE] && a != k_ptr->d_attr
            && (k_ptr->d_attr != 11 && k_ptr->d_char == '!')
            /* There are serious bugs with Flasks of Oil not having the attr set
               correctly */
        ) {
            /* Ignore "flavored" colors */
            if (!k_ptr->flavor)
                continue;
        }

        /* Actual movement (?) */
        if (z) {
            /* Update the grids */
            borg_grids[take->y][take->x].take = 0;

            /* Track it */
            take->x = x;
            take->y = y;

            /* Update the grids */
            borg_grids[take->y][take->x].take = i;

            /* Note */
            borg_note(
                format("# Tracking an object '%s' at (%d,%d) from (%d,%d)",
                    (k_ptr->name), take->y, take->x, ox, oy));

            /* Clear goals */
            if (borg.goal.type == GOAL_TAKE)
                borg.goal.type = 0;
        }

        /* Timestamp */
        take->when = borg_t;

        /* Mark as seen */
        take->seen = true;

        /* Mark floor underneath */
        borg_grids[take->y][take->x].feat = FEAT_FLOOR;

        /* Done */
        return true;
    }

    /* Oops */
    return false;
}

/*
 * Prepare to "flow" towards objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take(bool viewable, int nearness)
{
    int i, x, y;
    int b_stair = -1, j, b_j = -1;
    int leash = borg.trait[BI_CLEVEL] * 3 + 9;
    int full_quiver;

    borg_grid *ag;

    /* Missile carry limit */
    /* allow shooters to two quiver slots full */
    if (borg.trait[BI_FAST_SHOTS])
        full_quiver = (z_info->quiver_slot_size - 1) * 2;
    else
        full_quiver = z_info->quiver_slot_size - 1;

    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt)
        return false;

    /* Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty)
        return false;

    /* If ScaryGuy, no chasing down items */
    if (scaryguy_on_level)
        return false;

    /* If out of fuel, don't mess around */
    if (!borg.trait[BI_CURLITE])
        return false;

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position)
        return false;

    /* increase leash */
    if (borg.trait[BI_CLEVEL] >= 20)
        leash = 250;

    /* Starting over on count */
    borg_temp_n = 0;

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

    /* Scan the object list */
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take *take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind)
            continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* Skip ones that make me wander too far */
        if (b_stair != -1 && borg.trait[BI_CLEVEL] < 10) {
            /* Check the distance of this 'take' to the stair */
            j = borg_distance(
                track_less.y[b_stair], track_less.x[b_stair], y, x);
            /* skip far away takes while I am close to stair*/
            if (b_j <= leash && j >= leash)
                continue;
        }

        /* skip worthless items */
        if (take->value <= 0)
            continue;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW))
            continue;

        /* Don't bother with ammo if I am at capacity */

        if (take->tval == borg.trait[BI_AMMO_TVAL]
            && borg.trait[BI_AMISSILES] >= full_quiver)
            continue;
        /* No need to chase certain things down after a certain amount.  Don't
         * chase: Money Other spell books Wrong ammo
         */
        if (borg.trait[BI_GOLD] >= 500000) {
            if (take->tval == TV_GOLD)
                continue;
            if (tval_is_book_k(&k_info[take->kind->kidx])
                && !obj_kind_can_browse(&k_info[take->kind->kidx]))
                continue;
            if ((take->tval == TV_SHOT || take->tval == TV_ARROW
                    || take->tval == TV_BOLT)
                && take->tval != borg.trait[BI_AMMO_TVAL])
                continue;
            /*
            Restore Mana for warriors?
            low level potions
            low level scrolls
            */
        }

        /* Clear the flow codes */
        borg_flow_clear();


        /* Check the distance to stair for this proposed grid and leash*/
        if (nearness > 5 && borg.trait[BI_CLEVEL] < 20
            && borg_flow_cost_stair(y, x, b_stair) > leash)
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unkown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("item", GOAL_TAKE))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_scum(bool viewable, int nearness)
{
    int i, x, y;
    int j;
    int b_j     = -1;
    int b_stair = -1;

    borg_grid *ag;

    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt)
        return false;

    /* Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty)
        return false;

    /* Nothing yet */
    borg_temp_n = 0;

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

    /* Scan the object list -- set filter*/
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take *take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind)
            continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* skip worthless items */
        if (take->value <= 0)
            continue;

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW))
            continue;

        /* don't go too far from the stairs */
        if (borg_flow_far_from_stairs(x, y, b_stair))
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unknown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, true);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Scum item", GOAL_TAKE))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_lunal(bool viewable, int nearness)
{
    int i, ii, x, y;
    int j;
    int b_j     = -1;
    int b_stair = -1;

    borg_grid *ag;

    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt)
        return false;

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

    /* Nothing yet */
    borg_temp_n = 0;

    /* Scan the object list -- set filter*/
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take          *take  = &borg_takes[i];
        struct object_kind *k_ptr = take->kind;

        bool item_bad;

        /* Skip dead objects */
        if (!k_ptr)
            continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* all items start bad */
        item_bad = true;

        /* Gold is good to have */
        if (take->tval == TV_GOLD) {
            borg_note(
                format("# Lunal Item %s, at %d,%d", take->kind->name, y, x));
            item_bad = false;
        }

        /* If full can I absorb the item into an existing stack */
        if (item_bad && take->value > 0) {
            if (borg_is_ammo(take->tval)) {
                /* Scan the quiver */
                for (ii = QUIVER_START; ii < QUIVER_END; ii++) {
                    /* skip empty slots */
                    if (!borg_items[ii].iqty)
                        continue;

                    /* skip full slots */
                    if (borg_items[ii].iqty == z_info->quiver_slot_size)
                        continue;

                    /* Both objects should have the same ID value */
                    if (take->kind->kidx != borg_items[ii].kind)
                        continue;

                    if (k_ptr->sval == borg_items[ii].sval
                        && k_ptr->tval == borg_items[ii].tval) {
                        item_bad = false;
                    }
                }
            } else if (borg_items[PACK_SLOTS - 1].iqty) {
                /* Scan the inventory */
                for (ii = 0; ii < PACK_SLOTS; ii++) {
                    /* skip empty slots */
                    if (!borg_items[ii].iqty)
                        continue;

                    /* Both objects should have the same ID value */
                    if (take->kind->kidx != borg_items[ii].kind)
                        continue;

                    /* Certain types of items can stack */
                    if (k_ptr->sval == borg_items[ii].sval
                        && k_ptr->tval == borg_items[ii].tval
                        && (borg_items[ii].tval == TV_POTION
                            || borg_items[ii].tval == TV_SCROLL
                            || borg_items[ii].tval == TV_ROD)) {
                        item_bad = false;
                    }
                }
            }
        }

        /* Require one empty slot */
        if (!borg_items[PACK_SLOTS - 1].iqty && item_bad == true) {
            /* for ammo, make sure the quiver isn't full */
            if (!borg_is_ammo(take->tval)
                || borg_items[QUIVER_END - 1].iqty == 0) {

                /* Certain Potions are worthless */
                if (take->tval == TV_POTION
                    && (take->kind->sval >= sv_potion_inc_str)
                    && (take->kind->sval <= sv_potion_detect_invis)) {
                    borg_note(format(
                        "# Lunal Item %s, at %d,%d", take->kind->name, y, x));
                    item_bad = false;
                }

                /* Certain insta_arts are good.  Note that there is no top end
                 * of this.  So if an item were added after the last artifact,
                 * it would also be picked up.
                 */
                if (kf_has(take->kind->kind_flags, KF_INSTA_ART)) {
                    borg_note(format(
                        "# Lunal Item %s, at %d,%d", take->kind->name, y, x));
                    item_bad = false;
                }

                /* if scumming the start of the game, take all items to sell
                 * them */
                if (borg_cfg[BORG_MUNCHKIN_START]) {
                    /* Certain known items are junky and should be ignored. Grab
                     * only things of value
                     */
                    if (take->value >= 1)
                        item_bad = false;
                }
            }
        }

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW))
            continue;

        /* don't go too far from the stairs */
        if (borg_flow_far_from_stairs(x, y, b_stair))
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unknown squares.  This was for performance. */
    borg_flow_spread(nearness, false, !viewable, false, -1, true);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("munchkin item", GOAL_TAKE))
        return false;

    /* Check for monsters before walking over to the item */
    if (borg_check_light())
        return true;

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE))
        return false;

    /* Success */
    return true;
}

void borg_init_flow_take(void)
{
    /*** Object tracking ***/

    /* No objects yet */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* Array of objects */
    borg_takes = mem_zalloc(256 * sizeof(borg_take));
}

void borg_free_flow_take(void)
{
    mem_free(borg_takes);
    borg_takes = NULL;
}

#endif
