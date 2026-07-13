/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "volume.h"
#include "core/log.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include <stdlib.h>

struct volumeEngine {
	platform_t *platform;
	renderer_t *renderer;
	const game_t *game;
	bool running;
	bool initialized;
	double previous_time;
};

engine_t *engine_create(const engine_config_t *config) {
	engine_t *engine;
	platform_config_t platform_config;

	if (config == NULL || config->application_name == NULL ||
	    config->game == NULL) {
		log_error("Invalid engine configuration");
		return NULL;
	}

	engine = calloc(1, sizeof(*engine));
	if (engine == NULL) {
		log_error("Failed to allocate engine");
		return NULL;
	}

	platform_config.title = config->application_name;
	platform_config.width = config->window_width;
	platform_config.height = config->window_height;

	engine->platform = platform_create(&platform_config);
	if (engine->platform == NULL) {
		free(engine);
		return NULL;
	}

	engine->renderer = renderer_create(engine->platform);
	if (engine->renderer == NULL) {
		platform_destroy(engine->platform);
		free(engine);
		return NULL;
	}

	engine->game = config->game;

	if (engine->game->initialize != NULL &&
	    !engine->game->initialize(engine, engine->game->user_data)) {
		renderer_destroy(engine->renderer);
		platform_destroy(engine->platform);
		free(engine);
		return NULL;
	}

	engine->initialized = true;
	engine->running = true;
	engine->previous_time = platform_get_time();

	log_info("Volume initialized");

	return engine;
}

void engine_destroy(engine_t *engine) {
	if (engine == NULL) { return; }

	if (engine->initialized && engine->game != NULL &&
	    engine->game->shutdown != NULL) {
		engine->game->shutdown(engine, engine->game->user_data);
	}

	renderer_destroy(engine->renderer);
	platform_destroy(engine->platform);
	free(engine);

	log_info("Volume shut down");
}

bool engine_run(engine_t *engine) {
	double current_time;
	float delta_time;

	if (engine == NULL) { return false; }

	while (engine->running) {
		if (!platform_poll_events(engine->platform)) {
			engine->running = false;
			continue;
		}

		current_time = platform_get_time();
		delta_time = (float)(current_time - engine->previous_time);
		engine->previous_time = current_time;

		if (engine->game->update != NULL) {
			engine->game->update(engine, delta_time,
					     engine->game->user_data);
		}

		renderer_begin_frame(engine->renderer);

		if (engine->game->render != NULL) {
			engine->game->render(engine, engine->game->user_data);
		}

		renderer_end_frame(engine->renderer);
	}

	return true;
}

renderer_t *engine_get_renderer(engine_t *engine) {
	if (engine == NULL) {
		return NULL; }

	return engine->renderer;
}