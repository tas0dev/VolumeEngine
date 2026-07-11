#include "engine.h"
#include <stddef.h>

int main(void) {
	const engine_config_t config = {.application_name = "Volume",
					.window_width = 800,
					.window_height = 600};

	engine_t *engine = engine_create(&config);
	if (engine == NULL) { return 1; }

	engine_run(engine);

	engine_destroy(engine);
	return 0;
}
