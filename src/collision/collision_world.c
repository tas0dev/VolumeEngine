/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collision_world.h"
#include "collision/triangle.h"
#include "collision/triangle_mesh_collider.h"
#include "math/mat4.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct collision_entry {
	entity_id_t entity_id;
	collider_t collider;
	vec3_t position;
	collision_layer_t layer;
	collision_layer_t mask;
} collision_entry_t;

struct collision_world {
	collision_entry_t *entries;
	size_t count;
	size_t capacity;
};

static bool collision_world_reserve(collision_world_t *world, size_t capacity);
static unsigned int collision_side_from_normal(vec3_t normal);
static void collision_result_reset(collision_result_t *result);
static bool resolve_box_entry(const collision_entry_t *entry,
			      aabb_t local_bounds,
			      vec3_t *position,
			      collision_result_t *result);
static bool resolve_triangle_mesh_entry(const collision_entry_t *entry,
					aabb_t local_bounds,
					vec3_t *position,
					collision_result_t *result);
static void collision_result_record(collision_result_t *result,
				    entity_id_t entity_id,
				    vec3_t normal,
				    float depth);
static aabb_t
create_swept_bounds(aabb_t local_bounds, vec3_t start, vec3_t end);
static bool trace_aabb_against_triangle(aabb_t local_bounds,
					vec3_t start,
					vec3_t end,
					triangle_t triangle,
					collision_trace_t *trace);
static bool trace_triangle_mesh_entry(const collision_entry_t *entry,
				      aabb_t local_bounds,
				      vec3_t start,
				      vec3_t end,
				      collision_trace_t *trace);
static bool update_triangle_sweep_axis(vec3_t axis,
				       const vec3_t *vertices,
				       vec3_t half_extents,
				       vec3_t start_center,
				       vec3_t movement,
				       float *entry_time,
				       float *exit_time,
				       vec3_t *normal);
static bool get_aabb_obb_collision(aabb_t aabb,
				   const box_collider_t *box,
				   vec3_t box_position,
				   aabb_collision_t *collision);
static bool test_sat_axis(vec3_t axis,
			  vec3_t center_difference,
			  vec3_t aabb_half_extents,
			  const vec3_t box_axes[3],
			  vec3_t box_half_extents,
			  float *minimum_depth,
			  vec3_t *minimum_axis);
static bool trace_aabb_against_box(aabb_t local_bounds,
				   vec3_t start,
				   vec3_t end,
				   const box_collider_t *box,
				   vec3_t box_position,
				   collision_trace_t *trace);
static bool update_box_sweep_axis(vec3_t axis,
				  vec3_t start_center,
				  vec3_t movement,
				  vec3_t moving_half_extents,
				  vec3_t box_center,
				  const vec3_t box_axes[3],
				  vec3_t box_half_extents,
				  float *entry_time,
				  float *exit_time,
				  vec3_t *normal);

collision_world_t *collision_world_create(void) {
	return calloc(1, sizeof(collision_world_t));
}

void collision_world_destroy(collision_world_t *world) {
	if (world == NULL) { return; }

	free(world->entries);
	free(world);
}

bool collision_world_add_collider(collision_world_t *world,
				  const entity_id_t entity_id,
				  const collider_t collider,
				  const vec3_t position) {
	return collision_world_add_collider_filtered(
		world, entity_id, collider, position, COLLISION_LAYER_ALL,
		COLLISION_LAYER_ALL);
}

bool collision_world_add_collider_filtered(collision_world_t *world,
					   const entity_id_t entity_id,
					   const collider_t collider,
					   const vec3_t position,
					   const collision_layer_t layer,
					   const collision_layer_t mask) {
	size_t capacity;
	size_t index;

	if (world == NULL || entity_id == 0 ||
	    collider.type == COLLIDER_TYPE_NONE) {
		return false;
	}

	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id == entity_id) {
			return false;
		}
	}

	if (world->count == world->capacity) {
		capacity = world->capacity == 0 ? 16 : world->capacity * 2;

		if (capacity < world->capacity ||
		    !collision_world_reserve(world, capacity)) {
			return false;
		}
	}

	world->entries[world->count].entity_id = entity_id;
	world->entries[world->count].collider = collider;
	world->entries[world->count].position = position;
	world->entries[world->count].layer = layer;
	world->entries[world->count].mask = mask;
	world->count++;

	return true;
}

bool collision_world_update_collider(collision_world_t *world,
				     const entity_id_t entity_id,
				     const collider_t collider,
				     const vec3_t position) {
	return collision_world_update_collider_filtered(
		world, entity_id, collider, position, COLLISION_LAYER_ALL,
		COLLISION_LAYER_ALL);
}

