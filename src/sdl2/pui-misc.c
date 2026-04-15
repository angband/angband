/**
 * \file sdl2/pui-misc.c
 * \brief Define miscellaneous utilities for the primitive UI toolkit for SDL2.
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

#include "pui-misc.h"
#include "pui-ctrl.h"
#include "pui-dlg.h"


struct sdlpui_code {
	char *name;
	Uint32 code;
};

struct sdlpui_code_registry {
	SDL_mutex* lock;
	struct sdlpui_code *entries;
	size_t alloc;
	Uint32 count;
};

struct sdlpui_id_registry {
	SDL_mutex *lock;
	struct sdlpui_dialog *d_head;
	Uint32 count;
};

static struct sdlpui_code_registry my_codes = { NULL, NULL, 0, 0 };
static struct sdlpui_id_registry my_ids = { NULL, NULL, 0 };

/**
 * Initialize the resources needed by the sdlpui_*() calls.
 *
 * Can safely be called multiple times without an intervening sdlpui_quit().
 * For multithreaded applications, a race condition condition is possible
 * if sdlpui_init() can be called while a call to sdlpui_quit() is in progress.
 * Those applications should be structured to avoid that possibility.
 */
int sdlpui_init(void)
{
#if defined(SDLPUI_TRACE_EVENTS) || defined (SDLPUI_TRACE_RENDER)
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION,
		SDL_LOG_PRIORITY_VERBOSE);
#endif

	if (!my_codes.lock) {
		my_codes.lock = SDL_CreateMutex();
		if (!my_codes.lock) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Could not create code mutex in "
				"sdlpui_init(): %s", SDL_GetError());
			return 1;
		}

		/* Initialize predefined type codes from pui-dlg.h. */
		SDLPUI_DIALOG_SIMPLE_MENU =
			sdlpui_register_code("SDLPUI_DIALOG_SIMPLE_MENU");
		SDLPUI_DIALOG_SIMPLE_INFO =
			sdlpui_register_code("SDLPUI_DIALOG_SIMPLE_INFO");
		if (!SDLPUI_DIALOG_SIMPLE_MENU || !SDLPUI_DIALOG_SIMPLE_INFO) {
			sdlpui_quit();
			return 1;
		}

		/* Initialize predefined type codes from pui-ctrl.h. */
		SDLPUI_CTRL_IMAGE = sdlpui_register_code("SDLPUI_CTRL_IMAGE");
		SDLPUI_CTRL_LABEL = sdlpui_register_code("SDLPUI_CTRL_LABEL");
		SDLPUI_CTRL_MENU_BUTTON =
			sdlpui_register_code("SDLPUI_CTRL_MENU_BUTTON");
		SDLPUI_CTRL_PUSH_BUTTON =
			sdlpui_register_code("SDLPUI_CTRL_PUSH_BUTTON");
		if (!SDLPUI_CTRL_IMAGE || !SDLPUI_CTRL_LABEL
				|| !SDLPUI_CTRL_MENU_BUTTON
				|| !SDLPUI_CTRL_PUSH_BUTTON) {
			sdlpui_quit();
			return 1;
		}
	}

	if (!my_ids.lock) {
		my_ids.lock = SDL_CreateMutex();
		if (!my_ids.lock) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Could not create id mutex in "
				"sdlpui_init(): %s", SDL_GetError());
			sdlpui_quit();
			return 1;
		}
	}

	return 0;
}


/**
 * Release the resources allocated by sdlpui_init().
 *
 * Can safey be called multiple times without an intervening call to.
 * sdlpui_init().  Once called, the only sdlpui_*() calls that can be safely
 * used are sdlpui_init() and sdlpui_quit().  For multithreaded applications,
 * race conditions are possible if sdlpui_init() or sdlpui_quit() can be
 * called while a call to sdlpui_quit() is in progress.  Those applications
 * should be structured to avoid that possibility.
 */
void sdlpui_quit(void)
{
	SDL_mutex *lock = my_codes.lock;

	if (lock) {
		if (!SDL_LockMutex(lock)) {
			struct sdlpui_code *entries = my_codes.entries;
			Uint32 i, n = my_codes.count;

			my_codes.lock = NULL;
			my_codes.entries = NULL;
			my_codes.alloc = 0;
			my_codes.count = 0;
			if (SDL_UnlockMutex(lock)) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"Could not release code mutex in "
					"sdlpui_quit()");
			}
			for (i = 0; i < n; ++i) {
				SDL_free(entries[i].name);
			}
			SDL_free(entries);
			SDL_DestroyMutex(lock);
		} else {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Could not acquire code mutex in "
				"sdlpui_quit()");
		}
	}

	lock = my_ids.lock;
	if (lock) {
		if (!SDL_LockMutex(lock)) {
			my_ids.lock = NULL;
			my_ids.d_head = NULL;
			my_ids.count = 0;
			if (SDL_UnlockMutex(lock)) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"Could not release id mutex in "
					"sdlpui_quit()");
			}
			SDL_DestroyMutex(lock);
		} else {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Could not acquire id mutex in sdlpui_quit()");
		}
	}
}


