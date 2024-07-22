/**
 * \file borg-light.c
 * \brief Handle light and lighting
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

#include "borg-light.h"

#ifdef ALLOW_BORG

#include "../game-input.h"
#include "../ui-menu.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-messages-react.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Check to see if the surrounding dungeon should be illuminated, and if
 * it should, do it.
 *
 * Always case light when we have no torch.
 *
 * This is when resting to heal.  I don't want him teleporting into a room,
 * resting to heal while there is a dragon sitting in a dark corner waiting
 * to breathe on him.
 *
 * Returns true if it did something, false otherwise
 */
bool borg_check_light_only(void)
{
    /* Never in town, when blind or when hallucinating */
    if (!borg.trait[BI_CDEPTH])
        return false;
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISIMAGE])
        return false;

    /** Use wizard light sometimes **/

    if (!borg.when_wizard_light || (borg_t - borg.when_wizard_light >= 1000)) {
        if (borg_activate_item(act_clairvoyance)
            || borg_activate_item(act_enlightenment)
            || borg_spell_fail(FUME_OF_MORDOR, 40)
            || borg_spell_fail(CLAIRVOYANCE, 40)) {
            borg_note("# Wizard lighting the dungeon");
            /* borg_react("SELF:wizard lite", "SELF:wizard lite"); */
            borg.when_wizard_light = borg_t;
            return true;
        }
    }

    /** Work out if there's any reason to light */

    /* Don't bother because we only just did it */
    if (borg.when_call_light != 0 && (borg_t - borg.when_call_light) < 7)
        return false;

    if (borg.trait[BI_CURLITE] == 1) {
        int i;
        int corners = 0;

        /* Scan diagonal neighbors.
         *
         * 4 corners   3 corners    2 corners    1 corner    0 corners
         * ###         ##.  #..     ##.  #..     .#.         .#.  ... .#.
         * .@.         .@.  .@.     .@.  .@.     .@.         #@#  .@. .@.
         * ###         ###  ###     ##.  #..     ##.         .#.  ... .#.
         *
         * There's actually no way to tell which are rooms and which are
         * corridors from diagonals except 4 (always a corridor) and
         * 0 (always a room).  This is why we only use it for radius 1 light.
         */
        for (i = 4; i < 8; i++) {
            borg_grid *ag;

            /* Get location */
            int x = borg.c.x + ddx_ddd[i];
            int y = borg.c.y + ddy_ddd[i];

            /* Bounds check */
            if (!square_in_bounds_fully(cave, loc(x, y)))
                continue;

            /* Get grid */
            ag = &borg_grids[y][x];

            /* Location must be known */
            if (ag->feat == FEAT_NONE)
                corners++;

            /* Location must be a wall/door */
            if (!borg_cave_floor_grid(ag))
                corners++;
        }

        /* This is quite an arbitrary cutoff */
        if (corners > 2)
            return false;
    } else if (borg.trait[BI_CURLITE] > 1) {
        int x, y;
        int floors = 0;

        /*
         * Scan the surrounding 5x5 area for unlit tiles.
         *
         * Radius two light misses out the four corners but otherwise
         * illuminates a 5x5 grid, which is 21 grids illuminated incl player.
         *
         *  ...
         * .....
         * ..@..
         * .....
         *  ...
         */
        for (y = borg.c.y - 2; y <= borg.c.y + 2; y++) {
            for (x = borg.c.x - 2; x <= borg.c.x + 2; x++) {
                borg_grid *ag;

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(x, y)))
                    continue;

                /* Get grid */
                ag = &borg_grids[y][x];

                /* Must be a floor grid lit by torchlight, not by magic */
                if (borg_cave_floor_grid(ag) && (ag->info & BORG_LIGHT)
                    && !square_isglow(cave, loc(x, y))) {
                    floors++;
                }
            }
        }

        /* Don't bother unless there are enough unlit floors */
        /* 11 is the empirical cutoff point for sensible behaviour here */
        if (floors < 11)
            return false;
    }

    /* Light it up! */
    if (borg_activate_item(act_illumination) 
        || borg_activate_item(act_light)
        || borg_zap_rod(sv_rod_illumination) 
        || borg_use_staff(sv_staff_light)
        || borg_read_scroll(sv_scroll_light) 
        || borg_spell_fail(LIGHT_ROOM, 40)
        || borg_spell_fail(CALL_LIGHT, 40)) {
        borg_note("# Illuminating the dungeon");
        borg_react("SELF:lite", "SELF:lite");
        borg.when_call_light = borg_t;
        return true;
    }

    return false;
}

