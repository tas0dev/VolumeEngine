/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/aabb.h"
#include "collision/box_collider.h"
#include "common.h"

static bool test_separated(void) {
	const aabb_t first = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
					 vec3_create(1.0f, 1.0f, 1.0f));
	const aabb_t second = aabb_create(vec3_create(3.0f, 0.0f, 0.0f),
					  vec3_create(1.0f, 1.0f, 1.0f));

	CHECK(!aabb_intersects(first, second));

	return true;
}

static bool test_touching(void) {
	const aabb_t first = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
					 vec3_create(1.0f, 1.0f, 1.0f));
	const aabb_t second = aabb_create(vec3_create(2.0f, 0.0f, 0.0f),
					  vec3_create(1.0f, 1.0f, 1.0f));

	CHECK(!aabb_intersects(first, second));

	return true;
}

static bool test_overlap(void) {
	const aabb_t first = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
					 vec3_create(1.0f, 1.0f, 1.0f));
	const aabb_t second = aabb_create(vec3_create(1.5f, 0.0f, 0.0f),
					  vec3_create(1.0f, 1.0f, 1.0f));
	aabb_collision_t collision;

	CHECK(aabb_get_collision(first, second, &collision));
	CHECK(collision.depth == 0.5f);
	CHECK(collision.normal.x == -1.0f);
	CHECK(collision.normal.y == 0.0f);
	CHECK(collision.normal.z == 0.0f);

	return true;
}

static bool test_contained(void) {
	const aabb_t first = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
					 vec3_create(1.0f, 1.0f, 1.0f));
	const aabb_t second = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
					  vec3_create(5.0f, 5.0f, 5.0f));
	aabb_collision_t collision;

	CHECK(aabb_get_collision(first, second, &collision));
	CHECK(collision.depth == 6.0f);
	CHECK(collision.normal.x == -1.0f);
	CHECK(collision.normal.y == 0.0f);
	CHECK(collision.normal.z == 0.0f);

	return true;
}

static bool test_translation(void) {
	aabb_t aabb = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
				  vec3_create(1.0f, 2.0f, 3.0f));

	aabb = aabb_translate(aabb, vec3_create(5.0f, -1.0f, 2.0f));

	CHECK(aabb.minimum.x == 4.0f);
	CHECK(aabb.minimum.y == -3.0f);
	CHECK(aabb.minimum.z == -1.0f);
	CHECK(aabb.maximum.x == 6.0f);
	CHECK(aabb.maximum.y == 1.0f);
	CHECK(aabb.maximum.z == 5.0f);

	return true;
}

static bool test_box_collider(void) {
	const box_collider_t collider =
		box_collider_create(vec3_create(1.0f, 2.0f, 3.0f),
				    vec3_create(-0.5f, -1.0f, -1.5f));
	const aabb_t aabb = box_collider_get_aabb(
		collider, vec3_create(10.0f, 1.0f, -3.0f));

	CHECK(aabb.minimum.x == 10.5f);
	CHECK(aabb.minimum.y == 2.0f);
	CHECK(aabb.minimum.z == -1.5f);
	CHECK(aabb.maximum.x == 11.5f);
	CHECK(aabb.maximum.y == 4.0f);
	CHECK(aabb.maximum.z == 1.5f);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"separated AABBs",   test_separated	  },
		{"touching AABBs",	   test_touching	},
		{"overlapping AABBs", test_overlap	  },
		{"contained AABB",	   test_contained	 },
		{"AABB translation",  test_translation },
		{"box collider",	 test_box_collider},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}