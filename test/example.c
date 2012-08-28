#include "testrail.h"

_Bool assert_t1(void *unused) { (void)unused; return 2 + 2 == 4; }

struct tr_test t1 = {
	.name = "math still work",
	.assert = assert_t1,
};

_Bool assert_t2(void *unused) { (void)unused; int *p = 0; return *p; }

struct tr_test t2 = {
	.name = "expect segfault",
	.expect = TR_SEGV,
	.assert = assert_t2,
	.next = &t1,
};

struct tr_test tr_g_head = {
	.name = "test foo",
	.body = &t2,
};