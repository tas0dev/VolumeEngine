/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "game/game.h"
#include "volume.h"
#include <stdlib.h>

int main(void) {
	engine_config_t config = {0};
	engine_t *engine;
	bool result;

	config.application_name = "Volume";
	config.window_width = 1280;
	config.window_height = 720;
	config.game = game_get();

	engine = engine_create(&config);
	if (engine == NULL) { return EXIT_FAILURE; }

	result = engine_run(engine);
	engine_destroy(engine);

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}