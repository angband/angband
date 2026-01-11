/**
 * \file sdl2/pui-ctrl.c
 * \brief Define handlers for simple controls used by the primitive UI toolkit
 * for SDL2.
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

#include "pui-ctrl.h"
#include "pui-dlg.h"
#include "pui-misc.h"


static void render_image(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r);
static void resize_image(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
static void query_image_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height);
static void cleanup_image(struct sdlpui_control *c);

static void change_label_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption);
static void render_label(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r);
static void resize_label(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
static void query_label_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height);
static void cleanup_label(struct sdlpui_control *c);

static void change_pb_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption);
static void render_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r);
static void respond_default_pb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint);
static void gain_key_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
static void lose_key_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
static void gain_mouse_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
static void lose_mouse_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
static void arm_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
static void disarm_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
static int get_pb_interactable_component(struct sdlpui_control *c, bool first);
static void resize_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
static void query_pb_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height);
static bool is_pb_disabled(const struct sdlpui_control *c);
static bool set_pb_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled);
static int get_pb_tag(const struct sdlpui_control *c);
static int set_pb_tag(struct sdlpui_control *c, int new_tag);
static void cleanup_pb(struct sdlpui_control *c);

static bool handle_mb_mousemove(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseMotionEvent *e);
static bool handle_mb_mousewheel(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseWheelEvent *e);
static void change_mb_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption);
static void render_mb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		SDL_Renderer *r);
static void respond_default_mb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint);
static void gain_key_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
static void lose_key_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
static void gain_mouse_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind);
static void lose_mouse_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d);
static void lose_child_mb(struct sdlpui_control *c,
		struct sdlpui_dialog *child);
static void arm_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
static void disarm_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint);
static int get_mb_interactable_component(struct sdlpui_control *c, bool first);
static bool step_within_mb(struct sdlpui_control *c, bool forward);
static int get_mb_interactable_component_at(struct sdlpui_control *c, Sint32 x,
		Sint32 y);
static void resize_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height);
static void query_mb_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height);
static bool is_mb_disabled(const struct sdlpui_control *c);
static bool set_mb_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled);
static int get_mb_tag(const struct sdlpui_control *c);
static int set_mb_tag(struct sdlpui_control *c, int new_tag);
static void cleanup_mb(struct sdlpui_control *c);

Uint32 SDLPUI_CTRL_IMAGE = 0;
Uint32 SDLPUI_CTRL_LABEL = 0;
Uint32 SDLPUI_CTRL_MENU_BUTTON = 0;
Uint32 SDLPUI_CTRL_PUSH_BUTTON = 0;

/** Function table for sdlpui_image */
static const struct sdlpui_control_funcs image_funcs = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	render_image,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	resize_image,
	query_image_natural_size,
	NULL,
	NULL,
	NULL,
	NULL,
	cleanup_image
};

/** Function table for sdlpui_label */
static const struct sdlpui_control_funcs label_funcs = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	change_label_caption,
	render_label,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	resize_label,
	query_label_natural_size,
	NULL,
	NULL,
	NULL,
	NULL,
	cleanup_label
};

/** Function table for sdlpui_push_button */
const struct sdlpui_control_funcs push_button_funcs = {
	sdlpui_control_handle_key,
	NULL,
	NULL,
	sdlpui_control_handle_mouseclick,
	sdlpui_control_handle_mousemove,
	NULL,
	change_pb_caption,
	render_pb,
	respond_default_pb,
	gain_key_pb,
	lose_key_pb,
	gain_mouse_pb,
	lose_mouse_pb,
	NULL,
	arm_pb,
	disarm_pb,
	get_pb_interactable_component,
	NULL,
	NULL,
	resize_pb,
	query_pb_natural_size,
	is_pb_disabled,
	set_pb_disabled,
	get_pb_tag,
	set_pb_tag,
	cleanup_pb
};

/** Function table for sdlpui_menu_button */
static const struct sdlpui_control_funcs menu_button_funcs = {
	sdlpui_control_handle_key,
	NULL,
	NULL,
	sdlpui_control_handle_mouseclick,
	handle_mb_mousemove,
	handle_mb_mousewheel,
	change_mb_caption,
	render_mb,
	respond_default_mb,
	gain_key_mb,
	lose_key_mb,
	gain_mouse_mb,
	lose_mouse_mb,
	lose_child_mb,
	arm_mb,
	disarm_mb,
	get_mb_interactable_component,
	step_within_mb,
	get_mb_interactable_component_at,
	resize_mb,
	query_mb_natural_size,
	is_mb_disabled,
	set_mb_disabled,
	get_mb_tag,
	set_mb_tag,
	cleanup_mb
};


static void render_image(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r)
{
	struct sdlpui_image *ip;

	SDL_assert(c->type_code == SDLPUI_CTRL_IMAGE && c->priv);
	ip = c->priv;
	SDLPUI_RENDER_TRACER("image", c, "(none)", c->rect, ip->image_rect,
		d->texture);

	if (ip->image_rect.w > 0 && ip->image_rect.h > 0) {
		SDL_Rect dst_r = ip->image_rect;

		/* Get the coordinates relative to the dialog. */
		dst_r.x += c->rect.x;
		dst_r.y += c->rect.y;
		if (!d->texture) {
			/*
			 * Rendering directly to the window's buffer, so use
			 * its coordinates.
			 */
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		SDL_RenderCopy(r, ip->image, NULL, &dst_r);
	}
}


static void resize_image(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height)
{
	struct sdlpui_image *ip;
	int tw, th, mhor, mver, nw, nh;

	SDL_assert(c->type_code == SDLPUI_CTRL_IMAGE && c->priv);
	ip = c->priv;

	if (SDL_QueryTexture(ip->image, NULL, NULL, &tw, &th)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_QueryTexture() failed: %s", SDL_GetError());
		sdlpui_force_quit();
	}
	mhor = ip->left_margin + ip->right_margin;
	mver = ip->top_margin + ip->bottom_margin;
	nw = tw + mhor;
	nh = th + mver;

	if (width <= mhor || height <= mver) {
		/* No room to display anything at all. */
		ip->image_rect.x = 0;
		ip->image_rect.y = 0;
		ip->image_rect.w = 0;
		ip->image_rect.h = 0;
		return;
	}
	if (width >= nw && height >= nh) {
		/*
		 * Can display at the native size.  The internal rectangle
		 * has the same dimensions as the image.
		 */
		ip->image_rect.w = tw;
		ip->image_rect.h = th;
	} else {
		/*
		 * Set the internal rectangle dimensions to keep close to the
		 * original aspect ratio.
		 */
		long shor = (width >= nw) ?
			100L :
			(100L * tw + (width - mhor) / 2) / (width - mhor);
		long sver = (height >= nh) ?
			100L :
			(100L * th + (height - mver) / 2) / (height - mver);
		long scale = (shor <= sver) ? shor : sver;

		ip->image_rect.w = (int)((tw * scale + 50L) / 100L);
		ip->image_rect.h = (int)((th * scale + 50L) / 100L);
		if (ip->image_rect.w < 1) {
			ip->image_rect.w = 1;
		}
		if (ip->image_rect.h < 1) {
			ip->image_rect.h = 1;
		}
	}

	/*
	 * In x, honor the alignment specified when created.  In y, center if
	 * the full size of the control exceeds the inner rectangle's height
	 * plus the vertical margins.
	 */
	SDL_assert(width >= ip->image_rect.w + mhor);
	switch (ip->halign) {
	case SDLPUI_HOR_LEFT:
		ip->image_rect.x = ip->left_margin;
		break;

	case SDLPUI_HOR_RIGHT:
		ip->image_rect.x = width - ip->image_rect.w - ip->right_margin;
		break;

	default:
		ip->image_rect.x = (width - ip->image_rect.w - mhor) / 2
			+ ip->left_margin;
		break;
	}
	SDL_assert(height >= ip->image_rect.h + mver);
	ip->image_rect.y = (height - ip->image_rect.h - mver) / 2
		+ ip->top_margin;
	c->rect.w = width;
	c->rect.h = height;
}


static void query_image_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height)
{
	struct sdlpui_image *ip;

	SDL_assert(c->type_code == SDLPUI_CTRL_IMAGE && c->priv);
	ip = c->priv;
	if (SDL_QueryTexture(ip->image, NULL, NULL, width, height)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_QueryTexture() failed: %s", SDL_GetError());
		sdlpui_force_quit();
	}
	*width += ip->left_margin + ip->right_margin;
	*height += ip->top_margin + ip->bottom_margin;
}


