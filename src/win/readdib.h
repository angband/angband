/* File: readdib.h */

#ifndef INCLUDED_READDIB_H
#define INCLUDED_READDIB_H

/*
 * This file has been modified for use with "Angband 2.8.2"
 *
 * Copyright 1991 Microsoft Corporation. All rights reserved.
 */

/*
 * Information about a bitmap
 */
typedef struct {
	HANDLE   hDIB;
	HBITMAP  hBitmap;
	HPALETTE hPalette;
	BYTE     CellWidth;
	BYTE     CellHeight;
} DIBINIT;

/* Read a DIB from a file */
extern BOOL ReadDIB(HWND, LPSTR, DIBINIT *);

/* Free a DIB */
extern void FreeDIB(DIBINIT *dib);

/* new png stuff */
extern BOOL ReadDIB_PNG(HWND, LPSTR, DIBINIT *);
extern BOOL ReadDIB2_PNG(HWND, LPSTR, DIBINIT *, DIBINIT *);

#endif /* !INCLUDED_READDIB_H */
