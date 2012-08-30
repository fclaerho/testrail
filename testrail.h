/*  _____       _   ___      _ _ 
 * |_   _|__ __| |_| _ \__ _(_) |
 *   | |/ -_|_-<  _|   / _` | | |
 *   |_|\___/__/\__|_|_\__,_|_|_|
 * Yet another C unit test framework.
 * copyright (c) 2012, f.claerhout, licensed under the GPL.
 *
 * A test suite is a list of test nodes (hereafter referred to as "a list").
 * A test node can be itself divided into a list, called its "body".
 * The result of a list is the merging of the results of its nodes.
 * The result of a node is the merging of:
 * - the result of its assert() callback, if no exception is raised,
 * - the catching of an exception specified by its .expected field,
 * - the result of its body.
 * A node inherits its parent mode, except if specified otherwise.
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
	TR_NONE = 0,
	TR_ABRT,
	TR_FPE,
	TR_ILL,
	TR_INT,
	TR_SEGV,
	TR_TERM,
};

struct tr_test {
	_Bool (*assert)(void*);
	void (*cleanup)(void*);
	void* (*setup)(void);
	struct tr_test *body;
	struct tr_test *next;
	enum tr_ex expected;
	const char *story;
	enum tr_mode mode; /* specify a mode for this node and its body */
	_Bool ignored; /* if set, specify to ignore this test and its body */
};

extern struct tr_test tr_g_head;

#define TR_TEST(symbol, ...)\
	_Bool assert_##symbol(void*);\
	struct tr_test symbol = {\
		.assert = assert_##symbol,\
		.story = #symbol,\
		__VA_ARGS__\
	};\
	_Bool assert_##symbol(__attribute__(( unused )) void *datap)

#define TR_HEAD(symbol, ...) struct tr_test symbol = { __VA_ARGS__ };

#define TR_MAIN_HEAD(...) TR_HEAD(tr_g_head, __VA_ARGS__)

#endif /* TESTRAIL */
