/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "core/path.h"
#include <SDL3/SDL.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *path_from_executable(const char *path) {
	const char *base_path;
	char *result;
	size_t base_length;
	size_t path_length;
	size_t separator_length;
	size_t total_length;

	if (path == NULL || path[0] == '\0') { return NULL; }

	if (path[0] == '/' || path[0] == '\\' ||
	    (path[0] != '\0' && path[1] == ':')) {
		path_length = strlen(path);

		result = malloc(path_length + 1);
		if (result == NULL) { return NULL; }

		memcpy(result, path, path_length + 1);

		return result;
	}

	base_path = SDL_GetBasePath();
	if (base_path == NULL || base_path[0] == '\0') { return NULL; }

	base_length = strlen(base_path);
	path_length = strlen(path);

	separator_length = base_path[base_length - 1] == '/' ||
					   base_path[base_length - 1] == '\\'
				   ? 0
				   : 1;

	if (base_length > (size_t)-1 - path_length ||
	    base_length + path_length > (size_t)-1 - separator_length - 1) {
		return NULL;
	}

	total_length = base_length + separator_length + path_length;

	result = malloc(total_length + 1);
	if (result == NULL) { return NULL; }

	memcpy(result, base_path, base_length);

	if (separator_length != 0) { result[base_length] = '/'; }

	memcpy(result + base_length + separator_length, path, path_length);
	result[total_length] = '\0';

	return result;
}