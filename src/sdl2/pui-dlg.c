/**
 * \file sdl2/pui-dlg.c
 * \brief Define the interface for menus and dialogs created by the primitive
 * UI toolkit for SDL2.
 *
 * Copyright (c) 2023 Eric Branlund
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

#include "pui-dlg.h"
#include "pui-misc.h"
#include "pui-win.h"
#include <limits.h> /* INT_MAX */

static bool handle_simple_menu_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e);
static bool handle_simple_menu_textin(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextInputEvent *e);
static void render_simple_menu(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static void goto_simple_menu_first_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static void step_simple_menu_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *c,
		bool forward);
static struct sdlpui_control *find_simple_menu_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind);
static struct sdlpui_dialog *get_simple_menu_parent(struct sdlpui_dialog *d);
static struct sdlpui_dialog *get_simple_menu_child(struct sdlpui_dialog *d);
static struct sdlpui_control *get_simple_menu_parent_ctrl(
		struct sdlpui_dialog *d);
static void set_simple_menu_child(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child);
static void resize_simple_menu(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height);
static void query_simple_menu_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static void query_simple_menu_minimum_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static void cleanup_simple_menu(struct sdlpui_dialog *d);

static void render_simple_info(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static void goto_simple_info_first_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static struct sdlpui_control *find_simple_info_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind);
static void resize_simple_info(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height);
static void query_simple_info_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static void cleanup_simple_info(struct sdlpui_dialog *d);

Uint32 SDLPUI_DIALOG_SIMPLE_MENU = 0;
Uint32 SDLPUI_DIALOG_SIMPLE_INFO = 0;

static const struct sdlpui_dialog_funcs simple_menu_funcs = {
	handle_simple_menu_key,
	handle_simple_menu_textin,
	sdlpui_dialog_handle_textedit,
	sdlpui_dialog_handle_mouseclick,
	sdlpui_dialog_handle_mousemove,
	sdlpui_dialog_handle_mousewheel,
	sdlpui_menu_handle_loses_mouse,
	sdlpui_menu_handle_loses_key,
	sdlpui_menu_handle_window_loses_mouse,
	sdlpui_menu_handle_window_loses_key,
	render_simple_menu,
	NULL,
	goto_simple_menu_first_control,
	step_simple_menu_control,
	find_simple_menu_control_containing,
	get_simple_menu_parent,
	get_simple_menu_child,
	get_simple_menu_parent_ctrl,
	set_simple_menu_child,
	resize_simple_menu,
	query_simple_menu_natural_size,
	query_simple_menu_minimum_size,
	cleanup_simple_menu
};

static const struct sdlpui_dialog_funcs simple_info_funcs = {
	sdlpui_dialog_handle_key,
	sdlpui_dialog_handle_textin,
	sdlpui_dialog_handle_textedit,
	sdlpui_dialog_handle_mouseclick,
	sdlpui_dialog_handle_mousemove,
	sdlpui_dialog_handle_mousewheel,
	sdlpui_dialog_handle_loses_mouse,
	sdlpui_dialog_handle_loses_key,
	sdlpui_dialog_handle_window_loses_mouse,
	sdlpui_dialog_handle_window_loses_key,
	render_simple_info,
	sdlpui_dismiss_dialog,
	goto_simple_info_first_control,
	NULL,
	find_simple_info_control_containing,
	NULL,
	NULL,
	NULL,
	NULL,
	resize_simple_info,
	query_simple_info_natural_size,
	NULL,
	cleanup_simple_info
};


/**
 * Menus react to some additional keyboard events since the geometry allows
 * for easy interpretations of cursor movements.  Otherwise, they act like
 * simple dialogs.
 */
static bool handle_simple_menu_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e)
{
	/*
	 * Swap which keys do what depending on the orientation of the menu.
	 * fwd_alt1, fwd_alt2, fwd_alt3 are for the keys corresponding to
	 * the in-game east (for vertical menus) or south (for horizontal
	 * menus) motion commands from either keyset.
	 */
	struct k_dirsyms {
		SDL_Keycode fwd, fwd_alt1, fwd_alt2, fwd_alt3, bck, nxt, prv;
	};
	static const struct k_dirsyms vsyms = {
		SDLK_RIGHT,
		SDLK_6,
		SDLK_KP_6,
		SDLK_l,
		SDLK_LEFT,
		SDLK_DOWN,
		SDLK_UP
	};
	static const struct k_dirsyms hsyms = {
		SDLK_DOWN,
		SDLK_2,
		SDLK_KP_2,
		SDLK_j,
		SDLK_UP,
		SDLK_RIGHT,
		SDLK_LEFT
	};
	const struct k_dirsyms *csyms;
	/*
	 * Most of the additional event handling is as a proxy for the menu's
	 * control with keyboard focus so remember what that is.
	 */
	struct sdlpui_control *c = d->c_key;
	struct sdlpui_simple_menu *p;
	SDL_Keymod mods;

	/* Relay to the control with focus.  if it handles it we're done. */
	if (c && c->ftb->handle_key && (*c->ftb->handle_key)(c, d, w, e)) {
		return true;
	}

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	csyms = (p->vertical) ? &vsyms : &hsyms;

	mods = sdlpui_get_interesting_keymods();
	if (e->keysym.sym == csyms->fwd) {
		/*
		 * Invoke the default action for a menu entry which will
		 * descend deeper into the menu hierarchy if that entry leads
		 * to a submenu.  If there's no entry with keyboard focus,
		 * give focus to the first active entry.
		 */
		if (c) {
			if (e->state == SDL_PRESSED) {
				if (mods == KMOD_NONE) {
					(*c->ftb->arm)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				}
			} else {
				/*
				 * Always disarm, regardless of the modifier
				 * state, in case the modifier keys changed
				 * between the press and release.
				 */
				if (c->ftb->disarm) {
					(*c->ftb->disarm)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				}
				if (mods == KMOD_NONE
						&& c->ftb->respond_default) {
					SDLPUI_EVENT_TRACER("control", c,
						"(not extracted)",
						"invoking default response");
					(*c->ftb->respond_default)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				}
			}
		} else if (e->state == SDL_RELEASED
				&& d->ftb->goto_first_control) {
			(*d->ftb->goto_first_control)(d, w);
		}
		return true;
	}

	if (e->keysym.sym == csyms->fwd_alt1 || e->keysym.sym == csyms->fwd_alt2
			|| e->keysym.sym == csyms->fwd_alt3) {
		/*
		 * Like fwd, but as these symbols also trigger the text input
		 * handler, just handle arming and disarming of the menu
		 * button here.
		 */
		if (c) {
			if (e->state == SDL_PRESSED) {
				if (mods == KMOD_NONE && c->ftb->arm) {
					(*c->ftb->arm)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				}
			} else {
				/*
				 * Always disarm, regardless of the modifier
				 * state, in case the modifier keys changed
				 * between the press and release.
				 */
				if (c->ftb->disarm) {
					(*c->ftb->disarm)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				}
			}
		}
		return true;
	}

	if (e->keysym.sym == csyms->bck) {
		/*
		 * Back out to the previous level of the menu hierarchy, if any.
		 */
		if (e->state == SDL_RELEASED && mods == KMOD_NONE) {
			sdlpui_dialog_give_key_focus_to_parent(d, w);
		}
		return true;
	}

	if (e->keysym.sym == csyms->nxt) {
		/*
		 * Go to the next active button in the menu or wrap around to
		 * the first active button in the menu if already at the end.
		 * If the menu doesn't already have key focus, give it key
		 * focus and go to the first active button.
		 */
		if (e->state == SDL_RELEASED && mods == KMOD_NONE) {
			if (c) {
				if (d->ftb->step_control) {
					(*d->ftb->step_control)(d, w, c, true);
				}
			} else if (d->ftb->goto_first_control) {
				(*d->ftb->goto_first_control)(d, w);
			}
		}
		return true;
	}

	if (e->keysym.sym == csyms->prv) {
		/*
		 * Go to the previous active button in the menu or wrap around
		 * to the first active button in the menu if already at the end.
		 * If the menu doesn't already have key focus, give it key
		 * focus and go to the first active button.
		 */
		if (e->state == SDL_RELEASED && mods == KMOD_NONE) {
			if (c) {
				if (d->ftb->step_control) {
					(*d->ftb->step_control)(d, w, c, false);
				}
			} else if (d->ftb->goto_first_control) {
				(*d->ftb->goto_first_control)(d, w);
			}
		}
		return true;
	}

	return sdlpui_dialog_handle_key(d, w, e);
}


