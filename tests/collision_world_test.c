/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collision_world.h"
#include "common.h"
#include "math/math.h"

static collider_t create_floor_collider(void) {
	return collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(5.0f, 0.5f, 5.0f));
}

static collider_t create_wall_collider(void) {
	return collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(0.5f, 2.0f, 5.0f));
}

static aabb_t create_moving_bounds(void) {
	return aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			   vec3_create(0.5f, 0.5f, 0.5f));
}

static bool test_add_and_remove(void) {
	collision_world_t *world;
	collider_t collider;

	world = collision_world_create();
	CHECK(world != NULL);

	collider = create_floor_collider();

	CHECK(collision_world_add_collider(world, 1, collider,
					   vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_get_count(world) == 1);

	CHECK(!collision_world_add_collider(world, 1, collider,
					    vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_get_count(world) == 1);

	CHECK(collision_world_remove(world, 1));
	CHECK(collision_world_get_count(world) == 0);
	CHECK(!collision_world_remove(world, 1));

	collision_world_destroy(world);

	return true;
}

static bool test_reject_none_collider(void) {
	collision_world_t *world;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(!collision_world_add_collider(world, 1, collider_create_none(),
					    vec3_create(0.0f, 0.0f, 0.0f)));
	CHECK(collision_world_get_count(world) == 0);

	collision_world_destroy(world);

	return true;
}

static bool test_no_collision(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(collision_world_add_collider(world, 1, create_floor_collider(),
					   vec3_create(0.0f, -0.5f, 0.0f)));

	position = vec3_create(0.0f, 4.0f, 0.0f);

	CHECK(!collision_world_resolve_aabb(world, create_moving_bounds(),
					    &position, &result));
	CHECK(position.x == 0.0f);
	CHECK(position.y == 4.0f);
	CHECK(position.z == 0.0f);
	CHECK(result.correction.x == 0.0f);
	CHECK(result.correction.y == 0.0f);
	CHECK(result.correction.z == 0.0f);
	CHECK(result.contact_count == 0);
	CHECK(result.sides == COLLISION_SIDE_NONE);

	collision_world_destroy(world);

	return true;
}

static bool test_floor_collision(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(collision_world_add_collider(world, 1, create_floor_collider(),
					   vec3_create(0.0f, -0.5f, 0.0f)));

	position = vec3_create(0.0f, 0.25f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.x == 0.0f);
	CHECK(position.y == 0.5f);
	CHECK(position.z == 0.0f);
	CHECK(result.correction.y == 0.25f);
	CHECK(result.sides & COLLISION_SIDE_POSITIVE_Y);
	CHECK(result.contact_count >= 1);

	collision_world_destroy(world);

	return true;
}

static bool test_wall_collision(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(collision_world_add_collider(world, 1, create_wall_collider(),
					   vec3_create(1.5f, 2.0f, 0.0f)));

	position = vec3_create(1.25f, 0.5f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.x == 0.5f);
	CHECK(position.y == 0.5f);
	CHECK(position.z == 0.0f);
	CHECK(result.correction.x == -0.75f);
	CHECK(result.sides & COLLISION_SIDE_NEGATIVE_X);
	CHECK(result.contact_count >= 1);

	collision_world_destroy(world);

	return true;
}

static bool test_floor_and_wall_corner(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(collision_world_add_collider(world, 1, create_floor_collider(),
					   vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_add_collider(world, 2, create_wall_collider(),
					   vec3_create(1.5f, 2.0f, 0.0f)));

	position = vec3_create(1.25f, 0.25f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.x == 0.5f);
	CHECK(position.y == 0.5f);
	CHECK(position.z == 0.0f);
	CHECK(result.sides & COLLISION_SIDE_NEGATIVE_X);
	CHECK(result.sides & COLLISION_SIDE_POSITIVE_Y);
	CHECK(result.contact_count >= 2);

	collision_world_destroy(world);

	return true;
}

static bool test_invalid_arguments(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	position = vec3_create(0.0f, 0.0f, 0.0f);

	CHECK(!collision_world_resolve_aabb(NULL, create_moving_bounds(),
					    &position, &result));
	CHECK(!collision_world_resolve_aabb(world, create_moving_bounds(), NULL,
					    &result));

	collision_world_destroy(world);

	return true;
}

static bool test_resolve_ignores_entity(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider(
		world, 42,
		collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				    vec3_create(0.5f, 0.5f, 0.5f)),
		vec3_create(0.0f, 0.0f, 0.0f)));

	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(!collision_world_resolve_aabb_ignoring(
		world, create_moving_bounds(), &position, 42, &result));
	CHECK(position.x == 0.0f && position.y == 0.0f && position.z == 0.0f);

	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(collision_world_resolve_aabb_ignoring(
		world, create_moving_bounds(), &position, 0, &result));

	collision_world_destroy(world);
	return true;
}

static bool test_collision_layers_filter_queries(void) {
	collision_filter_t filter;
	collision_result_t result;
	collision_world_t *world;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider_filtered(
		world, 1,
		collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				    vec3_create(0.5f, 0.5f, 0.5f)),
		vec3_create(0.0f, 0.0f, 0.0f), COLLISION_LAYER_TRIGGER,
		COLLISION_LAYER_PLAYER));

	filter.layer = COLLISION_LAYER_PLAYER;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC;
	filter.ignored_entity_id = 0;
	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(!collision_world_resolve_aabb_filtered(
		world, create_moving_bounds(), &position, filter, &result));

	filter.mask |= COLLISION_LAYER_TRIGGER;
	CHECK(collision_world_resolve_aabb_filtered(
		world, create_moving_bounds(), &position, filter, &result));

	collision_world_destroy(world);
	return true;
}

