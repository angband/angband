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
#include "pui-foc.h"
#include "pui-misc.h"
#include "pui-win.h"
#include <limits.h> /* INT_MAX */

static SDL_bool handle_simple_menu_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e);
static void render_simple_menu(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static struct sdlpui_control* goto_simple_menu_first_control(
		struct sdlpui_dialog *d, struct sdlpui_window *w);
static void step_simple_menu_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *c,
		SDL_bool forward);
static struct sdlpui_control *find_simple_menu_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind);
static struct sdlpui_dialog *get_simple_menu_parent(struct sdlpui_dialog *d);
static int get_simple_menu_child_dialog_count(const struct sdlpui_dialog *d);
static struct sdlpui_dialog *get_simple_menu_child_dialog_by_index(
		struct sdlpui_dialog *d, int ind);
static int add_simple_menu_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child);
static SDL_bool remove_simple_menu_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child);
static struct sdlpui_control *get_simple_menu_parent_ctrl(
		struct sdlpui_dialog *d);
static void resize_simple_menu(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height);
static void query_simple_menu_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static void query_simple_menu_minimum_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static Uint32 reassign_simple_menu_ids(struct sdlpui_dialog *d, Uint32 start);
static void cleanup_simple_menu(struct sdlpui_dialog *d);

static void render_simple_info(struct sdlpui_dialog *d,
		struct sdlpui_window *w);
static struct sdlpui_control* goto_simple_info_first_control(
		struct sdlpui_dialog *d, struct sdlpui_window *w);
static struct sdlpui_control *find_simple_info_control_containing(
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		Sint32 x, Sint32 y, int *comp_ind);
static void resize_simple_info(struct sdlpui_dialog *d, struct sdlpui_window *w,
		int width, int height);
static void query_simple_info_natural_size(struct sdlpui_dialog *d,
		struct sdlpui_window *w, int *width, int *height);
static Uint32 reassign_simple_info_ids(struct sdlpui_dialog *d, Uint32 start);
static void cleanup_simple_info(struct sdlpui_dialog *d);

Uint32 SDLPUI_DIALOG_SIMPLE_MENU = 0;
Uint32 SDLPUI_DIALOG_SIMPLE_INFO = 0;

static const struct sdlpui_dialog_funcs simple_menu_funcs = {
	handle_simple_menu_key,
	sdlpui_dialog_handle_textin,
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
	get_simple_menu_child_dialog_count,
	get_simple_menu_child_dialog_by_index,
	add_simple_menu_child_dialog,
	remove_simple_menu_child_dialog,
	get_simple_menu_parent_ctrl,
	resize_simple_menu,
	query_simple_menu_natural_size,
	query_simple_menu_minimum_size,
	reassign_simple_menu_ids,
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
	NULL,
	NULL,
	resize_simple_info,
	query_simple_info_natural_size,
	NULL,
	reassign_simple_info_ids,
	cleanup_simple_info
};


/**
 * Menus react to some additional keyboard events since the geometry allows
 * for easy interpretations of cursor movements.  Otherwise, they act like
 * simple dialogs.
 */
static SDL_bool handle_simple_menu_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e)
{
	/*
	 * Swap which keys do what depending on the orientation of the menu.
	 * Allow motion commands from either keyset.
	 */
	struct k_dirsyms {
		SDL_Keycode fwd[4], bck[4], nxt[4], prv[4];
	};
	static const struct k_dirsyms vsyms = {
		/* East descends into the menu hierarchy. */
		{ SDLK_RIGHT, SDLK_6, SDLK_KP_6, SDLK_l },
		/* West backs out. */
		{ SDLK_LEFT, SDLK_4, SDLK_KP_4, SDLK_h },
		/* South goes to the next item in this menu. */
		{ SDLK_DOWN, SDLK_2, SDLK_KP_2, SDLK_j },
		/* North goes to the previous item in this menu. */
		{ SDLK_UP, SDLK_8, SDLK_KP_8, SDLK_k },
	};
	static const struct k_dirsyms hsyms = {
		/* South descends. */
		{ SDLK_DOWN, SDLK_2, SDLK_KP_2, SDLK_j },
		/* North backs out. */
		{ SDLK_UP, SDLK_8, SDLK_KP_8, SDLK_k },
		/* East goes to the next item in this menu. */
		{ SDLK_RIGHT, SDLK_6, SDLK_KP_6, SDLK_l },
		/* West goes to the previous item in this menu. */
		{ SDLK_LEFT, SDLK_4, SDLK_KP_4, SDLK_h },
	};
	const struct k_dirsyms *csyms;
	struct sdlpui_control *c;
	struct sdlpui_simple_menu *p;
	SDL_Keymod mods;

	sdlpui_begin_focus_transaction();

	/*
	 * Most of the additional event handling is as a proxy for the menu's
	 * control with keyboard focus so remember what that is.
	 */
	c = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_KEY);

	/* Relay to the control with focus.  if it handles it we're done. */
	if (c && c->ftb->handle_key && (*c->ftb->handle_key)(c, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	csyms = (p->vertical) ? &vsyms : &hsyms;

	mods = sdlpui_get_interesting_keymods();
	if (e->state == SDL_PRESSED) {
		if (mods == KMOD_NONE) {
			if (e->keysym.sym == csyms->fwd[0]
					|| e->keysym.sym == csyms->fwd[1]
					|| e->keysym.sym == csyms->fwd[2]
					|| e->keysym.sym == csyms->fwd[3]) {
				/*
				 * Invoke the default action for a menu entry.
				 * That will descend deeper into the menu
				 * hierarchy if that entry leads to a submenu.
				 * If there is no entry with keyboard focus,
				 * give focus to the first active entry.
				 */
				if (c) {
					if (c->ftb->respond_default) {
						SDLPUI_EVENT_TRACER(
							(*c->ftb->get_type_name)(c),
							c,
							(c->ftb->get_caption)
							? (*c->ftb->get_caption)(c)
							: "(none)",
							"invoking default response");
						(*c->ftb->respond_default)(c,
							d, w,
							SDLPUI_ACTION_HINT_KEY);
					}
				} else if (!sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)
						&& d->ftb->goto_first_control) {
					(void)(*d->ftb->goto_first_control)(d, w);
				}
				sdlpui_end_focus_transaction();
				return SDL_TRUE;
			} else if (e->keysym.sym == csyms->bck[0]
					|| e->keysym.sym == csyms->bck[1]
					|| e->keysym.sym == csyms->bck[2]
					|| e->keysym.sym == csyms->bck[3]) {
				/*
				 * Back out to the previous level of the menu
				 * hierarchy, if any.
				 */
				sdlpui_dialog_give_key_focus_to_parent(d, w);
				sdlpui_end_focus_transaction();
				return SDL_TRUE;
			} else if (e->keysym.sym == csyms->nxt[0]
					|| e->keysym.sym == csyms->nxt[1]
					|| e->keysym.sym == csyms->nxt[2]
					|| e->keysym.sym == csyms->nxt[3]) {
				/*
				 * Go to the next active button in the menu or
				 * wrap around to the first active button in
				 * the menu if already at the end.  If the
				 * menu does not already have key focus, give
				 * it key focus and go to the first active
				 * button.
				 */
				if (!sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)) {
					if (c) {
						if (d->ftb->step_control) {
							(*d->ftb->step_control)(
								d, w, c,
								SDL_TRUE);
						}
					} else if (d->ftb->goto_first_control) {
						(void)(*d->ftb->goto_first_control)(d, w);
					}
				}
				sdlpui_end_focus_transaction();
				return SDL_TRUE;
			} else if (e->keysym.sym == csyms->prv[0]
					|| e->keysym.sym == csyms->prv[1]
					|| e->keysym.sym == csyms->prv[2]
					|| e->keysym.sym == csyms->prv[3]) {
				/*
				 * Go to the previous active button in the
				 * menu or wrap around to the first active
				 * button in the menu if already at the end.
				 * If the menu does not already have key focus,
				 * give it key focus and go to the first
				 * active button.
				 */
				if (!sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)) {
					if (c) {
						if (d->ftb->step_control) {
							(*d->ftb->step_control)(
								d, w, c,
								SDL_FALSE);
						}
					} else if (d->ftb->goto_first_control) {
						(void)(*d->ftb->goto_first_control)(d, w);
					}
				}
				sdlpui_end_focus_transaction();
				return SDL_TRUE;
			}
		}
	}

	sdlpui_end_focus_transaction();

	return sdlpui_dialog_handle_key(d, w, e);
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
}


