/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "hud.h"
#include <stdio.h>

static void draw_shadowed_text(renderer_t *renderer,
			       float x,
			       float y,
			       float scale,
			       renderer_color_t color,
			       const char *text);
static void draw_vitals(renderer_t *renderer,
			int height,
			const sandbox_hud_values_t *values);
static void draw_ammunition(renderer_t *renderer,
			    int width,
			    int height,
			    const sandbox_hud_values_t *values);

static const renderer_color_t hud_color = {1.0f, 0.58f, 0.12f, 1.0f};
static const renderer_color_t danger_color = {1.0f, 0.23f, 0.08f, 1.0f};
static const renderer_color_t crosshair_color = {0.95f, 0.95f, 0.88f, 0.9f};

void sandbox_hud_initialize(sandbox_hud_t *hud, const float health) {
	if (hud == NULL) { return; }
	hud->previous_health = health;
	hud->damage_flash_time = 0.0f;
}

void sandbox_hud_update(sandbox_hud_t *hud,
			const float health,
			const float delta_time) {
	if (hud == NULL) { return; }
	if (delta_time > 0.0f) {
		hud->damage_flash_time -= delta_time;
		if (hud->damage_flash_time < 0.0f) {
			hud->damage_flash_time = 0.0f;
		}
	}
	if (health < hud->previous_health) { hud->damage_flash_time = 0.35f; }
	hud->previous_health = health;
}

void sandbox_hud_reset(sandbox_hud_t *hud, const float health) {
	sandbox_hud_initialize(hud, health);
}

void sandbox_hud_draw(const sandbox_hud_t *hud,
		      renderer_t *renderer,
		      const int width,
		      const int height,
		      const sandbox_hud_values_t *values) {
	float flash_alpha;

	if (hud == NULL || renderer == NULL || values == NULL || width <= 0 ||
	    height <= 0) {
		return;
	}
	if (!values->alive) {
		renderer_draw_rectangle(
			renderer, 0.0f, 0.0f, (float)width, (float)height,
			(renderer_color_t){0.22f, 0.0f, 0.0f, 0.68f});
		draw_shadowed_text(renderer, (float)width * 0.5f - 90.0f,
				   (float)height * 0.5f - 20.0f, 3.0f,
				   hud_color, "YOU DIED");
		draw_shadowed_text(renderer, (float)width * 0.5f - 155.0f,
				   (float)height * 0.5f + 24.0f, 1.5f,
				   hud_color, "CLICK TO RESPAWN");
		return;
	}
	if (hud->damage_flash_time > 0.0f) {
		flash_alpha = 0.28f * hud->damage_flash_time / 0.35f;
		renderer_draw_rectangle(
			renderer, 0.0f, 0.0f, (float)width, (float)height,
			(renderer_color_t){0.8f, 0.0f, 0.0f, flash_alpha});
	}

	draw_vitals(renderer, height, values);
	draw_ammunition(renderer, width, height, values);
	renderer_draw_rectangle(renderer, (float)width * 0.5f - 7.0f,
				(float)height * 0.5f - 1.0f, 14.0f, 2.0f,
				crosshair_color);
	renderer_draw_rectangle(renderer, (float)width * 0.5f - 1.0f,
				(float)height * 0.5f - 7.0f, 2.0f, 14.0f,
				crosshair_color);
}

static void draw_vitals(renderer_t *renderer,
			const int height,
			const sandbox_hud_values_t *values) {
	renderer_color_t health_color;
	char health[16];
	float label_y;
	float value_y;

	health_color = values->health <= 25 ? danger_color : hud_color;
	value_y = (float)height - 66.0f;
	label_y = (float)height - 36.0f;
	snprintf(health, sizeof(health), "%d", values->health);
	draw_shadowed_text(renderer, 30.0f, label_y, 1.7f, health_color,
			   "HEALTH");
	draw_shadowed_text(renderer, 106.0f, value_y, 4.2f, health_color,
			   health);
}

static void draw_ammunition(renderer_t *renderer,
			    const int width,
			    const int height,
			    const sandbox_hud_values_t *values) {
	char ammunition[16];
	char reserve[16];
	float label_y;
	float value_y;

	value_y = (float)height - 66.0f;
	label_y = (float)height - 36.0f;
	snprintf(ammunition, sizeof(ammunition), "%d", values->ammunition);
	snprintf(reserve, sizeof(reserve), "%d", values->reserve_ammunition);
	draw_shadowed_text(renderer, (float)width - 230.0f, label_y, 1.7f,
			   hud_color, "AMMO");
	draw_shadowed_text(renderer, (float)width - 150.0f, value_y, 4.2f,
			   hud_color, ammunition);
	draw_shadowed_text(renderer, (float)width - 72.0f,
			   (float)height - 47.0f, 2.0f, hud_color, reserve);
}

static void draw_shadowed_text(renderer_t *renderer,
			       const float x,
			       const float y,
			       const float scale,
			       const renderer_color_t color,
			       const char *text) {
	const renderer_color_t shadow = {0.02f, 0.015f, 0.0f, 0.8f};

	renderer_draw_text(renderer, x + 2.0f, y + 2.0f, scale, shadow, text);
	renderer_draw_text(renderer, x, y, scale, color, text);
}
