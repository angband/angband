/* z-dice/dice */

#include "unit-test.h"
#include "z-dice.h"

NOSETUP
NOTEARDOWN

static int test_alloc(void *state)
{
	dice_t *new = dice_new();
	require(new != NULL);
	dice_free(new);
	ok;
}

static int test_parse_success(void *state)
{
	dice_t *new = dice_new();

	/* Basic formatting. */
	require(dice_parse_string(new, "1+2d3M4"));
	require(dice_test_values(new, 1, 2, 3, 4));

	require(dice_parse_string(new, "1+d3M4"));
	require(dice_test_values(new, 1, 1, 3, 4));

	require(dice_parse_string(new, "1+M4"));
	require(dice_test_values(new, 1, 0, 0, 4));

	require(dice_parse_string(new, "1+2d3"));
	require(dice_test_values(new, 1, 2, 3, 0));

	require(dice_parse_string(new, "1+d3"));
	require(dice_test_values(new, 1, 1, 3, 0));

	require(dice_parse_string(new, "2d3M4"));
	require(dice_test_values(new, 0, 2, 3, 4));

	require(dice_parse_string(new, "d3M4"));
	require(dice_test_values(new, 0, 1, 3, 4));

	require(dice_parse_string(new, "M4"));
	require(dice_test_values(new, 0, 0, 0, 4));

	require(dice_parse_string(new, "2d3"));
	require(dice_test_values(new, 0, 2, 3, 0));

	require(dice_parse_string(new, "d3"));
	require(dice_test_values(new, 0, 1, 3, 0));

	require(dice_parse_string(new, "1"));
	require(dice_test_values(new, 1, 0, 0, 0));

	/* Multiple digits. */
	require(dice_parse_string(new, "11+22d33M44"));
	require(dice_test_values(new, 11, 22, 33, 44));

	/* Negative bases. */
    require(dice_parse_string(new, "-1+d3"));
	require(dice_test_values(new, -1, 1, 3, 0));

	/* Basic formats with variables. */
	require(dice_parse_string(new, "$A+$Bd$Cm$D"));
	require(dice_test_variables(new, "A", "B", "C", "D"));

	require(dice_parse_string(new, "$A+d$Cm$D"));
	require(dice_test_variables(new, "A", NULL, "C", "D"));

	require(dice_parse_string(new, "$A+m$D"));
	require(dice_test_variables(new, "A", NULL, NULL, "D"));

	require(dice_parse_string(new, "$A+$Bd$C"));
	require(dice_test_variables(new, "A", "B", "C", NULL));

	require(dice_parse_string(new, "$A+d$C"));
	require(dice_test_variables(new, "A", NULL, "C", NULL));

	require(dice_parse_string(new, "$Bd$Cm$D"));
	require(dice_test_variables(new, NULL, "B", "C", "D"));

	require(dice_parse_string(new, "d$Cm$D"));
	require(dice_test_variables(new, NULL, NULL, "C", "D"));

	require(dice_parse_string(new, "m$D"));
	require(dice_test_variables(new, NULL, NULL, NULL, "D"));

	require(dice_parse_string(new, "$Bd$C"));
	require(dice_test_variables(new, NULL, "B", "C", NULL));

	require(dice_parse_string(new, "d$C"));
	require(dice_test_variables(new, NULL, NULL, "C", NULL));

	require(dice_parse_string(new, "$A"));
	require(dice_test_variables(new, "A", NULL, NULL, NULL));

	/* Variable names. */
	require(dice_parse_string(new, "$BASEd$SIDES"));
	require(dice_test_variables(new, NULL, "BASE", "SIDES", NULL));

    require(dice_parse_string(new, "d$AMm4"));
	require(dice_test_variables(new, NULL, NULL, "AM", NULL));

    require(dice_parse_string(new, "$MAGE+M1"));
	require(dice_test_variables(new, "MAGE", NULL, NULL, NULL));

	/* Ignore spaces. */
    require(dice_parse_string(new, " 1 + 2 d 3 M 4 "));
    require(dice_parse_string(new, "1 1 +2d3M4"));
    require(dice_parse_string(new, "$ BIG BASE +2d3M4"));

	/* Token truncation. */
    require(dice_parse_string(new, "$ THIS IS A REALLY BIG TOKEN AND WILL BE CLIPPED"));

    /*
	 * While this probably should be an error, it keeps things simpler to just allow this.
	 * It might be useful for providing a placeholder, since it has a value of zero.
	 */
    require(dice_parse_string(new, "-"));

	dice_free(new);
	ok;
}

static int test_parse_failure(void *state)
{
	dice_t *new = dice_new();

	/* Empty string. */
	require(!dice_parse_string(new, ""));

	/* Disallowed minus tokens. */
	require(!dice_parse_string(new, "1+-2d3M4"));
    require(!dice_parse_string(new, "1+2d-3M4"));
    require(!dice_parse_string(new, "1+2d3M-4"));
    require(!dice_parse_string(new, "-$A+d3"));

	/* Bad variable names. */
    require(!dice_parse_string(new, "$base+2d3"));
    require(!dice_parse_string(new, "$BASE$B+2d3"));
	require(!dice_parse_string(new, "$$BASE+2d3"));
    require(!dice_parse_string(new, "$1+2d3M4"));
	require(!dice_parse_string(new, "1$+2d3M4"));
    require(!dice_parse_string(new, "1+$2d3M4"));
	require(!dice_parse_string(new, "1+2$d3M4"));
	require(!dice_parse_string(new, "1+2d$3M4"));
	require(!dice_parse_string(new, "1+2d3$M4"));
	require(!dice_parse_string(new, "1+2d3M$4"));
	require(!dice_parse_string(new, "1+2d3M4$"));

	/* Early termination. */
    require(!dice_parse_string(new, "1+"));
    require(!dice_parse_string(new, "1+2"));
    require(!dice_parse_string(new, "1+d"));
    require(!dice_parse_string(new, "1+2d"));
    require(!dice_parse_string(new, "1+2d3M"));
    require(!dice_parse_string(new, "+2d3"));

	dice_free(new);
	ok;
}

static int32_t test_evaluate_base(void)
{
	return 3;
}

static int test_evaluate(void *state)
{
	int value = 0;
	expression_t *expression = expression_new();
	dice_t *new = dice_new();
	random_value v;

	expression_set_base_value(expression, test_evaluate_base);
	require(expression_add_operations_string(expression, "* 3 - 1") > 0);
	require(dice_parse_string(new, "$A + 2d3"));
	require(dice_bind_expression(new, "A", expression) >= 0);

	value = dice_evaluate(new, 1, MAXIMISE, &v);
	require(value == 14);
	require(v.base == 8);
	require(v.dice == 2);
	require(v.sides == 3);
	require(v.m_bonus == 0);

	value = dice_roll(new, &v);
	require(v.base == 8);
	require(v.dice == 2);
	require(v.sides == 3);
	require(v.m_bonus == 0);

	dice_free(new);
	expression_free(expression);
	ok;
}

const char *suite_name = "z-dice/dice";
struct test tests[] = {
	{ "alloc", test_alloc },
	{ "parse-success", test_parse_success },
	{ "parse-failure", test_parse_failure },
	{ "evaluate", test_evaluate },
	{ NULL, NULL },
};
