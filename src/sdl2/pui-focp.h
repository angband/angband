/**
 * \file sdl2/pui-focp.h
 * \brief Declare private interfaces, used internally in sdlpui-misc.c or by
 * test cases, for initializing and cleaning up for the routines in
 * sdlpui-foc.c.
 */

#ifndef INCLUDED_SDL2_SDLPUI_FOCUSP_H
#define INCLUDED_SDL2_SDLPUI_FOCUSP_H

extern Uint32 sdlpui_redraw_event_type;

int sdlpui_foc_init(void);
void sdlpui_foc_quit(void);

#endif /* INCLUDED_SDL2_SDLPUI_FOCUSP_H */
