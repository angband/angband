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

#ifndef INCLUDED_BORG_FLOW_GLYPH_H
#define INCLUDED_BORG_FLOW_GLYPH_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-flow.h"

/*
 * Track glyphs
 */
extern struct borg_track track_glyph;

extern bool borg_needs_new_sea;

extern bool borg_flow_glyph(int why);

extern void borg_init_flow_glyph(void);
extern void borg_free_flow_glyph(void);

#endif
#endif
