/* File: borg.c */

/* Purpose: an "automatic" player -BEN- */

#include "angband.h"



#ifdef ALLOW_BORG

#include "borg.h"


/*
 * This file provides general support for various "Borg" files.
 *
 * For example, see "borg-map.c" and "borg-obj.c", which are based
 * directly on this file, and "borg-ext.c" and "borg-ben.c", which
 * are based on this file and "borg-map.c" and "borg-obj.c".
 *
 * When properly combined, the result is an automatic Angband player.
 *
 * The "theory" behind the Borg is that is should be able to run as a
 * separate process, playing Angband in a window just like a human, that
 * is, examining the screen for information, and sending keypresses to
 * the game.  The current Borg does not actually do this, because it would
 * be very slow and would not run except on Unix machines, but as far as
 * possible, I have attempted to make sure that the Borg *could* run that
 * way.  This involves "cheating" as little as possible, where "cheating"
 * means accessing information not available to a normal Angband player.
 * And whenever possible, this "cheating" should be optional, that is,
 * there should be software options to disable the cheating, and, if
 * necessary, to replace it with "complex" parsing of the screen.
 *
 * Thus, the Borg COULD be written as a separate process which runs Angband
 * in a pseudo-terminal and "examines" the "screen" and sends keypresses
 * directly (as with a terminal emulator), although it would then have
 * to explicitly "wait" to make sure that the game was completely done
 * sending information.
 *
 * The Borg is thus allowed to examine the screen directly (usually by
 * an efficient direct access to "Term->scr->a" and "Term->scr->c") and
 * to send keypresses directly (via "Term_keypress()").  The Borg also
 * accesses the cursor location ("Term_locate()") and visibility (via
 * a hack involving "Term_show/hide_cursor()") directly as well.
 *
 * The Borg should not know when the game is ready for a keypress, but it
 * checks this anyway by distinguishing between the "check for keypress"
 * and "wait for keypress" hooks sent by the "term.c" package.  Otherwise
 * it would have to "pause" between turns for some amount of time to ensure
 * that the game was done processing.  It might be possible to notice when
 * the game is ready for input by some other means, but it seems likely that
 * at least some "waiting" would be necessary, unless the terminal emulation
 * program explicitly sends a "ready" sequence when ready for a keypress.
 *
 * Various other "cheats" (mostly optional) are described where they are
 * used, primarily in "borg-ben.c", for example.
 *
 * Note that this file, though it contains the "high level" function which
 * intercepts the keypress requests, does not "directly" call any other Borg
 * modules, instead, it uses two "hooks" which must be assigned by an external
 * file (such as "borg-ben.c").  This maintains the "dependency tree".
 *
 * Note that any "user input" will be ignored, and will cancel the Borg.
 */



/*
 * Hack -- make sure we have a good "ANSI" definition for "CTRL()"
 */
#undef CTRL
#define CTRL(C) ((C)&037)




/*
 * Some variables
 */
 
bool auto_ready = FALSE;	/* Initialized */

bool auto_active = FALSE;	/* Actually active */



/*
 * Hack -- Pseudo-variables
 */
 
s32b c_t = 0L;			/* Current "time" */


/*
 * Location variables
 */

int c_x = 1;			/* Current location (X) */
int c_y = 1;			/* Current location (Y) */

int w_x = 0;			/* Current panel offset (X) */
int w_y = 0;			/* Current panel offset (Y) */




/*
 * State variables extracted from the screen
 */

bool do_weak;		/* Currently Weak */
bool do_hungry;		/* Currently Hungry/Weak */

bool do_full;		/* Currently Full/Gorged */
bool do_gorged;		/* Currently Gorged */

bool do_blind;		/* Currently Blind */
bool do_afraid;		/* Currently Afraid */
bool do_confused;	/* Currently Confused */
bool do_poisoned;	/* Currently Poisoned */

bool do_cut;		/* Currently bleeding */
bool do_stun;		/* Currently stunned */

bool do_image;		/* May be hallucinating */
bool do_study;		/* May learn spells */

bool do_fix_lev;	/* Drained LEV */
bool do_fix_exp;	/* Drained EXP */

