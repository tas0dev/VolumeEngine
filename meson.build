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

sdl3 = dependency('sdl3')
opengl = dependency('gl')

sources = files(
        'src/main.c',
        'src/engine.c',
        'src/platform/platform.c',
        'src/core/log.c',
        'src/renderer/renderer.c',
)

executable(
        'volume',
        sources,
        dependencies : [
                sdl3,
                opengl,
        ],
        include_directories : include_directories('src'),
        install : false,
)