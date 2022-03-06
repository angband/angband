/**
 * \file ui-input.c
 * \brief Some high-level UI functions, inkey()
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cmds.h"
#include "game-event.h"
#include "game-input.h"
#include "game-world.h"
#include "init.h"
#include "obj-gear.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-path.h"
#include "savefile.h"
#include "target.h"
#include "ui-command.h"
#include "ui-context.h"
#include "ui-curse.h"
#include "ui-display.h"
#include "ui-effect.h"
#include "ui-help.h"
#include "ui-keymap.h"
#include "ui-knowledge.h"
#include "ui-map.h"
#include "ui-menu.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-player-properties.h"
#include "ui-player.h"
#include "ui-prefs.h"
#include "ui-signals.h"
#include "ui-spell.h"
#include "ui-store.h"
#include "ui-target.h"

static bool inkey_xtra;
uint32_t inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */

/**
 * Flush all pending input.
 *
 * Actually, remember the flush, using the "inkey_xtra" flag, and in the
 * next call to "inkey()", perform the actual flushing, for efficiency,
 * and correctness of the "inkey()" function.
 */
void flush(game_event_type unused, game_event_data *data, void *user)
{
	/* Do it later */
	inkey_xtra = true;
}


/**
 * Helper function called only from "inkey()"
 */
static ui_event inkey_aux(int scan_cutoff)
{
	int w = 0;	

	ui_event ke;
	
	/* Wait for a keypress */
	if (scan_cutoff == SCAN_OFF) {
		(void)(Term_inkey(&ke, true, true));
	} else {
		w = 0;

		/* Wait only as long as macro activation would wait */
		while (Term_inkey(&ke, false, true) != 0) {
			/* Increase "wait" */
			w++;

			/* Excessive delay */
			if (w >= scan_cutoff) {
				ui_event empty = EVENT_EMPTY;
				return empty;
			}

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, 10);
		}
	}

	return (ke);
}



/**
 * Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()".  This key sequence cannot be
 * bypassed by the Borg.  We use it to implement keymaps.
 */
struct keypress *inkey_next = NULL;

/**
 * See if more propmts will be skipped while in a keymap.
 */
static bool keymap_auto_more;


/**
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to true before calling this function, will have an effect
 * on this function, and which are always reset to false by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is true, then all pending keypresses will be flushed.
 * This is set by flush(), which doesn't actually flush anything itself
 * but uses that flag to trigger delayed flushing.
 *
 * If "inkey_scan" is true, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_flag" is true, then we are waiting for a command in the main
 * map interface, and we shouldn't show a cursor.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
ui_event inkey_ex(void)
{
	bool cursor_state;
	ui_event kk;
	ui_event ke = EVENT_EMPTY;

	bool done = false;

	term *old = Term;

	/* Delayed flush */
	if (inkey_xtra) {
		Term_flush();
		inkey_next = NULL;
		inkey_xtra = false;
	}

	/* Hack -- Use the "inkey_next" pointer */
	while (inkey_next && inkey_next->code) {
		/* Get next character, and advance */
		ke.key = *inkey_next++;

		/* Cancel the various "global parameters" */
		inkey_flag = false;
		inkey_scan = 0;

		/* Peek at the key, and see if we want to skip more prompts */
		if (ke.key.code == '(') {
			keymap_auto_more = true;
			/* Since we are not returning this char, make sure the
			 * next key below works well */
			if (!inkey_next || !inkey_next->code) {
				ke.type = EVT_NONE;
				break;
			}
			continue;
		} else if (ke.key.code == ')') {
			keymap_auto_more = false;
			/* Since we are not returning this char, make sure the
			 * next key below works well */
			if (!inkey_next || !inkey_next->code) {
				ke.type = EVT_NONE;
				break;
			}
			continue;
		}

		/* Accept result */
		return (ke);
	}

	/* make sure that the flag to skip more prompts is off */
	keymap_auto_more = false;

	/* Forget pointer */
	inkey_next = NULL;

	/* Get the cursor state */
	(void)Term_get_cursor(&cursor_state);

	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag || screen_save_depth ||
						(OPT(player, show_target) && target_sighted())))
		(void)Term_set_cursor(true);


	/* Hack -- Activate main screen */
	Term_activate(term_screen);


	/* Get a key */
	while (ke.type == EVT_NONE) {
		/* Hack -- Handle "inkey_scan == SCAN_INSTANT */
		if (inkey_scan == SCAN_INSTANT &&
			(0 != Term_inkey(&kk, false, false)))
			break;


		/* Hack -- Flush output once when no key ready */
		if (!done && (0 != Term_inkey(&kk, false, false))) {
			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate main screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			character_saved = false;

			/* Mega-Hack -- reset signal counter */
			signal_count = 0;

			/* Only once */
			done = true;
		}


		/* Get a key (see above) */
		ke = inkey_aux(inkey_scan);

		if (inkey_scan && ke.type == EVT_NONE)
			/* The keypress timed out. We need to stop here. */
			break;

		/* Treat back-quote as escape */
		if (ke.key.code == '`')
			ke.key.code = ESCAPE;
	}

	/* Hack -- restore the term */
	Term_activate(old);

	/* Restore the cursor */
	Term_set_cursor(cursor_state);

	/* Cancel the various "global parameters" */
	inkey_flag = false;
	inkey_scan = 0;

	/* Return the keypress */
	return (ke);
}