static struct sdlpui_control* goto_simple_menu_first_control(
		struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	struct sdlpui_simple_menu *p;
	int i = 0;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	while (1) {
		int comp_ind;

		if (i >= p->n_vis) {
			/*
			 * There are no members that can take focus.  Still
			 * give focus to the dialog.
			 */
			sdlpui_begin_focus_transaction();
			(void)sdlpui_change_focus(NULL, 0, d, w,
				SDLPUI_ACTION_HINT_KEY, SDL_FALSE, SDL_TRUE);
			sdlpui_end_focus_transaction();
			return NULL;
		}
		comp_ind = (p->v_ctrls[i]->ftb->get_interactable_component) ?
			(p->v_ctrls[i]->ftb->get_interactable_component)(
				p->v_ctrls[i], SDL_TRUE) : 0;
		if (comp_ind) {
			sdlpui_begin_focus_transaction();
			(void)sdlpui_change_focus(p->v_ctrls[i], comp_ind - 1,
				d, w, SDLPUI_ACTION_HINT_KEY, SDL_FALSE,
				SDL_TRUE);
			sdlpui_end_focus_transaction();
			return p->v_ctrls[i];
		}
		++i;
	}
}


static void step_simple_menu_control(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *c,
		SDL_bool forward)
{
	struct sdlpui_simple_menu *p;
	int comp_ind, istart, itry;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	comp_ind = (c->ftb->step_within)
		? (*c->ftb->step_within)(c, forward) : 0;
	if (comp_ind) {
		sdlpui_begin_focus_transaction();
		(void)sdlpui_change_focus(c, comp_ind - 1, d, w,
				SDLPUI_ACTION_HINT_KEY, SDL_FALSE, SDL_TRUE);
		sdlpui_end_focus_transaction();
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
		if (c->id == p->v_ctrls[istart]->id) {
			break;
		}
		++istart;
	}
	itry = istart;
	while (1) {
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
		comp_ind = (p->v_ctrls[itry]->ftb->get_interactable_component) ?
			(*p->v_ctrls[itry]->ftb->get_interactable_component)(
				p->v_ctrls[itry], forward) : 0;
		if (comp_ind) {
			sdlpui_begin_focus_transaction();
			(void)sdlpui_change_focus(p->v_ctrls[itry],
				comp_ind - 1, d, w, SDLPUI_ACTION_HINT_KEY,
				SDL_FALSE, SDL_TRUE);
			sdlpui_end_focus_transaction();
			break;
		}
		if (itry == istart) {
			/*
			 * Wrapped around without finding a control that can
			 * accept focus.
			 */
			sdlpui_begin_focus_transaction();
			(void)sdlpui_change_focus(NULL, 0, d, w,
				SDLPUI_ACTION_HINT_KEY, SDL_FALSE, SDL_TRUE);
			sdlpui_end_focus_transaction();
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
					|| !(*p->v_ctrls[ilo]->ftb->get_interactable_component)(p->v_ctrls[ilo], SDL_TRUE)) {
				*comp_ind = 0;
				return NULL;
			} else {
				*comp_ind = 0;
			}
			return p->v_ctrls[ilo];
		}
		imid = ilo + (ihi - ilo) / 2;
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
				|| !(*p->v_ctrls[imid]->ftb->get_interactable_component)(p->v_ctrls[imid], SDL_TRUE)) {
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


static int get_simple_menu_child_dialog_count(const struct sdlpui_dialog *d)
{
	const struct sdlpui_simple_menu *p;
	int count = 0, i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	for (i = 0; i < (int)(sizeof(p->children) / sizeof(p->children[0]))
			&& p->children[i]; ++i) {
		++count;
	}
	return count;
}


static struct sdlpui_dialog *get_simple_menu_child_dialog_by_index(
		struct sdlpui_dialog *d, int ind)
{
	const struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	if (ind >= 0 && ind < 3) {
		return p->children[ind];
	}
	return NULL;
}


static int add_simple_menu_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	for (i = 0; i < (int)(sizeof(p->children) / sizeof(p->children[0]));
			++i) {
		if (!p->children[i]) {
			p->children[i] = child;
			return i;
		}
	}
	return -1;
}


static SDL_bool remove_simple_menu_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	for (i = 0; i < (int)(sizeof(p->children) / sizeof(p->children[0]));
			++i) {
		if (!p->children[i]) {
			return SDL_TRUE;
		}
		if (p->children[i]->id == child->id) {
			int j;

			for (j = i; j < (int)(sizeof(p->children)
					/ sizeof(p->children[0])) - 1; ++j) {
				p->children[j] = p->children[j + 1];
			}
			p->children[(sizeof(p->children)
				/ sizeof(p->children[0])) - 1] = NULL;
			return SDL_FALSE;
		}
	}
	return SDL_TRUE;
}


static struct sdlpui_control *get_simple_menu_parent_ctrl(
		struct sdlpui_dialog *d)
{
	struct sdlpui_simple_menu *p;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;
	return p->parent_ctrl;
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
	SDL_bool *vis;
	int i, ivis, first_end, work;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	/*
	 * First pass:  determine which buttons will definitely be visible
	 * and the boundary between the buttons attached to the front and
	 * the buttons attached to the end
	 */
	vis = SDL_calloc(p->number, sizeof(*vis));
	if (!vis) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"could not allocate working space in "
			"resize_simple_menu()");
		sdlpui_force_quit();
	}
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
		vis[i] = SDL_TRUE;
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
		vis[i] = SDL_TRUE;
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


