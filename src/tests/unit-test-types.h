/* unit-test-types.h 
 *
 * Stub header for testing harness data structures
 *
 */

#ifndef UNIT_TEST_TYPES_H
#define UNIT_TEST_TYPES_H

struct test {
	const char *name;
	int (*func)(void *data);
};

#endif /* !UNIT_TEST_TYPES_H */