/**
 * Get a keypress or mouse click from the user and ignore it.
 */
void anykey(void)
{
	ui_event ke = EVENT_EMPTY;
  
	/* Only accept a keypress or mouse click */
	while (ke.type != EVT_MOUSE && ke.type != EVT_KBRD)
		ke = inkey_ex();
}

/**
 * Get a "keypress" from the user.
 */
struct keypress inkey(void)
{
	ui_event ke = EVENT_EMPTY;

	while (ke.type != EVT_ESCAPE && ke.type != EVT_KBRD &&
		   ke.type != EVT_MOUSE && ke.type != EVT_BUTTON)
		ke = inkey_ex();

	/* Make the event a keypress */
	if (ke.type == EVT_ESCAPE) {
		ke.type = EVT_KBRD;
		ke.key.code = ESCAPE;
		ke.key.mods = 0;
	} else if (ke.type == EVT_MOUSE) {
		if (ke.mouse.button == 1) {
			ke.type = EVT_KBRD;
			ke.key.code = '\n';
			ke.key.mods = 0;
		} else {
			ke.type = EVT_KBRD;
			ke.key.code = ESCAPE;
			ke.key.mods = 0;
		}
	} else if (ke.type == EVT_BUTTON) {
		ke.type = EVT_KBRD;
	}

	return ke.key;
}

/**
 * Get a "keypress" or a "mousepress" from the user.
 * on return the event must be either a key press or a mouse press
 */
ui_event inkey_m(void)
{
	ui_event ke = EVENT_EMPTY;

	/* Only accept a keypress */
	while (ke.type != EVT_ESCAPE && ke.type != EVT_KBRD	&&
		   ke.type != EVT_MOUSE  && ke.type != EVT_BUTTON)
		ke = inkey_ex();
	if (ke.type == EVT_ESCAPE) {
		ke.type = EVT_KBRD;
		ke.key.code = ESCAPE;
		ke.key.mods = 0;
	} else if (ke.type == EVT_BUTTON) {
		ke.type = EVT_KBRD;
	}

  return ke;
}



/**
 * Hack -- flush
 */
static void msg_flush(int x)
{
	uint8_t a = COLOUR_L_BLUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	if ((!OPT(player, auto_more)) && !keymap_auto_more)
		anykey();

	/* Clear the line */
	Term_erase(0, 0, 255);
}

static int message_column = 0;


/**
 * Player has pending message
 */
bool msg_flag;

/**
 * Output a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do a "Term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to "erase" any
 * "pending" messages still on the screen, instead of using "msg_flush()".
 * This should only be done when the user is known to have read the message.
 *
 * We must be very careful about using the "msg("%s", )" functions without
 * explicitly calling the special "msg("%s", NULL)" function, since this may
 * result in the loss of information if the screen is cleared, or if anything
 * is displayed on the top line.
 *
 * Hack -- Note that "msg("%s", NULL)" will clear the top line even if no
 * messages are pending.
 */
void display_message(game_event_type unused, game_event_data *data, void *user)
{
	int n;
	char *t;
	char buf[1024];
	uint8_t color;
	int w, h;

	int type;
	const char *msg;

	if (!data) return;

	type = data->message.type;
	msg = data->message.msg;

	if (Term && type == MSG_BELL) {
		Term_xtra(TERM_XTRA_NOISE, 0);
		return;
	}

	if (!msg || !Term || !character_generated)
		return;

	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Message Length */
	n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (message_column && (!msg || ((message_column + n) > (w - 8)))) {
		/* Flush */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = false;

		/* Reset */
		message_column = 0;
	}

	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;

	/* Copy it */
	my_strcpy(buf, msg, sizeof(buf));

	/* Analyze the buffer */
	t = buf;

	/* Get the color of the message */
	color = message_type_color(type);

	/* Split message */
	while (n > w - 1) {
		char oops;

		int check, split;

		/* Default split */
		split = w - 8;

		/* Find the rightmost split point */
		for (check = (w / 2); check < w - 8; check++)
			if (t[check] == ' ') split = check;

		/* Save the split character */
		oops = t[split];

		/* Split the message */
		t[split] = '\0';

		/* Display part of the message */
		Term_putstr(0, 0, split, color, t);

		/* Flush it */
		msg_flush(split + 1);

		/* Restore the split character */
		t[split] = oops;

		/* Insert a space */
		t[--split] = ' ';

		/* Prepare to recurse on the rest of "buf" */
		t += split; n -= split;
	}

	/* Display the tail of the message */
	Term_putstr(message_column, 0, n, color, t);

	/* Remember the message */
	msg_flag = true;

	/* Remember the position */
	message_column += n + 1;
}

/**
 * Flush the output before displaying for emphasis
 */
void bell_message(game_event_type unused, game_event_data *data, void *user)
{
	/* Flush the output */
	Term_fresh();

	display_message(unused, data, user);
	player->upkeep->redraw |= PR_MESSAGE;
}

/**
 * Print the queued messages.
 */
