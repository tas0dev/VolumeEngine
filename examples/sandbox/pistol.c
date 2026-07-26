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

static void spawn_shot_effects(sandbox_pistol_t *pistol,
			       vec3_t origin,
			       vec3_t direction,
			       const collision_trace_t *trace) {
	fps_tracer_effect_config_t tracer = {0};
	fps_billboard_effect_config_t impact = {0};
	vec3_t normalized_direction;

	normalized_direction = vec3_normalize(direction);
	tracer.start =
		vec3_add(origin, vec3_scale(normalized_direction, 0.65f));
	tracer.end =
		trace->hit ? trace->position
			   : vec3_add(origin, vec3_scale(normalized_direction,
							 pistol->weapon.range));
	tracer.lifetime = 0.055f;
	tracer.start_color = (renderer_color_t){1.0f, 0.72f, 0.28f, 0.9f};
	tracer.end_color = (renderer_color_t){1.0f, 0.35f, 0.08f, 0.0f};
	(void)fps_effect_system_spawn_tracer(pistol->effects, &tracer);
	if (!trace->hit || vec3_length(trace->normal) <= 0.000001f) { return; }
	impact.position =
		vec3_add(trace->position, vec3_scale(trace->normal, 0.012f));
	impact.normal = trace->normal;
	impact.start_size = 0.055f;
	impact.end_size = 0.048f;
	impact.lifetime = 4.0f;
	impact.start_color = (renderer_color_t){0.12f, 0.075f, 0.025f, 0.9f};
	impact.end_color = (renderer_color_t){0.025f, 0.018f, 0.012f, 0.0f};
	impact.blend_mode = RENDERER_BLEND_ALPHA;
	(void)fps_effect_system_spawn_billboard(pistol->effects, &impact);
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
		assets, "models/wepons/pistol/pistol.glb", error, error_size);
	if (pistol->view_model_mesh == NULL) {
		pistol->view_model_mesh = asset_manager_load_mesh(
			assets, "models/wepons/pistol/pistol.obj", error,
			error_size);
	}
	pistol->view_model_material = asset_manager_load_material(
		assets, "materials/pistol.volmat", error, error_size);
	pistol->fire_sound = asset_manager_load_sound(assets, "audio/fire.mp3",
						      error, error_size);
	pistol->reload_sound = audio_sound_create_tone(520.0f, 0.09f);
	pistol->effects = fps_effect_system_create();
	config = create_pistol_config();
	recoil_config.return_strength = 48.0f;
	recoil_config.damping = 12.0f;
	recoil_config.maximum_pitch = 0.12f;
	recoil_config.maximum_yaw = 0.06f;
	if (pistol->view_model_mesh == NULL ||
	    pistol->view_model_material == NULL || pistol->fire_sound == NULL ||
	    pistol->reload_sound == NULL || pistol->effects == NULL ||
	    !fps_recoil_initialize(&pistol->recoil, &recoil_config) ||
	    !hitscan_weapon_initialize(&pistol->weapon, &config)) {
		sandbox_pistol_destroy(pistol);
		return false;
	}
	pistol->recoil_direction = 1.0f;
	pistol->has_animator = animator_initialize(
		&pistol->animator,
		mesh_get_animation_set(pistol->view_model_mesh));
	if (pistol->has_animator) {
		(void)animator_play(&pistol->animator, "idle", true);
	}
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
	if (pistol->has_animator) {
		animator_update(&pistol->animator, delta_time);
		if (!animator_is_playing(&pistol->animator)) {
			(void)animator_play(&pistol->animator, "idle", true);
		}
	}
	fps_effect_system_update(pistol->effects, delta_time);
	pistol->muzzle_flash_time =
		fmaxf(0.0f, pistol->muzzle_flash_time - delta_time);
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
	collision_trace_t result = {0};

	if (pistol == NULL ||
	    !hitscan_weapon_fire(&pistol->weapon, world, owner, origin,
				 direction, &result)) {
		return false;
	}
	if (trace != NULL) { *trace = result; }
	pistol->fire_voice =
		audio_system_play(pistol->audio, pistol->fire_sound, NULL);
	spawn_shot_effects(pistol, origin, direction, &result);
	pistol->muzzle_flash_time = 0.045f;
	fps_recoil_add_impulse(&pistol->recoil, 2.4f,
			       0.55f * pistol->recoil_direction);
	pistol->recoil_direction = -pistol->recoil_direction;
	if (pistol->has_animator) {
		(void)animator_play(&pistol->animator, "fire", false);
	}
	return true;
}

bool sandbox_pistol_reload(sandbox_pistol_t *pistol) {
	if (pistol == NULL || !hitscan_weapon_reload(&pistol->weapon)) {
		return false;
	}
	pistol->reload_voice =
		audio_system_play(pistol->audio, pistol->reload_sound, NULL);
	if (pistol->has_animator) {
		(void)animator_play(&pistol->animator, "reload", false);
	}
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
	pistol->muzzle_flash_time = 0.0f;
	fps_effect_system_clear(pistol->effects);
	if (pistol->has_animator) {
		(void)animator_play(&pistol->animator, "idle", true);
	}
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
	vec3_t muzzle_position;
	float bob_x;
	float bob_y;
	float flash_amount;

	if (pistol == NULL || renderer == NULL || world_view == NULL ||
	    pistol->view_model_mesh == NULL ||
	    pistol->view_model_material == NULL) {
		return;
	}
	fps_effect_system_draw(pistol->effects, renderer, world_view);
	recoil = fps_recoil_get_offset(&pistol->recoil);
	bob_x = sinf(pistol->bob_time) * 0.018f * pistol->bob_amount;
	bob_y = fabsf(cosf(pistol->bob_time)) * 0.022f * pistol->bob_amount;
	transform = transform_create();
	transform.position =
		vec3_create(0.30f + bob_x + pistol->sway_yaw * 0.3f,
			    -0.27f - bob_y + pistol->sway_pitch * 0.25f,
			    -0.58f + recoil.pitch * 0.35f);
	transform.rotation = vec3_create(
		-0.08f - recoil.pitch * 1.8f + pistol->sway_pitch,
		0.08f - recoil.yaw + pistol->sway_yaw, -0.06f + bob_x * 1.5f);
	transform.scale = vec3_create(1.8f, 1.8f, 1.8f);
	model = transform_get_matrix(&transform);
	if (!renderer_begin_view_model_pass(renderer, 75.0f, 0.01f, 5.0f)) {
		return;
	}
	if (pistol->has_animator) {
		renderer_draw_view_model_animated_mesh(
			renderer, pistol->view_model_mesh,
			pistol->view_model_material, &model, world_view,
			&pistol->animator);
	} else {
		renderer_draw_view_model_mesh(renderer, pistol->view_model_mesh,
					      pistol->view_model_material,
					      &model, world_view);
	}
	if (pistol->muzzle_flash_time > 0.0f) {
		flash_amount = pistol->muzzle_flash_time / 0.045f;
		muzzle_position = mat4_transform_point(
			model, vec3_create(0.004f, 0.0f, -0.13f));
		renderer_draw_view_model_sprite(
			renderer, muzzle_position,
			0.115f + flash_amount * 0.055f,
			(renderer_color_t){1.0f, 0.58f, 0.18f, flash_amount},
			RENDERER_BLEND_ADDITIVE);
	}
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
	fps_effect_system_destroy(pistol->effects);
	*pistol = (sandbox_pistol_t){0};
}
