#include "object/artifact.h"

#include "z-set.h"

struct artifacts {
	struct set *set;
};

struct artifacts *artifacts_new()
{
	struct artifacts *as = mem_zalloc(sizeof *as);
	as->set = set_new();
	return as;
}

void artifacts_add(struct artifacts *as, struct artifact *a)
{
	set_insert(as->set, a->aidx, a);
}

struct artifact *artifacts_get(struct artifacts *as, size_t idx)
{
	return set_get(as->set, idx);
}

struct artifacts *artifacts;
