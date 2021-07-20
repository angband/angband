
#ifndef DS_GFX_H
#define DS_GFX_H

#define C_RED		0x1
#define C_GREEN		0x2
#define C_BLUE		0x4
#define C_BRIGHT	0x8

extern u16b* tiles_bin; // rambank A + BG2(96k) + event queue(1k)
	// tiles_bin extends thru most of rambank B
	// the subscreen uses up all of rambank C

extern int total_tiles_used;
// y + 32 = draw on subscreen
void draw_tile(byte x, byte y, u16b tile);
void draw_char(byte x, byte y, char c);
void draw_color_char(byte x, byte y, char c, byte clr);
void draw_curs(byte x, byte y);
void render_all_windows();
bool nds_load_kbd();	// now this is just kbd gfx
bool nds_load_tiles();
void nds_fatal_err(const char* msg);


#endif

