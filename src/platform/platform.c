/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "platform.h"
#include "core/log.h"
#include "platform/platform.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

struct platform {
	SDL_Window *window;
	bool running;
};

platform_t *platform_create(const platform_config_t *config) {
	if (config == NULL || config->title == NULL) {
		log_error("Invalid platform configuration");
		return NULL;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		log_error("SDL initialization failed: %s", SDL_GetError());
		return NULL;
	}

	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
	    !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
	    !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
				 SDL_GL_CONTEXT_PROFILE_CORE)) {
		log_error("Failed to configure OpenGL context: %s",
			  SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	if (!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) ||
	    !SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) ||
	    !SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8)) {
		log_error("Failed to configure framebuffer: %s",
			  SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	platform_t *platform = calloc(1, sizeof(*platform));
	if (platform == NULL) {
		log_error("Failed to allocate platform");
		SDL_Quit();
		return NULL;
	}

	platform->window =
		SDL_CreateWindow(config->title, config->width, config->height,
				 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (platform->window == NULL) {
		log_error("Window creation failed: %s", SDL_GetError());
		free(platform);
		SDL_Quit();
		return NULL;
	}

	platform->running = true;

	log_info("Created window: %dx%d", config->width, config->height);

	return platform;
}

void platform_destroy(platform_t *platform) {
	if (platform == NULL) { return; }

	if (platform->window != NULL) { SDL_DestroyWindow(platform->window); }

	free(platform);
	SDL_Quit();
}

bool platform_poll_events(platform_t *platform) {
	SDL_Event event;

	if (platform == NULL) { return false; }

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT: platform->running = false; break;

		case SDL_EVENT_KEY_DOWN:
			if (event.key.key == SDLK_ESCAPE) {
				platform->running = false;
			}
			break;

		default: break;
		}
	}

	return platform->running;
}

void *platform_gl_create_context(platform_t *platform) {
	if (platform == NULL || platform->window == NULL) { return NULL; }

	const SDL_GLContext context = SDL_GL_CreateContext(platform->window);
	if (context == NULL) {
		log_error("OpenGL context creation failed: %s", SDL_GetError());
		return NULL;
	}

	return context;
}

void platform_gl_destroy_context(void *context) {
	if (context == NULL) { return; }

	SDL_GL_DestroyContext(context);
}

bool platform_gl_make_current(const platform_t *platform, void *context) {
	if (platform == NULL || platform->window == NULL || context == NULL) {
		return false;
	}

	if (!SDL_GL_MakeCurrent(platform->window, (SDL_GLContext)context)) {
		log_error("Failed to activate OpenGL context: %s",
			  SDL_GetError());
		return false;
	}

	return true;
}

void platform_gl_swap_buffers(const platform_t *platform) {
	if (platform == NULL || platform->window == NULL) { return; }

	SDL_GL_SwapWindow(platform->window);
}

void platform_get_drawable_size(const platform_t *platform,
				int *width,
				int *height) {
	if (width != NULL) { *width = 0; }

	if (height != NULL) { *height = 0; }

	if (platform == NULL || platform->window == NULL) { return; }

	SDL_GetWindowSizeInPixels(platform->window, width, height);
}

double platform_get_time(void) {
	return (double)SDL_GetTicksNS() / 1000000000.0;
}

void platform_sleep(const double seconds) {
	if (seconds <= 0.0) { return; }

	const Uint64 nanoseconds = (Uint64)(seconds * 1000000000.0);
	SDL_DelayNS(nanoseconds);
}