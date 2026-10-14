/**
 * \file sdl2/pui-foc.c
 * \brief Define the interface for working with a control that has focus
 * and the visual feedback that control provides.
 *
 * Copyright (c) 2026 Eric Branlund
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

#include "pui-foc.h"
#include "pui-focp.h"
#include "pui-dlg.h"
#include "pui-misc.h"
#include "pui-win.h"
#include <errno.h>	/* errno, ERANGE */
#include <limits.h>	/* UINT_MAX */


struct sdlpui_focus_state {
	/** Point to the control with focus; may be NULL */
	struct sdlpui_control *c;
	/**
	 * Point to the dialog with focus; may be NULL but must point to the
	 * dialog containing c if c is not NULL
	 */
	struct sdlpui_dialog *d;
	/**
	 * Point to the window containing c and d; may be NULL if c and d
	 * are NULL
	 */
	struct sdlpui_window *w;
	/**
	 * Installed timer to handle changes when momentary feedback lapses
	 * or zero if there is no timer active
	 */
	SDL_TimerID momentary_timer;
	/**
	 * If not equal to SDLPUI_FEEDBACK_NONE, momentary_lapses is valid:
	 * in that case, the focus indicator should be altered if
	 * !SDL_TICKS_PASSED(SDL_GetTicks(), momentary_lapses); otherwise,
	 * the focus indicator does not have to be changed to provide feedback
	 * about non-repeated actions
	 */
	enum sdlpui_feedback momentary;
	/**
	 * Value of the ticks counter when the current momentary feedback
	 * lapses; only useful if momentary is not SDLPUI_FEEDBACK_NONE
	 */
	Uint32 momentary_lapses;
	/**
	 * Number of continued actions in progress whose last repetition was
	 * successful
	 */
	unsigned int continued;
	/**
	 * Number of continued actions in progress whose last repetition was
	 * not successful
	 */
	unsigned int continued_unavail;
	/** Zero-based index of component in control that has focus */
	int comp_ind;
	/**
	 * Whether or not events, like mouse motion or a Tab key press, can
	 * move focus from the control that has it
	 */
	SDL_bool focus_locked;
};


Uint32 sdlpui_redraw_event_type = (Uint32)-1;

static struct sdlpui_focus_state key_focus = {
	NULL, NULL, NULL, 0, SDLPUI_FEEDBACK_NONE, 0, 0, 0, 0, SDL_FALSE
};

static struct sdlpui_focus_state mouse_focus = {
	NULL, NULL, NULL, 0, SDLPUI_FEEDBACK_NONE, 0, 0, 0, 0, SDL_FALSE
};

static SDL_mutex *lock = NULL;
static Uint32 momentary_default_duration = SDLPUI_MOMENTARY_DEFAULT;
static Uint32 momentary_duration = SDLPUI_MOMENTARY_DEFAULT;


/**
 * Respond to the lapsing of momentary feedback.
 */
static Uint32 handle_momentary_feedback_timer(Uint32 interval, void *param)
{
	struct sdlpui_focus_state *state = param;

	sdlpui_begin_focus_transaction();
	SDLPUI_EVENT_TRACER((state->c)
		? (*state->c->ftb->get_type_name)(state->c) : "none",
		state->c, (state->c && state->c->ftb->get_caption)
		? (*state->c->ftb->get_caption)(state->c) : "(none)",
		"momentary feedback timer lapses");
	state->momentary_timer = 0;
	state->momentary = SDLPUI_FEEDBACK_NONE;
	state->momentary_lapses = 0;
	if (state->d && state->w) {
		SDL_Event dummy;

		sdlpui_dialog_mark_for_redraw(state->d, state->w);
		/* Send a user event so the main thread will wake and redraw. */
		SDL_zero(dummy);
		dummy.type = sdlpui_redraw_event_type;
		dummy.user.data1 = state->d;
		dummy.user.data2 = state->w;
		SDL_PushEvent(&dummy);
	}
	sdlpui_end_focus_transaction();

	/* This is a one-shot action and does not repeat. */
	return 0;
}


/**
 * Acquire access to the focus and feedback state and prevent other
 * threads from accessing that state.
 */
void sdlpui_begin_focus_transaction(void)
{
	if (!lock || SDL_LockMutex(lock)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"sdlpui_begin_focus_transaction() could not lock the "
			"mutex");
		sdlpui_force_quit();
	}
}


/**
 * Release access to the focus and feedback state and allow other threads
 * to access that state.
 */