static Uint32 reassign_simple_menu_ids(struct sdlpui_dialog *d, Uint32 start)
{
	struct sdlpui_simple_menu *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_MENU && d->priv);
	p = d->priv;

	if (start > SDL_MAX_UINT32 - 1 - p->number) {
		return 0;
	}
	d->id = start;
	for (i = 0; i < p->number; ++i) {
		p->controls[i].id = start + 1 + i;
	}

	return p->number + 1;
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
}


static struct sdlpui_control* goto_simple_info_first_control(
		struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	struct sdlpui_simple_info *id;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	id = d->priv;
	sdlpui_begin_focus_transaction();
	(void)sdlpui_change_focus(&id->button, 0, d, w, SDLPUI_ACTION_HINT_KEY,
		SDL_FALSE, SDL_TRUE);
	sdlpui_end_focus_transaction();
	return &id->button;
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


static Uint32 reassign_simple_info_ids(struct sdlpui_dialog *d, Uint32 start)
{
	struct sdlpui_simple_info *p;
	int i;

	SDL_assert(d->type_code == SDLPUI_DIALOG_SIMPLE_INFO && d->priv);
	p = d->priv;

	if (start > SDL_MAX_UINT32 - 1 - p->number) {
		return 0;
	}
	d->id = start;
	for (i = 0; i < p->number; ++i) {
		p->labels[i].id = start + 1 + i;
	}

	return p->number + 1;
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
 * Return SDL_TRUE and reset the redraw flag if the given dialog should be
 * redrawn.  Otherwise, return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_should_redraw(struct sdlpui_dialog *d)
{
	return SDL_AtomicCAS(&d->dirty, SDL_TRUE, SDL_FALSE);
}


/**
 * Determine if a given coordinate, relative to the window, is in a dialog.
 *
 * \param d is the dialog of interest.
 * \param x is the horizontal coordinate, relative to the window's upper left
 * corner, to test.
 * \param y is the vertical coordinate, relative to the window's upper left
 * corner, to test.
 * \return SDL_TRUE if (x, y) is in the control and SDL_FALSE otherwise.
 */
SDL_bool sdlpui_is_in_dialog(const struct sdlpui_dialog *d, Sint32 x, Sint32 y)
{
	if (x < d->rect.x || y < d->rect.y || x >= d->rect.x + d->rect.w
			|| y >= d->rect.y + d->rect.h) {
		return SDL_FALSE;
	}
	return SDL_TRUE;
}


/**
 * Determine if the dialog, other, is a descendant of ancestor.
 *
 * \param ancestor is the dialog to test as a possible ancestor of other.
 * \param other is the dialog to test as a possible descendant of ancestor.
 * May be NULL.
 * \return SDL_TRUE if other is a descendant of ancestor; return
 * SDL_FALSE if other is NULL or is not a descendant of ancestor.
 */
SDL_bool sdlpui_is_descendant_dialog(struct sdlpui_dialog *ancestor,
		struct sdlpui_dialog *other)
{
	if (!ancestor || !other) {
		return SDL_FALSE;
	}
	while (1) {
		other = sdlpui_get_dialog_parent(other);
		if (!other) {
			return SDL_FALSE;
		}
		if (other->id == ancestor->id) {
			return SDL_TRUE;
		}
	}
}


/**
 * Return the base of the shortest tree containing both a and b.
 *
 * Returns NULL if no such tree exists or a or b are NULL.
 */
struct sdlpui_dialog *sdlpui_shortest_dialog_tree_with(struct sdlpui_dialog *a,
		struct sdlpui_dialog *b)
{
	if (!a || !b) {
		return NULL;
	}
	while (1) {
		if (a->id == b->id) {
			return b;
		}
		if (sdlpui_is_descendant_dialog(a, b)) {
			return a;
		}
		a = sdlpui_get_dialog_parent(a);
		if (!a) {
			return NULL;
		}
	}
}


/**
 * Mark that the given dialog should be redrawn.
 */
void sdlpui_dialog_mark_for_redraw(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	(void)SDL_AtomicSet(&d->dirty, SDL_TRUE);
	sdlpui_signal_redraw(w);
}


/**
 * Pop up a dialog.
 *
 * \param d is the dialog of interest.
 * \param w is the window containing the dialog.
 * \param give_key_focus causes, if not SDL_FALSE, the dialog to be given key
 * focus when it is popped up.
 */
void sdlpui_popup_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		SDL_bool give_key_focus)
{
	/* Assume it may have been obscured and needs to be redrawn. */
	sdlpui_dialog_mark_for_redraw(d, w);
	if (d->pop_callback) {
		(*d->pop_callback)(d, w, SDL_TRUE);
	}
	sdlpui_dialog_push_to_top(w, d);
	if (give_key_focus) {
		sdlpui_begin_focus_transaction();
		(void)sdlpui_change_focus(NULL, 0, d, w, SDLPUI_ACTION_HINT_KEY,
			SDL_FALSE, SDL_TRUE);
		sdlpui_end_focus_transaction();
	}
}


/**
 * Help sdlpui_popdown_dialog() determine if the given dialog should be popped
 * down.
 *
 * \param d is the dialog to test.
 * \param mouse_d is the dialog with mouse focus or NULL if not preserving
 * mouse focus.
 * \param mouse_c is the control with mouse focus or NULL if not preserving
 * mouse focus.
 * \param key_d is the dialog with key focus or NULL if not preserving key
 * focus.
 * \param key_c is the control with key focus or NULL if not preserving key
 * focus.
 * \return SDL_TRUE if the dialog should be popped.  Otherwise, return
 * SDL_FALSE.
 */
static SDL_bool sdlpui_popdown_dialog_test(struct sdlpui_dialog *d,
		struct sdlpui_dialog *mouse_d, struct sdlpui_control *mouse_c,
		struct sdlpui_dialog *key_d, struct sdlpui_control *key_c)
{
	if (mouse_d) {
		if (mouse_d->id == d->id
				|| sdlpui_is_descendant_dialog(d, mouse_d)) {
			return SDL_FALSE;
		}
		if (mouse_c) {
			struct sdlpui_control *parent_c =
				sdlpui_get_dialog_parent_ctrl(d);

			if (parent_c && parent_c->id == mouse_c->id) {
				return SDL_FALSE;
			}
		}
	}
	if (key_d) {
		if (key_d->id == d->id
				|| sdlpui_is_descendant_dialog(d, key_d)) {
			return SDL_FALSE;
		}
		if (key_c) {
			struct sdlpui_control *parent_c =
				sdlpui_get_dialog_parent_ctrl(d);

			if (parent_c && parent_c->id == key_c->id) {
				return SDL_FALSE;
			}
		}
	}
	return SDL_TRUE;
}


/**
 * Remove the dialog, any of its child dialogs, and, if all_parents is not
 * SDL_FALSE, any of its parents and their children up to the first parent
 * that is pinned.
 *
 * \param d is the dialog to remove.
 * \param all_parents will, if not SDL_FALSE, cause all of the parents to be
 * removed, up to the first parent that is pinned.
 * \param exclude_mouse will, if not SDL_FALSE, protect any dialog, and its
 * ancestors, from being popped down if the dialog's parent control has mouse
 * focus or the dialog or any descendant has mouse focus.
 * \param exclude_key will, if not SDL_FALSE, protect any dialog, and its
 * ancestors, from being popped down if the dialog's parent control has key
 * focus of the dialog or any descendant has key focus.
 * \return SDL_TRUE if d was popped down.  Otherwise, return SDL_FALSE.  The
 * latter is only possible if exclude_mouse or exclude_key is not SDL_FALSE.
 */
SDL_bool sdlpui_popdown_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w,
		SDL_bool all_parents, SDL_bool exclude_mouse,
		SDL_bool exclude_key)
{
	struct sdlpui_dialog *mouse_d = NULL, *key_d = NULL;
	struct sdlpui_control *mouse_c = NULL, *key_c = NULL;
	struct sdlpui_dialog **stack;
	int n_stack, i_stack, max_stack;
	SDL_bool result = SDL_TRUE;

	if (exclude_mouse || exclude_key) {
		sdlpui_begin_focus_transaction();
		if (exclude_mouse) {
			mouse_d = sdlpui_get_dialog_with_focus(
				SDLPUI_ACTION_HINT_MOUSE);
			mouse_c = sdlpui_get_control_with_focus(
				SDLPUI_ACTION_HINT_MOUSE);
		}
		if (exclude_key) {
			key_d = sdlpui_get_dialog_with_focus(
				SDLPUI_ACTION_HINT_KEY);
			key_c = sdlpui_get_control_with_focus(
				SDLPUI_ACTION_HINT_KEY);
		}
	}

	/* Remember which dialogs will be popped down. */
	max_stack = 32;
	stack = SDL_malloc(max_stack * sizeof(*stack));
	if (!stack) {
		sdlpui_force_quit();
	}
	i_stack = 0;
	n_stack = 0;

	/*
	 * If neccessary, add the predecessors of d and any of their children
	 * that are not d, to the stack of dialogs that will be popped down.
	 * They will be popped down after d is.
	 */
	if (all_parents) {
		struct sdlpui_dialog *parent_d = d;

		/* Ascend to the furthest ancestor that could be popped. */
		while (1) {
			struct sdlpui_dialog *next_parent_d =
				sdlpui_get_dialog_parent(parent_d);

			if (!next_parent_d || next_parent_d->pinned
					|| !sdlpui_popdown_dialog_test(
					next_parent_d, mouse_d, mouse_c, key_d,
					key_c)) {
				break;
			}
			parent_d = next_parent_d;
		}

		if (parent_d->id != d->id) {
			SDL_assert(i_stack == n_stack && n_stack < max_stack);
			stack[n_stack] = parent_d;
			++n_stack;

			while (i_stack < n_stack) {
				int i, n = sdlpui_get_child_dialog_count(
					stack[i_stack]);

				if (n > max_stack - n_stack) {
					if (n > max_stack) {
						if (n > INT_MAX - max_stack
								|| (size_t)n
								> SIZE_MAX
								/ sizeof(*stack)
								- max_stack) {
							sdlpui_force_quit();
						}
						max_stack += n;
					} else {
						if (max_stack > INT_MAX / 2
								|| (size_t)max_stack
								> SIZE_MAX
								/ (2 * sizeof(*stack))) {
							sdlpui_force_quit();
						}
						max_stack *= 2;
					}
					stack = SDL_realloc(stack,
						max_stack * sizeof(*stack));
					if (!stack) {
						sdlpui_force_quit();
					}
				}
				for (i = 0; i < n; ++i) {
					stack[n_stack] =
						sdlpui_get_child_dialog_by_index(
						stack[i_stack], i);
					if (stack[n_stack]->id != d->id
							&& sdlpui_popdown_dialog_test(
							stack[n_stack],
							mouse_d, mouse_c,
							key_d, key_c)) {
						++n_stack;
					}
				}
				++i_stack;
			}
		}
	}

	/*
	 * Assemble all the descendants of d in order of increasing distance
	 * from d.
	 */
	SDL_assert(i_stack == n_stack);
	if (!d->pinned && sdlpui_popdown_dialog_test(d, mouse_d, mouse_c, key_d,
			key_c)) {
		if (n_stack == max_stack) {
			if (max_stack > INT_MAX / 2 || (size_t)max_stack
					> SIZE_MAX / (2 * sizeof(*stack))) {
				sdlpui_force_quit();
			}
			max_stack *= 2;
			stack = SDL_realloc(stack, max_stack * sizeof(*stack));
			if (!stack) {
				sdlpui_force_quit();
			}
		}
		stack[n_stack] = d;
		++n_stack;
	} else {
		i_stack = -1;
		result = SDL_FALSE;
	}

	while (i_stack < n_stack) {
		struct sdlpui_dialog *parent_d;
		int i, n;

		if (i_stack == -1) {
			parent_d = d;
			i_stack = n_stack;
		} else {
			parent_d = stack[i_stack];
			++i_stack;
		}
		n = sdlpui_get_child_dialog_count(parent_d);
		if (n > max_stack - n_stack) {
			if (n > max_stack) {
				if (n > INT_MAX - max_stack
						|| (size_t)n > SIZE_MAX
						/ sizeof(*stack) - max_stack) {
					sdlpui_force_quit();
				}
				max_stack += n;
			} else {
				if (max_stack > INT_MAX / 2
						|| (size_t)max_stack > SIZE_MAX
						/ (2 * sizeof(*stack))) {
					sdlpui_force_quit();
				}
				max_stack *= 2;
			}
			stack = SDL_realloc(stack, max_stack * sizeof(*stack));
			if (!stack) {
				sdlpui_force_quit();
			}
		}
		for (i = 0; i < n; ++i) {
			stack[n_stack] = sdlpui_get_child_dialog_by_index(
				parent_d, i);

			if (sdlpui_popdown_dialog_test(stack[n_stack],
					mouse_d, mouse_c, key_d, key_c)) {
				++n_stack;
			}
		}
	}

	/* Pop down the dialogs in reverse order. */
	while (n_stack) {
		struct sdlpui_dialog *parent_d;
		struct sdlpui_control *parent_c;

		--n_stack;
		SDLPUI_EVENT_TRACER("dialog", stack[n_stack], "(not extracted)",
			"popping down");
		parent_d = sdlpui_get_dialog_parent(stack[n_stack]);
		parent_c = sdlpui_get_dialog_parent_ctrl(stack[n_stack]);
		if (stack[n_stack]->pop_callback) {
			(*stack[n_stack]->pop_callback)(stack[n_stack], w,
				SDL_FALSE);
		}
		sdlpui_begin_focus_transaction();
		if (sdlpui_dialog_has_focus(stack[n_stack],
				SDLPUI_ACTION_HINT_MOUSE)) {
			(void)sdlpui_change_focus(NULL, 0, NULL, w,
				SDLPUI_ACTION_HINT_MOUSE, SDL_TRUE, SDL_FALSE);
		} else if (sdlpui_dialog_has_focus(stack[n_stack],
				SDLPUI_ACTION_HINT_KEY)) {
			(void)sdlpui_change_focus(NULL, 0, NULL, w,
				SDLPUI_ACTION_HINT_KEY, SDL_TRUE, SDL_FALSE);
		}
		if (parent_c && parent_c->ftb->lose_child) {
			(*parent_c->ftb->lose_child)(parent_c, stack[n_stack]);
		}
		if (parent_d) {
			if (sdlpui_remove_child_dialog(parent_d,
					stack[n_stack])) {
				SDL_assert(0);
			}

			/*
			 * If the parent control has key or mouse focus, lose
			 * that focus when the child is lost (assumes that the
			 * parent control is part of a nested menu system and
			 * that it pops up its child automatically when it has
			 * focus).
			 */
			if (parent_c && sdlpui_control_has_focus(parent_c,
					SDLPUI_ACTION_HINT_MOUSE)) {
				(void)sdlpui_change_focus(NULL, 0, parent_d, w,
					SDLPUI_ACTION_HINT_MOUSE, SDL_FALSE,
					SDL_TRUE);
			} else if (parent_c && sdlpui_control_has_focus(parent_c,
					SDLPUI_ACTION_HINT_KEY)) {
				(void)sdlpui_change_focus(NULL, 0, parent_d, w,
					SDLPUI_ACTION_HINT_KEY, SDL_FALSE,
					SDL_TRUE);
			}
		}
		sdlpui_end_focus_transaction();
		if (stack[n_stack]->ftb->cleanup) {
			(*stack[n_stack]->ftb->cleanup)(stack[n_stack]);
		}
		sdlpui_dialog_pop(w, stack[n_stack]);
		sdlpui_unregister_dialog(stack[n_stack]);
		SDL_free(stack[n_stack]);
	}

	if (exclude_mouse || exclude_key) {
		sdlpui_end_focus_transaction();
	}

	SDL_free(stack);

	return result;
}