static bool handle_simple_menu_textin(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextInputEvent *e)
{
	/*
	 * Swap which keys do what depending on the orientation of the menu.
	 * All correspond to in-game motion commands from either keyset.
	 */
	struct ti_dirsyms {
		uint32_t fwd_alt1, fwd_alt2, bck_alt1, bck_alt2, nxt_alt1,
			nxt_alt2, prv_alt1, prv_alt2;
	};
	static const struct ti_dirsyms vsyms = {
		/* East descends into the menu hierarchy. */
		'6', 'l',
		/* West backs out. */
		'4', 'h',
		/* South goes to the next item in this menu. */
		'2', 'j',
		/* North goes to the previous item in this menu. */
		'8', 'k'
	};
	static const struct ti_dirsyms hsyms = {
		/* South descends. */
		'2', 'j',
		/* North backs out. */
		'8', 'k',
		/* East goes to the next item in this menu. */
		'6', 'l',
		/* West goes to the previous item in this menu. */
		'4', 'h'
	};
	const struct ti_dirsyms *csyms;
	/*
	 * Most of the additional event handling is as a proxy for the menu's
	 * control with keyboard focus so remember what that is.
	 */
	struct sdlpui_control *c = d->c_key;
	struct sdlpui_simple_menu *p;
	uint32_t ch = sdlpui_utf8_to_codepoint(e->text);

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	csyms = (p->vertical) ? &vsyms : &hsyms;

	if (ch == csyms->fwd_alt1 || ch == csyms->fwd_alt2) {
		/*
		 * Invoke the default action on the menu entry which will
		 * descend deeper into the menu hierarchy if the entry leads
		 * to a submenu.  If there's no menu entry with keyboard focus,
		 * give keyboard focus to the first active entry.
		 */
		if (c) {
			if (c->ftb->respond_default) {
				SDLPUI_EVENT_TRACER("control", c,
					"(not extracted)",
					"invoking default response");
				(*c->ftb->respond_default)(c, d, w,
					SDLPUI_ACTION_HINT_KEY);
			}
		} else if (d->ftb->goto_first_control) {
			(*d->ftb->goto_first_control)(d, w);
		}
		return true;
	}

	if (ch == csyms->bck_alt1 || ch == csyms->bck_alt2) {
		/*
		 * Back out to the previous level of the menu hierarchy, if any.
		 */
		sdlpui_dialog_give_key_focus_to_parent(d, w);
		return true;
	}

	if (ch == csyms->nxt_alt1 || ch == csyms->nxt_alt2) {
		/*
		 * Go to the next active button in the menu or wrap around to
		 * the first active button in the menu if already at the end.
		 * If the menu doesn't already have key focus, give it key
		 * focus and go to the first active button.
		 */
		if (c) {
			if (d->ftb->step_control) {
				(*d->ftb->step_control)(d, w, c, true);
			}
		} else if (d->ftb->goto_first_control) {
			(*d->ftb->goto_first_control)(d, w);
		}
		return true;
	}

	if (ch == csyms->prv_alt1 || ch == csyms->prv_alt2) {
		/*
		 * Got to the previous active button in the menu or wrap around
		 * to the last active button in the menu if already at the
		 * beginning.  If the menu doesn't already have key focus,
		 * give it key focus and go to the first active button.
		 */
		if (c) {
			if (d->ftb->step_control) {
				(*d->ftb->step_control)(d, w, c, false);
			}
		} else if (d->ftb->goto_first_control) {
			(*d->ftb->goto_first_control)(d, w);
		}
		return true;
	}

	return sdlpui_dialog_handle_textin(d, w, e);
}


static void render_simple_menu(struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	struct SDL_Renderer *r = sdlpui_get_renderer(w);
	SDL_Rect dst_r = d->rect;
	int i = 0;
	struct sdlpui_simple_menu *p;
	const SDL_Color *color;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	SDLPUI_RENDER_TRACER("simple menu", d, "(not extracted)", d->rect,
		d->rect, d->texture);

	SDL_SetRenderTarget(r, d->texture);
	color = sdlpui_get_color(w, SDLPUI_COLOR_MENU_BG);
	SDL_SetRenderDrawColor(r, color->r, color->g, color->b, color->a);
	if (d->texture) {
		dst_r.x = 0;
		dst_r.y = 0;
		SDL_RenderClear(r);
	} else {
		SDL_RenderFillRect(r, &dst_r);
	}
	while (1) {
		if (i >= p->n_vis) {
			break;
		}
		if (p->v_ctrls[i]->ftb->render) {
			(*p->v_ctrls[i]->ftb->render)(p->v_ctrls[i], d, w, r);
		}
		++i;
	}
	if (p->border) {
		color = sdlpui_get_color(w, SDLPUI_COLOR_MENU_BORDER);

		SDL_SetRenderDrawColor(r, color->r, color->g, color->b,
			color->a);
		SDL_RenderDrawRect(r, &dst_r);
	}
	d->dirty = false;
}


static void goto_simple_menu_first_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	struct sdlpui_simple_menu *p;
	int i = 0;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	while (1) {
		int comp_ind;

		if (i >= p->n_vis) {
			/* There's no members that can take focus. */
			SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
				"gains key focus");
			if (d->c_key && d->c_key->ftb->lose_key) {
				(*d->c_key->ftb->lose_key)(d->c_key, d, w,
					NULL, d);
			}
			d->c_key = NULL;
			/* Anyways, still give the dialog key focus. */
			sdlpui_dialog_gain_key_focus(w, d);
			return;
		}
		comp_ind = (p->v_ctrls[i]->ftb->get_interactable_component) ?
			(p->v_ctrls[i]->ftb->get_interactable_component)(
				p->v_ctrls[i], true) : 0;
		if (comp_ind) {
			SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
				"gains key focus");
			if (d->c_key && d->c_key != p->v_ctrls[i]
					&& d->c_key->ftb->lose_key) {
				(*d->c_key->ftb->lose_key)(d->c_key, d, w,
					p->v_ctrls[i], d);
			}
			SDL_assert(p->v_ctrls[i]->ftb->gain_key);
			(*p->v_ctrls[i]->ftb->gain_key)(
				p->v_ctrls[i], d, w, comp_ind - 1);
			d->c_key = p->v_ctrls[i];
			sdlpui_dialog_gain_key_focus(w, d);
			return;
		}
		++i;
	}
}


static void step_simple_menu_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *c,
		bool forward)
{
	struct sdlpui_simple_menu *p;
	int istart, itry;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	SDL_assert(c >= p->controls && c < p->controls + p->number);

	if (c->ftb->step_within && (*c->ftb->step_within)(c, forward)) {
		d->dirty = true;
		sdlpui_signal_redraw(w);
		return;
	}

	istart = 0;
	while (1) {
		if (istart >= p->n_vis) {
			/*
			 * c should be an active control in the menu, but it is
			 * not.
			 */
			SDL_assert(0);
			return;
		}
		if (c == p->v_ctrls[istart]) {
			break;
		}
		++istart;
	}
	itry = istart;
	while (1) {
		int comp_ind;

		if (forward) {
			++itry;
			if (itry == p->n_vis) {
				itry = 0;
			}
		} else {
			--itry;
			if (itry == -1) {
				itry = p->n_vis - 1;
			}
		}
		if (itry == istart) {
			/*
			 * Wrapped around without finding another control that
			 * can accept focus.
			 */
			SDL_assert(d->c_key == c);
			break;
		}
		comp_ind = (p->v_ctrls[itry]->ftb->get_interactable_component) ?
			(*p->v_ctrls[itry]->ftb->get_interactable_component)(
				p->v_ctrls[itry], forward) : 0;
		if (comp_ind) {
			if (d->c_key && d->c_key != p->v_ctrls[itry]
					&& d->c_key->ftb->lose_key) {
				(*d->c_key->ftb->lose_key)(d->c_key, d, w,
					p->v_ctrls[itry], d);
			}
			if (p->v_ctrls[itry]->ftb->gain_key) {
				(*p->v_ctrls[itry]->ftb->gain_key)(
					p->v_ctrls[itry], d, w, comp_ind - 1);
			}
			d->c_key = p->v_ctrls[itry];
			break;
		}
	}
}


static struct sdlpui_control *find_simple_menu_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind)
{
	struct sdlpui_simple_menu *p;
	int ilo, ihi;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	SDL_assert(p->n_vis >= 0);
	if (p->n_vis == 0 || !sdlpui_is_in_dialog(d, x, y)) {
		*comp_ind = 0;
		return NULL;
	}

	/* Make the coordinates relative to the dialog. */
	x -= d->rect.x;
	y -= d->rect.y;

