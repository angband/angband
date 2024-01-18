/**
 * \file sdl2/pui-ctrl.h
 * \brief Declare the interface for controls that can be included in menus or
 * dialogs created by the primitive UI toolkit for SDL2.
 */
#ifndef INCLUDED_SDL2_SDLPUI_CONTROL_H
#define INCLUDED_SDL2_SDLPUI_CONTROL_H

#include "SDL.h" /* SDL_*Event, SDL_Rect, SDL_Renderer, Sint32 */
#include <stdbool.h>

struct sdlpui_control;
struct sdlpui_dialog;
struct sdlpui_window;

/*
 * Default width for empty space around labels, push buttons, and menu buttons
 * Two is smallest useful value (one pixel to indicate focus; another to
 * indicate arming).  Larger than that will leave some blank space between
 * the caption for a control and what's drawn to indicate focus or arming.
 */
#define SDLPUI_DEFAULT_CTRL_BORDER 3

/*
 * Set out predefined values for the type_code field of struct sdlpui_control.
 * These are initialized by sdlpui_init().  For custom controls, you can get
 * a code with sdlpui_register_code().
 */
extern Uint32 SDLPUI_CTRL_IMAGE;
extern Uint32 SDLPUI_CTRL_LABEL;
extern Uint32 SDLPUI_CTRL_MENU_BUTTON;
extern Uint32 SDLPUI_CTRL_PUSH_BUTTON;

/*
 * Set out possible values for the subtype_code field of struct
 * sdlpui_menu_button.
 */
enum sdlpui_menu_button_type {
	SDLPUI_MB_INVALID = 0,
	SDLPUI_MB_NONE,
	SDLPUI_MB_INDICATOR,
	SDLPUI_MB_RANGED_INT,
	SDLPUI_MB_SUBMENU,
	SDLPUI_MB_TOGGLE,
};

/*
 * Since mouse focus and keyboard focus can be directed to different controls
 * within a compound control, provide a hint about the cause for invoking an
 * action on a control.
 */
enum sdlpui_action_hint {
	SDLPUI_ACTION_HINT_NONE,
	SDLPUI_ACTION_HINT_KEY,
	SDLPUI_ACTION_HINT_MOUSE
};

/* How to horizontally align something (usually a label) in a larger box. */
enum sdlpui_hor_align {
	SDLPUI_HOR_INVALID = 0,
	SDLPUI_HOR_CENTER,
	SDLPUI_HOR_LEFT,
	SDLPUI_HOR_RIGHT
};

/* How to place a created menu relative to its parent control. */
enum sdlpui_child_menu_placement {
	SDLPUI_CHILD_MENU_ABOVE,
	SDLPUI_CHILD_MENU_BELOW,
	SDLPUI_CHILD_MENU_LEFT,
	SDLPUI_CHILD_MENU_RIGHT
};

