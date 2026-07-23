/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "debug/hud.h"
#include "collision/collision_world.h"
#include "debug/debug_ui.h"
#include "entity/world.h"

#include <stdio.h>

void debug_hud_initialize(debug_hud_t *hud) {
	if (hud == NULL) { return; }

	hud->visible = true;
	hud->smoothed_frame_time = 1.0f / 60.0f;
}

void debug_hud_toggle(debug_hud_t *hud) {
	if (hud == NULL) { return; }

	hud->visible = !hud->visible;
}

bool debug_hud_is_visible(const debug_hud_t *hud) {
	return hud != NULL && hud->visible;
}

void debug_hud_update(debug_hud_t *hud, const float delta_time) {
	float blend;

	if (hud == NULL || delta_time <= 0.0f) { return; }

	blend = delta_time * 8.0f;
	if (blend > 1.0f) { blend = 1.0f; }

	hud->smoothed_frame_time +=
		(delta_time - hud->smoothed_frame_time) * blend;
}

void debug_hud_draw(const debug_hud_t *hud,
		    renderer_t *renderer,
		    const world_t *world,
		    const vec3_t player_position,
		    const vec3_t player_velocity,
		    const bool player_grounded,
		    const float fixed_delta_time) {
	debug_ui_t ui;
	renderer_frame_stats_t render_stats;
	const collision_world_t *collision_world;
	char value[96];
	float fps;
	float fixed_rate;

	if (!debug_hud_is_visible(hud) || renderer == NULL || world == NULL) {
		return;
	}

	fps = hud->smoothed_frame_time > 0.0f ? 1.0f / hud->smoothed_frame_time
					      : 0.0f;

	fixed_rate = fixed_delta_time > 0.0f ? 1.0f / fixed_delta_time : 0.0f;

	render_stats = renderer_get_frame_stats(renderer);
	collision_world = world_get_const_collision_world(world);

	debug_ui_begin(&ui, renderer, 12.0f, 12.0f);

	snprintf(value, sizeof(value), "%.1f", fps);
	debug_ui_draw_row(&ui, "fps", value);

	snprintf(value, sizeof(value), "%.2f ms",
		 hud->smoothed_frame_time * 1000.0f);
	debug_ui_draw_row(&ui, "frametime", value);

	snprintf(value, sizeof(value), "%.1f hz", fixed_rate);
	debug_ui_draw_row(&ui, "tickrate", value);

	debug_ui_add_spacing(&ui, 4.0f);

	snprintf(value, sizeof(value), "%zu", world_get_entity_count(world));
	debug_ui_draw_row(&ui, "entities", value);

	snprintf(value, sizeof(value), "%zu",
		 collision_world != NULL
			 ? collision_world_get_count(collision_world)
			 : 0);
	debug_ui_draw_row(&ui, "colliders", value);

	snprintf(value, sizeof(value), "%zu", render_stats.mesh_draw_calls);
	debug_ui_draw_row(&ui, "draws", value);

	snprintf(value, sizeof(value), "%zu", render_stats.shadow_draw_calls);
	debug_ui_draw_row(&ui, "shadow draws", value);

	snprintf(value, sizeof(value), "%zu",
		 world_get_pending_event_count(world));
	debug_ui_draw_row(&ui, "events", value);

	debug_ui_add_spacing(&ui, 4.0f);

	snprintf(value, sizeof(value), "%.2f %.2f %.2f", player_position.x,
		 player_position.y, player_position.z);
	debug_ui_draw_row(&ui, "position", value);

	snprintf(value, sizeof(value), "%.2f %.2f %.2f", player_velocity.x,
		 player_velocity.y, player_velocity.z);
	debug_ui_draw_row(&ui, "velocity", value);

	debug_ui_draw_row(&ui, "grounded", player_grounded ? "yes" : "no");
}