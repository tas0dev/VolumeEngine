/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_DYNAMIC_H
#define VOLUME_ENTITY_PROP_DYNAMIC_H

#include "entity/prop.h"

typedef prop_properties_t prop_dynamic_properties_t;
typedef prop_t prop_dynamic_t;

prop_dynamic_properties_t prop_dynamic_properties_create(void);
prop_dynamic_t *
prop_dynamic_create(entity_id_t id,
		    const prop_dynamic_properties_t *properties);
void prop_dynamic_destroy(prop_dynamic_t *prop);
entity_t *prop_dynamic_get_entity(prop_dynamic_t *prop);
const entity_t *prop_dynamic_get_const_entity(const prop_dynamic_t *prop);
prop_dynamic_t *prop_dynamic_from_entity(entity_t *entity);
const prop_dynamic_t *prop_dynamic_from_const_entity(const entity_t *entity);
bool prop_dynamic_register(void);

#endif
