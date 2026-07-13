/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ASSET_MANAGER_H
#define VOLUME_ASSET_MANAGER_H

#include "renderer/material.h"
#include "renderer/mesh.h"
#include <stdbool.h>

typedef struct asset_manager asset_manager_t;

asset_manager_t *asset_manager_create(void);
void asset_manager_destroy(asset_manager_t *manager);
bool asset_manager_register_mesh(asset_manager_t *manager,
				 const char *path,
				 mesh_t *mesh);
bool asset_manager_register_material(asset_manager_t *manager,
				     const char *path,
				     material_t *material);
mesh_t *asset_manager_get_mesh(const asset_manager_t *manager,
			       const char *path);
material_t *asset_manager_get_material(const asset_manager_t *manager,
				       const char *path);

#endif