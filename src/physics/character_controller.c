/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "physics/character_controller.h"
#include <math.h>

static void
character_controller_apply_friction(character_controller_t *controller,
				    float delta_time);
static void character_controller_accelerate(character_controller_t *controller,
					    vec3_t wish_direction,
					    float wish_speed,
					    float acceleration,
					    float delta_time);
static void
character_controller_air_accelerate(character_controller_t *controller,
				    vec3_t wish_direction,
				    float wish_speed,
				    float delta_time);
static void
character_controller_resolve_velocity(character_controller_t *controller,
				      const collision_result_t *collision);

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
	controller.maximum_speed = 4.0f;
	controller.ground_acceleration = 10.0f;
	controller.air_acceleration = 10.0f;
	controller.air_speed_cap = 0.4f;
	controller.friction = 4.0f;
	controller.stop_speed = 1.25f;
	controller.gravity = -20.0f;
	controller.jump_speed = 7.0f;
	controller.grounded = false;

	return controller;
}

bool character_controller_jump(character_controller_t *controller) {
	if (controller == NULL || !controller->grounded) {
		return false;
	}

	controller->velocity.y = controller->jump_speed;
	controller->grounded = false;

	return true;
}

void character_controller_move(character_controller_t *controller,
			       const collision_world_t *world,
	const character_move_input_t *input,
			       const float delta_time) {
	collision_result_t collision;
	vec3_t wish_direction;
	float wish_speed;
	float direction_length;

	if (controller == NULL || delta_time <= 0.0f) { return; }

	wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
	wish_speed = 0.0f;

	if (input != NULL) {
		wish_direction = input->wish_direction;
		wish_direction.y = 0.0f;
		wish_speed = input->wish_speed;

		if (input->jump) { character_controller_jump(controller); }
	}

	direction_length = vec3_length(wish_direction);

	if (direction_length > 0.0f && wish_speed > 0.0f) {
		wish_direction =
			vec3_scale(wish_direction, 1.0f / direction_length);

		if (wish_speed > controller->maximum_speed) {
			wish_speed = controller->maximum_speed;
		}
	} else {
		wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
		wish_speed = 0.0f;
	}

	if (controller->grounded) {
		character_controller_apply_friction(controller, delta_time);
		character_controller_accelerate(
			controller, wish_direction, wish_speed,
			controller->ground_acceleration, delta_time);
	} else {
		character_controller_air_accelerate(controller, wish_direction,
						    wish_speed, delta_time);
	}

	controller->velocity.y += controller->gravity * delta_time * 0.5f;

	controller->position =
		vec3_add(
		controller->position,
		vec3_scale(
			controller->velocity, delta_time));

	controller->grounded = false;

	if (world != NULL &&
	    collision_world_resolve_aabb(world, controller->bounds,
					 &controller->position, &collision)) {
		character_controller_resolve_velocity(controller, &collision);
	}

	if (!controller->grounded) {
		controller->velocity.y +=
			controller->gravity * delta_time * 0.5f;
	}
}

static void
character_controller_apply_friction(character_controller_t *controller,
				    const float delta_time) {
	float horizontal_speed;
	float control_speed;
	float speed_drop;
	float new_speed;
	float scale;

	horizontal_speed = controller->velocity.x * controller->velocity.x +
			   controller->velocity.z * controller->velocity.z;

	if (horizontal_speed <= 0.0f) { return; }

	horizontal_speed = sqrtf(horizontal_speed);

	if (horizontal_speed < 0.001f) {
		controller->velocity.x = 0.0f;
		controller->velocity.z = 0.0f;
		return;
	}

	control_speed = horizontal_speed;

	if (control_speed < controller->stop_speed) {
		control_speed = controller->stop_speed;
	}

	speed_drop = control_speed * controller->friction * delta_time;
	new_speed = horizontal_speed - speed_drop;

	if (new_speed < 0.0f) { new_speed = 0.0f; }

	if (new_speed == horizontal_speed) { return; }

	scale = new_speed / horizontal_speed;
	controller->velocity.x *= scale;
	controller->velocity.z *= scale;
}

static void character_controller_accelerate(character_controller_t *controller,
					    const vec3_t wish_direction,
					    const float wish_speed,
					    const float acceleration,
					    const float delta_time) {
	float current_speed;
	float additional_speed;
	float acceleration_speed;

	if (wish_speed <= 0.0f) { return; }

	current_speed = vec3_dot(controller->velocity, wish_direction);
	additional_speed = wish_speed - current_speed;

	if (additional_speed <= 0.0f) { return; }

	acceleration_speed = acceleration * wish_speed * delta_time;

	if (acceleration_speed > additional_speed) {
		acceleration_speed = additional_speed;
	}

	controller->velocity =
		vec3_add(controller->velocity,
			 vec3_scale(wish_direction, acceleration_speed));
}

static void
character_controller_air_accelerate(character_controller_t *controller,
				    const vec3_t wish_direction,
				    const float wish_speed,
				    const float delta_time) {
	float capped_wish_speed;
	float current_speed;
	float additional_speed;
	float acceleration_speed;

	if (wish_speed <= 0.0f) { return; }

	capped_wish_speed = wish_speed;

	if (capped_wish_speed > controller->air_speed_cap) {
		capped_wish_speed = controller->air_speed_cap;
	}

	current_speed = vec3_dot(controller->velocity, wish_direction);
	additional_speed = capped_wish_speed - current_speed;

	if (additional_speed <= 0.0f) { return; }

	acceleration_speed =
		controller->air_acceleration * wish_speed * delta_time;

	if (acceleration_speed > additional_speed) {
		acceleration_speed = additional_speed;
	}

	controller->velocity =
		vec3_add(controller->velocity,
			 vec3_scale(wish_direction, acceleration_speed));
}

static void
character_controller_resolve_velocity(character_controller_t *controller,
				      const collision_result_t *collision) {
	if (collision->sides &
	    (COLLISION_SIDE_NEGATIVE_X | COLLISION_SIDE_POSITIVE_X)) {
		controller->velocity.x = 0.0f;
	}

	if (collision->sides &
	    (COLLISION_SIDE_NEGATIVE_Z | COLLISION_SIDE_POSITIVE_Z)) {
		controller->velocity.z = 0.0f;
	}

	if (collision->sides & COLLISION_SIDE_NEGATIVE_Y) {
		controller->velocity.y = 0.0f;
	}

	if (collision->sides & COLLISION_SIDE_POSITIVE_Y) {
		controller->velocity.y = 0.0f;
		controller->grounded = true;
	}
}