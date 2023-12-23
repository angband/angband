/**
 * \file sdl2/pui-misc.h
 * \brief Declare miscellaneous utilities for the primitive UI toolkit for SDL2.
 */
#ifndef INCLUDED_SDL2_SDLPUI_MISC_H
#define INCLUDED_SDL2_SDLPUI_MISC_H

#include "pui-win.h"

#ifdef SDLPUI_TRACE_EVENTS
#define SDLPUI_EVENT_TRACER(type_name, address, label, event_name) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s (%p ; \"%s\") %s\n", type_name, (void*)address, label, event_name)
#else
#define SDLPUI_EVENT_TRACER(type_name, address, label, event_name) (void)0
#endif

#ifdef SDLPUI_TRACE_RENDER
#define SDLPUI_RENDER_TRACER(type_name, address, label, outer_rect, inner_rect, texture) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s (%p ; \"%s\") rendering in outer bounds, (%d,%d) %d x %d, and inner bounds, (%d,%d) %d x %d, to %p\n", type_name, (void*)address, label, outer_rect.x, outer_rect.y, outer_rect.w, outer_rect.h, inner_rect.x, inner_rect.y, inner_rect.w, inner_rect.h, (void*)texture)
#else
#define SDLPUI_RENDER_TRACER(type_name, address, label, outer_rect, inner_rect, texture) (void)0
#endif

int sdlpui_init(void);
void sdlpui_quit(void);
Uint32 sdlpui_register_code(const char *name);
SDL_Keymod sdlpui_get_interesting_keymods(void);
struct sdlpui_stipple sdlpui_compute_stipple(SDL_Renderer *r);
void sdlpui_stipple_rect(SDL_Renderer *r, struct sdlpui_stipple *stp,
		const SDL_Rect *dst_r);
void sdlpui_render_utf8_line(SDL_Renderer *r, TTF_Font *font,
		const SDL_Color *fg, const SDL_Rect *dst_r, const char *s);
void sdlpui_get_utf8_metrics(TTF_Font *font, const char *s, int *w, int *h);
Uint32 sdlpui_utf8_to_codepoint(const char *uft8_string);

#endif /* INCLUDED_SDL2_SDLPUI_MISC_H */
