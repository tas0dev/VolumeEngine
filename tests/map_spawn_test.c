/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "common.h"
#include "entity/entity.h"
#include "entity/properties.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include <stdlib.h>
#include <string.h>

typedef struct test_entity {
	entity_t entity;
	entity_properties_t properties;
	mesh_t *mesh;
	material_t *material;
	bool casts_shadow;
} test_entity_t;

static unsigned char mesh_marker;

static entity_t *create_test_entity(entity_id_t id,
				    const entity_spawn_context_t *context);
static void destroy_test_entity(entity_t *entity);

static const entity_class_t test_entity_class = {
	.classname = "test_entity",
	.create = create_test_entity,
	.update = NULL,
	.draw_shadow = NULL,
	.draw = NULL,
	.destroy = destroy_test_entity,
};

static entity_t *create_test_entity(const entity_id_t id,
				    const entity_spawn_context_t *context) {
	test_entity_t *entity;
	const char *model_path;
	const char *material_path;
	const char *casts_shadow;

	if (context == NULL || context->properties == NULL) { return NULL;
	}

	entity = calloc(1, sizeof(*entity));
	if (entity == NULL) { return NULL; }

	entity_initialize((entity_t *)entity, id, &test_entity_class);

	if (!entity_set_targetname((entity_t *)entity,
				   context->properties->targetname)) {
		entity_destroy((entity_t *)entity);
		return NULL;
	}

	entity->properties = *context->properties;
	entity->entity.transform = context->properties->transform;
	entity->casts_shadow = true;

	model_path = entity_property_get(context->source, "model");
	if (model_path != NULL) {
		entity->mesh = asset_manager_load_mesh(
			context->assets, model_path, context->error,
			context->error_size);

		if (entity->mesh == NULL) {
			entity_destroy((entity_t *)entity);
			return NULL;
		}
	}

	material_path = entity_property_get(context->source, "material");
	if (material_path != NULL) {
		entity->material = asset_manager_load_material(
			context->assets, material_path, context->error,
			context->error_size);

		if (entity->material == NULL) {
			entity_destroy((entity_t *)entity);
			return NULL;
		}
	}

	casts_shadow = entity_property_get(context->source, "casts_shadow");
	if (casts_shadow != NULL &&
	    !entity_property_parse_bool(casts_shadow, &entity->casts_shadow)) {
		entity_destroy((entity_t *)entity);
		return NULL;
	}

	return (entity_t *)entity;
}

static void destroy_test_entity(entity_t *entity) {
	free((test_entity_t *)entity);
}

static bool test_spawn_entities(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"test_entity\"\n"
				     "\t\"model\" \"models/test\"\n"
				     "\t\"material\" \"materials/test\"\n"
				     "\t\"origin\" \"1 2 3\"\n"
				     "\t\"angles\" \"0 90 0\"\n"
				     "\t\"scale\" \"2 3 4\"\n"
				     "\t\"casts_shadow\" \"0\"\n"
				     "\t\"targetname\" \"test_prop\"\n"
				     "}\n";
	asset_manager_t *assets;
	material_t material;
	test_entity_t *entity;
	world_t *world;
	map_t *map;
	mesh_t *mesh;
	char error[256];

	CHECK(entity_register_class(&test_entity_class));

	assets = asset_manager_create();
	CHECK(assets != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker;
	material = (material_t){0};

	CHECK(asset_manager_register_mesh(assets, "models/test", mesh));
	CHECK(asset_manager_register_material(assets, "materials/test",
					      &material));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 1);

	entity = (test_entity_t *)world_get_entity(world, 0);
	CHECK(entity != NULL);
	CHECK(entity->mesh == mesh);
	CHECK(entity->material == &material);
	CHECK(entity->entity.transform.position.x == 1.0f);
	CHECK(entity->entity.transform.position.y == 2.0f);
	CHECK(entity->entity.transform.position.z == 3.0f);
	CHECK(entity->entity.transform.rotation.y > 1.5707f);
	CHECK(entity->entity.transform.rotation.y < 1.5709f);
	CHECK(entity->entity.transform.scale.x == 2.0f);
	CHECK(entity->entity.transform.scale.y == 3.0f);
	CHECK(entity->entity.transform.scale.z == 4.0f);
	CHECK(!entity->casts_shadow);
	CHECK(strcmp(entity_get_targetname(&entity->entity), "test_prop") == 0);
	CHECK(world_find_by_targetname(world, "test_prop") == &entity->entity);
	CHECK(world_find_by_targetname(world, "missing") == NULL);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_missing_asset_rolls_back(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"test_entity\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"test_entity\"\n"
				     "\t\"model\" \"models/missing\"\n"
				     "}\n";
	asset_manager_t *assets;
	entity_properties_t properties;
	entity_spawn_context_t context = {0};
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(entity_register_class(&test_entity_class));

	assets = asset_manager_create();
	CHECK(assets != NULL);

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	properties = entity_properties_create();

	context.properties = &properties;
	context.error = error;
	context.error_size = sizeof(error);

	CHECK(world_spawn_entity(world, "test_entity", &context) != NULL);
	CHECK(world_get_entity_count(world) == 1);

	CHECK(!map_spawn_entities(map, world, assets, error, sizeof(error)));
	CHECK(strstr(error, "mesh asset not found") != NULL);
	CHECK(world_get_entity_count(world) == 1);

	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	entity_registry_shutdown();

	return true;
}

static bool test_invalid_transform_rolls_back(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"test_entity\"\n"
				     "\t\"origin\" \"1 2 3\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"test_entity\"\n"
				     "\t\"origin\" \"invalid\"\n"
				     "}\n";
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(entity_register_class(&test_entity_class));

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = world_create();
	CHECK(world != NULL);

	CHECK(!map_spawn_entities(map, world, NULL, error, sizeof(error)));
	CHECK(strstr(error, "invalid vec3 property") != NULL);
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"spawn entities",		   test_spawn_entities	      },
		{"missing asset rolls back",     test_missing_asset_rolls_back},
		{"invalid transform rolls back",
		 test_invalid_transform_rolls_back				  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}