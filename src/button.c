/*
 * File: mouse.c
 * Purpose: Mousebutton handling code
 *
 * Copyright (c) 2007 Nick McConnell
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
#include "button.h"

/*** Constants ***/

/**
 * Maximum number of mouse buttons
 */
#define MAX_MOUSE_BUTTONS  20

/**
 * Maximum length of a mouse button label
 */
#define MAX_MOUSE_LABEL 10


/*** Types ***/

/**
 * Mouse button structure
 */
typedef structz
{
	char label[MAX_MOUSE_LABEL]; /*!< Label on the button */
	int left;                    /*!< Column containing the left edge of the button */
	int right;                   /*!< Column containing the right edge of the button */
	unsigned char key;           /*!< Keypress corresponding to the button */
} button_mouse;



/*** Variables ***/

static button_mouse *button_mse;
static button_mouse *button_backup;

static int button_start_x;
static int button_start_y;
static int button_length;
static int button_num;


/*
 * Hooks for making and unmaking buttons
 */
button_add_f button_add_hook;
button_kill_f button_kill_hook;



/*** Code ***/

/*
 * The mousebutton code. Buttons should be created when neccessary and
 * destroyed when no longer necessary.  By default, buttons occupy the section
 * of the bottom line between the status display and the location display
 * in normal screen mode, and the bottom line after any prompt in alternate
 * screen mode.
 *
 * Individual ports may (and preferably will) handle this differently using
 * button_add_gui and button_kill_gui.
 */

/*
 * Add a button
 */
int button_add_text(const char *label, unsigned char keypress)
{
	int i;
	int length = strlen(label);

	/* Check the label length */
	if (length > MAX_MOUSE_LABEL)
	{
		bell("Label too long - button abandoned!");
		return 0;
	}

	/* Check we haven't already got a button for this keypress */
	for (i = 0; i < button_num; i++)
		if (button_mse[i].key == keypress)
			return 0;

	/* Make the button */
	button_length += length;
	my_strcpy(button_mse[button_num].label, label, MAX_MOUSE_LABEL);
	button_mse[button_num].left = button_length - length + 1;
	button_mse[button_num].right = button_length;
	button_mse[button_num++].key = keypress;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);

	/* Return the size of the button */
	return (length);
}

/*
 * Add a button
 */
int button_add(const char *label, unsigned char keypress)
{
	if (!button_add_hook)
		return 0;
	else
		return (*button_add_hook) (label, keypress);
}

/*
 * Make a backup of the current buttons
 */
void button_backup_all(void)
{
	/* Check we haven't already done this */
	if (button_backup[0].key) return;

	/* Straight memory copy */
	(void)C_COPY(button_backup, button_mse, MAX_MOUSE_BUTTONS, button_mouse);
}


/*
 * Restore the buttons from backup
 */
void button_restore(void)
{
	int i = 0;

	/* Remove the current lot */
	button_kill_all();

	/* Get all the previous buttons, copy them back */
	while (button_backup[i].key)
	{
		/* Add them all back, forget the backups */
		button_add(button_backup[i].label, button_backup[i].key);
		button_backup[i].key = '\0';
		i++;
	}
}


/*
 * Remove a button
 */
int button_kill_text(unsigned char keypress)
{
	int i, j, length;

	/* Find the button */
	for (i = 0; i < button_num; i++)
		if (button_mse[i].key == keypress) break;

	/* No such button */
	if (i == button_num)
	{
		return 0;
	}

	/* Find the length */
	length = button_mse[i].right - button_mse[i].left + 1;
	button_length -= length;

	/* Move each button up one */
	for (j = i; j < button_num - 1; j++)
	{
		button_mse[j] = button_mse[j + 1];

		/* Adjust length */
		button_mse[j].left -= length;
		button_mse[j].right -= length;
	}

	/* Wipe the data */
	button_mse[button_num].label[0] = '\0';
	button_mse[button_num].left = 0;
	button_mse[button_num].right = 0;
	button_mse[button_num--].key = 0;

	/* Redraw */
	p_ptr->redraw |= (PR_BUTTONS);
	redraw_stuff(p_ptr);

	/* Return the size of the button */
	return (length);
}

/*
 * Kill a button
 */
int button_kill(unsigned char keypress)
{
	if (!button_kill_hook) return 0;
	else
		return (*button_kill_hook) (keypress);
}

/*
 * Kill all buttons
 */