void message_flush(game_event_type unused, game_event_data *data, void *user)
{
	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Flush when needed */
	if (message_column) {
		/* Print pending messages */
		if (Term)
			msg_flush(message_column);

		/* Forget it */
		msg_flag = false;

		/* Reset */
		message_column = 0;
	}
}


/**
 * Clear the bottom part of the screen
 */
void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
		Term_erase(0, y, 255);
}

/**
 * The default "keypress handling function" for askfor_aux()/askfor_aux_ext(),
 * this takes the given keypress, input buffer, length, etc, and does the
 * appropriate action for that keypress, such as moving the cursor left or
 * inserting a character.
 *
 * It should return true when editing of the buffer is "complete" (e.g. on
 * the press of RETURN).
 */
bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len,
						 struct keypress keypress, bool firsttime)
{
	size_t ulen = utf8_strlen(buf);

	switch (keypress.code)
	{
		case ESCAPE:
		{
			*curs = 0;
			return true;
		}
		
		case KC_ENTER:
		{
			*curs = ulen;
			return true;
		}
		
		case ARROW_LEFT:
		{
			if (firsttime) {
				*curs = 0;
			} else if (*curs > 0) {
				(*curs)--;
			}
			break;
		}
		
		case ARROW_RIGHT:
		{
			if (firsttime) {
				*curs = ulen;
			} else if (*curs < ulen) {
				(*curs)++;
			}
			break;
		}
		
		case KC_BACKSPACE:
		case KC_DELETE:
		{
			char *ocurs, *oshift;

			/* If this is the first time round, backspace means "delete all" */
			if (firsttime) {
				buf[0] = '\0';
				*curs = 0;
				*len = 0;
				break;
			}

			/* Refuse to backspace into oblivion */
			if ((keypress.code == KC_BACKSPACE && *curs == 0) ||
				(keypress.code == KC_DELETE && *curs >= ulen))
				break;

			/*
			 * Move the string from k to nul along to the left
			 * by 1.  First, have to get offset corresponding to
			 * the cursor position.
			 */
			ocurs = utf8_fskip(buf, *curs, NULL);
			assert(ocurs);
			if (keypress.code == KC_BACKSPACE) {
				/* Get offset of the previous character. */
				oshift = utf8_rskip(ocurs, 1, buf);
				assert(oshift);
				memmove(oshift, ocurs, *len - (ocurs - buf));
				/* Decrement. */
				(*curs)--;
				*len -= ocurs - oshift;
			} else {
				/* Get offset of the next character. */
				oshift = utf8_fskip(buf + *curs, 1, NULL);
				assert(oshift);
				memmove(ocurs, oshift, *len - (oshift - buf));
				/* Decrement */
				*len -= oshift - ocurs;
			}

			/* Terminate */
			buf[*len] = '\0';

			break;
		}
		
		default:
		{
			bool atnull = (*curs == ulen);
			char encoded[5];
			size_t n_enc = 0;
			char *ocurs;

			if (keycode_isprint(keypress.code)) {
				n_enc = utf32_to_utf8(encoded,
					N_ELEMENTS(encoded), &keypress.code,
					1, NULL);
			}
			if (n_enc == 0) {
				bell();
				break;
			}

			/* Clear the buffer if this is the first time round */
			if (firsttime) {
				buf[0] = '\0';
				*curs = 0;
				*len = 0;
				atnull = 1;
			}

			/* Make sure we have enough room for the new character */
			if (*len + n_enc >= buflen) {
				break;
			}

			/* Insert the encoded character. */
			if (atnull) {
				ocurs = buf + *len;
			} else {
				ocurs = utf8_fskip(buf, *curs, NULL);
				assert(ocurs);
				/*
				 * Move the rest of the buffer along to make
				 * room.
				 */
				memmove(ocurs + n_enc, ocurs,
					*len - (ocurs - buf));
			}
			memcpy(ocurs, encoded, n_enc);

			/* Update position and length. */
			(*curs)++;
			*len += n_enc;

			/* Terminate */
			buf[*len] = '\0';

			break;
		}
	}

	/* By default, we aren't done. */
	return false;
}


/**
 * Handle a mouse event during editing of a string.  This is the default mouse
 * event handler for askfor_aux_ext().
 *
 * \param buf is the buffer with the string to be edited.
 * \param buflen is the maximum number of characters that may be stored in buf.
 * \param curs is the pointer to the position of the cursor in the buffer.
 * \param len is the pointer to position of the first null character in the
 * buffer.
 * \param mouse is a description of the mouse event to handle.
 * \param firsttime is whether or not this is the first call to the keypress or
 * mouse handler in this editing session.
 * \return zero if the editing session should continue, one if the editing
 * session should end and the current contents of the buffer be accepted, or
 * two if the editing session should end and the current contents of the buffer
 * be rejected.
 *
 * askfor_aux_mouse() is very simple.  Any mouse click terminates the editing
 * session, and if that click is with the second button, the result of the
 * editing is rejected.
 */
int askfor_aux_mouse(char *buf, size_t buflen, size_t *curs, size_t *len,
		struct mouseclick mouse, bool firsttime)
{
	return (mouse.button == 2) ? 2 : 1;
}


