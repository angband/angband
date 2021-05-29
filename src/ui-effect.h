/**
 * \file ui-effect.h
 * \brief Declarations for the text UI's handling of effects
 */

#ifndef INCLUDED_UI_EFFECT_H
#define INCLUDED_UI_EFFECT_H

struct effect;
int textui_get_effect_from_list(const char *prompt, struct effect *effect,
	int count, bool allow_random);

#endif /* INCLUDED_UI_EFFECT_H */
