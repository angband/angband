/*
 * File: ui-event.c
 * Purpose: Generic input event handling
 *
 * Copyright (c) 2007 Pete Mack and others.
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
#include "angband.h"

/*
 * Implementation of Extremely Basic Event Model.
 * Limits:
 *   all events are of the concrete type ui_event_type (see ui-event.h), 
 *   which are supposed to model simple UI actions:
 *	- < escape >
 *	- keystroke
 *	- mousepress
 *	- select menu element
 *	- move menu cursor
 *	- back to parent (hierarchical menu escape)
 *
 * There are 3 basic event-related classes:
 * The ui_event_type.
 * Concrete event, with at most 32 distinct types.
 *
 * The event_listener observer for key events
 *
 * The event_listener   The registrar for event_listeners.
 * For convenience, the event target is also an event_listener.
 */




/*
 * Primitive event loop.
 *
 *  - target = the event target
 *  - start - optional initial event that allows you to prime the loop without
 *    pushing the event queue.
 *  Returns:
 *    EVT_STOP - the loop was halted.
 *    EVT_AGAIN - start was not handled
 *    The first unhandled event.
 */
ui_event_data run_event_loop(event_listener *target, const ui_event_data *start)
{
	ui_event_data ke = EVENT_EMPTY;
	bool handled = TRUE;

	while (handled)
	{
		handled = FALSE;

		if (start) ke = *start;
		else ke = inkey_ex();

		if (ke.type == EVT_STOP)
			break;

		if (ke.type & target->event_flags)
			handled = target->handler(target->object, &ke);

		if (handled) start = NULL;
	}

	if (start)
	{
		ke.type = EVT_AGAIN;
		ke.key = '\xff';
	}

	return ke;
}