/**
 * Get some input at the cursor location.
 *
 * The buffer is assumed to have been initialized to a default string.
 * Note that this string is often "empty" (see below).
 *
 * The default buffer is displayed in yellow until cleared, which happens
 * on the first keypress, unless that keypress is Return.
 *
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * Return accepts the current buffer contents and returns true.
 * Escape clears the buffer and the window and returns false.
 *
 * Note that 'len' refers to the size of the buffer.  The maximum length
 * of the input is 'len-1'.
 *
 * 'keypress_h' is a pointer to a function to handle keypresses, altering
 * the input buffer, cursor position and suchlike as required.  See
 * 'askfor_aux_keypress' (the default handler if you supply NULL for
 * 'keypress_h') for an example.
 */
bool askfor_aux(char *buf, size_t len, bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool))
{
	int y, x;

	size_t k = 0;		/* Cursor position */
	size_t nul = 0;		/* Position of the null byte in the string */

	struct keypress ch = KEYPRESS_NULL;

	bool done = false;
	bool firsttime = true;

	if (keypress_h == NULL)
		keypress_h = askfor_aux_keypress;

	/* Locate the cursor */
	Term_locate(&x, &y);

	/* Paranoia */
	if ((x < 0) || (x >= 80)) x = 0;

	/* Restrict the length */
	if (x + len > 80) len = 80 - x;

	/* Truncate the default entry */
	buf[len-1] = '\0';

	/* Get the position of the null byte */
	nul = strlen(buf);

	/* Display the default answer */
	Term_erase(x, y, (int)len);
	Term_putstr(x, y, -1, COLOUR_YELLOW, buf);

	/* Process input */
	while (!done) {
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		ch = inkey();

		/* Let the keypress handler deal with the keypress */
		done = keypress_h(buf, len, &k, &nul, ch, firsttime);

		/* Update the entry */
		Term_erase(x, y, (int)len);
		Term_putstr(x, y, -1, COLOUR_WHITE, buf);

		/* Not the first time round anymore */
		firsttime = false;
	}

	/* Done */
	return (ch.code != ESCAPE);
}


/**
 * Act like askfor_aux() but allow customization of what happens with mouse
 * input.
 *
 * \param buf is the buffer with the string to edit.
 * \param len is the maximum number of characters buf can hold.
 * \param keypress_h is the function to call to handle a keypress.  It may be
 * NULL.  In that case, askfor_aux_keypress() is used.  The function takes
 * six arguments and should return whether or not to end this editing
 * session.  The first argument is the buffer with the string to be edited.  The
 * second argument is the maximum number of characters that can be stored in
 * that buffer.  The third argument is a pointer to the position of the cursor
 * in the buffer.  The fourth argument is a pointer to the position of the
 * first null character in the buffer.  The fifth argument is a description of
 * the keypress to be handled.  The sixth argument is whether or not this is
 * the first call to the keypress handler or mouse handler in this editing
 * session.
 * \param mouse_h is the function to call to handle a mouse click.  It may be
 * NULL.  In that case, askfor_aux_mouse() is used.  The function takes six
 * arguments and should either return zero (this editing should session should
 * continue), one (this editing session should end and the result in the buffer
 * be accepted), or a non-zero value other than one (this editing session should
 * end and the result in the buffer should not be accepted).  The first argument
 * is the buffer with the string to be edited.  The second argument is the
 * maximum number of characters that can be stored in that buffer.  The third
 * argument is a pointer to the position of the cursor in the buffer.  The
 * fourth argument is a pointer to the position of the first null character in
 * the buffer.  The fifth argument is a description of the keypress to be
 * handled.  The sixth argument is whether or not this is the first call to the
 * keypress handler or mouse handler in this editing session.
 */
bool askfor_aux_ext(char *buf, size_t len,
	bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool),
	int (*mouse_h)(char *, size_t, size_t *, size_t *, struct mouseclick, bool))
{
	size_t k = 0;		/* Cursor position */
	size_t nul = 0;		/* Position of the null byte in the string */
	bool firsttime = true;
	bool done = false;
	bool accepted = true;
	int y, x;

	if (keypress_h == NULL) {
		keypress_h = askfor_aux_keypress;
	}
	if (mouse_h == NULL) {
		mouse_h = askfor_aux_mouse;
	}

	/* Locate the cursor */
	Term_locate(&x, &y);

	/* Paranoia */
	if (x < 0 || x >= 80) x = 0;

	/* Restrict the length */
	if (x + len > 80) len = 80 - x;

	/* Truncate the default entry */
	buf[len-1] = '\0';

	/* Get the position of the null byte */
	nul = strlen(buf);

	/* Display the default answer */
	Term_erase(x, y, (int)len);
	Term_putstr(x, y, -1, COLOUR_YELLOW, buf);

	/* Process input */
	while (!done) {
		ui_event in;

		/* Place cursor */
		Term_gotoxy(x + k, y);

		/*
		 * Get input.  Emulate what inkey() does without the coercing
		 * mouse events to look like keystrokes.
		 */
		while (1) {
			in = inkey_ex();
			if (in.type == EVT_KBRD || in.type == EVT_MOUSE) {
				break;
			}
			if (in.type == EVT_BUTTON) {
				in.type = EVT_KBRD;
				break;
			}
			if (in.type == EVT_ESCAPE) {
				in.type = EVT_KBRD;
				in.key.code = ESCAPE;
				in.key.mods = 0;
				break;
			}
		}

		/* Pass on to the appropriate handler. */
		if (in.type == EVT_KBRD) {
			done = keypress_h(buf, len, &k, &nul, in.key,
				firsttime);
			accepted = (in.key.code != ESCAPE);
		} else if (in.type == EVT_MOUSE) {
			int result = mouse_h(buf, len, &k, &nul, in.mouse,
				firsttime);

			if (result != 0) {
				done = true;
				accepted = (result == 1);
			}
		}

		/* Update the entry */
		Term_erase(x, y, (int)len);
		Term_putstr(x, y, -1, COLOUR_WHITE, buf);

		/* Not the first time round anymore */
		firsttime = false;
	}

	return accepted;
}