static void cleanup_image(struct sdlpui_control *c)
{
	struct sdlpui_image *ip;

	SDL_assert(c->type_code == SDLPUI_CTRL_IMAGE && c->priv);
	ip = c->priv;
	SDL_DestroyTexture(ip->image);
	SDL_free(ip);
}


static void change_label_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption)
{
	struct sdlpui_label *lp;

	SDL_assert(c->type_code == SDLPUI_CTRL_LABEL && c->priv);
	lp = c->priv;
	SDL_free(lp->caption);
	lp->caption = SDL_strdup(new_caption);
	resize_label(c, d, w, c->rect.w, c->rect.h);
	d->dirty = true;
	sdlpui_signal_redraw(w);
}


static void render_label(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r)
{
	const SDL_Color *fg = sdlpui_get_color(w, SDLPUI_COLOR_DIALOG_FG);
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_label *lp;

	SDL_assert(c->type_code == SDLPUI_CTRL_LABEL && c->priv);
	lp = c->priv;
	SDLPUI_RENDER_TRACER("label", c, lp->caption, c->rect,
		lp->caption_rect, d->texture);

	if (lp->caption_rect.w > 0 && lp->caption_rect.h > 0) {
		SDL_Rect dst_r = lp->caption_rect;

		dst_r.x += c->rect.x;
		dst_r.y += c->rect.y;
		if (!d->texture) {
			/*
			 * Rendering directly to the window's buffer, so use
			 * its coordinates.
			 */
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		sdlpui_render_utf8_line(r, font, fg, &dst_r, lp->caption);
	}
}


static void resize_label(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_label *lp;
	int sw, sh, border, nw, nh;

	SDL_assert(c->type_code == SDLPUI_CTRL_LABEL && c->priv);
	lp = c->priv;

	sdlpui_get_utf8_metrics(font, lp->caption, &sw, &sh);
	border = 2 * SDLPUI_DEFAULT_CTRL_BORDER;
	nw = sw + border;
	nh = sh + border;

	if (width <= border || height <= border) {
		/* No room to display anything at all. */
		lp->caption_rect.x = 0;
		lp->caption_rect.y = 0;
		lp->caption_rect.w = 0;
		lp->caption_rect.h = 0;
		return;
	}

	/* If necessary, truncate what will be displaced to the given size. */
	lp->caption_rect.w = (width >= nw) ? sw : width - border;
	lp->caption_rect.h = (height >= nh) ? sh : height - border;

	/*
	 * In x, honor the alignment specified when created.  In y, center if
	 * the full size of the control exceeds the inner rectangle's height
	 * plus the vertical borders.
	 */
	SDL_assert(width >= lp->caption_rect.w + border);
	switch (lp->halign) {
	case SDLPUI_HOR_LEFT:
		lp->caption_rect.x = SDLPUI_DEFAULT_CTRL_BORDER;
		break;

	case SDLPUI_HOR_RIGHT:
		lp->caption_rect.x = width - lp->caption_rect.w
			- SDLPUI_DEFAULT_CTRL_BORDER;
		break;

	default:
		lp->caption_rect.x = (width - lp->caption_rect.w - border) / 2
			+ SDLPUI_DEFAULT_CTRL_BORDER;
		break;
	}
	SDL_assert(height >= lp->caption_rect.h + border);
	lp->caption_rect.y = (height - lp->caption_rect.h - border) / 2
			+ SDLPUI_DEFAULT_CTRL_BORDER;
	c->rect.w = width;
	c->rect.h = height;
}


static void query_label_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_label *lp;

	SDL_assert(c->type_code == SDLPUI_CTRL_LABEL && c->priv);
	lp = c->priv;

	sdlpui_get_utf8_metrics(font, lp->caption, width, height);
	*width += SDLPUI_DEFAULT_CTRL_BORDER * 2;
	*height += SDLPUI_DEFAULT_CTRL_BORDER * 2;
}


static void cleanup_label(struct sdlpui_control *c)
{
	struct sdlpui_label *lp;

	SDL_assert(c->type_code == SDLPUI_CTRL_LABEL && c->priv);
	lp = c->priv;
	SDL_free(lp->caption);
	SDL_free(lp);
}


static void change_pb_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption)
{
	struct sdlpui_label *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	SDL_free(pbp->caption);
	pbp->caption = SDL_strdup(new_caption);
	resize_pb(c, d, w, c->rect.w, c->rect.h);
	d->dirty = true;
	sdlpui_signal_redraw(w);
}


static void render_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, SDL_Renderer *r)
{
	const SDL_Color *fg = sdlpui_get_color(w, SDLPUI_COLOR_DIALOG_FG);
	const SDL_Color *countersink_color = sdlpui_get_color(w,
		SDLPUI_COLOR_COUNTERSINK);
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_push_button *pbp;
	SDL_Rect dst_r;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	SDLPUI_RENDER_TRACER("push button", c, pbp->caption, c->rect,
		pbp->caption_rect, d->texture);

	/*
	 * Always draw a border.  There's one pixel between these to leave
	 * space for indicating whether it's armed or not.
	 */
	SDL_SetRenderDrawColor(r, countersink_color->r, countersink_color->g,
		countersink_color->b, countersink_color->a);
	dst_r = c->rect;
	if (!d->texture) {
		/*
		 * Drawing directly to the window's buffer, use its coordinates.
		 */
		dst_r.x += d->rect.x;
		dst_r.y += d->rect.y;
	}
	SDL_RenderDrawRect(r, &dst_r);
	dst_r.x += 2;
	dst_r.y += 2;
	dst_r.w -= 4;
	dst_r.h -= 4;
	SDL_SetRenderDrawColor(r, fg->r, fg->g, fg->b, fg->a);
	SDL_RenderDrawRect(r, &dst_r);

	if (pbp->has_key || pbp->has_mouse) {
		/* Strengthen the inner border to signal that it has focus. */
		++dst_r.x;
		++dst_r.y;
		dst_r.w -= 2;
		dst_r.h -= 2;
		SDL_RenderDrawRect(r, &dst_r);
	}

	if (!pbp->armed) {
		/*
		 * Button is not depressed.  Highlight left and top edges
		 * as if there's lighting from the upper left since it
		 * protrudes above the surface.
		 */
		SDL_Point points[3];

		points[0].x = c->rect.x + 1;
		points[0].y = c->rect.y + c->rect.h - 2;
		if (!d->texture) {
			points[0].x += d->rect.x;
			points[0].y += d->rect.y;
		}
		points[1].x = points[0].x;
		points[1].y = points[0].y - c->rect.h + 3;
		points[2].x = points[0].x + c->rect.w - 3;
		points[2].y = points[1].y;
		SDL_RenderDrawLines(r, points, 3);
	}

	if (pbp->caption_rect.h > 0 && pbp->caption_rect.w > 0) {
		dst_r = pbp->caption_rect;
		dst_r.x += c->rect.x;
		dst_r.y += c->rect.y;
		if (!d->texture) {
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		sdlpui_render_utf8_line(r, font, fg, &dst_r, pbp->caption);
	}

	if (pbp->disabled) {
		struct sdlpui_stipple *stipple = sdlpui_get_stipple(w);

		dst_r = c->rect;
		if (!d->texture) {
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		dst_r.x += 2;
		dst_r.y += 2;
		dst_r.w -= 4;
		dst_r.h -= 4;
		sdlpui_stipple_rect(r, stipple, &dst_r);
	}
}


static void respond_default_pb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (!pbp->disabled && pbp->callback) {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			"default action invoked");
		(pbp->callback)(c, d, w);
	} else {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			(pbp->disabled) ?
				((pbp->callback) ? "default action suppressed; disable yes, callback available" : "default action suppressed; disable yes, callback unavailable") :
				((pbp->callback) ? "default action suppressed; disable no, callback available" : "default action suppressed; disable no, callback unavailable"));
	}
}