void sdlpui_end_focus_transaction(void)
{
	if (!lock || SDL_UnlockMutex(lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"sdlpui_end_focus_transaction() could not unlock the "
			"mutex");
		sdlpui_force_quit();
	}
}

/**
 * Handle a window losing focus.
 *
 * \param ah is SDLPUI_ACTION_HINT_KEY if the window lost key focus or
 * SDLPUI_ACTION_HINT_MOUSE if the window lost mouse focus.
 */
void sdlpui_window_loses_focus(enum sdlpui_action_hint ah)
{
	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		if (key_focus.d) {
			(*key_focus.d->ftb->handle_window_loses_key)(
				key_focus.d, key_focus.w);
		}
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		if (mouse_focus.d) {
			(*mouse_focus.d->ftb->handle_window_loses_mouse)(
				mouse_focus.d, mouse_focus.w);
		}
		break;

	default:
		/* Do nothing. */
		break;
	}
}

/**
 * Update which control or dialog has focus.
 *
 * \param c points to the control that now has the focus indicated by ah.  May
 * be NULL if no control has that type of focus.
 * \param comp_ind is the zero-based index of the element in c that has the
 * focus indicated by ah.  If c is a simple control (it does not have multiple
 * components that can accept focus), use zero for comp_ind.  The value of
 * comp_ind is not used when c is NULL.
 * \param d points to the dialog that now has the focus indicated by ah.  That
 * must be the dialog that contains c if c is not NULL.  May be NULL if c is
 * NULL and no dialog has that type of focus.
 * \param w points to the window that contains c and d.  If c and d are NULL,
 * it must point to the window that had focus.
 * \param ah is SDLPUI_ACTION_HINT_KEY if the caller is interested in changing
 * key focus or SDLPUI_ACTION_HINT_MOUSE if the caller is interested in changing
 * mouse focus.
 * \param force will, if not SDL_FALSE, clear a focus lock that is present and
 * would prevent the focus change from taking effect.  If force is false and a
 * focus lock is present that would prevent the change, the focus change will
 * not occur.
 * \param callbacks will, if not SDL_FALSE, cause the lose_focus and gain_focus
 * callbacks to be invoked on the controls involved.  Otherwise, those callbacks
 * are skipped:  that would normally only be done if c is NULL and d are NULL
 * and the control or dialog with focus is being dismissed.  When the callbacks
 * are invoked, calls querying the focus state will return the new state.
 * \return SDL_TRUE if the focus was changed.  Otherwise, return SDL_FALSE.
 *
 * By policy, any change to mouse focus also changes key focus:
 * "focus follows the mouse".  Also by policy, a change to focus clears any
 * visual feedback about an action that was triggered or repeating.
 */