/**
 * Give the keyboard focus for a dialog or menu to its parent.  If there is no
 * parent and the dialog is not pinned, acts like sdlpui_popdown_dialog(d, w,
 * SDL_TRUE, SDL_TRUE, SDL_FALSE).
 *
 * \param d is the dialog that's ceding focus.
 * \param w is the window containing the dialog.
 *
 * Does nothing if keyboard focus is currently locked.  Note that if invoked
 * for a nested menu when there is no parent and the menu or its descendants
 * do not have mouse focus, this will pop down menu and its children (via the
 * lose focus callback of the control in the menu with key focus).
 */
void sdlpui_dialog_give_key_focus_to_parent(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	struct sdlpui_dialog *parent_d;
	struct sdlpui_control *parent_c;

	sdlpui_begin_focus_transaction();
	if (sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)) {
		sdlpui_end_focus_transaction();
		return;
	}
	parent_d = sdlpui_get_dialog_parent(d);
	parent_c = sdlpui_get_dialog_parent_ctrl(d);
	(void)sdlpui_change_focus(parent_c, 0, parent_d, w,
		SDLPUI_ACTION_HINT_KEY, SDL_FALSE, SDL_TRUE);
	sdlpui_end_focus_transaction();
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
 * Return the number of active child dialogs for the given dialog.
 *
 * \param d is the dialog to query.
 * \return the number of active child dialogs.
 */