static void gain_key_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(comp_ind == 0 || comp_ind == -1);
	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (!pbp->has_key) {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			"gained key focus");
		pbp->has_key = true;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void lose_key_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (pbp->has_key) {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			"lost key focus");
		pbp->has_key = false;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void gain_mouse_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(comp_ind == 0 || comp_ind == -1);
	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (!pbp->has_mouse) {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			"gained mouse focus");
		pbp->has_mouse = true;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void lose_mouse_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (pbp->has_mouse) {
		SDLPUI_EVENT_TRACER("push button", c, pbp->caption,
			"lost mouse focus");
		pbp->has_mouse = false;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void arm_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (!pbp->armed) {
		pbp->armed = true;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void disarm_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	if (pbp->armed) {
		pbp->armed = false;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static int get_pb_interactable_component(struct sdlpui_control *c, bool first)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	return (pbp->disabled) ? 0 : 1;
}


static void resize_pb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_push_button *pbp;
	int sw, sh, border, nw, nh;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;

	sdlpui_get_utf8_metrics(font, pbp->caption, &sw, &sh);
	border = 2 * (SDLPUI_DEFAULT_CTRL_BORDER + 2);
	nw = sw + border;
	nh = sh + border;

	if (width <= border || height <= border) {
		/* No room to display anything at all. */
		pbp->caption_rect.x = 0;
		pbp->caption_rect.y = 0;
		pbp->caption_rect.w = 0;
		pbp->caption_rect.h = 0;
		return;
	}

	/* If necessary, truncate what will be displaced to the given size. */
	pbp->caption_rect.w = (width >= nw) ? sw : width - border;
	pbp->caption_rect.h = (height >= nh) ? sh : height - border;

	/*
	 * In x, honor the alignment specified when created.  In y, center if
	 * the full size of the control exceeds the inner rectangle's height
	 * plus the vertical borders.
	 */
	SDL_assert(width >= pbp->caption_rect.w + border);
	switch (pbp->halign) {
	case SDLPUI_HOR_LEFT:
		pbp->caption_rect.x = SDLPUI_DEFAULT_CTRL_BORDER + 2;
		break;

	case SDLPUI_HOR_RIGHT:
		pbp->caption_rect.x = width - pbp->caption_rect.w
			- SDLPUI_DEFAULT_CTRL_BORDER - 2;
		break;

	default:
		pbp->caption_rect.x = (width - pbp->caption_rect.w - border) / 2
			+ SDLPUI_DEFAULT_CTRL_BORDER + 2;
		break;
	}
	SDL_assert(height >= pbp->caption_rect.h + border);
	pbp->caption_rect.y = (height - pbp->caption_rect.h - border) / 2
		+ SDLPUI_DEFAULT_CTRL_BORDER + 2;
	c->rect.w = width;
	c->rect.h = height;
}


static void query_pb_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w, int *width,
		int *height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;

	sdlpui_get_utf8_metrics(font, pbp->caption, width, height);
	*width += (SDLPUI_DEFAULT_CTRL_BORDER + 2) * 2;
	*height += (SDLPUI_DEFAULT_CTRL_BORDER + 2) * 2;
}


static bool is_pb_disabled(const struct sdlpui_control *c)
{
	const struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	return pbp->disabled;
}


static bool set_pb_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled)
{
	struct sdlpui_push_button *pbp;
	bool old_value;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	old_value = pbp->disabled;
	if (old_value != disabled) {
		pbp->disabled = disabled;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	return old_value;
}


static int get_pb_tag(const struct sdlpui_control *c)
{
	const struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	return pbp->tag;
}


static int set_pb_tag(struct sdlpui_control *c, int new_tag)
{
	struct sdlpui_push_button *pbp;
	int old_tag;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	old_tag = pbp->tag;
	pbp->tag = new_tag;
	return old_tag;
}


static void cleanup_pb(struct sdlpui_control *c)
{
	struct sdlpui_push_button *pbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_PUSH_BUTTON && c->priv);
	pbp = c->priv;
	SDL_free(pbp->caption);
	SDL_free(pbp);
}


static void help_mb_popup_submenu(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	struct sdlpui_menu_button *mbp;
	int ul_x_win, ul_y_win;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	SDL_assert(mbp->subtype_code == SDLPUI_MB_SUBMENU);

	SDLPUI_EVENT_TRACER("menu submenu button", c, mbp->caption,
		"popping up child menu");

	ul_x_win = d->rect.x + c->rect.x;
	ul_y_win = d->rect.y + c->rect.y;
	switch (mbp->v.submenu.placement) {
	case SDLPUI_CHILD_MENU_ABOVE:
		ul_y_win -= mbp->v.submenu.child->rect.h;
		break;

	case SDLPUI_CHILD_MENU_BELOW:
		ul_y_win += c->rect.h;
		break;

	case SDLPUI_CHILD_MENU_LEFT:
		ul_x_win -= mbp->v.submenu.child->rect.w;
		break;

	case SDLPUI_CHILD_MENU_RIGHT:
		ul_x_win += c->rect.w;
		break;

	default:
		SDL_assert(0);
	}
	mbp->v.submenu.child = (*mbp->v.submenu.creator)(c, d, w, ul_x_win,
		ul_y_win);

	SDL_assert(d->ftb->set_child);
	(*d->ftb->set_child)(d, mbp->v.submenu.child);
	if (mbp->v.submenu.child->pop_callback) {
		(*mbp->v.submenu.child->pop_callback)(mbp->v.submenu.child,
			w, true);
	}
	sdlpui_dialog_push_to_top(w, mbp->v.submenu.child);
}


static bool handle_mb_mousemove(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseMotionEvent *e)
{
	/*
	 * Ignore moution events while a mouse button is pressed (at least up
	 * to the point that the mouse leaves the window).
	 */
	if (e->state != 0) {
		return true;
	}
	if (sdlpui_is_in_control(c, d, e->x, e->y)) {
		/*
		 * ranged_int buttons care whether the mouse is in the left or
		 * right side of the button.  Otherwise, motion within the
		 * button doesn't matter.
		 */
		struct sdlpui_menu_button *mbp;

		SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
		mbp = c->priv;
		if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
			int old = mbp->has_mouse;

			if (e->x <= d->rect.x + c->rect.x + c->rect.w / 2) {
				mbp->has_mouse = (mbp->v.ranged_int.curr
					> mbp->v.ranged_int.min) ? 1 : 0;
			} else {
				mbp->has_mouse = (mbp->v.ranged_int.curr
					< mbp->v.ranged_int.max) ? 2 : 0;
			}
			if (old != mbp->has_mouse) {
				if (!mbp->has_mouse) {
#ifdef SDLPUI_TRACE_EVENTS
					size_t ecap_sz =
						SDL_strlen(mbp->caption) + 16;
					char *ecap = SDL_malloc(ecap_sz);

					(void)SDL_snprintf(ecap, ecap_sz,
						mbp->caption,
						mbp->v.ranged_int.curr);
					SDLPUI_EVENT_TRACER("menu ranged int",
						c, ecap, "lost mouse focus");
					SDL_free(ecap);
#endif
					d->c_mouse = NULL;
				} else if (!old) {
#ifdef SDLPUI_TRACE_EVENTS
					size_t ecap_sz =
						SDL_strlen(mbp->caption) + 16;
					char *ecap = SDL_malloc(ecap_sz);

					(void)SDL_snprintf(ecap, ecap_sz,
						mbp->caption,
						mbp->v.ranged_int.curr);
					SDLPUI_EVENT_TRACER("menu ranged int",
						c, ecap, "gained mouse focus");
					SDL_free(ecap);
#endif
					d->c_mouse = c;
				}
				/* Have keyboard focus follow the mouse. */
				if (mbp->has_key != mbp->has_mouse) {
					if (!mbp->has_mouse) {
#ifdef SDLPUI_TRACE_EVENTS
						size_t ecap_sz =
							SDL_strlen(mbp->caption)
							+ 16;
						char *ecap =
							SDL_malloc(ecap_sz);

						(void)SDL_snprintf(ecap,
							ecap_sz, mbp->caption,
							mbp->v.ranged_int.curr);
						SDLPUI_EVENT_TRACER(
							"menu ranged int",
							c, ecap,
							"lost key focus");
						SDL_free(ecap);
#endif
						d->c_key = NULL;
					} else if (!mbp->has_key) {
#ifdef SDLPUI_TRACE_EVENTS
						size_t ecap_sz =
							SDL_strlen(mbp->caption)
							+ 16;
						char *ecap =
							SDL_malloc(ecap_sz);

						(void)SDL_snprintf(ecap,
							ecap_sz, mbp->caption,
							mbp->v.ranged_int.curr);
						SDLPUI_EVENT_TRACER(
							"menu ranged int",
							c, ecap,
							"gained key focus");
						SDL_free(ecap);
#endif
						d->c_key = c;
					}
					mbp->has_key = mbp->has_mouse;
				}
				d->dirty = true;
				sdlpui_signal_redraw(w);
			}
		}
		return true;
	}
	/*
	 * Otherwise, let the dialog handle the motion to see if it enters
	 * another control.
	 */
	return false;
}