void button_kill_all(void)
{
	int i;

	/* Paranoia */
	if (!button_kill_hook) return;

	/* One by one */
	for (i = button_num - 1; i >= 0; i--)
		(void)(*button_kill_hook) (button_mse[i].key);
}


/*
 * Initialise buttons.
 */
void button_init(button_add_f add, button_kill_f kill)
{
	/* Prepare mouse button arrays */
	button_mse = C_ZNEW(MAX_MOUSE_BUTTONS, button_mouse);
	button_backup = C_ZNEW(MAX_MOUSE_BUTTONS, button_mouse);

	/* Initialise the hooks */
	button_add_hook = add;
	button_kill_hook = kill;
}

void button_hook(button_add_f add, button_kill_f kill)
{
	/* Initialise the hooks */
	button_add_hook = add;
	button_kill_hook = kill;
}

/*
 * Dispose of the button memory
 */
void button_free(void)
{
	FREE(button_mse);
	FREE(button_backup);
}

/**
 * Return the character represented by a button at screen position (x, y),
 * or 0.
 */
char button_get_key(int x, int y)
{
	int i;

	for (i = 0; i < button_num; i++)
	{
		if ((y == button_start_y) &&
		    (x >= button_start + button_mse[i].left) &&
		    (x <= button_start + button_mse[i].right))
		{
			return button_mse[i].key;
		}
	}

	return 0;
}

/**
 * Print the current button list at the specified `row` and `col`umn.
 */
size_t button_print(int row, int col)
{
	int j;

	button_start_x = col;
	button_start_y = row;

	for (j = 0; j < button_num; j++)
		c_put_str(TERM_SLATE, button_mse[j].label, row, col + button_mse[j].left);

	return button_length;
}


#if 0
typedef struct _button_mouse
{
  struct _button_mouse *next;
  byte id;
	wchar_t label[MAX_MOUSE_LABEL]; /*!< Label on the button */
	int left;                    /*!< Column containing the left edge of the button */
	int right;                   /*!< Column containing the right edge of the button */
	int top;                     /*!< Row containing the left edge of the button */
	int bottom;                  /*!< Row containing the right edge of the button */
	ui_event_type type;          /*!< Is the event generated a key or mouse press */
	keycode_t code;              /*!< Keypress corresponding to the button */
  byte mods;                   /*!< modifiers sent with the press */
  byte list;                   /*!< button list to switch to on press */
} button_mouse2;
/* if type is mouse, it is stored in a global, to be used at the location of
 * next non button mouse press. */
typedef struct _button_list
{
  struct _button_list *next;
  byte id;
  byte prev_list;
  byte count;
  btye flags; /* if previous list is not removed from screen, if button miss goes back to previous list */
  button_mouse *list;
} button_list;

extern int button_platform_draw;

int button_add(byte list, button_mouse *src);
int button_add_key(byte list, byte id, keycode_t code, byte mods, wchar_t *label);
int button_add_mouse(byte list, byte id, byte button, byte mods, wchar_t *label);
int button_add_event(byte list, byte id, ui_event *event, wchar_t *label);

int button_set_delayed(byte list, byte id, bool delayed);
int button_set_size(byte list, byte id, int width, int height);
int button_set_pos(byte list, byte id, int y, int x);
int button_set_label(byte list, byte id, wchar_t *label);

int button_set_event(byte list, byte id, ui_event *event);
int button_get_event(byte list, byte id, ui_event *event);
int button_get_event(int x, int y, ui_event *event);
int button_get_event(button_mouse2 *button, ui_event *event);


int button_show(byte list, byte id);
int button_hide(byte list, byte id);

int button_list_push(byte list);
byte button_list_pop(byte list);

int button_kill(byte list, byte id);
int button_kill_list(byte list);
void button_kill_all(void);

void button_init(button_add_f add, button_kill_f kill);
void button_set_hooks(button_add_f add, button_kill_f kill);
void button_free(void);

button_mouse2 *button_get(int x, int y);
byte button_get_current_list_id(void);

size_t button_print(int row, int col);

      // need globals:
      //  ui_event *delayed_event;
      //  button_list *active_list;
      //  button * last_button;
      //  byte button_list_id;
      // global button lists:
      //  initial
      //  main
      //  store
      //  char screen
      //  get item


      /* if found, and has an ui event, inject the event */
      /* if found, and it switches the active button list, switch lists */
#endif
