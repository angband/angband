/**
 * \file sdl2/pui-ctrl.h
 * \brief Declare the interface for controls that can be included in menus or
 * dialogs created by the primitive UI toolkit for SDL2.
 */
#ifndef INCLUDED_SDL2_SDLPUI_CONTROL_H
#define INCLUDED_SDL2_SDLPUI_CONTROL_H

#include "SDL.h" /* SDL_*Event, SDL_Rect, SDL_Renderer, Sint32 */

struct sdlpui_control;
struct sdlpui_dialog;
struct sdlpui_window;

/**
 * Default width for empty space around labels, push buttons, and menu buttons
 * Two is the smallest useful value (one pixel of empty space and one pixel to
 * indicate focus and feedback about starting or continuing an action).
 * Larger than that will leave some blank space between the caption for a
 * control and what is drawn to indicate focus or feedback.
 */
#define SDLPUI_DEFAULT_CTRL_BORDER 3

/**
 * Set out predefined values for the type_code field of struct sdlpui_control.
 * These are initialized by sdlpui_init().  For custom controls, you can get
 * a code with sdlpui_register_code().
 */
extern Uint32 SDLPUI_CTRL_IMAGE;
extern Uint32 SDLPUI_CTRL_LABEL;
extern Uint32 SDLPUI_CTRL_MENU_BUTTON;
extern Uint32 SDLPUI_CTRL_PUSH_BUTTON;

/**
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

/**
 * Since mouse focus and keyboard focus can be directed to different controls
 * within a compound control, provide a hint about the cause for invoking an
 * action on a control.
 */
enum sdlpui_action_hint {
	SDLPUI_ACTION_HINT_NONE,
	SDLPUI_ACTION_HINT_KEY,
	SDLPUI_ACTION_HINT_MOUSE,
	SDLPUI_ACTION_HINT_KEY_OR_MOUSE,
	SDLPUI_ACTION_HINT_KEY_AND_MOUSE
};

/** How to horizontally align something (usually a label) in a larger box. */
enum sdlpui_hor_align {
	SDLPUI_HOR_INVALID = 0,
	SDLPUI_HOR_CENTER,
	SDLPUI_HOR_LEFT,
	SDLPUI_HOR_RIGHT
};

/** How to place a created menu relative to its parent control. */
enum sdlpui_child_menu_placement {
	SDLPUI_CHILD_MENU_ABOVE,
	SDLPUI_CHILD_MENU_BELOW,
	SDLPUI_CHILD_MENU_LEFT,
	SDLPUI_CHILD_MENU_RIGHT
};

