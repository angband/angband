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
    /* Flush the output */
    Term_fresh();

    /* Make a bell noise (if allowed) */
    if (ring_bell) Term_bell();

    /* Flush the input */
    flush();
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

	uint i = *str++;
	
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
 * Number of active macros
 * Slot zero is a fake entry.
 */
static int macro_num = 0;

/*
 * Array of macro patterns
 */
static cptr macro_pat[256];

/*
 * Array of macro actions
 */
static cptr macro_act[256];


/*
 * Mega-Hack -- dump all current macros to the given file
 * Note -- the macros are appended to the file.
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

	/* Skip empty macros */
	if (!macro_act[i] || !macro_pat[i]) continue;

	/* Start the macro */
	fprintf(fff, "# Macro number %d\n\n", i);
		
	/* Extract the action */
	ascii_to_text(tmp, macro_act[i]);
	
	/* Dump the macro */
	fprintf(fff, "A:%s\n", tmp);
	
	/* Extract the action */
	ascii_to_text(tmp, macro_pat[i]);
	
	/* Dump the macro */
	fprintf(fff, "P:%s\n", tmp);

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
 * Hack -- allow removal via "NULL" action
 */
void macro(cptr pat, cptr act)
{
    int n, m;

    /* Look for a slot */
    for (m = n = 1; n <= macro_num; n++) {

	/* Notice empty slots */
	if (!macro_pat[n]) m = n;
	
	/* Notice macro redefinition */
	else if (streq(macro_pat[n], pat)) {

	    /* Free the old macro action */
	    string_free(macro_act[n]);
	    
	    /* Save the macro action */
	    macro_act[n] = string_make(act);

	    /* All done */
	    return;
	}
    }

    /* Note new maximum */
    if (m >= macro_num) macro_num = m + 1;

    /* Save the pattern */
    macro_pat[m] = string_make(pat);
    
    /* Save the macro action */
    macro_act[m] = string_make(act);
}



/*
 * Check for macro "start" key
 */
static bool macro_ignore(char c)
{
    int i;

    /* Scan the macros */
    for (i = 1; i < macro_num; i++) {

	/* Check for "prefix" */
	if (macro_pat[i][0] == c) return (FALSE);
    }

    /* No matches, ignore that key */
    return (TRUE);
}


/*
 * Check for possibly pending macros
 */