bool do_fix_stat[6];	/* Drained Stats */



/*
 * State variables extracted from the screen
 */
 
int auto_depth;		/* Current dungeon "level" */

int auto_lev;		/* Current level */

s32b auto_exp;		/* Current experience */

s32b auto_gold;		/* Current gold */

int auto_speed;		/* Current speed */

int auto_ac;		/* Current class */

int auto_chp;		/* Current hitpoints */
int auto_mhp;		/* Maximum hitpoints */

int auto_csp;		/* Current spell points */
int auto_msp;		/* Maximum spell points */

int auto_stat[6];	/* Current stats */


/*
 * Constant state variables
 */
 
int auto_race;		/* Player race */
int auto_class;		/* Player class */


/*
 * Hack -- access the class/race records
 */

player_race *rb_ptr;	/* Player race info */
player_class *cb_ptr;	/* Player class info */

player_magic *mb_ptr;	/* Player magic info */




/*
 * Log file
 */
FILE *auto_fff = NULL;		/* Log file */




/*
 * Query the "attr/chars" at a given location on the screen
 * We return "TRUE" only if a string of some form existed
 * Note that the string must be done in a single attribute.
 * We will not grab more than "ABS(n)" characters for the string.
 * If "n" is "positive", we will grab exactly "n" chars, or fail.
 * If "n" is "negative", we will grab until the attribute changes.
 * Note that "a" points to a single "attr", "s" to a string of "chars".
 *
 * Assume that the given location is actually on the screen!
 */
errr borg_what_text(int x, int y, int n, byte *a, char *s)
{
    int i;
    byte t_a;
    char t_c;

    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;


    /* Direct access to the screen */
    t_a = Term->scr->a[y][x];
    t_c = Term->scr->c[y][x];

    /* Mega-Hack */
    if (!t_c) t_c = ' ';
    
    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Direct access to the screen */
        t_a = Term->scr->a[y][x+i];
        t_c = Term->scr->c[y][x+i];

        /* Mega-Hack */
        if (!t_c) t_c = ' ';
        
        /* Verify the "attribute" (or stop) */
        if (t_a != (*a)) break;

        /* Save the first character */
        s[i] = t_c;
    }

    /* Terminate the string */
    s[i] = '\0';

    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}


/*
 * As above, but automatically convert all "blank" characters into
 * spaces of the appropriate "attr".  This includes leading blanks.
 *
 * Note that we do NOT strip final spaces, so this function will
 * very often read characters all the way to the end of the line.
 *
 * Assume that the given location is actually on the screen!
 */
errr borg_what_text_hack(int x, int y, int n, byte *a, char *s)
{
    int i;
    byte t_a;
    char t_c;

    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;


    /* Direct access to the screen */
    t_a = Term->scr->a[y][x];
    t_c = Term->scr->c[y][x];

    /* Mega-Hack */
    if (!t_c) t_c = ' ';
    
    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Direct access to the screen */
        t_a = Term->scr->a[y][x+i];
        t_c = Term->scr->c[y][x+i];

        /* Mega-Hack */
        if (!t_c) t_c = ' ';
    
        /* Hack -- Save the first usable attribute */
        if (!(*a)) (*a) = t_a;

        /* Hack -- Convert all "blanks" */
        if (t_c == ' ') t_a = (*a);

        /* Verify the "attribute" (or stop) */
        if (t_a != (*a)) break;

        /* Save the first character */
        s[i] = t_c;
    }

    /* Terminate the string */
    s[i] = '\0';

    /* Too short */
    if ((n > 0) && (i != n)) return (1);

    /* Success */
    return (0);
}




/*
 * Hack -- Dump info to a log file
 */
void borg_info(cptr what)
{
    /* Dump a log file message */
    if (auto_fff) fprintf(auto_fff, "%s\n", what);
}



/*
 * Hack -- Display (and save) a note to the user
 */
