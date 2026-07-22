/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/player.h"
#include "entity/world.h"

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

int main(void) {
	static const test_case_t tests[] = {
		{"player moves without hitting itself",
		 test_player_moves_without_hitting_itself},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
