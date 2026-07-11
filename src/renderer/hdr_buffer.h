/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_RENDERER_HDR_BUFFER_H
#define VOLUME_RENDERER_HDR_BUFFER_H
#include <stdbool.h>

typedef struct hdr_buffer hdr_buffer_t;

hdr_buffer_t *hdr_buffer_create(int width, int height);
void hdr_buffer_destroy(hdr_buffer_t *buffer);
bool hdr_buffer_resize(hdr_buffer_t *buffer, int width, int height);
void hdr_buffer_bind(const hdr_buffer_t *buffer);
void hdr_buffer_unbind(void);
unsigned int hdr_buffer_get_texture(const hdr_buffer_t *buffer);

#endif