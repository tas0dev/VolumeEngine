/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "renderer/mesh.h"
#include "collision/triangle.h"
#include "collision/triangle_mesh_collider.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct mesh {
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLsizei index_count;
	aabb_t bounds;
	triangle_mesh_collider_t *collision_mesh;
};

static triangle_mesh_collider_t *
create_collision_mesh(const mesh_vertex_t *vertices,
		      const size_t vertex_count,
		      const unsigned int *indices,
		      const size_t index_count) {
	triangle_mesh_collider_t *collider;
	triangle_t *triangles;
	size_t triangle_count;
	size_t triangle_index;
	size_t index_offset;
	unsigned int first_index;
	unsigned int second_index;
	unsigned int third_index;
	vec3_t first;
	vec3_t second;
	vec3_t third;

	if (index_count % 3 != 0) {
		log_error("Mesh index count is not divisible by three");
		return NULL;
	}

	triangle_count = index_count / 3;

	if (triangle_count > SIZE_MAX / sizeof(*triangles)) {
		log_error("Mesh collision data is too large");
		return NULL;
	}

	triangles = malloc(triangle_count * sizeof(*triangles));

	if (triangles == NULL) {
		log_error("Failed to allocate mesh collision data");
		return NULL;
	}

	for (triangle_index = 0; triangle_index < triangle_count;
	     triangle_index++) {
		index_offset = triangle_index * 3;

		first_index = indices[index_offset];
		second_index = indices[index_offset + 1];
		third_index = indices[index_offset + 2];

		if (first_index >= vertex_count ||
		    second_index >= vertex_count ||
		    third_index >= vertex_count) {
			log_error("Mesh contains an invalid index");
			free(triangles);
			return NULL;
		}

		first = vec3_create(vertices[first_index].position[0],
				    vertices[first_index].position[1],
				    vertices[first_index].position[2]);
		second = vec3_create(vertices[second_index].position[0],
				     vertices[second_index].position[1],
				     vertices[second_index].position[2]);
		third = vec3_create(vertices[third_index].position[0],
				    vertices[third_index].position[1],
				    vertices[third_index].position[2]);

		triangles[triangle_index] =
			triangle_create(first, second, third);
	}

	collider = triangle_mesh_collider_create(triangles, triangle_count);

	free(triangles);

	return collider;
}

mesh_t *mesh_create(const mesh_vertex_t *vertices,
		    const size_t vertex_count,
		    const unsigned int *indices,
		    const size_t index_count) {
	mesh_t *mesh;

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

	mesh->collision_mesh = create_collision_mesh(vertices, vertex_count,
						     indices, index_count);

	if (mesh->collision_mesh == NULL ||
	    !triangle_mesh_collider_get_bounds(mesh->collision_mesh,
					       &mesh->bounds)) {
		triangle_mesh_collider_destroy(mesh->collision_mesh);
		free(mesh);
		return NULL;
	}

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
		3, 2, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
		(const void *)offsetof(mesh_vertex_t, texture_coordinate));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, tangent));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, bitangent));
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return mesh;
}

void mesh_destroy(mesh_t *mesh) {
	if (mesh == NULL) { return; }

	triangle_mesh_collider_destroy(mesh->collision_mesh);

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
	if (mesh == NULL || bounds == NULL) { return false; }

	*bounds = mesh->bounds;

	return true;
}

const triangle_mesh_collider_t *mesh_get_collision_mesh(const mesh_t *mesh) {
	if (mesh == NULL) { return NULL; }

	return mesh->collision_mesh;
}