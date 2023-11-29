/**
 * \file ui-command.h
 * \brief Deal with UI only command processing.
 *
 * Copyright (c) 1997-2014 Angband developers
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

#ifndef UI_COMMAND_H
#define UI_COMMAND_H

#include "ui-term.h"

void do_cmd_redraw(void);
void do_cmd_xxx_options(void);
void do_cmd_unknown(void);
void do_cmd_version(void);
void textui_cmd_retire(void);
void html_screenshot(const char *path, int mode, term *other_term);
void do_cmd_save_screen(void);
void textui_cmd_rest(void);
void textui_quit(void);

#endif /* UI_COMMAND_H */
