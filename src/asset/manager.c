/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/manager.h"
#include "asset/material_loader.h"
#include "asset/mesh_loader.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct asset_entry {
	char *path;
	void *asset;
	bool owned;
} asset_entry_t;

typedef struct asset_table {
	asset_entry_t *entries;
	size_t count;
	size_t capacity;
} asset_table_t;

struct asset_manager {
	asset_table_t meshes;
	asset_table_t materials;
	char *root_path;
};

typedef void (*asset_destroy_function_t)(void *asset);

static void set_error(char *error, size_t error_size, const char *format, ...);
static char *build_asset_path(const char *root_path, const char *path);
static void destroy_mesh_asset(void *asset);
static void asset_table_destroy(asset_table_t *table,
				asset_destroy_function_t destroy);
static bool asset_table_register(asset_table_t *table,
				 const char *path,
				 void *asset,
				 bool owned);
static void *asset_table_find(const asset_table_t *table, const char *path);
static void destroy_material_asset(void *asset);

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	length = strlen(string);

	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }

	memcpy(copy, string, length + 1);

	return copy;
}

static void asset_table_destroy(asset_table_t *table,
				asset_destroy_function_t destroy) {
	size_t index;

	if (table == NULL) { return; }

	for (index = 0; index < table->count; index++) {
		if (table->entries[index].owned && destroy != NULL) {
			destroy(table->entries[index].asset);
		}

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

static bool asset_table_register(asset_table_t *table,
				 const char *path,
				 void *asset,
				 const bool owned) {
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
	table->entries[table->count].owned = owned;
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

asset_manager_t *asset_manager_create_at(const char *root_path) {
	asset_manager_t *manager;

	if (root_path == NULL || root_path[0] == '\0') { return NULL; }

	manager = asset_manager_create();
	if (manager == NULL) { return NULL; }

	manager->root_path = duplicate_string(root_path);
	if (manager->root_path == NULL) {
		asset_manager_destroy(manager);
		return NULL;
	}

	return manager;
}

void asset_manager_destroy(asset_manager_t *manager) {
	if (manager == NULL) { return; }

	asset_table_destroy(&manager->materials, destroy_material_asset);
	asset_table_destroy(&manager->meshes, destroy_mesh_asset);

	free(manager->root_path);
	free(manager);
}

bool asset_manager_register_mesh(asset_manager_t *manager,
				 const char *path,
				 mesh_t *mesh) {
	if (manager == NULL) { return false; }

	return asset_table_register(&manager->meshes, path, mesh, false);
}

bool asset_manager_register_material(asset_manager_t *manager,
				     const char *path,
				     material_t *material) {
	if (manager == NULL) { return false; }

	return asset_table_register(&manager->materials, path, material, false);
}

mesh_t *asset_manager_load_mesh(asset_manager_t *manager,
				const char *path,
				char *error,
				const size_t error_size) {
	mesh_t *mesh;
	char *full_path;
	char loader_error[512];

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (manager == NULL || path == NULL || path[0] == '\0') {
		set_error(error, error_size,
			  "invalid asset manager or mesh path");
		return NULL;
	}

	mesh = asset_manager_get_mesh(manager, path);
	if (mesh != NULL) { return mesh; }

	if (manager->root_path == NULL) {
		set_error(error, error_size,
			  "mesh asset not found: \"%s\" "
			  "(asset root is not configured)",
			  path);
		return NULL;
	}

	full_path = build_asset_path(manager->root_path, path);
	if (full_path == NULL) {
		set_error(error, error_size,
			  "failed to build mesh asset path: \"%s\"", path);
		return NULL;
	}

	mesh = mesh_load(full_path, loader_error, sizeof(loader_error));

	free(full_path);

	if (mesh == NULL) {
		set_error(error, error_size,
			  "mesh asset not found or could not be loaded: "
			  "\"%s\": %s",
			  path, loader_error);
		return NULL;
	}

	if (!asset_table_register(&manager->meshes, path, mesh, true)) {
		mesh_destroy(mesh);

		set_error(error, error_size,
			  "failed to cache mesh asset: \"%s\"", path);
		return NULL;
	}

	return mesh;
}

material_t *asset_manager_load_material(asset_manager_t *manager,
					const char *path,
					char *error,
					const size_t error_size) {
	material_t *material;
	char *full_path;
	char loader_error[512];

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (manager == NULL || path == NULL || path[0] == '\0') {
		set_error(error, error_size,
			  "invalid asset manager or material path");
		return NULL;
	}

	material = asset_manager_get_material(manager, path);
	if (material != NULL) { return material; }

	if (manager->root_path == NULL) {
		set_error(error, error_size,
			  "material asset not found: \"%s\" "
			  "(asset root is not configured)",
			  path);
		return NULL;
	}

	full_path = build_asset_path(manager->root_path, path);
	if (full_path == NULL) {
		set_error(error, error_size,
			  "failed to build material path: \"%s\"", path);
		return NULL;
	}

	material = malloc(sizeof(*material));
	if (material == NULL) {
		free(full_path);

		set_error(error, error_size,
			  "failed to allocate material: \"%s\"", path);
		return NULL;
	}

	if (!material_load(full_path, material, loader_error,
			   sizeof(loader_error))) {
		free(material);
		free(full_path);

		set_error(error, error_size,
			  "failed to load material \"%s\": %s", path,
			  loader_error);
		return NULL;
	}

	free(full_path);

	if (!asset_table_register(&manager->materials, path, material, true)) {
		free(material);

		set_error(error, error_size,
			  "failed to cache material asset: \"%s\"", path);
		return NULL;
	}

	return material;
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

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static char *build_asset_path(const char *root_path, const char *path) {
	char *result;
	size_t root_length;
	size_t path_length;
	size_t separator_length;
	size_t total_length;

	if (path[0] == '/') { return duplicate_string(path); }

	root_length = strlen(root_path);
	path_length = strlen(path);

	separator_length =
		root_length > 0 && root_path[root_length - 1] != '/' ? 1 : 0;

	if (root_length > SIZE_MAX - path_length ||
	    root_length + path_length > SIZE_MAX - separator_length - 1) {
		return NULL;
	}

	total_length = root_length + separator_length + path_length;

	result = malloc(total_length + 1);
	if (result == NULL) { return NULL; }

	memcpy(result, root_path, root_length);

	if (separator_length != 0) { result[root_length] = '/'; }

	memcpy(result + root_length + separator_length, path, path_length);

	result[total_length] = '\0';

	return result;
}

static void destroy_mesh_asset(void *asset) { mesh_destroy(asset); }

static void destroy_material_asset(void *asset) { free(asset); }