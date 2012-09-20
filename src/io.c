/* File: io.c */

/* Purpose: mid-level I/O (uses term.c) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "TERM_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, TERM_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * The following info is from "Torbjorn Lindgren" (see "main-xaw.c").
 *
 * These values are NOT gamma-corrected.  On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7.  The converted value for each of the five
 * different "quarter" values is given below:
 *
 *  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
 *  -----       ----            ----            ----          ---
 *   0/4        0.00            0.00            0.00          #00
 *   1/4        0.25            0.27            0.28          #47
 *   2/4        0.50            0.55            0.56          #8f
 *   3/4        0.75            0.82            0.84          #d7
 *   4/4        1.00            1.00            1.00          #ff
 *
 * Note that some machines (i.e. most IBM machines) are limited to a
 * hard-coded set of colors, and so the information above is useless.
 *
 * Also, some machines are limited to a pre-determined set of colors,
 * for example, the IBM can only display 16 colors, and only 14 of
 * those colors resemble colors used by Angband, and then only when
 * you ignore the fact that "Slate" and "cyan" are not really matches,
 * so on the IBM, we use "orange" for both "Umber", and "Light Umber"
 * in addition to the obvious "Orange", since by combining all of the
 * "indeterminate" colors into a single color, the rest of the colors
 * are left with "meaningful" values.
 */


#include "angband.h"




/*
 * Move the cursor
 */
void move_cursor(int row, int col)
{
    Term_gotoxy(col, row);
}



/*
 * Convert a decimal to a single digit octal number
 */
