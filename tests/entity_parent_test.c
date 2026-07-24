/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/entity.h"
#include "entity/world.h"
#include "math/math.h"

#include <math.h>

static bool nearly_equal(const float left, const float right) {
	return fabsf(left - right) < 0.0001f;
}

static bool test_sets_and_clears_parent(void) {
	entity_t parent;
	entity_t child;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	CHECK(entity_set_parent(&child, &parent, false));
	CHECK(entity_get_parent(&child) == &parent);
	CHECK(entity_get_first_child(&parent) == &child);
	CHECK(entity_get_next_sibling(&child) == NULL);

	entity_clear_parent(&child, false);

	CHECK(entity_get_parent(&child) == NULL);
	CHECK(entity_get_first_child(&parent) == NULL);
	CHECK(entity_get_next_sibling(&child) == NULL);

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_manages_multiple_children(void) {
	entity_t parent;
	entity_t first_child;
	entity_t second_child;
	entity_t third_child;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&first_child, 2, NULL);
	entity_initialize(&second_child, 3, NULL);
	entity_initialize(&third_child, 4, NULL);

	CHECK(entity_set_parent(&first_child, &parent, false));
	CHECK(entity_set_parent(&second_child, &parent, false));
	CHECK(entity_set_parent(&third_child, &parent, false));

	CHECK(entity_get_first_child(&parent) == &third_child);
	CHECK(entity_get_next_sibling(&third_child) == &second_child);
	CHECK(entity_get_next_sibling(&second_child) == &first_child);
	CHECK(entity_get_next_sibling(&first_child) == NULL);

	entity_clear_parent(&second_child, false);

	CHECK(entity_get_first_child(&parent) == &third_child);
	CHECK(entity_get_next_sibling(&third_child) == &first_child);
	CHECK(entity_get_next_sibling(&first_child) == NULL);
	CHECK(entity_get_parent(&second_child) == NULL);
	CHECK(entity_get_next_sibling(&second_child) == NULL);

	entity_destroy(&third_child);

	CHECK(entity_get_first_child(&parent) == &first_child);
	CHECK(entity_get_next_sibling(&first_child) == NULL);

	entity_destroy(&second_child);
	entity_destroy(&first_child);
	entity_destroy(&parent);

	return true;
}

static bool test_rejects_invalid_hierarchy(void) {
	entity_t first;
	entity_t second;
	entity_t third;

	entity_initialize(&first, 1, NULL);
	entity_initialize(&second, 2, NULL);
	entity_initialize(&third, 3, NULL);

	CHECK(!entity_set_parent(NULL, &first, false));
	CHECK(!entity_set_parent(&first, &first, false));

	CHECK(entity_set_parent(&second, &first, false));
	CHECK(entity_set_parent(&third, &second, false));

	CHECK(!entity_set_parent(&first, &third, false));
	CHECK(entity_get_parent(&first) == NULL);
	CHECK(entity_get_parent(&second) == &first);
	CHECK(entity_get_parent(&third) == &second);

	entity_destroy(&third);
	entity_destroy(&second);
	entity_destroy(&first);

	return true;
}

static bool test_rejects_different_worlds(void) {
	entity_t parent;
	entity_t child;
	world_t *first_world;
	world_t *second_world;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	first_world = world_create();
	second_world = world_create();

	CHECK(first_world != NULL);
	CHECK(second_world != NULL);

	parent.world = first_world;
	child.world = second_world;

	CHECK(!entity_set_parent(&child, &parent, false));
	CHECK(entity_get_parent(&child) == NULL);
	CHECK(entity_get_first_child(&parent) == NULL);

	child.world = NULL;

	CHECK(!entity_set_parent(&child, &parent, false));

	child.world = first_world;

	CHECK(entity_set_parent(&child, &parent, false));

	entity_clear_parent(&child, false);
	parent.world = NULL;
	child.world = NULL;

	world_destroy(second_world);
	world_destroy(first_world);

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_destroying_child_unlinks_from_parent(void) {
	entity_t parent;
	entity_t first_child;
	entity_t second_child;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&first_child, 2, NULL);
	entity_initialize(&second_child, 3, NULL);

	CHECK(entity_set_parent(&first_child, &parent, false));
	CHECK(entity_set_parent(&second_child, &parent, false));

	CHECK(entity_get_first_child(&parent) == &second_child);
	CHECK(entity_get_next_sibling(&second_child) == &first_child);

	entity_destroy(&second_child);

	CHECK(entity_get_first_child(&parent) == &first_child);
	CHECK(entity_get_next_sibling(&first_child) == NULL);

	entity_destroy(&first_child);

	CHECK(entity_get_first_child(&parent) == NULL);

	entity_destroy(&parent);

	return true;
}

