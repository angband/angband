/**
 * \file z-expression.c
 * \brief Creating, storing, and deserializing simple math expressions
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

#include "z-expression.h"
#include "z-virt.h"
#include "z-util.h"

struct expression_operation_s {
	uint8_t operator;
	int16_t operand;
};

struct expression_s {
	expression_base_value_f base_value;
	size_t operation_count;
	size_t operations_size;
	expression_operation_t *operations;
};

/**
 * Operator types.
 */
typedef enum expression_operator_e {
	OPERATOR_NONE,
	OPERATOR_ADD,
	OPERATOR_SUB,
	OPERATOR_MUL,
	OPERATOR_DIV,
	OPERATOR_NEG,
} expression_operator_t;

/**
 * States for parser state table.
 */
typedef enum expression_state_e {
	EXPRESSION_STATE_START,
	EXPRESSION_STATE_OPERATOR,
	EXPRESSION_STATE_OPERAND,
	EXPRESSION_STATE_MAX,
} expression_state_t;

/**
 * Input types for parser state table.
 */
typedef enum expression_input_e {
	EXPRESSION_INPUT_INVALID,
	EXPRESSION_INPUT_NEEDS_OPERANDS,
	EXPRESSION_INPUT_UNARY_OPERATOR,
	EXPRESSION_INPUT_VALUE,
	EXPRESSION_INPUT_MAX,
} expression_input_t;

/**
 * Allocation block size for the operations array in expression_t.
 */
#define EXPRESSION_ALLOC_SIZE 5

/**
 * Token delimiter for the parser.
 */
#define EXPRESSION_DELIMITER " "

/**
 * Maximum number of operations in an expression. This number is used in the
 * parser to allocate an array of expression_operation_t.
 */
#define EXPRESSION_MAX_OPERATIONS 50

/**
 * Return an operator type based on the input token.
 */
static expression_operator_t expression_operator_from_token(const char *token)
{
	switch (token[0]) {
		case '+':
			return OPERATOR_ADD;
		case '-':
			return OPERATOR_SUB;
		case '*':
			return OPERATOR_MUL;
		case '/':
			return OPERATOR_DIV;
		case 'n':
		case 'N':
			return OPERATOR_NEG;
	}

	return OPERATOR_NONE;
}

/**
 * Return the state table input type for a given operator type.
 */
static expression_input_t expression_input_for_operator(expression_operator_t operator)
{
	switch (operator) {
		case OPERATOR_NONE:
			return EXPRESSION_INPUT_INVALID;
		case OPERATOR_ADD:
		case OPERATOR_SUB:
		case OPERATOR_MUL:
		case OPERATOR_DIV:
			return EXPRESSION_INPUT_NEEDS_OPERANDS;
		case OPERATOR_NEG:
			return EXPRESSION_INPUT_UNARY_OPERATOR;
	}

	return EXPRESSION_INPUT_INVALID;
}

/**
 * Allocate and initialize a new expression object. Returns NULL if it was
 * unable to be created.
 */
expression_t *expression_new(void)
{
	expression_t *expression = mem_zalloc(sizeof(expression_t));

	if (expression == NULL)
		return NULL;

	expression->base_value = NULL;
	expression->operation_count = 0;
	expression->operations_size = EXPRESSION_ALLOC_SIZE;
	expression->operations = mem_zalloc(expression->operations_size *
										sizeof(expression_operation_t));

	if (expression->operations == NULL) {
		mem_free(expression);
		return NULL;
	}

	return expression;
}

/**
 * Deallocate an expression object.
 */
void expression_free(expression_t *expression)
{
	if (expression == NULL)
		return;

	if (expression->operations != NULL) {
		mem_free(expression->operations);
		expression->operations = NULL;
	}

	mem_free(expression);
}

/**
 * Return a deep copy of the given expression.
 */
