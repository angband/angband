/* File: util.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "randname.h"



/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(int i)
{
	return (hexsym[i % 16]);
}



/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
	if (isdigit((unsigned char)c)) return (D2I(c));
	if (isalpha((unsigned char)c)) return (A2I(tolower((unsigned char)c)) + 10);
	return (0);
}


/*
 * Transform macro trigger name ('\[alt-D]' etc..)
 * into macro trigger key code ('^_O_64\r' or etc..)
 */
static size_t trigger_text_to_ascii(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	bool mod_status[MAX_MACRO_MOD];

	int i, len = 0;
	int shiftstatus = 0;
	cptr key_code;
	
	size_t current_len = strlen(buf);

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Initialize modifier key status */	
	for (i = 0; macro_modifier_chr[i]; i++)
		mod_status[i] = FALSE;

	str++;

	/* Examine modifier keys */
	while (1)
	{
		/* Look for modifier key name */
		for (i = 0; macro_modifier_chr[i]; i++)
		{
			len = strlen(macro_modifier_name[i]);

			if (!my_strnicmp(str, macro_modifier_name[i], len))
				break;
		}

		/* None found? */
		if (!macro_modifier_chr[i]) break;

		/* Proceed */
		str += len;

		/* This modifier key is pressed */
		mod_status[i] = TRUE;

		/* Shift key might be going to change keycode */
		if ('S' == macro_modifier_chr[i])
			shiftstatus = 1;
	}

	/* Look for trigger name */
	for (i = 0; i < max_macrotrigger; i++)
	{
		len = strlen(macro_trigger_name[i]);

		/* Found it and it is ending with ']' */
		if (!my_strnicmp(str, macro_trigger_name[i], len) && (']' == str[len]))
			break;
	}

	/* Invalid trigger name? */
	if (i == max_macrotrigger)
	{
		/*
		 * If this invalid trigger name is ending with ']',
		 * skip whole of it to avoid defining strange macro trigger
		 */
		str = strchr(str, ']');

		if (str)
		{
			strnfcat(buf, max, &current_len, "\x1F\r");

			*strptr = str; /* where **strptr == ']' */
		}

		return current_len;
	}

	/* Get keycode for this trigger name */
	key_code = macro_trigger_keycode[shiftstatus][i];

	/* Proceed */
	str += len;

	/* Begin with '^_' */
	strnfcat(buf, max, &current_len, "\x1F");

	/* Write key code style trigger using template */
	for (i = 0; macro_template[i]; i++)
	{
		char ch = macro_template[i];
		int j;

		switch(ch)
		{
		case '&':
			/* Modifier key character */
			for (j = 0; macro_modifier_chr[j]; j++)
			{
				if (mod_status[j])
					strnfcat(buf, max, &current_len, "%c", macro_modifier_chr[j]);
			}
			break;
		case '#':
			/* Key code */
			strnfcat(buf, max, &current_len, "%s", key_code);
			break;
		default:
			/* Fixed string */
			strnfcat(buf, max, &current_len, "%c", ch);
			break;
		}
	}

	/* End with '\r' */
	strnfcat(buf, max, &current_len, "\r");

	/* Succeed */
	*strptr = str; /* where **strptr == ']' */
	
	return current_len;
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * This function will not work on non-ascii systems.
 *
 * To be safe, "buf" should be at least as large as "str".
 */
void text_to_ascii(char *buf, size_t len, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		/* Check if the buffer is long enough */
		if (s >= buf + len - 1) break;

		/* Backslash codes */
		if (*str == '\\')
		{
			/* Skip the backslash */
			str++;

			/* Paranoia */
			if (!(*str)) break;

			/* Macro Trigger */
			if (*str == '[')
			{
				/* Terminate before appending the trigger */
				*s = '\0';

				s += trigger_text_to_ascii(buf, len, &str);
			}

			/* Hack -- simple way to specify Escape */
			else if (*str == 'e')
			{
				*s++ = ESCAPE;
			}

			/* Hack -- simple way to specify "space" */
			else if (*str == 's')
			{
				*s++ = ' ';
			}

			/* Backspace */
			else if (*str == 'b')
			{
				*s++ = '\b';
			}

			/* Newline */
			else if (*str == 'n')
			{
				*s++ = '\n';
			}

			/* Return */
			else if (*str == 'r')
			{
				*s++ = '\r';
			}

			/* Tab */
			else if (*str == 't')
			{
				*s++ = '\t';
			}

			/* Bell */
			else if (*str == 'a')
			{
				*s++ = '\a';
			}

			/* Actual "backslash" */
			else if (*str == '\\')
			{
				*s++ = '\\';
			}

			/* Hack -- Actual "caret" */
			else if (*str == '^')
			{
				*s++ = '^';
			}

			/* Hack -- Hex-mode */
			else if (*str == 'x')
			{
				if (isxdigit((unsigned char)(*(str + 1))) &&
				    isxdigit((unsigned char)(*(str + 2))))
				{
					*s = 16 * dehex(*++str);
					*s++ += dehex(*++str);
				}
				else
				{
					/* HACK - Invalid hex number */
					*s++ = '?';
				}
			}

			/* Oops */
			else
			{
				*s = *str;
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;

			if (*str)
			{
				*s++ = KTRL(*str);
				str++;
			}
		}

		/* Normal chars */
		else
		{
			*s++ = *str++;
		}
	}

	/* Terminate */
	*s = '\0';
}


/*
 * Transform macro trigger key code ('^_O_64\r' or etc..) 
 * into macro trigger name ('\[alt-D]' etc..)
 */
static size_t trigger_ascii_to_text(char *buf, size_t max, cptr *strptr)
{
	cptr str = *strptr;
	char key_code[100];
	int i;
	cptr tmp;
	size_t current_len = strlen(buf);
	

	/* No definition of trigger names */
	if (macro_template == NULL) return 0;

	/* Trigger name will be written as '\[name]' */
	strnfcat(buf, max, &current_len, "\\[");

	/* Use template to read key-code style trigger */
	for (i = 0; macro_template[i]; i++)
	{
		int j;
		char ch = macro_template[i];

		switch(ch)
		{
		case '&':
			/* Read modifier */
			while ((tmp = strchr(macro_modifier_chr, *str)))
			{
				j = (int)(tmp - macro_modifier_chr);
				strnfcat(buf, max, &current_len, "%s", macro_modifier_name[j]);
				str++;
			}
			break;
		case '#':
			/* Read key code */
			for (j = 0; *str && (*str != '\r') && (j < (int)sizeof(key_code) - 1); j++)
				key_code[j] = *str++;
			key_code[j] = '\0';
			break;
		default:
			/* Skip fixed strings */
			if (ch != *str) return 0;
			str++;
		}
	}

	/* Key code style triggers always end with '\r' */
	if (*str++ != '\r') return 0;

	/* Look for trigger name with given keycode (normal or shifted keycode) */
	for (i = 0; i < max_macrotrigger; i++)
	{
		if (!my_stricmp(key_code, macro_trigger_keycode[0][i]) ||
		    !my_stricmp(key_code, macro_trigger_keycode[1][i]))
			break;
	}

	/* Not found? */
	if (i == max_macrotrigger) return 0;

	/* Write trigger name + "]" */
	strnfcat(buf, max, &current_len, "%s]", macro_trigger_name[i]);

	/* Succeed */
	*strptr = str;
	return current_len;
}


/*
 * Hack -- convert a string into a printable form
 *
 * This function will not work on non-ascii systems.
 */
void ascii_to_text(char *buf, size_t len, cptr str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		/* Check if the buffer is long enough */
		/* HACK - always assume worst case (hex-value + '\0') */
		if (s >= buf + len - 5) break;

		if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\a')
		{
			*s++ = '\\';
			*s++ = 'a';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		/* Macro Trigger */
		else if (i == 31)
		{
			size_t offset;

			/* Terminate before appending the trigger */
			*s = '\0';

			offset = trigger_ascii_to_text(buf, len, &str);
			
			if (offset == 0)
			{
				/* No trigger found */
				*s++ = '^';
				*s++ = '_';
			}
			else
				s += offset;
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = UN_KTRL(i);
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify((int)i / 16);
			*s++ = hexify((int)i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}



/*
 * The "macro" package
 *
 * Functions are provided to manipulate a collection of macros, each
 * of which has a trigger pattern string and a resulting action string
 * and a small set of flags.
 */



/*
 * Determine if any macros have ever started with a given character.
 */
static bool macro__use[256];


/*
 * Find the macro (if any) which exactly matches the given pattern
 */
int macro_find_exact(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return (-1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not match the pattern */
		if (!streq(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* No matches */
	return (-1);
}


/*
 * Find the first macro (if any) which contains the given pattern
 */
static int macro_find_check(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return (-1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not contain the pattern */
		if (!prefix(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* Nothing */
	return (-1);
}



/*
 * Find the first macro (if any) which contains the given pattern and more
 */
static int macro_find_maybe(cptr pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return (-1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not contain the pattern */
		if (!prefix(macro__pat[i], pat)) continue;

		/* Skip macros which exactly match the pattern XXX XXX */
		if (streq(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* Nothing */
	return (-1);
}


/*
 * Find the longest macro (if any) which starts with the given pattern
 */
static int macro_find_ready(cptr pat)
{
	int i, t, n = -1, s = -1;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return (-1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which are not contained by the pattern */
		if (!prefix(pat, macro__pat[i])) continue;

		/* Obtain the length of this macro */
		t = strlen(macro__pat[i]);

		/* Only track the "longest" pattern */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Result */
	return (n);
}


/*
 * Add a macro definition (or redefinition).
 *
 * We should use "act == NULL" to "remove" a macro, but this might make it
 * impossible to save the "removal" of a macro definition.  XXX XXX XXX
 *
 * We should consider refusing to allow macros which contain existing macros,
 * or which are contained in existing macros, because this would simplify the
 * macro analysis code.  XXX XXX XXX
 *
 * We should consider removing the "command macro" crap, and replacing it
 * with some kind of "powerful keymap" ability, but this might make it hard
 * to change the "roguelike" option from inside the game.  XXX XXX XXX
 */
errr macro_add(cptr pat, cptr act)
{
	int n;


	/* Paranoia -- require data */
	if (!pat || !act) return (-1);


	/* Look for any existing macro */
	n = macro_find_exact(pat);

	/* Replace existing macro */
	if (n >= 0)
	{
		/* Free the old macro action */
		string_free(macro__act[n]);
	}

	/* Create a new macro */
	else
	{
		/* Get a new index */
		n = macro__num++;

		/* Boundary check */
		if (macro__num >= MACRO_MAX) quit("Too many macros!");

		/* Save the pattern */
		macro__pat[n] = string_make(pat);
	}

	/* Save the action */
	macro__act[n] = string_make(act);

	/* Efficiency */
	macro__use[(byte)(pat[0])] = TRUE;

	/* Success */
	return (0);
}



/*
 * Initialize the "macro" package
 */
errr macro_init(void)
{
	/* Macro patterns */
	C_MAKE(macro__pat, MACRO_MAX, cptr);

	/* Macro actions */
	C_MAKE(macro__act, MACRO_MAX, cptr);

	/* Success */
	return (0);
}


/*
 * Free the macro package
 */
errr macro_free(void)
{
	int i, j;

	/* Free the macros */
	for (i = 0; i < macro__num; ++i)
	{
		string_free(macro__pat[i]);
		string_free(macro__act[i]);
	}

	FREE((void*)macro__pat);
	FREE((void*)macro__act);

	/* Free the keymaps */
	for (i = 0; i < KEYMAP_MODES; ++i)
	{
		for (j = 0; j < (int)N_ELEMENTS(keymap_act[i]); ++j)
		{
			string_free(keymap_act[i][j]);
			keymap_act[i][j] = NULL;
		}
	}

	/* Success */
	return (0);
}


/*
 * Free the macro trigger package
 */
errr macro_trigger_free(void)
{
	int i;
	int num;

	if (macro_template != NULL)
	{
		/* Free the template */
		string_free(macro_template);
		macro_template = NULL;

		/* Free the trigger names and keycodes */
		for (i = 0; i < max_macrotrigger; i++)
		{
			string_free(macro_trigger_name[i]);

			string_free(macro_trigger_keycode[0][i]);
			string_free(macro_trigger_keycode[1][i]);
		}

		/* No more macro triggers */
		max_macrotrigger = 0;

		/* Count modifier-characters */
		num = strlen(macro_modifier_chr);

		/* Free modifier names */
		for (i = 0; i < num; i++)
		{
			string_free(macro_modifier_name[i]);
		}

		/* Free modifier chars */
		string_free(macro_modifier_chr);
		macro_modifier_chr = NULL;
	}

	/* Success */
	return (0);
}


/*
 * Flush all pending input.
 *
 * Actually, remember the flush, using the "inkey_xtra" flag, and in the
 * next call to "inkey()", perform the actual flushing, for efficiency,
 * and correctness of the "inkey()" function.
 */
void flush(void)
{
	/* Do it later */
	inkey_xtra = TRUE;
}


/*
 * Flush all pending input if the flush_failure option is set.
 */
void flush_fail(void)
{
	if (flush_failure) flush();
}


/*
 * Local variable -- we are inside a "macro action"
 *
 * Do not match any macros until "ascii 30" is found.
 */
static bool parse_macro = FALSE;


/*
 * Local variable -- we are inside a "macro trigger"
 *
 * Strip all keypresses until a low ascii value is found.
 */
static bool parse_under = FALSE;



/*
 * Helper function called only from "inkey()"
 *
 * This function does almost all of the "macro" processing.
 *
 * We use the "Term_key_push()" function to handle "failed" macros, as well
 * as "extra" keys read in while choosing the proper macro, and also to hold
 * the action for the macro, plus a special "ascii 30" character indicating
 * that any macro action in progress is complete.  Embedded macros are thus
 * illegal, unless a macro action includes an explicit "ascii 30" character,
 * which would probably be a massive hack, and might break things.
 *
 * Only 500 (0+1+2+...+29+30) milliseconds may elapse between each key in
 * the macro trigger sequence.  If a key sequence forms the "prefix" of a
 * macro trigger, 500 milliseconds must pass before the key sequence is
 * known not to be that macro trigger.  XXX XXX XXX
 */
static event_type inkey_aux(void)
{
	int k = 0, n, p = 0, w = 0;
	
	event_type ke, ke0;
	char ch;
	
	cptr pat, act;
	
	char buf[1024];
	
	/* Initialize the no return */
	ke0.type = EVT_KBRD;
	ke0.key = 0;
	ke0.index = 0; /* To fix GCC warnings on X11 */
	ke0.mousey = 0;
	ke0.mousex = 0;
 
	/* Wait for a keypress */
	(void)(Term_inkey(&ke, TRUE, TRUE));
	ch = ke.key;
	
	/* End "macro action" */
	if ((ch == 30) || (ch == '\xff'))
	{
		parse_macro = FALSE;
		return (ke);
	}
	
	/* Inside "macro action" */
	if (parse_macro) return (ke);
	
	/* Inside "macro trigger" */
	if (parse_under) return (ke);
	

	/* Save the first key, advance */
	buf[p++] = ch;
	buf[p] = '\0';
	
	
	/* Check for possible macro */
	k = macro_find_check(buf);
	
	/* No macro pending */
	if (k < 0) return (ke);
	
	
	/* Wait for a macro, or a timeout */
	while (TRUE)
	{
		/* Check for pending macro */
		k = macro_find_maybe(buf);
		
		/* No macro pending */
		if (k < 0) break;
		
		/* Check for (and remove) a pending key */
		if (0 == Term_inkey(&ke, FALSE, TRUE))
		{
			/* Append the key */
			buf[p++] = ke.key;
			buf[p] = '\0';
		
			/* Restart wait */
			w = 0;
		}
		
		/* No key ready */
		else
		{
			/* Increase "wait" */
			w += 10;
		
			/* Excessive delay */
			if (w >= 100) break;
		
			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, w);
		}
	}
	
	
	/* Check for available macro */
	k = macro_find_ready(buf);

	/* No macro available */
	if (k < 0)
	{
		/* Push all the "keys" back on the queue */
		/* The most recent event may not be a keypress. */
		if(p)
		{
			if(Term_event_push(&ke)) return (ke0);
			p--;
		}
		while (p > 0)
		{
			/* Push the key, notice over-flow */
			if (Term_key_push(buf[--p])) return (ke0);
		}
		
		/* Wait for (and remove) a pending key */
		(void)Term_inkey(&ke, TRUE, TRUE);
		
		/* Return the key */
		return (ke);
	}
	
	
	/* Get the pattern */
	pat = macro__pat[k];
	
	/* Get the length of the pattern */
	n = strlen(pat);
	
	/* Push the "extra" keys back on the queue */
	while (p > n)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(buf[--p])) return (ke0);
	}
	
	
	/* Begin "macro action" */
	parse_macro = TRUE;
	
	/* Push the "end of macro action" key */
	if (Term_key_push(30)) return (ke0);
	
	
	/* Access the macro action */
	act = macro__act[k];
	
	/* Get the length of the action */
	n = strlen(act);
	
	/* Push the macro "action" onto the key queue */
	while (n > 0)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(act[--n])) return (ke0);
	}
	
	
	/* Hack -- Force "inkey()" to call us again */
	return (ke0);
}



/*
 * Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()".  This key sequence will not
 * trigger any macros, and cannot be bypassed by the Borg.  It is used
 * in Angband to handle "keymaps".
 */
static cptr inkey_next = NULL;


#ifdef ALLOW_BORG

/*
 * Mega-Hack -- special "inkey_hack" hook.  XXX XXX XXX
 *
 * This special function hook allows the "Borg" (see elsewhere) to take
 * control of the "inkey()" function, and substitute in fake keypresses.
 */
char (*inkey_hack)(int flush_first) = NULL;

#endif /* ALLOW_BORG */


/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is TRUE, then all pending keypresses will be flushed,
 * and any macro processing in progress will be aborted.  This flag is
 * set by the "flush()" function, which does not actually flush anything
 * itself, but rather, triggers delayed input flushing via "inkey_xtra".
 *
 * If "inkey_scan" is TRUE, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_base" is TRUE, then all macro processing will be bypassed.
 * If "inkey_base" and "inkey_scan" are both TRUE, then this function will
 * not return immediately, but will wait for a keypress for as long as the
 * normal macro matching code would, allowing the direct entry of macro
 * triggers.  The "inkey_base" flag is extremely dangerous!
 *
 * If "inkey_flag" is TRUE, then we will assume that we are waiting for a
 * normal command, and we will only show the cursor if "hilite_player" is
 * TRUE (or if the player is in a store), instead of always showing the
 * cursor.  The various "main-xxx.c" files should avoid saving the game
 * in response to a "menu item" request unless "inkey_flag" is TRUE, to
 * prevent savefile corruption.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.  This is done after the
 * macro matching, so the user can still make a macro for "backquote".
 *
 * Note the special handling of "ascii 30" (ctrl-caret, aka ctrl-shift-six)
 * and "ascii 31" (ctrl-underscore, aka ctrl-shift-minus), which are used to
 * provide support for simple keyboard "macros".  These keys are so strange
 * that their loss as normal keys will probably be noticed by nobody.  The
 * "ascii 30" key is used to indicate the "end" of a macro action, which
 * allows recursive macros to be avoided.  The "ascii 31" key is used by
 * some of the "main-xxx.c" files to introduce macro trigger sequences.
 *
 * Hack -- we use "ascii 29" (ctrl-right-bracket) as a special "magic" key,
 * which can be used to give a variety of "sub-commands" which can be used
 * any time.  These sub-commands could include commands to take a picture of
 * the current screen, to start/stop recording a macro action, etc.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
event_type inkey_ex(void)
{
	bool cursor_state;
	event_type kk;
	event_type ke;
	
	bool done = FALSE;
	
	term *old = Term;
	
	
	/* Initialise keypress */
	ke.key = 0;
	ke.type = EVT_KBRD;
	
	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next && !inkey_xtra)
	{
		/* Get next character, and advance */
		ke.key = *inkey_next++;
		
		/* Cancel the various "global parameters" */
		inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
		
		/* Accept result */
		return (ke);
	}
	
	/* Forget pointer */
	inkey_next = NULL;
	
	
#ifdef ALLOW_BORG
	
	/* Mega-Hack -- Use the special hook */
	if (inkey_hack && ((ch = (*inkey_hack)(inkey_xtra)) != 0))
	{
		/* Cancel the various "global parameters" */
		inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
		ke.type = EVT_KBRD;
		
		/* Accept result */
		return (ke);
	}
	
#endif /* ALLOW_BORG */
	
	
	/* Hack -- handle delayed "flush()" */
	if (inkey_xtra)
	{
		/* End "macro action" */
		parse_macro = FALSE;
		ke.type = EVT_KBRD;
		
		/* End "macro trigger" */
		parse_under = FALSE;
		
		/* Forget old keypresses */
		Term_flush();
	}
	
	
	/* Get the cursor state */
	(void)Term_get_cursor(&cursor_state);
	
	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag || hilite_player || character_icky))
	{
		/* Show the cursor */
		(void)Term_set_cursor(TRUE);
	}
	
	
	/* Hack -- Activate main screen */
	Term_activate(term_screen);
	
	
	/* Get a key */
	while (!ke.key)
	{
		/* Hack -- Handle "inkey_scan" */
		if (!inkey_base && inkey_scan &&
		   (0 != Term_inkey(&kk, FALSE, FALSE)))
		{
			break;
		}
		

		/* Hack -- Flush output once when no key ready */
		if (!done && (0 != Term_inkey(&kk, FALSE, FALSE)))
		{

			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate main screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			character_saved = FALSE;
		
			/* Mega-Hack -- reset signal counter */
			signal_count = 0;
		
			/* Only once */
			done = TRUE;
		}
		
		
		/* Hack -- Handle "inkey_base" */
		if (inkey_base)
		{
		  int w = 0;

		  /* Wait forever */
		  if (!inkey_scan)
		  {
			/* Wait for (and remove) a pending key */
			if (0 == Term_inkey(&ke, TRUE, TRUE))
			{
				/* Done */
				break;
			}

			/* Oops */
			break;
		  }

		  /* Wait */
		  while (TRUE)
		  {
			/* Check for (and remove) a pending key */
			if (0 == Term_inkey(&ke, FALSE, TRUE))
			{
				/* Done */
				break;
			}

			/* No key ready */
			else
			{
				/* Increase "wait" */
				w += 10;
			
				/* Excessive delay */
				if (w >= 100) break;
			
				/* Delay */
				Term_xtra(TERM_XTRA_DELAY, w);
				}
		  }

		  /* Done */
		  break;
		}
		
		
			/* Get a key (see above) */
			ke = inkey_aux();
		
		
		/* Handle "control-right-bracket" */
		if (ke.key == 29)
		{
			/* Strip this key */
			ke.key = 0;
		
			/* Continue */
			continue;
		}
		
		
		/* Treat back-quote as escape */
		if (ke.key == '`') ke.key = ESCAPE;
		
		
		/* End "macro trigger" */
		if (parse_under && (ke.key <= 32))
		{
			/* Strip this key */
			ke.key = 0;
		
			/* End "macro trigger" */
			parse_under = FALSE;
		}

		/* Handle "control-caret" */
		if (ke.key == 30)
		{
			/* Strip this key */
			ke.key = 0;
		}

		/* Handle "control-underscore" */
		else if (ke.key == 31)
		{
			/* Strip this key */
			ke.key = 0;

			/* Begin "macro trigger" */
			parse_under = TRUE;
		}

		/* Inside "macro trigger" */
    	else if (parse_under)
		{
			/* Strip this key */
			ke.key = 0;
		}
	}
	
	
	/* Hack -- restore the term */
	Term_activate(old);
	
	
	/* Restore the cursor */
	Term_set_cursor(cursor_state);
	
	
	/* Cancel the various "global parameters" */
	inkey_base = inkey_xtra = inkey_flag = inkey_scan = FALSE;
	
	
	/* Return the keypress */
	return (ke);
}


/*
 * Get a keypress or mouse click from the user.
 */
char anykey(void)
{
  event_type ke;
  
  /* Only accept a keypress or mouse click*/
  do
    {
      ke = inkey_ex();
    } while (!(ke.type & (EVT_MOUSE|EVT_KBRD)));
  
  return ke.key;
}

/*
 * Get a "keypress" from the user.
 */
char inkey(void)
{
	event_type ke;

	/* Only accept a keypress */
	do
	{
		ke = inkey_ex();
	} while (!(ke.type  & (EVT_KBRD|EVT_ESCAPE)));
	/* Paranoia */
	if(ke.type == EVT_ESCAPE) ke.key = ESCAPE;

	return ke.key;
}



/*
 * Flush the screen, make a noise
 */
void bell(cptr reason)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Hack -- memorize the reason if possible */
	if (character_generated && reason)
	{
		message_add(reason, MSG_BELL);

		/* Window stuff */
		p_ptr->window |= (PW_MESSAGE);

		/* Force window redraw */
		window_stuff();
	}

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}



/*
 * Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
	/* No sound */
	if (!use_sound) return;

	/* Make a noise */
	if (sound_hook)
		sound_hook(val);
}




/*
 * The "quark" package
 *
 * This package is used to reduce the memory usage of object inscriptions.
 *
 * We use dynamic string allocation because otherwise it is necessary to
 * pre-guess the amount of quark activity.  We limit the total number of
 * quarks, but this is much easier to "expand" as needed.  XXX XXX XXX
 *
 * Two objects with the same inscription will have the same "quark" index.
 *
 * Some code uses "zero" to indicate the non-existance of a quark.
 *
 * Note that "quark zero" is NULL and should never be "dereferenced".
 *
 * ToDo: Add reference counting for quarks, so that unused quarks can
 * be overwritten.
 *
 * ToDo: Automatically resize the array if necessary.
 */


/*
 * The number of quarks (first quark is NULL)
 */
static s16b quark__num = 1;


/*
 * The array[QUARK_MAX] of pointers to the quarks
 */
static cptr *quark__str;


/*
 * Add a new "quark" to the set of quarks.
 */
s16b quark_add(cptr str)
{
	int i;

	/* Look for an existing quark */
	for (i = 1; i < quark__num; i++)
	{
		/* Check for equality */
		if (streq(quark__str[i], str)) return (i);
	}

	/* Hack -- Require room XXX XXX XXX */
	if (quark__num == QUARK_MAX) return (0);

	/* New quark */
	i = quark__num++;

	/* Add a new quark */
	quark__str[i] = string_make(str);

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

	/* Get the quark */
	q = quark__str[i];

	/* Return the quark */
	return (q);
}


/*
 * Initialize the "quark" package
 */
errr quarks_init(void)
{
	/* Quark variables */
	C_MAKE(quark__str, QUARK_MAX, cptr);

	/* Success */
	return (0);
}


/*
 * Free the "quark" package
 */
errr quarks_free(void)
{
	int i;

	/* Free the "quarks" */
	for (i = 1; i < quark__num; i++)
	{
		string_free(quark__str[i]);
	}

	/* Free the list of "quarks" */
	FREE(quark__str);

	/* Success */
	return (0);
}


/*
 * The "message memorization" package.
 *
 * Each call to "message_add(s)" will add a new "most recent" message
 * to the "message recall list", using the contents of the string "s".
 *
 * The number of memorized messages is available as "message_num()".
 *
 * Old messages can be retrieved by "message_str(age)", where the "age"
 * of the most recently memorized message is zero, and the oldest "age"
 * which is available is "message_num() - 1".  Messages outside this
 * range are returned as the empty string.
 *
 * The messages are stored in a special manner that maximizes "efficiency",
 * that is, we attempt to maximize the number of semi-sequential messages
 * that can be retrieved, given a limited amount of storage space, without
 * causing the memorization of new messages or the recall of old messages
 * to be too expensive.
 *
 * We keep a buffer of chars to hold the "text" of the messages, more or
 * less in the order they were memorized, and an array of offsets into that
 * buffer, representing the actual messages, but we allow the "text" to be
 * "shared" by two messages with "similar" ages, as long as we never cause
 * sharing to reach too far back in the the buffer.
 *
 * The implementation is complicated by the fact that both the array of
 * offsets, and the buffer itself, are both treated as "circular arrays"
 * for efficiency purposes, but the strings may not be "broken" across
 * the ends of the array.
 *
 * When we want to memorize a new message, we attempt to "reuse" the buffer
 * space by checking for message duplication within the recent messages.
 *
 * Otherwise, if we need more buffer space, we grab a full quarter of the
 * total buffer space at a time, to keep the reclamation code efficient.
 *
 * The "message_add()" function is rather "complex", because it must be
 * extremely efficient, both in space and time, for use with the Borg.
 */


/*
 * The next "free" index to use
 */
static u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
static u16b message__last;

/*
 * The next "free" offset
 */
static u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
static u16b message__tail;

/*
 * The array[MESSAGE_MAX] of offsets, by index
 */
static u16b *message__ptr;

/*
 * The array[MESSAGE_BUF] of chars, by offset
 */
static char *message__buf;

/*
 * The array[MESSAGE_MAX] of u16b for the types of messages
 */
static u16b *message__type;

/*
 * The array[MESSAGE_MAX] of u16b for the count of messages
 */
static u16b *message__count;


/*
 * Table of colors associated to message-types
 */
static byte message__color[MSG_MAX];


/*
 * Calculate the index of a message
 */
static s16b message_age2idx(int age)
{
	return ((message__next + MESSAGE_MAX - (age + 1)) % MESSAGE_MAX);
}


/*
 * How many messages are "available"?
 */
s16b message_num(void)
{
	/* Determine how many messages are "available" */
	return (message_age2idx(message__last - 1));
}



/*
 * Recall the "text" of a saved message
 */
cptr message_str(s16b age)
{
	static char buf[1024];
	s16b x;
	u16b o;
	cptr s;

	/* Forgotten messages have no text */
	if ((age < 0) || (age >= message_num())) return ("");

	/* Get the "logical" index */
	x = message_age2idx(age);

	/* Get the "offset" for the message */
	o = message__ptr[x];

	/* Get the message text */
	s = &message__buf[o];

	/* HACK - Handle repeated messages */
	if (message__count[x] > 1)
	{
		strnfmt(buf, sizeof(buf), "%s <%dx>", s, message__count[x]);
		s = buf;
	}

	/* Return the message text */
	return (s);
}


/*
 * Recall the "type" of a saved message
 */
u16b message_type(s16b age)
{
	s16b x;

	/* Paranoia */
	if (!message__type) return (MSG_GENERIC);

	/* Forgotten messages are generic */
	if ((age < 0) || (age >= message_num())) return (MSG_GENERIC);

	/* Get the "logical" index */
	x = message_age2idx(age);

	/* Return the message type */
	return (message__type[x]);
}


/*
 * Recall the "color" of a message type
 */
static byte message_type_color(u16b type)
{
	byte color = message__color[type];

	if (color == TERM_DARK) color = TERM_WHITE;

	return (color);
}


/*
 * Recall the "color" of a saved message
 */
byte message_color(s16b age)
{
	return message_type_color(message_type(age));
}


errr message_color_define(u16b type, byte color)
{
	/* Ignore illegal types */
	if (type >= MSG_MAX) return (1);

	/* Store the color */
	message__color[type] = color;

	/* Success */
	return (0);
}


/*
 * Add a new message, with great efficiency
 *
 * We must ignore long messages to prevent internal overflow, since we
 * assume that we can always get enough space by advancing "message__tail"
 * by one quarter the total buffer space.
 *
 * We must not attempt to optimize using a message index or buffer space
 * which is "far away" from the most recent entries, or we will lose a lot
 * of messages when we "expire" the old message index and/or buffer space.
 */
void message_add(cptr str, u16b type)
{
	int k, i, x, o;
	size_t n;

	cptr s;

	cptr u;
	char *v;


	/*** Step 1 -- Analyze the message ***/

	/* Hack -- Ignore "non-messages" */
	if (!str) return;

	/* Message length */
	n = strlen(str);

	/* Hack -- Ignore "long" messages */
	if (n >= MESSAGE_BUF / 4) return;


	/*** Step 2 -- Attempt to optimize ***/

	/* Get the "logical" last index */
	x = message_age2idx(0);

	/* Get the "offset" for the last message */
	o = message__ptr[x];

	/* Get the message text */
	s = &message__buf[o];

	/* Last message repeated? */
	if (streq(str, s))
	{
		/* Increase the message count */
		message__count[x]++;

		/* Success */
		return;
	}

	/*** Step 3 -- Attempt to optimize ***/

	/* Limit number of messages to check */
	k = message_num() / 4;

	/* Limit number of messages to check */
	if (k > 32) k = 32;

	/* Start just after the most recent message */
	i = message__next;

	/* Check the last few messages for duplication */
	for ( ; k; k--)
	{
		u16b q;

		cptr old;

		/* Back up, wrap if needed */
		if (i-- == 0) i = MESSAGE_MAX - 1;

		/* Stop before oldest message */
		if (i == message__last) break;

		/* Index */
		o = message__ptr[i];

		/* Extract "distance" from "head" */
		q = (message__head + MESSAGE_BUF - o) % MESSAGE_BUF;

		/* Do not optimize over large distances */
		if (q >= MESSAGE_BUF / 4) continue;

		/* Get the old string */
		old = &message__buf[o];

		/* Continue if not equal */
		if (!streq(str, old)) continue;

		/* Get the next available message index */
		x = message__next;

		/* Advance 'message__next', wrap if needed */
		if (++message__next == MESSAGE_MAX) message__next = 0;

		/* Kill last message if needed */
		if (message__next == message__last)
		{
			/* Advance 'message__last', wrap if needed */
			if (++message__last == MESSAGE_MAX) message__last = 0;
		}

		/* Assign the starting address */
		message__ptr[x] = message__ptr[i];

		/* Store the message type */
		message__type[x] = type;

		/* Store the message count */
		message__count[x] = 1;

		/* Success */
		return;
	}

	/*** Step 4 -- Ensure space before end of buffer ***/

	/* Kill messages, and wrap, if needed */
	if (message__head + (n + 1) >= MESSAGE_BUF)
	{
		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Get offset */
			o = message__ptr[i];

			/* Kill "dead" messages */
			if (o >= message__head)
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}

		/* Wrap "tail" if needed */
		if (message__tail >= message__head) message__tail = 0;

		/* Start over */
		message__head = 0;
	}


	/*** Step 5 -- Ensure space for actual characters ***/

	/* Kill messages, if needed */
	if (message__head + (n + 1) > message__tail)
	{
		/* Advance to new "tail" location */
		message__tail += (MESSAGE_BUF / 4);

		/* Kill all "dead" messages */
		for (i = message__last; TRUE; i++)
		{
			/* Wrap if needed */
			if (i == MESSAGE_MAX) i = 0;

			/* Stop before the new message */
			if (i == message__next) break;

			/* Get offset */
			o = message__ptr[i];

			/* Kill "dead" messages */
			if ((o >= message__head) && (o < message__tail))
			{
				/* Track oldest message */
				message__last = i + 1;
			}
		}
	}


	/*** Step 6 -- Grab a new message index ***/

	/* Get the next available message index */
	x = message__next;

	/* Advance 'message__next', wrap if needed */
	if (++message__next == MESSAGE_MAX) message__next = 0;

	/* Kill last message if needed */
	if (message__next == message__last)
	{
		/* Advance 'message__last', wrap if needed */
		if (++message__last == MESSAGE_MAX) message__last = 0;
	}


	/*** Step 7 -- Insert the message text ***/

	/* Assign the starting address */
	message__ptr[x] = message__head;

	/* Inline 'strcpy(message__buf + message__head, str)' */
	v = message__buf + message__head;
	for (u = str; *u; ) *v++ = *u++;
	*v = '\0';

	/* Advance the "head" pointer */
	message__head += (n + 1);

	/* Store the message type */
	message__type[x] = type;

	/* Store the message count */
	message__count[x] = 1;
}


/*
 * Initialize the "message" package
 */
errr messages_init(void)
{
	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u16b);
	C_MAKE(message__buf, MESSAGE_BUF, char);
	C_MAKE(message__type, MESSAGE_MAX, u16b);
	C_MAKE(message__count, MESSAGE_MAX, u16b);

	/* Init the message colors to white */
	(void)C_BSET(message__color, TERM_WHITE, MSG_MAX, byte);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;

	/* Success */
	return (0);
}


/*
 * Free the "message" package
 */
void messages_free(void)
{
	/* Free the messages */
	FREE(message__ptr);
	FREE(message__buf);
	FREE(message__type);
	FREE(message__count);
}


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




/*
 * Hack -- flush
 */
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	if (!auto_more)
	{
		/* Get an acceptable keypress */
		while (1)
		{
			char ch;
			ch = inkey();
			if (quick_messages) break;
			if ((ch == ESCAPE) || (ch == ' ')) break;
			if ((ch == '\n') || (ch == '\r')) break;
			bell("Illegal response to a 'more' prompt!");
		}
	}

	/* Clear the line */
	Term_erase(0, 0, 255);
}


static int message_column = 0;


/*
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
 * We must be very careful about using the "msg_print()" functions without
 * explicitly calling the special "msg_print(NULL)" function, since this may
 * result in the loss of information if the screen is cleared, or if anything
 * is displayed on the top line.
 *
 * Hack -- Note that "msg_print(NULL)" will clear the top line even if no
 * messages are pending.
 */
static void msg_print_aux(u16b type, cptr msg)
{
	int n;
	char *t;
	char buf[1024];
	byte color;
	int w, h;


	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Message Length */
	n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (message_column && (!msg || ((message_column + n) > (w - 8))))
	{
		/* Flush */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}


	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;


	/* Memorize the message (if legal) */
	if (character_generated && !(p_ptr->is_dead))
		message_add(msg, type);

	/* Window stuff */
	p_ptr->window |= (PW_MESSAGE);

	/* Copy it */
	my_strcpy(buf, msg, sizeof(buf));

	/* Analyze the buffer */
	t = buf;

	/* Get the color of the message */
	color = message_type_color(type);

	/* Split message */
	while (n > (w - 8))
	{
		char oops;

		int check, split;

		/* Default split */
		split = (w - 8);

		/* Find the "best" split point */
		for (check = (w / 2); check < (w - 8); check++)
		{
			/* Found a valid split point */
			if (t[check] == ' ') split = check;
		}

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
	msg_flag = TRUE;

	/* Remember the position */
	message_column += n + 1;
}


/*
 * Print a message in the default color (white)
 */
void msg_print(cptr msg)
{
	msg_print_aux(MSG_GENERIC, msg);
}


/*
 * Display a formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_aux(MSG_GENERIC, buf);
}


/*
 * Display a message and play the associated sound.
 *
 * The "extra" parameter is currently unused.
 */
void message(u16b message_type, s16b extra, cptr message)
{
	/* Unused parameter */
	(void)extra;

	sound(message_type);

	msg_print_aux(message_type, message);
}



/*
 * Display a formatted message and play the associated sound.
 *
 * The "extra" parameter is currently unused.
 */
void message_format(u16b message_type, s16b extra, cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	message(message_type, extra, buf);
}


/*
 * Print the queued messages.
 */
void message_flush(void)
{
	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Flush when needed */
	if (message_column)
	{
		/* Print pending messages */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}
}



/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Save the screen (if legal) */
	Term_save();

	/* Increase "icky" depth */
	character_icky++;
}


/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Load the screen (if legal) */
	Term_load();

	/* Decrease "icky" depth */
	character_icky--;
}


