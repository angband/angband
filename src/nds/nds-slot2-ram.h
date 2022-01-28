/**********************************
  Copyright (C) Rick Wong (Lick)

 Credits: Amadeus, Chishm, Cory1492, Lazy1, Pepsiman, Viruseb

Dual licensed under the MIT (http://www.opensource.org/licenses/mit-license.php) 
and GPLv2 (http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt) licenses. 

***********************************/
#ifndef _NDS_SLOT2_RAM
#define _NDS_SLOT2_RAM

#ifndef __3DS__

#include <nds/ndstypes.h>

typedef enum { DETECT_RAM=0, SC_RAM, M3_RAM, OPERA_RAM, G6_RAM, EZ_RAM } RAM_TYPE;

//  Call this before the others
bool  ram_init (RAM_TYPE);

//  Returns the type of the RAM device
RAM_TYPE   ram_type (void);

//  Returns the type of the RAM device in a string
const char*   ram_type_string (void);

//  Returns the total amount of RAM in bytes
u32   ram_size (void);


//  Unlocks the RAM and returns a pointer to the begin
vu16* ram_unlock (void);

//  Locks the RAM
void  ram_lock (void);

//  enable = set lowest waitstates, disable = set default waitstates
void  ram_turbo (bool enable);

#endif

#endif
