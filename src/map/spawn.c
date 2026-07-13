/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "map/spawn.h"
#include "entity/properties.h"
#include "math/math.h"
#include <stdarg.h>
#include <stdio.h>

static void set_error(char *error, size_t error_size, const char *format, ...);
static bool read_vec3_property(const map_entity_t *entity,
			       const char *key,
			       vec3_t *value,
			       char *error,
			       size_t error_size);
static bool read_bool_property(const map_entity_t *entity,
			       const char *key,
			       bool *value,
			       char *error,
			       size_t error_size);
static void convert_angles(vec3_t *angles);
static bool build_properties(const map_entity_t *entity,
			     const asset_manager_t *assets,
			     entity_properties_t *properties,
			     char *error,
			     size_t error_size);
static void rollback_entities(world_t *world, size_t initial_count);

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static bool read_vec3_property(const map_entity_t *entity,
			       const char *key,
			       vec3_t *value,
			       char *error,
			       const size_t error_size) {
	const char *text;

	text = map_entity_get_property(entity, key);
	if (text == NULL) { return true; }

	if (!map_entity_get_vec3(entity, key, value)) {
		set_error(error, error_size,
			  "invalid vec3 property \"%s\": \"%s\"", key, text);
		return false;
	}

	return true;
}

static bool read_bool_property(const map_entity_t *entity,
			       const char *key,
			       bool *value,
			       char *error,
			       const size_t error_size) {
	const char *text;

	text = map_entity_get_property(entity, key);
	if (text == NULL) { return true; }

	if (!map_entity_get_bool(entity, key, value)) {
		set_error(error, error_size,
			  "invalid boolean property \"%s\": \"%s\"", key, text);
		return false;
	}

	return true;
}

static void convert_angles(vec3_t *angles) {
	const float degrees_to_radians = PI / 180.0f;

	angles->x *= degrees_to_radians;
	angles->y *= degrees_to_radians;
	angles->z *= degrees_to_radians;
}

static bool build_properties(const map_entity_t *entity,
			     const asset_manager_t *assets,
			     entity_properties_t *properties,
			     char *error,
			     const size_t error_size) {
	const char *model_path;
	const char *material_path;

	*properties = entity_properties_create();

	properties->targetname = map_entity_get_property(entity, "targetname");

	if (!read_vec3_property(entity, "origin",
				&properties->transform.position, error,
				error_size)) {
		return false;
	}

	if (!read_vec3_property(entity, "angles",
				&properties->transform.rotation, error,
				error_size)) {
		return false;
	}

	convert_angles(&properties->transform.rotation);

	if (!read_vec3_property(entity, "scale", &properties->transform.scale,
				error, error_size)) {
		return false;
	}

	if (!read_bool_property(entity, "casts_shadow",
				&properties->casts_shadow, error, error_size)) {
		return false;
	}

	model_path = map_entity_get_property(entity, "model");
	if (model_path != NULL) {
		if (assets == NULL) {
			set_error(error, error_size,
				  "model \"%s\" requires an asset manager",
				  model_path);
			return false;
		}

		properties->mesh = asset_manager_get_mesh(assets, model_path);
		if (properties->mesh == NULL) {
			set_error(error, error_size,
				  "mesh asset not found: \"%s\"", model_path);
			return false;
		}
	}

	material_path = map_entity_get_property(entity, "material");
	if (material_path != NULL) {
		if (assets == NULL) {
			set_error(error, error_size,
				  "material \"%s\" requires an asset manager",
				  material_path);
			return false;
		}

		properties->material =
			asset_manager_get_material(assets, material_path);
		if (properties->material == NULL) {
			set_error(error, error_size,
				  "material asset not found: \"%s\"",
				  material_path);
			return false;
		}
	}

	return true;
}

static void rollback_entities(world_t *world, const size_t initial_count) {
	entity_t *entity;
	entity_id_t id;
	size_t count;

	while (world_get_entity_count(world) > initial_count) {
		count = world_get_entity_count(world);
		entity = world_get_entity(world, count - 1);

		if (entity == NULL) { return; }

		id = entity->id;

		if (!world_remove_entity(world, id)) { return; }
	}
}

bool map_spawn_entities(const map_t *map,
			world_t *world,
			const asset_manager_t *assets,
			char *error,
			const size_t error_size) {
	const map_entity_t *map_entity;
	const char *classname;
	entity_properties_t properties;
	size_t initial_count;
	size_t index;
	size_t count;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (map == NULL || world == NULL) {
		set_error(error, error_size, "invalid map or world");
		return false;
	}

	initial_count = world_get_entity_count(world);
	count = map_get_entity_count(map);

	for (index = 0; index < count; index++) {
		map_entity = map_get_entity(map, index);
		if (map_entity == NULL) {
			set_error(error, error_size,
				  "invalid entity at index %zu", index);
			rollback_entities(world, initial_count);
			return false;
		}

		classname = map_entity_get_property(map_entity, "classname");
		if (classname == NULL) {
			set_error(error, error_size,
				  "entity %zu has no classname", index);
			rollback_entities(world, initial_count);
			return false;
		}

		if (!build_properties(map_entity, assets, &properties, error,
				      error_size)) {
			rollback_entities(world, initial_count);
			return false;
		}

		if (world_spawn_entity(world, classname, &properties) == NULL) {
			set_error(error, error_size,
				  "failed to spawn entity %zu with classname "
				  "\"%s\"",
				  index, classname);
			rollback_entities(world, initial_count);
			return false;
		}
	}

	return true;
}

bool world_load_map(world_t *world,
		    const asset_manager_t *assets,
		    const char *path,
		    char *error,
		    const size_t error_size) {
	map_t *map;
	bool result;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (world == NULL || path == NULL) {
		set_error(error, error_size, "invalid world or map path");
		return false;
	}

	map = map_load(path, error, error_size);
	if (map == NULL) { return false; }

	result = map_spawn_entities(map, world, assets, error, error_size);

	map_destroy(map);

	return result;
}