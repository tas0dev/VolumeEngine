/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_BLOOM_BUFFER_H
#define VOLUME_RENDERER_BLOOM_BUFFER_H

#include <stdbool.h>

typedef struct bloom_buffer bloom_buffer_t;

bloom_buffer_t *bloom_buffer_create(int width, int height);
void bloom_buffer_destroy(bloom_buffer_t *buffer);
bool bloom_buffer_resize(bloom_buffer_t *buffer, int width, int height);
void bloom_buffer_bind(const bloom_buffer_t *buffer, int index);
unsigned int bloom_buffer_get_texture(const bloom_buffer_t *buffer, int index);

#endif