/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "map/map.h"
#include "map/keyvalues.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct map_entity {
	const keyvalues_node_t *node;
};

struct map {
	keyvalues_document_t *document;
	map_entity_t world;
	map_entity_t *entities;
	size_t entity_count;
	size_t entity_capacity;
	bool has_world;
};

static void set_error(char *error, size_t error_size, const char *format, ...);
static bool reserve_entities(map_t *map, size_t capacity);
static bool validate_block(const keyvalues_node_t *node,
			   const char *block_name,
			   char *error,
			   size_t error_size);
static map_t *
build_map(keyvalues_document_t *document, char *error, size_t error_size);

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static bool reserve_entities(map_t *map, const size_t capacity) {
	map_entity_t *entities;

	if (capacity <= map->entity_capacity) { return true; }

	if (capacity > SIZE_MAX / sizeof(*map->entities)) { return false; }

	entities = realloc(map->entities, capacity * sizeof(*map->entities));
	if (entities == NULL) { return false; }

	map->entities = entities;
	map->entity_capacity = capacity;

	return true;
}

static bool validate_block(const keyvalues_node_t *node,
			   const char *block_name,
			   char *error,
			   const size_t error_size) {
	const keyvalues_node_t *property;
	const char *key;
	size_t index;
	size_t count;

	count = keyvalues_node_get_child_count(node);

	for (index = 0; index < count; index++) {
		property = keyvalues_node_get_child(node, index);
		if (property == NULL) {
			set_error(error, error_size,
				  "invalid property in %s block", block_name);
			return false;
		}

		key = keyvalues_node_get_key(property);
		if (key == NULL) {
			set_error(error, error_size,
				  "property without key in %s block",
				  block_name);
			return false;
		}

		if (keyvalues_node_is_block(property)) {
			set_error(error, error_size,
				  "nested block \"%s\" is not supported in %s",
				  key, block_name);
			return false;
		}
	}

	return true;
}

static map_t *build_map(keyvalues_document_t *document,
			char *error,
			const size_t error_size) {
	map_t *map;
	const keyvalues_node_t *root;
	const keyvalues_node_t *node;
	const char *block_name;
	const char *classname;
	size_t index;
	size_t count;
	size_t capacity;

	if (document == NULL) { return NULL; }

	map = calloc(1, sizeof(*map));
	if (map == NULL) {
		set_error(error, error_size, "out of memory");
		keyvalues_destroy(document);
		return NULL;
	}

	map->document = document;
	root = keyvalues_get_root(document);
	count = keyvalues_node_get_child_count(root);

	for (index = 0; index < count; index++) {
		node = keyvalues_node_get_child(root, index);
		if (node == NULL || !keyvalues_node_is_block(node)) {
			set_error(error, error_size,
				  "top-level entries must be blocks");
			map_destroy(map);
			return NULL;
		}

		block_name = keyvalues_node_get_key(node);
		if (block_name == NULL) {
			set_error(error, error_size,
				  "top-level block has no name");
			map_destroy(map);
			return NULL;
		}

		if (!validate_block(node, block_name, error, error_size)) {
			map_destroy(map);
			return NULL;
		}

		if (strcmp(block_name, "world") == 0) {
			if (map->has_world) {
				set_error(error, error_size,
					  "multiple world blocks are not "
					  "allowed");
				map_destroy(map);
				return NULL;
			}

			map->world.node = node;
			map->has_world = true;
			continue;
		}

		if (strcmp(block_name, "entity") == 0) {
			classname = map_entity_get_property(
				&(map_entity_t){.node = node}, "classname");
			if (classname == NULL || classname[0] == '\0') {
				set_error(error, error_size,
					  "entity block has no classname");
				map_destroy(map);
				return NULL;
			}

			if (map->entity_count == map->entity_capacity) {
				capacity = map->entity_capacity == 0
						   ? 16
						   : map->entity_capacity * 2;

				if (capacity < map->entity_capacity ||
				    !reserve_entities(map, capacity)) {
					set_error(error, error_size,
						  "out of memory");
					map_destroy(map);
					return NULL;
				}
			}

			map->entities[map->entity_count].node = node;
			map->entity_count++;
			continue;
		}

		set_error(error, error_size,
			  "unsupported top-level block \"%s\"", block_name);
		map_destroy(map);
		return NULL;
	}

	if (!map->has_world) {
		set_error(error, error_size, "world block is required");
		map_destroy(map);
		return NULL;
	}

	classname = map_entity_get_property(&map->world, "classname");
	if (classname == NULL || strcmp(classname, "worldspawn") != 0) {
		set_error(error, error_size,
			  "world classname must be \"worldspawn\"");
		map_destroy(map);
		return NULL;
	}

	return map;
}

map_t *map_parse(const char *source, char *error, const size_t error_size) {
	keyvalues_document_t *document;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	document = keyvalues_parse(source, error, error_size);
	if (document == NULL) { return NULL; }

	return build_map(document, error, error_size);
}

map_t *map_load(const char *path, char *error, const size_t error_size) {
	keyvalues_document_t *document;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	document = keyvalues_load(path, error, error_size);
	if (document == NULL) { return NULL; }

	return build_map(document, error, error_size);
}

void map_destroy(map_t *map) {
	if (map == NULL) { return; }

	free(map->entities);
	keyvalues_destroy(map->document);
	free(map);
}

const map_entity_t *map_get_world(const map_t *map) {
	if (map == NULL || !map->has_world) { return NULL; }

	return &map->world;
}

size_t map_get_entity_count(const map_t *map) {
	if (map == NULL) { return 0; }

	return map->entity_count;
}

const map_entity_t *map_get_entity(const map_t *map, const size_t index) {
	if (map == NULL || index >= map->entity_count) { return NULL; }

	return &map->entities[index];
}

const char *map_entity_get_property(const map_entity_t *entity,
				    const char *key) {
	const keyvalues_node_t *property;

	if (entity == NULL || entity->node == NULL || key == NULL) {
		return NULL;
	}

	property = keyvalues_node_find_child(entity->node, key);
	if (property == NULL || keyvalues_node_is_block(property)) {
		return NULL;
	}

	return keyvalues_node_get_value(property);
}