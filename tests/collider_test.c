/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collider.h"
#include "common.h"

static bool test_none_collider(void) {
	const collider_t collider = collider_create_none();
	aabb_t bounds;

	CHECK(collider.type == COLLIDER_TYPE_NONE);
	CHECK(!collider_get_aabb(&collider, vec3_create(0.0f, 0.0f, 0.0f),
				 &bounds));

	return true;
}

static bool test_box_collider(void) {
	const collider_t collider = collider_create_box(
		vec3_create(1.0f, 2.0f, 3.0f), vec3_create(0.5f, 1.0f, 1.5f));

	CHECK(collider.type == COLLIDER_TYPE_BOX);
	CHECK(collider.shape.box.center.x == 1.0f);
	CHECK(collider.shape.box.center.y == 2.0f);
	CHECK(collider.shape.box.center.z == 3.0f);
	CHECK(collider.shape.box.half_extents.x == 0.5f);
	CHECK(collider.shape.box.half_extents.y == 1.0f);
	CHECK(collider.shape.box.half_extents.z == 1.5f);

	return true;
}

static bool test_box_aabb(void) {
	const collider_t collider = collider_create_box(
		vec3_create(1.0f, 2.0f, 3.0f), vec3_create(0.5f, 1.0f, 1.5f));
	aabb_t bounds;

	CHECK(collider_get_aabb(&collider, vec3_create(10.0f, 1.0f, -3.0f),
				&bounds));

	CHECK(bounds.minimum.x == 10.5f);
	CHECK(bounds.minimum.y == 2.0f);
	CHECK(bounds.minimum.z == -1.5f);
	CHECK(bounds.maximum.x == 11.5f);
	CHECK(bounds.maximum.y == 4.0f);
	CHECK(bounds.maximum.z == 1.5f);

	return true;
}

static bool test_invalid_arguments(void) {
	const collider_t collider = collider_create_box(
		vec3_create(0.0f, 0.0f, 0.0f), vec3_create(1.0f, 1.0f, 1.0f));
	aabb_t bounds;

	CHECK(!collider_get_aabb(NULL, vec3_create(0.0f, 0.0f, 0.0f), &bounds));
	CHECK(!collider_get_aabb(&collider, vec3_create(0.0f, 0.0f, 0.0f),
				 NULL));

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"none collider",		  test_none_collider    },
		{"box collider",		 test_box_collider	  },
		{"box collider AABB",	      test_box_aabb	   },
		{"invalid collider arguments", test_invalid_arguments},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}