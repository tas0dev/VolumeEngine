/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "common.h"
#include "entity/func_button.h"
#include "entity/player.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include <math.h>

static unsigned char mesh_marker;

static bool test_raycast_uses_button_and_fires_output(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"player\"\n"
		"\t\"origin\" \"0 -0.85 0\"\n}\n"
		"entity\n{\n\t\"classname\" \"func_button\"\n"
		"\t\"targetname\" \"button\"\n"
		"\t\"model\" \"models/button\"\n"
		"\t\"material\" \"materials/button\"\n"
		"\t\"origin\" \"0 0 -2\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"starts_disabled\" \"1\"\n"
		"\t\"move_offset\" \"0 0 -0.2\"\n"
		"\t\"speed\" \"0.2\"\n"
		"\t\"wait\" \"1\"\n"
		"\t\"OnPressed\" \"player,Disable,,0,-1\"\n}\n";
	asset_manager_t *assets;
	collision_filter_t filter;
	collision_trace_t trace;
	func_button_t *button;
	entity_t *button_entity;
	entity_t *player;
	material_t material = {0};
	mesh_t *mesh;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(player_register());
	CHECK(func_button_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;

	CHECK(asset_manager_register_mesh(assets, "models/button", mesh));
	CHECK(asset_manager_register_material(assets, "materials/button",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));

	player = world_find_by_targetname(world, "player");
	button_entity = world_find_by_targetname(world, "button");
	button = func_button_from_entity(button_entity);

	CHECK(player != NULL);
	CHECK(button != NULL);
	CHECK(entity_is_active(button_entity));
	CHECK(!func_button_is_enabled(button));

	filter.layer = COLLISION_LAYER_PLAYER;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC;
	filter.ignored_entity_id = player->id;

	CHECK(collision_world_trace_ray_filtered(
		world_get_const_collision_world(world),
		vec3_create(0.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, -3.0f),
		filter, &trace));
	CHECK(trace.entity_id == button_entity->id);

	CHECK(!world_send_input_to_entity(world, button_entity, "Use", "",
					  player, player));

	CHECK(world_send_input_to_entity(world, button_entity, "Enable", "",
					 player, player));
	CHECK(func_button_is_enabled(button));

	CHECK(world_send_input_to_entity(world, button_entity, "Use", "",
					 player, player));
	CHECK(func_button_is_pressed(button));
	CHECK(func_button_get_state(button) ==
	      FUNC_BUTTON_PRESSING);
	CHECK(entity_is_active(player));

	world_update(world, 0.0f);

	CHECK(!entity_is_active(player));
	CHECK(func_button_get_state(button) ==
	      FUNC_BUTTON_PRESSING);

	world_update(world, 0.5f);

	CHECK(func_button_get_state(button) == FUNC_BUTTON_PRESSING);
	CHECK(fabsf(button_entity->transform.position.z + 2.1f) < 0.0001f);

	world_update(world, 0.5f);

	CHECK(func_button_get_state(button) == FUNC_BUTTON_PRESSED);
	CHECK(fabsf(button_entity->transform.position.z + 2.2f) < 0.0001f);

	world_update(world, 1.0f);

	CHECK(func_button_get_state(button) ==
	      FUNC_BUTTON_RELEASING);

	world_update(world, 1.0f);

	CHECK(!func_button_is_pressed(button));
	CHECK(func_button_get_state(button) ==
	      FUNC_BUTTON_IDLE);
	CHECK(fabsf(
		      button_entity->transform.position.z + 2.0f) < 0.0001f);

	CHECK(world_send_input_to_entity(world, button_entity, "Lock", "",
					 player, player));
	CHECK(world_send_input_to_entity(world, button_entity, "Use", "",
					 player, player));
	CHECK(!func_button_is_pressed(button));

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"raycast uses button and fires output",
		 test_raycast_uses_button_and_fires_output},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
