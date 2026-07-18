/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_PHYSICS_CHARACTER_CONTROLLER_H
#define VOLUME_PHYSICS_CHARACTER_CONTROLLER_H
#include "collision/aabb.h"
#include "collision/collision_world.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef struct character_controller {
	aabb_t bounds;
	vec3_t position;
	vec3_t velocity;
	float gravity;
	float jump_speed;
	bool grounded;
} character_controller_t;

character_controller_t
character_controller_create(vec3_t position, float radius, float height);
void character_controller_set_horizontal_velocity(
	character_controller_t *controller, vec3_t velocity);
bool character_controller_jump(character_controller_t *controller);
void character_controller_update(character_controller_t *controller,
				 const collision_world_t *world,
				 float delta_time);

#endif