SDL_bool sdlpui_change_focus(struct sdlpui_control *c, int comp_ind,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint ah, SDL_bool force, SDL_bool callbacks)
{
	struct sdlpui_control *kc = NULL, *mc = NULL;
	struct sdlpui_dialog *kd = NULL, *md = NULL;
	struct sdlpui_window *kw = NULL, *mw = NULL;
	SDL_bool key_changed = SDL_FALSE, key_only_ind = SDL_FALSE;
	SDL_bool mouse_changed = SDL_FALSE, mouse_only_ind = SDL_FALSE;
	SDL_bool result = SDL_FALSE;

	SDL_assert(w);

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		if (c && key_focus.c && key_focus.c->id == c->id) {
			if (comp_ind != key_focus.comp_ind) {
				key_changed = SDL_TRUE;
				key_only_ind = SDL_TRUE;
			}
		} else if (c || key_focus.c || ((key_focus.d && d
				&& key_focus.d->id != d->id)
				|| (!key_focus.d && d)
				|| (key_focus.d && !d))) {
			key_changed = SDL_TRUE;
		}
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		if (c && mouse_focus.c && mouse_focus.c->id == c->id) {
			if (comp_ind != mouse_focus.comp_ind) {
				mouse_changed = SDL_TRUE;
				mouse_only_ind = SDL_TRUE;
			}
			/* Focus follows the mouse. */
			if (!key_focus.c || key_focus.c->id != c->id) {
				key_changed = SDL_TRUE;
			} else if (comp_ind != key_focus.comp_ind) {
				key_changed = SDL_TRUE;
				key_only_ind = SDL_TRUE;
			}
		} else if (c || mouse_focus.c || ((mouse_focus.d && d &&
				mouse_focus.d->id != d->id)
				|| (!mouse_focus.d && d)
				|| (mouse_focus.d && !d))) {
			mouse_changed = SDL_TRUE;
			/* Focus follows the mouse. */
			if (c && key_focus.c && key_focus.c->id == c->id) {
				if (comp_ind != key_focus.comp_ind) {
					key_changed = SDL_TRUE;
					key_only_ind = SDL_TRUE;
				}
			} else if (c || key_focus.c || ((key_focus.d && d
					&& key_focus.d->id != d->id)
					|| (!key_focus.d && d)
					|| (key_focus.d && !d))) {
				key_changed = SDL_TRUE;
			}
		}
		break;

	default:
		/* Do nothing. */
		break;
	}

	if (key_changed && key_focus.focus_locked) {
		if (force) {
#ifdef SDLPUI_TRACE_EVENTS
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
				"key focus lock forcibly unlocked; key focus "
				"is currently with control, %s (%s), and is "
				"moving to the control, %s (%s)\n",
				(key_focus.c) ?
				(*key_focus.c->ftb->get_type_name)(key_focus.c)
				: "none",
				(key_focus.c && key_focus.c->ftb->get_caption)
				? (*key_focus.c->ftb->get_caption)(key_focus.c)
				: "none", (c) ? (*c->ftb->get_type_name)(c)
				: "none", (c && c->ftb->get_caption)
				? (*c->ftb->get_caption)(c) : "none");
#endif
			key_focus.focus_locked = SDL_FALSE;
		} else {
#ifdef SDLPUI_TRACE_EVENTS
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
				"key focus lock prevents focus change; key "
				"focus is currently with control, %s (%s), and "
				"would have moved to the control, %s (%s)\n",
				(key_focus.c) ?
				(*key_focus.c->ftb->get_type_name)(key_focus.c)
				: "none",
				(key_focus.c && key_focus.c->ftb->get_caption)
				? (*key_focus.c->ftb->get_caption)(key_focus.c)
				: "none", (c) ? (*c->ftb->get_type_name)(c)
				: "none", (c && c->ftb->get_caption)
				? (*c->ftb->get_caption)(c) : "none");
#endif
			key_changed = SDL_FALSE;
		}
	}
	if (mouse_changed && mouse_focus.focus_locked) {
		if (force) {
#ifdef SDLPUI_TRACE_EVENTS
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
				"mouse focus lock forcibly unlocked; mouse "
				"focus is currently with control, %s (%s), "
				"and is moving to the control, %s (%s)\n",
				(mouse_focus.c) ?
				(*mouse_focus.c->ftb->get_type_name)(mouse_focus.c)
				: "none", (mouse_focus.c
				&& mouse_focus.c->ftb->get_caption)
				? (*mouse_focus.c->ftb->get_caption)(mouse_focus.c)
				: "none", (c) ? (*c->ftb->get_type_name)(c)
				: "none", (c && c->ftb->get_caption)
				? (*c->ftb->get_caption)(c) : "none");
#endif
			mouse_focus.focus_locked = SDL_FALSE;
		} else {
#ifdef SDLPUI_TRACE_EVENTS
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
				"mouse focus lock prevents focus change; "
				"mouse focus is currently with control, %s "
				"(%s), and would have moved to the control, "
				"%s (%s)\n", (mouse_focus.c) ?
				(*mouse_focus.c->ftb->get_type_name)(mouse_focus.c)
				: "none", (mouse_focus.c
				&& mouse_focus.c->ftb->get_caption)
				? (*mouse_focus.c->ftb->get_caption)(mouse_focus.c)
				: "none", (c) ? (*c->ftb->get_type_name)(c)
				: "none", (c && c->ftb->get_caption)
				? (*c->ftb->get_caption)(c) : "none");
