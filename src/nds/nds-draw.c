#include "nds-draw.h"

#ifdef _3DS
# include <3ds.h>
#else
# include <nds.h>
#endif

#ifdef _3DS
const nds_font_handle *nds_font = &nds_font_5x8;
#else
const nds_font_handle *nds_font = &nds_font_3x8;
#endif

void nds_video_init() {
#ifdef _3DS
	gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	gfxSetDoubleBuffering(GFX_TOP, false);
#else
	powerOn(POWER_ALL_2D | POWER_SWAP_LCDS);
	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000); /* BG2, event buf, fonts */
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000); /* for storage (tileset) */
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_LCD);
	vramSetBankE(VRAM_E_LCD);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);
	REG_BG2CNT = BG_BMP16_256x256;
	REG_BG2PA = 1 << 8;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 1 << 8;
	REG_BG2Y = 0;
	REG_BG2X = 0;
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

static inline nds_pixel *nds_get_framebuffer(uint16_t *y) {
#ifdef _3DS
	nds_pixel *fb = (nds_pixel *) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
#else
	nds_pixel *fb = BG_GFX;
#endif

	/* Bottom screen? */
	if (*y >= NDS_SCREEN_HEIGHT) {
#ifdef _3DS
		fb = (nds_pixel *) gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
#else
		fb = &BG_GFX_SUB[16 * 1024];
#endif
		*y -= NDS_SCREEN_HEIGHT;
	}

	return fb;
}

void nds_draw_pixel(uint16_t x, uint16_t y, nds_pixel data) {
	nds_pixel *fb = nds_get_framebuffer(&y);

#ifdef _3DS
	fb[x * NDS_SCREEN_HEIGHT + (NDS_SCREEN_HEIGHT - y - 1)] = data;
#else
	fb[y * NDS_SCREEN_WIDTH + x] = data;
#endif
}

void nds_draw_char_px(uint16_t x, uint16_t y, char c, nds_pixel clr_fg, nds_pixel clr_bg)
{
	nds_pixel *fb = nds_get_framebuffer(&y);

#ifdef _3DS
	nds_font->draw_char(c, fb + (x * NDS_SCREEN_HEIGHT) + (NDS_SCREEN_HEIGHT - y - 1), clr_fg, clr_bg);
#else
	nds_font->draw_char(c, fb + (y * NDS_SCREEN_WIDTH) + x, clr_fg, clr_bg);
#endif
}

void nds_draw_str_px(uint16_t x, uint16_t y, const char *str, nds_pixel clr_fg, nds_pixel clr_bg)
{
	while (*str != '\0') {
		nds_draw_char_px(x, y, *(str++), clr_fg, clr_bg);
		x += nds_font->width;
	}
}

void nds_draw_char(uint8_t x, uint8_t y, char c, nds_pixel clr_fg, nds_pixel clr_bg)
{
	nds_draw_char_px(x * nds_font->width, y * nds_font->height, c, clr_fg, clr_bg);
}

void nds_draw_str(uint8_t x, uint8_t y, const char *str, nds_pixel clr_fg, nds_pixel clr_bg)
{
	nds_draw_str_px(x * nds_font->width, y * nds_font->height, str, clr_fg, clr_bg);
}

void nds_draw_cursor(int x, int y) {
	for (uint8_t xx = 0; xx < nds_font->width; xx++) {
		nds_draw_pixel(x * nds_font->width + xx,
		               y * nds_font->height,
		               NDS_CURSOR_COLOR);
		nds_draw_pixel(x * nds_font->width + xx,
		               y * nds_font->height + (nds_font->height - 1),
		               NDS_CURSOR_COLOR);
	}
	for (uint8_t yy = 0; yy < nds_font->height; yy++) {
		nds_draw_pixel(x * nds_font->width,
		               y * nds_font->height + yy,
		               NDS_CURSOR_COLOR);
		nds_draw_pixel(x * nds_font->width + (nds_font->width - 1),
		               y * nds_font->height + yy,
		               NDS_CURSOR_COLOR);
	}
}

/*
 * Find the square a particular pixel is part of.
 */
void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy)
{
	(*x) = ox / nds_font->width;
	(*y) = oy / nds_font->height;
}

void nds_log(const char *msg)
{
	static uint8_t x = 2, y = 1;
	for (uint8_t i = 0; msg[i] != '\0'; i++) {
		nds_draw_char(x, y, msg[i], NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
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
	static uint16_t x = 0, y = 32;
	while (*str) {
		nds_draw_char(x, y, *(str++), NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
		x++;
		if (x > 78) {
			x = 0;
			y++;
			if (y > 34)
				y = 32;
		}
	}
	nds_draw_char(x, y, 219, NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
	fflush(0);
}