/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, cptr str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/*
 * As above, but in "white"
 */
void put_str(cptr str, int row, int col)
{
	/* Spawn */
	Term_putstr(col, row, -1, TERM_WHITE, str);
}



/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, cptr str, int row, int col)
{
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}


/*
 * As above, but in "white"
 */
void prt(cptr str, int row, int col)
{
	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}


/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "text_out()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void text_out_to_screen(byte a, cptr str)
{
	int x, y;

	int wid, h;

	int wrap;

	cptr s;


	/* Obtain the size */
	(void)Term_get_size(&wid, &h);

	/* Obtain the cursor */
	(void)Term_locate(&x, &y);

	/* Use special wrapping boundary? */
	if ((text_out_wrap > 0) && (text_out_wrap < wid))
		wrap = text_out_wrap;
	else
		wrap = wid;

	/* Process the string */
	for (s = str; *s; s++)
	{
		char ch;

		/* Force wrap */
		if (*s == '\n')
		{
			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			continue;
		}

		/* Clean up the char */
		ch = (isprint((unsigned char)*s) ? *s : ' ');

		/* Wrap words as needed */
		if ((x >= wrap - 1) && (ch != ' '))
		{
			int i, n = 0;

			byte av[256];
			char cv[256];

			/* Wrap word */
			if (x < wrap)
			{
				/* Scan existing text */
				for (i = wrap - 2; i >= 0; i--)
				{
					/* Grab existing attr/char */
					Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == ' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == 0) n = wrap;

			/* Clear line */
			Term_erase(n, y, 255);

			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			/* Wrap the word (if any) */
			for (i = n; i < wrap - 1; i++)
			{
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > wrap) x = wrap;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance */
		if (++x > wrap) x = wrap;
	}
}


/*
 * Write text to the given file and apply line-wrapping.
 *
 * Hook function for text_out(). Make sure that text_out_file points
 * to an open text-file.
 *
 * Long lines will be wrapped at text_out_wrap, or at column 75 if that
 * is not set; or at a newline character.  Note that punctuation can
 * sometimes be placed one column beyond the wrap limit.
 *
 * You must be careful to end all file output with a newline character
 * to "flush" the stored line position.
 */
void text_out_to_file(byte a, cptr str)
{
	/* Current position on the line */
	static int pos = 0;

	/* Wrap width */
	int wrap = (text_out_wrap ? text_out_wrap : 75);

	/* Current location within "str" */
	cptr s = str;

	/* Unused parameter */
	(void)a;

	/* Process the string */
	while (*s)
	{
		char ch;
		int n = 0;
		int len = wrap - pos;
		int l_space = -1;

		/* If we are at the start of the line... */
		if (pos == 0)
		{
			int i;

			/* Output the indent */
			for (i = 0; i < text_out_indent; i++)
			{
				fputc(' ', text_out_file);
				pos++;
			}
		}

		/* Find length of line up to next newline or end-of-string */
		while ((n < len) && !((s[n] == '\n') || (s[n] == '\0')))
		{
			/* Mark the most recent space in the string */
			if (s[n] == ' ') l_space = n;

			/* Increment */
			n++;
		}

		/* If we have encountered no spaces */
		if ((l_space == -1) && (n == len))
		{
			/* If we are at the start of a new line */
			if (pos == text_out_indent)
			{
				len = n;
			}
			/* HACK - Output punctuation at the end of the line */
			else if ((s[0] == ' ') || (s[0] == ',') || (s[0] == '.'))
			{
				len = 1;
			}
			else
			{
				/* Begin a new line */
				fputc('\n', text_out_file);

				/* Reset */
				pos = 0;

				continue;
			}
		}
		else
		{
			/* Wrap at the newline */
			if ((s[n] == '\n') || (s[n] == '\0')) len = n;

			/* Wrap at the last space */
			else len = l_space;
		}

		/* Write that line to file */
		for (n = 0; n < len; n++)
		{
			/* Ensure the character is printable */
			ch = (isprint(s[n]) ? s[n] : ' ');

			/* Write out the character */
			fputc(ch, text_out_file);

			/* Increment */
			pos++;
		}

		/* Move 's' past the stuff we've written */
		s += len;

		/* If we are at the end of the string, end */
		if (*s == '\0') return;

		/* Skip newlines */
		if (*s == '\n') s++;

		/* Begin a new line */
		fputc('\n', text_out_file);

		/* Reset */
		pos = 0;

		/* Skip whitespace */
		while (*s == ' ') s++;
	}

	/* We are done */
	return;
}


/*
 * Output text to the screen or to a file depending on the selected
 * text_out hook.
 */
void text_out(cptr str)
{
	text_out_c(TERM_WHITE, str);
}


/*
 * Output text to the screen (in color) or to a file depending on the
 * selected hook.
 */
void text_out_c(byte a, cptr str)
{
	text_out_hook(a, str);
}


/*
 * Clear part of the screen
 */
void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}

/*
 * The default "keypress handling function" for askfor_aux, this takes the
 * given keypress, input buffer, length, etc, and does the appropriate action
 * for each keypress, such as moving the cursor left or inserting a character.
 *
 * It should return TRUE when editing of the buffer is "complete" (e.g. on
 * the press of RETURN).
 */
bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	switch (keypress)
	{
		case ESCAPE:
		{
			*curs = 0;
			return TRUE;
			break;
		}
		
		case '\n':
		case '\r':
		{
			*curs = *len;
			return TRUE;
			break;
		}
		
		case ARROW_LEFT:
		{
			if (firsttime) *curs = 0;
			if (*curs > 0) (*curs)--;
			break;
		}
		
		case ARROW_RIGHT:
		{
			if (firsttime) *curs = *len - 1;
			if (*curs < *len) (*curs)++;
			break;
		}
		
		case 0x7F:
		case '\010':
		{
			/* If this is the first time round, backspace means "delete all" */
			if (firsttime)
			{
				buf[0] = '\0';
				*curs = 0;
				*len = 0;

				break;
			}

			/* Refuse to backspace into oblivion */
			if (*curs == 0) break;

			/* Move the string from k to nul along to the left by 1 */
			memmove(&buf[*curs - 1], &buf[*curs], *len - *curs);

			/* Decrement */
			(*curs)--;
			(*len)--;

			/* Terminate */
			buf[*len] = '\0';

			break;
		}
		
		default:
		{
			bool atnull = (buf[*curs] == 0);


			if (!isprint((unsigned char)keypress))
			{
				bell("Illegal edit key!");
				break;
			}

			/* Clear the buffer if this is the first time round */
			if (firsttime)
			{
				buf[0] = '\0';
				*curs = 0;
				*len = 0;
				atnull = 1;
			}

			if (atnull)
			{
				/* Make sure we have enough room for a new character */
				if ((*curs + 1) >= buflen) break;
			}
			else
			{
				/* Make sure we have enough room to add a new character */
				if ((*len + 1) >= buflen) break;

				/* Move the rest of the buffer along to make room */
				memmove(&buf[*curs+1], &buf[*curs], *len - *curs);
			}

			/* Insert the character */
			buf[(*curs)++] = keypress;
			(*len)++;

			/* Terminate */
			buf[*len] = '\0';

			break;
		}
	}

	/* By default, we aren't done. */
	return FALSE;
}

/*
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
 * Return accepts the current buffer contents and returns TRUE.
 * Escape clears the buffer and the window and returns FALSE.
 *
 * Note that 'len' refers to the size of the buffer.  The maximum length
 * of the input is 'len-1'.
 *
 * 'keypress_h' is a pointer to a function to handle keypresses, altering
 * the input buffer, cursor position and suchlike as required.  See
 * 'askfor_aux_keypress' (the default handler if you supply NULL for
 * 'keypress_h') for an example.
 */
bool askfor_aux(char *buf, size_t len, bool keypress_h(char *, size_t, size_t *, size_t *, char, bool))
{

	int y, x;

	size_t k = 0;		/* Cursor position */
	size_t nul = 0;		/* Position of the null byte in the string */

	char ch = '\0';

	bool done = FALSE;
	bool firsttime = TRUE;

	if (keypress_h == NULL)
	{
		keypress_h = askfor_aux_keypress;
	}

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
	Term_putstr(x, y, -1, TERM_YELLOW, buf);

	/* Process input */
	while (!done)
	{
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		ch = inkey();

		/* Let the keypress handler deal with the keypress */
		done = keypress_h(buf, len, &k, &nul, ch, firsttime);

		/* Update the entry */
		Term_erase(x, y, (int)len);
		Term_putstr(x, y, -1, TERM_WHITE, buf);

		/* Not the first time round anymore */
		firsttime = FALSE;
	}

	/* Done */
	return (ch != ESCAPE);
}

/*
 * A "keypress" handling function for askfor_aux, that handles the special
 * case of '*' for a new random "name" and passes any other "keypress"
 * through to the default "editing" handler.
 */
bool get_name_keypress(char *buf, size_t buflen, size_t *curs, size_t *len, char keypress, bool firsttime)
{
	bool result;

	switch (keypress)
	{
		case '*':
		{
			*len = randname_make(RANDNAME_TOLKIEN, 4, 8, buf, buflen);
			buf[0] = toupper((unsigned char) buf[0]);
			*curs = 0;
			result = FALSE;
			break;
		}
		
		
		default:
		{
			result = askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
			break;
		}
	}

	return result;
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * If sf is TRUE, we change the savefile name depending on the character name.
 *
 * What a horrible name for a global function.  XXX XXX XXX
 */
void get_name(bool sf)
{
	bool res;
	char tmp[32];

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prt("Enter a name for your character (* for a random name): ", 0, 0);

	/* Save the player name */
	my_strcpy(tmp, op_ptr->full_name, sizeof(tmp));

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), get_name_keypress);

	/* Clear prompt */
	prt("", 0, 0);

	if (res)
	{
		/* Use the name */
		my_strcpy(op_ptr->full_name, tmp, sizeof(op_ptr->full_name));

		/* Process the player name */
		process_player_name(sf);
	}
}



