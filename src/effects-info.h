/**
 * \file effects-info.h
 * \brief Declare interfaces for providing information about effects
 */

#ifndef EFFECTS_INFO_H
#define EFFECTS_INFO_H

#include "z-textblock.h"

struct effect;

/**
 * Flags for effect descriptions
 */
enum {
	EFINFO_NONE,
	EFINFO_DICE,
	EFINFO_HEAL,
	EFINFO_CONST,
	EFINFO_FOOD,
	EFINFO_CURE,
	EFINFO_TIMED,
	EFINFO_STAT,
	EFINFO_SEEN,
	EFINFO_SUMM,
	EFINFO_TELE,
	EFINFO_QUAKE,
	EFINFO_BALL,
	EFINFO_SPOT,
	EFINFO_BREATH,
	EFINFO_SHORT,
	EFINFO_LASH,
	EFINFO_BOLT,
	EFINFO_BOLTD,
	EFINFO_TOUCH
};

enum effect_object_property_kind {
	EFPROP_OBJECT_FLAG, /* provides an object flag temporarily */
	EFPROP_RESIST, /* provides a temporary elemental resist */
	EFPROP_CURE, /* cures a condition related to an object flag */
	EFPROP_CONFLICT, /* conflicts with an object flag */
	EFPROP_BRAND, /* provides a temporary brand */
	EFPROP_SLAY, /* provides a temporary slay */
};

struct effect_object_property {
	struct effect_object_property *next;
	union {
		/*
		 * Object flag provided temporarily and whether the effect
		 * only does that or also does other things (heroism for
		 * example)
		 */
		struct { int flag; bool syn; } temp_flag;
		/* Element index for the resist */
		int temp_resist;
		/* The object flag providing resistance to what's cured */
		int cure_flag;
		int conflict_flag;
		int temp_brand;
		int temp_slay;
	} prop;
	enum effect_object_property_kind kind;
};

textblock *effect_describe(const struct effect *e, const char *prefix,
	int dev_skill_boost, bool only_first);
size_t effect_get_menu_name(char *buf, size_t max, const struct effect *e);
struct effect *effect_next(struct effect *effect);
bool effect_damages(const struct effect *effect);
int effect_avg_damage(const struct effect *effect);
const char *effect_projection(const struct effect *effect);
struct effect_object_property *effect_summarize_properties(
	const struct effect *ef, int *unsummarized_count);

#endif /* !EFFECTS_INFO_H */