	/* Use a binary search to locate the control */
	ilo = 0;
	ihi = p->n_vis;
	while (1) {
		int imid;

		if (ilo == ihi - 1) {
			if (x < p->v_ctrls[ilo]->rect.x
					|| x >= p->v_ctrls[ilo]->rect.x
						+ p->v_ctrls[ilo]->rect.w
					|| y < p->v_ctrls[ilo]->rect.y
					|| y >= p->v_ctrls[ilo]->rect.y
						+ p->v_ctrls[ilo]->rect.h) {
				*comp_ind = 0;
				return NULL;
			}
			if (p->v_ctrls[ilo]->ftb->get_interactable_component_at) {
				int ind = (*p->v_ctrls[ilo]->ftb->get_interactable_component_at)(
					p->v_ctrls[ilo], x, y);

				if (ind == 0) {
					*comp_ind = 0;
					return NULL;
				}
				*comp_ind = ind - 1;
			} else if (!p->v_ctrls[ilo]->ftb->get_interactable_component
					|| !(*p->v_ctrls[ilo]->ftb->get_interactable_component)(p->v_ctrls[ilo], true)) {
				*comp_ind = 0;
				return NULL;
			} else {
				*comp_ind = 0;
			}
			return p->v_ctrls[ilo];
		}
		imid = (ilo + ihi) / 2;
		if (p->vertical) {
			if (p->v_ctrls[imid]->rect.y > y) {
				ihi = imid;
				continue;
			}
			if (p->v_ctrls[imid]->rect.y + p->v_ctrls[imid]->rect.h
					<= y) {
				ilo = imid;
				continue;
			}
			if (x < p->v_ctrls[imid]->rect.x
					|| x >= p->v_ctrls[imid]->rect.x
						+ p->v_ctrls[imid]->rect.w) {
				*comp_ind = 0;
				return NULL;
			}
		} else {
			if (p->v_ctrls[imid]->rect.x > x) {
				ihi = imid;
				continue;
			}
			if (p->v_ctrls[imid]->rect.x + p->v_ctrls[imid]->rect.w
					<= x) {
				ilo = imid;
				continue;
			}
			if (y < p->v_ctrls[imid]->rect.y
					|| y >= p->v_ctrls[imid]->rect.y
						+ p->v_ctrls[imid]->rect.h) {
				*comp_ind = 0;
				return NULL;
			}
		}
		if (p->v_ctrls[imid]->ftb->get_interactable_component_at) {
			int ind = (*p->v_ctrls[imid]->ftb->get_interactable_component_at)(
				p->v_ctrls[imid], x, y);

			if (ind == 0) {
				*comp_ind = 0;
				return NULL;
			}
			*comp_ind = ind - 1;
		} else if (!p->v_ctrls[imid]->ftb->get_interactable_component
				|| !(*p->v_ctrls[imid]->ftb->get_interactable_component)(p->v_ctrls[imid], true)) {
			*comp_ind = 0;
			return NULL;
		} else {
			*comp_ind = 0;
		}
		return p->v_ctrls[imid];
	}
}


static struct sdlpui_dialog *get_simple_menu_parent(struct sdlpui_dialog *d)
{
	struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	return p->parent;
}


static struct sdlpui_dialog *get_simple_menu_child(struct sdlpui_dialog *d)
{
	struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	return p->child;
}


static struct sdlpui_control *get_simple_menu_parent_ctrl(
		struct sdlpui_dialog *d)
{
	struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	return p->parent_ctrl;
}


static void set_simple_menu_child(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child)
{
	struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	p->child = child;
}


/*
 * Requires that the height (if the menu is vertical) or width (if the menu is
 * horizontal) is at least as large as the minimum returned by
 * query_simple_menu_minimum_size().
 */
static void resize_simple_menu(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height)
{
	struct sdlpui_simple_menu *p;
	bool *vis;
	int i, ivis, first_end, work;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	/*
	 * First pass:  determine which buttons will definitely be visible
	 * and the boundary between the buttons attached to the front and
	 * the buttons attached to the end
	 */
	vis = SDL_calloc(p->number, sizeof(*vis));
	first_end = p->number;
	work = 0;
	for (i = 0; i < p->number; ++i) {
		int cw, ch;

		if (p->control_flags[i] & SDLPUI_MFLG_END_GRAVITY) {
			if (first_end == p->number) {
				first_end = i;
			}
		} else {
			/*
			 * Fail if there's more than one boundary between
			 * the buttons with different gravities.
			 */
			SDL_assert(first_end == p->number);
		}
		if ((p->control_flags[i] & SDLPUI_MFLG_CAN_HIDE)) {
			continue;
		}
		vis[i] = true;
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i],
			d, w, &cw, &ch);
		if (p->vertical) {
			work += ch;
			SDL_assert(work <= height);
		} else {
			work += cw;
			SDL_assert(work <= width);
		}
	}

	/*
	 * Second pass:  make others visible up to the size imposed;
	 * give preference to those earlier in the array of controls
	 */
	for (i = 0; i < p->number; ++i) {
		int cw, ch;

		if (!(p->control_flags[i] & SDLPUI_MFLG_CAN_HIDE)) {
			continue;
		}
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i],
			d, w, &cw, &ch);
		if (p->vertical) {
			if (work + ch > height) {
				break;
			}
			work += ch;
		} else {
			if (work + cw > width) {
				break;
			}
			work += cw;
		}
		vis[i] = true;
	}

	/*
	 * Third pass:  fill in v_ctrls and assign positions to all of the
	 * visible controls that are anchored to the front of the menu.
	 */
	work = 0;
	ivis = 0;
	for (i = 0; i < first_end; ++i) {
		int cw, ch;

		if (!vis[i]) {
			continue;
		}
		p->v_ctrls[ivis] = &p->controls[i];
		++ivis;
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i],
			d, w, &cw, &ch);
		if (p->vertical) {
			p->controls[i].rect.x = 0;
			p->controls[i].rect.y = work;
			work += ch;
			if (p->controls[i].ftb->resize) {
				(*p->controls[i].ftb->resize)(&p->controls[i],
					d, w, width, ch);
			} else {
				p->controls[i].rect.w = width;
				p->controls[i].rect.h = ch;
			}
		} else {
			p->controls[i].rect.x = work;
			p->controls[i].rect.y = 0;
			work += cw;
			if (p->controls[i].ftb->resize) {
				(*p->controls[i].ftb->resize)(&p->controls[i],
					d, w, cw, height);
			} else {
				p->controls[i].rect.w = cw;
				p->controls[i].rect.h = height;
			}
		}
	}

	/*
	 * Fourth pass:  fill in v_ctrls for the visible controls anchored to
	 * the end of the menu.
	 */
	for (i = first_end; i < p->number; ++i) {
		if (!vis[i]) {
			continue;
		}
		p->v_ctrls[ivis] = &p->controls[i];
		++ivis;
	}
	p->n_vis = ivis;

	/*
	 * Fifth pass:  assign positions to all of the visible controls
	 * that are anchored to the end of the menu.
	 */
	i = p->number;
	work = (p->vertical) ? height : width;
	while (i > first_end) {
		int cw, ch;

		--i;
		if (!vis[i]) {
			continue;
		}
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i],
			d, w, &cw, &ch);
		if (p->vertical) {
			work -= ch;
			p->controls[i].rect.x = 0;
			p->controls[i].rect.y = work;
			if (p->controls[i].ftb->resize) {
				(*p->controls[i].ftb->resize)(&p->controls[i],
					d, w, width, ch);
			} else {
				p->controls[i].rect.w = width;
				p->controls[i].rect.h = ch;
			}
		} else {
			work -= cw;
			p->controls[i].rect.x = work;
			p->controls[i].rect.y = 0;
			if (p->controls[i].ftb->resize) {
				(*p->controls[i].ftb->resize)(&p->controls[i],
					d, w, cw, height);
			} else {
				p->controls[i].rect.w = cw;
				p->controls[i].rect.h = height;
			}
		}
	}

	SDL_free(vis);
	d->rect.w = width;
	d->rect.h = height;
}


static void query_simple_menu_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	*width = 0;
	*height = 0;
	for (i = 0; i < p->number; ++i) {
		int cw, ch;

		SDL_assert(p->controls[i].ftb->query_natural_size);
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i], d, w,
			&cw, &ch);
		if (p->vertical) {
			if (*width < cw) {
				*width = cw;
			}
			*height += ch;
		} else {
			*width += cw;
			if (*height < ch) {
				*height = ch;
			}
		}
	}
}


