/**
 * \file borg-log.c
 * \brief Log borg activity either to the screen or a log file
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

#include "borg-log.h"

#ifdef ALLOW_BORG

#include "../game-input.h"
#include "../game-world.h"
#include "../init.h"
#include "../obj-desc.h"
#include "../obj-util.h"
#include "../player-timed.h"
#include "../player-util.h"
#include "../store.h"
#include "../ui-menu.h"

#include "borg-cave.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-take.h"
#include "borg-home-notice.h"
#include "borg-magic.h"
#include "borg-prepared.h"
#include "borg-store.h"
#include "borg.h"

/*
 * write a death to borg-log.txt
 */
void borg_log_death(void)
{
    char      buf[1024];
    ang_file *borg_log_file;
    time_t    death_time;

    if (!borg_cfg[BORG_SAVE_DEATH])
        return;

    /* Build path to location of the definition file */
    path_build(buf, 1024, ANGBAND_DIR_USER, "borg-log.txt");

    /* Append to the file */
    borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Failure */
    if (!borg_log_file)
        return;

    /* Get time of death */
    (void)time(&death_time);

    /* Save the date */
    strftime(buf, 80, "%Y/%m/%d %H:%M\n", localtime(&death_time));

    file_put(borg_log_file, buf);

    file_putf(borg_log_file, "%s the %s %s, Level %d/%d\n", player->full_name,
        player->race->name, player->class->name, player->lev, player->max_lev);

    file_putf(borg_log_file, "Exp: %lu  Gold: %lu  Turn: %lu\n",
        (long)player->max_exp + (100 * player->max_depth), (long)player->au,
        (long)turn);
    file_putf(borg_log_file, "Killed on level: %d (max. %d) by %s\n",
        player->depth, player->max_depth, player->died_from);

    file_putf(borg_log_file, "Borg Compile Date: %s\n", borg_engine_date);

    file_put(borg_log_file, "----------\n\n");

    file_close(borg_log_file);
}

/*
 * write a death to borg.dat
 */
void borg_log_death_data(void)
{
    char      buf[1024];
    ang_file *borg_log_file;
    time_t    death_time;

    if (!borg_cfg[BORG_SAVE_DEATH])
        return;

    path_build(buf, 1024, ANGBAND_DIR_USER, "borg.dat");

    /* Append to the file */
    borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Failure */
    if (!borg_log_file)
        return;

    /* Get time of death */
    (void)time(&death_time);

    /* dump stuff for easy import to database */
    file_putf(borg_log_file, "%s, %s, %s, %d, %d, %s\n", borg_engine_date,
        player->race->name, player->class->name, player->lev, player->depth,
        player->died_from);

    file_close(borg_log_file);
}

/*
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
static char borg_index_to_label(int i)
{
    /* Indexes for "inven" are easy */
    if (i < INVEN_WIELD)
        return all_letters_nohjkl[i];

    /* Indexes for "equip" are offset */
    return all_letters_nohjkl[i - INVEN_WIELD];
}

/*
 * Write a file with the current dungeon info (Borg)
 * and his equipment, inventory and home (Player)
 * and his swap armor, weapon (Borg)
 * From Dennis Van Es,  With an addition of last messages from me (APW)
 * NOTE: this uses internal game data.  This is okay since we are just dumping 
 * the information rather than using it.
 */
