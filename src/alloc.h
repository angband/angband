#ifndef INCLUDED_ALLOC_H
#define INCLUDED_ALLOC_H

/*
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */
typedef struct alloc_entry
{
	int index;		/* The actual index */

	int level;		/* Base dungeon level */
	int prob1;		/* Probability, pass 1 */
	int prob2;		/* Probability, pass 2 */
	int prob3;		/* Probability, pass 3 */
} alloc_entry;

#endif
