/**
 * \file borg-commands.c
 * \brief borg commands accessed through ^z
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

#ifdef ALLOW_BORG

#include "../game-input.h"
#include "../game-world.h"
#include "../project.h"
#include "../target.h"
#include "../ui-input.h"
#include "../ui-map.h"
#include "../ui-menu.h"
#include "../ui-target.h"
#include "../ui-term.h"

#include "borg.h"
#include "borg-cave.h"
#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-danger.h"
#include "borg-flow.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-take.h"
#include "borg-home-notice.h"
#include "borg-home-power.h"
#include "borg-init.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-id.h"
#include "borg-item-wear.h"
#include "borg-log.h"
#include "borg-magic.h"
#include "borg-messages.h"
#include "borg-power.h"
#include "borg-projection.h"
#include "borg-prepared.h"
#include "borg-reincarnate.h"
#include "borg-store.h"
#include "borg-update.h"

struct borg_commands
{
    const char  selection;
    const char* description;
    void        (*function)(void);
};

/*
 * List of borg commands
 */
/* predefine all the functions used in the array */
static void borg_cmd_help(void);
static void borg_cmd_start(void);
static void borg_cmd_init_txt_file(void);
static void borg_cmd_step(void);
static void borg_cmd_update(void);
static void borg_cmd_flags(void);
static void borg_cmd_cheat(void);
static void borg_cmd_nasties(void);
static void borg_cmd_search_string(void);
static void borg_cmd_map(void);
static void borg_cmd_grid_target_feature(void);
static void borg_cmd_grid_target_info(void);
static void borg_cmd_grid_target_danger(void);
static void borg_cmd_steps(void);
static void borg_cmd_targeting(void);
static void borg_cmd_flow(void);
static void borg_cmd_fear(void);
static void borg_cmd_power(void);
static void borg_cmd_time(void);
static void borg_cmd_grid_line_of_sight(void);
static void borg_cmd_prepare(void);
static void borg_cmd_stats(void);
static void borg_cmd_swaps(void);
static void borg_cmd_spells(void);
static void borg_cmd_has(void);
static void borg_cmd_dump(void);
static void borg_cmd_object_desc(void);
static void borg_cmd_respawn(void);
static void borg_cmd_version(void);
extern void do_cmd_borg(void);
static struct borg_commands borg_commands[] = {
    { '?', "borg command help", borg_cmd_help },
    { '$', "reload borg.txt file", borg_cmd_init_txt_file },
    { 'z', "activate borg", borg_cmd_start },
    { 'x', "step the borg", borg_cmd_step },
    { 'm', "show map information", borg_cmd_map },
    { 'G', "target grid features", borg_cmd_grid_target_feature },
    { 'I', "target grid info", borg_cmd_grid_target_info },
    { 'D', "target grid danger", borg_cmd_grid_target_danger },
    { '@', "line of sight map", borg_cmd_grid_line_of_sight },
    { 'F', "map of fear levels", borg_cmd_fear },
    { 'f', "update config flags", borg_cmd_flags },
    { 'u', "refresh borg vars", borg_cmd_update },
    { 'c', "toggle cheat death", borg_cmd_cheat },
    { 's', "set search string", borg_cmd_search_string },
    { '^', "show the borg path", borg_cmd_flow },
    { 'y', "step history", borg_cmd_steps },
    { 't', "current targeting", borg_cmd_targeting },
    { 'h', "things the borg has", borg_cmd_has },
    { 'p', "borg power", borg_cmd_power },
    { 'P', "preparation depth", borg_cmd_prepare },
    { '!', "time tracking flags", borg_cmd_time },
    { 'C', "list count of 'nasties'", borg_cmd_nasties },
    { '0', "borg stats (str/int etc)", borg_cmd_stats },
    { 'w', "swap weapons/armor", borg_cmd_swaps },
    { 'S', "spells", borg_cmd_spells },
    { 'l', "creates snapshot log file", borg_cmd_dump },
    { 'o', "object details", borg_cmd_object_desc },
    { 'R', "respawn", borg_cmd_respawn },
    { 'v', "version", borg_cmd_version },
};


 /*
  * Get a grid square to use for command
  */
static bool borg_cmd_target(void)
{
    char cmd = 'n';

    /* refresh the screen in case we were in help */
    Term_load();
    Term_flush();

    /* Check for use of current target */
    if (target_okay()) {
        if (!get_com("Use current target? (y/n) ", &cmd)) {
            borg_note("command aborted");
            return false;
        }
    }

    if (cmd != 'y' && cmd != 'Y') {
        ui_event ke;
        /* Get a command (or Cancel) */
        /* note this forces an extra keypress to start looking around but */
        /* that is what allowed printing of a message to start. */
        /* otherwise I would have to write my own targeting and that */
        /* doesn't seem worth it. */
        if (!get_com_ex("Select grid location (t to target): ", &ke))
        {
            borg_note("targeting aborted");
            return false;
        }

        if (!target_set_interactive(TARGET_LOOK, borg.c.x, borg.c.y, true)) {
            borg_note("targeting aborted");
            return false;
        }
    }
    return true;
}

