/* File: io.c */

/* Purpose: mid-level I/O (uses term.c) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


#include "angband.h"



/*
 * Flush the screen, make a noise
 */
void bell()
{
    /* Mega-Hack -- Flush the output */
    Term_fresh();

    /* Make a bell noise (if allowed) */
    if (ring_bell) Term_bell();

    /* Mega-Hack -- Flush the input */
    Term_flush();
}



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
    if (i < 8) return ('0' + i);
    return ('0');
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(uint i)
{
    if (i < 10) return ('0' + i);
    if (i < 16) return ('A' + i - 10);
    return ('0');
}


/*
 * Convert a hex-digit into a decimal
 */
static int deoct(char c)
{
    return (c - '0');
}

/*
 * Convert a hex-digit into a decimal
 */
static int dehex(char c)
{
    return ((c>='a') ? (10+c-'a') : (c>='A') ? (10+c-'A') : (c-'0'));
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
 * Maximum number of macros
 */
#define MACRO_MAX	512


/*
 * Legal bit-flags for macro_use[X]
 */
#define MACRO_USE_CMD	0x01	/* X triggers a command macro */
#define MACRO_USE_STD	0x02	/* X triggers a standard macro */



/*
 * Number of active macros
 * Slot zero is a fake entry.
 */
static int macro_num = 0;

/*
 * Array of macro patterns
 */
static cptr macro_pat[MACRO_MAX];

/*
 * Array of macro actions
 */
static cptr macro_act[MACRO_MAX];

/*
 * Specify if a macro requires "cmd_flag"
 */
static bool macro_cmd[MACRO_MAX];

/*
 * Fast check for trigger of any macros
 */
static byte macro_use[256];



/*
 * Hack -- append all current macros to the given file
 */
void macro_dump(cptr fname)
{
    int i;
    FILE *fff;
    char tmp[1024];

#ifdef MACINTOSH
    _ftype = 'TEXT';
#endif

    /* Append to the file */
    fff = my_tfopen(fname, "a");

    /* Failure */
    if (!fff) return;

    /* Start dumping */
    fprintf(fff, "\n\n# Automatic macro dump\n\n");

    /* Dump them */
    for (i = 1; i < macro_num; i++) {

        /* Paranoia -- Skip empty macros */
        if (!macro_act[i] || !macro_pat[i]) continue;

        /* Start the macro */
        fprintf(fff, "# Macro number %d\n\n", i);

        /* Extract the action */
        ascii_to_text(tmp, macro_act[i]);

        /* Dump the macro */
        fprintf(fff, "A:%s\n", tmp);

        /* Extract the action */
        ascii_to_text(tmp, macro_pat[i]);

        /* Dump command macros */
        if (macro_cmd[i]) fprintf(fff, "C:%s\n", tmp);

        /* Dump normal macros */
        else fprintf(fff, "P:%s\n", tmp);

        /* End the macro */
        fprintf(fff, "\n\n");		
    }

    /* Start dumping */
    fprintf(fff, "\n\n\n\n");

    /* Close */
    fclose(fff);
}


/*
 * Hack -- add a macro definition (or redefinition).
 *
 * If "cmd_flag" is set then this macro is only active when
 * the user is being asked for a command (see "inkey_flag").
 *
 * Hack -- You might get useful results from "act == NULL".
 */
void macro_add(cptr pat, cptr act, bool cmd_flag)
{
    int n;

    /* Look for a slot */
    for (n = 1; n <= macro_num; n++) {

        /* Notice empty slots (including final one) */
        if (!macro_pat[n]) break;

        /* Notice macro redefinition */
        if (streq(macro_pat[n], pat)) {

            /* Free the old macro action */
            string_free(macro_act[n]);

            /* Save the macro action */
            macro_act[n] = string_make(act);

            /* Save the "cmd_flag" */
            macro_cmd[n] = cmd_flag;

            /* All done */
            return;
        }
    }

    /* Note new maximum */
    if (n >= macro_num) macro_num = n + 1;

    /* Save the pattern */
    macro_pat[n] = string_make(pat);

    /* Save the macro action */
    macro_act[n] = string_make(act);

    /* Save the "cmd_flag" */
    macro_cmd[n] = cmd_flag;

    /* Hack -- Note the "trigger" char */
    macro_use[(byte)(pat[0])] |= MACRO_USE_STD;

    /* Hack -- Note the "trigger" char of command macros */
    if (cmd_flag) macro_use[(byte)(pat[0])] |= MACRO_USE_CMD;
}



/*
 * Check for possibly pending macros
 */
static int macro_maybe(cptr buf, int n)
{
    int i;

    /* Scan the macros */
    for (i = n; i < macro_num; i++) {

        /* Skip inactive macros */
        if (macro_cmd[i] && !inkey_flag) continue;

        /* Check for "prefix" */
        if (prefix(macro_pat[i], buf)) {

            /* Ignore complete macros */
            if (!streq(macro_pat[i], buf)) return (i);
        }
    }

    /* No matches */
    return (0);
}


/*
 * Find the longest completed macro
 */
static int macro_ready(cptr buf)
{
    int i, t, n = 0, s = 0;

    /* Scan the macros */
    for (i = 1; i < macro_num; i++) {

        /* Skip inactive macros */
        if (macro_cmd[i] && !inkey_flag) continue;

        /* Check for "prefix" */
        if (!prefix(buf, macro_pat[i])) continue;

        /* Check the length of this entry */
        t = strlen(macro_pat[i]);

        /* Keep track of the longest entry */
        if (!n || (t > s)) n = i, s = t;
    }

    /* Return the result */
    return (n);
}



/*
 * Local "need flush" variable
 */
static bool flush_later = FALSE;


/*
 * Flush all input chars.  Actually, remember the flush,
 * and do a "Term_flush()" before the next "inkey()".
 * This makes "disturb()" and such much more efficient.
 */
void flush(void)
{
    /* Do it later */
    flush_later = TRUE;
}



/*
 * Help get a keypress (process macros)
 *
 * We use the "Term_key_push()" function to handle "failed" macros,
 * as well as "extra" keys read in while choosing a macro, and the
 * actual action for the macro.  Note that embedded macros are legal,
 * but not really suggested.  The user can easily overflow the key
 * buffer by defining a macro from, say, "x" to "xx".  Be careful.
 *
 * The user only gets 500 (1+2+...+29+30) milliseconds for the macro.
 */
static char inkey_aux(void)
{
    int		i, k, n, p = 0, w = 0;

    cptr	pat, act;

    char	buf[1024];


    /* Get a keypress */
    i = Term_inkey();

    /* Efficiency -- Ignore impossible macros */
    if (!macro_use[(byte)(i)]) return (i);

    /* Efficiency -- Ignore inactive macros */
    if (!inkey_flag && (macro_use[(byte)(i)] == MACRO_USE_CMD)) return (i);


    /* Save the first key, advance */
    buf[p++] = i;
    buf[p] = '\0';


    /* Wait for a macro, or a timeout */
    for (k = 1; ((k = macro_maybe(buf, k))); ) {

        /* Check for a key */
        if (Term_kbhit()) {

            /* Get the key */
            i = Term_inkey();

            /* Append the key */
            buf[p++] = i;
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
    if (!k) {

        /* Push all the keys back on the queue */
        while (p > 0) {

            /* Push the key, notice over-flow */
            if (Term_key_push(buf[--p])) return (0);
        }

        /* Grab and return the first key pressed */
        return (Term_inkey());
    }


    /* Access the pattern and action */
    pat = macro_pat[k];
    act = macro_act[k];

    /* Get the length of the pattern */
    n = strlen(pat);

    /* Push the "extra" keys back on the queue */
    while (p > n) {

        /* Push the key, notice over-flow */
        if (Term_key_push(buf[--p])) return (0);
    }

    /* XXX XXX XXX XXX */
    /* Mega-Hack -- strip "control-backslash" strings */
    if (Term_kbhit() == 28) {

        /* Strip the code */
        (void)Term_inkey();

        /* Strip the string and terminator */
        while (Term_inkey() != 28);
    }

    /* Get the length of the action */
    n = strlen(act);

    /* Push the macro "action" onto the key queue */
    while (n > 0) {

        /* Push the key, notice over-flow */
        if (Term_key_push(act[--n])) return (0);
    }

    /* Return the first key of the macro */
    return (Term_inkey());
}




/*
 * Get a keypress from the user.  Handle macros and stuff.
 *
 * Hack -- special treatment of "inkey()" from "request_command()"
 *
 * Hack -- make sure the cursor is visible (usually).
 *
 * Only refresh the screen if we will have to wait for a keypress.
 *
 * Note the special handling of a few "special" control-keys, which
 * are reserved to simplify the use of various "main-xxx.c" files.
 */
char inkey(void)
{
    int i;

    bool fix = FALSE;


    /* Hack -- handle delayed "flush()" */
    if (flush_later) {

        /* Forget old keypresses */
        Term_flush();

        /* Done */
        flush_later = FALSE;
    }


    /* Show the cursor (usually) */
    if (!inkey_flag || hilite_player) {

        /* Successful showing requires fixing later */
        if (0 == Term_show_cursor()) fix = TRUE;
    }

    /* Get a keypress */
    while (TRUE) {

        /* Mega-Hack -- Flush the output if no key is ready */
        if (!Term_kbhit()) Term_fresh();

        /* Get a key */
        i = inkey_aux();

        /* Hack -- convert back-quote into escape */
        if (i == '`') i = ESCAPE;

        /* Hack -- strip "control-caret" */
        if (i == 30) continue;

        /* Hack -- strip "control-underscore" macro triggers */
        if (i == 31) {

            /* Strip the text */
            while (Term_inkey() > 32);

            /* Try again */
            continue;
        }

        /* XXX XXX XXX XXX */
        /* Mega-Hack -- strip "control-backslash" macro triggers */
        if (i == 28) {

            /* Strip the text */
            while (Term_inkey() != 28);

            /* Try again */
            continue;
        }

        /* Accept real keys */
        if (i) break;
    }

    /* Fix the cursor if necessary */
    if (fix) Term_hide_cursor();

    /* Return the keypress */
    return (i);
}





/*
 * We save messages in the order they are "memorized", and only forget
 * messages if we use all MESSAFE_BUF bytes of storage, or if we save
 * more than MESSAGE_MAX messages.  We attempt to forget messages in a
 * graceful manner.  The most recent message may be "appended" to, which
 * may cause it to "move" (similar to realloc()).  Every message has an
 * "age", which starts at "zero" and increases every time a new message
 * is memorized.  Identical sequential messages share buffer memory.
 */
#define MESSAGE_MAX 512
#define MESSAGE_BUF 8192

/* The "age" of the oldest message */
static uint message__age = 0;

/* The "index" of the "current" message */
static uint message__cur = 0;

/* The "offset" of each message */
static u16b message__ptr[MESSAGE_MAX];

/* The "length" of each message */
static u16b message__len[MESSAGE_MAX];

/* The buffer to hold the text for all the messages */
static char message__buf[MESSAGE_BUF];


/*
 * How many messages are available?
 */
uint message_num()
{
  /* Be sure to count the current message */
  return (message__age + 1);
}


/*
 * Convert an "age" into an "index"
 */
static uint message_ind(int age)
{
  int x = message__cur;
  x = (x - age) % MESSAGE_MAX;
  while (x < 0) x += MESSAGE_MAX;
  return (x);
}


/*
 * Recall the "length" of a saved message
 */
uint message_len(uint age)
{
  uint x;

  /* Forgotten messages have no length */
  if (age > message__age) return (0);

  /* Access the message */
  x = message_ind(age);

  /* Return the length */
  return (message__len[x]);
}

/*
 * Recall the "text" of a saved message
 */
cptr message_str(uint age)
{
  uint x, o;
  cptr s;

  /* Forgotten messages have no text */
  if (age > message__age) return ("");

  /* Access the message */
  x = message_ind(age);

  /* Get the "offset" for the message */
  o = message__ptr[x];

  /* Access the message text */
  s = &message__buf[o];

  /* Return the message text */
  return (s);
}


/*
 * Append some text to the most recent message
 *
 * If "len" is negative, use the entire "msg",
 * else use only the first "len" chars of "msg".
 */
void message_add(cptr msg, int len)
{
  int i;

  uint x = message__cur;
  uint o = message__ptr[x];
  uint n = message__len[x];

  /* Negative length -- use entire message */
  if (len < 0) len = strlen(msg);

  /* Be sure the message can fit! */
  if (len + n + 2 >= MESSAGE_BUF) len = MESSAGE_BUF - (n + 2);

  /* Sometimes, we run out of buffer space */
  if (o + len + n + 1 >= MESSAGE_BUF)
  {
    /* Forget the older messages */
    for (i = message__age; i > 0; i--)
    {
      uint x2 = message_ind(i);
      uint o2 = message__ptr[x2];
      uint n2 = message__len[x2];

      /* Stop on the first "safe" message */
      if ((o2 + n2 < o) && (o2 > n + len + 1)) break;

      /* Forget the over-written message */
      message__age--;
    }

    /* Slide the pre-existing part of the message */
    for (i = 0; i < n; i++)
    {
      message__buf[i] = message__buf[o + i];
    }

    /* Take note of the new message location */
    message__ptr[x] = o = 0;
  }

  /* Forget the older messages */
  for (i = message__age; i > 0; i--)
  {
    uint x2 = message_ind(i);
    uint o2 = message__ptr[x2];
    uint n2 = message__len[x2];

    /* Stop on the first "safe" message */
    if ((o2 + n2 < o) || (o2 > o + n + len + 1)) break;

    /* Forget the over-written message */
    message__age--;
  }

  /* Append the new part of the message */
  for (i = 0; i < len; i++)
  {
    message__buf[o + n + i] = msg[i];
  }

  /* Terminate the message */
  message__buf[o + n + len] = '\0';

  /* Save the new length */
  message__len[x] += len;
}


/*
 * Create a new "current" message, with (optional) text
 *
 * First, attempt to "optimize" the "current" message, by allowing sequential
 * messages to "share" suffix space if they match exactly.  See below...
 */
void message_new(cptr msg, int len)
{
  int age = message__age;

  /* Access the "current" message */
  int x = message__cur;
  int o = message__ptr[x];
  int n = message__len[x];

  /* Access the "previous" message */
  int x1 = message_ind(1);
  int o1 = message__ptr[x1];
  int n1 = message__len[x1];


  /* Hack -- always append "something" */
  if (!msg) len = 0;


  /* Hack -- prevent first call to "message_new()" from making new message */
  if (!age && !n) {

      /* Append the message */
      message_add(msg, len);

      /* All done */
      return;
  }



  /* Attempt to optimize (if it looks possible) */
  if (age && n && n1 && (n <= n1)) {

    cptr s, s1;

    /* Access the current message text */
    s = &message__buf[o];

    /* Access the message text */
    s1 = &message__buf[o1];

    /* Check for actual "suffix sharing" */
    if (streq(s, s1 + (n1 - n))) {

      /* Let the messages "share suffixes" */
      o = o1 + (n1 - n);

      /* Save the optimized location */
      message__ptr[x] = o;
    }
  }


  /* Advance the index */
  x = x + 1;

  /* Wrap if needed */
  if (x >= MESSAGE_MAX) x = 0;

  /* Advance the offset (skip the nul) */
  o = o + n + 1;

  /* Wrap if necessary */
  if (o >= MESSAGE_BUF) o = 0;

  /* One more message */
  message__age++;

  /* Save the message index */
  message__cur = x;

  /* No message yet */
  message__len[x] = 0;

  /* Remember where it starts */
  message__ptr[x] = o;


  /* Append the message */
  message_add(msg, len);
}





/*
 * Output a message to top line of screen.
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
 * "ignore" any "pending" messages (see dungeon.c and store.c).
 *
 * Note that this function should probably NOT be called with
 * messages longer than 72 characters, or data will be lost.
 * Consider using "message()" instead, which can split messages.
 *
 * Note that multiple messages on a single line are separated
 * by a blank space, not a normal space, since the line is erased
 * using Term_erase() and not by printing spaces.
 *
 * Consider an option to force each message to get its own line.
 * This would be very nice for the Borg, and some people as well.
 */
void msg_print(cptr msg)
{
    int len = (msg ? strlen(msg) : 0);

    static pos = 0;


    /* Process global flag */
    if (!msg_flag) {

        /* Clear the line */
        Term_erase(0, 0, 80-1, 0);

        /* Reset the cursor */
        pos = 0;
    }


    /* Hack -- flush when requested or needed */
    if (pos && (!msg || ((pos + len) > 72))) {

        /* Hack -- stay on screen */
        if (pos > 73) pos = 73;

        /* Pause for response */
        Term_putstr(pos, 0, -1, TERM_L_BLUE, "-more-");

        /* Get an acceptable keypress */
        while (1) {
            int cmd = inkey();
            if ((quick_messages) || (cmd == ESCAPE)) break;
            if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) break;
            bell();
        }

        /* Clear the line */
        Term_erase(0, 0, 80-1, 0);

        /* Forget it */
        msg_flag = FALSE;

        /* Reset cursor */
        pos = 0;
    }


    /* No message */
    if (!msg) return;


    /* Memorize the message */
    message_new(msg, len);

    /* Display the new message */
    Term_putstr(pos, 0, len, TERM_WHITE, msg);

    /* Remember the message */
    msg_flag = TRUE;

    /* Skip a space before next message */
    pos += len + 1;
}



/*
 * This function displays messages, using "msg_print()".
 *
 * This routine also takes a set of flags to determine what
 * additional processing, if any, to apply to the message.
 *
 * Mode:
 *   0x01 = Capitalize "msg" before using (restore when done).
 *   0x02 = This is just a piece of the final message.
 *   0x04 = Split the message into pieces if needed.
 *
 * Notes:
 *
 *   Since "buffer" over-rides "split", you must specify "split"
 *   in the *final* piece of the message.  If this is unknown, you may
 *   use "message(NULL,SPLIT)" to split and flush the stored message.
 *
 *   Capitalization only affects the actual "msg" parameter, so it
 *   should usually only be requested for the *first* piece of a message.
 *
 *   Combining the "capitalize" and "buffer" modes (into 0x03) is
 *   very useful for dumping a monster name, followed by another
 *   call to message() with the rest of the sentence, and no mode.
 */
void message(cptr msg, int mode)
{
    static int len = 0;
    static char buf[512] = "";

    /* Start on the first letter */
    cptr s = msg;

    /* Splitter */
    char *t;

    /* If this is just a piece, memorize it */
    if (mode & 0x02) {

        /* No message */
        if (!s || !*s) return;

        /* Capitalize it */
        if ((mode & 0x01) && (islower(*s))) {

            /* Hack -- toupper() may be a macro */
            buf[len++] = toupper(*s); s++;
        }

        /* Append the message */
        while (*s) buf[len++] = *s++;

        /* Terminate it (optional, just to be safe) */
        buf[len] = '\0';

        /* All done */
        return;
    }


    /* Speed -- handle a very simple special case */
    if (!len && !(mode & 0x01) && !(mode & 0x04)) {

        /* Just dump it */
        msg_print(msg);

        /* All done */
        return;
    }



    /* Append the current message if given */
    if (s && *s) {

        /* Capitalize it */
        if ((mode & 0x01) && (islower(*s))) {

            /* Hack -- toupper() may be a macro */
            buf[len++] = toupper(*s); s++;
        }

        /* Append the message */
        while (*s) buf[len++] = *s++;

        /* Terminate it */
        buf[len] = '\0';
    }


    /* Now start using the "split" buffer */
    t = buf;

    /* Split if requested and required (72 or more chars) */
    while ((mode & 0x04) && (len > 71)) {

        int check, split;

        /* Assume no split */
        split = -1;

        /* Find the (farthest) "useful" split point */
        for (check = 40; check < 70; check++) {
            if (t[check] == ' ') split = check;
        }

        /* No split?  XXX Should split it anyway */
        if (split < 0) break;

        /* Split the message (and advance the split point) */
        t[split++] = '\0';

        /* Print the first part */
        msg_print(t);

        /* Prepare to recurse on the rest of "buf" */
        t += split; len -= split;

        /* Hack -- indent subsequent lines two spaces */
        *--t = ' '; len++;
        *--t = ' '; len++;
    }

    /* Print the whole thing (or remains of the split) */
    msg_print(t);

    /* Forget the buffer */
    len = 0; buf[0] = '\0';
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
    Term_erase(0, row, 80-1, 24-1);
}




/*
 * Move to a given location, and without clearing the line,
 * Print a string, using a color (never multi-hued)
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
    /* Hack -- Flush messages */
    if (!row && !col) msg_print(NULL);

#ifdef USE_COLOR
    if (!use_color) attr = TERM_WHITE;
#else
    attr = TERM_WHITE;
#endif

    Term_putstr(col, row, -1, attr, str);
}

void put_str(cptr str, int row, int col)
{
    c_put_str(TERM_WHITE, str, row, col);
}


/*
 * Clear a line at a given location, and, at the same location,
 * Print a string, using a real attribute
 * Hack -- Be sure to flush msg_print if necessary.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
    int x, y;

    /* Hack -- Flush messages */
    if (!row && !col) msg_print(NULL);

#ifdef USE_COLOR
    if (!use_color) attr = TERM_WHITE;
#else
    attr = TERM_WHITE;
#endif

    /* Position the cursor */
    Term_gotoxy(col, row);

    /* Dump the text */
    Term_addstr(-1, attr, str);

    /* Clear the rest of the line */
    if ((Term_locate(&x, &y) == 0) && (y == row)) Term_erase(x, y, 80-1, y);
}

