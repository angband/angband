/**
 * \file borg-prepared.c
 * \brief Check how deep the borg is prepared to go
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

#include "borg-prepared.h"

#ifdef ALLOW_BORG

#include "borg-flow-kill.h"
#include "borg-formulas.h"
#include "borg-home-notice.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait-swap.h"
#include "borg-trait.h"
#include "borg.h"

/* buffer for borg_prepared message
 */
#define MAX_REASON 1024
static char borg_prepared_buffer[MAX_REASON];

/* Track how many uniques are around at your depth */
int          borg_numb_live_unique;
unsigned int borg_living_unique_index;
int          borg_unique_depth;

/*
 * Determine if the Borg meets the "minimum" requirements for a level
 */
static const char *borg_prepared_aux(int depth)
{
    if (-1 == borg.ready_morgoth)
        borg.ready_morgoth = 0;
    if (borg.trait[BI_KING]) {
        borg.ready_morgoth = 1;
        return (NULL);
    }

    /* Always ready for the town */
    if (!depth)
        return (NULL);

    /*** Essential Items for Level 1 ***/

    /* Require lite (any) */
    if (borg.trait[BI_CURLITE] < 1)
        return ("1 Lite");

    /* Require food */
    if (borg.trait[BI_FOOD] < 5)
        return ("5 Food");

    /* Usually ready for level 1 */
    if (depth <= 1)
        return ((char *)NULL);

    /*** Essential Items for Level 2 ***/

    /* Require fuel */
    if (borg.trait[BI_AFUEL] < 5 && !borg.trait[BI_LIGHT])
        return ("5 Fuel");

    /* Require recall */
    /* if (borg.trait[BI_RECALL] < 1) return ("1 recall"); */

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* Require 30 hp */
        if (borg.trait[BI_MAXHP] < 30)
            return ("30 hp");
    }

    /* Usually ready for level 2 */
    if (depth <= 2)
        return ((char *)NULL);

    /*** Essential Items for Level 3 and 4 ***/

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* class specific requirement */
        switch (borg.trait[BI_CLASS]) {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
            if (borg.trait[BI_MAXHP] < 50)
                return ("50 hp");
            if (borg.trait[BI_MAXCLEVEL] < 4)
                return ("4 clevel");
            break;
        case CLASS_ROGUE:
            if (borg.trait[BI_MAXHP] < 50)
                return ("50 hp");
            if (borg.trait[BI_MAXCLEVEL] < 8)
                return ("8 clevel");
            break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
            if (borg.trait[BI_MAXHP] < 40)
                return ("40 hp");
            if (borg.trait[BI_MAXCLEVEL] < 9)
                return ("9 level");
            break;
        case CLASS_PALADIN:
            if (borg.trait[BI_MAXHP] < 50)
                return ("50 hp");
            if (borg.trait[BI_MAXCLEVEL] < 4)
                return ("4 clevel");
            break;
        case CLASS_RANGER:
            if (borg.trait[BI_MAXHP] < 50)
                return ("50 hp");
            if (borg.trait[BI_MAXCLEVEL] < 4)
                return ("4 clevel");
            break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
            if (borg.trait[BI_MAXHP] < 60)
                return ("60 hp");
            if (borg.trait[BI_MAXCLEVEL] < 11)
                return ("11 clevel");
            break;
        }
    }

    /* Potions of Cure Serious Wounds */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && borg.trait[BI_ACLW] + borg.trait[BI_ACSW] + borg.trait[BI_ACCW] < 2)
        return ("2 cure");

    /* Usually ready for level 3 and 4 */
    if (depth <= 4)
        return ((char *)NULL);

    /*** Essential Items for Level 5 to 9 ***/

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* class specific requirement */
        if (borg.trait[BI_CDEPTH]) {
            switch (borg.trait[BI_CLASS]) {
            case CLASS_WARRIOR:
            case CLASS_BLACKGUARD:
                if (borg.trait[BI_MAXHP] < 60)
                    return ("60 hp");
                if (borg.trait[BI_MAXCLEVEL] < 6)
                    return ("6 clevel");
                break;
            case CLASS_ROGUE:
                if (borg.trait[BI_MAXHP] < 60)
                    return ("60 hp");
                if (borg.trait[BI_MAXCLEVEL] < 10)
                    return ("10 clevel");
                break;
            case CLASS_PRIEST:
            case CLASS_DRUID:
                if (borg.trait[BI_MAXHP] < 60)
                    return ("60 hp");
                if (borg.trait[BI_MAXCLEVEL] < 15)
                    return ("15 clevel");
                break;
            case CLASS_PALADIN:
                if (borg.trait[BI_MAXHP] < 60)
                    return ("60 hp");
                if (borg.trait[BI_MAXCLEVEL] < 6)
                    return ("6 clevel");
                break;
            case CLASS_RANGER:
                if (borg.trait[BI_MAXHP] < 60)
                    return ("60 hp");
                if (borg.trait[BI_MAXCLEVEL] < 6)
                    return ("6 clevel");
                break;
            case CLASS_MAGE:
            case CLASS_NECROMANCER:
                if (borg.trait[BI_MAXHP] < 80)
                    return ("80 hp");
                if (borg.trait[BI_MAXCLEVEL] < 15)
                    return ("15 level");
                break;
            }
        }
    }

    /* Potions of Cure Serious/Critical Wounds */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && borg.trait[BI_ACLW] + borg.trait[BI_ACSW] + borg.trait[BI_ACCW] < 2)
        return ("2 cures (clw + csw + ccw)");

    /* Scrolls of Word of Recall */
    if (borg.trait[BI_RECALL] < 1)
        return ("1 recall");

    /* Usually ready for level 5 to 9 */
    if (depth <= 9)
        return ((char *)NULL);

    /*** Essential Items for Level 10 to 19 ***/

    /* Require light (radius 2) */
    if (borg.trait[BI_CURLITE] < 2)
        return "2 light radius";

    /* Escape or Teleport */
    if (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] < 2)
        return ("2 tele + teleport staffs");

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* class specific requirement */
        switch (borg.trait[BI_CLASS]) {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
            if (borg.trait[BI_MAXCLEVEL] < (depth - 4) && depth <= 19)
                return ("dlevel - 4 >= clevel");
            break;
        case CLASS_ROGUE:
            if (borg.trait[BI_MAXCLEVEL] < depth && depth <= 19)
                return ("dlevel >= clevel");
            break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
            if (borg.trait[BI_MAXCLEVEL] < depth && depth <= 19)
                return ("dlevel >= clevel");
            break;
        case CLASS_PALADIN:
            if (borg.trait[BI_MAXCLEVEL] < depth && depth <= 19)
                return ("dlevel >= clevel");
            break;
        case CLASS_RANGER:
            if (borg.trait[BI_MAXCLEVEL] < depth && depth <= 19)
                return ("dlevel >= clevel");
            break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 5)
                && borg.trait[BI_MAXCLEVEL] <= 28)
                return ("dlevel + 5 > = clevel");
            break;
        }
    }

    /* Potions of Cure Critical Wounds */
    if ((borg.trait[BI_MAXCLEVEL] < 30) && borg.trait[BI_ACCW] < 3)
        return ("ccw < 3");

    /* See invisible */
    /* or telepathy */
    if ((!borg.trait[BI_SINV] && !borg.trait[BI_DINV] && !borg.trait[BI_ESP]))
        return ("See Invis : ESP");

    /* Usually ready for level 10 to 19 */
    if (depth <= 19)
        return ((char *)NULL);

    /*** Essential Items for Level 20 ***/

    /* Free action */
    if (!borg.trait[BI_FRACT])
        return ("free action");

    /* ready for level 20 */
    if (depth <= 20)
        return ((char *)NULL);

    /*** Essential Items for Level 25 ***/

    /* must have fire + 2 other basic resists */
    if (!borg.trait[BI_SRFIRE])
        return ("resist fire");
    {
        int basics = borg.trait[BI_RACID] + borg.trait[BI_RCOLD]
                     + borg.trait[BI_RELEC];

        if (basics < 2)
            return ("2 basic resists");
    }
    /* have some minimal stats */
    if (borg.stat_cur[STAT_STR] < 7)
        return ("low STR");

    int spell_stat = borg_spell_stat();
    if (spell_stat != -1) {
        if (borg.stat_cur[spell_stat] < 7)
            return ("low spell stat");
    }
    if (borg.stat_cur[STAT_DEX] < 7)
        return ("low DEX");
    if (borg.stat_cur[STAT_CON] < 7)
        return ("low CON");

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* class specific requirement */
        switch (borg.trait[BI_CLASS]) {
        case CLASS_WARRIOR:
        case CLASS_BLACKGUARD:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 5)
                && borg.trait[BI_MAXCLEVEL] <= 38)
                return ("dlevel + 5 >= clevel");
            break;
        case CLASS_ROGUE:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 10)
                && borg.trait[BI_MAXCLEVEL] <= 43)
                return ("dlevel + 10 >= clevel");
            break;
        case CLASS_PRIEST:
        case CLASS_DRUID:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 13)
                && borg.trait[BI_MAXCLEVEL] <= 46)
                return ("dlevel + 13 >= clevel");
            break;
        case CLASS_PALADIN:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 7)
                && borg.trait[BI_MAXCLEVEL] <= 40)
                return ("dlevel + 7 >= clevel");
            break;
        case CLASS_RANGER:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 8)
                && borg.trait[BI_MAXCLEVEL] <= 41
                && borg.trait[BI_MAXCLEVEL] > 28)
                return ("dlevel + 8 >= clevel");
            break;
        case CLASS_MAGE:
        case CLASS_NECROMANCER:
            if (borg.trait[BI_MAXCLEVEL] < (depth + 8)
                && borg.trait[BI_MAXCLEVEL] <= 38)
                return ("dlevel + 8 >= clevel");
            if (((borg.trait[BI_MAXCLEVEL] - 38) * 2 + 30) < depth
                && borg.trait[BI_MAXCLEVEL] <= 44
                && borg.trait[BI_MAXCLEVEL] > 38)
                return ("(clevel-38)*2+30 < dlevel");
            break;
        }
    }

    /* Ready for level 25 */
    if (depth <= 25)
        return ((char *)NULL);

    /*** Essential Items for Level 25 to 39 ***/

    /* All Basic resistance*/
    if (!borg.trait[BI_SRCOLD])
        return ("resist cold");
    if (!borg.trait[BI_SRELEC])
        return ("resist elec");
    if (!borg.trait[BI_SRACID])
        return ("resist acid");

    /* Escape and Teleport */
    if (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] < 6)
        return ("6 tell + telep staffs");

    /* Cure Critical Wounds */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && (borg.trait[BI_ACCW] + borg.trait[BI_ACSW]) < 10)
        return ("10 ccw + csw");

    /* Ready for level 33 */
    if (depth <= 33)
        return ((char *)NULL);

    /* Minimal level */
    if (borg.trait[BI_MAXCLEVEL] < 40 && !borg_cfg[BORG_PLAYS_RISKY])
        return ("level 40");

    /* Usually ready for level 20 to 39 */
    if (depth <= 39)
        return ((char *)NULL);

    /*** Essential Items for Level 40 to 45 ***/

    /* Resist */
    if (!borg.trait[BI_SRPOIS])
        return ("resist pois");
    if (!borg.trait[BI_SRCONF])
        return ("resist conf");

    if (borg.stat_cur[STAT_STR] < 16)
        return ("STR < 16");

    if (spell_stat != -1) {
        if (borg.stat_cur[spell_stat] < 16)
            return ("spell stat < 16");
    }
    if (borg.stat_cur[STAT_DEX] < 16)
        return ("dex < 16");
    if (borg.stat_cur[STAT_CON] < 16)
        return ("con < 16");

    /* Ok to continue */
    if (depth <= 45)
        return ((char *)NULL);

    /*** Essential Items for Level 46 to 55 ***/

    /*  Must have +5 speed after level 46 */
    if (borg.trait[BI_SPEED] < 115)
        return ("+5 speed");

    /* Potions of heal */
    if (borg.trait[BI_AHEAL] < 1 && (borg.trait[BI_AEZHEAL] < 1))
        return ("1 heal");

    if (!borg_cfg[BORG_PLAYS_RISKY]) {
        /* Minimal hitpoints */
        if (borg.trait[BI_MAXHP] < 500)
            return ("HP 500");
    }

    /* High stats XXX XXX XXX */
    if (borg.stat_cur[STAT_STR] < 18 + 40)
        return ("str < 18(40)");

    if (spell_stat != -1) {
        if (borg.stat_cur[spell_stat] < 18 + 100)
            return ("spell stat needs to be max");
    }
    if (borg.stat_cur[STAT_DEX] < 18 + 60)
        return ("dex < 18 (60)");
    if (borg.stat_cur[STAT_CON] < 18 + 60)
        return ("con < 18 (60)");

    /* Hold Life */
    if (!borg.trait[BI_SHLIFE] && (borg.trait[BI_MAXCLEVEL] < 50))
        return ("hold life");

    /* Usually ready for level 46 to 55 */
    if (depth <= 55)
        return ((char *)NULL);

    /*** Essential Items for Level 55 to 59 ***/

    /* Potions of heal */
    if (borg.trait[BI_AHEAL] < 2 && borg.trait[BI_AEZHEAL] < 1)
        return ("2 heal + *heal*");

    /* Resists */
    if (!borg.trait[BI_SRBLIND])
        return ("resist blind");

    /* Must have resist nether */
    /*    if (!borg_settings[BORG_PLAYS_RISKY] && !borg.trait[BI_SRNTHR]) return
     * ("resist nether"); */

    /* Telepathy, better have it by now */
    if (!borg.trait[BI_ESP])
        return ("ESP");

    /* Usually ready for level 55 to 59 */
    if (depth <= 59)
        return ((char *)NULL);

    /*** Essential Items for Level 61 to 80 ***/

    /* Must have +10 speed */
    if (borg.trait[BI_SPEED] < 120)
        return ("+10 speed");

    /* Resists */
    if (!borg.trait[BI_SRKAOS])
        return ("resist chaos");
    if (!borg.trait[BI_SRDIS])
        return ("resist disenchant");

    /* Usually ready for level 61 to 80 */
    if (depth <= 80)
        return ((char *)NULL);

    /*** Essential Items for Level 81-85 ***/
    /* Minimal Speed */
    if (borg.trait[BI_SPEED] < 130)
        return ("+20 Speed");

    /* Usually ready for level 81 to 85 */
    if (depth <= 85)
        return ((char *)NULL);

    /*** Essential Items for Level 86-99 ***/

    /* Usually ready for level 86 to 99 */
    if (depth <= 99)
        return ((char *)NULL);

    /*** Essential Items for Level 100 ***/

    /* must have lots of restore mana to go after MORGOTH */
    if (!borg.trait[BI_KING]) {
        if ((borg.trait[BI_MAXSP] > 100)
            && (borg.has[kv_potion_restore_mana] < 15))
            return ("10 restore mana");

        /* must have lots of heal */
        if (borg.has[kv_potion_healing] < 5)
            return ("5 Heal");

        /* must have lots of ez-heal */
        if (borg.trait[BI_AEZHEAL] < 15)
            return ("15 *heal*");

        /* must have lots of speed */
        if (borg.trait[BI_ASPEED] < 10)
            return ("10 speed potions");
    }

    /* Its good to be the king */
    if (depth <= 127)
        return ((char *)NULL);

    /* all bases covered */
    return ((char *)NULL);
}