static void borg_cmd_help(void)
{
    int i, line;
    const int command_count = sizeof(borg_commands) / sizeof(borg_commands[0]);

    /* Save the screen */
    Term_save();

    /* Clear the screen */
    Term_clear();

    /* list the commands in two columns */
    for (i = 0, line = 2; i < command_count; i += 2, line++) {
        Term_putstr(2, line, -1, COLOUR_WHITE,
            format("Command '%c' %s", borg_commands[i].selection,
                borg_commands[i].description));
        if (i+1 < command_count) {
            Term_putstr(42, line, -1, COLOUR_WHITE,
                format("Command '%c' %s", borg_commands[i + 1].selection,
                    borg_commands[i + 1].description));
        }
    }

    do_cmd_borg();

    /* Refresh the screen */
    Term_load();

    /* Done */
    return;
}

/*
 * Setup the borg to start
 */
static void borg_cmd_start(void)
{
    /* make sure the important game options are set correctly */
    borg_reinit_options();

    borg_clear_best();

    /* Activate */
    borg_active = true;

    /* Reset cancel */
    borg_cancel = false;

    /* Step forever */
    borg_step = 0;

    if (player->opts.lazymove_delay != 0) {
        borg_note("# Turning off lazy movement controls");
        player->opts.lazymove_delay = 0;
    }

    /* Message */
    borg_note("# Installing keypress hook");

    /* If the clock overflowed, fix that  */
    if (borg_t > 9000)
        borg_t = 9000;

    /* Activate the key stealer */
    borg_update_entrypoint(true);
}

/*
 * Setup the borg to start
 */
static void borg_cmd_init_txt_file(void)
{
    borg_init_txt_file();
}

/*
 * Show on the map how dangerous certain areas are.
 */
static void borg_cmd_avoidances(void)
{
    int x, y, p;

    /* Scan map */
    for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
        for (x = 1; x <= AUTO_MAX_X - 1; x++) {
            uint8_t a = COLOUR_RED;

            /* Obtain danger */
            p = borg_danger(y, x, 1, true, false);

            /* Skip non-avoidances */
            if (p < avoidance / 10)
                continue;

            /* Use colors for less painful */
            if (p < avoidance / 2)
                a = COLOUR_ORANGE;
            if (p < avoidance / 4)
                a = COLOUR_YELLOW;
            if (p < avoidance / 6)
                a = COLOUR_GREEN;
            if (p < avoidance / 8)
                a = COLOUR_BLUE;

            /* Display */
            print_rel('*', a, y, x);
        }
    }

    /* Get keypress */
    msg("(%d,%d of %d,%d) Avoidance value %d.", borg.c.y, borg.c.x,
        Term->offset_y / borg_panel_hgt(),
        Term->offset_x / borg_panel_wid(), avoidance);
    event_signal(EVENT_MESSAGE_FLUSH);
}

/*
 * take one or more steps
 */
static void borg_cmd_step(void)
{
    /* make sure the important game options are set correctly */
    borg_reinit_options();

    borg_clear_best();

    /* Activate */
    borg_active = true;

    /* Reset cancel */
    borg_cancel = false;

    /* Step N times */
    borg_step = get_quantity("Step how many times? ", 1000);
    if (borg_step < 1)
        borg_step = 1;

    borg_notice(true);

    /* Message */
    borg_note("# Installing keypress hook");
    borg_note(format("# Stepping Borg %d times", borg_step));

    /* If the clock overflowed, fix that  */
    if (borg_t > 9000)
        borg_t = 9000;

    /* Activate the key stealer */
    borg_update_entrypoint(true);
}

/*
 * Update borg internal variables
 */
static void borg_cmd_update(void)
{
    /* make sure the important game options are set correctly */
    borg_reinit_options();

    /* Activate */
    borg_active = true;

    /* Immediate cancel */
    borg_cancel = true;

    /* Step forever */
    borg_step = 0;

    borg_notice(true);

    /* Message */
    borg_note("# Installing keypress hook");

    /* Activate the key stealer */
    borg_update_entrypoint(true);
}

/**
 * Display an entry on the config menu
 */
static void get_cfg_display(struct menu* menu, int oid, bool cursor, int row,
    int col, int width)
{
    char value[40];
    int label_attr = cursor ? COLOUR_L_BLUE : COLOUR_WHITE;

    /* Clear the line */
    Term_erase(MAX(col - 1, 0), row, width);
    c_put_str(label_attr, borg_settings[oid].setting_string, row, col);

    if (borg_settings[oid].setting_type == 'b') {
        if (borg_cfg[oid])
            strcpy(value, "TRUE");
        else
            strcpy(value, "FALSE");
    }
    else { /* setting type i (integer) */
        snprintf(value, 30, "%d", borg_cfg[oid]);
    }
    c_put_str(label_attr, value, row, col + 30);
}

/**
 * Request a "quantity" from the user with default
 */