#endif
			mouse_changed = SDL_FALSE;
		}
	}
	if (key_changed) {
		result = SDL_TRUE;
		kc = key_focus.c;
		kd = key_focus.d;
		kw = key_focus.w;
		key_focus.c = c;
		key_focus.d = d;
		key_focus.w = (c || d) ? w : NULL;
		if (key_focus.momentary_timer) {
			SDLPUI_EVENT_TRACER((key_focus.c)
				? (*key_focus.c->ftb->get_type_name)(
				key_focus.c) : "none", key_focus.c, (key_focus.c
				&& key_focus.c->ftb->get_caption)
				? (*key_focus.c->ftb->get_caption)(key_focus.c)
				: "(none)", "prior momentary feedback "
				"timer canceled");
			(void)SDL_RemoveTimer(key_focus.momentary_timer);
			key_focus.momentary_timer = 0;
		}
		key_focus.continued = 0;
		key_focus.continued_unavail = 0;
		key_focus.momentary = SDLPUI_FEEDBACK_NONE;
		key_focus.comp_ind = (c) ? comp_ind : 0;
	}
	if (mouse_changed) {
		result = SDL_TRUE;
		mc = mouse_focus.c;
		md = mouse_focus.d;
		mw = mouse_focus.w;
		mouse_focus.c = c;
		mouse_focus.d = d;
		mouse_focus.w = (c || d) ? w : NULL;
		if (mouse_focus.momentary_timer) {
			SDLPUI_EVENT_TRACER((mouse_focus.c)
				? (*mouse_focus.c->ftb->get_type_name)(
				mouse_focus.c) : "none", mouse_focus.c,
				(mouse_focus.c
				&& mouse_focus.c->ftb->get_caption)
				? (*mouse_focus.c->ftb->get_caption)(
				mouse_focus.c) : "(none)", "prior momentary "
				"feedback timer canceled");
			(void)SDL_RemoveTimer(mouse_focus.momentary_timer);
			mouse_focus.momentary_timer = 0;
		}
		mouse_focus.continued = 0;
		mouse_focus.continued_unavail = 0;
		mouse_focus.momentary = SDLPUI_FEEDBACK_NONE;
		mouse_focus.comp_ind = (c) ? comp_ind : 0;
	}
	if (key_changed && !key_only_ind && kc) {
		SDL_assert(kd && kw);
		SDLPUI_EVENT_TRACER((*kc->ftb->get_type_name)(kc), kc,
			(kc->ftb->get_caption) ? (*kc->ftb->get_caption)(kc)
			: "(none)", "loses key focus");
		sdlpui_dialog_mark_for_redraw(kd, kw);
		if (callbacks && kc->ftb->lose_focus) {
			(*kc->ftb->lose_focus)(kc, kd, kw,
				SDLPUI_ACTION_HINT_KEY, c, d);
		}
	}
	if (mouse_changed && !mouse_only_ind && mc) {
		SDL_assert(md && mw);
		SDLPUI_EVENT_TRACER((*mc->ftb->get_type_name)(mc), mc,
			(mc->ftb->get_caption) ? (*mc->ftb->get_caption)(mc)
			: "(none)", "loses mouse focus");
		sdlpui_dialog_mark_for_redraw(md, mw);
		if (callbacks && mc->ftb->lose_focus) {
			(*mc->ftb->lose_focus)(mc, md, mw,
				SDLPUI_ACTION_HINT_MOUSE, c, d);
		}
	}
	if (key_changed && c) {
		SDL_assert(d);
		SDLPUI_EVENT_TRACER((*c->ftb->get_type_name)(c),
			c, (c->ftb->get_caption)
			? (*c->ftb->get_caption)(c) : "(none)",
			(key_only_ind) ? "changes component with key focus"
			: "gains key focus");
		sdlpui_dialog_mark_for_redraw(d, w);
		if (callbacks && c->ftb->gain_focus) {
			(*c->ftb->gain_focus)(c, d, w,
				SDLPUI_ACTION_HINT_KEY, comp_ind);
		}
	}
	if (mouse_changed && c) {
		SDL_assert(d);
		SDLPUI_EVENT_TRACER((*c->ftb->get_type_name)(c),
			c, (c->ftb->get_caption)
			? (*c->ftb->get_caption)(c) : "(none)",
			(mouse_only_ind) ? "changes component with mouse focus"
			: "gains mouse focus");
		sdlpui_dialog_mark_for_redraw(d, w);
		if (callbacks && c->ftb->gain_focus) {
			(*c->ftb->gain_focus)(c, d, w,
				SDLPUI_ACTION_HINT_MOUSE, comp_ind);
		}
	}
	return result;
}


/**
 * Return a pointer to control that has the specified focus.
 *
 * \param ah is SDLPUI_ACTION_HINT_KEY if the caller is interested in the
 * control that has key focus or SDLPUI_ACTION_HINT_MOUSE if the caller is
 * interested in the control that has mouse focus.
 * \return a pointer to the control with the type of focus specified by ah
 * or NULL if no control has that focus.
 */
