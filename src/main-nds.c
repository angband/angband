/* File: main-nds.c */

/* Purpose: Main file for playing on the Nintendo DS */

/*
 * This file written by Nick McConnell, based on the template by Ben Harrison 
 * (benh@phial.com).  Many of the routines are based on, or lifted directly
 * from, brettk's excellent NethackDS:
 *         http://frodo.dyn.gno.org/~brettk/NetHackDS
 *
 */

#include <nds.h>
#include <fat.h>

#include "angband.h"
#include "buildid.h"
#include "main.h"

/* DS includes */
#include "nds/ds_errfont.h"
#include "nds/ds_main.h"
u16* subfont_rgb_bin = (u16*)(0x06018400);
u16* subfont_bgr_bin = (u16*)(0x0601C400);
u16* top_font_bin;
u16* btm_font_bin;
u16* tiles_bin;// = (u16*)0x06020400;


#define NDS_BUTTON_FILE		"buttons.dat"

#define NDS_MAPPABLE_MASK	(KEY_A | KEY_B | KEY_X | KEY_Y | KEY_START | KEY_SELECT)
#define NDS_MODIFIER_MASK	(KEY_L | KEY_R)
#define NDS_BUTTON_MASK		(NDS_MAPPABLE_MASK | NDS_MODIFIER_MASK)
#define NDS_NUM_MAPPABLE	6      // A, B, X, Y, Select, Start
#define NDS_NUM_MODIFIER	2      // R, L
#define NDS_CMD_LENGTH		16     // max. 15 keys/button + null terminator

//[mappable]*2^[mods] things to map commands to, [cmd_length] chars per command
byte nds_btn_cmds[NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER][NDS_CMD_LENGTH];

/* make sure there's something there to start with - NRM */
byte btn_defaults[] = 
  {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'z'};

const s16 mappables[] = { KEY_A, KEY_B, KEY_X, KEY_Y, KEY_SELECT, KEY_START };
const s16 modifiers[] = { KEY_L, KEY_R };
s16 nds_buttons_to_btnid(u16 kd, u16 kh) {
	if (!(kd & NDS_MAPPABLE_MASK)) return -1;
	u16 i, mods = 0;
	for (i=0;i<NDS_NUM_MODIFIER;i++) {
		if (kh & modifiers[i]) mods |= (1 << i);
	}
	for (i=0;i<NDS_NUM_MAPPABLE;i++) {
		if (kd & mappables[i]) return i + NDS_NUM_MAPPABLE * (mods);
	}
	return -1;
}

//extern int total_tiles_used;	//in tile.c, used only here
#define total_tiles_used 512	//hack, guess NRM

#define DEF_TILE_WIDTH		8
#define DEF_TILE_HEIGHT		8
#define DEF_TILE_FILE		"/angband/lib/xtra/graf/8x8.bmp"
#define DEF_TILES_PER_ROW       32

// don't change these
//NRM #define TILE_WIDTH		iflags.wc_tile_width
//NRM #define TILE_HEIGHT		iflags.wc_tile_height
//NRM #define TILE_FILE		iflags.wc_tile_file
u16b TILE_WIDTH;
u16b TILE_HEIGHT;
char *TILE_FILE;
u16b NDS_SCREEN_COLS;
u16b NDS_SCREEN_ROWS;
#define c1(a,i)		(RGB15((a[i]>>3),(a[i+1]>>3),(a[i+2]>>3)))
#define c2(a,i)		(RGB15((a[i+2]>>3),(a[i+1]>>3),(a[i]>>3)))
#define TILE_BUFFER_SIZE		(TILE_WIDTH*TILE_HEIGHT*(total_tiles_used+1)*2)

const nds_kbd_key row0[] = {
  {16,(u16)'`'}, {16,(u16)'1'}, {16,(u16)'2'}, {16,(u16)'3'}, {16,(u16)'4'}, 
  {16,(u16)'5'}, {16,(u16)'6'}, {16,(u16)'7'}, {16,(u16)'8'}, {16,(u16)'9'}, 
  {16,(u16)'0'}, {16,(u16)'-'}, {16,(u16)'='}, {32,(u16)'\b'}, {0,0}};
const nds_kbd_key row1[] = {
  {24,(u16)'\t'}, {16,(u16)'q'}, {16,(u16)'w'}, {16,(u16)'e'}, {16,(u16)'r'}, 
  {16,(u16)'t'}, {16,(u16)'y'}, {16,(u16)'u'}, {16,(u16)'i'}, {16,(u16)'o'}, 
  {16,(u16)'p'}, {16,(u16)'['}, {16,(u16)']'}, {24,(u16)'\\'}, {0,0}};
const nds_kbd_key row2[] = {
  {32,K_CAPS}, {16,(u16)'a'}, {16,(u16)'s'}, {16,(u16)'d'}, {16,(u16)'f'}, 
  {16,(u16)'g'}, {16,(u16)'h'}, {16,(u16)'j'}, {16,(u16)'k'}, {16,(u16)'l'}, 
  {16,(u16)';'}, {16,(u16)'\''}, {32,(u16)'\n'}, {0,0}};
const nds_kbd_key row3[] = {
  {40,K_SHIFT}, {16,(u16)'z'}, {16,(u16)'x'}, {16,(u16)'c'}, {16,(u16)'v'}, 
  {16,(u16)'b'}, {16,(u16)'n'}, {16,(u16)'m'}, {16,(u16)','}, {16,(u16)'.'}, 
  {16,(u16)'/'}, {40,K_SHIFT}, {0,0}};
const nds_kbd_key row4[] = {
  {32,K_CTRL}, {24,K_ALT}, {128,(u16)' '}, {24,K_ALT}, {32,K_CTRL}, {0,0}};
char shifts[] = "`~1!2@3#4$5%6^7&8*9(0)-_=+[{]}\\|;:\'\",<.>/?";
const nds_kbd_key *kbdrows[] = {row0, row1, row2, row3, row4};





/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data
{
  term t;
  
  byte rows;
  byte cols;
  
  int tile_height;
  int tile_width; 
};



/*
 * Number of "term_data" structures to support XXX XXX XXX
 *
 * You MUST support at least one "term_data" structure, and the
 * game will currently use up to eight "term_data" structures if
 * they are available.
 *
 * If only one "term_data" structure is supported, then a lot of
 * the things that would normally go into a "term_data" structure
 * could be made into global variables instead.
 */
#define MAX_TERM_DATA 1


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * Colour data
 */

u16b color_data[] = {
	RGB15(  0,  0,  0), 		/* TERM_DARK */
	RGB15( 31, 31, 31), 		/* TERM_WHITE */
	RGB15( 15, 15, 15), 		/* TERM_SLATE */
	RGB15( 31, 15,  0),		/* TERM_ORANGE */ 
	RGB15( 23,  0,  0), 		/* TERM_RED */
	RGB15(  0, 15,  9), 		/* TERM_GREEN */
	RGB15(  0,  0, 31), 		/* TERM_BLUE */
	RGB15( 15,  9,  0), 		/* TERM_UMBER */
	RGB15(  9,  9,  9), 		/* TERM_L_DARK */
	RGB15( 23, 23, 23), 		/* TERM_L_WHITE */
	RGB15( 31,  0, 31), 		/* TERM_VIOLET */
	RGB15( 31, 31,  0), 		/* TERM_YELLOW */
	RGB15( 31,  0,  0), 		/* TERM_L_RED */
	RGB15(  0, 31,  0), 		/* TERM_L_GREEN */
	RGB15(  0, 31, 31), 		/* TERM_L_BLUE */
	RGB15( 23, 15,  9)		/* TERM_L_UMBER */
};