static bool handle_mb_mousewheel(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseWheelEvent *e)
{
	struct sdlpui_menu_button *mbp;
	Sint32 change, result;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	/*
	 * ranged_int buttons allow vertical mouse wheel changes to adjust the
	 * value.  Everything else swallows the mouse wheel event without
	 * doing anything.
	 */
	if (mbp->subtype_code != SDLPUI_MB_RANGED_INT) {
		return true;
	}

	change = e->y;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (e->direction == SDL_MOUSEWHEEL_FLIPPED) {
		change *= -1;
	}
#endif
	/*
	 * Also flip if the left side of the control has focus (so the default
	 * direction of changes is negative).
	 */
	if (mbp->has_mouse == 1) {
		change *= -1;
	}
	result = mbp->v.ranged_int.curr + change;
	if (result < mbp->v.ranged_int.min) {
		result = mbp->v.ranged_int.min;
	} else if (result > mbp->v.ranged_int.max) {
		result = mbp->v.ranged_int.max;
	}
	if (mbp->v.ranged_int.curr != result) {
#ifdef SDLPUI_TRACE_EVENTS
		size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
		char *ecap = SDL_malloc(ecap_sz);

		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.curr);
		SDLPUI_EVENT_TRACER("menu ranged int", c, ecap,
			"value changed by mouse wheel");
		SDL_free(ecap);
#endif
		mbp->v.ranged_int.old = mbp->v.ranged_int.curr;
		mbp->v.ranged_int.curr = (int)result;
		d->dirty = true;
		sdlpui_signal_redraw(w);
		if (mbp->callback) {
			(*mbp->callback)(c, d, w);
		}
	} else {
#ifdef SDLPUI_TRACE_EVENTS
		size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
		char *ecap = SDL_malloc(ecap_sz);

		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.curr);
		SDLPUI_EVENT_TRACER("menu ranged int", c, ecap,
			"value left as is by mouse wheel");
		SDL_free(ecap);
#endif
	}
	return true;
}


static void change_mb_caption(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const char *new_caption)
{
	struct sdlpui_label *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	SDL_free(mbp->caption);
	mbp->caption = SDL_strdup(new_caption);
	resize_mb(c, d, w, c->rect.w, c->rect.h);
	d->dirty = true;
	sdlpui_signal_redraw(w);
}


static void render_mb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		SDL_Renderer *r)
{
	const SDL_Color *fg = sdlpui_get_color(w, SDLPUI_COLOR_MENU_FG);
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_menu_button *mbp;
	struct SDL_Rect dst_r;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	SDLPUI_RENDER_TRACER("menu button", c, mbp->caption, c->rect,
		mbp->caption_rect, d->texture);

	SDL_SetRenderDrawColor(r, fg->r, fg->g, fg->b, fg->a);

	if (mbp->caption_rect.h > 0 && mbp->caption_rect.w > 0) {
		dst_r = mbp->caption_rect;
		dst_r.x += c->rect.x;
		dst_r.y += c->rect.y;
		if (!d->texture) {
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		if (mbp->subtype_code != SDLPUI_MB_RANGED_INT) {
			sdlpui_render_utf8_line(r, font, fg, &dst_r,
				mbp->caption);
		} else {
			/* Fill in the current value in the caption. */
			size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
			char *ecap = SDL_malloc(ecap_sz);

			(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
				mbp->v.ranged_int.curr);
			sdlpui_render_utf8_line(r, font, fg, &dst_r, ecap);
			SDL_free(ecap);
		}

		if ((mbp->subtype_code == SDLPUI_MB_TOGGLE
				|| mbp->subtype_code == SDLPUI_MB_INDICATOR)
				&& mbp->caption_rect.h > 4
				&& c->rect.w > mbp->caption_rect.x
					+ mbp->caption_rect.w
					+ SDLPUI_DEFAULT_CTRL_BORDER
					+ mbp->caption_rect.h) {
			/*
			 * Add an indicator for the toggled state of this
			 * control.  Use a square drawn on the right side of the
			 * control with a side length set by the height of the
			 * caption minus 4.  Fill the square if the toggled
			 * state is on.
			 */
			dst_r.x = c->rect.x + c->rect.w
				- SDLPUI_DEFAULT_CTRL_BORDER
				- (mbp->caption_rect.h - 2);
			dst_r.y = c->rect.y
				+ (c->rect.h - mbp->caption_rect.h + 4) / 2;
			dst_r.w = mbp->caption_rect.h - 4;
			dst_r.h = dst_r.w;

			if (!d->texture) {
				dst_r.x += d->rect.x;
				dst_r.y += d->rect.y;
			}
			if (mbp->v.toggled) {
				SDL_RenderFillRect(r, &dst_r);
			} else {
				SDL_RenderDrawRect(r, &dst_r);
			}
		}
	}

	if (mbp->has_key || mbp->has_mouse) {
		/*
		 * Put a border on it to signal that it has focus.  The border
		 * leaves space for the highlighting that happens if armed.
		 */
		if (mbp->subtype_code != SDLPUI_MB_RANGED_INT
				|| (mbp->has_key && mbp->has_mouse
				&& mbp->has_key != mbp->has_mouse)) {
			dst_r = c->rect;
			if (!d->texture) {
				/*
				 * Rendering directly to the window's buffer
				 * so use its coordinates.
				 */
				dst_r.x += d->rect.x;
				dst_r.y += d->rect.y;
			}
			--dst_r.w;
			--dst_r.h;
			SDL_RenderDrawRect(r, &dst_r);
		} else {
			/*
			 * Only the left or right side has focus.  Don't draw
			 * the line that would split the button in two.
			 */
			SDL_Point points[4];

			if (mbp->has_key == 1 || mbp->has_mouse == 1) {
				points[0].x = c->rect.x + c->rect.w / 2;
				points[0].y = c->rect.y;
				if (!d->texture) {
					points[0].x += d->rect.x;
					points[0].y += d->rect.y;
				}
				points[1].x = points[0].x - c->rect.w / 2;
				points[1].y = points[0].y;
			} else {
				points[0].x = c->rect.x + c->rect.w / 2 + 1;
				points[0].y = c->rect.y;
				if (!d->texture) {
					points[0].x += d->rect.x;
					points[0].y += d->rect.y;
				}
				points[1].x = points[0].x +
					(c->rect.w + 1) / 2 - 2;
				points[1].y = points[0].y;
			}
			points[2].x = points[1].x;
			points[2].y = points[1].y + c->rect.h - 1;
			points[3].x = points[0].x;
			points[3].y = points[2].y;
			SDL_RenderDrawLines(r, points, 4);
		}
	}

	if (mbp->armed) {
		/*
		 * Button is depressed.  Was flat with the surface; now
		 * highlight right and bottom edges as if there's lighting from
		 * the upper left.
		 */
		SDL_Point points[3];
		int npt;

		if (mbp->subtype_code != SDLPUI_MB_RANGED_INT
				|| mbp->armed == 3) {
			points[0].x = c->rect.x;
			points[0].y = c->rect.y + c->rect.h - 1;
			if (!d->texture) {
				points[0].x += d->rect.x;
				points[0].y += d->rect.y;
			}
			points[1].x = points[0].x + c->rect.w - 1;
			points[1].y = points[0].y;
			points[2].x = points[1].x;
			points[2].y = points[1].y - c->rect.h + 1;
			npt = 3;
		} else if (mbp->armed == 1) {
			/* Only the left side is depressed. */
			points[0].x = c->rect.x;
			points[0].y = c->rect.y + c->rect.h - 1;
			if (!d->texture) {
				points[0].x += d->rect.x;
				points[0].y += d->rect.y;
			}
			points[1].x = points[0].x + c->rect.w / 2;
			points[1].y = points[0].y;
			npt = 2;
		} else {
			SDL_assert(mbp->armed == 2);
			/* Only the right side is depressed. */
			points[0].x = c->rect.x + c->rect.w / 2 + 1;
			points[0].y = c->rect.y + c->rect.h - 1;
			if (!d->texture) {
				points[0].x += d->rect.x;
				points[0].y += d->rect.y;
			}
			points[1].x = points[0].x + (c->rect.w + 1) / 2 - 2;
			points[1].y = points[0].y;
			points[2].x = points[1].x;
			points[2].y = points[1].y - c->rect.h + 1;
			npt = 3;
		}
		SDL_RenderDrawLines(r, points, npt);
	}

	if (mbp->disabled) {
		struct sdlpui_stipple *stipple = sdlpui_get_stipple(w);

		dst_r = c->rect;
		if (!d->texture) {
			dst_r.x += d->rect.x;
			dst_r.y += d->rect.y;
		}
		sdlpui_stipple_rect(r, stipple, &dst_r);
	}
}


static void respond_default_mb(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		enum sdlpui_action_hint hint)
{
	struct sdlpui_menu_button *mbp;
	int inci, newi;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->disabled) {
		return;
	}

	switch (mbp->subtype_code) {
	case SDLPUI_MB_SUBMENU:
		if (!mbp->v.submenu.child) {
			struct sdlpui_dialog *other_child =
				sdlpui_get_dialog_child(d);

			if (other_child) {
				sdlpui_popdown_dialog(other_child, w, false);
			}
			help_mb_popup_submenu(c, d, w);
		} else {
			SDLPUI_EVENT_TRACER("menu submenu button", c,
				mbp->caption, "child menu already displayed");
		}
		/* Give the first button in the child menu keyboard focus. */
		if (mbp->v.submenu.child->ftb->goto_first_control) {
			(*mbp->v.submenu.child->ftb->goto_first_control)(
				mbp->v.submenu.child, w);
		}
		break;

	case SDLPUI_MB_RANGED_INT:
		if (hint == SDLPUI_ACTION_HINT_KEY
				|| mbp->has_key == mbp->has_mouse) {
			inci = (mbp->has_key == 1) ?
				-1 : ((mbp->has_key == 2) ? 1 : 0);
		} else if (hint == SDLPUI_ACTION_HINT_MOUSE
				|| mbp->has_key == 0) {
			inci = (mbp->has_mouse == 1) ?
				-1 : ((mbp->has_mouse == 2) ? 1 : 0);
		} else if (mbp->has_mouse == 0) {
			inci = (mbp->has_key == 1) ?
				-1 : ((mbp->has_key == 2) ? 1 : 0);
		} else {
			/* It's not clear what should be done, so do nothing. */
			inci = 0;
		}
		newi = mbp->v.ranged_int.curr + inci;
		if (newi < mbp->v.ranged_int.min) {
			newi = mbp->v.ranged_int.min;
		} else if (newi > mbp->v.ranged_int.max) {
			newi = mbp->v.ranged_int.max;
		}
		if (newi == mbp->v.ranged_int.curr) {
#ifdef SDLPUI_TRACE_EVENTS
			size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
			char *ecap = SDL_malloc(ecap_sz);

			(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
				mbp->v.ranged_int.curr);
			SDLPUI_EVENT_TRACER("menu ranged int", c, ecap,
				"left unchanged by default response");
			SDL_free(ecap);
#endif
			return;
		}
#ifdef SDLPUI_TRACE_EVENTS
		{
			size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
			char *ecap = SDL_malloc(ecap_sz);

			(void)SDL_snprintf(ecap, ecap_sz, mbp->caption, newi);
			SDLPUI_EVENT_TRACER("menu ranged int", c, ecap,
				"changed by default response");
			SDL_free(ecap);
		}
#endif
		mbp->v.ranged_int.old = mbp->v.ranged_int.curr;
		mbp->v.ranged_int.curr = newi;
		d->dirty = true;
		sdlpui_signal_redraw(w);
		break;

	case SDLPUI_MB_TOGGLE:
		SDLPUI_EVENT_TRACER("menu toggle", c, mbp->caption,
			"changed by default response");
		mbp->v.toggled = !mbp->v.toggled;
		d->dirty = true;
		sdlpui_signal_redraw(w);
		break;

	default:
		break;
	}

	if (mbp->callback) {
		(*mbp->callback)(c, d, w);
	}
}


