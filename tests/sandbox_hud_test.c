/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "../examples/sandbox/hud.h"
#include "common.h"

static bool test_sandbox_hud_tracks_damage_and_reset(void) {
	sandbox_hud_t hud;

	sandbox_hud_initialize(&hud, 100.0f);
	CHECK(hud.previous_health == 100.0f);
	CHECK(hud.damage_flash_time == 0.0f);
	sandbox_hud_update(&hud, 80.0f, 0.1f);
	CHECK(hud.previous_health == 80.0f);
	CHECK(hud.damage_flash_time == 0.35f);
	sandbox_hud_update(&hud, 80.0f, 0.1f);
	CHECK(hud.damage_flash_time > 0.24f);
	CHECK(hud.damage_flash_time < 0.26f);
	sandbox_hud_reset(&hud, 100.0f);
	CHECK(hud.previous_health == 100.0f);
	CHECK(hud.damage_flash_time == 0.0f);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"sandbox HUD tracks damage and reset",
		 test_sandbox_hud_tracks_damage_and_reset},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
