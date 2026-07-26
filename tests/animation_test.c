/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "animation/animation.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *text) {
	size_t length = strlen(text);
	char *copy = malloc(length + 1);
	if (copy != NULL) { memcpy(copy, text, length + 1); }
	return copy;
}

static bool test_animator_interpolation(void) {
	animation_set_t *set;
	animation_channel_t *channel;
	animator_t animator;
	const mat4_t *matrices;
	size_t count;

	set = calloc(1, sizeof(*set));
	CHECK(set != NULL);
	set->node_count = 1;
	set->nodes = calloc(1, sizeof(*set->nodes));
	set->clip_count = 1;
	set->clips = calloc(1, sizeof(*set->clips));
	CHECK(set->nodes != NULL);
	CHECK(set->clips != NULL);
	set->nodes[0].name = copy_text("root");
	set->nodes[0].parent_index = -1;
	set->nodes[0].bone_index = 0;
	set->nodes[0].bind_transform = mat4_identity();
	set->bone_count = 1;
	set->inverse_bind_matrices[0] = mat4_identity();
	set->inverse_root_transform = mat4_identity();
	set->clips[0].name = copy_text("fire");
	set->clips[0].duration = 1.0f;
	set->clips[0].ticks_per_second = 1.0f;
	set->clips[0].channel_count = 1;
	set->clips[0].channels = calloc(1, sizeof(*set->clips[0].channels));
	CHECK(set->nodes[0].name != NULL);
	CHECK(set->clips[0].name != NULL);
	CHECK(set->clips[0].channels != NULL);
	channel = &set->clips[0].channels[0];
	channel->node_index = 0;
	channel->position_count = 2;
	channel->positions = calloc(2, sizeof(*channel->positions));
	channel->rotation_count = 1;
	channel->rotations = calloc(1, sizeof(*channel->rotations));
	channel->scale_count = 1;
	channel->scales = calloc(1, sizeof(*channel->scales));
	CHECK(channel->positions != NULL);
	CHECK(channel->rotations != NULL);
	CHECK(channel->scales != NULL);
	channel->positions[0] =
		(animation_vector_key_t){0.0f, vec3_create(0.0f, 0.0f, 0.0f)};
	channel->positions[1] =
		(animation_vector_key_t){1.0f, vec3_create(2.0f, 0.0f, 0.0f)};
	channel->rotations[0].value =
		(animation_quaternion_t){0.0f, 0.0f, 0.0f, 1.0f};
	channel->scales[0].value = vec3_create(1.0f, 1.0f, 1.0f);
	CHECK(animator_initialize(&animator, set));
	CHECK(animator_play(&animator, "fire", false));
	animator_update(&animator, 0.5f);
	matrices = animator_get_bone_matrices(&animator, &count);
	CHECK(matrices != NULL);
	CHECK(count == 1);
	CHECK(fabsf(matrices[0].elements[12] - 1.0f) < 0.0001f);
	animator_update(&animator, 0.5f);
	CHECK(!animator_is_playing(&animator));
	animation_set_destroy(set);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"animator interpolation", test_animator_interpolation},
	};
	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