bool collision_world_update_collider_filtered(collision_world_t *world,
					      const entity_id_t entity_id,
					      const collider_t collider,
					      const vec3_t position,
					      const collision_layer_t layer,
					      const collision_layer_t mask) {
	size_t index;

	if (world == NULL || entity_id == 0) { return false; }

	if (collider.type == COLLIDER_TYPE_NONE) {
		return collision_world_remove(world, entity_id);
	}

	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id != entity_id) { continue; }

		world->entries[index].collider = collider;
		world->entries[index].position = position;
		world->entries[index].layer = layer;
		world->entries[index].mask = mask;
		return true;
	}

	return collision_world_add_collider_filtered(world, entity_id, collider,
						     position, layer, mask);
}

size_t collision_world_get_count(const collision_world_t *world) {
	if (world == NULL) { return 0; }

	return world->count;
}

bool collision_world_resolve_aabb(const collision_world_t *world,
				  const aabb_t local_bounds,
				  vec3_t *position,
				  collision_result_t *result) {
	return collision_world_resolve_aabb_ignoring(world, local_bounds,
						     position, 0, result);
}

bool collision_world_resolve_aabb_ignoring(const collision_world_t *world,
					   const aabb_t local_bounds,
					   vec3_t *position,
					   const entity_id_t ignored_entity_id,
					   collision_result_t *result) {
	collision_filter_t filter;

	filter.layer = COLLISION_LAYER_ALL;
	filter.mask = COLLISION_LAYER_ALL;
	filter.ignored_entity_id = ignored_entity_id;
	return collision_world_resolve_aabb_filtered(world, local_bounds,
						     position, filter, result);
}

bool collision_world_resolve_aabb_filtered(const collision_world_t *world,
					   const aabb_t local_bounds,
					   vec3_t *position,
					   const collision_filter_t filter,
					   collision_result_t *result) {
	const unsigned int maximum_iterations = 8;
	bool entry_collided;
	bool iteration_collided;
	bool collided;
	unsigned int iteration;
	size_t index;

	if (world == NULL || position == NULL) { return false; }

	collision_result_reset(result);
	collided = false;

	for (iteration = 0; iteration < maximum_iterations; iteration++) {
		iteration_collided = false;

		for (index = 0; index < world->count; index++) {
			if (world->entries[index].entity_id ==
				    filter.ignored_entity_id ||
			    (filter.mask & world->entries[index].layer) == 0 ||
			    (world->entries[index].mask & filter.layer) == 0) {
				continue;
			}

			entry_collided = false;

			switch (world->entries[index].collider.type) {
			case COLLIDER_TYPE_BOX:
				entry_collided = resolve_box_entry(
					&world->entries[index], local_bounds,
					position, result);
				break;

			case COLLIDER_TYPE_TRIANGLE_MESH:
				entry_collided = resolve_triangle_mesh_entry(
					&world->entries[index], local_bounds,
					position, result);
				break;

			case COLLIDER_TYPE_NONE:
			default: break;
			}

			if (entry_collided) {
				collided = true;
				iteration_collided = true;
			}
		}

		if (!iteration_collided) { break; }
	}

	return collided;
}

static bool collision_world_reserve(collision_world_t *world,
				    const size_t capacity) {
	collision_entry_t *entries;

	if (capacity <= world->capacity) { return true; }

	if (capacity > SIZE_MAX / sizeof(*world->entries)) { return false; }

	entries = realloc(world->entries, capacity * sizeof(*world->entries));
	if (entries == NULL) { return false; }

	world->entries = entries;
	world->capacity = capacity;

	return true;
}

static unsigned int collision_side_from_normal(const vec3_t normal) {
	if (normal.x < 0.0f) { return COLLISION_SIDE_NEGATIVE_X; }
	if (normal.x > 0.0f) { return COLLISION_SIDE_POSITIVE_X; }
	if (normal.y < 0.0f) { return COLLISION_SIDE_NEGATIVE_Y; }
	if (normal.y > 0.0f) { return COLLISION_SIDE_POSITIVE_Y; }
	if (normal.z < 0.0f) { return COLLISION_SIDE_NEGATIVE_Z; }
	if (normal.z > 0.0f) { return COLLISION_SIDE_POSITIVE_Z; }

	return COLLISION_SIDE_NONE;
}

static void collision_result_reset(collision_result_t *result) {
	if (result == NULL) { return; }

	memset(result, 0, sizeof(*result));
}

