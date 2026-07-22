/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_IO_H
#define VOLUME_ENTITY_IO_H

#include "core/types.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct entity_input_context {
	world_t *world;
	entity_t *activator;
	entity_t *caller;
	const char *parameter;
} entity_input_context_t;

typedef struct entity_output_connection {
	char *output_name;
	char *target_name;
	char *input_name;
	char *parameter;
	float delay;
	int remaining_fires;
} entity_output_connection_t;

bool entity_add_output(entity_t *entity,
		       const char *output_name,
		       const char *target_name,
		       const char *input_name,
		       const char *parameter,
		       float delay,
		       int maximum_fires);
bool entity_add_output_from_string(entity_t *entity,
				   const char *output_name,
				   const char *value,
				   char *error,
				   size_t error_size);
size_t entity_get_output_count(const entity_t *entity);
const entity_output_connection_t *entity_get_output(const entity_t *entity,
						    size_t index);
bool entity_accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
void entity_io_destroy(entity_t *entity);

#endif
