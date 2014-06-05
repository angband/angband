/**
   \file obj-ignore.h
   \brief Item ignoring
 *
 * Copyright (c) 2007 David T. Blackston, Iain McFall, DarkGod, Jeff Greene,
 * David Vestal, Pete Mack, Andi Sidwell.
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
#ifndef OBJ_IGNORE_H
#define OBJ_IGNORE_H

/*
 * Used for mapping the values below to names.
 */
typedef struct
{
	unsigned int enum_val;
	const char *name;
} quality_name_struct;

/*
 * List of kinds of item, for pseudo-id and ego ignore.
 */
typedef enum
{
	#define ITYPE(a, b) ITYPE_##a,
	#include "list-ignore-types.h"
	#undef ITYPE

	ITYPE_MAX
} ignore_type_t;


#define ITYPE_SIZE              FLAG_SIZE(ITYPE_MAX)

#define itype_has(f, flag)        	flag_has_dbg(f, ITYPE_SIZE, flag, #f, #flag)
#define itype_on(f, flag)         	flag_on_dbg(f, ITYPE_SIZE, flag, #f, #flag)
#define itype_wipe(f)             	flag_wipe(f, ITYPE_SIZE)

/*
 * The different kinds of quality ignore
 */
enum
{
	IGNORE_NONE,
	IGNORE_BAD,
	IGNORE_AVERAGE,
	IGNORE_GOOD,
	IGNORE_EXCELLENT_NO_HI,
	IGNORE_EXCELLENT_NO_SPL,
	IGNORE_ALL,

	IGNORE_MAX
};


/**
 * Structure to describe ego item short name. 
 */
typedef struct ego_desc
{
  s16b e_idx;
  u16b itype;
  const char *short_name;
} ego_desc;

/*
 * Ignore flags
 */
#define IGNORE_IF_AWARE	0x01
#define IGNORE_IF_UNAWARE	0x02



extern quality_name_struct quality_values[IGNORE_MAX];
extern quality_name_struct quality_choices[ITYPE_MAX];
bool **ego_ignore_types;


/* obj-ignore.c */
void ignore_birth_init(void);
const char *get_autoinscription(object_kind *kind);
int apply_autoinscription(object_type *o_ptr);
int remove_autoinscription(s16b kind);
int add_autoinscription(s16b kind, const char *inscription);
void autoinscribe_ground(void);
void autoinscribe_pack(void);
void object_ignore_flavor_of(const object_type *o_ptr);
ignore_type_t ignore_type_of(const object_type *o_ptr);
byte ignore_level_of(const object_type *o_ptr);
bool tval_has_ignore_type(int tval, ignore_type_t itype);
void kind_ignore_clear(object_kind *k_ptr);
void ego_ignore(struct object *obj);
void ego_ignore_clear(struct object *obj);
void ego_ignore_toggle(int e_idx, int itype);
bool ego_is_ignored(int e_idx, int itype);
bool kind_is_ignored_aware(const object_kind *k_ptr);
bool kind_is_ignored_unaware(const object_kind *k_ptr);
void kind_ignore_when_aware(object_kind *k_ptr);
void kind_ignore_when_unaware(object_kind *k_ptr);
bool object_is_ignored(const object_type *o_ptr);
bool ignore_item_ok(const object_type *o_ptr);
void ignore_drop(void);
const char *ignore_name_for_type(ignore_type_t type);

extern byte ignore_level[];
extern const size_t ignore_size;

/* ui-options.c */
int ego_item_name(char *buf, size_t buf_size, ego_desc *desc);

#endif /* !OBJ_IGNORE_H */
