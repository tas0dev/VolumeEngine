/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/health.h"
#include <math.h>
#include <stddef.h>

bool health_initialize(health_t *health, const float maximum) {
	if (health == NULL || !isfinite(maximum) || maximum <= 0.0f) {
		return false;
	}
	health->current = maximum;
	health->maximum = maximum;
	health->dead = false;
	return true;
}

float health_apply_damage(health_t *health, const float amount) {
	float applied;

	if (health == NULL || health->dead || !isfinite(amount) ||
	    amount <= 0.0f) {
		return 0.0f;
	}
	applied = fminf(health->current, amount);
	health->current -= applied;
	if (health->current <= 0.0f) {
		health->current = 0.0f;
		health->dead = true;
	}
	return applied;
}

float health_get_current(const health_t *health) {
	return health == NULL ? 0.0f : health->current;
}

float health_get_maximum(const health_t *health) {
	return health == NULL ? 0.0f : health->maximum;
}

bool health_is_alive(const health_t *health) {
	return health != NULL && !health->dead;
}
