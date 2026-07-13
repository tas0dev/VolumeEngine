/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_TESTS_COMMON_H
#define VOLUME_TESTS_COMMON_H

#include <stdbool.h>
#include <stddef.h>

typedef struct test_case {
	const char *name;
	bool (*run)(void);
} test_case_t;

#define CHECK(condition)                                                      \
	do {                                                                  \
		if (!test_check(condition, #condition, __FILE__, __LINE__)) { \
			return false;                                         \
		}                                                             \
	} while (0)

bool test_check(bool condition,
		const char *expression,
		const char *file,
		int line);
int test_run_all(const test_case_t *tests, size_t count);

#endif
