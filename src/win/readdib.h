/**
 * \file readdib.h
 *
 * This file has been modified for use with "Angband 2.8.2"
 *
 * Copyright 1991 Microsoft Corporation. All rights reserved.
 */

#ifndef INCLUDED_READDIB_H
#define INCLUDED_READDIB_H

#include <stdbool.h>

/**
 * Information about a bitmap
 */
typedef struct {
	HANDLE   hDIB;
	HBITMAP  hBitmap;
	HPALETTE hPalette;
	BYTE     CellWidth;
	BYTE     CellHeight;
  int      ImageWidth;
  int      ImageHeight;
} DIBINIT;

/**
 * Read a DIB from a file
 */
extern bool ReadDIB(HWND, LPSTR, DIBINIT *);

/**
 * Free a DIB
 */
extern void FreeDIB(DIBINIT *dib);

/**
 * New png stuff
 */
extern bool ReadDIB_PNG(HWND, LPSTR, DIBINIT *);
extern bool ReadDIB2_PNG(HWND, LPSTR, DIBINIT *, DIBINIT *, bool);

#endif /* !INCLUDED_READDIB_H */
