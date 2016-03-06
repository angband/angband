/**
   \file obj-properties.h
   \brief definitions and functions for object flags and modifiers
 *
 * Copyright (c) 2014 Chris Carr, Nick McConnell
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
#ifndef INCLUDED_OBJPROPERTIES_H
#define INCLUDED_OBJPROPERTIES_H

#include "z-file.h"
#include "z-bitflag.h"

/**
 * ------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */

/**
 * The object flags
 */
enum {
	OF_NONE,
	#define STAT(a, b, c, d, e, f, g, h, i) OF_##c,
    #include "list-stats.h"
    #undef STAT
	#define OF(a, b, c, d, e, f) OF_##a,
    #include "list-object-flags.h"
    #undef OF
};

/**
 * The object kind flags
 */
enum {
    #define KF(a, b) KF_##a,
    #include "list-kind-flags.h"
    #undef KF
};

/**
 * The object modifiers
 */
enum {
	#define STAT(a, b, c, d, e, f, g, h, i) OBJ_MOD_##a,
    #include "list-stats.h"
    #undef STAT
    #define OBJ_MOD(a, b, c, d) OBJ_MOD_##a,
    #include "list-object-modifiers.h"
    #undef OBJ_MOD
	OBJ_MOD_MAX
};

/* Where the stats start in modifiers */
#define OBJ_MOD_MIN_STAT OBJ_MOD_STR

/**
 * The object flag types
 */
enum object_flag_type {
	OFT_NONE = 0,	/* placeholder flag */
	OFT_SUST,		/* sustains a stat */
	OFT_PROT,		/* protection from an effect */
	OFT_MISC,		/* a good property, suitable for ego items */
	OFT_LIGHT,		/* applicable only to light sources */
	OFT_MELEE,		/* applicable only to melee weapons */
	OFT_CURSE,		/* a "sticky" curse */
	OFT_BAD,		/* an undesirable flag that isn't a curse */

	OFT_MAX
};

/**
 * How object flags are IDd
 */
enum object_flag_id {
	OFID_NONE = 0,		/* never shown */
	OFID_NORMAL,		/* normal ID on use */
	OFID_TIMED,			/* obvious after time */
	OFID_WIELD			/* obvious on wield */
};

#define OF_SIZE                	FLAG_SIZE(OF_MAX)

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

#define KF_SIZE                	FLAG_SIZE(KF_MAX)

#define kf_has(f, flag)        	flag_has_dbg(f, KF_SIZE, flag, #f, #flag)
#define kf_next(f, flag)       	flag_next(f, KF_SIZE, flag)
#define kf_is_empty(f)         	flag_is_empty(f, KF_SIZE)
#define kf_is_full(f)          	flag_is_full(f, KF_SIZE)
#define kf_is_inter(f1, f2)    	flag_is_inter(f1, f2, KF_SIZE)
#define kf_is_subset(f1, f2)   	flag_is_subset(f1, f2, KF_SIZE)
#define kf_is_equal(f1, f2)    	flag_is_equal(f1, f2, KF_SIZE)
#define kf_on(f, flag)         	flag_on_dbg(f, KF_SIZE, flag, #f, #flag)
#define kf_off(f, flag)        	flag_off(f, KF_SIZE, flag)
#define kf_wipe(f)             	flag_wipe(f, KF_SIZE)
#define kf_setall(f)           	flag_setall(f, KF_SIZE)
#define kf_negate(f)           	flag_negate(f, KF_SIZE)
#define kf_copy(f1, f2)        	flag_copy(f1, f2, KF_SIZE)
#define kf_union(f1, f2)       	flag_union(f1, f2, KF_SIZE)
#define kf_comp_union(f1, f2)  	flag_comp_union(f1, f2, KF_SIZE)
#define kf_inter(f1, f2)       	flag_inter(f1, f2, KF_SIZE)
#define kf_diff(f1, f2)        	flag_diff(f1, f2, KF_SIZE)


/**
 * ------------------------------------------------------------------------
 * Structures
 * ------------------------------------------------------------------------ */

/**
 * The object flag structure
 */
struct object_flag {
	u16b index;				/* the OF_ index */
	u16b id;				/* how is it identified */
	u16b type;				/* OFT_ category */
	s16b power;				/* base power rating */
	const char *message;	/* id message */
};

/**
 * The object modifier structure
 */
struct object_mod {
	u16b index;				/* the OBJ_MOD_ index */
	s16b power;				/* base power rating */
	s16b mod_mult;			/* modifier weight rating */
	const char *name;		/* id message */
};


/**
 * ------------------------------------------------------------------------
 * Functions
 * ------------------------------------------------------------------------ */
bool cursed_p(const bitflag *f);
void create_mask(bitflag *f, bool id, ...);
s32b flag_power(int flag);
void log_flags(bitflag *f, ang_file *log_file);
const char *flag_name(int flag);
s16b flag_slot_mult(int flag, int slot);
int obj_flag_type(int flag);
void flag_message(int flag, char *name);
int sustain_flag(int stat);
const char *mod_name(int mod);
s32b mod_power(int mod);
int mod_mult(int mod);
s16b mod_slot_mult(int mod, int slot);

#endif /* !INCLUDED_OBJPROPERTIES_H */