/*
 * Determine if the Borg is "prepared" for the given level
 *
 * This routine does not help him decide how to get ready for the
 * given level, so it must work closely with "borg_power()".
 *
 * Note that we ignore any "town fear", and we allow fear of one
 * level up to and including the relevant depth.
 *
 * This now returns a string with the reason you are not prepared.
 *
 */
const char *borg_prepared(int depth)
{
    const char *reason;

    /* Town and First level */
    if (depth == 1)
        return ((char *)NULL);

    if (borg_cfg[BORG_USES_DYNAMIC_CALCS]) {

        /* use the base restock so special checks can be done */
        if ((reason = borg_restock(depth)))
            return reason;

        if ((reason = borg_prepared_dynamic(depth)))
            return reason;

    } else {
        /* Not prepared if I need to restock */
        if ((reason = borg_restock(depth)))
            return (reason);

        /*** Require his Clevel to be greater than or equal to Depth */
        if (borg.trait[BI_MAXCLEVEL] < depth && borg.trait[BI_MAXCLEVEL] < 50)
            return ("Clevel < depth");

        /* Must meet minimal requirements */
        if (depth <= 99) {
            if ((reason = borg_prepared_aux(depth)))
                return (reason);
        }

        /* Not if No_Deeper is set */
        if (depth >= borg_cfg[BORG_NO_DEEPER]) {
            strnfmt(borg_prepared_buffer, MAX_REASON, "No deeper %d.",
                borg_cfg[BORG_NO_DEEPER]);
            return (borg_prepared_buffer);
        }
    }

    /* Once Morgoth is dead */
    if (borg.trait[BI_KING]) {
        return ((char *)NULL);
    }

    /* Always okay from town */
    if (!borg.trait[BI_CDEPTH])
        return (reason);

    /* Scum on depth 80-81 for some *heal* potions */
    if (depth >= 82 && (num_ezheal < 10 && borg.trait[BI_AEZHEAL] < 10)) {
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, false);

        strnfmt(borg_prepared_buffer, MAX_REASON,
            "Scumming *Heal* potions (%d to go).", 10 - num_ezheal);
        return (borg_prepared_buffer);
    }

    /* Scum on depth 80-81 for lots of *Heal* potions preparatory for Endgame */
    if (depth >= 82 && borg.trait[BI_MAXDEPTH] >= 97) {
        /* Must know exact number of Potions  in home */
        borg_notice_home(NULL, false);

        /* Scum for 30*/
        if (num_ezheal_true + borg.trait[BI_AEZHEAL] < 30) {
            strnfmt(borg_prepared_buffer, MAX_REASON,
                "Scumming *Heal* potions (%d to go).",
                30 - (num_ezheal_true + borg.trait[BI_AEZHEAL]));
            return (borg_prepared_buffer);
        }

        /* Return to town to get your stock from the home*/
        if (num_ezheal_true + borg.trait[BI_AEZHEAL] >= 30
            && /* Enough combined EZ_HEALS */
            num_ezheal_true >= 1
            && borg.trait[BI_MAXDEPTH]
                   >= 99) /* Still some sitting in the house */
        {
            strnfmt(borg_prepared_buffer, MAX_REASON,
                "Collect from house (%d potions).", num_ezheal_true);
            return (borg_prepared_buffer);
        }
    }

    /* Check to make sure the borg does not go below where 3 living */
    /* uniques are. */
    if (borg.trait[BI_MAXDEPTH] <= 98) {
        struct monster_race *r_ptr = &r_info[borg_living_unique_index];

        /* are too many uniques alive */
        if (borg_numb_live_unique < 3 || borg_cfg[BORG_PLAYS_RISKY]
            || borg.trait[BI_CLEVEL] == 50
            || borg_cfg[BORG_KILLS_UNIQUES] == false)
            return ((char *)NULL);

        /* Check for the dlevel of the unique */
        if (depth < borg_unique_depth)
            return ((char *)NULL);

        /* To avoid double calls to format() */
        /* Reset our description for not diving */
        strnfmt(borg_prepared_buffer, MAX_REASON, "Must kill %s.", r_ptr->name);
        return (borg_prepared_buffer);

    } else if (borg.trait[BI_MAXDEPTH] >= 98 && depth >= 98)
    /* check to make sure the borg does not go to level 100 */
    /* unless all the uniques are dead. */
    {
        struct monster_race *r_ptr;

        /* Access the living unique obtained from borg_update() */
        r_ptr = &r_info[borg_living_unique_index];

        /* -1 is unknown. */
        borg.ready_morgoth = -1;

        if (borg_numb_live_unique < 1
            || borg_living_unique_index == borg_morgoth_id) /* Morgoth */
        {
            if (depth >= 99)
                borg.ready_morgoth = 1;
            return ((char *)NULL);
        }

        /* Under special cases allow the borg to dive to 99 then quickly
         * get his butt to dlevel 98
         */
        if (borg.trait[BI_MAXDEPTH] == 99 && depth <= 98
            && (borg_spell_legal_fail(TELEPORT_LEVEL, 20)
                || /* Teleport Level */
                borg.trait[BI_ATELEPORTLVL] >= 1)) /* Teleport Level scroll */
        {
            return ((char *)NULL);
        }

        /* To avoid double calls to format() */
        strnfmt(
            borg_prepared_buffer, MAX_REASON, "%s still alive!", r_ptr->name);
        return (borg_prepared_buffer);
    }
    return (char *)NULL;
}

