/**
 * \file z-dice.c
 * \brief Represent more complex dice than random_value
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

#include "z-dice.h"
#include "z-virt.h"
#include "z-util.h"
#include "z-rand.h"
#include "z-expression.h"

typedef struct dice_expression_entry_s {
	const char *name;
	const expression_t *expression;
} dice_expression_entry_t;

struct dice_s {
	int b, x, y, m;
	bool ex_b, ex_x, ex_y, ex_m;
	dice_expression_entry_t *expressions;
};

/**
 * String parser states.
 */
typedef enum dice_state_e {
	DICE_STATE_START,
	DICE_STATE_BASE_DIGIT,
	DICE_STATE_FLUSH_BASE,
	DICE_STATE_DICE_DIGIT,
	DICE_STATE_FLUSH_DICE,
	DICE_STATE_SIDE_DIGIT,
	DICE_STATE_FLUSH_SIDE,
	DICE_STATE_BONUS,
	DICE_STATE_BONUS_DIGIT,
	DICE_STATE_FLUSH_BONUS,
	DICE_STATE_VAR,
	DICE_STATE_VAR_CHAR,
	DICE_STATE_FLUSH_ALL,
	DICE_STATE_MAX,
} dice_state_t;

/**
 * Input types for the parser state table.
 */
typedef enum dice_input_e {
	DICE_INPUT_AMP,
	DICE_INPUT_MINUS,
	DICE_INPUT_BASE,
	DICE_INPUT_DICE,
	DICE_INPUT_BONUS,
	DICE_INPUT_VAR,
	DICE_INPUT_DIGIT,
	DICE_INPUT_UPPER,
	DICE_INPUT_NULL,
	DICE_INPUT_MAX,
} dice_input_t;

/**
 * Hard limit on the number of variables/expressions. Shouldn't need more than
 * the possible values.
 */
#define DICE_MAX_EXPRESSIONS 4

/**
 * Max size for a token/number to be parsed. Longer strings will be truncated.
 */
#define DICE_TOKEN_SIZE 16

/**
 * Return the appropriate input type based on the given character.
 */
static dice_input_t dice_input_for_char(char c)
{
	/* Catch specific characters before checking bigger char categories. */
	switch (c) {
		case '&':
			return DICE_INPUT_AMP;
		case '-':
			return DICE_INPUT_MINUS;
		case '+':
			return DICE_INPUT_BASE;
		case 'd':
			return DICE_INPUT_DICE;
		case 'M':
		case 'm':
			return DICE_INPUT_BONUS;
		case '$':
			return DICE_INPUT_VAR;
		case '\0':
			return DICE_INPUT_NULL;
		default:
			break;
	}

	if (isdigit(c))
		return DICE_INPUT_DIGIT;

	if (isupper(c))
		return DICE_INPUT_UPPER;

	return DICE_INPUT_MAX;
}

/**
 * Perform a state transition for the given state and input.
 *
 * The state table is contained within this function, using a compact
 * char-based format.
 *
 * \param state is the current state.
 * \param input is the input type to transition with.
 * \return The next state for the input, Or DICE_STATE_MAX for an invalid transition.
 */
static dice_state_t dice_parse_state_transition(dice_state_t state,
												dice_input_t input)
{
	static unsigned char state_table[DICE_STATE_MAX][DICE_INPUT_MAX] = {
		/* Input:			        { '&', '-', '+', 'd', 'm', '$', 'D', 'U', '0' */
		/*[DICE_STATE_START] = */	/* A */ { '.', 'B', '.', 'E', 'H', 'K', 'B', '.', '.' },
		/*[DICE_STATE_BASE_DIGIT] = */	/* B */ { '.', '.', 'C', 'E', '.', '.', 'B', '.', 'C' },
		/*[DICE_STATE_FLUSH_BASE] = */  /* C */ { '.', '.', '.', 'E', 'H', 'K', 'D', '.', '.' },
		/*[DICE_STATE_DICE_DIGIT] = */  /* D */ { '.', '.', '.', 'E', '.', '.', 'D', '.', '.' },
		/*[DICE_STATE_FLUSH_DICE] = */  /* E */ { '.', '.', '.', '.', '.', 'K', 'F', '.', '.' },
		/*[DICE_STATE_SIDE_DIGIT] = */  /* F */ { 'G', '.', '.', '.', 'H', '.', 'F', '.', 'G' },
		/*[DICE_STATE_FLUSH_SIDE] = */  /* G */ { '.', '.', '.', '.', 'H', '.', '.', '.', '.' },
		/*[DICE_STATE_BONUS] = */	/* H */ { '.', '.', '.', '.', '.', 'K', 'I', '.', '.' },
		/*[DICE_STATE_BONUS_DIGIT] = */ /* I */ { '.', '.', '.', '.', '.', '.', 'I', '.', 'J' },
		/*[DICE_STATE_FLUSH_BONUS] = */ /* J */ { '.', '.', '.', '.', '.', '.', '.', '.', '.' },
		/*[DICE_STATE_VAR] = */		/* K */ { '.', '.', '.', '.', '.', '.', '.', 'L', '.' },
		/*[DICE_STATE_VAR_CHAR] = */	/* L */ { 'G', '.', 'C', 'E', 'H', '.', '.', 'L', 'M' },
		/*[DICE_STATE_FLUSH_ALL] = */	/* M */ { '.', '.', '.', '.', '.', '.', '.', '.', '.' }
	};

	if (state == DICE_STATE_MAX || input == DICE_INPUT_MAX)
		return DICE_STATE_MAX;

	if (state_table[state][input] == '.')
		return DICE_STATE_MAX;

	return state_table[state][input] - 'A';
}