/** Holds a function table to be used for a class of controls. */
struct sdlpui_control_funcs {
	/*
	 * Respond to events.  Return SDL_TRUE if the event was handled and
	 * should not be passed on to another handler.  Otherwise, return
	 * SDL_FALSE.  Any can be NULL if the control does not do anything
	 * with that type of event and wants the dialog or window to handle
	 * the event.
	 */
	/**
	 * Respond to a key event.  Return SDL_TRUE if the event was handled
	 * and should not be passed on to another handler.  Otherwise, return
	 * SDL_FALSE.  Can be NULL if the control does not do anything with a
	 * key event and wants the dialog or window to handle the event.
	 */
	SDL_bool (*handle_key)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_KeyboardEvent *e);
	/**
	 * Respond to a text input event.  Return SDL_TRUE if the event was
	 * handled and should not be passed on to another handler.  Otherwise,
	 * return SDL_FALSE.  Can be NULL if the control does not do anything
	 * with a text input event and wants the dialog or window to handle
	 * the event.
	 */
	SDL_bool (*handle_textin)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_TextInputEvent *e);
	/**
	 * Respond to a text edit event.  Return SDL_TRUE if the event was
	 * handled and should not be passed on to another handler.  Otherwise,
	 * return SDL_FALSE.  Can be NULL if the control does not do anything
	 * with a text edit event and wants the dialog or window to handle the
	 * event.
	 */
	SDL_bool (*handle_textedit)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_TextEditingEvent *e);
	/**
	 * Respond to a mouse button event.  Return SDL_TRUE if the event was
	 * handled and should not be passed on to another handler.  Otherwise,
	 * return SDL_FALSE.  Can be NULL if the control does not do anything
	 * with a mouse button event and wants the dialog or window to handle
	 * the event.
	 */
	SDL_bool (*handle_mouseclick)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseButtonEvent *e);
	/**
	 * Respond to a mouse motion event.  Return SDL_TRUE if the event was
	 * handled and should not be passed on to another handler.  Otherwise,
	 * return SDL_FALSE.  Can be NULL if the control does not do anything
	 * with a mouse motion event and wants the dialog or window to handle
	 * the event.
	 */
	SDL_bool (*handle_mousemove)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseMotionEvent *e);
	/**
	 * Respond to a mouse wheel event.  Return SDL_TRUE if the event was
	 * handled and should not be passed on to another handler.  Otherwise,
	 * return SDL_FALSE.  Can be NULL if the control does not do anything
	 * with a mouse wheel event and wants the dialog or window to handle
	 * the event.
	 */
	SDL_bool (*handle_mousewheel)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_MouseWheelEvent *e);
	/**
	 * Return the name for the type of a specific control.  May not be
	 * NULL.
	 */
	const char *(*get_type_name)(const struct sdlpui_control *c);
	/**
	 * Return the caption for the control.  May be NULL.  In that case,
	 * tracing statements use "(none)" when displaying the caption.
	 */
	const char *(*get_caption)(const struct sdlpui_control *c);
	/**
	 * Change the caption for the control.  May be NULL if the control
	 * does not have a caption or otherwise does not want
	 * sdlpui_change_caption() to work with the control.  Does resize
	 * the internals of the control for the new caption but does not
	 * change its external dimensions.
	 */
	void (*change_caption)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption);
	/**
	 * Render the control.  Can be NULL, but then the control will be
	 * invisible.  Must not affect parts of the screen outside the
	 * bounds of c->rect.  Can assume the renderer's target has been set
	 * to d->texture.  If it modifies the renderer's target, it must
	 * restore it to d->texture before returning.  Can not assume
	 * anything about the drawing color in the renderer and does not
	 * need to restore the prior drawing color before returning.  Can
	 * assume that the blending mode in the renderer is SDL_BLENDMODE_NONE.
	 * Must restore the blending mode to SDL_BLENDMODE_NONE before
	 * returning.  If changes the clipping rectangle, scale, integer
	 * scale, logical size, or viewport in the renderer, must restore
	 * the prior state before returning.  Must not alter the VSync of the
	 * renderer.
	 */
	void (*render)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r);
	/** Do the default action for a control.  Can be NULL. */
	void (*respond_default)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint);
	/**
	 * Signal that the given control has gained focus or already has focus
	 * but the component with focus has changed.  ah specifies the type of
	 * focus:  SDLPUI_ACTION_HINT_KEY for keyboard focus or
	 * SDLPUI_ACTION_HINT_MOUSE_FOCUS for mouse focus.  comp_ind is only
	 * relevant for compound controls and is the zero-based index of the
	 * component that should receive focus.  Can be NULL:  the control
	 * either does not accept focus (get_interactable_component is NULL
	 * or always returns zero and get_interactable_component_at is NULL
	 * or always returns zero) or does not need to do anything beyond
	 * what is done by the functions in pui-foc.h.  The caller will signal
	 * that the containing dialog and window need redrawing so this hook
	 * does not need to do that.  When called, the state returned by the
	 * routines in pui-foc.h refers to the state after the change of
	 * focus.  This hook should be treated as for the private use of the
	 * routines in pui-foc.h:  all others should use those routines to
	 * manage where focus is routed.
	 */
	void (*gain_focus)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint ah,
		int comp_ind);
	/**
	 * Signal that the given control has lost focus.  ah specifies the type
	 * of focus:  SDLPUI_ACTION_HINT_KEY for keyboard focus or
	 * SDLPUI_ACTION_HINT_MOUSE_FOCUS for mouse focus.  new_c is the
	 * control gaining key focus; it will be NULL if no control is taking
	 * focus or new_d is NULL.  new_d is the dialog gaining key focus; it
	 * will be NULL if no dialog is taking focus or the dialog is unknown
	 * (in another window).  Can be NULL:  the control either does not
	 * accept focus (get_interactable_component is NULL or always returns
	 * zero and get_interactable_component_at is NULL or always returns
	 * zero) or does not need to do anything beyond what is done by the
	 * functions in pui-foc.h.  The caller will signal that the containing
	 * dialog and window need redrawing so this hook does not need to do
	 * that.  When called, the state returned by the routines in pui-foc.h
	 * refers to the state after the change of focus.  This hook should be
	 * treated as for the private use of the routines in pui-foc.h:  all
	 * others should use those routines to manage where focus is routed.
	 */
	void (*lose_focus)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint ah,
		struct sdlpui_control *new_c, struct sdlpui_dialog *new_d);
	/**
	 * Signal that the child dialog for a control has been removed.  Can
	 * be NULL if the control does not create a dialog, set the created
	 * dialog's parent control to the control, or record the pointer
	 * to the dialog.
	 */
	void (*lose_child)(struct sdlpui_control *c,
		struct sdlpui_dialog *child);
	/**
	 * For simple controls, either returns zero (the control does not
	 * accept focus) or one (it does).  For compound controls it returns:
	 *     a) Zero if none of the components accepts focus.
	 *     b) If first is not SDL_FALSE, returns the one-based index of
	 *        the first component that can accept focus.
	 *     c) If first is SDL_FALSE, returns the one-based index of the last
	 *        component that can accept focus.
	 * May be NULL:  callers will then assume the control cannot accept
	 * focus.
	 */
	int (*get_interactable_component)(struct sdlpui_control *c,
		SDL_bool first);
	/**
	 * Return the one-based index of the next (if forward is true) or
	 * previous (if forward is false) component to receive focus.  The
	 * current component with focus can be retrieved with
	 * sdlpui_control_has_focus().  Return zero if there is not a component
	 * that can receive focus for the step specified by forward.  Do not
	 * wrap around when considering which component should receive focus.
	 * May be NULL:  callers will then assume that any attempt to step
	 * within the control will be ineffective.
	 */
	int (*step_within)(struct sdlpui_control *c, SDL_bool forward);
	/**
	 * For a simple control, either returns zero (the control does not
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
	/**
	 * Resize the control so it has the given dimensions.  May be NULL
	 * if resizing the control is as simple as setting c->rect.w and
	 * c->rect.h to the desired dimensions.
	 */
	void (*resize)(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
	/**
	 * Set width and height to the natural size for the control.  May not
	 * be NULL.
	 */
	void (*query_natural_size)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w, int *width,
		int *height);
	/**
	 * Get whether the control is disabled.  May be NULL:  the control
	 * does not support enabling/disabling.
	 */
	SDL_bool (*is_disabled)(const struct sdlpui_control *c);
	/**
	 * Change whether the control is disabled and return whether or not
	 * its prior state was disabled.  May be NULL:  the control does not
	 * support enabling/disabling.
	 */
	SDL_bool (*set_disabled)(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		SDL_bool disabled);
	/**
	 * Get the application-assigned tag for a control.  May be NULL:  the
	 * control does not support application-assigned tags.
	 */
	int (*get_tag)(const struct sdlpui_control *c);
	/**
	 * Change the application-assigned tag for a control and return its
	 * prior tag.  May be NULL:  the control does not support
	 * application-assigned tags.
	 */
	int (*set_tag)(struct sdlpui_control *c, int new_tag);
	/**
	 * Handle releasing resources for the private data, if any.  May be
	 * NULL to have no special cleanup done.
	 */
	void (*cleanup)(struct sdlpui_control *c);
};

