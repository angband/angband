/* Tests for tests. */
#include "../z-tests.h"


TEST(this_fails){ FAIL; } TEST_END
TEST(this_is_blank) {;} TEST_END
TEST(this_succeeds) {;} TEST_END 

TEST(this_checks_1){ CHECK(1); } TEST_END
TEST(this_checks_0){ CHECK(0); } TEST_END
TEST(this_checks_42){ CHECK(42); } TEST_END

TEST(plan)
{
	TEST_PLAN(t, 12);
	CHECK(t.num_tests_planned == 12);
	CHECK(t.num_tests_executed == 0);
	CHECK(t.num_tests_failed == 0);
}
TEST_END

TEST(run)
{
	TEST_PLAN(t, 3);
	TEST_RUN(&t, this_succeeds);
	CHECK(t.num_tests_executed == 1);
	CHECK(t.num_tests_failed == 0);
	TEST_RUN(&t, this_is_blank);
	CHECK(t.num_tests_executed == 2);
	CHECK(t.num_tests_failed == 0);
	TEST_RUN(&t, this_fails);
	CHECK(t.num_tests_executed == 3);
	CHECK(t.num_tests_failed == 1);
	CHECK(t.num_tests_planned == 3); /* sanity */
}
TEST_END

TEST(check)
{
	TEST_PLAN(t, 3);
	TEST_RUN(&t, this_checks_0);
	TEST_RUN(&t, this_checks_1);
	TEST_RUN(&t, this_checks_42);
	CHECK(t.num_tests_executed == 3);
	CHECK(t.num_tests_failed == 1);
}
TEST_END

TEST(test_success)
{
	/* success! */
	TEST_PLAN(a, 1);
	CHECK(!test_success(&a));
	TEST_RUN(&a, this_succeeds);
	CHECK(test_success(&a));

	/* failure! */
	TEST_PLAN(b, 1);
	TEST_RUN(&b, this_fails);
	CHECK(!test_success(&b));

	/* no premature success */
	TEST_PLAN(c, 2);
	TEST_RUN(&c, this_succeeds);
	CHECK(!test_success(&c));
	TEST_RUN(&c, this_succeeds);
	CHECK(test_success(&c));

	/* too many */
	TEST_PLAN(d, 1);
	TEST_RUN(&d, this_succeeds);
	TEST_RUN(&d, this_succeeds);
	CHECK(!test_success(&d));

	/* no plan => don't worry about count */
	TEST_PLAN(e, 0);
	CHECK(test_success(&e));
	TEST_RUN(&e, this_succeeds);
	CHECK(test_success(&e));
	TEST_RUN(&e, this_fails);
	CHECK(!test_success(&e));
}
TEST_END


/* my Inglish iz good too */
int this_completes_good(void)
{
	TEST_PLAN(t, 1);
	TEST_RUN(&t, this_succeeds);
	TESTS_COMPLETE(t);
}

int this_completes_bad(void)
{
	TEST_PLAN(t,3);
	TEST_RUN(&t, this_fails);
	TEST_RUN(&t, this_fails);
	TEST_RUN(&t, this_succeeds);
	TEST_RUN(&t, this_succeeds);
	/* two of each, in the event someone      */
	/* tries to get fancy and overload the    */
	/* return code to be a count of some sort */
	TESTS_COMPLETE(t);
}

TEST(complete)
{
	CHECK(this_completes_good() == 0);
	CHECK(this_completes_bad()  == 1);
}
TEST_END

int main(void)
{
	puts("Running test tests. You should see some failures.\n"
		"This is OK. We're testing failure.");
	TEST_PLAN(t, 5);

	TEST_RUN(&t, plan);
	TEST_RUN(&t, run);
	TEST_RUN(&t, check);
	TEST_RUN(&t, test_success);
	TEST_RUN(&t, complete);

	puts("\n\nThese are the numbers you're looking for:");
	TESTS_COMPLETE(t);
}

