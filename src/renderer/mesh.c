/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/mesh.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdlib.h>

struct mesh {
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLsizei vertex_count;
};

mesh_t *mesh_create(const mesh_vertex_t *vertices, size_t vertex_count) {
	if (vertices == NULL || vertex_count == 0) {
		log_error("Invalid mesh data");
		return NULL;
	}

	if (vertex_count > INT32_MAX) {
		log_error("Mesh contains too many vertices");
		return NULL;
	}

	mesh_t *mesh = calloc(1, sizeof(*mesh));
	if (mesh == NULL) {
		log_error("Failed to allocate mesh");
		return NULL;
	}

	mesh->vertex_count = (GLsizei)vertex_count;

	glGenVertexArrays(1, &mesh->vertex_array);
	glBindVertexArray(mesh->vertex_array);

	glGenBuffers(1, &mesh->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		     (GLsizeiptr)(vertex_count * sizeof(*vertices)), vertices,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t),
			      (const void *)offsetof(mesh_vertex_t, color));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return mesh;
}

void mesh_destroy(mesh_t *mesh) {
	if (mesh == NULL) { return; }

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
	glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
	glBindVertexArray(0);
}