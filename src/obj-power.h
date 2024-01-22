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
 * Constants for the power algorithm:
 * - fudge factor for extra damage from rings etc. (used if extra blows)
 * - assumed damage for off-weapon brands
 * - base power for jewelry
 * - base power for armour items (for halving acid damage)
 * - power per point of damage
 * - power per point of +to_hit
 * - power per point of base AC
 * - power per point of +to_ac
 * (these four are all halved in the algorithm)
 * - assumed max blows
 * - inhibiting values for +blows/might/shots/immunities (max is one less)
 * - power per 10th pound difference from standard weight (less than adds power;
 *   more than subtracts) for something that does not provide base AC
 * - power per 10th pound difference from standard weight (more than adds power;
 *   less than subtracts) for something with the THROWING flag
 */
#define NONWEAP_DAMAGE   		15 /* fudge to boost extra blows */
#define WEAP_DAMAGE				12 /* and for off-weapon combat flags */
#define BASE_JEWELRY_POWER		 4
#define BASE_ARMOUR_POWER		 1
#define DAMAGE_POWER             5 /* i.e. 2.5 */
#define TO_HIT_POWER             3 /* i.e. 1.5 */
#define BASE_AC_POWER            2 /* i.e. 1 */
#define TO_AC_POWER              2 /* i.e. 1 */
#define MAX_BLOWS                5
#define WGT_POWER_NUM_NOBASEAC   1
#define WGT_POWER_DEN_NOBASEAC   50 /* i.e. 5 pounds adds 1 point of power */
#define WGT_POWER_NUM_THROW      15
#define WGT_POWER_DEN_THROW      12 /* for non-O, throwing multiplier is 2
					+ weight / 12; shooting power multiplier
					for power calculations is 30; throwing
					is typically useful than shooting so use
					15 * (weight difference / 12) as the
					power change */

/**
 * Some constants used in randart generation and power calculation
 * - thresholds for limiting to_hit, to_dam and to_ac
 * - fudge factor for rescaling ammo cost
 * (a stack of this many equals a weapon of the same damage output)
 */
#define INHIBIT_POWER		20000
#define INHIBIT_BLOWS		3
#define INHIBIT_MIGHT		4
#define INHIBIT_SHOTS		21
#define HIGH_TO_AC			26
#define VERYHIGH_TO_AC		36
#define INHIBIT_AC			56
#define HIGH_TO_HIT			16
#define VERYHIGH_TO_HIT		26
#define HIGH_TO_DAM			16
#define VERYHIGH_TO_DAM		26
#define AMMO_RESCALER		20 /* this value is also used for torches */



/*** Functions ***/

int32_t object_power(const struct object *obj, bool verbose, ang_file *log_file);
int object_value_real(const struct object *obj, int qty);
int object_value(const struct object *obj, int qty);


#endif /* OBJECT_POWER_H */