static void query_simple_menu_minimum_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	*width = 0;
	*height = 0;
	for (i = 0; i < p->number; ++i) {
		int cw, ch;

		if (p->control_flags[i] & SDLPUI_MFLG_CAN_HIDE) {
			continue;
		}
		SDL_assert(p->controls[i].ftb->query_natural_size);
		(*p->controls[i].ftb->query_natural_size)(&p->controls[i], d, w,
			&cw, &ch);
		if (p->vertical) {
			if (*width < cw) {
				*width = cw;
			}
			*height += ch;
		} else {
			*width += cw;
			if (*height < ch) {
				*height = ch;
			}
		}
	}
}


static void cleanup_simple_menu(struct sdlpui_dialog *d)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	for (i = 0; i < p->number; ++i) {
		if (p->controls[i].ftb->cleanup) {
			(*p->controls[i].ftb->cleanup)(&p->controls[i]);
		}
	}
	SDL_free(p->controls);
	SDL_free(p->v_ctrls);
	SDL_free(p->control_flags);
	SDL_free(p);
}


static void render_simple_info(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	SDL_Renderer *r = sdlpui_get_renderer(w);
	SDL_Rect dst_r = d->rect;
	struct sdlpui_simple_info *id;
	const SDL_Color *color;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	id = d->priv;
	SDLPUI_RENDER_TRACER("simple info", d, "(not extracted)", d->rect,
		d->rect, d->texture);

	SDL_SetRenderTarget(r, d->texture);
	color = sdlpui_get_color(w, SDLPUI_COLOR_DIALOG_BG);
	SDL_SetRenderDrawColor(r, color->r, color->g, color->b, color->a);
	if (d->texture) {
		SDL_RenderClear(r);
		dst_r.x = 0;
		dst_r.y = 0;
	} else {
		SDL_RenderFillRect(r, &dst_r);
	}
	for (i = 0; i < id->number; ++i) {
		if (id->labels[i].ftb->render) {
			(*id->labels[i].ftb->render)(&id->labels[i], d, w, r);
		}
	}
	if (id->button.ftb->render) {
		(*id->button.ftb->render)(&id->button, d, w, r);
	}
	/* Give it a border. */
	color = sdlpui_get_color(w, SDLPUI_COLOR_DIALOG_BORDER);
	SDL_RenderDrawRect(r, &dst_r);
	color = sdlpui_get_color(w, SDLPUI_COLOR_COUNTERSINK);
	++dst_r.x;
	++dst_r.y;
	dst_r.w -= 2;
	dst_r.h -= 2;
	SDL_RenderDrawRect(r, &dst_r);
	d->dirty = false;
}


static void goto_simple_info_first_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	struct sdlpui_simple_info *id;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	id = d->priv;

	SDL_assert(id->button.ftb->gain_key);
	(*id->button.ftb->gain_key)(&id->button, d, w, 0);
	SDL_assert(!d->c_key || d->c_key == &id->button);
	d->c_key = &id->button;
	sdlpui_dialog_gain_key_focus(w, d);
}


static struct sdlpui_control *find_simple_info_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind)
{
	struct sdlpui_simple_info *id;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	id = d->priv;

	*comp_ind = 0;
	return (sdlpui_is_in_control(&id->button, d, x, y)) ?
		&id->button : NULL;
}


static void resize_simple_info(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height)
{
	struct sdlpui_simple_info *psi;
	int i, y, ch, cw;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO);
	psi = (struct sdlpui_simple_info*)d->priv;
#ifdef NDEBUG
	{
		int dw, dh;

		query_simple_info_natural_size(d, w, &dw, &dh);
		SDL_assert(width >= dw && height >= dh);
	}
#endif

	y = 0;
	for (i = 0; i < psi->number; ++i) {
		(*psi->labels[i].ftb->query_natural_size)(&psi->labels[i],
			d, w, &cw, &ch);
		if (psi->labels[i].ftb->resize) {
			(*psi->labels[i].ftb->resize)(&psi->labels[i], d, w,
				width, ch);
		} else {
			psi->labels[i].rect.w = width;
			psi->labels[i].rect.h = ch;
		}
		psi->labels[i].rect.x = 0;
		psi->labels[i].rect.y = y;
		y += ch;
	}
	(*psi->button.ftb->query_natural_size)(&psi->button, d, w, &cw, &ch);
	if (psi->button.ftb->resize) {
		(*psi->button.ftb->resize)(&psi->button, d, w, cw, ch);
	} else {
		psi->button.rect.w = cw;
		psi->button.rect.h = ch;
	}
	SDL_assert(width >= cw);
	psi->button.rect.x = (width - cw) / 2;
	SDL_assert(y < height - ch);
	psi->button.rect.y = height - ch;
	d->rect.w = width;
	d->rect.h = height;
}


static void query_simple_info_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height)
{
	int dw = 0, dh = 0, cw, ch, i;
	struct sdlpui_simple_info *psi;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO);
	psi = (struct sdlpui_simple_info*)d->priv;
	for (i = 0; i < psi->number; ++i) {
		(*psi->labels[i].ftb->query_natural_size)(&psi->labels[i],
			d, w, &cw, &ch);
		dw = (dw >= cw) ? dw : cw;
		dh += ch;
	}
	(*psi->button.ftb->query_natural_size)(&psi->button, d, w, &cw, &ch);
	dw = (dw >= cw) ? dw : cw;
	/* Leave space between the labels and the button. */
	dh += ch + ch;
	*width = dw;
	*height = dh;
}


static void cleanup_simple_info(struct sdlpui_dialog *d)
{
	struct sdlpui_simple_info *id;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	id = d->priv;

	for (i = 0; i < id->number; ++i) {
		if (id->labels[i].ftb->cleanup) {
			(*id->labels[i].ftb->cleanup)(&id->labels[i]);
		}
	}
	SDL_free(id->labels);
	if (id->button.ftb->cleanup) {
		(*id->button.ftb->cleanup)(&id->button);
	}
	SDL_free(id);
}


/**
 * Determine if a given coordinate, relative to the window, is in a dialog.
 *
 * \param d is the dialog of interest.
 * \param x is the horizontal coordinate, relative to the window's upper left
 * corner, to test.
 * \param y is the vertical coordinate, relative to the window's upper left
 * corner, to test.
 * \return true if (x, y) is in the control and false otherwise.
 */
bool sdlpui_is_in_dialog(const struct sdlpui_dialog *d, Sint32 x, Sint32 y)
{
	if (x < d->rect.x || y < d->rect.y || x >= d->rect.x + d->rect.w
			|| y >= d->rect.y + d->rect.h) {
		return false;
	}
	return true;
}


/**
 * Determine if the dialog, other, is a descendant of ancestor.
 *
 * \param ancestor is the dialog to test as a possible ancestor of other.
 * Must not be NULL.
 * \param other is the dialog to test as a possible descendant of ancestor.
 * May be NULL.
 * \return true if other is a descendant of ancestor; return false if
 * other is NULL or is not a descendant of ancestor.
 */
bool sdlpui_is_descendant_dialog(struct sdlpui_dialog *ancestor,
		const struct sdlpui_dialog *other)
{
	while (1) {
		struct sdlpui_dialog *child =
			sdlpui_get_dialog_child(ancestor);

		if (!child) {
			return false;
		}
		if (child == other) {
			return true;
		}
		ancestor = child;
	}
}

/**
 * Pop up a dialog.
 *
 * \param d is the dialog of interest.
 * \param w is the window containing the dialog.
 * \param give_key_focus causes, if true, the dialog to be given key focus
 * when it is popped up.
 */
void sdlpui_popup_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		bool give_key_focus)
{
	/* Assume it may have been obscured and needs to be redrawn. */
	d->dirty = true;
	sdlpui_dialog_push_to_top(w, d);
	if (give_key_focus) {
		sdlpui_dialog_gain_key_focus(w, d);
	}
}


/**
 * Remove the dialog, any of its child dialogs, and, if all_parents is true,
 * any of its parents up to the first parent that is pinned.
 *
 * \param d is the dialog to remove.
 * \param w is the window containing the dialog.
 * \param all_parents will cause all of the parents, up to the first parent
 * that is pinned, if true.
 */
