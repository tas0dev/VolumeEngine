/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "pistol.h"
#include "scene/transform.h"
#include <math.h>
#include <stdlib.h>

static hitscan_weapon_config_t create_pistol_config(void) {
	hitscan_weapon_config_t config;

	config.damage = 20.0f;
	config.range = 100.0f;
	config.fire_interval = 0.2f;
	config.magazine_size = 12;
	config.reserve_ammo = 48;
	return config;
}

bool sandbox_pistol_initialize(sandbox_pistol_t *pistol,
			       asset_manager_t *assets,
			       audio_system_t *audio,
			       char *error,
			       const size_t error_size) {
	hitscan_weapon_config_t config;
	fps_recoil_config_t recoil_config;

	if (pistol == NULL || assets == NULL || audio == NULL) { return false; }
	*pistol = (sandbox_pistol_t){0};
	pistol->audio = audio;
	pistol->view_model_mesh = asset_manager_load_mesh(
		assets, "models/test_cube.obj", error, error_size);
	pistol->view_model_material = asset_manager_load_material(
		assets, "materials/test_cube.volmat", error, error_size);
	pistol->fire_sound = audio_sound_create_tone(145.0f, 0.07f);
	pistol->reload_sound = audio_sound_create_tone(520.0f, 0.09f);
	config = create_pistol_config();
	recoil_config.return_strength = 48.0f;
	recoil_config.damping = 12.0f;
	recoil_config.maximum_pitch = 0.12f;
	recoil_config.maximum_yaw = 0.06f;
	if (pistol->view_model_mesh == NULL ||
	    pistol->view_model_material == NULL || pistol->fire_sound == NULL ||
	    pistol->reload_sound == NULL ||
	    !fps_recoil_initialize(&pistol->recoil, &recoil_config) ||
	    !hitscan_weapon_initialize(&pistol->weapon, &config)) {
		sandbox_pistol_destroy(pistol);
		return false;
	}
	pistol->recoil_direction = 1.0f;
	return true;
}

void sandbox_pistol_update(sandbox_pistol_t *pistol,
			   const float delta_time,
			   const float movement_speed,
			   const bool grounded) {
	float target_bob;
	float blend;

	if (pistol == NULL) { return; }
	hitscan_weapon_update(&pistol->weapon, delta_time);
	fps_recoil_update(&pistol->recoil, delta_time);
	target_bob = grounded ? fminf(movement_speed / 4.0f, 1.0f) : 0.0f;
	blend = 1.0f - expf(-10.0f * delta_time);
	pistol->bob_amount += (target_bob - pistol->bob_amount) * blend;
	pistol->bob_time += delta_time * (7.0f + movement_speed * 1.5f);
	pistol->sway_pitch *= expf(-8.0f * delta_time);
	pistol->sway_yaw *= expf(-8.0f * delta_time);
}

void sandbox_pistol_add_look_delta(sandbox_pistol_t *pistol,
				   const float yaw_delta,
				   const float pitch_delta) {
	if (pistol == NULL) { return; }
	pistol->sway_yaw = fmaxf(
		-0.06f, fminf(0.06f, pistol->sway_yaw - yaw_delta * 0.8f));
	pistol->sway_pitch = fmaxf(
		-0.05f, fminf(0.05f, pistol->sway_pitch - pitch_delta * 0.8f));
}

bool sandbox_pistol_fire(sandbox_pistol_t *pistol,
			 world_t *world,
			 entity_t *owner,
			 const vec3_t origin,
			 const vec3_t direction,
			 collision_trace_t *trace) {
	if (pistol == NULL ||
	    !hitscan_weapon_fire(&pistol->weapon, world, owner, origin,
				 direction, trace)) {
		return false;
	}
	pistol->fire_voice =
		audio_system_play(pistol->audio, pistol->fire_sound, NULL);
	fps_recoil_add_impulse(&pistol->recoil, 2.4f,
			       0.55f * pistol->recoil_direction);
	pistol->recoil_direction = -pistol->recoil_direction;
	return true;
}

bool sandbox_pistol_reload(sandbox_pistol_t *pistol) {
	if (pistol == NULL || !hitscan_weapon_reload(&pistol->weapon)) {
		return false;
	}
	pistol->reload_voice =
		audio_system_play(pistol->audio, pistol->reload_sound, NULL);
	return true;
}

bool sandbox_pistol_reset(sandbox_pistol_t *pistol) {
	hitscan_weapon_config_t config;

	if (pistol == NULL) { return false; }
	config = create_pistol_config();
	fps_recoil_reset(&pistol->recoil);
	pistol->bob_amount = 0.0f;
	pistol->sway_pitch = 0.0f;
	pistol->sway_yaw = 0.0f;
	pistol->recoil_direction = 1.0f;
	return hitscan_weapon_initialize(&pistol->weapon, &config);
}

fps_recoil_offset_t sandbox_pistol_get_recoil(const sandbox_pistol_t *pistol) {
	return pistol == NULL ? (fps_recoil_offset_t){0}
			      : fps_recoil_get_offset(&pistol->recoil);
}

void sandbox_pistol_draw(const sandbox_pistol_t *pistol,
			 renderer_t *renderer,
			 const render_view_t *world_view) {
	fps_recoil_offset_t recoil;
	transform_t transform;
	mat4_t model;
	float bob_x;
	float bob_y;

	if (pistol == NULL || renderer == NULL || world_view == NULL ||
	    pistol->view_model_mesh == NULL ||
	    pistol->view_model_material == NULL) {
		return;
	}
	recoil = fps_recoil_get_offset(&pistol->recoil);
	bob_x = sinf(pistol->bob_time) * 0.018f * pistol->bob_amount;
	bob_y = fabsf(cosf(pistol->bob_time)) * 0.022f * pistol->bob_amount;
	transform = transform_create();
	transform.position =
		vec3_create(0.38f + bob_x + pistol->sway_yaw * 0.3f,
			    -0.33f - bob_y + pistol->sway_pitch * 0.25f,
			    -0.9f + recoil.pitch * 0.35f);
	transform.rotation = vec3_create(
		-0.08f - recoil.pitch * 1.8f + pistol->sway_pitch,
		0.08f - recoil.yaw + pistol->sway_yaw, -0.06f + bob_x * 1.5f);
	transform.scale = vec3_create(0.12f, 0.12f, 0.48f);
	model = transform_get_matrix(&transform);
	if (!renderer_begin_view_model_pass(renderer, 75.0f, 0.01f, 5.0f)) {
		return;
	}
	renderer_draw_view_model_mesh(renderer, pistol->view_model_mesh,
				      pistol->view_model_material, &model,
				      world_view);
	renderer_end_view_model_pass(renderer);
}

int sandbox_pistol_get_ammo(const sandbox_pistol_t *pistol) {
	return pistol == NULL ? 0 : hitscan_weapon_get_ammo(&pistol->weapon);
}

int sandbox_pistol_get_reserve_ammo(const sandbox_pistol_t *pistol) {
	return pistol == NULL
		       ? 0
		       : hitscan_weapon_get_reserve_ammo(&pistol->weapon);
}

void sandbox_pistol_destroy(sandbox_pistol_t *pistol) {
	if (pistol == NULL) { return; }
	audio_system_stop(pistol->audio, pistol->reload_voice);
	audio_system_stop(pistol->audio, pistol->fire_voice);
	audio_sound_destroy(pistol->reload_sound);
	audio_sound_destroy(pistol->fire_sound);
	*pistol = (sandbox_pistol_t){0};
}
