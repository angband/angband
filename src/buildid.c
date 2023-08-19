/**
 * \file buildid.c
 * \brief Compile in build details
 *
 * Copyright (c) 2011 Andi Sidwell
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

#include "buildid.h"

/*
 * Allow the build system to generate version.h (and define
 * the HAVE_VERSION_H preprocessor macro) or get the version via the BUILD_ID
 * preprocessor macro.  If neither is available, use a sensible default.
 */
#ifdef HAVE_VERSION_H
#include "version.h"
#elif defined(BUILD_ID)
#define STR(x) #x
#define XSTR(x) STR(x)
#define VERSION_STRING XSTR(BUILD_ID)
#endif
#ifndef VERSION_STRING
#define VERSION_STRING "4.2.5"
#endif

const char *buildid = VERSION_NAME " " VERSION_STRING;
const char *buildver = VERSION_STRING;

/**
 * Hack -- Link a copyright message into the executable
 */
const char *copyright =
	"Copyright (c) 1987-2022 Angband contributors.\n"
	"\n"
	"This work is free software; you can redistribute it and/or modify it\n"
	"under the terms of either:\n"
	"\n"
	"a) the GNU General Public License as published by the Free Software\n"
	"   Foundation, version 2, or\n"
	"\n"
	"b) the Angband licence:\n"
	"   This software may be copied and distributed for educational, research,\n"
	"   and not for profit purposes provided that this copyright and statement\n"
	"   are included in all such copies.  Other copyrights may also apply.\n";
