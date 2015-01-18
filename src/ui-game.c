/**
 * \file ui-game.c
 * \brief Game management for the traditional text UI
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2015 Nick McConnell
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
#include "ui-command.h"
#include "ui-context.h"
#include "ui-input.h"


/**
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void textui_process_command(void)
{
	int count = 0;
	bool done = TRUE;
	ui_event e;
	struct cmd_info *cmd = NULL;

	e = textui_get_command(&count);

	switch (e.type) {
		case EVT_RESIZE: do_cmd_redraw(); break;
		case EVT_MOUSE: textui_process_click(e); break;
		case EVT_BUTTON:
		case EVT_KBRD: done = textui_process_key(e.key, cmd, count); break;
		default: ;
	}

	if (!done)
		do_cmd_unknown();
}

errr textui_get_cmd(cmd_context context)
{
	if (context == CMD_GAME)
		textui_process_command();

	/* If we've reached here, we haven't got a command. */
	return 1;
}


