/**
 * \file ui-signals.h
 * \brief Handle various OS signals
 *
 * Copyright (c) 1997 Ben Harrison
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

#ifndef INCLUDED_UI_SIGNALS_H
#define INCLUDED_UI_SIGNALS_H

extern int16_t signal_count;

void signals_ignore_tstp(void);
void signals_handle_tstp(void);
void signals_init(void);

#endif /* INCLUDED_UI_SIGNALS_H */