/**
 * A "keypress" handling function for askfor_aux, that handles the special
 * case of '*' for a new random "name" and passes any other "keypress"
 * through to the default "editing" handler.
 */
static bool get_name_keypress(char *buf, size_t buflen, size_t *curs,
							  size_t *len, struct keypress keypress,
							  bool firsttime)
{
	bool result;

	switch (keypress.code)
	{
		case '*':
		{
			*len = player_random_name(buf, buflen);
			*curs = 0;
			result = false;
			break;
		}

		default:
		{
			result = askfor_aux_keypress(buf, buflen, curs, len, keypress,
										 firsttime);
			break;
		}
	}

	return result;
}


/**
 * Handle a mouse event during editing of a string:  presents a context menu
 * with options appropriate for handling editing a character's name.
 *
 * \param buf is the buffer with the string to be edited.
 * \param buflen is the maximum number of characters that may be stored in buf.
 * \param curs is the pointer to the position of the cursor in the buffer.
 * \param len is the pointer to position of the first null character in the
 * buffer.
 * \param mouse is a description of the mouse event to handle.
 * \param firsttime is whether or not this is the first call to the keypress or
 * mouse handler in this editing session.
 * \return zero if the editing session should continue, one if the editing
 * session should end and the current contents of the buffer be accepted, or
 * two if the editing session should end and the current contents of the buffer
 * be rejected.
 */
static int handle_name_mouse(char *buf, size_t buflen, size_t *curs,
		size_t *len, struct mouseclick mouse, bool firsttime)
{
	enum { ACT_CTX_NAME_ACCEPT, ACT_CTX_NAME_RANDOM, ACT_CTX_NAME_CLEAR };
	int result = 2;
	char *labels;
	struct menu *m;
	int action;

	/*
	 * A mouse click with the second button ends the editing session and
	 * indicates that the result of editing should be rejected.
	 */
	if (mouse.button == 2) {
		return result;
	}

	/* By default, don't end the editing session. */
	result = 0;

	/* Present a context menu with the possible actions. */
	labels = string_make(lower_case);
	m = menu_dynamic_new();

	m->selections = labels;
	menu_dynamic_add_label(m, "Accept", 'a', ACT_CTX_NAME_ACCEPT, labels);
	menu_dynamic_add_label(m, "Set to random name", 'r',
		ACT_CTX_NAME_RANDOM, labels);
	menu_dynamic_add_label(m, "Clear name", 'c', ACT_CTX_NAME_CLEAR,
		labels);

	screen_save();

	menu_dynamic_calc_location(m, mouse.x, mouse.y);
	region_erase_bordered(&m->boundary);

	action = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	/* Do what was requested. */
	switch (action) {
	case ACT_CTX_NAME_ACCEPT:
		/* End the editing session and accept the result. */
		result = 1;
		break;

	case ACT_CTX_NAME_RANDOM:
		*len = player_random_name(buf, buflen);
		*curs = 0;
		break;

	case ACT_CTX_NAME_CLEAR:
		assert(buflen > 0);
		buf[0] = '\0';
		*len = 0;
		*curs = 0;
		break;
	}

	return result;
}


/**
 * Gets a name for the character, reacting to name changes.
 *
 * If sf is true, we change the savefile name depending on the character name.
 */
bool get_character_name(char *buf, size_t buflen)
{
	bool res;

	/* Paranoia */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Display prompt */
	prt("Enter a name for your character (* for a random name): ", 0, 0);

	/* Save the player name */
	my_strcpy(buf, player->full_name, buflen);

	/* Ask the user for a string */
	res = askfor_aux_ext(buf, buflen, get_name_keypress, handle_name_mouse);

	/* Clear prompt */
	prt("", 0, 0);

	/* Revert to the old name if the player doesn't pick a new one. */
	if (!res)
		my_strcpy(buf, player->full_name, buflen);

	return res;
}



/**
 * Prompt for a string from the user.
 *
 * The "prompt" should take the form "Prompt: ".
 *
 * See "askfor_aux" for some notes about "buf" and "len", and about
 * the return value of this function.
 */
static bool textui_get_string(const char *prompt, char *buf, size_t len)
{
	bool res;

	/* Paranoia */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len, NULL);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}



/**
 * Request a "quantity" from the user
 */
