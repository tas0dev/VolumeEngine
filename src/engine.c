#include "engine.h"
#include "core/log.h"
#include "platform/platform.h"
#include <stdlib.h>

struct engine {
	platform_t *platform;
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

	engine->running = true;
	engine->previous_time = platform_get_time();

	log_info("Volume initialized");

	return engine;
}

void engine_destroy(engine_t *engine) {
	if (engine == NULL) { return; }

	platform_destroy(engine->platform);
	free(engine);

	log_info("Volume shut down");
}

bool engine_run(engine_t *engine) {
	const double target_frame_time = 1.0 / 60.0;

	if (engine == NULL) { return false; }

	while (engine->running) {
		double current_time;
		double delta_time;
		double frame_end;
		double frame_time;

		if (!platform_poll_events(engine->platform)) {
			engine->running = false;
			continue;
		}

		current_time = platform_get_time();
		delta_time = current_time - engine->previous_time;
		engine->previous_time = current_time;

		(void)delta_time;

		frame_end = platform_get_time();
		frame_time = frame_end - current_time;

		if (frame_time < target_frame_time) {
			platform_sleep(target_frame_time - frame_time);
		}
	}

	return true;
}