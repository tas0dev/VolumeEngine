/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_PLATFORM_PLATFORM_H
#define VOLUME_PLATFORM_PLATFORM_H

#include <stdbool.h>

typedef struct platform_config {
	const char *title;
	int width;
	int height;
} platform_config_t;

typedef struct platform platform_t;

platform_t *platform_create(const platform_config_t *config);
void platform_destroy(platform_t *platform);
bool platform_poll_events(platform_t *platform);
void *platform_gl_create_context(const platform_t *platform);
void platform_gl_destroy_context(void *context);
bool platform_gl_make_current(const platform_t *platform, void *context);
void platform_gl_swap_buffers(const platform_t *platform);
void platform_get_drawable_size(const platform_t *platform,
				int *width,
				int *height);
double platform_get_time(void);
void platform_sleep(double seconds);

#endif