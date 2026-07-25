/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "common.h"
#include "entity/prop_breakable.h"
#include "entity/world.h"
#include "game/hitscan_weapon.h"
#include "map/map.h"
#include "map/spawn.h"

static unsigned char mesh_marker;

static bool test_hitscan_breaks_prop_and_reloads(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"prop_breakable\"\n"
		"\t\"targetname\" \"target\"\n"
		"\t\"model\" \"models/target\"\n"
		"\t\"material\" \"materials/target\"\n"
		"\t\"origin\" \"0 0 -5\"\n"
		"\t\"collision\" \"box\"\n"
		"\t\"collision_size\" \"1 1 1\"\n"
		"\t\"health\" \"40\"\n}\n";
	asset_manager_t *assets;
	collision_trace_t trace;
	hitscan_weapon_config_t config;
	hitscan_weapon_t weapon;
	material_t material = {0};
	prop_breakable_t *prop;
	entity_t *target;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(prop_breakable_register());
	assets = asset_manager_create();
	CHECK(assets != NULL);
	CHECK(asset_manager_register_mesh(assets, "models/target",
					  (mesh_t *)(void *)&mesh_marker));
	CHECK(asset_manager_register_material(assets, "materials/target",
					      &material));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	target = world_find_by_targetname(world, "target");
	prop = prop_breakable_from_entity(target);
	CHECK(prop != NULL);

	config = hitscan_weapon_config_create();
	CHECK(hitscan_weapon_initialize(&weapon, &config));
	CHECK(hitscan_weapon_fire(&weapon, world, NULL,
				  vec3_create(0.0f, 0.0f, 0.0f),
				  vec3_create(0.0f, 0.0f, -1.0f), &trace));
	CHECK(trace.hit);
	CHECK(trace.entity_id == target->id);
	CHECK(prop_breakable_get_health(prop) == 20.0f);
	CHECK(hitscan_weapon_get_ammo(&weapon) == 11);
	CHECK(!hitscan_weapon_fire(&weapon, world, NULL,
				   vec3_create(0.0f, 0.0f, 0.0f),
				   vec3_create(0.0f, 0.0f, -1.0f), NULL));

	hitscan_weapon_update(&weapon, config.fire_interval);
	CHECK(hitscan_weapon_fire(&weapon, world, NULL,
				  vec3_create(0.0f, 0.0f, 0.0f),
				  vec3_create(0.0f, 0.0f, -1.0f), NULL));
	CHECK(target->pending_destroy);
	CHECK(hitscan_weapon_get_ammo(&weapon) == 10);
	CHECK(hitscan_weapon_reload(&weapon));
	CHECK(hitscan_weapon_get_ammo(&weapon) == 12);
	CHECK(hitscan_weapon_get_reserve_ammo(&weapon) == 46);
	world_update(world, 0.0f);
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"hitscan breaks prop and reloads",
		 test_hitscan_breaks_prop_and_reloads},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
