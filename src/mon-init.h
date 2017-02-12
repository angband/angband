/**
 * \file mon-init.h
 * \brief Parsing functions for monsters and monster base types.
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2011 noz
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

#ifndef MONSTER_INIT_H_
#define MONSTER_INIT_H_

extern const char *r_info_flags[];
extern const char *r_info_spell_flags[];
extern struct file_parser lore_parser;
extern struct file_parser meth_parser;
extern struct file_parser eff_parser;
extern struct file_parser mon_spell_parser;
extern struct file_parser monster_parser;
extern struct file_parser mon_base_parser;
extern struct file_parser pit_parser;
extern struct file_parser pain_parser;


#endif /* MONSTER_INIT_H_ */
