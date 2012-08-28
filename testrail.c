#define _ANSI_SOURCE 1

#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "testrail.h"

/* Traces are redirected to the unbuffered standard error stream.
 */

void vtrace(const char*, va_list) __attribute__(( nonnull(1), format(printf, 1, 0) ));

void vtrace(const char *rawfmt, va_list list) {
	time_t t = time(0);
	struct tm tm = *localtime(&t);
	char prefix[20];
	(void)strftime(prefix, sizeof(prefix), "%d/%m/%Y-%T", &tm);
	char *sep = ": ";
	char *suffix = "\n";
	char fmt[strlen(prefix) + strlen(sep) + strlen(rawfmt) + strlen(suffix) + 1];
	strcpy(fmt, prefix);
	strcat(fmt, sep);
	strcat(fmt, rawfmt);
	strcat(fmt, suffix);
	vfprintf(stderr, fmt, list);
}

void trace(const char*, ...) __attribute__(( nonnull(1), format(printf, 1, 2) ));

void trace(const char *fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vtrace(fmt, list);
	va_end(list);
}

/* Termination routine.
 */

void fail(const char*, ...) __attribute__(( noreturn, nonnull(1), format(printf, 1, 2) ));

void fail(const char *rawfmt, ...) {
	va_list list;
	va_start(list, rawfmt);
	char *prefix = "fatal error: ";
	char *suffix = ", execution terminated";
	char fmt[strlen(prefix) + strlen(rawfmt) + strlen(suffix) + 1];
	strcpy(fmt, prefix);
	strcat(fmt, fmt);
	strcat(fmt, suffix);
	vtrace(fmt, list);
	va_end(list);
	exit(EXIT_FAILURE);
}

/* Exception handling.
 */

static struct {
	const char *s;
	int sig;
} g_exdesc[] = {
	[TR_ABRT] = { "ABRT", SIGABRT },
	[TR_FP] = { "FP", SIGFPE },
	[TR_ILL] = { "ILL", SIGILL },
	[TR_INT] = { "INT", SIGINT },
	[TR_SEGV] = { "SEGV", SIGSEGV },
	[TR_TERM] = { "TERM", SIGTERM },
};

static size_t g_exdesc_max = sizeof(g_exdesc) / sizeof(*g_exdesc);
static enum tr_ex g_ex;
static jmp_buf g_env;

static void undo(int sig) {
	g_ex = -1;
	for(size_t i = 0; i < g_exdesc_max; ++i) if(g_exdesc[i].sig == sig) g_ex = i;
	if(g_ex == -1) fail("caught non-standard signal %i", sig); /* FIXME */
	longjmp(g_env, 0); /* requires g_env to be set */
}

static void trapsig(void) {
	for(int i = 0; i < g_exdesc_max; ++i)
		if(signal(g_exdesc[i].sig, undo) == SIG_ERR)
			fail("cannot trap signal (%i, %s), %s", g_exdesc[i].sig, g_exdesc[i].s, strerror(errno));
}

/* Tests handling.
 */

enum tr_res { /* the order is important here! don't move symbols, see merge() */
	TR_UNKNOWN,
	TR_IGNORED,
	TR_PASSED,
	TR_FAILED,
};

enum tr_res merge(enum tr_res cur, enum tr_res new) { return cur > new? cur: new; } /* i.e. max */

const char* strres(enum tr_res r) {
	switch(r) {
		case TR_UNKNOWN: return "<UNKNOWN>";
		case TR_IGNORED: return "[ignored]";
		case TR_PASSED: return "[passed]";
		case TR_FAILED: return "<FAILED>";
		default: fail("unknown result value %u", r);
	}
}

typedef size_t tr_depth;

enum tr_res runtest(tr_depth, struct tr_test*, enum tr_mode) __attribute__(( nonnull(2) ));

enum tr_res runlist(tr_depth d, struct tr_test *t, enum tr_mode m) {
	enum tr_res r = TR_UNKNOWN;
	for(; t; t = t->next) r = merge(r, runtest(d, t, m));
	return r;
}

struct tr_stat {
	unsigned res[TR_FAILED + 1];
	unsigned total;
} g_stats;

enum tr_res runtest(tr_depth d, struct tr_test *t, enum tr_mode m) {
	if(t->mode != TR_INHERITED) m = t->mode;
	enum tr_res r = TR_UNKNOWN;
	char offset[2 * d + 4];
	strcpy(offset + (2 * d), "+- ");
	memset(offset, ' ', 2 * d);
	offset[2 * d + 3] = 0;
	if(t->ignored) {
		r = TR_IGNORED;
	} else {
		if(!setjmp(g_env)) {
			trace("%sstart test '%s'...", offset, t->name);
			trapsig();
			void *ctx;
			if(t->setup) ctx = t->setup();
			if(t->assert) r = t->assert(ctx)? TR_PASSED: TR_FAILED;
			if(t->cleanup) t->cleanup(ctx);
			if(t->body && !(m == TR_STOP_ON_FAILURE && r == TR_FAILED)) r = merge(r, runlist(d + 1, t->body, m));
		} else {
			_Bool b = t->expect == g_ex;
			trace("%scaught %s exception (%u, %s)", offset, b? "expected": "unexpected", g_ex, g_exdesc[g_ex].s);
			r = b? TR_PASSED: TR_FAILED;
		}
	}
	trace("%send test '%s': %s", offset, t->name, strres(r));
	++g_stats.res[r];
	++g_stats.total;
	if(m == TR_STOP_ON_FAILURE && r == TR_FAILED) fail("test failed");
	return r;
}

void sumup(void) {
	trace("summary: %u test(s), %u unknown, %u ignored, %u failed, %u passed, %s.",
		g_stats.total,
		g_stats.res[TR_UNKNOWN],
		g_stats.res[TR_IGNORED],
		g_stats.res[TR_FAILED],
		g_stats.res[TR_PASSED],
		g_stats.res[TR_PASSED] + g_stats.res[TR_IGNORED] == g_stats.total? "OK": "KO :(");
}

int main(void) {
	atexit(sumup);
	runlist(0, &tr_g_head, TR_RESUME_ON_FAILURE);;
}