static int borg_cfg_get_quantity(int config)
{
    int amt = borg_cfg[config];
    char buf[80];

    /* Build the default */
    strnfmt(buf, sizeof(buf), "%d", amt);

    /* Ask for a quantity */
    if (!get_string(format("Enter new temp value for %s: ",
        borg_settings[config].setting_string), buf, 7)) return (0);

    /* Extract a number */
    amt = atoi(buf);

    /* Enforce the minimum */
    if (amt < -1) amt = -1;

    /* Return the result */
    return (amt);
}

/*
 * Allow the flags to change
 */
static void borg_cmd_flags(void)
{
    static menu_iter borg_cfg_menu_iter =
    {
        NULL,
        NULL,
        get_cfg_display,
        NULL,
        NULL
    };

    region area = { 21, 5, 1, 10 };

    struct cmd_info* choice = NULL;
	struct menu* m;
    ui_event result;

    msg("Pick a configuration setting to change (runtime only)");

	m = menu_new(MN_SKIN_SCROLL, &borg_cfg_menu_iter);

    menu_setpriv(m, BORG_MAX_SETTINGS, &borg_cfg_menu_iter);

    menu_setpriv(m, BORG_MAX_SETTINGS, &choice);
    menu_layout(m, &area);

    /* Set up the screen */
    screen_save();
    window_make(19, 4, 58, 15);

    result = menu_select(m, 0, true);

    screen_load();

    if (result.type == EVT_ESCAPE)
        return;

    if (result.type == EVT_SELECT) {
        if (m->cursor < 0 || m->cursor >= BORG_MAX_SETTINGS) {
            borg_note("oops");
            return;
        }
        if (borg_settings[m->cursor].setting_type == 'b')
        {
            borg_cfg[m->cursor] = !borg_cfg[m->cursor];

            msg(format("new temp config for %s is %s",
                borg_settings[m->cursor].setting_string,
                borg_cfg[m->cursor] ? "TRUE" : "FALSE"));
        }
        else {
            int new_value = borg_cfg_get_quantity(m->cursor);
            borg_cfg[m->cursor] = new_value;
            msg(format("new temp config for %s is %d",
                borg_settings[m->cursor].setting_string,
                borg_cfg[m->cursor]));
        }
    }
}

/*
 * change cheat death
 */
static void borg_cmd_cheat(void)
{
    borg_cheat_death = !borg_cheat_death;
    msg("Borg -- borg_cheat_death is now %d.", borg_cheat_death);
}

/*
 * list the number of creatures that are banishable
 */
static void borg_cmd_nasties(void)
{
    int i;

    /* Log Header */
    borg_note("Borg Nasties Count");

    /* Find the numerous nasty in order of nastiness */
    for (i = 0; i < borg_nasties_num; i++) {
        borg_note(format("Nasty: [%c] Count: %d, limited: %d",
            borg_nasties[i], borg_nasties_count[i], borg_nasties_limit[i]));
    }

    /* Done */
    msg("Borg Nasty Dump Complete.  Examine Log.");
}

/*
 * change search string for stopping borg
 */
static void borg_cmd_search_string(void)
{
    /* Get the new search string (or cancel the matching) */
    if (!get_string("Borg Match String: ", borg_match, 70)) {
        /* Cancel it */
        my_strcpy(borg_match, "", sizeof(borg_match));

        /* Message */
        msg("Borg Match String de-activated.");
    }
}

/**
 * Display an entry on the features menu
 */
static void get_feature_display(struct menu* menu, int oid, bool cursor, int row,
    int col, int width)
{
    int label_attr = cursor ? COLOUR_L_BLUE : COLOUR_WHITE;

    /* Clear the line */
    Term_erase(MAX(col - 1, 0), row, width);

    if (oid < FEAT_MAX)
        c_put_str(label_attr, f_info[oid].name, row, col);
    else {
        int i = oid - FEAT_MAX - 2;
        if (i >= 0) {
            switch (i) {
            case 0:
                c_put_str(label_attr, "MARK", row, col);
                break;
            case 1:
                c_put_str(label_attr, "GLOW", row, col);
                break;
            case 2:
                c_put_str(label_attr, "DARK", row, col);
                break;
            case 3:
                c_put_str(label_attr, "OKAY", row, col);
                break;
            case 4:
                c_put_str(label_attr, "LIGHT", row, col);
                break;
            case 5:
                c_put_str(label_attr, "VIEW", row, col);
                break;
            case 6:
                c_put_str(label_attr, "TEMP", row, col);
                break;
            case 7:
                c_put_str(label_attr, "XTRA", row, col);
                break;
            case 8:
                c_put_str(label_attr, "IGNORE_MAP", row, col);
                break;
            }
        } else {
            if (oid == FEAT_MAX)
                c_put_str(label_attr, "Traps", row, col);
            if (oid == FEAT_MAX + 1)
                c_put_str(label_attr, "Glyphs", row, col);
        }
    }
}


/*
 * show map grid features
 */
