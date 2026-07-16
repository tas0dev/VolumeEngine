/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_RENDERER_H
#define VOLUME_RENDERER_RENDERER_H

#include "core/types.h"
#include "math/mat4.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/view.h"

renderer_t *renderer_create(platform_t *platform);
void renderer_destroy(renderer_t *renderer);
void renderer_end_frame(const renderer_t *renderer);
void renderer_get_size(const renderer_t *renderer, int *width, int *height);
void renderer_begin_shadow_pass(renderer_t *renderer,
				const mat4_t *light_view_projection);
void renderer_draw_shadow_mesh(renderer_t *renderer,
			       const mesh_t *mesh,
			       const mat4_t *model);
void renderer_end_shadow_pass(renderer_t *renderer);
void renderer_draw_mesh(renderer_t *renderer,
			const mesh_t *mesh,
			const material_t *material,
			const mat4_t *model,
			const render_view_t *view);
void renderer_begin_frame(renderer_t *renderer);

#endif