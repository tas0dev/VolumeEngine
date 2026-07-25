/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/damage.h"
#include "entity/entity.h"
#include <math.h>

bool entity_take_damage(entity_t *entity, const damage_info_t *damage) {
	if (entity == NULL || damage == NULL || !isfinite(damage->amount) ||
	    damage->amount <= 0.0f || entity->pending_destroy ||
	    entity->class == NULL || entity->class->take_damage == NULL) {
		return false;
	}
	return entity->class->take_damage(entity, damage);
}
