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

textblock *effect_describe(const struct effect *e, const char *prefix,
	int dev_skill_boost, bool only_first);
size_t effect_get_menu_name(char *buf, size_t max, const struct effect *e);
struct effect *effect_next(struct effect *effect);
bool effect_damages(const struct effect *effect);
int effect_avg_damage(const struct effect *effect);
const char *effect_projection(const struct effect *effect);

#endif /* !EFFECTS_INFO_H */
