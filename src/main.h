/* File: main.h */

/*
 * Copyright (c) 2002 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include "angband.h"

extern errr init_gtk(int argc, char **argv);
extern errr init_xaw(int argc, char **argv);
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


extern const char help_xpj[];
extern const char help_xaw[];
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


struct module
{
	cptr name;
	cptr help;
	errr (*init)(int argc, char **argv);
};

#endif /* INCLUDED_MAIN_H */
