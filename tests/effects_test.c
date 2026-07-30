/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "fps/effects.h"

int main(void) {
	fps_effect_system_t *system;
	fps_billboard_effect_config_t billboard = {0};
	fps_tracer_effect_config_t tracer = {0};

	system = fps_effect_system_create();
	CHECK(system != NULL);
	billboard.position = vec3_create(1.0f, 2.0f, 3.0f);
	billboard.start_size = 0.2f;
	billboard.end_size = 0.1f;
	billboard.lifetime = 1.0f;
	billboard.start_color = (renderer_color_t){1.0f, 0.5f, 0.0f, 1.0f};
	billboard.end_color = (renderer_color_t){1.0f, 0.0f, 0.0f, 0.0f};
	billboard.blend_mode = RENDERER_BLEND_ADDITIVE;
	CHECK(fps_effect_system_spawn_billboard(system, &billboard));
	tracer.start = vec3_create(0.0f, 0.0f, 0.0f);
	tracer.end = vec3_create(0.0f, 0.0f, -10.0f);
	tracer.start_width = 0.04f;
	tracer.end_width = 0.01f;
	tracer.lifetime = 0.1f;
	tracer.start_color = (renderer_color_t){1.0f, 1.0f, 1.0f, 1.0f};
	tracer.end_color = (renderer_color_t){1.0f, 1.0f, 1.0f, 0.0f};
	CHECK(fps_effect_system_spawn_tracer(system, &tracer));
	CHECK(fps_effect_system_get_count(system) == 2);
	fps_effect_system_update(system, 0.11f);
	CHECK(fps_effect_system_get_count(system) == 1);
	fps_effect_system_update(system, 0.89f);
	CHECK(fps_effect_system_get_count(system) == 0);
	billboard.lifetime = 0.0f;
	CHECK(!fps_effect_system_spawn_billboard(system, &billboard));
	tracer.end = tracer.start;
	CHECK(!fps_effect_system_spawn_tracer(system, &tracer));
	tracer.end = vec3_create(0.0f, 0.0f, -10.0f);
	tracer.start_width = -0.01f;
	CHECK(!fps_effect_system_spawn_tracer(system, &tracer));
	fps_effect_system_destroy(system);
	return 0;
}
