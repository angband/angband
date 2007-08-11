/* File: readdib.c */

/*
 * This package provides a routine to read a DIB file and set up the
 * device dependent version of the image.
 *
 * This file has been modified for use with "Angband 2.9.2"
 *
 * COPYRIGHT:
 *
 *   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
 *
 *   You have a royalty-free right to use, modify, reproduce and
 *   distribute the Sample Files (and/or any modified version) in
 *   any way you find useful, provided that you agree that
 *   Microsoft has no warranty obligations or liability for any
 *   Sample Application Files which are modified.
 */

#if defined(USE_WIN) || defined(WINDOWS) || defined(WIN32)

#include <windows.h>

#include "readdib.h"


/*
 * Needed for lcc-win32
 */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif


/*
 * Number of bytes to be read during each read operation
 */
#define MAXREAD  32768

/*
 * Private routine to read more than 64K at a time
 *
 * Reads data in steps of 32k till all the data has been read.
 *
 * Returns number of bytes requested, or zero if something went wrong.
 */
static DWORD PASCAL lread(HFILE fh, VOID FAR *pv, DWORD ul)
{
	DWORD ulT = ul;
	BYTE *hp = pv;

	while (ul > (DWORD)MAXREAD)
	{
		if (_lread(fh, (LPSTR)hp, (WORD)MAXREAD) != MAXREAD)
				return 0;
		ul -= MAXREAD;
		hp += MAXREAD;
	}
	if (_lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
		return 0;
	return ulT;
}


/*
 * Given a BITMAPINFOHEADER, create a palette based on the color table.
 *
 * Returns the handle of a palette, or zero if something went wrong.
 */
static HPALETTE PASCAL NEAR MakeDIBPalette(LPBITMAPINFOHEADER lpInfo)
{
	PLOGPALETTE npPal;
	RGBQUAD FAR *lpRGB;
	HPALETTE hLogPal;
	DWORD i;

	/*
	 * since biClrUsed field was filled during the loading of the DIB,
	 * we know it contains the number of colors in the color table.
	 */
	if (lpInfo->biClrUsed)
	{
		npPal = (PLOGPALETTE)LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
		                                 (WORD)lpInfo->biClrUsed * sizeof(PALETTEENTRY));
		if (!npPal)
			return (FALSE);

		npPal->palVersion = 0x300;
		npPal->palNumEntries = (WORD)lpInfo->biClrUsed;

		/* get pointer to the color table */
		lpRGB = (RGBQUAD FAR *)((LPSTR)lpInfo + lpInfo->biSize);

		/* copy colors from the color table to the LogPalette structure */
		for (i = 0; i < lpInfo->biClrUsed; i++, lpRGB++)
		{
			npPal->palPalEntry[i].peRed = lpRGB->rgbRed;
			npPal->palPalEntry[i].peGreen = lpRGB->rgbGreen;
			npPal->palPalEntry[i].peBlue = lpRGB->rgbBlue;
			npPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		}

		hLogPal = CreatePalette((LPLOGPALETTE)npPal);
		LocalFree((HANDLE)npPal);
		return (hLogPal);
	}

	/*
	 * 24-bit DIB with no color table.  return default palette.  Another
	 * option would be to create a 256 color "rainbow" palette to provide
	 * some good color choices.
	 */
	else
	{
		return (GetStockObject(DEFAULT_PALETTE));
	}
}


/*
 * Given a DIB, create a bitmap and corresponding palette to be used for a
 * device-dependent representation of the image.
 *
 * Returns TRUE on success (phPal and phBitmap are filled with appropriate
 * handles.  Caller is responsible for freeing objects) and FALSE on failure
 * (unable to create objects, both pointer are invalid).
 */
static BOOL NEAR PASCAL MakeBitmapAndPalette(HDC hDC, HANDLE hDIB,
                                             HPALETTE * phPal, HBITMAP * phBitmap)
{
	LPBITMAPINFOHEADER lpInfo;
	BOOL result = FALSE;
	HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
	LPSTR lpBits;

	lpInfo = (LPBITMAPINFOHEADER) GlobalLock(hDIB);
	if ((hPalette = MakeDIBPalette(lpInfo)) != 0)
	{
		/* Need to realize palette for converting DIB to bitmap. */
		hOldPal = SelectPalette(hDC, hPalette, TRUE);
		RealizePalette(hDC);

		lpBits = ((LPSTR)lpInfo + (WORD)lpInfo->biSize +
		          (WORD)lpInfo->biClrUsed * sizeof(RGBQUAD));
		hBitmap = CreateDIBitmap(hDC, lpInfo, CBM_INIT, lpBits,
		                         (LPBITMAPINFO)lpInfo, DIB_RGB_COLORS);

		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);

		if (!hBitmap)
		{
			DeleteObject(hPalette);
		}
		else
		{
			*phBitmap = hBitmap;
			*phPal = hPalette;
			result = TRUE;
		}
	}
	return (result);
}



/*
 * Reads a DIB from a file, obtains a handle to its BITMAPINFO struct, and
 * loads the DIB.  Once the DIB is loaded, the function also creates a bitmap
 * and palette out of the DIB for a device-dependent form.
 *
 * Returns TRUE if the DIB is loaded and the bitmap/palette created, in which
 * case, the DIBINIT structure pointed to by pInfo is filled with the appropriate
 * handles, and FALSE if something went wrong.
 */
