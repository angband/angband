/*
 * File: slays.c
 * Purpose: encapsulation of slay_table and accessor functions for slays/brands
 *
 * Copyright (c) 2010 Chris Carr
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
#include "effects.h"
#include "monster/monster.h"
#include "spells.h"
#include "slays.h"

/*
 * list[] and mult[] must be > 16 in size
 */
static int collect_slays(const char *desc[], int mult[], bitflag *flags)
{
        int cnt = 0;
        u16b i;
        const slays *s_ptr;

        /* Remove "duplicate" flags e.g. *slay* and slay the same
         * monster type
         */
        for (i = 0; i < N_ELEMENTS(slay_dups); i++) {
                if (of_has(flags, slay_dups[i].minor) &&
                                of_has(flags, slay_dups[i].major)) {
                        of_off(flags, slay_dups[i].minor);
                }
        }

        /* Collect slays */
        for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++) {
                if (of_has(flags, s_ptr->slay_flag)) {
                        mult[cnt] = s_ptr->mult;
                        desc[cnt++] = s_ptr->desc;
                }
        }

        return cnt;
}

