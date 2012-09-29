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
 * This file uses no "specifically Angband" knowledge.
 * XXX So it should get its own header file, and include "term.h".
 */


#include "angband.h"


#ifdef KEYMAP
/* The "keymap" table -- initialized to all zeros */
static char keymap_table[KEYMAP_MODES][KEYMAP_CHARS];
#endif




#ifdef MACINTOSH

# include <Events.h>

#else

extern char *getenv();

#endif



#ifdef MACINTOSH

/*
 * Delay for "x" milliseconds
 */
void delay(int x)
{
    long t; t=TickCount(); while(TickCount()<t+(x*60)/1000);
}

#else

#ifdef __EMX__

void delay(int x)
{
    _sleep2(x);
}

#else

#ifndef MSDOS

/*
 * Unix port for "delay"
 */
void delay(int x)
{
    /* Do it in micro-seconds */
    usleep(1000 * x);
}

#endif	/* MSDOS */

#endif	/* __EMX__ */

#endif	/* MACINTOSH */



/* 
 * Flush the screen, make a noise
 */
void bell()
{
    /* Hack -- be sure everything is visible */
    Term_fresh();

    /* Make a bell noise (if allowed) */
    if (ring_bell) Term_bell();
}



/*
 * Move the cursor
 */
void move_cursor(int row, int col)
{
    Term_gotoxy(col, row);
}




/*
 * Flush all input chars
 */
void flush(void)
{
    Term_flush();
}



/*
 * Check for a keypress
 * Hack -- First, refresh.
 */
int kbhit(void)
{
    Term_fresh();
    return (Term_kbhit());
}


/*
 * Get a keypress from the user.
 * First, enforce a visible cursor and refresh.
 * Hack -- convert "zero" to "Escape".
 */
char inkey(void)
{
    int i, okay;
    okay = Term_show_cursor();
    Term_fresh();
    i = Term_inkey();
    if (!okay) Term_hide_cursor();
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
    Term_putstr(x, MSG_LINE, -1, COLOR_L_BLUE, " -more-");

    /* Hack -- warn the signal stuff */
    wait_for_more = 1;

    /* Get an acceptable keypress */
    while (1) {
	int cmd = inkey();
	if ((quick_messages) || (cmd == ESCAPE)) break;
	if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r')) break;
	bell();
    }

    /* Hack -- turn off the signal stuff */
    wait_for_more = 0;

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
	    Term_putstr(0, MSG_LINE, -1, COLOR_WHITE, msg);
	    Term_fresh();

	    /* Let other messages share the line */
	    len = strlen(msg) + 1;
	    msg_flag = TRUE;

	    /* Memorize the message */
	    message_new(msg, -1);
	}

	/* The message fits */
	else {

	    /* Display the new message */
	    Term_putstr(len, MSG_LINE, -1, COLOR_WHITE, msg);
	    Term_fresh();

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
	    Term_fresh();

	    /* Forget the message */
	    len = 0;
	    msg_flag = FALSE;
	}

	/* Display the message */
	else {

	    /* Clear the line */
	    Term_erase(0, MSG_LINE, 80-1, MSG_LINE);
	    Term_fresh();

	    /* Display it */
	    Term_putstr(0, MSG_LINE, -1, COLOR_WHITE, msg);
	    Term_fresh();

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
	    buf[len++] = toupper(*s++);
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
	    buf[len++] = toupper(*s++);
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
void c_put_str(int8u attr, cptr str, int row, int col)
{
    if (msg_flag && (row <= MSG_LINE)) msg_print(NULL);

#ifdef USE_COLOR
    if (!use_color) attr = COLOR_WHITE;
#else
    attr = COLOR_WHITE;
#endif

    Term_putstr(col, row, -1, attr, str);
}

void put_str(cptr str, int row, int col)
{
    c_put_str(COLOR_WHITE, str, row, col);
}


/*
 * Clear a line at a given location, and, at the same location,
 * Print a string, using a real attribute
 * Hack -- Be sure to flush msg_print if necessary.
 */
void c_prt(int8u attr, cptr str, int row, int col)
{
    int x, y;

#ifdef USE_COLOR
    if (!use_color) attr = COLOR_WHITE;
#else
    attr = COLOR_WHITE;
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
    c_prt(COLOR_WHITE, str, row, col);
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
    Term_putstr(x, 0, -1, COLOR_WHITE, "[y/n]");

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
	  case CTRL('J'):
	  case CTRL('M'):
	    done = TRUE;
	    break;
	  case CTRL('H'):
	  case DELETE:
	    if (k > 0) {
		buf[--k] = '\0';
		Term_erase(x1 + k, row, x2, row);
	    }
	    break;
	  default:
	    if ((k < len) && (isprint(i))) {
		Term_putch(x1 + k, row, COLOR_WHITE, i);
		buf[k++] = i;
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




