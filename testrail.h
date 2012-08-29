/*  _____       _   ___      _ _ 
 * |_   _|__ __| |_| _ \__ _(_) |
 *   | |/ -_|_-<  _|   / _` | | |
 *   |_|\___/__/\__|_|_\__,_|_|_|
 * Yet another C unit test framework.
 * copyright (c) 2012, f.claerhout, licensed under the GPL.
 *
 * A test suite is a list of test nodes.
 * A test node can be itself divided into a list of test nodes (its "body").
 * The result of a list is the merging of the results of its nodes.
 * The result of a node is the merging of the results of its assert() callback and body.
 * To start your suite, define the head node, tr_g_head.
 */

#ifndef TESTRAIL
#define TESTRAIL

enum tr_mode {
	TR_INHERITED = 0, /* inherit parent mode */
	TR_STOP_ON_FAILURE,
	TR_RESUME_ON_FAILURE, /* run all tests (default) */
};

enum tr_ex {
	TR_ABRT,
	TR_FP,
	TR_ILL,
	TR_INT,
	TR_SEGV,
	TR_TERM,
	TR_OTHER,
};

struct tr_test {
	_Bool (*assert)(void*);
	void (*cleanup)(void*);
	void* (*setup)(void);
	struct tr_test *body;
	struct tr_test *next;
	enum tr_mode mode; /* specify a mode for this test node and its body */
	enum tr_ex expect; /* specify an exception which assert() should raise */
	const char *name;
	_Bool ignored; /* if set, specify to ignore this test and its body */
};

extern struct tr_test tr_g_head; /* user-defined head node */

#endif /* TESTRAIL */
