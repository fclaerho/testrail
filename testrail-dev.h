/* Testrail internal interfaces.
 */

#ifndef TESTRAIL_TEST
#define TESTRAIL_TEST

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
	enum tr_ex *caught;
	enum tr_res res;
	unsigned sum;
	jmp_buf *env;
	FILE *file;
};

int extosig(enum tr_ex);

enum tr_ex sigtoex(int);

const char* strex(enum tr_ex);

const char* strres(enum tr_res);

struct tr_ctx run(FILE*, enum tr_ex*, jmp_buf*, void(*)(int), struct tr_test*, void*) __attribute__(( nonnull(1, 2, 3, 4, 5) ));

#endif /* TESTRAIL_TEST */
