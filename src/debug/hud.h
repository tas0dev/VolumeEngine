/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_DEBUG_HUD_H
#define VOLUME_DEBUG_HUD_H

#include "math/vec3.h"
#include "renderer/renderer.h"
#include <stdbool.h>

typedef struct debug_hud {
	bool visible;
	float smoothed_frame_time;
} debug_hud_t;

void debug_hud_initialize(debug_hud_t *hud);
void debug_hud_toggle(debug_hud_t *hud);
bool debug_hud_is_visible(const debug_hud_t *hud);
void debug_hud_update(debug_hud_t *hud, float delta_time);
void debug_hud_draw(const debug_hud_t *hud,
		    renderer_t *renderer,
		    const world_t *world,
		    vec3_t player_position,
		    vec3_t player_velocity,
		    bool player_grounded,
		    float fixed_delta_time);

#endif