struct sdlpui_control *sdlpui_get_control_with_focus(
		enum sdlpui_action_hint ah)
{
	struct sdlpui_control *result;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		result = key_focus.c;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		result = mouse_focus.c;
		break;

	default:
		result = NULL;
		break;
	}

	return result;
}


/**
 * Return a pointer to dialog that has the specified focus.
 *
 * \param ah is SDLPUI_ACTION_HINT_KEY if the caller is interested in the
 * dialog that has key focus or SDLPUI_ACTION_HINT_MOUSE if the caller is
 * interested in the dialog that has mouse focus.
 * \return a pointer to the dialog with the type of focus specified by ah
 * or NULL if no dialog has that focus.
 */
struct sdlpui_dialog *sdlpui_get_dialog_with_focus(
		enum sdlpui_action_hint ah)
{
	struct sdlpui_dialog *result;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		result = key_focus.d;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		result = mouse_focus.d;
		break;

	default:
		result = NULL;
		break;
	}

	return result;
}


/**
 * Return whether the given control has focus.
 *
 * \param c is the control of interest.
 * \param ah is SDLPUI_ACTION_HINT_KEY if the caller is interested in whether
 * c has key focus or SDLPUI_ACTION_HINT_MOUSE if the caller is interested
 * in whether c has mouse focus.
 * \return the one-base index of the control's component has the type of focus
 * indicated by ah.  Otherwise, return zero.
 */
int sdlpui_control_has_focus(const struct sdlpui_control *c,
		enum sdlpui_action_hint ah)
{
	const struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return 0;
	}

	return (c && state->c && c->id == state->c->id)
		? state->comp_ind + 1 : 0;
}


/**
 * Return whether the given dialog has focus.
 *
 * \param d is the dialog of interest.
 * \param ah is SDLPUI_ACTION_HINT_KEY if the caller is interested in whether
 * d has key focus or SDLPUI_ACTION_HINT_MOUSE if the caller is interested
 * in whether d has mouse focus.
 * \return SDL_TRUE if the dialog has the type of focus indicated by ah.
 * Otherwise, return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_has_focus(const struct sdlpui_dialog *d,
		enum sdlpui_action_hint ah)
{
	const struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return SDL_FALSE;
	}

	return (d && state->d && d->id == state->d->id) ? SDL_TRUE : SDL_FALSE;
}


/**
 * Return whether any dialog has focus.
 */
SDL_bool sdlpui_any_dialog_with_focus(void)
{
	return (key_focus.d || mouse_focus.d) ? SDL_TRUE : SDL_FALSE;
}


/**
 * Gather what kinds of visual feedback the given control should provide.
 *
 * \param c points to the control of interest.
 * \param cf points to where the results should be stored.
 * \return SDL_TRUE if the control has key or mouse focus.  Otherwise, return
 * SDL_FALSE.
 */
SDL_bool sdlpui_query_feedback(const struct sdlpui_control *c,
		struct sdlpui_control_feedback *cf)
{
	SDL_bool result = SDL_FALSE;

	cf->key_focus = 0;
	cf->mouse_focus = 0;
	cf->mf_mouse = SDLPUI_FEEDBACK_NONE;
	cf->cf_mouse = SDLPUI_FEEDBACK_NONE;
	cf->mf_key = SDLPUI_FEEDBACK_NONE;
	cf->cf_key = SDLPUI_FEEDBACK_NONE;

	if (mouse_focus.c && mouse_focus.c->id == c->id) {
		result = SDL_TRUE;
		cf->mouse_focus = 1 + mouse_focus.comp_ind;
		if (mouse_focus.momentary != SDLPUI_FEEDBACK_NONE
				&& !SDL_TICKS_PASSED(SDL_GetTicks(),
				mouse_focus.momentary_lapses)) {
			cf->mf_mouse = mouse_focus.momentary;
		}
		if (mouse_focus.continued_unavail) {
			cf->cf_mouse = SDLPUI_FEEDBACK_UNAVAIL;
		} else if (mouse_focus.continued) {
			cf->cf_mouse = SDLPUI_FEEDBACK_ACTION;
		}
	}
	if (key_focus.c && key_focus.c->id == c->id) {
		result = SDL_TRUE;
		cf->key_focus = 1 + key_focus.comp_ind;
		if (key_focus.momentary != SDLPUI_FEEDBACK_NONE
				&& !SDL_TICKS_PASSED(SDL_GetTicks(),
				key_focus.momentary_lapses)) {
			cf->mf_key = key_focus.momentary;
		}
		if (key_focus.continued_unavail) {
			cf->cf_key = SDLPUI_FEEDBACK_UNAVAIL;
		} else if (key_focus.continued) {
			cf->cf_key = SDLPUI_FEEDBACK_ACTION;
		}
	}
	return result;
}


