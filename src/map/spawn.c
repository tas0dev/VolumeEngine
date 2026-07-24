/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "map/spawn.h"
#include "entity/io.h"
#include "entity/properties.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void set_error(char *error, size_t error_size, const char *format, ...);
static void rollback_entities(world_t *world, size_t initial_count);
static const char *get_map_property(const void *context, const char *key);
static bool load_entity_outputs(entity_t *entity,
				const map_entity_t *map_entity,
				char *error,
				size_t error_size);

static bool load_entity_parent(entity_t *entity,
			       const map_entity_t *map_entity,
			       world_t *world,
			       char *error,
			       const size_t error_size) {
	const char *parent_name;
	entity_t *parent;

	if (entity == NULL || map_entity == NULL || world == NULL) {
		set_error(error, error_size, "invalid entity parent context");
		return false;
	}

	parent_name = map_entity_get_property(map_entity, "parent");

	if (parent_name == NULL || parent_name[0] == '\0') { return true; }

	parent = world_find_by_targetname(world, parent_name);

	if (parent == NULL) {
		set_error(error, error_size,
			  "parent entity \"%s\" was not found", parent_name);
		return false;
	}

	if (!entity_set_parent(entity, parent)) {
		set_error(error, error_size,
			  "failed to set parent \"%s\" for entity \"%s\"",
			  parent_name,
			  entity_get_targetname(entity) == NULL
				  ? entity_get_classname(entity)
				  : entity_get_targetname(entity));
		return false;
	}

	return true;
}

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
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

static const char *get_map_property(const void *context, const char *key) {
	return map_entity_get_property(context, key);
}

static bool load_entity_outputs(entity_t *entity,
				const map_entity_t *map_entity,
				char *error,
				const size_t error_size) {
	const char *key;
	const char *value;
	size_t index;
	size_t count;

	count = map_entity_get_property_count(map_entity);

	for (index = 0; index < count; index++) {
		if (!map_entity_get_property_at(map_entity, index, &key,
						&value)) {
			set_error(error, error_size,
				  "invalid entity property at index %zu",
				  index);
			return false;
		}

		if (key == NULL || key[0] != 'O' || key[1] != 'n') { continue; }

		if (!entity_add_output_from_string(entity, key, value, error,
						   error_size)) {
			return false;
		}
	}

	return true;
}

bool map_spawn_entities(const map_t *map,
			world_t *world,
			asset_manager_t *assets,
			char *error,
			const size_t error_size) {
	const map_entity_t *map_entity;
	const char *classname;
	entity_property_source_t source;
	entity_properties_t properties;
	entity_spawn_context_t context;
	size_t initial_count;
	size_t index;
	size_t count;
	entity_t *entity;

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

		source.context = map_entity;
		source.get = get_map_property;

		if (!entity_properties_load(&properties, &source, error,
					    error_size)) {
			rollback_entities(world, initial_count);
			return false;
		}

		context.properties = &properties;
		context.source = &source;
		context.assets = assets;
		context.error = error;
		context.error_size = error_size;

		entity =
			world_spawn_entity_deferred(world, classname, &context);

		if (entity == NULL) {
			if (error == NULL || error_size == 0 ||
			    error[0] == '\0') {
				set_error(error, error_size,
					  "failed to spawn entity %zu "
					  "with classname \"%s\"",
					  index, classname);
			}

			rollback_entities(world, initial_count);
			return false;
		}
	}

	for (index = 0; index < count; index++) {
		map_entity = map_get_entity(map, index);
		entity = world_get_entity(world, initial_count + index);

		if (map_entity == NULL || entity == NULL) {
			set_error(error, error_size,
				  "failed to resolve entity %zu", index);
			rollback_entities(world, initial_count);
			return false;
		}

		if (!load_entity_parent(entity, map_entity, world, error,
					error_size)) {
			rollback_entities(world, initial_count);
			return false;
		}
	}

	for (index = 0; index < count; index++) {
		map_entity = map_get_entity(map, index);
		entity = world_get_entity(world, initial_count + index);

		if (map_entity == NULL || entity == NULL ||
		    !load_entity_outputs(entity, map_entity, error,
					 error_size)) {
			if ((error == NULL || error_size == 0 ||
			     error[0] == '\0') &&
			    (map_entity == NULL || entity == NULL)) {
				set_error(error, error_size,
					  "failed to load references for "
					  "entity %zu",
					  index);
			}

			rollback_entities(world, initial_count);
			return false;
		}
	}

	for (index = 0; index < count; index++) {
		entity = world_get_entity(world, initial_count + index);
		entity_activate(entity);
	}

	return true;
}

bool world_load_map(world_t *world,
		    asset_manager_t *assets,
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
