/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/prop_static.h"
#include "asset/manager.h"
#include "entity/prop_internal.h"
#include "renderer/renderer.h"
#include "scene/transform.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static void
set_error(const entity_spawn_context_t *context, const char *format, ...) {
	va_list arguments;

	if (context->error == NULL || context->error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(context->error, context->error_size, format, arguments);
	va_end(arguments);
}

static bool create_model_collider(const mesh_t *mesh,
				  const transform_t *transform,
				  collider_t *collider) {
	aabb_t bounds;
	mat4_t rotation_x;
	mat4_t rotation_y;
	mat4_t rotation_z;
	mat4_t rotation;
	mat4_t scale;
	mat4_t matrix;
	vec3_t corners[8];
	vec3_t minimum;
	vec3_t maximum;
	vec3_t transformed;
	vec3_t center;
	vec3_t half_extents;
	size_t index;

	if (mesh == NULL || transform == NULL || collider == NULL ||
	    !mesh_get_bounds(mesh, &bounds)) {
		return false;
	}

	corners[0] = vec3_create(bounds.minimum.x, bounds.minimum.y,
				 bounds.minimum.z);
	corners[1] = vec3_create(bounds.maximum.x, bounds.minimum.y,
				 bounds.minimum.z);
	corners[2] = vec3_create(bounds.minimum.x, bounds.maximum.y,
				 bounds.minimum.z);
	corners[3] = vec3_create(bounds.maximum.x, bounds.maximum.y,
				 bounds.minimum.z);
	corners[4] = vec3_create(bounds.minimum.x, bounds.minimum.y,
				 bounds.maximum.z);
	corners[5] = vec3_create(bounds.maximum.x, bounds.minimum.y,
				 bounds.maximum.z);
	corners[6] = vec3_create(bounds.minimum.x, bounds.maximum.y,
				 bounds.maximum.z);
	corners[7] = vec3_create(bounds.maximum.x, bounds.maximum.y,
				 bounds.maximum.z);

	rotation_x = mat4_rotation_x(transform->rotation.x);
	rotation_y = mat4_rotation_y(transform->rotation.y);
	rotation_z = mat4_rotation_z(transform->rotation.z);
	scale = mat4_scale(transform->scale);

	rotation = mat4_multiply(rotation_z, rotation_y);
	rotation = mat4_multiply(rotation, rotation_x);
	matrix = mat4_multiply(rotation, scale);

	minimum = mat4_transform_point(matrix, corners[0]);
	maximum = minimum;

	for (index = 1; index < 8; index++) {
		transformed = mat4_transform_point(matrix, corners[index]);

		if (transformed.x < minimum.x) { minimum.x = transformed.x; }

		if (transformed.y < minimum.y) { minimum.y = transformed.y; }

		if (transformed.z < minimum.z) { minimum.z = transformed.z; }

		if (transformed.x > maximum.x) { maximum.x = transformed.x; }

		if (transformed.y > maximum.y) { maximum.y = transformed.y; }

		if (transformed.z > maximum.z) { maximum.z = transformed.z; }
	}

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);

	*collider = collider_create_box(center, half_extents);

	return true;
}

static bool create_triangle_mesh_collider(const mesh_t *mesh,
					  const transform_t *transform,
					  collider_t *collider) {
	const triangle_mesh_collider_t *collision_mesh;
	transform_t local_transform;
	mat4_t matrix;

	if (mesh == NULL || transform == NULL || collider == NULL) {
		return false;
	}

	collision_mesh = mesh_get_collision_mesh(mesh);

	if (collision_mesh == NULL) { return false; }

	local_transform = *transform;
	local_transform.position = vec3_create(0.0f, 0.0f, 0.0f);
	matrix = transform_get_matrix(&local_transform);

	*collider = collider_create_triangle_mesh(collision_mesh, matrix);

	return collider->type == COLLIDER_TYPE_TRIANGLE_MESH;
}

prop_static_properties_t prop_static_properties_create(void) {
	prop_static_properties_t properties;

	properties.entity = entity_properties_create();
	properties.mesh = NULL;
	properties.material = NULL;
	properties.casts_shadow = true;
	properties.has_collider = false;
	properties.collider = collider_create_none();

	return properties;
}

static const entity_class_t prop_static_class = {
	.classname = "prop_static",
	.create = prop_internal_create_static,
	.update = NULL,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.destroy = prop_internal_destroy,
};

prop_static_t *prop_static_create(const entity_id_t id,
				  const prop_static_properties_t *properties) {
	prop_static_t *prop;

	if (properties == NULL || properties->mesh == NULL ||
	    properties->material == NULL) {
		return NULL;
	}

	prop = calloc(1, sizeof(*prop));
	if (prop == NULL) { return NULL; }

	entity_initialize((entity_t *)prop, id, &prop_static_class);
	entity_set_collision_filter((entity_t *)prop,
				    COLLISION_LAYER_WORLD_STATIC,
				    COLLISION_LAYER_ALL);

	if (!entity_set_targetname((entity_t *)prop,
				   properties->entity.targetname)) {
		free(prop);
		return NULL;
	}

	prop->entity.transform = properties->entity.transform;
	prop->mesh = properties->mesh;
	prop->material = properties->material;
	prop->casts_shadow = properties->casts_shadow;

	if (properties->has_collider) {
		entity_set_collider((entity_t *)prop, properties->collider);
	}

	return prop;
}

void prop_static_destroy(prop_static_t *prop) {
	if (prop == NULL) { return; }

	entity_destroy((entity_t *)prop);
}