/*
 * Refuel, call lite, detect traps/doors/walls/evil, etc
 *
 * Note that we refuel whenever our lite starts to get low.
 *
 * Note that we detect traps/doors/walls/evil at least once in each
 * panel, as soon as possible after entering a new panel.
 *
 * We use the special "SELF" messages to "borg_react()" to delay the
 * processing of DETECTION and "call lite" until we know if it has
 * worked or not.
 */
bool borg_check_light(void)
{
    int q_x, q_y;

    bool do_trap;
    bool do_door;
    bool do_wall;
    bool do_evil;
    bool do_obj;

    /* Never in town when mature (scary guy)*/
    if (borg.trait[BI_MAXCLEVEL] > 10 && !borg.trait[BI_CDEPTH])
        return false;

    /* Never when compromised, save your mana */
    if (borg.trait[BI_ISBLIND] 
        || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE] 
        || borg.trait[BI_ISPOISONED]
        || borg.trait[BI_ISCUT] 
        || borg.trait[BI_ISWEAK])
        return false;

    /* XXX XXX XXX Dark */

    /* Extract the panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* Start */
    do_trap = false;

    /* Determine if we need to detect traps */
    if (!borg_detect_trap[q_y + 0][q_x + 0])
        do_trap = true;
    if (!borg_detect_trap[q_y + 0][q_x + 1])
        do_trap = true;
    if (!borg_detect_trap[q_y + 1][q_x + 0])
        do_trap = true;
    if (!borg_detect_trap[q_y + 1][q_x + 1])
        do_trap = true;

    /* Hack -- check traps every few turns anyway */
    /* if (!when_detect_traps || (borg_t - when_detect_traps >= 183)) do_trap =
     * true; */

    /* Start */
    do_door = false;

    /* Determine if we need to detect doors */
    if (!borg_detect_door[q_y + 0][q_x + 0])
        do_door = true;
    if (!borg_detect_door[q_y + 0][q_x + 1])
        do_door = true;
    if (!borg_detect_door[q_y + 1][q_x + 0])
        do_door = true;
    if (!borg_detect_door[q_y + 1][q_x + 1])
        do_door = true;

    /* Hack -- check doors every few turns anyway */
    /* if (!when_detect_doors || (borg_t - when_detect_doors >= 731)) do_door =
     * true; */

    /* Start */
    do_wall = false;

    /* Determine if we need to detect walls */
    if (!borg_detect_wall[q_y + 0][q_x + 0])
        do_wall = true;
    if (!borg_detect_wall[q_y + 0][q_x + 1])
        do_wall = true;
    if (!borg_detect_wall[q_y + 1][q_x + 0])
        do_wall = true;
    if (!borg_detect_wall[q_y + 1][q_x + 1])
        do_wall = true;

    /* Hack -- check walls every few turns anyway */
    /* if (!when_detect_walls || (borg_t - when_detect_walls >= 937)) do_wall =
     * true; */

    /* Start */
    do_evil = false;

    /* Determine if we need to detect evil */
    if (!borg_detect_evil[q_y + 0][q_x + 0])
        do_evil = true;
    if (!borg_detect_evil[q_y + 0][q_x + 1])
        do_evil = true;
    if (!borg_detect_evil[q_y + 1][q_x + 0])
        do_evil = true;
    if (!borg_detect_evil[q_y + 1][q_x + 1])
        do_evil = true;

    /* Start */
    do_obj = false;

    /* Determine if we need to detect evil */
    if (!borg_detect_obj[q_y + 0][q_x + 0])
        do_obj = true;
    if (!borg_detect_obj[q_y + 0][q_x + 1])
        do_obj = true;
    if (!borg_detect_obj[q_y + 1][q_x + 0])
        do_obj = true;
    if (!borg_detect_obj[q_y + 1][q_x + 1])
        do_obj = true;

    /* Hack -- check evil every few turns anyway- more fq if low level */
    /* if (!when_detect_evil ||
       (borg_t - when_detect_evil  >= 183 - (80 - borg.trait[BI_MAXCLEVEL])))
       do_evil = true; */

    /* Really low level */
    /* if (borg.trait[BI_CLEVEL] <= 3 &&
        (!when_detect_evil ||
        (borg_t - when_detect_evil  >= 50))) do_evil = true; */

    /* Not too frequent in town */
    /* if (borg.trait[BI_CDEPTH] == 0 &&
        (!when_detect_evil ||
        (borg_t - when_detect_evil  >= 250))) do_evil = true; */

    /* Dont bother if I have ESP */
    if (borg.trait[BI_ESP])
        do_evil = false;

    /* Only look for monsters in town, not walls, etc (scary guy)*/
    if (!borg.trait[BI_CDEPTH]) {
        do_trap = false;
        do_door = false;
        do_wall = false;
    }

    /*** Do Things ***/

    /* Hack -- find traps and doors and evil*/
    if ((do_trap || do_door || do_evil)
        && ((!borg.when_detect_traps || (borg_t - borg.when_detect_traps >= 5))
            || (!borg.when_detect_evil || (borg_t - borg.when_detect_evil >= 5))
            || (!borg.when_detect_doors
                || (borg_t - borg.when_detect_doors >= 5)))
        && borg.trait[BI_CDEPTH]) /* Never in town */
    {

        /* Check for traps and doors and evil*/
        if (borg_activate_item(act_detect_all)
            || borg_activate_item(act_mapping) 
            || borg_zap_rod(sv_rod_detection)
            || borg_spell_fail(SENSE_SURROUNDINGS, 40)) {
            borg_note("# Checking for traps, doors, and evil.");

            borg_react("SELF:TDE", "SELF:TDE");

            borg.when_detect_traps = borg_t;
            borg.when_detect_doors = borg_t;
            borg.when_detect_evil  = borg_t;
            borg.when_detect_obj   = borg_t;

            return true;
        }
    }

    /* Hack -- find evil */
    if (do_evil
        && (!borg.when_detect_evil || (borg_t - borg.when_detect_evil >= 20))) {
        /* Check for evil */
        if (borg_spell_fail(DETECT_EVIL, 40)
            || borg_spell_fail(DETECT_MONSTERS, 40)
            || borg_spell_fail(READ_MINDS, 40)
            || borg_spell_fail(SEEK_BATTLE, 40)) {
            borg_note("# Checking for monsters.");

            borg_react("SELF:evil", "SELF:evil");

            borg.when_detect_evil = borg_t;

            return true;
        }
    }

    /* Hack -- find traps and doors (and stairs) */
    if ((do_trap || do_door)
        && ((!borg.when_detect_traps || (borg_t - borg.when_detect_traps >= 5))
            || (!borg.when_detect_doors
                || (borg_t - borg.when_detect_doors >= 5)))
        && borg.trait[BI_CDEPTH]) /* Never in town */
    {
        /* Check for traps and doors */
        if (borg_activate_item(act_detect_all)
            || borg_activate_item(act_mapping) 
            || borg_spell_fail(DETECTION, 40)
            || borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40)
            || borg_spell_fail(DETECT_STAIRS, 40)) {
            borg_note("# Checking for traps, doors & stairs.");

            borg_react("SELF:both", "SELF:both");

            borg.when_detect_traps = borg_t;
            borg.when_detect_doors = borg_t;

            return true;
        }
    }

    /* Hack -- find traps */
    if (do_trap
        && (!borg.when_detect_traps || (borg_t - borg.when_detect_traps >= 7))
        && borg.trait[BI_CDEPTH]) /* Never in town */
    {
        /* Check for traps */
        if (borg_spell_fail(DETECTION, 40)
            || borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40)) {
            borg_note("# Checking for traps.");

            borg_react("SELF:trap", "SELF:trap");

            borg.when_detect_traps = borg_t;

            return true;
        }
    }

    /* Hack -- find doors */
    if (do_door
        && (!borg.when_detect_doors || (borg_t - borg.when_detect_doors >= 9))
        && borg.trait[BI_CDEPTH]) /* Never in town */
    {
        /* Check for traps */
        if (borg_activate_item(act_detect_all)
            || borg_activate_item(act_mapping) || borg_spell_fail(DETECTION, 40)
            || borg_spell_fail(FIND_TRAPS_DOORS_STAIRS, 40)) {
            borg_note("# Checking for doors.");

            borg_react("SELF:door", "SELF:door");

            borg.when_detect_doors = borg_t;

            return true;
        }
    }

    /* Hack -- find walls */
    if (do_wall
        && (!borg.when_detect_walls || (borg_t - borg.when_detect_walls >= 15))
        /* Never in town */
        && borg.trait[BI_CDEPTH]) {
        /* Check for walls */
        if (borg_activate_item(act_mapping)
            || borg_read_scroll(sv_scroll_mapping)
            || borg_use_staff(sv_staff_mapping) || borg_zap_rod(sv_rod_mapping)
            || borg_spell(SENSE_SURROUNDINGS)) {
            int y, x;

            borg_note("# Checking for walls.");

            borg_react("SELF:wall", "SELF:wall");

            borg.when_detect_walls = borg_t;

            /*
             * Clear the BORG_IGNORE_MAP flag:  immediately after detection
             * want to use the map's information rather than what the borg
             * remembers.
             */
            for (y = 0; y < AUTO_MAX_Y; ++y) {
                for (x = 0; x < AUTO_MAX_X; ++x) {
                    borg_grids[y][x].info &= ~BORG_IGNORE_MAP;
                }
            }
            return true;
        }
    }

    /* Hack -- find objects */
    if (do_obj
        && (!borg.when_detect_obj || (borg_t - borg.when_detect_obj >= 20))) {
        /* Check for objects */
        if (borg_activate_item(act_detect_objects)
            || borg_spell_fail(OBJECT_DETECTION, 40)) {
            borg_note("# Checking for objects.");

            borg_react("SELF:obj", "SELF:obj");

            borg.when_detect_obj = borg_t;

            return true;
        }
    }

    /* Do the actual illumination bit */
    return borg_check_light_only();
}