expression_t *expression_copy(const expression_t *source)
{
	size_t i;
	expression_t *copy = mem_zalloc(sizeof(expression_t));

	if (copy == NULL)
		return NULL;

	copy->base_value = source->base_value;
	copy->operation_count = source->operation_count;
	copy->operations_size = source->operations_size;

	if (copy->operations_size == 0) {
		copy->operations = NULL;
		return copy;
	}

	copy->operations = mem_zalloc(copy->operations_size *
								  sizeof(expression_operation_t));

	if (copy->operations == NULL && source->operations != NULL) {
		mem_free(copy);
		return NULL;
	}

	for (i = 0; i < copy->operation_count; i++) {
		copy->operations[i].operand = source->operations[i].operand;
		copy->operations[i].operator = source->operations[i].operator;
	}

	return copy;
}

/**
 * Set the base value function that the operations operate on.
 */
void expression_set_base_value(expression_t *expression,
							   expression_base_value_f function)
{
	expression->base_value = function;
}

/**
 * Evaluate the given expression. If the base value function is NULL,
 * expression is evaluated from zero.
 */
int32_t expression_evaluate(expression_t const * const expression)
{
	size_t i;
	int32_t value = 0;

	if (expression->base_value != NULL)
		value = expression->base_value();

	for (i = 0; i < expression->operation_count; i++) {
		switch (expression->operations[i].operator) {
			case OPERATOR_ADD:
				value += expression->operations[i].operand;
				break;
			case OPERATOR_SUB:
				value -= expression->operations[i].operand;
				break;
			case OPERATOR_MUL:
				value *= expression->operations[i].operand;
				break;
			case OPERATOR_DIV:
				value /= expression->operations[i].operand;
				break;
			case OPERATOR_NEG:
				value = -value;
				break;
			default:
				break;
		}
	}

	return value;
}

/**
 * Add an operation to an expression, allocating more memory as needed.
 */
static void expression_add_operation(expression_t *expression,
									 const expression_operation_t operation)
{
	if (expression->operation_count >= expression->operations_size) {
		expression->operations_size += EXPRESSION_ALLOC_SIZE;
		expression->operations = mem_realloc(expression->operations, expression->operations_size * sizeof(expression_operation_t));
	}

	expression->operations[expression->operation_count] = operation;
	expression->operation_count++;
}

/**
 * Parse a string and add operations and operands to an expression.
 *
 * The string must be in prefix notation and must start with an operator.
 * Basic operators (add, subtract, multiply, and divide) can have multiple
 * operands after the operator. Unary operators (negation) must be followed
 * by another operator. Parsing is done using a state table which is
 * contained in the function.
 *
 * \param expression is an initialized expression object.
 * \param string is the string to be parsed.
 * \return The number of operations added to the expression or an error (expression_err_e).
 */
