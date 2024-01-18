/**
 * \file sdl2/pui-win.h
 * \brief Make declarations to connect the SDL2 front end to a primitive UI
 * toolkit based on SDL2 that will handle dialogs and menus overlayed on the
 * front end's windows.  All functions declared here have to be implemented
 * by the front end:  they are not implemented by the primitive UI toolkit.
 */
#ifndef INCLUDED_SDL2_SDLPUI_WINDOW_H
#define INCLUDED_SDL2_SDLPUI_WINDOW_H

#include "SDL.h"	/* SDL_Color, SDL_Renderer, SDL_Texture */
#include "SDL_ttf.h"	/* TTF_Font */


/* Forward declarations for the primitive UI toolkit implementation */
struct sdlpui_dialog;

/*
 * Forward declaration for the application's data associated with an
 * SDL_Window; the UI toolkit does not care about its internals
 */
struct sdlpui_window;

struct sdlpui_stipple {
	SDL_Texture *texture;
	int w, h;		/* width and height */
};


/**
 * Get the SDL_Renderer that the UI toolkit can use to render.
 *
 * \param w is the window containing what's to be rendered.
 * \return the renderer that can be used to render directly to the window
 * or to a texture.
 */
SDL_Renderer *sdlpui_get_renderer(struct sdlpui_window *w);

/**
 * Retrieve a reference to the font to use for all dialogs or menus.
 *
 * \param w is the window containing what's to be rendered.
 * \return a pointer to the TTF font to use.
 */
TTF_Font *sdlpui_get_ttf(struct sdlpui_window *w);

/**
 * Retrieve a reference to a stipple pattern.
 *
 * \param w is the window containing what'll be stippled.
 * \return a pointer to the structure describing the stippling.
 *
 * One can use sdlpui_compute_stipple() from sdlpui-misc.h to construct
 * the stipple pattern.
 */
struct sdlpui_stipple *sdlpui_get_stipple(struct sdlpui_window *w);

/*
 * These are the roles where the primitive UI toolkit uses color.  You could
 * either assign unique values to each (so you can lookup the appropriate
 * color in sdlpui_get_color()) or optimize away that indirection and assign
 * them the appopriate index for a color table that'll be directly accessed by
 * sdlpui_get_color().
 */
#define SDLPUI_COLOR_MENU_BG (0)	/* background color for all menus */
#define SDLPUI_COLOR_MENU_FG (1)	/* foreground color for all menus */
#define SDLPUI_COLOR_MENU_BORDER (2)	/* border color for the menus that
						have a border */
#define SDLPUI_COLOR_DIALOG_BG (3)	/* background color for any dialog
						that is not a menu */
#define SDLPUI_COLOR_DIALOG_FG (4)	/* foreground color for any dialog
						that is not a menu */
#define SDLPUI_COLOR_DIALOG_BORDER (5)	/* border color for any dialog that
						is not a menu */
#define SDLPUI_COLOR_COUNTERSINK (6)	/* outer border color for push buttons
						in dialogs and inner boundary
						for dialogs that are not
						menus */

/**
 * Retrieve a reference to the color to use for a specific role when
 * rendering dialogs or menus.
 *
 * \param w is the window containing what's to be rendered.
 * \param role is one of the SDLPUI_COLOR_* constants referring to how the
 * color will be used.
 * \return a pointer to color to use.
 */
const SDL_Color *sdlpui_get_color(struct sdlpui_window *w, int role);

/**
 * Signal that the window needs to be redrawn to reflect the state of the
 * dialogs and menus.
 */
void sdlpui_signal_redraw(struct sdlpui_window *w);

/**
 * Push the given dialog (adding it if not already present) to the top of the
 * window's stack of dialogs.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog.
 *
 * Should also signal a redraw of the window is necessary if the dialog is
 * not already at the top of the dialog stack.
 */
void sdlpui_dialog_push_to_top(struct sdlpui_window *w,
		struct sdlpui_dialog *d);

/**
 * Remove the given dialog from the window's stack of dialogs.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog.
 *
 * Has the side effect of ceding mouse and keyboard focus if the dialog has
 * them and signalling that a redraw is necessary for the window.
 */
void sdlpui_dialog_pop(struct sdlpui_window *w, struct sdlpui_dialog *d);

/**
 * Tell the given window that the given dialog wants to take keyboard focus
 * and receive all keyboard, text input, or text editing events in the window
 * until it yields keyboard focus.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog that wants to gain focus.
 */
void sdlpui_dialog_gain_key_focus(struct sdlpui_window *w,
		struct sdlpui_dialog *d);

/**
 * Tell the given window that the given dialog wants to give up keyboard focus
 * and not be sent keyboard, text input, or text editing events until it
 * reacquires keyboard focus.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog that is yielding focus.
 */
void sdlpui_dialog_yield_key_focus(struct sdlpui_window *w,
		struct sdlpui_dialog *d);

/**
 * Tell the given window that the given dialog wants to take mouse focus and
 * receive all mouse button, mouse wheel, and mouse motion events in the window
 * until it yields mouse focus.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog that wants to gain focus.
 */
void sdlpui_dialog_gain_mouse_focus(struct sdlpui_window *w,
		struct sdlpui_dialog *d);

/**
 * Tell the given window that the given dialog wants to give up mouse focus
 * and not be sent mouse button or mouse wheel events until it reacquires
 * mouse focus.  Mouse motion events may be sent to a dialog without focus to
 * see if that event would cause it to reacquire focus.
 *
 * \param w is the window containing the dialog.
 * \param d is the dialog that is yielding focus.
 */
void sdlpui_dialog_yield_mouse_focus(struct sdlpui_window *w,
		struct sdlpui_dialog *d);

/**
 * Get the dialog with key focus, if any.
 */
struct sdlpui_dialog *sdlpui_dialog_with_key_focus(struct sdlpui_window *w);

/**
 * Get the dialog with mouse focus, if any.
 */
struct sdlpui_dialog *sdlpui_dialog_with_mouse_focus(struct sdlpui_window *w);

/**
 * Quit the application.  Expected to not return.
 */
void sdlpui_force_quit(void);

#endif /* INCLUDED_SDL2_SDLPUI_WINDOW_H */