static void gain_key_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind)
{
	struct sdlpui_menu_button *mbp;
	int old;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption,
		"gained key focus");
	old = mbp->has_key;
	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		if (comp_ind < 0) {
			comp_ind += 2;
		}
		SDL_assert(comp_ind == 0 || comp_ind == 1);
		mbp->has_key = comp_ind + 1;
	} else if (mbp->subtype_code == SDLPUI_MB_INDICATOR) {
		SDL_assert(0);
	} else {
		SDL_assert(comp_ind == 0 || comp_ind == -1);
		mbp->has_key = 1;
	}
	if (old != mbp->has_key) {
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	if (mbp->subtype_code == SDLPUI_MB_SUBMENU && !mbp->v.submenu.child) {
		help_mb_popup_submenu(c, d, w);
	}
}


static void lose_key_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->has_key) {
		SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption,
			"lost key focus");
		mbp->has_key = 0;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	/*
	 * Pop down the child dialog if what is gaining focus is not a
	 * descendant of it.
	 */
	if (mbp->subtype_code == SDLPUI_MB_SUBMENU && mbp->v.submenu.child
			&& (!new_d || !sdlpui_is_descendant_dialog(d, new_d))) {
		SDLPUI_EVENT_TRACER("submenu entry", c, mbp->caption,
			"popping down submenu");
		sdlpui_popdown_dialog(mbp->v.submenu.child, w, false);
		mbp->v.submenu.child = NULL;
	}
}


static void gain_mouse_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int comp_ind)
{
	struct sdlpui_menu_button *mbp;
	int old;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption,
		"gained mouse focus");
	old = mbp->has_mouse;
	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		if (comp_ind < 0) {
			comp_ind += 2;
		}
		SDL_assert(comp_ind == 0 || comp_ind == 1);
		mbp->has_mouse = comp_ind + 1;
	} else if (mbp->subtype_code == SDLPUI_MB_INDICATOR) {
		SDL_assert(0);
	} else {
		SDL_assert(comp_ind == 0 || comp_ind == -1);
		mbp->has_mouse = 1;
	}
	if (old != mbp->has_mouse) {
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	if (mbp->subtype_code == SDLPUI_MB_SUBMENU && !mbp->v.submenu.child) {
		help_mb_popup_submenu(c, d, w);
	}
}


static void lose_mouse_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, struct sdlpui_control *new_c,
		struct sdlpui_dialog *new_d)

{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->has_mouse) {
		SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption,
			"lost mouse focus");
		mbp->has_mouse = 0;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	/*
	 * Pop down the child dialog if what is gaining focus is not a
	 * descendant of it.
	 */
	if (mbp->subtype_code == SDLPUI_MB_SUBMENU && mbp->v.submenu.child
			&& (!new_d || !sdlpui_is_descendant_dialog(d, new_d))) {
		SDLPUI_EVENT_TRACER("submenu entry", c, mbp->caption,
			"popping down submenu");
		sdlpui_popdown_dialog(mbp->v.submenu.child, w, false);
		mbp->v.submenu.child = NULL;
	}
}


static void lose_child_mb(struct sdlpui_control *c, struct sdlpui_dialog *child)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	SDL_assert(mbp->subtype_code == SDLPUI_MB_SUBMENU);

	if (mbp->v.submenu.child) {
		SDL_assert(mbp->v.submenu.child == child);
		mbp->v.submenu.child = NULL;
	}
}


