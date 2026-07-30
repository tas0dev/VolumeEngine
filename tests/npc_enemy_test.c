/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "common.h"
#include "entity/damage.h"
#include "entity/player.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include "npc_enemy.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char mesh_marker;

static world_t *create_test_world(const char *player_origin,
				  asset_manager_t **assets_out,
				  map_t **map_out) {
	char source[1024];
	char error[256];
	asset_manager_t *assets;
	material_t *material;
	map_t *map;
	world_t *world;

	material = calloc(1, sizeof(*material));
	if (material == NULL) { return NULL; }
	assets = asset_manager_create();
	if (assets == NULL ||
	    !asset_manager_register_mesh(assets, "models/enemy",
					 (mesh_t *)(void *)&mesh_marker) ||
	    !asset_manager_register_material(assets, "materials/enemy",
					     material)) {
		free(material);
		asset_manager_destroy(assets);
		return NULL;
	}
	snprintf(source, sizeof(source),
		 "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		 "entity\n{\n\t\"classname\" \"npc_enemy\"\n"
		 "\t\"targetname\" \"enemy\"\n"
		 "\t\"model\" \"models/enemy\"\n"
		 "\t\"material\" \"materials/enemy\"\n"
		 "\t\"origin\" \"0 0 0\"\n"
		 "\t\"collision\" \"none\"\n}\n"
		 "entity\n{\n\t\"classname\" \"player\"\n"
		 "\t\"targetname\" \"player\"\n"
		 "\t\"origin\" \"%s\"\n}\n",
		 player_origin);
	map = map_parse(source, error, sizeof(error));
	if (map == NULL) {
		free(material);
		asset_manager_destroy(assets);
		return NULL;
	}
	world = world_create();
	if (world == NULL ||
	    !map_spawn_entities(map, world, assets, error, sizeof(error))) {
		world_destroy(world);
		map_destroy(map);
		free(material);
		asset_manager_destroy(assets);
		return NULL;
	}
	*assets_out = assets;
	*map_out = map;
	return world;
}

static void
destroy_test_world(world_t *world, asset_manager_t *assets, map_t *map) {
	material_t *material;

	material = asset_manager_get_material(assets, "materials/enemy");
	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	free(material);
	entity_registry_shutdown();
}

static bool test_enemy_chases_visible_player(void) {
	asset_manager_t *assets;
	sandbox_npc_enemy_t *enemy;
	entity_t *enemy_entity;
	map_t *map;
	world_t *world;
	float start_z;
	size_t tick;

	CHECK(player_register());
	CHECK(sandbox_npc_enemy_register());
	world = create_test_world("0 0 4", &assets, &map);
	CHECK(world != NULL);
	enemy_entity = world_find_by_targetname(world, "enemy");
	enemy = sandbox_npc_enemy_from_entity(enemy_entity);
	CHECK(enemy != NULL);
	start_z = enemy_entity->transform.position.z;
	for (tick = 0; tick < 10; tick++) {
		world_update(world, 0.1f);
	}
	CHECK(sandbox_npc_enemy_get_state(enemy) == SANDBOX_NPC_ENEMY_CHASE);
	CHECK(enemy_entity->transform.position.z > start_z);

	destroy_test_world(world, assets, map);
	return true;
}

static bool test_enemy_attacks_and_can_die(void) {
	asset_manager_t *assets;
	damage_info_t damage = {0};
	sandbox_npc_enemy_t *enemy;
	entity_t *enemy_entity;
	player_t *player;
	map_t *map;
	world_t *world;

	CHECK(player_register());
	CHECK(sandbox_npc_enemy_register());
	world = create_test_world("0 0 1", &assets, &map);
	CHECK(world != NULL);
	enemy_entity = world_find_by_targetname(world, "enemy");
	enemy = sandbox_npc_enemy_from_entity(enemy_entity);
	player = player_from_entity(world_find_by_targetname(world, "player"));
	CHECK(enemy != NULL);
	CHECK(player != NULL);
	world_update(world, 0.1f);
	CHECK(sandbox_npc_enemy_get_state(enemy) == SANDBOX_NPC_ENEMY_ATTACK);
	CHECK(player_get_health(player) == 90.0f);

	damage.amount = 60.0f;
	damage.type = DAMAGE_TYPE_BULLET;
	damage.attacker = player_get_entity(player);
	CHECK(entity_take_damage(enemy_entity, &damage));
	CHECK(sandbox_npc_enemy_get_health(enemy) == 0.0f);
	CHECK(enemy_entity->pending_destroy);
	world_update(world, 0.0f);
	CHECK(world_find_by_targetname(world, "enemy") == NULL);

	destroy_test_world(world, assets, map);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"enemy chases a visible player",
		 test_enemy_chases_visible_player				 },
		{"enemy attacks and can die",     test_enemy_attacks_and_can_die},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
