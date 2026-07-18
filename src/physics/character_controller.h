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

typedef struct character_move_input {
	vec3_t wish_direction;
	float wish_speed;
	bool jump;
} character_move_input_t;

typedef struct character_controller {
	aabb_t bounds;
	vec3_t position;
	vec3_t velocity;
	vec3_t ground_normal;
	float maximum_speed;
	float ground_acceleration;
	float air_acceleration;
	float air_speed_cap;
	float friction;
	float stop_speed;
	float gravity;
	float jump_speed;
	float ground_stick_speed;
	float minimum_ground_normal_y;
	bool grounded;
} character_controller_t;

character_controller_t
character_controller_create(vec3_t position, float radius, float height);
bool character_controller_jump(character_controller_t *controller);
void character_controller_move(character_controller_t *controller,
			       const collision_world_t *world,
			       const character_move_input_t *input,
			       float delta_time);

#endif