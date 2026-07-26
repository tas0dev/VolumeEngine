/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "audio/audio.h"
#include "audio/sound_internal.h"
#include "core/log.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>

#define AUDIO_MAXIMUM_VOICES 64

typedef struct audio_voice {
	audio_voice_id_t id;
	const audio_sound_t *sound;
	size_t frame;
	audio_play_params_t params;
	bool active;
} audio_voice_t;

struct audio_system {
	SDL_AudioStream *stream;
	SDL_Mutex *mutex;
	audio_voice_t voices[AUDIO_MAXIMUM_VOICES];
	audio_voice_id_t next_voice_id;
	vec3_t listener_position;
	vec3_t listener_forward;
	vec3_t listener_up;
	float *mix_buffer;
	size_t mix_buffer_capacity;
};

static void SDLCALL audio_callback(void *userdata,
				   SDL_AudioStream *stream,
				   int additional_amount,
				   int total_amount);
static void mix_voice(const audio_system_t *audio,
		      audio_voice_t *voice,
		      float *output,
		      size_t frame_count);
static audio_voice_t *find_voice(audio_system_t *audio, audio_voice_id_t id);

audio_system_t *audio_system_create(void) {
	SDL_AudioSpec spec;
	audio_system_t *audio;

	audio = calloc(1, sizeof(*audio));
	if (audio == NULL) { return NULL; }
	audio->mutex = SDL_CreateMutex();
	if (audio->mutex == NULL) {
		free(audio);
		return NULL;
	}
	audio->next_voice_id = 1;
	audio->listener_forward = vec3_create(0.0f, 0.0f, -1.0f);
	audio->listener_up = vec3_create(0.0f, 1.0f, 0.0f);
	spec.format = SDL_AUDIO_F32;
	spec.channels = AUDIO_CHANNEL_COUNT;
	spec.freq = AUDIO_SAMPLE_RATE;
	audio->stream =
		SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
					  &spec, audio_callback, audio);
	if (audio->stream == NULL) {
		log_info("Audio output unavailable: %s", SDL_GetError());
		return audio;
	}
	if (!SDL_ResumeAudioStreamDevice(audio->stream)) {
		log_info("Failed to start audio output: %s", SDL_GetError());
		SDL_DestroyAudioStream(audio->stream);
		audio->stream = NULL;
	}
	return audio;
}

void audio_system_destroy(audio_system_t *audio) {
	if (audio == NULL) { return; }
	if (audio->stream != NULL) { SDL_DestroyAudioStream(audio->stream); }
	SDL_DestroyMutex(audio->mutex);
	free(audio->mix_buffer);
	free(audio);
}

audio_play_params_t audio_play_params_create(void) {
	audio_play_params_t params = {0};

	params.volume = 1.0f;
	params.minimum_distance = 1.0f;
	params.maximum_distance = 16.0f;
	return params;
}

audio_voice_id_t audio_system_play(audio_system_t *audio,
				   const audio_sound_t *sound,
				   const audio_play_params_t *params) {
	audio_play_params_t settings;
	audio_voice_t *voice;
	size_t index;

	if (audio == NULL || audio->stream == NULL || sound == NULL ||
	    sound->samples == NULL || sound->frame_count == 0) {
		return AUDIO_INVALID_VOICE;
	}
	settings = params == NULL ? audio_play_params_create() : *params;
	if (!isfinite(settings.volume) || settings.volume < 0.0f ||
	    !isfinite(settings.minimum_distance) ||
	    !isfinite(settings.maximum_distance) ||
	    settings.minimum_distance < 0.0f ||
	    settings.maximum_distance <= settings.minimum_distance) {
		return AUDIO_INVALID_VOICE;
	}
	SDL_LockMutex(audio->mutex);
	voice = NULL;
	for (index = 0; index < AUDIO_MAXIMUM_VOICES; index++) {
		if (!audio->voices[index].active) {
			voice = &audio->voices[index];
			break;
		}
	}
	if (voice == NULL) {
		SDL_UnlockMutex(audio->mutex);
		return AUDIO_INVALID_VOICE;
	}
	voice->id = audio->next_voice_id++;
	if (audio->next_voice_id == AUDIO_INVALID_VOICE) {
		audio->next_voice_id = 1;
	}
	voice->sound = sound;
	voice->frame = 0;
	voice->params = settings;
	voice->active = true;
	SDL_UnlockMutex(audio->mutex);
	return voice->id;
}

void audio_system_stop(audio_system_t *audio, const audio_voice_id_t voice) {
	audio_voice_t *found;

	if (audio == NULL || voice == AUDIO_INVALID_VOICE) { return; }
	SDL_LockMutex(audio->mutex);
	found = find_voice(audio, voice);
	if (found != NULL) { found->active = false; }
	SDL_UnlockMutex(audio->mutex);
}

void audio_system_stop_all(audio_system_t *audio) {
	size_t index;

	if (audio == NULL) { return; }
	SDL_LockMutex(audio->mutex);
	for (index = 0; index < AUDIO_MAXIMUM_VOICES; index++) {
		audio->voices[index].active = false;
	}
	SDL_UnlockMutex(audio->mutex);
}

