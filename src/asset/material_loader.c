/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/material_loader.h"
#include "map/keyvalues.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_error(char *error, size_t error_size, const char *format, ...);
static const char *skip_whitespace(const char *text);
static bool parse_float_component(const char **text, float *value);
static bool parse_float_value(const char *text, float *value);
static bool parse_vec3_value(const char *text, vec3_t *value);
static bool parse_material_document(const keyvalues_document_t *document,
				    material_definition_t *definition,
				    char *error,
				    size_t error_size);

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static const char *skip_whitespace(const char *text) {
	while (*text != '\0' && isspace((unsigned char)*text)) {
		text++;
	}

	return text;
}

static bool parse_float_component(const char **text, float *value) {
	char *end;
	float result;

	if (*text == NULL) { return false; }

	*text = skip_whitespace(*text);
	if (**text == '\0') { return false; }

	errno = 0;
	result = strtof(*text, &end);

	if (end == *text || errno == ERANGE || !isfinite(result)) {
		return false;
	}

	*text = end;
	*value = result;

	return true;
}

static bool parse_float_value(const char *text, float *value) {
	if (!parse_float_component(&text, value)) { return false; }

	text = skip_whitespace(text);

	return *text == '\0';
}

static bool parse_vec3_value(const char *text, vec3_t *value) {
	vec3_t result;

	if (text == NULL) { return false; }

	if (!parse_float_component(&text, &result.x) ||
	    !parse_float_component(&text, &result.y) ||
	    !parse_float_component(&text, &result.z)) {
		return false;
	}

	text = skip_whitespace(text);
	if (*text != '\0') { return false; }

	*value = result;

	return true;
}

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	if (string == NULL) { return NULL; }

	length = strlen(string);

	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }

	memcpy(copy, string, length + 1);

	return copy;
}

static bool parse_material_document(const keyvalues_document_t *document,
				    material_definition_t *definition,
				    char *error,
				    const size_t error_size) {
	const keyvalues_node_t *root;
	const keyvalues_node_t *material_node;
	const keyvalues_node_t *property;
	const char *key;
	const char *value;
	material_definition_t result;
	size_t index;
	bool has_color;
	bool has_ambient_strength;
	bool has_specular_strength;
	bool has_shininess;
	bool has_albedo_texture;
	bool has_normal_texture;

	result = (material_definition_t){0};
	result.material = material_create(vec3_create(1.0f, 1.0f, 1.0f));

	root = keyvalues_get_root(document);
	if (root == NULL) {
		set_error(error, error_size, "material has no root");
		return false;
	}

	if (keyvalues_node_get_child_count(root) != 1) {
		set_error(error, error_size,
			  "material file must contain exactly "
			  "one material block");
		return false;
	}

	material_node = keyvalues_node_get_child(root, 0);
	if (material_node == NULL || !keyvalues_node_is_block(material_node) ||
	    strcmp(keyvalues_node_get_key(material_node), "material") != 0) {
		set_error(error, error_size,
			  "expected top-level material block");
		return false;
	}

	has_color = false;
	has_ambient_strength = false;
	has_specular_strength = false;
	has_shininess = false;
	has_albedo_texture = false;
	has_normal_texture = false;

	for (index = 0; index < keyvalues_node_get_child_count(material_node);
	     index++) {
		property = keyvalues_node_get_child(material_node, index);

		if (property == NULL || keyvalues_node_is_block(property)) {
			set_error(error, error_size,
				  "invalid material property");
			goto fail;
		}

		key = keyvalues_node_get_key(property);
		value = keyvalues_node_get_value(property);

		if (key == NULL || value == NULL) {
			set_error(error, error_size,
				  "invalid material property");
			goto fail;
		}

		if (strcmp(key, "color") == 0) {
			if (has_color) {
				set_error(error, error_size,
					  "duplicate material color");
				goto fail;
			}

			if (!parse_vec3_value(value, &result.material.color)) {
				set_error(error, error_size,
					  "invalid material color: \"%s\"",
					  value);
				goto fail;
			}

			if (result.material.color.x < 0.0f ||
			    result.material.color.y < 0.0f ||
			    result.material.color.z < 0.0f) {
				set_error(error, error_size,
					  "material color cannot be negative");
				goto fail;
			}

			has_color = true;
			continue;
		}

		if (strcmp(key, "ambient_strength") == 0) {
			if (has_ambient_strength ||
			    !parse_float_value(
				    value, &result.material.ambient_strength) ||
			    result.material.ambient_strength < 0.0f) {
				set_error(error, error_size,
					  "invalid ambient_strength: \"%s\"",
					  value);
				goto fail;
			}

			has_ambient_strength = true;
			continue;
		}

		if (strcmp(key, "specular_strength") == 0) {
			if (has_specular_strength ||
			    !parse_float_value(
				    value,
				    &result.material.specular_strength) ||
			    result.material.specular_strength < 0.0f) {
				set_error(error, error_size,
					  "invalid specular_strength: \"%s\"",
					  value);
				goto fail;
			}

			has_specular_strength = true;
			continue;
		}

		if (strcmp(key, "shininess") == 0) {
			if (has_shininess ||
			    !parse_float_value(value,
					       &result.material.shininess) ||
			    result.material.shininess <= 0.0f) {
				set_error(error, error_size,
					  "invalid shininess: \"%s\"", value);
				goto fail;
			}

			has_shininess = true;
			continue;
		}

		if (strcmp(key, "albedo_texture") == 0) {
			if (has_albedo_texture || value[0] == '\0') {
				set_error(error, error_size,
					  "invalid or duplicate "
					  "albedo_texture");
				goto fail;
			}

			result.albedo_texture_path = duplicate_string(value);
			if (result.albedo_texture_path == NULL) {
				set_error(error, error_size,
					  "failed to allocate "
					  "albedo texture path");
				goto fail;
			}

			has_albedo_texture = true;
			continue;
		}

		if (strcmp(key, "normal_texture") == 0) {
			if (has_normal_texture || value[0] == '\0') {
				set_error(error, error_size,
					  "invalid or duplicate "
					  "normal_texture");
				goto fail;
			}

			result.normal_texture_path = duplicate_string(value);
			if (result.normal_texture_path == NULL) {
				set_error(error, error_size,
					  "failed to allocate "
					  "normal texture path");
				goto fail;
			}

			has_normal_texture = true;
			continue;
		}

		set_error(error, error_size, "unknown material property \"%s\"",
			  key);
		goto fail;
	}

	*definition = result;

	return true;

fail:
	free(result.normal_texture_path);
	free(result.albedo_texture_path);
	return false;
}