static void borg_cmd_grid_feature(void)
{

    static menu_iter borg_feature_menu_iter =
    {
        NULL,
        NULL,
        get_feature_display,
        NULL,
        NULL
    };

    region area = { 21, 5, 1, 10 };

    struct menu* m;
    ui_event result;
    int x, y;

    m = menu_new(MN_SKIN_SCROLL, &borg_feature_menu_iter);

    menu_setpriv(m, FEAT_MAX + 9 + 2, &borg_feature_menu_iter);

    menu_layout(m, &area);

    /* Set up the screen */
    screen_save();
    window_make(19, 4, 58, 15);

    result = menu_select(m, 0, true);

    screen_load();

    if (result.type == EVT_ESCAPE)
        return;

    /* Scan map */
    for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
        for (x = 1; x <= AUTO_MAX_X - 1; x++) {
            uint8_t a = COLOUR_RED;

            borg_grid* ag = &borg_grids[y][x];

            /* show only those grids */
            if (m->cursor < FEAT_MAX)
            {
                if (!(ag->feat == m->cursor))
                    continue;
            } else {
                int i = m->cursor - FEAT_MAX - 2;
                if (i >= 0)
                {
                    switch (i) {
                    case 0:
                        if (!(ag->info & BORG_MARK))
                            continue;
                        break;
                    case 1:
                        if (!(ag->info & BORG_GLOW))
                            continue;
                        break;
                    case 2:
                        if (!(ag->info & BORG_DARK))
                            continue;
                        break;
                    case 3:
                        if (!(ag->info & BORG_OKAY))
                            continue;
                        break;
                    case 4:
                        if (!(ag->info & BORG_LIGHT))
                            continue;
                        break;
                    case 5:
                        if (!(ag->info & BORG_VIEW))
                            continue;
                        break;
                    case 6:
                        if (!(ag->info & BORG_TEMP))
                            continue;
                        break;
                    case 7:
                        if (!(ag->info & BORG_XTRA))
                            continue;
                        break;
                    case 8:
                        if (!(ag->info & BORG_IGNORE_MAP))
                            continue;
                        break;
                    }
                }
                else {
                    if (m->cursor == FEAT_MAX)  /* trap */
                        if (!ag->trap)
                            continue;
                    if (m->cursor == FEAT_MAX + 1)  /* glyph */
                        if (!ag->glyph)
                            continue;
                }
            }

            /* Color */
            if (borg_cave_floor_bold(y, x))
                a = COLOUR_YELLOW;

            /* Display */
            print_rel('*', a, y, x);
        }
    }


    /* Get keypress */
    msg("Press any key.");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();
}

/*
 * Display Feature of a targetted grid
 */
static void borg_cmd_grid_target_feature(void)
{
    int y = 1;
    int x = 1;

    uint16_t mask;

    if (!borg_cmd_target())
        return;

    struct loc l;

    target_get(&l);
    y = l.y;
    x = l.x;

    mask = borg_grids[y][x].feat;

    uint8_t feat = square(cave, borg.c)->feat;

    borg_note(format("Borg's Feat for grid (%d, %d) is %d, game Feat is %d",
        y, x, mask, feat));
    prt_map();
}

/*
 * Display Feature of a targetted grid
 */
static void borg_cmd_grid_target_info(void)
{
    int        i;
    int        y = 1;
    int        x = 1;
    struct loc l;

    if (!borg_cmd_target())
        return;

    target_get(&l);
    y = l.y;
    x = l.x;

    if (borg_grids[y][x].info & BORG_MARK)
        msg("Borg Info for grid (%d, %d) is MARK", y, x);
    if (borg_grids[y][x].info & BORG_GLOW)
        msg("Borg Info for grid (%d, %d) is GLOW", y, x);
    if (borg_grids[y][x].info & BORG_DARK)
        msg("Borg Info for grid (%d, %d) is DARK", y, x);
    if (borg_grids[y][x].info & BORG_OKAY)
        msg("Borg Info for grid (%d, %d) is OKAY", y, x);
    if (borg_grids[y][x].info & BORG_LIGHT)
        msg("Borg Info for grid (%d, %d) is LITE", y, x);
    if (borg_grids[y][x].info & BORG_VIEW)
        msg("Borg Info for grid (%d, %d) is VIEW", y, x);
    if (borg_grids[y][x].info & BORG_TEMP)
        msg("Borg Info for grid (%d, %d) is TEMP", y, x);
    if (borg_grids[y][x].info & BORG_XTRA)
        msg("Borg Info for grid (%d, %d) is XTRA", y, x);

    for (i = 0; i < SQUARE_MAX; i++)
        if (sqinfo_has(square(cave, l)->info, i))
            msg(format("Sys Info for grid (%d, %d) is %d", y, x, i));
    prt_map();
}

/*
 * Display borgs assessed danger of a targetted grid
 */
static void borg_cmd_grid_target_danger(void)
{
    int        n;
    struct loc l;

    if (!borg_cmd_target())
        return;

    target_get(&l);

    /* Turns */
    n = get_quantity("Time on square? ", 1000);

    /* Danger of grid */
    msg("Danger(%d,%d,%d) is %d", l.x, l.y, n,
        borg_danger(l.y, l.x, n, true, false));
    prt_map();
}

/*
 * List previous steps
 */
