#ifndef TESTRAILDEV
#define TESTRAILDEV

#include <setjmp.h>
#include <stdio.h>

#include "testrail.h"

enum tr_res { /* WARNING: symbols are sorted by precedence, needed by max() */
	TR_UNKNOWN,
	TR_IGNORED,
	TR_PASSED,
	TR_FAILED,
};

struct tr_ctx {
	unsigned cnt[TR_FAILED + 1];
	void (*handler)(int);
	enum tr_res res;
	enum tr_ex *ex;
	unsigned sum;
	jmp_buf *env;
	FILE *file;
};

struct tr_ctx run(FILE*, enum tr_ex*, jmp_buf*, void(*)(int), struct tr_test*) __attribute__(( nonnull(1, 2, 3, 4, 5) ));

#endif /* TESTRAILDEV */