/*** Function hooks needed by "Term" ***/


/*
 * Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_nds(term *t)
{
	term_data *td = (term_data*)(t->data);

	/* XXX XXX XXX */
}



/*
 * Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_nds(term *t)
{
	term_data *td = (term_data*)(t->data);

	/* XXX XXX XXX */
}


/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int * const x, int * const y,
	const int ox, const int oy)
{
	(*x) = ox / TILE_WIDTH;
	(*y) = oy / TILE_HEIGHT;
}


/*
 * Handle a touch on the touch screen.
 */
static void handle_touch(int x, int y, int button, bool press)
{
	/* The co-ordinates are only used in Angband format. */
	pixel_to_square(&x, &y, x, y);

	if (press) Term_mousepress(x, y, button);
}

/*
 * Touchscreen keyboard handling
 */

static bool shift = false, ctrl = false, alt = false, caps = false;

u16b kbd_mod_code(u16 ret) 
{
  if (ret & K_MODIFIER) return ret;
  if (caps && !shift) 
    {
      if (ret >= 'a' && ret <= 'z') ret -= 0x20;
    }
  if (shift) 
    {
      char* temp;
      if (!caps && ret >= 'a' && ret <= 'z') ret -= 0x20;
      if ((temp = strchr(shifts,ret)) != NULL) ret = *(temp + 1);
    }
  if (alt) 
    {
      ret |= 0x80;
    }
  if (ctrl/* && ret >= 'a' && ret < 'a'+32*/) 
    {
      ret = ret & 0x1f;
    }
  return ret;
}

void kbd_set_color_from_pos(u16b r, u16b k, byte color) 
{
  u16b ii, xx = 0, jj;
  u16b *map[] = { 
    (u16b*)(BG_MAP_RAM_SUB(8)+3*32*2), 
    (u16b*)(BG_MAP_RAM_SUB(9)+3*32*2), 
    (u16b*)(BG_MAP_RAM_SUB(10)+3*32*2),
    (u16b*)(BG_MAP_RAM_SUB(11)+3*32*2) 
  };
  for (ii = 0; ii < k; ii++) 
    {
      xx += kbdrows[r][ii].width >> 3;
    }
  for (ii = 0; ii < (kbdrows[r][k].width>>3); ii++) 
    {
      for (jj = 0; jj < 4; jj++) 
	{
	  map[jj][(10 + r * 2) * 32 + ii + xx + 1] 
	    = (map[jj][(10+r*2)*32+ii+xx+1] & 0x0FFF) | (color << 12);
	  map[jj][(10 + r * 2 + 1) * 32 + ii + xx + 1] 
	    = (map[jj][(10+r*2+1)*32+ii+xx+1] & 0x0FFF) | (color << 12);
    }
  }
}

void kbd_set_color_from_code(u16b code, byte color) 
{
  u16b r,k;
  for (r = 0; r < 5; r++) 
    {
      for (k = 0; kbdrows[r][k].width != 0; k++) 
	{
	  if (kbd_mod_code(kbdrows[r][k].code) == code)  
	    {
	      kbd_set_color_from_pos(r,k,color);
	    }
	  // do not break!! there may be >1 key with this code (modifier keys)
	}
    }
}

void kbd_set_map() {
  SUB_BG0_CR = BG_TILE_BASE(0) | BG_MAP_BASE(8 + (caps | (shift<<1))) | BG_PRIORITY(0) | BG_16_COLOR;
  
}

u16b kbd_xy2key(byte x, byte y) 
{
  if (x >= 104 && x < 152 && y >=24 && y < 72) 
    {	// on arrow-pad
      byte kx = (x-104)/16, ky = (y-24)/16;
      return (kx + (2 - ky) * 3 + 1 + '0')/* | (shift ? K_SHIFTED_MOVE : 0)*/;
    
    }
  if (y >=80 && y < 96) 
    {
      if (x >= 8 && x < 24) return '\033';
      if (x >= 40 && x < 248) {	// F-key
	x -= 40;
	y = x/72;	// which section
	x -= y*72;	// offset in section
	if (x < 64) 
	  {
	    return K_F(y*4+(x>>4)+1);	// section*4 + offset/16 + 1
	  } 
	else 
	  {
	    return 0;
	  }
      }
      
    }
  s16b ox = x - 8, oy = y-104;
  if (ox < 0 || ox >= 240) return 0;
  if (oy < 0 || oy >= 80) return 0;
  u16b row = oy / 16;
  int i;
  for (i = 0; ox > 0; ox -= kbdrows[row][i++].width);
  u16b ret = kbdrows[row][i-1].code;
  return kbd_mod_code(ret);
}

void kbd_dotoggle(bool *flag, int how) 
{
  switch (how) 
    {
    case 0: *flag = false; return;
    case 1: *flag = true; return;
    default:
    case -1: *flag = !*flag; return;
    }
}

// which: K_SHIFT, K_CTRL, K_ALT, K_MODIFIER=all keys
// how: -1 = toggle, 0 = off, 1 = on
void kbd_togglemod(int which, int how) 
{
  //boolean old_shift = shift, old_ctrl = ctrl, old_alt = alt, old_caps = caps;
  switch (which) 
    {
    case K_CTRL: kbd_dotoggle(&ctrl,how); break;
    case K_SHIFT: kbd_dotoggle(&shift,how); break;
    case K_ALT: kbd_dotoggle(&alt,how); break;
    case K_CAPS: kbd_dotoggle(&caps,how); break;
    case K_MODIFIER:
      kbd_dotoggle(&ctrl,how);
      kbd_dotoggle(&shift,how);
      kbd_dotoggle(&alt,how);
      // NOT caps!!  This is called to un-set shift, ctrl, and alt after
      // a key is pressed.  Unsetting caps here would cause it to be the
      // same as shift.
      break;
    }
  
  /* if (old_shift != shift) */
  kbd_set_color_from_code(K_SHIFT,shift);
  
  /* if (old_ctrl != ctrl) */
  kbd_set_color_from_code(K_CTRL,ctrl);

  /* if (old_alt != alt) */
  kbd_set_color_from_code(K_ALT,alt);

  /* if (old_caps != caps) */
  kbd_set_color_from_code(K_CAPS,caps);

  kbd_set_map();
}


// clear this to prevent alt-b, f5, and f6 from having their special effects
// it's cleared during getlin, yn_function, etc
byte process_special_keystrokes = 1;

