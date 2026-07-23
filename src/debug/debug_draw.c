/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "debug/debug_draw.h"

static void draw_aabb(renderer_t *renderer,
		      aabb_t bounds,
		      renderer_color_t color,
		      const render_view_t *view);

void debug_draw_colliders(renderer_t *renderer,
			  const collision_world_t *collision_world,
			  const render_view_t *view) {
	const renderer_color_t color = {0.15f, 1.0f, 0.25f, 1.0f};
	collider_t collider;
	vec3_t position;
	aabb_t bounds;
	size_t count;
	size_t index;

	if (renderer == NULL || collision_world == NULL || view == NULL) {
		return;
	}

	count = collision_world_get_count(collision_world);

	for (index = 0; index < count; index++) {
		if (!collision_world_get_collider(collision_world, index, NULL,
						  &collider, &position)) {
			continue;
		}

		if (!collider_get_aabb(&collider, position, &bounds)) {
			continue;
		}

		draw_aabb(renderer, bounds, color, view);
	}
}

static void draw_aabb(renderer_t *renderer,
		      const aabb_t bounds,
		      const renderer_color_t color,
		      const render_view_t *view) {
	const vec3_t corners[8] = {
		{bounds.minimum.x, bounds.minimum.y, bounds.minimum.z},
		{bounds.maximum.x, bounds.minimum.y, bounds.minimum.z},
		{bounds.maximum.x, bounds.maximum.y, bounds.minimum.z},
		{bounds.minimum.x, bounds.maximum.y, bounds.minimum.z},
		{bounds.minimum.x, bounds.minimum.y, bounds.maximum.z},
		{bounds.maximum.x, bounds.minimum.y, bounds.maximum.z},
		{bounds.maximum.x, bounds.maximum.y, bounds.maximum.z},
		{bounds.minimum.x, bounds.maximum.y, bounds.maximum.z},
	};
	static const unsigned int edges[12][2] = {
		{0, 1},
		{1, 2},
		{2, 3},
		{3, 0},
		{4, 5},
		{5, 6},
		{6, 7},
		{7, 4},
		{0, 4},
		{1, 5},
		{2, 6},
		{3, 7},
	};
	size_t index;

	for (index = 0; index < 12; index++) {
		renderer_draw_debug_line(renderer, corners[edges[index][0]],
					 corners[edges[index][1]], color, view);
	}
}