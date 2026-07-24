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

static void
draw_triangle_mesh(renderer_t *renderer,
		   const triangle_mesh_collider_instance_t *instance,
		   vec3_t position,
		   renderer_color_t color);
static renderer_color_t get_collider_color(collision_layer_t layer);
static void draw_box(renderer_t *renderer,
		     const box_collider_t *box,
		     vec3_t position,
		     renderer_color_t color);

void debug_draw_colliders(renderer_t *renderer,
			  const collision_world_t *collision_world) {
	collider_t collider;
	collision_layer_t layer;
	renderer_color_t color;
	vec3_t position;
	size_t count;
	size_t index;

	if (renderer == NULL || collision_world == NULL) { return; }

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
			draw_box(renderer,
				 &collider.shape.box, position,
				 color);
			break;

		case COLLIDER_TYPE_TRIANGLE_MESH:
			draw_triangle_mesh(renderer,
					   &collider.shape.triangle_mesh,
					   position, color);
			break;

		case COLLIDER_TYPE_NONE:
		default: break;
		}
	}
}

static void
draw_triangle_mesh(renderer_t *renderer,
		   const triangle_mesh_collider_instance_t *instance,
		   const vec3_t position,
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

		first = vec3_add(mat4_transform_point(instance->transform,
						      triangle.vertices[0]),
				 position);
		second = vec3_add(mat4_transform_point(instance->transform,
						       triangle.vertices[1]),
				  position);
		third = vec3_add(mat4_transform_point(instance->transform,
						      triangle.vertices[2]),
				 position);

		renderer_add_debug_line(renderer, first, second, color);
		renderer_add_debug_line(renderer, second, third, color);
		renderer_add_debug_line(renderer, third, first, color);
	}
}

static void draw_box(renderer_t *renderer,
		     const box_collider_t *box,
		     const vec3_t position,
		     const renderer_color_t color) {
	static const unsigned int edges[12][2] = {
		{0, 1},
		{1, 3},
		{3, 2},
		{2, 0},
		{4, 5},
		{5, 7},
		{7, 6},
		{6, 4},
		{0, 4},
		{1, 5},
		{2, 6},
		{3, 7},
	};
	vec3_t axes[3];
	vec3_t corners[8];
	vec3_t center;
	vec3_t half_extents;
	vec3_t offset_x;
	vec3_t offset_y;
	vec3_t offset_z;
	size_t index;

	if (renderer == NULL || box == NULL) { return; }

	if (!box_collider_get_world_box(*box, position, &center, axes,
					&half_extents)) {
		return;
	}

	offset_x = vec3_scale(axes[0], half_extents.x);
	offset_y = vec3_scale(axes[1], half_extents.y);
	offset_z = vec3_scale(axes[2], half_extents.z);

	corners[0] = vec3_subtract(
		vec3_subtract(vec3_subtract(center, offset_x), offset_y),
		offset_z);
	corners[1] = vec3_add(
		vec3_subtract(vec3_subtract(center, offset_y), offset_z),
		offset_x);
	corners[2] = vec3_add(
		vec3_subtract(vec3_subtract(center, offset_x), offset_z),
		offset_y);
	corners[3] = vec3_add(
		vec3_add(vec3_subtract(center, offset_z), offset_x), offset_y);

	corners[4] = vec3_add(
		vec3_subtract(vec3_subtract(center, offset_x), offset_y),
		offset_z);
	corners[5] = vec3_add(
		vec3_add(vec3_subtract(center, offset_y), offset_x), offset_z);
	corners[6] = vec3_add(
		vec3_add(vec3_subtract(center, offset_x), offset_y), offset_z);
	corners[7] = vec3_add(vec3_add(vec3_add(center, offset_x), offset_y),
			      offset_z);

	for (index = 0; index < 12; index++) {
		renderer_add_debug_line(renderer, corners[edges[index][0]],
					corners[edges[index][1]], color);
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
	const renderer_color_t contact_color = {
		1.0f, 0.2f, 0.15f, 1.0f
	};
	const renderer_color_t normal_color = {
		1.0f, 0.85f, 0.1f, 1.0f};
	const renderer_color_t correction_color = {0.2f, 0.8f, 1.0f, 1.0f};
	const float contact_radius = 0.06f;
	const float normal_length = 0.45f;
	character_debug_contact_t contact;
	vec3_t contact_end;
	vec3_t correction_end;
	size_t index;

	if (renderer == NULL || state == NULL || !state->valid) {
		return;
	}

	for (index = 0; index < state->contact_count; index++) {
		contact = state->contacts[index];

		renderer_add_debug_line(
			renderer,
			vec3_add(
				contact.position,
				vec3_create(-contact_radius, 0.0f, 0.0f)),
			vec3_add(
				contact.position,
				vec3_create(contact_radius, 0.0f, 0.0f)),
			contact_color);

		renderer_add_debug_line(
			renderer,
			vec3_add(contact.position,
				 vec3_create(0.0f, -contact_radius, 0.0f)),
			vec3_add(
				contact.position,
				vec3_create(0.0f, contact_radius, 0.0f)),
			contact_color);

		renderer_add_debug_line(
			renderer,
			vec3_add(contact.position,
				 vec3_create(0.0f, 0.0f, -contact_radius)),
			vec3_add(
				contact.position,
				vec3_create(0.0f, 0.0f, contact_radius)),
			contact_color);

		contact_end =
			vec3_add(contact.position,
				 vec3_scale(contact.normal, normal_length));

		renderer_add_debug_line(renderer, contact.position,
			contact_end,
					normal_color);
	}

	if (vec3_length(state->correction) <= 0.000001f) { return; }

	correction_end = vec3_add(origin, state->correction);

	renderer_add_debug_line(renderer,
		origin,
		correction_end,
		correction_color);
}