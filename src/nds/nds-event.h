#ifndef _NDS_EVENT_H
#define _NDS_EVENT_H

#include "../h-basic.h"
#include "../ui-event.h"

typedef struct {
    keycode_t key;
    uint8_t mods;
} nds_event_keyboard;

typedef struct {
    uint8_t x;
    uint8_t y;
} nds_event_mouse;

typedef struct {
    enum {
        NDS_EVENT_INVALID = 0,
        NDS_EVENT_KEYBOARD,
        NDS_EVENT_MOUSE,
    } type;
    union {
        nds_event_keyboard keyboard;
        nds_event_mouse mouse;
    };
} nds_event;

bool nds_event_init();
bool nds_event_ready();
nds_event nds_event_get();
void nds_event_put_key(keycode_t k, uint8_t mods);
void nds_event_put_mouse(uint8_t x, uint8_t y);

#endif