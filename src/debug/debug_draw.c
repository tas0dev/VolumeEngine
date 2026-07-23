/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "debug/debug_draw.h"
#include "collision/triangle.h"
#include "collision/triangle_mesh_collider.h"
#include "math/mat4.h"

static void draw_aabb(renderer_t *renderer,
		      aabb_t bounds,
		      renderer_color_t color);
static void
draw_triangle_mesh(renderer_t *renderer,
		   const triangle_mesh_collider_instance_t *instance,
		   renderer_color_t color);
static renderer_color_t get_collider_color(collision_layer_t layer);

void debug_draw_colliders(renderer_t *renderer,
			  const collision_world_t *collision_world) {
	collider_t collider;
	collision_layer_t layer;
	renderer_color_t color;
	vec3_t position;
	aabb_t bounds;
	size_t count;
	size_t index;

	if (renderer == NULL || collision_world == NULL) { return;
	}

	count = collision_world_get_count(collision_world);

	for (index = 0; index < count; index++) {
		if (!collision_world_get_collider(collision_world, index, NULL,
						  &collider, &position,
						  &layer)) {
			continue;
		}

		color = get_collider_color(layer);

		switch (collider.type) {
		case COLLIDER_TYPE_BOX:
			if (collider_get_aabb(&collider, position, &bounds)) {
				draw_aabb(renderer, bounds, color);
			}
			break;

		case COLLIDER_TYPE_TRIANGLE_MESH:
			draw_triangle_mesh(renderer,
					   &collider.shape.triangle_mesh, color);
			break;

		default: break;
		}
	}
}

static void draw_aabb(renderer_t *renderer,
		      const aabb_t bounds,
		      const renderer_color_t color) {
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
		renderer_add_debug_line(renderer, corners[edges[index][0]],
					corners[edges[index][1]], color);
	}
}

static void
draw_triangle_mesh(renderer_t *renderer,
		   const triangle_mesh_collider_instance_t *instance,
		   const renderer_color_t color) {
	triangle_t triangle;
	vec3_t first;
	vec3_t second;
	vec3_t third;
	size_t triangle_count;
	size_t index;

	if (renderer == NULL || instance == NULL || instance->mesh == NULL) {
		return;
	}

	triangle_count =
		triangle_mesh_collider_get_triangle_count(instance->mesh);

	for (index = 0; index < triangle_count; index++) {
		if (!triangle_mesh_collider_get_triangle(instance->mesh, index,
							 &triangle)) {
			continue;
		}

		first = mat4_transform_point(instance->transform,
					     triangle.vertices[0]);
		second = mat4_transform_point(instance->transform,
					      triangle.vertices[1]);
		third = mat4_transform_point(instance->transform,
					     triangle.vertices[2]);

		renderer_add_debug_line(renderer, first, second, color);
		renderer_add_debug_line(renderer, second, third, color);
		renderer_add_debug_line(renderer, third, first, color);
	}
}

static renderer_color_t get_collider_color(const collision_layer_t layer) {
	if ((layer & COLLISION_LAYER_PLAYER) != 0) {
		return (renderer_color_t){0.2f, 0.8f, 1.0f, 1.0f};
	}

	if ((layer & COLLISION_LAYER_TRIGGER) != 0) {
		return (renderer_color_t){0.85f, 0.3f, 1.0f, 1.0f};
	}

	if ((layer & COLLISION_LAYER_DYNAMIC) != 0) {
		return (renderer_color_t){1.0f, 0.8f, 0.15f, 1.0f};
	}

	return (renderer_color_t){0.2f, 1.0f, 0.3f, 1.0f};
}

void debug_draw_character_contacts(renderer_t *renderer,
				   const vec3_t origin,
				   const character_debug_state_t *state) {
	const renderer_color_t contact_color = {1.0f, 0.2f, 0.15f, 1.0f};
	const renderer_color_t normal_color = {1.0f, 0.85f, 0.1f, 1.0f};
	const renderer_color_t correction_color = {0.2f, 0.8f, 1.0f, 1.0f};
	const float contact_radius = 0.06f;
	const float normal_length = 0.45f;
	character_debug_contact_t contact;
	vec3_t contact_end;
	vec3_t correction_end;
	size_t index;

	if (renderer == NULL || state == NULL || !state->valid) { return; }

	for (index = 0; index < state->contact_count; index++) {
		contact = state->contacts[index];

		renderer_add_debug_line(
			renderer,
			vec3_add(contact.position,
				 vec3_create(-contact_radius, 0.0f, 0.0f)),
			vec3_add(contact.position,
				 vec3_create(contact_radius, 0.0f, 0.0f)),
			contact_color);
		renderer_add_debug_line(
			renderer,
			vec3_add(contact.position,
				 vec3_create(0.0f, -contact_radius, 0.0f)),
			vec3_add(contact.position,
				 vec3_create(0.0f, contact_radius, 0.0f)),
			contact_color);
		renderer_add_debug_line(
			renderer,
			vec3_add(contact.position,
				 vec3_create(0.0f, 0.0f, -contact_radius)),
			vec3_add(contact.position,
				 vec3_create(0.0f, 0.0f, contact_radius)),
			contact_color);

		contact_end =
			vec3_add(contact.position,
				 vec3_scale(contact.normal, normal_length));

		renderer_add_debug_line(renderer, contact.position, contact_end,
					normal_color);
	}

	if (vec3_length(state->correction) <= 0.000001f) { return; }

	correction_end = vec3_add(origin, state->correction);

	renderer_add_debug_line(renderer, origin, correction_end,
				correction_color);
}
