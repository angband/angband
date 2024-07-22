/**
 * \file borg-io.c
 * \brief Simple input (keypresses) and output (screen scraping)
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "borg-io.h"

#ifdef ALLOW_BORG

#include "../ui-term.h"

#include "borg-think.h"
#include "borg.h"

bool borg_confirm_target;

/*
 * Query the "attr/chars" at a given location on the screen
 *
 * Note that "a" points to a single "attr", and "s" to an array
 * of "chars", into which the attribute and text at the given
 * location are stored.
 *
 * We will not grab more than "ABS(n)" characters for the string.
 * If "n" is "positive", we will grab exactly "n" chars, or fail.
 * If "n" is "negative", we will grab until the attribute changes.
 *
 * We automatically convert all "blanks" and "invisible text" into
 * spaces, and we ignore the attribute of such characters.
 *
 * We do not strip final spaces, so this function will very often
 * read characters all the way to the end of the line.
 *
 * We succeed only if a string of some form existed, and all of
 * the non-space characters in the string have the same attribute,
 * and the string was long enough.
 *
 * XXX XXX XXX We assume the given location is legal
 */
errr borg_what_text(int x, int y, int n, uint8_t *a, char *s)
{
    int     i;
    wchar_t screen_str[1024];

    int     t_a;
    wchar_t t_c;

    int     *aa;
    wchar_t *cc;

    int w, h;

    /* Current attribute */
    int d_a = 0;

    /* Max length to scan for */
    int m = ABS(n);

    /* Activate */
    /* Do I need to get the right window? Term_activate(angband_term[0]); */

    /* Obtain the size */
    (void)Term_get_size(&w, &h);

    /* Hack -- Do not run off the screen */
    if (x + m > w)
        m = w - x;

    /* Direct access XXX XXX XXX */
    aa = &(Term->scr->a[y][x]);
    cc = &(Term->scr->c[y][x]);

    /* Grab the string */
    for (i = 0; i < m; i++) {
        /* Access */
        t_a = *aa++;
        t_c = *cc++;

        /* Handle spaces */
        if ((t_c == L' ') || !t_a) {
            /* Save space */
            screen_str[i] = L' ';
        }

        /* Handle real text */
        else {
            /* Attribute ready */
            if (d_a) {
                /* Verify the "attribute" (or stop) */
                if (t_a != d_a)
                    break;
            }

            /* Acquire attribute */
            else {
                /* Save it */
                d_a = t_a;
            }

            /* Save char */
            screen_str[i] = t_c;
        }
    }

    /* Terminate the string */
    screen_str[i] = L'\0';

    /* Save the attribute */
    (*a) = d_a;

    /* Convert back to a char string */
    wcstombs(s, screen_str, ABS(n) + 1);
    /* Too short */
    if ((n > 0) && (i != n))
        return 1;

    /* Success */
    return 0;
}

/*
 * Log a message to a file
 */
static void borg_info(const char *what) { }


/*
 * Memorize a message, Log it, Search it, and Display it in pieces
 */
