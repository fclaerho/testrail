/* Inspired by https://github.com/Tordek/cheat/blob/master/sampletest.c
 */

#include "testrail.h"
#include <string.h>
#include <stdlib.h>

	TR_TEST(math_still_work) { return 2 + 2 == 4; }

	void* mysetup(void) { return calloc(1, 50); }

	void mycleanup(void *p) { free(p); }

	TR_TEST(strcat_makes_sense, .next = &math_still_work, .setup = mysetup, .cleanup = mycleanup) {
		char *s = datap;
		strcpy(s, "Hello, ");
		strcat(s, "World!");
		return strcmp(s, "Hello, World!") == 0;
	}

	TR_TEST(segfault, .next = &strcat_makes_sense, .expected = TR_SEGV) { int *p = 0; *p = 42; return 0; }

TR_MAIN_HEAD(.story = "check C works", .body = &segfault);
