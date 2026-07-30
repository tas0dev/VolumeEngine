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
#include "map/map.h"
#include "map/spawn.h"
#include "weapon/hitscan_weapon.h"

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

	config.damage = 20.0f;
	config.range = 100.0f;
	config.fire_interval = 0.2f;
	config.magazine_size = 12;
	config.reserve_ammo = 48;
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
	hitscan_weapon_update(&weapon, config.fire_interval);
	CHECK(hitscan_weapon_start_reload(&weapon, 1.0f));
	CHECK(hitscan_weapon_is_busy(&weapon));
	CHECK(hitscan_weapon_is_reloading(&weapon));
	CHECK(hitscan_weapon_get_ammo(&weapon) == 10);
	hitscan_weapon_update(&weapon, 0.5f);
	CHECK(hitscan_weapon_get_ammo(&weapon) == 10);
	CHECK(hitscan_weapon_get_reserve_ammo(&weapon) == 48);
	CHECK(hitscan_weapon_get_action_time_remaining(&weapon) > 0.49f);
	CHECK(!hitscan_weapon_fire_timed(
		&weapon, world, NULL, vec3_create(0.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 0.0f, -1.0f), 0.6f, NULL));
	hitscan_weapon_update(&weapon, 0.5f);
	CHECK(!hitscan_weapon_is_reloading(&weapon));
	CHECK(hitscan_weapon_get_ammo(&weapon) == 12);
	CHECK(hitscan_weapon_get_reserve_ammo(&weapon) == 46);
	CHECK(hitscan_weapon_fire_timed(
		&weapon, world, NULL, vec3_create(0.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 0.0f, -1.0f), 0.6f, NULL));
	CHECK(hitscan_weapon_is_busy(&weapon));
	hitscan_weapon_update(&weapon, 0.2f);
	CHECK(hitscan_weapon_is_busy(&weapon));
	hitscan_weapon_update(&weapon, 0.41f);
	CHECK(!hitscan_weapon_is_busy(&weapon));
	world_update(world, 0.0f);
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();
	return true;
}

static bool test_hitscan_accuracy_uses_player_state_and_recovers(void) {
	hitscan_accuracy_config_t accuracy = {0};
	hitscan_accuracy_context_t context = {0};
	hitscan_shot_result_t first;
	hitscan_shot_result_t second;
	hitscan_weapon_config_t config;
	hitscan_weapon_t weapon_a;
	hitscan_weapon_t weapon_b;
	world_t *world;

	config.damage = 20.0f;
	config.range = 100.0f;
	config.fire_interval = 0.0f;
	config.magazine_size = 12;
	config.reserve_ammo = 0;
	accuracy.standing_inaccuracy = 0.01f;
	accuracy.moving_inaccuracy = 0.04f;
	accuracy.airborne_inaccuracy = 0.1f;
	accuracy.crouched_multiplier = 0.5f;
	accuracy.firing_penalty_per_shot = 0.02f;
	accuracy.maximum_firing_penalty = 0.06f;
	accuracy.penalty_recovery_delay = 0.2f;
	accuracy.penalty_recovery_rate = 0.1f;
	accuracy.reference_move_speed = 4.0f;
	accuracy.random_seed = 1234;
	CHECK(hitscan_weapon_initialize(&weapon_a, &config));
	CHECK(hitscan_weapon_initialize(&weapon_b, &config));
	CHECK(hitscan_weapon_set_accuracy(&weapon_a, &accuracy));
	CHECK(hitscan_weapon_set_accuracy(&weapon_b, &accuracy));
	world = world_create();
	CHECK(world != NULL);
	context.grounded = true;
	CHECK(hitscan_weapon_fire_accurate_timed(
		&weapon_a, world, NULL, vec3_create(0.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 0.0f, -1.0f), &context, 0.0f, &first));
	CHECK(first.inaccuracy == accuracy.standing_inaccuracy);
	CHECK(hitscan_weapon_get_firing_penalty(&weapon_a) == 0.02f);
	hitscan_weapon_update(&weapon_a, 0.1f);
	CHECK(hitscan_weapon_get_firing_penalty(&weapon_a) == 0.02f);
	hitscan_weapon_update(&weapon_a, 0.2f);
	CHECK(hitscan_weapon_get_firing_penalty(&weapon_a) < 0.0001f);
	context.grounded = false;
	CHECK(hitscan_weapon_fire_accurate_timed(
		&weapon_b, world, NULL, vec3_create(0.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 0.0f, -1.0f), &context, 0.0f, &second));
	CHECK(second.inaccuracy == accuracy.airborne_inaccuracy);
	CHECK(vec3_length(vec3_subtract(first.direction, second.direction)) >
	      0.001f);
	world_destroy(world);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"hitscan breaks prop and reloads",
		 test_hitscan_breaks_prop_and_reloads},
		{"hitscan accuracy uses player state and recovers",
		 test_hitscan_accuracy_uses_player_state_and_recovers},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