static void borg_note_internal(bool warning, const char *what)
{
    int j, n, i, k;

    int w, h, x, y;

    term *old = Term;

    /* Memorize it */
    if (warning) {
        msg("%s", what);
    } else {
        message_add(what, MSG_GENERIC);
    }

    /* Log the message */
    borg_info(what);

    /* Mega-Hack -- Check against the swap loops */
    if (strstr(what, "Best Combo") || strstr(what, "Taking off ")) {
        /* Tick the anti loop clock */
        borg.time_this_panel += 10;
        borg_note(
            format("# Anti-loop variable tick (%d).", borg.time_this_panel));
    }

    /* Scan windows */
    for (j = 0; j < 8; j++) {
        if (!angband_term[j])
            continue;

        /* Check flag */
        if (!(window_flag[j] & PW_BORG_1))
            continue;

        /* Activate */
        Term_activate(angband_term[j]);

        /* Access size */
        Term_get_size(&w, &h);

        /* Access cursor */
        Term_locate(&x, &y);

        /* Erase current line */
        Term_erase(0, y, 255);

        /* Total length */
        n = strlen(what);

        /* Too long */
        if (n > w - 2) {
            char buf[1024];

            /* Split */
            while (n > w - 2) {
                /* Default */
                k = w - 2;

                /* Find a split point */
                for (i = w / 2; i < w - 2; i++) {
                    /* Pre-emptive split point */
                    if (isspace(what[i]))
                        k = i;
                }

                /* Copy over the split message */
                for (i = 0; i < k; i++) {
                    /* Copy */
                    buf[i] = what[i];
                }

                /* Indicate split */
                buf[i++] = '\\';

                /* Terminate */
                buf[i] = '\0';

                /* Show message */
                Term_addstr(-1, COLOUR_WHITE, buf);

                /* Advance (wrap) */
                if (++y >= h)
                    y = 0;

                /* Erase next line */
                Term_erase(0, y, 255);

                /* Advance */
                what += k;

                /* Reduce */
                n -= k;
            }

            /* Show message tail */
            Term_addstr(-1, COLOUR_WHITE, what);

            /* Advance (wrap) */
            if (++y >= h)
                y = 0;

            /* Erase next line */
            Term_erase(0, y, 255);
        }

        /* Normal */
        else {
            /* Show message */
            Term_addstr(-1, COLOUR_WHITE, what);

            /* Advance (wrap) */
            if (++y >= h)
                y = 0;

            /* Erase next line */
            Term_erase(0, y, 255);
        }

        /* Flush output */
        Term_fresh();

        /* Use correct window */
        Term_activate(old);
    }
}

void borg_warning(const char *what)
{
    borg_note_internal(true, what);
}

/*
 * Memorize a message, Log it, Search it, and Display it in pieces
 */
void borg_note(const char *what)
{
    borg_note_internal(false, what);
}

/*
 * A Queue of keypresses to be sent
 */
static keycode_t *borg_key_queue;
static int16_t    borg_key_head;
static int16_t    borg_key_tail;

/*
 * A history of keypresses to be sent
 */
static struct keypress  *borg_key_history;
static int16_t    borg_key_history_head;

/*
 * since the code now only asks for a direction if
 * the instruction is ambiguous and it is hard for
 * the borg to know if it is asking for something
 * ambiguous, it queues up a direction in case it is
 * requested.
 */
static keycode_t borg_queued_direction = 0;

/*
 * Add a keypress to the "queue" (fake event)
 */
errr borg_keypress(keycode_t k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) {
        borg_note(" & Key * *BAD KEY * *");
        return (-1);
    }

    /* Store the char, advance the queue */
    borg_key_queue[borg_key_head++] = k;

    /* Circular queue, handle wrap */
    if (borg_key_head == KEY_SIZE)
        borg_key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (borg_key_head == borg_key_tail)
        borg_oops("overflow");

    /* Hack -- Overflow may induce circular queue */
    if (borg_key_tail == KEY_SIZE)
        borg_key_tail = 0;

    /* Success */
    return 0;
}

/*
 * Add a keypress to the history of what has been passed back to the game
 */
void save_keypress_history(struct keypress *kp)
{
    /* Note the keypress */
    if (borg_cfg[BORG_VERBOSE]) {
        if (kp->type == EVT_KBRD) {
            keycode_t k = kp->code;
            if (k >= 32 && k <= 126) {
                borg_note(format("& Key <%c> (0x%02X)", k, k));
            } else {
                if (k == KC_ENTER)
                    borg_note(format("& Key <Enter> (0x%02X)", k));
                else if (k == ESCAPE)
                    borg_note(format("& Key <Esc> (0x%02X)", k));
                else
                    borg_note(format("& Key <0x%02X>", k));
            }
        } else {
            borg_note(format("& non-Keyboard <0x%02X>", kp->type));
        }

    }

    /* Store the char, advance the queue */
    borg_key_history[borg_key_history_head].code = kp->code;
    borg_key_history[borg_key_history_head++].type = kp->type;

    /* on full array, keep the last 100 */
    if (borg_key_history_head == KEY_SIZE) {
        memcpy(borg_key_history, &borg_key_history[KEY_SIZE - 101], sizeof(struct keypress) * 100);
        borg_key_history_head = 100;
    }
}


