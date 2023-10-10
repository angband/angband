/* File: borg9.h */
/* Purpose: Header file for "borg9.c" -BEN- */

#ifndef INCLUDED_BORG9_H
#define INCLUDED_BORG9_H

#include "../angband.h"
#include "../cave.h"
#include "../obj-tval.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg9.c".
 */
#ifndef BABLOS
extern void resurrect_borg(void);
extern void borg_write_map(bool ask);

#endif /* bablos */

extern void borg_save_scumfile(void);
extern void borg_status(void);
/*
 * Initialize this file
 */
extern void borg_init_9(void);
extern void borg_clean_9(void);

#endif

#endif