bool collision_world_remove(collision_world_t *world,
			    const entity_id_t entity_id) {
	size_t index;

	if (world == NULL || entity_id == 0) { return false; }

	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id != entity_id) { continue; }

		if (index + 1 < world->count) {
			memmove(&world->entries[index],
				&world->entries[index + 1],
				(world->count - index - 1) *
					sizeof(*world->entries));
		}

		world->count--;
		world->entries[world->count] = (collision_entry_t){0};

		return true;
	}

	return false;
}

static bool resolve_box_entry(const collision_entry_t *entry,
			      const aabb_t local_bounds,
			      vec3_t *position,
			      collision_result_t *result) {
	aabb_collision_t collision;
	aabb_t moving_bounds;
	aabb_t broad_phase_bounds;
	vec3_t correction;

	moving_bounds = aabb_translate(local_bounds, *position);

	if (!collider_get_aabb(&entry->collider, entry->position,
			       &broad_phase_bounds) ||
	    !aabb_intersects(moving_bounds, broad_phase_bounds)) {
		return false;
	}

	if (!get_aabb_obb_collision(moving_bounds, &entry->collider.shape.box,
				    entry->position, &collision)) {
		return false;
		    }

	correction = vec3_scale(collision.normal, collision.depth);
	*position = vec3_add(*position, correction);

	collision_result_record(result, entry->entity_id, collision.normal,
				collision.depth);

	return true;
}

static bool resolve_triangle_mesh_entry(const collision_entry_t *entry,
					const aabb_t local_bounds,
					vec3_t *position,
					collision_result_t *result) {
	const triangle_mesh_collider_instance_t *instance;
	aabb_collision_t collision;
	aabb_t moving_bounds;
	aabb_t mesh_bounds;
	aabb_t triangle_bounds;
	triangle_t local_triangle;
	triangle_t triangle;
	vec3_t first;
	vec3_t second;
	vec3_t third;
	vec3_t correction;
	size_t triangle_count;
	size_t index;
	bool collided;

	instance = &entry->collider.shape.triangle_mesh;

	moving_bounds = aabb_translate(local_bounds, *position);
	mesh_bounds = aabb_translate(instance->bounds, entry->position);

	if (!aabb_intersects(moving_bounds, mesh_bounds)) { return false; }

	triangle_count =
		triangle_mesh_collider_get_triangle_count(instance->mesh);
	collided = false;

	for (index = 0; index < triangle_count; index++) {
		if (!triangle_mesh_collider_get_triangle(instance->mesh, index,
							 &local_triangle)) {
			continue;
		}

		first = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[0]),
			entry->position);
		second = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[1]),
			entry->position);
		third = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[2]),
			entry->position);

		triangle = triangle_create(first, second, third);
		triangle_bounds = triangle_get_bounds(triangle);
		moving_bounds = aabb_translate(local_bounds, *position);

		if (!aabb_intersects(moving_bounds, triangle_bounds) ||
		    !aabb_get_triangle_collision(moving_bounds, triangle,
						 &collision)) {
			continue;
		}

		correction = vec3_scale(collision.normal, collision.depth);
		*position = vec3_add(*position, correction);

		collision_result_record(result, entry->entity_id,
					collision.normal, collision.depth);

		collided = true;
	}

	return collided;
}

static void collision_result_record(collision_result_t *result,
				    const entity_id_t entity_id,
				    const vec3_t normal,
				    const float depth) {
	collision_contact_t *contact;

	if (result == NULL) { return; }

	result->correction =
		vec3_add(result->correction, vec3_scale(normal, depth));
	result->sides |= collision_side_from_normal(normal);

	if (result->contact_count >= COLLISION_RESULT_MAX_CONTACTS) { return; }

	contact = &result->contacts[result->contact_count];
	contact->normal = normal;
	contact->depth = depth;
	contact->entity_id = entity_id;
	result->contact_count++;
}

bool collision_world_trace_aabb(const collision_world_t *world,
				const aabb_t local_bounds,
				const vec3_t start,
				const vec3_t end,
				collision_trace_t *trace) {
	return collision_world_trace_aabb_ignoring(world, local_bounds, start,
						   end, 0, trace);
}

bool collision_world_trace_aabb_ignoring(const collision_world_t *world,
					 const aabb_t local_bounds,
					 const vec3_t start,
					 const vec3_t end,
					 const entity_id_t ignored_entity_id,
					 collision_trace_t *trace) {
	collision_filter_t filter;

	filter.layer = COLLISION_LAYER_ALL;
	filter.mask = COLLISION_LAYER_ALL;
	filter.ignored_entity_id = ignored_entity_id;
	return collision_world_trace_aabb_filtered(world, local_bounds, start,
						   end, filter, trace);
}

