/*
 * File: effects.h
 * Purpose: List of effect types
 *
 * Copyright (c) 2007 Andrew Sidwell
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


/* Function */
extern bool do_effect(object_type *o_ptr, bool *ident, int dir);


/*
 * If we're being included in the init1.c file, change definitions to avoid
 * needless duplication of lists...
 */
#if defined(LIST_STRINGS)
# define START				const char *effect_list[] = {
# define EFFECT(x, y, z)	#x,
# define END				};
#elif defined(MAKE_TABLE)
# define START				info_entry effects[] = {
# define EFFECT(x, y, z)	{ EF_##x, y, z },
# define END				};
#else
# define START				typedef enum {
# define EFFECT(x, y, z)	EF_##x,
# define END				EFFECT(MAX, FALSE, NULL) } effect_type;
#endif

START
	EFFECT(XXX,            FALSE, NULL)
	EFFECT(POISON,         FALSE, "poisons you for 1d10+10 turns")
	EFFECT(BLIND,          FALSE, "blinds you for 1d200+200 turns")
	EFFECT(SCARE,          FALSE, "induces fear in you for 1d10+10 turns")
	EFFECT(CONFUSE,        FALSE, "confuses you for ")
	EFFECT(HALLUC,         FALSE, "causes you to hallucinate")
	EFFECT(PARALYZE,       FALSE, "induces paralysis for 1d10+10 turns")
	EFFECT(LOSE_STR,       FALSE, "reduces your strength with damage 6d6")
	EFFECT(LOSE_STR2,      FALSE, "reduces your strength with damage 10d10")
	EFFECT(LOSE_CON,       FALSE, "reduces your constitution with damage 6d6")
	EFFECT(LOSE_CON2,      FALSE, "reduces your constitution with damage 10d10")
	EFFECT(LOSE_INT,       FALSE, "reduces your intelligence with damage 8d8")
	EFFECT(LOSE_WIS,       FALSE, "reduces your wisdom with damage 8d8")
	EFFECT(CURE_POISON,    FALSE, "neutralizes poison")
	EFFECT(CURE_BLINDNESS, FALSE, "cures blindness")
	EFFECT(CURE_PARANOIA,  FALSE, "removes your fear")
	EFFECT(CURE_CONFUSION, FALSE, "cures confusion")
	EFFECT(CW_SERIOUS,     FALSE, "restores 4d8 hit points")
	EFFECT(RESTORE_STR,    FALSE, "restores your strength")
	EFFECT(RESTORE_CON,    FALSE, "restores your constitution")
	EFFECT(RESTORE_ALL,    FALSE, "restores all your stats")
	EFFECT(FOOD_GOOD,      FALSE, "nourishes you")
	EFFECT(FOOD_WAYBREAD,  FALSE, "restores 4d8 hit points and neutralizes poison")
END

#undef START
#undef EFFECT
#undef END