void sdlpui_popdown_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		bool all_parents)
{
	struct sdlpui_dialog *stopping_point = (all_parents) ?
		NULL : sdlpui_get_dialog_parent(d);

	while (1) {
		struct sdlpui_dialog *child = sdlpui_get_dialog_child(d);

		if (!child) {
			break;
		}
		d = child;
	}

	while (1) {
		struct sdlpui_dialog *parent = sdlpui_get_dialog_parent(d);
		struct sdlpui_control *parent_ctrl =
			sdlpui_get_dialog_parent_ctrl(d);

		if (d->pop_callback) {
			(*d->pop_callback)(d, w, false);
		}
		sdlpui_dialog_pop(w, d);
		if (parent_ctrl && parent_ctrl->ftb->lose_child) {
			(*parent_ctrl->ftb->lose_child)(parent_ctrl, d);
		}
		if (parent) {
			SDL_assert(parent->ftb->set_child);
			(*parent->ftb->set_child)(parent, NULL);

			/*
			 * If the parent control has key focus but not mouse
			 * focus, lose that focus when the child is lost.
			 */
			if (parent_ctrl && parent->c_key == parent_ctrl
					&& parent->c_mouse != parent_ctrl) {
				if (parent_ctrl->ftb->lose_key) {
					(*parent_ctrl->ftb->lose_key)(
						parent_ctrl, parent, w,
						NULL, NULL);
				}
				parent->c_key = NULL;
			}
		}
		if (d->ftb->cleanup) {
			(*d->ftb->cleanup)(d);
		}
		SDL_free(d);

		if (parent == stopping_point || (parent && parent->pinned)) {
			break;
		}
		d = parent;
	}
}


/**
 * Give the keyboard focus for a dialog or menu to its parent.  If there is no
 * parent and the dialog is not pinned, acts like sdlpui_popdown_dialog(d, w,
 * false).
 *
 * \param d is the dialog that's ceding focus.
 * \param w is the window containing the dialog.
 */
void sdlpui_dialog_give_key_focus_to_parent(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	struct sdlpui_dialog *parent = sdlpui_get_dialog_parent(d);

	if (parent) {
		if (d->c_key != NULL && d->c_key->ftb->lose_key) {
			(*d->c_key->ftb->lose_key)(d->c_key, d, w, NULL,
				parent);
		}
		d->c_key = NULL;
		SDLPUI_EVENT_TRACER("dialog", d,
			"(not extracted)", "yields key focus");
		sdlpui_dialog_yield_key_focus(w, d);
		SDLPUI_EVENT_TRACER("dialog", parent,
			"(not extracted)", "gains key focus");
		sdlpui_dialog_gain_key_focus(w, parent);
	} else {
		SDLPUI_EVENT_TRACER("dialog", d,
			"(not extracted)", "yields key focus");
		sdlpui_dialog_yield_key_focus(w, d);
		if (!d->pinned) {
			SDLPUI_EVENT_TRACER("dialog", d,
				"(not extracted)", "popping down");
			sdlpui_popdown_dialog(d, w, false);
		}
	}
}


/**
 * For a nested menu/dialog, return its parent.
 *
 * \param d is the dialog to query.
 * \return the parent.  That will be NULL if the menu/dialog does not have
 * a parent.
 */
struct sdlpui_dialog *sdlpui_get_dialog_parent(struct sdlpui_dialog *d)
{
	return (d->ftb->get_parent) ? (*d->ftb->get_parent)(d) : NULL;
}


/**
 * For a nested menu/dialog, return its child.
 *
 * \param d is the dialog to query.
 * \return the child.  That will be NULL if the menu/dialog does not have
 * a child.
 */
struct sdlpui_dialog *sdlpui_get_dialog_child(struct sdlpui_dialog *d)
{
	return (d->ftb->get_child) ? (*d->ftb->get_child)(d) : NULL;
}


/**
 * For a nested menu/dialog, return its parent control.
 *
 * \param d is the dialog to query.
 * \return the parent control for the dialog.  That will be NULL if the
 * menu/dialog is not a nested menu.
 */
struct sdlpui_control *sdlpui_get_dialog_parent_ctrl(struct sdlpui_dialog *d)
{
	return (d->ftb->get_parent_ctrl) ? (*d->ftb->get_parent_ctrl)(d) : NULL;
}


/**
 * Perform basic handling of a keyboard event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e)
{
	SDL_Keymod mods;

	/* Relay to the control with focus.  If it handles it, we are done. */
	if (d->c_key && d->c_key->ftb->handle_key
			&& (*d->c_key->ftb->handle_key)(d->c_key, d, w, e)) {
		return true;
	}

	mods = sdlpui_get_interesting_keymods();
	switch (e->keysym.sym) {
	case SDLK_ESCAPE:
		if (e->state == SDL_RELEASED && mods == KMOD_NONE) {
			while (1) {
				struct sdlpui_dialog *child =
					sdlpui_get_dialog_child(d);

				if (!child) {
					break;
				}
				d = child;
			}
			if (!d->pinned) {
				SDLPUI_EVENT_TRACER("dialog", d,
					"(not extracted)", "popping down");
				sdlpui_popdown_dialog(d, w, true);
			}
		}
		break;

	case SDLK_RETURN:
		if (e->state == SDL_RELEASED && mods == KMOD_NONE
				&& d->ftb->respond_default) {
			SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
				"invoking default response");
			(*d->ftb->respond_default)(d, w);
		}
		break;

	case SDLK_TAB:
		if (e->state == SDL_RELEASED
				&& (mods & ~(KMOD_SHIFT | KMOD_CTRL))
				== KMOD_NONE) {
			if (!d->c_key) {
				if (d->ftb->goto_first_control) {
					(*d->ftb->goto_first_control)(d, w);
				}
			} else if (d->ftb->step_control) {
				(*d->ftb->step_control)(d, w, d->c_key,
					(mods & (KMOD_SHIFT)) == 0);
			}
		}
		break;
	}

	/* Swallow the event, even if nothing was done. */
	return true;
}


/**
 * Perform basic handling of a text input event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_textin(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextInputEvent *e)
{
	/* Relay to the control with focus.  If it handles it, we are done. */
	if (d->c_key && d->c_key->ftb->handle_textin
			&& (*d->c_key->ftb->handle_textin)(d->c_key, d, w, e)) {
		return true;
	}

	/* Do nothing and swallow the event. */
	return true;
}


/**
 * Perform basic handling of a text editing event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_textedit(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextEditingEvent *e)
{
	/* Relay to the control with focus.  If it handles it, we are done. */
	if (d->c_key && d->c_key->ftb->handle_textedit
			&& (*d->c_key->ftb->handle_textedit)(
				d->c_key, d, w, e)) {
		return true;
	}

	/* Do nothing and swallow the event. */
	return true;
}


/**
 * Perform basic handling of a mouse button event for a dialog or menu.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_mouseclick(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseButtonEvent *e)
{
	/* Relay to the control with focus.  If it handles it, we are done. */
	if (d->c_mouse && d->c_mouse->ftb->handle_mouseclick
			&& (*d->c_mouse->ftb->handle_mouseclick)(
				d->c_mouse, d, w, e)) {
		return true;
	}

	/* Do nothing and swallow the event. */
	return true;
}


/**
 * Perform basic handling of a mouse motion event for a menu or dialog.
 *
 * \param d is the menu or dialog.
 * \param w is the window containing the menu or dialog.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_mousemove(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseMotionEvent *e)
{
	struct sdlpui_control *c_mouse = d->c_mouse, *c;
	int comp_ind;

	/* Relay to the control with focus.  If it handles it, we are done. */
	if (c_mouse && c_mouse->ftb->handle_mousemove
			&& (*c_mouse->ftb->handle_mousemove)(
				c_mouse, d, w, e)) {
		return true;
	}

	/*
	 * Ignore motion events while a mouse button is pressed (at least up
	 * to the point that the mouse leaves the window).
	 */
	if (e->state != 0) {
		return true;
	}

	/*
	 * Otherwise, see if the mouse has entered another control in the
	 * dialog.  If it has, give focus to that control.
	 */
	c = (d->ftb->find_control_containing) ?
		(*d->ftb->find_control_containing)(d, w, e->x, e->y, &comp_ind) :
		NULL;
	if (c) {
		if (c_mouse && c_mouse->ftb->lose_mouse) {
			SDLPUI_EVENT_TRACER("control", d->c_mouse,
				"(not extracted)", "loses mouse focus");
			(*c_mouse->ftb->lose_mouse)(c_mouse, d, w, c, d);
		}
		if (c->ftb->gain_mouse) {
			SDLPUI_EVENT_TRACER("control", c, "(not extracted)",
				"gains mouse focus");
			(*c->ftb->gain_mouse)(c, d, w, comp_ind);
		}
		d->c_mouse = c;

		/* Have keyboard focus follow the mouse. */
		if (d->c_key != c) {
			if (d->c_key && d->c_key->ftb->lose_key) {
				SDLPUI_EVENT_TRACER("control", d->c_key,
					"(not extracted)", "loses key focus");
				(*d->c_key->ftb->lose_key)(d->c_key, d, w, c,
					d);
			}
			if (c && c->ftb->gain_key) {
				SDLPUI_EVENT_TRACER("control", c,
					"(not extracted)", "gains key focus");
				(c->ftb->gain_key)(c, d, w, comp_ind);
			}
			d->c_key = c;
		}
	}
	if (c || sdlpui_is_in_dialog(d, e->x, e->y)) {
		if (!c) {
			if (c_mouse) {
				if (c_mouse && c_mouse->ftb->lose_mouse) {
					SDLPUI_EVENT_TRACER("control",
						c_mouse, "(not extracted)",
						"loses mouse focus");
					(*c_mouse->ftb->lose_mouse)(
						c_mouse, d, w, NULL, d);
				}
				d->c_mouse = NULL;
			}
			/* Have keyboard focus follow the mouse. */
			if (d->c_key) {
				if (d->c_key && d->c_key->ftb->lose_key) {
					SDLPUI_EVENT_TRACER("control",
						d->c_key, "(not extracted)",
						"loses key focus");
					(*d->c_key->ftb->lose_key)(
						d->c_key, d, w, NULL, d);
				}
				d->c_key = NULL;
			}
		}
		SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
			"gains mouse focus");
		SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
			"gains key focus");
		sdlpui_dialog_gain_mouse_focus(w, d);
		sdlpui_dialog_gain_key_focus(w, d);
		return true;
	}

	/*
	 * Let the window handle the mouse motion.  For now keep focus though
	 * moving into another dialog could cause it to be lost.
	 */
	return false;
}


