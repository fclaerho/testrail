#include "testrail.h"

	TR_TEST(math_still_work) { return 2 + 2 == 4; }

	TR_TEST(raised_segv_is_caught, .next = &math_still_work, .caught = TR_SEGV) { int *p = 0; *p = 42; return 0; }

TR_G_HEAD(.story = "check things", .body = &raised_segv_is_caught);
