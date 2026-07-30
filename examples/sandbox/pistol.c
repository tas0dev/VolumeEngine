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
#include <string.h>

static const fps_recoil_pattern_point_t pistol_recoil_pattern[] = {
	{0.010f, 0.000f  },
	    {0.011f, -0.0015f},
	{0.012f, 0.0020f },
	{0.013f, -0.0025f},
	    {0.014f, 0.0030f },
	{0.014f, 0.0010f },
	{0.015f, -0.0035f},
	    {0.015f, -0.0015f},
	{0.016f, 0.0038f },
	{0.016f, 0.0018f },
	    {0.017f, -0.0040f},
	{0.017f, 0.0025f },
};

static hitscan_weapon_config_t create_pistol_config(void) {
	hitscan_weapon_config_t config;

	config.damage = 20.0f;
	config.range = 100.0f;
	config.fire_interval = 0.2f;
	config.magazine_size = 12;
	config.reserve_ammo = 48;
	return config;
}

static bool configure_pistol_weapon(sandbox_pistol_t *pistol) {
	hitscan_accuracy_config_t accuracy = {0};
	hitscan_weapon_config_t config;

	config = create_pistol_config();
	accuracy.standing_inaccuracy = 0.0022f;
	accuracy.moving_inaccuracy = 0.018f;
	accuracy.airborne_inaccuracy = 0.075f;
	accuracy.crouched_multiplier = 0.72f;
	accuracy.firing_penalty_per_shot = 0.0045f;
	accuracy.maximum_firing_penalty = 0.032f;
	accuracy.penalty_recovery_delay = 0.18f;
	accuracy.penalty_recovery_rate = 0.028f;
	accuracy.reference_move_speed = 4.0f;
	accuracy.random_seed = UINT32_C(0x51a7c2d9);
	return hitscan_weapon_initialize(&pistol->weapon, &config) &&
	       hitscan_weapon_set_accuracy(&pistol->weapon, &accuracy);
}

static void pistol_animation_event(void *user_data, const char *event_name) {
	sandbox_pistol_t *pistol = user_data;

	if (pistol == NULL || event_name == NULL) { return; }
	if (strcmp(event_name, "reload_sound") == 0) {
		pistol->reload_voice = audio_system_play(
			pistol->audio, pistol->reload_sound, NULL);
	}
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
	tracer.start_width = 0.035f;
	tracer.end_width = 0.012f;
	tracer.lifetime = 0.13f;
	tracer.start_color = (renderer_color_t){1.0f, 0.82f, 0.35f, 1.0f};
	tracer.end_color = (renderer_color_t){1.0f, 0.42f, 0.08f, 0.0f};
	tracer.blend_mode = RENDERER_BLEND_ADDITIVE;
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
	fps_recoil_config_t aim_recoil_config;
	fps_recoil_config_t viewmodel_recoil_config;
	fps_recoil_pattern_config_t pattern_config;

	if (pistol == NULL || assets == NULL || audio == NULL) { return false; }
	*pistol = (sandbox_pistol_t){0};
	pistol->audio = audio;
	pistol->view_model_mesh = asset_manager_load_mesh(
		assets, "models/wepons/pistol/pistol.glb", error, error_size);
	pistol->view_model_material = asset_manager_load_material(
		assets, "materials/pistol.volmat", error, error_size);
	pistol->fire_sound = asset_manager_load_sound(assets, "audio/fire.mp3",
						      error, error_size);
	pistol->reload_sound = asset_manager_load_sound(
		assets, "audio/reload.mp3", error, error_size);
	pistol->effects = fps_effect_system_create();
	config = create_pistol_config();
	aim_recoil_config.return_strength = 48.0f;
	aim_recoil_config.damping = 12.0f;
	aim_recoil_config.maximum_pitch = 0.18f;
	aim_recoil_config.maximum_yaw = 0.09f;
	viewmodel_recoil_config.return_strength = 52.0f;
	viewmodel_recoil_config.damping = 13.0f;
	viewmodel_recoil_config.maximum_pitch = 0.11f;
	viewmodel_recoil_config.maximum_yaw = 0.055f;
	pattern_config.points = pistol_recoil_pattern;
	pattern_config.point_count = sizeof(pistol_recoil_pattern) /
				     sizeof(pistol_recoil_pattern[0]);
	pattern_config.recovery_delay = 0.20f;
	pattern_config.pattern_reset_time = 0.42f;
	pattern_config.recovery_speed = 7.5f;
	pattern_config.follow_speed = 32.0f;
	if (pistol->view_model_mesh == NULL ||
	    pistol->view_model_material == NULL || pistol->fire_sound == NULL ||
	    pistol->reload_sound == NULL || pistol->effects == NULL ||
	    !fps_recoil_initialize(&pistol->aim_recoil, &aim_recoil_config) ||
	    !fps_recoil_set_pattern(&pistol->aim_recoil, &pattern_config) ||
	    !fps_recoil_initialize(&pistol->viewmodel_recoil,
				   &viewmodel_recoil_config) ||
	    !configure_pistol_weapon(pistol)) {
		sandbox_pistol_destroy(pistol);
		return false;
	}
	pistol->fire_action_duration = config.fire_interval;
	pistol->reload_action_duration = 1.0f;
	pistol->has_animator = animator_initialize(
		&pistol->animator,
		mesh_get_animation_set(pistol->view_model_mesh));
	if (pistol->has_animator) {
		float clip_duration;

		clip_duration =
			animator_get_clip_duration(&pistol->animator, "fire");
		if (clip_duration > 0.0f) {
			pistol->fire_action_duration = clip_duration;
		}
		clip_duration =
			animator_get_clip_duration(&pistol->animator, "reload");
		if (clip_duration > 0.0f) {
			pistol->reload_action_duration = clip_duration;
		}
		animator_set_event_callback(&pistol->animator,
					    pistol_animation_event, pistol);
		pistol->reload_sound_event = animator_add_event(
			&pistol->animator, "reload", "reload_sound", 0.0f);
		(void)animator_play(&pistol->animator, "idle", true);
	}
	return true;
}