/**
 * Perform basic handling of a mouse wheel event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the window; otherwise return false.
 */
bool sdlpui_dialog_handle_mousewheel(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseWheelEvent *e)
{
	/* Relay to the control with focus.  If it handles it, we're done. */
	if (d->c_mouse && d->c_mouse->ftb->handle_mousewheel
			&& (*d->c_mouse->ftb->handle_mousewheel)(
				d->c_mouse, d, w, e)) {
		return true;
	}

	/* Do nothing and swallow the event. */
	return true;
}


/**
 * This is a synonym for sdlpui_popdown_dialog(d, w, false), usable as the
 * respond_default hook for a dialog.
 *
 * \param d is the dialog or menu to pop down.
 * \param w is the window containing the dialog or menu.
 */
void sdlpui_dismiss_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
		"popping down dialog");
	sdlpui_popdown_dialog(d, w, false);
}


/**
 * Respond to the mouse leaving the containing window.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog.
 */
void sdlpui_dialog_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	if (d->c_mouse) {
		if (d->c_mouse->ftb->disarm) {
			(*d->c_mouse->ftb->disarm)(d->c_mouse, d, w,
				SDLPUI_ACTION_HINT_NONE);
		}
		if (d->c_mouse->ftb->lose_mouse) {
			(*d->c_mouse->ftb->lose_mouse)(d->c_mouse, d, w, NULL,
				NULL);
		}
		/* Key focus follows mouse. */
		if (d->c_key == d->c_mouse && d->c_mouse->ftb->lose_key) {
			(*d->c_mouse->ftb->lose_key)(d->c_mouse, d, w, NULL,
				NULL);
		}
	}
	/* Key focus follows mouse. */
	if (d->c_key && d->c_key != d->c_mouse) {
		if (d->c_key->ftb->disarm) {
			(*d->c_key->ftb->disarm)(d->c_key, d, w,
				SDLPUI_ACTION_HINT_NONE);
		}
		if (d->c_key->ftb->lose_key) {
			(*d->c_key->ftb->lose_key)(d->c_key, d, w, NULL,
				NULL);
		}
	}
	d->c_mouse = NULL;
	d->c_key = NULL;
	sdlpui_dialog_yield_mouse_focus(w, d);
	sdlpui_dialog_yield_key_focus(w, d);
}


/**
 * Respond to the mouse leaving the containing window for a pulldown/popup menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 */
void sdlpui_menu_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	if (d->pinned) {
		sdlpui_dialog_handle_window_loses_mouse(d, w);
	} else {
		SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
			"popping down");
		sdlpui_popdown_dialog(d, w, true);
	}
}


/**
 * Respond to the containing window losing key focus.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog or menu.
 */
void sdlpui_dialog_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	if (d->c_key) {
		if (d->c_key->ftb->disarm) {
			(*d->c_key->ftb->disarm)(d->c_key, d, w,
				SDLPUI_ACTION_HINT_KEY);
		}
		if (d->c_key->ftb->lose_key) {
			(*d->c_key->ftb->lose_key)(d->c_key, d, w, NULL, NULL);
		}
	}
	d->c_key = NULL;
	sdlpui_dialog_yield_key_focus(w, d);
}


/**
 * Respond to the containing window losing key focus for a pulldown/popup menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 */
void sdlpui_menu_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	/*
	 * Dimiss all the child dialogs up to the one which has mouse focus
	 * or has a parent control with mouse focus.  If the dialog itself
	 * is not dismissed in that process, treat it like a generic dialog
	 * when the window loses key focus.
	 */
	struct sdlpui_dialog *deepest = d;

	while (1) {
		struct sdlpui_dialog *child = sdlpui_get_dialog_child(deepest);

		if (!child) {
			break;
		}
		deepest = child;
	}
	while (1) {
		struct sdlpui_dialog *d_mouse =
			sdlpui_dialog_with_mouse_focus(w);
		struct sdlpui_dialog *parent =
			sdlpui_get_dialog_parent(deepest);
		struct sdlpui_control *parent_ctrl =
			sdlpui_get_dialog_parent_ctrl(deepest);

		if (deepest->pinned || deepest->c_mouse
				|| deepest == d_mouse
				|| (parent && parent->c_mouse == parent_ctrl)) {
			if (deepest == d) {
				sdlpui_dialog_handle_window_loses_key(d, w);
			}
			break;
		}
		SDLPUI_EVENT_TRACER("dialog", deepest, "(not extracted)",
			"popping down");
		sdlpui_popdown_dialog(deepest, w, false);
		if (!parent) {
			break;
		}
		deepest = parent;
	}
}


/**
 * Respond to another dialog or menu taking mouse focus from this dialog.
 *
 * \param d is the dialog.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining mouse focus.  It may be NULL.
 * \param new_d is the dialog gaining mouse focus.  It may be NULL.
 */
void sdlpui_dialog_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	/*
	 * Gets the same treatment as if the mouse left the window containing
	 * the dialog.
	 */
	sdlpui_dialog_handle_window_loses_mouse(d, w);
}


/**
 * Respond to another dialog or menu taking mouse focus from this pulldown/popup
 * menu.
 *
 * \param d is the dialog.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining mouse focus.  It may be NULL.
 * \param new_d is the dialog gaining mouse focus.  It may be NULL.
 */