void borg_note(cptr what)
{
    /* Log the message */
    borg_info(what);

    /* Hack -- Add to the message recall */
    message_new(what, -1);

#ifdef BORG_NOTE_ROWS

    /* Mega-Hack -- use the recall window */
    if ( /* use_recall_win && */ term_recall) {

        static int row = 0;

        /* Use it */
        Term_activate(term_recall);

        /* Erase current line */
        Term_erase(0, row, 80, row);

        /* Show message */
        Term_putstr(0, row, -1, TERM_WHITE, what);

        /* Advance to next line */
        if (++row >= BORG_NOTE_ROWS) row = 0;

        /* Erase that line */
        Term_erase(0, row, 80, row);

        /* Flush output */
        Term_fresh();
        
        /* Use correct window */
        Term_activate(term_screen);
    }
    
#endif

}


/*
 * Hack -- Stop processing on errors
 */
void borg_oops(cptr what)
{
    /* No longer active */
    auto_active = 0;

    /* Give a warning */
    borg_note(format("The BORG has broken (%s).", what));
}



/*
 * A Queue of keypresses to be sent
 */
char auto_key_queue[KEY_SIZE];
s16b auto_key_head = 0;
s16b auto_key_tail = 0;


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Hack -- note the keypress */
    if (auto_fff) borg_info(format("Key: '%c'", k));

    /* Store the char, advance the queue */
    auto_key_queue[auto_key_head++] = k;

    /* Circular queue, handle wrap */
    if (auto_key_head == KEY_SIZE) auto_key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (auto_key_head == auto_key_tail) borg_oops("Overflow!");

    /* Hack -- Overflow may induce circular queue */
    if (auto_key_tail == KEY_SIZE) auto_key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Get the next Borg keypress
 */
char borg_inkey(void)
{
    int i;

    /* Nothing ready */
    if (auto_key_head == auto_key_tail) return (0);

    /* Extract the keypress, advance the queue */
    i = auto_key_queue[auto_key_tail++];

    /* Circular queue requires wrap-around */
    if (auto_key_tail == KEY_SIZE) auto_key_tail = 0;

    /* Return the key */
    return (i);
}



/*
 * Get the next Borg keypress
 */
void borg_flush(void)
{
    /* Hack -- store the keypress */
    if (auto_fff) borg_info("Flushing key-buffer.");

    /* Simply forget old keys */
    auto_key_tail = auto_key_head;
}






/*
 * Hack -- take a note later
 */
bool borg_tell(cptr what)
{
    cptr s;

    /* Hack -- self note */
    borg_keypress(':');
    for (s = what; *s; s++) borg_keypress(*s);
    borg_keypress('\n');
    
    /* Success */
    return (TRUE);
}



/*
 * Attempt to change the borg's name
 */
bool borg_change_name(cptr str)
{
    cptr s;
    
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Change the name */
    borg_keypress('c');

    /* Enter the new name */
    for (s = str; *s; s++) borg_keypress(*s);
    
    /* End the name */
    borg_keypress('\r');
    
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    
    /* Success */
    return (TRUE);
}




/*
 * Attempt to save the game
 */
bool borg_save_game(void)
{
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Save the game */
    borg_keypress(CTRL('S'));

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    
    /* Success */
    return (TRUE);
}




/*
 * Convert a stat into the 0-39 scale
 */
int borg_stat_index(int stat)
{
    int value = auto_stat[stat];
  
    /* Values: 3, 4, ..., 18 */
    if (value <= 18) return (value - 3);

    /* Ranges: 18/01-18/09, 18/10-18/19, ..., 18/90-18/99 */
    if (value <= 18+99) return (16 + (value - 18) / 10);

    /* Value: 18/100 */
    if (value == 18+100) return (26);

    /* Ranges: 18/101-18/109, 18/110-18/119, ..., 18/210-18/219 */
    if (value <= 18+219) return (27 + (value - (18+100)) / 10);

    /* Range: 18/220+ */
    return (39);
}






/*
 * Update the Borg based on the current "frame"
 *
 * Assumes the Borg is actually in the dungeon.
 */