static char octify(uint i)
{
    return (hexsym[i%8]);
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(uint i)
{
    return (hexsym[i%16]);
}


/*
 * Convert a octal-digit into a decimal
 */
static int deoct(char c)
{
    if (isdigit(c)) return (D2I(c));
    return (0);
}

/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
    if (isdigit(c)) return (D2I(c));
    if (islower(c)) return (A2I(c) + 10);
    if (isupper(c)) return (A2I(tolower(c)) + 10);
    return (0);
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * I have no clue if this function correctly handles, for example,
 * parsing "\xFF" into a (signed) char.  Whoever thought of making
 * the "sign" of a "char" undefined is a complete moron.  Oh well.
 */
void text_to_ascii(char *buf, cptr str)
{
    char *s = buf;

    /* Analyze the "ascii" string */
    while (*str) {

        /* Backslash codes */
        if (*str == '\\') {

            /* Skip the backslash */
            str++;

            /* Hex-mode XXX */
            if (*str == 'x') {
                *s = 16 * dehex(*++str);
                *s++ += dehex(*++str);
            }

            /* Hack -- simple way to specify "backslash" */
            else if (*str == '\\') {
                *s++ = '\\';
            }

            /* Hack -- simple way to specify "caret" */
            else if (*str == '^') {
                *s++ = '^';
            }

            /* Hack -- simple way to specify "space" */
            else if (*str == 's') {
                *s++ = ' ';
            }

            /* Hack -- simple way to specify Escape */
            else if (*str == 'e') {
                *s++ = ESCAPE;
            }

            /* Backspace */
            else if (*str == 'b') {
                *s++ = '\b';
            }

            /* Newline */
            else if (*str == 'n') {
                *s++ = '\n';
            }

            /* Return */
            else if (*str == 'r') {
                *s++ = '\r';
            }

            /* Tab */
            else if (*str == 't') {
                *s++ = '\t';
            }

            /* Octal-mode */
            else if (*str == '0') {
                *s = 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '1') {
                *s = 64 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '2') {
                *s = 64 * 2 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Octal-mode */
            else if (*str == '3') {
                *s = 64 * 3 + 8 * deoct(*++str);
                *s++ += deoct(*++str);
            }

            /* Skip the final char */
            str++;
        }

        /* Normal Control codes */
        else if (*str == '^') {
            str++;
            *s++ = (*str++ & 037);
        }

        /* Normal chars */
        else {
            *s++ = *str++;
        }
    }

    /* Terminate */
    *s = '\0';
}


/*
 * Hack -- convert a string into a printable form
 */
void ascii_to_text(char *buf, cptr str)
{
    char *s = buf;

    /* Analyze the "ascii" string */
    while (*str) {

        byte i = (byte)(*str++);

        if (i == ESCAPE) {
            *s++ = '\\';
            *s++ = 'e';
        }
        else if (i == ' ') {
            *s++ = '\\';
            *s++ = 's';
        }
        else if (i == '\b') {
            *s++ = '\\';
            *s++ = 'b';
        }
        else if (i == '\t') {
            *s++ = '\\';
            *s++ = 't';
        }
        else if (i == '\n') {
            *s++ = '\\';
            *s++ = 'n';
        }
        else if (i == '\r') {
            *s++ = '\\';
            *s++ = 'r';
        }
        else if (i == '^') {
            *s++ = '\\';
            *s++ = '^';
        }
        else if (i == '\\') {
            *s++ = '\\';
            *s++ = '\\';
        }
        else if (i < 32) {
            *s++ = '^';
            *s++ = i + 64;
        }
        else if (i < 127) {
            *s++ = i;
        }
        else if (i < 64) {
            *s++ = '\\';
            *s++ = '0';
            *s++ = octify(i / 8);
            *s++ = octify(i % 8);
        }
        else {
            *s++ = '\\';
            *s++ = 'x';
            *s++ = hexify(i / 16);
            *s++ = hexify(i % 16);
        }
    }

    /* Terminate */
    *s = '\0';
}



/*
 * Variable used by the functions below
 */
static int hack_dir = 0;


/*
 * Convert a "Rogue" keypress into an "Angband" keypress
 * Pass extra information as needed via "hack_dir"
 *
 * Note that many "Rogue" keypresses encode a direction.
 */
static char roguelike_commands(char command)
{
    char b1 = '[', b2 = ']';

    /* Process the command */
    switch (command) {

        /* Movement (rogue keys) */
        case 'b': hack_dir = 1; return (';');
        case 'j': hack_dir = 2; return (';');
        case 'n': hack_dir = 3; return (';');
        case 'h': hack_dir = 4; return (';');
        case 'l': hack_dir = 6; return (';');
        case 'y': hack_dir = 7; return (';');
        case 'k': hack_dir = 8; return (';');
        case 'u': hack_dir = 9; return (';');

        /* Running (shift + rogue keys) */
        case 'B': hack_dir = 1; return ('.');
        case 'J': hack_dir = 2; return ('.');
        case 'N': hack_dir = 3; return ('.');
        case 'H': hack_dir = 4; return ('.');
        case 'L': hack_dir = 6; return ('.');
        case 'Y': hack_dir = 7; return ('.');
        case 'K': hack_dir = 8; return ('.');
        case 'U': hack_dir = 9; return ('.');

        /* Tunnelling (control + rogue keys) */
        case KTRL('B'): hack_dir = 1; return ('+');
        case KTRL('J'): hack_dir = 2; return ('+');
        case KTRL('N'): hack_dir = 3; return ('+');
        case KTRL('H'): hack_dir = 4; return ('+');
        case KTRL('L'): hack_dir = 6; return ('+');
        case KTRL('Y'): hack_dir = 7; return ('+');
        case KTRL('K'): hack_dir = 8; return ('+');
        case KTRL('U'): hack_dir = 9; return ('+');

        /* Hack -- KTRL('M') == return == linefeed == KTRL('J') */
        case KTRL('M'): hack_dir = 2; return ('+');

        /* Hack -- KTRL('I') == tab == white space == space */
        case KTRL('I'): return (' ');

        /* Allow use of the "destroy" command */
        case KTRL('D'): return ('k');

        /* Hack -- Commit suicide */
        case KTRL('C'): return ('Q');

        /* Locate player on map */
        case 'W': return ('L');

        /* Browse a book (Peruse) */
        case 'P': return ('b');

        /* Jam a door (Spike) */
        case 'S': return ('j');

        /* Toggle search mode */
        case '#': return ('S');

        /* Use a staff (Zap) */
        case 'Z': return ('u');

        /* Wear/Wield equipment */
        case 'w': return (b1);

        /* Take off equipment */
        case 'T': return (b2);

        /* Fire an item */
        case 't': return ('f');

        /* Bash a door (Force) */
        case 'f': return ('B');

        /* Look around (examine) */
        case 'x': return ('l');

        /* Aim a wand (Zap) */
        case 'z': return ('a');

        /* Zap a rod (Activate) */
        case 'a': return ('z');

        /* Run */
        case ',': return ('.');

        /* Stay still (fake direction) */
        case '.': hack_dir = 5; return (',');

        /* Stay still (fake direction) */
        case '5': hack_dir = 5; return (',');

        /* Standard walking */
        case '1': hack_dir = 1; return (';');
        case '2': hack_dir = 2; return (';');
        case '3': hack_dir = 3; return (';');
        case '4': hack_dir = 4; return (';');
        case '6': hack_dir = 6; return (';');
        case '7': hack_dir = 7; return (';');
        case '8': hack_dir = 8; return (';');
        case '9': hack_dir = 9; return (';');
    }

    /* Default */
    return (command);
}


/*
 * Convert an "Original" keypress into an "Angband" keypress
 * Pass direction information back via "hack_dir".
 *
 * Note that "Original" and "Angband" are very similar.
 */
static char original_commands(char command)
{
    char b1 = '[', b2 = ']';

    /* Process the command */
    switch (command) {

        /* White space */
        case KTRL('I'): return (' ');
        case KTRL('J'): return (' ');
        case KTRL('M'): return (' ');

        /* Wield */
        case 'w': return (b1);

        /* Take off */
        case 't': return (b2);

        /* Tunnel */
        case 'T': return ('+');

        /* Run */
        case '.': return ('.');

        /* Stay still (fake direction) */
        case ',': hack_dir = 5; return (',');

        /* Stay still (fake direction) */
        case '5': hack_dir = 5; return (',');

        /* Standard walking */
        case '1': hack_dir = 1; return (';');
        case '2': hack_dir = 2; return (';');
        case '3': hack_dir = 3; return (';');
        case '4': hack_dir = 4; return (';');
        case '6': hack_dir = 6; return (';');
        case '7': hack_dir = 7; return (';');
        case '8': hack_dir = 8; return (';');
        case '9': hack_dir = 9; return (';');

        /* Hack -- Commit suicide */
        case KTRL('K'): return ('Q');
        case KTRL('C'): return ('Q');
    }

    /* Default */
    return (command);
}


/*
 * React to new value of "rogue_like_commands".
 *
 * Initialize the "keymap" arrays based on the current value of
 * "rogue_like_commands".  Note that all "undefined" keypresses
 * by default map to themselves with no direction.  This allows
 * "standard" commands to use the same keys in both keysets.
 *
 * To reset the keymap, simply set "rogue_like_commands" to -1,
 * call this function, restore its value, call this function.
 *
 * The keymap arrays map keys to "command_cmd" and "command_dir".
 *
 * It is illegal for keymap_cmds[N] to be zero, except perhaps for
 * keymaps_cmds[0], which is unused.  Use "space" to "cancel" a key.
 */
void keymap_init(void)
{
    int i, k;

    /* Notice changes in the "rogue_like_commands" flag */
    static old_rogue_like = -1;

    /* Hack -- notice changes in "rogue_like_commands" */
    if (old_rogue_like == rogue_like_commands) return;

    /* Initialize every entry */
    for (i = 0; i < 256; i++) {

        /* Default to "no direction" */
        hack_dir = 0;

        /* Attempt to translate */
        if (rogue_like_commands) {
            k = roguelike_commands(i);
        }
        else {
            k = original_commands(i);
        }

        /* Save the keypress */
        keymap_cmds[i] = k;

        /* Save the direction */
        keymap_dirs[i] = hack_dir;
    }

    /* Save the "rogue_like_commands" setting */
    old_rogue_like = rogue_like_commands;
}








/*
 * Legal bit-flags for macro__use[X]
 */
#define MACRO_USE_CMD	0x01	/* X triggers a command macro */
#define MACRO_USE_STD	0x02	/* X triggers a standard macro */

/*
 * Fast check for trigger of any macros
 */
static byte macro__use[256];



/*
 * Hack -- add a macro definition (or redefinition).
 *
 * If "cmd_flag" is set then this macro is only active when
 * the user is being asked for a command (see below).
 */
void macro_add(cptr pat, cptr act, bool cmd_flag)
{
    int n;


    /* Paranoia -- require data */
    if (!pat || !act) return;


    /* Look for a re-usable slot */
    for (n = 0; n < macro__num; n++) {

        /* Notice macro redefinition */
        if (streq(macro__pat[n], pat)) {

            /* Free the old macro action */
            string_free(macro__act[n]);

            /* Save the macro action */
            macro__act[n] = string_make(act);

            /* Save the "cmd_flag" */
            macro__cmd[n] = cmd_flag;

            /* All done */
            return;
        }
    }


    /* Save the pattern */
    macro__pat[macro__num] = string_make(pat);

    /* Save the macro action */
    macro__act[macro__num] = string_make(act);

    /* Save the "cmd_flag" */
    macro__cmd[macro__num] = cmd_flag;

    /* One more macro */
    macro__num++;


    /* Hack -- Note the "trigger" char */
    macro__use[(byte)(pat[0])] |= MACRO_USE_STD;

    /* Hack -- Note the "trigger" char of command macros */
    if (cmd_flag) macro__use[(byte)(pat[0])] |= MACRO_USE_CMD;
}



/*
 * Check for possibly pending macros
 */
static int macro_maybe(cptr buf, int n)
{
    int i;

    /* Scan the macros */
    for (i = n; i < macro__num; i++) {

        /* Skip inactive macros */
        if (macro__cmd[i] && !inkey_flag) continue;

        /* Check for "prefix" */
        if (prefix(macro__pat[i], buf)) {

            /* Ignore complete macros */
            if (!streq(macro__pat[i], buf)) return (i);
        }
    }

    /* No matches */
    return (-1);
}


/*
 * Find the longest completed macro
 */
static int macro_ready(cptr buf)
{
    int i, t, n = -1, s = -1;

    /* Scan the macros */
    for (i = 0; i < macro__num; i++) {

        /* Skip inactive macros */
        if (macro__cmd[i] && !inkey_flag) continue;

        /* Check for "prefix" */
        if (!prefix(buf, macro__pat[i])) continue;

        /* Check the length of this entry */
        t = strlen(macro__pat[i]);

        /* Find the "longest" entry */
        if ((n >= 0) && (s > t)) continue;

        /* Track the entry */
        n = i;
        s = t;
    }

    /* Return the result */
    return (n);
}



/*
 * Local "need flush" variable
 */
static bool flush_later = FALSE;


/*
 * Local variable -- we just finished a macro action
 */
static bool after_macro = FALSE;

/*
 * Local variable -- we are inside a macro action
 */
static bool parse_macro = FALSE;

/*
 * Local variable -- we are inside a "control-underscore" sequence
 */
static bool parse_under = FALSE;

/*
 * Local variable -- we are inside a "control-backslash" sequence
 */
static bool parse_slash = FALSE;

/*
 * Local variable -- we are stripping symbols for a while
 */
static bool strip_chars = FALSE;



/*
 * Flush all input chars.  Actually, remember the flush,
 * and do a "special flush" before the next "inkey()".
 *
 * This is not only more efficient, but also necessary to make sure
 * that various "inkey()" codes are not "lost" along the way.
 */
void flush(void)
{
    /* Do it later */
    flush_later = TRUE;
}


/*
 * Flush the screen, make a noise
 */
void bell()
{
    /* Mega-Hack -- Flush the output */
    Term_fresh();

    /* Make a bell noise (if allowed) */
    if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

    /* Flush the input (later!) */
    flush();
}


/*
 * Mega-Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
    /* Make a sound (if allowed) */
    if (use_sound) Term_xtra(TERM_XTRA_SOUND, val);
}




/*
 * Helper function called only from "inkey()"
 *
 * This function does most of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros,
 * as well as "extra" keys read in while choosing a macro, and the
 * actual action for the macro.
 *
 * Embedded macros are illegal, although "clever" use of special
 * control chars may bypass this restriction.  Be very careful.
 *
 * The user only gets 500 (1+2+...+29+30) milliseconds for the macro.
 *
 * Note the annoying special processing to "correctly" handle the
 * special "control-backslash" codes following a "control-underscore"
 * macro sequence.  See "main-x11.c" and "main-xaw.c" for details.
 */
static char inkey_aux(void)
{
    int		k = 0, n, p = 0, w = 0;

    char	ch;
    
    cptr	pat, act;

    char	buf[1024];


    /* Wait for a keypress */
    (void)Term_inkey(&ch, TRUE, TRUE);
    
    /* Hack -- Allow "raw" mode */
    if (inkey_base) return (ch);


    /* End of internal macro */
    if (ch == 29) parse_macro = FALSE;


    /* Do not check "ascii 28" */
    if (ch == 28) return (ch);

    /* Do not check "ascii 29" */
    if (ch == 29) return (ch);


    /* Do not check macro actions */
    if (parse_macro) return (ch);

    /* Do not check "control-underscore" sequences */
    if (parse_under) return (ch);

    /* Do not check "control-backslash" sequences */
    if (parse_slash) return (ch);


    /* Efficiency -- Ignore impossible macros */
    if (!macro__use[(byte)(ch)]) return (ch);

    /* Efficiency -- Ignore inactive macros */
    if (!inkey_flag && (macro__use[(byte)(ch)] == MACRO_USE_CMD)) return (ch);


    /* Save the first key, advance */
    buf[p++] = ch;
    buf[p] = '\0';


    /* Wait for a macro, or a timeout */
    while (TRUE) {

        /* Check for possible macros */
        k = macro_maybe(buf, k);

        /* Nothing matches */
        if (k < 0) break;

        /* Check for (and remove) a pending key */
        if (0 == Term_inkey(&ch, FALSE, TRUE)) {

            /* Append the key */
            buf[p++] = ch;
            buf[p] = '\0';

            /* Restart wait */
            w = 0;	
        }

        /* No key ready */
        else {

            /* Increase the wait */
            if (w > 30) break;

            /* Hack -- delay */
            delay(++w);
        }
    }


    /* Check for a successful macro */
    k = macro_ready(buf);

    /* No macro available */
    if (k < 0) {

        /* Push all the keys back on the queue */
        while (p > 0) {

            /* Push the key, notice over-flow */
            if (Term_key_push(buf[--p])) return (0);
        }

        /* Wait for (and remove) a pending key */
        (void)Term_inkey(&ch, TRUE, TRUE);
        
        /* Return the key */
        return (ch);
    }


    /* Access the macro pattern */
    pat = macro__pat[k];

    /* Get the length of the pattern */
    n = strlen(pat);

    /* Push the "extra" keys back on the queue */
    while (p > n) {

        /* Push the key, notice over-flow */
        if (Term_key_push(buf[--p])) return (0);
    }


    /* We are now inside a macro */
    parse_macro = TRUE;

    /* Push the "macro complete" key */
    if (Term_key_push(29)) return (0);


    /* Access the macro action */
    act = macro__act[k];

    /* Get the length of the action */
    n = strlen(act);

    /* Push the macro "action" onto the key queue */
    while (n > 0) {

        /* Push the key, notice over-flow */
        if (Term_key_push(act[--n])) return (0);
    }


    /* Force "inkey()" to call us again */
    return (0);
}




/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * Normally, this function will process "macros", but if "inkey_base" is
 * TRUE, then we will bypass all "macro" processing.  This allows direct
 * usage of the "Term_inkey()" function.
 *
 * Normally, this function will do something, but if "inkey_xtra" is TRUE,
 * then something else will happen.
 *
 * Normally, this function will wait until a "real" key is ready, but if
 * "inkey_scan" is TRUE, then we will return zero if no keys are ready.
 *
 * Normally, this function will show the cursor, and will process all normal
 * macros, but if "inkey_flag" is TRUE, then we will only show the cursor if
 * "hilite_player" is TRUE, and also, we will only process "command" macros.
 *
 * Note that the "flush()" function does not actually flush the input queue,
 * but waits until "inkey()" is called to perform the "flush".
 *
 * Refresh the screen if waiting for a keypress and no key is ready.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of a few "special" control-keys, which
 * are reserved to simplify the use of various "main-xxx.c" files,
 * or used by the "macro" code above.
 *
 * Ascii 27 is "control left bracket" -- normal "Escape" key
 * Ascii 28 is "control backslash" -- special macro delimiter
 * Ascii 29 is "control right bracket" -- end of macro action
 * Ascii 30 is "control caret" -- indicates "keypad" key
 * Ascii 31 is "control underscore" -- begin macro-trigger
 *
 * Hack -- Make sure to allow calls to "inkey()" even if "term_screen"
 * is not the active Term, this allows the various "main-xxx.c" files
 * to only handle input when "term_screen" is "active".
 */
char inkey(void)
{
    int v;

    char ch;
    
    bool fix = FALSE;

    bool done = FALSE;

    term *old = Term;
    

    /* Hack -- handle delayed "flush()" */
    if (flush_later) {

        /* Done */
        flush_later = FALSE;

        /* Forget old keypresses */
        Term_flush();

        /* Cancel "macro" info */
        parse_macro = after_macro = FALSE;

        /* Cancel "sequence" info */
        parse_under = parse_slash = FALSE;

        /* Cancel "strip" mode */
        strip_chars = FALSE;
    }


    /* Show the cursor (usually) if waiting */
    if (!inkey_scan && (!inkey_flag || hilite_player)) {

        /* Successful showing requires fixing later */
        if (0 == Term_show_cursor()) fix = TRUE;
    }

    
    /* Hack -- Activate the screen */
    Term_activate(term_screen);
    
    
    /* Get a (non-zero) keypress */
    for (ch = 0; !ch; ) {


        /* Hack -- if not waiting, an no key ready, break */
        if (inkey_scan && (0 != Term_inkey(&ch, FALSE, FALSE))) break;


        /* Hack -- flush output once when no key ready */
        if (!done && (0 != Term_inkey(&ch, FALSE, FALSE))) {
    
            /* Hack -- activate proper term */
            Term_activate(old);
 
            /* Flush output */        
            Term_fresh();

            /* Hack -- activate the screen */
            Term_activate(term_screen);

            /* Mega-Hack -- reset signal counter */
            signal_count = 0;

            /* Only once */
            done = TRUE;
        }


        /* Get a key (see above) */
        v = ch = inkey_aux();

        /* Mega-Hack -- raw mode */
        if (inkey_base && ch) break;
        

        /* Finished a "control-underscore" sequence */
        if (parse_under && (ch <= 32)) {

            /* Found the edge */
            parse_under = FALSE;

            /* Stop stripping */
            strip_chars = FALSE;

            /* Strip this key */
            ch = 0;
        }


        /* Finished a "control-backslash" sequence */
        if (parse_slash && (ch == 28)) {

            /* Found the edge */
            parse_slash = FALSE;

            /* Stop stripping */
            strip_chars = FALSE;

            /* Strip this key */
            ch = 0;
        }


        /* Handle some special keys */
        switch (ch) {

            /* Hack -- convert back-quote into escape */
            case '`':

                /* Convert to "Escape" */
                ch = ESCAPE;

                /* Done */
                break;

            /* Hack -- strip "control-right-bracket" end-of-macro-action */
            case 29:

                /* Strip this key */
                ch = 0;

                /* Done */
                break;

            /* Hack -- strip "control-caret" special-keypad-indicator */
            case 30:

                /* Strip this key */
                ch = 0;

                /* Done */
                break;

            /* Hack -- strip "control-underscore" special-macro-triggers */
            case 31:

                /* Strip this key */
                ch = 0;

                /* Inside a "underscore" sequence */
                parse_under = TRUE;

                /* Strip chars (always) */
                strip_chars = TRUE;

                /* Done */
                break;

            /* Hack -- strip "control-backslash" special-fallback-strings */
            case 28:

                /* Strip this key */
                ch = 0;

                /* Inside a "control-backslash" sequence */
                parse_slash = TRUE;

                /* Strip chars (sometimes) */
                strip_chars = after_macro;

                /* Done */
                break;
        }


        /* Hack -- Set "after_macro" code */
        after_macro = ((v == 29) ? TRUE : FALSE);


        /* Hack -- strip chars */
        if (strip_chars) ch = 0;
    }


    /* Hack -- restore the term */
    Term_activate(old);
    

    /* Fix the cursor if necessary */
    if (fix) Term_hide_cursor();


    /* Cancel the various "global parameters" */
    inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;


    /* Return the keypress */
    return (ch);
}




/*
 * We use a global array for all inscriptions to reduce the memory
 * spent maintaining inscriptions.  Of course, it is still possible
 * to run out of inscription memory, especially if too many different
 * inscriptions are used, but hopefully this will be rare.
 *
 * We use dynamic string allocation because otherwise it is necessary
 * to pre-guess the amount of quark activity.  We limit the total
 * number of quarks, but this is much easier to "expand" as needed.
 *
 * Any two items with the same inscription will have the same "quark"
 * index, which should greatly reduce the need for inscription space.
 *
 * Note that "quark zero" is NULL and should not be "dereferenced".
 */

/*
 * Add a new "quark" to the set of quarks.
 */
s16b quark_add(cptr str)
{
    int i;

    /* Look for an existing quark */
    for (i = 1; i < quark__num; i++) {

        /* Check for equality */
        if (streq(quark__str[i], str)) return (i);
    }

    /* Paranoia -- Require room */
    if (quark__num == QUARK_MAX) return (0);

    /* Add a new quark */
    quark__str[i] = string_make(str);

    /* Count the quarks */
    quark__num++;

    /* Return the index */
    return (i);
}


/*
 * This function looks up a quark
 */
cptr quark_str(s16b i)
{
    cptr q;

    /* Verify */
    if ((i < 0) || (i >= quark__num)) i = 0;

    /* Access the quark */
    q = quark__str[i];

    /* Return the quark */
    return (q);
}




/*
 * Second try for the "message" handling routines.
 *
 * Each call to "message_add(s)" will add a new "most recent" message
 * to the "message recall list", using the contents of the string "s".
 *
 * The messages will be stored in such a way as to maximize "efficiency",
 * that is, the number of sequential messages that can be retrieved, given
 * a limited amount of space in which to store them.
 *
 * We keep a buffer of chars to hold the "text" of the messages, not
 * necessarily in "order", and an array of offsets into that buffer,
 * representing the actual messages.  This is made more complicated
 * by the fact that both the array of indexes, and the buffer itself,
 * are both treated as "circular arrays" for efficiency purposes, but
 * the strings may not be "broken" across the ends of the array.
 *
 * The "message_add()" function is rather "complex", because it must be
 * extremely efficient, both in space and time, for use with the Borg.
 */



/*
 * How many messages are "available"?
 */
s16b message_num(void)
{
    int last, next, n;

    /* Extract the indexes */
    last = message__last;
    next = message__next;

    /* Handle "wrap" */
    if (next < last) next += MESSAGE_MAX;

    /* Extract the space */
    n = (next - last);

    /* Return the result */
    return (n);
}



/*
 * Recall the "text" of a saved message
 */
cptr message_str(s16b age)
{
    s16b x;
    s16b o;
    cptr s;

    /* Forgotten messages have no text */
    if ((age < 0) || (age >= message_num())) return ("");

    /* Acquire the "logical" index */
    x = (message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX;

    /* Get the "offset" for the message */
    o = message__ptr[x];

    /* Access the message text */
    s = &message__buf[o];

    /* Return the message text */
    return (s);
}



/*
 * Add a new message, with great efficiency
 */
void message_add(cptr str)
{
    int i, k, x, n;


    /*** Step 1 -- Analyze the message ***/

    /* Hack -- Ignore "non-messages" */
    if (!str) return;

    /* Message length */
    n = strlen(str);

    /* Important Hack -- Ignore "long" messages */
    if (n >= MESSAGE_BUF / 4) return;


    /*** Step 2 -- Attempt to optimize ***/

    /* Limit number of messages to check */
    k = message_num() / 4;

    /* Limit number of messages to check */
    if (k > MESSAGE_MAX / 32) k = MESSAGE_MAX / 32;

    /* Check the last few messages (if any to count) */
    for (i = message__next; k; k--) {

        u16b q;

        cptr old;

        /* Back up and wrap if needed */
        if (i-- == 0) i = MESSAGE_MAX - 1;

        /* Stop before oldest message */
        if (i == message__last) break;

        /* Extract "distance" from "head" */
        q = (message__head + MESSAGE_BUF - message__ptr[i]) % MESSAGE_BUF;

        /* Do not optimize over large distance */
        if (q > MESSAGE_BUF / 2) continue;

        /* Access the old string */
        old = &message__buf[message__ptr[i]];

        /* Compare */
        if (!streq(old, str)) continue;

        /* Get the next message index, advance */
        x = message__next++;

        /* Handle wrap */
        if (message__next == MESSAGE_MAX) message__next = 0;

        /* Kill last message if needed */
        if (message__next == message__last) message__last++;

        /* Handle wrap */
        if (message__last == MESSAGE_MAX) message__last = 0;

        /* Assign the starting address */
        message__ptr[x] = message__ptr[i];

        /* Success */
        return;
    }


    /*** Step 3 -- Ensure space before end of buffer ***/

    /* Kill messages and Wrap if needed */
    if (message__head + n + 1 >= MESSAGE_BUF) {

        /* Kill all "dead" messages */
        for (i = message__last; TRUE; i++) {

            /* Wrap if needed */
            if (i == MESSAGE_MAX) i = 0;

            /* Stop before the new message */
            if (i == message__next) break;

            /* Kill "dead" messages */
            if (message__ptr[i] >= message__head) {

                /* Track oldest message */
                message__last = i + 1;
            }
        }

        /* Wrap "tail" if needed */
        if (message__tail >= message__head) message__tail = 0;

        /* Start over */
        message__head = 0;
    }


    /*** Step 4 -- Ensure space before next message ***/

    /* Kill messages if needed */
    if (message__head + n + 1 > message__tail) {

        /* Grab new "tail" */
        message__tail = message__head + n + 1;

        /* Advance tail while possible past first "nul" */
        while (message__buf[message__tail-1]) message__tail++;

        /* Kill all "dead" messages */
        for (i = message__last; TRUE; i++) {

            /* Wrap if needed */
            if (i == MESSAGE_MAX) i = 0;

            /* Stop before the new message */
            if (i == message__next) break;

            /* Kill "dead" messages */
            if ((message__ptr[i] >= message__head) &&
                (message__ptr[i] < message__tail)) {

                /* Track oldest message */
                message__last = i + 1;
            }
        }
    }


    /*** Step 5 -- Grab a new message index ***/

    /* Get the next message index, advance */
    x = message__next++;

    /* Handle wrap */
    if (message__next == MESSAGE_MAX) message__next = 0;

    /* Kill last message if needed */
    if (message__next == message__last) message__last++;

    /* Handle wrap */
    if (message__last == MESSAGE_MAX) message__last = 0;



    /*** Step 6 -- Insert the message text ***/

    /* Assign the starting address */
    message__ptr[x] = message__head;

    /* Append the new part of the message */
    for (i = 0; i < n; i++) {

        /* Copy the message */
        message__buf[message__head + i] = str[i];
    }

    /* Terminate */
    message__buf[message__head + i] = '\0';

    /* Advance the "head" pointer */
    message__head += n + 1;
}



/*
 * Output a (short) message to the top line of the screen.
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are kept for later reference (see above).
 *
 * We could do "Term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to
 * "erase" any "pending" messages still on the screen.
 *
 * Be sure to use "msg_format()" for long, or formatted, messages.
 *
 * Note the "filch_message" option which explicitly forces every
 * message onto a line of its own.  This is useful for the Borg,
 * and also for people who want maximal feedback.  There is another
 * similar option, "filch_disturb", see the "disturb()" function.
 */
void msg_print(cptr msg)
{
    int len = (msg ? strlen(msg) : 0);

    static pos = 0;


    /* Clear known messages */
    if (!msg_flag) {

        /* Clear the line */
        Term_erase(0, 0, 80, 1);

        /* Reset the cursor */
        pos = 0;
    }


    /* Hack -- flush when requested or needed */
    if (pos && (!msg || filch_message || ((pos + len) > 72))) {

        byte a = TERM_WHITE;

        /* Hack -- stay on screen */
        if (pos > 73) pos = 73;

#ifdef USE_COLOR
        /* Use light blue */
        if (use_color) a = TERM_L_BLUE;
#endif

        /* Pause for response */
        Term_putstr(pos, 0, -1, a, "-more-");

        /* Get an acceptable keypress */
        while (1) {
            int cmd = inkey();
            if ((quick_messages) || (cmd == ESCAPE)) break;
            if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) break;
            bell();
        }

        /* Clear the line */
        Term_erase(0, 0, 80, 1);

        /* Forget it */
        msg_flag = FALSE;

        /* Reset cursor */
        pos = 0;
    }


    /* No message */
    if (!msg) return;

    /* Display the new message */
    Term_putstr(pos, 0, len, TERM_WHITE, msg);

    /* Remember the message */
    msg_flag = TRUE;

    /* Skip a space before next message */
    pos += len + 1;


    /* Hack -- Message recall not ready */
    if (!character_generated) return;

    /* Memorize the message */
    message_add(msg);
}



/*
 * Display a (formatted) message, using "vstrnfmt()" and "msg_print()".
 *
 * Break the message into pieces (40-70 chars) if needed.
 */
void msg_format(cptr fmt, ...)
{
    va_list vp;

    int n;

    char *t;

    char buf[1024];


    /* Begin the Varargs Stuff */
    va_start(vp, fmt);

    /* Format the args, save the length */
    n = vstrnfmt(buf, 1024, fmt, vp);

    /* End the Varargs Stuff */
    va_end(vp);


    /* Analyze the buffer */
    t = buf;

    /* Split if "required" */
    while (n > 71) {

        int check, split;

        /* Assume no split */
        split = -1;

        /* Find the (farthest) "useful" split point */
        for (check = 40; check < 70; check++) {

            /* Found a valid split point */
            if (t[check] == ' ') split = check;
        }

        /* Hack -- could not split! */
        if (split < 0) break;

        /* Split the message, advance the split point */
        t[split++] = '\0';

        /* Print the first part */
        msg_print(t);

        /* Prepare to recurse on the rest of "buf" */
        t += split; n -= split;

        /* Mega-Hack -- indent subsequent lines two spaces */
        *--t = ' '; n++;
        *--t = ' '; n++;
    }

    /* Print the whole thing (or remains of the split) */
    msg_print(t);
}



/*
 * Erase the screen
 */
void clear_screen(void)
{
    /* Hack -- flush old messages */
    msg_print(NULL);

    /* Clear the screen */
    Term_clear();
}


/*
 * Clear part of the screen
 */
void clear_from(int row)
{
    /* Erase part of the screen */
    Term_erase(0, row, 80, 24);
}




/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 *
 * XXX XXX XXX Hack -- flush messages if necessary
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
    /* Hack -- Flush messages */
    if (!row && !col) msg_print(NULL);

#ifdef USE_COLOR

    /* Force mono-chrome if requested */
    if (!use_color) attr = TERM_WHITE;

#else

    /* Force mono-chrome */
    attr = TERM_WHITE;

#endif

    /* Put the string */
    Term_putstr(col, row, -1, attr, str);
}