entity_t *prop_static_get_entity(prop_static_t *prop) {
	if (prop == NULL) { return NULL; }

	return (entity_t *)prop;
}

bool prop_internal_initialize(prop_t *prop,
			      const entity_id_t id,
			      const entity_class_t *class,
			      const entity_spawn_context_t *context) {
	entity_t *temporary;

	if (prop == NULL || class == NULL) { return false; }

	temporary = prop_internal_create_static(id, context);
	if (temporary == NULL) { return false; }

	*prop = *(prop_t *)temporary;
	prop->entity.class = class;
	free(temporary);
	return true;
}

const entity_t *prop_static_get_const_entity(const prop_static_t *prop) {
	if (prop == NULL) { return NULL; }

	return (const entity_t *)prop;
}

prop_static_t *prop_static_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &prop_static_class) {
		return NULL;
	}

	return (prop_static_t *)entity;
}

const prop_static_t *prop_static_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &prop_static_class) {
		return NULL;
	}

	return (const prop_static_t *)entity;
}

void prop_internal_draw_shadow(entity_t *entity, renderer_t *renderer) {
	prop_static_t *prop;
	mat4_t model;

	prop = (prop_static_t *)entity;
	if (prop == NULL || !prop->casts_shadow) { return; }

	model = entity_get_world_matrix(entity);
	renderer_draw_shadow_mesh(renderer, prop->mesh, &model);
}

void prop_internal_draw(entity_t *entity,
			renderer_t *renderer,
			const render_view_t *view) {
	prop_static_t *prop;
	mat4_t model;

	prop = (prop_static_t *)entity;
	if (prop == NULL) { return; }

	model = entity_get_world_matrix(entity);

	renderer_draw_mesh(renderer, prop->mesh, prop->material, &model, view);
}

void prop_internal_destroy(entity_t *entity) { free((prop_static_t *)entity); }

bool prop_static_register(void) {
	return entity_register_class(&prop_static_class);
}

entity_t *prop_internal_create_static(const entity_id_t id,
				      const entity_spawn_context_t *context) {
	prop_static_properties_t properties;
	prop_static_t *prop;
	const char *model_path;
	const char *material_path;
	const char *casts_shadow;
	const char *collision_type;
	const char *collision_size;
	const char *collision_center;
	vec3_t size;
	vec3_t center;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}

	properties = prop_static_properties_create();
	properties.entity = *context->properties;

	model_path = entity_property_get(context->source, "model");

	if (model_path == NULL || model_path[0] == '\0') {
		set_error(context, "prop_static requires a model");
		return NULL;
	}

	material_path = entity_property_get(context->source, "material");

	if (material_path == NULL || material_path[0] == '\0') {
		set_error(context, "prop_static requires a material");
		return NULL;
	}

	if (context->assets == NULL) {
		set_error(context,
			  "prop_static assets require an asset manager");
		return NULL;
	}

	properties.mesh =
		asset_manager_load_mesh(context->assets, model_path,
					context->error, context->error_size);

	if (properties.mesh == NULL) { return NULL; }

	properties.material = asset_manager_load_material(
		context->assets, material_path, context->error,
		context->error_size);

	if (properties.material == NULL) { return NULL; }

	casts_shadow = entity_property_get(context->source, "casts_shadow");

	if (casts_shadow != NULL &&
	    !entity_property_parse_bool(casts_shadow,
					&properties.casts_shadow)) {
		set_error(context,
			  "invalid boolean property "
			  "\"casts_shadow\": \"%s\"",
			  casts_shadow);
		return NULL;
	}

	collision_type = entity_property_get(context->source, "collision");

	if (collision_type != NULL && strcmp(collision_type, "none") != 0) {
		if (strcmp(collision_type, "model") == 0) {
			if (!create_triangle_mesh_collider(
				    properties.mesh,
				    &properties.entity.transform,
				    &properties.collider)) {
				set_error(context,
					  "failed to create model collider");
				return NULL;
			}

			properties.has_collider = true;
		} else if (strcmp(collision_type, "bounds") == 0) {
			if (!create_model_collider(properties.mesh,
						   &properties.entity.transform,
						   &properties.collider)) {
				set_error(context,
					  "failed to create bounds collider");
				return NULL;
			}

			properties.has_collider = true;
		} else if (strcmp(collision_type, "box") == 0) {
			collision_size = entity_property_get(context->source,
							     "collision_size");

			if (collision_size == NULL ||
			    !entity_property_parse_vec3(collision_size,
							&size)) {
				set_error(context, "prop_static box collision "
						   "requires a valid "
						   "collision_size");
				return NULL;
			}

			if (size.x <= 0.0f || size.y <= 0.0f ||
			    size.z <= 0.0f) {
				set_error(context,
					  "collision_size must be positive");
				return NULL;
			}

			center = vec3_create(0.0f, 0.0f, 0.0f);

			collision_center = entity_property_get(
				context->source, "collision_center");

			if (collision_center != NULL &&
			    !entity_property_parse_vec3(collision_center,
							&center)) {
				set_error(context,
					  "invalid collision_center: \"%s\"",
					  collision_center);
				return NULL;
			}

			properties.collider = collider_create_box(
				center, vec3_scale(size, 0.5f));
			properties.has_collider = true;
		} else {
			set_error(context, "unsupported collision type: \"%s\"",
				  collision_type);
			return NULL;
		}
	}

	prop = prop_static_create(id, &properties);

	if (prop == NULL) {
		set_error(context, "failed to create prop_static");
		return NULL;
	}

	return (entity_t *)prop;
}