static int macro_maybe(cptr buf, int n)
{
    int i;

    /* Scan the macros */
    for (i = n; i < macro_num; i++) {

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
 * Help get a keypress.
 *
 * Uses the "key_macro" facility from "term.c" for successful macros,
 * and the "Term_key_push()" function to handle "failed" macros, as
 * well as "extra" keys read in while choosing a macro.
 *
 * The user only gets 500 (1+2+...+29+30) milliseconds for the macro.
 */
static char inkey_aux(void)
{
    int i, k, n = 0, p = 0, w = 0;

    char buf[128];


    /* Hack -- Forget completed macros */
    if (Term->key_macro && !Term->key_macro[0]) Term->key_macro = NULL;
    
    /* Wait for a new keypress from the current term */
    i = Term_inkey();

    /* Hack -- no recursive macro building */
    if (Term->key_macro) return (i);


    /* Do not attempt to macro-ize impossible keys */
    if (macro_ignore(i)) return (i);


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


    /* Successful macro */
    if ((k = macro_ready(buf))) {
     
        /* Get the length */
	n = strlen(macro_pat[k]);
	
	/* Prepare a macro */
	Term->key_macro = macro_act[k];
    }

    /* Push the "extra" keys back on the queue */
    while (p > n) Term_key_push(buf[--p]);

    /* Return a real key if the macro failed */
    if (!n) return (Term_inkey());
    
    /* Make sure we get called again */
    return (0);
}




/*
 * Flush all input chars
 */
void flush(void)
{
    /* Forget old keypresses */
    Term_flush();
}




/*
 * Get a keypress from the user.  Handle macros and stuff.
 *
 * Hack -- special treatment of "inkey()" from "request_command()"
 *
 * Hack -- make sure the cursor is visible (usually).
 */
char inkey(void)
{
    int i;

    bool fix = FALSE;

    /* Show the cursor (usually) */
    if (!inkey_flag || hilite_player) {

	/* Successful showing requires fixing later */
	if (0 == Term_show_cursor()) fix = TRUE;
    }
    
    /* Flush the output */
    Term_fresh();

    /* Get a keypress */
    for (i = 0; !i; i = inkey_aux());

    /* Hack -- convert back-quote into escape */
    if (i == '`') i = ESCAPE;
    
    /* Fix the cursor if necessary */
    if (fix) Term_hide_cursor();

    /* Return the keypress */
    return (i);
}




/*
 * Get a "more" message (on the message line)
 * Then erase the entire message line
 * Leave the cursor on the message line.
 */
void msg_more(int x)
{
    /* Do NOT run out of space */
    if (x > 72) x = 72;

    /* Print a message */
    Term_putstr(x, MSG_LINE, -1, TERM_L_BLUE, " -more-");

    /* Get an acceptable keypress */
    while (1) {
	int cmd = inkey();
	if ((quick_messages) || (cmd == ESCAPE)) break;
	if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) break;
	bell();
    }

    /* Note -- Do *NOT* call erase_line() */
    Term_erase(0, MSG_LINE, 80-1, MSG_LINE);
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
#define MESSAGE_MAX 500
#define MESSAGE_BUF 5000

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
 * These messages are kept for later reference (see above).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * We intentionally erase and flush the line to provide "flicker".
 *
 * When "msg_flag" is set, it means "verify old messages"
 * Note: msg_flag must be explicitly "unset" if desired
 * Note that "msg_flag" affects other routines.  Icky.
 */
void msg_print(cptr msg)
{
    static len = 0;


    /* Old messages need verification */
    if (msg_flag) {

	/* Special case: flush */
	if (!msg) {

	    /* Wait for it... */
	    msg_more(len);

	    /* Forget it */
	    len = 0;
	    msg_flag = FALSE;
	}

	/* We must start a new line */
	else if ((len + strlen(msg)) > 72) {

	    /* Wait for it */
	    msg_more(len);

	    /* Display the new message */
	    Term_putstr(0, MSG_LINE, -1, TERM_WHITE, msg);

	    /* Let other messages share the line */
	    len = strlen(msg) + 1;
	    msg_flag = TRUE;

	    /* Memorize the message */
	    message_new(msg, -1);
	}

	/* The message fits */
	else {

	    /* Display the new message */
	    Term_putstr(len, MSG_LINE, -1, TERM_WHITE, msg);

	    /* Let other messages share the line */
	    len += strlen(msg) + 1;
	    msg_flag = TRUE;

	    /* Memorize the message */
	    message_new(msg, -1);
	}
    }


    /* Ignore old messages */
    else {

	/* Special case -- flush */
	if (!msg) {

	    /* Clear the line */
	    Term_erase(0, MSG_LINE, 80-1, MSG_LINE);

	    /* Forget the message */
	    len = 0;
	    msg_flag = FALSE;
	}

	/* Display the message */
	else {

	    /* Clear the line */
	    Term_erase(0, MSG_LINE, 80-1, MSG_LINE);

	    /* Display it */
	    Term_putstr(0, MSG_LINE, -1, TERM_WHITE, msg);

	    /* Let other messages share the line */
	    len = strlen(msg) + 1;
	    msg_flag = TRUE;

	    /* Memorize it */
	    message_new(msg, -1);
	}
    }
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
 *   Since "buffer" over-rides "split", you must specify "split"
 *   in the *final* piece of the message.  If this is unknown, you may
 *   use "message(NULL,SPLIT)" to split and flush the stored message.
 *
 *   Capitalization only affects the actual "msg" parameter, so it
 *   should usually only be requested for the *first* piece of a message.
 *
 *   There is a global flag "msg_flag" which may affect the results
 *   of sending messages to "msg_print".
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
    register cptr s = msg;

    /* Splitter */
    register char *t;

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
 * Erase a line (flush msg_print first)
 */
void erase_line(int row, int col)
{
    if (msg_flag && (row == MSG_LINE)) msg_print(NULL);

    Term_erase(col, row, 80-1, row);
}

/*
 * Erase the screen (flush msg_print first)
 */
void clear_screen(void)
{
    if (msg_flag) msg_print(NULL);

    Term_clear();
}

/*
 * Clear part of the screen
 */
void clear_from(int row)
{
    if (msg_flag && (row <= MSG_LINE)) msg_print(NULL);

    Term_erase(0, row, 80-1, 24-1);
}




/*
 * Move to a given location, and without clearing the line,
 * Print a string, using a color (never multi-hued)
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
    if (msg_flag && (row <= MSG_LINE)) msg_print(NULL);

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

#ifdef USE_COLOR
    if (!use_color) attr = TERM_WHITE;
#else
    attr = TERM_WHITE;
#endif

    if (msg_flag && (row == MSG_LINE)) msg_print(NULL);

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
    erase_line(0, 0);

    /* Extract the answer */
    return (strchr("Yy", res) ? TRUE : FALSE);
}


/*
 * Prompts (optional), then gets and stores a keypress
 * Returns TRUE unless the character is "Escape"
 */
int get_com(cptr prompt, char *command)
{
    if (prompt) prt(prompt, 0, 0);

    *command = inkey();

    if (prompt) erase_line(MSG_LINE, 0);

    return (*command != ESCAPE);
}


/*
 * Gets a string terminated by <RETURN>, and return TRUE.
 * Otherwise, erase the input, and return FALSE.
 * Hack -- force legal col and len values
 */
int get_string(char *buf, int row, int col, int len)
{
    register int i, k, x1, x2;
    int done;

    if (msg_flag && (row <= MSG_LINE)) msg_print(NULL);

    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Paranoia -- check column */
    if ((col < 0) || (col >= 80)) col = 0;

    /* Find the box bounds */
    x1 = col;
    x2 = x1 + len - 1;
    if (x2 >= 80) {
	len = 80 - x1;
	x2 = 80 - 1;
    }

    /* Erase the "answer box" and place the cursor */
    Term_erase(x1, row, x2, row);

    /* Assume no answer (yet) */
    buf[0] = '\0';

    /* Process input */    
    for (k = 0, done = 0; !done; ) {

	i = inkey();

	switch (i) {

	  case ESCAPE:
	    buf[0] = '\0';
	    Term_erase(x1, row, x2, row);
	    return (FALSE);

	  case '\n':
	  case '\r':
	    done = TRUE;
	    break;

	  case '\010':
	  case DELETE:
	    if (k > 0) {
		buf[--k] = '\0';
		Term_erase(x1 + k, row, x2, row);
	    }
	    break;

	  default:
	    if ((k < len) && (isprint(i))) {
		Term_putch(x1 + k, row, TERM_WHITE, i);
		buf[k++] = i;
		buf[k] = '\0';
	    }
	    else {
		bell();
	    }
	    break;
	}
    }

    /* Remove trailing blanks */
    while ((k > 0) && (buf[k-1] == ' ')) k--;

    /* Terminate it */
    buf[k] = '\0';

    /* Return the result */
    return (TRUE);
}


/*
 * Ask for a string, at the current cursor location.
 * Return "Was a legal answer entered?"
 *
 * Might be even better to give this routine a "prompt"
 * and to ask the questions at (0,0) with the prompt...
 */
int askfor(char *buf, int len)
{
    int x, y;
    Term_locate(&x, &y);
    return (get_string(buf, y, x, len));
}



/*
 * Pauses for user response before returning		-RAK-	 
 */
void pause_line(int prt_line)
{
    int i;
    erase_line(prt_line, 0);
    put_str("[Press any key to continue]", prt_line, 23);
    i = inkey();
    erase_line(prt_line, 0);
}




/*
 * Save and restore the screen -- no flushing
 */

void save_screen()
{
    if (msg_flag) msg_print(NULL);

    Term_save();
}

void restore_screen()
{
    if (msg_flag) msg_print(NULL);

    Term_load();
}