static void arm_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint)
{
	struct sdlpui_menu_button *mbp;
	int old;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption, "arming");
	old = mbp->armed;
	if (mbp->subtype_code != SDLPUI_MB_RANGED_INT) {
		mbp->armed = 1;
	} else {
		if (hint == SDLPUI_ACTION_HINT_KEY
				|| hint == SDLPUI_ACTION_HINT_NONE) {
			mbp->armed |= mbp->has_key;
		}
		if (hint == SDLPUI_ACTION_HINT_MOUSE
				|| hint == SDLPUI_ACTION_HINT_NONE) {
			mbp->armed |= mbp->has_mouse;
		}
	}
	if (old != mbp->armed) {
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static void disarm_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, enum sdlpui_action_hint hint)
{
	struct sdlpui_menu_button *mbp;
	int old;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	SDLPUI_EVENT_TRACER("menu entry", c, mbp->caption, "disarming");
	old = mbp->armed;
	if (mbp->subtype_code != SDLPUI_MB_RANGED_INT
			|| hint == SDLPUI_ACTION_HINT_NONE) {
		mbp->armed = 0;
	} else {
		if (hint == SDLPUI_ACTION_HINT_KEY) {
			mbp->armed &= ~mbp->has_key;
		} else if (hint == SDLPUI_ACTION_HINT_MOUSE) {
			mbp->armed &= ~mbp->has_mouse;
		}
	}
	if (old != mbp->armed) {
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
}


static int get_mb_interactable_component(struct sdlpui_control *c, bool first)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->disabled || mbp->subtype_code == SDLPUI_MB_INDICATOR) {
		return 0;
	}
	if (mbp->subtype_code != SDLPUI_MB_RANGED_INT) {
		return 1;
	}
	if (first) {
		if (mbp->v.ranged_int.curr > mbp->v.ranged_int.min) {
			return 1;
		} else if (mbp->v.ranged_int.curr < mbp->v.ranged_int.max) {
			return 2;
		}
	} else {
		if (mbp->v.ranged_int.curr < mbp->v.ranged_int.max) {
			return 2;
		} else if (mbp->v.ranged_int.curr > mbp->v.ranged_int.min) {
			return 1;
		}
	}
	return 0;
}


static bool step_within_mb(struct sdlpui_control *c, bool forward)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	SDL_assert(mbp->has_key);
	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		if (forward) {
			if (mbp->has_key == 1 && mbp->v.ranged_int.curr
					< mbp->v.ranged_int.max) {
				mbp->has_key = 2;
				return true;
			}
		} else {
			if (mbp->has_key == 2 && mbp->v.ranged_int.curr
					> mbp->v.ranged_int.min) {
				mbp->has_key = 1;
				return true;
			}
		}
	}
	return false;
}


static int get_mb_interactable_component_at(struct sdlpui_control *c, Sint32 x,
		Sint32 y)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->disabled || mbp->subtype_code == SDLPUI_MB_INDICATOR
			|| x < c->rect.x || x >= c->rect.x + c->rect.w
			|| y < c->rect.y || y >= c->rect.y + c->rect.h) {
		return 0;
	}
	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		if (x <= c->rect.x + c->rect.w / 2) {
			return (mbp->v.ranged_int.curr > mbp->v.ranged_int.min) ?
				1 : 0;
		}
		return (mbp->v.ranged_int.curr < mbp->v.ranged_int.max) ? 2 : 0;
	}
	return 1;
}


static void resize_mb(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, int width, int height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_menu_button *mbp;
	int sw, sh, tw, border, nw, nh;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
		char *ecap = SDL_malloc(ecap_sz);
		int wtmp, htmp;

		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.min);
		sdlpui_get_utf8_metrics(font, ecap, &sw, &sh);
		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.max);
		sdlpui_get_utf8_metrics(font, ecap, &wtmp, &htmp);
		SDL_free(ecap);
		if (sw < wtmp) {
			sw = wtmp;
		}
		if (sh < htmp) {
			sh = htmp;
		}
	} else {
		sdlpui_get_utf8_metrics(font, mbp->caption, &sw, &sh);
	}
	if (mbp->subtype_code == SDLPUI_MB_TOGGLE
			|| mbp->subtype_code == SDLPUI_MB_INDICATOR) {
		tw = sh;
	} else {
		tw = 0;
	}
	border = 2 * SDLPUI_DEFAULT_CTRL_BORDER;
	nw = sw + tw + border;
	nh = sh + border;

	if (width <= tw + border || height <= border) {
		/* No room to display anything at all. */
		mbp->caption_rect.x = 0;
		mbp->caption_rect.y = 0;
		mbp->caption_rect.w = 0;
		mbp->caption_rect.h = 0;
		return;
	}

	/*
	 * If necessary, truncate what will be displaced to the given size.
	 */
	mbp->caption_rect.h = (height >= nh) ? sh : height - border;
	if (width >= nw) {
		mbp->caption_rect.w = sw;
	} else {
		mbp->caption_rect.w = width - border - tw;
	}

	/*
	 * In x, honor the alignment specified when created.  In y, center
	 * if the full size of the control exceeds the inner rectangle's height
	 * plus the vertical borders.
	 */
	SDL_assert(width >= mbp->caption_rect.w + tw + border);
	switch(mbp->halign) {
	case SDLPUI_HOR_LEFT:
		mbp->caption_rect.x = SDLPUI_DEFAULT_CTRL_BORDER;
		break;

	case SDLPUI_HOR_RIGHT:
		mbp->caption_rect.x = width - mbp->caption_rect.w - tw
			- SDLPUI_DEFAULT_CTRL_BORDER;
		break;

	default:
		mbp->caption_rect.x =
			(width - mbp->caption_rect.w - tw - border) / 2
			+ SDLPUI_DEFAULT_CTRL_BORDER;
		break;
	}
	SDL_assert(height >= mbp->caption_rect.h + border);
	mbp->caption_rect.y = (height - mbp->caption_rect.h - border) / 2
		+ SDLPUI_DEFAULT_CTRL_BORDER;
	c->rect.w = width;
	c->rect.h = height;
}


static void query_mb_natural_size(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		int *width, int *height)
{
	TTF_Font *font = sdlpui_get_ttf(w);
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;

	if (mbp->subtype_code == SDLPUI_MB_RANGED_INT) {
		size_t ecap_sz = SDL_strlen(mbp->caption) + 16;
		char *ecap = SDL_malloc(ecap_sz);
		int wtmp, htmp;

		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.min);
		sdlpui_get_utf8_metrics(font, ecap, width, height);
		(void)SDL_snprintf(ecap, ecap_sz, mbp->caption,
			mbp->v.ranged_int.max);
		sdlpui_get_utf8_metrics(font, ecap, &wtmp, &htmp);
		SDL_free(ecap);
		if (*width < wtmp) {
			*width = wtmp;
		}
		if (*height < htmp) {
			*height = htmp;
		}
	} else {
		sdlpui_get_utf8_metrics(font, mbp->caption, width, height);
	}
	if (mbp->subtype_code == SDLPUI_MB_TOGGLE
			|| mbp->subtype_code == SDLPUI_MB_INDICATOR) {
		*width += (*height) * 3;
	}
	*width += SDLPUI_DEFAULT_CTRL_BORDER * 2;
	*height += SDLPUI_DEFAULT_CTRL_BORDER * 2;
}


static bool is_mb_disabled(const struct sdlpui_control *c)
{
	const struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	return mbp->disabled;
}


static bool set_mb_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled)
{
	struct sdlpui_menu_button *mbp;
	bool old_value;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	old_value = mbp->disabled;
	if (old_value != disabled) {
		mbp->disabled = disabled;
		d->dirty = true;
		sdlpui_signal_redraw(w);
	}
	return old_value;
}


static int get_mb_tag(const struct sdlpui_control *c)
{
	const struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	return mbp->tag;
}


static int set_mb_tag(struct sdlpui_control *c, int new_tag)
{
	struct sdlpui_menu_button *mbp;
	int old_tag;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	old_tag = mbp->tag;
	mbp->tag = new_tag;
	return old_tag;
}


static void cleanup_mb(struct sdlpui_control *c)
{
	struct sdlpui_menu_button *mbp;

	SDL_assert(c->type_code == SDLPUI_CTRL_MENU_BUTTON && c->priv);
	mbp = c->priv;
	SDL_free(mbp->caption);
	SDL_free(mbp);
}


/**
 * Determine if a given coordinate, relative to the window, is in a control.
 *
 * \param c is the control of interest.
 * \param d is the dialog that holds the control.
 * \param x is the horizontal coordinate, relative to the window's upper left
 * corner, to test.
 * \param y is the vertical coordinate, relative to the window's upper left
 * corner, to test.
 * \return true if (x, y) is in the control and false otherwise.
 */
bool sdlpui_is_in_control(const struct sdlpui_control *c,
		const struct sdlpui_dialog *d, Sint32 x, Sint32 y)
{
	if (x < d->rect.x + c->rect.x || y < d->rect.y + c->rect.y
			|| x >= d->rect.x + c->rect.x + c->rect.w
			|| y >= d->rect.y + c->rect.y + c->rect.h) {
		return false;
	}
	return true;
}


