#define _ANSI_SOURCE 1

#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "testrail-test.h"

/* Traces.
 */

static void vtrace(FILE*, const char*, va_list) __attribute__(( nonnull(1, 2), format(printf, 2, 0) ));

static void vtrace(FILE *file, const char *rawfmt, va_list list) {
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
	vfprintf(file, fmt, list);
}

static void trace(FILE*, const char*, ...) __attribute__(( nonnull(1, 2), format(printf, 2, 3) ));

static void trace(FILE *file, const char *fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vtrace(file, fmt, list);
	va_end(list);
}

/* Exception handling.
 */

static struct {
	const char *s;
	int sig;
} g_exdesc[] = {
	[TR_NONE] = { "(no signal)", -1 },
	[TR_ABRT] = { "ABRT", SIGABRT },
	[TR_FPE] = { "FPE", SIGFPE },
	[TR_ILL] = { "ILL", SIGILL },
	[TR_INT] = { "INT", SIGINT },
	[TR_SEGV] = { "SEGV", SIGSEGV },
	[TR_TERM] = { "TERM", SIGTERM },
};

static enum tr_ex g_caught;
static jmp_buf g_env;

enum tr_ex sigtoex(int sig) { for(enum tr_ex i = TR_NONE + 1; ; ++i) if(g_exdesc[i].sig == sig) return i; }

static void catch(int sig) {
	g_caught = sigtoex(sig);
	longjmp(g_env, 0);
}

static _Bool trapsig(void(*)(int)) __attribute__(( nonnull(1) ));

static _Bool trapsig(void(*handler)(int)) {
	_Bool b = 1;
	for(enum tr_ex i = TR_NONE + 1; i < sizeof(g_exdesc) / sizeof(*g_exdesc) && (b &= signal(g_exdesc[i].sig, handler) != SIG_ERR); ++i);
	return b;
}

/* Tests handling.
 */

static const char* strres(enum tr_res r) {
	switch(r) {
		case TR_UNKNOWN: return "<UNKNOWN>";
		case TR_IGNORED: return "[ignored]";
		case TR_PASSED: return "[passed]";
		case TR_FAILED: return "<FAILED>";
		default: return "<OTHER?>";
	}
}

static enum tr_res max(enum tr_res cur, enum tr_res new) { return cur > new? cur: new; }

static _Bool stop(enum tr_mode m, enum tr_res r) { return m == TR_STOP_ON_FAILURE && r == TR_FAILED; }

typedef size_t tr_depth;

static enum tr_res runlist(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode) __attribute__(( nonnull(1, 2) ));

static enum tr_res runtest(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode) __attribute__(( nonnull(1, 2) ));

static const char* prefix(tr_depth d, _Bool new) {
	static char s[32];
	s[0] = 0;
	for(size_t i = 0; i < sizeof(s) && i < d; ++i) (void)strcat(s, "|  ");
	return strcat(s, new? "+- ": "|  ");
}

static enum tr_res runtest(struct tr_ctx *ctx, struct tr_test *t, tr_depth d, enum tr_mode m) {
	if(t->mode != TR_INHERITED) m = t->mode;
	enum tr_res r = TR_UNKNOWN;
	trace(ctx->file, "%s%s", prefix(d, 1), t->story);
	if(t->ignored) {
		r = TR_IGNORED;
	} else {
		if(trapsig(ctx->handler)) {
			void *usrp = 0;
			if(t->setup) usrp = t->setup();
			if(!setjmp(*(ctx->env))) {
				if(t->assert) r = t->assert(usrp)? TR_PASSED: TR_FAILED;
			} else {
				_Bool b = t->expected == *(ctx->caught);
				trace(ctx->file, "%scaught %sexpected exception (%u, %s)", prefix(d, 0), b? "": "un", *(ctx->caught), g_exdesc[*(ctx->caught)].s);
				r = b? TR_PASSED: TR_FAILED;
			}
			if(t->cleanup) t->cleanup(usrp);
		} else {
			trace(ctx->file, "%scannot trap signals", prefix(d + 1, 0));
			r = TR_FAILED;
		}
		if(t->body && !stop(m, r)) r = max(r, runlist(ctx, t->body, d + 1, m));
	}
	trace(ctx->file, "%s%s", prefix(d, 0), strres(r));
	ctx->cnt[r] += 1;
	ctx->sum += 1;
	return r;
}

static enum tr_res runlist(struct tr_ctx *ctx, struct tr_test *head, tr_depth d, enum tr_mode m) {
	enum tr_res r = TR_UNKNOWN;
	for(struct tr_test *t = head; t && !stop(m, r); t = t->next) r = max(r, runtest(ctx, t, d, m));
	return r;
}

struct tr_ctx run(FILE *file, enum tr_ex *caught, jmp_buf *env, void (*handler)(int), struct tr_test *head) {
	struct tr_ctx ctx = {
		.handler = handler,
		.caught = caught,
		.file = file,
		.env = env,
	};
	ctx.res = runlist(&ctx, head, 0, TR_RESUME_ON_FAILURE);
	trace(file, "summary: %u node(s), %u unknown, %u ignored, %u passed, %u failed, %s.",
		ctx.sum,
		ctx.cnt[TR_UNKNOWN],
		ctx.cnt[TR_IGNORED],
		ctx.cnt[TR_PASSED],
		ctx.cnt[TR_FAILED],
		ctx.cnt[TR_UNKNOWN] + ctx.cnt[TR_FAILED] == 0? "OK": "FAILED");
	return ctx;
}

int main(void) {
	struct tr_ctx ctx = run(stderr, &g_caught, &g_env, catch, &tr_g_head);
	return ctx.cnt[TR_UNKNOWN] + ctx.cnt[TR_FAILED];
}
