/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "fps/recoil.h"
#include <math.h>

static bool test_recoil_impulse_recovers(void) {
	fps_recoil_config_t config;
	fps_recoil_offset_t offset;
	fps_recoil_t recoil;
	int index;

	config.return_strength = 48.0f;
	config.damping = 12.0f;
	config.maximum_pitch = 0.12f;
	config.maximum_yaw = 0.08f;
	CHECK(fps_recoil_initialize(&recoil, &config));
	fps_recoil_add_impulse(&recoil, 2.4f, -1.2f);
	for (index = 0; index < 8; index++) {
		fps_recoil_update(&recoil, 1.0f / 120.0f);
	}
	offset = fps_recoil_get_offset(&recoil);
	CHECK(offset.pitch > 0.0f);
	CHECK(offset.pitch <= config.maximum_pitch);
	CHECK(offset.yaw < 0.0f);
	CHECK(offset.yaw >= -config.maximum_yaw);
	for (index = 0; index < 600; index++) {
		fps_recoil_update(&recoil, 1.0f / 120.0f);
	}
	offset = fps_recoil_get_offset(&recoil);
	CHECK(fabsf(offset.pitch) < 0.0001f);
	CHECK(fabsf(offset.yaw) < 0.0001f);
	return true;
}

static bool test_recoil_rejects_invalid_config(void) {
	fps_recoil_config_t config = {0};
	fps_recoil_t recoil;

	CHECK(!fps_recoil_initialize(&recoil, &config));
	config.return_strength = 1.0f;
	config.damping = 1.0f;
	config.maximum_pitch = 0.1f;
	config.maximum_yaw = 0.1f;
	CHECK(fps_recoil_initialize(&recoil, &config));
	fps_recoil_add_impulse(&recoil, 1.0f, 1.0f);
	fps_recoil_reset(&recoil);
	CHECK(fps_recoil_get_offset(&recoil).pitch == 0.0f);
	CHECK(fps_recoil_get_offset(&recoil).yaw == 0.0f);
	return true;
}

static bool test_pattern_recoil_advances_and_recovers(void) {
	static const fps_recoil_pattern_point_t points[] = {
		{0.02f, -0.01f},
		{0.03f, 0.02f },
	};
	fps_recoil_config_t config;
	fps_recoil_pattern_config_t pattern;
	fps_recoil_offset_t applied;
	fps_recoil_offset_t offset;
	fps_recoil_t recoil;
	int index;

	config.return_strength = 48.0f;
	config.damping = 12.0f;
	config.maximum_pitch = 0.2f;
	config.maximum_yaw = 0.1f;
	CHECK(fps_recoil_initialize(&recoil, &config));
	pattern.points = points;
	pattern.point_count = sizeof(points) / sizeof(points[0]);
	pattern.recovery_delay = 0.1f;
	pattern.pattern_reset_time = 0.3f;
	pattern.recovery_speed = 10.0f;
	pattern.follow_speed = 30.0f;
	CHECK(fps_recoil_set_pattern(&recoil, &pattern));
	applied = fps_recoil_fire_pattern(&recoil);
	CHECK(applied.pitch == points[0].pitch);
	CHECK(applied.yaw == points[0].yaw);
	CHECK(fps_recoil_get_shot_index(&recoil) == 1);
	for (index = 0; index < 6; index++) {
		fps_recoil_update(&recoil, 1.0f / 120.0f);
	}
	offset = fps_recoil_get_offset(&recoil);
	CHECK(offset.pitch > 0.0f);
	applied = fps_recoil_fire_pattern(&recoil);
	CHECK(applied.pitch == points[1].pitch);
	CHECK(fps_recoil_get_shot_index(&recoil) == 2);
	for (index = 0; index < 600; index++) {
		fps_recoil_update(&recoil, 1.0f / 120.0f);
	}
	offset = fps_recoil_get_offset(&recoil);
	CHECK(fabsf(offset.pitch) < 0.0001f);
	CHECK(fabsf(offset.yaw) < 0.0001f);
	applied = fps_recoil_fire_pattern(&recoil);
	CHECK(applied.pitch == points[0].pitch);
	CHECK(fps_recoil_get_shot_index(&recoil) == 1);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"recoil impulse recovers",	    test_recoil_impulse_recovers},
		{"recoil rejects invalid config",
		 test_recoil_rejects_invalid_config			   },
		{"pattern recoil advances and recovers",
		 test_pattern_recoil_advances_and_recovers				  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
