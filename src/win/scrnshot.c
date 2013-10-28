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

#include <windows.h>
#include "png.h"

/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif

/*
 * Save a window to a PNG file.
 *
 * Returns TRUE if the PNG file is written, and FALSE if something went wrong.
 */
BOOL SaveWindow_PNG(HWND hWnd, LPSTR lpFileName)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers = NULL;

	BOOL noerror = TRUE;

	png_byte color_type;
	png_byte bit_depth;
	png_byte channels;

	int x, y;

	int width, height;
	char *c;
	FILE *fp;
	RECT rect;

	/* make sure that we have a window pointer */
	if (hWnd == NULL) {
		return (FALSE);
	}
	c = strrchr(lpFileName, '.');
	if (!c) {
		return (FALSE);
	}
	c+=1;
	if ((strncmp(c, "png", 3) != 0) && (strncmp(c, "PNG", 3) != 0)) {
		return (FALSE);
	}

	if (!GetClientRect(hWnd, &rect)) {
		/* ??? */
	}
	width = rect.right;
	height = rect.bottom;

	color_type = PNG_COLOR_TYPE_RGB;
	bit_depth = 8;
	channels = 3;

	/* open the file and test it for being a png */
	fp = fopen(lpFileName, "wb");
	if (!fp) {
		return (FALSE);
	}

	/* Create the png structure */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) {
		//plog_fmt("Unable to initialize PNG library");
		fclose(fp);
		return (FALSE);
	}

	/* create the info structure */
	if (noerror) {
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			//plog_fmt("Failed to create PNG info structure.");
			noerror = FALSE;
		}
	}

	if (noerror) {
		/* setup error handling for init */
		png_init_io(png_ptr, fp);

		png_set_IHDR(png_ptr, info_ptr, width, height,
					 bit_depth, color_type, PNG_INTERLACE_NONE,
					 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		if (bit_depth < 8) {
			png_set_packing(png_ptr);
		}
		png_write_info(png_ptr, info_ptr);
	}

	/* copy the allocate the memory libpng can access */
	if (noerror) {
		/* setup error handling for read */
		row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
		if (!row_pointers)
		{
			//plog_fmt("Failed to alloc temporary memory for PNG data.");
			noerror = FALSE;
		}
		if (noerror) {
			for (y = 0; y < height; ++y)
			{
				row_pointers[y] = (png_bytep) malloc(sizeof(png_bytep)*width*channels);
				if (!row_pointers[y])
				{
					//plog_fmt("Failed to alloc temporary memory for PNG data.");
					noerror = FALSE;
					break;
				}
			}
		}
	}

	/* copy the data to it */
	if (noerror) {
		COLORREF bgr;
		byte b[3], *data;
		HDC hDC, hdcWnd;
    		HBITMAP hbmScreen, hbmOld;


		hdcWnd = GetDC(hWnd);
		if (!hdcWnd) {
		}
		hbmScreen = CreateCompatibleBitmap(hdcWnd, width, height);
		if (!hbmScreen) {
		}
		hDC = CreateCompatibleDC(hdcWnd);
		if (!hDC) {
		}
		hbmOld = SelectObject(hDC, hbmScreen);
		BitBlt(hDC, 0, 0, width, height, hdcWnd, 0, 0, SRCCOPY);
		ReleaseDC(hWnd,hdcWnd);

		/* copy just the color data */
		for (y = 0; y < height; ++y) {
			data = row_pointers[y];
			for (x = 0; x < width; ++x) {
				bgr = GetPixel(hDC, x,y);
				b[2] = ((bgr&0x000000FF));
				b[1] = ((bgr&0x0000FF00)>>8);
				b[0] = ((bgr&0x00FF0000)>>16);
				*(data++) = b[2];
				*(data++) = b[1];
				*(data++) = b[0];
			}
		}

		SelectObject(hDC, hbmOld);
		DeleteObject(hbmScreen);
		DeleteDC(hDC);
	}  

	/* write the file */
	if (noerror)
	{
		//png_set_bgr(png_ptr);
		png_write_image(png_ptr, row_pointers);
		png_write_end(png_ptr, NULL);
	}

	/* release the image memory */
	for (y = 0; y < height; ++y)
	{
		free(row_pointers[y]);
	}
	free(row_pointers);

	/* we are done with the file pointer, so
	 * release all the the PNG Structures */
	if (info_ptr) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		info_ptr = NULL;
		png_ptr = NULL;
	}
	else if (png_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		png_ptr = NULL;
	}

	/* we are done with the file pointer, so close it */
	if (fp) {
		fclose(fp);
		fp = NULL;
	}

	if (!noerror)
	{
		return (FALSE);
	}
	return (TRUE);
}