static void borg_cmd_steps(void)
{
    int i;

    /* Scan map */
    uint8_t a = COLOUR_RED;
    /* Check for an existing step */
    for (i = 0; i < track_step.num; i++) {
        /* Display */
        print_rel('*', a, track_step.y[track_step.num - i],
            track_step.x[track_step.num - i]);
        msg("(-%d) Steps noted %d,%d", i, track_step.y[track_step.num - i],
            track_step.x[track_step.num - i]);
        event_signal(EVENT_MESSAGE_FLUSH);
        print_rel('*', COLOUR_ORANGE, track_step.y[track_step.num - i],
            track_step.x[track_step.num - i]);
    }
    /* Redraw map */
    prt_map();

}

/*
 * Highlight where the borg thinks there are monsters
 */
static void borg_cmd_grid_monsters(void)
{
    int i, n = 0;

    /* Scan the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill* kill = &borg_kills[i];

        /* Still alive */
        if (kill->r_idx) {
            int x = kill->pos.x;
            int y = kill->pos.y;

            /* Display */
            print_rel('*', COLOUR_RED, y, x);

            /* Count */
            n++;
        }
    }

    /* Get keypress */
    msg("There are %d known monsters.", n);
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();
}

/*
 * Highlight where the borg thinks there are objects
 */
static void borg_cmd_grid_objects(void)
{
    int i, n = 0;

    /* Scan the objects */
    for (i = 1; i < borg_takes_nxt; i++) {
        borg_take* take = &borg_takes[i];

        /* Still alive */
        if (take->kind) {
            int x = take->x;
            int y = take->y;

            /* Display */
            print_rel('*', COLOUR_RED, y, x);

            /* Count */
            n++;
        }
    }

    /* Get keypress */
    msg("There are %d known objects.", n);
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();
}

/*
 * Highlight where the borg is currently targeting
 */
static void borg_cmd_targeting(void)
{
    int        x, y;
    int        n_x;
    int        n_y;
    struct loc l;
    char       ch;

    uint8_t svDelay = player->opts.delay_factor;
    player->opts.delay_factor = 200;

    /* Determine "path" */
    n_x = borg.c.x;
    n_y = borg.c.y;
    target_get(&l);
    x = l.x;
    y = l.y;

    /* Borg's pathway */
    while (1) {

        /* Display */
        print_rel('*', COLOUR_RED, n_y, n_x);

        if (n_x == x && n_y == y)
            break;

        /* Calculate the new location */
        borg_inc_motion(&n_y, &n_x, borg.c.y, borg.c.x, y, x);
    }

    msg("Borg's Targeting Path");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Determine "path" */
    n_x = borg.c.x;
    n_y = borg.c.y;
    x = l.x;
    y = l.y;

    /* Get a "Borg command", or abort */
    if (!get_com("Borg command: Show Arc (Y/y): ", &ch))
        return;

    msg("Actual Targeting Path");
    event_signal(EVENT_MESSAGE_FLUSH);

    if (ch != 'Y' && ch != 'y')
        /* Real LOS - beam*/
        project(source_player(), 0, loc(x, y), 1, PROJ_MISSILE, PROJECT_BEAM, 0,
            0, NULL);
    else
        /* Real LOS - arc */
        project(source_player(), 10, loc(x, y), 50, PROJ_MISSILE, PROJECT_ARC, 60,
            4, NULL);

    player->opts.delay_factor = svDelay;

    /* Redraw map */
    prt_map();
}

/*
 * Highlight where the borg is currently targeting
 */
static void borg_cmd_flow(void)
{
    int x, y;
    int o;
    int false_y, false_x;

    false_y = borg.c.y;
    false_x = borg.c.x;

    /* Continue */
    for (o = 0; o < 250; o++) {
        int b_n = 0;

        int i, b_i = -1;

        int c, b_c;

        /* Flow cost of current grid */
        b_c = borg_data_flow->data[borg.c.y][borg.c.x] * 10;

        /* Prevent loops */
        b_c = b_c - 5;

        /* Look around */
        for (i = 0; i < 8; i++) {
            /* Grid in that direction */
            x = false_x + ddx_ddd[i];
            y = false_y + ddy_ddd[i];

            /* Flow cost at that grid */
            c = borg_data_flow->data[y][x] * 10;

            /* Never backtrack */
            if (c > b_c)
                continue;

            /* Notice new best value */
            if (c < b_c)
                b_n = 0;

            /* Apply the randomizer to equivalent values */
            if ((++b_n >= 2) && (randint0(b_n) != 0))
                continue;

            /* Track it */
            b_i = i;
            b_c = c;
        }

        /* Try it */
        if (b_i >= 0) {
            /* Access the location */
            x = false_x + ddx_ddd[b_i];
            y = false_y + ddy_ddd[b_i];

            /* Display */
            print_rel('*', COLOUR_RED, y, x);

            /* Simulate motion */
            false_y = y;
            false_x = x;
        }
    }
    print_rel('*', COLOUR_YELLOW, borg_flow_y[0], borg_flow_x[0]);
    msg("Probable Flow Path");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();
}


/*
 * Show a heat map of the borgs fear levels
 */
