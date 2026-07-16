/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ASSET_MESH_LOADER_H
#define VOLUME_ASSET_MESH_LOADER_H

#include "renderer/mesh.h"
#include <stddef.h>

mesh_t *mesh_load(const char *path, char *error, size_t error_size);

#endif