/**
 * Zero out the internal state of the dice object. This will only deallocate
 * entries in the expressions table; it will not deallocate the table itself.
 */
static void dice_reset(dice_t *dice)
{
	int i;

	dice->b = 0;
	dice->x = 0;
	dice->y = 0;
	dice->m = 0;

	dice->ex_b = false;
	dice->ex_x = false;
	dice->ex_y = false;
	dice->ex_m = false;

	if (dice->expressions == NULL)
		return;

	for (i = 0; i < DICE_MAX_EXPRESSIONS; i++) {
		if (dice->expressions[i].name != NULL) {
			string_free((char *)dice->expressions[i].name);
			dice->expressions[i].name = NULL;
		}

		if (dice->expressions[i].expression != NULL) {
			expression_free((expression_t *)dice->expressions[i].expression);
			dice->expressions[i].expression = NULL;
		}
	}
}

/**
 * Allocate and initialize a new dice object. Returns NULL if it was unable to
 * be created.
 */
dice_t *dice_new(void)
{
	dice_t *dice = mem_zalloc(sizeof(dice_t));

	if (dice == NULL)
		return NULL;

	dice_reset(dice);

	return dice;
}

/**
 * Deallocate a dice object.
 */
void dice_free(dice_t *dice)
{
	if (dice == NULL)
		return;

	/* Free any variable names and expression objects. */
	dice_reset(dice);

	if (dice->expressions != NULL) {
		mem_free(dice->expressions);
		dice->expressions = NULL;
	}

	mem_free(dice);
}

/**
 * Add an entry to the dice object's symbol list.
 *
 * \param dice is the object the variable is being added to.
 * \param name is the name of the variable.
 * \return The index of the variable name (if added or already found), or -1 for error.
 */
static int dice_add_variable(dice_t *dice, const char *name)
{
	int i;

	if (dice->expressions == NULL) {
		dice->expressions = mem_zalloc(DICE_MAX_EXPRESSIONS *
									   sizeof(dice_expression_entry_t));
	}

	for (i = 0; i < DICE_MAX_EXPRESSIONS; i++) {
		if (dice->expressions[i].name == NULL) {
			/* Add the variable to an empty slot. */
			dice->expressions[i].name = string_make(name);
			return i;
		}
		else if (my_stricmp(dice->expressions[i].name, name) == 0) {
			/* We already have the variable and will use this expression. */
			return i;
		}
	}

	/* No space left for variables. */
	return -1;
}

/**
 * Bind an expression to a variable name.
 *
 * This function creates a deep copy of the expression that the dice object owns
 *
 * \param dice is the object that will use the expression..
 * \param name is the variable that the expression should be bound to.
 * \param expression is the expression to bind.
 * \return The index of the expression or -1 for error.
 */
int dice_bind_expression(dice_t *dice, const char *name,
						 const expression_t *expression)
{
	int i;

	if (dice->expressions == NULL)
		return -1;

	for (i = 0; i < DICE_MAX_EXPRESSIONS; i++) {
		if (dice->expressions[i].name == NULL)
			continue;

		if (my_stricmp(name, dice->expressions[i].name) == 0) {
			dice->expressions[i].expression = expression_copy(expression);

			if (dice->expressions[i].expression == NULL)
				return -1;

			return i;
		}
	}

	/* Couldn't find variable name to bind to. */
	return -1;
}

/**
 * Parse a formatted string for values and variables to represent a dice roll.
 *
 * This function can parse a number of formats in the general style of "1+2d3M4"
 * (base, dice, sides, and bonus). Varibles (to which expressions can be bound)
 * can be subsitituted for numeric values by using an all-uppercase name
 * starting with $.
 * Spaces are ignored, concatenating the strings on either side of the space
 * character. Tokens (numbers and variable names) longer than the maximum will
 * be truncated. The unit test demonstrates the variety of valid strings.
 *
 * \param dice is the dice object to parse the string into.
 * \param string is the string to be parsed.
 * \return true if parsing was successful, false if not.
 */
