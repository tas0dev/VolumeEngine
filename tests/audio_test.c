/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "volume.h"
#include <string.h>

static bool test_tone_validation(void) {
	audio_sound_t *sound;

	CHECK(audio_sound_create_tone(0.0f, 1.0f) == NULL);
	CHECK(audio_sound_create_tone(440.0f, 0.0f) == NULL);
	sound = audio_sound_create_tone(440.0f, 0.02f);
	CHECK(sound != NULL);
	audio_sound_destroy(sound);
	return true;
}

#ifdef TEST_MP3_FILE
static bool test_mp3_loading(void) {
	audio_sound_t *sound;
	char error[512];

	sound = audio_sound_load_mp3(TEST_MP3_FILE, error, sizeof(error));
	CHECK(sound != NULL);
	audio_sound_destroy(sound);
	return true;
}
#endif

static bool test_ambient_generic_inputs(void) {
	const char *source = "world\n"
			     "{\n"
			     "  \"classname\" \"worldspawn\"\n"
			     "}\n"
			     "entity\n"
			     "{\n"
			     "  \"classname\" \"ambient_generic\"\n"
			     "  \"targetname\" \"room_tone\"\n"
			     "  \"message\" \"sounds/test.wav\"\n"
			     "  \"volume\" \"0.5\"\n"
			     "  \"looping\" \"1\"\n"
			     "  \"StartSilent\" \"1\"\n"
			     "}\n";
	asset_manager_t *assets;
	audio_sound_t *sound;
	map_t *map;
	world_t *world;
	entity_t *ambient;
	char error[512];

	CHECK(ambient_generic_register());
	assets = asset_manager_create();
	sound = audio_sound_create_tone(220.0f, 0.02f);
	CHECK(assets != NULL);
	CHECK(sound != NULL);
	CHECK(asset_manager_register_sound(assets, "sounds/test.wav", sound));
	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);
	world = world_create();
	CHECK(world != NULL);
	CHECK(map_spawn_entities(map, world, assets, error, sizeof(error)));
	ambient = world_find_by_targetname(world, "room_tone");
	CHECK(ambient != NULL);
	CHECK(strcmp(entity_get_classname(ambient), "ambient_generic") == 0);
	CHECK(world_send_input_to_entity(world, ambient, "PlaySound", "", NULL,
					 NULL));
	CHECK(world_send_input_to_entity(world, ambient, "Volume", "0.25", NULL,
					 NULL));
	CHECK(world_send_input_to_entity(world, ambient, "ToggleSound", "",
					 NULL, NULL));
	CHECK(world_send_input_to_entity(world, ambient, "StopSound", "", NULL,
					 NULL));
	CHECK(!world_send_input_to_entity(world, ambient, "Volume", "2", NULL,
					  NULL));
	world_destroy(world);
	map_destroy(map);
	asset_manager_destroy(assets);
	audio_sound_destroy(sound);
	entity_registry_shutdown();
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"tone validation",	    test_tone_validation	},
#ifdef TEST_MP3_FILE
		{"MP3 loading",		test_mp3_loading		},
#endif
		{"ambient_generic inputs", test_ambient_generic_inputs},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
