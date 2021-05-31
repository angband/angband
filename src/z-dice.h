/**
   \file z-dice.h
   \brief Represent more complex dice than random_value
 *
 * Copyright (c) 2013 Ben Semmler
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

#ifndef INCLUDED_Z_DICE_H
#define INCLUDED_Z_DICE_H

#include "h-basic.h"

#include "z-rand.h"
#include "z-expression.h"

typedef struct dice_s dice_t;

dice_t *dice_new(void);
void dice_free(dice_t *dice);
bool dice_parse_string(dice_t *dice, const char *string);
int dice_bind_expression(dice_t *dice, const char *name,
						 const expression_t *expression);
void dice_random_value(dice_t *dice, random_value *v);
int dice_evaluate(dice_t *dice, int level, aspect asp, random_value *v);
int dice_roll(dice_t *dice, random_value *v);
bool dice_test_values(dice_t *dice, int base, int dice_count, int sides,
					  int bonus);
bool dice_test_variables(dice_t *dice, const char *base, const char *dice_name,
						 const char *sides, const char *bonus);

#endif /* INCLUDED_Z_DICE_H */