int sdlpui_get_child_dialog_count(const struct sdlpui_dialog *d)
{
	return (d->ftb->get_child_dialog_count)
		? (*d->ftb->get_child_dialog_count)(d) : 0;
}


/**
 * Return a child dialog for the given dialog.
 *
 * \param d is the dialog to query.
 * \param ind is the zero-based index for the child dialog to return.  If ind
 * is in [0, sdlpui_dialog_get_child_dialog_count(d) - 1], the return value
 * will not be NULL.
 * \return the child dialog at ind.  That will be NULL if the menu/dialog does
 * not have child dialogs or ind is not in [0,
 * sdlpui_dialog_get_child_dialog_count(d) - 1].
 */
struct sdlpui_dialog *sdlpui_get_child_dialog_by_index(struct sdlpui_dialog *d,
		int ind)
{
	if (d->ftb->get_child_dialog_by_index) {
		return (*d->ftb->get_child_dialog_by_index)(d, ind);
	}
	return NULL;
}


/**
 * Add a child dialog to the given dialog.
 *
 * \param d is the dialog to modify.
 * \param child is the dialog to add as a child.  It must not be d itself or
 * have a descendant that is d.
 * \return the index for the added child or -1 if the child could not be added.
 */
int sdlpui_add_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child)
{
	if (d->ftb->add_child_dialog) {
		return (*d->ftb->add_child_dialog)(d, child);
	}
	return -1;
}


/**
 * Remove a child dialog from the given child dialog.
 *
 * \param d is the dialog to modify.
 * \param child is the dialog to remove.
 * \return SDL_FALSE if child was among the children of d.  Otherwise, return
 * SDL_TRUE.
 */
SDL_bool sdlpui_remove_child_dialog(struct sdlpui_dialog *d,
		struct sdlpui_dialog *child)
{
	if (d->ftb->remove_child_dialog) {
		return (*d->ftb->remove_child_dialog)(d, child);
	}
	return SDL_FALSE;
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
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_KeyboardEvent *e)
{
	struct sdlpui_control *c_key;
	SDL_Keymod mods;
	SDL_bool handled;

	sdlpui_begin_focus_transaction();

	/* Relay to the control with focus.  If it handles it, we are done. */
	c_key = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_KEY);
	if (c_key && c_key->ftb->handle_key
			&& (*c_key->ftb->handle_key)(c_key, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	mods = sdlpui_get_interesting_keymods();
	handled = SDL_FALSE;
	switch (e->keysym.sym) {
	case SDLK_ESCAPE:
		if (e->state == SDL_PRESSED && mods == KMOD_NONE
				&& !sdlpui_is_focus_locked(
				SDLPUI_ACTION_HINT_KEY)) {
			/*
			 * Dismiss the dialog or menu; if that is not
			 * possible (it is pinned), lose focus.
			 */
			handled = SDL_TRUE;
			if (!sdlpui_popdown_dialog(d, w, SDL_TRUE, SDL_FALSE,
					SDL_FALSE)) {
				(void)sdlpui_change_focus(NULL, 0, NULL, w,
					SDLPUI_ACTION_HINT_MOUSE, SDL_FALSE,
					SDL_TRUE);
			}
		}
		break;

	case SDLK_RETURN:
		if (e->state == SDL_PRESSED && mods == KMOD_NONE
				&& d->ftb->respond_default) {
			handled = SDL_TRUE;
			SDLPUI_EVENT_TRACER("dialog", d, "(not extracted)",
				"invoking default response");
			(*d->ftb->respond_default)(d, w);
		}
		break;

	case SDLK_TAB:
		if (e->state == SDL_PRESSED
				&& (mods & ~(KMOD_SHIFT | KMOD_CTRL))
				== KMOD_NONE
				&& !sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)) {
			handled = SDL_TRUE;
			if (!c_key) {
				if (d->ftb->goto_first_control) {
					(void)(*d->ftb->goto_first_control)(d, w);
				}
			} else if (d->ftb->step_control) {
				(*d->ftb->step_control)(d, w, c_key,
					(mods & (KMOD_SHIFT)) == 0);
			}
		}
		break;
	}

	sdlpui_end_focus_transaction();

	return handled;
}


