#ifndef INCLUDED_TYPES_H
#define INCLUDED_TYPES_H

/*
 * This file contains various defined types used by the game.
 *
 * Be careful when creating data structures; most of these are designed
 * to be serialised to file, so be careful to use exact-size data types
 * (like u32b and s32b) and not just "int"s.
 */

#include "z-term.h"

/**** Available Structs ****/

typedef struct quest quest;
typedef struct color_type color_type;


/**
 * Information about maximal indices of certain arrays.
 *
 * These are actually not the maxima, but the maxima plus one, because of
 * 0-based indexing issues.
 */
typedef struct maxima
{
	u16b f_max;       /**< Maximum number of terrain features */
	u16b k_max;       /**< Maximum number of object base kinds */
	u16b a_max;       /**< Maximum number of artifact kinds */
	u16b e_max;       /**< Maximum number of ego-item kinds */
	u16b r_max;       /**< Maximum number of monster races */
	u16b mp_max;	  /**< Maximum number of monster pain message sets */
	u16b s_max;       /**< Maximum number of magic spells */
	u16b pit_max;	  /**< Maximum number of monster pit types */

	u16b o_max;       /**< Maximum number of objects on a given level */
	u16b m_max;       /**< Maximum number of monsters on a given level */
} maxima;


/*
 * Structure for the "quests"
 *
 * Hack -- currently, only the "level" parameter is set, with the
 * semantics that "one (QUEST) monster of that level" must be killed,
 * and then the "level" is reset to zero, meaning "all done".  Later,
 * we should allow quests like "kill 100 fire hounds", and note that
 * the "quest level" is then the level past which progress is forbidden
 * until the quest is complete.  Note that the "QUESTOR" flag then could
 * become a more general "never out of depth" flag for monsters.
 */
struct quest
{
	byte level;		/* Dungeon level */
	int r_idx;		/* Monster race */

	int cur_num;	/* Number killed (unused) */
	int max_num;	/* Number required (unused) */
};


/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct
{
	byte tval;
	const char *name;
} grouper;

/*
 * A game color.
 */
struct color_type
{
	char index_char;            /* Character index:  'r' = red, etc. */
	char name[32];              /* Color name */
	byte color_translate[MAX_ATTR];       /* Index for various in-game translations */
};

/*
 * A hint.
 */
struct hint
{
        char *hint;
        struct hint *next;
};


#endif /* !INCLUDED_TYPES_H */
