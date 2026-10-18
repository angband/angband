/**
 * \file sdl2/pui-foc.h
 * \brief Declare the interface for working with a control that has focus
 * and the visual feedback that control provides.
 *
 * The policy is:
 *     A control and dialog or a dialog can be marked for key focus.
 *     The control, if any, will have first priority for handling a
 *     SDL_KeyboardEvent, SDL_TextInputEvent, or SDL_TextEditEvent.  The dialog
 *     will have the next highest priority for handling such an event.
 *
 *     A control and dialog or a dialog can be marked for mouse focus.
 *     The control, if any, will have first priority for handling a
 *     SDL_MouseButtonEvent, SDL_MouseMotionEvent, or SDL_MouseWheelEvent.
 *     The dialog will have the next highest priority for handling such an
 *     event.
 *
 *     The key focus can be different than the mouse focus, but a change
 *     to which control has mouse focus (because of a SDL_MouseMotionEvent,
 *     for instance) will change which change the key focus to match the
 *     mouse focus.  The user will be able to navigate between controls
 *     with the keyboard, but something that changes the mouse focus will
 *     snap the key focus to the same control and dialog or same dialog.
 *
 *     An event which moves focus between controls or between members of a
 *     compound control does not require any other visual feedback besides
 *     the changes to the focus indicators.
 *
 *     If an event triggers a non-repeating action from a control with focus,
 *     the control momentarily changes the color (to either
 *     SDLPUI_COLOR_MENU_ACTION_FEEDBACK is in a menu or
 *     SDLPUI_COLOR_ACTION_FEEDBACK if it is not in a menu) for what it
 *     draws to indicate that it has focus.  If the visual appearance of a
 *     control already changes at the start of an action, then the control can
 *     omit momentarily changing the color of the focus indicator.  If focus
 *     moves away from the control while it is momentarily displaying the
 *     altered focus indicator, the indicator is cleared, as it would be if
 *     the control had not initiated an action before the change to focus.
 *     For example, a push button changes the color of its focus rectangle
 *     briefly when it receives a left mouse button press event and starts the
 *     action bound to the button.  A toggle button changes its appearance
 *     upon receipt of a similar event so the action, toggling the state
 *     associated with the button, does not trigger a change to the focus
 *     indicator.
 *
 *     If the control with focus repeats an action as long as a key or mouse
 *     button remains pressed, the control changes the color of its focus
 *     indicator (the altered color is the same as above) upon receipt of the
 *     press event and retains that color change until the receipt of the
 *     release event.  If the control loses focus before receipt of the
 *     release event, it clears the focus indicator and stops the repeated
 *     action.  If the press continues but the repeated action no longer has
 *     an effect, change the color displayed for the focus indicator to
 *     SDLPUI_COLOR_MENU_ACTION_UNAVAIL if the control is in a menu or
 *     SDLPUI_COLOR_ACTION_UNAVAIL if it is not in a menu.
 *
 *     If a non-disabled control with focus receives an event that would
 *     trigger an action but, in the current circumstances, that action is
 *     not possible, the control momentarily changes the color (to
 *     SDLPUI_COLOR_MENU_ACTION_UNAVAIL if the control is in a menu or
 *     SDLPUI_COLOR_ACTION_UNAVAIL if it is not in a menu) for what it draws
 *     to indicate that it has focus.  For an example, a scrolled list that
 *     normally responds to a press of the Home key by changing the display
 *     to show the start of the list would briefly change the color of its
 *     focus indicator to SDLPUI_COLOR_ACTION_UNAVAIL if the Home key is
 *     pressed when the scrolled list is already displaying the start of the
 *     list.
 *
 *     If an element within a control with focus is being dragged, change
 *     the color of the focus indicator to SDLPUI_COLOR_MENU_ACTION_DRAG if
 *     the control is in a menu or SDLPUI_COLOR_ACTION_DRAG if the control
 *     is not in a menu.
 *
 *     If momentary changes to the focus indicator overlap, the most recent
 *     supersedes any earlier ones.
 *
 *     If a control allows for more than one type of action, the changes to
 *     the focus indicator for a repeated action continue until all of the
 *     repeated actions complete.  If the feedback for multiple actions
 *     mandates different colors for the focus indicator, the control could
 *     draw alternating segments of the indicator in the different colors.
 *     If it chooses not to do that and only displays the focus indicator in
 *     one color, the color for any momentary feedback should supersede the
 *     color for feedback related to continued actions and feedback for an
 *     unavailable repeated action should supersede feedback about available
 *     repeated actions.
 *
 * Issues:
 *     The extra use of color decreases usability in environments where
 *     there is a limited gamut of colors, either by choice or by necessity,
 *     and gets away from the spirit of https://github.com/angband/angband/commit/d6886368017822967efa5911a64b8819502fd7e4 .
 *     Could use alternate line styles instead of color:  do that all the
 *     time or only in certain conditions like if a command line argument
 *     or an environment variable is set?
 *
 *     Currently does nothing to assist a control that wants to use the
 *     respond_default callback (and let the dialog handle the key and
 *     button events) and also wants to have the action fire repeatedly
 *     while a key or button remains pressed.
 *
 *     Currently does nothing to assist a control with implementing drags.
 *
 *     If compatibility with SDL versions prior to 2.0.18 is no longer
 *     necessary, can change internal details about tick handling,
 *     Uint32, SDL_GetTicks(), and SDL_TICKS_PASSED, to instead use
 *     Uint64, SDL_GetTicks64(), and direct comparison of tick values.
 */