// run this every frame
// returns a key code if one has been typed, else returns 0
// assumes scankeys() was already called this frame (in real vblank handler)
byte kbd_vblank() 
{
  // frames the stylus has been held down for
  static u16b touched = 0;
  // coordinates from each frame, the median is used to get the keycode
  static s16b xarr[3],yarr[3];
  // the keycode of the last key pressed, so it can be un-highlighted
  static u16b last_code;
  // the keycode of the currently pressed key, is usu. returned
  u16b keycode;
  
  // if screen is being touched...
  if (keysHeld() & KEY_TOUCH) {
    if (touched < 3) {	// if counter < 3...
      touched++;				// add to counter
      xarr[touched-1] = IPC->touchXpx;	// add this to the array for
      yarr[touched-1] = IPC->touchYpx;	// finding the median
    }
  } 
  else 
    {	// not being touched
      touched = 0;	// so reset the counter for next time
    }
  
  // if the stylus was released
  if (keysUp() & KEY_TOUCH) 
    {
      // if last_code is set and it wasn't a modifier
      if (last_code && !(last_code & K_MODIFIER)) 
	{
	  // clear the hiliting on this key
	  kbd_set_color_from_code(last_code,0);
	  // and also clear all modifiers (except caps)   
	  kbd_togglemod(K_MODIFIER, 0);
	}
      last_code = 0;
    }
  
  // if the screen has been touched for 3 frames...
  if (touched == 3) 
    {
      touched++;	// do not return the keycode again
      // also, not setting to zero prevents the keysHeld() thing
      //  from starting the process over and getting 3 more samples
      
      u16b i, tmp, the_x=0, the_y=0;
      
      // x/yarr now contains 3 values from each of the 3 frames
      // take the median of each array and put into the_x/y
      
      // sort the array
      // bubble sort, ugh
      for (i = 1; i < 3; i++) 
	{
	  if (xarr[i] < xarr[i-1]) 
	    {
	      tmp = xarr[i];
	      xarr[i] = xarr[i-1];
	      xarr[i-1] = tmp;
	    }
	  if (yarr[i] < yarr[i-1]) 
	    {
	      tmp = yarr[i];
	      yarr[i] = yarr[i-1];
	      yarr[i-1] = tmp;
	    }
	}
    
      // get the middle value (median)
      // if it's -1, take the top value
      if (xarr[1] == -1) the_x = xarr[2];
      else the_x = xarr[1];
      if (yarr[1] == -1) the_y = yarr[2];
      else the_y = yarr[1];
      
      // get the keycode that corresponds to this key
      u16b keycode = kbd_xy2key(the_x, the_y);
      
      // if it's not a modifier, highlight it
      if (keycode && !(keycode & K_MODIFIER)) 
	kbd_set_color_from_code(keycode,1);
      // set last_code so it can be un-highlighted later
      last_code = keycode;
      
      /*// check for special keystrokes: alt-b, f5, f6
      if (process_special_keystrokes) {
	// alt-b: assign button macro
	if (keycode == ('b' | 0x80)) {
	  // clear hiliting
	  kbd_set_color_from_code(keycode,0);
	  kbd_togglemod(K_MODIFIER,0);
	  nds_assign_button();
	  keycode = last_code = 0;	// don't let nethack process it
	}
	
	if (keycode & K_F(0)) {	// its an f-key
	  if (keycode == K_F(5)) {	// F5: toggle to text mode
	    nds_ascii_graphics = ~nds_ascii_graphics;
	    iflags.use_color = nds_ascii_graphics;
	    //doredraw();
	    keycode = 'R' & 0x1F;	// send a redraw command to nethack
	    last_code = 0;
	  } else if (keycode == K_F(6)) {	// F6: toggle top font
	    swap_font(false);
	    nds_updated = 0xFF;
	    if (access("/NetHack/swapfont",04)!= -1) {
	      unlink("/NetHack/swapfont");
	    } else {
	      FILE* f = fopen("/NetHack/swapfont","w");
	      fwrite(&f,4,1,f);	// otherwise FileExists doesnt work
	      fclose(f);
	    }
	    keycode = last_code = 0;
	  }
	  kbd_togglemod(K_MODIFIER,0);
	}
	}*/
      
      // if it's a modifier, toggle it
      if (keycode & K_MODIFIER) kbd_togglemod(keycode,-1);
      else if ((keycode & 0x7F) != 0) {	// it's an actual keystroke, return it
	return (keycode & 0xFF);
      }
    }
  
  return 0;
}


void nds_check_buttons(u16b kd, u16b kh) 
{
  s16b btn = nds_buttons_to_btnid(kd,kh);
  if (btn == -1) return;
  byte *cmd = &nds_btn_cmds[btn][0];
  while (*cmd != 0) {
    put_key_event(*(cmd++));
  }
}

/*
 * All event handling 
 */
u16b *ebuf = (u16b*)(&BG_GFX[256*192]);
// store the queue just past mainscreen display data
u16b ebuf_read = 0, ebuf_write = 0;
byte nds_updated = 0;	// windows that have been updated and should be redrawn

bool has_event() {
  return ((ebuf[ebuf_read] & EVENT_SET) || (ebuf_read < ebuf_write));
  // read < write should never happen without EVENT_SET, but
  // just in case...
}

u16b get_event() {
  if (!has_event()) return 0;
  u16b r = ebuf[ebuf_read];
  ebuf[ebuf_read] = 0;
  ebuf_read++;
  if (ebuf_read > ebuf_write) 
    {
      ebuf_write++;
      if (ebuf_write >= MAX_EBUF) ebuf_write = 0;
    }
  if (ebuf_read >= MAX_EBUF) ebuf_read = 0;
  return r;
}

void put_key_event(byte c) 
{
  ebuf[ebuf_write++] = EVENT_SET | (u16)c;
  if (ebuf_write >= MAX_EBUF) ebuf_write = 0;
}

void put_mouse_event(byte x, byte y) 
{
  ebuf[ebuf_write++] = EVENT_SET | MEVENT_FLAG | (u16b)x | (((u16b)y) << 7);
  if (ebuf_write >= MAX_EBUF) ebuf_write = 0;
}

