project(
        'Volume',
        'c',
        version : '0.1.0',
        meson_version : '>=1.3.0',
        default_options : [
                'c_std=c99',
                'warning_level=3',
                'werror=true',
        ],
)

cc = meson.get_compiler('c')

sdl3 = dependency('sdl3')
opengl = dependency('gl')
epoxy = dependency('epoxy')
assimp = dependency('assimp')
math = cc.find_library('m', required : true)

sources = files(
        'src/main.c',
        'src/volume.c',
        'src/platform/platform.c',
        'src/core/log.c',
        'src/entity/entity.c',
        'src/entity/builtin.c',
        'src/entity/prop_static.c',
        'src/entity/world.c',
        'src/entity/properties.c',
        'src/renderer/renderer.c',
        'src/renderer/mesh.c',
        'src/renderer/shader.c',
        'src/renderer/material.c',
        'src/renderer/shadow_map.c',
        'src/renderer/hdr_buffer.c',
        'src/renderer/bloom_buffer.c',
        'src/math/mat4.c',
        'src/math/vec3.c',
        'src/scene/camera.c',
        'src/scene/transform.c',
        'src/input/input.c',
        'src/map/keyvalues.c',
        'src/map/map.c',
        'src/map/spawn.c',
        'src/asset/manager.c',
)

volume_c_args = [
    '-DVOLUME_ASSET_DIR="@0@"'.format(
        meson.project_source_root() / 'assets'
    ),
]

executable(
        'volume',
        sources,
        dependencies : [
                sdl3,
                opengl,
                epoxy,
                assimp,
                math,
        ],
        c_args : volume_c_args,
        include_directories : include_directories('src'),
        install : false,
)

subdir('tests')