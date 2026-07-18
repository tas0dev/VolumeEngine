/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "physics/character_controller.h"

character_controller_t character_controller_create(const vec3_t position,
						   const float radius,
						   const float height) {
	character_controller_t controller;
	float half_height;

	half_height = height * 0.5f;

	controller.bounds =
		aabb_create(vec3_create(0.0f, half_height, 0.0f),
			    vec3_create(radius, half_height, radius));
	controller.position = position;
	controller.velocity = vec3_create(0.0f, 0.0f, 0.0f);
	controller.gravity = -20.0f;
	controller.jump_speed = 7.0f;
	controller.grounded = false;

	return controller;
}

void character_controller_set_horizontal_velocity(
	character_controller_t *controller, const vec3_t velocity) {
	if (controller == NULL) { return; }

	controller->velocity.x = velocity.x;
	controller->velocity.z = velocity.z;
}

bool character_controller_jump(character_controller_t *controller) {
	if (controller == NULL || !controller->grounded) { return false; }

	controller->velocity.y = controller->jump_speed;
	controller->grounded = false;

	return true;
}

void character_controller_update(character_controller_t *controller,
				 const collision_world_t *world,
				 const float delta_time) {
	collision_result_t collision;

	if (controller == NULL || delta_time <= 0.0f) { return; }

	controller->velocity.y += controller->gravity * delta_time;
	controller->position =
		vec3_add(controller->position,
			 vec3_scale(controller->velocity, delta_time));
	controller->grounded = false;

	if (world == NULL ||
	    !collision_world_resolve_aabb(world, controller->bounds,
					  &controller->position, &collision)) {
		return;
	}

	if (collision.sides &
	    (COLLISION_SIDE_NEGATIVE_X | COLLISION_SIDE_POSITIVE_X)) {
		controller->velocity.x = 0.0f;
	}

	if (collision.sides &
	    (COLLISION_SIDE_NEGATIVE_Z | COLLISION_SIDE_POSITIVE_Z)) {
		controller->velocity.z = 0.0f;
	}

	if (collision.sides & COLLISION_SIDE_NEGATIVE_Y) {
		controller->velocity.y = 0.0f;
	}

	if (collision.sides & COLLISION_SIDE_POSITIVE_Y) {
		controller->velocity.y = 0.0f;
		controller->grounded = true;
	}
}