static bool test_destroying_parent_detaches_children(void) {
	entity_t parent;
	entity_t first_child;
	entity_t second_child;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&first_child, 2, NULL);
	entity_initialize(&second_child, 3, NULL);

	CHECK(entity_set_parent(&first_child, &parent, false));
	CHECK(entity_set_parent(&second_child, &parent, false));

	entity_destroy(&parent);

	CHECK(entity_get_parent(&first_child) == NULL);
	CHECK(entity_get_parent(&second_child) == NULL);
	CHECK(entity_get_next_sibling(&first_child) == NULL);
	CHECK(entity_get_next_sibling(&second_child) == NULL);

	entity_destroy(&second_child);
	entity_destroy(&first_child);

	return true;
}

static bool test_world_transform_includes_parent(void) {
	entity_t parent;
	entity_t child;
	entity_t grandchild;
	vec3_t position;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);
	entity_initialize(&grandchild, 3, NULL);

	parent.transform.position = vec3_create(10.0f, 2.0f, -3.0f);
	child.transform.position = vec3_create(2.0f, 3.0f, 4.0f);
	grandchild.transform.position = vec3_create(-1.0f, 1.0f, 2.0f);

	CHECK(entity_set_parent(&child, &parent, false));
	CHECK(entity_set_parent(&grandchild, &child, false));

	position = entity_get_world_position(&grandchild);

	CHECK(nearly_equal(position.x, 11.0f));
	CHECK(nearly_equal(position.y, 6.0f));
	CHECK(nearly_equal(position.z, 3.0f));

	entity_destroy(&grandchild);
	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_world_transform_includes_rotation_and_scale(void) {
	entity_t parent;
	entity_t child;
	vec3_t position;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	parent.transform.position = vec3_create(5.0f, 0.0f, 0.0f);
	parent.transform.rotation.z = PI * 0.5f;
	parent.transform.scale = vec3_create(2.0f, 2.0f, 2.0f);
	child.transform.position = vec3_create(1.0f, 0.0f, 0.0f);

	CHECK(entity_set_parent(&child, &parent, false));

	position = entity_get_world_position(&child);

	CHECK(nearly_equal(position.x, 5.0f));
	CHECK(nearly_equal(position.y, 2.0f));
	CHECK(nearly_equal(position.z, 0.0f));

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_parenting_can_keep_world_transform(void) {
	entity_t parent;
	entity_t child;
	vec3_t position;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	parent.transform.position = vec3_create(10.0f, 4.0f, -2.0f);
	parent.transform.rotation.z = PI * 0.5f;
	parent.transform.scale = vec3_create(2.0f, 2.0f, 2.0f);

	child.transform.position = vec3_create(3.0f, 6.0f, 1.0f);

	CHECK(entity_set_parent(&child, &parent, true));
	CHECK(entity_get_parent(&child) == &parent);

	position = entity_get_world_position(&child);

	CHECK(nearly_equal(position.x, 3.0f));
	CHECK(nearly_equal(position.y, 6.0f));
	CHECK(nearly_equal(position.z, 1.0f));

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_clearing_parent_can_keep_world_transform(void) {
	entity_t parent;
	entity_t child;
	mat4_t before;
	mat4_t after;
	size_t index;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	parent.transform.position = vec3_create(4.0f, 2.0f, -3.0f);
	parent.transform.rotation.y = 0.5f;
	parent.transform.scale = vec3_create(2.0f, 2.0f, 2.0f);

	child.transform.position = vec3_create(1.0f, 2.0f, 3.0f);
	child.transform.rotation.x = 0.2f;
	child.transform.scale = vec3_create(0.5f, 0.5f, 0.5f);

	CHECK(entity_set_parent(&child, &parent, false));

	before = entity_get_world_matrix(&child);

	CHECK(entity_clear_parent(&child, true));
	CHECK(entity_get_parent(&child) == NULL);

	after = entity_get_world_matrix(&child);

	for (index = 0; index < 16; index++) {
		CHECK(nearly_equal(before.elements[index],
				   after.elements[index]));
	}

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

static bool test_parenting_failure_preserves_existing_state(void) {
	entity_t first_parent;
	entity_t second_parent;
	entity_t child;
	transform_t previous_transform;

	entity_initialize(&first_parent, 1, NULL);
	entity_initialize(&second_parent, 2, NULL);
	entity_initialize(&child, 3, NULL);

	CHECK(entity_set_parent(&child, &first_parent, false));

	second_parent.transform.scale = vec3_create(0.0f, 1.0f, 1.0f);
	previous_transform = child.transform;

	CHECK(!entity_set_parent(&child, &second_parent, true));
	CHECK(entity_get_parent(&child) == &first_parent);
	CHECK(child.transform.position.x == previous_transform.position.x);
	CHECK(child.transform.position.y == previous_transform.position.y);
	CHECK(child.transform.position.z == previous_transform.position.z);

	entity_destroy(&child);
	entity_destroy(&second_parent);
	entity_destroy(&first_parent);

	return true;
}

static bool test_world_box_collider_includes_parent_transform(void) {
	collider_t collider;
	entity_t parent;
	entity_t child;
	aabb_t bounds;

	entity_initialize(&parent, 1, NULL);
	entity_initialize(&child, 2, NULL);

	parent.transform.position = vec3_create(10.0f, 5.0f, -2.0f);
	parent.transform.rotation.z = PI * 0.5f;
	parent.transform.scale = vec3_create(2.0f, 2.0f, 2.0f);

	child.transform.position = vec3_create(1.0f, 0.0f, 0.0f);

	entity_set_collider(
		&child, collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
					    vec3_create(0.5f, 1.0f, 0.25f)));

	CHECK(entity_set_parent(&child, &parent, false));
	CHECK(entity_get_world_collider(&child, &collider));
	CHECK(collider_get_aabb(&collider, vec3_create(0.0f, 0.0f, 0.0f),
				&bounds));

	CHECK(nearly_equal(bounds.minimum.x, 8.0f));
	CHECK(nearly_equal(bounds.maximum.x, 12.0f));
	CHECK(nearly_equal(bounds.minimum.y, 6.0f));
	CHECK(nearly_equal(bounds.maximum.y, 8.0f));
	CHECK(nearly_equal(bounds.minimum.z, -2.5f));
	CHECK(nearly_equal(bounds.maximum.z, -1.5f));

	entity_destroy(&child);
	entity_destroy(&parent);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"sets and clears parent",			   test_sets_and_clears_parent   },
		{"manages multiple children",		      test_manages_multiple_children},
		{"rejects invalid hierarchy",		      test_rejects_invalid_hierarchy},
		{"rejects different worlds",		     test_rejects_different_worlds },
		{"destroying child unlinks from parent",
		 test_destroying_child_unlinks_from_parent					  },
		{"destroying parent detaches children",
		 test_destroying_parent_detaches_children					 },
		{"world transform includes parent",
		 test_world_transform_includes_parent					     },
		{"world transform includes rotation and scale",
		 test_world_transform_includes_rotation_and_scale				 },
		{"parenting can keep world transform",
		 test_parenting_can_keep_world_transform					},
		{"clearing parent can keep world transform",
		 test_clearing_parent_can_keep_world_transform				      },
		{"parenting failure preserves existing state",
		 test_parenting_failure_preserves_existing_state				},
		{"world box collider includes parent transform",
		 test_world_box_collider_includes_parent_transform				  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}