static int textui_get_quantity(const char *prompt, int max)
{
	int amt = 1;

	/* Prompt if needed */
	if (max != 1) {
		char tmp[80];
		char buf[80];

		/* Build a prompt if needed */
		if (!prompt) {
			/* Build a prompt */
			strnfmt(tmp, sizeof(tmp), "Quantity (0-%d, *=all): ", max);

			/* Use that prompt */
			prompt = tmp;
		}

		/* Build the default */
		strnfmt(buf, sizeof(buf), "%d", amt);

		/* Ask for a quantity */
		if (!get_string(prompt, buf, 7)) return (0);

		/* Extract a number */
		amt = atoi(buf);

		/* A star or letter means "all" */
		if ((buf[0] == '*') || isalpha((unsigned char)buf[0])) amt = max;
	}

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	/* Return the result */
	return (amt);
}


/**
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
static bool textui_get_check(const char *prompt)
{
	ui_event ke;

	char buf[80];

	/*
	 * Hack -- Build a "useful" prompt; do this first so prompts built by
	 * format() won't run afoul of event_signal()'s side effects.
	 */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* Paranoia */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Prompt for it */
	prt(buf, 0, 0);
	ke = inkey_m();

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if (ke.type == EVT_MOUSE) {
		if ((ke.mouse.button != 1) && (ke.mouse.y != 0))
			return (false);
	} else {
		if ((ke.key.code != 'Y') && (ke.key.code != 'y'))
			return (false);
	}

	/* Success */
	return (true);
}

/* TODO: refactor get_check() in terms of get_char() */
/**
 * Ask the user to respond with a character. Options is a constant string,
 * e.g. "yns"; len is the length of the constant string, and fallback should
 * be the default answer if the user hits escape or an invalid key.
 *
 * Example: get_char("Study? ", "yns", 3, 'n')
 *     This prompts "Study? [yns]" and defaults to 'n'.
 *
 */
char get_char(const char *prompt, const char *options, size_t len, char fallback)
{
	struct keypress key;
	char buf[80];

	/* Paranoia */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[%s] ", prompt, options);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	key = inkey();

	/* Lowercase answer if necessary */
	if (key.code >= 'A' && key.code <= 'Z') key.code += 32;

	/* See if key is in our options string */
	if (!strchr(options, (char)key.code))
		key.code = fallback;

	/* Erase the prompt */
	prt("", 0, 0);

	/* Success */
	return key.code;
}


/**
 * Text-native way of getting a filename.
 */
static bool get_file_text(const char *suggested_name, char *path, size_t len)
{
	char buf[160];

	/* Get filename */
	my_strcpy(buf, suggested_name, sizeof buf);
	
	if (!arg_force_name) {
			
			if (!get_string("File name: ", buf, sizeof buf)) return false;

			/* Make sure it's actually a filename */
			if (buf[0] == '\0' || buf[0] == ' ') return false;
	} else {
		int old_len;
		time_t ltime;
		struct tm *today;

		/* Get the current time */
		time(&ltime);
		today = localtime(&ltime);

		prt("File name: ", 0,0);

		/* Overwrite the ".txt" that was added */
		assert(strlen(buf) >= 4);
		old_len = strlen(buf) - 4;
		strftime(buf + old_len, sizeof(buf) - len, "-%Y-%m-%d-%H-%M.txt", today);

		/* Prompt the user to confirm or cancel the file dump */
		if (!get_check(format("Confirm writing to %s? ", buf))) return false;


	}

	/* Build the path */
	path_build(path, len, ANGBAND_DIR_USER, buf);

	/* Check if it already exists */
	if (file_exists(path) && !get_check("Replace existing file? "))
		return false;

	/* Tell the user where it's saved to. */
	prt(format("Saving as %s.", path), 0, 0);
	anykey();
	prt("", 0, 0);

	return true;
}




/**
 * Get a pathname to save a file to, given the suggested name.  Returns the
 * result in "path".
 */
bool (*get_file)(const char *suggested_name, char *path, size_t len) = get_file_text;




/**
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 * -------
 * Warning - this function assumes that the entered command is an ASCII
 *           character, and so should be used with great caution - NRM
 * -------
 * Returns true unless the character is "Escape"
 */
static bool textui_get_com(const char *prompt, char *command)
{
	ui_event ke;
	bool result;

	result = get_com_ex(prompt, &ke);
	*command = (char)ke.key.code;

	return result;
}


bool get_com_ex(const char *prompt, ui_event *command)
{
	ui_event ke;

	/* Paranoia XXX XXX XXX */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	ke = inkey_m();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Save the command */
	*command = ke;

	/* Done */
	if ((ke.type == EVT_KBRD && ke.key.code != ESCAPE) ||
		(ke.type == EVT_MOUSE))
		return true;
	else
		return false;
}


/**
 * Pause for user response
 *
 * This function is stupid.  XXX XXX XXX
 */
void pause_line(struct term *tm)
{
	prt("", tm->hgt - 1, 0);
	put_str("[Press any key to continue]", tm->hgt - 1, (tm->wid - 27) / 2);
	(void)anykey();
	prt("", tm->hgt - 1, 0);
}

