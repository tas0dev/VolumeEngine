/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/player.h"
#include "entity/trigger.h"
#include "entity/world.h"
#include "map/map.h"
#include "map/spawn.h"
#include <stdlib.h>
#include <string.h>

typedef struct trigger_receiver {
	entity_t entity;
	entity_id_t activator_id;
	size_t input_count;
} trigger_receiver_t;

static entity_t *create_receiver(entity_id_t id,
				 const entity_spawn_context_t *context);
static bool receiver_accept_input(entity_t *entity,
				  const char *input_name,
				  const entity_input_context_t *context);
static void destroy_receiver(entity_t *entity);

static const entity_class_t receiver_class = {
	.classname = "trigger_receiver",
	.create = create_receiver,
	.accept_input = receiver_accept_input,
	.destroy = destroy_receiver,
};

static entity_t *create_receiver(const entity_id_t id,
				 const entity_spawn_context_t *context) {
	trigger_receiver_t *receiver;

	if (context == NULL || context->properties == NULL) { return NULL; }
	receiver = calloc(1, sizeof(*receiver));
	if (receiver == NULL) { return NULL; }
	entity_initialize(&receiver->entity, id, &receiver_class);
	if (!entity_set_targetname(&receiver->entity,
				   context->properties->targetname)) {
		free(receiver);
		return NULL;
	}
	return &receiver->entity;
}

static bool receiver_accept_input(entity_t *entity,
				  const char *input_name,
				  const entity_input_context_t *context) {
	trigger_receiver_t *receiver;

	if (strcmp(input_name, "Hit") != 0) { return false; }
	receiver = (trigger_receiver_t *)entity;
	receiver->input_count++;
	receiver->activator_id = context == NULL || context->activator == NULL
					 ? 0
					 : context->activator->id;
	return true;
}

static void destroy_receiver(entity_t *entity) { free(entity); }

static bool register_test_classes(void) {
	return player_register() && trigger_once_register() &&
	       trigger_multiple_register() &&
	       entity_register_class(&receiver_class);
}

static bool test_trigger_once_fires_with_player_activator(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"trigger_once\"\n"
		"\t\"targetname\" \"trigger\"\n"
		"\t\"size\" \"4 4 4\"\n"
		"\t\"OnTrigger\" \"receiver,Hit,,0,-1\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"player\"\n"
		"\t\"origin\" \"0 -0.85 0\"\n}\n"
		"entity\n{\n\t\"classname\" \"trigger_receiver\"\n"
		"\t\"targetname\" \"receiver\"\n}\n";
	trigger_receiver_t *receiver;
	entity_t *player;
	entity_t *trigger;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(register_test_classes());
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, NULL, error, sizeof(error)));
	trigger = world_find_by_targetname(world, "trigger");
	player = world_find_by_targetname(world, "player");
	receiver = (trigger_receiver_t *)world_find_by_targetname(world,
								  "receiver");
	CHECK(trigger != NULL && player != NULL && receiver != NULL);

	world_update(world, 0.1f);
	world_update(world, 0.1f);
	world_update(world, 1.0f);
	CHECK(receiver->input_count == 1);
	CHECK(receiver->activator_id == player->id);
	CHECK(!entity_is_active(trigger));

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
	return true;
}

static bool test_trigger_multiple_repeats_and_ends_touch(void) {
	static const char source[] =
		"world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
		"entity\n{\n\t\"classname\" \"trigger_multiple\"\n"
		"\t\"size\" \"4 4 4\"\n"
		"\t\"wait\" \"0.5\"\n"
		"\t\"OnTrigger\" \"repeat,Hit,,0,-1\"\n"
		"\t\"OnEndTouch\" \"ended,Hit,,0,-1\"\n}\n"
		"entity\n{\n\t\"classname\" \"player\"\n"
		"\t\"targetname\" \"player\"\n"
		"\t\"origin\" \"0 -0.85 0\"\n}\n"
		"entity\n{\n\t\"classname\" \"trigger_receiver\"\n"
		"\t\"targetname\" \"repeat\"\n}\n"
		"entity\n{\n\t\"classname\" \"trigger_receiver\"\n"
		"\t\"targetname\" \"ended\"\n}\n";
	trigger_receiver_t *repeat;
	trigger_receiver_t *ended;
	entity_t *player;
	world_t *world;
	map_t *map;
	char error[256];

	CHECK(register_test_classes());
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, NULL, error, sizeof(error)));
	player = world_find_by_targetname(world, "player");
	repeat =
		(trigger_receiver_t *)world_find_by_targetname(world, "repeat");
	ended = (trigger_receiver_t *)world_find_by_targetname(world, "ended");
	CHECK(player != NULL && repeat != NULL && ended != NULL);

	world_update(world, 0.1f);
	world_update(world, 0.2f);
	world_update(world, 0.3f);
	world_update(world, 0.0f);
	CHECK(repeat->input_count == 2);

	player->transform.position = vec3_create(10.0f, 0.0f, 0.0f);
	CHECK(collision_world_update_collider_filtered(
		world_get_collision_world(world), player->id, player->collider,
		player->transform.position, player->collision_layer,
		player->collision_mask));
	world_update(world, 0.1f);
	world_update(world, 0.0f);
	CHECK(ended->input_count == 1);
	CHECK(ended->activator_id == player->id);

	world_destroy(world);
	map_destroy(map);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"trigger_once fires with player activator",
		 test_trigger_once_fires_with_player_activator},
		{"trigger_multiple repeats and ends touch",
		 test_trigger_multiple_repeats_and_ends_touch },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
