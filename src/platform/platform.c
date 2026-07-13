/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "platform.h"
#include "core/log.h"
#include "input/input.h"
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

	if (config->capture_mouse &&
	    !SDL_SetWindowRelativeMouseMode(platform->window, true)) {
		log_error("Failed to capture mouse: %s", SDL_GetError());
		SDL_DestroyWindow(platform->window);
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

static input_key_t translate_key(const SDL_Keycode key) {
	switch (key) {
	case SDLK_A: return INPUT_KEY_A;
	case SDLK_B: return INPUT_KEY_B;
	case SDLK_C: return INPUT_KEY_C;
	case SDLK_D: return INPUT_KEY_D;
	case SDLK_E: return INPUT_KEY_E;
	case SDLK_F: return INPUT_KEY_F;
	case SDLK_G: return INPUT_KEY_G;
	case SDLK_H: return INPUT_KEY_H;
	case SDLK_I: return INPUT_KEY_I;
	case SDLK_J: return INPUT_KEY_J;
	case SDLK_K: return INPUT_KEY_K;
	case SDLK_L: return INPUT_KEY_L;
	case SDLK_M: return INPUT_KEY_M;
	case SDLK_N: return INPUT_KEY_N;
	case SDLK_O: return INPUT_KEY_O;
	case SDLK_P: return INPUT_KEY_P;
	case SDLK_Q: return INPUT_KEY_Q;
	case SDLK_R: return INPUT_KEY_R;
	case SDLK_S: return INPUT_KEY_S;
	case SDLK_T: return INPUT_KEY_T;
	case SDLK_U: return INPUT_KEY_U;
	case SDLK_V: return INPUT_KEY_V;
	case SDLK_W: return INPUT_KEY_W;
	case SDLK_X: return INPUT_KEY_X;
	case SDLK_Y: return INPUT_KEY_Y;
	case SDLK_Z: return INPUT_KEY_Z;
	case SDLK_SPACE: return INPUT_KEY_SPACE;
	case SDLK_LCTRL:
	case SDLK_RCTRL: return INPUT_KEY_CONTROL;
	case SDLK_ESCAPE: return INPUT_KEY_ESCAPE;
	default: return INPUT_KEY_COUNT;
	}
}

static input_mouse_button_t translate_mouse_button(const Uint8 button) {
	switch (button) {
	case SDL_BUTTON_LEFT: return INPUT_MOUSE_BUTTON_LEFT;
	case SDL_BUTTON_MIDDLE: return INPUT_MOUSE_BUTTON_MIDDLE;
	case SDL_BUTTON_RIGHT: return INPUT_MOUSE_BUTTON_RIGHT;
	default: return INPUT_MOUSE_BUTTON_COUNT;
	}
}

bool platform_poll_events(platform_t *platform, input_t *input) {
	SDL_Event event;

	if (platform == NULL || input == NULL) { return false; }

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT: platform->running = false; break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP: {
			const input_key_t key = translate_key(event.key.key);

			if (key != INPUT_KEY_COUNT) {
				input_set_key(input, key,
					      event.type == SDL_EVENT_KEY_DOWN);
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			const input_mouse_button_t button =
				translate_mouse_button(event.button.button);

			if (button != INPUT_MOUSE_BUTTON_COUNT) {
				input_set_mouse_button(
					input, button,
					event.type ==
						SDL_EVENT_MOUSE_BUTTON_DOWN);
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
			input_set_mouse_position(input, event.motion.x,
						 event.motion.y);
			input_add_mouse_delta(input, event.motion.xrel,
					      event.motion.yrel);
			break;

		default: break;
		}
	}

	return platform->running;
}

void *platform_gl_create_context(const platform_t *platform) {
	if (platform == NULL || platform->window == NULL) { return NULL; }

	// ReSharper disable once CppLocalVariableMayBeConst
	SDL_GLContext context = SDL_GL_CreateContext(platform->window);
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

	if (!SDL_GL_MakeCurrent(platform->window, context)) {
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

bool platform_set_mouse_captured(platform_t *platform, const bool captured) {
	if (platform == NULL || platform->window == NULL) { return false; }

	if (!SDL_SetWindowRelativeMouseMode(platform->window, captured)) {
		log_error("Failed to change mouse capture: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool platform_is_mouse_captured(const platform_t *platform) {
	if (platform == NULL || platform->window == NULL) { return false; }

	return SDL_GetWindowRelativeMouseMode(platform->window);
}