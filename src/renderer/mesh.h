/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_RENDERER_MESH_H
#define VOLUME_RENDERER_MESH_H

#include <stddef.h>

typedef struct mesh_vertex {
	float position[3];
	float color[3];
} mesh_vertex_t;

typedef struct mesh mesh_t;

mesh_t *mesh_create(const mesh_vertex_t *vertices, size_t vertex_count);
void mesh_destroy(mesh_t *mesh);
void mesh_draw(const mesh_t *mesh);

#endif