bool dice_parse_string(dice_t *dice, const char *string)
{
	char token[DICE_TOKEN_SIZE + 1] = { '\0' };
	size_t token_end = 0;
	size_t current = 0;
	dice_state_t state = 0;

	/* We need to keep track of the last thing we saw, since the parser isn't complex. */
	enum last_seen_e {
		DICE_SEEN_NONE,
		DICE_SEEN_BASE,
		DICE_SEEN_DICE,
		DICE_SEEN_SIDE,
		DICE_SEEN_BONUS,
	} last_seen = DICE_SEEN_NONE;

	if (dice == NULL || string == NULL)
		return false;

	/* Reset all internal state, since this object might be reused. */
	dice_reset(dice);

	/* Note that we are including the string terminator as part of the parse. */
	for (current = 0; current <= strlen(string); current++) {
		bool flush;
		dice_input_t input_type = DICE_INPUT_MAX;

		/* Skip spaces; this will concatenate digits and variable names. */
		if (isspace(string[current]))
			continue;

		input_type = dice_input_for_char(string[current]);

		/*
		 * Get the next state, based on the type of input char. If it's a
		 * possible number or varible name, we'll store the character in the
		 * token buffer.
		 */
		switch (input_type) {
			case DICE_INPUT_AMP:
			case DICE_INPUT_BASE:
			case DICE_INPUT_DICE:
			case DICE_INPUT_VAR:
			case DICE_INPUT_NULL:
				state = dice_parse_state_transition(state, input_type);
				break;

			case DICE_INPUT_MINUS:
			case DICE_INPUT_DIGIT:
			case DICE_INPUT_UPPER:
				/* Truncate tokens if they are too long to fit. */
				if (token_end < DICE_TOKEN_SIZE) {
					token[token_end] = string[current];
					token_end++;
				}

				state = dice_parse_state_transition(state, input_type);
				break;

			default:
				break;
		}

		/*
		 * Allow 'M' to be used as the bonus marker and to be used in variable
		 * names.
		 * Ideally, 'm' should be the only marker and this could go away by
		 * adding a case to the switch above for DICE_INPUT_BONUS
		 * (underneath DICE_INPUT_NULL).
		 */
		if (string[current] == 'M') {
			if (state == DICE_STATE_VAR || state == DICE_STATE_VAR_CHAR) {
				if (token_end < DICE_TOKEN_SIZE) {
					token[token_end] = string[current];
					token_end++;
				}

				state = dice_parse_state_transition(state, DICE_INPUT_UPPER);
			}
			else
				state = dice_parse_state_transition(state, DICE_INPUT_BONUS);
		}
		else if (string[current] == 'm') {
			state = dice_parse_state_transition(state, DICE_INPUT_BONUS);
		}

		/* Illegal transition. */
		if (state >= DICE_STATE_MAX)
			return false;

		/*
		 * Default flushing to true, since there are more states that don't
		 * need to be flushed. For some states, we need to do a bit of extra
		 * work, since the parser isn't that complex. A more complex parser
		 * would have more explicit states for variable names.
		 */
		flush = true;

		switch (state) {
			case DICE_STATE_FLUSH_BASE:
				last_seen = DICE_SEEN_BASE;
				break;

			case DICE_STATE_FLUSH_DICE:
				last_seen = DICE_SEEN_DICE;
				/* If we see a 'd' without a number before it, we assume it
				 * to be one die. */
				if (strlen(token) == 0) {
					token[0] = '1';
					token[1] = '\0';
				}
				break;

			case DICE_STATE_FLUSH_SIDE:
				last_seen = DICE_SEEN_SIDE;
				break;

			case DICE_STATE_FLUSH_BONUS:
				last_seen = DICE_SEEN_BONUS;
				break;

			case DICE_STATE_FLUSH_ALL:
				/* Flushing all means that we are flushing whatever comes after
				 * it was that we last saw. */
				if (last_seen < DICE_SEEN_BONUS)
					last_seen++;
				break;

			case DICE_STATE_BONUS:
				/* The bonus state is weird, so if we last saw dice, we're now
				 * seeing sides. */
				if (last_seen == DICE_SEEN_DICE)
					last_seen = DICE_SEEN_SIDE;
				else
					last_seen = DICE_SEEN_BONUS;
				break;

			default:
				/* We're in a state that shouldn't flush anything. */
				flush = false;
				break;
		}

		/*
		 * If we have a token that we need to flush, put it where it needs to
		 * go in the dice object. If the token is an uppercase letter, it's
		 * a variable and needs to go in the expression table. Otherwise, we
		 * try to parse it as a number, where it is set directly as a value.
		 */
		if (flush && strlen(token) > 0) {
			int value = 0;
			bool is_variable = false;

			if (isupper(token[0])) {
				value = dice_add_variable(dice, token);
				is_variable = true;
			}
			else {
				value = (int)strtol(token, NULL, 0);
				is_variable = false;
			}

			switch (last_seen) {
				case DICE_SEEN_BASE:
					dice->b = value;
					dice->ex_b = is_variable;
					break;
				case DICE_SEEN_DICE:
					dice->x = value;
					dice->ex_x = is_variable;
					break;
				case DICE_SEEN_SIDE:
					dice->y = value;
					dice->ex_y = is_variable;
					break;
				case DICE_SEEN_BONUS:
					dice->m = value;
					dice->ex_m = is_variable;
					break;
				default:
					break;
			}

			memset(token, 0, DICE_TOKEN_SIZE + 1);
			token_end = 0;
		}
	}

	return true;
}

