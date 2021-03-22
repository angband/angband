/**
 * \file main.h
 * \brief Core game initialisation for UNIX (and other) machines
 *
 * Copyright (c) 1997 Ben Harrison, and others
 * Copyright (c) 2002 Robert Ruehlmann
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

#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include "angband.h"
#include "ui-term.h"

extern errr init_lfb(int argc, char **argv);
extern errr init_x11(int argc, char **argv);
extern errr init_xpj(int argc, char **argv);
extern errr init_gcu(int argc, char **argv);
extern errr init_cap(int argc, char **argv);
extern errr init_dos(int argc, char **argv);
extern errr init_ibm(int argc, char **argv);
extern errr init_emx(int argc, char **argv);
extern errr init_sla(int argc, char **argv);
extern errr init_lsl(int argc, char **argv);
extern errr init_ami(int argc, char **argv);
extern errr init_vme(int argc, char **argv);
extern errr init_vcs(int argc, char **argv);
extern errr init_sdl(int argc, char **argv);
extern errr init_sdl2(int argc, char **argv);
extern errr init_test(int argc, char **argv);
extern errr init_stats(int argc, char **argv);
extern errr init_ibm(int argc, char **argv);


extern const char help_lfb[];
extern const char help_xpj[];
extern const char help_x11[];
extern const char help_vcs[];
extern const char help_gtk[];
extern const char help_gcu[];
extern const char help_cap[];
extern const char help_vme[];
extern const char help_ami[];
extern const char help_lsl[];
extern const char help_sla[];
extern const char help_emx[];
extern const char help_ibm[];
extern const char help_dos[];
extern const char help_sdl[];
extern const char help_sdl2[];
extern const char help_test[];
extern const char help_stats[];
extern const char help_ibm[];

//phantom server play
extern bool arg_force_name;


struct module
{
	const char *name;
	const char *help;
	errr (*init)(int argc, char **argv);
};

#endif /* INCLUDED_MAIN_H */
