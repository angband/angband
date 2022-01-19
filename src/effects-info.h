/**
 * \file effects-info.h
 * \brief Declare interfaces for providing information about effects
 */

#ifndef EFFECTS_INFO_H
#define EFFECTS_INFO_H

#include "z-dice.h"
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
	EFPROP_OBJECT_FLAG_EXACT, /* provides an object flag temporarily */
	EFPROP_OBJECT_FLAG, /* provides an object flag (and something more) temporarily */
	EFPROP_RESIST, /* provides a temporary elemental resist */
	EFPROP_CURE_FLAG, /* cures a condition that's avoided by an object flag */
	EFPROP_CURE_RESIST, /* cures a condition that's avoided by a resist */
	EFPROP_CONFLICT_FLAG, /* conflicts with an object flag */
	EFPROP_CONFLICT_RESIST, /* conflicts with an elemental resist */
	EFPROP_CONFLICT_VULN, /* conflicts with an elemental vulnerability */
	EFPROP_BRAND, /* provides a temporary brand */
	EFPROP_SLAY, /* provides a temporary slay */
};

struct effect_object_property {
	struct effect_object_property *next;
	/*
	 * Is an object flag index for EFPROP_OBJECT_FLAG_EXACT,
	 * EFPROP_OBJECT_FLAG, EFPROP_CURE_FLAG, or EFPROP_CONFLICT_FLAG.  Is
	 * an element index for EFPROP_RESIST, EFPROP_CURE_RESIST,
	 * EFPROP_CONFLICT_RESIST, or EFPROP_CONFLICT_VULN.  Is a brand index
	 * for EFPROP_BRAND.  Is a slay index for EFPROP_SLAY.
	 */
	int idx;
	/*
	 * These are only relevant for EFPROP_RESIST, EFPROP_CURE_RESIST,
	 * EFPROP_CONFLICT_RESIST, or EFPROP_CONFLICT_VULN.  They are the
	 * inclusive range for resistant levels in the element that are
	 * compatible with the effect.
	 */
	int reslevel_min, reslevel_max;
	enum effect_object_property_kind kind;
};

textblock *effect_describe(const struct effect *e, const char *prefix,
	int dev_skill_boost, bool only_first);
size_t effect_get_menu_name(char *buf, size_t max, const struct effect *e);
struct effect *effect_next(struct effect *effect);
bool effect_damages(const struct effect *effect);
int effect_avg_damage(const struct effect *effect, dice_t *shared_dice);
const char *effect_projection(const struct effect *effect);
struct effect_object_property *effect_summarize_properties(
	const struct effect *ef, int *unsummarized_count);

#endif /* !EFFECTS_INFO_H */
