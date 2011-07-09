/*
 * File: scrnshot.c
 * Purpose: Save a graphical screen shot on Windows systems.
 *
 * Copyright (c) 2011 Brett Reid
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
#ifndef INCLUDED_GRAFMODE_H
#define INCLUDED_GRAFMODE_H

#include "h-basic.h"
/* Specifications for graphics modes.  */
typedef struct _graphics_mode {
  struct _graphics_mode *pNext;
	byte grafID;			  // id of tile set should be >0 and unique for anything new
	byte alphablend;	 	// bool whether or not the tileset needs alpha blending
	byte overdrawRow;	  // row in the file where tiles in that row or lower draw the tile above as well
	byte overdrawMax;	  // row in the file where tiles in that row or above draw the tile above as well
  u16b cell_width;    // width of an individual tile in pixels
  u16b cell_height;   // height of an individual tile in pixels
	char name[8];			  // Value of ANGBAND_GRAF variable
 	char file[32];			// name of png file (if any)
	char menuname[32];	// Name of the tileset in menu
} graphics_mode;

extern graphics_mode *graphics_modes;
extern graphics_mode *current_graphics_mode;
extern int graphics_mode_high_id;

bool init_graphics_modes(const char *filename);
void close_graphics_modes(void);

#endif /* INCLUDED_GRAFMODE_H */