bool collision_world_trace_aabb_filtered(const collision_world_t *world,
					 const aabb_t local_bounds,
					 const vec3_t start,
					 const vec3_t end,
					 const collision_filter_t filter,
					 collision_trace_t *trace) {
	return collision_world_trace_aabb_filtered_ignoring(
		world, local_bounds, start, end, filter, 0, trace);
}

bool collision_world_trace_aabb_filtered_ignoring(
	const collision_world_t *world,
	const aabb_t local_bounds,
	const vec3_t start,
	const vec3_t end,
	const collision_filter_t filter,
	const entity_id_t additional_ignored_entity_id,
	collision_trace_t *trace) {
	collision_trace_t candidate;
	aabb_t swept_bounds;
	aabb_t static_bounds;
	size_t index;

	if (trace == NULL) { return false; }

	trace->hit = false;
	trace->started_inside = false;
	trace->fraction = 1.0f;
	trace->position = end;
	trace->normal = vec3_create(0.0f, 0.0f, 0.0f);
	trace->entity_id = 0;

	if (world == NULL) { return false; }

	swept_bounds = create_swept_bounds(local_bounds, start, end);

	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id ==
			    filter.ignored_entity_id ||
		    world->entries[index].entity_id ==
			    additional_ignored_entity_id ||
		    (filter.mask & world->entries[index].layer) == 0 ||
		    (world->entries[index].mask & filter.layer) == 0) {
			continue;
		}

		if (!collider_get_aabb(&world->entries[index].collider,
				       world->entries[index].position,
				       &static_bounds) ||
		    !aabb_intersects(swept_bounds, static_bounds)) {
			continue;
		}

		candidate.hit = false;
		candidate.started_inside = false;
		candidate.fraction = 1.0f;
		candidate.position = end;
		candidate.normal = vec3_create(0.0f, 0.0f, 0.0f);
		candidate.entity_id = world->entries[index].entity_id;

		switch (world->entries[index].collider.type) {
		case COLLIDER_TYPE_BOX:
			if (!trace_aabb_against_box(
				    local_bounds, start, end,
				    &world->entries[index].collider.shape.box,
				    world->entries[index].position,
				    &candidate)) {
				continue;
			}
			break;

		case COLLIDER_TYPE_TRIANGLE_MESH:
			if (!trace_triangle_mesh_entry(&world->entries[index],
						       local_bounds, start, end,
						       &candidate)) {
				continue;
			}
			break;

		case COLLIDER_TYPE_NONE:
		default:
			continue;
		}

		if (candidate.started_inside) {
			*trace = candidate;
			return true;
		}

		if (!trace->hit ||
		    candidate.fraction < trace->fraction) {
			*trace = candidate;
		}
	}

	return trace->hit;
}

size_t collision_world_query_aabb(const collision_world_t *world,
				  const aabb_t bounds,
				  const collision_filter_t filter,
				  entity_id_t *entity_ids,
				  const size_t capacity) {
	aabb_t entry_bounds;
	size_t count;
	size_t index;

	if (world == NULL) { return 0; }

	count = 0;
	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id ==
			    filter.ignored_entity_id ||
		    (filter.mask & world->entries[index].layer) == 0 ||
		    !collider_get_aabb(&world->entries[index].collider,
				       world->entries[index].position,
				       &entry_bounds) ||
		    !aabb_intersects(bounds, entry_bounds)) {
			continue;
		}

		if (entity_ids != NULL && count < capacity) {
			entity_ids[count] = world->entries[index].entity_id;
		}
		count++;
	}

	return count;
}

bool collision_world_trace_ray_filtered(const collision_world_t *world,
					const vec3_t start,
					const vec3_t end,
					const collision_filter_t filter,
					collision_trace_t *trace) {
	aabb_t point_bounds;

	point_bounds = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(0.0f, 0.0f, 0.0f));
	return collision_world_trace_aabb_filtered(world, point_bounds, start,
						   end, filter, trace);
}

