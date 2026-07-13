/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/mesh_loader.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void set_error(char *error, size_t error_size, const char *format, ...);
static bool is_triangle_mesh(const struct aiMesh *mesh);
static bool count_mesh_data(const struct aiScene *scene,
			    size_t *vertex_count,
			    size_t *index_count,
			    char *error,
			    size_t error_size);
static bool copy_mesh_data(const struct aiScene *scene,
			   mesh_vertex_t *vertices,
			   unsigned int *indices,
			   char *error,
			   size_t error_size);

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static bool is_triangle_mesh(const struct aiMesh *mesh) {
	if (mesh == NULL) { return false; }

	return (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != 0;
}

static bool count_mesh_data(const struct aiScene *scene,
			    size_t *vertex_count,
			    size_t *index_count,
			    char *error,
			    const size_t error_size) {
	const struct aiMesh *mesh;
	const struct aiFace *face;
	size_t mesh_index;
	size_t face_index;

	*vertex_count = 0;
	*index_count = 0;

	for (mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {
		mesh = scene->mMeshes[mesh_index];

		if (!is_triangle_mesh(mesh)) { continue; }

		if (mesh->mVertices == NULL) {
			set_error(error, error_size, "mesh %zu has no vertices",
				  mesh_index);
			return false;
		}

		if (mesh->mNormals == NULL) {
			set_error(error, error_size, "mesh %zu has no normals",
				  mesh_index);
			return false;
		}

		if (*vertex_count > UINT_MAX - mesh->mNumVertices) {
			set_error(error, error_size,
				  "model contains too many vertices");
			return false;
		}

		*vertex_count += mesh->mNumVertices;

		for (face_index = 0; face_index < mesh->mNumFaces;
		     face_index++) {
			face = &mesh->mFaces[face_index];

			if (face->mNumIndices != 3) {
				set_error(
					error, error_size,
					"mesh %zu contains a non-triangle face",
					mesh_index);
				return false;
			}

			if (*index_count > SIZE_MAX - 3) {
				set_error(error, error_size,
					  "model contains too many indices");
				return false;
			}

			*index_count += 3;
		}
	}

	if (*vertex_count == 0 || *index_count == 0) {
		set_error(error, error_size,
			  "model contains no triangle meshes");
		return false;
	}

	if (*vertex_count > SIZE_MAX / sizeof(mesh_vertex_t)) {
		set_error(error, error_size, "vertex buffer is too large");
		return false;
	}

	if (*index_count > SIZE_MAX / sizeof(unsigned int)) {
		set_error(error, error_size, "index buffer is too large");
		return false;
	}

	return true;
}

static bool copy_mesh_data(const struct aiScene *scene,
			   mesh_vertex_t *vertices,
			   unsigned int *indices,
			   char *error,
			   const size_t error_size) {
	const struct aiMesh *mesh;
	const struct aiFace *face;
	const struct aiVector3D *position;
	const struct aiVector3D *normal;
	const struct aiVector3D *texture_coordinate;
	const struct aiColor4D *color;
	size_t mesh_index;
	size_t vertex_index;
	size_t face_index;
	size_t face_vertex_index;
	size_t vertex_offset;
	size_t index_offset;
	size_t destination_vertex;
	size_t destination_index;

	vertex_offset = 0;
	index_offset = 0;

	for (mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {
		mesh = scene->mMeshes[mesh_index];

		if (!is_triangle_mesh(mesh)) { continue; }

		for (vertex_index = 0; vertex_index < mesh->mNumVertices;
		     vertex_index++) {
			destination_vertex = vertex_offset + vertex_index;

			position = &mesh->mVertices[vertex_index];
			normal = &mesh->mNormals[vertex_index];

			vertices[destination_vertex].position[0] =
				(float)position->x;
			vertices[destination_vertex].position[1] =
				(float)position->y;
			vertices[destination_vertex].position[2] =
				(float)position->z;

			vertices[destination_vertex].normal[0] =
				(float)normal->x;
			vertices[destination_vertex].normal[1] =
				(float)normal->y;
			vertices[destination_vertex].normal[2] =
				(float)normal->z;

			color = mesh->mColors[0] == NULL
					? NULL
					: &mesh->mColors[0][vertex_index];

			if (color == NULL) {
				vertices[destination_vertex].color[0] = 1.0f;
				vertices[destination_vertex].color[1] = 1.0f;
				vertices[destination_vertex].color[2] = 1.0f;
			} else {
				vertices[destination_vertex].color[0] =
					(float)color->r;
				vertices[destination_vertex].color[1] =
					(float)color->g;
				vertices[destination_vertex].color[2] =
					(float)color->b;
			}

			texture_coordinate =
				mesh->mTextureCoords[0] == NULL
					? NULL
					: &mesh->mTextureCoords[0]
							       [vertex_index];

			if (texture_coordinate == NULL) {
				vertices[destination_vertex]
					.texture_coordinate[0] = 0.0f;
				vertices[destination_vertex]
					.texture_coordinate[1] = 0.0f;
			} else {
				vertices[destination_vertex]
					.texture_coordinate[0] =
					(float)texture_coordinate->x;
				vertices[destination_vertex]
					.texture_coordinate[1] =
					(float)texture_coordinate->y;
			}
		}

		for (face_index = 0; face_index < mesh->mNumFaces;
		     face_index++) {
			face = &mesh->mFaces[face_index];

			for (face_vertex_index = 0; face_vertex_index < 3;
			     face_vertex_index++) {
				if (face->mIndices[face_vertex_index] >=
				    mesh->mNumVertices) {
					set_error(error, error_size,
						  "mesh %zu contains "
						  "an invalid index",
						  mesh_index);
					return false;
				}

				destination_index =
					vertex_offset +
					face->mIndices[face_vertex_index];

				indices[index_offset++] =
					(unsigned int)destination_index;
			}
		}

		vertex_offset += mesh->mNumVertices;
	}

	return true;
}

mesh_t *mesh_load(const char *path, char *error, const size_t error_size) {
	const struct aiScene *scene;
	mesh_vertex_t *vertices;
	unsigned int *indices;
	mesh_t *mesh;
	size_t vertex_count;
	size_t index_count;
	const unsigned int flags =
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices |
		aiProcess_ImproveCacheLocality | aiProcess_SortByPType |
		aiProcess_ValidateDataStructure;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (path == NULL || path[0] == '\0') {
		set_error(error, error_size, "invalid mesh path");
		return NULL;
	}

	scene = aiImportFile(path, flags);
	if (scene == NULL) {
		set_error(error, error_size, "failed to import \"%s\": %s",
			  path, aiGetErrorString());
		return NULL;
	}

	if (scene->mMeshes == NULL || scene->mNumMeshes == 0) {
		set_error(error, error_size, "model \"%s\" contains no meshes",
			  path);
		aiReleaseImport(scene);
		return NULL;
	}

	if (!count_mesh_data(scene, &vertex_count, &index_count, error,
			     error_size)) {
		aiReleaseImport(scene);
		return NULL;
	}

	vertices = malloc(vertex_count * sizeof(*vertices));
	if (vertices == NULL) {
		set_error(error, error_size,
			  "failed to allocate vertex buffer");
		aiReleaseImport(scene);
		return NULL;
	}

	indices = malloc(index_count * sizeof(*indices));
	if (indices == NULL) {
		set_error(error, error_size, "failed to allocate index buffer");
		free(vertices);
		aiReleaseImport(scene);
		return NULL;
	}

	if (!copy_mesh_data(scene, vertices, indices, error, error_size)) {
		free(indices);
		free(vertices);
		aiReleaseImport(scene);
		return NULL;
	}

	mesh = mesh_create(vertices, vertex_count, indices, index_count);

	free(indices);
	free(vertices);
	aiReleaseImport(scene);

	if (mesh == NULL) {
		set_error(error, error_size,
			  "failed to create GPU mesh for \"%s\"", path);
		return NULL;
	}

	return mesh;
}
