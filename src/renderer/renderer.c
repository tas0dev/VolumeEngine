#include "renderer/renderer.h"
#include "core/log.h"
#include "platform/platform.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <stdlib.h>

struct renderer {
	platform_t *platform;
	void *context;
};

renderer_t *renderer_create(platform_t *platform) {
	if (platform == NULL) {
		log_error("Cannot create renderer without a platform");
		return NULL;
	}

	renderer_t *renderer = calloc(1, sizeof(*renderer));
	if (renderer == NULL) {
		log_error("Failed to allocate renderer");
		return NULL;
	}

	renderer->platform = platform;
	renderer->context = platform_gl_create_context(platform);

	if (renderer->context == NULL) {
		free(renderer);
		return NULL;
	}

	if (!platform_gl_make_current(platform, renderer->context)) {
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	if (!SDL_GL_SetSwapInterval(1)) {
		log_info("Failed to enable VSync: %s", SDL_GetError());
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	log_info("OpenGL renderer initialized: %s",
		 (const char *)glGetString(GL_VERSION));

	return renderer;
}

void renderer_destroy(renderer_t *renderer) {
	if (renderer == NULL) { return; }

	platform_gl_destroy_context(renderer->context);
	free(renderer);

	log_info("Renderer shut down");
}

void renderer_begin_frame(const renderer_t *renderer) {
	int width;
	int height;

	if (renderer == NULL) { return; }

	platform_get_drawable_size(renderer->platform, &width, &height);

	glViewport(0, 0, width, height);
	glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(const renderer_t *renderer) {
	if (renderer == NULL) { return; }

	platform_gl_swap_buffers(renderer->platform);
}