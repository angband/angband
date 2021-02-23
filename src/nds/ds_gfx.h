
#ifndef DS_GFX_H
#define DS_GFX_H

#define C_RED		0x1
#define C_GREEN		0x2
#define C_BLUE		0x4
#define C_BRIGHT	0x8

// y + 32 = draw on subscreen
void draw_char(byte x, byte y, char c);
void draw_color_char(byte x, byte y, char c, byte clr);
void draw_curs(byte x, byte y);
void render_all_windows();
bool nds_load_kbd();	// now this is just kbd gfx
void nds_log(const char* msg);


#endif

