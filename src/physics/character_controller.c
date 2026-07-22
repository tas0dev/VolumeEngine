/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "physics/character_controller.h"
#include <math.h>

static vec3_t character_controller_project_on_plane(vec3_t vector,
						    vec3_t normal);
static void
character_controller_update_crouch(character_controller_t *controller,
				   const collision_world_t *world,
				   collision_filter_t filter,
				   bool wants_crouch,
				   float delta_time);
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
character_controller_clip_velocity(character_controller_t *controller,
				   vec3_t normal);
static bool
character_controller_find_ground(const character_controller_t *controller,
				 const collision_result_t *collision,
				 vec3_t *ground_normal,
				 entity_id_t *ground_entity_id);
static void
character_controller_resolve_contacts(character_controller_t *controller,
				      const collision_result_t *collision);
static void character_controller_slide_move(character_controller_t *controller,
					    const collision_world_t *world,
					    collision_filter_t filter,
					    bool allow_step,
					    float delta_time);
static void
character_controller_slide_move_core(character_controller_t *controller,
				     const collision_world_t *world,
				     collision_filter_t filter,
				     float delta_time);
static float horizontal_distance_squared(vec3_t first, vec3_t second);
static vec3_t character_controller_clip_against_planes(vec3_t velocity,
						       const vec3_t *planes,
						       size_t plane_count);

character_controller_t character_controller_create(const vec3_t position,
						   const float radius,
						   const float height) {
	character_controller_t controller;
	float half_height;

	half_height = height * 0.5f;

	controller.bounds =
		aabb_create(vec3_create(0.0f, half_height, 0.0f),
			    vec3_create(radius, half_height, radius));
	controller.standing_bounds = controller.bounds;
	half_height = height * 0.3f;
	controller.crouched_bounds =
		aabb_create(vec3_create(0.0f, half_height, 0.0f),
			    vec3_create(radius, half_height, radius));
	controller.position = position;
	controller.velocity = vec3_create(0.0f, 0.0f, 0.0f);
	controller.ground_normal = vec3_create(0.0f, 1.0f, 0.0f);
	controller.maximum_speed = 4.0f;
	controller.ground_acceleration = 10.0f;
	controller.air_acceleration = 12.0f;
	controller.air_speed_cap = 0.8f;
	controller.friction = 4.0f;
	controller.stop_speed = 1.25f;
	controller.gravity = -20.0f;
	controller.jump_speed = 7.0f;
	controller.ground_stick_speed = 0.5f;
	controller.step_height = 0.35f;
	controller.standing_view_height = height - 0.15f;
	controller.crouched_view_height = height * 0.6f - 0.15f;
	controller.view_height = controller.standing_view_height;
	controller.crouch_transition_speed = 4.0f;
	controller.crouched_speed_multiplier = 0.34f;
	controller.minimum_ground_normal_y = 0.7f;
	controller.ground_entity_id = 0;
	controller.grounded = false;
	controller.crouched = false;

	return controller;
}

bool character_controller_jump(character_controller_t *controller) {
	if (controller == NULL || !controller->grounded) { return false; }

	controller->velocity.y = controller->jump_speed;
	controller->ground_normal = vec3_create(0.0f, 1.0f, 0.0f);
	controller->ground_entity_id = 0;
	controller->grounded = false;

	return true;
}

void character_controller_move(character_controller_t *controller,
			       const collision_world_t *world,
			       const character_move_input_t *input,
			       const float delta_time) {
	character_controller_move_ignoring(controller, world, 0, input,
					   delta_time);
}

void character_controller_move_ignoring(character_controller_t *controller,
					const collision_world_t *world,
					const entity_id_t ignored_entity_id,
					const character_move_input_t *input,
					const float delta_time) {
	collision_filter_t filter;

	filter.layer = COLLISION_LAYER_ALL;
	filter.mask = COLLISION_LAYER_ALL;
	filter.ignored_entity_id = ignored_entity_id;
	character_controller_move_filtered(controller, world, filter, input,
					   delta_time);
}

