#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "testrail-dev.h"

/* Traces.
 */

static void vtrace(FILE*, const char*, va_list) __attribute__(( nonnull, format(printf, 2, 0) ));

static void vtrace(FILE *file, const char *usrfmt, va_list list) {
	time_t t = time(0);
	struct tm tm = *localtime(&t);
	char prefix[20]; /* length of the standard expansion of %d/%m/%Y-%T */
	(void)strftime(prefix, sizeof(prefix), "%d/%m/%Y-%T", &tm);
	char *sep = ": ";
	char *suffix = "\n";
	char fmt[strlen(prefix) + strlen(sep) + strlen(usrfmt) + strlen(suffix) + 1];
	(void)strcpy(fmt, prefix);
	(void)strcat(fmt, sep);
	(void)strcat(fmt, usrfmt);
	(void)strcat(fmt, suffix);
	(void)vfprintf(file, fmt, list);
}

static void trace(FILE*, const char*, ...) __attribute__(( nonnull, format(printf, 2, 3) ));

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

const char* strex(enum tr_ex i) { return g_exdesc[i].s; }

int extosig(enum tr_ex i) { return g_exdesc[i].sig; }

enum tr_ex sigtoex(int sig) { for(enum tr_ex i = TR_NONE + 1; ; ++i) if(g_exdesc[i].sig == sig) return i; }

static void catch(int sig) {
	g_caught = sigtoex(sig);
	longjmp(g_env, 0);
}

void trapsig(void(*)(int)) __attribute__(( nonnull ));

void trapsig(void(*handler)(int)) {
	for(enum tr_ex i = TR_NONE + 1; i < sizeof(g_exdesc) / sizeof(*g_exdesc); ++i)
		if(signal(g_exdesc[i].sig, handler) == SIG_ERR) {
			perror("cannot trap signal");
			exit(EXIT_FAILURE);
		}
}

/* Tests handling.
 */

const char* strres(enum tr_res r) {
	switch(r) {
		case TR_UNKNOWN: return "*UNKNOWN*";
		case TR_IGNORED: return "ignored";
		case TR_PASSED: return "passed";
		case TR_FAILED: return "*FAILED*";
		default: return "illegal result";
	}
}

static enum tr_res max(enum tr_res cur, enum tr_res new) { return cur > new? cur: new; }

static _Bool stop(enum tr_mode m, enum tr_res r) { return m == TR_STOP_ON_FAILURE && r == TR_FAILED; }

typedef size_t tr_depth;

static const char* prefix(tr_depth d, _Bool new) {
	static char s[32];
	s[0] = 0;
	for(size_t i = 0; i < sizeof(s) && i < d; ++i) (void)strcat(s, "|  ");
	return strcat(s, new? "+- ": "|  ");
}

double elapsed(clock_t t0) { return ((double)clock() - t0) / CLOCKS_PER_SEC; }

static enum tr_res runlist(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode, const char*(*)(void*), void*) __attribute__(( nonnull(1, 2) ));

static enum tr_res runtest(struct tr_ctx*, struct tr_test*, tr_depth, enum tr_mode, const char*(*)(void*), void*) __attribute__(( nonnull(1, 2) ));

static enum tr_res runtest(struct tr_ctx *ctx, struct tr_test *t, tr_depth d, enum tr_mode mode, const char* (*strdata)(void*), void *data) {
	if(t->mode != TR_INHERITED) mode = t->mode;
	if(t->strdata) strdata = t->strdata;
	if(t->recycle) data = t->recycle(data);
	enum tr_res r = TR_UNKNOWN;
	trace(ctx->file, "%s%s%s", prefix(d, 1), t->story, t->assert && data && strdata? strdata(data): "");
	clock_t t0 = clock();
	if(t->ignored) {
		r = TR_IGNORED;
	} else {
		do {
			if(t->assert) {
				trapsig(ctx->handler);
				if(!setjmp(*(ctx->env))) {
					r = t->assert(data)? TR_PASSED: TR_FAILED;
				} else {
					_Bool b = t->expected == *(ctx->caught);
					trace(ctx->file, "%scaught %sexpected exception %s", prefix(d, 0), b? "": "un", strex(*(ctx->caught)));
					r = b? TR_PASSED: TR_FAILED;
				}
			}
			if(t->body && !stop(mode, r)) r = max(r, runlist(ctx, t->body, d + 1, mode, strdata, data));
			data = t->recycle? t->recycle(data): 0;
		} while(!stop(mode, r) && data);
	}
	trace(ctx->file, "%stest %s in %fs", prefix(d, 0), strres(r), elapsed(t0));
	ctx->cnt[r] += 1;
	ctx->sum += 1;
	return r;
}

static enum tr_res runlist(struct tr_ctx *ctx, struct tr_test *head, tr_depth d, enum tr_mode mode, const char* (*strdata)(void*), void *data) {
	enum tr_res r = TR_UNKNOWN;
	for(struct tr_test *t = head; t && !stop(mode, r); t = t->next) r = max(r, runtest(ctx, t, d, mode, strdata, data));
	return r;
}

struct tr_ctx run(FILE *file, enum tr_ex *caught, jmp_buf *env, void (*handler)(int), struct tr_test *head, void *data) {
	clock_t t0 = clock();
	struct tr_ctx ctx = {
		.handler = handler,
		.caught = caught,
		.file = file,
		.env = env,
	};
	ctx.res = runlist(&ctx, head, 0, TR_RESUME_ON_FAILURE, 0, data);
	trace(file, "summary: %fs, %u node%s, %u unknown, %u ignored, %u passed, %u failed, %s.",
		elapsed(t0),
		ctx.sum,
		ctx.sum > 1? "s": "",
		ctx.cnt[TR_UNKNOWN],
		ctx.cnt[TR_IGNORED],
		ctx.cnt[TR_PASSED],
		ctx.cnt[TR_FAILED],
		ctx.cnt[TR_UNKNOWN] + ctx.cnt[TR_FAILED] == 0? "OK": "FAILED");
	return ctx;
}

int main(void) {
	struct tr_ctx ctx = run(stderr, &g_caught, &g_env, catch, &tr_g_head, 0);
	return ctx.cnt[TR_UNKNOWN] + ctx.cnt[TR_FAILED];
}