/**
 * Return whether a control is disabled.
 *
 * \param c is the control to query.
 * \return whether or not the control is disabled.  If the control does not
 * support being disabled/enabled, the return value will be false.
 */
bool sdlpui_is_disabled(const struct sdlpui_control *c)
{
	return (c->ftb->is_disabled) ? (*c->ftb->is_disabled)(c) : false;
}


/**
 * Change whether a control is disabled or not.
 *
 * \param c is the control to modify.
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 * \param disabled is whether the control should be disabled or not.
 * \return whether or not the prior state of the control was disabled.  If
 * the control does not support changing the disabled/enabled state, the
 * return value will be false.
 */
bool sdlpui_set_disabled(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, bool disabled)
{
	return (c->ftb->set_disabled) ?
		(*c->ftb->set_disabled)(c, d, w, disabled) : false;
}


/**
 * Get the application-specified tag on a control, if it has one.
 *
 * \param c is the control to query.
 * \return the tag's value.  If the control does not have a tag, the
 * returned value will be zero.
 */
int sdlpui_get_tag(const struct sdlpui_control *c)
{
	return (c->ftb->get_tag) ? (*c->ftb->get_tag)(c) : 0;
}


/**
 * Set the application-specified tag on a control.
 *
 * \param c is the control to modify.
 * \param new_tag is new value for the tag.
 * \return the old value for the tag.  If the control does not have a tag,
 * the returned value will be zero and the new tag will not be applied.
 */
int sdlpui_set_tag(struct sdlpui_control *c, int new_tag)
{
	return (c->ftb->set_tag) ? (*c->ftb->set_tag)(c, new_tag) : 0;
}


/**
 * Change the caption for a control.
 *
 * \param c is the control to modify.
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 * \param new_caption is the new caption for the control.
 *
 * If the control does not support changing the caption, has no effect.
 * The internals of the control are adjusted for the new caption, but the
 * external dimensions of the control remain unchanged.
 */
void sdlpui_change_caption(struct sdlpui_control *c, struct sdlpui_dialog *d,
		struct sdlpui_window *w, const char *new_caption)
{
	if (c->ftb->change_caption) {
		(*c->ftb->change_caption)(c, d, w, new_caption);
	}
}


/**
 * Handle a keyboard event for a simple control.
 *
 * \param c is the control receiving the event.  The events handled here
 * are appropriate if c is simple:  not a compound control and only having
 * at most one action (the default one; with the associated arming and
 * disarming).
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the dialog; otherwise return false.
 */
bool sdlpui_control_handle_key(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_KeyboardEvent *e)
{
	if (e->keysym.sym == SDLK_RETURN) {
		SDL_Keymod mods = sdlpui_get_interesting_keymods();

		if (e->state == SDL_PRESSED) {
			if (mods == KMOD_NONE && c->ftb->arm) {
				(*c->ftb->arm)(c, d, w, SDLPUI_ACTION_HINT_KEY);
			}
		} else {
			/*
			 * Always disarm, regardless of the modifier state,
			 * in case a modifier key was pressed between
			 * depressing and releasing the key.
			 */
			if (c->ftb->disarm) {
				(*c->ftb->disarm)(c, d, w,
					SDLPUI_ACTION_HINT_KEY);
			}
			if (mods == KMOD_NONE) {
				if (c->ftb->respond_default) {
					SDLPUI_EVENT_TRACER("control", c,
						"(not extracted)",
						"invoking default reponse");
					(*c->ftb->respond_default)(c, d, w,
						SDLPUI_ACTION_HINT_KEY);
				} else if (d->ftb->respond_default) {
					SDLPUI_EVENT_TRACER("dialog", d,
						"(not extracted)",
						"invoking default reponse");
					(*d->ftb->respond_default)(d, w);
				}
			}
		}
		return true;
	}

	/* Let the containing dialog handle everything else. */
	return false;
}


/**
 * Handle a mouse button event for a simple control.
 *
 * \param c is the control receiving the event.  The events handled here
 * are appropriate if c is simple:  not a compound control and only having
 * at most one action (the default one; with the associated arming and
 * disarming).
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the dialog; otherwise return false.
 */
bool sdlpui_control_handle_mouseclick(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseButtonEvent *e)
{
	if (e->button == SDL_BUTTON_LEFT) {
		if (e->state == SDL_PRESSED) {
			if (c->ftb->arm) {
				(*c->ftb->arm)(c, d, w,
					 SDLPUI_ACTION_HINT_MOUSE);
			}
		} else {
			if (c->ftb->disarm) {
				(*c->ftb->disarm)(c, d, w,
					SDLPUI_ACTION_HINT_MOUSE);
			}
			if (c->ftb->respond_default) {
				SDLPUI_EVENT_TRACER("control", c,
					"(not extracted)",
					"invoking default response");
				(*c->ftb->respond_default)(c, d, w,
					SDLPUI_ACTION_HINT_MOUSE);
			}
		}
	}
	/* Swallow the event, even if nothing was done. */
	return true;
}


/**
 * Handle a mouse motion event for a simple control.
 *
 * \param c is the control receiving the event.  The events handled here
 * are appropriate if c is simple:  not a compound control and only having
 * at most one action (the default one; with the associated arming and
 * disarming).
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 * \param e is the event to handle.
 * \return true if the event is handled and doesn't need further processing by
 * the dialog; otherwise return false.
 */
bool sdlpui_control_handle_mousemove(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w,
		const struct SDL_MouseMotionEvent *e)
{
	/*
	 * Ignore motion events while a mouse button is pressed (at least
	 * up to the point that the mouse leaves the window).  If the mouse
	 * is moving within the control, it also does nothing.
	 */
	if (e->state != 0 || sdlpui_is_in_control(c, d, e->x, e->y)) {
		return true;
	}
	/*
	 * Otherwise, let the dialog handle the motion to see if it enters
	 * another control.
	 */
	return false;
}


/**
 * Invoke the given dialog's default action.  Suitable for use as a callback
 * for buttons.
 *
 * \param c is the control which was activated.
 * \param d is the dialog containing the control.
 * \param w is the window containing the dialog.
 */
void sdlpui_invoke_dialog_default_action(struct sdlpui_control *c,
		struct sdlpui_dialog *d, struct sdlpui_window *w)
{
	if (d->ftb->respond_default) {
		SDLPUI_EVENT_TRACER("control", c, "(not extracted)",
			"invoking containing dialog's default action");
		(*d->ftb->respond_default)(d, w);
	}
}


