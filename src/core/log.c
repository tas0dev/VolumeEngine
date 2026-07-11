/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

// ReSharper disable CppParameterMayBeConst
#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static void
log_write(FILE *stream, const char *level, const char *fmt, va_list arguments) {
	fprintf(stream, "[%s] ", level);
	vfprintf(stream, fmt, arguments);
	fputc('\n', stream);
}

void log_info(const char *fmt, ...) {
	va_list arguments;

	va_start(arguments, fmt);
	log_write(stdout, "INFO", fmt, arguments);
	va_end(arguments);
}

void log_error(const char *fmt, ...) {
	va_list arguments;

	va_start(arguments, fmt);
	log_write(stderr, "ERROR", fmt, arguments);
	va_end(arguments);
}