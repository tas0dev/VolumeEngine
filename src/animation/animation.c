/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "animation/animation.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static vec3_t sample_vector(const animation_vector_key_t *keys,
			    size_t count,
			    float time,
			    vec3_t fallback);
static animation_quaternion_t
sample_rotation(const animation_rotation_key_t *keys, size_t count, float time);
static mat4_t compose_transform(vec3_t position,
				animation_quaternion_t rotation,
				vec3_t scale);
static void decompose_transform(mat4_t transform,
				vec3_t *position,
				animation_quaternion_t *rotation,
				vec3_t *scale);
static void animator_evaluate(animator_t *animator);

void animation_set_destroy(animation_set_t *set) {
	size_t clip_index;
	size_t channel_index;
	size_t node_index;

	if (set == NULL) { return; }
	for (clip_index = 0; set->clips != NULL && clip_index < set->clip_count;
	     clip_index++) {
		for (channel_index = 0;
		     channel_index < set->clips[clip_index].channel_count;
		     channel_index++) {
			free(set->clips[clip_index]
				     .channels[channel_index]
				     .positions);
			free(set->clips[clip_index]
				     .channels[channel_index]
				     .rotations);
			free(set->clips[clip_index]
				     .channels[channel_index]
				     .scales);
		}
		free(set->clips[clip_index].channels);
		free(set->clips[clip_index].name);
	}
	for (node_index = 0; set->nodes != NULL && node_index < set->node_count;
	     node_index++) {
		free(set->nodes[node_index].name);
	}
	free(set->clips);
	free(set->nodes);
	free(set);
}

const animation_clip_t *animation_set_find_clip(const animation_set_t *set,
						const char *name) {
	size_t index;

	if (set == NULL || name == NULL) { return NULL; }
	for (index = 0; index < set->clip_count; index++) {
		if (strcmp(set->clips[index].name, name) == 0) {
			return &set->clips[index];
		}
	}
	return NULL;
}

bool animator_initialize(animator_t *animator, const animation_set_t *set) {
	size_t index;

	if (animator == NULL || set == NULL || set->node_count == 0 ||
	    set->node_count > ANIMATION_MAX_NODES || set->bone_count == 0 ||
	    set->bone_count > ANIMATION_MAX_BONES) {
		return false;
	}
	*animator = (animator_t){0};
	animator->set = set;
	for (index = 0; index < ANIMATION_MAX_BONES; index++) {
		animator->bone_matrices[index] = mat4_identity();
	}
	animator_evaluate(animator);
	return true;
}

bool animator_play(animator_t *animator,
		   const char *clip_name,
		   const bool looping) {
	const animation_clip_t *clip;

	if (animator == NULL || animator->set == NULL) { return false; }
	clip = animation_set_find_clip(animator->set, clip_name);
	if (clip == NULL || !isfinite(clip->duration) ||
	    clip->duration <= 0.0f || !isfinite(clip->ticks_per_second) ||
	    clip->ticks_per_second <= 0.0f) {
		return false;
	}
	animator->clip = clip;
	animator->time = 0.0f;
	animator->looping = looping;
	animator->playing = true;
	animator_evaluate(animator);
	return true;
}