void do_vblank() {
  swiWaitForVBlank();
  
  // ---------------------------
  //  Handle the arrow buttons
  scanKeys();
  u32b kd = keysDown();
  u32b kh = keysHeld();
  // order of keys: Right, Left, Up, Down
  // map keys to dirs, depends on order of keys in nds/input.h
  //  and order of directions in ndir & sdir in decl.c
  //const s8 k2d[] = {	// indexes into ndir/sdir, 10 = end of string = '\0'
  // 10, 4, 0, 10, 2, 3, 1, 10, 6, 5, 7	// no working combinations >= 11
  //};
  const byte k2d[] = {'6','4','8','2','3','7','9','1'  };
  // only do stuff if a key was pressed last frame
  if (kd & (KEY_RIGHT | KEY_LEFT | KEY_UP | KEY_DOWN)) {
    u16b dirs_down = 0;
    int i;
    if (kh & KEY_LEFT) dirs_down++;
    if (kh & KEY_RIGHT) dirs_down++;
    if (kh & KEY_UP) dirs_down++;
    if (kh & KEY_DOWN) dirs_down++;
    if (dirs_down == 1 && !(kh & (KEY_R | KEY_L))) 
      {
	/*if (iflags.num_pad) put_key_event(ndir[k2d[(kh >> 4) & 0xF]]);
	  else put_key_event(sdir[k2d[(kh >> 4) & 0xF]]);*/
	//NRM put_key_event(k2d[(kh >> 4) & 0xF]);
	for (i = 0; i < 4; i++)
	  if (kh & (1 << (i + 4))) 
	    //Term_keypress(k2d[i]);
	    put_key_event(k2d[i]);
      } 
    else 
      if (dirs_down == 2 && (kh & (KEY_R | KEY_L))) 
	{
	  /*if (iflags.num_pad) put_key_event(ndir[k2d[(kh >> 4) & 0xF]]);
	    else put_key_event(sdir[k2d[(kh >> 4) & 0xF]]);*/
	  //NRM put_key_event(k2d[(kh >> 4) & 0xF]);
	  for (i = 0; i < 4; i++)
	    if (kh & (1 << (i + 4))) 
	      //Term_keypress(k2d[i + 4]);
	      put_key_event(k2d[i + 4]);
	}
  }
  
  // ---------------------------
  //  Check for button macros
  nds_check_buttons(kd, kh);
  
  // ---------------------------
  //  Check for typing on the touchscreen kbd
  byte keycode = kbd_vblank();
  if ((keycode & 0x7F) != 0) {	// it's an actual keystroke, return it
    put_key_event(keycode & 0xFF);
    //Term_keypress(keycode & 0xFF);
  }
  
  // ---------------------------
  //  Print free RAM
  //NRM freeprint();
  
  // ---------------------------
  //  If the screen needs to be redrawn, do so now
  //NRM if (nds_updated) {
    //NRM render_all_windows();
    //NRM nds_updated = 0;
}

//END JUST MOVED

/*
 * An event handler XXX XXX XXX
 *
 * You may need an event handler, which can be used by both
 * by the "TERM_XTRA_BORED" and "TERM_XTRA_EVENT" entries in
 * the "Term_xtra_xxx()" function, and also to wait for the
 * user to perform whatever user-interface operation is needed
 * to request the start of a new game or the loading of an old
 * game, both of which should launch the "play_game()" function.
 */
static errr CheckEvents(bool wait)
{
  u16b e = 0;

  do_vblank();

  if (!wait && !has_event()) return (1);

  while (!e) 
    {
      e = get_event();

      do_vblank();
    }

  /* Mouse */
  if (IS_MEVENT(e)) 
    handle_touch(EVENT_X(e) + 1, EVENT_Y(e), 1, TRUE);

  /* Undefined */
  else if ((EVENT_C(e) & 0x7F) == 0)
    return (1);

  /* Key */
  else
    Term_keypress(EVENT_C(e));

#if 0
  u32b kd, kh;
  const byte k2d[] = {'6','4','8','2','3','7','9','1'  };

  /* Check the event queue */
  swiWaitForVBlank();
  scanKeys();
  kd = keysDown();
  kh = keysHeld();
  if (!wait && !kd)
    return (1);

  /* Wait for an event */
  while (!kd && !kh)
    {
      swiWaitForVBlank();
      scanKeys();
      kd = keysDown();
      kh = keysHeld();

      // only do stuff if a key was pressed last frame
      /* Arrow keys */
      if (kd & (KEY_RIGHT | KEY_LEFT | KEY_UP | KEY_DOWN)) 
	{
	  u16b dirs_down = 0;
	  int i;
	  if (kh & KEY_LEFT) dirs_down++;
	  if (kh & KEY_RIGHT) dirs_down++;
	  if (kh & KEY_UP) dirs_down++;
	  if (kh & KEY_DOWN) dirs_down++;
	  if (dirs_down == 1 && !(kh & (KEY_R | KEY_L))) 
	    {
	      for (i = 0; i < 4; i++)
		if (kh & (1 << (i + 4))) Term_keypress(k2d[i]);
	    } 
	  else 
	    if (dirs_down == 2 && (kh & (KEY_R | KEY_L))) 
	      {
		for (i = 0; i < 4; i++)
		  if (kh & (1 << (i + 4))) Term_keypress(k2d[i + 4]);
	      }
	}
  
      // ---------------------------
      //  Check for button macros
      nds_check_buttons(kd, kh);
  
      // ---------------------------
      //  Check for typing on the touchscreen kbd
      u8 keycode = kbd_vblank();
      if ((keycode & 0x7F) != 0) {	// it's an actual keystroke, return it
	//put_key_event(keycode & 0xFF);
	Term_keypress(keycode & 0xFF);
      }
    }
#endif

  return (0);
}


/*
 * Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "z-term.c" package,
 * or by the application itself.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_nds(int n, int v)
{
  term_data *td = (term_data*)(Term->data);
  
  /* Analyze */
  switch (n)
    {
    case TERM_XTRA_EVENT:
      {
	/*
	 * Process some pending events 
	 */
	return (CheckEvents(v));
      }
      
    case TERM_XTRA_FLUSH:
      {
	/*
	 * Flush all pending events 
	 */
	while (!CheckEvents(FALSE)); 
	
	return (0);
      }
      
    case TERM_XTRA_CLEAR:
      {
	/*
	 * Clear the entire window 
	 */
	int x, y;
	u32b vram_offset;
	u16b *fb = BG_GFX;

	for (y = 0; y < 24; y++)
	  {
	    for (x = 0; x < 80; x++)
	      {
		vram_offset = (y & 0x1F)*8*256+x*3;
		//if (y&32) 
		//  {
		//    fb = &BG_GFX_SUB[16*1024];
		//    chardata = btm_font_bin;
		//  }
	
		byte xx,yy;
		for (yy=0;yy<8;yy++)
		  for (xx=0;xx<3;xx++) 
		    fb[yy*256+xx+vram_offset] = 0;
	      }
	  }
	
	return (0);
      }
      
    case TERM_XTRA_SHAPE:
      {
	/*
	 * Set the cursor visibility XXX XXX XXX
	 *
	 * This action should change the visibility of the cursor,
	 * if possible, to the requested value (0=off, 1=on)
	 *
	 * This action is optional, but can improve both the
	 * efficiency (and attractiveness) of the program.
	 */
	
	return (0);
      }
      
    case TERM_XTRA_FROSH:
      {
	return (0);
      }
      
    case TERM_XTRA_FRESH:
      {
	return (0);
      }
      
    case TERM_XTRA_NOISE:
      {
	/*
	 * Make a noise XXX XXX XXX
	 *
	 * This action should produce a "beep" noise.
	 *
	 * This action is optional, but convenient.
	 */
	
	return (0);
      }
      
    case TERM_XTRA_SOUND:
      {
	/*
	 * Make a sound XXX XXX XXX
	 *
	 * This action should produce sound number "v", where the
	 * "name" of that sound is "sound_names[v]".  This method
	 * is still under construction.
	 *
	 * This action is optional, and not very important.
	 */
	
	return (0);
      }
      
    case TERM_XTRA_BORED:
      {
	/*
	 * Handle random events when bored 
	 */
	return (CheckEvents(0));
      }
      
    case TERM_XTRA_REACT:
      {
	/*
	 * React to global changes XXX XXX XXX
	 *
	 * For example, this action can be used to react to
	 * changes in the global "color_table[256][4]" array.
	 *
	 * This action is optional, but can be very useful for
	 * handling "color changes" and the "arg_sound" and/or
	 * "arg_graphics" options.
	 */
	
	return (0);
      }
      
    case TERM_XTRA_ALIVE:
      {
	/*
	 * Change the "hard" level XXX XXX XXX
	 *
	 * This action is used if the program changes "aliveness"
	 * by being either "suspended" (v=0) or "resumed" (v=1)
	 * This action is optional, unless the computer uses the
	 * same "physical screen" for multiple programs, in which
	 * case this action should clean up to let other programs
	 * use the screen, or resume from such a cleaned up state.
	 *
	 * This action is currently only used by "main-gcu.c",
	 * on UNIX machines, to allow proper "suspending".
	 */
	
	return (0);
      }
      
    case TERM_XTRA_LEVEL:
      {
	/*
	 * Change the "soft" level XXX XXX XXX
	 *
	 * This action is used when the term window changes "activation"
	 * either by becoming "inactive" (v=0) or "active" (v=1)
	 *
	 * This action can be used to do things like activate the proper
	 * font / drawing mode for the newly active term window.  This
	 * action should NOT change which window has the "focus", which
	 * window is "raised", or anything like that.
	 *
	 * This action is optional if all the other things which depend
	 * on what term is active handle activation themself, or if only
	 * one "term_data" structure is supported by this file.
	 */
	
	return (0);
      }
      
    case TERM_XTRA_DELAY:
      {
	/*
	 * Delay for some milliseconds 
	 */
	int i;
	for (i = 0; i < v; i++)
	  swiWaitForVBlank();
	
	return (0);
      }
    }
  
  /* Unknown or Unhandled action */
  return (1);
}


