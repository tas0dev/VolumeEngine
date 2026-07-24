/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/io.h"

#include "entity/entity.h"
#include "world.h"
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *duplicate_string(const char *string);
static bool reserve_outputs(entity_t *entity, size_t capacity);
static void set_error(char *error, size_t error_size, const char *format, ...);
static bool parse_float(const char *text, float *value);
static bool parse_integer(const char *text, int *value);
static entity_t *resolve_parent_target(entity_t *entity,
				       const entity_input_context_t *context);

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	if (string == NULL) { string = ""; }

	length = strlen(string);
	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }

	memcpy(copy, string, length + 1);
	return copy;
}

static bool reserve_outputs(entity_t *entity, const size_t capacity) {
	entity_output_connection_t *outputs;

	if (capacity <= entity->output_capacity) { return true; }
	if (capacity > SIZE_MAX / sizeof(*entity->outputs)) { return false; }

	outputs = realloc(entity->outputs, capacity * sizeof(*entity->outputs));
	if (outputs == NULL) { return false; }

	entity->outputs = outputs;
	entity->output_capacity = capacity;
	return true;
}

bool entity_add_output(entity_t *entity,
		       const char *output_name,
		       const char *target_name,
		       const char *input_name,
		       const char *parameter,
		       const float delay,
		       const int maximum_fires) {
	entity_output_connection_t connection = {0};
	size_t capacity;

	if (entity == NULL || output_name == NULL || output_name[0] == '\0' ||
	    target_name == NULL || target_name[0] == '\0' ||
	    input_name == NULL || input_name[0] == '\0' || delay < 0.0f ||
	    maximum_fires == 0 || maximum_fires < -1) {
		return false;
	}

	connection.output_name = duplicate_string(output_name);
	connection.target_name = duplicate_string(target_name);
	connection.input_name = duplicate_string(input_name);
	connection.parameter = duplicate_string(parameter);
	connection.delay = delay;
	connection.remaining_fires = maximum_fires;

	if (connection.output_name == NULL || connection.target_name == NULL ||
	    connection.input_name == NULL || connection.parameter == NULL) {
		free(connection.parameter);
		free(connection.input_name);
		free(connection.target_name);
		free(connection.output_name);
		return false;
	}

	if (entity->output_count == entity->output_capacity) {
		capacity = entity->output_capacity == 0
				   ? 4
				   : entity->output_capacity * 2;

		if (capacity < entity->output_capacity ||
		    !reserve_outputs(entity, capacity)) {
			free(connection.parameter);
			free(connection.input_name);
			free(connection.target_name);
			free(connection.output_name);
			return false;
		}
	}

	entity->outputs[entity->output_count++] = connection;
	return true;
}

bool entity_add_output_from_string(entity_t *entity,
				   const char *output_name,
				   const char *value,
				   char *error,
				   const size_t error_size) {
	char *copy;
	char *fields[5] = {0};
	char *cursor;
	char *separator;
	float delay;
	int maximum_fires;
	size_t index;
	bool result;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (entity == NULL || output_name == NULL || value == NULL) {
		set_error(error, error_size, "invalid entity output");
		return false;
	}

	copy = duplicate_string(value);
	if (copy == NULL) {
		set_error(error, error_size, "out of memory");
		return false;
	}

	cursor = copy;
	for (index = 0; index < 5; index++) {
		fields[index] = cursor;
		separator = strchr(cursor, ',');

		if (separator == NULL) {
			cursor = NULL;
			break;
		}

		*separator = '\0';
		cursor = separator + 1;
	}

	if (cursor != NULL || fields[0] == NULL || fields[1] == NULL) {
		set_error(error, error_size,
			  "output \"%s\" must use "
			  "target,input,parameter,delay,max_fires",
			  output_name);
		free(copy);
		return false;
	}

	if (fields[2] == NULL) { fields[2] = ""; }
	if (fields[3] == NULL) { fields[3] = "0"; }
	if (fields[4] == NULL) { fields[4] = "-1"; }

	if (!parse_float(fields[3], &delay) || delay < 0.0f ||
	    !parse_integer(fields[4], &maximum_fires) || maximum_fires == 0 ||
	    maximum_fires < -1) {
		set_error(error, error_size,
			  "invalid delay or fire count in output \"%s\"",
			  output_name);
		free(copy);
		return false;
	}

	result = entity_add_output(entity, output_name, fields[0], fields[1],
				   fields[2], delay, maximum_fires);
	if (!result) {
		set_error(error, error_size, "failed to add output \"%s\"",
			  output_name);
	}

	free(copy);
	return result;
}

