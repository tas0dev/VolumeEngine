/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/info_player_start.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include "math/math.h"
#include <math.h>

static bool test_player_start_loads_transform(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"info_player_start\"\n"
		"\t\"targetname\" \"spawn\"\n"
		"\t\"origin\" \"1 2 3\"\n"
		"\t\"angles\" \"10 90 0\"\n}\n";
	info_player_start_t *start;
	entity_t *entity;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(info_player_start_register());
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, NULL, error, sizeof(error)));
	entity = world_find_by_classname(world, "info_player_start");
	start = info_player_start_from_entity(entity);
	CHECK(start != NULL);
	CHECK(entity->transform.position.x == 1.0f);
	CHECK(entity->transform.position.y == 2.0f);
	CHECK(entity->transform.position.z == 3.0f);
	CHECK(fabsf(entity->transform.rotation.x - PI / 18.0f) < 0.0001f);
	CHECK(fabsf(entity->transform.rotation.y - PI / 2.0f) < 0.0001f);
	CHECK(!entity->has_collider);

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"player start loads transform",
		 test_player_start_loads_transform},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
