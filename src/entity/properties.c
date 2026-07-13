/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/properties.h"
#include "math/math.h"
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

entity_properties_t entity_properties_create(void) {
	entity_properties_t properties;

	properties.targetname = NULL;
	properties.transform = transform_create();

	return properties;
}

const char *entity_property_get(const entity_property_source_t *source,
				const char *key) {
	if (source == NULL || source->get == NULL || key == NULL) {
		return NULL;
	}

	return source->get(source->context, key);
}

bool entity_property_parse_float(const char *text, float *value) {
	if (text == NULL || value == NULL) { return false; }

	if (!parse_float_component(&text, value)) { return false; }

	text = skip_whitespace(text);

	return *text == '\0';
}

bool entity_property_parse_vec3(const char *text, vec3_t *value) {
	vec3_t result;

	if (text == NULL || value == NULL) { return false; }

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

bool entity_property_parse_bool(const char *text, bool *value) {
	if (text == NULL || value == NULL) { return false; }

	if (strcmp(text, "1") == 0 || strcmp(text, "true") == 0) {
		*value = true;
		return true;
	}

	if (strcmp(text, "0") == 0 || strcmp(text, "false") == 0) {
		*value = false;
		return true;
	}

	return false;
}

bool entity_properties_load(entity_properties_t *properties,
			    const entity_property_source_t *source,
			    char *error,
			    const size_t error_size) {
	const char *text;
	const float degrees_to_radians = PI / 180.0f;

	if (properties == NULL || source == NULL || source->get == NULL) {
		set_error(error, error_size, "invalid entity property source");
		return false;
	}

	*properties = entity_properties_create();

	properties->targetname = entity_property_get(source, "targetname");

	text = entity_property_get(source, "origin");
	if (text != NULL && !entity_property_parse_vec3(
				    text, &properties->transform.position)) {
		set_error(error, error_size,
			  "invalid vec3 property \"origin\": \"%s\"", text);
		return false;
	}

	text = entity_property_get(source, "angles");
	if (text != NULL && !entity_property_parse_vec3(
				    text, &properties->transform.rotation)) {
		set_error(error, error_size,
			  "invalid vec3 property \"angles\": \"%s\"", text);
		return false;
	}

	properties->transform.rotation.x *= degrees_to_radians;
	properties->transform.rotation.y *= degrees_to_radians;
	properties->transform.rotation.z *= degrees_to_radians;

	text = entity_property_get(source, "scale");
	if (text != NULL &&
	    !entity_property_parse_vec3(text, &properties->transform.scale)) {
		set_error(error, error_size,
			  "invalid vec3 property \"scale\": \"%s\"", text);
		return false;
	}

	return true;
}