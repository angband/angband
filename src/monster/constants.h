/*
 * File: monster/constants.h
 * Purpose: magic numbers for monster spell attacks
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2009 Chris Carr
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

#define ARROW1_HIT	40
#define ARROW1_DMG	damroll(1, 6)

#define ARROW2_HIT	40
#define ARROW2_DMG	damroll(3, 6)

#define ARROW3_HIT	50
#define ARROW3_DMG	damroll(5, 6)

#define ARROW4_HIT	50
#define ARROW4_DMG	damroll(7, 6)

#define BR_ACID_MAX	1600
#define BR_ACID_DIVISOR 3

#define BR_ELEC_MAX	1600
#define BR_ELEC_DIVISOR 3

#define BR_FIRE_MAX	1600
#define BR_FIRE_DIVISOR 3

#define BR_COLD_MAX	1600
#define BR_COLD_DIVISOR 3

#define BR_POIS_MAX	800
#define BR_POIS_DIVISOR 3

#define BR_NETH_MAX	550
#define BR_NETH_DIVISOR 6

#define BR_LITE_MAX	400
#define BR_LITE_DIVISOR 6

#define BR_DARK_MAX	400
#define BR_DARK_DIVISOR 6

#define BR_CONF_MAX	400
#define BR_CONF_DIVISOR 6

#define BR_SOUN_MAX	500
#define BR_SOUN_DIVISOR 6

#define BR_CHAO_MAX	500
#define BR_CHAO_DIVISOR 6

#define BR_DISE_MAX	500
#define BR_DISE_DIVISOR 6

#define BR_NEXU_MAX	400
#define BR_NEXU_DIVISOR 6

#define BR_TIME_MAX	150
#define BR_TIME_DIVISOR 3

#define BR_INER_MAX	200
#define BR_INER_DIVISOR 6

#define BR_GRAV_MAX	200
#define BR_GRAV_DIVISOR 3

#define BR_SHAR_MAX	500
#define BR_SHAR_DIVISOR 6

#define BR_PLAS_MAX	150
#define BR_PLAS_DIVISOR 6

#define BR_FORC_MAX	200
#define BR_FORC_DIVISOR 6

/*
#define BR_MANA_MAX	1600
#define BR_MANA_DIVISOR 3
*/

#define BOULDER_HIT 	60
#define BOULDER_DMG	damroll(1 + r_ptr->level / 7, 12)

#define BA_ACID_DMG	randint1(rlev * 3) + 15
#define BA_ELEC_DMG	randint1(rlev * 3 / 2) + 8
#define BA_FIRE_DMG	randint1(rlev * 7 / 2) + 10
#define BA_COLD_DMG	randint1(rlev * 3 / 2) + 10
#define BA_POIS_DMG	damroll(12, 2)
#define BA_NETH_DMG	(50 + damroll(10, 10) + rlev)
#define BA_WATE_DMG	randint1(rlev * 5 / 2) + 50
#define BA_MANA_DMG	(rlev * 5) + damroll(10, 10)   /* manastorm */
#define BA_DARK_DMG	(rlev * 5) + damroll(10, 10)   /* darkness storm */

#define MIND_BLAST_DMG	damroll(8, 8)
#define BRAIN_SMASH_DMG damroll(12, 15)

#define CAUSE1_DMG	damroll(3, 8)
#define CAUSE2_DMG	damroll(8, 8)
#define CAUSE3_DMG	damroll(10, 15)
#define CAUSE4_DMG	damroll(15, 15)
#define CAUSE4_CUT	damroll(10, 10)

#define BO_ACID_DMG	damroll(7, 8) + (rlev / 3)
#define BO_ELEC_DMG	damroll(4, 8) + (rlev / 3)
#define BO_FIRE_DMG 	damroll(9, 8) + (rlev / 3)
#define BO_COLD_DMG 	damroll(6, 8) + (rlev / 3)
/* #define BO_POIS_DMG 	damroll(9, 8) + (rlev / 3) */
#define BO_NETH_DMG 	30 + damroll(5, 5) + (rlev / 3) / 2
#define BO_WATE_DMG	damroll(10, 10) + (rlev)
#define BO_MANA_DMG	randint1(rlev * 7 / 2) + 50
#define BO_PLAS_DMG	10 + damroll(8, 7) + (rlev)
#define BO_ICEE_DMG	damroll(6, 6) + (rlev)
#define MISSILE_DMG	damroll(2, 6) + (rlev / 3)