/*
 * Hack -- refuel a lantern
 */
static bool borg_refuel_lantern(void)
{
    int i;

    /* Look for a flask of oil */
    i = borg_slot(TV_FLASK, sv_flask_oil);

    /* None available check for lantern */
    if (i < 0) {
        i = borg_slot(TV_LIGHT, sv_light_lantern);

        /* It better have some oil left */
        if (i >= 0 && borg_items[i].timeout <= 0)
            i = -1;
    }

    /* Still none */
    if (i < 0)
        return false;

    /* Cant refuel a torch with oil */
    if (borg_items[INVEN_LIGHT].sval != sv_light_lantern) {
        return false;
    }

    /* Log the message */
    borg_note(format("# Refueling with %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Refuel the light if it is needed
 */
enum borg_need borg_maintain_light(void)
{
    int        i;
    borg_item *current_light = &borg_items[INVEN_LIGHT];

    if (of_has(current_light->flags, OF_NO_FUEL))
        return BORG_NO_NEED;

    /*  current torch */
    if (current_light->tval == TV_LIGHT) {
        if (current_light->sval == sv_light_torch) {
            if (current_light->timeout > 250) {
                return BORG_NO_NEED;
            } else {
                /* Look for another torch */
                i = borg_slot(TV_LIGHT, sv_light_torch);
                if (i < 0)
                    return BORG_UNMET_NEED;

                /* Torches automatically disappear when they get to 0 turns
                 * so we don't need to actively swap them out */
                return BORG_NO_NEED;
            }
        }

        /* Refuel current lantern */
        if (current_light->sval == sv_light_lantern) {
            /* Refuel the lantern if needed */
            if (borg_items[INVEN_LIGHT].timeout < 1000) {
                if (borg_refuel_lantern())
                    return BORG_MET_NEED;

                return BORG_UNMET_NEED;
            }
        }
        return BORG_NO_NEED;
    } else {
        i = borg_slot(TV_LIGHT, sv_light_lantern);
        if (i < 0) {
            i = borg_slot(TV_LIGHT, sv_light_torch);
        }

        if (i < 0) {
            return BORG_UNMET_NEED;
        } else {
            borg_keypress('w');
            borg_keypress(all_letters_nohjkl[i]);
            return BORG_MET_NEED;
        }
    }
}

/*
 * This will look down a hallway and possibly light it up using
 * the Light Beam mage spell.  This spell is mostly used when
 * the borg is moving through the dungeon under boosted bravery.
 * This will allow him to "see" if anyone is there.
 *
 * It might also come in handy if he's in a hallway and gets shot, or
 * if resting in a hallway.  He may want to cast it to make
 * sure no previously unknown monsters are in the hall.
 * NOTE:  ESP will alter the value of this spell.
 *
 * Borg has a problem when not on map centering mode and casting the beam
 * repeatedly, down or up when at the edge of a panel.
 */
bool borg_light_beam(bool simulation)
{
    int  dir      = 5;
    bool spell_ok = false;
    int  i;
    bool blocked  = false;
    bool bold     = false;

    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    /* Hack -- weak/dark is very unhappy */
    if (borg.trait[BI_ISWEAK])
        return false;

    /* Require the ability */
    if (borg_spell_okay_fail(SPEAR_OF_LIGHT, 20)
        || (-1 != borg_slot(TV_WAND, sv_wand_light)
            && borg_items[borg_slot(TV_WAND, sv_wand_light)].pval)
        || borg_equips_rod(sv_rod_light))
        spell_ok = true;

    /*** North Direction Test***/

    /* Quick Boundary check */
    if (borg.c.y - borg.trait[BI_CURLITE] - 1 > 0) {
        /* Look just beyond my light */
        ag = &borg_grids[borg.c.y - borg.trait[BI_CURLITE] - 1][borg.c.x];
        bold = borg_cave_floor_bold(
            borg.c.y - borg.trait[BI_CURLITE] - 1, borg.c.x);

        /* Must be on the panel */
        if (panel_contains(borg.c.y - borg.trait[BI_CURLITE] - 1, borg.c.x)) {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg.trait[BI_CURLITE]; i++) {
                if (borg_cave_floor_bold(borg.c.y - i, borg.c.x) && !bold
                    && ag->feat < FEAT_SECRET && ag->feat != FEAT_CLOSED
                    && blocked == false) {
                    /* note the direction */
                    dir = 8;
                } else {
                    dir     = 5;
                    blocked = true;
                    break;
                }
            }
        }
    }

    /*** South Direction Test***/

    /* Quick Boundary check */
    if (borg.c.y + borg.trait[BI_CURLITE] + 1 < AUTO_MAX_Y && dir == 5) {
        blocked = false;
        /* Look just beyond my light */
        ag = &borg_grids[borg.c.y + borg.trait[BI_CURLITE] + 1][borg.c.x];
        bold = borg_cave_floor_bold(
            borg.c.y + borg.trait[BI_CURLITE] + 1, borg.c.x);

        /* Must be on the panel */
        if (panel_contains(borg.c.y + borg.trait[BI_CURLITE] + 1, borg.c.x)) {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg.trait[BI_CURLITE]; i++) {
                /* all floors */
                if (borg_cave_floor_bold(borg.c.y + i, borg.c.x) && !bold
                    && ag->feat < FEAT_SECRET && ag->feat != FEAT_CLOSED
                    && blocked == false) {
                    /* note the direction */
                    dir = 2;
                } else {
                    dir     = 5;
                    blocked = true;
                    break;
                }
            }
        }
    }

    /*** East Direction Test***/

    /* Quick Boundary check */
    if (borg.c.x + borg.trait[BI_CURLITE] + 1 < AUTO_MAX_X && dir == 5) {
        blocked = false;
        /* Look just beyond my light */
        ag = &borg_grids[borg.c.y][borg.c.x + borg.trait[BI_CURLITE] + 1];
        bold = borg_cave_floor_bold(
            borg.c.y, borg.c.x + borg.trait[BI_CURLITE] + 1);

        /* Must be on the panel */
        if (panel_contains(borg.c.y, borg.c.x + borg.trait[BI_CURLITE] + 1)) {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg.trait[BI_CURLITE]; i++) {
                /* all floors */
                if (borg_cave_floor_bold(borg.c.y, borg.c.x + i) && !bold
                    && ag->feat < FEAT_SECRET && ag->feat != FEAT_CLOSED
                    && blocked == false) {
                    /* note the direction */
                    dir = 6;
                } else {
                    dir     = 5;
                    blocked = true;
                    break;
                }
            }
        }
    }

    /*** West Direction Test***/

    /* Quick Boundary check */
    if (borg.c.x - borg.trait[BI_CURLITE] - 1 > 0 && dir == 5) {
        blocked = false;
        /* Look just beyond my light */
        ag   = &borg_grids[borg.c.y][borg.c.x - borg.trait[BI_CURLITE] - 1];
        bold = borg_cave_floor_bold(
            borg.c.y, borg.c.x - borg.trait[BI_CURLITE] - 1);

        /* Must be on the panel */
        if (panel_contains(borg.c.y, borg.c.x - borg.trait[BI_CURLITE] - 1)) {
            /* Check each grid in our light radius along the course */
            for (i = 1; i <= borg.trait[BI_CURLITE]; i++) {
                /* Verify that there are no blockers in my light radius and
                 * the 1st grid beyond my light is not a floor nor a blocker
                 */
                if (borg_cave_floor_bold(borg.c.y, borg.c.x - i) && !bold
                    && ag->feat < FEAT_SECRET && ag->feat != FEAT_CLOSED
                    && blocked == false) {
                    /* note the direction */
                    dir = 4;
                } else {
                    dir     = 5;
                    blocked = true;
                    break;
                }
            }
        }
    }

    /* Don't do it if on the edge of shifting the panel. */
    if (dir == 5 || spell_ok == false || blocked == true
// !FIX !TODO !AJG make sure these panel edge checks are right.
        || (dir == 2
            && (borg.c.y == 18 
                || borg.c.y == 19 
                || borg.c.y == 29
                || borg.c.y == 30 
                || borg.c.y == 40 
                || borg.c.y == 41
                || borg.c.y == 51 
                || borg.c.y == 52))
        || (dir == 8
            && (borg.c.y == 13 
                || borg.c.y == 14 
                || borg.c.y == 24
                || borg.c.y == 25 
                || borg.c.y == 35 
                || borg.c.y == 36
                || borg.c.y == 46 
                || borg.c.y == 47)))
        return false;

    /* simulation */
    if (simulation)
        return true;

    /* cast the light beam */
    if (borg_spell_fail(SPEAR_OF_LIGHT, 20) || borg_zap_rod(sv_rod_light)
        || borg_aim_wand(sv_wand_light)) {
        /* apply the direction */
        borg_keypress(I2D(dir));
        borg_note("# Illuminating this hallway");
        return true;
    }

    /* cant do it */
    return false;
}

#endif
