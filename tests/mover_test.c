/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "entity/mover.h"
#include <math.h>

static bool test_accelerates_decelerates_and_interpolates_transform(void) {
	entity_t entity = {0};
	mover_config_t config = {0};
	mover_t mover;
	size_t iteration;

	entity_initialize(&entity, 1, NULL);
	config.start_transform = transform_create();
	config.end_transform = transform_create();
	config.end_transform.position.x = 4.0f;
	config.end_transform.rotation.y = 2.0f;
	config.end_transform.scale = vec3_create(3.0f, 3.0f, 3.0f);
	config.speed = 4.0f;
	config.acceleration = 2.0f;
	config.deceleration = 2.0f;
	config.wait = -1.0f;
	config.block_policy = MOVER_BLOCK_STOP;

	CHECK(mover_initialize(&mover, &entity, &config));
	CHECK(entity.collider_follows_transform);
	CHECK(mover_move_to_end(&mover, NULL));
	mover_update(&mover, 0.5f);
	CHECK(fabsf(entity.transform.position.x - 0.5f) < 0.0001f);
	CHECK(fabsf(entity.transform.rotation.y - 0.25f) < 0.0001f);
	CHECK(fabsf(entity.transform.scale.x - 1.25f) < 0.0001f);

	mover_update(&mover, 0.5f);
	CHECK(fabsf(entity.transform.position.x - 1.5f) < 0.0001f);
	CHECK(fabsf(entity.transform.rotation.y - 0.75f) < 0.0001f);
	CHECK(fabsf(entity.transform.scale.x - 1.75f) < 0.0001f);

	for (iteration = 0;
	     iteration < 200 && mover_get_state(&mover) != MOVER_AT_END;
	     iteration++) {
		mover_update(&mover, 0.05f);
	}
	CHECK(mover_get_state(&mover) == MOVER_AT_END);
	CHECK(fabsf(entity.transform.position.x - 4.0f) < 0.0001f);
	CHECK(fabsf(entity.transform.rotation.y - 2.0f) < 0.0001f);
	CHECK(fabsf(entity.transform.scale.x - 3.0f) < 0.0001f);
	return true;
}

static bool test_move_towards_and_block_policy_parser(void) {
	mover_block_policy_t policy;
	vec3_t position = {0};

	CHECK(!mover_move_towards(&position, vec3_create(2.0f, 0.0f, 0.0f),
				  -1.0f));
	CHECK(!mover_move_towards(&position, vec3_create(2.0f, 0.0f, 0.0f),
				  0.5f));
	CHECK(fabsf(position.x - 0.5f) < 0.0001f);
	CHECK(mover_move_towards(&position, vec3_create(2.0f, 0.0f, 0.0f),
				 2.0f));
	CHECK(fabsf(position.x - 2.0f) < 0.0001f);

	CHECK(mover_parse_block_policy("stop", &policy));
	CHECK(policy == MOVER_BLOCK_STOP);
	CHECK(mover_parse_block_policy("reverse", &policy));
	CHECK(policy == MOVER_BLOCK_REVERSE);
	CHECK(mover_parse_block_policy("ignore", &policy));
	CHECK(policy == MOVER_BLOCK_IGNORE);
	CHECK(!mover_parse_block_policy("invalid", &policy));
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"accelerates, decelerates, and interpolates transform",
		 test_accelerates_decelerates_and_interpolates_transform},
		{"move towards and block policy parser",
		 test_move_towards_and_block_policy_parser		  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