void character_controller_move_filtered(character_controller_t *controller,
					const collision_world_t *world,
					const collision_filter_t filter,
					const character_move_input_t *input,
					const float delta_time) {
	vec3_t wish_direction;
	float wish_speed;
	float direction_length;
	bool allow_step;
	bool wants_crouch;

	if (controller == NULL || delta_time <= 0.0f) { return; }

	wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
	wish_speed = 0.0f;
	wants_crouch = false;

	if (input != NULL) {
		wish_direction = input->wish_direction;
		wish_direction.y = 0.0f;
		wish_speed = input->wish_speed;
		wants_crouch = input->crouch;

		if (input->jump) { character_controller_jump(controller); }
	}

	character_controller_update_crouch(controller, world, filter,
					   wants_crouch, delta_time);

	direction_length = vec3_length(wish_direction);

	if (direction_length > 0.000001f && wish_speed > 0.0f) {
		wish_direction =
			vec3_scale(wish_direction, 1.0f / direction_length);

		if (wish_speed > controller->maximum_speed) {
			wish_speed = controller->maximum_speed;
		}
	} else {
		wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
		wish_speed = 0.0f;
	}

	if (controller->crouched) {
		wish_speed *= controller->crouched_speed_multiplier;
	}

	allow_step = controller->grounded;

	if (controller->grounded) {
		wish_direction = character_controller_project_on_plane(
			wish_direction, controller->ground_normal);

		direction_length = vec3_length(wish_direction);

		if (direction_length > 0.000001f) {
			wish_direction = vec3_scale(wish_direction,
						    1.0f / direction_length);
		} else {
			wish_direction = vec3_create(0.0f, 0.0f, 0.0f);
		}

		character_controller_clip_velocity(controller,
						   controller->ground_normal);
		character_controller_apply_friction(controller, delta_time);
		character_controller_accelerate(
			controller, wish_direction, wish_speed,
			controller->ground_acceleration, delta_time);

		controller->velocity = vec3_subtract(
			controller->velocity,
			vec3_scale(controller->ground_normal,
				   controller->ground_stick_speed));
	} else {
		character_controller_air_accelerate(controller, wish_direction,
						    wish_speed, delta_time);

		controller->velocity.y +=
			controller->gravity * delta_time * 0.5f;
	}

	controller->grounded = false;
	controller->ground_normal = vec3_create(0.0f, 1.0f, 0.0f);
	controller->ground_entity_id = 0;

	character_controller_slide_move(controller, world, filter, allow_step,
					delta_time);

	if (!controller->grounded) {
		controller->velocity.y +=
			controller->gravity * delta_time * 0.5f;
	} else {
		character_controller_clip_velocity(controller,
						   controller->ground_normal);
	}
}

static void
character_controller_update_crouch(character_controller_t *controller,
				   const collision_world_t *world,
				   const collision_filter_t filter,
				   const bool wants_crouch,
				   const float delta_time) {
	vec3_t test_position;
	float target_height;
	float change;

	if (controller == NULL) { return; }

	if (wants_crouch && !controller->crouched) {
		controller->bounds = controller->crouched_bounds;
		controller->crouched = true;
	} else if (!wants_crouch && controller->crouched) {
		test_position = controller->position;
		if (world == NULL || !collision_world_resolve_aabb_filtered(
					     world, controller->standing_bounds,
					     &test_position, filter, NULL)) {
			controller->bounds = controller->standing_bounds;
			controller->crouched = false;
		}
	}

	target_height = controller->crouched ? controller->crouched_view_height
					     : controller->standing_view_height;
	change = controller->crouch_transition_speed * delta_time;
	if (controller->view_height < target_height) {
		controller->view_height += change;
		if (controller->view_height > target_height) {
			controller->view_height = target_height;
		}
	} else if (controller->view_height > target_height) {
		controller->view_height -= change;
		if (controller->view_height < target_height) {
			controller->view_height = target_height;
		}
	}
}

static vec3_t character_controller_project_on_plane(const vec3_t vector,
						    const vec3_t normal) {
	return vec3_subtract(vector,
			     vec3_scale(normal, vec3_dot(vector, normal)));
}