static aabb_t create_swept_bounds(const aabb_t local_bounds,
				  const vec3_t start,
				  const vec3_t end) {
	aabb_t start_bounds;
	aabb_t end_bounds;
	vec3_t minimum;
	vec3_t maximum;
	vec3_t center;
	vec3_t half_extents;

	start_bounds = aabb_translate(local_bounds, start);
	end_bounds = aabb_translate(local_bounds, end);

	minimum = vec3_create(
		fminf(start_bounds.minimum.x, end_bounds.minimum.x),
		fminf(start_bounds.minimum.y, end_bounds.minimum.y),
		fminf(start_bounds.minimum.z, end_bounds.minimum.z));
	maximum = vec3_create(
		fmaxf(start_bounds.maximum.x, end_bounds.maximum.x),
		fmaxf(start_bounds.maximum.y, end_bounds.maximum.y),
		fmaxf(start_bounds.maximum.z, end_bounds.maximum.z));

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);

	return aabb_create(center, half_extents);
}

static bool trace_aabb_against_triangle(const aabb_t local_bounds,
					const vec3_t start,
					const vec3_t end,
					const triangle_t triangle,
					collision_trace_t *trace) {
	aabb_collision_t overlap;
	aabb_t start_bounds;
	vec3_t box_axes[3];
	vec3_t vertices[3];
	vec3_t edges[3];
	vec3_t start_center;
	vec3_t half_extents;
	vec3_t movement;
	vec3_t normal;
	float entry_time;
	float exit_time;
	size_t edge_index;
	size_t axis_index;

	start_bounds = aabb_translate(local_bounds, start);

	if (aabb_get_triangle_collision(start_bounds, triangle, &overlap)) {
		trace->hit = true;
		trace->started_inside = true;
		trace->fraction = 0.0f;
		trace->position = start;
		trace->normal = overlap.normal;
		return true;
	}

	start_center = aabb_get_center(start_bounds);
	half_extents = aabb_get_half_extents(start_bounds);
	movement = vec3_subtract(end, start);

	vertices[0] = triangle.vertices[0];
	vertices[1] = triangle.vertices[1];
	vertices[2] = triangle.vertices[2];

	edges[0] = vec3_subtract(vertices[1], vertices[0]);
	edges[1] = vec3_subtract(vertices[2], vertices[1]);
	edges[2] = vec3_subtract(vertices[0], vertices[2]);

	box_axes[0] = vec3_create(1.0f, 0.0f, 0.0f);
	box_axes[1] = vec3_create(0.0f, 1.0f, 0.0f);
	box_axes[2] = vec3_create(0.0f, 0.0f, 1.0f);

	entry_time = -INFINITY;
	exit_time = INFINITY;
	normal = vec3_create(0.0f, 0.0f, 0.0f);

	for (axis_index = 0; axis_index < 3; axis_index++) {
		if (!update_triangle_sweep_axis(box_axes[axis_index], vertices,
						half_extents, start_center,
						movement, &entry_time,
						&exit_time, &normal)) {
			return false;
		}
	}

	if (!update_triangle_sweep_axis(triangle.normal, vertices, half_extents,
					start_center, movement, &entry_time,
					&exit_time, &normal)) {
		return false;
	}

	for (edge_index = 0; edge_index < 3; edge_index++) {
		for (axis_index = 0; axis_index < 3; axis_index++) {
			if (!update_triangle_sweep_axis(
				    vec3_cross(edges[edge_index],
					       box_axes[axis_index]),
				    vertices, half_extents, start_center,
				    movement, &entry_time, &exit_time,
				    &normal)) {
				return false;
			}
		}
	}

	if (entry_time < 0.0f || entry_time > 1.0f || entry_time > exit_time) {
		return false;
	}

	trace->hit = true;
	trace->started_inside = false;
	trace->fraction = entry_time;
	trace->position = vec3_add(start, vec3_scale(movement, entry_time));
	trace->normal = normal;

	return true;
}