/* Holds a function table to be used for a class of controls. */
struct sdlpui_control_funcs {
	/*
	 * Respond to events.  Return true if the event was handled and
	 * shouldn't be passed on to another handler.  Otherwise, return false.
	 * Any can be NULL if the control doesn't do anything with that type
	 * of event and wants the dialog or window to handle the event.
	 */
	bool (*handle_key)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_KeyboardEvent *e);
	bool (*handle_textin)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_TextInputEvent *e);
	bool (*handle_textedit)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_TextEditingEvent *e);
	bool (*handle_mouseclick)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseButtonEvent *e);
	bool (*handle_mousemove)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseMotionEvent *e);
	bool (*handle_mousewheel)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseWheelEvent *e);
	/*
	 * Change the caption for the control.  May be NULL if the control
	 * does not have a caption or otherwise does not want
	 * sdlpui_change_caption() to work with the control.  Does resize
         * the internals of the control for the new caption but does not
         * change its external dimensions.
	 */
	void (*change_caption)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption);
	/*
	 * Render the control.  Can be NULL, but then the control will be
	 * invisible.  Assumes the renderer's target has been set to
	 * d->texture.
	 */
	void (*render)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r);
	/* Do the default action for a control.  Can be NULL. */
	void (*respond_default)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint);
	/*
	 * Signal that the given control has gained keyboard focus and
	 * perhaps should change its appearance when rendered.  comp_ind is
	 * only relevant for compound controls and is the index of the component
	 * that should receive focus.  Negative indices are relative to the
	 * number of controls in the compound control so -1 is the "last"
	 * control.  Can be NULL.
	 */
	void (*gain_key)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
	/*
	 * Signal that the given control has lost keyboard focus.  Can be NULL.
	 * new_c is the control gaining key focus; it will be NULL if no
	 * control is taking focus or new_d is NULL.
	 * new_d is the dialog gaining key focus; it will be NULL if no
	 * dialog is taking focus or the dialog is unknown (in another
	 * window).
	 */
	void (*lose_key)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
	/*
	 * Signal that the given control has gained mouse focus and
	 * perhaps should change its appearance when rendered.  comp_ind
	 * has the same meaning as for gain_key above.  Can be NULL.
	 */
	void (*gain_mouse)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
	/*
	 * Signal that the given control has lost mouse focus.  Can be NULL.
	 * new_c is the control gaining mouse focus; it will be NULL if no
	 * control is taking focus or new_d is NULL.
	 * new_d is the dialog gaining mouse focus; it will be NULL if no
	 * dialog is taking focus or the dialog is unknown (in another
	 * window).
	 */
	void (*lose_mouse)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
	/*
	 * Signal that the child dialog for a control has been removed.  Can
	 * be NULL if the control doesn't create a dialog, set the created
	 * dialog's parent control to the control, or record the pointer
	 * to the dialog.
	 */
	void (*lose_child)(struct sdlpui_control *c,
		struct sdlpui_dialog *child);
	/*
	 * Signal that the control has become armed (i.e. key or mouse button
	 * depressed while the control has focus).  Can be NULL.
	 */
	void (*arm)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
	/*
	 * Signal that the control has become disarmed (i.e. key or mouse button
	 * released while the control has focus).  Can be NULL.
	 */
	void (*disarm)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
	/*
	 * For simple controls, either returns zero (the control doesn't
	 * accept focus) or one (it does).  For compound controls it returns:
	 *     a) Zero if none of the components accepts focus.
	 *     b) If first is true, returns the one-based index of the first
	 *        component that can accept focus.
	 *     c) If first is false, returns the one-based index of the last
	 *        component that can accept focus.
	 * May be NULL:  callers will then assume the control can't accept
	 * focus.
	 */
	int (*get_interactable_component)(struct sdlpui_control *c, bool first);
	/*
	 * Step (forward if forward is true; backward otherwise; never wrap
	 * around) between the interactable components within the given
	 * control, assumed to already have key focus, and transfer the key
	 * focus to the result of the step.  Return true if stepping was
	 * possible or false otherwise.  May be NULL:  callers will then assume
	 * that any attempt to step within the control will be ineffective.
	 */
	bool (*step_within)(struct sdlpui_control *c, bool forward);
	/*
	 * For a simple control, either returns zero (the control doesn't
	 * accept focus or contain the given coordinates, relative to the
	 * dialog) or one (the control accepts focus and the given coordinates
	 * are in the control).  For a compound control return zero if there's
	 * no component that accepts focus and contains the given coordinates.
	 * May be NULL:  callers will then assume the control is simple,
	 * accepts focus if get_interactable_component is not NULL and
	 * returns a non-zero value, and will test the coordinate directly
	 * against the control's rectangle.
	 */
	int (*get_interactable_component_at)(struct sdlpui_control *c,
		Sint32 x, Sint32 y);
	/*
	 * Resize the control so it has the given dimensions.  May be NULL
	 * if resizing the control is as simple as setting c->rect.w and
	 * c->rect.h to the desired dimensions.
	 */
	void (*resize)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
	/*
	 * Set width and height to the natural size for the control.  May not
	 * be NULL.
	 */
	void (*query_natural_size)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w, int *width,
		int *height);
	/*
	 * Get whether the control is disabled.  May be NULL:  the control
	 * doesn't support enabling/disabling.
	 */
	bool (*is_disabled)(const struct sdlpui_control *c);
	/*
	 * Change whether the control is disabled and return whether or not
	 * its prior state was disabled.  May be NULL:  the control doesn't
	 * support enabling/disabling.
	 */
	bool (*set_disabled)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled);
	/*
	 * Get the application-assigned tag for a control.  May be NULL:  the
	 * control doesn't support application-assigned tags.
	 */
	int (*get_tag)(const struct sdlpui_control *c);
	/*
	 * Change the application-assigned tag for a control and return its
	 * prior tag.  May be NULL:  the control doesn't support
	 * application-assigned tags.
	 */
	int (*set_tag)(struct sdlpui_control *c, int new_tag);
	/*
	 * Handle releasing resources for the private data, if any.  May be
	 * NULL to have no special cleanup done.
	 */
	void (*cleanup)(struct sdlpui_control *c);
};

/* Represents a button or other sort of control in a dialog or menu. */
struct sdlpui_control {
	const struct sdlpui_control_funcs *ftb;
	/* Holds data specific to the particular type of control. */
	void *priv;
	/*
	 * Holds the position, relative to the containing dialog/menu's
	 * upper left corner, and size of the control.
	 */
	SDL_Rect rect;
	/* Allow for a check before casting priv to another type. */
	Uint32 type_code;
};

/*
 * Holds the private data for a sdlpui_control used to represent an image for
 * use in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_IMAGE.
 */
