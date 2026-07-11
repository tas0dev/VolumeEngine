/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/hdr_buffer.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct hdr_buffer {
	GLuint framebuffer;
	GLuint color_texture;
	GLuint depth_buffer;
	int width;
	int height;
};

static bool hdr_buffer_allocate(hdr_buffer_t *buffer, int width, int height) {
	GLenum status;

	glBindTexture(GL_TEXTURE_2D, buffer->color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
		     GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width,
			      height);

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, buffer->color_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_RENDERBUFFER, buffer->depth_buffer);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		log_error("HDR framebuffer is incomplete: 0x%x", status);
		return false;
	}

	buffer->width = width;
	buffer->height = height;

	return true;
}

hdr_buffer_t *hdr_buffer_create(int width, int height) {
	hdr_buffer_t *buffer;

	if (width <= 0 || height <= 0) {
		log_error("Invalid HDR framebuffer size: %dx%d", width, height);
		return NULL;
	}

	buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		log_error("Failed to allocate HDR framebuffer");
		return NULL;
	}

	glGenFramebuffers(1, &buffer->framebuffer);
	glGenTextures(1, &buffer->color_texture);
	glGenRenderbuffers(1, &buffer->depth_buffer);

	if (!hdr_buffer_allocate(buffer, width, height)) {
		hdr_buffer_destroy(buffer);
		return NULL;
	}

	log_info("Created HDR framebuffer: %dx%d", width, height);

	return buffer;
}

void hdr_buffer_destroy(hdr_buffer_t *buffer) {
	if (buffer == NULL) { return; }

	if (buffer->depth_buffer != 0) {
		glDeleteRenderbuffers(1, &buffer->depth_buffer);
	}

	if (buffer->color_texture != 0) {
		glDeleteTextures(1, &buffer->color_texture);
	}

	if (buffer->framebuffer != 0) {
		glDeleteFramebuffers(1, &buffer->framebuffer);
	}

	free(buffer);
}

bool hdr_buffer_resize(hdr_buffer_t *buffer, int width, int height) {
	if (buffer == NULL || width <= 0 || height <= 0) { return false; }

	if (buffer->width == width && buffer->height == height) { return true; }

	if (!hdr_buffer_allocate(buffer, width, height)) { return false; }

	log_info("Resized HDR framebuffer: %dx%d", width, height);

	return true;
}

void hdr_buffer_bind(const hdr_buffer_t *buffer) {
	if (buffer == NULL) { return; }

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
}

void hdr_buffer_unbind(void) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int hdr_buffer_get_texture(const hdr_buffer_t *buffer) {
	if (buffer == NULL) { return 0; }

	return buffer->color_texture;
}
