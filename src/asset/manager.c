/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/manager.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct asset_entry {
	char *path;
	void *asset;
} asset_entry_t;

typedef struct asset_table {
	asset_entry_t *entries;
	size_t count;
	size_t capacity;
} asset_table_t;

struct asset_manager {
	asset_table_t meshes;
	asset_table_t materials;
};

static char *duplicate_string(const char *string);
static void asset_table_destroy(asset_table_t *table);
static bool asset_table_reserve(asset_table_t *table, size_t capacity);
static bool
asset_table_register(asset_table_t *table, const char *path, void *asset);
static void *asset_table_find(const asset_table_t *table, const char *path);

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	length = strlen(string);

	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }

	memcpy(copy, string, length + 1);

	return copy;
}

static void asset_table_destroy(asset_table_t *table) {
	size_t index;

	if (table == NULL) { return; }

	for (index = 0; index < table->count; index++) {
		free(table->entries[index].path);
	}

	free(table->entries);

	table->entries = NULL;
	table->count = 0;
	table->capacity = 0;
}

static bool asset_table_reserve(asset_table_t *table, const size_t capacity) {
	asset_entry_t *entries;

	if (capacity <= table->capacity) { return true; }

	if (capacity > SIZE_MAX / sizeof(*table->entries)) { return false; }

	entries = realloc(table->entries, capacity * sizeof(*table->entries));
	if (entries == NULL) { return false; }

	table->entries = entries;
	table->capacity = capacity;

	return true;
}

static bool
asset_table_register(asset_table_t *table, const char *path, void *asset) {
	char *path_copy;
	size_t capacity;

	if (table == NULL || path == NULL || path[0] == '\0' || asset == NULL) {
		return false;
	}

	if (asset_table_find(table, path) != NULL) { return false; }

	if (table->count == table->capacity) {
		capacity = table->capacity == 0 ? 16 : table->capacity * 2;

		if (capacity < table->capacity ||
		    !asset_table_reserve(table, capacity)) {
			return false;
		}
	}

	path_copy = duplicate_string(path);
	if (path_copy == NULL) { return false; }

	table->entries[table->count].path = path_copy;
	table->entries[table->count].asset = asset;
	table->count++;

	return true;
}

static void *asset_table_find(const asset_table_t *table, const char *path) {
	size_t index;

	if (table == NULL || path == NULL) { return NULL; }

	for (index = 0; index < table->count; index++) {
		if (strcmp(table->entries[index].path, path) == 0) {
			return table->entries[index].asset;
		}
	}

	return NULL;
}

asset_manager_t *asset_manager_create(void) {
	return calloc(1, sizeof(asset_manager_t));
}

void asset_manager_destroy(asset_manager_t *manager) {
	if (manager == NULL) { return; }

	asset_table_destroy(&manager->materials);
	asset_table_destroy(&manager->meshes);
	free(manager);
}

bool asset_manager_register_mesh(asset_manager_t *manager,
				 const char *path,
				 mesh_t *mesh) {
	if (manager == NULL) { return false; }

	return asset_table_register(&manager->meshes, path, mesh);
}

bool asset_manager_register_material(asset_manager_t *manager,
				     const char *path,
				     material_t *material) {
	if (manager == NULL) { return false; }

	return asset_table_register(&manager->materials, path, material);
}

mesh_t *asset_manager_get_mesh(const asset_manager_t *manager,
			       const char *path) {
	if (manager == NULL) { return NULL; }

	return asset_table_find(&manager->meshes, path);
}

material_t *asset_manager_get_material(const asset_manager_t *manager,
				       const char *path) {
	if (manager == NULL) { return NULL; }

	return asset_table_find(&manager->materials, path);
}