/*
 * Display the cursor
 */
static errr Term_curs_nds(int x, int y)
{
  //term_data *td = (term_data*)(Term->data);
  u32b vram_offset = (y - 1) * TILE_HEIGHT * 256 + x * TILE_WIDTH + 8 * 256;
  byte xx, yy;
  for (xx = 0; xx < TILE_WIDTH; xx++) 
    {
      BG_GFX[xx + vram_offset] 
	= RGB15(31, 31, 0)| BIT(15);
      BG_GFX[256 * (TILE_HEIGHT-1) + xx + vram_offset] 
	= RGB15(31, 31, 0)| BIT(15);
    }
  for (yy = 0; yy < TILE_HEIGHT; yy++) 
    {
      BG_GFX[yy * 256 + vram_offset] 
	= RGB15(31, 31, 0)| BIT(15);
      BG_GFX[yy * 256 + TILE_WIDTH - 1 + vram_offset] 
	= RGB15(31, 31, 0)| BIT(15);
    }
  
  
  /* Success */
  return (0);
}


void draw_char(byte x, byte y, char c) 
{
  u32b vram_offset = (y & 0x1F) * 8 * 256 + x * 3, tile_offset = c * 24;
  u16b* fb = BG_GFX;
  const u16b* chardata = top_font_bin;
  if (y & 32) 
    {
      fb = &BG_GFX_SUB[16 * 1024];
      chardata = btm_font_bin;
    }
  byte xx, yy;
  for (yy = 0; yy < 8; yy++)
    for (xx = 0; xx < 3; xx++) 
      fb[yy * 256 + xx + vram_offset] 
	= chardata[yy * 3 + xx + tile_offset] | BIT(15);
}

void draw_color_char(byte x, byte y, char c, byte clr) 
{
  u32b vram_offset = (y & 0x1F) * 8 * 256 + x * 3, tile_offset = c * 24;
  u16b* fb = BG_GFX;
  const u16b* chardata = top_font_bin;
  if (y & 32) 
    {
      fb = &BG_GFX_SUB[16*1024];
      chardata = btm_font_bin;
    }
  byte xx, yy;
  u16b val;
  u16b fgc = color_data[clr & 0xF];//, bgc = color_data[(clr & 0xFF) >> 4];
  for (yy = 0; yy < 8; yy++) 
    {
      for (xx = 0;xx < 3; xx++) 
	{
	  val = (chardata[yy * 3 + xx + tile_offset]);
	  fb[yy * 256 + xx + vram_offset] = (val & fgc) | BIT(15);//(~val&bgc) | 0x8000;
	}
    }
}

/*
 * Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_nds(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	int i;

	/* Draw a blank */
	for (i = 0; i < n; i++)
	  draw_color_char(x + i, y, 0, 0);

	/* Success */
	return (0);
}


/*
 * Draw some text on the screen
 *
 * This function should actually display an array of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which contains
 * exactly "n" characters and which is NOT null-terminated.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * In color environments, you should activate the color contained
 * in "color_data[a & 0x0F]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that if you have changed the "attr_blank" to something
 * which is not black, then this function must be able to draw
 * the resulting "blank" correctly.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_nds(int x, int y, int n, byte a, const char *cp)
{
  //term_data *td = (term_data*)(Term->data);
  int i;
  
  /* Do nothing if the string is null */
  if (!cp || !*cp) return (-1);
  
  /* Get the length of the string */
  if ((n > strlen(cp)) || (n < 0)) n = strlen(cp);

  /* Put the characters directly */
  for (i = 0; i < n, *cp; i++) 
    {
      /* Check it's the right attr */
      if ((x + i < Term->wid) && (Term->scr->a[y][x + i] == a))
	/* Put the char */
	draw_color_char(x + i, y, (*(cp++)), a);
      else 
	break;
    }
  /* Success */
  return (0);
}


void draw_tile(byte x, byte y, u16b tile) {
  u32b vram_offset = (y & 0x7F) * TILE_HEIGHT * 256 + x * TILE_WIDTH + 
    8 * 256, 
    tile_offset = (tile & 0x7FFF) * TILE_WIDTH * TILE_HEIGHT;
  u16b* fb = BG_GFX;
  byte xx, yy;
  for (yy = 0; yy < TILE_HEIGHT; yy++)
    for (xx = 0; xx < TILE_WIDTH; xx++) 
      fb[yy * 256 + xx + vram_offset] = 
	tiles_bin[yy * TILE_WIDTH + xx + tile_offset] | BIT(15);
}

/*
 * Draw some attr/char pairs on the screen
 *
 * This routine should display the given "n" attr/char pairs at
 * the given location (x,y).  This function is only used if one
 * of the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pairs, when displayed, will
 * erase anything (including any visual cursor) that used to be at
 * the given location.  On many machines this is automatic, but on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * the attr/char pair as an encoded index into a pixmap of special
 * "pictures".
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn by this function, which can be
 * very useful if this file can optimize its own display calls.
 *
 * This function is often associated with the "arg_graphics" flag.
 *
 * This function is only used if one of the "higher_pict" and/or
 * "always_pict" flags are set.
 */
