/**
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

#ifndef INCLUDED_BORG_MESSAGES_H
#define INCLUDED_BORG_MESSAGES_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

extern int16_t  borg_msg_len;
extern int16_t  borg_msg_siz;
extern char    *borg_msg_buf;
extern int16_t  borg_msg_num;
extern int16_t  borg_msg_max;
extern int16_t *borg_msg_pos;
extern int16_t *borg_msg_use;

/* Autostop search string */
extern char borg_match[128];

extern void borg_parse(char *msg);

extern void borg_init_messages(void);

extern void borg_free_messages(void);

#endif
#endif