void borg_update_frame(void)
{
    int i;
    
    byte t_a;

    char buf[160];


    /* Check for "   Town" or "  Lev 8" or " Lev 13" */
    if (0 == borg_what_text(COL_DEPTH, ROW_DEPTH, -7, &t_a, buf)) {

        cptr s;

        /* Skip the non-digits */
        for (s = buf; *s && !isdigit(*s); s++);

        /* Extract the current level */
        auto_depth = atoi(s);
    }




    /* XXX XXX XXX Title (ignore) */

    

    /* XXX XXX XXX Info (monster health) */
    


    /* Assume level is fine */
    do_fix_lev = FALSE;
    
    /* Check for drained level */
    if (0 == borg_what_text(COL_LEVEL, ROW_LEVEL, -3, &t_a, buf)) {
    
        /* Note "Lev" vs "LEV" */
        if (islower(buf[2])) do_fix_lev = TRUE;
    }

    /* Extract current level */
    if (0 == borg_what_text(COL_LEVEL + 6, ROW_LEVEL, -6, &t_a, buf)) {
    
        /* Extract "LEVEL xxxxxx" */
        auto_lev = atoi(buf);
    }


    /* Assume experience is fine */
    do_fix_exp = FALSE;
    
    /* Check for drained experience */
    if (0 == borg_what_text(COL_EXP, ROW_EXP, -3, &t_a, buf)) {

        /* Note "Exp" vs "EXP" */
        if (islower(buf[2])) do_fix_exp = TRUE;
    }

    /* Extract current experience */
    if (0 == borg_what_text(COL_EXP + 4, ROW_EXP, -8, &t_a, buf)) {

        /* Extract "EXP xxxxxxxx" */
        auto_exp = atol(buf);
    }


    /* Extract current gold */
    if (0 == borg_what_text(COL_GOLD + 3, ROW_GOLD, -9, &t_a, buf)) {
    
        /* Extract "AU xxxxxxxxx" */
        auto_gold = atol(buf);
    }


    /* Extract speed */
    if (0 == borg_what_text(COL_SPEED, ROW_SPEED, -14, &t_a, buf)) {
    
        /* Extract "Fast (+x)" or "Slow (-x)" */
        auto_speed = atoi(buf + 6);
    }
    
    /* Extract armor class */
    if (0 == borg_what_text(COL_AC + 7, ROW_AC, -5, &t_a, buf)) {
    
        /* Extract "Cur AC xxxxx" */
        auto_ac = atoi(buf);
    }


    /* Extract maximum hitpoints */
    if (0 == borg_what_text(COL_MAXHP + 7, ROW_MAXHP, -5, &t_a, buf)) {

        /* Extract "Max HP xxxxx" */
        auto_mhp = atoi(buf);
    }

    /* Extract current hitpoints */
    if (0 == borg_what_text(COL_CURHP + 7, ROW_CURHP, -5, &t_a, buf)) {

        /* Extract "Cur HP xxxxx" */
        auto_chp = atoi(buf);
    }


    /* Extract maximum spell points */
    if (0 == borg_what_text(COL_MAXSP + 7, ROW_MAXSP, -5, &t_a, buf)) {

        /* Extract "Max SP xxxxx" (or zero) */
        auto_msp = atoi(buf);
    }

    /* Extract current spell points */
    if (0 == borg_what_text(COL_CURSP + 7, ROW_CURSP, -5, &t_a, buf)) {
    
        /* Extract "Cur SP xxxxx" (or zero) */
        auto_csp = atoi(buf);
    }



    /* Clear all the "state flags" */
    do_weak = do_hungry = do_full = do_gorged = FALSE;
    do_blind = do_confused = do_afraid = do_poisoned = FALSE;
    do_cut = do_stun = do_image = do_study = FALSE;

    /* Check for hunger */
    if (0 == borg_what_text(COL_HUNGRY, ROW_HUNGRY, -1, &t_a, buf)) {

        /* Check for "Hungry" */
        if (buf[0] == 'H') do_hungry = TRUE;

        /* Check for "Weak" */
        if (buf[0] == 'W') do_weak = do_hungry = TRUE;

        /* Check for "Full" */
        if (buf[0] == 'F') do_full = TRUE;

        /* Check for "Gorged" */
        if (buf[0] == 'G') do_gorged = do_full = TRUE;
    }

    /* Check for blind */
    if (0 == borg_what_text(COL_BLIND, ROW_BLIND, -1, &t_a, buf)) {
    
        /* Check for "Blind" */
        if (buf[0] == 'B') do_blind = TRUE;
    }

    /* Check for confused */
    if (0 == borg_what_text(COL_CONFUSED, ROW_CONFUSED, -1, &t_a, buf)) {

        /* Check for "Confused" */
        if (buf[0] == 'C') do_confused = TRUE;
    }

    /* Check for afraid */
    if (0 == borg_what_text(COL_AFRAID, ROW_AFRAID, -1, &t_a, buf)) {

        /* Check for "Afraid" */
        if (buf[0] == 'A') do_afraid = TRUE;
    }

    /* Check for poisoned */
    if (0 == borg_what_text(COL_POISONED, ROW_POISONED, -1, &t_a, buf)) {

        /* Check for "Poisoned" */
        if (buf[0] == 'P') do_poisoned = TRUE;
    }


    /* XXX XXX Check for cut */
    if (0 == borg_what_text(COL_CUT, ROW_CUT, -1, &t_a, buf)) {

        /* Check for any text */
        if (isalpha(buf[0])) do_cut = TRUE;
    }

    /* XXX XXX Check for stun */
    if (0 == borg_what_text(COL_STUN, ROW_STUN, -1, &t_a, buf)) {

        /* Check for any text */
        if (isalpha(buf[0])) do_stun = TRUE;
    }


    /* XXX XXX Parse "State" */
    
    /* XXX XXX Parse "Speed" */
    

    /* Check for study */
    if (0 == borg_what_text(COL_STUDY, ROW_STUDY, -1, &t_a, buf)) {

        /* Check for "Study" */
        if (buf[0] == 'S') do_study = TRUE;
    }


    /* Parse stats */
    for (i = 0; i < 6; i++) {

        /* Check "NNN   xxxxxx" */
        if (0 == borg_what_text(COL_STAT, ROW_STAT+i, -3, &t_a, buf)) {

            /* Note "Nnn" vs "NNN" */
            do_fix_stat[i] = (islower(buf[2]));
        }
    
        /* Check "NNN   xxxxxx" */
        if (0 == borg_what_text(COL_STAT+6, ROW_STAT+i, -6, &t_a, buf)) {

            /* Parse "18/..." */
            if (buf[5] == '*') auto_stat[i] = 18 + 220;

            /* Parse "18/NNN" */
            else if (buf[2] == '/') auto_stat[i] = 18 + atoi(buf+3);

            /* Parse " 18/NN" */
            else if (buf[3] == '/') auto_stat[i] = 18 + atoi(buf+4);

            /* Parse "    NN" */
            else auto_stat[i] = atoi(buf+4);
        }
    }
}