static errr Term_pict_nds(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	u16b tile_number = DEF_TILES_PER_ROW * (*ap - 0x80) + (*cp - 0x80); 
	/* XXX XXX XXX */

	int i;
	
	/* Put the characters directly */
	for (i = 0; i < n, *cp; i++) 
	  {
	    if ((x + i < Term->wid) && (*cp != '\0')) 
	      draw_tile(x + i, y, tile_number);
	    else 
	      break;
	  }
	/* Success */
	return (0);
}


/*** Internal Functions ***/


/*
 * Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.  One way to allow some
 * variation is to add fields to the "term_data" structure listing
 * parameters for that window, initialize them in the "init_xxx()"
 * function, and then use them in the code below.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(int i)
{
  term_data *td = &data[i];
  
  term *t = &td->t;
  
  /* Initialize the term */
  term_init(t, 85, 24, 256);
  
  /* Choose "soft" or "hard" cursor XXX XXX XXX */
  /* A "soft" cursor must be explicitly "drawn" by the program */
  /* while a "hard" cursor has some "physical" existance and is */
  /* moved whenever text is drawn on the screen.  See "term.c". */
  t->soft_cursor = TRUE;
  
  /* Use "Term_pict()" for all attr/char pairs XXX XXX XXX */
  /* See the "Term_pict_xxx()" function above. */
  /* td->t->always_pict = TRUE; */
  
  /* Use "Term_pict()" for some attr/char pairs XXX XXX XXX */
  /* See the "Term_pict_xxx()" function above. */
  t->higher_pict = TRUE;
  
  /* Use "Term_text()" even for "black" text XXX XXX XXX */
  /* See the "Term_text_xxx()" function above. */
  /* t->always_text = TRUE; */
  
  /* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
  /* This may make things slightly more efficient. */
  t->never_bored = TRUE;
  
  /* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
  /* This may make things slightly more efficient. */
  /* td->t->never_frosh = TRUE; */
  
  /* Prepare the init/nuke hooks */
  t->init_hook = Term_init_nds;
  t->nuke_hook = Term_nuke_nds;
  
  /* Prepare the template hooks */
  t->xtra_hook = Term_xtra_nds;
  t->curs_hook = Term_curs_nds;
  t->wipe_hook = Term_wipe_nds;
  t->text_hook = Term_text_nds;
  t->pict_hook = Term_pict_nds;

  /* Remember where we came from */
  t->data = (vptr)(td);
  
  /* Activate it */
  Term_activate(t);
  
  /* Global pointer  - maybe need for some configurations */
  //ang_term[i] = t;
}



/*
 * Initialization function
 */
errr init_nds(void)
{
  /* Initialize globals */
  
  /* Initialize "term_data" structures */
  
  int i;
  bool none = TRUE;
  
  term_data *td;

  /* Main window */
  td = &data[0];
  WIPE(td, term_data);
  td->rows = 24;
  td->cols = 37;//80;
  td->tile_height = 8;
  td->tile_width = 3;
        
   /* Create windows (backwards!) */
  for (i = MAX_TERM_DATA - 1; i >= 0; i--)
    {
      /* Link */
      term_data_link(i);
      none = FALSE;
      
      /* Set global pointer */
      angband_term[0] = Term;
    }
  
  if (none) return (1);
  
  /* Success */
  return (0);
}


/*
 * Init some stuff
 *
 * This function is used to keep the "path" variable off the stack.
 */
static void init_stuff(void)
{
	char path[1024];

	/* Prepare the path */
	strcpy(path, "/angband/lib/");

	/* Prepare the filepaths */
	init_file_paths(path, path, path);

	/* Hack */
	strcpy(savefile, "/angband/lib/save/PLAYER");

	//small_screen = TRUE;
}

void nds_init_fonts() {
  // the font is now compiled in as ds_subfont for error reporting purposes
  // ds_subfont contains the bgr version
  //subfont_bgr_bin = &ds_subfont[0];
  u16b i;
  u16b t,t2;
  for (i=0;i<8*3*256;i++) {
    t = ds_subfont[i];
    t2 = t & 0x8000;
    t2 |= (t & 0x001f)<<10;
    t2 |= (t & 0x03e0);
    t2 |= (t & 0x7c00)>>10;
    subfont_bgr_bin[i] = t;
    subfont_rgb_bin[i] = t2;
  }
  top_font_bin = subfont_rgb_bin;
  btm_font_bin = subfont_bgr_bin;
}

// if you are calling this function, not much should be happening after
// since it clobbers the font pointers
void nds_fatal_err(const char* msg) {
  static byte x = 2, y = 1;
  byte i = 0;
  //top_font_bin = btm_font_bin = &ds_subfont[0];
  //	x = 2;
  //	y = 1;
  for (i = 0; msg[i] != '\0'; i++) {
    draw_char( x, y, msg[i]);
    x++;
    if (msg[i] == '\n' || x > 80) {
      x = 2;
      y++;
    }
  }
}

//NRM should be replaced with open and read from z-file.c
bool nds_load_file(const char* name, u16b* dest, u32b len) {
  FILE* f = fopen(name,"r");
  if (f == NULL) return false;
  u16b readbuf[1024];
  u32b i,l,wi=0;
  if (len == 0) len = 0xffffffff;	// max possible len
  for (i=0;i<1024;i++) readbuf[i] = 0;
  while ((l=fread(readbuf,2,1024,f)) > 0 && wi*2 < len) {
    for (i = 0; i < (l) && wi * 2 < len; i++) 
      {	// 0 to l/2
	dest[wi++] = readbuf[i];
      }
    for (i = 0; i < 1024; i++) readbuf[i] = 0;
  }
  fclose(f);
  return true;
}

bool nds_load_kbd() {
#define NUM_FILES	3
  const char *files[] = 
    {
      //	"subfont_rgb.bin","subfont_bgr.bin",
      "kbd.bin","kbd.pal","kbd.map",
    };
  const u16b* dests[] = 
    {
      //	subfont_rgb_bin, subfont_bgr_bin,
      (u16b*)BG_TILE_RAM_SUB(0), BG_PALETTE_SUB, (u16*)BG_MAP_RAM_SUB(8),
    };
  
  char buf[64] = "\0";
  u16b i;
  for (i = 0; i < NUM_FILES; i++) {
    if (!nds_load_file(files[i], dests[i], 0)) 
      {
	sprintf(buf,"Error opening %s (errno=%d)\n",files[i],errno);
	nds_fatal_err(buf);
	return FALSE;
      }
  }
#undef NUM_FILES
  
  return TRUE;
}

void kbd_init() {
  u16b i;
  for (i = 0; i < 16; i++) 
    {
      BG_PALETTE_SUB[i+16] = BG_PALETTE_SUB[i] ^ 0x7FFF;
    }
}