/*
 * Prompt for a string from the user.
 *
 * The "prompt" should take the form "Prompt: ".
 *
 * See "askfor_aux" for some notes about "buf" and "len", and about
 * the return value of this function.
 */
bool get_string(cptr prompt, char *buf, size_t len)
{
	bool res;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len, NULL);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}



/*
 * Request a "quantity" from the user
 *
 * Allow "p_ptr->command_arg" to specify a quantity
 */
s16b get_quantity(cptr prompt, int max)
{
	int amt = 1;


	/* Use "command_arg" */
	if (p_ptr->command_arg)
	{
		/* Extract a number */
		amt = p_ptr->command_arg;

		/* Clear "command_arg" */
		p_ptr->command_arg = 0;
	}

	/* Get the item index */
	else if ((max != 1) && repeat_pull(&amt))
	{
		/* nothing */
	}

	/* Prompt if needed */
	else if ((max != 1))
	{
		char tmp[80];

		char buf[80];

		/* Build a prompt if needed */
		if (!prompt)
		{
			/* Build a prompt */
			strnfmt(tmp, sizeof(tmp), "Quantity (0-%d): ", max);

			/* Use that prompt */
			prompt = tmp;
		}

		/* Build the default */
		strnfmt(buf, sizeof(buf), "%d", amt);

		/* Ask for a quantity */
		if (!get_string(prompt, buf, 7)) return (0);

		/* Extract a number */
		amt = atoi(buf);

		/* A letter means "all" */
		if (isalpha((unsigned char)buf[0])) amt = max;
	}

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	if (amt) repeat_push(amt);

	/* Return the result */
	return (amt);
}

