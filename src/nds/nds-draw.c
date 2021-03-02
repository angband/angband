#include "nds-draw.h"

#include "../h-basic.h"

/*
 * Find the square a particular pixel is part of.
 */
void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy)
{
	(*x) = ox / NDS_FONT_WIDTH;
	(*y) = oy / NDS_FONT_HEIGHT;
}

void nds_draw_color_char(byte x, byte y, char c, u16 clr)
{
	u32 vram_offset = (y & 0x1F) * NDS_FONT_HEIGHT * NDS_SCREEN_WIDTH + x * NDS_FONT_WIDTH;

	u16 *fb = BG_GFX;
	if (y & 32) {
		fb = &BG_GFX_SUB[16 * 1024];
	}

	for (byte yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		for (byte xx = 0; xx < NDS_FONT_WIDTH; xx++) {
			fb[yy * NDS_SCREEN_WIDTH + xx + vram_offset] = (nds_font_pixel(c, xx, yy) & clr) | BIT(15);
		}
	}
}

void nds_log(const char *msg)
{
	static byte x = 2, y = 1;
	for (byte i = 0; msg[i] != '\0'; i++) {
		nds_draw_char(x, y, msg[i]);
		x++;
		if (msg[i] == '\n' || x > NDS_SCREEN_COLS - 2) {
			x = 2;
			y++;
		}
	}
}

void nds_logf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	int len = vsnprintf(NULL, 0, format, args);

	char buf[len];

	vsprintf(buf, format, args);
	nds_log(buf);

	va_end(args);
}

void nds_raw_print(const char *str)
{
	static u16 x = 0, y = 32;
	while (*str) {
		nds_draw_char(x, y, *(str++));
		x++;
		if (x > 78) {
			x = 0;
			y++;
			if (y > 34)
				y = 32;
		}
	}
	nds_draw_char(x, y, 219);
	fflush(0);
}