void nds_init_buttons() {
  u16b i, j;
  for (i = 0; i < (NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER); i++) 
    {
     for (j = 0; j < NDS_CMD_LENGTH; j++) 
       {
	 nds_btn_cmds[i][j] = 0;
       }
    }
  if (access(NDS_BUTTON_FILE,0444) == -1) 
    {
      /* Set defaults */
      for (i = 0; i < (NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER); i++) 
	nds_btn_cmds[i][0] = btn_defaults[i];
      
      return;
    }
  
  FILE* f = fopen(NDS_BUTTON_FILE, "r");
  fread(&nds_btn_cmds[0], NDS_CMD_LENGTH,
	(NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER), f);
  fclose(f);
}

void swap_font(bool bottom) 
{
  if (!bottom) 
    {
      if (top_font_bin == subfont_rgb_bin) top_font_bin = subfont_bgr_bin;
      else top_font_bin = subfont_rgb_bin;
    } 
  else 
    {
      if (btm_font_bin == subfont_rgb_bin) btm_font_bin = subfont_bgr_bin;
      else btm_font_bin = subfont_rgb_bin;
    }
}

void on_irq();

// on_irq, do nothing
void on_irq() {
  REG_IME = 0;
  if(REG_IF & IRQ_VBLANK) 
    {
      // Tell the DS we handled the VBLANK interrupt
      VBLANK_INTR_WAIT_FLAGS |= IRQ_VBLANK;
      REG_IF |= IRQ_VBLANK;
    } 
  else 
    {
      // Ignore all other interrupts
      REG_IF = REG_IF;
    }
  REG_IME=1;
}

void nds_raw_print(const char* str) 
{
  static u16b x=0,y=32;
  while (*str) 
    {
      draw_char(x,y,(u8)(*(str++)));
      x++;
      if (x > 78) 
	{
	  x = 0;
	  y++;
	  if (y > 34) y = 32;
	}
    }
  draw_char(x,y,219);
  fflush(0);
}

bool nds_load_tile_bmp(const char *name, u16b *dest, u32b len) 
{
  //NRM#define h	iflags.wc_tile_height
  //NRM#define w	iflags.wc_tile_width
#define h	TILE_HEIGHT
#define w	TILE_WIDTH
  // bmpxy2off works ONLY inside nds_load_tile_bmp!
#define bmpxy2off(x,y)	(((y-(y%h))*iw+(y%h))*w + x*h)
  FILE* f = fopen(name,"r");
  u32b writeidx = 0;
  u32b i,j,l;
  s16b y;
  u32b off;
  s32b iw2, ih2;
  u16b iw = 0, ih = 0;
  //s32 ty;
  u16b depth;
  char buf[10];
  //if (f) nds_raw_print("File OK");
  //else nds_raw_print("No file opened");
  fseek(f, 10, SEEK_SET);
  fread(&off,4,1,f);
  fseek(f, 4, SEEK_CUR);
  fread(&iw2,4,1,f);
  fread(&ih2,4,1,f);
  fseek(f, 2, SEEK_CUR);
  fread(&depth,2,1,f);
  strnfmt(buf, 10, "depth = %d", depth);
  //nds_raw_print(buf);
  if (depth != 24) 
    //NRM if (depth != 8) 
    {
      fclose(f);
      nds_raw_print(" depth problem");
      return false;
    }
  y = ih2 - 1;	// some crazy person decided to store the lines in a .bmp backwards
  ih = ih2 / h;
  iw = iw2 / w;
  
  if (len == 0) len = 0xffffffff;
  
  fseek(f,off,SEEK_SET);
  
  //NRM u8 temp[3];
  byte temp[1];
  while (y >= 0) 
    {
      for (i = 0; i < iw; i++) 
	{
	  writeidx = bmpxy2off(i*w, y);
	  for (j = 0; j < w; j++) 
	    {
	      //NRM fread(temp,1,3,f);
	      fread(temp, 1, 1, f);
	      if (writeidx * 2 < len) dest[writeidx++] = c2(temp, 0);
	    }
	}
      // x&3 == x%4
      //NRM if (((iw*w*3) & 3) != 0) fseek(f,4-((iw*w*3)&3),SEEK_CUR);
      fseek(f, 2 - (iw * w), SEEK_CUR);
      y--;
    }
  //*/
  
  fclose(f); 
  return true;
#undef bmpxy2off
#undef h
#undef w
} 

bool nds_load_tile_file(char* name, u16b* dest, u32b len) {
  char ext[4];
  u16b slen = strlen(name);
  strcpy(ext, name + slen - 3);
  nds_raw_print(name + len - 3);
  if (strcmpi(ext, "bmp") == 0) 
    {
      //    nds_raw_print("=isbmp ");
      return nds_load_tile_bmp(name, dest, len);
    } 
  else 
    {	// assume .bin maybe w/ funny ext
      return nds_load_file(name, dest, len);
    }
}

bool nds_load_tiles() 
{
  //char temp[30];
  //sprintf(temp,"%d,%s ",TILE_FILE == NULL, 
  //  TILE_FILE==NULL ? DEF_TILE_FILE : TILE_FILE);
  //nds_raw_print(temp);
  char buf[64];
  int died1 = -1, died2 = -1;;
  if (TILE_FILE != NULL) 
    {
      if (TILE_WIDTH == 0) TILE_WIDTH = DEF_TILE_WIDTH;
      if (TILE_HEIGHT == 0) TILE_HEIGHT = DEF_TILE_HEIGHT;
      tiles_bin = (u16b*)malloc(TILE_BUFFER_SIZE);
      if (!nds_load_tile_file(TILE_FILE, tiles_bin, TILE_BUFFER_SIZE) ) 
	{
	  died1 = errno;
	  free(tiles_bin);
	} 
      else 
	{
	  goto finish;
	}
    }
  TILE_WIDTH = DEF_TILE_WIDTH;
  TILE_HEIGHT = DEF_TILE_HEIGHT;
  tiles_bin = (u16b*)malloc(TILE_BUFFER_SIZE);
  if (!nds_load_tile_file(DEF_TILE_FILE, tiles_bin, TILE_BUFFER_SIZE) ) 
    {
      died2 = errno;
      free(tiles_bin);
    }
  
  if (died1 != -1) 
    {
      sprintf(buf, "Error loading tileset %s (errno=%d)\n", TILE_FILE, died1);
      if (died2 == -1) 
	{
	  nds_raw_print(buf);
	} 
      else 
	{
	  nds_fatal_err(buf);
	}
    }
  if (died2 != -1) 
    {
    sprintf(buf, "Error loading default tileset %s %s\n", DEF_TILE_FILE,
	    strerror(died2));
    nds_fatal_err(buf);
    return FALSE;
    }
  
  
 finish:
  NDS_SCREEN_ROWS = 168 / TILE_HEIGHT;
  NDS_SCREEN_COLS = 256 / TILE_WIDTH;
  return TRUE;
  //	nds_raw_print("r/c set");
}

/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
  /* Warning */
  if (str)
    {
      nds_raw_print(str);
    }
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(const char *str)
{
  int i, j;
  
  
  /* Give a warning */
  if (str)
    {
      nds_fatal_err(str);
    }

  /* Bail */
  nds_exit(0);
}


void nds_exit(int code) {
  u16b i;
  for (i = 0; i < 60; i++) {
    nds_updated = 0xFF;
    do_vblank();	// wait 1 sec.
  }
  IPC->mailData = 0xDEADC0DE;	// tell arm7 to shut down the DS
}