void sandbox_pistol_update(sandbox_pistol_t *pistol,
			   const float delta_time,
			   const float movement_speed,
			   const bool grounded,
			   const bool crouched) {
	float target_bob;
	float blend;

	if (pistol == NULL) { return; }
	hitscan_weapon_update(&pistol->weapon, delta_time);
	fps_recoil_update(&pistol->aim_recoil, delta_time);
	fps_recoil_update(&pistol->viewmodel_recoil, delta_time);
	pistol->movement_speed = movement_speed;
	pistol->grounded = grounded;
	pistol->crouched = crouched;
	if (pistol->has_animator) {
		animator_update(&pistol->animator, delta_time);
		if (!animator_is_playing(&pistol->animator)) {
			(void)animator_play_blended(&pistol->animator, "idle",
						    true, 0.12f);
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
	hitscan_accuracy_context_t accuracy = {0};
	hitscan_shot_result_t shot = {0};
	fps_recoil_offset_t applied_recoil;

	if (pistol == NULL) { return false; }
	accuracy.movement_speed = pistol->movement_speed;
	accuracy.grounded = pistol->grounded;
	accuracy.crouched = pistol->crouched;
	if (!hitscan_weapon_fire_accurate_timed(
		    &pistol->weapon, world, owner, origin, direction, &accuracy,
		    pistol->fire_action_duration, &shot)) {
		return false;
	}
	if (trace != NULL) { *trace = shot.trace; }
	pistol->fire_voice =
		audio_system_play(pistol->audio, pistol->fire_sound, NULL);
	spawn_shot_effects(pistol, origin, shot.direction, &shot.trace);
	pistol->muzzle_flash_time = 0.045f;
	applied_recoil = fps_recoil_fire_pattern(&pistol->aim_recoil);
	fps_recoil_add_impulse(&pistol->viewmodel_recoil, 2.4f,
			       applied_recoil.yaw * 35.0f);
	if (pistol->has_animator) {
		(void)animator_play_blended(&pistol->animator, "fire", false,
					    0.035f);
	}
	return true;
}

bool sandbox_pistol_reload(sandbox_pistol_t *pistol) {
	if (pistol == NULL ||
	    !hitscan_weapon_start_reload(&pistol->weapon,
					 pistol->reload_action_duration)) {
		return false;
	}
	if (pistol->has_animator) {
		if (animator_play_blended(&pistol->animator, "reload", false,
					  0.08f) &&
		    pistol->reload_sound_event) {
			return true;
		}
	}
	pistol->reload_voice =
		audio_system_play(pistol->audio, pistol->reload_sound, NULL);
	return true;
}

bool sandbox_pistol_reset(sandbox_pistol_t *pistol) {
	if (pistol == NULL) { return false; }
	fps_recoil_reset(&pistol->aim_recoil);
	fps_recoil_reset(&pistol->viewmodel_recoil);
	pistol->bob_amount = 0.0f;
	pistol->sway_pitch = 0.0f;
	pistol->sway_yaw = 0.0f;
	pistol->movement_speed = 0.0f;
	pistol->grounded = false;
	pistol->crouched = false;
	pistol->muzzle_flash_time = 0.0f;
	fps_effect_system_clear(pistol->effects);
	if (pistol->has_animator) {
		(void)animator_play_blended(&pistol->animator, "idle", true,
					    0.1f);
	}
	return configure_pistol_weapon(pistol);
}

fps_recoil_offset_t sandbox_pistol_get_recoil(const sandbox_pistol_t *pistol) {
	return pistol == NULL ? (fps_recoil_offset_t){0}
			      : fps_recoil_get_offset(&pistol->aim_recoil);
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
	recoil = fps_recoil_get_offset(&pistol->viewmodel_recoil);
	bob_x = sinf(pistol->bob_time) * 0.018f * pistol->bob_amount;
	bob_y = fabsf(cosf(pistol->bob_time)) * 0.022f * pistol->bob_amount;
	transform = transform_create();
	transform.position =
		vec3_create(0.30f + bob_x + pistol->sway_yaw * 0.3f,
			    -0.27f - bob_y + pistol->sway_pitch * 0.25f,
			    -0.58f + recoil.pitch * 0.35f);
	transform.rotation = vec3_create(
		-0.08f + recoil.pitch * 1.8f + pistol->sway_pitch,
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
	fps_effect_system_destroy(pistol->effects);
	*pistol = (sandbox_pistol_t){0};
}
