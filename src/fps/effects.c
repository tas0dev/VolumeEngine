/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "fps/effects.h"
#include <math.h>
#include <stdlib.h>

#define FPS_EFFECT_MAX_BILLBOARDS 128
#define FPS_EFFECT_MAX_TRACERS 128

typedef struct fps_billboard_effect {
	fps_billboard_effect_config_t config;
	float age;
} fps_billboard_effect_t;

typedef struct fps_tracer_effect {
	fps_tracer_effect_config_t config;
	float age;
} fps_tracer_effect_t;

struct fps_effect_system {
	fps_billboard_effect_t billboards[FPS_EFFECT_MAX_BILLBOARDS];
	size_t billboard_count;
	fps_tracer_effect_t tracers[FPS_EFFECT_MAX_TRACERS];
	size_t tracer_count;
};

static float lerp(float start, float end, float amount);
static renderer_color_t
lerp_color(renderer_color_t start, renderer_color_t end, float amount);
static bool color_is_finite(renderer_color_t color);
static bool vector_is_finite(vec3_t vector);

fps_effect_system_t *fps_effect_system_create(void) {
	return calloc(1, sizeof(fps_effect_system_t));
}

void fps_effect_system_destroy(fps_effect_system_t *system) { free(system); }

void fps_effect_system_clear(fps_effect_system_t *system) {
	if (system == NULL) { return; }
	system->billboard_count = 0;
	system->tracer_count = 0;
}

