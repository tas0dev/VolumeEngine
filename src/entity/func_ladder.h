/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_LADDER_H
#define VOLUME_ENTITY_FUNC_LADDER_H

#include "entity/entity.h"

typedef struct func_ladder func_ladder_t;

bool func_ladder_register(void);
func_ladder_t *func_ladder_from_entity(entity_t *entity);
const func_ladder_t *func_ladder_from_const_entity(const entity_t *entity);
entity_t *func_ladder_get_entity(func_ladder_t *ladder);
vec3_t func_ladder_get_normal(const func_ladder_t *ladder);

#endif