static bool test_ray_trace_uses_layers_and_nearest_hit(void) {
	collision_filter_t filter;
	collision_trace_t trace;
	collision_world_t *world;
	collider_t collider;

	world = collision_world_create();
	CHECK(world != NULL);
	collider = collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				       vec3_create(0.5f, 0.5f, 0.5f));
	CHECK(collision_world_add_collider_filtered(
		world, 1, collider, vec3_create(0.0f, 0.0f, -0.5f),
		COLLISION_LAYER_TRIGGER, COLLISION_LAYER_PLAYER));
	CHECK(collision_world_add_collider_filtered(
		world, 2, collider, vec3_create(0.0f, 0.0f, -2.0f),
		COLLISION_LAYER_DYNAMIC, COLLISION_LAYER_ALL));
	CHECK(collision_world_add_collider_filtered(
		world, 3, collider, vec3_create(0.0f, 0.0f, -4.0f),
		COLLISION_LAYER_WORLD_STATIC, COLLISION_LAYER_ALL));

	filter.layer = COLLISION_LAYER_PLAYER;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC;
	filter.ignored_entity_id = 0;
	CHECK(collision_world_trace_ray_filtered(
		world, vec3_create(0.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 0.0f, -6.0f), filter, &trace));
	CHECK(trace.entity_id == 2);
	CHECK(trace.fraction > 0.24f && trace.fraction < 0.26f);

	collision_world_destroy(world);
	return true;
}

static bool test_rotated_box_does_not_use_aabb_approximation(void) {
	collision_result_t result;
	collision_world_t *world;
	collider_t collider;
	mat4_t transform;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	transform = mat4_rotation_z(PI * 0.25f);
	transform = mat4_multiply(
		mat4_translation(vec3_create(0.0f, 0.0f, 0.0f)), transform);

	collider = collider_create_box_transformed(
		vec3_create(0.0f, 0.0f, 0.0f), vec3_create(2.0f, 0.1f, 0.5f),
		transform);

	CHECK(collision_world_add_collider(world, 1, collider,
					   vec3_create(0.0f, 0.0f, 0.0f)));

	position = vec3_create(1.35f, -1.35f, 0.0f);

	CHECK(!collision_world_resolve_aabb(
		world,
		aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			    vec3_create(0.1f, 0.1f, 0.1f)),
		&position, &result));

	collision_world_destroy(world);

	return true;
}

static bool test_rotated_box_returns_sat_correction(void) {
	collision_result_t result;
	collision_world_t *world;
	collider_t collider;
	mat4_t transform;
	vec3_t position;
	float original_x;
	float original_y;

	world = collision_world_create();
	CHECK(world != NULL);

	transform = mat4_rotation_z(PI * 0.25f);

	collider = collider_create_box_transformed(
		vec3_create(0.0f, 0.0f, 0.0f), vec3_create(2.0f, 0.25f, 0.5f),
		transform);

	CHECK(collision_world_add_collider(world, 1, collider,
					   vec3_create(0.0f, 0.0f, 0.0f)));

	position = vec3_create(0.0f, 0.2f, 0.0f);
	original_x = position.x;
	original_y = position.y;

	CHECK(collision_world_resolve_aabb(
		world,
		aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			    vec3_create(0.2f, 0.2f, 0.2f)),
		&position, &result));

	CHECK(result.contact_count >= 1);
	CHECK(result.correction.x != 0.0f);
	CHECK(result.correction.y != 0.0f);
	CHECK(position.x != original_x);
	CHECK(position.y != original_y);

	collision_world_destroy(world);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"add and remove collider",		    test_add_and_remove	       },
		{"reject none collider",			 test_reject_none_collider  },
		{"no collision",				 test_no_collision	  },
		{"floor collision",			    test_floor_collision	},
		{"wall collision",			   test_wall_collision	      },
		{"floor and wall corner",		  test_floor_and_wall_corner },
		{"invalid collision world arguments",     test_invalid_arguments	    },
		{"resolve ignores an entity",	      test_resolve_ignores_entity},
		{"collision layers filter queries",
		 test_collision_layers_filter_queries				     },
		{"ray trace uses layers and nearest hit",
		 test_ray_trace_uses_layers_and_nearest_hit			   },
		{"rotated box does not use AABB approximation",
		 test_rotated_box_does_not_use_aabb_approximation				 },
		{"rotated box returns SAT correction",
		 test_rotated_box_returns_sat_correction					},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