void sdlpui_menu_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	/*
	 * The mouse left the menu.  If the mouse is in a descendant, do
	 * not pop anything down and update what has focus in this menu.  If
	 * the mouse is in the parent control, pop down any children of this
	 * menu and update what has focus in this menu.  In other cases, pop
	 * down the menu, any children, and any parents up to the parent that
	 * contains the mouse or is pinned.
	 */
	bool pop_parents = true;

	if (new_d && sdlpui_is_descendant_dialog(d, new_d)) {
		pop_parents = false;
	} else if (new_c && new_d) {
		struct sdlpui_dialog *parent_d = sdlpui_get_dialog_parent(d);
		struct sdlpui_control *parent_c =
			sdlpui_get_dialog_parent_ctrl(d);

		if (new_d == parent_d && new_c == parent_c) {
			struct sdlpui_dialog *child_d =
				sdlpui_get_dialog_child(d);

			if (child_d && !child_d->pinned) {
				SDLPUI_EVENT_TRACER("dialog", child_d,
					"(not extracted)", "popping down");
				sdlpui_popdown_dialog(child_d, w, false);
			}
			pop_parents = false;
		}
	} else if (d->pinned) {
		pop_parents = false;
	}

	if (pop_parents) {
		while (1) {
			struct sdlpui_dialog *parent_d =
				sdlpui_get_dialog_parent(d);

			if (!parent_d || parent_d->pinned
					|| parent_d == new_d) {
				SDLPUI_EVENT_TRACER("dialog", d,
					"(not extracted)", "popping down");
				sdlpui_popdown_dialog(d, w, false);
				break;
			}
			if (new_d) {
				struct sdlpui_control *parent_c =
					sdlpui_get_dialog_parent_ctrl(d);

				if (new_c && new_d == parent_d
						&& new_c == parent_c) {
					struct sdlpui_dialog *child_d =
						sdlpui_get_dialog_child(d);

					SDL_assert(child_d);
					SDLPUI_EVENT_TRACER("dialog",
						child_d, "(not extracted)",
						"popping down");
					SDL_assert(!child_d->pinned);
					sdlpui_popdown_dialog(child_d, w,
						false);
					break;
				} else if (new_d == parent_d) {
					SDLPUI_EVENT_TRACER("dialog", d,
						"(not extracted)",
						"popping down");
					sdlpui_popdown_dialog(d, w, false);
					break;
				}
			}
			d = parent_d;
		}
	} else {
		if (d->c_mouse) {
			if (d->c_mouse->ftb->disarm) {
				(*d->c_mouse->ftb->disarm)(d->c_mouse, d, w,
					SDLPUI_ACTION_HINT_NONE);
			}
			if (d->c_mouse->ftb->lose_mouse) {
				(*d->c_mouse->ftb->lose_mouse)(d->c_mouse, d,
					w, new_c, new_d);
			}
			/* Key focus follows mouse. */
			if (d->c_key == d->c_mouse
					&& d->c_mouse->ftb->lose_key) {
				(*d->c_mouse->ftb->lose_key)(d->c_mouse, d,
					w, new_c, new_d);
			}
		}
		/* Key focus follows mouse. */
		if (d->c_key && d->c_key != d->c_mouse) {
			if (d->c_key->ftb->disarm) {
				(*d->c_key->ftb->disarm)(d->c_key, d, w,
					SDLPUI_ACTION_HINT_NONE);
			}
			if (d->c_key->ftb->lose_key) {
				(*d->c_key->ftb->lose_key)(d->c_key, d,
					w, new_c, new_d);
			}
		}
		d->c_mouse = NULL;
		d->c_key = NULL;
		sdlpui_dialog_yield_mouse_focus(w, d);
		sdlpui_dialog_yield_key_focus(w, d);
	}
}


/**
 * Respond to another dialog or menu taking key focus from this dialog.
 *
 * \param d is the dialog.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining key focus.  It may be NULL.
 * \param new_d is the dialog gaining key focus.  It may be NULL.
 */
void sdlpui_dialog_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	/*
	 * Gets the same treatment as if the window containing the dialog lost
	 * key focus.
	 */
	sdlpui_dialog_handle_window_loses_key(d, w);
}


/**
 * Respond to another dialog or menu taking key focus from this popup/pulldown
 * menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 * \param new_c is the new control with key focus.
 * \param new_d is the dialog or menu that contains new_c.
 */
void sdlpui_menu_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	/*
	 * The menu lost key focus.  If the dialog receiving focus is a
	 * descendant of this one, only update focus for this dialog.  If the
	 * control receiving focus is the parent control for this dialog or
	 * the parent control for this dialog has mouse focus, pop down the
	 * children of this dialog unless they have mouse focus and update focus
	 * for this dialog.  Othrwise, descend to the deepest child of
	 * this dialog and pop down it and all parents up to the first dialog
	 * that is either pinned, has mouse focus, or has a parent control
	 * that has mouse focus.  If this dialog is not popped down in that
	 * process, update what has focus.
	 */
	bool pop_parents = true;

	if (new_d && sdlpui_is_descendant_dialog(d, new_d)) {
		pop_parents = false;
	} else {
		struct sdlpui_dialog *parent_d = sdlpui_get_dialog_parent(d);
		struct sdlpui_control *parent_c =
			sdlpui_get_dialog_parent_ctrl(d);

		if ((new_c && new_d && new_d == parent_d
				&& new_c == parent_c)
				|| (parent_d && parent_c
				&& parent_d->c_mouse == parent_c)) {
			struct sdlpui_dialog *child_d =
				sdlpui_get_dialog_child(d);
			struct sdlpui_dialog *mouse_d =
				sdlpui_dialog_with_mouse_focus(w);

			if (child_d && !child_d->pinned
					&& (!mouse_d
					|| !sdlpui_is_descendant_dialog(d,
					mouse_d))) {
				SDLPUI_EVENT_TRACER("dialog", child_d,
					"(not extracted)", "popping down");
				sdlpui_popdown_dialog(child_d, w, false);
			}
			pop_parents = false;
		} else if (d->pinned) {
			pop_parents = false;
		}
	}

	if (pop_parents) {
		struct sdlpui_dialog *deepest = d;

		while (1) {
			struct sdlpui_dialog *child =
				sdlpui_get_dialog_child(deepest);

			if (!child) {
				break;
			}
			deepest = child;
		}
		while (1) {
			struct sdlpui_dialog *d_mouse =
				sdlpui_dialog_with_mouse_focus(w);
			struct sdlpui_dialog *parent =
				sdlpui_get_dialog_parent(deepest);
			struct sdlpui_control *parent_ctrl =
				sdlpui_get_dialog_parent_ctrl(deepest);

			if (deepest->pinned || deepest->c_mouse
					|| deepest == d_mouse
					|| (parent && parent->c_mouse
					== parent_ctrl)) {
				if (deepest == d) {
					pop_parents = false;
				}
				break;
			}
			SDLPUI_EVENT_TRACER("dialog", deepest,
				"(not extracted)", "popping down");
			sdlpui_popdown_dialog(deepest, w, false);
			if (!parent) {
				break;
			}
			deepest = parent;
		}
		if (pop_parents) {
			return;
		}
	}

	if (d->c_key) {
		if (d->c_key->ftb->disarm) {
			(*d->c_key->ftb->disarm)(d->c_key, d, w,
				SDLPUI_ACTION_HINT_NONE);
		}
		if (d->c_key->ftb->lose_key) {
			(*d->c_key->ftb->lose_key)(d->c_key, d, w, new_c,
				new_d);
		}
		d->c_key = NULL;
	}
	sdlpui_dialog_yield_key_focus(w, d);
}


/**
 * Begin constructing a simple menu.
 *
 * \param parent is the parent menu for the menu to be created or NULL if
 * the menu to be created is not nested.
 * \param parent_ctrl is the control in parent which the user interacted
 * with to create the new menu or NULL if the menu to be created does not
 * have a parent.
 * \param preallocated is the number of controls to allocate space for when
 * creating the menu.  The actual number of controls added can be greater than
 * that but that'll incur the extra work to resize the storage for the
 * controls.
 * \param vertical will,  if true, causes the controls in the new menu to
 * be layed out in a single column; if false, it causes the controls to layed
 * out in a single row.
 * \param border will, if true, cause a border to be drawn about the menu.
 * \param pop_callback will, if not NULL, be the function called when the
 * menu is popped up or down.
 * \param recreate_textures_callback will, if not NULL, be the function
 * called by the controlling application in response to
 * SDL_RENDER_TARGETS_RESET (all set to false) or SDL_RENDER_DEVICE_RESET
 * (all set to true) events.
 * \param tag sets the tag field of the generated menu so different menus using
 * the same callbacks can be distinguished.
 * \return a pointer to the structure describing the menu.
 */
struct sdlpui_dialog *sdlpui_start_simple_menu(struct sdlpui_dialog *parent,
		struct sdlpui_control *parent_ctrl, int preallocated,
		bool vertical, bool border, void (*pop_callback)(
			struct sdlpui_dialog *d, struct sdlpui_window *w,
			bool up),
		void (*recreate_textures_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, bool all),
		int tag)
{
	struct sdlpui_dialog *result = SDL_malloc(sizeof(*result));
	struct sdlpui_simple_menu *psm = SDL_malloc(sizeof(*psm));

	psm->parent = parent;
	psm->child = NULL;
	psm->parent_ctrl = parent_ctrl;
	if (preallocated > 0) {
		psm->size = preallocated;
		psm->controls = SDL_malloc(psm->size * sizeof(*psm->controls));
		psm->v_ctrls = SDL_malloc(psm->size * sizeof(*psm->v_ctrls));
		psm->control_flags = SDL_malloc(psm->size
			* sizeof(*psm->control_flags));
	} else {
		psm->size = 0;
		psm->controls = NULL;
		psm->v_ctrls = NULL;
		psm->control_flags = NULL;
	}
	psm->number = 0;
	psm->n_vis = 0;
	psm->vertical = vertical;
	psm->border = border;

	result->ftb = &simple_menu_funcs;
	result->pop_callback = pop_callback;
	result->recreate_textures_callback = recreate_textures_callback;
	result->next = NULL;
	result->prev = NULL;
	result->texture = NULL;
	result->c_mouse = NULL;
	result->c_key = NULL;
	result->priv = psm;
	result->type_code = SDLPUI_DIALOG_SIMPLE_MENU;
	result->tag = tag;
	result->pinned = false;
	result->dirty = true;

	return result;
}