/**
 * Remember a dialog so that its ID and the IDs of the controls it contains
 * can be reassigned, as necessary.
 *
 * \param d points to the dialog to forget.  It may not be NULL.
 */
void sdlpui_register_dialog(struct sdlpui_dialog *d)
{
	if (SDL_LockMutex(my_ids.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not acquire mutex in sdlpui_register_dialog()");
		sdlpui_force_quit();
	}
	/* Only register if not already registered. */
	if (!d->next_r && !d->prev_r && (!my_ids.d_head
			|| my_ids.d_head->id != d->id)) {
		d->next_r = my_ids.d_head;
		if (my_ids.d_head) {
			my_ids.d_head->prev_r = d;
		}
		my_ids.d_head = d;
	}
	if (SDL_UnlockMutex(my_ids.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not release mutex in sdlpui_register_dialog()");
		sdlpui_force_quit();
	}
}


/**
 * Forget a dialog so that it is no longer considered when reassigning IDs.
 *
 * \param d points to the dialog to forget.  It may not be NULL.
 */
void sdlpui_unregister_dialog(struct sdlpui_dialog *d)
{
	if (SDL_LockMutex(my_ids.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not acquire mutex in "
			"sdlpui_unregister_dialog()");
		sdlpui_force_quit();
	}
	/* Only unregister if already registered. */
	if (d->next_r || d->prev_r || (my_ids.d_head
			&& my_ids.d_head->id == d->id)) {
		if (d->next_r) {
			d->next_r->prev_r = d->prev_r;
		}
		if (d->prev_r) {
			d->prev_r->next_r = d->next_r;
		} else {
			my_ids.d_head = d->next_r;
		}
		d->next_r = NULL;
		d->prev_r = NULL;
	}
	if (SDL_UnlockMutex(my_ids.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not release mutex in "
			"sdlpui_unregister_dialog()");
		sdlpui_force_quit();
	}
}


Uint32 sdlpui_register_code(const char *name)
{
	Uint32 code = 0, ilo, ihi;

	if (!name) {
		return code;
	}
	if (SDL_LockMutex(my_codes.lock)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Could not acquire mutex in sdlpui_register_code()");
		return code;
	}

	/* Sorted alphabetically by name so use a binary search. */
	SDL_assert(my_codes.count <= my_codes.alloc);
	ilo = 0;
	ihi = my_codes.count;
	while (1) {
		Uint32 imid;
		int cmp;

		if (ilo == ihi) {
			/*
			 * It is not present.  Shift entries starting with ilo
			 * up by one to create space for a new entry.
			 */
			char *name_copy;

			name_copy = SDL_strdup(name);
			if (!name_copy) {
				break;
			}
			if (my_codes.count == my_codes.alloc) {
				size_t new_alloc;
				struct sdlpui_code *new_entries;

				if (my_codes.alloc == 0) {
					new_alloc = 8;
				} else if (my_codes.alloc < SDL_MAX_UINT32
						&& my_codes.alloc
						< (size_t)-1
						/ sizeof(*new_entries)) {
					new_alloc = (my_codes.alloc
						< SDL_MAX_UINT32 / 2)
						? my_codes.alloc +
						my_codes.alloc : SDL_MAX_UINT32;
				} else {
					/*
					 * Will exceed the range of an Uint32
					 * or size_t.  Give up.
					 */
					SDL_free(name_copy);
					break;
				}
				new_entries = SDL_realloc(my_codes.entries,
					new_alloc * sizeof(*new_entries));
				if (!new_entries) {
					SDL_free(name_copy);
					break;
				}
				my_codes.entries = new_entries;
				my_codes.alloc = new_alloc;
			}
			if (ilo < my_codes.count) {
				SDL_memmove(my_codes.entries + ilo + 1,
					my_codes.entries + ilo,
					(my_codes.count - ilo)
					* sizeof(*my_codes.entries));
			}
			code = ++my_codes.count;
			my_codes.entries[ilo].name = name_copy;
			my_codes.entries[ilo].code = code;
			break;
		}

		imid = ilo + (ihi - ilo) / 2;
		cmp = SDL_strcmp(my_codes.entries[imid].name, name);
		if (cmp == 0) {
			code = my_codes.entries[imid].code;
			break;
		}
		if (cmp < 0) {
			ilo = imid + 1;
		} else {
			ihi = imid;
		}
	}

	if (SDL_UnlockMutex(my_codes.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not release mutex in sdlpui_register_code()");
		sdlpui_force_quit();
	}

	return code;
}


Uint32 sdlpui_reserve_id(void)
{
	Uint32 result = 0;

	if (SDL_LockMutex(my_ids.lock)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Could not acquire mutex in sdlpui_reserve_id()");
		return result;
	}

	if (my_ids.count == SDL_MAX_UINT32) {
		struct sdlpui_dialog *d = my_ids.d_head;
		Uint32 start = 1;

		while (1) {
			Uint32 count;

			if (!d) {
				result = start;
				my_ids.count = start;
				break;
			}
			count = (d->ftb->reassign_ids)(d, start);
			if (!count || start > SDL_MAX_UINT32 - count) {
				break;
			}
			start += count;
			d = d->next_r;
		}
	} else {
		result = ++my_ids.count;
	}

	if (SDL_UnlockMutex(my_ids.lock)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"Could not release mutex in sdlpui_reserve_id()");
		sdlpui_force_quit();
	}

	return result;
}


/**
 * Strip modifiers from SDL_GetModState() that are not relevant to the
 * primitive UI toolkit.
 *
 * \return the modified set of keyboard modifiers.
 *
 * Does this also need to strip off KMOD_CAPS?
 */
SDL_Keymod sdlpui_get_interesting_keymods(void)
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
	return SDL_GetModState() & ~(KMOD_NUM | KMOD_SCROLL);
#else
	return SDL_GetModState() & ~(KMOD_NUM);
#endif
}


/**
 * Compute a stipple pattern for use with a given renderer.
 *
 * \param r is the renderer that will be used.
 * \return the structure describing the stipple pattern.  If an error
 * occurs, the texture element in the structure will be NULL and SDL_GetError()
 * will describe the cause of the error.  When the stipple pattern is no
 * longer needed, the returned texture in the structure should be released
 * with SDL_DestroyTexture().
 */
struct sdlpui_stipple sdlpui_compute_stipple(SDL_Renderer *r)
{
	/*
	 * The dimensions must be a multiple of two:  see the loop logic
	 * below.
	 */
	const int width = 16, height = 16;
	struct sdlpui_stipple result = { NULL, 0, 0 };
	SDL_Surface *s;
	Uint32 *pixels;
	Uint32 rmask, gmask, bmask, amask, on_pixel, off_pixel;
	int y;

	/*
	 * on_pixel is black and completely transparent.  off_pixel is gray
	 * (0x40, 0x40, 0x40) and slightly opaque.
	 */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
	on_pixel = 0x000000ff;
	off_pixel = 0x40404040;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
	on_pixel = 0xff000000;
	off_pixel = 0x40404040;
#endif

	SDL_assert(!(width & 1) && !(height & 1));
	pixels = SDL_malloc(width * height * sizeof(*pixels));
	if (!pixels) {
		return result;
	}
	for (y = 0; y < height; y += 2) {
		uint32_t *row = pixels + y * width;
		int x;

		for (x = 0; x < width; x += 2) {
			row[x] = on_pixel;
			row[x + 1] = off_pixel;
			row[x + width] = off_pixel;
			row[x + width + 1] = on_pixel;
		}
	}

	s = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, 4 * width,
		rmask, gmask, bmask, amask);
	if (s) {
		result.texture = SDL_CreateTextureFromSurface(r, s);
		SDL_FreeSurface(s);
		if (result.texture) {
			result.w = width;
			result.h = height;
		}
	}
	SDL_free(pixels);

	return result;
}


