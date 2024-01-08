/**
 * \file borg-messages-react.c
 * \brief React to messages that have been received and parsed
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

#include "borg-messages-react.h"

#ifdef ALLOW_BORG

#include "borg-io.h"
#include "borg-messages.h"
#include "borg-trait.h"
#include "borg.h"

bool borg_dont_react = false;

/*
 * Handle various "important" messages
 *
 * Actually, we simply "queue" them for later analysis
 */
void borg_react(const char *msg, const char *buf)
{
    int len;

    if (borg_dont_react || borg.trait[BI_ISPARALYZED]) {
        borg_note("# Ignoring messages.");
        return;
    }

    /* Note actual message */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("> Reacting Msg (%s)", msg));

    /* Extract length of parsed message */
    len = strlen(buf);

    /* trim off trailing , if there is one, seems to have been introduced to
     * some uniques messages */
    if (len && buf[len - 1] == ',') {
        char *tmp  = (char *)buf; /* cast away const */
        tmp[--len] = 0;
    }

    /* Verify space */
    if (borg_msg_num + 1 > borg_msg_max) {
        borg_note("too many messages");
        return;
    }

    /* Verify space */
    if (borg_msg_len + len + 1 > borg_msg_siz) {
        borg_note("too much messages");
        return;
    }

    /* Assume not used yet */
    borg_msg_use[borg_msg_num] = 0;

    /* Save the message position */
    borg_msg_pos[borg_msg_num] = borg_msg_len;

    /* Save the message text */
    my_strcpy(borg_msg_buf + borg_msg_len, buf, borg_msg_siz - borg_msg_len);

    /* Advance the buf */
    borg_msg_len += len + 1;

    /* Advance the pos */
    borg_msg_num++;
}

/*
 * Clear saved messages
 */
void borg_clear_reactions(void)
{
    borg_msg_len = 0;
    borg_msg_num = 0;
}

#endif
