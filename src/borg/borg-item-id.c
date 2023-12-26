/**
 * \file borg-item-id.c
 * \brief Code around identification of items (objects)
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

#include "borg-item-id.h"

#ifdef ALLOW_BORG

#include "../obj-util.h"
#include "../ui-menu.h"

#include "borg-io.h"
#include "borg-item-analyze.h"
#include "borg-item.h"
#include "borg-trait.h"

/*
 * Get the ID information
 *
 * This function pulls the information from the screen if it is not passed
 * a *real* item.  It is only passed in *real* items if the borg is allowed
 * to 'cheat' for inventory.
 * This function returns true if space needs to be pressed
 */
bool borg_object_fully_id_aux(borg_item *item, struct object *real_item)
{
    bitflag f[OF_SIZE];
    bitflag i = OF_SIZE;

    /* the data directly from the real item    */
    object_flags(real_item, f);
    for (i = 0; i < 12 && i < OF_SIZE; i++)
        item->flags[i] = f[i];

    return (false);
}

/*
 * Look for an item that needs to be analysed because it has been IDd
 *
 * This will go through inventory and look for items that were just ID'd
 * and examine them for their bonuses.
 */
bool borg_object_fully_id(void)
{
    int i;

    /* look in inventory and equiptment for something to *id* */
    for (i = 0; i < QUIVER_END; i++) /* or INVEN_TOTAL */
    {
        borg_item       *item  = &borg_items[i];
        struct ego_item *e_ptr = &e_info[item->ego_idx];

        /* inscribe certain objects */
        const char *note = borg_get_note(item);
        if (borg.trait[BI_CDEPTH] && item->tval >= TV_BOW
            && item->tval <= TV_RING
            && (borg_ego_has_random_power(e_ptr) || item->art_idx)
            && (streq(note, "{ }") || streq(note, "")
                || strstr(note, "uncursed"))) {

            /* make the inscription */
            borg_keypress('{');

            if (i >= INVEN_WIELD) {
                borg_keypress('/');
                borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
            } else {
                borg_keypress(all_letters_nohjkl[i]);
            }

            /* make the inscription */
            if (item->modifiers[OBJ_MOD_SPEED]) {
                borg_keypresses("Spd");
            }
            /* slays and immunities */
            if (item->el_info[ELEM_POIS].res_level > 0) {
                borg_keypresses("Poisn");
            }
            if (item->el_info[ELEM_FIRE].res_level == 3) {
                borg_keypresses("IFir");
            }
            if (item->el_info[ELEM_COLD].res_level == 3) {
                borg_keypresses("ICld");
            }
            if (item->el_info[ELEM_ACID].res_level == 3) {
                borg_keypresses("IAcd");
            }
            if (item->el_info[ELEM_ELEC].res_level == 3) {
                borg_keypresses("IElc");
            }
            if (item->el_info[ELEM_LIGHT].res_level > 0) {
                borg_keypresses("Lite");
            }
            if (item->el_info[ELEM_DARK].res_level > 0) {
                borg_keypresses("Dark");
            }
            if (of_has(item->flags, OF_PROT_BLIND)) {
                borg_keypresses("Blnd");
            }
            if (of_has(item->flags, OF_PROT_CONF)) {
                borg_keypresses("Conf");
            }
            if (item->el_info[ELEM_SOUND].res_level > 0) {
                borg_keypresses("Sound");
            }
            if (item->el_info[ELEM_SHARD].res_level > 0) {
                borg_keypresses("Shrd");
            }
            if (item->el_info[ELEM_NETHER].res_level > 0) {
                borg_keypresses("Nthr");
            }
            if (item->el_info[ELEM_NEXUS].res_level > 0) {
                borg_keypresses("Nxs");
            }
            if (item->el_info[ELEM_CHAOS].res_level > 0) {
                borg_keypresses("Chaos");
            }
            if (item->el_info[ELEM_DISEN].res_level > 0) {
                borg_keypresses("Disn");
            }
            /* TR2_activate was removed */
            if (item->activ_idx) {
                borg_keypresses("Actv");
            }
            if (of_has(item->flags, OF_TELEPATHY)) {
                borg_keypresses("ESP");
            }
            if (of_has(item->flags, OF_HOLD_LIFE)) {
                borg_keypresses("HL");
            }
            if (of_has(item->flags, OF_FREE_ACT)) {
                borg_keypresses("FA");
            }
            if (of_has(item->flags, OF_SEE_INVIS)) {
                borg_keypresses("SInv");
            }

            /* end the inscription */
            borg_keypress(KC_ENTER);
        }
    }
    return true;
}

/*
 * The code currently inscribes items with {??} if they have unknown powers.
 *
 * This helper keeps the check isolated in case this ever changes
 */
bool borg_item_note_needs_id(const borg_item *item)
{
    if (item->ident)
        return false;

    /* save a string check */
    if (item->needs_ident)
        return true;

    return strstr(borg_get_note(item), "{??}");
}

#endif