void put_str(cptr str, int row, int col)
{
    c_put_str(TERM_WHITE, str, row, col);
}


/*
 * Print a string to the screen using an attribute
 *
 * At the given location, clear to the end of the line, and then,
 * using the given attribute, if allowed, add the given string.
 *
 * XXX XXX XXX Hack -- We flush messages if appropriate
 */
void c_prt(byte attr, cptr str, int row, int col)
{
    /* Hack -- Flush messages */
    if (!row && !col) msg_print(NULL);

    /* Clear the line, position the cursor */
    Term_erase(col, row, 80, 1);

#ifdef USE_COLOR

    /* Force mono-chrome */
    if (!use_color) attr = TERM_WHITE;

#else

    /* Use white */
    attr = TERM_WHITE;

#endif

    /* Dump the text (in White) */
    Term_addstr(-1, attr, str);
}

void prt(cptr str, int row, int col)
{
    c_prt(TERM_WHITE, str, row, col);
}





/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 */
bool askfor_aux(char *buf, int len)
{
    int y, x;

    int i = 0;

    int k = 0;

    bool done = FALSE;


    /* Locate the cursor */
    Term_locate(&x, &y);


    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Paranoia -- check column */
    if ((x < 0) || (x >= 80)) x = 0;

    /* Restrict the length */
    if (x + len > 80) len = 80 - x;


    /* Paranoia -- Clip the default entry */
    buf[len] = '\0';


    /* Display the default answer */
    Term_erase(x, y, len, 1);
    Term_putstr(x, y, -1, TERM_YELLOW, buf);


    /* Process input */
    while (!done) {

        /* Place cursor */
        Term_gotoxy(x + k, y);

        /* Get a key */
        i = inkey();

        /* Analyze the key */
        switch (i) {

          case ESCAPE:
            k = 0;
            done = TRUE;
            break;

          case '\n':
          case '\r':
            k = strlen(buf);
            done = TRUE;
            break;

          case 0x7F:
          case '\010':
            if (k > 0) k--;
            break;

          default:
            if ((k < len) && (isprint(i))) {
                buf[k++] = i;
            }
            else {
                bell();
            }
            break;
        }

        /* Terminate */
        buf[k] = '\0';

        /* Update the entry */
        Term_erase(x, y, len, 1);
        Term_putstr(x, y, -1, TERM_WHITE, buf);
    }

    /* Aborted */
    if (i == ESCAPE) return (FALSE);

    /* Success */
    return (TRUE);
}