/*
 * Main function
 *
 * This function must do a lot of stuff.
 */
int main(int argc, char *argv[])
{
  bool new_game = FALSE;
  int i;

  /* Initialize the machine itself  */
  //START NETHACK STUFF
  
  powerON(POWER_ALL_2D | POWER_SWAP_LCDS);
  videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
  videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
  vramSetBankA(VRAM_A_MAIN_BG_0x06000000); // BG2, event buf, fonts
  vramSetBankB(VRAM_B_MAIN_BG_0x06020000);       // for storage (tileset)
  vramSetBankC(VRAM_C_SUB_BG_0x06200000);
  vramSetBankD(VRAM_D_MAIN_BG_0x06040000);       // for storage (tileset)
  vramSetBankE(VRAM_E_LCD);	// for storage (WIN_TEXT)
  vramSetBankF(VRAM_F_LCD);	// for storage (WIN_TEXT)
  BG2_CR = BG_BMP16_256x256;
  BG2_XDX = 1<<8;
  BG2_XDY = 0;
  BG2_YDX = 0;
  BG2_YDY = 1<<8;
  BG2_CY = 0;
  BG2_CX = 0;
  SUB_BG0_CR = BG_TILE_BASE(0) | BG_MAP_BASE(8) | BG_PRIORITY(0) | BG_16_COLOR;
  SUB_BG2_CR = BG_BMP16_256x256 | BG_BMP_BASE(2);
  SUB_BG2_XDX = 1<<8;
  SUB_BG2_XDY = 0;
  SUB_BG2_YDX = 0;
  SUB_BG2_YDY = 1<<8;
  SUB_BG2_CY = 0;
  SUB_BG2_CX = 0;
  
  // Enable the V-blank interrupt
  REG_IME = 0;
  IRQ_HANDLER = on_irq;
  REG_IE = IRQ_VBLANK;
  REG_IF = ~0;
  REG_DISPSTAT = DISP_VBLANK_IRQ;
  REG_IME = 1;
  
  
  register int fd;
  
  
  nds_init_fonts();
  
  //nds_raw_print("testing raw_print...\n");
  //draw_char(10,10,(byte)'N');
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  
  if (!fatInitDefault()) 
    {
      nds_fatal_err("\nError initializing FAT drivers.\n");
      nds_fatal_err("Make sure the game is patched with the correct DLDI.\n");
      nds_fatal_err(" (see http://chishm.drunkencoders.com/DLDI/ for more info).\n");
      nds_fatal_err("\n\nUnable to access filesystem.\nCannot continue.\n");
      return 1;
    }
  //nds_raw_print("filesystem loaded\n");
  
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  swiWaitForVBlank();
  
  chdir("/angband");
  if (!nds_load_kbd()) 
    {
      nds_fatal_err("\nError loading keyboard graphics.\nCannot continue.\n");
      return 1;	// die
    }
  kbd_init();
  nds_init_buttons();
  
  IPC->mailData = 0x00424242;	// to arm7: everything has init'ed
  while ((IPC->mailData & 0xFFFFFF00) != 0x42424200); 
  //wait for arm7's reply
  if (IPC->mailData & 0x00000001) 
    {	// it's a DS lite
      swap_font(false);
    } 
  else if (access("/angband/swapfont",04) != -1) 
    {
      swap_font(false);
    }
  
  use_graphics = TRUE;
  //ANGBAND_GRAF = "old";

  //NRM	initoptions();
  if (!nds_load_tiles()) 
    {
      nds_fatal_err("\n\nNo tileset could be loaded.\nCannot continue.\n");
      return 1;
    }

  if (!use_graphics) 
    {
      TILE_HEIGHT = 8;
      TILE_WIDTH = 3;
    }

  
  //NRM u.uhp = 1;	/* prevent RIP on early quits */
  //NRM u.ux = 0;	/* prevent flush_screen() */
  //NRM	init_nhwindows(0,0);
  //NRM	display_gamewindows();	// need this for askname()
#if 0
  
  nds_curs(WIN_MAP,0,30);		// put cursor offscreen
  nds_clear_nhwindow(WIN_MAP);	// somehow there is garbage in these 
  nds_clear_nhwindow(WIN_MESSAGE);// windows even after create_nhwindow
  nds_clear_nhwindow(WIN_STATUS);	// calls clear_nhwindow
  //process_options(argc, argv);
  if (!*plname)
    askname();
  if (!stricmp(plname,"wizard")) 
    wizard = 1;
  plnamesuffix();
  set_savefile_name();
  Strcpy(lock,plname);
  Strcat(lock,"-99");
  regularize(lock);
  fd = create_levelfile(0, (char *)0);
  if (fd < 0) {
    raw_print("Cannot create lock file");
  } else {
    hackpid = 1;
    write(fd, (genericonst char *_t) &hackpid, sizeof(hackpid));
    close(fd);
  }
  
  x_maze_max = COLNO-1;
  if (x_maze_max % 2)
    x_maze_max--;
  y_maze_max = ROWNO-1;
  if (y_maze_max % 2)
    y_maze_max--;
  
  vision_init();
  
  dlb_init();
  
  
  
  if ((fd = restore_saved_game()) >= 0) {
    
    pline("Restoring save file...");
    //mark_synch();	/* flush output */
    
    if(!dorecover(fd))
      goto not_recovered;
    check_special_room(FALSE);
    if (discover)
      You("are in non-scoring discovery mode.");
    
    if (discover || wizard) {
      if(yn("Do you want to keep the save file?") == 'n'){
	(void) delete_savefile();
      }
    }
    
    flags.move = 0;
  } else {
  not_recovered:
    // TODO: add code to load a game
    player_selection();
    newgame();
    if (discover)
      You("are in non-scoring discovery mode.");
    
    flags.move = 0;
    set_wear();
    (void) pickup(1);
    read_engr_at(u.ux,u.uy);
  }
  
  // it's safe to turn on the vblank intr now
  REG_IME = 1;
  
  moveloop();
  goto top;
  return 0;
  //FINISH NETHACK STUFF
#endif /* 0 */
  
  /* Activate hooks */
  plog_aux = hook_plog;
  quit_aux = hook_quit;
  
  /* Initialize the windows */
  if (init_nds()) quit("Oops!");
  
  /* XXX XXX XXX */
  ANGBAND_SYS = "nds";
  
  /* Initialize some stuff */
  init_stuff();
  
  draw_tile(2, 2, 5);
  draw_tile(4, 2, 15);
  draw_tile(6, 2, 25);
  draw_tile(8, 2, 35);

  /* About to start */
  game_start = TRUE;
  
  while (game_start)
    {
      /* Initialize */
      init_angband();

      for (i = 0; i < 50; i++)
	draw_tile(i % 10, i/10, i+600);      
      /* Wait for response */
      pause_line(Term);
      
      /* Play the game */
      play_game(new_game);
      
      /* Free resources */
      cleanup_angband();
    }
  
  /* Quit */
  quit(NULL);
  
  /* Exit */
  return (0);
}


