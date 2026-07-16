#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texture_coordinate;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_view_projection;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec3 fragment_tangent;
out vec3 fragment_bitangent;
out vec3 fragment_color;
out vec2 fragment_texture_coordinate;
out vec4 fragment_light_position;

vec3 create_fallback_tangent(vec3 surface_normal) {
    vec3 reference;

    if (abs(surface_normal.y) < 0.999) {
        reference = vec3(0.0, 1.0, 0.0);
    } else {
        reference = vec3(1.0, 0.0, 0.0);
    }

    return normalize(cross(reference, surface_normal));
}

void main(void) {
    mat3 normal_matrix;
    vec4 world_position;
    vec4 view_position;
    vec3 transformed_normal;
    vec3 transformed_tangent;
    vec3 transformed_bitangent;
    float handedness;

    world_position = model * vec4(position, 1.0);
    view_position = view * world_position;

    normal_matrix =
    transpose(inverse(mat3(view * model)));

    transformed_normal =
    normalize(normal_matrix * normal);

    transformed_tangent = normal_matrix * tangent;

    if (dot(transformed_tangent, transformed_tangent) < 0.00000001) {
        transformed_tangent =
        create_fallback_tangent(transformed_normal);
    } else {
        transformed_tangent -=
        transformed_normal *
        dot(transformed_normal, transformed_tangent);

        if (dot(transformed_tangent, transformed_tangent) <
            0.00000001) {
            transformed_tangent =
            create_fallback_tangent(transformed_normal);
        } else {
            transformed_tangent =
            normalize(transformed_tangent);
        }
    }

    transformed_bitangent = normal_matrix * bitangent;
    handedness = 1.0;

    if (dot(transformed_bitangent, transformed_bitangent) >=
        0.00000001 &&
        dot(cross(transformed_normal, transformed_tangent),
                transformed_bitangent) < 0.0) {
        handedness = -1.0;
    }

    transformed_bitangent =
    normalize(cross(transformed_normal, transformed_tangent)) *
    handedness;

    fragment_position = view_position.xyz;
    fragment_normal = transformed_normal;
    fragment_tangent = transformed_tangent;
    fragment_bitangent = transformed_bitangent;
    fragment_color = color;
    fragment_texture_coordinate = texture_coordinate;
    fragment_light_position =
    light_view_projection * world_position;

    gl_Position = projection * view_position;
}