/**
 * Signal that the control with focus is starting an action that does not
 * repeat.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for the control with key focus or
 * SDLPUI_ACTION_HINT_MOUSE for the control with mouse focus.
 * \param avail, if true, specifies that the action is possible.  If false,
 * the action does nothing.
 */
void sdlpui_trigger_momentary_feedback(enum sdlpui_action_hint ah,
		SDL_bool avail)
{
	struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return;
	}

	if (state->c) {
		if (state->momentary_timer) {
			SDLPUI_EVENT_TRACER((*state->c->ftb->get_type_name)(
				state->c), state->c,
				(state->c->ftb->get_caption)
				? (state->c->ftb->get_caption)(state->c)
				: "(none)", "prior momentary feedback timer "
				"canceled");
			(void)SDL_RemoveTimer(state->momentary_timer);
		}
		SDLPUI_EVENT_TRACER((*state->c->ftb->get_type_name)(
			state->c), state->c, (state->c->ftb->get_caption)
			? (state->c->ftb->get_caption)(state->c) : "(none)",
			"adding momentary feedback");
		state->momentary_timer = SDL_AddTimer(momentary_duration,
			handle_momentary_feedback_timer, state);
		if (state->momentary_timer) {
			SDL_assert(momentary_duration
				!= SDLPUI_MOMENTARY_DEFAULT);
			state->momentary_lapses = SDL_GetTicks()
				+ momentary_duration;
			state->momentary = (avail) ? SDLPUI_FEEDBACK_ACTION
				: SDLPUI_FEEDBACK_UNAVAIL;
		} else {
			state->momentary = SDLPUI_FEEDBACK_NONE;
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"SDL_AddTimer() failed while adding visual "
				"feedback for control with focus: %s",
				SDL_GetError());
		}
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"sdl_trigger_momentary_feedback() called without "
			"a control having focus");
	}
}


/**
 * Signal that the control with focus is starting an action that automatically
 * repeats.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for the control with key focus or
 * SDLPUI_ACTION_HINT_MOUSE for the control with mouse focus.
 * \param avail, if true, specifies that the initial repetition of the action
 * is possible.  If false, the initial repetition of the action does nothing.
 */
void sdlpui_start_continued_feedback(enum sdlpui_action_hint ah, SDL_bool avail)
{
	struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return;
	}

	if (state->c) {
		if (avail) {
			if (state->continued < UINT_MAX) {
				++state->continued;
			} else {
				SDL_assert(0);
			}
		} else {
			if (state->continued_unavail < UINT_MAX) {
				++state->continued_unavail;
			} else {
				SDL_assert(0);
			}
		}
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"sdl_start_continued_feedback() called without "
			"a control having focus");
	}
}


/**
 * Signal that a repeating action tied to the control with focus is changing
 * its effect.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for the control with key focus or
 * SDLPUI_ACTION_HINT_MOUSE for the control with mouse focus.
 * \param now_avail, if true, specifies that the last repetition of the action
 * did nothing, but the next repetition will do something.  If false, the last
 * repetition of the action did someing, but the next repetition will do
 * nothing.
 */
void sdlpui_switch_continued_feedback(enum sdlpui_action_hint ah,
		SDL_bool now_avail)
{
	struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return;
	}

	if (state->c) {
		if (now_avail) {
			if (state->continued_unavail && state->continued
					< UINT_MAX) {
				--state->continued_unavail;
				++state->continued;
			} else {
				SDL_assert(0);
			}
		} else {
			if (state->continued && state->continued_unavail
					< UINT_MAX) {
				--state->continued;
				++state->continued_unavail;
			} else {
				SDL_assert(0);
			}
		}
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"sdl_switch_continued_feedback() called without "
			"a control having focus");
	}
}


/**
 * Signal that the control with focus is stopping an action that automatically
 * repeats.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for the control with key focus or
 * SDLPUI_ACTION_HINT_MOUSE for the control with mouse focus.
 * \param avail, if true, specifies that the last repetition of the action
 * was possible.  If false, the last repetition of the action did nothing.
 */