void prt(cptr str, int row, int col)
{
    c_prt(TERM_WHITE, str, row, col);
}





/*
 * Let the player verify a choice.  -CJS-
 * Refuse to accept anything but y/n/Y/N/Escape
 * Return TRUE on "yes", else FALSE
 */
int get_check(cptr prompt)
{
    int res, x;

    /* Hack -- no prompt? */
    if (!prompt) prompt = "Yes or no?";

    /* Display the prompt (and clear the line) */
    prt(prompt, 0, 0);

    /* See how long the prompt is */
    x = strlen(prompt) + 1;

    /* Do NOT wrap */
    if (x > 74) x = 74;

    /* Ask the question */
    Term_putstr(x, 0, -1, TERM_WHITE, "[y/n]");

    /* Get an acceptable answer */
    while (1) {
        res = inkey();
        if (quick_messages) break;
        if (res == ESCAPE) break;
        if (strchr("YyNn", res)) break;
        bell();
    }

    /* Erase the prompt */
    prt("", 0, 0);

    /* Catch "yes" */
    if ((res == 'Y') || (res == 'y')) return (TRUE);

    /* Assume "no" */
    return (FALSE);
}


/*
 * Prompts (optional), then gets and stores a keypress
 * Returns TRUE unless the character is "Escape"
 */
