/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "audio/sound_internal.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define COBJMACROS
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <objbase.h>
#include <windows.h>

static void set_error(char *error, size_t error_size, const char *format, ...);
static wchar_t *utf8_to_wide(const char *text);
static bool append_samples(float **samples,
			   size_t *sample_count,
			   const BYTE *data,
			   DWORD byte_count);

audio_sound_t *
audio_sound_load_mp3(const char *path, char *error, const size_t error_size) {
	IMFSourceReader *reader = NULL;
	IMFMediaType *media_type = NULL;
	IMFSample *sample = NULL;
	IMFMediaBuffer *buffer = NULL;
	audio_sound_t *sound = NULL;
	float *samples = NULL;
	size_t sample_count = 0;
	wchar_t *wide_path = NULL;
	BYTE *data = NULL;
	DWORD byte_count = 0;
	DWORD flags = 0;
	HRESULT result;
	HRESULT com_result;
	bool media_foundation_started = false;
	bool buffer_locked = false;
	bool com_initialized = false;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }
	if (path == NULL || path[0] == '\0') {
		set_error(error, error_size, "invalid MP3 path");
		return NULL;
	}
	wide_path = utf8_to_wide(path);
	if (wide_path == NULL) {
		set_error(error, error_size, "failed to convert MP3 path");
		goto cleanup;
	}
	com_result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	com_initialized = SUCCEEDED(com_result);
	if (FAILED(com_result) && com_result != RPC_E_CHANGED_MODE) {
		set_error(error, error_size,
			  "failed to initialize COM: 0x%08lx",
			  (unsigned long)com_result);
		goto cleanup;
	}
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	if (FAILED(result)) {
		set_error(error, error_size,
			  "failed to initialize Media Foundation: 0x%08lx",
			  (unsigned long)result);
		goto cleanup;
	}
	media_foundation_started = true;
	result = MFCreateSourceReaderFromURL(wide_path, NULL, &reader);
	if (FAILED(result)) {
		set_error(error, error_size, "failed to open MP3: 0x%08lx",
			  (unsigned long)result);
		goto cleanup;
	}
	result = MFCreateMediaType(&media_type);
	if (SUCCEEDED(result)) {
		result = IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE,
					      &MFMediaType_Audio);
	}
	if (SUCCEEDED(result)) {
		result = IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE,
					      &MFAudioFormat_Float);
	}
	if (SUCCEEDED(result)) {
		result = IMFMediaType_SetUINT32(media_type,
						&MF_MT_AUDIO_NUM_CHANNELS,
						AUDIO_CHANNEL_COUNT);
	}
	if (SUCCEEDED(result)) {
		result = IMFMediaType_SetUINT32(media_type,
						&MF_MT_AUDIO_SAMPLES_PER_SECOND,
						AUDIO_SAMPLE_RATE);
	}
	if (SUCCEEDED(result)) {
		result = IMFMediaType_SetUINT32(
			media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
	}
	if (SUCCEEDED(result)) {
		result = IMFSourceReader_SetCurrentMediaType(
			reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL,
			media_type);
	}
	if (FAILED(result)) {
		set_error(error, error_size,
			  "failed to configure MP3 decoder: 0x%08lx",
			  (unsigned long)result);
		goto cleanup;
	}
	for (;;) {
		result = IMFSourceReader_ReadSample(
			reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL,
			&flags, NULL, &sample);
		if (FAILED(result)) {
			set_error(error, error_size,
				  "failed to decode MP3: 0x%08lx",
				  (unsigned long)result);
			goto cleanup;
		}
		if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) { break; }
		if (sample == NULL) { continue; }
		result = IMFSample_ConvertToContiguousBuffer(sample, &buffer);
		if (FAILED(result)) {
			set_error(error, error_size,
				  "failed to read decoded MP3: 0x%08lx",
				  (unsigned long)result);
			goto cleanup;
		}
		result = IMFMediaBuffer_Lock(buffer, &data, NULL, &byte_count);
		if (FAILED(result)) {
			set_error(error, error_size,
				  "failed to lock decoded MP3: 0x%08lx",
				  (unsigned long)result);
			goto cleanup;
		}
		buffer_locked = true;
		if (!append_samples(&samples, &sample_count, data,
				    byte_count)) {
			set_error(error, error_size,
				  "failed to allocate decoded MP3 samples");
			goto cleanup;
		}
		IMFMediaBuffer_Unlock(buffer);
		buffer_locked = false;
		IMFMediaBuffer_Release(buffer);
		buffer = NULL;
		IMFSample_Release(sample);
		sample = NULL;
	}
	if (sample_count == 0 || sample_count % AUDIO_CHANNEL_COUNT != 0) {
		set_error(error, error_size, "decoded MP3 contains no audio");
		goto cleanup;
	}
	sound = calloc(1, sizeof(*sound));
	if (sound == NULL) {
		set_error(error, error_size, "failed to allocate MP3 sound");
		goto cleanup;
	}
	sound->samples = samples;
	sound->frame_count = sample_count / AUDIO_CHANNEL_COUNT;
	samples = NULL;

cleanup:
	if (buffer_locked) { IMFMediaBuffer_Unlock(buffer); }
	if (buffer != NULL) { IMFMediaBuffer_Release(buffer); }
	if (sample != NULL) { IMFSample_Release(sample); }
	if (media_type != NULL) { IMFMediaType_Release(media_type); }
	if (reader != NULL) { IMFSourceReader_Release(reader); }
	if (media_foundation_started) { MFShutdown(); }
	if (com_initialized) { CoUninitialize(); }
	free(wide_path);
	free(samples);
	return sound;
}

static wchar_t *utf8_to_wide(const char *text) {
	wchar_t *wide;
	int length;

	length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1,
				     NULL, 0);
	if (length <= 0) { return NULL; }
	wide = malloc((size_t)length * sizeof(*wide));
	if (wide == NULL) { return NULL; }
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, wide,
				length) <= 0) {
		free(wide);
		return NULL;
	}
	return wide;
}

static bool append_samples(float **samples,
			   size_t *sample_count,
			   const BYTE *data,
			   const DWORD byte_count) {
	float *resized;
	size_t added_count;

	if (byte_count == 0) { return true; }
	if (byte_count % sizeof(float) != 0) { return false; }
	added_count = byte_count / sizeof(float);
	if (*sample_count > SIZE_MAX - added_count ||
	    *sample_count + added_count > SIZE_MAX / sizeof(float)) {
		return false;
	}
	resized = realloc(*samples,
			  (*sample_count + added_count) * sizeof(float));
	if (resized == NULL) { return false; }
	memcpy(resized + *sample_count, data, byte_count);
	*samples = resized;
	*sample_count += added_count;
	return true;
}

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }
	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}
#else
audio_sound_t *
audio_sound_load_mp3(const char *path, char *error, const size_t error_size) {
	(void)path;
	if (error != NULL && error_size > 0) {
		snprintf(error, error_size,
			 "MP3 decoding is not available on this platform");
	}
	return NULL;
}
#endif