/** Represents a button or other sort of control in a dialog or menu. */
struct sdlpui_control {
	const struct sdlpui_control_funcs *ftb;
	/** Holds data specific to the particular type of control. */
	void *priv;
	/**
	 * Holds the position, relative to the containing dialog/menu's
	 * upper left corner, and size of the control.
	 */
	SDL_Rect rect;
	/**
	 * Holds a number from sdlpui_reserve_id() to use in identity checks.
	 */
	Uint32 id;
	/** Allow for a check before casting priv to another type. */
	Uint32 type_code;
};

/**
 * Holds the private data for a sdlpui_control used to represent an image for
 * use in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_IMAGE.
 */
struct sdlpui_image {
	/** x and y are relative to the (x, y) from the control's rectangle. */
	SDL_Rect image_rect;
	SDL_Texture *image;
	enum sdlpui_hor_align halign;
	int top_margin, bottom_margin, left_margin, right_margin;
};

/**
 * Holds the private data for a sdlpui_control used to represent a single line
 * label for use in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_LABEL.
 */
struct sdlpui_label {
	/** x and y are relative to the (x, y) from the control's rectangle. */
	SDL_Rect caption_rect;
	char *caption;
	enum sdlpui_hor_align halign;
};

/**
 * Holds the private data for a sdlpui_control used to represent a button in
 * menu.  The corressponding type_code value is SDLPUI_CTRL_MENU_BUTTON.
 */