int get_com(cptr prompt, char *command)
{
    if (prompt) prt(prompt, 0, 0);

    *command = inkey();

    if (prompt) prt("", 0, 0);

    return (*command != ESCAPE);
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
    int y, x, x1, x2;

    int i = 0;
    
    int k = 0;
    
    bool done = FALSE;
    

    /* Locate the cursor */
    Term_locate(&x, &y);


    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Paranoia -- check column */
    if ((x < 0) || (x >= 80)) x = 0;

    /* Find the box bounds */
    x1 = x;
    x2 = x1 + len - 1;
    if (x2 >= 80) {
        len = 80 - x1;
        x2 = 80 - 1;
    }


    /* Paranoia -- Clip the default entry */
    buf[len] = '\0';
    

    /* Display the default answer */
    Term_erase(x1, y, x2, y);
    Term_putstr(x1, y, -1, TERM_YELLOW, buf);


    /* Process input */
    while (!done) {

        /* Place cursor */
        Term_gotoxy(x1 + k, y);
        
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

          case '\010':
          case DELETE:
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
        Term_erase(x1, y, x2, y);
        Term_putstr(x1, y, -1, TERM_WHITE, buf);
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




/*
 * Save and restore the screen -- no flushing
 */

void save_screen()
{
    /* Flush messages */
    msg_print(NULL);

    /* Save the screen */
    Term_save();
}

void restore_screen()
{
    /* Flush messages */
    msg_print(NULL);

    /* Load the screen */
    Term_load();
}




