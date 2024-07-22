/**
 * \file borg-item.c
 * \brief definitions of the lists of items the borg is tracking
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

#include "borg-item.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../ui-menu.h"

#include "borg-io.h"
#include "borg-item-val.h"

/*
 * Current "inventory"
 */
borg_item *borg_items;

/*
 * Safety arrays for simulating possible worlds
 */

borg_item *safe_items; /* Safety "inventory" */

/*
 * get the items inscription (note)
 */
const char *borg_get_note(const borg_item *item)
{
    if (item->note)
        return item->note;
    return "";
}

/*
 * Send a command to de-inscribe item number "i" .
 */
void borg_deinscribe(int i)
{

    /* Ok to inscribe Slime Molds */
    if (borg_items[i].tval == TV_FOOD
        && borg_items[i].sval == sv_food_slime_mold)
        return;

    /* Label it */
    borg_keypress('}');

    /* Choose from inventory */
    if (i < INVEN_WIELD) {
        /* Choose the item */
        borg_keypress(all_letters_nohjkl[i]);
    }

    /* Choose from equipment */
    else {
        if (i < INVEN_FEET) {
            for (int j = 0; j < INVEN_WIELD; j++) {
                /* Go to equipment (if necessary) */
                if (borg_items[j].iqty && borg_items[j].note[0] == '{') {
                    borg_keypress('/');
                    break;
                }
            }
            /* Choose the item */
            borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);

        } 
        else {
            for (int j = 0; j <= INVEN_FEET; j++) {
                /* Go to quiver (if necessary) */
                if (borg_items[j].iqty && borg_items[j].note[0] == '{') {
                    borg_keypress('|');
                    break;
                }
            }
            /* Choose the item */
            borg_keypress('0' + (i - QUIVER_START));
        }
    }

    /* May ask for a confirmation */
    borg_keypress('y');
    borg_keypress('y');
}

/*
 * allocate the item arrays
 */
void borg_init_item(void)
{
    /*** Item/Ware arrays ***/

    /* Make the inventory array */
    borg_items = mem_zalloc(QUIVER_END * sizeof(borg_item));

    /* Make the "safe" inventory array */
    safe_items = mem_zalloc(QUIVER_END * sizeof(borg_item));
}

/*
 * free the item arrays
 */
void borg_free_item(void)
{
    /*** Item/Ware arrays ***/

    mem_free(safe_items);
    safe_items = NULL;
    mem_free(borg_items);
    borg_items = NULL;
}

#endif
