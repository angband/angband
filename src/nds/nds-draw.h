#ifndef _NDS_DRAW_H
#define _NDS_DRAW_H

#include "../h-basic.h"


typedef uint16_t nds_pixel;
#define NDS_WHITE_PIXEL 0xffff
#define NDS_BLACK_PIXEL 0x0000
#define NDS_CURSOR_COLOR 0x83ff

#define NDS_SCREEN_WIDTH 256
#define NDS_SCREEN_HEIGHT 192

#include "nds-font-3x8.h"


#define NDS_SCREEN_COLS (NDS_SCREEN_WIDTH / NDS_FONT_WIDTH)
#define NDS_SCREEN_LINES (NDS_SCREEN_HEIGHT / NDS_FONT_HEIGHT)


void nds_video_init();
void nds_video_vblank();

void nds_draw_color_char(byte x, byte y, char c, nds_pixel clr);

inline void nds_draw_char(byte x, byte y, char c) {
    nds_draw_color_char(x, y, c, NDS_WHITE_PIXEL);
}

void nds_draw_cursor(int x, int y);

void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy);

void nds_log(const char *msg);
void nds_logf(const char* format, ...);
void nds_raw_print(const char *str);

#endif