/**
 * Stipple a rectangle.
 *
 * \param r is the renderer to use.
 * \param stp points to the texture and the texture's dimensions to use for
 * stippling.  Must not be NULL.
 * \param dst_r points to the rectangle bounding the area to stipple.  Must not
 * be NULL.
 */
void sdlpui_stipple_rect(SDL_Renderer *r, struct sdlpui_stipple *stp,
		const SDL_Rect *dst_r)
{
	SDL_Rect src_r = { 0, 0, 0, 0 }, dst2_r;
	int ylim = dst_r->y + dst_r->h, xlim = dst_r->x + dst_r->w;

	if (!stp->texture) {
		return;
	}
	for (dst2_r.y = dst_r->y; dst2_r.y < ylim; dst2_r.y += stp->h) {
		dst2_r.h = (dst2_r.y + stp->h > ylim) ?
			ylim - dst2_r.y : stp->h;
		src_r.h = dst2_r.h;
		for (dst2_r.x = dst_r->x; dst2_r.x < xlim; dst2_r.x += stp->w) {
			dst2_r.w = (dst2_r.x + stp->w > xlim) ?
				xlim - dst2_r.x : stp->w;
			src_r.w = dst2_r.w;
			SDL_RenderCopy(r, stp->texture, &src_r, &dst2_r);
		}
	}
}


