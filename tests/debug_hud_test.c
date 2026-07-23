/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "debug/hud.h"

static bool test_hud_starts_visible_and_toggles(void) {
	debug_hud_t hud = {0};

	debug_hud_initialize(&hud);
	CHECK(debug_hud_is_visible(&hud));

	debug_hud_toggle(&hud);
	CHECK(!debug_hud_is_visible(&hud));

	debug_hud_toggle(&hud);
	CHECK(debug_hud_is_visible(&hud));
	return true;
}

static bool test_hud_smooths_frame_time(void) {
	debug_hud_t hud = {0};
	float initial;

	debug_hud_initialize(&hud);
	initial = hud.smoothed_frame_time;
	debug_hud_update(&hud, 1.0f / 30.0f);
	CHECK(hud.smoothed_frame_time > initial);
	CHECK(hud.smoothed_frame_time < 1.0f / 30.0f);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"HUD starts visible and toggles",
		 test_hud_starts_visible_and_toggles			    },
		{"HUD smooths frame time",	   test_hud_smooths_frame_time},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
