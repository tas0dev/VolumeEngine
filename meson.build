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
math = cc.find_library('m', required : true)

sources = files(
        'src/main.c',
        'src/engine.c',
        'src/platform/platform.c',
        'src/core/log.c',
        'src/renderer/renderer.c',
        'src/renderer/mesh.c',
        'src/renderer/shader.c',
        'src/renderer/material.c',
        'src/math/mat4.c',
        'src/math/vec3.c',
        'src/scene/camera.c',
)

executable(
        'volume',
        sources,
        dependencies : [
                sdl3,
                opengl,
                epoxy,
                math,
        ],
        include_directories : include_directories('src'),
        install : false,
)