int16_t expression_add_operations_string(expression_t *expression,
									  const char *string)
{
	char *parse_string;
	expression_operation_t operations[EXPRESSION_MAX_OPERATIONS];
	int16_t count = 0, i = 0, nmax = EXPRESSION_MAX_OPERATIONS;
	char *token = NULL;
	expression_operator_t parsed_operator = OPERATOR_NONE;
	expression_operator_t current_operator = OPERATOR_NONE;
	expression_input_t current_input = EXPRESSION_INPUT_INVALID;
	int state = EXPRESSION_STATE_START;

	/* The named initializers are left commented out for when this all goes
	 * to C99. */
	static int state_table[EXPRESSION_STATE_MAX][EXPRESSION_INPUT_MAX] = {
		/*[EXPRESSION_STATE_START] = */{
			/*[EXPRESSION_INPUT_INVALID] = */			EXPRESSION_ERR_INVALID_OPERATOR,
			/*[EXPRESSION_INPUT_NEEDS_OPERANDS] = */		EXPRESSION_STATE_OPERATOR,
			/*[EXPRESSION_INPUT_UNARY_OPERATOR] = */		EXPRESSION_STATE_START,
			/*[EXPRESSION_INPUT_VALUE] = */				EXPRESSION_ERR_EXPECTED_OPERATOR,
		},

		/* found operator */
		/*[EXPRESSION_STATE_OPERATOR] = */{
			/*[EXPRESSION_INPUT_INVALID] = */			EXPRESSION_ERR_INVALID_OPERATOR,
			/*[EXPRESSION_INPUT_NEEDS_OPERANDS] = */		EXPRESSION_ERR_EXPECTED_OPERAND,
			/*[EXPRESSION_INPUT_UNARY_OPERATOR] = */		EXPRESSION_ERR_EXPECTED_OPERAND,
			/*[EXPRESSION_INPUT_VALUE] = */				EXPRESSION_STATE_OPERAND,
		},

		/* found one operand */
		/*[EXPRESSION_STATE_OPERAND] = */{
			/*[EXPRESSION_INPUT_INVALID] = */			EXPRESSION_ERR_INVALID_OPERATOR,
			/*[EXPRESSION_INPUT_NEEDS_OPERANDS] = */		EXPRESSION_STATE_OPERATOR,
			/*[EXPRESSION_INPUT_UNARY_OPERATOR] = */		EXPRESSION_STATE_START,
			/*[EXPRESSION_INPUT_VALUE] = */				EXPRESSION_STATE_OPERAND,
		},
	};

	if (expression == NULL || string == NULL)
		return EXPRESSION_ERR_GENERIC;

	/* Empty string is an identity operation. */
	if (my_stricmp(string, "") == 0)
		return 0;

	parse_string = string_make(string);
	token = strtok(parse_string, EXPRESSION_DELIMITER);

	while (token != NULL) {
		char *end = NULL;
		long value = strtol(token, &end, 0);

		if (end == token) {
			parsed_operator = expression_operator_from_token(token);
			current_input = expression_input_for_operator(parsed_operator);
			state = state_table[state][current_input];
		}
		else {
			state = state_table[state][EXPRESSION_INPUT_VALUE];
		}

		/* Perform actions based on the new state. */
		if (state < EXPRESSION_STATE_START) {
			/* An error occurred, according to the state table. */
			string_free(parse_string);
			return state;
		}
		else if (state == EXPRESSION_STATE_START) {
			/* Flush the operation, since we are restarting or using a
			 * unary operator. */
			operations[count].operator = parsed_operator;
			operations[count].operand = 0;
			count++;
		}
		else if (state == EXPRESSION_STATE_OPERATOR) {
			/* Remember the operator, since we found an operator which needs
			 * operands. */
			current_operator = parsed_operator;
		}
		else if (state == EXPRESSION_STATE_OPERAND) {
			if (value < -32768 || value > 32767) {
				string_free(parse_string);
				return EXPRESSION_ERR_OPERAND_OUT_OF_BOUNDS;
			}
			/* Try to catch divide by zero. */
			if (current_operator == OPERATOR_DIV && value == 0) {
				string_free(parse_string);
				return EXPRESSION_ERR_DIVIDE_BY_ZERO;
			}

			/* Flush the operator and operand pair. */
			operations[count].operator = current_operator;
			operations[count].operand = (int16_t)value;
			count++;
		}

		/* Limit the number of expressions, saving what we have. */
		if (count >= nmax)
			break;

		token = strtok(NULL, EXPRESSION_DELIMITER);
	}

	for (i = 0; i < count; i++) {
		expression_add_operation(expression, operations[i]);
	}

	string_free(parse_string);
	return count;
}

/**
 * Test to make sure that the deep copy from expression_copy() is equal in value
 */
bool expression_test_copy(const expression_t *a, const expression_t *b)
{
	size_t i;
	bool success = true;

	if (a == NULL || b == NULL)
		return false;

	success &= (a != b);
	success &= (a->base_value == b->base_value);
	success &= (a->operation_count == b->operation_count);
	success &= (a->operations_size == b->operations_size);
	success &= (a->operations != b->operations);

	if (a->operation_count != b->operation_count)
		return false;

	for (i = 0; i < a->operation_count; i++) {
		success &= (a->operations[i].operand == b->operations[i].operand);
		success &= (a->operations[i].operator == b->operations[i].operator);
	}

	return success;
}