void fps_effect_system_update(fps_effect_system_t *system,
			      const float delta_time) {
	size_t read_index;
	size_t write_index;

	if (system == NULL || !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	write_index = 0;
	for (read_index = 0; read_index < system->billboard_count;
	     read_index++) {
		system->billboards[read_index].age += delta_time;
		if (system->billboards[read_index].age <
		    system->billboards[read_index].config.lifetime) {
			system->billboards[write_index++] =
				system->billboards[read_index];
		}
	}
	system->billboard_count = write_index;
	write_index = 0;
	for (read_index = 0; read_index < system->tracer_count; read_index++) {
		system->tracers[read_index].age += delta_time;
		if (system->tracers[read_index].age <
		    system->tracers[read_index].config.lifetime) {
			system->tracers[write_index++] =
				system->tracers[read_index];
		}
	}
	system->tracer_count = write_index;
}

bool fps_effect_system_spawn_billboard(
	fps_effect_system_t *system,
	const fps_billboard_effect_config_t *config) {
	fps_billboard_effect_t *effect;

	if (system == NULL || config == NULL ||
	    system->billboard_count >= FPS_EFFECT_MAX_BILLBOARDS ||
	    !isfinite(config->start_size) || config->start_size <= 0.0f ||
	    !isfinite(config->end_size) || config->end_size <= 0.0f ||
	    !isfinite(config->lifetime) || config->lifetime <= 0.0f ||
	    !vector_is_finite(config->position) ||
	    !vector_is_finite(config->normal) ||
	    !color_is_finite(config->start_color) ||
	    !color_is_finite(config->end_color)) {
		return false;
	}
	effect = &system->billboards[system->billboard_count++];
	effect->config = *config;
	effect->age = 0.0f;
	return true;
}

bool fps_effect_system_spawn_tracer(fps_effect_system_t *system,
				    const fps_tracer_effect_config_t *config) {
	fps_tracer_effect_t *effect;

	if (system == NULL || config == NULL ||
	    system->tracer_count >= FPS_EFFECT_MAX_TRACERS ||
	    !isfinite(config->start_width) || config->start_width < 0.0f ||
	    !isfinite(config->end_width) || config->end_width < 0.0f ||
	    !isfinite(config->lifetime) || config->lifetime <= 0.0f ||
	    !vector_is_finite(config->start) ||
	    !vector_is_finite(config->end) ||
	    vec3_length(vec3_subtract(config->end, config->start)) <=
		    0.000001f ||
	    !color_is_finite(config->start_color) ||
	    !color_is_finite(config->end_color)) {
		return false;
	}
	effect = &system->tracers[system->tracer_count++];
	effect->config = *config;
	if (effect->config.start_width == 0.0f) {
		effect->config.start_width = 0.012f;
	}
	if (effect->config.end_width == 0.0f) {
		effect->config.end_width = effect->config.start_width;
	}
	effect->age = 0.0f;
	return true;
}

void fps_effect_system_draw(const fps_effect_system_t *system,
			    renderer_t *renderer,
			    const render_view_t *view) {
	size_t index;
	float amount;
	float width;
	renderer_color_t color;
	renderer_color_t glow_color;
	vec3_t normal;
	vec3_t reference;
	vec3_t right;
	vec3_t up;

	if (system == NULL || renderer == NULL || view == NULL) { return; }
	for (index = 0; index < system->tracer_count; index++) {
		amount = system->tracers[index].age /
			 system->tracers[index].config.lifetime;
		color = lerp_color(system->tracers[index].config.start_color,
				   system->tracers[index].config.end_color,
				   amount);
		width = lerp(system->tracers[index].config.start_width,
			     system->tracers[index].config.end_width, amount);
		glow_color = color;
		glow_color.a *= 0.28f;
		renderer_draw_world_beam(
			renderer, system->tracers[index].config.start,
			system->tracers[index].config.end, width * 3.0f,
			glow_color, RENDERER_BLEND_ADDITIVE, view);
		renderer_draw_world_beam(
			renderer, system->tracers[index].config.start,
			system->tracers[index].config.end, width, color,
			system->tracers[index].config.blend_mode, view);
	}
	for (index = 0; index < system->billboard_count; index++) {
		amount = system->billboards[index].age /
			 system->billboards[index].config.lifetime;
		color = lerp_color(system->billboards[index].config.start_color,
				   system->billboards[index].config.end_color,
				   amount);
		if (vec3_length(system->billboards[index].config.normal) <=
		    0.000001f) {
			renderer_draw_world_billboard(
				renderer,
				system->billboards[index].config.position,
				lerp(system->billboards[index]
					     .config.start_size,
				     system->billboards[index].config.end_size,
				     amount),
				color,
				system->billboards[index].config.blend_mode,
				view);
			continue;
		}
		normal =
			vec3_normalize(system->billboards[index].config.normal);
		reference = fabsf(normal.y) < 0.9f
				    ? vec3_create(0.0f, 1.0f, 0.0f)
				    : vec3_create(1.0f, 0.0f, 0.0f);
		right = vec3_normalize(vec3_cross(normal, reference));
		up = vec3_normalize(vec3_cross(right, normal));
		renderer_draw_world_sprite(
			renderer, system->billboards[index].config.position,
			right, up,
			lerp(system->billboards[index].config.start_size,
			     system->billboards[index].config.end_size, amount),
			color, system->billboards[index].config.blend_mode,
			view);
	}
}

size_t fps_effect_system_get_count(const fps_effect_system_t *system) {
	return system == NULL ? 0
			      : system->billboard_count + system->tracer_count;
}

static float lerp(const float start, const float end, const float amount) {
	return start + (end - start) * amount;
}

static renderer_color_t lerp_color(const renderer_color_t start,
				   const renderer_color_t end,
				   const float amount) {
	return (renderer_color_t){
		lerp(start.r, end.r, amount), lerp(start.g, end.g, amount),
		lerp(start.b, end.b, amount), lerp(start.a, end.a, amount)};
}

static bool color_is_finite(const renderer_color_t color) {
	return isfinite(color.r) && isfinite(color.g) && isfinite(color.b) &&
	       isfinite(color.a);
}

static bool vector_is_finite(const vec3_t vector) {
	return isfinite(vector.x) && isfinite(vector.y) && isfinite(vector.z);
}
