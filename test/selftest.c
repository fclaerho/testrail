#include "testrail-test.h"
#include <signal.h>

/* Sandboxed tests.
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

unsigned marker;

void* setup_marker(void) { marker = 0xBEEF; return (void*)&marker; }

void cleanup_marker(void *p) { *(unsigned*)p = 0xDEAD; }

TR_TEST(setup_works, .setup = setup_marker, .cleanup = cleanup_marker) { return data? *(unsigned*)data == 0xBEEF: 0; }

	TR_TEST(body_has_data) { return data? *(unsigned*)data == 0xBEEF: 0; }

TR_HEAD(setup_data_propagates_to_body, .setup = setup_marker, .cleanup = cleanup_marker, .body = &body_has_data)

/* Sandbox environment.
 */

enum tr_ex my_caught;
jmp_buf my_env;
FILE *my_file;

void* my_setup(void) {
	my_file = tmpfile();
	return 0;
}

void my_cleanup(__attribute__(( unused )) void *p) {
	(void)fclose(my_file);
	my_file = 0;
}

static void my_catch(int sig) {
	my_caught = sigtoex(sig);
	longjmp(my_env, 0);
}

static _Bool is_ignored(struct tr_test *t) { return run(my_file, &my_caught, &my_env, my_catch, t).res == TR_IGNORED; }

static _Bool is_passed(struct tr_test *t) { return run(my_file, &my_caught, &my_env, my_catch, t).res == TR_PASSED; }

static _Bool is_failed(struct tr_test *t) { return run(my_file, &my_caught, &my_env, my_catch, t).res == TR_FAILED; }

/* Testrail tests.
 */

		TR_TEST(setup_data_propagates_to_body_is_passed) {
			marker = 0xAAAA;
			return is_passed(&setup_data_propagates_to_body); /* assumes aggregation of results works */
		}

		TR_TEST(cleanup_works, .next = &setup_data_propagates_to_body_is_passed) {
			marker = 0xAAAA;
			(void)is_passed(&setup_works); /* now assumed ok */
			return marker == 0xDEAD;
		}

		TR_TEST(setup_works_is_passed, .next = &cleanup_works) {
			marker = 0xAAAA;
			return is_passed(&setup_works);
		}

	TR_HEAD(setup_cleanup, .story = "check setup() and cleanup()", .body = &setup_works_is_passed)

		TR_TEST(failed_test_is_failed) { return is_failed(&failed_test); }

		TR_TEST(passed_test_is_passed, .next = &failed_test_is_failed) { return is_passed(&passed_test); }

		TR_TEST(ignored_test_is_ignored, .next = &passed_test_is_passed) { return is_ignored(&ignored_test); }

	TR_HEAD(basics, .story = "check basic status", .body = &ignored_test_is_ignored, .next = &setup_cleanup)

		TR_TEST(failed_and_failed_is_failed) { return is_failed(&failed_and_failed); }

		TR_TEST(failed_and_passed_is_failed, .next = &failed_and_failed_is_failed) { return is_failed(&failed_and_passed); }

		TR_TEST(failed_and_ignored_is_failed, .next = &failed_and_passed_is_failed) { return is_failed(&failed_and_ignored); }

		TR_TEST(passed_and_failed_is_failed, .next = &failed_and_ignored_is_failed) { return is_failed(&passed_and_failed); }

		TR_TEST(passed_and_passed_is_passed, .next = &passed_and_failed_is_failed) { return is_passed(&passed_and_passed); }

		TR_TEST(passed_and_ignored_is_passed, .next = &passed_and_passed_is_passed) { return is_passed(&passed_and_ignored); }

		TR_TEST(ignored_and_failed_is_failed, .next = &passed_and_ignored_is_passed) { return is_failed(&ignored_and_failed); }

		TR_TEST(ignored_and_passed_is_passed, .next = &ignored_and_failed_is_failed) { return is_passed(&ignored_and_passed); }

		TR_TEST(ignored_and_ignored_is_ignored, .next = &ignored_and_passed_is_passed) { return is_ignored(&ignored_and_ignored); }

	TR_HEAD(list, .story = "check aggregation of results", .body = &ignored_and_ignored_is_ignored, .next = &basics)

		TR_TEST(expected_sigabrt_is_passed) { return is_passed(&expected_sigabrt); }

		TR_TEST(unexpected_sigabrt_is_failed, .next = &expected_sigabrt_is_passed) { return is_failed(&unexpected_sigabrt); }

		TR_TEST(expected_sigfpe_is_passed, .next = &unexpected_sigabrt_is_failed) { return is_passed(&expected_sigfpe); }

		TR_TEST(unexpected_sigfpe_is_failed, .next = &expected_sigfpe_is_passed) { return is_failed(&unexpected_sigfpe); }

		TR_TEST(expected_sigill_is_passed, .next = &unexpected_sigfpe_is_failed) { return is_passed(&expected_sigill); }

		TR_TEST(unexpected_sigill_is_failed, .next = &expected_sigill_is_passed) { return is_failed(&unexpected_sigill); }

		TR_TEST(expected_sigint_is_passed, .next = &unexpected_sigill_is_failed) { return is_passed(&expected_sigint); }

		TR_TEST(unexpected_sigint_is_failed, .next = &expected_sigint_is_passed) { return is_failed(&unexpected_sigint); }

		TR_TEST(expected_sigsegv_is_passed, .next = &unexpected_sigint_is_failed) { return is_passed(&expected_sigsegv); }

		TR_TEST(unexpected_sigsegv_is_failed, .next = &expected_sigsegv_is_passed) { return is_failed(&unexpected_sigsegv); }

		TR_TEST(expected_sigterm_is_passed, .next = &unexpected_sigsegv_is_failed) { return is_passed(&expected_sigterm); }

		TR_TEST(unexpected_sigterm_is_failed, .next = &expected_sigterm_is_passed) { return is_failed(&unexpected_sigterm); }

	TR_HEAD(signals, .story = "check signal handling", .body = &unexpected_sigterm_is_failed, .next = &list)

TR_MAIN_HEAD(.story = "check testrail", .body = &signals, .setup = my_setup, .cleanup = my_cleanup)
