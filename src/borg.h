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
 * Some variables
 */

extern bool auto_ready;			/* Initialized */

extern bool auto_active;		/* Actually active */


/*
 * Other variables
 */
 
extern s32b c_t;		/* Current "time" */

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
 * State variables extracted from the screen
 */

extern int auto_depth;		/* Current dungeon "level" */

extern int auto_lev;		/* Current level */

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
 * Size of Keypress buffer
 */
#define KEY_SIZE 1024

/*
 * A Queue of keypresses to be sent
 */
extern char auto_key_queue[KEY_SIZE];
extern s16b auto_key_head;
extern s16b auto_key_tail;



/*
 * Hook -- Parse messages
 */
extern void (*borg_hook_parse)(cptr);

/*
 * Hook -- Think about the world
 */
extern void (*borg_hook_think)(void);




/*
 * Queue a keypress
 */
extern errr borg_keypress(int k);

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
 * Save the game (but do not quit)
 */
extern bool borg_save_game(void);

/*
 * Convert a stat to a stat index
 */
extern int borg_stat_index(int stat);

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