/*
 * Request a string at the current cursor location.
 * Simply call "askfor_aux()" with *no* default string.
 * Return FALSE if the user presses ESCAPE, else TRUE.
 */
bool askfor(char *buf, int len)
{
    /* Wipe the buffer */
    buf[0] = '\0';

    /* Get some input */
    return (askfor_aux(buf, len));
}



/*
 * Verify something with the user.
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(cptr prompt)
{
    int i;

    char buf[80];

    /* Hack -- Build a "useful" prompt */
    strnfmt(buf, 78, "%.70s[y/n] ", prompt);

    /* Prompt for it */
    prt(buf, 0, 0);

    /* Get an acceptable answer */
    while (TRUE) {
        i = inkey();
        if (quick_messages) break;
        if (i == ESCAPE) break;
        if (strchr("YyNn", i)) break;
        bell();
    }

    /* Erase the prompt */
    prt("", 0, 0);

    /* Catch "yes" */
    if ((i == 'Y') || (i == 'y')) return (TRUE);

    /* Assume "no" */
    return (FALSE);
}


/*
 * Prompts for a keypress, and clears the prompt
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
    if (!prompt) prompt = "Command: ";
    
    /* Display a prompt */
    prt(prompt, 0, 0);

    /* Get a key */
    *command = inkey();

    /* Clear the prompt */
    prt("", 0, 0);

    /* Return TRUE unless ESCAPE */
    return (*command != ESCAPE);
}


