/*
 * Sample tests.
 * Generally you'll want to test your own code in here. =)
 */
#include "../z-tests.h"

/* basic test setup */
TEST(addition)
{
	int x = 2;
	int y = 2;
	CHECK(x + y == 4);
}
TEST_END

/*
 * If CHECK doesn't make sense somewhere, you can
 * use FAIL instead. Note that a FAILure will
 * immediately abort execution.
 */
TEST(less_than)
{
	int x = 4;
	int y = 2;
	if(x < y) FAIL;
}
TEST_END

/* typical 'main' block */
int main(void)
{
	TEST_PLAN(t, 2);

	TEST_RUN(&t, addition);
	TEST_RUN(&t, less_than);

	TESTS_COMPLETE(t);
}

