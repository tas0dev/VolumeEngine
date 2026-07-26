/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "audio/sound_internal.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void set_error(char *error, size_t error_size, const char *format, ...);

audio_sound_t *
audio_sound_load_wav(const char *path, char *error, const size_t error_size) {
	SDL_AudioSpec source_spec;
	SDL_AudioSpec destination_spec;
	audio_sound_t *sound;
	Uint8 *source_data;
	Uint8 *converted_data;
	Uint32 source_length;
	int converted_length;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }
	if (path == NULL || path[0] == '\0') {
		set_error(error, error_size, "invalid WAV path");
		return NULL;
	}
	if (!SDL_LoadWAV(path, &source_spec, &source_data, &source_length)) {
		set_error(error, error_size, "failed to load WAV: %s",
			  SDL_GetError());
		return NULL;
	}
	destination_spec.format = SDL_AUDIO_F32;
	destination_spec.channels = AUDIO_CHANNEL_COUNT;
	destination_spec.freq = AUDIO_SAMPLE_RATE;
	converted_data = NULL;
	converted_length = 0;
	if (!SDL_ConvertAudioSamples(&source_spec, source_data,
				     (int)source_length, &destination_spec,
				     &converted_data, &converted_length)) {
		set_error(error, error_size, "failed to convert WAV: %s",
			  SDL_GetError());
		SDL_free(source_data);
		return NULL;
	}
	SDL_free(source_data);
	sound = calloc(1, sizeof(*sound));
	if (sound == NULL) {
		SDL_free(converted_data);
		return NULL;
	}
	sound->samples = malloc((size_t)converted_length);
	if (sound->samples == NULL) {
		SDL_free(converted_data);
		free(sound);
		return NULL;
	}
	SDL_memcpy(sound->samples, converted_data, (size_t)converted_length);
	SDL_free(converted_data);
	sound->frame_count = (size_t)converted_length /
			     (sizeof(float) * AUDIO_CHANNEL_COUNT);
	return sound;
}

audio_sound_t *audio_sound_create_tone(const float frequency,
				       const float duration) {
	audio_sound_t *sound;
	size_t frame;
	float envelope;
	float sample;

	if (!isfinite(frequency) || frequency <= 0.0f || !isfinite(duration) ||
	    duration <= 0.0f) {
		return NULL;
	}
	sound = calloc(1, sizeof(*sound));
	if (sound == NULL) { return NULL; }
	sound->frame_count = (size_t)(duration * AUDIO_SAMPLE_RATE);
	if (sound->frame_count == 0 ||
	    sound->frame_count >
		    SIZE_MAX / (sizeof(float) * AUDIO_CHANNEL_COUNT)) {
		free(sound);
		return NULL;
	}
	sound->samples = calloc(sound->frame_count * AUDIO_CHANNEL_COUNT,
				sizeof(*sound->samples));
	if (sound->samples == NULL) {
		free(sound);
		return NULL;
	}
	for (frame = 0; frame < sound->frame_count; frame++) {
		envelope = 1.0f - (float)frame / (float)sound->frame_count;
		sample = sinf(6.28318530718f * frequency * (float)frame /
			      AUDIO_SAMPLE_RATE) *
			 envelope * 0.35f;
		sound->samples[frame * 2] = sample;
		sound->samples[frame * 2 + 1] = sample;
	}
	return sound;
}

void audio_sound_destroy(audio_sound_t *sound) {
	if (sound == NULL) { return; }
	free(sound->samples);
	free(sound);
}

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }
	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}
