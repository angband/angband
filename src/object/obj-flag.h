/*
 * File: src/object/obj-flag.h
 * Purpose: definitions and functions for object flags
 *
 * Copyright (c) 2011 Chris Carr
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
#ifndef INCLUDED_OBJFLAG_H
#define INCLUDED_OBJFLAG_H

#include "z-rand.h"
#include "z-file.h"
#include "z-textblock.h"
#include "z-quark.h"
#include "z-bitflag.h"
#include "game-cmd.h"
#include "cave.h"

/*** Constants ***/

/* The object flags */
enum {
    #define OF(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) OF_##a,
    #include "list-object-flags.h"
    #undef OF
};

/* The object flag types */
enum {
    #define OFT(a, b, c) OFT_##a,
    #include "list-flag-types.h"
    #undef OFT
};

/* How object flags are IDd */
enum object_flag_id {
	OFID_NONE = 0,		/* never shown */
	OFID_NORMAL,		/* normal ID on use */
	OFID_TIMED,			/* obvious after time */
	OFID_WIELD			/* obvious on wield */
};

#define OF_SIZE                	FLAG_SIZE(OF_MAX)
#define OF_BYTES           		32  /* savefile bytes, i.e. 256 flags */

#define of_has(f, flag)        	flag_has_dbg(f, OF_SIZE, flag, #f, #flag)
#define of_next(f, flag)       	flag_next(f, OF_SIZE, flag)
#define of_is_empty(f)         	flag_is_empty(f, OF_SIZE)
#define of_is_full(f)          	flag_is_full(f, OF_SIZE)
#define of_is_inter(f1, f2)    	flag_is_inter(f1, f2, OF_SIZE)
#define of_is_subset(f1, f2)   	flag_is_subset(f1, f2, OF_SIZE)
#define of_is_equal(f1, f2)    	flag_is_equal(f1, f2, OF_SIZE)
#define of_on(f, flag)         	flag_on_dbg(f, OF_SIZE, flag, #f, #flag)
#define of_off(f, flag)        	flag_off(f, OF_SIZE, flag)
#define of_wipe(f)             	flag_wipe(f, OF_SIZE)
#define of_setall(f)           	flag_setall(f, OF_SIZE)
#define of_negate(f)           	flag_negate(f, OF_SIZE)
#define of_copy(f1, f2)        	flag_copy(f1, f2, OF_SIZE)
#define of_union(f1, f2)       	flag_union(f1, f2, OF_SIZE)
#define of_comp_union(f1, f2)  	flag_comp_union(f1, f2, OF_SIZE)
#define of_inter(f1, f2)       	flag_inter(f1, f2, OF_SIZE)
#define of_diff(f1, f2)        	flag_diff(f1, f2, OF_SIZE)


/*** Structures ***/

/**
 * The object flag structures
 */
struct object_flag {
	u16b index;				/* the OF_ index */
	bool pval;				/* is it granular (TRUE) or binary (FALSE) */
	u16b timed;				/* the corresponding TMD_ flag */
	u16b id;				/* how is it identified */
	u16b type;				/* OFT_ category */
	s16b power;				/* base power rating */
	s16b pval_mult;			/* pval weight rating */
	s16b weapon;			/* power mult for melee weapon */
	s16b bow;				/* power mult for launcher */
	s16b ring;				/* etc. ... */
	s16b amulet;
	s16b light;
	s16b body;
	s16b cloak;
	s16b shield;
	s16b hat;
	s16b gloves;
	s16b boots;
	const char *message;	/* id message */
};

struct object_flag_type {
	u16b index;				/* the OFT_ index */
	bool pval;				/* is it granular (TRUE) or binary (FALSE) */
	char *desc;				/* description */
};

/*** Functions ***/
bool cursed_p(bitflag *f);
void create_mask(bitflag *f, bool id, ...);
void flag_message(int flag, char *name);
s32b flag_power(int flag);
void log_flags(bitflag *f, ang_file *log_file);
const char *flag_name(int flag);
s16b slot_mult(int flag, int slot);
bool flag_uses_pval(int flag);
int obj_flag_type(int flag);
int pval_mult(int flag);
bool check_state(struct player *p, int flag, bitflag *f);
const char *obj_flagtype_name(int of_type);
void create_pval_mask(bitflag *f);

#endif /* !INCLUDED_OBJFLAG_H */