/**
 * Perform basic handling of a text input event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_textin(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextInputEvent *e)
{
	struct sdlpui_control *c_key;

	sdlpui_begin_focus_transaction();

	/* Relay to the control with focus.  If it handles it, we are done. */
	c_key = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_KEY);
	if (c_key && c_key->ftb->handle_textin
			&& (*c_key->ftb->handle_textin)(c_key, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	sdlpui_end_focus_transaction();

	/* Do nothing and swallow the event. */
	return SDL_TRUE;
}


/**
 * Perform basic handling of a text editing event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_textedit(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_TextEditingEvent *e)
{
	struct sdlpui_control *c_key;

	sdlpui_begin_focus_transaction();

	/* Relay to the control with focus.  If it handles it, we are done. */
	c_key = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_KEY);
	if (c_key && c_key->ftb->handle_textedit
			&& (*c_key->ftb->handle_textedit)(c_key, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	sdlpui_end_focus_transaction();

	/* Do nothing and swallow the event. */
	return SDL_TRUE;
}


/**
 * Perform basic handling of a mouse button event for a dialog or menu.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog.
 * \param e is the event to handle.
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_mouseclick(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseButtonEvent *e)
{
	struct sdlpui_control *c_mouse;

	sdlpui_begin_focus_transaction();

	/*
	 * If there is no control with focus but the mouse click is in a child,
	 * give that child focus.
	 */
	c_mouse = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_MOUSE);
	if (!c_mouse && sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY_OR_MOUSE)
			&& d->ftb->find_control_containing) {
		int comp_ind;

		c_mouse = (*d->ftb->find_control_containing)(d, w, e->x, e->y,
			&comp_ind);
		if (c_mouse) {
			sdlpui_change_focus(c_mouse, comp_ind, d, w,
				SDLPUI_ACTION_HINT_MOUSE, SDL_FALSE, SDL_TRUE);
		}
	}

	/* Relay to the control with focus.  If it handles it, we are done. */
	if (c_mouse && c_mouse->ftb->handle_mouseclick
			&& (*c_mouse->ftb->handle_mouseclick)(
				c_mouse, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	sdlpui_end_focus_transaction();

	/* Do nothing and swallow the event. */
	return SDL_TRUE;
}


/**
 * Perform basic handling of a mouse motion event for a menu or dialog.
 *
 * \param d is the menu or dialog.
 * \param w is the window containing the menu or dialog.
 * \param e is the event to handle.
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_mousemove(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseMotionEvent *e)
{
	struct sdlpui_control *c, *c_mouse;
	int comp_ind;

	sdlpui_begin_focus_transaction();

	/* Relay to the control with focus.  If it handles it, we are done. */
	c_mouse = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_MOUSE);
	if (c_mouse && c_mouse->ftb->handle_mousemove
			&& (*c_mouse->ftb->handle_mousemove)(
				c_mouse, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	/* If not allowed to change focus, swallow the event. */
	if (sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY_OR_MOUSE)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	/*
	 * See if the mouse has entered another control in the dialog.  If it
	 * has, give focus to that control.
	 */
	c = (d->ftb->find_control_containing) ?
		(*d->ftb->find_control_containing)(d, w, e->x, e->y, &comp_ind) :
		NULL;
	if (c || sdlpui_is_in_dialog(d, e->x, e->y)) {
		(void)sdlpui_change_focus(c, comp_ind, d, w,
			SDLPUI_ACTION_HINT_MOUSE, SDL_FALSE, SDL_TRUE);
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	sdlpui_end_focus_transaction();

	/*
	 * Let the window handle the mouse motion.  For now keep focus though
	 * moving into another dialog could cause it to be lost.
	 */
	return SDL_FALSE;
}


/**
 * Perform basic handling of a mouse wheel event for a dialog or menu.
 *
 * \param d is the dialog or menu.
 * \param w is the window containing the dialog or menu.
 * \param e is the event to handle.
 * \return SDL_TRUE if the event is handled and does not need further
 * processing by the window; otherwise return SDL_FALSE.
 */
SDL_bool sdlpui_dialog_handle_mousewheel(struct sdlpui_dialog *d,
		struct sdlpui_window *w, const struct SDL_MouseWheelEvent *e)
{
	struct sdlpui_control *c_mouse;

	sdlpui_begin_focus_transaction();

	/* Relay to the control with focus.  If it handles it, we're done. */
	c_mouse = sdlpui_get_control_with_focus(SDLPUI_ACTION_HINT_MOUSE);
	if (c_mouse && c_mouse->ftb->handle_mousewheel
			&& (*c_mouse->ftb->handle_mousewheel)(
				c_mouse, d, w, e)) {
		sdlpui_end_focus_transaction();
		return SDL_TRUE;
	}

	sdlpui_end_focus_transaction();

	/* Do nothing and swallow the event. */
	return SDL_TRUE;
}


/**
 * This is a synonym for sdlpui_popdown_dialog(d, w, SDL_TRUE, SDL_FALSE,
 * SDL_FALSE), usable as the respond_default hook for a dialog.
 *
 * \param d is the dialog or menu to pop down.
 * \param w is the window containing the dialog or menu.
 */
void sdlpui_dismiss_dialog(struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	(void)sdlpui_popdown_dialog(d, w, SDL_TRUE, SDL_FALSE, SDL_FALSE);
}


/**
 * Respond to the mouse leaving the containing window.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog.
 *
 * Called from routines in pui-foc.h so do not need calls to
 * sdlpui_begin_focus_transaction() and sdlpui_end_focus_transaction().
 */
void sdlpui_dialog_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	(void)sdlpui_change_focus(NULL, 0, NULL, w, SDLPUI_ACTION_HINT_MOUSE,
		SDL_TRUE, SDL_TRUE);
}


/**
 * Respond to the mouse leaving the containing window for a pulldown/popup menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 *
 * Called from routines in pui-foc.h so do not need calls to
 * sdlpui_begin_focus_transaction() and sdlpui_end_focus_transaction().
 */
void sdlpui_menu_handle_window_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	if (d->pinned) {
		sdlpui_dialog_handle_window_loses_mouse(d, w);
	} else {
		(void)sdlpui_popdown_dialog(d, w, SDL_TRUE, SDL_FALSE,
			SDL_FALSE);
	}
}


