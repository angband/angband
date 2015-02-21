/**
 * \file ui-prefs.h
 * \brief Pref file handling code
 *
 * Copyright (c) 2003 Takeshi Mogami, Robert Ruehlmann
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

#ifndef UI_PREFS_H
#define UI_PREFS_H

#include "cave.h"
#include "z-file.h"

extern int use_graphics;
extern int arg_graphics;
extern bool arg_graphics_nice;

byte *monster_x_attr;
wchar_t *monster_x_char;
byte *kind_x_attr;
wchar_t *kind_x_char;
byte *feat_x_attr[LIGHTING_MAX];
wchar_t *feat_x_char[LIGHTING_MAX];
byte *trap_x_attr[LIGHTING_MAX];
wchar_t *trap_x_char[LIGHTING_MAX];
byte *flavor_x_attr;
wchar_t *flavor_x_char;

void dump_monsters(ang_file *fff);
void dump_objects(ang_file *fff);
void dump_autoinscriptions(ang_file *f);
void dump_features(ang_file *fff);
void dump_flavors(ang_file *fff);
void dump_colors(ang_file *fff);
void option_dump(ang_file *fff);
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title);
errr process_pref_file_command(const char *buf);
bool process_pref_file(const char *name, bool quiet, bool user);
void reset_visuals(bool load_prefs);
void textui_prefs_init(void);
void textui_prefs_free(void);
void do_cmd_pref(void);

#endif /* !UI_PREFS_H */
