/* File: borg.h */

/* Purpose: Header file for "borg.c" -BEN- */

#ifndef INCLUDED_BORG_H
#define INCLUDED_BORG_H

#include "angband.h"



#ifdef ALLOW_BORG


/*
 * This file provides support for "borg.c".
 */



/*
 * Enable the "borg_note()" usage of the Recall Window
 * Also specify the number of "rolling rows" to use
 */
#define BORG_NOTE_ROWS		12



/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 8192



/*
 * Some variables
 */

extern bool auto_active;		/* Actually active */


/*
 * Hack -- time variables
 */

extern s16b c_t;		/* Current "time" */


/*
 * Other variables
 */
 
extern int c_x;			/* Current location (X) */
extern int c_y;			/* Current location (Y) */

extern int w_x;			/* Current panel offset (X) */
extern int w_y;			/* Current panel offset (Y) */



/*
 * State variables extracted from the screen
 */

extern bool do_weak;		/* Currently Weak */
extern bool do_hungry;		/* Currently Hungry */

extern bool do_full;		/* Currently Full */
extern bool do_gorged;		/* Currently Gorged */

extern bool do_blind;		/* Currently Blind */
extern bool do_afraid;		/* Currently Afraid */
extern bool do_confused;	/* Currently Confused */
extern bool do_poisoned;	/* Currently Poisoned */

extern bool do_cut;		/* Currently bleeding */
extern bool do_stun;		/* Currently stunned */

extern bool do_image;		/* May be hallucinating */
extern bool do_study;		/* May learn spells */

extern bool do_fix_lev;		/* Drained LEV */
extern bool do_fix_exp;		/* Drained EXP */

extern bool do_fix_stat[6];	/* Drained Stats */



/*
 * Hack -- extra state variables
 */

extern int auto_feeling;	/* Current level "feeling" */

extern int auto_max_level;	/* Maximum player level */
extern int auto_max_depth;	/* Maximum dungeon depth */



/*
 * Hack -- current shop index
 */
 
extern s16b shop_num;		/* Current shop index */



/*
 * State variables extracted from the screen
 */

extern int auto_depth;		/* Current dungeon "level" */

extern int auto_level;		/* Current level */

extern s32b auto_exp;		/* Current experience */

extern s32b auto_gold;		/* Current gold */

extern int auto_speed;		/* Current speed */

extern int auto_ac;		/* Current armor */

extern int auto_mhp;		/* Maximum hitpoints */
extern int auto_chp;		/* Current hitpoints */

extern int auto_msp;		/* Maximum spell points */
extern int auto_csp;		/* Current spell points */

extern int auto_stat[6];	/* Current stats */


/*
 * State variables extracted from the inventory/equipment
 */

extern int auto_cur_wgt;	/* Current weight */


/*
 * Constant state variables
 */
 
extern int auto_race;		/* Current race */
extern int auto_class;		/* Current class */



/*
 * Constant state structures
 */
 
extern player_race *rb_ptr;	/* Player race info */
extern player_class *cb_ptr;	/* Player class info */
extern player_magic *mb_ptr;	/* Player magic info */



/*
 * Log file
 */
extern FILE *auto_fff;		/* Log file */



/*
 * Hack -- single character constants
 */

extern const char p1, p2, c1, c2, b1, b2;


/*
 * Queue a keypress
 */
extern errr borg_keypress(char k);

/*
 * Queue several keypresses
 */
extern errr borg_keypresses(cptr str);

/*
 * Dequeue a keypress
 */
extern char borg_inkey(void);

/*
 * Flush the keypresses
 */
extern void borg_flush(void);


/*
 * Obtain some text from the screen
 */
extern errr borg_what_text(int x, int y, int n, byte *a, char *s);

/*
 * Obtain some text from the screen, accepting any (non-final) spaces
 */
extern errr borg_what_text_hack(int x, int y, int n, byte *a, char *s);


/*
 * Log a message to a file
 */
extern void borg_info(cptr what);

/*
 * Show a message to the user (and log it)
 */
extern void borg_note(cptr what);

/*
 * Show a message and turn off "auto_active"
 */
extern void borg_oops(cptr what);


/*
 * Take a "memory note"
 */
extern bool borg_tell(cptr what);

/*
 * Change the player name
 */
extern bool borg_change_name(cptr str);

/*
 * Dump a character description
 */
extern bool borg_dump_character(cptr str);

/*
 * Save the game (but do not quit)
 */
extern bool borg_save_game(void);


/*
 * Update the "frame" info from the screen
 */
extern void borg_update_frame(void);


/*
 * Initialize the "borg.c" file
 */
extern void borg_init(void);


#endif

#endif