struct sdlpui_image {
	/* x and y are relative to the (x, y) from the control's rectangle. */
	SDL_Rect image_rect;
	SDL_Texture *image;
	enum sdlpui_hor_align halign;
	int top_margin, bottom_margin, left_margin, right_margin;
};

/*
 * Holds the private data for a sdlpui_control used to represent a single line
 * label for use in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_LABEL.
 */
struct sdlpui_label {
	/* x and y are relative to the (x, y) from the control's rectangle. */
	SDL_Rect caption_rect;
	char *caption;
	enum sdlpui_hor_align halign;
};

/*
 * Holds the private data for a sdlpui_control used to represent a button in
 * menu.  The corressponding type_code value is SDLPUI_CTRL_MENU_BUTTON.
 */
struct sdlpui_menu_button {
	SDL_Rect caption_rect;
	char *caption;
	/* Invoked by the menu button's respond_default handler. */
	void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window *w);
	enum sdlpui_hor_align halign;
	/*
	 * This is a hook for the application to differentiate buttons with
	 * the same contents for callback, subtype_code, and v.
	 */
	int tag;
	int has_key;	/* 0 = no key focus, 1 = key focus (for ranged_int
				button, left side has focus), 2 = key focus
				for right side of ranged_int button */
	int has_mouse;	/* 0 = no mouse focus, 1 = mouse focus (for ranged_int
				button, left side has focus), 2 = mouse focus
				for right side of ranged_int button */
	int armed;	/* 0 = not depressed, 1 = button depressed (for
				ranged_int button, left side depressed), 2 =
				ranged_int button right side depressed,
				3 = ranged_int button both sides depressed */
	bool disabled;	/* if true, no response to events and different look */
	enum sdlpui_menu_button_type subtype_code;
	union {
		struct { int min, max, curr, old; } ranged_int;
		struct {
			struct sdlpui_dialog *(*creator)(
				struct sdlpui_control*,
				struct sdlpui_dialog*,
				struct sdlpui_window*,
				int ul_x_win,
				int ul_y_win);
			struct sdlpui_dialog *child;
			enum sdlpui_child_menu_placement placement;
		} submenu;
		bool toggled; /* subtype_code is SDLPUI_MB_INDICATOR or
					SDLPUI_MB_TOGGLE */
	} v;
};

/*
 * Holds the private data for a sdlpui_control used to represent a push button
 * used in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_PUSH_BUTTON.
 */
struct sdlpui_push_button {
	SDL_Rect caption_rect;
	char *caption;
	/* Invoked by the push button's respond_default handler. */
	void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*);
	enum sdlpui_hor_align halign;
	/*
	 * This is a hook for the application to differentiate push buttons
	 * with the same callback.
	 */
	int tag;
	bool disabled;	/* if true, no response to events and different look */
	bool has_key;	/* if true, has key focus */
	bool has_mouse;	/* if true, has mouse focus */
	bool armed;	/* if true, button is depressed */
};


bool sdlpui_is_in_control(const struct sdlpui_control *c,
		const struct sdlpui_dialog *d, Sint32 x, Sint32 y);
bool sdlpui_is_disabled(const struct sdlpui_control *c);
bool sdlpui_set_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled);
int sdlpui_get_tag(const struct sdlpui_control *c);
int sdlpui_set_tag(struct sdlpui_control *c, int new_tag);
void sdlpui_change_caption(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, const char *new_caption);

/* Standard event handlers for simple controls */
bool sdlpui_control_handle_key(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_KeyboardEvent *e);
bool sdlpui_control_handle_mouseclick(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseButtonEvent *e);
bool sdlpui_control_handle_mousemove(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseMotionEvent *e);

/* Standard callbacks for button controls */
void sdlpui_invoke_dialog_default_action(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w);

/* Constructors for controls in generic dialogs */
void sdlpui_create_image(struct sdlpui_control *c, SDL_Texture *image,
		enum sdlpui_hor_align halign, int top_margin, int bottom_margin,
		int left_margin, int right_margin);
void sdlpui_create_label(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign);
void sdlpui_create_push_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled);

/* Constructors for controls in menus */
void sdlpui_create_menu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled);
void sdlpui_create_menu_indicator(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, int tag, bool curr_value);
void sdlpui_create_menu_ranged_int(struct sdlpui_control *c,
		const char *caption, enum sdlpui_hor_align halign,
		void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled, int curr_value,
		int min_value, int max_value);
void sdlpui_create_menu_toggle(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled,
		bool curr_value);
void sdlpui_create_submenu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, struct sdlpui_dialog *(*creator)(
		struct sdlpui_control*, struct sdlpui_dialog*, struct
		sdlpui_window*, int ul_x_win, int ul_y_win),
		enum sdlpui_child_menu_placement placement, int tag,
		bool disabled);

#endif /* INCLUDED_SDL2_SDLPUI_CONTROL_H */
