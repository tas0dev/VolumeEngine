/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#define STB_IMAGE_IMPLEMENTATION

#include "renderer/texture.h"
#include <epoxy/gl.h>
#include <stb/stb_image.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct texture {
	GLuint id;
	int width;
	int height;
};

static void set_error(char *error, size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

texture_t *
texture_load(const char *path, char *error, const size_t error_size) {
	const char *reason;
	unsigned char *pixels;
	texture_t *texture;
	int channel_count;
	int width;
	int height;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (path == NULL || path[0] == '\0') {
		set_error(error, error_size, "invalid texture path");
		return NULL;
	}

	stbi_set_flip_vertically_on_load(1);

	pixels = stbi_load(path, &width, &height, &channel_count,
			   STBI_rgb_alpha);
	if (pixels == NULL) {
		reason = stbi_failure_reason();

		set_error(error, error_size,
			  "failed to load texture \"%s\": %s", path,
			  reason != NULL ? reason : "unknown error");
		return NULL;
	}

	if (width <= 0 || height <= 0) {
		stbi_image_free(pixels);
		set_error(error, error_size,
			  "texture \"%s\" has an invalid size", path);
		return NULL;
	}

	texture = calloc(1, sizeof(*texture));
	if (texture == NULL) {
		stbi_image_free(pixels);
		set_error(error, error_size, "failed to allocate texture");
		return NULL;
	}

	texture->width = width;
	texture->height = height;

	glGenTextures(1, &texture->id);
	if (texture->id == 0) {
		stbi_image_free(pixels);
		free(texture);
		set_error(error, error_size, "failed to create OpenGL texture");
		return NULL;
	}

	glBindTexture(GL_TEXTURE_2D, texture->id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, pixels);

	glGenerateMipmap(GL_TEXTURE_2D);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(pixels);

	return texture;
}

void texture_destroy(texture_t *texture) {
	if (texture == NULL) { return; }

	if (texture->id != 0) { glDeleteTextures(1, &texture->id); }

	free(texture);
}

void texture_bind(const texture_t *texture, const unsigned int unit) {
	if (texture == NULL) { return; }

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_unbind(const unsigned int unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);
}
