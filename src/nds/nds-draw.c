#include "nds-draw.h"

#ifdef _3DS
# include <3ds.h>
#else
# include <nds.h>
#endif

void nds_video_init() {
#ifdef _3DS
	gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	gfxSetDoubleBuffering(GFX_TOP, false);
#else
	powerOn(POWER_ALL_2D | POWER_SWAP_LCDS);
	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000); /* BG2, event buf, fonts */
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000); /* for storage (tileset) */
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000); /* for storage (tileset) */
	vramSetBankE(VRAM_E_LCD);                /* for storage (WIN_TEXT) */
	vramSetBankF(VRAM_F_LCD);                /* for storage (WIN_TEXT) */
	REG_BG2CNT = BG_BMP16_256x256;
	REG_BG2PA = 1 << 8;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 1 << 8;
	REG_BG2Y = 0;
	REG_BG2X = 0;
	REG_BG0CNT_SUB = BG_TILE_BASE(0) | BG_MAP_BASE(8) | BG_PRIORITY(0) | BG_COLOR_16;
	REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(2);
	REG_BG2PA_SUB = 1 << 8;
	REG_BG2PB_SUB = 0;
	REG_BG2PC_SUB = 0;
	REG_BG2PD_SUB = 1 << 8;
	REG_BG2Y_SUB = 0;
	REG_BG2X_SUB = 0;
#endif
}

void nds_video_vblank() {
#ifdef _3DS
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
#else
	swiWaitForVBlank();
#endif
}

void nds_draw_color_pixel(u16b x, u16b y, nds_pixel data) {
#ifdef _3DS
	nds_pixel *fb = (nds_pixel *) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
#else
	nds_pixel *fb = BG_GFX;
#endif

	/* Bottom screen? */
	if (y >= NDS_SCREEN_HEIGHT) {
#ifdef _3DS
		fb = (nds_pixel *) gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
#else
		fb = &BG_GFX_SUB[16 * 1024];
#endif
		y -= NDS_SCREEN_HEIGHT;
	}

#ifdef _3DS
	fb[x * NDS_SCREEN_HEIGHT + (NDS_SCREEN_HEIGHT - y - 1)] = data;
#else
	fb[y * NDS_SCREEN_WIDTH + x] = data | BIT(15);
#endif
}


void nds_draw_color_char(byte x, byte y, char c, nds_pixel clr)
{
	for (byte yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		for (byte xx = 0; xx < NDS_FONT_WIDTH; xx++) {
			nds_draw_color_pixel(x * NDS_FONT_WIDTH + xx,
			                     y * NDS_FONT_HEIGHT + yy,
			                     nds_font_pixel(c, xx, yy) & clr);
		}
	}
}

void nds_draw_cursor(int x, int y) {
	for (byte xx = 0; xx < NDS_FONT_WIDTH; xx++) {
		nds_draw_color_pixel(x * NDS_FONT_WIDTH + xx,
		                     y * NDS_FONT_HEIGHT,
		                     NDS_CURSOR_COLOR);
		nds_draw_color_pixel(x * NDS_FONT_WIDTH + xx,
		                     y * NDS_FONT_HEIGHT + (NDS_FONT_HEIGHT - 1),
		                     NDS_CURSOR_COLOR);
	}
	for (byte yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		nds_draw_color_pixel(x * NDS_FONT_WIDTH,
		                     y * NDS_FONT_HEIGHT + yy,
		                     NDS_CURSOR_COLOR);
		nds_draw_color_pixel(x * NDS_FONT_WIDTH + (NDS_FONT_WIDTH - 1),
		                     y * NDS_FONT_HEIGHT + yy,
		                     NDS_CURSOR_COLOR);
	}
}

/*
 * Find the square a particular pixel is part of.
 */
void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy)
{
	(*x) = ox / NDS_FONT_WIDTH;
	(*y) = oy / NDS_FONT_HEIGHT;
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
	static u16b x = 0, y = 32;
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