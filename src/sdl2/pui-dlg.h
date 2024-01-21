/**
 * \file sdl2/pui-dlg.h
 * \brief Declare the interface for menus and dialogs created by the primitive
 * UI toolkit for SDL2.
 */
#ifndef INCLUDED_SDL2_SDLPUI_DIALOG_H
#define INCLUDED_SDL2_SDLPUI_DIALOG_H

#include "pui-ctrl.h"

struct sdlpui_dialog;
struct sdlpui_window;

/*
 * Set out predefined values for the type_code field of struct sdlpui_dialog.
 * These are initialized by sdlpui_init().  For custom dialogs, you can get
 * a code with sdlpui_register_code().
 */
extern Uint32 SDLPUI_DIALOG_SIMPLE_MENU;
extern Uint32 SDLPUI_DIALOG_SIMPLE_INFO;

/* Set out possible flags that can be set for buttons in a simple menu. */
enum sdlpui_menu_flags {
	SDLPUI_MFLG_NONE = 0,
	SDLPUI_MFLG_END_GRAVITY = 1,	/* when the menu is bigger than needed
						for the buttons, the button
						prefers to have its position
						stack from the end of the
						menu */
	SDLPUI_MFLG_CAN_HIDE = 2,	/* if the menu is smaller than its
						natural size, this button can
						be hidden */
};