/**
 * Respond to the containing window losing key focus.
 *
 * \param d is the dialog.
 * \param w is the window containing the dialog or menu.
 *
 * Called from routines in pui-foc.h so do not need calls to
 * sdlpui_begin_focus_transaction() and sdlpui_end_focus_transaction().
 */
void sdlpui_dialog_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	(void)sdlpui_change_focus(NULL, 0, NULL, w, SDLPUI_ACTION_HINT_KEY,
		SDL_TRUE, SDL_TRUE);
}


/**
 * Respond to the containing window losing key focus for a pulldown/popup menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 *
 * Called from routines in pui-foc.h so do not need calls to
 * sdlpui_begin_focus_transaction() and sdlpui_end_focus_transaction().
 */
void sdlpui_menu_handle_window_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w)
{
	SDL_bool popped = sdlpui_popdown_dialog(d, w, SDL_TRUE, SDL_TRUE,
		SDL_FALSE);

	if (!popped) {
		sdlpui_dialog_handle_window_loses_key(d, w);
	}
}


/**
 * Respond to another dialog or menu taking mouse focus from this dialog.
 *
 * \param d is the dialog.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining mouse focus.  It may be NULL.
 * \param new_ind is the zero-based index of the component of new_c gaining
 * mouse focus.  It is ignored if new_c is NULL.
 * \param new_d is the dialog gaining mouse focus.  It may be NULL.
 */
void sdlpui_dialog_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		int new_ind, struct sdlpui_dialog *new_d)
{
	sdlpui_begin_focus_transaction();
	if (sdlpui_dialog_has_focus(d, SDLPUI_ACTION_HINT_MOUSE)) {
		(void)sdlpui_change_focus(new_c, new_ind, new_d, w,
			SDLPUI_ACTION_HINT_MOUSE, SDL_FALSE, SDL_TRUE);
	}
	sdlpui_end_focus_transaction();
}


/**
 * Help sdlpui_menu_handle_loses_mouse() and sdlpui_menu_handle_loses_key().
 *
 * \param d is the menu losing focus.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining focus.  It may be NULL and must be NULL
 * \param new_ind is the zero-based index of the component of new_c gaining
 * mouse focus.  It is ignored if new_c is NULL.
 * if new_d is NULL.
 * \param new_d is the dialog gaining focus.  It may be NULL.
 * \param ah specifies the type of focus in question it must be
 * SDLPUI_ACTION_HINT_MOUSE or SDLPUI_ACTION_HINT_KEY.
 *
 * Assumes a focus transaction is in effect.
 */
static void sdlpui_help_menu_loses_focus(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		int new_ind, struct sdlpui_dialog *new_d,
		enum sdlpui_action_hint ah)
{
	/*
	 * Find the base of the shortest tree containing d and new_d.  If that
	 * tree does not exist, pop down d.  If it exists and is not rooted at
	 * d, pop down the child of that base that has d as a descendant (when
	 * the base is new_d, may need to stop and not pop anything down if
	 * d is that child or descend one deeper if the child's parent control
	 * is new_c).  Update the focus when done popping down whatever needed
	 * to be popped down.  When popping down, avoid popping down stuff with
	 * mouse focus when what is being lost is key focus.
	 */
	struct sdlpui_dialog *base_d =
		sdlpui_shortest_dialog_tree_with(d, new_d);

	if (base_d) {
		if (base_d->id != d->id) {
			int n = sdlpui_get_child_dialog_count(base_d), i = 0;
			struct sdlpui_dialog *child_d;

			while (1) {
				SDL_assert(i < n);
				child_d = sdlpui_get_child_dialog_by_index(
					base_d, i);
				SDL_assert(child_d
					&& !sdlpui_is_descendant_dialog(child_d,
					new_d));
				if (child_d->id == d->id
						|| sdlpui_is_descendant_dialog(
						child_d, d)) {
					if (new_c) {
						struct sdlpui_control *parent_c
							= sdlpui_get_dialog_parent_ctrl(child_d);

						if (parent_c && parent_c->id
								== new_c->id) {
							if (child_d->id == d->id) {
								break;
							}
							n = sdlpui_get_child_dialog_count(child_d);
							i = 0;
							base_d = child_d;
							continue;
						}
					}
					(void)sdlpui_popdown_dialog(child_d, w,
						SDL_FALSE,
						ah == SDLPUI_ACTION_HINT_KEY,
						SDL_FALSE);
					break;
				}
				++i;
			}
		}
	} else {
		(void)sdlpui_popdown_dialog(d, w, SDL_TRUE,
			ah == SDLPUI_ACTION_HINT_KEY, SDL_FALSE);
	}

	(void)sdlpui_change_focus(new_c, new_ind, new_d, w, ah, SDL_FALSE,
		SDL_TRUE);
}


/**
 * Respond to another dialog or menu taking mouse focus from this pulldown/popup
 * menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining mouse focus.  It may be NULL and must
 * be NULL if new_d is NULL.
 * \param new_ind is the zero-based index of the component of new_c gaining
 * mouse focus.  It is ignored if new_c is NULL.
 * \param new_d is the dialog gaining mouse focus.  It may be NULL.
 */
void sdlpui_menu_handle_loses_mouse(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		int new_ind, struct sdlpui_dialog *new_d)
{
	sdlpui_begin_focus_transaction();
	if (sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY_OR_MOUSE)) {
		sdlpui_end_focus_transaction();
		return;
	}
	sdlpui_help_menu_loses_focus(d, w, new_c, new_ind, new_d,
		SDLPUI_ACTION_HINT_MOUSE);
	sdlpui_end_focus_transaction();
}


/**
 * Respond to another dialog or menu taking key focus from this dialog.
 *
 * \param d is the dialog.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining key focus.  It may be NULL.
 * \param new_ind is the zero-based index of the component of new_c gaining
 * key focus.  It is ignored if new_c is NULL.
 * \param new_d is the dialog gaining key focus.  It may be NULL.
 */
void sdlpui_dialog_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		int new_ind, struct sdlpui_dialog *new_d)
{
	sdlpui_begin_focus_transaction();
	if (sdlpui_dialog_has_focus(d, SDLPUI_ACTION_HINT_KEY)) {
		(void)sdlpui_change_focus(new_c, new_ind, new_d, w,
			SDLPUI_ACTION_HINT_KEY, SDL_FALSE, SDL_TRUE);
	}
	sdlpui_end_focus_transaction();
}


/**
 * Respond to another dialog or menu taking key focus from this popup/pulldown
 * menu.
 *
 * \param d is the menu.
 * \param w is the window containing the menu.
 * \param new_c is the control gaining mouse focus.  It may be NULL and must
 * be NULL if new_d is NULL.
 * \param new_ind is the zero-based index of the component of new_c gaining
 * key focus.  It is ignored if new_c is NULL.
 * \param new_d is the dialog or menu that contains new_c.
 */
