
#ifndef DS_WIN_H
#define DS_WIN_H

//#include "hack.h"

extern struct window_procs nds_procs;
extern bool nds_draw_text;
extern u8 nds_updated;

/*
typedef struct NDS_MENU_ITEM_STRUCT{
	struct NDS_MENU_ITEM_STRUCT* n;
	u8 acc, gacc;
	char title[80];
	menu_item mi;
} nds_nhmenuitem;

typedef struct {
	nds_nhmenuitem* f;
	nds_nhmenuitem* l;
	//char prompt[BUFSZ];
	u8 width;
} nds_nhmenu;

typedef struct {
	u16 type;
	u16 cx, cy;		// in chars/tiles
	u16 x, y;		// in chars/tiles
#define wrote_this_turn		x	// WIN_MESSAGE only
#define center_x			x	// WIN_MAP only
#define center_y			y	// WIN_MAP only
// for compatibility with old ds_gfx.c:
//#define center_pos			x	// WIN_MAP only
	nds_nhmenu m;
	u16* data;	// what to draw on the screen
	u16* text;	// text buffer for later display (NHW_MENU, NHW_TEXT)
	u16 cols, rows;
	boolean vis;
} nds_nhwindow;

#define MAX_WINDOWS		8	// if you change this, you must also change the size
							// of nds_updated in this file and ds_winhelp.c
extern nds_nhwindow wins[MAX_WINDOWS];
// only need WIN_TEXT for windows 3-7 = 0x014000 bytes
// WIN_TEXT(0) = 0x06874000 - 0x06878000, WIN_TEXT(3) = 0x06880000 - 0x06884000,
// WIN_TEXT(7) = 0x06890000 - 0x06894000
// memory used is vram banks E & F
#define WIN_TEXT(w)			((u16*)(0x06874000 + (0x4000 * (w))))
*/
#define MAX_EBUF 		512		// max of 512 events in queue = 1024 bytes
#define MEVENT_FLAG			(1<<15)
#define EVENT_SET			(1<<14)
#define EVENT_X(e)			((u8)((e) & 0x7F))
#define EVENT_Y(e)			((u8)(((e) & 0xF8) >> 7))
#define EVENT_C(e)			((u8)((e) & 0xFF))
#define IS_MEVENT(e)		((e) & MEVENT_FLAG)

#define MORE_MSG			"--More--"
#define MORE_LEN			8
#define LINE_LEN			85
#define LINE_LIMIT			((LINE_LEN)-(MORE_LEN))

extern short glyph2tile[];

bool has_event();
u16 get_event();
void put_key_event(u8 c);
void put_mouse_event(u8 x, u8 y);
//winid find_unused_window(int dir);
void more_print(const char*);
//void nds_win_updated(winid);
void do_more();

void nds_raw_print(const char *);
void nds_raw_print_bold(const char *);
//void nds_putstr(winid, int, const char *);
//void nds_clear_nhwindow(winid);
int  nds_nhgetch(void);
//void nds_curs(winid, int, int);
void nds_getlin2(const char*, char*, int);
//winid nds_create_nhwindow(int);
//void nds_destroy_nhwindow(winid);
//void nds_display_nhwindow(winid, BOOLEAN_P);
//void nds_start_menu(winid);
//void nds_add_menu(winid, int, const ANY_P *, CHAR_P, CHAR_P, int, const char *, BOOLEAN_P);
//void nds_end_menu(winid, const char*);
//int nds_select_menu(winid, int, menu_item**);
//void nds_free_menu(winid);
void nds_exit_nhwindows(const char *);


#endif

