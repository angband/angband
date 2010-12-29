/*
 * File: monster/constants.h
 * Purpose: magic numbers for monster spell attacks and player resistances
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

/*
 * These macros are used in three places:
 *
 * melee2.c  - to calculate the damage for monster spell and breath attacks
 * spells1.c - to apply resistances to reduce this damage
 * init1.c   - to evaluate the "power" of monsters
 *
 * The first two uses can involve randomness, but the third requires minimum
 * values (of resistance) and maxima (of damage). So we use z-rand.c's
 * damcalc() as a wrapper for damroll(), which allows us to obtain the values
 * we need in each case. All uses of randint1(x) have been replaced with
 * damroll(1, x) in order to pass cleanly through the wrapper.
 *
 * If you want to change any damage or resistance calculations, you only have
 * to edit this file, without touching init1.c or melee2.c or spells1.c
 *
 * This complexity does not apply to breath damage, because there is
 * no random element. But it also means that if you want to make breath
 * damage dependent on rlev (for example), you have more work to do.
 */

#define NOT_USED	/* to avoid confusion in spells1.c */

#define ARROW1_HIT				40
#define ARROW1_DMG(level, dam_aspect)		(damcalc(1, 6, (dam_aspect)))

#define ARROW2_HIT				40
#define ARROW2_DMG(level, dam_aspect)		(damcalc(3, 6, (dam_aspect)))

#define ARROW3_HIT				50
#define ARROW3_DMG(level, dam_aspect)		(damcalc(5, 6, (dam_aspect)))

#define ARROW4_HIT				50
#define ARROW4_DMG(level, dam_aspect)		(damcalc(7, 6, (dam_aspect)))

#define BR_ACID_MAX				1600
#define BR_ACID_DIVISOR				3
#define RES_ACID_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_ACID_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_ACID_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_ELEC_MAX				1600
#define BR_ELEC_DIVISOR				3
#define RES_ELEC_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_ELEC_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_ELEC_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_FIRE_MAX				1600
#define BR_FIRE_DIVISOR				3
#define RES_FIRE_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_FIRE_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_FIRE_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_COLD_MAX				1600
#define BR_COLD_DIVISOR				3
#define RES_COLD_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_COLD_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)
#define VULN_COLD_ADJ(dam, dam_aspect)		(((dam) * 4) / 3)

#define BR_POIS_MAX				800
#define BR_POIS_DIVISOR				3
#define RES_POIS_ADJ(dam, dam_aspect)		(((dam) + 2) / 3)
#define DBLRES_POIS_ADJ(dam, dam_aspect)	(((dam) + 8) / 9)

#define BR_NETH_MAX				550
#define BR_NETH_DIVISOR				6
#define RES_NETH_ADJ(dam, dam_aspect)		(((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_LIGHT_MAX				400
#define BR_LIGHT_DIVISOR			6
#define RES_LIGHT_ADJ(dam, dam_aspect)		(((dam) * 4) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_DARK_MAX				400
#define BR_DARK_DIVISOR				6
#define RES_DARK_ADJ(dam, dam_aspect)           (((dam) * 4) / (damcalc(1, 6, dam_aspect) + 6))
/* Confusion no longer used as an element, post-3.2 
#define BR_CONF_MAX				400
#define BR_CONF_DIVISOR				6
#define RES_CONF_ADJ(dam, dam_aspect)           (((dam) * 5) / (damcalc(1, 6, dam_aspect) + 6))
 */
#define BR_SOUN_MAX				500
#define BR_SOUN_DIVISOR				6
#define RES_SOUN_ADJ(dam, dam_aspect)           (((dam) * 5) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_CHAO_MAX				500
#define BR_CHAO_DIVISOR				6
#define RES_CHAO_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_DISE_MAX				500
#define BR_DISE_DIVISOR				6
#define RES_DISE_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_NEXU_MAX				400
#define BR_NEXU_DIVISOR				6
#define RES_NEXU_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_TIME_MAX				150
#define BR_TIME_DIVISOR				3
/* no resist */

