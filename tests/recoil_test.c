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

int main(void) {
	static const test_case_t tests[] = {
		{"recoil impulse recovers",	    test_recoil_impulse_recovers},
		{"recoil rejects invalid config",
		 test_recoil_rejects_invalid_config			   },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