static bool trace_triangle_mesh_entry(const collision_entry_t *entry,
				      const aabb_t local_bounds,
				      const vec3_t start,
				      const vec3_t end,
				      collision_trace_t *trace) {
	const triangle_mesh_collider_instance_t *instance;
	collision_trace_t candidate;
	aabb_t swept_bounds;
	aabb_t mesh_bounds;
	aabb_t triangle_bounds;
	triangle_t local_triangle;
	triangle_t triangle;
	vec3_t first;
	vec3_t second;
	vec3_t third;
	size_t triangle_count;
	size_t index;
	bool hit;

	instance = &entry->collider.shape.triangle_mesh;

	swept_bounds = create_swept_bounds(local_bounds, start, end);
	mesh_bounds = aabb_translate(instance->bounds, entry->position);

	if (!aabb_intersects(swept_bounds, mesh_bounds)) { return false; }

	hit = false;
	triangle_count =
		triangle_mesh_collider_get_triangle_count(instance->mesh);

	for (index = 0; index < triangle_count; index++) {
		if (!triangle_mesh_collider_get_triangle(instance->mesh, index,
							 &local_triangle)) {
			continue;
		}

		first = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[0]),
			entry->position);
		second = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[1]),
			entry->position);
		third = vec3_add(
			mat4_transform_point(instance->transform,
					     local_triangle.vertices[2]),
			entry->position);

		triangle = triangle_create(first, second, third);
		triangle_bounds = triangle_get_bounds(triangle);

		if (!aabb_intersects(swept_bounds, triangle_bounds)) {
			continue;
		}

		candidate.hit = false;
		candidate.started_inside = false;
		candidate.fraction = 1.0f;
		candidate.position = end;
		candidate.normal = vec3_create(0.0f, 0.0f, 0.0f);
		candidate.entity_id = entry->entity_id;

		if (!trace_aabb_against_triangle(local_bounds, start, end,
						 triangle, &candidate)) {
			continue;
		}

		if (candidate.started_inside) {
			*trace = candidate;
			return true;
		}

		if (!hit || candidate.fraction < trace->fraction) {
			*trace = candidate;
			hit = true;
		}
	}

	return hit;
}

static bool update_triangle_sweep_axis(vec3_t axis,
				       const vec3_t *vertices,
				       const vec3_t half_extents,
				       const vec3_t start_center,
				       const vec3_t movement,
				       float *entry_time,
				       float *exit_time,
				       vec3_t *normal) {
	float axis_length;
	float triangle_first;
	float triangle_second;
	float triangle_third;
	float triangle_minimum;
	float triangle_maximum;
	float box_center;
	float box_radius;
	float moving_minimum;
	float moving_maximum;
	float axis_movement;
	float first_time;
	float second_time;
	float axis_entry;
	float axis_exit;

	axis_length = vec3_length(axis);

	if (axis_length <= 0.000001f) { return true; }

	axis = vec3_scale(axis, 1.0f / axis_length);

	triangle_first = vec3_dot(vertices[0], axis);
	triangle_second = vec3_dot(vertices[1], axis);
	triangle_third = vec3_dot(vertices[2], axis);

	triangle_minimum =
		fminf(triangle_first, fminf(triangle_second, triangle_third));
	triangle_maximum =
		fmaxf(triangle_first, fmaxf(triangle_second, triangle_third));

	box_center = vec3_dot(start_center, axis);
	box_radius = half_extents.x * fabsf(axis.x) +
		     half_extents.y * fabsf(axis.y) +
		     half_extents.z * fabsf(axis.z);

	moving_minimum = box_center - box_radius;
	moving_maximum = box_center + box_radius;
	axis_movement = vec3_dot(movement, axis);

	if (fabsf(axis_movement) <= 0.000001f) {
		return moving_maximum >= triangle_minimum &&
		       moving_minimum <= triangle_maximum;
	}

	first_time = (triangle_minimum - moving_maximum) / axis_movement;
	second_time = (triangle_maximum - moving_minimum) / axis_movement;

	axis_entry = fminf(first_time, second_time);
	axis_exit = fmaxf(first_time, second_time);

	if (axis_entry > *entry_time) {
		*entry_time = axis_entry;
		*normal = axis_movement > 0.0f ? vec3_scale(axis, -1.0f) : axis;
	}

	if (axis_exit < *exit_time) { *exit_time = axis_exit; }

	return *entry_time <= *exit_time;
}

bool collision_world_get_collider(const collision_world_t *world,
				  const size_t index,
				  entity_id_t *entity_id,
				  collider_t *collider,
				  vec3_t *position,
				  collision_layer_t *layer) {
	const collision_entry_t *entry;

	if (world == NULL || index >= world->count) { return false; }

	entry = &world->entries[index];

	if (entity_id != NULL) { *entity_id = entry->entity_id; }

	if (collider != NULL) { *collider = entry->collider; }

	if (position != NULL) { *position = entry->position; }

	if (layer != NULL) { *layer = entry->layer; }

	return true;
}

