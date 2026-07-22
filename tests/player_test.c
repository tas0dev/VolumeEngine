/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/player.h"
#include "entity/world.h"
#include <math.h>

static bool test_player_moves_without_hitting_itself(void) {
	character_move_input_t input;
	entity_properties_t properties;
	entity_spawn_context_t context = {0};
	entity_t *entity;
	player_t *player;
	vec3_t position;
	world_t *world;

	CHECK(player_register());
	world = world_create();
	CHECK(world != NULL);

	properties = entity_properties_create();
	properties.targetname = "test_player";
	context.properties = &properties;
	entity = world_spawn_entity(world, "player", &context);
	player = player_from_entity(entity);
	CHECK(player != NULL);
	CHECK(collision_world_get_count(world_get_collision_world(world)) == 1);

	input.wish_direction = vec3_create(1.0f, 0.0f, 0.0f);
	input.wish_speed = 4.0f;
	input.jump = false;
	player_move(player, &input, 0.1f);

	position = player_get_position(player);
	CHECK(position.x > 0.0f);
	CHECK(player_get_entity(player)->transform.position.x == position.x);

	world_destroy(world);
	entity_registry_shutdown();
	return true;
}

static bool test_air_strafe_exceeds_ground_speed(void) {
	character_controller_t controller;
	character_move_input_t input;
	vec3_t perpendicular;
	float horizontal_speed;
	size_t tick;

	controller = character_controller_create(vec3_create(0.0f, 0.0f, 0.0f),
						 0.35f, 1.7f);
	controller.gravity = 0.0f;
	controller.velocity = vec3_create(controller.maximum_speed, 0.0f, 0.0f);
	controller.grounded = false;
	input.wish_speed = controller.maximum_speed;
	input.jump = false;

	for (tick = 0; tick < 60; tick++) {
		perpendicular = vec3_create(-controller.velocity.z, 0.0f,
					    controller.velocity.x);
		input.wish_direction = vec3_normalize(perpendicular);
		character_controller_move(&controller, NULL, &input,
					  1.0f / 60.0f);
	}

	horizontal_speed = sqrtf(controller.velocity.x * controller.velocity.x +
				 controller.velocity.z * controller.velocity.z);
	CHECK(horizontal_speed > controller.maximum_speed * 1.5f);
	return true;
}

static bool test_ground_move_steps_over_small_ledge(void) {
	character_controller_t controller;
	character_move_input_t input;
	collision_world_t *world;

	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider(
		world, 1,
		collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				    vec3_create(5.0f, 0.5f, 2.0f)),
		vec3_create(0.0f, -0.5f, 0.0f)));
	CHECK(collision_world_add_collider(
		world, 2,
		collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				    vec3_create(0.4f, 0.15f, 1.0f)),
		vec3_create(1.0f, 0.15f, 0.0f)));

	controller = character_controller_create(vec3_create(0.0f, 0.0f, 0.0f),
						 0.35f, 1.7f);
	controller.grounded = true;
	input.wish_direction = vec3_create(1.0f, 0.0f, 0.0f);
	input.wish_speed = controller.maximum_speed;
	input.jump = false;
	character_controller_move(&controller, world, &input, 0.3f);

	CHECK(controller.position.x > 0.8f);
	CHECK(controller.position.y > 0.29f && controller.position.y < 0.31f);
	CHECK(controller.grounded);

	collision_world_destroy(world);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"player moves without hitting itself",
		 test_player_moves_without_hitting_itself},
		{"air strafe exceeds ground speed",
		 test_air_strafe_exceeds_ground_speed    },
		{"ground move steps over a small ledge",
		 test_ground_move_steps_over_small_ledge },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
