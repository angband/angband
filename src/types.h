#ifndef INCLUDED_TYPES_H
#define INCLUDED_TYPES_H

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

/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct
{
	byte tval;
	const char *name;
} grouper;

#endif /* !INCLUDED_TYPES_H */