void sdlpui_stop_continued_feedback(enum sdlpui_action_hint ah, SDL_bool avail)
{
	struct sdlpui_focus_state *state;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		state = &key_focus;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		state = &mouse_focus;
		break;

	default:
		return;
	}

	if (state->c) {
		if (avail) {
			if (state->continued) {
				--state->continued;
			} else {
				SDL_assert(0);
			}
		} else {
			if (state->continued_unavail) {
				--state->continued_unavail;
			} else {
				SDL_assert(0);
			}
		}
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"sdl_stop_continued_feedback() called without "
			"a control having focus");
	}
}


/**
 * Return whether normal focus changes are temporarily disabled.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for key focus, SDLPUI_ACTION_HINT_MOUSE for mouse
 * focus, SDLPUI_ACTION_HINT_KEY_OR_MOUSE for whether focus changes are
 * temporarily disabled for key or mouse, or SDLPUI_ACTION_HINT_KEY_AND_MOUSE
 * for whether focus changes are temporarily disabled for key and mouse.
 * \return SDL_TRUE if normal focus changes are disabled for the focus specified
 * by ah.  Otherwise, return SDL_FALSE.
 */
SDL_bool sdlpui_is_focus_locked(enum sdlpui_action_hint ah)
{
	SDL_bool result;

	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		result = key_focus.focus_locked;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		result = mouse_focus.focus_locked;
		break;

	case SDLPUI_ACTION_HINT_KEY_OR_MOUSE:
		result = key_focus.focus_locked || mouse_focus.focus_locked;
		break;

	case SDLPUI_ACTION_HINT_KEY_AND_MOUSE:
		result = key_focus.focus_locked && mouse_focus.focus_locked;
		break;

	default:
		result = SDL_FALSE;
		break;
	}

	return result;
}


/**
 * Temporarily disable normal focus changes.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for key focus, SDLPUI_ACTION_HINT_MOUSE for mouse
 * focus, or SDLPUI_ACTION_HINT_KEY_AND_MOUSE for both key and mouse focus.
 *
 * Focus changes should not be disabled in most circumstances.  One exception
 * is for a control that allows dragging with the mouse.  In that case,
 * definitely do not want the focus to change for small excursions of the
 * mouse from the bounds of the control (and if allowing small excursions, it
 * is easiest to allow any excursions up to the point that the mouse pointer
 * leaves the window).  For consistency, also lock key focus during the drag.
 * So, at the start of the drag, the control would call
 * sdlpui_lock_focus(SDLPUI_ACTION_HINT_KEY_AND_MOUSE) and then would call
 * sdlpui_unlock_focus(SDLPUI_ACTION_HINT_KEY_AND_MOUSE) at the end of the drag.
 *
 * Some events, like the mouse pointer leaving the window, will always change
 * focus regardless of whether focus has been locked.
 */
void sdlpui_lock_focus(enum sdlpui_action_hint ah)
{
	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		key_focus.focus_locked = SDL_TRUE;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		mouse_focus.focus_locked = SDL_TRUE;
		break;

	case SDLPUI_ACTION_HINT_KEY_AND_MOUSE:
		key_focus.focus_locked = SDL_TRUE;
		mouse_focus.focus_locked = SDL_TRUE;
		break;

	default:
		/* Do nothing. */
		break;
	}
}


/**
 * Reenable normal focus changes.
 *
 * \param ah specifies what type of focus is of interest:
 * SDLPUI_ACTION_HINT_KEY for key focus or SDLPUI_ACTION_HINT_MOUSE for mouse
 * focus, or SDLPUI_ACTION_HINT_KEY_AND_MOUSE for both key and mouse focus.
 */
void sdlpui_unlock_focus(enum sdlpui_action_hint ah)
{
	switch (ah) {
	case SDLPUI_ACTION_HINT_KEY:
		key_focus.focus_locked = SDL_FALSE;
		break;

	case SDLPUI_ACTION_HINT_MOUSE:
		mouse_focus.focus_locked = SDL_FALSE;
		break;

	case SDLPUI_ACTION_HINT_KEY_AND_MOUSE:
		key_focus.focus_locked = SDL_FALSE;
		mouse_focus.focus_locked = SDL_FALSE;
		break;

	default:
		/* Do nothing. */
		break;
	}
}


