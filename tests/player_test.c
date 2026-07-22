/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "collision/triangle.h"
#include "collision/triangle_mesh_collider.h"
#include "entity/func_ladder.h"
#include "entity/player.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include "math/mat4.h"
#include <math.h>

static triangle_mesh_collider_t *create_ramp(const float slope) {
	triangle_t triangles[2];

	triangles[0] = triangle_create(vec3_create(-5.0f, -5.0f * slope, -5.0f),
				       vec3_create(-5.0f, -5.0f * slope, 5.0f),
				       vec3_create(5.0f, 5.0f * slope, 5.0f));
	triangles[1] = triangle_create(vec3_create(-5.0f, -5.0f * slope, -5.0f),
				       vec3_create(5.0f, 5.0f * slope, 5.0f),
				       vec3_create(5.0f, 5.0f * slope, -5.0f));
	return triangle_mesh_collider_create(triangles, 2);
}

static bool test_player_moves_without_hitting_itself(void) {
	character_move_input_t input = {0};
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
	input.crouch = false;
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
	character_move_input_t input = {0};
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
	input.crouch = false;

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
	character_move_input_t input = {0};
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
	input.crouch = false;
	character_controller_move(&controller, world, &input, 0.3f);

	CHECK(controller.position.x > 0.8f);
	CHECK(controller.position.y > 0.29f && controller.position.y < 0.31f);
	CHECK(controller.grounded);

	collision_world_destroy(world);
	return true;
}

static bool test_crouch_waits_for_standing_clearance(void) {
	character_controller_t controller;
	character_move_input_t input = {0};
	collision_world_t *world;
	float crouched_view_height;

	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider(
		world, 1,
		collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
				    vec3_create(1.0f, 0.1f, 1.0f)),
		vec3_create(0.0f, 1.3f, 0.0f)));

	controller = character_controller_create(vec3_create(0.0f, 0.0f, 0.0f),
						 0.35f, 1.7f);
	controller.gravity = 0.0f;
	input.wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
	input.wish_speed = 0.0f;
	input.jump = false;
	input.crouch = true;
	character_controller_move(&controller, world, &input, 0.1f);
	CHECK(controller.crouched);
	CHECK(aabb_get_half_extents(controller.bounds).y <
	      aabb_get_half_extents(controller.standing_bounds).y);
	CHECK(controller.view_height < controller.standing_view_height);
	crouched_view_height = controller.view_height;

	input.crouch = false;
	character_controller_move(&controller, world, &input, 0.1f);
	CHECK(controller.crouched);
	CHECK(controller.view_height <= crouched_view_height);

	CHECK(collision_world_remove(world, 1));
	character_controller_move(&controller, world, &input, 0.1f);
	CHECK(!controller.crouched);
	CHECK(aabb_get_half_extents(controller.bounds).y > 0.8f);
	CHECK(controller.view_height > crouched_view_height);

	collision_world_destroy(world);
	return true;
}

static bool test_walkable_ramp_becomes_ground(void) {
	character_controller_t controller;
	character_move_input_t input = {0};
	triangle_mesh_collider_t *ramp;
	collision_world_t *world;
	size_t tick;

	ramp = create_ramp(0.5f);
	CHECK(ramp != NULL);
	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider(
		world, 10, collider_create_triangle_mesh(ramp, mat4_identity()),
		vec3_create(0.0f, 0.0f, 0.0f)));

	controller = character_controller_create(vec3_create(0.0f, 2.0f, 0.0f),
						 0.35f, 1.7f);
	input.wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
	input.wish_speed = 0.0f;
	input.jump = false;
	input.crouch = false;
	for (tick = 0; tick < 90; tick++) {
		character_controller_move(&controller, world, &input,
					  1.0f / 60.0f);
	}

	CHECK(controller.grounded);
	CHECK(controller.ground_entity_id == 10);
	CHECK(controller.ground_normal.y > controller.minimum_ground_normal_y);

	collision_world_destroy(world);
	triangle_mesh_collider_destroy(ramp);
	return true;
}

static bool test_steep_ramp_surfs_without_grounding(void) {
	character_controller_t controller;
	character_move_input_t input = {0};
	triangle_mesh_collider_t *ramp;
	collision_world_t *world;
	size_t tick;

	ramp = create_ramp(2.0f);
	CHECK(ramp != NULL);
	world = collision_world_create();
	CHECK(world != NULL);
	CHECK(collision_world_add_collider(
		world, 11, collider_create_triangle_mesh(ramp, mat4_identity()),
		vec3_create(0.0f, 0.0f, 0.0f)));

	controller = character_controller_create(vec3_create(0.0f, 2.0f, 0.0f),
						 0.35f, 1.7f);
	input.wish_direction = vec3_create(0.0f, 0.0f, 1.0f);
	input.wish_speed = controller.maximum_speed;
	input.jump = false;
	input.crouch = false;
	for (tick = 0; tick < 45; tick++) {
		character_controller_move(&controller, world, &input,
					  1.0f / 60.0f);
	}

	CHECK(!controller.grounded);
	CHECK(controller.surfing);
	CHECK(controller.surf_normal.y > 0.0f);
	CHECK(controller.surf_normal.y < controller.minimum_ground_normal_y);
	CHECK(controller.position.x < -0.1f);
	CHECK(controller.position.z > 0.2f);
	CHECK(controller.velocity.x < -0.1f);

	collision_world_destroy(world);
	triangle_mesh_collider_destroy(ramp);
	return true;
}

static bool test_player_climbs_and_jumps_off_ladder(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_ladder\"\n"
		"\t\"targetname\" \"ladder\"\n"
		"\t\"origin\" \"0 1 0\"\n"
		"\t\"size\" \"2 4 0.5\"\n"
		"\t\"normal\" \"0 0 1\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"climber\"\n"
		"\t\"origin\" \"0 0 0.3\"\n}\n";
	character_move_input_t input = {0};
	player_t *player;
	world_t *world;
	map_t *map;
	vec3_t climbed_position;
	char error[256];

	CHECK(func_ladder_register());
	CHECK(player_register());
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, NULL, error, sizeof(error)));
	player = player_from_entity(world_find_by_targetname(world, "climber"));
	CHECK(player != NULL);

	input.wish_direction = vec3_create(0.0f, 0.0f, -1.0f);
	input.look_direction = vec3_create(0.0f, 0.0f, -1.0f);
	input.wish_speed = 4.0f;
	player_move(player, &input, 0.3f);
	CHECK(player_is_on_ladder(player));
	CHECK(player_get_position(player).y > 0.89f);
	climbed_position = player_get_position(player);

	input.wish_direction = vec3_create(0.0f, 0.0f, 1.0f);
	player_move(player, &input, 0.1f);
	CHECK(player_is_on_ladder(player));
	CHECK(player_get_position(player).y < climbed_position.y);

	input.wish_direction = vec3_create(0.0f, 0.0f, -1.0f);
	input.jump = true;
	player_move(player, &input, 0.01f);
	CHECK(!player_is_on_ladder(player));
	CHECK(player_get_velocity(player).z > 0.0f);

	input.jump = false;
	player_move(player, &input, 0.05f);
	CHECK(!player_is_on_ladder(player));

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
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
		{"crouch waits for standing clearance",
		 test_crouch_waits_for_standing_clearance},
		{"walkable ramp becomes ground",
		 test_walkable_ramp_becomes_ground	  },
		{"steep ramp surfs without grounding",
		 test_steep_ramp_surfs_without_grounding },
		{"player climbs and jumps off ladder",
		 test_player_climbs_and_jumps_off_ladder },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
