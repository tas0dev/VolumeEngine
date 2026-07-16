/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "renderer/bloom_buffer.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct bloom_buffer {
	GLuint framebuffers[2];
	GLuint textures[2];
	int width;
	int height;
};

static bool bloom_buffer_allocate(bloom_buffer_t *buffer,
				  const int width,
				  const int height) {
	int index;

	for (index = 0; index < 2; index++) {
		GLenum status;

		glBindTexture(GL_TEXTURE_2D, buffer->textures[index]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
			     GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffers[index]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, buffer->textures[index],
				       0);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			log_error("Bloom framebuffer %d is incomplete: 0x%x",
				  index, status);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			return false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	buffer->width = width;
	buffer->height = height;

	return true;
}

bloom_buffer_t *bloom_buffer_create(const int width, const int height) {
	bloom_buffer_t *buffer;

	if (width <= 0 || height <= 0) {
		log_error("Invalid bloom buffer size: %dx%d", width, height);
		return NULL;
	}

	buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		log_error("Failed to allocate bloom buffer");
		return NULL;
	}

	glGenFramebuffers(2, buffer->framebuffers);
	glGenTextures(2, buffer->textures);

	if (!bloom_buffer_allocate(buffer, width, height)) {
		bloom_buffer_destroy(buffer);
		return NULL;
	}

	log_info("Created bloom buffer: %dx%d", width, height);

	return buffer;
}

void bloom_buffer_destroy(bloom_buffer_t *buffer) {
	if (buffer == NULL) { return; }

	glDeleteTextures(2, buffer->textures);
	glDeleteFramebuffers(2, buffer->framebuffers);
	free(buffer);
}

bool bloom_buffer_resize(bloom_buffer_t *buffer,
			 const int width,
			 const int height) {
	if (buffer == NULL || width <= 0 || height <= 0) { return false; }

	if (buffer->width == width && buffer->height == height) { return true; }

	if (!bloom_buffer_allocate(buffer, width, height)) { return false; }

	log_info("Resized bloom buffer: %dx%d", width, height);

	return true;
}

void bloom_buffer_bind(const bloom_buffer_t *buffer, const int index) {
	if (buffer == NULL || index < 0 || index >= 2) { return; }

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffers[index]);
}

unsigned int bloom_buffer_get_texture(const bloom_buffer_t *buffer,
				      const int index) {
	if (buffer == NULL || index < 0 || index >= 2) { return 0; }

	return buffer->textures[index];
}