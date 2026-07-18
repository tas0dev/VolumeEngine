/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/triangle.h"
#include <float.h>
#include <math.h>
#include <stddef.h>

static bool test_separating_axis(vec3_t axis,
				 const vec3_t *vertices,
				 vec3_t half_extents,
				 float *minimum_depth,
				 vec3_t *minimum_normal);

triangle_t
triangle_create(const vec3_t first, const vec3_t second, const vec3_t third) {
	triangle_t triangle;
	vec3_t first_edge;
	vec3_t second_edge;

	triangle.vertices[0] = first;
	triangle.vertices[1] = second;
	triangle.vertices[2] = third;

	first_edge = vec3_subtract(second, first);
	second_edge = vec3_subtract(third, first);
	triangle.normal = vec3_normalize(vec3_cross(first_edge, second_edge));

	return triangle;
}

aabb_t triangle_get_bounds(const triangle_t triangle) {
	vec3_t minimum;
	vec3_t maximum;
	vec3_t center;
	vec3_t half_extents;
	size_t index;

	minimum = triangle.vertices[0];
	maximum = triangle.vertices[0];

	for (index = 1; index < 3; index++) {
		if (triangle.vertices[index].x < minimum.x) {
			minimum.x = triangle.vertices[index].x;
		}

		if (triangle.vertices[index].y < minimum.y) {
			minimum.y = triangle.vertices[index].y;
		}

		if (triangle.vertices[index].z < minimum.z) {
			minimum.z = triangle.vertices[index].z;
		}

		if (triangle.vertices[index].x > maximum.x) {
			maximum.x = triangle.vertices[index].x;
		}

		if (triangle.vertices[index].y > maximum.y) {
			maximum.y = triangle.vertices[index].y;
		}

		if (triangle.vertices[index].z > maximum.z) {
			maximum.z = triangle.vertices[index].z;
		}
	}

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);

	return aabb_create(center, half_extents);
}

bool aabb_get_triangle_collision(const aabb_t aabb,
				 const triangle_t triangle,
				 aabb_collision_t *collision) {
	vec3_t box_center;
	vec3_t half_extents;
	vec3_t vertices[3];
	vec3_t edges[3];
	vec3_t box_axes[3];
	vec3_t minimum_normal;
	float minimum_depth;
	size_t edge_index;
	size_t axis_index;

	if (collision == NULL) { return false; }

	box_center = aabb_get_center(aabb);
	half_extents = aabb_get_half_extents(aabb);

	vertices[0] = vec3_subtract(triangle.vertices[0], box_center);
	vertices[1] = vec3_subtract(triangle.vertices[1], box_center);
	vertices[2] = vec3_subtract(triangle.vertices[2], box_center);

	edges[0] = vec3_subtract(vertices[1], vertices[0]);
	edges[1] = vec3_subtract(vertices[2], vertices[1]);
	edges[2] = vec3_subtract(vertices[0], vertices[2]);

	box_axes[0] = vec3_create(1.0f, 0.0f, 0.0f);
	box_axes[1] = vec3_create(0.0f, 1.0f, 0.0f);
	box_axes[2] = vec3_create(0.0f, 0.0f, 1.0f);

	minimum_depth = FLT_MAX;
	minimum_normal = vec3_create(0.0f, 0.0f, 0.0f);

	for (axis_index = 0; axis_index < 3; axis_index++) {
		if (!test_separating_axis(box_axes[axis_index], vertices,
					  half_extents, &minimum_depth,
					  &minimum_normal)) {
			return false;
		}
	}

	if (!test_separating_axis(triangle.normal, vertices, half_extents,
				  &minimum_depth, &minimum_normal)) {
		return false;
	}

	for (edge_index = 0; edge_index < 3; edge_index++) {
		for (axis_index = 0; axis_index < 3; axis_index++) {
			if (!test_separating_axis(
				    vec3_cross(edges[edge_index],
					       box_axes[axis_index]),
				    vertices, half_extents, &minimum_depth,
				    &minimum_normal)) {
				return false;
			}
		}
	}

	if (minimum_depth == FLT_MAX || minimum_depth <= 0.000001f) {
		return false;
	}

	collision->normal = minimum_normal;
	collision->depth = minimum_depth;

	return true;
}

static bool test_separating_axis(vec3_t axis,
				 const vec3_t *vertices,
				 const vec3_t half_extents,
				 float *minimum_depth,
				 vec3_t *minimum_normal) {
	float axis_length;
	float first_projection;
	float second_projection;
	float third_projection;
	float triangle_minimum;
	float triangle_maximum;
	float box_radius;
	float negative_shift;
	float positive_shift;
	float depth;
	vec3_t normal;

	axis_length = vec3_length(axis);

	if (axis_length <= 0.000001f) { return true; }

	axis = vec3_scale(axis, 1.0f / axis_length);

	first_projection = vec3_dot(vertices[0], axis);
	second_projection = vec3_dot(vertices[1], axis);
	third_projection = vec3_dot(vertices[2], axis);

	triangle_minimum = fminf(first_projection,
				 fminf(second_projection, third_projection));
	triangle_maximum = fmaxf(first_projection,
				 fmaxf(second_projection, third_projection));

	box_radius = half_extents.x * fabsf(axis.x) +
		     half_extents.y * fabsf(axis.y) +
		     half_extents.z * fabsf(axis.z);

	if (box_radius <= triangle_minimum || triangle_maximum <= -box_radius) {
		return false;
	}

	negative_shift = triangle_minimum - box_radius;
	positive_shift = triangle_maximum + box_radius;

	if (-negative_shift < positive_shift) {
		depth = -negative_shift;
		normal = vec3_scale(axis, -1.0f);
	} else {
		depth = positive_shift;
		normal = axis;
	}

	if (depth < *minimum_depth) {
		*minimum_depth = depth;
		*minimum_normal = normal;
	}

	return true;
}
