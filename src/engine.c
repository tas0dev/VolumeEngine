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

	engine->running = true;
	engine->previous_time = platform_get_time();

	log_info("Volume initialized");

	return engine;
}

void engine_destroy(engine_t *engine) {
	if (engine == NULL) { return; }

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
		const double delta_time = current_time - engine->previous_time;
		engine->previous_time = current_time;

		(void)delta_time;

		renderer_begin_frame(engine->renderer);
		renderer_draw(engine->renderer);
		renderer_end_frame(engine->renderer);
	}

	return true;
}