/*
 * Add a keypresses to the "queue" (fake event)
 */
errr borg_keypresses(const char *str)
{
    const char *s;

    /* Enqueue them */
    for (s = str; *s; s++)
        borg_keypress(*s);

    /* Success */
    return 0;
}

/*
 * Get the next Borg keypress
 */
keycode_t borg_inkey(bool take)
{
    int i;

    /* Nothing ready */
    if (borg_key_head == borg_key_tail)
        return 0;

    /* Extract the keypress */
    i = borg_key_queue[borg_key_tail];

    /* Do not advance */
    if (!take)
        return i;

    /* Advance the queue */
    borg_key_tail++;

    /* Circular queue requires wrap-around */
    if (borg_key_tail == KEY_SIZE)
        borg_key_tail = 0;

    /* Return the key */
    return i;
}

/*
 * Clear all Borg keypress
 */
void borg_flush(void)
{
    /* Simply forget old keys */
    borg_key_tail         = borg_key_head;

    borg_queued_direction = 0;
}

/*
 *  Save and retrieve direction when the command may be ambiguous.
 *  Watch for "Direction or <click> (Escape to cancel)?"
 */
void borg_queue_direction(keycode_t k)
{
    borg_queued_direction = k;
    borg_confirm_target   = true;
}

keycode_t borg_get_queued_direction(void)
{
    keycode_t k           = borg_queued_direction;
    borg_queued_direction = 0;
    return k;
}

/*
 * *HACK* this handles the é and á in some monster names but, gods it is
 * ugly convert to wide and back to match the processing of special characters
 * this routine will allocate any memory it needs and it is up to the caller 
 * to detect that memory was allocated and free it.
 */
char *borg_massage_special_chars(char *name)
{
    wchar_t wide_name[1024];
    char *  memory = mem_zalloc((strlen(name) + 1) * sizeof(char));

    text_mbstowcs(wide_name, name, strlen(name) + 1);
    wcstombs(memory, wide_name, strlen(name) + 1);

    return memory;
}

/*
 * print the recent keypresses to the message history
 */
void borg_dump_recent_keys(int num)
{
    int end = borg_key_history_head;
    int start = borg_key_history_head < num ? 0 : borg_key_history_head - num;
    for (; start < end; start++) {
        struct keypress *kp = &borg_key_history[start];
        if (kp->type == EVT_KBRD) {
            keycode_t k = kp->code;
            if (k >= 32 && k <= 126) {
                borg_note(format("& Key history <%c> (0x%02X)", k, k));
            } else {
                if (k == KC_ENTER)
                    borg_note(format("& Key history <Enter> (0x%02X)", k));
                else if (k == ESCAPE)
                    borg_note(format("& Key history <Esc> (0x%02X)", k));
                else
                    borg_note(format("& Key history <0x%02X>", k));
            }
        }
        else {
            borg_note(format("& non-Keyboard <0x%02X>", kp->type));
        }
    }
}

/*
 * The bell should never sound when the borg is running.  If it does,
 * log ... something.
 */
static void borg_bell(game_event_type unused, game_event_data *data, void *user)
{
    borg_note("** BELL SOUNDED Dumping keypress history ***");

    borg_dump_recent_keys(20);

    if (borg_cfg[BORG_STOP_ON_BELL])
        borg_oops("** BELL SOUNDED***");
}


void borg_init_io(void)
{
    /* Allocate the "keypress queue" */
    borg_key_queue = mem_zalloc(KEY_SIZE * sizeof(keycode_t));

    /* Allocate the keypress history */
    borg_key_history = mem_zalloc(KEY_SIZE * sizeof(struct keypress));

    /* When the bell goes off, log an error */
    event_add_handler(EVENT_BELL, borg_bell, NULL);
}

void borg_free_io(void)
{
    event_remove_handler(EVENT_BELL, borg_bell, NULL);
 
    mem_free(borg_key_history);
    borg_key_history = NULL;

    mem_free(borg_key_queue);
    borg_key_queue = NULL;
}
#endif