BOOL ReadDIB(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo)
{
	HFILE fh;
	LPBITMAPINFOHEADER lpbi;
	OFSTRUCT of;
	BITMAPFILEHEADER bf;
	WORD nNumColors;
	BOOL result = FALSE;
	DWORD offBits;
	HDC hDC;
	BOOL bCoreHead = FALSE;

	/* Open the file and get a handle to it's BITMAPINFO */
	fh = OpenFile(lpFileName, &of, OF_READ);
	if (fh == -1)
		return (FALSE);

	pInfo->hDIB = GlobalAlloc(GHND, (DWORD)(sizeof(BITMAPINFOHEADER) +
	                          256 * sizeof(RGBQUAD)));

	if (!pInfo->hDIB)
		return (FALSE);

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(pInfo->hDIB);

	/* read the BITMAPFILEHEADER */
	if (sizeof (bf) != _lread(fh, (LPSTR)&bf, sizeof(bf)))
		goto ErrExit;

	/* 'BM' */
	if (bf.bfType != 0x4d42)
		goto ErrExit;

	if (sizeof(BITMAPCOREHEADER) != _lread(fh, (LPSTR)lpbi, sizeof(BITMAPCOREHEADER)))
		goto ErrExit;

	if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
	{
		lpbi->biSize = sizeof(BITMAPINFOHEADER);
		lpbi->biBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
		lpbi->biPlanes = ((LPBITMAPCOREHEADER)lpbi)->bcPlanes;
		lpbi->biHeight = ((LPBITMAPCOREHEADER)lpbi)->bcHeight;
		lpbi->biWidth = ((LPBITMAPCOREHEADER)lpbi)->bcWidth;
		bCoreHead = TRUE;
	}
	else
	{
		/* get to the start of the header and read INFOHEADER */
		_llseek(fh, sizeof(BITMAPFILEHEADER), SEEK_SET);
		if (sizeof(BITMAPINFOHEADER) != _lread(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)))
			goto ErrExit;
	}

	if ((nNumColors = (WORD)lpbi->biClrUsed) == 0)
	{
		/* no color table for 24-bit, default size otherwise */
		if (lpbi->biBitCount != 24)
			nNumColors = 1 << lpbi->biBitCount;
	}

	/* fill in some default values if they are zero */
	if (lpbi->biClrUsed == 0)
		lpbi->biClrUsed = nNumColors;

	if (lpbi->biSizeImage == 0)
	{
		lpbi->biSizeImage = (((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31) & ~31) >> 3)
		                     * lpbi->biHeight);
	}

	/* otherwise wouldn't work with 16 color bitmaps -- S.K. */
	else if ((nNumColors == 16) && (lpbi->biSizeImage > bf.bfSize))
	{
		lpbi->biSizeImage /= 2;
	}

	/* get a proper-sized buffer for header, color table and bits */
	GlobalUnlock(pInfo->hDIB);
	pInfo->hDIB = GlobalReAlloc(pInfo->hDIB, lpbi->biSize +
										nNumColors * sizeof(RGBQUAD) +
										lpbi->biSizeImage, 0);

	/* can't resize buffer for loading */
	if (!pInfo->hDIB)
		goto ErrExit2;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(pInfo->hDIB);

	/* read the color table */
	if (!bCoreHead)
	{
		_lread(fh, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));
	}
	else
	{
		signed int i;
		RGBQUAD FAR *pQuad;
		RGBTRIPLE FAR *pTriple;

		_lread(fh, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBTRIPLE));

		pQuad = (RGBQUAD FAR *)((LPSTR)lpbi + lpbi->biSize);
		pTriple = (RGBTRIPLE FAR *) pQuad;
		for (i = nNumColors - 1; i >= 0; i--)
		{
			pQuad[i].rgbRed = pTriple[i].rgbtRed;
			pQuad[i].rgbBlue = pTriple[i].rgbtBlue;
			pQuad[i].rgbGreen = pTriple[i].rgbtGreen;
			pQuad[i].rgbReserved = 0;
		}
	}

	/* offset to the bits from start of DIB header */
	offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

	if (bf.bfOffBits != 0L)
	{
		_llseek(fh,bf.bfOffBits,SEEK_SET);
	}

	/* Use local version of '_lread()' above */
	if (lpbi->biSizeImage == lread(fh, (LPSTR)lpbi + offBits, lpbi->biSizeImage))
	{
		GlobalUnlock(pInfo->hDIB);

		hDC = GetDC(hWnd);
		if (!MakeBitmapAndPalette(hDC, pInfo->hDIB, &(pInfo->hPalette),
		                          &(pInfo->hBitmap)))
		{
			ReleaseDC(hWnd,hDC);
			goto ErrExit2;
		}
		else
		{
			ReleaseDC(hWnd,hDC);
			result = TRUE;
		}
	}
	else
	{
ErrExit:
		GlobalUnlock(pInfo->hDIB);
ErrExit2:
		GlobalFree(pInfo->hDIB);
	}

	_lclose(fh);
	return (result);
}


/* Free a DIB */
void FreeDIB(DIBINIT *dib)
{
	/* Free the bitmap stuff */
	if (dib->hPalette) DeleteObject(dib->hPalette);
	if (dib->hBitmap) DeleteObject(dib->hBitmap);
	if (dib->hDIB) GlobalFree(dib->hDIB);

	dib->hPalette = NULL;
	dib->hBitmap = NULL;
	dib->hDIB = NULL;
}

#endif /* USE_WIN || WINDOWS || WIN32 */
