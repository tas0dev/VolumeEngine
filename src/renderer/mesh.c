/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "renderer/mesh.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct mesh {
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLsizei index_count;
	aabb_t bounds;
};

mesh_t *mesh_create(const mesh_vertex_t *vertices,
		    const size_t vertex_count,
		    const unsigned int *indices,
		    const size_t index_count) {
	mesh_t *mesh;
	vec3_t minimum;
	vec3_t maximum;
	vec3_t center;
	vec3_t half_extents;
	size_t index;

	if (vertices == NULL || vertex_count == 0 || indices == NULL ||
	    index_count == 0) {
		log_error("Invalid mesh data");
		return NULL;
	}

	if (index_count > INT32_MAX) {
		log_error("Mesh contains too many indices");
		return NULL;
	}

	mesh = calloc(1, sizeof(*mesh));
	if (mesh == NULL) {
		log_error("Failed to allocate mesh");
		return NULL;
	}

	minimum = vec3_create(vertices[0].position[0], vertices[0].position[1],
			      vertices[0].position[2]);
	maximum = minimum;

	for (index = 1; index < vertex_count; index++) {
		if (vertices[index].position[0] < minimum.x) {
			minimum.x = vertices[index].position[0];
		}

		if (vertices[index].position[1] < minimum.y) {
			minimum.y = vertices[index].position[1];
		}

		if (vertices[index].position[2] < minimum.z) {
			minimum.z = vertices[index].position[2];
		}

		if (vertices[index].position[0] > maximum.x) {
			maximum.x = vertices[index].position[0];
		}

		if (vertices[index].position[1] > maximum.y) {
			maximum.y = vertices[index].position[1];
		}

		if (vertices[index].position[2] > maximum.z) {
			maximum.z = vertices[index].position[2];
		}
	}

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);

	mesh->bounds = aabb_create(center, half_extents);
	mesh->index_count = (GLsizei)index_count;

	glGenVertexArrays(1, &mesh->vertex_array);
	glBindVertexArray(mesh->vertex_array);

	glGenBuffers(1, &mesh->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		     (GLsizeiptr)(vertex_count * sizeof(*vertices)), vertices,
		     GL_STATIC_DRAW);

	glGenBuffers(1, &mesh->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     (GLsizeiptr)(index_count * sizeof(*indices)), indices,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, color));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		3,
		2,
		GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
		(const void *)offsetof(mesh_vertex_t, texture_coordinate));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, tangent));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(
		5, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, bitangent));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return mesh;
}

void mesh_destroy(mesh_t *mesh) {
	if (mesh == NULL) { return; }

	if (mesh->index_buffer != 0) {
		glDeleteBuffers(1, &mesh->index_buffer);
	}

	if (mesh->vertex_buffer != 0) {
		glDeleteBuffers(1, &mesh->vertex_buffer);
	}

	if (mesh->vertex_array != 0) {
		glDeleteVertexArrays(1, &mesh->vertex_array);
	}

	free(mesh);
}

void mesh_draw(const mesh_t *mesh) {
	if (mesh == NULL) { return; }

	glBindVertexArray(mesh->vertex_array);
	glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

bool mesh_get_bounds(const mesh_t *mesh, aabb_t *bounds) {
	if (mesh == NULL || bounds == NULL) {
		return false;
	}

	*bounds = mesh->bounds;

	return true;
}