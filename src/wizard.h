/**
 * \file wizard.h
 * \brief Debug mode commands, stats collection, spoiler generation
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef INCLUDED_WIZARD_H
#define INCLUDED_WIZARD_H

/* wiz-debug.c */
void wiz_cheat_death(void);

/* wiz-stats.c */
bool stats_are_enabled(void);
void stats_collect(int nsim, int simtype);
void disconnect_stats(int nsim, bool stop_on_disconnect);
void pit_stats(int nsim, int pittype, int depth);

/* wiz-spoil.c */
void spoil_artifact(const char *fname);
void spoil_mon_desc(const char *fname);
void spoil_mon_info(const char *fname);
void spoil_obj_desc(const char *fname);

#endif /* !INCLUDED_WIZARD_H */
