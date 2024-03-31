/* z-queue/qp.c */
/* Exercise the priority queues declared in z-queue.h. */

#include "unit-test.h"
#include "z-queue.h"
#include "z-virt.h"

struct iidata { int priority, payload; };
struct ipdata { int priority; void *payload; };

NOSETUP
NOTEARDOWN

static int compare_iidata(const void *a, const void *b)
{
	const struct iidata *ai = a;
	const struct iidata *bi = b;

	if (ai->priority < bi->priority) {
		return -1;
	}
	if (ai->priority > bi->priority) {
		return 1;
	}
	return 0;
}

static int compare_ipdata(const void *a, const void *b)
{
	const struct ipdata *ai = a;
	const struct iidata *bi = b;

	if (ai->priority < bi->priority) {
		return -1;
	}
	if (ai->priority > bi->priority) {
		return 1;
	}
	return 0;
}

static int test_qp_trivial(void *state)
{
	struct priority_queue *qp = qp_new(0);
	void *p;
	int i;

	eq(qp_size(qp), 0);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_free(qp, NULL);

	qp = qp_new(4);
	eq(qp_size(qp), 4);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_int(qp, 7, 10);
	eq(qp_size(qp), 4);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	i = qp_peek_int(qp);
	eq(i, 10);
	i = qp_pop_int(qp);
	eq(i, 10);
	eq(qp_size(qp), 4);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_free(qp, NULL);

	qp = qp_new(5);
	eq(qp_size(qp), 5);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_ptr(qp, -3, &qp);
	eq(qp_size(qp), 5);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	p = qp_peek_ptr(qp);
	ptreq(p, &qp);
	p = qp_pop_ptr(qp);
	ptreq(p, &qp);
	eq(qp_size(qp), 5);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_free(qp, NULL);

	ok;
}

static int test_qp_integer(void *state)
{
	const size_t size = 15;
	struct iidata data[12] = {
		{ 6, -3 }, { 3, 15 }, { 17, 0 }, { 16, -2 },
		{ 0, 8 }, { 9, 11 }, { -3, -7 }, { 14, 10 },
		{ 20, 1 }, { 11, 18 }, { 5, 7 }, { -4, 6 },
	};
	struct iidata sorted[12];
	struct priority_queue *qp = qp_new(size);
	size_t i;
	int payload;

	memcpy(sorted, data, sizeof(data));
	qsort(sorted, sizeof(data) / sizeof(data[0]), sizeof(data[0]),
		compare_iidata);

	eq(qp_size(qp), size);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_int(qp, data[0].priority, data[0].payload);
	eq(qp_size(qp), size);
	eq(qp_len(qp), 1);
	payload = qp_peek_int(qp);
	eq(payload, data[0].payload);
	require(!qp_isinvalid(qp));
	for (i = 1; i < sizeof(data) / sizeof(data[0]); ++i) {
		qp_push_int(qp, data[i].priority, data[i].payload);
		eq(qp_size(qp), size);
		eq(qp_len(qp), i + 1);
		require(!qp_isinvalid(qp));
	}
	for (i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
		payload = qp_peek_int(qp);
		eq(payload, sorted[i].payload);
		payload = qp_pop_int(qp);
		eq(payload, sorted[i].payload);
		eq(qp_size(qp), size);
		eq(qp_len(qp), (sizeof(data) / sizeof(data[0]) - i) - 1);
		require(!qp_isinvalid(qp));
	}
	qp_free(qp, NULL);

	ok;
}

static int test_qp_pointer(void *state)
{
	const size_t size = 15;
	struct ipdata data[14] = {
		{ 8, data + 8 }, { 19, data + 3 }, { 12, data }, { -7, data + 4},
		{ 0, data + 2 }, { 15, data + 9 }, { 1, data + 7 }, { 13, data + 13 },
		{ 9, data + 1 }, { 11, data + 10 }, { 3, data + 5 }, { 16, data + 11 },
		{ -12, data + 6}, { 23, data + 12 },
	};
	struct ipdata sorted[14];
	struct priority_queue *qp = qp_new(size);
	size_t i;
	void *payload;

	memcpy(sorted, data, sizeof(data));
	qsort(sorted, sizeof(data) / sizeof(data[0]), sizeof(data[0]),
		compare_ipdata);

	eq(qp_size(qp), size);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_ptr(qp, data[0].priority, data[0].payload);
	eq(qp_size(qp), size);
	eq(qp_len(qp), 1);
	payload = qp_peek_ptr(qp);
	ptreq(payload, data[0].payload);
	require(!qp_isinvalid(qp));
	for (i = 1; i < sizeof(data) / sizeof(data[0]); ++i) {
		qp_push_ptr(qp, data[i].priority, data[i].payload);
		eq(qp_size(qp), size);
		eq(qp_len(qp), i + 1);
		require(!qp_isinvalid(qp));
	}
	for (i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
		payload = qp_peek_ptr(qp);
		ptreq(payload, sorted[i].payload);
		payload = qp_pop_ptr(qp);
		ptreq(payload, sorted[i].payload);
		require(!qp_isinvalid(qp));
		eq(qp_size(qp), size);
		eq(qp_len(qp), (sizeof(data) / sizeof(data[0]) - i) - 1);
	}
	qp_free(qp, NULL);

	ok;
}