static int dir_transitions[10][10] =
{
	/* 0-> */ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
	/* 1-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 2-> */ { 0, 0, 2, 0, 1, 0, 3, 0, 5, 0 },
	/* 3-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 4-> */ { 0, 0, 1, 0, 4, 0, 5, 0, 7, 0 },
	/* 5-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 6-> */ { 0, 0, 3, 0, 5, 0, 6, 0, 9, 0 },
	/* 7-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 8-> */ { 0, 0, 5, 0, 7, 0, 9, 0, 8, 0 },
	/* 9-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

/**
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user.
 *
 * Return true if a direction was chosen, otherwise return false.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player.
 *
 * Directions "5" and "0" are illegal and will not be accepted.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", if it is set.
 */
static bool textui_get_rep_dir(int *dp, bool allow_5)
{
	int dir = 0;

	ui_event ke;

	/* Initialize */
	(*dp) = 0;

	/* Get a direction */
	while (!dir) {
		/* Paranoia*/
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Get first keypress - the first test is to avoid displaying the
		 * prompt for direction if there's already a keypress queued up
		 * and waiting - this just avoids a flickering prompt if there is
		 * a "lazy" movement delay. */
		inkey_scan = SCAN_INSTANT;
		ke = inkey_ex();
		inkey_scan = SCAN_OFF;

		if (ke.type == EVT_NONE ||
			(ke.type == EVT_KBRD && target_dir(ke.key) == 0)) {
			prt("Direction or <click> (Escape to cancel)? ", 0, 0);
			ke = inkey_ex();
		}

		/* Check mouse coordinates, or get keypresses until a dir is chosen */
		if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				int y = KEY_GRID_Y(ke);
				int x = KEY_GRID_X(ke);
				struct loc from = player->grid;
				struct loc to = loc(x, y);

				dir = pathfind_direction_to(from, to);
			} else if (ke.mouse.button == 2) {
				/* Clear the prompt */
				prt("", 0, 0);

				return (false);
			}
		} else if (ke.type == EVT_KBRD) {
			int keypresses_handled = 0;

			while (ke.type == EVT_KBRD && ke.key.code != 0) {
				int this_dir;

				if (ke.key.code == ESCAPE) {
					/* Clear the prompt */
					prt("", 0, 0);

					return (false);
				}

				/* XXX Ideally show and move the cursor here to indicate
				 the currently "Pending" direction. XXX */
				this_dir = target_dir_allow(ke.key, allow_5);

				if (this_dir)
					dir = dir_transitions[dir][this_dir];

				if (player->opts.lazymove_delay == 0 || ++keypresses_handled > 1)
					break;

				inkey_scan = player->opts.lazymove_delay;
				ke = inkey_ex();
			}

			/* 5 is equivalent to "escape" */
			if (dir == 5 && !allow_5) {
				/* Clear the prompt */
				prt("", 0, 0);

				return (false);
			}
		}

		/* Oops */
		if (!dir) bell();
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Save direction */
	(*dp) = dir;

	/* Success */
	return (true);
}

/**
 * Get an "aiming direction" (1,2,3,4,6,7,8,9 or 5) from the user.
 *
 * Return true if a direction was chosen, otherwise return false.
 *
 * The direction "5" is special, and means "use current target".
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", if it is set.
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 */
static bool textui_get_aim_dir(int *dp)
{
	/* Global direction */
	int dir = 0;

	ui_event ke;

	const char *p;

	/* Initialize */
	(*dp) = 0;

	/* Hack -- auto-target if requested */
	if (OPT(player, use_old_target) && target_okay() && !dir) dir = 5;

	/* Ask until satisfied */
	while (!dir) {
		/* Choose a prompt */
		if (!target_okay())
			p = "Direction ('*' or <click> to target, \"'\" for closest, Escape to cancel)? ";
		else
			p = "Direction ('5' for target, '*' or <click> to re-target, Escape to cancel)? ";

		/* Get a command (or Cancel) */
		if (!get_com_ex(p, &ke)) break;

		if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				if (target_set_interactive(TARGET_KILL, KEY_GRID_X(ke),
										   KEY_GRID_Y(ke)))
					dir = 5;
			} else if (ke.mouse.button == 2) {
				break;
			}
		} else if (ke.type == EVT_KBRD) {
			if (ke.key.code == '*') {
				/* Set new target, use target if legal */
				if (target_set_interactive(TARGET_KILL, -1, -1))
					dir = 5;
			} else if (ke.key.code == '\'') {
				/* Set to closest target */
				if (target_set_closest(TARGET_KILL, NULL))
					dir = 5;
			} else if (ke.key.code == 't' || ke.key.code == '5' ||
					   ke.key.code == '0' || ke.key.code == '.') {
				if (target_okay())
					dir = 5;
			} else {
				/* Possible direction */
				int keypresses_handled = 0;

				while (ke.key.code != 0){
					int this_dir;

					/* XXX Ideally show and move the cursor here to indicate
					 * the currently "Pending" direction. XXX */
					this_dir = target_dir(ke.key);

					if (this_dir)
						dir = dir_transitions[dir][this_dir];
					else
						break;

					if (player->opts.lazymove_delay == 0 || ++keypresses_handled > 1)
						break;

					/* See if there's a second keypress within the defined
					 * period of time. */
					inkey_scan = player->opts.lazymove_delay;
					ke = inkey_ex();
				}
			}
		}

		/* Error */
		if (!dir) bell();
	}

	/* No direction */
	if (!dir) return (false);
	
	/* Save direction */
	(*dp) = dir;
	
	/* A "valid" direction was entered */
	return (true);
}

