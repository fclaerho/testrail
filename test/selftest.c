#include "testrail-dev.h"
#include <signal.h>

/* Basic test assert callbacks.
 */

_Bool false(__attribute__(( unused )) void *data) { return 0; }

_Bool true(__attribute__(( unused )) void *data) { return 1; }

_Bool raisex(void *data) {
	int sig = extosig(*(enum tr_ex*)data);
	raise(sig);
	return 0;
}

/* Sandbox environment.
 */

enum tr_ex g_caught;
jmp_buf g_env;

static void catch(int sig) {
	g_caught = sigtoex(sig);
	longjmp(g_env, 0);
}

struct tr_ctx run_(struct tr_test *t, void *data) {
	FILE *file = tmpfile();
	struct tr_ctx ctx = run(file, &g_caught, &g_env, catch, t, data);
	fclose(file);
	return ctx;
}

static _Bool is_ignored(struct tr_test *t, void *data) { return run_(t, data).res == TR_IGNORED; }

static _Bool is_passed(struct tr_test *t, void *data) { return run_(t, data).res == TR_PASSED; }

static _Bool is_failed(struct tr_test *t, void *data) { return run_(t, data).res == TR_FAILED; }

/* Testrail tests.
 */

		struct {
			enum tr_res arg1, arg2, res;
			const char *desc;
		} op[] = {
			{TR_FAILED, TR_FAILED, TR_FAILED, "failed and failed is failed"},
			{TR_FAILED, TR_PASSED, TR_FAILED, "failed and passed is failed"},
			{TR_FAILED, TR_IGNORED, TR_FAILED, "failed and ignored is failed"},
			{TR_PASSED, TR_FAILED, TR_FAILED, "passed and failed is failed"},
			{TR_PASSED, TR_PASSED, TR_PASSED, "passed and passed is passed"},
			{TR_PASSED, TR_IGNORED, TR_PASSED, "passed and ignored is passed"},
			{TR_IGNORED, TR_FAILED, TR_FAILED, "ignored and failed is failed"},
			{TR_IGNORED, TR_PASSED, TR_PASSED, "ignored and passed is passed"},
			{TR_IGNORED, TR_IGNORED, TR_IGNORED, "ignored and ignored is ignored"},
		};

		TR_TEST(equ, .story = "check: ") {
			size_t i = *(size_t*)data;
			struct tr_test t1 = { .assert = op[i].arg2 == TR_FAILED? false: true, .ignored = op[i].arg2 == TR_IGNORED? 1: 0};
			struct tr_test t2 = { .assert = op[i].arg1 == TR_FAILED? false: true, .ignored = op[i].arg1 == TR_IGNORED? 1: 0, .next = &t1 };
			struct tr_test grp = { .body = &t2 };
			return run_(&grp, 0).res == op[i].res;
		}

		const char* strop(void *data) { return op[*(size_t*)data].desc; }
		
		void* listop(void *p) {
			static _Bool init = 1;
			static size_t i = 0;
			if(init) {
				p = &i;
				init = 0;
			} else {
				++*(size_t*)p;
				if(*(size_t*)p >= sizeof(op)/sizeof(*op)) return 0;
			}
			return p;
		}

	TR_HEAD(aggreg, .story = "check aggregation of results", .body = &equ, .recycle = listop, .strdata = strop)

		TR_TEST(unexpected, .story = "test raising unexpected signal is failed: ") {
			struct tr_test t = { .assert = raisex, .next = 0 };
			return is_failed(&t, data);
		}

		TR_TEST(expected, .story = "test raising expected signal is passed: ", .next = &unexpected) {
			struct tr_test t = { .assert = raisex, .next = 0, .expected = *(enum tr_ex*)data };
			return is_passed(&t, data);
		}

		const char* strex_(void *data) { return strex(*(enum tr_ex*)data); }
		
		void* listex(void *p) {
			static enum tr_ex i = TR_NONE;
			if(i == TR_NONE) p = &i;
			++*(enum tr_ex*)p;
			if(*(enum tr_ex*)p > TR_TERM) return 0;
			return p;
		}

	TR_HEAD(signals, .story = "check signal handling", .body = &expected, .recycle = listex, .strdata = strex_, .next = &aggreg)

		static void* setbeef(__attribute__(( unused )) void *data) {
			if(*(unsigned*)data == 0xDEAD) *(unsigned*)data = 0xBEEF;
			else data = 0;
			return data;
		}

		static _Bool gotbeef(void *data) { return *(unsigned*)data == 0xBEEF; }

		TR_TEST(inherited, .story = "data propagates to body") {
			static unsigned mark = 0xDEAD;
			struct tr_test body = { .assert = gotbeef };
			struct tr_test head = { .recycle = setbeef, .body = &body };
			return is_passed(&head, &mark );
		}

		TR_TEST(input, .story = "recycle() provides input to assert()", .next = &inherited) {
			static unsigned mark = 0xDEAD;
			struct tr_test t = { .recycle = setbeef, .assert = gotbeef };
			return is_passed(&t, &mark);
		}

		void* loop5(__attribute__(( unused )) void *data) {
			static _Bool init = 1;
			static size_t i = 0; /* first result may be 0 */
			if(init) init = 0;
			else {
				++i;
				if(i>=5) return 0;
			}
			return &i;
		}

		static size_t g_counter;

		static _Bool count(__attribute__(( unused )) void *data) {
			++g_counter;
			return 1;
		}

		TR_TEST(loop, .story = "a test is repeated as long as recycle() returns non-null, except the 1st time", .next = &input) {
			struct tr_test t = { .recycle = loop5, .assert = count };
			(void)run_(&t, 0);
			return g_counter == 5;
		}

	TR_HEAD(data, .story = "check recycle()", .body = &loop, .next = &signals)

		TR_TEST(failed, .story = "test returning false is failed") {
			struct tr_test t = { .assert = false };
			return is_failed(&t, 0);
		}

		TR_TEST(passed, .story = "test returning true is passed", .next = &failed) {
			struct tr_test t = { .assert = true };
			return is_passed(&t, 0);
		}

		TR_TEST(ignored, .story = "test marked as ignored is ignored", .next = &passed) {
			struct tr_test t = { .ignored = 1 };
			return is_ignored(&t, 0);
		}

	TR_HEAD(basics, .story = "check basic status", .body = &ignored, .next = &data)

TR_MAIN_HEAD(.story = "check testrail", .body = &basics)