/* Holds a function table to be used for a class of dialogs. */
struct sdlpui_dialog_funcs {
	/*
	 * Respond to events.  Return true if the event was handled and
	 * should not be passed on to another handler.  Otherwise, return false.
	 * Any can be NULL if the dialog and the controls it contains do not
	 * do anything with that type of event and want the window to handle it.
	 */
	bool (*handle_key)(struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_KeyboardEvent *e);
	bool (*handle_textin)(struct sdlpui_dialog *d, struct sdlpui_window *w,
		const SDL_TextInputEvent *e);
	bool (*handle_textedit)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_TextEditingEvent *e);
	bool (*handle_mouseclick)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_MouseButtonEvent *e);
	bool (*handle_mousemove)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_MouseMotionEvent *e);
	bool (*handle_mousewheel)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const SDL_MouseWheelEvent *e);
	/*
	 * Respond to the mouse focus being taken by another dialog.  May be
	 * NULL.  new_c is the control taking focus, it may be NULL.  new_d
	 * is the dialog taking focus, it may be NULL.
	 */
	void (*handle_loses_mouse)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
	/*
	 * Respond to the key focus being taken by another dialog.  May be NULL.
	 * new_c is the control taking focus, it may be NULL.  new_d is the
	 * dialog taking focus; it may be NULL.
	 */
	void (*handle_loses_key)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
	/*
	 * Respond to the mouse leaving the containing window if the dialog
	 * had mouse focus when that happened.  May be NULL.
	 */
	void (*handle_window_loses_mouse)(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
	/*
	 * Respond to the containing window losing key focus if the dialog
	 * had key focus when that happened.  May be NULL.
	 */
	void (*handle_window_loses_key)(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
	/*
	 * Redraw for the current state of the dialog/menu.  Can be NULL,
	 * but then the dialog is not redrawn by standard event handling.
	 */
	void (*render)(struct sdlpui_dialog *d, struct sdlpui_window *w);
	/* Do the default action for a dialog or menu.  Can be NULL. */
	void (*respond_default)(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
	/*
	 * Go to the dialog's primary (or first) control that can accept
	 * focus and give it key focus.  May be NULL if there is nothing in the
	 * dialog that can accept focus.
	 */
	void (*goto_first_control)(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
	/*
	 * If forward is true, go to the dialog's next (with wrap around)
	 * control after c that can accept focus.  If forward is false, go to
	 * the dialog's previous (with wrap around) control before c that
	 * can accept focus.  May be NULL if the dialog never accepts focus
	 * (goto_first_control is NULL or never changes d->c_key from NULL and
	 * find_control_containing is NULL or always returns NULL) or if it
	 * only has one, simple, control that can accept focus.
	 */
	void (*step_control)(struct sdlpui_dialog *d, struct sdlpui_window *w,
		struct sdlpui_control *c, bool forward);
	/*
	 * Find the dialog's control that's willing to accept focus and
	 * contains the given coordinate, relative to the window.  For simple
	 * controls, set *comp_ind to zero.  For compound controls, set
	 * *comp_ind to the index of the control, in the compound control, that
	 * accepts focus and holds the coordinate.  May be NULL:  then mouse
	 * motion will never cause the dialog to accept focus.
	 */
	struct sdlpui_control *(*find_control_containing)(
		struct sdlpui_dialog *d, struct sdlpui_window *w, Sint32 x,
		Sint32 y, int *comp_ind);
	/*
	 * For a nested menu, return the parent or child respectively for the
	 * menu.  May be NULL for a dialog that is not a nested menu.
	 */
	struct sdlpui_dialog *(*get_parent)(struct sdlpui_dialog *d);
	struct sdlpui_dialog *(*get_child)(struct sdlpui_dialog *d);
	/*
	 * For a nested menu, get the parent control for the menu.  May be
	 * NULL for a dialog that is not a nested menu.
	 */
	struct sdlpui_control *(*get_parent_ctrl)(struct sdlpui_dialog *d);
	/*
	 * For a nested menu, allow changing the child menu.  May be NULL
	 * for a dialog that is not a nested menu.
	 */
	void (*set_child)(struct sdlpui_dialog *d, struct sdlpui_dialog *child);
	/*
	 * Resize the dialog so it has the given dimensions.  May be NULL
	 * if resizing the dialog is as simple as setting d->rect.w and
	 * d->rect.h to the desired dimensions.
	 */
	void (*resize)(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height);
	/* Get the natural size for the dialog.  May not be NULL. */
	void (*query_natural_size)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
	/*
	 * Get the minimum size for the dialog.  May be NULL:  the caller
	 * will assume the natural size is the minimum size.
	 */
	void (*query_minimum_size)(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
	/*
	 * Handle releasing resources for the private data, if any.  May
	 * be NULL to have no special cleanup done.
	 */
	void (*cleanup)(struct sdlpui_dialog *d);
};

/* Represents a menu or dialog displayed on top of a window. */
struct sdlpui_dialog {
	const struct sdlpui_dialog_funcs *ftb;
	/*
	 * Called with up set to true when popping the dialog up.  Called
	 * with up set to false when popping the dialog down.
	 */
	void (*pop_callback)(struct sdlpui_dialog *d, struct sdlpui_window *w,
		bool up);
	/*
	 * Managed by the containing window so it can keep a stack of its
	 * menus and dialogs.
	 */
	struct sdlpui_dialog *next, *prev;
	/*
	 * Holds the rendered contents of the dialog/menu.  May be NULL to
	 * directly render to the window's backing buffer.
	 */
	SDL_Texture *texture;
	/*
	 * These point to the control which should receive mouse or keyboard
	 * events, respectively.  If NULL, events will be directed to the
	 * dialog itself.
	 */
	struct sdlpui_control *c_mouse, *c_key;
	/* Holds menu/dialog-specific data. */
	void *priv;
	/*
	 * Holds the position, relative to the window's upper left corner,
	 * and size of the dialog/menu.
	 */
	SDL_Rect rect;
	/* Allow for a check before casting priv to another type. */
	Uint32 type_code;
	/*
	 * Allow for different behavior for different dialogs with the same
	 * pop_callback.
	 */
	int tag;
	/*
	 * The dialog/menu is pinned and should not be automatically removed
	 * when popping down a child.
	 */
	bool pinned;
	/*
	 * Dialog/menu's texture is out-of-date with respect to the state of
	 * the dialog and should be rerendered.
	 */
	bool dirty;
};


/*
 * Holds the private data for a sdlpui_dialog used to represent a simple menu
 * (either a standalone popup menu, a vertical menu pane that's part of a
 * system of nested menus, or a horizontal menu bar).  The corresponding
 * type_code value is SDLPUI_DIALOG_SIMPLE_MENU.
 */
struct sdlpui_simple_menu {
	struct sdlpui_dialog *parent, *child;
	struct sdlpui_control *parent_ctrl;
	struct sdlpui_control *controls;
					/* flat array of the menu buttons */
	struct sdlpui_control **v_ctrls;
					/* flat array referring to the subset
						of buttons that are visible */
	int *control_flags;		/* flat array of flags
						(SDLPUI_MFLG_END_GRAVITY and
						SDLPUI_MFLG_CAN_HIDE) for each
						button */
	int size, number, n_vis;	/* allocated number of buttons for
						controls, a_ctrls, and
						control_flags; the number of
						buttons in controls and
						control_flags; the number of
						buttons in v_ctrls */
	bool vertical;
	bool border;			/* is it rendered with a border */
};


/*
 * Holds the private data for a sdlpui_dialog used to a dialog with zero or
 * more labels or images and a button that dismisses the dialog.  The
 * corresponding type_code value is SDLPUI_DIALOG_SIMPLE_INFO.
 */
struct sdlpui_simple_info {
	struct sdlpui_control *labels;	/* flat array of controls like labels
						or images that don't accept
						focus */
	struct sdlpui_control button;	/* single button to dismiss the
						dialog */
	int size, number;		/* allocated number of controls for
						labels; the number of
						controls in labels */
};


bool sdlpui_is_in_dialog(const struct sdlpui_dialog *d, Sint32 x, Sint32 y);
bool sdlpui_is_descendant_dialog(struct sdlpui_dialog *ancestor,
		const struct sdlpui_dialog *other);
void sdlpui_popup_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		bool give_key_focus);
void sdlpui_popdown_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		bool all_parents);
void sdlpui_dialog_give_key_focus_to_parent(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
struct sdlpui_dialog *sdlpui_get_dialog_parent(struct sdlpui_dialog *d);
struct sdlpui_dialog *sdlpui_get_dialog_child(struct sdlpui_dialog *d);
struct sdlpui_control *sdlpui_get_dialog_parent_ctrl(struct sdlpui_dialog *d);

/* Standard event handlers for dialogs. */
bool sdlpui_dialog_handle_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e);
bool sdlpui_dialog_handle_textin(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextInputEvent *e);
bool sdlpui_dialog_handle_textedit(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextEditingEvent *e);
bool sdlpui_dialog_handle_mouseclick(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseButtonEvent *e);
bool sdlpui_dialog_handle_mousemove(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseMotionEvent *e);
bool sdlpui_dialog_handle_mousewheel(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseWheelEvent *e);
void sdlpui_dismiss_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w);
void sdlpui_dialog_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
void sdlpui_menu_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
void sdlpui_dialog_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
void sdlpui_menu_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
void sdlpui_dialog_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
void sdlpui_menu_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
void sdlpui_dialog_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
void sdlpui_menu_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);

