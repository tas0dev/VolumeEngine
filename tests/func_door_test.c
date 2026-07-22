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
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"

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

int main(void) {
	static const test_case_t tests[] = {
		{"door motion, inputs, and collision",
		 test_door_motion_inputs_and_collision},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