void borg_write_map(bool ask)
{
    char      buf2[1024];
    char      buf[80];
    ang_file *borg_map_file = NULL;
    wchar_t  *line;
    char     *ch_line;

    borg_item *item;
    int        i, j;
    int        to, itemm;

    int16_t m_idx;

    char o_name[80];

    if (!borg_cfg[BORG_SAVE_DEATH])
        return;

    /* Process the player name */
    for (i = 0; player->full_name[i]; i++) {
        char c = player->full_name[i];

        /* No control characters */
        if (iscntrl(c)) {
            /* Illegal characters */
            quit_fmt("Illegal control char (0x%02X) in player name", c);
        }

        /* Convert all non-alphanumeric symbols */
        if (!isalpha(c) && !isdigit(c))
            c = '_';

        /* Build "file_name" */
        buf[i] = c;
    }

    /* Terminate */
    buf[i++] = '.';
    buf[i++] = 'm';
    buf[i++] = 'a';
    buf[i++] = 'p';
    buf[i++] = '\0';

    path_build(buf2, 1024, ANGBAND_DIR_USER, buf);

    /* XXX XXX XXX Get the name and open the map file */
    if (ask && get_string("Borg map File: ", buf2, 70)) {
        /* Open a new file */
        borg_map_file = file_open(buf2, MODE_WRITE, FTYPE_TEXT);

        /* Failure */
        if (!borg_map_file)
            msg("Cannot open that file.");
    } else if (!ask)
        borg_map_file = file_open(buf2, MODE_WRITE, FTYPE_TEXT);

    file_putf(borg_map_file, "%s the %s %s, Level %d/%d\n", player->full_name,
        player->race->name, player->class->name, player->lev, player->max_lev);

    file_putf(borg_map_file, "Exp: %lu  Gold: %lu  Turn: %lu\n",
        (long)(player->max_exp + (100 * player->max_depth)), (long)player->au,
        (long)turn);
    file_putf(borg_map_file, "Killed on level: %d (max. %d) by %s\n\n",
        player->depth, player->max_depth, player->died_from);
    file_putf(borg_map_file, "Borg Compile Date: %s\n", borg_engine_date);

    line    = mem_zalloc((DUNGEON_WID + 1) * sizeof(wchar_t));
    ch_line = mem_zalloc((DUNGEON_WID + 1) * sizeof(char));
    for (i = 0; i < DUNGEON_HGT; i++) {
        for (j = 0; j < DUNGEON_WID; j++) {
            wchar_t ch;

            borg_grid *ag = &borg_grids[i][j];
            if (square_monster(cave, loc(j, i)))
                m_idx = square_monster(cave, loc(j, i))->midx;
            else
                m_idx = 0;

            /* reset the ch each time through */
            ch = ' ';

            /* Known grids */
            if (ag->feat) {
                ch = f_info[ag->feat].d_char;
            }

            /* Known Items */
            if (ag->take) {
                borg_take *take = &borg_takes[ag->take];
                if (take->kind) {
                    struct object_kind *k_ptr = take->kind;
                    ch                        = k_ptr->d_char;
                }
            }

            /* UnKnown Monsters */
            if (m_idx) {
                ch = '&';
            }

            /* Known Monsters */
            if (ag->kill) {
                borg_kill           *kill  = &borg_kills[ag->kill];
                struct monster_race *r_ptr = &r_info[kill->r_idx];
                ch                         = r_ptr->d_char;
            }

            /* The Player */
            if ((i == borg.c.y) && (j == borg.c.x))
                ch = '@';

            line[j] = ch;
        }
        /* terminate the line */
        line[j++] = '\0';

        wcstombs(ch_line, line, wcslen(line) + 1);

        file_putf(borg_map_file, "%s\n", ch_line);
    }
    mem_free(line);
    line = NULL;
    mem_free(ch_line);
    ch_line = NULL;

    /* Known/Seen monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Note */
        file_putf(borg_map_file, "monster '%s' (%d) at (%d,%d) speed:%d \n",
            (r_info[kill->r_idx].name), kill->r_idx, kill->pos.y, kill->pos.x,
            kill->speed);
    }

    /*** Dump the last few messages ***/
    i = messages_num();
    if (i > 250)
        i = 250;
    file_putf(borg_map_file, "\n\n  [Last Messages]\n\n");
    while (i-- > 0) {
        const char *msg = message_str((int16_t)i);

        /* Eliminate some lines */
        if (prefix(msg, "# Matched") || prefix(msg, "# There is")
            || prefix(msg, "# Tracking") || prefix(msg, "# MISS_BY:")
            || prefix(msg, "# HIT_BY:") || prefix(msg, "> "))
            continue;

        file_putf(borg_map_file, "%s\n", msg);
    }

    /*** Player Equipment ***/
    file_putf(borg_map_file, "\n\n  [Character Equipment]\n\n");
    for (i = 0; i < player->body.count; i++) {
        struct object *obj = player->body.slots[i].obj;
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", borg_index_to_label(i), o_name);
    }
    file_putf(borg_map_file, "\n\n  [Character Quiver]\n\n");
    for (i = 0; i < z_info->quiver_size; i++) {
        struct object *obj = player->upkeep->quiver[i];
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", borg_index_to_label(i), o_name);
    }

    file_putf(borg_map_file, "\n\n");

    /* Dump the inventory */
    file_putf(borg_map_file, "  [Character Inventory]\n\n");
    for (i = 0; i < z_info->pack_size; i++) {
        item = &borg_items[i];

        file_putf(
            borg_map_file, "%c) %s\n", borg_index_to_label(i), item->desc);
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Home (page 1) */
    file_putf(borg_map_file, "  [Home Inventory (page 1)]\n\n");
    struct object **list
        = mem_zalloc(sizeof(struct object *) * z_info->store_inven_max);
    store_stock_list(&stores[BORG_HOME], list, z_info->store_inven_max);
    for (i = 0; i < z_info->store_inven_max / 2; i++) {
        object_desc(o_name, sizeof(o_name), list[i], ODESC_FULL, player);
        file_putf(
            borg_map_file, "%c) %s\n", all_letters_nohjkl[i % 12], o_name);
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Home (page 2) */
    file_putf(borg_map_file, "  [Home Inventory (page 2)]\n\n");
    for (i = z_info->store_inven_max / 2; i < z_info->store_inven_max; i++) {
        object_desc(o_name, sizeof(o_name), list[i], ODESC_FULL, player);
        file_putf(
            borg_map_file, "%c) %s\n", all_letters_nohjkl[i % 12], o_name);
    }
    file_putf(borg_map_file, "\n\n");
    mem_free(list);
    list = NULL;

    /* Write swap info */
    if (borg_cfg[BORG_USES_SWAPS]) {
        file_putf(borg_map_file, "  [Swap info]\n\n");
        if (weapon_swap) {
            item = &borg_items[weapon_swap - 1];
            file_putf(borg_map_file, "Swap Weapon:  %s\n", item->desc);
        } else {
            file_put(borg_map_file, "Swap Weapon:  NONE\n");
        }
        if (armour_swap) {
            item = &borg_items[armour_swap - 1];
            file_putf(borg_map_file, "Swap Armour:  %s\n", item->desc);
        } else {
            file_put(borg_map_file, "Swap Armour:  NONE\n");
        }
        file_putf(borg_map_file, "\n\n");
    }
    file_putf(borg_map_file, "   [Player State at Death] \n\n");

    /* Dump the player state */
    file_putf(borg_map_file, "Current speed: %d. \n", borg.trait[BI_SPEED]);

    if (player->timed[TMD_BLIND]) {
        file_putf(borg_map_file, "You cannot see.\n");
    }
    if (player->timed[TMD_CONFUSED]) {
        file_putf(borg_map_file, "You are confused.\n");
    }
    if (player->timed[TMD_AFRAID]) {
        file_putf(borg_map_file, "You are terrified.\n");
    }
    if (player->timed[TMD_CUT]) {
        file_putf(borg_map_file, "You are bleeding.\n");
    }
    if (player->timed[TMD_STUN]) {
        file_putf(borg_map_file, "You are stunned.\n");
    }
    if (player->timed[TMD_POISONED]) {
        file_putf(borg_map_file, "You are poisoned.\n");
    }
    if (player->timed[TMD_IMAGE]) {
        file_putf(borg_map_file, "You are hallucinating.\n");
    }
    if (player_of_has(player, OF_AGGRAVATE)) {
        file_putf(borg_map_file, "You aggravate monsters.\n");
    }
    if (player->timed[TMD_BLESSED]) {
        file_putf(borg_map_file, "You feel rightous.\n");
    }
    if (player->timed[TMD_HERO]) {
        file_putf(borg_map_file, "You feel heroic.\n");
    }
    if (player->timed[TMD_SHERO]) {
        file_putf(borg_map_file, "You are in a battle rage.\n");
    }
    if (player->timed[TMD_PROTEVIL]) {
        file_putf(borg_map_file, "You are protected from evil.\n");
    }
    if (player->timed[TMD_SHIELD]) {
        file_putf(borg_map_file, "You are protected by a mystic shield.\n");
    }
    if (player->timed[TMD_INVULN]) {
        file_putf(borg_map_file, "You are temporarily invulnerable.\n");
    }
    if (player->timed[TMD_CONFUSED]) {
        file_putf(borg_map_file, "Your hands are glowing dull red.\n");
    }
    if (player->word_recall) {
        file_putf(borg_map_file, "You will soon be recalled.  (%d turns)\n",
            player->word_recall);
    }
    if (player->timed[TMD_OPP_FIRE]) {
        file_putf(borg_map_file, "You resist fire exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_ACID]) {
        file_putf(borg_map_file, "You resist acid exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_ELEC]) {
        file_putf(borg_map_file, "You resist elec exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_COLD]) {
        file_putf(borg_map_file, "You resist cold exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_POIS]) {
        file_putf(borg_map_file, "You resist poison exceptionally well.\n");
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Time Variables */
    file_putf(borg_map_file, "Time on this panel; %d\n", borg.time_this_panel);
    file_putf(borg_map_file, "Time on this level; %d\n", borg_t - borg_began);
    file_putf(borg_map_file, "Time since left town; %d\n",
        borg_time_town + (borg_t - borg_began));
    file_putf(borg_map_file, "Food in town; %d\n", borg_food_onsale);
    file_putf(borg_map_file, "Fuel in town; %d\n", borg_fuel_onsale);
    file_putf(borg_map_file, "Borg_no_retreat; %d\n", borg.no_retreat);
    file_putf(borg_map_file, "Breeder_level; %d\n", breeder_level);
    file_putf(borg_map_file, "Unique_on_level; %d\n", unique_on_level);
    if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2))
        file_putf(borg_map_file, "It is daytime in town.\n");
    else
        file_putf(borg_map_file, "It is night-time in town.\n");
    file_putf(borg_map_file, "\n\n");

    file_putf(
        borg_map_file, "borg_uses_swaps; %d\n", borg_cfg[BORG_USES_SWAPS]);
    file_putf(borg_map_file, "borg_worships_damage; %d\n",
        borg_cfg[BORG_WORSHIPS_DAMAGE]);
    file_putf(borg_map_file, "borg_worships_speed; %d\n",
        borg_cfg[BORG_WORSHIPS_SPEED]);
    file_putf(
        borg_map_file, "borg_worships_hp; %d\n", borg_cfg[BORG_WORSHIPS_HP]);
    file_putf(borg_map_file, "borg_worships_mana; %d\n",
        borg_cfg[BORG_WORSHIPS_MANA]);
    file_putf(
        borg_map_file, "borg_worships_ac; %d\n", borg_cfg[BORG_WORSHIPS_AC]);
    file_putf(borg_map_file, "borg_worships_gold; %d\n",
        borg_cfg[BORG_WORSHIPS_GOLD]);
    file_putf(
        borg_map_file, "borg_plays_risky; %d\n", borg_cfg[BORG_PLAYS_RISKY]);
    file_putf(borg_map_file, "borg_slow_optimizehome; %d\n\n",
        borg_cfg[BORG_SLOW_OPTIMIZEHOME]);
    file_putf(borg_map_file, "prepping for big fight; %d\n\n", borg.trait[BI_PREP_BIG_FIGHT]);
    file_putf(borg_map_file, "\n\n");

    /* Dump the spells */
    if (borg_can_cast()) {
        file_putf(borg_map_file, "\n\n   [ Spells ] \n\n");
        file_putf(
            borg_map_file, "Name                           Legal Times cast\n");
        for (i = 0; i < player->class->magic.total_spells; i++) {
            borg_magic *as          = &borg_magics[i];
            int         failpercent = 0;

            if (as->level < 99) {
                const char *legal
                    = (borg_spell_legal(as->spell_enum) ? "Yes" : "No ");
                failpercent = (borg_spell_fail_rate(as->spell_enum));

                file_putf(borg_map_file, "%-30s   %s   %ld   fail:%d \n",
                    as->name, legal, (long)as->times, failpercent);
            }
            file_putf(borg_map_file, "\n");
        }
    }

    /* Dump the borg.trait[] information */
    itemm = 0;
    to    = BI_MAX;
    for (; itemm < to; itemm++) {
        file_putf(borg_map_file, "skill %d (%s) value= %d.\n", itemm,
            prefix_pref[itemm], borg.trait[itemm]);
    }

#if 0
    /* Allocate the "okay" array */
    C_MAKE(okay, z_info->a_max, bool);

    /*** Dump the Uniques and Artifact Lists ***/

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++) {
        artifact_type *a_ptr = &a_info[k];

        /* Default */
        okay[k] = false;

        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Skip "uncreated" artifacts */
        if (!a_ptr->cur_num) continue;

        /* Assume okay */
        okay[k] = true;
    }

    /* Check the dungeon */
    for (loc_y = 0; loc_y < DUNGEON_HGT; loc_y++) {
        for (loc_x = 0; loc_x < DUNGEON_WID; loc_x++) {
            int16_t this_o_idx, next_o_idx = 0;

            /* Scan all objects in the grid */
            for (this_o_idx = cave->o_idx[loc_y][loc_x]; this_o_idx; this_o_idx = next_o_idx) {
                object_type *o_ptr;

                /* Get the object */
                o_ptr = &o_list[this_o_idx];

                /* Get the next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Ignore non-artifacts */
                if (!artifact_p(o_ptr)) continue;

                /* Ignore known items */
                if (object_is_known(o_ptr)) continue;

                /* Note the artifact */
                okay[o_ptr->name1] = false;
            }
        }
    }

    /* Check the inventory and equipment */
    for (i = 0; i < INVEN_TOTAL; i++) {
        object_type *o_ptr = &player->inventory[i];

        /* Ignore non-objects */
        if (!o_ptr->k_idx) continue;

        /* Ignore non-artifacts */
        if (!artifact_p(o_ptr)) continue;

        /* Ignore known items */
        if (object_is_known(o_ptr)) continue;

        /* Note the artifact */
        okay[o_ptr->name1] = false;
    }

    file_putf(borg_map_file, "\n\n");


    /* Hack -- Build the artifact name */
    file_putf(borg_map_file, "   [Artifact Info] \n\n");

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++) {
        artifact_type *a_ptr = &a_info[k];

        /* List "dead" ones */
        if (!okay[k]) continue;

        /* Paranoia */
        my_strcpy(o_name, "Unknown Artifact", sizeof(o_name));

        /* Obtain the base object type */
        z = borg_lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Real object */
        if (z) {
            object_type *i_ptr;
            object_type object_type_body;

            /* Get local object */
            i_ptr = &object_type_body;

            /* Create fake object */
            object_prep(i_ptr, z);

            /* Make it an artifact */
            i_ptr->name1 = k;

            /* Describe the artifact */
            object_desc_spoil(o_name, sizeof(o_name), i_ptr, false, 0);
        }

        /* Hack -- Build the artifact name */
        file_putf(borg_map_file, "The %s\n", o_name);
    }

    /* Free the "okay" array */
    mem_free(okay);
    file_putf(borg_map_file, "\n\n");

    /* Display known uniques
     *
     * Note that the player ghosts are ignored.  XXX XXX XXX
     */
     /* Allocate the "who" array */
    C_MAKE(who, z_info->r_max, uint16_t);

    /* Collect matching monsters */
    for (i = 1, n = 0; i < z_info->r_max; i++) {
        monster_race *r_ptr = &r_info[i];
        monster_lore *l_ptr = &l_list[i];

        /* Require known monsters */
        if (!cheat_know && !l_ptr->r_sights) continue;

        /* Require unique monsters */
        if (!(rf_has(r_ptr->flags, RF_UNIQUE))) continue;

        /* Collect "appropriate" monsters */
        who[n++] = i;
    }

    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort the array by dungeon depth of monsters */
    borg_sort(who, &why, n);


    /* Hack -- Build the artifact name */
    file_putf(borg_map_file, "   [Unique Info] \n\n");

    /* Print the monsters */
    for (i = 0; i < n; i++) {
        monster_race *r_ptr = &r_info[who[i]];
        bool dead = (r_ptr->max_num == 0);

        /* Print a message */
        file_putf(borg_map_file, "%s is %s\n",
            (r_ptr->name),
            (dead ? "dead" : "alive"));
    }

    /* Free the "who" array */
    mem_free(who);

#endif /* extra dump stuff */

    file_close(borg_map_file);
}

/*
 * Output a long int in binary format.
 */
static void borg_prt_binary(uint32_t flags, int row, int col)
{
    int      i;
    uint32_t bitmask;

    /* Scan the flags */
    for (i = bitmask = 1; i <= 32; i++, bitmask *= 2) {
        /* Dump set bits */
        if (flags & bitmask) {
            Term_putch(col++, row, COLOUR_BLUE, '*');
        }

        /* Dump unset bits */
        else {
            Term_putch(col++, row, COLOUR_WHITE, '-');
        }
    }
}

/* this will display the values which the borg believes an
 * item has.  Select the item by inven # prior to hitting
 * the ^zo.
 */
void borg_display_item(struct object *item2, int n)
{
    int j = 0;

    bitflag f[OF_SIZE];

    borg_item *item;

    item = &borg_items[n];

    /* Extract the flags */
    object_flags(item2, f);

    /* Clear screen */
    Term_clear();

    /* Describe fully */
    prt(item->desc, 2, j);

    prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
            item->kind, item->level, item->tval, item->sval),
        4, j);

    prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
            item->iqty, item->weight, item->ac, item->dd, item->ds),
        5, j);

    prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
            item->pval, item->to_a, item->to_h, item->to_d),
        6, j);

    prt(format("name1 = %-4d  name2 = %-4d  value = %ld   cursed = %d   can "
               "uncurse = %d",
            item->art_idx, item->ego_idx, (long)item->value, item->cursed,
            item->uncursable),
        7, j);

    prt(format("ident = %d      timeout = %-d", item->ident, item->timeout), 8,
        j);

    /* maybe print the inscription */
    prt(format("Inscription: %s, chance: %d", borg_get_note(item),
            borg.trait[BI_DEV] - item->level),
        9, j);

    prt("+------------FLAGS1------------+", 10, j);
    prt("AFFECT..........SLAY.......BRAND", 11, j);
    prt("                ae      xxxpaefc", 12, j);
    prt("siwdcc  ssidsasmnvudotgddduoclio", 13, j);
    prt("tnieoh  trnipthgiinmrrnrrmniierl", 14, j);
    prt("rtsxna..lcfgdkttmldncltggndsdced", 15, j);
    if (item->ident)
        borg_prt_binary(f[0], 16, j);

    prt("+------------FLAGS2------------+", 17, j);
    prt("SUST........IMM.RESIST.........", 18, j);
    prt("            afecaefcpfldbc s n  ", 19, j);
    prt("siwdcc      cilocliooeialoshnecd", 20, j);
    prt("tnieoh      irelierliatrnnnrethi", 21, j);
    prt("rtsxna......decddcedsrekdfddxhss", 22, j);
    if (item->ident)
        borg_prt_binary(f[1], 23, j);

    prt("+------------FLAGS3------------+", 10, j + 32);
    prt("s   ts h     tadiiii   aiehs  hp", 11, j + 32);
    prt("lf  eefo     egrgggg  bcnaih  vr", 12, j + 32);
    prt("we  lerln   ilgannnn  ltssdo  ym", 13, j + 32);
    prt("da reiedo   merirrrr  eityew ccc", 14, j + 32);
    prt("itlepnelf   ppanaefc  svaktm uuu", 15, j + 32);
    prt("ghigavaiu   aoveclio  saanyo rrr", 16, j + 32);
    prt("seteticfe   craxierl  etropd sss", 17, j + 32);
    prt("trenhstel   tttpdced  detwes eee", 18, j + 32);
    if (item->ident)
        borg_prt_binary(f[2], 19, j + 32);
}