/**
 * Initialise the UI hooks to give input asked for by the game
 */
void textui_input_init(void)
{
	get_string_hook = textui_get_string;
	get_quantity_hook = textui_get_quantity;
	get_check_hook = textui_get_check;
	get_com_hook = textui_get_com;
	get_rep_dir_hook = textui_get_rep_dir;
	get_aim_dir_hook = textui_get_aim_dir;
	get_spell_from_book_hook = textui_get_spell_from_book;
	get_spell_hook = textui_get_spell;
	get_effect_from_list_hook = textui_get_effect_from_list;
	get_item_hook = textui_get_item;
	get_curse_hook = textui_get_curse;
	get_panel_hook = textui_get_panel;
	panel_contains_hook = textui_panel_contains;
	map_is_visible_hook = textui_map_is_visible;
	view_abilities_hook = textui_view_ability_menu;
}


/*** Input processing ***/


/**
 * Get a command count, with the '0' key.
 */
static int textui_get_count(void)
{
	int count = 0;

	while (1) {
		struct keypress ke;

		prt(format("Repeat: %d", count), 0, 0);

		ke = inkey();
		if (ke.code == ESCAPE)
			return -1;

		/* Simple editing (delete or backspace) */
		else if (ke.code == KC_DELETE || ke.code == KC_BACKSPACE)
			count = count / 10;

		/* Actual numeric data */
		else if (isdigit((unsigned char) ke.code)) {
			count = count * 10 + D2I(ke.code);

			if (count >= 9999) {
				bell();
				count = 9999;
			}
		} else {
			/* Anything non-numeric passes straight to command input */
			/* XXX nasty hardcoding of action menu key */
			if (ke.code != KC_ENTER)
				Term_keypress(ke.code, ke.mods);

			break;
		}
	}

	return count;
}



/**
 * Hack -- special buffer to hold the action of the current keymap
 */
static struct keypress request_command_buffer[256];


/**
 * Request a command from the user.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character.  This is useful for macros.
 */
ui_event textui_get_command(int *count)
{
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	struct keypress tmp[2] = { KEYPRESS_NULL, KEYPRESS_NULL };

	ui_event ke = EVENT_EMPTY;

	const struct keypress *act = NULL;



	/* Get command */
	while (1) {
		/* Hack -- no flush needed */
		msg_flag = false;

		/* Activate "command mode" */
		inkey_flag = true;

		/* Toggle on cursor if requested */
		if (OPT(player, highlight_player)) {
			Term_set_cursor(true);
			move_cursor_relative(player->grid.y, player->grid.x);
		}

		/* Get a command */
		ke = inkey_ex();

		/* Toggle off cursor */
		if (OPT(player, highlight_player)) {
			Term_set_cursor(false);
		}

		if (ke.type == EVT_KBRD) {
			bool keymap_ok = true;
			switch (ke.key.code) {
				case '0': {
					if(ke.key.mods & KC_MOD_KEYPAD) break;

					int c = textui_get_count();

					if (c == -1 || !get_com_ex("Command: ", &ke))
						continue;
					else
						*count = c;
					break;
				}

				case '\\': {
					/* Allow keymaps to be bypassed */
					(void)get_com_ex("Command: ", &ke);
					keymap_ok = false;
					break;
				}

				case '^': {
					char ch;
					/* Allow "control chars" to be entered */
					if (get_com("Control: ", &ch))
						ke.key.code = KTRL(ch);
					break;
				}
			}

			/* Find any relevant keymap */
			if (keymap_ok)
				act = keymap_find(mode, ke.key);
		}

		/* Erase the message line */
		prt("", 0, 0);

		if (ke.type == EVT_BUTTON) {
			/* Buttons are always specified in standard keyset */
			act = tmp;
			tmp[0] = ke.key;
		}

		/* Apply keymap if not inside a keymap already */
		if (ke.key.code && act && !inkey_next) {
			size_t n = 0;
			while (act[n].type)
				n++;

			/* Make room for the terminator */
			n += 1;

			/* Install the keymap */
			memcpy(request_command_buffer, act, n * sizeof(struct keypress));

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}

		/* Done */
		break;
	}

	return ke;
}

/**
 * Check no currently worn items are stopping the action 'c'
 */
bool key_confirm_command(unsigned char c)
{
	int i;

	/* Hack -- Scan equipment */
	for (i = 0; i < player->body.count; i++) {
		char verify_inscrip[] = "^*";
		unsigned n;

		struct object *obj = slot_object(player, i);
		if (!obj) continue;

		/* Set up string to look for, e.g. "^d" */
		verify_inscrip[1] = c;

		/* Verify command */
		n = check_for_inscrip(obj, "^*") +
				check_for_inscrip(obj, verify_inscrip);
		while (n--) {
			if (!get_check("Are you sure? "))
				return false;
		}
	}

	return true;
}


/**
 * Process a textui keypress.
 */
bool textui_process_key(struct keypress kp, unsigned char *c, int count)
{
	keycode_t key = kp.code;

	/* Null command */
	if (key == '\0' || key == ESCAPE || key == ' ' || key == '\a')
		return true;

	/* Invalid keypress */
	if (key > UCHAR_MAX)
		return false;

	*c = key;
	return true;
}