static void borg_cmd_fear(void)
{
    int x, y, p;

    /* Scan map */
    for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
        for (x = 1; x <= AUTO_MAX_X - 1; x++) {
            uint8_t a = COLOUR_RED;

            /* Obtain danger */
            p = borg_fear_region[y / 11][x / 11];

            /* Skip non-fears */
            if (p < avoidance / 10)
                continue;

            /* Use colors = less painful */
            if (p < avoidance / 2)
                a = COLOUR_ORANGE;
            if (p < avoidance / 4)
                a = COLOUR_YELLOW;
            if (p < avoidance / 6)
                a = COLOUR_GREEN;
            if (p < avoidance / 8)
                a = COLOUR_BLUE;

            /* Display */
            print_rel('*', a, y, x);
        }
    }

    /* Get keypress */
    msg("(%d,%d of %d,%d) Regional Fear.", borg.c.y, borg.c.x,
        Term->offset_y / borg_panel_hgt(),
        Term->offset_x / borg_panel_wid());
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();

    /* Scan map */
    for (y = 1; y <= AUTO_MAX_Y; y++) {
        for (x = 1; x <= AUTO_MAX_X; x++) {
            uint8_t a = COLOUR_BLUE;

            /* Obtain danger */
            p = borg_fear_monsters[y][x];

            /* Skip non-fears */
            if (p <= 0)
                continue;

            /* Color Defines */
            if (p == 1)
                a = COLOUR_L_BLUE;

            /* Color Defines */
            if (p < avoidance / 20 && p > 1)
                a = COLOUR_BLUE;

            /* Color Defines */
            if (p < avoidance / 10 && p > avoidance / 20)
                a = COLOUR_GREEN;

            /* Color Defines */
            if (p < avoidance / 4 && p > avoidance / 10)
                a = COLOUR_YELLOW;

            /* Color Defines */
            if (p < avoidance / 2 && p > avoidance / 4)
                a = COLOUR_ORANGE;

            /* Color Defines */
            if (p > avoidance / 2)
                a = COLOUR_RED;

            /* Display */
            print_rel('*', a, y, x);
        }
    }

    /* Get keypress */
    msg("(%d,%d of %d,%d) Monster Fear.", borg.c.y, borg.c.x,
        Term->offset_y / borg_panel_hgt(),
        Term->offset_x / borg_panel_wid());
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Redraw map */
    prt_map();
}

/*
 * Display the time flags the borg keeps
 */
static void borg_cmd_time(void)
{
    int32_t time = borg_t - borg_began;
    msg("time: (%d) ", time);
    time = (borg_time_town + (borg_t - borg_began));
    msg("; from town (%d)", time);
    msg("; on this panel (%d)", borg.time_this_panel);
    msg("; need inviso (%d)", borg.need_see_invis);
}

/*
 * Display the LOS for the borg
 */
static void borg_cmd_grid_line_of_sight(void)
{
    int x, y;

    /* Scan map */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {
            uint8_t a = COLOUR_RED;

            /* Obtain danger */
            if (!borg_los(borg.c.y, borg.c.x, y, x))
                continue;

            /* Display */
            print_rel('*', a, y, x);
        }
    }

    /* Get keypress */
    msg("Borg has Projectable to these places.");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Scan map */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {
            uint8_t a = COLOUR_YELLOW;

            if (!square_in_bounds(cave, loc(x, y)))
                continue;

            /* Obtain danger */
            if (!borg_projectable_dark(borg.c.y, borg.c.x, y, x))
                continue;

            /* Display */
            print_rel('*', a, y, x);
        }
    }
    msg("Borg has Projectable Dark to these places.");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Scan map */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {
            uint8_t a = COLOUR_GREEN;

            /* Obtain danger */
            if (!borg_los(borg.c.y, borg.c.x, y, x))
                continue;

            /* Display */
            print_rel('*', a, y, x);
        }
    }
    msg("Borg has LOS to these places.");
    event_signal(EVENT_MESSAGE_FLUSH);
    /* Redraw map */
    prt_map();
}


/*
 * Show the borgs power
 */
static void borg_cmd_power(void)
{
    int32_t p;

    /* Cheat the "equip" screen */
    borg_cheat_equip();

    /* Cheat the "inven" screen */
    borg_cheat_inven();

    /* Cheat the "inven" screen */
    borg_cheat_store();

    /* Examine the screen */
    borg_notice(true);

    /* Examine the screen */
    borg_update();

    /* Examine the inventory */
    borg_object_fully_id();

    /* Evaluate */
    p = borg_power();

    borg_notice_home(NULL, false);

    /* Report it */
    msg("Current Borg Power %ld", p);
    msg("Current Home Power %ld", borg_power_home());
}

/*
 * Show the borgs prep level for depth
 */