/**
 * Initializes the contents of c to be appropriate for an image control.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param image is the texture holding the image to be used for the control.
 * The control will assume ownership of the texture and call
 * SDL_DestroyTexture() on it when the control is destroyed.
 * \param halign specifies how the image should be horizontally aligned within
 * the control's bounds if the control is wider than the image.
 * \param top_margin is the amount of space, in pixels, to leave between the
 * top of the control and the top of the image.  Must be non-negative.
 * \param bottom_margin is the amount of space, in pixels, to leave between the
 * bottom of the control and the bottom of the image.  Must be non-negative.
 * \param left_margin is the amount of space, in pixels to leave between the
 * left side of the control and the left side of the image.  Must be
 * non-negative.
 * \param right_margin is the amount of space, in pixels to leave between the
 * right side of the control and the right side of the image.  Must be
 * non-negative.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_image(struct sdlpui_control *c, SDL_Texture *image,
		enum sdlpui_hor_align halign, int top_margin, int bottom_margin,
		int left_margin, int right_margin)
{
	struct sdlpui_image *ip = SDL_malloc(sizeof(*ip));

	ip->image = image;
	ip->halign = halign;
	ip->top_margin = (top_margin > 0) ? top_margin : 0;
	ip->bottom_margin = (bottom_margin > 0) ? bottom_margin : 0;
	ip->left_margin = (left_margin > 0) ? left_margin : 0;
	ip->right_margin = (right_margin > 0) ? right_margin : 0;
	c->ftb = &image_funcs;
	c->priv = ip;
	c->type_code = SDLPUI_CTRL_IMAGE;
}


/**
 * Initializes the contents of c to be appropriate for a label control.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_label(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign)
{
	struct sdlpui_label *lp = SDL_malloc(sizeof(*lp));

	lp->caption = SDL_strdup(caption);
	lp->halign = halign;
	c->ftb = &label_funcs;
	c->priv = lp;
	c->type_code = SDLPUI_CTRL_LABEL;
}


/**
 * Initializes the contents of c to be appropriate for a push button control.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param callback is the function to invoke when the button is released.  It
 * may be NULL.
 * \param tag is a value the application can use as it wishes to have buttons
 * with the same callback act differently.
 * \param disabled will, if true, cause the button to ignore button or keyboard
 * events and be displayed with an altered appearance until the button is
 * reenabled.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_push_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled)
{
	struct sdlpui_push_button *pbp = SDL_malloc(sizeof(*pbp));

	pbp->caption = SDL_strdup(caption);
	pbp->callback = callback;
	pbp->halign = halign;
	pbp->tag = tag;
	pbp->disabled = disabled;
	pbp->has_key = false;
	pbp->has_mouse = false;
	pbp->armed = false;
	c->ftb = &push_button_funcs;
	c->priv = pbp;
	c->type_code = SDLPUI_CTRL_PUSH_BUTTON;
}


/**
 * Initializes the contents of c to be appropriate for a menu button control.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param callback is the function to invoke when the button is released.  It
 * may be NULL.
 * \param tag is a value the application can use as it wishes to have buttons
 * with the same callback act differently.
 * \param disabled will, if true, cause the button to ignore button or keyboard
 * events and be displayed with an altered appearance until the button is
 * reenabled.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_menu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*w), int tag, bool disabled)
{
	struct sdlpui_menu_button *mbp = SDL_malloc(sizeof(*mbp));

	mbp->caption = SDL_strdup(caption);
	mbp->callback = callback;
	mbp->halign = halign;
	mbp->tag = tag;
	mbp->has_key = 0;
	mbp->has_mouse = 0;
	mbp->armed = 0;
	mbp->disabled = disabled;
	mbp->subtype_code = SDLPUI_MB_NONE;
	c->ftb = &menu_button_funcs;
	c->priv = mbp;
	c->type_code = SDLPUI_CTRL_MENU_BUTTON;
}


/**
 * Initializes the contents of c to be appropriate for a menu indicator control.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param tag is a value the application can use as it wishes to have otherwise
 * similar indicators act differently.
 * \param curr_value is whether or not the indicator is currently on.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_menu_indicator(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, int tag, bool curr_value)
{
	struct sdlpui_menu_button *mbp = SDL_malloc(sizeof(*mbp));

	mbp->caption = SDL_strdup(caption);
	mbp->callback = NULL;
	mbp->halign = halign;
	mbp->tag = tag;
	mbp->has_key = 0;
	mbp->has_mouse = 0;
	mbp->armed = 0;
	mbp->disabled = false;
	mbp->subtype_code = SDLPUI_MB_INDICATOR;
	mbp->v.toggled = curr_value;
	c->ftb = &menu_button_funcs;
	c->priv = mbp;
	c->type_code = SDLPUI_CTRL_MENU_BUTTON;
}


/**
 * Initializes the contents of c to be appropriate for a menu button
 * controlling an integer parameter limited to a fixed range.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param callback is the function to invoke when the value associated with
 * button changes due to a user interface event.  It may be NULL.
 * \param tag is a value the application can use as it wishes to have buttons
 * with the same callback act differently.
 * \param disabled will, if true, cause the button to ignore button or keyboard
 * events and be displayed with an altered appearance until the button is
 * reenabled.
 * \param curr_value is the current value for the integer associated with the
 * button.
 * \param min_value is the minimum value for the integer associated with the
 * button.
 * \param max_value is the minimum value for the integer associated with the
 * button.  max_value must be greater than or equal to min_value.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_menu_ranged_int(struct sdlpui_control *c,
		const char *caption, enum sdlpui_hor_align halign,
		void (*callback)(struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled, int curr_value,
		int min_value, int max_value)
{
	struct sdlpui_menu_button *mbp = SDL_malloc(sizeof(*mbp));

	mbp->caption = SDL_strdup(caption);
	mbp->callback = callback;
	mbp->halign = halign;
	mbp->tag = tag;
	mbp->has_key = 0;
	mbp->has_mouse = 0;
	mbp->armed = 0;
	mbp->disabled = disabled;
	mbp->subtype_code = SDLPUI_MB_RANGED_INT;
	mbp->v.ranged_int.min = min_value;
	mbp->v.ranged_int.max = max_value;
	mbp->v.ranged_int.curr = curr_value;
	mbp->v.ranged_int.old = curr_value;
	c->ftb = &menu_button_funcs;
	c->priv = mbp;
	c->type_code = SDLPUI_CTRL_MENU_BUTTON;
}


/**
 * Initializes the contents of c to be appropriate for a toggleable control
 * in a menu.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param callback is the function to invoke when the button is released.  It
 * may be NULL.
 * \param tag is a value the application can use as it wishes to have buttons
 * with the same callback act differently.
 * \param disabled will, if true, cause the button to ignore button or keyboard
 * events and be displayed with an altered appearance until the button is
 * reenabled.
 * \param curr_value is whether the toggle is currently on.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_menu_toggle(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, void (*callback)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*), int tag, bool disabled,
		bool curr_value)
{
	struct sdlpui_menu_button *mbp = SDL_malloc(sizeof(*mbp));

	mbp->caption = SDL_strdup(caption);
	mbp->callback = callback;
	mbp->halign = halign;
	mbp->tag = tag;
	mbp->has_key = 0;
	mbp->has_mouse = 0;
	mbp->armed = 0;
	mbp->disabled = disabled;
	mbp->subtype_code = SDLPUI_MB_TOGGLE;
	mbp->v.toggled = curr_value;
	c->ftb = &menu_button_funcs;
	c->priv = mbp;
	c->type_code = SDLPUI_CTRL_MENU_BUTTON;
}


/**
 * Initializes the contents of c to be appropriate for a menu button that
 * leads to a nested menu.
 *
 * \param c points to the control to be initialized.  Must not be NULL.
 * \param caption is the null-terminated UTF-8 string to use as the label.
 * The contents of caption are copied, so the lifetime of what's passed is
 * independent of the lifetime of the control.
 * \param halign specifies how the label should be horizontally aligned within
 * the control's bounds if the control is wider than the label.
 * \param creator is the function to invoke to create the nested menu.  It must
 * not be NULL.  It takes five arguments:  a pointer to the parent control for
 * menu, a pointer to the parent dialog for that control, a pointer to the
 * window containing that dialog and control, the x coordinate (relative to
 * the window) for the upper right corner of the nested menu, and the y
 * coordinate (relative to the window) for the upper rgith corner of the
 * nested menu.
 * \param placement specifies how the nested menu will be placed relative to
 * the menu button.
 * \param tag is a value the application can use as it wishes to have otherwise
 * similar buttons act differently.
 * \param disabled will, if true, cause the button to ignore button or keyboard
 * events and be displayed with an altered appearance until the button is
 * reenabled.
 *
 * Note that the bounding rectangle for the control is not set.  The caller
 * should use c->ftb->resize to set that (perhaps in conjunction with
 * c->ftb->query_natural_size) and set c->rect.x and c->rect.y directly to
 * position the control in a dialog.
 */
void sdlpui_create_submenu_button(struct sdlpui_control *c, const char *caption,
		enum sdlpui_hor_align halign, struct sdlpui_dialog *(*creator)(
		struct sdlpui_control*, struct sdlpui_dialog*,
		struct sdlpui_window*, int ul_x_win, int ul_y_win),
		enum sdlpui_child_menu_placement placement, int tag,
		bool disabled)
{
	struct sdlpui_menu_button *mbp = SDL_malloc(sizeof(*mbp));

	mbp->caption = SDL_strdup(caption);
	mbp->callback = NULL;
	mbp->halign = halign;
	mbp->tag = tag;
	mbp->has_key = 0;
	mbp->has_mouse = 0;
	mbp->armed = 0;
	mbp->disabled = disabled;
	mbp->subtype_code = SDLPUI_MB_SUBMENU;
	mbp->v.submenu.creator = creator;
	mbp->v.submenu.child = NULL;
	mbp->v.submenu.placement = placement;
	c->ftb = &menu_button_funcs;
	c->priv = mbp;
	c->type_code = SDLPUI_CTRL_MENU_BUTTON;
}