/**
 * Get the space for the next control to be added to a simple menu.
 * \param d is the menu to add the control to.  d must be the result of a
 * sdlpui_start_simple_menu() and has not had sdlpui_complete_simple_menu()
 * called for it.
 * \param flags controls how the menu handles the new control.  It can be
 * bitwise-or of one or more of the following:
 *     SDLPUI_MFLG_NONE:  if no other bit is set, the new control has the
 *         default behavior.  It is positioned from the top edge (if the menu
 *         is vertical) or left edge (if the menu is horizontal):  with the
 *         controls added before it being placed between that edge and the
 *         new control.  The new control will also always be visible, provided
 *         that the menu has space for it.
 *    SDLPUI_MFLG_END_GRAVITY:  if this bit is set, the new control is
 *         positioned from the bottom edge (if the menu is vertical) or
 *         right edge (if the menu is vertical):  with the controls added
 *         after if being placed between that edge and the new control.  All
 *         the controls after it must also have the SDLPUI_MFLG_END_GRAVITY
 *         bit set when they are created.
 *    SDLPUI_MFLG_CAN_HIDE:  if this bit is set and the menu is not large
 *         enough to display all of its controls, the menu may hide this
 *         control so there's enough space to display the controls that do
 *         not have the SDLPUI_MFLG_CAN_HIDE bit set.  When there are
 *         multiple controls with the SDLPUI_MFLG_CAN_HIDE bit set, the ones
 *         that are added later will be the ones that are hidden first.
 * \return a pointer to the space for the new control.
 */
struct sdlpui_control* sdlpui_get_simple_menu_next_unused(
		struct sdlpui_dialog *d, int flags)
{
	struct sdlpui_simple_menu *psm;
	int n;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU);
	psm = (struct sdlpui_simple_menu*)d->priv;
	SDL_assert(psm->number >= 0 && psm->number <= psm->size);
	if (psm->number == psm->size) {
		if (psm->size > INT_MAX / 2) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"Too many menu entries");
			sdlpui_force_quit();
		}
		psm->size = (psm->size) ? psm->size + psm->size : 8;
		psm->controls = SDL_realloc(psm->controls,
			psm->size * sizeof(*psm->controls));
		psm->v_ctrls = SDL_realloc(psm->v_ctrls,
			psm->size * sizeof(*psm->v_ctrls));
		psm->control_flags = SDL_realloc(psm->control_flags,
			psm->size * sizeof(*psm->control_flags));
	}
	n = psm->number;
	++psm->number;
	SDL_memset(psm->controls + n, 0, sizeof(*psm->controls));
	psm->control_flags[n] = flags;
	return psm->controls + n;
}


/**
 * Complete the construction of a simple menu.
 *
 * \param d is the menu of interest.
 * \param w is the window containing the menu.
 *
 * Once this function is called for a menu, sdlpui_start_simple_menu() and
 * sdlpui_get_simple_menu_next_unused() must not be called for that menu.
 */
void sdlpui_complete_simple_menu(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	int dw, dh;

	(*d->ftb->query_natural_size)(d, w, &dw, &dh);
	if (d->ftb->resize) {
		(*d->ftb->resize)(d, w, dw, dh);
	} else {
		d->rect.w = dw;
		d->rect.h = dh;
	}
}


/**
 * Begin constructing a simple information dialog.
 *
 * \param button_label is the text label to use for the button that dismisses
 * the dialog.
 * \param pop_callback will, if not NULL, be the function called when the
 * dialog is popped up or down.
 * \param recreate_textures_callback will, if not NULL, be the function
 * called by the controlling application in response to
 * SDL_RENDER_TARGETS_RESET (all set to false) or SDL_RENDER_DEVICE_RESET
 * (all set to true) events.
 * \param tag sets the tag field of the generated dialog so different dialogs
 * using the same callbacks can be distinguished.
 * \return a pointer to the structure describing the dialog.
 */
struct sdlpui_dialog *sdlpui_start_simple_info(const char *button_label,
		void (*pop_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, bool up),
		void (*recreate_textures_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, bool all),
		int tag)
{
	struct sdlpui_dialog *result = SDL_malloc(sizeof(*result));
	struct sdlpui_simple_info *psi = SDL_malloc(sizeof(*psi));

	psi->labels = NULL;
	sdlpui_create_push_button(&psi->button, button_label,
		SDLPUI_HOR_CENTER, sdlpui_invoke_dialog_default_action,
		0, false);
	psi->size = 0;
	psi->number = 0;

	result->ftb = &simple_info_funcs;
	result->pop_callback = pop_callback;
	result->recreate_textures_callback = recreate_textures_callback;
	result->next = NULL;
	result->prev = NULL;
	result->texture = NULL;
	result->c_mouse = NULL;
	result->c_key = NULL;
	result->priv = psi;
	result->type_code = SDLPUI_DIALOG_SIMPLE_INFO;
	result->tag = tag;
	result->pinned = false;
	result->dirty = true;

	return result;
}


/**
 * Add an image to a simple information dialog.
 *
 * \param d is the dialog to which the image will be added.
 * \param image is the texture containing the image to add.  The dialog assumes
 * ownership of the texture and calls SDL_DestroyTexture() on it when the dialog
 * is destroyed.
 * \param halign specifies how to horizontally align the image within the
 * dialog if the dialog is wider than the image.
 * \param top_margin specifies the height, in pixels, of an empty space
 * to leave along the top of the image.
 * \param bottom_margin specifies the height, in pixels, of an empty space
 * to leave along the bottom of the image.
 * \param left_margin specifies the width, in pixels, of an empty space
 * to leave along the left of the image.
 * \param right_margin specifies the width, in pixels, of an empty space
 * to leave along the left of the image.
 *
 * Images and labels added to the dialog are displayed from top to bottom in
 * the dialog in the order they were added.
 */
void sdlpui_simple_info_add_image(struct sdlpui_dialog *d, SDL_Texture *image,
		enum sdlpui_hor_align halign, int top_margin, int bottom_margin,
		int left_margin, int right_margin)
{
	struct sdlpui_simple_info *psi;
	int n;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO);
	psi = (struct sdlpui_simple_info*)d->priv;
	SDL_assert(psi->number >= 0 && psi->number <= psi->size);
	if (psi->number == psi->size) {
		if (psi->size > INT_MAX / 2) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"Too many info dialog entries");
			sdlpui_force_quit();
		}
		psi->size = (psi->size) ? psi->size + psi->size : 8;
		psi->labels = SDL_realloc(psi->labels,
			psi->size * sizeof(*psi->labels));
	}
	n = psi->number;
	++psi->number;
	sdlpui_create_image(&psi->labels[n], image, halign, top_margin,
		bottom_margin, left_margin, right_margin);
}


/**
 * Add a label to a simple information dialog.
 *
 * \param d is the dialog to which the label will be added.
 * \param label is the null-terminated UTF-8 string to use as the label.
 * The contents of label are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how to horizontally align the label within the
 * dialog if the dialog is wider than the label.
 *
 * Images and labels added to the dialog are displayed from top to bottom in
 * the dialog in the order they were added.
 */
void sdlpui_simple_info_add_label(struct sdlpui_dialog *d, const char *label,
		enum sdlpui_hor_align halign)
{
	struct sdlpui_simple_info *psi;
	int n;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO);
	psi = (struct sdlpui_simple_info*)d->priv;
	SDL_assert(psi->number >= 0 && psi->number <= psi->size);
	if (psi->number == psi->size) {
		if (psi->size > INT_MAX / 2) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"Too many info dialog entries");
			sdlpui_force_quit();
		}
		psi->size = (psi->size) ? psi->size + psi->size : 8;
		psi->labels = SDL_realloc(psi->labels,
			psi->size * sizeof(*psi->labels));
	}
	n = psi->number;
	++psi->number;
	sdlpui_create_label(&psi->labels[n], label, halign);
}


/**
 * Complete the construction of a simple information dialog.
 *
 * \param d is the dialog of interest.
 * \param w is the window containing the dialog.
 *
 * Once this function is called for a menu, sdlpui_start_simple_info(),
 * sdlpui_simple_info_add_image(), and sdlpui_simple_info_add_label()
 * must not be called for that menu.
 */
void sdlpui_complete_simple_info(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	int dw, dh;

	(*d->ftb->query_natural_size)(d, w, &dw, &dh);
	if (d->ftb->resize) {
		(*d->ftb->resize)(d, w, dw, dh);
	} else {
		d->rect.w = dw;
		d->rect.h = dh;
	}
}
