/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "fps/recoil.h"
#include <math.h>
#include <stddef.h>

static float clamp(float value, float minimum, float maximum);

bool fps_recoil_initialize(fps_recoil_t *recoil,
			   const fps_recoil_config_t *config) {
	if (recoil == NULL || config == NULL ||
	    !isfinite(config->return_strength) ||
	    config->return_strength <= 0.0f || !isfinite(config->damping) ||
	    config->damping <= 0.0f || !isfinite(config->maximum_pitch) ||
	    config->maximum_pitch <= 0.0f || !isfinite(config->maximum_yaw) ||
	    config->maximum_yaw <= 0.0f) {
		return false;
	}
	*recoil = (fps_recoil_t){0};
	recoil->config = *config;
	return true;
}

void fps_recoil_add_impulse(fps_recoil_t *recoil,
			    const float pitch,
			    const float yaw) {
	if (recoil == NULL || !isfinite(pitch) || !isfinite(yaw)) { return; }
	recoil->velocity.pitch += pitch;
	recoil->velocity.yaw += yaw;
}

void fps_recoil_update(fps_recoil_t *recoil, float delta_time) {
	float damping;

	if (recoil == NULL || !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	if (delta_time > 0.05f) { delta_time = 0.05f; }
	recoil->velocity.pitch -= recoil->offset.pitch *
				  recoil->config.return_strength * delta_time;
	recoil->velocity.yaw -= recoil->offset.yaw *
				recoil->config.return_strength * delta_time;
	damping = expf(-recoil->config.damping * delta_time);
	recoil->velocity.pitch *= damping;
	recoil->velocity.yaw *= damping;
	recoil->offset.pitch += recoil->velocity.pitch * delta_time;
	recoil->offset.yaw += recoil->velocity.yaw * delta_time;
	recoil->offset.pitch =
		clamp(recoil->offset.pitch, -recoil->config.maximum_pitch,
		      recoil->config.maximum_pitch);
	recoil->offset.yaw =
		clamp(recoil->offset.yaw, -recoil->config.maximum_yaw,
		      recoil->config.maximum_yaw);
}

fps_recoil_offset_t fps_recoil_get_offset(const fps_recoil_t *recoil) {
	return recoil == NULL ? (fps_recoil_offset_t){0} : recoil->offset;
}

void fps_recoil_reset(fps_recoil_t *recoil) {
	if (recoil == NULL) { return; }
	recoil->offset = (fps_recoil_offset_t){0};
	recoil->velocity = (fps_recoil_offset_t){0};
}

static float
clamp(const float value, const float minimum, const float maximum) {
	if (value < minimum) { return minimum; }
	if (value > maximum) { return maximum; }
	return value;
}