/* Construct a simple menu */
struct sdlpui_dialog *sdlpui_start_simple_menu(struct sdlpui_dialog *parent,
		struct sdlpui_control *parent_ctrl, int preallocated,
		bool vertical, bool border, void (*pop_callback)(
			struct sdlpui_dialog *d, struct sdlpui_window *w,
			bool up), int tag);
struct sdlpui_control *sdlpui_get_simple_menu_next_unused(
		struct sdlpui_dialog *d, int flags);
void sdlpui_complete_simple_menu(struct sdlpui_dialog *d,
		struct sdlpui_window *w);

/* Construct a simple information dialog. */
struct sdlpui_dialog *sdlpui_start_simple_info(const char *button_label,
		void (*pop_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, bool up), int tag);
void sdlpui_simple_info_add_image(struct sdlpui_dialog *d, SDL_Texture *image,
		enum sdlpui_hor_align halign, int top_margin, int bottom_margin,
		int left_margin, int right_margin);
void sdlpui_simple_info_add_label(struct sdlpui_dialog *d, const char *label,
		enum sdlpui_hor_align halign);
void sdlpui_complete_simple_info(struct sdlpui_dialog *d,
		struct sdlpui_window *w);

#endif /* INCLUDED_SDL2_SDLPUI_DIALOG_H */
