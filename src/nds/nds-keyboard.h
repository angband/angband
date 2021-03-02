#ifndef _NDS_KEYBOARD_H
#define _NDS_KEYBOARD_H

#include "../h-basic.h"

typedef struct {
	u16b width;
	u16b code;
} nds_kbd_key;

#define K_MODIFIER	0x100
#define K_CAPS		0x101
#define K_SHIFT		0x102
#define K_CTRL		0x103
#define K_ALT		0x104
#define K_F(n)		(0x200 + n)
#define K_SHIFTED_MOVE	0x0200

bool nds_kbd_init();
byte nds_kbd_vblank();

#endif