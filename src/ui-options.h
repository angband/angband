/**
 * \file ui-options.h
 * \brief Text UI options handling code (everything accessible from '=')
 *
 * Copyright (c) 1997-2000 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 2007 Pete Mack
 * Copyright (c) 2010 Andi Sidwell
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
#ifndef INCLUDED_UI_OPTIONS_H
#define INCLUDED_UI_OPTIONS_H

#include "obj-ignore.h"

void do_cmd_options_birth(void);
int ego_item_name(char *buf, size_t buf_size, struct ego_desc *desc);
bool ignore_tval(int tval);
void do_cmd_options_item(const char *title, int row);
void do_cmd_options(void);
void cleanup_options(void);

#endif