static bool test_sat_axis(const vec3_t axis,
			  const vec3_t center_difference,
			  const vec3_t aabb_half_extents,
			  const vec3_t box_axes[3],
			  const vec3_t box_half_extents,
			  float *minimum_depth,
			  vec3_t *minimum_axis) {
	const float epsilon = 0.000001f;
	vec3_t normalized_axis;
	float length;
	float aabb_radius;
	float box_radius;
	float distance;
	float depth;

	length = vec3_length(axis);

	if (length <= epsilon) { return true; }

	normalized_axis = vec3_scale(axis, 1.0f / length);

	aabb_radius =
		aabb_half_extents.x * fabsf(normalized_axis.x) +
		aabb_half_extents.y * fabsf(normalized_axis.y) +
		aabb_half_extents.z * fabsf(normalized_axis.z);

	box_radius =
		box_half_extents.x *
			fabsf(vec3_dot(normalized_axis, box_axes[0])) +
		box_half_extents.y *
			fabsf(vec3_dot(normalized_axis, box_axes[1])) +
		box_half_extents.z *
			fabsf(vec3_dot(normalized_axis, box_axes[2]));

	distance = vec3_dot(center_difference, normalized_axis);
	depth = aabb_radius + box_radius - fabsf(distance);

	if (depth <= 0.0f) { return false; }

	if (depth < *minimum_depth) {
		if (distance < 0.0f) {
			normalized_axis =
				vec3_scale(normalized_axis, -1.0f);
		}

		*minimum_depth = depth;
		*minimum_axis = normalized_axis;
	}

	return true;
}

static bool get_aabb_obb_collision(const aabb_t aabb,
				   const box_collider_t *box,
				   const vec3_t box_position,
				   aabb_collision_t *collision) {
	const vec3_t aabb_axes[3] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
	};
	vec3_t box_axes[3];
	vec3_t aabb_center;
	vec3_t aabb_half_extents;
	vec3_t box_center;
	vec3_t box_half_extents;
	vec3_t center_difference;
	vec3_t minimum_axis;
	float minimum_depth;
	size_t aabb_axis_index;
	size_t box_axis_index;

	if (box == NULL || collision == NULL) { return false; }

	if (!box_collider_get_world_box(*box,
					box_position,
					&box_center,
					box_axes,
					&box_half_extents)) {
		return false;
	}

	aabb_center = aabb_get_center(aabb);
	aabb_half_extents = aabb_get_half_extents(aabb);
	center_difference = vec3_subtract(aabb_center, box_center);

	minimum_depth = INFINITY;
	minimum_axis = vec3_create(0.0f, 0.0f, 0.0f);

	for (aabb_axis_index = 0;
	     aabb_axis_index < 3;
	     aabb_axis_index++) {
		if (!test_sat_axis(aabb_axes[aabb_axis_index],
				   center_difference,
				   aabb_half_extents,
				   box_axes,
				   box_half_extents,
				   &minimum_depth,
				   &minimum_axis)) {
			return false;
		}
	}

	for (box_axis_index = 0;
	     box_axis_index < 3;
	     box_axis_index++) {
		if (!test_sat_axis(box_axes[box_axis_index],
				   center_difference,
				   aabb_half_extents,
				   box_axes,
				   box_half_extents,
				   &minimum_depth,
				   &minimum_axis)) {
			return false;
		}
	}

	for (aabb_axis_index = 0;
	     aabb_axis_index < 3;
	     aabb_axis_index++) {
		for (box_axis_index = 0;
		     box_axis_index < 3;
		     box_axis_index++) {
			if (!test_sat_axis(
				    vec3_cross(
					    aabb_axes[aabb_axis_index],
					    box_axes[box_axis_index]),
				    center_difference,
				    aabb_half_extents,
				    box_axes,
				    box_half_extents,
				    &minimum_depth,
				    &minimum_axis)) {
				return false;
			}
		}
	}

	collision->normal = minimum_axis;
	collision->depth = minimum_depth;

	return true;
}