/*
 * Request a "quantity" from the user
 *
 * Hack -- allow "command_arg" to specify a quantity
 */
s16b get_quantity(cptr prompt, int max)
{
    int amt;

    char out_val[80];


    /* Use "command_arg" */
    if (command_arg) {

        /* Extract a number */
        amt = command_arg;

        /* Clear "command_arg" */
        command_arg = 0;

        /* Enforce the maximum */
        if (amt > max) amt = max;

        /* Use it */
        return (amt);
    }


    /* Build a prompt if needed */
    if (!prompt) {

        /* Build a prompt */
        sprintf(out_val, "Quantity (1-%d): ", max);

        /* Use that prompt */
        prompt = out_val;
    }

    /* Prompt for the quantity */
    prt(prompt, 0, 0);


    /* Default to one */
    amt = 1;
    
    /* Build the default */
    sprintf(out_val, "%d", amt);

    /* Ask for a quantity */
    if (!askfor_aux(out_val, 6)) out_val[0] = '\0';

    /* Extract a number */
    amt = atoi(out_val);

    /* A letter means "all" */
    if (isalpha(out_val[0])) amt = max;

    /* Enforce the maximum */
    if (amt > max) amt = max;

    /* Enforce the minimum */
    if (amt < 0) amt = 0;

    /* Clear the prompt */
    prt("", 0, 0);
    
    /* Return the result */
    return (amt);
}


/*
 * Pauses for user response before returning		-RAK-	
 */
void pause_line(int row)
{
    int i;
    prt("", row, 0);
    put_str("[Press any key to continue]", row, 23);
    i = inkey();
    prt("", row, 0);
}