#define BR_INER_MAX				200
#define BR_INER_DIVISOR				6
/* no resist */

#define BR_GRAV_MAX				200
#define BR_GRAV_DIVISOR				3
/* no resist */

#define BR_SHAR_MAX				500
#define BR_SHAR_DIVISOR				6
#define RES_SHAR_ADJ(dam, dam_aspect)           (((dam) * 6) / (damcalc(1, 6, dam_aspect) + 6))

#define BR_PLAS_MAX				150
#define BR_PLAS_DIVISOR				6
/* no resist */

#define BR_FORC_MAX				200
#define BR_FORC_DIVISOR				6
/* no resist */

/*
#define BR_MANA_MAX	1600
#define BR_MANA_DIVISOR 3
*/

#define BOULDER_HIT			60
#define BOULDER_DMG(level, dam_aspect)	(damcalc(1 + ((level) / 7), 12, (dam_aspect)))

#define BA_ACID_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3), (dam_aspect)) + 15)
#define BA_ELEC_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3 / 2), (dam_aspect)) + 8)
#define BA_FIRE_DMG(level, dam_aspect)	(damcalc(1, ((level) * 7 / 2), (dam_aspect)) + 10)
#define BA_COLD_DMG(level, dam_aspect)	(damcalc(1, ((level) * 3 / 2), (dam_aspect)) + 10)
#define BA_POIS_DMG(level, dam_aspect)	(damcalc(12, 2, (dam_aspect)))
#define BA_NETH_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + (level) + 50)
#define BA_WATE_DMG(level, dam_aspect)	(damcalc(1, ((level) * 5 / 2), (dam_aspect)) + 50)
#define BA_MANA_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + ((level) * 5))  /* manastorm */
#define BA_DARK_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + ((level) * 5))  /* darkness storm */

#define MIND_BLAST_DMG(level, dam_aspect)	(damcalc(8, 8, (dam_aspect)))
#define BRAIN_SMASH_DMG(level, dam_aspect)	(damcalc(12, 15, (dam_aspect)))

#define CAUSE1_DMG(level, dam_aspect)	(damcalc(3, 8, (dam_aspect)))
#define CAUSE2_DMG(level, dam_aspect)	(damcalc(8, 8, (dam_aspect)))
#define CAUSE3_DMG(level, dam_aspect)	(damcalc(10, 15, (dam_aspect)))
#define CAUSE4_DMG(level, dam_aspect)	(damcalc(15, 15, (dam_aspect)))
#define CAUSE4_CUT			damroll(10, 10)

#define BO_ACID_DMG(level, dam_aspect)	(damcalc(7, 8, (dam_aspect)) + ((level) / 3))
#define BO_ELEC_DMG(level, dam_aspect)	(damcalc(4, 8, (dam_aspect)) + ((level) / 3))
#define BO_FIRE_DMG(level, dam_aspect) 	(damcalc(9, 8, (dam_aspect)) + ((level) / 3))
#define BO_COLD_DMG(level, dam_aspect) 	(damcalc(6, 8, (dam_aspect)) + ((level) / 3))
/* #define BO_POIS_DMG(level, dam_aspect) 	(damcalc(9, 8, (dam_aspect)) + ((level) / 3)) */
#define BO_NETH_DMG(level, dam_aspect) 	(damcalc(5, 5, (dam_aspect)) + ((level) * 3 / 2) + 30)
#define BO_WATE_DMG(level, dam_aspect)	(damcalc(10, 10, (dam_aspect)) + (level))
#define BO_MANA_DMG(level, dam_aspect)	(damcalc(1, ((level) * 7 / 2), (dam_aspect)) + 50)
#define BO_PLAS_DMG(level, dam_aspect)	(damcalc(8, 7, (dam_aspect)) + (level) + 10)
#define BO_ICEE_DMG(level, dam_aspect)	(damcalc(6, 6, (dam_aspect)) + (level))
#define MISSILE_DMG(level, dam_aspect)	(damcalc(2, 6, (dam_aspect)) + ((level) / 3))
