/* Inspired by https://github.com/Tordek/cheat/blob/master/sampletest.c
 */

#include "testrail.h"
#include <string.h>
#include <stdlib.h>

	TR_TEST(math_still_work, .next = 0) { return 2 + 2 == 4; }

	void* alloc_str(void) { return calloc(1, 50); }

	void* free_str(void *p) {
		free(p);
		return 0;
	}

	TR_TEST(strcat_makes_sense, .next = &math_still_work, .setup = alloc_str, .recycle = free_str) {
		char *str = data;
		strcpy(str, "Hello, ");
		strcat(str, "World!");
		return strcmp(str, "Hello, World!") == 0;
	}

	TR_TEST(segfault, .next = &strcat_makes_sense, .expected = TR_SEGV) { int *ptr = 0; *ptr = 42; return 0; }

TR_MAIN_HEAD(.story = "check C works", .body = &segfault)
