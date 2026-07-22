/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "collision/aabb.h"
#include "collision/collision_world.h"
#include "common.h"
#include "entity/entity.h"
#include "entity/func_door.h"
#include "entity/player.h"
#include "entity/prop_static.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include <math.h>

static unsigned char mesh_marker;

static bool test_door_motion_inputs_and_collision(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_door\"\n"
		"\t\"targetname\" \"door\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"move_offset\" \"4 0 0\"\n"
		"\t\"speed\" \"2\"\n"
		"\t\"wait\" \"-1\"\n"
		"\t\"OnFullyOpen\" \"!self,Close,,0.5,1\"\n}\n";
	asset_manager_t *assets;
	collision_result_t collision;
	func_door_t *door;
	material_t material = {0};
	mesh_t *mesh;
	world_t *world;
	map_t *map;
	aabb_t bounds;
	vec3_t position;
	char error[256];

	CHECK(func_door_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	mesh = (mesh_t *)(void *)&mesh_marker;
	CHECK(asset_manager_register_mesh(assets, "models/door", mesh));
	CHECK(asset_manager_register_material(assets, "materials/door",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	door = func_door_from_entity(world_get_entity(world, 0));
	CHECK(door != NULL);
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSED);

	CHECK(world_send_input(world, "door", "Open", "", NULL, NULL) == 1);
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPENING);
	world_update(world, 1.0f);
	CHECK(door->prop.entity.transform.position.x == 2.0f);

	bounds = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			     vec3_create(0.1f, 0.1f, 0.1f));
	position = vec3_create(2.0f, 0.0f, 0.0f);
	CHECK(collision_world_resolve_aabb(world_get_collision_world(world),
					   bounds, &position, &collision));
	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(!collision_world_resolve_aabb(world_get_collision_world(world),
					    bounds, &position, &collision));

	world_update(world, 1.0f);
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPEN);
	CHECK(door->prop.entity.transform.position.x == 4.0f);
	world_update(world, 0.5f);
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSING);
	world_update(world, 2.0f);
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSED);
	CHECK(door->prop.entity.transform.position.x == 0.0f);

	CHECK(world_send_input(world, "door", "Lock", "", NULL, NULL) == 1);
	CHECK(world_send_input(world, "door", "Open", "", NULL, NULL) == 1);
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSED);
	CHECK(world_send_input(world, "door", "Unlock", "", NULL, NULL) == 1);
	CHECK(world_send_input(world, "door", "Toggle", "", NULL, NULL) == 1);
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPENING);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

