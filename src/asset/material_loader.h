/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ASSET_MATERIAL_LOADER_H
#define VOLUME_ASSET_MATERIAL_LOADER_H

#include "renderer/material.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct material_definition {
	material_t material;
	char *albedo_texture_path;
	char *normal_texture_path;
} material_definition_t;

bool material_definition_parse(const char *source,
			       material_definition_t *definition,
			       char *error,
			       size_t error_size);
bool material_definition_load(const char *path,
			      material_definition_t *definition,
			      char *error,
			      size_t error_size);
void material_definition_destroy(material_definition_t *definition);
bool material_parse(const char *source,
		    material_t *material,
		    char *error,
		    size_t error_size);
bool material_load(const char *path,
		   material_t *material,
		   char *error,
		   size_t error_size);

#endif