/**
 * Extract a random_value by evaluating any bound expressions.
 *
 * \param dice is the object to get the random_value from.
 * \param v is the random_value to place the values into.
 */
void dice_random_value(const dice_t *dice, random_value *v)
{
	if (v == NULL)
		return;

	if (dice->ex_b) {
		if (dice->expressions != NULL && dice->expressions[dice->b].expression != NULL)
			v->base = expression_evaluate(dice->expressions[dice->b].expression);
		else
			v->base = 0;
	}
	else
		v->base = dice->b;

	if (dice->ex_x) {
		if (dice->expressions != NULL && dice->expressions[dice->x].expression != NULL)
			v->dice = expression_evaluate(dice->expressions[dice->x].expression);
		else
			v->dice = 0;
	}
	else
		v->dice = dice->x;

	if (dice->ex_y) {
		if (dice->expressions != NULL && dice->expressions[dice->y].expression != NULL)
			v->sides = expression_evaluate(dice->expressions[dice->y].expression);
		else
			v->sides = 0;
	}
	else
		v->sides = dice->y;

	if (dice->ex_m) {
		if (dice->expressions != NULL && dice->expressions[dice->m].expression != NULL)
			v->m_bonus = expression_evaluate(dice->expressions[dice->m].expression);
		else
			v->m_bonus = 0;
	}
	else
		v->m_bonus = dice->m;
}

/**
 * Fully evaluates the dice object, using randcalc(). The random_value used is
 * returned if desired.
 *
 * \param dice is the dice object to evaluate.
 * \param level is the level value that is passed to randcalc().
 * \param asp is the aspect that is passed to randcalc().
 * \param v is a pointer used to return the random_value used.
 */
int dice_evaluate(const dice_t *dice, int level, aspect asp, random_value *v)
{
	random_value rv;
	dice_random_value(dice, &rv);

	if (v != NULL) {
		v->base = rv.base;
		v->dice = rv.dice;
		v->sides = rv.sides;
		v->m_bonus = rv.m_bonus;
	}

	return randcalc(rv, level, asp);
}

/**
 * Evaluates the dice object, using damroll() (base + XdY). The random_value
 * used is returned if desired.
 *
 * \param dice is the dice object to evaluate.
 * \param v is a pointer used to return the random_value used.
 */
int dice_roll(const dice_t *dice, random_value *v)
{
	random_value rv;
	dice_random_value(dice, &rv);

	if (v != NULL) {
		v->base = rv.base;
		v->dice = rv.dice;
		v->sides = rv.sides;
		v->m_bonus = rv.m_bonus;
	}

	return rv.base + damroll(rv.dice, rv.sides);
}

/**
 * Test the dice object against the given values.
 */
bool dice_test_values(const dice_t *dice, int base, int dice_count, int sides,
		int bonus)
{
	bool success = true;
	success &= dice->b == base;
	success &= dice->x == dice_count;
	success &= dice->y == sides;
	success &= dice->m == bonus;
	return success;
}

/**
 * Check that the dice object has the given variables for the component.
 */
bool dice_test_variables(const dice_t *dice, const char *base,
		const char *dice_name, const char *sides, const char *bonus)
{
	bool success = true;

	if (dice->expressions == NULL)
		return false;

	if (base == NULL)
		success &= !dice->ex_b;
	else
		success &= (dice->ex_b && dice->b >= 0 && my_stricmp(dice->expressions[dice->b].name, base) == 0);

	if (dice_name == NULL)
		success &= !dice->ex_x;
	else
		success &= (dice->ex_x && dice->x >= 0 && my_stricmp(dice->expressions[dice->x].name, dice_name) == 0);

	if (sides == NULL)
		success &= !dice->ex_y;
	else
		success &= (dice->ex_y && dice->y >= 0 && my_stricmp(dice->expressions[dice->y].name, sides) == 0);

	if (bonus == NULL)
		success &= !dice->ex_m;
	else
		success &= (dice->ex_m && dice->m >= 0 && my_stricmp(dice->expressions[dice->m].name, bonus) == 0);

	return success;
}
