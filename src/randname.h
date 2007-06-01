/*
 * File: randname.h
 * Purpose: Random name generation
 * Based on W. Sheldon Simms name generator originally in randart.c
 *
 * Copyright (c) 2007 Antony Sidwell and others
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

#ifndef INCLUDED_RANDNAME_H
#define INCLUDED_RANDNAME_H

/* The different types of name make_word can generate */
typedef enum 
{
  RANDNAME_TOLKIEN = 1,
  RANDNAME_SCROLL,

  /* End of type marker - not a valid name type */
  RANDNAME_NUM_TYPES
} randname_type;


extern size_t make_word(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen);

#endif /* INCLUDED_RANDNAME_H */
