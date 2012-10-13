/* Inspired by https://github.com/Tordek/cheat/blob/master/sampletest.c
 */

#include "testrail.h"
#include <string.h>
#include <stdlib.h>

	TR_TEST(math, .story = "math still work") { return 2 + 2 == 4; }

	void* str(__attribute__(( unused )) void *data) {
		static char *s = 0;
		if(!s) s = calloc(1, 50);
		else {
			free(s);
			s = 0;
		}
		return s;
	}

	TR_TEST(cat, .story = "strcat makes sense", .recycle = str, .next = &math) {
		char *str = data;
		(void)strcpy(str, "Hello, ");
		(void)strcat(str, "World!");
		return strcmp(str, "Hello, World!") == 0;
	}

	TR_TEST(segv, .story = "dereferencing NULL raises SEGV", .expected = TR_SEGV, .next = &cat) { int *ptr = 0; *ptr = 42; return 0; }

TR_MAIN_HEAD(.story = "C works", .body = &segv)
