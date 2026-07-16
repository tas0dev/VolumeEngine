/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_SHADOW_MAP_H
#define VOLUME_RENDERER_SHADOW_MAP_H

typedef struct shadow_map shadow_map_t;

shadow_map_t *shadow_map_create(int width, int height);
void shadow_map_destroy(shadow_map_t *shadow_map);
void shadow_map_begin(const shadow_map_t *shadow_map);
void shadow_map_end(void);
unsigned int shadow_map_get_texture(const shadow_map_t *shadow_map);
int shadow_map_get_width(const shadow_map_t *shadow_map);
int shadow_map_get_height(const shadow_map_t *shadow_map);

#endif