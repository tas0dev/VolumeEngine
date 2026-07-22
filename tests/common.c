/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include <stdio.h>

bool test_check(const bool condition,
		const char *expression,
		const char *file,
		const int line) {
	if (condition) { return true; }

	fprintf(stderr, "%s:%d: check failed: %s\n", file, line, expression);

	return false;
}

int test_run_all(const test_case_t *tests, const size_t count) {
	size_t index;
	size_t passed;

	if (tests == NULL) { return 1; }

	passed = 0;

	for (index = 0; index < count; index++) {
		if (!tests[index].run()) {
			fprintf(stderr, "FAIL: %s\n", tests[index].name);
			continue;
		}

		printf("PASS: %s\n", tests[index].name);
		passed++;
	}

	printf("%zu/%zu tests passed\n", passed, count);

	return passed == count ? 0 : 1;
}
