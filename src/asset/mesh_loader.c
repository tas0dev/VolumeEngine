/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/mesh_loader.h"
#include "animation/animation.h"
#include "math/vec3.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
			   const animation_set_t *animations,
			   char *error,
			   size_t error_size);
static void write_tangent_basis(const struct aiMesh *mesh,
				size_t vertex_index,
				const struct aiVector3D *normal,
				mesh_vertex_t *vertex);
static bool scene_has_bones(const struct aiScene *scene);
static animation_set_t *
load_animation_set(const struct aiScene *scene, char *error, size_t error_size);
static size_t count_nodes(const struct aiNode *node);
static bool copy_nodes(const struct aiNode *node,
		       int parent_index,
		       animation_set_t *set,
		       size_t *next_index);
static int find_node(const animation_set_t *set, const char *name);
static mat4_t convert_matrix(const struct aiMatrix4x4 *matrix);
static char *duplicate_string(const char *text);
static void
add_bone_weight(mesh_vertex_t *vertex, int bone_index, float weight);
static void normalize_bone_weights(mesh_vertex_t *vertex);

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
			   const animation_set_t *animations,
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
	size_t bone_index;
	size_t weight_index;
	int node_index;
	int mapped_bone_index;

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

			write_tangent_basis(mesh, vertex_index, normal,
					    &vertices[destination_vertex]);
		}

		if (animations != NULL) {
			for (bone_index = 0; bone_index < mesh->mNumBones;
			     bone_index++) {
				node_index = find_node(
					animations,
					mesh->mBones[bone_index]->mName.data);
				if (node_index < 0) { continue; }
				mapped_bone_index =
					animations->nodes[node_index]
						.bone_index;
				for (weight_index = 0;
				     weight_index <
				     mesh->mBones[bone_index]->mNumWeights;
				     weight_index++) {
					if (mesh->mBones[bone_index]
						    ->mWeights[weight_index]
						    .mVertexId >=
					    mesh->mNumVertices) {
						continue;
					}
					add_bone_weight(
						&vertices
							[vertex_offset +
							 mesh->mBones[bone_index]
								 ->mWeights
									 [weight_index]
								 .mVertexId],
						mapped_bone_index,
						(float)mesh->mBones[bone_index]
							->mWeights[weight_index]
							.mWeight);
				}
			}
			for (vertex_index = 0;
			     vertex_index < mesh->mNumVertices;
			     vertex_index++) {
				normalize_bone_weights(&vertices[vertex_offset +
								 vertex_index]);
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

static void write_tangent_basis(const struct aiMesh *mesh,
				const size_t vertex_index,
				const struct aiVector3D *normal,
				mesh_vertex_t *vertex) {
	const struct aiVector3D *source_tangent;
	const struct aiVector3D *source_bitangent;
	vec3_t normal_vector;
	vec3_t tangent;
	vec3_t bitangent;
	vec3_t reference;
	float tangent_length;
	float bitangent_length;

	source_tangent =
		mesh->mTangents == NULL ? NULL : &mesh->mTangents[vertex_index];
	source_bitangent = mesh->mBitangents == NULL
				   ? NULL
				   : &mesh->mBitangents[vertex_index];

	if (source_tangent != NULL && source_bitangent != NULL) {
		tangent = vec3_create((float)source_tangent->x,
				      (float)source_tangent->y,
				      (float)source_tangent->z);
		bitangent = vec3_create((float)source_bitangent->x,
					(float)source_bitangent->y,
					(float)source_bitangent->z);

		tangent_length = vec3_length(tangent);
		bitangent_length = vec3_length(bitangent);

		if (tangent_length > 0.000001f &&
		    bitangent_length > 0.000001f) {
			vertex->tangent[0] = tangent.x;
			vertex->tangent[1] = tangent.y;
			vertex->tangent[2] = tangent.z;

			vertex->bitangent[0] = bitangent.x;
			vertex->bitangent[1] = bitangent.y;
			vertex->bitangent[2] = bitangent.z;
			return;
		}
	}

	normal_vector = vec3_create((float)normal->x, (float)normal->y,
				    (float)normal->z);

	if (vec3_length(normal_vector) <= 0.000001f) {
		normal_vector = vec3_create(0.0f, 1.0f, 0.0f);
	} else {
		normal_vector = vec3_normalize(normal_vector);
	}

	if (fabsf(normal_vector.y) < 0.999f) {
		reference = vec3_create(0.0f, 1.0f, 0.0f);
	} else {
		reference = vec3_create(1.0f, 0.0f, 0.0f);
	}

	tangent = vec3_normalize(vec3_cross(reference, normal_vector));
	bitangent = vec3_normalize(vec3_cross(normal_vector, tangent));

	vertex->tangent[0] = tangent.x;
	vertex->tangent[1] = tangent.y;
	vertex->tangent[2] = tangent.z;

	vertex->bitangent[0] = bitangent.x;
	vertex->bitangent[1] = bitangent.y;
	vertex->bitangent[2] = bitangent.z;
}

mesh_t *mesh_load(const char *path, char *error, const size_t error_size) {
	const struct aiScene *scene;
	mesh_vertex_t *vertices;
	unsigned int *indices;
	mesh_t *mesh;
	size_t vertex_count;
	size_t index_count;
	animation_set_t *animations = NULL;
	unsigned int flags =
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality | aiProcess_SortByPType |
		aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights;

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
	if (!scene_has_bones(scene)) {
		aiReleaseImport(scene);
		flags |= aiProcess_PreTransformVertices;
		scene = aiImportFile(path, flags);
		if (scene == NULL) {
			set_error(error, error_size,
				  "failed to import \"%s\": %s", path,
				  aiGetErrorString());
			return NULL;
		}
	} else {
		animations = load_animation_set(scene, error, error_size);
		if (animations == NULL) {
			aiReleaseImport(scene);
			return NULL;
		}
	}

	if (!count_mesh_data(scene, &vertex_count, &index_count, error,
			     error_size)) {
		aiReleaseImport(scene);
		return NULL;
	}

	vertices = calloc(vertex_count, sizeof(*vertices));
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

	if (!copy_mesh_data(scene, vertices, indices, animations, error,
			    error_size)) {
		free(indices);
		free(vertices);
		animation_set_destroy(animations);
		aiReleaseImport(scene);
		return NULL;
	}

	mesh = animations == NULL
		       ? mesh_create(vertices, vertex_count, indices,
				     index_count)
		       : mesh_create_animated(vertices, vertex_count, indices,
					      index_count, animations);

	free(indices);
	free(vertices);
	aiReleaseImport(scene);

	if (mesh == NULL) {
		animation_set_destroy(animations);
		set_error(error, error_size,
			  "failed to create GPU mesh for \"%s\"", path);
		return NULL;
	}

	return mesh;
}

static bool scene_has_bones(const struct aiScene *scene) {
	size_t index;

	for (index = 0; index < scene->mNumMeshes; index++) {
		if (scene->mMeshes[index]->mNumBones > 0) { return true; }
	}
	return false;
}

static animation_set_t *load_animation_set(const struct aiScene *scene,
					   char *error,
					   const size_t error_size) {
	animation_set_t *set;
	size_t next_node = 0;
	size_t mesh_index;
	size_t source_bone_index;
	size_t clip_index;
	size_t source_channel_index;
	size_t channel_index;
	size_t key_index;
	int node_index;
	int bone_index;
	const struct aiAnimation *source_clip;
	const struct aiNodeAnim *source_channel;
	animation_channel_t *channel;
	mat4_t root_transform;

	set = calloc(1, sizeof(*set));
	if (set == NULL) { return NULL; }
	set->node_count = count_nodes(scene->mRootNode);
	if (set->node_count > ANIMATION_MAX_NODES) {
		set_error(error, error_size, "model exceeds the %d node limit",
			  ANIMATION_MAX_NODES);
		animation_set_destroy(set);
		return NULL;
	}
	set->nodes = calloc(set->node_count, sizeof(*set->nodes));
	if (set->nodes == NULL ||
	    !copy_nodes(scene->mRootNode, -1, set, &next_node)) {
		animation_set_destroy(set);
		return NULL;
	}
	root_transform = convert_matrix(&scene->mRootNode->mTransformation);
	if (!mat4_inverse_affine(&root_transform,
				 &set->inverse_root_transform)) {
		set->inverse_root_transform = mat4_identity();
	}
	for (mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++) {
		for (source_bone_index = 0;
		     source_bone_index < scene->mMeshes[mesh_index]->mNumBones;
		     source_bone_index++) {
			node_index = find_node(
				set, scene->mMeshes[mesh_index]
					     ->mBones[source_bone_index]
					     ->mName.data);
			if (node_index < 0) { continue; }
			if (set->nodes[node_index].bone_index >= 0) {
				continue;
			}
			if (set->bone_count >= ANIMATION_MAX_BONES) {
				set_error(error, error_size,
					  "model exceeds the %d bone limit",
					  ANIMATION_MAX_BONES);
				animation_set_destroy(set);
				return NULL;
			}
			bone_index = (int)set->bone_count++;
			set->nodes[node_index].bone_index = bone_index;
			set->inverse_bind_matrices[bone_index] = convert_matrix(
				&scene->mMeshes[mesh_index]
					 ->mBones[source_bone_index]
					 ->mOffsetMatrix);
		}
	}
	set->clip_count = scene->mNumAnimations;
	if (set->clip_count == 0) { return set; }
	set->clips = calloc(set->clip_count, sizeof(*set->clips));
	if (set->clips == NULL) {
		animation_set_destroy(set);
		return NULL;
	}
	for (clip_index = 0; clip_index < set->clip_count; clip_index++) {
		source_clip = scene->mAnimations[clip_index];
		set->clips[clip_index].name = duplicate_string(
			source_clip->mName.length > 0 ? source_clip->mName.data
						      : "animation");
		set->clips[clip_index].duration = (float)source_clip->mDuration;
		set->clips[clip_index].ticks_per_second =
			source_clip->mTicksPerSecond > 0.0
				? (float)source_clip->mTicksPerSecond
				: 25.0f;
		set->clips[clip_index].channels =
			calloc(source_clip->mNumChannels,
			       sizeof(*set->clips[clip_index].channels));
		if (set->clips[clip_index].name == NULL ||
		    set->clips[clip_index].channels == NULL) {
			animation_set_destroy(set);
			return NULL;
		}
		channel_index = 0;
		for (source_channel_index = 0;
		     source_channel_index < source_clip->mNumChannels;
		     source_channel_index++) {
			source_channel =
				source_clip->mChannels[source_channel_index];
			node_index =
				find_node(set, source_channel->mNodeName.data);
			if (node_index < 0) { continue; }
			channel = &set->clips[clip_index]
					   .channels[channel_index++];
			set->clips[clip_index].channel_count = channel_index;
			channel->node_index = (size_t)node_index;
			channel->position_count =
				source_channel->mNumPositionKeys;
			channel->rotation_count =
				source_channel->mNumRotationKeys;
			channel->scale_count = source_channel->mNumScalingKeys;
			channel->positions =
				calloc(channel->position_count,
				       sizeof(*channel->positions));
			channel->rotations =
				calloc(channel->rotation_count,
				       sizeof(*channel->rotations));
			channel->scales = calloc(channel->scale_count,
						 sizeof(*channel->scales));
			if ((channel->position_count > 0 &&
			     channel->positions == NULL) ||
			    (channel->rotation_count > 0 &&
			     channel->rotations == NULL) ||
			    (channel->scale_count > 0 &&
			     channel->scales == NULL)) {
				animation_set_destroy(set);
				return NULL;
			}
			for (key_index = 0; key_index < channel->position_count;
			     key_index++) {
				channel->positions[key_index].time =
					(float)source_channel
						->mPositionKeys[key_index]
						.mTime;
				channel->positions[key_index]
					.value = vec3_create(
					(float)source_channel
						->mPositionKeys[key_index]
						.mValue.x,
					(float)source_channel
						->mPositionKeys[key_index]
						.mValue.y,
					(float)source_channel
						->mPositionKeys[key_index]
						.mValue.z);
			}
			for (key_index = 0; key_index < channel->rotation_count;
			     key_index++) {
				channel->rotations[key_index].time =
					(float)source_channel
						->mRotationKeys[key_index]
						.mTime;
				channel->rotations[key_index]
					.value = (animation_quaternion_t){
					(float)source_channel
						->mRotationKeys[key_index]
						.mValue.x,
					(float)source_channel
						->mRotationKeys[key_index]
						.mValue.y,
					(float)source_channel
						->mRotationKeys[key_index]
						.mValue.z,
					(float)source_channel
						->mRotationKeys[key_index]
						.mValue.w};
			}
			for (key_index = 0; key_index < channel->scale_count;
			     key_index++) {
				channel->scales[key_index].time =
					(float)source_channel
						->mScalingKeys[key_index]
						.mTime;
				channel->scales[key_index].value = vec3_create(
					(float)source_channel
						->mScalingKeys[key_index]
						.mValue.x,
					(float)source_channel
						->mScalingKeys[key_index]
						.mValue.y,
					(float)source_channel
						->mScalingKeys[key_index]
						.mValue.z);
			}
		}
		set->clips[clip_index].channel_count = channel_index;
	}
	return set;
}

static size_t count_nodes(const struct aiNode *node) {
	size_t count = 1;
	size_t index;
	for (index = 0; index < node->mNumChildren; index++) {
		count += count_nodes(node->mChildren[index]);
	}
	return count;
}

static bool copy_nodes(const struct aiNode *node,
		       const int parent_index,
		       animation_set_t *set,
		       size_t *next_index) {
	size_t own_index;
	size_t index;

	own_index = (*next_index)++;
	set->nodes[own_index].name = duplicate_string(node->mName.data);
	if (set->nodes[own_index].name == NULL) { return false; }
	set->nodes[own_index].parent_index = parent_index;
	set->nodes[own_index].bone_index = -1;
	set->nodes[own_index].bind_transform =
		convert_matrix(&node->mTransformation);
	for (index = 0; index < node->mNumChildren; index++) {
		if (!copy_nodes(node->mChildren[index], (int)own_index, set,
				next_index)) {
			return false;
		}
	}
	return true;
}

static int find_node(const animation_set_t *set, const char *name) {
	size_t index;
	for (index = 0; index < set->node_count; index++) {
		if (strcmp(set->nodes[index].name, name) == 0) {
			return (int)index;
		}
	}
	return -1;
}

static mat4_t convert_matrix(const struct aiMatrix4x4 *matrix) {
	mat4_t result;
	result.elements[0] = (float)matrix->a1;
	result.elements[1] = (float)matrix->b1;
	result.elements[2] = (float)matrix->c1;
	result.elements[3] = (float)matrix->d1;
	result.elements[4] = (float)matrix->a2;
	result.elements[5] = (float)matrix->b2;
	result.elements[6] = (float)matrix->c2;
	result.elements[7] = (float)matrix->d2;
	result.elements[8] = (float)matrix->a3;
	result.elements[9] = (float)matrix->b3;
	result.elements[10] = (float)matrix->c3;
	result.elements[11] = (float)matrix->d3;
	result.elements[12] = (float)matrix->a4;
	result.elements[13] = (float)matrix->b4;
	result.elements[14] = (float)matrix->c4;
	result.elements[15] = (float)matrix->d4;
	return result;
}

static char *duplicate_string(const char *text) {
	size_t length = strlen(text);
	char *copy = malloc(length + 1);
	if (copy != NULL) { memcpy(copy, text, length + 1); }
	return copy;
}

static void add_bone_weight(mesh_vertex_t *vertex,
			    const int bone_index,
			    const float weight) {
	size_t index;
	size_t smallest = 0;

	for (index = 0; index < 4; index++) {
		if (vertex->bone_weights[index] == 0.0f) {
			vertex->bone_indices[index] = bone_index;
			vertex->bone_weights[index] = weight;
			break;
		}
		if (vertex->bone_weights[index] <
		    vertex->bone_weights[smallest]) {
			smallest = index;
		}
	}
	if (index == 4 && weight > vertex->bone_weights[smallest]) {
		vertex->bone_indices[smallest] = bone_index;
		vertex->bone_weights[smallest] = weight;
	}
}

static void normalize_bone_weights(mesh_vertex_t *vertex) {
	size_t index;
	float total = 0.0f;

	for (index = 0; index < 4; index++) {
		total += vertex->bone_weights[index];
	}
	if (total > 0.000001f) {
		for (index = 0; index < 4; index++) {
			vertex->bone_weights[index] /= total;
		}
	}
}