void animator_update(animator_t *animator, const float delta_time) {
	float duration_seconds;

	if (animator == NULL || !animator->playing || animator->clip == NULL ||
	    !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	duration_seconds =
		animator->clip->duration / animator->clip->ticks_per_second;
	animator->time += delta_time;
	if (animator->time >= duration_seconds) {
		if (animator->looping && duration_seconds > 0.0f) {
			animator->time =
				fmodf(animator->time, duration_seconds);
		} else {
			animator->time = duration_seconds;
			animator->playing = false;
		}
	}
	animator_evaluate(animator);
}

bool animator_is_playing(const animator_t *animator) {
	return animator != NULL && animator->playing;
}

const mat4_t *animator_get_bone_matrices(const animator_t *animator,
					 size_t *count) {
	if (count != NULL) {
		*count = animator == NULL || animator->set == NULL
				 ? 0
				 : animator->set->bone_count;
	}
	return animator == NULL || animator->set == NULL
		       ? NULL
		       : animator->bone_matrices;
}

static void animator_evaluate(animator_t *animator) {
	mat4_t local;
	mat4_t global;
	const animation_channel_t *channel;
	float tick_time;
	size_t node_index;
	size_t channel_index;
	int bone_index;
	vec3_t bind_position;
	vec3_t bind_scale;
	animation_quaternion_t bind_rotation;

	tick_time = animator->clip == NULL
			    ? 0.0f
			    : animator->time * animator->clip->ticks_per_second;
	for (node_index = 0; node_index < animator->set->node_count;
	     node_index++) {
		local = animator->set->nodes[node_index].bind_transform;
		if (animator->clip != NULL) {
			decompose_transform(local, &bind_position,
					    &bind_rotation, &bind_scale);
			for (channel_index = 0;
			     channel_index < animator->clip->channel_count;
			     channel_index++) {
				channel = &animator->clip
						   ->channels[channel_index];
				if (channel->node_index != node_index) {
					continue;
				}
				local = compose_transform(
					sample_vector(channel->positions,
						      channel->position_count,
						      tick_time, bind_position),
					channel->rotation_count == 0
						? bind_rotation
						: sample_rotation(
							  channel->rotations,
							  channel->rotation_count,
							  tick_time),
					sample_vector(channel->scales,
						      channel->scale_count,
						      tick_time, bind_scale));
				break;
			}
		}
		if (animator->set->nodes[node_index].parent_index >= 0) {
			global = mat4_multiply(
				animator->global_transforms
					[animator->set->nodes[node_index]
						 .parent_index],
				local);
		} else {
			global = local;
		}
		animator->global_transforms[node_index] = global;
		bone_index = animator->set->nodes[node_index].bone_index;
		if (bone_index >= 0 && bone_index < ANIMATION_MAX_BONES) {
			animator->bone_matrices[bone_index] = mat4_multiply(
				animator->set->inverse_root_transform,
				mat4_multiply(
					global,
					animator->set->inverse_bind_matrices
						[bone_index]));
		}
	}
}

static vec3_t sample_vector(const animation_vector_key_t *keys,
			    const size_t count,
			    const float time,
			    const vec3_t fallback) {
	size_t index;
	float amount;
	float span;

	if (keys == NULL || count == 0) { return fallback; }
	if (count == 1 || time <= keys[0].time) { return keys[0].value; }
	for (index = 0; index + 1 < count; index++) {
		if (time >= keys[index + 1].time) { continue; }
		span = keys[index + 1].time - keys[index].time;
		amount = span <= 0.000001f ? 0.0f
					   : (time - keys[index].time) / span;
		return vec3_add(keys[index].value,
				vec3_scale(vec3_subtract(keys[index + 1].value,
							 keys[index].value),
					   amount));
	}
	return keys[count - 1].value;
}

static animation_quaternion_t
sample_rotation(const animation_rotation_key_t *keys,
		const size_t count,
		const float time) {
	animation_quaternion_t first;
	animation_quaternion_t second;
	animation_quaternion_t result;
	float dot;
	float amount;
	float span;
	float length;
	size_t index;

	if (keys == NULL || count == 0) {
		return (animation_quaternion_t){0.0f, 0.0f, 0.0f, 1.0f};
	}
	if (count == 1 || time <= keys[0].time) { return keys[0].value; }
	index = count - 2;
	for (size_t candidate = 0; candidate + 1 < count; candidate++) {
		if (time < keys[candidate + 1].time) {
			index = candidate;
			break;
		}
	}
	first = keys[index].value;
	second = keys[index + 1].value;
	span = keys[index + 1].time - keys[index].time;
	amount = span <= 0.000001f ? 0.0f : (time - keys[index].time) / span;
	if (amount > 1.0f) { amount = 1.0f; }
	dot = first.x * second.x + first.y * second.y + first.z * second.z +
	      first.w * second.w;
	if (dot < 0.0f) {
		second.x = -second.x;
		second.y = -second.y;
		second.z = -second.z;
		second.w = -second.w;
	}
	result.x = first.x + (second.x - first.x) * amount;
	result.y = first.y + (second.y - first.y) * amount;
	result.z = first.z + (second.z - first.z) * amount;
	result.w = first.w + (second.w - first.w) * amount;
	length = sqrtf(result.x * result.x + result.y * result.y +
		       result.z * result.z + result.w * result.w);
	if (length <= 0.000001f) {
		return (animation_quaternion_t){0.0f, 0.0f, 0.0f, 1.0f};
	}
	result.x /= length;
	result.y /= length;
	result.z /= length;
	result.w /= length;
	return result;
}

static mat4_t compose_transform(const vec3_t position,
				const animation_quaternion_t rotation,
				const vec3_t scale) {
	mat4_t result = mat4_identity();
	float xx = rotation.x * rotation.x;
	float yy = rotation.y * rotation.y;
	float zz = rotation.z * rotation.z;
	float xy = rotation.x * rotation.y;
	float xz = rotation.x * rotation.z;
	float yz = rotation.y * rotation.z;
	float wx = rotation.w * rotation.x;
	float wy = rotation.w * rotation.y;
	float wz = rotation.w * rotation.z;

	result.elements[0] = (1.0f - 2.0f * (yy + zz)) * scale.x;
	result.elements[1] = (2.0f * (xy + wz)) * scale.x;
	result.elements[2] = (2.0f * (xz - wy)) * scale.x;
	result.elements[4] = (2.0f * (xy - wz)) * scale.y;
	result.elements[5] = (1.0f - 2.0f * (xx + zz)) * scale.y;
	result.elements[6] = (2.0f * (yz + wx)) * scale.y;
	result.elements[8] = (2.0f * (xz + wy)) * scale.z;
	result.elements[9] = (2.0f * (yz - wx)) * scale.z;
	result.elements[10] = (1.0f - 2.0f * (xx + yy)) * scale.z;
	result.elements[12] = position.x;
	result.elements[13] = position.y;
	result.elements[14] = position.z;
	return result;
}

static void decompose_transform(const mat4_t transform,
				vec3_t *position,
				animation_quaternion_t *rotation,
				vec3_t *scale) {
	float m00;
	float m11;
	float m22;
	float trace;
	float factor;
	float length;

	*position = vec3_create(transform.elements[12], transform.elements[13],
				transform.elements[14]);
	scale->x = vec3_length(vec3_create(transform.elements[0],
					   transform.elements[1],
					   transform.elements[2]));
	scale->y = vec3_length(vec3_create(transform.elements[4],
					   transform.elements[5],
					   transform.elements[6]));
	scale->z = vec3_length(vec3_create(transform.elements[8],
					   transform.elements[9],
					   transform.elements[10]));
	if (scale->x <= 0.000001f || scale->y <= 0.000001f ||
	    scale->z <= 0.000001f) {
		*rotation = (animation_quaternion_t){0.0f, 0.0f, 0.0f, 1.0f};
		return;
	}
	m00 = transform.elements[0] / scale->x;
	m11 = transform.elements[5] / scale->y;
	m22 = transform.elements[10] / scale->z;
	trace = m00 + m11 + m22;
	if (trace > 0.0f) {
		factor = sqrtf(trace + 1.0f) * 2.0f;
		rotation->w = 0.25f * factor;
		rotation->x = (transform.elements[6] / scale->y -
			       transform.elements[9] / scale->z) /
			      factor;
		rotation->y = (transform.elements[8] / scale->z -
			       transform.elements[2] / scale->x) /
			      factor;
		rotation->z = (transform.elements[1] / scale->x -
			       transform.elements[4] / scale->y) /
			      factor;
	} else if (m00 > m11 && m00 > m22) {
		factor = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
		rotation->w = (transform.elements[6] / scale->y -
			       transform.elements[9] / scale->z) /
			      factor;
		rotation->x = 0.25f * factor;
		rotation->y = (transform.elements[4] / scale->y +
			       transform.elements[1] / scale->x) /
			      factor;
		rotation->z = (transform.elements[8] / scale->z +
			       transform.elements[2] / scale->x) /
			      factor;
	} else if (m11 > m22) {
		factor = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
		rotation->w = (transform.elements[8] / scale->z -
			       transform.elements[2] / scale->x) /
			      factor;
		rotation->x = (transform.elements[4] / scale->y +
			       transform.elements[1] / scale->x) /
			      factor;
		rotation->y = 0.25f * factor;
		rotation->z = (transform.elements[9] / scale->z +
			       transform.elements[6] / scale->y) /
			      factor;
	} else {
		factor = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
		rotation->w = (transform.elements[1] / scale->x -
			       transform.elements[4] / scale->y) /
			      factor;
		rotation->x = (transform.elements[8] / scale->z +
			       transform.elements[2] / scale->x) /
			      factor;
		rotation->y = (transform.elements[9] / scale->z +
			       transform.elements[6] / scale->y) /
			      factor;
		rotation->z = 0.25f * factor;
	}
	length = sqrtf(rotation->x * rotation->x + rotation->y * rotation->y +
		       rotation->z * rotation->z + rotation->w * rotation->w);
	if (length > 0.000001f) {
		rotation->x /= length;
		rotation->y /= length;
		rotation->z /= length;
		rotation->w /= length;
	}
}