/*
 * Hook -- Parse "messages"
 */
void (*borg_hook_parse)(cptr) = NULL;

/*
 * Hook -- Think and act
 */
void (*borg_hook_think)(void) = NULL;



/*
 * Hook -- The normal "Term_xtra()" hook
 */
static errr (*Term_xtra_hook_old)(int n, int v) = NULL;



/*
 * Our own hook.  Allow the Borg to steal the "key checker".
 */
static errr Term_xtra_borg(int n, int v)
{
    static bypass = 0;
    
    static inside = 0;


    /* Hack -- The Borg intercepts "TERM_XTRA_CHECK" */
    while (auto_active && !bypass && (n == TERM_XTRA_CHECK)) {

        errr res = 0;


        /* XXX XXX Paranoia */
        
        /* Paranoia */
        if (Term != term_screen) {
            borg_oops("Bizarre request!");
            break;
        }

        /* Paranoia -- prevent recursion */
        if (inside) {
            borg_oops("Bizarre recursion!");
            break;
        }


        /* XXX XXX Mega-Hack -- Allow User Cancel */
        
        /* Open Bypass */
        bypass++;
        
        /* Check the user */
        res = (*Term_xtra_hook_old)(TERM_XTRA_CHECK, v);

        /* Shut Bypass */
        bypass--;

        /* User input cancels the Borg */
        if (res == 0) {
            borg_oops("Cancelled by User Input!");
            Term_flush();
            break;
        }


        /* No keys */
        return (1);
    }


    /* Hack -- The Borg intercepts "TERM_XTRA_EVENT" */
    while (auto_active && !bypass && (n == TERM_XTRA_EVENT)) {

        int i, x, y;
        byte t_a;
        char buf[128];

        errr res = 0;

        bool visible;


        /* XXX XXX Paranoia */
        
        /* Paranoia */
        if (Term != term_screen) {
            borg_oops("Bizarre request!");
            break;
        }

        /* Paranoia -- prevent recursion */
        if (inside) {
            borg_oops("Bizarre recursion!");
            break;
        }


        /* XXX XXX Mega-Hack -- Allow User Cancel */
        
        /* Open Bypass */
        bypass++;
        
        /* Check the user */
        res = (*Term_xtra_hook_old)(TERM_XTRA_CHECK, v);

        /* Shut Bypass */
        bypass--;

        /* User input cancels the Borg */
        if (res == 0) {
            borg_oops("Cancelled by User Input!");
            Term_flush();
            break;
        }
                

        /* XXX XXX Mega-Hack -- Check the "cursor state" */
        
        /* Note that the cursor visibility determines whether the */
        /* game is asking us for a "command" or for some other key. */
        /* XXX XXX This requires that "hilite_player" be FALSE. */

        /* Hack -- Extract the cursor visibility */
        visible = (!Term_hide_cursor());
        if (visible) Term_show_cursor();


        /* XXX XXX XXX Mega-Hack -- Catch "-more-" messages */
        
        /* If the cursor is visible... */
        /* And the cursor is on the top line... */
        /* And there is text before the cursor... */
        /* And that text is "-more-" */
        if (visible &&
            (0 == Term_locate(&x, &y)) && (y == 0) && (x >= 6) &&
            (0 == borg_what_text(x-6, y, 6, &t_a, buf)) &&
            (streq(buf, "-more-"))) {

            int col = 0;

            /* Get each message */
            while ((0 == borg_what_text(col, 0, -80, &t_a, buf)) &&
                   (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

                /* Advance the column */
                col += strlen(buf) + 1;

                /* Parse */
                if (borg_hook_parse) (*borg_hook_parse)(buf);
            }

            /* Hack -- Clear the message */
            Term_keypress(' ');

            /* Done */
            return (0);
        }


        /* XXX XXX XXX Mega-Hack -- catch normal messages */
        
        /* If the cursor is NOT visible... */
        /* And there is text on the first line... */
        if (!visible &&
            (borg_what_text(0, 0, 1, &t_a, buf) == 0) &&
            (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

            int col = 0;

            /* Get each message */
            while ((0 == borg_what_text(col, 0, -80, &t_a, buf)) &&
                   (t_a == TERM_WHITE) && (buf[0] && (buf[0] != ' '))) {

                /* Advance the column */
                col += strlen(buf) + 1;

                /* Parse */
                if (borg_hook_parse) (*borg_hook_parse)(buf);
            }

            /* Hack -- Clear the message */
            Term_keypress(' ');

            /* Done */
            return (0);
        }


        /* XXX XXX Mega-Hack -- Allow key-queuing */
        
        /* Check for a Borg keypress */
        i = borg_inkey();

        /* Take the next keypress */
        if (i) {

            /* Enqueue the keypress */
            Term_keypress(i);

            /* Success */
            return (0);
        }


        /* XXX XXX Mega-Hack -- Allow thinking */
        
        /* Inside */
        inside++;

        /* Think */
        if (borg_hook_think) (*borg_hook_think)();

        /* Outside */
        inside--;
    }


    /* Hack -- Usually just pass the call through */
    return ((*Term_xtra_hook_old)(n, v));
}



/*
 * Initialize the "borg.c" file
 */
void borg_init(void)
{
    /* Remember the "normal" event scanner */
    Term_xtra_hook_old = Term->xtra_hook;

    /* Cheat -- drop a hook into the "event scanner" */
    Term->xtra_hook = Term_xtra_borg;
}







#endif