/*
 * Hack - duplication of get_check prompt to give option of setting destroyed
 * option to squelch.
 *
 * 0 - No
 * 1 = Yes
 * 2 = third option
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n/{char}]" is appended to the prompt.
 */
int get_check_other(cptr prompt, char other)
{
	char ch;
	char buf[80];

	int result;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n/%c] ", prompt, other);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		ch = inkey();
		if (quick_messages) break;
		if (ch == ESCAPE) break;
		if (strchr("YyNn", ch)) break;
		if (ch == toupper(other)) break;
		if (ch == tolower(other)) break;
		bell("Illegal response to question!");
	}

	/* Erase the prompt */
	prt("", 0, 0);


	/* Yes */
	if ((ch == 'Y') || (ch == 'y'))
		result = 1;

	/* Third option */
	else if ((ch == toupper(other)) || (ch == tolower(other)))
		result = 2;

	/* Default to no */
	else
		result = 0;


	/* Success */
	return (result);
}


/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(cptr prompt)
{
	char ch;

	char buf[80];

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		ch = inkey();
		if (quick_messages) break;
		if (ch == ESCAPE) break;
		if (strchr("YyNn", ch)) break;
		bell("Illegal response to a 'yes/no' question!");
	}

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if ((ch != 'Y') && (ch != 'y')) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
	event_type ke;
	bool result;

	result = get_com_ex(prompt, &ke);
	*command = ke.key;

	return result;
}