struct sdlpui_menu_button {
	SDL_Rect caption_rect;
	char *caption;
	/** Invoked by the menu button's respond_default handler. */
	void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
	enum sdlpui_hor_align halign;
	/**
	 * This is a hook for the application to differentiate buttons with
	 * the same contents for callback, subtype_code, and v.
	 */
	int tag;
	SDL_bool disabled;
			/**< if not SDL_FALSE, no response to events and
				different look */
	enum sdlpui_menu_button_type subtype_code;
	union {
		struct {
			char *expanded_caption;
			int min, max, curr, old;
		} ranged_int;
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
		SDL_bool toggled; /**< subtype_code is SDLPUI_MB_INDICATOR or
					SDLPUI_MB_TOGGLE */
	} v;
};

/**
 * Holds the private data for a sdlpui_control used to represent a push button
 * used in general dialogs.  The corresponding type_code value is
 * SDLPUI_CTRL_PUSH_BUTTON.
 */
struct sdlpui_push_button {
	SDL_Rect caption_rect;
	char *caption;
	/** Invoked by the push button's respond_default handler. */
	void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, enum sdlpui_action_hint hint);
	enum sdlpui_hor_align halign;
	/**
	 * This is a hook for the application to differentiate push buttons
	 * with the same callback.
	 */
	int tag;
	SDL_bool disabled;	/**< if not SDL_FALSE, no response to events and
					different look */
};


SDL_bool sdlpui_is_in_control(const struct sdlpui_control *c,
		const struct sdlpui_dialog *d, Sint32 x, Sint32 y);
SDL_bool sdlpui_is_disabled(const struct sdlpui_control *c);
SDL_bool sdlpui_set_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_bool disabled);
int sdlpui_get_tag(const struct sdlpui_control *c);
int sdlpui_set_tag(struct sdlpui_control *c, int new_tag);
void sdlpui_change_caption(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, const char *new_caption);

/* Standard event handlers for simple controls */
SDL_bool sdlpui_control_handle_key(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_KeyboardEvent *e);
SDL_bool sdlpui_control_handle_mouseclick(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseButtonEvent *e);
SDL_bool sdlpui_control_handle_mousemove(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseMotionEvent *e);

/* Standard callbacks for button controls */
void sdlpui_invoke_dialog_default_action(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint);

/* Constructors for controls in generic dialogs */
void sdlpui_create_image(struct sdlpui_control *c, SDL_Texture *image,
		enum sdlpui_hor_align halign, int top_margin, int bottom_margin,
		int left_margin, int right_margin);
void sdlpui_create_label(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign);
void sdlpui_create_push_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, enum sdlpui_action_hint), int tag,
		SDL_bool disabled);

/* Constructors for controls in menus */
void sdlpui_create_menu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, enum sdlpui_action_hint), int tag,
		SDL_bool disabled);
void sdlpui_create_menu_indicator(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, int tag, SDL_bool curr_value);
void sdlpui_create_menu_ranged_int(struct sdlpui_control *c,
		const char *caption, enum sdlpui_hor_align halign,
		void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, enum sdlpui_action_hint), int tag,
		SDL_bool disabled,
		int curr_value, int min_value, int max_value);
void sdlpui_create_menu_toggle(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, enum sdlpui_action_hint), int tag,
		SDL_bool disabled, SDL_bool curr_value);
void sdlpui_create_submenu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, struct sdlpui_dialog *(*creator)(
		struct sdlpui_control*, struct sdlpui_dialog*, struct
		sdlpui_window*, int ul_x_win, int ul_y_win),
		enum sdlpui_child_menu_placement placement, int tag,
		SDL_bool disabled);

#endif /* INCLUDED_SDL2_SDLPUI_CONTROL_H */
