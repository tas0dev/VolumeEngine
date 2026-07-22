/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "collision/collider.h"
#include "collision/collision_world.h"
#include "common.h"
#include "entity/entity.h"
#include "entity/prop_dynamic.h"
#include "entity/prop_static.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"

static unsigned char mesh_marker;

static bool test_prop_static_box_collider(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"model\" \"models/floor\"\n"
				     "\t\"material\" \"materials/floor\"\n"
				     "\t\"origin\" \"0 -1 0\"\n"
				     "\t\"collision\" \"box\"\n"
				     "\t\"collision_size\" \"6 0.2 6\"\n"
				     "}\n";
	asset_manager_t *assets;
	collision_world_t *collision_world;
	collider_t collider;
	material_t material;
	entity_t *entity;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	char error[256];

	CHECK(prop_static_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/floor", mesh));
	CHECK(asset_manager_register_material(assets, "materials/floor",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 1);

	entity = world_get_entity(world, 0);
	CHECK(entity != NULL);
	CHECK(entity->has_collider);
	CHECK(entity_get_collider(entity, &collider));
	CHECK(collider.type == COLLIDER_TYPE_BOX);

	CHECK(collider.shape.box.center.x == 0.0f);
	CHECK(collider.shape.box.center.y == 0.0f);
	CHECK(collider.shape.box.center.z == 0.0f);
	CHECK(collider.shape.box.half_extents.x == 3.0f);
	CHECK(collider.shape.box.half_extents.y == 0.1f);
	CHECK(collider.shape.box.half_extents.z == 3.0f);

	collision_world = world_get_collision_world(world);
	CHECK(collision_world != NULL);
	CHECK(collision_world_get_count(collision_world) == 1);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_prop_static_without_collider(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"model\" \"models/prop\"\n"
				     "\t\"material\" \"materials/prop\"\n"
				     "}\n";
	asset_manager_t *assets;
	collision_world_t *collision_world;
	material_t material;
	entity_t *entity;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	char error[256];

	CHECK(prop_static_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/prop", mesh));
	CHECK(asset_manager_register_material(assets, "materials/prop",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 1);

	entity = world_get_entity(world, 0);
	CHECK(entity != NULL);
	CHECK(!entity->has_collider);
	CHECK(!entity_get_collider(entity, NULL));

	collision_world = world_get_collision_world(world);
	CHECK(collision_world != NULL);
	CHECK(collision_world_get_count(collision_world) == 0);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_invalid_collision_type(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"model\" \"models/prop\"\n"
				     "\t\"material\" \"materials/prop\"\n"
				     "\t\"collision\" \"sphere\"\n"
				     "}\n";
	asset_manager_t *assets;
	material_t material;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	char error[256];

	CHECK(prop_static_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/prop", mesh));
	CHECK(asset_manager_register_material(assets, "materials/prop",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(!map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_invalid_collision_size(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"model\" \"models/prop\"\n"
				     "\t\"material\" \"materials/prop\"\n"
				     "\t\"collision\" \"box\"\n"
				     "\t\"collision_size\" \"6 0 6\"\n"
				     "}\n";
	asset_manager_t *assets;
	material_t material;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	char error[256];

	CHECK(prop_static_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/prop", mesh));
	CHECK(asset_manager_register_material(assets, "materials/prop",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(!map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_prop_dynamic_collision_follows_transform(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_dynamic\"\n"
				     "\t\"model\" \"models/prop\"\n"
				     "\t\"material\" \"materials/prop\"\n"
				     "\t\"collision\" \"box\"\n"
				     "\t\"collision_size\" \"2 2 2\"\n"
				     "}\n";
	asset_manager_t *assets;
	collision_result_t result;
	material_t material;
	entity_t *entity;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	aabb_t bounds;
	vec3_t position;
	char error[256];

	CHECK(prop_dynamic_register());

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/prop", mesh));
	CHECK(asset_manager_register_material(assets, "materials/prop",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));

	entity = world_get_entity(world, 0);
	CHECK(prop_dynamic_from_entity(entity) != NULL);
	CHECK(entity->collider_follows_transform);

	bounds = aabb_create(vec3_create(0.0f, 0.0f, 0.0f),
			     vec3_create(0.25f, 0.25f, 0.25f));
	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(collision_world_resolve_aabb(world_get_collision_world(world),
					   bounds, &position, &result));

	entity->transform.position = vec3_create(5.0f, 0.0f, 0.0f);
	world_update(world, 0.0f);

	position = vec3_create(0.0f, 0.0f, 0.0f);
	CHECK(!collision_world_resolve_aabb(world_get_collision_world(world),
					    bounds, &position, &result));

	position = vec3_create(5.0f, 0.0f, 0.0f);
	CHECK(collision_world_resolve_aabb(world_get_collision_world(world),
					   bounds, &position, &result));

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"prop_static box collider",	     test_prop_static_box_collider},
		{"prop_static without collider",
		 test_prop_static_without_collider				  },
		{"invalid prop_static collision type",
		 test_invalid_collision_type					    },
		{"invalid prop_static collision size",
		 test_invalid_collision_size					    },
		{"prop_dynamic collision follows transform",
		 test_prop_dynamic_collision_follows_transform			      },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
