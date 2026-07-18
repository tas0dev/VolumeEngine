/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collision_world.h"
#include "common.h"

static box_collider_t create_floor_collider(void) {
	return box_collider_create(vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(5.0f, 0.5f, 5.0f));
}

static box_collider_t create_wall_collider(void) {
	return box_collider_create(vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(0.5f, 2.0f, 5.0f));
}

static aabb_t create_moving_bounds(void) {
	return aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			   vec3_create(0.5f, 0.5f, 0.5f));
}

static bool test_add_and_remove(void) {
	collision_world_t *world;
	box_collider_t collider;

	world = collision_world_create();
	CHECK(world != NULL);

	collider = create_floor_collider();

	CHECK(collision_world_add_box(world, 1, collider,
				      vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_get_count(world) == 1);

	CHECK(!collision_world_add_box(world, 1, collider,
				       vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_get_count(world) == 1);

	CHECK(collision_world_remove(world, 1));
	CHECK(collision_world_get_count(world) == 0);
	CHECK(!collision_world_remove(world, 1));

	collision_world_destroy(world);

	return true;
}

static bool test_no_collision(void) {
	collision_world_t *world;
	collision_result_t result;
	vec3_t position;

	world = collision_world_create();
	CHECK(world != NULL);

	CHECK(collision_world_add_box(world, 1, create_floor_collider(),
				      vec3_create(0.0f, -0.5f, 0.0f)));

	position = vec3_create(0.0f, 4.0f, 0.0f);

	CHECK(!collision_world_resolve_aabb(world, create_moving_bounds(),
					    &position, &result));
	CHECK(position.x == 0.0f);
	CHECK(position.y == 4.0f);
	CHECK(position.z == 0.0f);
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

	CHECK(collision_world_add_box(world, 1, create_floor_collider(),
				      vec3_create(0.0f, -0.5f, 0.0f)));

	position = vec3_create(0.0f, 0.25f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.y == 0.5f);
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

	CHECK(collision_world_add_box(world, 1, create_wall_collider(),
				      vec3_create(1.5f, 2.0f, 0.0f)));

	position = vec3_create(1.25f, 0.5f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.x == 0.5f);
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

	CHECK(collision_world_add_box(world, 1, create_floor_collider(),
				      vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_add_box(world, 2, create_wall_collider(),
				      vec3_create(1.5f, 2.0f, 0.0f)));

	position = vec3_create(1.25f, 0.25f, 0.0f);

	CHECK(collision_world_resolve_aabb(world, create_moving_bounds(),
					   &position, &result));
	CHECK(position.x == 0.5f);
	CHECK(position.y == 0.5f);
	CHECK(result.sides & COLLISION_SIDE_NEGATIVE_X);
	CHECK(result.sides & COLLISION_SIDE_POSITIVE_Y);
	CHECK(result.contact_count >= 2);

	collision_world_destroy(world);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"add and remove box",    test_add_and_remove	  },
		{"no collision",		 test_no_collision	  },
		{"floor collision",	    test_floor_collision	},
		{"wall collision",	   test_wall_collision	      },
		{"floor and wall corner", test_floor_and_wall_corner},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}