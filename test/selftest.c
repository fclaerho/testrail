#include "testrail-test.h"
#include <signal.h>

/* Tested tests.
 */

TR_TEST(failed_test) { return 0; }

TR_TEST(passed_test) { return 1; }

TR_TEST(ignored_test, .ignored = 1) { return 0; }

	TR_TEST(failed_test2) { return 0; }

	TR_TEST(failed_test3, .next = &failed_test2) { return 0; }

TR_HEAD(failed_and_failed, .body = &failed_test3)

	TR_TEST(passed_test2) { return 1; }

	TR_TEST(failed_test4, .next = &passed_test2) { return 0; }

TR_HEAD(failed_and_passed, .body = &failed_test4)

	TR_TEST(ignored_test2, .ignored = 1) { return 0; }

	TR_TEST(failed_test5, .next = &ignored_test2) { return 0; }

TR_HEAD(failed_and_ignored, .body = &failed_test5)

	TR_TEST(failed_test6) { return 0; }

	TR_TEST(passed_test3, .next = &failed_test6) { return 1; }

TR_HEAD(passed_and_failed, .body = &passed_test3)

	TR_TEST(passed_test4) { return 1; }

	TR_TEST(passed_test5, .next = &passed_test4) { return 1; }

TR_HEAD(passed_and_passed, .body = &passed_test5)

	TR_TEST(ignored_test3, .ignored = 1) { return 0; }

	TR_TEST(passed_test6, .next = &ignored_test3) { return 1; }

TR_HEAD(passed_and_ignored, .body = &passed_test6)

	TR_TEST(failed_test7) { return 0; }

	TR_TEST(ignored_test4, .ignored = 1, .next = &failed_test7) { return 0; }

TR_HEAD(ignored_and_failed, .body = &ignored_test4)

	TR_TEST(passed_test7) { return 1; }

	TR_TEST(ignored_test5, .ignored = 1, .next = &passed_test7) { return 0; }

TR_HEAD(ignored_and_passed, .body = &ignored_test5)

	TR_TEST(ignored_test6, .ignored = 1) { return 0; }

	TR_TEST(ignored_test7, .ignored = 1, .next = &ignored_test6) { return 0; }

TR_HEAD(ignored_and_ignored, .body = &ignored_test7)

TR_TEST(expected_sigabrt, .expected = TR_ABRT) { raise(SIGABRT); return 0; }

TR_TEST(unexpected_sigabrt) { raise(SIGABRT); return 0; }

TR_TEST(expected_sigfpe, .expected = TR_FPE) { raise(SIGFPE); return 0; }

TR_TEST(unexpected_sigfpe) { raise(SIGFPE); return 0; }

TR_TEST(expected_sigill, .expected = TR_ILL) { raise(SIGILL); return 0; }

TR_TEST(unexpected_sigill) { raise(SIGILL); return 0; }

TR_TEST(expected_sigint, .expected = TR_INT) { raise(SIGINT); return 0; }

TR_TEST(unexpected_sigint) { raise(SIGINT); return 0; }

TR_TEST(expected_sigsegv, .expected = TR_SEGV) { raise(SIGSEGV); return 0; }

TR_TEST(unexpected_sigsegv) { raise(SIGSEGV); return 0; }

TR_TEST(expected_sigterm, .expected = TR_TERM) { raise(SIGTERM); return 0; }

TR_TEST(unexpected_sigterm) { raise(SIGTERM); return 0; }

/* Env.
 */

enum tr_ex g_caught;
jmp_buf g_env;
FILE *g_file;

void* setup(void) {
	g_file = tmpfile();
	return 0;
}

void cleanup(__attribute__(( unused )) void *p) {
	(void)fclose(g_file);
	g_file = 0;
}

static void catch(int sig) {
	g_caught = sigtoex(sig);
	longjmp(g_env, 0);
}

/* Tests.
 */

		TR_TEST(failed_test_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &failed_test).res == TR_FAILED;
		}

		TR_TEST(passed_test_is_passed, .next = &failed_test_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &passed_test).res == TR_PASSED;
		}

		TR_TEST(ignored_test_is_ignored, .next = &passed_test_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &ignored_test).res == TR_IGNORED;
		}

	TR_HEAD(basics, .story = "check basic status", .body = &ignored_test_is_ignored)

		TR_TEST(failed_and_failed_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &failed_and_failed).res == TR_FAILED;
		}

		TR_TEST(failed_and_passed_is_failed, .next = &failed_and_failed_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &failed_and_passed).res == TR_FAILED;
		}

		TR_TEST(failed_and_ignored_is_failed, .next = &failed_and_passed_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &failed_and_ignored).res == TR_FAILED;
		}

		TR_TEST(passed_and_failed_is_failed, .next = &failed_and_ignored_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &passed_and_failed).res == TR_FAILED;
		}

		TR_TEST(passed_and_passed_is_passed, .next = &passed_and_failed_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &passed_and_passed).res == TR_PASSED;
		}

		TR_TEST(passed_and_ignored_is_passed, .next = &passed_and_passed_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &passed_and_ignored).res == TR_PASSED;
		}

		TR_TEST(ignored_and_failed_is_failed, .next = &passed_and_ignored_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &ignored_and_failed).res == TR_FAILED;
		}

		TR_TEST(ignored_and_passed_is_passed, .next = &ignored_and_failed_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &ignored_and_passed).res == TR_PASSED;
		}

		TR_TEST(ignored_and_ignored_is_ignored, .next = &ignored_and_passed_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &ignored_and_ignored).res == TR_IGNORED;
		}

	TR_HEAD(list, .story = "check aggregation of results", .body = &ignored_and_ignored_is_ignored, .next = &basics)

		TR_TEST(expected_sigabrt_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigabrt).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigabrt_is_failed, .next = &expected_sigabrt_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigabrt).res == TR_FAILED;
		}

		TR_TEST(expected_sigfpe_is_passed, .next = &unexpected_sigabrt_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigfpe).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigfpe_is_failed, .next = &expected_sigfpe_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigfpe).res == TR_FAILED;
		}

		TR_TEST(expected_sigill_is_passed, .next = &unexpected_sigfpe_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigill).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigill_is_failed, .next = &expected_sigill_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigill).res == TR_FAILED;
		}

		TR_TEST(expected_sigint_is_passed, .next = &unexpected_sigill_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigint).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigint_is_failed, .next = &expected_sigint_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigint).res == TR_FAILED;
		}

		TR_TEST(expected_sigsegv_is_passed, .next = &unexpected_sigint_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigsegv).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigsegv_is_failed, .next = &expected_sigsegv_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigsegv).res == TR_FAILED;
		}

		TR_TEST(expected_sigterm_is_passed, .next = &unexpected_sigsegv_is_failed) {
			return run(g_file, &g_caught, &g_env, catch, &expected_sigterm).res == TR_PASSED;
		}

		TR_TEST(unexpected_sigterm_is_failed, .next = &expected_sigterm_is_passed) {
			return run(g_file, &g_caught, &g_env, catch, &unexpected_sigterm).res == TR_FAILED;
		}

	TR_HEAD(signals, .story = "check signal handling", .body = &unexpected_sigterm_is_failed, .next = &list)

TR_MAIN_HEAD(.story = "check testrail", .body = &signals, .setup = setup, .cleanup = cleanup)
