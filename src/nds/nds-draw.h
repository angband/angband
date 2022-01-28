#ifndef _NDS_DRAW_H
#define _NDS_DRAW_H

#include "../h-basic.h"


#ifdef __3DS__

/* RGBA8 */
typedef uint32_t nds_pixel;
#define NDS_WHITE_PIXEL 0xffffffff
#define NDS_BLACK_PIXEL 0x000000ff
#define NDS_CURSOR_COLOR 0xffff00ff

#define NDS_SCREEN_WIDTH 400
#define NDS_SCREEN_HEIGHT 240

#define NDS_X_PITCH	NDS_SCREEN_HEIGHT
#define NDS_Y_PITCH	-1

#else

typedef uint16_t nds_pixel;
#define NDS_WHITE_PIXEL 0xffff
#define NDS_BLACK_PIXEL 0x0000
#define NDS_CURSOR_COLOR 0x83ff

#define NDS_SCREEN_WIDTH 256
#define NDS_SCREEN_HEIGHT 192

#define NDS_X_PITCH	1
#define NDS_Y_PITCH	256

#endif

typedef struct {
	uint8_t width;
	uint8_t height;
	void (*draw_char)(char c, nds_pixel *pixels, nds_pixel clr_fg, nds_pixel clr_bg);
} nds_font_handle;

extern const nds_font_handle *nds_font;

extern const nds_font_handle nds_font_3x8;
extern const nds_font_handle nds_font_5x8;

#define NDS_SCREEN_COLS (NDS_SCREEN_WIDTH / nds_font->width)
#define NDS_SCREEN_LINES (NDS_SCREEN_HEIGHT / nds_font->height)

void nds_video_init();
void nds_video_vblank();

void nds_draw_pixel(uint16_t x, uint16_t y, nds_pixel data);

/* Same as nds_draw_char, but x/y is pixels instead of tiles */
void nds_draw_char_px(uint16_t x, uint16_t y, char c, nds_pixel clr_fg, nds_pixel clr_bg);
void nds_draw_char(uint8_t x, uint8_t y, char c, nds_pixel clr_fg, nds_pixel clr_bg);

void nds_draw_str_px(uint16_t x, uint16_t y, const char *str, nds_pixel clr_fg, nds_pixel clr_bg);
void nds_draw_str(uint8_t x, uint8_t y, const char *str, nds_pixel clr_fg, nds_pixel clr_bg);

void nds_draw_cursor(int x, int y);

void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy);

void nds_log(const char *msg);
void nds_logf(const char* format, ...);
void nds_raw_print(const char *str);

#endif
