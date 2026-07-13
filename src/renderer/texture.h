/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_RENDERER_TEXTURE_H
#define VOLUME_RENDERER_TEXTURE_H

#include <stddef.h>

typedef struct texture texture_t;

texture_t *texture_load(const char *path, char *error, size_t error_size);
void texture_destroy(texture_t *texture);
void texture_bind(const texture_t *texture, unsigned int unit);
void texture_unbind(unsigned int unit);

#endif