/**
 * Set the duration for momentary activation feedback and return the previous
 * value.
 *
 * \param newv is the new duration, in milliseconds.  Two values have
 * special meanings.  SDLPUI_MOMENTARY_DEFAULT sets the value to what it was
 * when the first successful call to sdlpui_init() returned.
 * SDLPUI_MOMENTARY_UNCHANGED leaves the value at its prior value.
 * \return the previous value for momentary activation feedback duration, in
 * milliseconds.
 *
 * The default value is derived from the environment variable,
 * SDLPUI_MOMENTARY_DEFAULT, if that variable is set and is a positive decimal
 * integer value that can fit in a Uint32.
 */
Uint32 sdlpui_set_momentary_feedback_duration(Uint32 newv)
{
	Uint32 prev;

	if (momentary_duration == SDLPUI_MOMENTARY_DEFAULT) {
		if (momentary_default_duration == SDLPUI_MOMENTARY_DEFAULT) {
			char *ev = SDL_getenv("SDLPUI_MOMENTARY_DEFAULT");

			if (ev) {
				char *ep;
				unsigned long uv;
				SDL_bool valid = SDL_TRUE;

				errno = 0;
				uv = SDL_strtoul(ev, &ep, 10);
				valid = *ev && !*ep && uv > 0;

#if 4294967295 < ULONG_MAX
				if (uv >= SDL_MAX_UINT32) {
					valid = SDL_FALSE;
				}
#else
				if (uv == ULONG_MAX
						&& errno == ERANGE) {
					valid = SDL_FALSE;
				}
#endif
				momentary_default_duration =
					(valid) ? (Uint32)uv : 100;
			} else {
				momentary_default_duration = 100;
			}
		}
		momentary_duration = momentary_default_duration;
	}

	prev = momentary_duration;
	if (newv != SDLPUI_MOMENTARY_UNCHANGED) {
		momentary_duration = (newv == SDLPUI_MOMENTARY_DEFAULT)
			? momentary_default_duration : newv;
	}

	return prev;
}


/**
 * This is a private interface for sdlpui_init() or test cases to initialize
 * static variables in sdlpui-foc.c.
 *
 * \return zero if successful.  Otherwise, return a non-zero value.
 *
 * Can safely be called multiple times without an intervening sdlpui_foc_quit().
 * For multithreaded applications, race conditions are possible if
 * sdlpui_foc_init() can be called while a call to sdlpui_foc_init() or
 * sdlpui_foc_quit() is in progress.  Those applications should be structured
 * to avoid that possibility.
 */
int sdlpui_foc_init(void)
{
	if (!lock) {
		sdlpui_redraw_event_type = SDL_RegisterEvents(1);
		if (sdlpui_redraw_event_type == (Uint32)-1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"sdlpui_foc_init() could not register one "
				"type of events with SDL_RegisterEvent()");
			return 1;
		}
		lock = SDL_CreateMutex();
		if (!lock) {
			sdlpui_redraw_event_type = (Uint32)-1;
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"sdlpui_foc_init() could not create a "
				"mutex: %s", SDL_GetError());
			return 1;
		}
		sdlpui_begin_focus_transaction();
		(void)sdlpui_set_momentary_feedback_duration(
			SDLPUI_MOMENTARY_DEFAULT);
		sdlpui_end_focus_transaction();
	}

	return 0;
}


/**
 * This is a private interface for sdlpui_quit() or test cases to release
 * resources allocated in sdlpui_foc_init().
 *
 * Can safely be called multiple times without an intervening call to
 * sdlpui_foc_init().  Once called, the only routines that from sdlpui-foc.h
 * that are useful are sdlpui_foc_init() and sdlpui_foc_quit().  For
 * multithreaded applications, race conditions are possible if
 * sdlpui_foc_quit() can be called while another call to a routine in
 * sdlpui-foc.c is in progress.  Those applications should be structured to
 * avoid that possibility.
 */
void sdlpui_foc_quit(void)
{
	if (lock) {
		SDL_mutex *l = lock;

		if (SDL_LockMutex(l)) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"sdlpui_foc_quit() could not lock the mutex");
			sdlpui_force_quit();
		}
		SDL_zero(key_focus);
		key_focus.c = NULL;
		key_focus.d = NULL;
		key_focus.w = NULL;
		SDL_zero(mouse_focus);
		mouse_focus.c = NULL;
		mouse_focus.d = NULL;
		mouse_focus.w = NULL;
		sdlpui_redraw_event_type = (Uint32)-1;
		lock = NULL;
		if (SDL_UnlockMutex(l)) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"sdlpui_foc_quit() could not unlock the mutex");
			sdlpui_force_quit();
		}
		SDL_DestroyMutex(l);
	}
}
