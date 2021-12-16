/**
   \file z-expression.h
   \brief Creating, storing, and deserializing simple math expressions
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

#ifndef INCLUDED_Z_EXPRESSION_H
#define INCLUDED_Z_EXPRESSION_H

#include "h-basic.h"

enum expression_err_e {
	EXPRESSION_ERR_GENERIC = -1,
	EXPRESSION_ERR_INVALID_OPERATOR = -2,
	EXPRESSION_ERR_EXPECTED_OPERATOR = -3,
	EXPRESSION_ERR_EXPECTED_OPERAND = -4,
	EXPRESSION_ERR_DIVIDE_BY_ZERO = -5,
};

typedef struct expression_operation_s expression_operation_t;
typedef struct expression_s expression_t;
typedef int32_t (*expression_base_value_f)(void);

expression_t *expression_new(void);
void expression_free(expression_t *expression);
expression_t *expression_copy(const expression_t *source);
void expression_set_base_value(expression_t *expression,
							   expression_base_value_f function);
int32_t expression_evaluate(expression_t const * const expression);
int16_t expression_add_operations_string(expression_t *expression,
									  const char *string);
bool expression_test_copy(const expression_t *a, const expression_t *b);

#endif /* INCLUDED_Z_EXPRESSION_H */
