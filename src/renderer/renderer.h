/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_RENDERER_RENDERER_H
#define VOLUME_RENDERER_RENDERER_H

#include "core/types.h"
#include "math/mat4.h"
#include "mesh.h"
#include "platform/platform.h"

renderer_t *renderer_create(platform_t *platform);
void renderer_destroy(renderer_t *renderer);
void renderer_begin_frame(const renderer_t *renderer);
void renderer_end_frame(const renderer_t *renderer);
void renderer_draw_mesh(const renderer_t *renderer,
			const mesh_t *mesh,
			const mat4_t *model,
			const mat4_t *view,
			const mat4_t *projection);

#endif