bool get_com_ex(cptr prompt, event_type *command)
{
	event_type ke;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	ke = inkey_ex();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Save the command */
	*command = ke;

	/* Done */
	return (ke.key != ESCAPE);
}


/*
 * Pause for user response
 *
 * This function is stupid.  XXX XXX XXX
 */
void pause_line(int row)
{
	prt("", row, 0);
	put_str("[Press any key to continue]", row, 23);
	(void)inkey();
	prt("", row, 0);
}




/*
 * Hack -- special buffer to hold the action of the current keymap
 */
static char request_command_buffer[256];


/*
 * Request a command from the user.
 *
 * Sets p_ptr->command_cmd, p_ptr->command_dir, p_ptr->command_rep,
 * p_ptr->command_arg.  May modify p_ptr->command_new.
 *
 * Note that "caret" ("^") is treated specially, and is used to
 * allow manual input of control characters.  This can be used
 * on many machines to request repeated tunneling (Ctrl-H) and
 * on the Macintosh to request "Control-Caret".
 *
 * Note that "backslash" is treated specially, and is used to bypass any
 * keymap entry for the following character.  This is useful for macros.
 *
 * Note that this command is used both in the dungeon and in
 * stores, and must be careful to work in both situations.
 *
 * Note that "p_ptr->command_new" may not work any more.  XXX XXX XXX
 */