#ifndef INCLUDED_SDL2_SDLPUI_FOCUS_H
#define INCLUDED_SDL2_SDLPUI_FOCUS_H

#include "pui-ctrl.h"

struct sdlpui_dialog;
struct sdlpui_window;

/**
 * Set out possible types of visual feedback that a control with focus
 * can provide.
 */
enum sdlpui_feedback {
	/**
	 * No recent attempt to start an action, no ongoing actions, and
	 * no ongoing drag
	 */
	SDLPUI_FEEDBACK_NONE = 0,
	/**
	 * Recent successful attempt to start an action or an ongoing repeating
	 * action whose last repetition was successful
	 */
	SDLPUI_FEEDBACK_ACTION,
	/**
	 * Recent attempt to start an action but that action was not available
	 * in those circumstances or an ongoing repeating action whose last
	 * repetition did not do anything because of the circumstances at the
	 * time
	 */
	SDLPUI_FEEDBACK_UNAVAIL,
	/** An element of the control with focus is being dragged */
	SDLPUI_FEEDBACK_DRAGGING,
};

/**
 * Hold the results gathered by sdlpui_query_feedback.
 */
struct sdlpui_control_feedback {
	/**
	 * Hold the one-based index of the component with mouse focus.
	 * If zero, no component has mouse focus.
	 */
	int mouse_focus;
	/**
	 * Hold the one-based index of the component with key focus.
	 * If zero, no component has key focus.
	 */
	int key_focus;
	/** Specify the type of momentary feedback for latest mouse input. */
	enum sdlpui_feedback mf_mouse;
	/** Specify the type of continued feedback for mouse input. */
	enum sdlpui_feedback cf_mouse;
	/** Specify the type of momentary feedback for latest key input. */
	enum sdlpui_feedback mf_key;
	/** Specify the type of continued feedback for key input. */
	enum sdlpui_feedback cf_key;
};

/**
 * Special value for sdlpui_set_momentary_feedback_duration():  restore the
 * value to what it was when the first successful call to sdlpui_init()
 * returned
 */
#define SDLPUI_MOMENTARY_DEFAULT (0)

/**
 * Special value for sdlpui_set_momentary_feedback_duration():  leave the
 * current value unchanged
 */
#define SDLPUI_MOMENTARY_UNCHANGED (SDL_MAX_UINT32)


void sdlpui_begin_focus_transaction(void);
void sdlpui_end_focus_transaction(void);

void sdlpui_window_loses_focus(enum sdlpui_action_hint ah);
SDL_bool sdlpui_change_focus(struct sdlpui_control *c, int comp_ind,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint ah, SDL_bool force, SDL_bool callbacks);
struct sdlpui_control *sdlpui_get_control_with_focus(
		enum sdlpui_action_hint ah);
struct sdlpui_dialog *sdlpui_get_dialog_with_focus(
		enum sdlpui_action_hint ah);
int sdlpui_control_has_focus(const struct sdlpui_control *c,
		enum sdlpui_action_hint ah);
SDL_bool sdlpui_dialog_has_focus(const struct sdlpui_dialog *d,
		enum sdlpui_action_hint ah);
SDL_bool sdlpui_any_dialog_with_focus(void);

SDL_bool sdlpui_query_feedback(const struct sdlpui_control *c,
		struct sdlpui_control_feedback *cf);
void sdlpui_trigger_momentary_feedback(enum sdlpui_action_hint ah,
		SDL_bool avail);
void sdlpui_start_continued_feedback(enum sdlpui_action_hint ah,
		SDL_bool avail);
void sdlpui_switch_continued_feedback(enum sdlpui_action_hint ah,
		SDL_bool now_avail);
void sdlpui_stop_continued_feedback(enum sdlpui_action_hint ah, SDL_bool avail);

SDL_bool sdlpui_is_focus_locked(enum sdlpui_action_hint ah);
void sdlpui_lock_focus(enum sdlpui_action_hint ah);
void sdlpui_unlock_focus(enum sdlpui_action_hint ah);

Uint32 sdlpui_set_momentary_feedback_duration(Uint32 newv);

#endif /* INCLUDED_SDL2_SDLPUI_FOCUS_H */
