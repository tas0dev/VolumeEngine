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

bool fps_recoil_set_pattern(fps_recoil_t *recoil,
			    const fps_recoil_pattern_config_t *config) {
	if (recoil == NULL || config == NULL || config->points == NULL ||
	    config->point_count == 0 || !isfinite(config->recovery_delay) ||
	    config->recovery_delay < 0.0f ||
	    !isfinite(config->pattern_reset_time) ||
	    config->pattern_reset_time <= config->recovery_delay ||
	    !isfinite(config->recovery_speed) ||
	    config->recovery_speed <= 0.0f || !isfinite(config->follow_speed) ||
	    config->follow_speed <= 0.0f) {
		return false;
	}
	recoil->pattern = *config;
	recoil->shot_index = 0;
	recoil->time_since_shot = config->pattern_reset_time;
	recoil->target = (fps_recoil_offset_t){0};
	recoil->uses_pattern = true;
	return true;
}

fps_recoil_offset_t fps_recoil_fire_pattern(fps_recoil_t *recoil) {
	fps_recoil_pattern_point_t point;
	size_t index;

	if (recoil == NULL || !recoil->uses_pattern) {
		return (fps_recoil_offset_t){0};
	}
	if (recoil->time_since_shot >= recoil->pattern.pattern_reset_time) {
		recoil->shot_index = 0;
	}
	index = recoil->shot_index;
	if (index >= recoil->pattern.point_count) {
		index = recoil->pattern.point_count - 1;
	}
	point = recoil->pattern.points[index];
	if (!isfinite(point.pitch) || !isfinite(point.yaw)) {
		return (fps_recoil_offset_t){0};
	}
	recoil->target.pitch = clamp(recoil->target.pitch + point.pitch,
				     -recoil->config.maximum_pitch,
				     recoil->config.maximum_pitch);
	recoil->target.yaw =
		clamp(recoil->target.yaw + point.yaw,
		      -recoil->config.maximum_yaw, recoil->config.maximum_yaw);
	recoil->shot_index++;
	recoil->time_since_shot = 0.0f;
	return (fps_recoil_offset_t){point.pitch, point.yaw};
}

size_t fps_recoil_get_shot_index(const fps_recoil_t *recoil) {
	return recoil == NULL ? 0 : recoil->shot_index;
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
	float blend;

	if (recoil == NULL || !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	if (delta_time > 0.05f) { delta_time = 0.05f; }
	if (recoil->uses_pattern) {
		recoil->time_since_shot += delta_time;
		if (recoil->time_since_shot >= recoil->pattern.recovery_delay) {
			blend = 1.0f - expf(-recoil->pattern.recovery_speed *
					    delta_time);
			recoil->target.pitch +=
				(0.0f - recoil->target.pitch) * blend;
			recoil->target.yaw +=
				(0.0f - recoil->target.yaw) * blend;
		}
		blend = 1.0f - expf(-recoil->pattern.follow_speed * delta_time);
		recoil->offset.pitch +=
			(recoil->target.pitch - recoil->offset.pitch) * blend;
		recoil->offset.yaw +=
			(recoil->target.yaw - recoil->offset.yaw) * blend;
		return;
	}
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
	recoil->target = (fps_recoil_offset_t){0};
	recoil->shot_index = 0;
	recoil->time_since_shot = recoil->uses_pattern
					  ? recoil->pattern.pattern_reset_time
					  : 0.0f;
}

static float
clamp(const float value, const float minimum, const float maximum) {
	if (value < minimum) { return minimum; }
	if (value > maximum) { return maximum; }
	return value;
}
