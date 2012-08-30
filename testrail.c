#define _ANSI_SOURCE 1

#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "testrail-dev.h"

/* Traces.
 */

void vtrace(FILE*, const char*, va_list) __attribute__(( nonnull(1, 2), format(printf, 2, 0) ));

void vtrace(FILE *file, const char *rawfmt, va_list list) {
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

void trace(FILE*, const char*, ...) __attribute__(( nonnull(1, 2), format(printf, 2, 3) ));

void trace(FILE *file, const char *fmt, ...) {
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
	[TR_ABRT] = { "ABRT", SIGABRT },
	[TR_FPE] = { "FPE", SIGFPE },
	[TR_ILL] = { "ILL", SIGILL },
	[TR_INT] = { "INT", SIGINT },
	[TR_SEGV] = { "SEGV", SIGSEGV },
	[TR_TERM] = { "TERM", SIGTERM },
};

static enum tr_ex g_ex;
static jmp_buf g_env;

static void on_sig_set_g_ex_restore_g_env(int sig) {
	for(g_ex = 0; g_exdesc[g_ex].sig != sig; ++g_ex); /* bound not checked */
	longjmp(g_env, 0); /* requires g_env to be set */
}

static _Bool trapsig(void(*)(int)) __attribute__(( nonnull(1) ));

static _Bool trapsig(void(*handler)(int)) {
	_Bool b = 1;
	for(size_t i = 0; i < sizeof(g_exdesc) / sizeof(*g_exdesc) && (b &= signal(g_exdesc[i].sig, handler) != SIG_ERR); ++i);
	return b;
}

/* Tests handling.
 */

const char* strres(enum tr_res r) {
	switch(r) {
		case TR_UNKNOWN: return "<UNKNOWN>";
		case TR_IGNORED: return "[ignored]";
		case TR_PASSED: return "[passed]";
		case TR_FAILED: return "<FAILED>";
		default: return "<OTHER?>";
	}
}

enum tr_res max(enum tr_res cur, enum tr_res new) { return cur > new? cur: new; }

_Bool stop(enum tr_mode m, enum tr_res r) { return m == TR_STOP_ON_FAILURE && r == TR_FAILED; }

typedef size_t tr_depth;

enum tr_res runlist(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode) __attribute__(( nonnull(1, 2) ));

enum tr_res runtest(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode) __attribute__(( nonnull(1, 2) ));

const char* prefix(tr_depth d) {
	static char s[32];
	strcpy(s + (2 * d), "+- ");
	memset(s, ' ', 2 * d);
	s[2 * d + 3] = 0;
	return s;
}

enum tr_res runtest(struct tr_ctx *ctx, struct tr_test *t, tr_depth d, enum tr_mode m) {
	if(t->mode != TR_INHERITED) m = t->mode;
	enum tr_res r = TR_UNKNOWN;
	if(t->ignored) {
		r = TR_IGNORED;
	} else {
		if(!setjmp(*(ctx->env))) {
			if(trapsig(ctx->handler)) {
				if(t->setup) t->setup();
				if(t->expect) r = t->expect()? TR_PASSED: TR_FAILED;
				if(t->cleanup) t->cleanup();
			} else {
				trace(ctx->file, "%scannot install signal handlers", prefix(d + 1));
				r = TR_FAILED;
			}
			if(t->body && !stop(m, r)) r = max(r, runlist(ctx, t->body, d + 1, m));
		} else {
			_Bool b = t->caught == *(ctx->ex);
			trace(ctx->file, "%scaught %s exception (%u, %s)", prefix(d + 1), b? "expected": "unexpected", *(ctx->ex), g_exdesc[*(ctx->ex)].s);
			r = b? TR_PASSED: TR_FAILED;
		}
	}
	trace(ctx->file, "%s%s: %s", prefix(d), t->story, strres(r));
	ctx->cnt[r] += 1;
	ctx->sum += 1;
	return r;
}

enum tr_res runlist(struct tr_ctx *ctx, struct tr_test *head, tr_depth d, enum tr_mode m) {
	enum tr_res r = TR_UNKNOWN;
	for(struct tr_test *t = head; t && !stop(m, r); t = t->next) r = max(r, runtest(ctx, t, d, m));
	return r;
}

struct tr_ctx run(FILE *file, enum tr_ex *ex, jmp_buf *env, void (*handler)(int), struct tr_test *head) {
	struct tr_ctx ctx = {
		.handler = handler,
		.file = file,
		.env = env,
		.ex = ex,
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
	struct tr_ctx ctx = run(stderr, &g_ex, &g_env, on_sig_set_g_ex_restore_g_env, &tr_g_head);
	return ctx.cnt[TR_UNKNOWN] + ctx.cnt[TR_FAILED];
}
