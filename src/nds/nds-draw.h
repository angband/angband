#ifndef _NDS_DRAW_H
#define _NDS_DRAW_H

#include <nds.h>
#include "nds-font-3x8.h"

#define NDS_SCREEN_WIDTH 256
#define NDS_SCREEN_HEIGHT 192

#define NDS_SCREEN_COLS (NDS_SCREEN_WIDTH / NDS_FONT_WIDTH)
#define NDS_SCREEN_LINES (NDS_SCREEN_HEIGHT / NDS_FONT_HEIGHT)

void nds_pixel_to_square(int *const x, int *const y, const int ox,
                         const int oy);

void nds_draw_color_char(byte x, byte y, char c, u16 clr);

inline void nds_draw_char(byte x, byte y, char c) {
    nds_draw_color_char(x, y, c, 0xffff);
}

void nds_log(const char *msg);
void nds_logf(const char* format, ...);
void nds_raw_print(const char *str);

#endif