static bool test_door_stops_and_resumes_after_blocked(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_door\"\n"
		"\t\"targetname\" \"door\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"move_offset\" \"4 0 0\"\n"
		"\t\"speed\" \"4\"\n"
		"\t\"OnBlocked\" \"blocker,Kill,,0,1\"\n}\n"
		"entity\n{\n\t\"classname\" \"prop_static\"\n"
		"\t\"targetname\" \"blocker\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"origin\" \"2 0 0\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n}\n";
	asset_manager_t *assets;
	func_door_t *door;
	entity_t *blocker;
	material_t material = {0};
	mesh_t *mesh;
	world_t *world;
	map_t *map;
	float blocked_position;
	char error[256];

	CHECK(func_door_register());
	CHECK(prop_static_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	mesh = (mesh_t *)(void *)&mesh_marker;
	CHECK(asset_manager_register_mesh(assets, "models/door", mesh));
	CHECK(asset_manager_register_material(assets, "materials/door",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	door = func_door_from_entity(world_get_entity(world, 0));
	blocker = world_find_by_targetname(world, "blocker");
	CHECK(door != NULL && blocker != NULL);

	CHECK(world_send_input(world, "door", "Open", "", NULL, NULL) == 1);
	world_update(world, 1.0f);
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPENING);
	CHECK(func_door_is_blocked(door));
	blocked_position = door->prop.entity.transform.position.x;
	CHECK(blocked_position > 0.9f && blocked_position < 1.1f);

	world_update(world, 0.1f);
	CHECK(world_find_by_targetname(world, "blocker") == NULL);
	CHECK(func_door_is_blocked(door));
	CHECK(door->prop.entity.transform.position.x == blocked_position);

	world_update(world, 1.0f);
	CHECK(!func_door_is_blocked(door));
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPEN);
	CHECK(door->prop.entity.transform.position.x == 4.0f);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

static bool test_door_is_blocked_by_player(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_door\"\n"
		"\t\"targetname\" \"door\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"move_offset\" \"4 0 0\"\n"
		"\t\"speed\" \"4\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"player\"\n"
		"\t\"origin\" \"2 -0.85 0\"\n"
		"\t\"height\" \"1.7\"\n"
		"\t\"radius\" \"0.35\"\n}\n";
	asset_manager_t *assets;
	func_door_t *door;
	material_t material = {0};
	mesh_t *mesh;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(func_door_register());
	CHECK(player_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	mesh = (mesh_t *)(void *)&mesh_marker;
	CHECK(asset_manager_register_mesh(assets, "models/door", mesh));
	CHECK(asset_manager_register_material(assets, "materials/door",
					      &material));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	door = func_door_from_entity(world_find_by_targetname(world, "door"));
	CHECK(door != NULL);
	CHECK(world_find_by_targetname(world, "player") != NULL);

	CHECK(world_send_input(world, "door", "Open", "", NULL, NULL) == 1);
	world_update(world, 1.0f);
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPENING);
	CHECK(func_door_is_blocked(door));
	CHECK(door->prop.entity.transform.position.x > 1.1f);
	CHECK(door->prop.entity.transform.position.x < 1.2f);
	CHECK(world_remove_entity(
		world, world_find_by_targetname(world, "player")->id));
	world_update(world, 1.0f);
	CHECK(!func_door_is_blocked(door));
	CHECK(func_door_get_state(door) == FUNC_DOOR_OPEN);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

static bool test_door_reverses_after_blocked(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_door\"\n"
		"\t\"targetname\" \"door\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"move_offset\" \"4 0 0\"\n"
		"\t\"speed\" \"4\"\n"
		"\t\"block_policy\" \"reverse\"\n}\n"
		"entity\n{\n\t\"classname\" \"prop_static\"\n"
		"\t\"targetname\" \"blocker\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"origin\" \"2 0 0\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n}\n";
	asset_manager_t *assets;
	func_door_t *door;
	material_t material = {0};
	mesh_t *mesh;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(func_door_register());
	CHECK(prop_static_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	mesh = (mesh_t *)(void *)&mesh_marker;
	CHECK(asset_manager_register_mesh(assets, "models/door", mesh));
	CHECK(asset_manager_register_material(assets, "materials/door",
					      &material));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	door = func_door_from_entity(world_find_by_targetname(world, "door"));
	CHECK(door != NULL);

	CHECK(world_send_input(world, "door", "Open", "", NULL, NULL) == 1);
	world_update(world, 1.0f);
	CHECK(func_door_is_blocked(door));
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSING);
	CHECK(door->prop.entity.transform.position.x > 0.9f);
	CHECK(door->prop.entity.transform.position.x < 1.1f);

	world_update(world, 1.0f);
	CHECK(!func_door_is_blocked(door));
	CHECK(func_door_get_state(door) == FUNC_DOOR_CLOSED);
	CHECK(fabsf(door->prop.entity.transform.position.x) < 0.0001f);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

static bool test_door_carries_rider_and_passes_velocity_to_jump(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_door\"\n"
		"\t\"targetname\" \"platform\"\n"
		"\t\"model\" \"models/door\"\n"
		"\t\"material\" \"materials/door\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"4 0.5 4\"\n"
		"\t\"move_offset\" \"0 2 0\"\n"
		"\t\"speed\" \"1\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"rider\"\n"
		"\t\"origin\" \"0 0.25 0\"\n"
		"\t\"height\" \"1.7\"\n"
		"\t\"radius\" \"0.35\"\n}\n";
	asset_manager_t *assets;
	character_move_input_t input = {0};
	entity_t *platform_entity;
	func_door_t *door;
	material_t material = {0};
	mesh_t *mesh;
	player_t *player;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(func_door_register());
	CHECK(player_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	mesh = (mesh_t *)(void *)&mesh_marker;
	CHECK(asset_manager_register_mesh(assets, "models/door", mesh));
	CHECK(asset_manager_register_material(assets, "materials/door",
					      &material));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	platform_entity = world_find_by_targetname(world, "platform");
	door = func_door_from_entity(platform_entity);
	player = player_from_entity(world_find_by_targetname(world, "rider"));
	CHECK(door != NULL);
	CHECK(player != NULL);

	input.wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
	input.wish_speed = 0.0f;
	input.jump = false;
	input.crouch = false;
	player_move(player, &input, 0.05f);
	CHECK(player_is_grounded_on(player, platform_entity->id));

	CHECK(world_send_input(world, "platform", "Open", "", NULL, NULL) == 1);
	world_update(world, 0.5f);
	CHECK(!func_door_is_blocked(door));
	CHECK(platform_entity->transform.position.y > 0.49f);
	CHECK(player_get_position(player).y > 0.74f);
	CHECK(player_is_grounded_on(player, platform_entity->id));

	input.jump = true;
	player_move(player, &input, 0.01f);
	CHECK(!player_is_grounded_on(player, platform_entity->id));
	CHECK(player_get_velocity(player).y > 7.5f);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"door carries rider and passes velocity to jump",
		 test_door_carries_rider_and_passes_velocity_to_jump				    },
		{"door reverses after blocked",
		 test_door_reverses_after_blocked						 },
		{"door motion, inputs, and collision",
		 test_door_motion_inputs_and_collision},
		{"door stops and resumes after OnBlocked",
		 test_door_stops_and_resumes_after_blocked				  },
		{"door is blocked by player",	      test_door_is_blocked_by_player},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