void request_command(void)
{
	int i;

	event_type ke;

	int mode;

	cptr act;


	/* Roguelike */
	if (rogue_like_commands)
	{
		mode = KEYMAP_MODE_ROGUE;
	}

	/* Original */
	else
	{
		mode = KEYMAP_MODE_ORIG;
	}


	/* No command yet */
	p_ptr->command_cmd = 0;

	/* No "argument" yet */
	p_ptr->command_arg = 0;

	/* No "direction" yet */
	p_ptr->command_dir = 0;


	/* Get command */
	while (1)
	{
		/* Hack -- auto-commands */
		if (p_ptr->command_new)
		{
			/* Flush messages */
			message_flush();

			/* Use auto-command */
			ke.key = (char)p_ptr->command_new;

			/* Forget it */
			p_ptr->command_new = 0;
		}

		/* Get a keypress in "command" mode */
		else
		{
			/* Hack -- no flush needed */
			msg_flag = FALSE;

			/* Activate "command mode" */
			inkey_flag = TRUE;

			/* Get a command */
			ke = inkey_ex();
		}

		/* Clear top line */
		prt("", 0, 0);


		/* Resize events XXX XXX */
		if (ke.type == EVT_RESIZE)
		{
			p_ptr->command_cmd_ex = ke;
			p_ptr->command_new = ' ';
		}


		/* Command Count */
		if (ke.key == '0')
		{
			int old_arg = p_ptr->command_arg;

			/* Reset */
			p_ptr->command_arg = 0;

			/* Begin the input */
			prt("Count: ", 0, 0);

			/* Get a command count */
			while (1)
			{
				/* Get a new keypress */
				ke.key = inkey();

				/* Simple editing (delete or backspace) */
				if ((ke.key == 0x7F) || (ke.key == KTRL('H')))
				{
					/* Delete a digit */
					p_ptr->command_arg = p_ptr->command_arg / 10;

					/* Show current count */
					prt(format("Count: %d", p_ptr->command_arg), 0, 0);
				}

				/* Actual numeric data */
				else if (isdigit((unsigned char)ke.key))
				{
					/* Stop count at 9999 */
					if (p_ptr->command_arg >= 1000)
					{
						/* Warn */
						bell("Invalid repeat count!");

						/* Limit */
						p_ptr->command_arg = 9999;
					}

					/* Increase count */
					else
					{
						/* Incorporate that digit */
						p_ptr->command_arg = p_ptr->command_arg * 10 + D2I(ke.key);
					}

					/* Show current count */
					prt(format("Count: %d", p_ptr->command_arg), 0, 0);
				}

				/* Exit on "unusable" input */
				else
				{
					break;
				}
			}

			/* Hack -- Handle "zero" */
			if (p_ptr->command_arg == 0)
			{
				/* Default to 99 */
				p_ptr->command_arg = 99;

				/* Show current count */
				prt(format("Count: %d", p_ptr->command_arg), 0, 0);
			}

			/* Hack -- Handle "old_arg" */
			if (old_arg != 0)
			{
				/* Restore old_arg */
				p_ptr->command_arg = old_arg;

				/* Show current count */
				prt(format("Count: %d", p_ptr->command_arg), 0, 0);
			}

			/* Hack -- white-space means "enter command now" */
			if ((ke.key == ' ') || (ke.key == '\n') || (ke.key == '\r'))
			{
				/* Get a real command */
				if (!get_com("Command: ", &ke.key))
				{
					/* Clear count */
					p_ptr->command_arg = 0;

					/* Continue */
					continue;
				}
			}
		}


		/* Special case for the arrow keys */
		if (isarrow(ke.key))
		{
			switch (ke.key)
			{
				case ARROW_DOWN:    ke.key = '2'; break;
				case ARROW_LEFT:    ke.key = '4'; break;
				case ARROW_RIGHT:   ke.key = '6'; break;
				case ARROW_UP:      ke.key = '8'; break;
			}
		}


		/* Allow "keymaps" to be bypassed */
		if (ke.key == '\\')
		{
			/* Get a real command */
			(void)get_com("Command: ", &ke.key);

			/* Hack -- bypass keymaps */
			if (!inkey_next) inkey_next = "";
		}


		/* Allow "control chars" to be entered */
		if (ke.key == '^')
		{
			/* Get a new command and controlify it */
			if (get_com("Control: ", &ke.key)) ke.key = KTRL(ke.key);
		}


		/* Look up applicable keymap */
		act = keymap_act[mode][(byte)(ke.key)];

		/* Apply keymap if not inside a keymap already */
		if (act && !inkey_next)
		{
			/* Install the keymap */
			my_strcpy(request_command_buffer, act,
			          sizeof(request_command_buffer));

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}


		/* Paranoia */
		if (ke.key == '\0') continue;


		/* Use command */
		p_ptr->command_cmd = ke.key;
		p_ptr->command_cmd_ex = ke;

		/* Done */
		break;
	}

	/* Hack -- Auto-repeat certain commands */
	if (p_ptr->command_arg <= 0)
	{
		/* Hack -- auto repeat certain commands */
		if (strchr(AUTO_REPEAT_COMMANDS, p_ptr->command_cmd))
		{
			/* Repeat 99 times */
			p_ptr->command_arg = 99;
		}
	}


	/* Hack -- Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		cptr s;

		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* No inscription */
		if (!o_ptr->note) continue;

		/* Find a '^' */
		s = strchr(quark_str(o_ptr->note), '^');

		/* Process preventions */
		while (s)
		{
			/* Check the "restriction" character */
			if ((s[1] == p_ptr->command_cmd) || (s[1] == '*'))
			{
				/* Hack -- Verify command */
				if (!get_check("Are you sure? "))
				{
					/* Hack -- Use "newline" */
					p_ptr->command_cmd = '\n';
				}
			}

			/* Find another '^' */
			s = strchr(s + 1, '^');
		}
	}


	/* Hack -- erase the message line. */
	prt("", 0, 0);

	/* Hack again -- apply the modified key command */
	p_ptr->command_cmd_ex.key = p_ptr->command_cmd;
}




