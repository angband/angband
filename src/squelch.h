#ifndef SQUELCH_H
#define SQUELCH_H

/*
 * Used for mapping the values below to names.
 */
typedef struct
{
	unsigned int enum_val;
	const char *name;
} quality_name_struct;

/*
 * List of kinds of item, for pseudo-id squelch.
 */
typedef enum
{
	#define ITYPE(a, b) ITYPE_##a,
	#include "list-ignore-types.h"
	#undef ITYPE

	ITYPE_MAX
} squelch_type_t;


/*
 * The different kinds of quality squelch
 */
enum
{
	SQUELCH_NONE,
	SQUELCH_BAD,
	SQUELCH_AVERAGE,
	SQUELCH_GOOD,
	SQUELCH_EXCELLENT_NO_HI,
	SQUELCH_EXCELLENT_NO_SPL,
	SQUELCH_ALL,

	SQUELCH_MAX
};


/**
 * Structure to describe ego item short name. 
 */
typedef struct ego_desc
{
  s16b e_idx;
  const char *short_name;
} ego_desc;

/*
 * Squelch flags
 */
#define SQUELCH_IF_AWARE	0x01
#define SQUELCH_IF_UNAWARE	0x02



extern quality_name_struct quality_values[SQUELCH_MAX];
extern quality_name_struct quality_choices[ITYPE_MAX];


/* squelch.c */
void squelch_birth_init(void);
const char *get_autoinscription(object_kind *kind);
int apply_autoinscription(object_type *o_ptr);
int remove_autoinscription(s16b kind);
int add_autoinscription(s16b kind, const char *inscription);
void autoinscribe_ground(void);
void autoinscribe_pack(void);
void object_squelch_flavor_of(const object_type *o_ptr);
squelch_type_t squelch_type_of(const object_type *o_ptr);
byte squelch_level_of(const object_type *o_ptr);
void kind_squelch_clear(object_kind *k_ptr);
void ego_squelch_clear(struct ego_item *ego);
bool kind_is_squelched_aware(const object_kind *k_ptr);
bool kind_is_squelched_unaware(const object_kind *k_ptr);
void kind_squelch_when_aware(object_kind *k_ptr);
void kind_squelch_when_unaware(object_kind *k_ptr);
bool object_is_squelched(const object_type *o_ptr);
bool squelch_item_ok(const object_type *o_ptr);
void squelch_drop(void);
const char *squelch_name_for_type(squelch_type_t type);

extern byte squelch_level[];
extern const size_t squelch_size;

/* ui-options.c */
int ego_item_name(char *buf, size_t buf_size, ego_desc *desc);

#endif /* !SQUELCH_H */