/* DVE's function for displaying the status of various info */
/* Display what the borg is thinking DvE*/
void borg_status(void)
{
    int j;

    /* Scan windows */
    for (j = 0; j < 8; j++) {
        term *old = Term;

        /* Unused */
        if (!angband_term[j])
            continue;

        /* Check for borg status term */
        if (window_flag[j] & (PW_BORG_2)) {
            uint8_t attr;

            /* Activate */
            Term_activate(angband_term[j]);

            /* Display what resists the borg (thinks he) has */
            Term_putstr(5, 0, -1, COLOUR_WHITE, "RESISTS");

            /* Basic four */
            attr = COLOUR_SLATE;
            if (borg.trait[BI_RACID])
                attr = COLOUR_BLUE;
            if (borg.temp.res_acid)
                attr = COLOUR_GREEN;
            if (borg.trait[BI_IACID])
                attr = COLOUR_WHITE;
            Term_putstr(1, 1, -1, attr, "Acid");

            attr = COLOUR_SLATE;
            if (borg.trait[BI_RELEC])
                attr = COLOUR_BLUE;
            if (borg.temp.res_elec)
                attr = COLOUR_GREEN;
            if (borg.trait[BI_IELEC])
                attr = COLOUR_WHITE;
            Term_putstr(1, 2, -1, attr, "Elec");

            attr = COLOUR_SLATE;
            if (borg.trait[BI_RFIRE])
                attr = COLOUR_BLUE;
            if (borg.temp.res_fire)
                attr = COLOUR_GREEN;
            if (borg.trait[BI_IFIRE])
                attr = COLOUR_WHITE;
            Term_putstr(1, 3, -1, attr, "Fire");

            attr = COLOUR_SLATE;
            if (borg.trait[BI_RCOLD])
                attr = COLOUR_BLUE;
            if (borg.temp.res_cold)
                attr = COLOUR_GREEN;
            if (borg.trait[BI_ICOLD])
                attr = COLOUR_WHITE;
            Term_putstr(1, 4, -1, attr, "Cold");

            /* High resists */
            attr = COLOUR_SLATE;
            if (borg.trait[BI_RPOIS])
                attr = COLOUR_BLUE;
            if (borg.temp.res_pois)
                attr = COLOUR_GREEN;
            Term_putstr(1, 5, -1, attr, "Pois");

            if (borg.trait[BI_RFEAR])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 6, -1, attr, "Fear");

            if (borg.trait[BI_RLITE])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 7, -1, attr, "Lite");

            if (borg.trait[BI_RDARK])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 8, -1, attr, "Dark");

            if (borg.trait[BI_RBLIND])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 1, -1, attr, "Blind");

            if (borg.trait[BI_RCONF])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 2, -1, attr, "Confu");

            if (borg.trait[BI_RSND])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 3, -1, attr, "Sound");

            if (borg.trait[BI_RSHRD])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 4, -1, attr, "Shard");

            if (borg.trait[BI_RNXUS])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 5, -1, attr, "Nexus");

            if (borg.trait[BI_RNTHR])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 6, -1, attr, "Nethr");

            if (borg.trait[BI_RKAOS])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 7, -1, attr, "Chaos");

            if (borg.trait[BI_RDIS])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(6, 8, -1, attr, "Disen");

            /* Other abilities */
            if (borg.trait[BI_SDIG])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 1, -1, attr, "S.Dig");

            if (borg.trait[BI_FEATH])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 2, -1, attr, "Feath");

            if (borg.trait[BI_LIGHT])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 3, -1, attr, "PLite");

            if (borg.trait[BI_REG])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 4, -1, attr, "Regen");

            if (borg.trait[BI_ESP])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 5, -1, attr, "Telep");

            if (borg.trait[BI_SINV])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 6, -1, attr, "Invis");

            if (borg.trait[BI_FRACT])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 7, -1, attr, "FrAct");

            if (borg.trait[BI_HLIFE])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(12, 8, -1, attr, "HLife");

            /* Display the slays */
            Term_putstr(5, 10, -1, COLOUR_WHITE, "Weapon Slays:");

            if (borg.trait[BI_WS_ANIMAL])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 11, -1, attr, "Animal");

            if (borg.trait[BI_WS_EVIL])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(8, 11, -1, attr, "Evil");

            if (borg.trait[BI_WS_UNDEAD])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(15, 11, -1, attr, "Undead");

            if (borg.trait[BI_WS_DEMON])
                attr = COLOUR_BLUE;
            if (borg.trait[BI_WK_DEMON])
                attr = COLOUR_GREEN;
            else
                attr = COLOUR_SLATE;
            Term_putstr(22, 11, -1, attr, "Demon");

            if (borg.trait[BI_WS_ORC])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 12, -1, attr, "Orc");

            if (borg.trait[BI_WS_TROLL])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(8, 12, -1, attr, "Troll");

            if (borg.trait[BI_WS_GIANT])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(15, 12, -1, attr, "Giant");

            if (borg.trait[BI_WS_DRAGON])
                attr = COLOUR_BLUE;
            if (borg.trait[BI_WK_DRAGON])
                attr = COLOUR_GREEN;
            else
                attr = COLOUR_SLATE;
            Term_putstr(22, 12, -1, attr, "Dragon");

            if (borg.trait[BI_WB_ACID])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(1, 13, -1, attr, "Acid");

            if (borg.trait[BI_WB_COLD])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(8, 13, -1, attr, "Cold");

            if (borg.trait[BI_WB_ELEC])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(15, 13, -1, attr, "Elec");

            if (borg.trait[BI_WB_FIRE])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(22, 13, -1, attr, "Fire");

            /* Display the Concerns */
            Term_putstr(36, 10, -1, COLOUR_WHITE, "Concerns:");

            if (borg.trait[BI_FIRST_CURSED])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(29, 11, -1, attr, "Cursed");

            if (borg.trait[BI_ISWEAK])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(36, 11, -1, attr, "Weak");

            if (borg.trait[BI_ISPOISONED])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(43, 11, -1, attr, "Poison");

            if (borg.trait[BI_ISCUT])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(29, 12, -1, attr, "Cut");

            if (borg.trait[BI_ISSTUN])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(36, 12, -1, attr, "Stun");

            if (borg.trait[BI_ISCONFUSED])
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(43, 12, -1, attr, "Confused");

            if (borg.goal.fleeing)
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(29, 13, -1, attr, "Goal Fleeing");

            if (borg.no_rest_prep > 0)
                attr = COLOUR_BLUE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(43, 13, -1, attr, "No Resting");

            /* Display the Time */
            Term_putstr(60, 10, -1, COLOUR_WHITE, "Time:");

            Term_putstr(54, 11, -1, COLOUR_SLATE, "This Level         ");
            Term_putstr(
                65, 11, -1, COLOUR_WHITE, format("%d", borg_t - borg_began));

            Term_putstr(54, 12, -1, COLOUR_SLATE, "Since Town         ");
            Term_putstr(65, 12, -1, COLOUR_WHITE,
                format("%d", borg_time_town + (borg_t - borg_began)));

            Term_putstr(54, 13, -1, COLOUR_SLATE, "This Panel         ");
            Term_putstr(
                65, 13, -1, COLOUR_WHITE, format("%d", borg.time_this_panel));

            /* Sustains */
            Term_putstr(19, 0, -1, COLOUR_WHITE, "Sustains");

            if (borg.trait[BI_SSTR])
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(21, 1, -1, attr, "STR");

            if (borg.trait[BI_SINT])
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(21, 2, -1, attr, "INT");

            if (borg.trait[BI_SWIS])
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(21, 3, -1, attr, "WIS");

            if (borg.trait[BI_SDEX])
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(21, 4, -1, attr, "DEX");

            if (borg.trait[BI_SCON])
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(21, 5, -1, attr, "CON");

            /* Temporary effects */
            Term_putstr(28, 0, -1, COLOUR_WHITE, "Temp Effects");

            if (borg.temp.prot_from_evil)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 1, -1, attr, "Prot. Evil");

            if (borg.temp.fastcast)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 2, -1, attr, "Fastcast");

            if (borg.temp.hero)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 3, -1, attr, "Heroism");

            if (borg.temp.berserk)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 4, -1, attr, "Berserk");

            if (borg.temp.shield)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 5, -1, attr, "Shielded");

            if (borg.temp.bless)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 6, -1, attr, "Blessed");

            if (borg.temp.fast)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 7, -1, attr, "Fast");

            if (borg.see_inv >= 1)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(28, 8, -1, attr, "See Inv");

            /* Temporary effects */
            Term_putstr(42, 0, -1, COLOUR_WHITE, "Level Information");

            if (vault_on_level)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(42, 1, -1, attr, "Vault on level");

            if (unique_on_level)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(42, 2, -1, attr, "Unique on level");
            if (unique_on_level)
                Term_putstr(58, 2, -1, attr,
                    format("(%s)", r_info[unique_on_level].name));
            else
                Term_putstr(58, 2, -1, attr,
                    "                                                ");

            if (scaryguy_on_level)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(42, 3, -1, attr, "Scary Guy on level");

            if (breeder_level)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(42, 4, -1, attr, "Breeder level (closing doors)");

            if (borg_kills_summoner != -1)
                attr = COLOUR_WHITE;
            else
                attr = COLOUR_SLATE;
            Term_putstr(42, 5, -1, attr, "Summoner very close (AS-Corridor)");

            /* level preparedness */
            attr = COLOUR_SLATE;
            Term_putstr(42, 6, -1, attr, "Reason for not diving:");
            attr = COLOUR_WHITE;
            Term_putstr(64, 6, -1, attr,
                format("%s                              ",
                    borg_prepared(borg.trait[BI_MAXDEPTH] + 1)));

            attr = COLOUR_SLATE;
            Term_putstr(42, 7, -1, attr,
                "Scumming: not active                          ");
            if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0) {
                attr = COLOUR_WHITE;
                Term_putstr(42, 7, -1, attr,
                    format("Scumming: $%d                  ",
                        borg_cfg[BORG_MONEY_SCUM_AMOUNT]));
            }
            attr = COLOUR_SLATE;
            Term_putstr(42, 8, -1, attr, "Maximal Depth:");
            attr = COLOUR_WHITE;
            Term_putstr(
                56, 8, -1, attr, format("%d    ", borg.trait[BI_MAXDEPTH]));

            /* Important endgame information */
            if (borg.trait[BI_MAXDEPTH] >= 50) /* 85 */
            {
                Term_putstr(5, 15, -1, COLOUR_WHITE, "Important Deep Events:");

                attr = COLOUR_SLATE;
                Term_putstr(1, 16, -1, attr, "Home *Heal*:        ");
                attr = COLOUR_WHITE;
                Term_putstr(13, 16, -1, attr, format("%d   ", num_ezheal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 17, -1, attr, "Home Heal:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 17, -1, attr, format("%d   ", num_heal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 18, -1, attr, "Home Life:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 18, -1, attr, format("%d   ", num_life));

                attr = COLOUR_SLATE;
                Term_putstr(1, 19, -1, attr, "Res_Mana:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 19, -1, attr, format("%d   ", num_mana));

                if (morgoth_on_level)
                    attr = COLOUR_BLUE;
                else
                    attr = COLOUR_SLATE;
                Term_putstr(1, 20, -1, attr,
                    format("Morgoth on Level.  Last seen:%d       ",
                        borg_t - borg_t_morgoth));

                if (borg_morgoth_position)
                    attr = COLOUR_BLUE;
                else
                    attr = COLOUR_SLATE;
                if (borg_needs_new_sea)
                    attr = COLOUR_WHITE;
                Term_putstr(1, 21, -1, attr, "Sea of Runes.");

                if (borg.ready_morgoth)
                    attr = COLOUR_BLUE;
                else
                    attr = COLOUR_SLATE;
                Term_putstr(1, 22, -1, attr, "Ready for Morgoth.");
            } else {
                Term_putstr(
                    5, 15, -1, COLOUR_WHITE, "                        ");

                attr = COLOUR_SLATE;
                Term_putstr(1, 16, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(10, 16, -1, attr, format("%d       ", num_ezheal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 17, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(10, 17, -1, attr, format("%d       ", num_life));

                attr = COLOUR_SLATE;
                Term_putstr(1, 18, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 18, -1, attr, format("%d       ", num_heal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 19, -1, attr, "                   ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 19, -1, attr, format("%d       ", num_mana));
            }

            /* Fresh */
            Term_fresh();

            /* Restore */
            Term_activate(old);
        }
    }
}
#endif