void sdlpui_menu_handle_loses_key(struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		int new_ind, struct sdlpui_dialog *new_d)
{
	sdlpui_begin_focus_transaction();
	if (sdlpui_is_focus_locked(SDLPUI_ACTION_HINT_KEY)) {
		sdlpui_end_focus_transaction();
		return;
	}
	sdlpui_help_menu_loses_focus(d, w, new_c, new_ind, new_d,
		SDLPUI_ACTION_HINT_KEY);
	sdlpui_end_focus_transaction();
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
 * \param vertical will, if not SDL_FALSE, causes the controls in the new menu
 * to be layed out in a single column; if SDL_FALSE, it causes the controls to
 * layed out in a single row.
 * \param border will, if not SDL_FALSE, cause a border to be drawn about the
 * menu.
 * \param pop_callback will, if not NULL, be the function called when the
 * menu is popped up or down.
 * \param recreate_textures_callback will, if not NULL, be the function
 * called by the controlling application in response to
 * SDL_RENDER_TARGETS_RESET (all set to SDL_FALSE) or SDL_RENDER_DEVICE_RESET
 * (all not SDL_FALSE) events.
 * \param tag sets the tag field of the generated menu so different menus using
 * the same callbacks can be distinguished.
 * \return a pointer to the structure describing the menu.
 */
struct sdlpui_dialog *sdlpui_start_simple_menu(struct sdlpui_dialog *parent,
		struct sdlpui_control *parent_ctrl, int preallocated,
		SDL_bool vertical, SDL_bool border, void (*pop_callback)(
			struct sdlpui_dialog *d, struct sdlpui_window *w,
			SDL_bool up),
		void (*recreate_textures_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, SDL_bool all),
		int tag)
{
	Uint32 id = sdlpui_reserve_id();
	struct sdlpui_dialog *result;
	struct sdlpui_simple_menu *psm;

	if (!id) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"could not acquire dialog ID in "
			"sdlpui_start_simple_menu()");
		sdlpui_force_quit();
	}
	result = SDL_malloc(sizeof(*result));
	psm = SDL_malloc(sizeof(*psm));
	if (!result || !psm) {
		if (result) {
			SDL_free(result);
		}
		if (psm) {
			SDL_free(psm);
		}
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"out of memory in sdlpui_start_simple_menu()");
		sdlpui_force_quit();
	}

	psm->parent = parent;
	psm->children[0] = NULL;
	psm->children[1] = NULL;
	psm->children[2] = NULL;
	psm->parent_ctrl = parent_ctrl;
	if (preallocated > 0) {
		psm->size = preallocated;
		psm->controls = SDL_malloc(psm->size * sizeof(*psm->controls));
		psm->v_ctrls = SDL_malloc(psm->size * sizeof(*psm->v_ctrls));
		psm->control_flags = SDL_malloc(psm->size
			* sizeof(*psm->control_flags));
		if (!psm->controls || !psm->v_ctrls || !psm->control_flags) {
			psm->size = 0;
			if (psm->controls) {
				SDL_free(psm->controls);
				psm->controls = NULL;
			}
			if (psm->v_ctrls) {
				SDL_free(psm->v_ctrls);
				psm->v_ctrls = NULL;
			}
			if (psm->control_flags) {
				SDL_free(psm->control_flags);
				psm->control_flags = NULL;
			}
		}
	} else {
		psm->size = 0;
		psm->controls = NULL;
		psm->v_ctrls = NULL;
		psm->control_flags = NULL;
	}
	psm->number = 0;
	psm->n_vis = 0;
	psm->vertical = (vertical) ? SDL_TRUE : SDL_FALSE;
	psm->border = (border) ? SDL_TRUE : SDL_FALSE;

	result->ftb = &simple_menu_funcs;
	result->pop_callback = pop_callback;
	result->recreate_textures_callback = recreate_textures_callback;
	result->next = NULL;
	result->prev = NULL;
	result->next_r = NULL;
	result->prev_r = NULL;
	result->texture = NULL;
	result->priv = psm;
	result->id = id;
	result->type_code = SDLPUI_DIALOG_SIMPLE_MENU;
	result->tag = tag;
	result->pinned = SDL_FALSE;
	result->dirty.value = SDL_TRUE;
	sdlpui_register_dialog(result);

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
		if (!psm->controls || !psm->v_ctrls || !psm->control_flags) {
			if (psm->controls) {
				SDL_free(psm->controls);
			}
			if (psm->v_ctrls) {
				SDL_free(psm->v_ctrls);
			}
			if (psm->control_flags) {
				SDL_free(psm->control_flags);
			}
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"out of memory in "
				"sdlpui_get_simple_menu_next_unused()");
			sdlpui_force_quit();
		}
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
 * SDL_RENDER_TARGETS_RESET (all set to SDL_FALSE) or SDL_RENDER_DEVICE_RESET
 * (all not SDL_FALSE) events.
 * \param tag sets the tag field of the generated dialog so different dialogs
 * using the same callbacks can be distinguished.
 * \return a pointer to the structure describing the dialog.
 */
struct sdlpui_dialog *sdlpui_start_simple_info(const char *button_label,
		void (*pop_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, SDL_bool up),
		void (*recreate_textures_callback)(struct sdlpui_dialog *d,
			struct sdlpui_window *w, SDL_bool all),
		int tag)
{
	Uint32 id = sdlpui_reserve_id();
	struct sdlpui_dialog *result;
	struct sdlpui_simple_info *psi;

	if (!id) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"could not acquire dialog ID in "
			"sdlpui_start_simple_info()");
		sdlpui_force_quit();
	}
	result = SDL_malloc(sizeof(*result));
	psi = SDL_malloc(sizeof(*psi));
	if (!result || !psi) {
		if (result) {
			SDL_free(result);
		}
		if (psi) {
			SDL_free(psi);
		}
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"out of memory in sdlpui_start_simple_info()");
		sdlpui_force_quit();
	}

	psi->labels = NULL;
	sdlpui_create_push_button(&psi->button, button_label,
		SDLPUI_HOR_CENTER, sdlpui_invoke_dialog_default_action,
		0, SDL_FALSE);
	psi->size = 0;
	psi->number = 0;

	result->ftb = &simple_info_funcs;
	result->pop_callback = pop_callback;
	result->recreate_textures_callback = recreate_textures_callback;
	result->next = NULL;
	result->prev = NULL;
	result->next_r = NULL;
	result->prev_r = NULL;
	result->texture = NULL;
	result->priv = psi;
	result->id = id;
	result->type_code = SDLPUI_DIALOG_SIMPLE_INFO;
	result->tag = tag;
	result->pinned = SDL_FALSE;
	result->dirty.value = SDL_TRUE;
	sdlpui_register_dialog(result);

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
		if (!psi->labels) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"out of memory in "
				"sdlpui_simple_info_add_image()");
			sdlpui_force_quit();
		}
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
		if (!psi->labels) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
				"out of memory in "
				"sdlpui_simple_info_add_label()");
			sdlpui_force_quit();
		}
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
