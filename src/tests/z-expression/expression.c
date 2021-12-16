/* z-expression/expression.c */

#include "unit-test.h"
#include "z-expression.h"

NOSETUP
NOTEARDOWN

static int test_alloc(void *state)
{
	expression_t *new = expression_new();
	expression_t *copy;
	require(new != NULL);

	expression_add_operations_string(new, "+ 1");
	copy = expression_copy(new);
	require(expression_test_copy(new, copy));

	expression_free(new);
	expression_free(copy);
	ok;
}

static int test_parse_success(void *state)
{
	int result = 0;
	expression_t *new = expression_new()	;

	/* Test basic operators. */
	result = expression_add_operations_string(new, "+ 1");
	require(result == 1);
	result = expression_add_operations_string(new, "- 1");
	require(result == 1);
	result = expression_add_operations_string(new, "* 1");
	require(result == 1);
	result = expression_add_operations_string(new, "/ 1");
	require(result == 1);

	/* Test various negation situations. */
	result = expression_add_operations_string(new, "n");
	require(result == 1);
	result = expression_add_operations_string(new, "n n");
	require(result == 2);
	result = expression_add_operations_string(new, "n + 1");
	require(result == 2);
	result = expression_add_operations_string(new, "+ 1 n");
	require(result == 2);

	/* Multiple operands. */
	result = expression_add_operations_string(new, "+ 1 2 3");
	require(result == 3);

	/* Identity expression. */
	result = expression_add_operations_string(new, "");
	require(result == 0);

	/* Negative operands. */
	result = expression_add_operations_string(new, "+ -1");
	require(result == 1);
	result = expression_add_operations_string(new, "- -1");
	require(result == 1);
	result = expression_add_operations_string(new, "+ 1 -1");
	require(result == 2);
	result = expression_add_operations_string(new, "+ -1 1");
	require(result == 2);

	/* More complex examples. */
	result = expression_add_operations_string(new, "* 4 / 3 ");
	require(result == 2);
	result = expression_add_operations_string(new, "- 1 / 5 + 3");
	require(result == 3);

	expression_free(new);
	ok;
}

static int test_parse_failure(void *state)
{
	int result = 0;
	expression_t *new = expression_new();

	/* Basic problems. */
	result = expression_add_operations_string(new, NULL);
	require(result == EXPRESSION_ERR_GENERIC);
	result = expression_add_operations_string(NULL, "+ 1");
	require(result == EXPRESSION_ERR_GENERIC);

	/* Expressions must start with an operator. */
	result = expression_add_operations_string(new, "44 / 3");
	require(result == EXPRESSION_ERR_EXPECTED_OPERATOR);

	/* Can't have operators without operands. */
	result = expression_add_operations_string(new, "* + 4");
	require(result == EXPRESSION_ERR_EXPECTED_OPERAND);

	/* Invalid operator. */
	result = expression_add_operations_string(new, "+ 4 % 4");
	require(result == EXPRESSION_ERR_INVALID_OPERATOR);

	/* No operands after negation. */
	result = expression_add_operations_string(new, "n 4 + 1");
	require(result == EXPRESSION_ERR_EXPECTED_OPERATOR);

	/* Catch divide by zero. */
	result = expression_add_operations_string(new, "/ 0");
	require(result == EXPRESSION_ERR_DIVIDE_BY_ZERO);
	result = expression_add_operations_string(new, "/ 10 0");
	require(result == EXPRESSION_ERR_DIVIDE_BY_ZERO);

	/* Too many operations (see EXPRESSION_MAX_OPERATIONS). */
	result = expression_add_operations_string(new,
											  "+ 1 2 3 4 5 6 7 8 9 0"
											  "+ 1 2 3 4 5 6 7 8 9 0"
											  "+ 1 2 3 4 5 6 7 8 9 0"
											  "+ 1 2 3 4 5 6 7 8 9 0"
											  "+ 1 2 3 4 5 6 7 8 9 0"
											  "+ 1 2 3 4 5 6 7 8 9 0");
	require(result == 50);

	expression_free(new);
	ok;
}

static int32_t base_value_2(void)
{
	return 9;
}

static int test_evaluate(void *state)
{
	expression_t *new = expression_new();

	/* Basic evaluation with base of zero. */
	expression_add_operations_string(new, "+ 1 2 3");
	require(expression_evaluate(new) == 6);
	expression_add_operations_string(new, "* 2");
	require(expression_evaluate(new) == 12);
	expression_add_operations_string(new, "n");
	require(expression_evaluate(new) == -12);
	expression_add_operations_string(new, "- -3");
	require(expression_evaluate(new) == -9);
	expression_add_operations_string(new, "n / 3");
	require(expression_evaluate(new) == 3);

	/* Evaluate with base value function. */
	expression_set_base_value(new, base_value_2);
	require(expression_evaluate(new) == 9);

	expression_free(new);
	ok;
}

const char *suite_name = "z-expression/expression";
struct test tests[] = {
	{ "alloc", test_alloc },
	{ "parse-success", test_parse_success },
	{ "parse-failure", test_parse_failure },
	{ "evaluate", test_evaluate },
	{ NULL, NULL },
};