static void
character_controller_apply_friction(character_controller_t *controller,
				    const float delta_time) {
	float speed;
	float control_speed;
	float speed_drop;
	float new_speed;
	float scale;

	speed = sqrtf(controller->velocity.x * controller->velocity.x +
		      controller->velocity.z * controller->velocity.z);

	if (speed < 0.001f) {
		controller->velocity.x = 0.0f;
		controller->velocity.z = 0.0f;
		return;
	}

	control_speed = speed;

	if (control_speed < controller->stop_speed) {
		control_speed = controller->stop_speed;
	}

	speed_drop = control_speed * controller->friction * delta_time;
	new_speed = speed - speed_drop;

	if (new_speed < 0.0f) { new_speed = 0.0f; }

	if (new_speed == speed) { return; }

	scale = new_speed / speed;
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
character_controller_clip_velocity(character_controller_t *controller,
				   const vec3_t normal) {
	float into_surface;

	into_surface = vec3_dot(controller->velocity, normal);

	if (into_surface >= 0.0f) { return; }

	controller->velocity = vec3_subtract(controller->velocity,
					     vec3_scale(normal, into_surface));
}

static bool
character_controller_find_ground(const character_controller_t *controller,
				 const collision_result_t *collision,
				 vec3_t *ground_normal,
				 entity_id_t *ground_entity_id) {
	vec3_t normal;
	bool found;
	size_t index;

	if (controller == NULL || collision == NULL || ground_normal == NULL ||
	    ground_entity_id == NULL) {
		return false;
	}

	found = false;
	*ground_normal = vec3_create(0.0f, 1.0f, 0.0f);
	*ground_entity_id = 0;

	for (index = 0; index < collision->contact_count; index++) {
		normal = collision->contacts[index].normal;

		if (normal.y < controller->minimum_ground_normal_y) {
			continue;
		}

		if (!found || normal.y > ground_normal->y) {
			*ground_normal = normal;
			*ground_entity_id =
				collision->contacts[index].entity_id;
			found = true;
		}
	}

	return found;
}

static void
character_controller_resolve_contacts(character_controller_t *controller,
				      const collision_result_t *collision) {
	vec3_t ground_normal;
	entity_id_t ground_entity_id;
	size_t index;

	if (controller == NULL || collision == NULL) { return; }

	for (index = 0; index < collision->contact_count; index++) {
		character_controller_clip_velocity(
			controller, collision->contacts[index].normal);
	}

	if (!character_controller_find_ground(controller, collision,
					      &ground_normal, &ground_entity_id)) {
		return;
	}

	controller->ground_normal = ground_normal;
	controller->ground_entity_id = ground_entity_id;
	controller->grounded = true;

	character_controller_clip_velocity(controller,
					   controller->ground_normal);
}

static void character_controller_slide_move(character_controller_t *controller,
					    const collision_world_t *world,
					    const collision_filter_t filter,
					    const bool allow_step,
					    const float delta_time) {
	character_controller_t start;
	character_controller_t normal_result;
	character_controller_t step_result;
	collision_trace_t trace;
	vec3_t up_end;
	vec3_t down_end;
	float normal_distance;
	float step_distance;

	if (controller == NULL || delta_time <= 0.0f) { return; }
	if (world == NULL || !allow_step || controller->step_height <= 0.0f) {
		character_controller_slide_move_core(controller, world, filter,
						     delta_time);
		return;
	}

	start = *controller;
	character_controller_slide_move_core(controller, world, filter,
					     delta_time);
	normal_result = *controller;

	*controller = start;
	up_end = vec3_add(start.position,
			  vec3_create(0.0f, controller->step_height, 0.0f));
	if (collision_world_trace_aabb_filtered(world, controller->bounds,
						start.position, up_end, filter,
						&trace) &&
	    (trace.started_inside || trace.fraction <= 0.000001f)) {
		*controller = normal_result;
		return;
	}
	controller->position = trace.hit ? trace.position : up_end;
	if (controller->velocity.y < 0.0f) { controller->velocity.y = 0.0f; }
	controller->grounded = false;

	character_controller_slide_move_core(controller, world, filter,
					     delta_time);
	down_end = vec3_subtract(
		controller->position,
		vec3_create(0.0f, controller->step_height + 0.05f, 0.0f));
	if (!collision_world_trace_aabb_filtered(world, controller->bounds,
						 controller->position, down_end,
						 filter, &trace) ||
	    trace.started_inside ||
	    trace.normal.y < controller->minimum_ground_normal_y) {
		*controller = normal_result;
		return;
	}

	controller->position = trace.position;
	controller->grounded = true;
	controller->ground_normal = trace.normal;
	controller->ground_entity_id = trace.entity_id;
	character_controller_clip_velocity(controller, trace.normal);
	step_result = *controller;

	normal_distance = horizontal_distance_squared(start.position,
						      normal_result.position);
	step_distance = horizontal_distance_squared(start.position,
						    step_result.position);
	*controller = step_distance > normal_distance + 0.000001f
			      ? step_result
			      : normal_result;
}

static void
character_controller_slide_move_core(character_controller_t *controller,
				     const collision_world_t *world,
				     const collision_filter_t filter,
				     const float delta_time) {
	const unsigned int maximum_bumps = 4;
	const size_t maximum_planes = 5;
	const float movement_epsilon = 0.0001f;
	collision_result_t overlap;
	collision_trace_t trace;
	vec3_t planes[5];
	vec3_t movement;
	vec3_t start;
	vec3_t end;
	float movement_length;
	float backed_fraction;
	float remaining_time;
	size_t plane_count;
	unsigned int bump;

	if (controller == NULL || delta_time <= 0.0f) { return; }

	if (world == NULL) {
		controller->position =
			vec3_add(controller->position,
				 vec3_scale(controller->velocity, delta_time));
		return;
	}

	if (collision_world_resolve_aabb_filtered(world, controller->bounds,
						  &controller->position, filter,
						  &overlap)) {
		character_controller_resolve_contacts(controller, &overlap);
	}

	remaining_time = delta_time;
	plane_count = 0;

	for (bump = 0; bump < maximum_bumps; bump++) {
		if (vec3_length(controller->velocity) <= 0.000001f) {
			controller->velocity = vec3_create(0.0f, 0.0f, 0.0f);
			break;
		}

		start = controller->position;
		movement = vec3_scale(controller->velocity, remaining_time);
		end = vec3_add(start, movement);

		if (!collision_world_trace_aabb_filtered(
			    world, controller->bounds, start, end, filter,
			    &trace)) {
			controller->position = end;
			break;
		}

		if (trace.started_inside) {
			if (!collision_world_resolve_aabb_filtered(
				    world, controller->bounds,
				    &controller->position, filter, &overlap)) {
				controller->velocity =
					vec3_create(0.0f, 0.0f, 0.0f);
				break;
			}

			character_controller_resolve_contacts(controller,
							      &overlap);

			controller->velocity =
				character_controller_clip_against_planes(
					controller->velocity, planes,
					plane_count);

			continue;
		}

		movement_length = vec3_length(movement);
		backed_fraction = trace.fraction;

		if (movement_length > movement_epsilon) {
			backed_fraction -= movement_epsilon / movement_length;

			if (backed_fraction < 0.0f) { backed_fraction = 0.0f; }
		}

		controller->position =
			vec3_add(start, vec3_scale(movement, backed_fraction));

		if (trace.normal.y >= controller->minimum_ground_normal_y) {
			controller->grounded = true;
			controller->ground_normal = trace.normal;
			controller->ground_entity_id = trace.entity_id;
		}

		if (plane_count < maximum_planes) {
			planes[plane_count] = trace.normal;
			plane_count++;
		} else {
			controller->velocity = vec3_create(0.0f, 0.0f, 0.0f);
			break;
		}

		controller->velocity = character_controller_clip_against_planes(
			controller->velocity, planes, plane_count);

		remaining_time *= 1.0f - trace.fraction;

		if (remaining_time <= 0.000001f) { break; }
	}

	if (collision_world_resolve_aabb_filtered(world, controller->bounds,
						  &controller->position, filter,
						  &overlap)) {
		character_controller_resolve_contacts(controller, &overlap);
	}
}

static float horizontal_distance_squared(const vec3_t first,
					 const vec3_t second) {
	const float x = second.x - first.x;
	const float z = second.z - first.z;
	return x * x + z * z;
}

static vec3_t character_controller_clip_against_planes(
	const vec3_t velocity, const vec3_t *planes, const size_t plane_count) {
	const float plane_epsilon = 0.0001f;
	vec3_t clipped;
	vec3_t direction;
	float direction_length;
	float speed;
	size_t first_index;
	size_t second_index;
	size_t test_index;
	bool valid;

	if (planes == NULL || plane_count == 0) { return velocity; }

	for (first_index = 0; first_index < plane_count; first_index++) {
		clipped = velocity;

		if (vec3_dot(clipped, planes[first_index]) < 0.0f) {
			clipped = vec3_subtract(
				clipped,
				vec3_scale(planes[first_index],
					   vec3_dot(clipped,
						    planes[first_index])));
		}

		valid = true;

		for (test_index = 0; test_index < plane_count; test_index++) {
			if (test_index == first_index) { continue; }

			if (vec3_dot(clipped, planes[test_index]) <
			    -plane_epsilon) {
				valid = false;
				break;
			}
		}

		if (valid) { return clipped; }
	}

	if (plane_count == 2) {
		direction = vec3_cross(planes[0], planes[1]);
		direction_length = vec3_length(direction);

		if (direction_length <= plane_epsilon) {
			return vec3_create(0.0f, 0.0f, 0.0f);
		}

		direction = vec3_scale(direction, 1.0f / direction_length);
		speed = vec3_dot(velocity, direction);

		return vec3_scale(direction, speed);
	}

	for (first_index = 0; first_index < plane_count; first_index++) {
		for (second_index = first_index + 1; second_index < plane_count;
		     second_index++) {
			direction = vec3_cross(planes[first_index],
					       planes[second_index]);
			direction_length = vec3_length(direction);

			if (direction_length <= plane_epsilon) { continue; }

			direction =
				vec3_scale(direction, 1.0f / direction_length);
			speed = vec3_dot(velocity, direction);
			clipped = vec3_scale(direction, speed);
			valid = true;

			for (test_index = 0; test_index < plane_count;
			     test_index++) {
				if (vec3_dot(clipped, planes[test_index]) <
				    -plane_epsilon) {
					valid = false;
					break;
				}
			}

			if (valid) { return clipped; }
		}
	}

	return vec3_create(0.0f, 0.0f, 0.0f);
}