static int test_qp_pushpop(void *state)
{
	struct priority_queue *qp = qp_new(8);
	void *p;
	int i;

	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	i = qp_pushpop_int(qp, 9, 76);
	eq(i, 76);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_int(qp, -7, 34);
	i = qp_pushpop_int(qp, -9, 20);
	eq(i, 20);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	i = qp_pushpop_int(qp, 11, 13);
	eq(i, 34);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	i = qp_pop_int(qp);
	eq(i, 13);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));

	p = qp_pushpop_ptr(qp, 13, &i);
	ptreq(p, &i);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_ptr(qp, 6, &p);
	p = qp_pushpop_ptr(qp, 3, &qp);
	ptreq(p, &qp);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	p = qp_pushpop_ptr(qp, 8, &i);
	ptreq(p, &p);
	eq(qp_len(qp), 1);
	require(!qp_isinvalid(qp));
	p = qp_pop_ptr(qp);
	ptreq(p, &i);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));

	qp_free(qp, NULL);

	ok;
}

static int test_qp_resize(void *state)
{
	struct priority_queue *qp = qp_new(3);
	void *a, *b, *c, *d;
	void *p;
	bool failed;

	a = mem_alloc(1);
	b = mem_alloc(1);
	c = mem_alloc(1);
	d = mem_alloc(1);
	qp_push_ptr(qp, 10, a);
	qp_push_ptr(qp, 8, b);
	qp_push_ptr(qp, 9, c);
	require(!qp_isinvalid(qp));
	eq(qp_len(qp), 3);
	p = qp_peek_ptr(qp);
	ptreq(p, b);
	failed = qp_resize(qp, 10, mem_free);
	require(!failed);
	eq(qp_size(qp), 10);
	eq(qp_len(qp), 3);
	p = qp_peek_ptr(qp);
	ptreq(p, b);
	qp_push_ptr(qp, 12, d);
	eq(qp_len(qp), 4);
	p = qp_pop_ptr(qp);
	ptreq(p, b);
	mem_free(b);
	p = qp_pop_ptr(qp);
	ptreq(p, c);
	mem_free(c);
	p = qp_pop_ptr(qp);
	ptreq(p, a);
	mem_free(a);
	p = qp_pop_ptr(qp);
	ptreq(p, d);
	mem_free(d);
	qp_free(qp, mem_free);

	qp = qp_new(3);
	a = mem_alloc(1);
	b = mem_alloc(1);
	c = mem_alloc(1);
	qp_push_ptr(qp, 10, a);
	qp_push_ptr(qp, 8, b);
	qp_push_ptr(qp, 9, c);
	require(!qp_isinvalid(qp));
	eq(qp_len(qp), 3);
	p = qp_peek_ptr(qp);
	ptreq(p, b);
	failed = qp_resize(qp, 1, mem_free);
	require(!failed);
	require(!qp_isinvalid(qp));
	eq(qp_size(qp), 1);
	eq(qp_len(qp), 1);
	p = qp_peek_ptr(qp);
	ptreq(p, b);
	qp_free(qp, mem_free);

	ok;
}

static int test_qp_flush(void *state)
{
	struct priority_queue *qp = qp_new(3);
	void *a, *b, *c;

	qp_flush(qp, NULL);
	eq(qp_size(qp), 3);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));
	qp_push_int(qp, 7, 21);
	qp_push_int(qp, 5, 32);
	qp_push_int(qp, 10, 19);
	qp_flush(qp, NULL);
	eq(qp_size(qp), 3);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));

	a = mem_alloc(1);
	b = mem_alloc(1);
	c = mem_alloc(1);
	qp_push_ptr(qp, 9, a);
	qp_push_ptr(qp, 12, b);
	qp_push_ptr(qp, 4, c);
	qp_flush(qp, mem_free);
	eq(qp_size(qp), 3);
	eq(qp_len(qp), 0);
	require(!qp_isinvalid(qp));

	qp_free(qp, NULL);

	ok;
}

const char *suite_name = "z-queue/qp";
struct test tests[] = {
	{ "priority queue trivial", test_qp_trivial },
	{ "priority queue integers", test_qp_integer },
	{ "priority queue pointers", test_qp_pointer },
	{ "priority queue pushpop", test_qp_pushpop },
	{ "priority queue resize", test_qp_resize },
	{ "priority queue flush", test_qp_flush },
	{ NULL, NULL }
};
