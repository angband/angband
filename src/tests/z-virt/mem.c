/* z-virt/mem */

#include "unit-test.h"
#include "z-virt.h"

NOSETUP
NOTEARDOWN

int test_alloc(void *state) {
	void *p1 = mem_alloc(16);
	void *p2 = mem_alloc(16);
	require(p1 != p2);
	memset(p1, 0x1, 16);
	memset(p2, 0x2, 16);
	mem_free(p1);
	mem_free(p2);
	return 0;
}

int test_realloc(void *state) {
	void *p1 = mem_realloc(NULL, 32);
	void *p2 = mem_realloc(p1, 64);
	memset(p2, 0x3, 64);
	mem_free(p2);
	return 0;
}

const char *suite_name = "z-virt/mem";
struct test tests[] = {
	{ "alloc", test_alloc },
	{ "realloc", test_realloc },
	{ NULL, NULL }
};
