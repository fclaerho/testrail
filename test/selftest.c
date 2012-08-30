#include "testrail-dev.h"
#include <signal.h>

/* Tested tests.
 */

TR_TEST(failed_test) { return 0; }

TR_TEST(passed_test) { return 1; }

	TR_TEST(failed_test2) { return 0; }

	TR_TEST(passed_test2, .next = &failed_test2) { return 1; }

TR_HEAD(passed_and_failed, .body = &passed_test2)

	TR_TEST(failed_test3) { return 0; }

	TR_TEST(failed_test4, .next = &failed_test3) { return 0; }

TR_HEAD(failed_and_failed, .body = &failed_test4)

	TR_TEST(passed_test3) { return 1; }

	TR_TEST(passed_test4, .next = &passed_test3) { return 1; }

TR_HEAD(passed_and_passed, .body = &passed_test4)

	TR_TEST(passed_test5) { return 1; }

	TR_TEST(failed_test5, .next = &passed_test5) { return 0; }

TR_HEAD(failed_and_passed, .body = &failed_test5)

TR_TEST(expected_abrt, .caught = TR_ABRT) { raise(SIGABRT); return 0; }

TR_TEST(unexpected_abrt) { raise(SIGABRT); return 0; }

TR_TEST(expected_fpe, .caught = TR_FPE) { raise(SIGFPE); return 0; }

TR_TEST(unexpected_fpe) { raise(SIGFPE); return 0; }

TR_TEST(expected_ill, .caught = TR_ILL) { raise(SIGILL); return 0; }

TR_TEST(unexpected_ill) { raise(SIGILL); return 0; }

TR_TEST(expected_segv, .caught = TR_SEGV) { raise(SIGSEGV); return 0; }

TR_TEST(unexpected_segv) { raise(SIGSEGV); return 0; }

/* Env.
 */

enum tr_ex g_ex;
jmp_buf g_env;
FILE *g_file;

__attribute__(( constructor )) void setup(void) { g_file = fopen("/dev/null", "w"); }

__attribute__(( destructor )) void cleanup(void) { fclose(g_file); }

static void undo(int sig) {
	g_ex = TR_SEGV; /* FIXME */
	longjmp(g_env, 0);
}

/* Tests.
 */

		TR_TEST(failed_test_is_failed) {
			return run(g_file, &g_ex, &g_env, undo, &failed_test).res == TR_FAILED;
		}

		TR_TEST(passed_test_is_passed, .next = &failed_test_is_failed) {
			return run(g_file, &g_ex, &g_env, undo, &passed_test).res == TR_PASSED;
		}

	TR_HEAD(basics, .story = "check basics", .body = &passed_test_is_passed)

		TR_TEST(passed_and_failed_is_failed) {
			return run(g_file, &g_ex, &g_env, undo, &passed_and_failed).res == TR_FAILED;
		}

		TR_TEST(failed_and_failed_is_failed, .next = &passed_and_failed_is_failed) {
			return run(g_file, &g_ex, &g_env, undo, &failed_and_failed).res == TR_FAILED;
		}

		TR_TEST(passed_and_passed_is_passed, .next = &failed_and_failed_is_failed) {
			return run(g_file, &g_ex, &g_env, undo, &passed_and_passed).res == TR_PASSED;
		}

		TR_TEST(failed_and_passed_is_failed, .next = &passed_and_passed_is_passed) {
			return run(g_file, &g_ex, &g_env, undo, &failed_and_passed).res == TR_FAILED;
		}

	TR_HEAD(list, .story = "check list result", .body = &failed_and_passed_is_failed, .next = &basics)

		TR_TEST(expected_segv_is_passed) {
			return run(g_file, &g_ex, &g_env, undo, &expected_segv).res == TR_PASSED;
		}

		TR_TEST(unexpected_segv_is_failed, .next = &expected_segv_is_passed) {
			return run(g_file, &g_ex, &g_env, undo, &unexpected_segv).res == TR_FAILED;
		}

	TR_HEAD(signals, .story = "check signal handling", .body = &unexpected_segv_is_failed, .next = &list)

TR_G_HEAD(.story = "check testrail", .body = &signals)
