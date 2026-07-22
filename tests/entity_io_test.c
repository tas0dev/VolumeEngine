/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/entity.h"
#include "entity/io.h"
#include "entity/logic_relay.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include <stdlib.h>
#include <string.h>

typedef struct io_test_entity {
	entity_t entity;
	char value[64];
	entity_id_t activator_id;
	entity_id_t caller_id;
	size_t input_count;
} io_test_entity_t;

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_entity(entity_t *entity);

static const entity_class_t io_test_class = {
	.classname = "io_test",
	.create = create_entity,
	.accept_input = accept_input,
	.destroy = destroy_entity,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	io_test_entity_t *entity;

	if (context == NULL || context->properties == NULL) { return NULL; }
	entity = calloc(1, sizeof(*entity));
	if (entity == NULL) { return NULL; }
	entity_initialize(&entity->entity, id, &io_test_class);
	entity->entity.transform = context->properties->transform;

	if (!entity_set_targetname(&entity->entity,
				   context->properties->targetname)) {
		free(entity);
		return NULL;
	}
	return &entity->entity;
}

static bool accept_input(entity_t *base,
			 const char *input_name,
			 const entity_input_context_t *context) {
	io_test_entity_t *entity;
	const char *parameter;

	if (strcmp(input_name, "SetValue") != 0) { return false; }
	entity = (io_test_entity_t *)base;
	parameter = context == NULL || context->parameter == NULL
			    ? ""
			    : context->parameter;
	strncpy(entity->value, parameter, sizeof(entity->value) - 1);
	entity->value[sizeof(entity->value) - 1] = '\0';
	entity->activator_id = context == NULL || context->activator == NULL
				       ? 0
				       : context->activator->id;
	entity->caller_id = context == NULL || context->caller == NULL
				    ? 0
				    : context->caller->id;
	entity->input_count++;
	return true;
}

static void destroy_entity(entity_t *entity) {
	free((io_test_entity_t *)entity);
}

static bool test_map_output_delay_and_fire_count(void) {
	static const char source[] =
		"world\n"
		"{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"logic_relay\"\n"
		"\t\"targetname\" \"sender\"\n"
		"\t\"OnTrigger\" \"receivers,SetValue,hello,0.5,1\"\n"
		"\t\"OnTrigger\" \"missing,Enable,,0,1\"\n}\n"
		"entity\n{\n\t\"classname\" \"io_test\"\n"
		"\t\"targetname\" \"receivers\"\n}\n"
		"entity\n{\n\t\"classname\" \"io_test\"\n"
		"\t\"targetname\" \"receivers\"\n}\n";
	const entity_output_connection_t *output;
	io_test_entity_t *first;
	io_test_entity_t *second;
	entity_t *sender;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(entity_register_class(&io_test_class));
	CHECK(logic_relay_register());
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, NULL, error, sizeof(error)));

	sender = world_get_entity(world, 0);
	first = (io_test_entity_t *)world_get_entity(world, 1);
	second = (io_test_entity_t *)world_get_entity(world, 2);
	CHECK(entity_get_output_count(sender) == 2);
	output = entity_get_output(sender, 0);
	CHECK(output != NULL);
	CHECK(strcmp(output->target_name, "receivers") == 0);
	CHECK(strcmp(output->input_name, "SetValue") == 0);
	CHECK(output->delay == 0.5f);
	CHECK(output->remaining_fires == 1);

	CHECK(world_send_input(world, "sender", "Trigger", "", &first->entity,
			       &first->entity) == 1);
	CHECK(world_get_pending_event_count(world) == 2);
	world_update(world, 0.25f);
	CHECK(first->input_count == 0 && second->input_count == 0);
	CHECK(world_get_pending_event_count(world) == 1);
	world_update(world, 0.25f);
	CHECK(first->input_count == 1 && second->input_count == 1);
	CHECK(strcmp(first->value, "hello") == 0);
	CHECK(first->activator_id == first->entity.id);
	CHECK(first->caller_id == sender->id);
	CHECK(output->remaining_fires == 0);
	CHECK(world_send_input(world, "sender", "Trigger", "", &first->entity,
			       &first->entity) == 1);
	CHECK(world_get_pending_event_count(world) == 0);

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
	return true;
}

static bool test_generic_inputs_and_deferred_kill(void) {
	entity_properties_t properties;
	entity_spawn_context_t context = {0};
	entity_t *entity;
	world_t *world;

	CHECK(entity_register_class(&io_test_class));
	world = world_create();
	CHECK(world != NULL);
	properties = entity_properties_create();
	properties.targetname = "target";
	context.properties = &properties;
	entity = world_spawn_entity(world, "io_test", &context);
	CHECK(entity != NULL);

	CHECK(world_send_input(world, "target", "Disable", "", NULL, NULL) ==
	      1);
	CHECK(!entity_is_active(entity));
	CHECK(world_send_input(world, "target", "Toggle", "", NULL, NULL) == 1);
	CHECK(entity_is_active(entity));
	CHECK(world_send_input(world, "!self", "Kill", "", NULL, entity) == 1);
	CHECK(world_get_entity_count(world) == 1);
	world_update(world, 0.0f);
	CHECK(world_get_entity_count(world) == 0);

	world_destroy(world);
	entity_registry_shutdown();
	return true;
}

static bool test_invalid_map_output_rolls_back(void) {
	static const char source[] = "world\n"
				     "{\n\t\"classname\" \"worldspawn\"\n}\n"
				     "entity\n{\n\t\"classname\" \"io_test\"\n"
				     "\t\"OnTrigger\" \"missing_input\"\n}\n";
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(entity_register_class(&io_test_class));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(!map_spawn_entities(map, world, NULL, error, sizeof(error)));
	CHECK(world_get_entity_count(world) == 0);
	CHECK(strstr(error, "must use") != NULL);
	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"map output delay and fire count",
		 test_map_output_delay_and_fire_count },
		{"generic inputs and deferred kill",
		 test_generic_inputs_and_deferred_kill},
		{"invalid map output rolls back",
		 test_invalid_map_output_rolls_back   },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
