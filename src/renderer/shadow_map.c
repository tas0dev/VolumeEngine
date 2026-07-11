/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/shadow_map.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct shadow_map {
	GLuint framebuffer;
	GLuint depth_texture;
	int width;
	int height;
};

shadow_map_t *shadow_map_create(int width, int height) {
	shadow_map_t *shadow_map;
	GLenum framebuffer_status;

	if (width <= 0 || height <= 0) {
		log_error("Invalid shadow map size: %dx%d", width, height);
		return NULL;
	}

	shadow_map = calloc(1, sizeof(*shadow_map));
	if (shadow_map == NULL) {
		log_error("Failed to allocate shadow map");
		return NULL;
	}

	shadow_map->width = width;
	shadow_map->height = height;

	glGenFramebuffers(1, &shadow_map->framebuffer);
	glGenTextures(1, &shadow_map->depth_texture);

	glBindTexture(GL_TEXTURE_2D, shadow_map->depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	{
		static const float border_color[] = {
			1.0f,
			1.0f,
			1.0f,
			1.0f,
		};

		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
				 border_color);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			       GL_TEXTURE_2D, shadow_map->depth_texture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		log_error("Shadow framebuffer is incomplete: 0x%x",
			  framebuffer_status);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteTextures(1, &shadow_map->depth_texture);
		glDeleteFramebuffers(1, &shadow_map->framebuffer);
		free(shadow_map);

		return NULL;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	log_info("Created shadow map: %dx%d", width, height);

	return shadow_map;
}

void shadow_map_destroy(shadow_map_t *shadow_map) {
	if (shadow_map == NULL) { return; }

	if (shadow_map->depth_texture != 0) {
		glDeleteTextures(1, &shadow_map->depth_texture);
	}

	if (shadow_map->framebuffer != 0) {
		glDeleteFramebuffers(1, &shadow_map->framebuffer);
	}

	free(shadow_map);
}

void shadow_map_begin(const shadow_map_t *shadow_map) {
	if (shadow_map == NULL) { return; }

	glViewport(0, 0, shadow_map->width, shadow_map->height);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->framebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void shadow_map_end(void) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int shadow_map_get_texture(const shadow_map_t *shadow_map) {
	if (shadow_map == NULL) { return 0; }

	return shadow_map->depth_texture;
}

int shadow_map_get_width(const shadow_map_t *shadow_map) {
	if (shadow_map == NULL) { return 0; }

	return shadow_map->width;
}

int shadow_map_get_height(const shadow_map_t *shadow_map) {
	if (shadow_map == NULL) { return 0; }

	return shadow_map->height;
}