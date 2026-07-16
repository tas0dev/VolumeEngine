#version 330 core

in vec3 fragment_position;
in vec3 fragment_normal;
in vec3 fragment_tangent;
in vec3 fragment_bitangent;
in vec3 fragment_color;
in vec2 fragment_texture_coordinate;
in vec4 fragment_light_position;

uniform mat4 view;
uniform sampler2D shadow_map;
uniform sampler2D albedo_texture;
uniform sampler2D normal_texture;
uniform int has_albedo_texture;
uniform int has_normal_texture;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform vec3 material_color;
uniform float ambient_strength;
uniform float specular_strength;
uniform float shininess;
uniform float bloom_threshold;
uniform float bloom_knee;

layout(location = 0) out vec4 output_color;
layout(location = 1) out vec4 output_brightness;

vec3 srgb_to_linear(vec3 color) {
    return pow(
            max(color, vec3(0.0)),
            vec3(2.2)
    );
}

vec3 calculate_surface_normal(void) {
    vec3 surface_normal;
    vec3 surface_tangent;
    vec3 surface_bitangent;
    vec3 texture_normal;
    mat3 tangent_basis;

    surface_normal = normalize(fragment_normal);

    if (has_normal_texture == 0) {
        return surface_normal;
    }

    surface_tangent =
    fragment_tangent -
    surface_normal *
    dot(surface_normal, fragment_tangent);

    if (dot(surface_tangent, surface_tangent) < 0.00000001) {
        return surface_normal;
    }

    surface_tangent = normalize(surface_tangent);
    surface_bitangent =
    normalize(cross(surface_normal, surface_tangent));

    if (dot(surface_bitangent, fragment_bitangent) < 0.0) {
        surface_bitangent = -surface_bitangent;
    }

    texture_normal =
    texture(
            normal_texture,
            fragment_texture_coordinate
    ).rgb *
    2.0 -
    1.0;

    tangent_basis = mat3(
            surface_tangent,
            surface_bitangent,
            surface_normal
    );

    return normalize(tangent_basis * texture_normal);
}

float calculate_shadow(
        vec4 light_position,
        vec3 normal,
        vec3 light
) {
    vec3 projected;
    vec2 texel_size;
    float current_depth;
    float normal_dot_light;
    float bias;
    float shadow;
    float sample_count;

    if (light_position.w <= 0.0) {
        return 0.0;
    }

    projected =
    light_position.xyz / light_position.w;
    projected = projected * 0.5 + 0.5;

    if (projected.x < 0.0 ||
        projected.x > 1.0 ||
        projected.y < 0.0 ||
        projected.y > 1.0 ||
        projected.z < 0.0 ||
        projected.z > 1.0) {
        return 0.0;
    }

    current_depth = projected.z;
    normal_dot_light =
    max(dot(normal, light), 0.0);

    bias = mix(
            0.004,
            0.0004,
            normal_dot_light
    );

    texel_size =
    1.0 / vec2(textureSize(shadow_map, 0));
    shadow = 0.0;
    sample_count = 0.0;

    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            vec2 offset;
            float closest_depth;

            offset = vec2(
                    float(x),
                    float(y)
            ) * texel_size;

            closest_depth = texture(
                    shadow_map,
                    projected.xy + offset
            ).r;

            if (current_depth - bias >
                closest_depth) {
                shadow += 1.0;
            }

            sample_count += 1.0;
        }
    }

    return shadow / sample_count;
}

vec3 extract_bloom(
        vec3 color,
        float threshold,
        float knee
) {
    float brightness;
    float soft;
    float contribution;

    brightness = dot(
            color,
            vec3(0.2126, 0.7152, 0.0722)
    );

    soft = brightness - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft =
    soft * soft /
    max(4.0 * knee, 0.00001);

    contribution = max(
            brightness - threshold,
            soft
    );

    contribution /=
    max(brightness, 0.00001);

    return color * contribution;
}

void main(void) {
    vec3 geometric_normal;
    vec3 normal;
    vec3 light;
    vec3 view_direction;
    vec3 halfway_direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 albedo_color;
    vec3 base_color;
    vec3 color;
    vec3 bloom_color;
    float diffuse_factor;
    float specular_factor;
    float shadow;

    geometric_normal = normalize(fragment_normal);
    normal = calculate_surface_normal();

    light =
    normalize(mat3(view) * -light_direction);

    view_direction =
    normalize(-fragment_position);

    halfway_direction =
    normalize(light + view_direction);

    diffuse_factor =
    max(dot(normal, light), 0.0);

    specular_factor = 0.0;

    if (diffuse_factor > 0.0) {
        specular_factor = pow(
                max(
                        dot(
                                normal,
                                halfway_direction
                        ),
                        0.0
                ),
                shininess
        );
    }

    shadow = calculate_shadow(
            fragment_light_position,
            geometric_normal,
            light
    );

    ambient =
    ambient_strength * light_color;

    diffuse =
    (1.0 - shadow) *
    diffuse_factor *
    light_color;

    specular =
    (1.0 - shadow) *
    specular_strength *
    specular_factor *
    light_color;

    albedo_color =
    has_albedo_texture != 0
    ? texture(
            albedo_texture,
            fragment_texture_coordinate
    ).rgb
    : vec3(1.0);

    base_color = srgb_to_linear(
            fragment_color *
            material_color *
            albedo_color
    );

    color =
    base_color * (ambient + diffuse) +
    specular;

    bloom_color = extract_bloom(
            color,
            bloom_threshold,
            bloom_knee
    );

    output_color = vec4(color, 1.0);
    output_brightness =
    vec4(bloom_color, 1.0);
}