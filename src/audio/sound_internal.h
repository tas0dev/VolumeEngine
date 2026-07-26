/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_AUDIO_SOUND_INTERNAL_H
#define VOLUME_AUDIO_SOUND_INTERNAL_H

#include "audio/sound.h"
#include <stddef.h>

enum {
	AUDIO_SAMPLE_RATE = 48000,
	AUDIO_CHANNEL_COUNT = 2,
};

struct audio_sound {
	float *samples;
	size_t frame_count;
};

#endif
