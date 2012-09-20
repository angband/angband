/* File: borg.c */

/* Purpose: Helper file for "borg-ben.c" -BEN- */

#include "angband.h"


#ifdef ALLOW_BORG

#include "borg.h"


/*
 * See "borg-ben.c" for more information.
 *
 * This file contains various low level variables and routines.
 */




/*
 * Some variables
 */

bool auto_active = FALSE;	/* Actually active */



/*
 * Hack -- Time variables
 */

s16b c_t = 0L;			/* Current "time" */


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
 * Hack -- extra state variables
 */

int auto_feeling;	/* Current level "feeling" */

int auto_max_level;	/* Maximum player level */
int auto_max_depth;	/* Maximum dungeon depth */



/*
 * Hack -- current shop index
 */
 
s16b shop_num = -1;		/* Current shop index */



/*
 * State variables extracted from the screen
 */

int auto_depth;		/* Current dungeon "level" */

int auto_level;		/* Current level */

s32b auto_exp;		/* Current experience */

s32b auto_gold;		/* Current gold */

int auto_speed;		/* Current speed */

int auto_ac;		/* Current class */

int auto_chp;		/* Current hitpoints */
int auto_mhp;		/* Maximum hitpoints */

int auto_csp;		/* Current spell points */
int auto_msp;		/* Maximum spell points */

int auto_stat[6];	/* Current stat values */


/*
 * State variables extracted from the inventory/equipment
 */

int auto_cur_wgt;	/* Current weight */


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
 * Hack -- single character constants
 */

const char p1 = '(', p2 = ')';
const char c1 = '{', c2 = '}';
const char b1 = '[', b2 = ']';




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

    byte *aa;
    char *cc;

    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;

    /* Direct access XXX XXX */
    aa = &(Term->old->a[y][x]);
    cc = &(Term->old->c[y][x]);

    /* Access */
    t_a = aa[0];
    t_c = cc[0];

    /* Mega-Hack */
    if (!t_c) t_c = ' ';

    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Direct access to the screen */
        t_a = aa[i];
        t_c = cc[i];

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

    byte *aa;
    char *cc;
    
    /* Max length to scan for */
    int m = ABS(n);

    /* Hack -- Do not run off the screen */
    if (x + m > 80) m = 80 - x;

    /* Direct access XXX XXX */
    aa = &(Term->old->a[y][x]);
    cc = &(Term->old->c[y][x]);

    /* Access */
    t_a = aa[0];
    t_c = cc[0];

    /* Mega-Hack */
    if (!t_c) t_c = ' ';

    /* Save the attribute */
    (*a) = t_a;

    /* Scan for the rest */
    for (i = 0; i < m; i++) {

        /* Access */
        t_a = aa[i];
        t_c = cc[i];

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
    message_add(what);

#ifdef BORG_NOTE_ROWS

#ifdef GRAPHIC_MIRROR

    /* Use the "mirror" window */
    if (term_mirror) {

        static int row = 0;

        /* Use it */
        Term_activate(term_mirror);

        /* Erase current line */
        Term_erase(0, row, 80, 1);

        /* Show message */
        Term_putstr(0, row, -1, TERM_WHITE, what);

        /* Advance to next line */
        if (++row >= BORG_NOTE_ROWS) row = 0;

        /* Erase that line */
        Term_erase(0, row, 80, 1);

        /* Flush output */
        Term_fresh();

        /* Use correct window */
        Term_activate(term_screen);
    }

#endif

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
    borg_note(format("# Aborted (%s).", what));

    /* Forget borg keys */
    borg_flush();
}



/*
 * A Queue of keypresses to be sent
 */
static char *auto_key_queue;
static s16b auto_key_head;
static s16b auto_key_tail;


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(char k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Hack -- note the keypress */
    if (auto_fff) borg_info(format("& Key <%c>", k));

    /* Store the char, advance the queue */
    auto_key_queue[auto_key_head++] = k;

    /* Circular queue, handle wrap */
    if (auto_key_head == KEY_SIZE) auto_key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (auto_key_head == auto_key_tail) borg_oops("overflow");

    /* Hack -- Overflow may induce circular queue */
    if (auto_key_tail == KEY_SIZE) auto_key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypresses(cptr str)
{
    cptr s;
    
    /* Enqueue them */
    for (s = str; *s; s++) borg_keypress(*s);

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
    borg_note("# Flushing key-buffer.");

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
 * Attempt to dump a character description file
 */
bool borg_dump_character(cptr str)
{
    cptr s;

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Character description */
    borg_keypress('C');

    /* Dump character file */
    borg_keypress('f');

    /* Enter the new name */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the file name */
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
    borg_keypress('^');
    borg_keypress('S');

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return (TRUE);
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
        for (s = buf; *s && !isdigit(*s); s++) ;

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
        auto_level = atoi(buf);
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
        auto_speed = 110 + atoi(buf + 6);
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


    /* XXX XXX XXX Parse "State" */


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
 * Initialize the "borg.c" file
 */
void borg_init(void)
{
    /* Allocate the "keypress queue" */
    C_MAKE(auto_key_queue, KEY_SIZE, char);
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

