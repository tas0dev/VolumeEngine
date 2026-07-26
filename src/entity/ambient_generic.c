/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/ambient_generic.h"
#include "asset/manager.h"
#include "audio/audio.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ambient_generic {
	entity_t entity;
	const audio_sound_t *sound;
	audio_voice_id_t voice;
	float volume;
	float minimum_distance;
	float maximum_distance;
	bool looping;
	bool spatial;
	bool requested;
} ambient_generic_t;

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void activate_entity(entity_t *entity);
static void update_entity(entity_t *entity, float delta_time);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_entity(entity_t *entity);
static void play_sound(ambient_generic_t *ambient, entity_t *activator);
static void stop_sound(ambient_generic_t *ambient, entity_t *activator);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t ambient_generic_class = {
	.classname = "ambient_generic",
	.create = create_entity,
	.activate = activate_entity,
	.update = update_entity,
	.accept_input = accept_input,
	.destroy = destroy_entity,
};

bool ambient_generic_register(void) {
	return entity_register_class(&ambient_generic_class);
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	ambient_generic_t *ambient;
	const char *sound_path;
	const char *text;
	bool starts_silent;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL || context->assets == NULL) {
		return NULL;
	}
	sound_path = entity_property_get(context->source, "message");
	if (sound_path == NULL) {
		sound_path = entity_property_get(context->source, "sound");
	}
	if (sound_path == NULL || sound_path[0] == '\0') {
		set_error(context, "ambient_generic requires a sound path");
		return NULL;
	}
	ambient = calloc(1, sizeof(*ambient));
	if (ambient == NULL) { return NULL; }
	entity_initialize(&ambient->entity, id, &ambient_generic_class);
	ambient->entity.transform = context->properties->transform;
	ambient->volume = 1.0f;
	ambient->minimum_distance = 1.0f;
	ambient->maximum_distance = 16.0f;
	ambient->spatial = true;
	starts_silent = false;

	text = entity_property_get(context->source, "volume");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &ambient->volume) ||
	     ambient->volume < 0.0f || ambient->volume > 1.0f)) {
		set_error(context, "invalid ambient_generic volume: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	text = entity_property_get(context->source, "minimum_distance");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &ambient->minimum_distance) ||
	     ambient->minimum_distance < 0.0f)) {
		set_error(context,
			  "invalid ambient_generic minimum_distance: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	text = entity_property_get(context->source, "maximum_distance");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &ambient->maximum_distance) ||
	     ambient->maximum_distance <= ambient->minimum_distance)) {
		set_error(context,
			  "invalid ambient_generic maximum_distance: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	text = entity_property_get(context->source, "looping");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &ambient->looping)) {
		set_error(context, "invalid ambient_generic looping: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	text = entity_property_get(context->source, "spatial");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &ambient->spatial)) {
		set_error(context, "invalid ambient_generic spatial: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	text = entity_property_get(context->source, "StartSilent");
	if (text == NULL) {
		text = entity_property_get(context->source, "starts_silent");
	}
	if (text != NULL && !entity_property_parse_bool(text, &starts_silent)) {
		set_error(context,
			  "invalid ambient_generic starts_silent: \"%s\"",
			  text);
		free(ambient);
		return NULL;
	}
	ambient->sound =
		asset_manager_load_sound(context->assets, sound_path,
					 context->error, context->error_size);
	if (ambient->sound == NULL ||
	    !entity_set_targetname(&ambient->entity,
				   context->properties->targetname)) {
		free(ambient);
		return NULL;
	}
	ambient->requested = !starts_silent;
	return &ambient->entity;
}

static void activate_entity(entity_t *entity) {
	ambient_generic_t *ambient;

	if (entity == NULL) { return; }
	ambient = (ambient_generic_t *)entity;
	if (ambient->requested) { play_sound(ambient, NULL); }
}

static void update_entity(entity_t *entity, const float delta_time) {
	ambient_generic_t *ambient;
	audio_system_t *audio;

	(void)delta_time;
	if (entity == NULL || entity->world == NULL) { return; }
	ambient = (ambient_generic_t *)entity;
	audio = world_get_audio_system(entity->world);
	if (ambient->voice != AUDIO_INVALID_VOICE &&
	    !audio_system_is_playing(audio, ambient->voice)) {
		ambient->voice = AUDIO_INVALID_VOICE;
		if (!ambient->looping) { ambient->requested = false; }
	}
	if (ambient->requested && ambient->voice == AUDIO_INVALID_VOICE) {
		play_sound(ambient, NULL);
	}
	if (ambient->voice != AUDIO_INVALID_VOICE) {
		audio_system_set_voice_position(
			audio, ambient->voice,
			entity_get_world_position(entity));
	}
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	ambient_generic_t *ambient;
	float volume;

	if (entity == NULL || input_name == NULL) { return false; }
	ambient = (ambient_generic_t *)entity;
	if (strcmp(input_name, "PlaySound") == 0) {
		play_sound(ambient,
			   context == NULL ? NULL : context->activator);
		return true;
	}
	if (strcmp(input_name, "StopSound") == 0) {
		stop_sound(ambient,
			   context == NULL ? NULL : context->activator);
		return true;
	}
	if (strcmp(input_name, "ToggleSound") == 0) {
		if (ambient->requested) {
			stop_sound(ambient,
				   context == NULL ? NULL : context->activator);
		} else {
			play_sound(ambient,
				   context == NULL ? NULL : context->activator);
		}
		return true;
	}
	if (strcmp(input_name, "Volume") == 0 && context != NULL &&
	    entity_property_parse_float(context->parameter, &volume) &&
	    volume >= 0.0f && volume <= 1.0f) {
		ambient->volume = volume;
		if (ambient->requested) {
			play_sound(ambient, context->activator);
		}
		return true;
	}
	return false;
}

static void destroy_entity(entity_t *entity) {
	ambient_generic_t *ambient;

	if (entity == NULL) { return; }
	ambient = (ambient_generic_t *)entity;
	if (entity->world != NULL) {
		audio_system_stop(world_get_audio_system(entity->world),
				  ambient->voice);
	}
	free(ambient);
}

static void play_sound(ambient_generic_t *ambient, entity_t *activator) {
	audio_play_params_t params;
	audio_system_t *audio;

	if (ambient == NULL) { return; }
	ambient->requested = true;
	if (ambient->entity.world == NULL) { return; }
	audio = world_get_audio_system(ambient->entity.world);
	if (audio == NULL) { return; }
	audio_system_stop(audio, ambient->voice);
	params = audio_play_params_create();
	params.volume = ambient->volume;
	params.looping = ambient->looping;
	params.spatial = ambient->spatial;
	params.position = entity_get_world_position(&ambient->entity);
	params.minimum_distance = ambient->minimum_distance;
	params.maximum_distance = ambient->maximum_distance;
	ambient->voice = audio_system_play(audio, ambient->sound, &params);
	if (ambient->voice != AUDIO_INVALID_VOICE) {
		(void)world_fire_output(ambient->entity.world, &ambient->entity,
					"OnSoundStarted", activator);
	}
}

static void stop_sound(ambient_generic_t *ambient, entity_t *activator) {
	if (ambient == NULL) { return; }
	if (ambient->entity.world != NULL) {
		audio_system_stop(world_get_audio_system(ambient->entity.world),
				  ambient->voice);
		(void)world_fire_output(ambient->entity.world, &ambient->entity,
					"OnSoundStopped", activator);
	}
	ambient->voice = AUDIO_INVALID_VOICE;
	ambient->requested = false;
}

static void
set_error(const entity_spawn_context_t *context, const char *format, ...) {
	va_list arguments;

	if (context == NULL || context->error == NULL ||
	    context->error_size == 0) {
		return;
	}
	va_start(arguments, format);
	vsnprintf(context->error, context->error_size, format, arguments);
	va_end(arguments);
}