static void borg_cmd_prepare(void)
{
    int i = 0;

    /* Extract some "hidden" variables */
    /* note: if we recode to do screen scraping again, this will fail */
    borg_cheat_equip();
    borg_cheat_inven();

    borg_notice(true);

    /* Examine the screen */
    borg_update();

    /* Examine the inventory */
    borg_object_fully_id();
    borg_notice_home(NULL, false);

    /* Dump prep codes */
    for (i = 1; i <= 101; i++) {
        /* Dump fear code*/
        if ((char*)NULL != borg_prepared(i))
            break;
    }
    msg("Max Level: %d  Prep'd For: %d  Reason: %s",
        borg.trait[BI_MAXDEPTH], i - 1, borg_prepared(i));
    if (!borg.trait[BI_CDEPTH]) {
        msg("Unable to check for big fight from town.");
    }
    else if (borg.ready_morgoth == 1) {
        msg("You are ready for the big fight!!");
    }
    else if (borg.ready_morgoth == 0) {
        msg("You are NOT ready for the big fight!!");
    }
    else if (borg.ready_morgoth == -1) {
        msg("No readiness check done.");
    }
}

/*
 * Show the borgs stats as known by him
 */
static void borg_cmd_stats(void)
{
    int i;
    for (i = 0; i < STAT_MAX; i++) {
        borg_note(format("stat # %s, is: cur %d, used %d",
            prefix_pref[BI_STR + i],
            borg.trait[BI_CSTR + i],
            borg.trait[BI_STR + i]));
    }
}

/*
 * Show the borgs current swap items
 */
static void borg_cmd_swaps(void)
{
    borg_item* item;

    /* Cheat the "equip" screen */
    borg_cheat_equip();
    /* Cheat the "inven" screen */
    borg_cheat_inven();

    /* Examine the inventory */
    borg_notice(true);
    borg_notice_home(NULL, false);
    /* Check the power */
    borg_power();

    /* Examine the screen */
    borg_update();

    /* note the swap items */
    if (weapon_swap) {
        item = &borg_items[weapon_swap - 1];
        msg("Swap Weapon:  %s, value= %d", item->desc, weapon_swap_value);
    }
    else {
        msg("Swap Weapon:  NONE");
    }

    if (armour_swap) {
        item = &borg_items[armour_swap - 1];
        msg("Swap Armour:  %s, value= %d", item->desc, armour_swap_value);
    }
    else {
        msg("Swap Armour:  NONE");
    }
}

/*
 * Show the borgs spells
 */
static void borg_cmd_spells(void)
{
    int ii = 2;
    char cmd;

    /* Save the screen */
    Term_save();

    /* Dump the spells */
    if (borg_can_cast()) {

        int i;
        int book = -1;

        for (i = 0; i < player->class->magic.total_spells; i++) {

            borg_magic* as = &borg_magics[i];
            int         failpercent = 0;

            if (as->level < 99) {
                if (book != as->book) {
                    if (book != -1)
                        get_com(format("Exam spell book %d.  Press any key for next book.", as->book),
                            &cmd);

                    ii = 2;
                    /* Clear the screen */
                    Term_clear();
                    Term_putstr(1, ii++, -1, COLOUR_WHITE, "[ Spells ].");
                    book = as->book;
                }

                const char* legal
                    = (borg_spell_legal(as->spell_enum) ? "legal"
                        : "Not Legal ");
                failpercent = (borg_spell_fail_rate(as->spell_enum));

                Term_putstr(1, ii++, -1, COLOUR_WHITE,
                    format("%s, %s, attempted %ld times, fail rate:%d",
                        as->name, legal, (long int)as->times, failpercent));
            }
        } /* dumps */
        get_com(format("Exam spell book %d.  Press any key for next book.", book), &cmd);
    } /* spells */

    /* Restore the screen */
    Term_load();

}

/*
 * Dump the borgs items/inventory/worn/artifacts/skills
 */
static void borg_cmd_has(void)
{
    int item, to;
    char cmd;

    /* Get a "Borg command", or abort */
    if (!get_com("Dynamic Borg Has What: "
        "((a)ny/(i)nv/(w)orn/a(r)tifact/(s)kill) ",
        &cmd))
        return;

    switch (cmd) {
    case 'a':
    case 'A':
        item = 0;
        to = z_info->k_max;
        break;
    case 'i':
    case 'I':
        item = 0;
        to = z_info->pack_size;
        break;
    case 'w':
    case 'W':
        item = INVEN_WIELD;
        to = QUIVER_END;
        break;
    case 'r':
    case 'R':
        item = 0;
        to = QUIVER_END;
        break;
    default:
        item = 0;
        to = BI_MAX;
        break;
    }
    /* Cheat the "equip" screen */
    borg_cheat_equip();

    /* Cheat the "inven" screen */
    borg_cheat_inven();

    /* Examine the screen */
    borg_notice(true);

    /* Examine the screen */
    borg_update();

    /* Examine the inventory */
    borg_object_fully_id();
    borg_notice_home(NULL, false);
    for (; item < to; item++) {
        switch (cmd) {
        case 'a':
        case 'A':
            if (borg.has[item]) {
                borg_note(format("Item-Kind:%03d name=%s value= %d.", item,
                    k_info[item].name, borg.has[item]));
            }
            break;
        case 'i':
        case 'I':
            if (borg_items[item].iqty) {
                borg_note(format("Item-Invn:%03d desc= %s qty %d.", item,
                    borg_items[item].desc, borg_items[item].iqty));
            }
            break;
        case 'w':
        case 'W':
            if (borg_items[item].iqty) {
                borg_note(format("Item-Worn:%03d desc= %s qty %d.", item,
                    borg_items[item].desc, borg_items[item].iqty));
            }
            break;
        case 'r':
        case 'R':
            if (borg_items[item].iqty && borg_items[item].art_idx)
                borg_note(format("Item-Arti:%03d name= %s.", item,
                    a_info[borg_items[item].art_idx].name));
            break;
        default: {
            borg_note(format("trait %d (%s) value= %d.", item,
                prefix_pref[item], borg.trait[item]));
            break;
        }
        }
    }

    /* note the completion. */
    msg("Borg_has[] dump complete.  Examine Log. ");
}


