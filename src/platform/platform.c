#include "platform/platform.h"
#include "core/log.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

struct platform {
	SDL_Window *window;
	bool running;
};

platform_t *
platform_create(const platform_config_t *config) {
	platform_t *platform;

	if (config == NULL || config->title == NULL) {
		log_error("Invalid platform configuration");
		return NULL;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		log_error("SDL initialization failed: %s",
		          SDL_GetError());
		return NULL;
	}

	platform = calloc(1, sizeof(*platform));
	if (platform == NULL) {
		log_error("Failed to allocate platform");
		SDL_Quit();
		return NULL;
	}

	platform->window = SDL_CreateWindow(
		config->title,
		config->width,
		config->height,
		SDL_WINDOW_RESIZABLE
	);

	if (platform->window == NULL) {
		log_error("Window creation failed: %s",
		          SDL_GetError());
		free(platform);
		SDL_Quit();
		return NULL;
	}

	platform->running = true;

	log_info(
		"Created window: %dx%d",
		config->width,
		config->height
	);

	return platform;
}

void
platform_destroy(platform_t *platform) {
	if (platform == NULL) { return; }

	if (platform->window != NULL) { SDL_DestroyWindow(platform->window); }

	free(platform);
	SDL_Quit();
}

bool
platform_poll_events(platform_t *platform) {
	SDL_Event event;

	if (platform == NULL) { return false; }

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT: platform->running = false;
			break;

		case SDL_EVENT_KEY_DOWN: if (event.key.key == SDLK_ESCAPE) {
				platform->running =
						false;
			}
			break;

		default: break;
		}
	}

	return platform->running;
}

double
platform_get_time(void) { return (double)SDL_GetTicksNS() / 1000000000.0; }

void
platform_sleep(double seconds) {
	Uint64 nanoseconds;

	if (seconds <= 0.0) { return; }

	nanoseconds = (Uint64)(seconds * 1000000000.0);
	SDL_DelayNS(nanoseconds);
}