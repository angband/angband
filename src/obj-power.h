/**
 * \file obj-power.h
 * \brief calculation of object power and value
 *
 * Copyright (c) 2001 Chris Carr, Chris Robertson
 * Revised in 2009-11 by Chris Carr, Peter Denison
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

#ifndef OBJECT_POWER_H
#define OBJECT_POWER_H

/**
 * Some constants used in randart generation and power calculation
 * - thresholds for limiting to_hit, to_dam and to_ac
 * - fudge factor for rescaling ammo cost
 * (a stack of this many equals a weapon of the same damage output)
 */
#define INHIBIT_POWER		20000
#define INHIBIT_BLOWS		3
#define INHIBIT_MIGHT		4
#define INHIBIT_SHOTS		5
#define HIGH_TO_AC			26
#define VERYHIGH_TO_AC		36
#define INHIBIT_AC			56
#define HIGH_TO_HIT			16
#define VERYHIGH_TO_HIT		26
#define HIGH_TO_DAM			16
#define VERYHIGH_TO_DAM		26
#define AMMO_RESCALER		20 /* this value is also used for torches */


enum power_calc_operation {
	POWER_CALC_NONE,
	POWER_CALC_ADD,
	POWER_CALC_ADD_IF_POSITIVE,
	POWER_CALC_SQUARE_ADD_IF_POSITIVE,
	POWER_CALC_MULTIPLY,
	POWER_CALC_DIVIDE,
	POWER_CALC_MAX
};

/*** Structures ***/

struct iterate {
	int property_type;
	int max;
};

struct power_calc {
	struct power_calc *next;
	char *name;			/**< Name of the calculation */
	struct poss_item *poss_items;
	dice_t *dice;		/**< Dice expression used in the calculation */
	int operation;		/**< How the calculation operates on power */
	struct iterate iterate;	/**< What the calculation iterates over */
	char *apply_to;		/**< What the calculation is applied to */
};


extern struct power_calc *calculations;

/*** Functions ***/

extern expression_base_value_f power_calculation_by_name(const char *name);

int object_power(const struct object *obj, bool verbose, ang_file *log_file);
int object_value_real(const struct object *obj, int qty);
int object_value(const struct object *obj, int qty);


#endif /* OBJECT_POWER_H */