bool material_definition_parse(const char *source,
			       material_definition_t *definition,
			       char *error,
			       const size_t error_size) {
	keyvalues_document_t *document;
	bool result;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (source == NULL || definition == NULL) {
		set_error(error, error_size,
			  "invalid material source or output");
		return false;
	}

	*definition = (material_definition_t){0};

	document = keyvalues_parse(source, error, error_size);
	if (document == NULL) { return false; }

	result = parse_material_document(document, definition, error,
					 error_size);

	keyvalues_destroy(document);

	return result;
}

bool material_definition_load(const char *path,
			      material_definition_t *definition,
			      char *error,
			      const size_t error_size) {
	keyvalues_document_t *document;
	bool result;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (path == NULL || path[0] == '\0' || definition == NULL) {
		set_error(error, error_size, "invalid material path or output");
		return false;
	}

	*definition = (material_definition_t){0};

	document = keyvalues_load(path, error, error_size);
	if (document == NULL) { return false; }

	result = parse_material_document(document, definition, error,
					 error_size);

	keyvalues_destroy(document);

	return result;
}

void material_definition_destroy(material_definition_t *definition) {
	if (definition == NULL) { return; }

	free(definition->albedo_texture_path);
	free(definition->normal_texture_path);
	definition->albedo_texture_path = NULL;
	definition->normal_texture_path = NULL;
}

bool material_parse(const char *source,
		    material_t *material,
		    char *error,
		    const size_t error_size) {
	material_definition_t definition;

	if (material == NULL) {
		set_error(error, error_size, "invalid material output");
		return false;
	}

	if (!material_definition_parse(source, &definition, error,
				       error_size)) {
		return false;
	}

	*material = definition.material;

	material_definition_destroy(&definition);

	return true;
}

bool material_load(const char *path,
		   material_t *material,
		   char *error,
		   const size_t error_size) {
	material_definition_t definition;

	if (material == NULL) {
		set_error(error, error_size, "invalid material output");
		return false;
	}

	if (!material_definition_load(path, &definition, error, error_size)) {
		return false;
	}

	*material = definition.material;

	material_definition_destroy(&definition);

	return true;
}