size_t entity_get_output_count(const entity_t *entity) {
	return entity == NULL ? 0 : entity->output_count;
}

const entity_output_connection_t *entity_get_output(const entity_t *entity,
						    const size_t index) {
	if (entity == NULL || index >= entity->output_count) { return NULL; }
	return &entity->outputs[index];
}

bool entity_accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	entity_t *parent;

	if (entity == NULL || input_name == NULL || input_name[0] == '\0') {
		return false;
	}

	if (strcmp(input_name, "SetParent") == 0) {
		parent = resolve_parent_target(entity, context);

		if (parent == NULL || parent == entity) { return false; }

		return entity_set_parent(entity, parent, true);
	}

	if (strcmp(input_name, "ClearParent") == 0) {
		return entity_clear_parent(entity, true);
	}

	if (strcmp(input_name, "Kill") == 0) {
		entity->pending_destroy = true;
		return true;
	}

	if (entity->class != NULL && entity->class->accept_input != NULL &&
	    entity->class->accept_input(entity, input_name, context)) {
		return true;
	}

	if (strcmp(input_name, "Enable") == 0) {
		entity_set_active(entity, true);
		return true;
	}

	if (strcmp(input_name, "Disable") == 0) {
		entity_set_active(entity, false);
		return true;
	}

	if (strcmp(input_name, "Toggle") == 0) {
		entity_set_active(entity, !entity_is_active(entity));
		return true;
	}

	return false;
}

void entity_io_destroy(entity_t *entity) {
	size_t index;

	if (entity == NULL) { return; }

	for (index = 0; index < entity->output_count; index++) {
		free(entity->outputs[index].parameter);
		free(entity->outputs[index].input_name);
		free(entity->outputs[index].target_name);
		free(entity->outputs[index].output_name);
	}

	free(entity->outputs);
	entity->outputs = NULL;
	entity->output_count = 0;
	entity->output_capacity = 0;
}

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static bool parse_float(const char *text, float *value) {
	char *end;
	float parsed;

	if (text == NULL || text[0] == '\0' || value == NULL) { return false; }
	errno = 0;
	parsed = strtof(text, &end);
	if (errno == ERANGE || end == text || *end != '\0' ||
	    !isfinite(parsed)) {
		return false;
	}
	*value = parsed;
	return true;
}

static bool parse_integer(const char *text, int *value) {
	char *end;
	long parsed;

	if (text == NULL || text[0] == '\0' || value == NULL) { return false; }
	errno = 0;
	parsed = strtol(text, &end, 10);
	if (errno == ERANGE || end == text || *end != '\0' || parsed < -1 ||
	    parsed > 2147483647L) {
		return false;
	}
	*value = (int)parsed;
	return true;
}

static entity_t *resolve_parent_target(
	entity_t *entity,
	const entity_input_context_t *context) {
	const char *parameter;

	if (entity == NULL || context == NULL || context->world == NULL) {
		return NULL;
	}

	parameter = context->parameter;

	if (parameter == NULL || parameter[0] == '\0') { return NULL; }

	if (strcmp(parameter, "!self") == 0) { return entity; }

	if (strcmp(parameter, "!activator") == 0) {
		return context->activator;
	}

	if (strcmp(parameter, "!caller") == 0) {
		return context->caller;
	}

	if (parameter[0] == '!') { return NULL; }

	return world_find_by_targetname(context->world, parameter);
}