/**
 * Render a line of UTF-8 text in a given font and color.
 *
 * \param r is the renderer to use.
 * \param font is the font to use.
 * \param fg points to the color to use; must not be NULL.
 * \param dst_r points to the rectangle to be affected by the rendering.
 * Must not be NULL.
 * \param s is the null-terminated UTF-8 string to render, must not be NULL.
 * Note that the rendered result is always a single line, even if s contains
 * embedded newlines.
 */
void sdlpui_render_utf8_line(SDL_Renderer *r, TTF_Font *font,
		const SDL_Color *fg, const SDL_Rect *dst_r, const char *s)
{
	SDL_Surface *surface = TTF_RenderUTF8_Blended(font, s, *fg);
	SDL_Texture *src_t;
	SDL_Rect src_r;

	if (!surface) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"TTF_RenderUTF8_Blended() failed: %s", TTF_GetError());
		sdlpui_force_quit();
	}
	src_t = SDL_CreateTextureFromSurface(r, surface);
	if (!src_t) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_CreateTextureFromSurface() failed: %s",
			SDL_GetError());
		sdlpui_force_quit();
	}
	SDL_FreeSurface(surface);
	src_r.x = 0;
	src_r.y = 0;
	if (SDL_QueryTexture(src_t, NULL, NULL, &src_r.w, &src_r.h)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_QueryTexture() failed: %s", SDL_GetError());
		sdlpui_force_quit();
	}
	/*
	 * Truncate rather than compress if the rendered string is bigger than
	 * the destination.
	 */
	if (src_r.w > dst_r->w) {
		src_r.w = dst_r->w;
	}
	if (src_r.h > dst_r->h) {
		src_r.h = dst_r->h;
	}
	SDL_RenderCopy(r, src_t, &src_r, dst_r);
	SDL_DestroyTexture(src_t);
}


/**
 * Get the width and height of rendered UTF-8 text.
 *
 * \param font is the font to use for rendering.
 * \param s is the null-terminated UTF-8 string.
 * \param w is dereferenced and set to the width of the renderered string.
 * \param h is dereferenced and set to the height of the renderered string.
 */
void sdlpui_get_utf8_metrics(TTF_Font *font, const char *s, int *w, int *h)
{
	if (TTF_SizeUTF8(font, s, w, h)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"TTF_SizeUTF8() failed for '%s': %s", s,
			TTF_GetError());
		sdlpui_force_quit();
	}
}


/**
 * Return the UTF-32 encoding for the first codepoint in a UTF-8 string.
 *
 * \param utf8_string is the null-terminated string of interest.
 * \return the UTF-32 encoding, in the native byte order, for the first
 * codepoint in the string.
 */
uint32_t sdlpui_utf8_to_codepoint(const char *utf8_string)
{
        /* hex  == binary
         * 0x00 == 00000000
         * 0x80 == 10000000
         * 0xc0 == 11000000
         * 0xe0 == 11100000
         * 0xf0 == 11110000
         * 0xf8 == 11111000
         * 0x3f == 00111111
         * 0x1f == 00011111
         * 0x0f == 00001111
         * 0x07 == 00000111 */

        uint32_t key = 0;

#define IS_UTF8_INFO(mask, result) (((unsigned char) utf8_string[0] & (mask)) == (result))
#define EXTRACT_UTF8_INFO(pos, mask, shift) (((unsigned char) utf8_string[(pos)] & (mask)) << (shift))
        /* 6 is the number of information bits in a utf8 continuation byte (10xxxxxx) */
        if (IS_UTF8_INFO(0x80, 0)) {
                key = utf8_string[0];
        } else if (IS_UTF8_INFO(0xe0, 0xc0)) {
                key = EXTRACT_UTF8_INFO(0, 0x1f, 6)
                        | EXTRACT_UTF8_INFO(1, 0x3f, 0);
        } else if (IS_UTF8_INFO(0xf0, 0xe0)) {
                key = EXTRACT_UTF8_INFO(0, 0x0f, 12)
                        | EXTRACT_UTF8_INFO(1, 0x3f, 6)
                        | EXTRACT_UTF8_INFO(2, 0x3f, 0);
        } else if (IS_UTF8_INFO(0xf8, 0xf0)) {
                key = EXTRACT_UTF8_INFO(0, 0x07, 18)
                        | EXTRACT_UTF8_INFO(1, 0x3f, 12)
                        | EXTRACT_UTF8_INFO(2, 0x3f, 6)
                        | EXTRACT_UTF8_INFO(3, 0x3f, 0);
        }
#undef IS_UTF8_INFO
#undef EXTRACT_UTF8_INFO

        return key;
}