/*
 * Generates damage for "2d6" style dice rolls
 */
int damroll(int num, int sides)
{
	int i;
	int sum = 0;


	/* HACK - prevent undefined behaviour */
	if (sides <= 0) return (0);

	for (i = 0; i < num; i++)
	{
		sum += rand_die(sides);
	}

	return (sum);
}


/*
 * Same as above, but always maximal
 */
int maxroll(int num, int sides)
{
	return (num * sides);
}




/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		return (TRUE);
	}

	return (FALSE);
}


/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
int color_char_to_attr(char c)
{
	switch (c)
	{
		case 'd': return (TERM_DARK);
		case 'w': return (TERM_WHITE);
		case 's': return (TERM_SLATE);
		case 'o': return (TERM_ORANGE);
		case 'r': return (TERM_RED);
		case 'g': return (TERM_GREEN);
		case 'b': return (TERM_BLUE);
		case 'u': return (TERM_UMBER);

		case 'D': return (TERM_L_DARK);
		case 'W': return (TERM_L_WHITE);
		case 'v': return (TERM_VIOLET);
		case 'y': return (TERM_YELLOW);
		case 'R': return (TERM_L_RED);
		case 'G': return (TERM_L_GREEN);
		case 'B': return (TERM_L_BLUE);
		case 'U': return (TERM_L_UMBER);
	}

	return (-1);
}


/*
 * Converts a string to a terminal color byte.
 */