/*
 * Determine if the Borg is out of "crucial" supplies.
 *
 * Note that we ignore "restock" issues for the first several turns
 * on each level, to prevent repeated "level bouncing".
 */
const char *borg_restock(int depth)
{

    /* We are now looking at our preparedness */
    if (-1 == borg.ready_morgoth)
        borg.ready_morgoth = 0;

    /* Always ready for the town */
    if (!depth)
        return ((char *)NULL);

    /* Always Ready to leave town */
    if (borg.trait[BI_CDEPTH] == 0)
        return ((char *)NULL);

    /* Always spend time on a level unless 100*/
    if (borg_t - borg_began < 100 && borg.trait[BI_CDEPTH] != 100)
        return ((char *)NULL);

    if (borg_cfg[BORG_USES_DYNAMIC_CALCS]) 
        return borg_restock_dynamic(depth);

    /*** Level 1 ***/

    /* Must have some lite */
    if (borg.trait[BI_CURLITE] < 1)
        return ("restock light radius < 1");

    /* Must have "fuel" */
    if (borg.trait[BI_AFUEL] < 1 && !borg.trait[BI_LIGHT])
        return ("restock fuel");

    /* Assume happy at level 1 */
    if (depth <= 1)
        return ((char *)NULL);

    /*** Level 2 and 3 ***/

    /* Must have "fuel" */
    if (borg.trait[BI_AFUEL] < 2 && !borg.trait[BI_LIGHT])
        return ("restock fuel < 2");

    /* Must have "food" */
    if (borg.trait[BI_FOOD] < 3)
        return ("restock food < 3");

    /* Must have "recall" */
    /* if (borg.trait[BI_RECALL] < 2) return ("restock recall"); */

    /* Assume happy at level 3 */
    if (depth <= 3)
        return ((char *)NULL);

    /*** Level 3 to 5 ***/

    if (depth <= 5)
        return ((char *)NULL);

    /*** Level 6 to 9 ***/

    /* Must have "phase" */
    if (borg.trait[BI_APHASE] < 1)
        return ("restock phase door");

    /* Potions of Cure Wounds */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && borg.trait[BI_ACLW] + borg.trait[BI_ACSW] + borg.trait[BI_ACCW] < 1)
        return ("restock clw+csw+ccw");

    /* Assume happy at level 9 */
    if (depth <= 9)
        return ((char *)NULL);

    /*** Level 10 - 19  ***/

    /* Must have good light */
    if (borg.trait[BI_CURLITE] < 2)
        return "2 light radius";

    /* Must have "cure" */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && borg.trait[BI_ACLW] + borg.trait[BI_ACSW] + borg.trait[BI_ACCW] < 2)
        return ("restock clw + csw + ccw ");

    /* Must have "teleport" */
    if (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] < 2)
        return ("restock tele + tele staff < 2");

    /* Assume happy at level 19 */
    if (depth <= 19)
        return ((char *)NULL);

    /*** Level 20 - 35  ***/

    /* Must have "cure" */
    if ((borg.trait[BI_MAXCLEVEL] < 30)
        && borg.trait[BI_ACSW] + borg.trait[BI_ACCW] < 4)
        return ("restock csw + ccw < 4");

    /* Must have "teleport" or Staff */
    if (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] < 4)
        return ("restock 4 > teleport + teleport staff ");

    /* Assume happy at level 44 */
    if (depth <= 35)
        return ((char *)NULL);

    /*** Level 36 - 45  ***/

    /* Must have Scroll of Teleport (or good 2nd choice) */
    if (borg.trait[BI_ATELEPORT] + borg.trait[BI_ATELEPORTLVL] < 2)
        return ("restock teleport + teleport level");

    /* Assume happy at level 44 */
    if (depth <= 45)
        return ((char *)NULL);

    /*** Level 46 - 64  ***/

    /* Assume happy at level 65 */
    if (depth <= 64)
        return ((char *)NULL);

    /*** Level 65 - 99  ***/

    /* Must have "Heal" */
    if (borg.trait[BI_AHEAL] + borg.has[kv_rod_healing] + borg.trait[BI_AEZHEAL]
        < 1)
        return ("restock heal");

    /* Assume happy at level 99 */
    if (depth <= 99)
        return ((char *)NULL);

    /*** Level 100  ***/

    /* Must have "Heal" */
    /* If I just got to dlevel 100 and low on heals, get out now. */
    if (borg_t - borg_began < 10 && borg.trait[BI_AEZHEAL] < 15)
        return ("restock *heal*");

    /* Assume happy */
    return ((char *)NULL);
}

#endif