static bool update_box_sweep_axis(
	vec3_t axis,
	const vec3_t start_center,
	const vec3_t movement,
	const vec3_t moving_half_extents,
	const vec3_t box_center,
	const vec3_t box_axes[3],
	const vec3_t box_half_extents,
	float *entry_time,
	float *exit_time,
	vec3_t *normal) {
	const float epsilon = 0.000001f;
	float axis_length;
	float moving_center;
	float moving_radius;
	float static_center;
	float static_radius;
	float moving_minimum;
	float moving_maximum;
	float static_minimum;
	float static_maximum;
	float axis_movement;
	float first_time;
	float second_time;
	float axis_entry;
	float axis_exit;

	axis_length = vec3_length(axis);

	if (axis_length <= epsilon) { return true; }

	axis = vec3_scale(axis, 1.0f / axis_length);

	moving_center = vec3_dot(start_center, axis);
	moving_radius =
		moving_half_extents.x * fabsf(axis.x) +
		moving_half_extents.y * fabsf(axis.y) +
		moving_half_extents.z * fabsf(axis.z);

	static_center = vec3_dot(box_center, axis);
	static_radius =
		box_half_extents.x *
			fabsf(vec3_dot(box_axes[0], axis)) +
		box_half_extents.y *
			fabsf(vec3_dot(box_axes[1], axis)) +
		box_half_extents.z *
			fabsf(vec3_dot(box_axes[2], axis));

	moving_minimum = moving_center - moving_radius;
	moving_maximum = moving_center + moving_radius;
	static_minimum = static_center - static_radius;
	static_maximum = static_center + static_radius;

	axis_movement = vec3_dot(movement, axis);

	if (fabsf(axis_movement) <= epsilon) {
		return moving_maximum >= static_minimum &&
		       moving_minimum <= static_maximum;
	}

	first_time =
		(static_minimum - moving_maximum) / axis_movement;
	second_time =
		(static_maximum - moving_minimum) / axis_movement;

	axis_entry = fminf(first_time, second_time);
	axis_exit = fmaxf(first_time, second_time);

	if (axis_entry > *entry_time) {
		*entry_time = axis_entry;
		*normal = axis_movement > 0.0f
				  ? vec3_scale(axis, -1.0f)
				  : axis;
	}

	if (axis_exit < *exit_time) {
		*exit_time = axis_exit;
	}

	return *entry_time <= *exit_time;
}

static bool trace_aabb_against_box(
	const aabb_t local_bounds,
	const vec3_t start,
	const vec3_t end,
	const box_collider_t *box,
	const vec3_t box_position,
	collision_trace_t *trace) {
	const vec3_t aabb_axes[3] = {
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
	};
	aabb_collision_t overlap;
	aabb_t start_bounds;
	vec3_t box_axes[3];
	vec3_t start_center;
	vec3_t moving_half_extents;
	vec3_t box_center;
	vec3_t box_half_extents;
	vec3_t movement;
	vec3_t normal;
	vec3_t axis;
	float entry_time;
	float exit_time;
	size_t aabb_axis_index;
	size_t box_axis_index;

	if (box == NULL || trace == NULL) { return false; }

	start_bounds = aabb_translate(local_bounds, start);

	if (get_aabb_obb_collision(start_bounds,
				   box,
				   box_position,
				   &overlap)) {
		trace->hit = true;
		trace->started_inside = true;
		trace->fraction = 0.0f;
		trace->position = start;
		trace->normal = overlap.normal;
		return true;
	}

	if (!box_collider_get_world_box(*box,
					box_position,
					&box_center,
					box_axes,
					&box_half_extents)) {
		return false;
	}

	start_center = aabb_get_center(start_bounds);
	moving_half_extents =
		aabb_get_half_extents(start_bounds);
	movement = vec3_subtract(end, start);

	entry_time = -INFINITY;
	exit_time = INFINITY;
	normal = vec3_create(0.0f, 0.0f, 0.0f);

	for (aabb_axis_index = 0;
	     aabb_axis_index < 3;
	     aabb_axis_index++) {
		if (!update_box_sweep_axis(
			    aabb_axes[aabb_axis_index],
			    start_center,
			    movement,
			    moving_half_extents,
			    box_center,
			    box_axes,
			    box_half_extents,
			    &entry_time,
			    &exit_time,
			    &normal)) {
			return false;
		}
	}

	for (box_axis_index = 0;
	     box_axis_index < 3;
	     box_axis_index++) {
		if (!update_box_sweep_axis(
			    box_axes[box_axis_index],
			    start_center,
			    movement,
			    moving_half_extents,
			    box_center,
			    box_axes,
			    box_half_extents,
			    &entry_time,
			    &exit_time,
			    &normal)) {
			return false;
		}
	}

	for (aabb_axis_index = 0;
	     aabb_axis_index < 3;
	     aabb_axis_index++) {
		for (box_axis_index = 0;
		     box_axis_index < 3;
		     box_axis_index++) {
			axis = vec3_cross(
				aabb_axes[aabb_axis_index],
				box_axes[box_axis_index]);

			if (!update_box_sweep_axis(
				    axis,
				    start_center,
				    movement,
				    moving_half_extents,
				    box_center,
				    box_axes,
				    box_half_extents,
				    &entry_time,
				    &exit_time,
				    &normal)) {
				return false;
			}
		}
	}

	if (entry_time < 0.0f ||
	    entry_time > 1.0f ||
	    entry_time > exit_time) {
		return false;
	}

	trace->hit = true;
	trace->started_inside = false;
	trace->fraction = entry_time;
	trace->position =
		vec3_add(start, vec3_scale(movement, entry_time));
	trace->normal = normal;

	return true;
}