int color_text_to_attr(cptr name)
{
	if (my_stricmp(name, "dark")       == 0) return (TERM_DARK);
	if (my_stricmp(name, "white")      == 0) return (TERM_WHITE);
	if (my_stricmp(name, "slate")      == 0) return (TERM_SLATE);
	if (my_stricmp(name, "orange")     == 0) return (TERM_ORANGE);
	if (my_stricmp(name, "red")        == 0) return (TERM_RED);
	if (my_stricmp(name, "green")      == 0) return (TERM_GREEN);
	if (my_stricmp(name, "blue")       == 0) return (TERM_BLUE);
	if (my_stricmp(name, "umber")      == 0) return (TERM_UMBER);
	if (my_stricmp(name, "violet")     == 0) return (TERM_VIOLET);
	if (my_stricmp(name, "yellow")     == 0) return (TERM_YELLOW);
	if (my_stricmp(name, "lightdark")  == 0) return (TERM_L_DARK);
	if (my_stricmp(name, "lightwhite") == 0) return (TERM_L_WHITE);
	if (my_stricmp(name, "lightred")   == 0) return (TERM_L_RED);
	if (my_stricmp(name, "lightgreen") == 0) return (TERM_L_GREEN);
	if (my_stricmp(name, "lightblue")  == 0) return (TERM_L_BLUE);
	if (my_stricmp(name, "lightumber") == 0) return (TERM_L_UMBER);

	/* Oops */
	return (-1);
}


/*
 * Extract a textual representation of an attribute
 */
cptr attr_to_text(byte a)
{
	switch (a)
	{
		case TERM_DARK:    return ("Dark");
		case TERM_WHITE:   return ("White");
		case TERM_SLATE:   return ("Slate");
		case TERM_ORANGE:  return ("Orange");
		case TERM_RED:     return ("Red");
		case TERM_GREEN:   return ("Green");
		case TERM_BLUE:    return ("Blue");
		case TERM_UMBER:   return ("Umber");
		case TERM_L_DARK:  return ("L.Dark");
		case TERM_L_WHITE: return ("L.Slate");
		case TERM_VIOLET:  return ("Violet");
		case TERM_YELLOW:  return ("Yellow");
		case TERM_L_RED:   return ("L.Red");
		case TERM_L_GREEN: return ("L.Green");
		case TERM_L_BLUE:  return ("L.Blue");
		case TERM_L_UMBER: return ("L.Umber");
	}

	/* Oops */
	return ("Icky");
}



#if 0

/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * Could be made more efficient, especially in the case where "insert"
 * is smaller than "target".
 */
static bool insert_str(char *buf, cptr target, cptr insert)
{
	int i, len;
	int b_len, t_len, i_len;

	/* Attempt to find the target (modify "buf") */
	buf = strstr(buf, target);

	/* No target found */
	if (!buf) return (FALSE);

	/* Be sure we have an insertion string */
	if (!insert) insert = "";

	/* Extract some lengths */
	t_len = strlen(target);
	i_len = strlen(insert);
	b_len = strlen(buf);

	/* How much "movement" do we need? */
	len = i_len - t_len;

	/* We need less space (for insert) */
	if (len < 0)
	{
		for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
	}

	/* We need more space (for insert) */
	else if (len > 0)
	{
		for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
	}

	/* If movement occured, we need a new terminator */
	if (len) buf[b_len+len] = '\0';

	/* Now copy the insertion string */
	for (i = 0; i < i_len; ++i) buf[i] = insert[i];

	/* Successful operation */
	return (TRUE);
}


#endif


#define REPEAT_MAX 20

/* Number of chars saved */
static int repeat__cnt = 0;

/* Current index */
static int repeat__idx = 0;

/* Saved "stuff" */
static int repeat__key[REPEAT_MAX];


/*
 * Push data.
 */
void repeat_push(int what)
{
	/* Too many keys */
	if (repeat__cnt == REPEAT_MAX) return;

	/* Push the "stuff" */
	repeat__key[repeat__cnt++] = what;

	/* Prevents us from pulling keys */
	++repeat__idx;
}


/*
 * Pull data.
 */
bool repeat_pull(int *what)
{
	/* All out of keys */
	if (repeat__idx == repeat__cnt) return (FALSE);

	/* Grab the next key, advance */
	*what = repeat__key[repeat__idx++];

	/* Success */
	return (TRUE);
}


void repeat_clear(void)
{
	/* Start over from the failed pull */
	if (repeat__idx)
		repeat__cnt = --repeat__idx;
	/* Paranoia */
	else
		repeat__cnt = repeat__idx;

	return;
}


/*
 * Repeat previous command, or begin memorizing new command.
 */
void repeat_check(void)
{
	int what;

	/* Ignore some commands */
	if (p_ptr->command_cmd == ESCAPE) return;
	if (p_ptr->command_cmd == ' ') return;
	if (p_ptr->command_cmd == '\n') return;
	if (p_ptr->command_cmd == '\r') return;

	/* Repeat Last Command */
	if (p_ptr->command_cmd == KTRL('V'))
	{
		/* Reset */
		repeat__idx = 0;

		/* Get the command */
		if (repeat_pull(&what))
		{
			/* Save the command */
			p_ptr->command_cmd = what;
		}
	}

	/* Start saving new command */
	else
	{
		/* Reset */
		repeat__cnt = 0;
		repeat__idx = 0;

		/* Get the current command */
		what = p_ptr->command_cmd;

		/* Save this command */
		repeat_push(what);
	}
}


#ifdef SUPPORT_GAMMA

/* Table of gamma values */
byte gamma_table[256];

/* Table of ln(x / 256) * 256 for x going from 0 -> 255 */
static const s16b gamma_helper[256] =
{
	0, -1420, -1242, -1138, -1065, -1007, -961, -921, -887, -857, -830,
	-806, -783, -762, -744, -726, -710, -694, -679, -666, -652, -640,
	-628, -617, -606, -596, -586, -576, -567, -577, -549, -541, -532,
	-525, -517, -509, -502, -495, -488, -482, -475, -469, -463, -457,
	-451, -455, -439, -434, -429, -423, -418, -413, -408, -403, -398,
	-394, -389, -385, -380, -376, -371, -367, -363, -359, -355, -351,
	-347, -343, -339, -336, -332, -328, -325, -321, -318, -314, -311,
	-308, -304, -301, -298, -295, -291, -288, -285, -282, -279, -276,
	-273, -271, -268, -265, -262, -259, -257, -254, -251, -248, -246,
	-243, -241, -238, -236, -233, -231, -228, -226, -223, -221, -219,
	-216, -214, -212, -209, -207, -205, -203, -200, -198, -196, -194,
	-192, -190, -188, -186, -184, -182, -180, -178, -176, -174, -172,
	-170, -168, -166, -164, -162, -160, -158, -156, -155, -153, -151,
	-149, -147, -146, -144, -142, -140, -139, -137, -135, -134, -132,
	-130, -128, -127, -125, -124, -122, -120, -119, -117, -116, -114,
	-112, -111, -109, -108, -106, -105, -103, -102, -100, -99, -97, -96,
	-95, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78,
	-76, -75, -74, -72, -71, -70, -68, -67, -66, -65, -63, -62, -61,
	-59, -58, -57, -56, -54, -53, -52, -51, -50, -48, -47, -46, -45,
	-44, -42, -41, -40, -39, -38, -37, -35, -34, -33, -32, -31, -30,
	-29, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16,
	-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1
};


/*
 * Build the gamma table so that floating point isn't needed.
 *
 * Note gamma goes from 0->256.  The old value of 100 is now 128.
 */
void build_gamma_table(int gamma)
{
	int i, n;

	/*
	 * value is the current sum.
	 * diff is the new term to add to the series.
	 */
	long value, diff;

	/* Hack - convergence is bad in these cases. */
	gamma_table[0] = 0;
	gamma_table[255] = 255;

	for (i = 1; i < 255; i++)
	{
		/*
		 * Initialise the Taylor series
		 *
		 * value and diff have been scaled by 256
		 */
		n = 1;
		value = 256L * 256L;
		diff = ((long)gamma_helper[i]) * (gamma - 256);

		while (diff)
		{
			value += diff;
			n++;

			/*
			 * Use the following identiy to calculate the gamma table.
			 * exp(x) = 1 + x + x^2/2 + x^3/(2*3) + x^4/(2*3*4) +...
			 *
			 * n is the current term number.
			 *
			 * The gamma_helper array contains a table of
			 * ln(x/256) * 256
			 * This is used because a^b = exp(b*ln(a))
			 *
			 * In this case:
			 * a is i / 256
			 * b is gamma.
			 *
			 * Note that everything is scaled by 256 for accuracy,
			 * plus another factor of 256 for the final result to
			 * be from 0-255.  Thus gamma_helper[] * gamma must be
			 * divided by 256*256 each itteration, to get back to
			 * the original power series.
			 */
			diff = (((diff / 256) * gamma_helper[i]) * (gamma - 256)) / (256 * n);
		}

		/*
		 * Store the value in the table so that the
		 * floating point pow function isn't needed.
		 */
		gamma_table[i] = ((long)(value / 256) * i) / 256;
	}
}

#endif /* SUPPORT_GAMMA */