/*
 * create dump map
 */
static void borg_cmd_dump(void)
{
    /* Cheat the "inven" screen */
    borg_cheat_inven();

    borg_write_map(true);
}

/*
 * dump the version
 */
static void borg_cmd_version(void)
{
    msg("Borg Version: %s", borg_engine_date);
}

/*
 * object information
 */
static void borg_cmd_object_desc(void)
{
    int n = 0;

    struct object* item2;

    /* use this item */
    // XXX replace this with an item selector
    n = get_quantity("Which item?", z_info->pack_size);

    /* Cheat the "equip" screen */
    borg_cheat_equip();
    /* Cheat the "inven" screen */
    borg_cheat_inven();
    /* Examine the inventory */
    borg_notice(true);
    borg_notice_home(NULL, false);
    /* Check the power */
    borg_power();

    /* Examine the screen */
    borg_update();

    /* Save the screen */
    Term_save();

    /* get the item */
    item2 = player->upkeep->inven[n];

    /* Display the special screen */
    borg_display_item(item2, n);

    /* pause for study */
    msg("Borg believes: ");
    event_signal(EVENT_MESSAGE_FLUSH);

    /* Restore the screen */
    Term_load();
}

/*
 * respawn/reincarnate the borg
 */
static void borg_cmd_respawn(void)
{
    char cmd;

    /* Confirm it */
    get_com("Are you sure you want to Respawn this borg? (y or n)? ", &cmd);

    if (cmd == 'y' || cmd == 'Y') {
        reincarnate_borg();
    }
}

/*
 * show where the borg things glyphs are
 */
static void borg_cmd_grid_glyphs(void)
{
    int glyph_check;

    uint8_t a = COLOUR_RED;

    for (glyph_check = 0; glyph_check < track_glyph.num; glyph_check++) {
        /* Display */
        print_rel(
            '*', a, track_glyph.y[glyph_check], track_glyph.x[glyph_check]);
        msg("Borg has Glyph (%d)noted.", glyph_check);
        event_signal(EVENT_MESSAGE_FLUSH);
    }
}

/*
 * Map information
 */
static void borg_cmd_map(void)
{
    char option;
    if (!get_com("What to show ((a)voidances, (f)eatures, (g)lyphs, (m)onsters, (o)bjects): ", &option))
        return;

    switch (option)
    {
    case 'a':
        borg_cmd_avoidances();
        return;
    case 'f':
        borg_cmd_grid_feature();
        return;
    case 'g':
        borg_cmd_grid_glyphs();
        return;
    case 'm':
        borg_cmd_grid_monsters();
        return;
    case 'o':
        borg_cmd_grid_objects();
        return;
    default:
        msg("That is not a legal option.");
    }
}

/*
 * Interact with the Borg
 */
void do_cmd_borg(void)
{
    int  i;
    char cmd = 0;
    const int command_count = sizeof(borg_commands) / sizeof(borg_commands[0]);

    /* Set the Borg's player location using the game's internal state.
     *
     * Normally, the Borg is only supposed to "know" what is visible on the screen.
     * But at this point the screen has not yet been scanned, so we cannot rely on
     * screen data. Some Borg commands (a/%/@/#/^/_/y/!/G) require knowing the
     * player's position before screen processing occurs, so we temporarily break
     * the abstraction here to ensure correct behavior for those commands.
     *
     * This does not affect normal Borg behavior. All automated play and
     * decision-making still rely solely on information obtained by scanning the screen.
     */
    borg.c = player->grid;

    /* Get a "Borg command", or abort */
    if (!get_com("Borg command (? for help): ", &cmd)) {
        /* Restore the screen */
        Term_load();
        return;
    }

    for (i = 0; i < command_count; i++) {
        if (borg_commands[i].selection == cmd) {
            /*
             * Force initialization or reinitialize if the game was closed
             * and restarted without exiting since the last initialization
             */
            if (!borg_initialized) {
                if (borg_initialized) {
                    borg_free();
                }
                borg_init();

                if (borg_init_failure) {
                    borg_initialized = false;
                    borg_free();
                    borg_note("** startup failure borg cannot run ** ");
                    Term_fresh();
                    return;
                }
            }

            borg_commands[i].function();
            return;
        }
    }

    /* Message */
    msg("That is not a legal Borg command.");
    return;
}

#endif