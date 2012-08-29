#include "testrail-dev.h"

/* Tested tests.
 */

_Bool false(void *unused) { (void)unused; return 0; }

struct tr_test failed_test = {
	.name = "failed test",
	.assert = false,
};

_Bool true(void *unused) { (void)unused; return 1; }

struct tr_test passed_test = {
	.name = "passed test",
	.assert = true,
};

/* Tests.
 */

enum tr_ex g_ex;
jmp_buf g_env;
FILE *g_file;

__attribute__(( constructor )) void setup(void) { g_file = fopen("/dev/null", "w"); }

__attribute__(( destructor )) void cleanup(void) { fclose(g_file); }

static void undo(int sig) {
	g_ex = TR_OTHER;
	longjmp(g_env, 0);
}

_Bool assert_failed_test_is_failed(void *unused) {
	(void)unused;
	return run(g_file, &g_ex, &g_env, undo, &failed_test).res == TR_FAILED;
}

struct tr_test failed_test_is_failed = {
	.name = "a failed test shall not pass",
	.assert = assert_failed_test_is_failed,
};

_Bool assert_passed_test_is_passed(void *unused) {
	(void)unused;
	return run(g_file, &g_ex, &g_env, undo, &passed_test).res == TR_PASSED;
}

struct tr_test passed_test_is_passed = {
	.name = "a passed test shall pass",
	.assert = assert_passed_test_is_passed,
	.next = &failed_test_is_failed,
};

struct tr_test tr_g_head = {
	.name = "test framework",
	.body = &passed_test_is_passed,
};