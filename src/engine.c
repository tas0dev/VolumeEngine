/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "engine.h"
#include "core/log.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include <stdlib.h>

struct engine {
	platform_t *platform;
	renderer_t *renderer;
	bool running;
	double previous_time;
	engine_initialize_fn initialize;
	engine_update_fn update;
	engine_render_fn render;
	engine_shutdown_fn shutdown;
	void *user_data;
	bool initialized;
};

engine_t *engine_create(const engine_config_t *config) {
	platform_config_t platform_config;
	if (config == NULL || config->application_name == NULL) {
		log_error("Invalid engine configuration");
		return NULL;
	}

	engine_t *engine = calloc(1, sizeof(*engine));
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

	engine->initialize = config->initialize;
	engine->update = config->update;
	engine->render = config->render;
	engine->shutdown = config->shutdown;
	engine->user_data = config->user_data;

	if (engine->initialize != NULL &&
	    !engine->initialize(engine, engine->user_data)) {
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

	if (engine->initialized && engine->shutdown != NULL) {
		engine->shutdown(engine, engine->user_data);
	}

	renderer_destroy(engine->renderer);
	platform_destroy(engine->platform);
	free(engine);

	log_info("Volume shut down");
}

bool engine_run(engine_t *engine) {
	if (engine == NULL) { return false; }

	while (engine->running) {

		if (!platform_poll_events(engine->platform)) {
			engine->running = false;
			continue;
		}

		const double current_time = platform_get_time();
		const float delta_time =
			(float)(current_time - engine->previous_time);
		engine->previous_time = current_time;

		if (engine->update != NULL) {
			engine->update(engine, delta_time, engine->user_data);
		}

		renderer_begin_frame(engine->renderer);

		if (engine->render != NULL) {
			engine->render(engine, engine->user_data);
		}

		renderer_end_frame(engine->renderer);
	}

	return true;
}

renderer_t *engine_get_renderer(engine_t *engine) {
	if (engine == NULL) { return NULL; }

	return engine->renderer;
}