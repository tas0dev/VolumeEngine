/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_INTERNAL_H
#define VOLUME_ENTITY_PROP_INTERNAL_H

#include "entity/prop_static.h"

entity_t *prop_internal_create_static(entity_id_t id,
				      const entity_spawn_context_t *context);
void prop_internal_draw_shadow(entity_t *entity, renderer_t *renderer);
void prop_internal_draw(entity_t *entity,
			renderer_t *renderer,
			const render_view_t *view);
void prop_internal_destroy(entity_t *entity);

#endif
