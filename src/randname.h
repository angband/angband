/*
 * randname.h - random name generation
 * Copyright (c) 2007 Antony Sidwell (antony@isparp.co.uk) and others,
 * W. Sheldon Simms name generator originally in randart.c
 *
 * This file is distributed under the terms of both the Angband licence and
 * under the GPL licence (version 2 or any later version).  It may be 
 * redistributed under the terms of either licence. 
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