bool audio_system_is_playing(audio_system_t *audio,
			     const audio_voice_id_t voice) {
	bool playing;

	if (audio == NULL || voice == AUDIO_INVALID_VOICE) { return false; }
	SDL_LockMutex(audio->mutex);
	playing = find_voice(audio, voice) != NULL;
	SDL_UnlockMutex(audio->mutex);
	return playing;
}

void audio_system_set_voice_position(audio_system_t *audio,
				     const audio_voice_id_t voice,
				     const vec3_t position) {
	audio_voice_t *found;

	if (audio == NULL || voice == AUDIO_INVALID_VOICE) { return; }
	SDL_LockMutex(audio->mutex);
	found = find_voice(audio, voice);
	if (found != NULL) { found->params.position = position; }
	SDL_UnlockMutex(audio->mutex);
}

void audio_system_set_listener(audio_system_t *audio,
			       const vec3_t position,
			       const vec3_t forward,
			       const vec3_t up) {
	if (audio == NULL) { return; }
	SDL_LockMutex(audio->mutex);
	audio->listener_position = position;
	audio->listener_forward = vec3_normalize(forward);
	audio->listener_up = vec3_normalize(up);
	SDL_UnlockMutex(audio->mutex);
}

static void SDLCALL audio_callback(void *userdata,
				   SDL_AudioStream *stream,
				   const int additional_amount,
				   const int total_amount) {
	audio_system_t *audio;
	float *output;
	float *resized_buffer;
	size_t frame_count;
	size_t sample_count;
	size_t index;

	(void)total_amount;
	if (userdata == NULL || stream == NULL || additional_amount <= 0) {
		return;
	}
	audio = userdata;
	sample_count = (size_t)additional_amount / sizeof(float);
	frame_count = sample_count / AUDIO_CHANNEL_COUNT;
	if (sample_count > audio->mix_buffer_capacity) {
		resized_buffer =
			realloc(audio->mix_buffer,
				sample_count * sizeof(*resized_buffer));
		if (resized_buffer == NULL) { return; }
		audio->mix_buffer = resized_buffer;
		audio->mix_buffer_capacity = sample_count;
	}
	output = audio->mix_buffer;
	SDL_memset(output, 0, sample_count * sizeof(*output));
	SDL_LockMutex(audio->mutex);
	for (index = 0; index < AUDIO_MAXIMUM_VOICES; index++) {
		if (audio->voices[index].active) {
			mix_voice(audio, &audio->voices[index], output,
				  frame_count);
		}
	}
	SDL_UnlockMutex(audio->mutex);
	for (index = 0; index < sample_count; index++) {
		output[index] = fmaxf(-1.0f, fminf(1.0f, output[index]));
	}
	(void)SDL_PutAudioStreamData(stream, output,
				     (int)(sample_count * sizeof(*output)));
}

static void mix_voice(const audio_system_t *audio,
		      audio_voice_t *voice,
		      float *output,
		      const size_t frame_count) {
	vec3_t offset;
	vec3_t direction;
	vec3_t right;
	float distance;
	float attenuation;
	float pan;
	float left_gain;
	float right_gain;
	size_t output_frame;
	size_t source_frame;

	attenuation = 1.0f;
	pan = 0.0f;
	if (voice->params.spatial) {
		offset = vec3_subtract(voice->params.position,
				       audio->listener_position);
		distance = vec3_length(offset);
		if (distance >= voice->params.maximum_distance) {
			attenuation = 0.0f;
		} else if (distance > voice->params.minimum_distance) {
			attenuation =
				1.0f -
				(distance - voice->params.minimum_distance) /
					(voice->params.maximum_distance -
					 voice->params.minimum_distance);
		}
		if (distance > 0.000001f) {
			direction = vec3_scale(offset, 1.0f / distance);
			right = vec3_normalize(vec3_cross(
				audio->listener_forward, audio->listener_up));
			pan = fmaxf(-1.0f,
				    fminf(1.0f, vec3_dot(direction, right)));
		}
	}
	if (voice->params.spatial) {
		left_gain = voice->params.volume * attenuation *
			    sqrtf((1.0f - pan) * 0.5f);
		right_gain = voice->params.volume * attenuation *
			     sqrtf((1.0f + pan) * 0.5f);
	} else {
		left_gain = voice->params.volume;
		right_gain = voice->params.volume;
	}
	for (output_frame = 0; output_frame < frame_count; output_frame++) {
		if (voice->frame >= voice->sound->frame_count) {
			if (!voice->params.looping) {
				voice->active = false;
				break;
			}
			voice->frame = 0;
		}
		source_frame = voice->frame++ * AUDIO_CHANNEL_COUNT;
		output[output_frame * 2] +=
			voice->sound->samples[source_frame] * left_gain;
		output[output_frame * 2 + 1] +=
			voice->sound->samples[source_frame + 1] * right_gain;
	}
}

static audio_voice_t *find_voice(audio_system_t *audio,
				 const audio_voice_id_t id) {
	size_t index;

	for (index = 0; index < AUDIO_MAXIMUM_VOICES; index++) {
		if (audio->voices[index].active &&
		    audio->voices[index].id == id) {
			return &audio->voices[index];
		}
	}
	return NULL;
}
