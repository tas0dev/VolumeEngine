project(
        'VolumeEngine',
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

volume_dependencies = [
        sdl3,
        opengl,
        epoxy,
        assimp,
        math,
]

volume_sources = files(
        'src/volume.c',
        'src/platform/platform.c',
        'src/core/log.c',
        'src/core/path.c',
        'src/entity/entity.c',
        'src/entity/mover.c',
        'src/entity/func_door.c',
        'src/entity/func_button.c',
        'src/entity/func_ladder.c',
        'src/entity/info_player_start.c',
        'src/entity/io.c',
        'src/entity/logic_auto.c',
        'src/entity/logic_relay.c',
        'src/entity/player.c',
        'src/entity/trigger.c',
        'src/entity/builtin.c',
        'src/entity/prop_static.c',
        'src/entity/prop_dynamic.c',
        'src/entity/light_environment.c',
        'src/entity/world.c',
        'src/entity/properties.c',
        'src/renderer/renderer.c',
        'src/renderer/mesh.c',
        'src/renderer/shader.c',
        'src/renderer/material.c',
        'src/renderer/shadow_map.c',
        'src/renderer/hdr_buffer.c',
        'src/renderer/bloom_buffer.c',
        'src/renderer/texture.c',
        'src/renderer/ui_renderer.c',
        'src/debug/debug_ui.c',
        'src/debug/hud.c',
        'src/math/mat4.c',
        'src/math/vec3.c',
        'src/scene/camera.c',
        'src/scene/transform.c',
        'src/input/input.c',
        'src/map/keyvalues.c',
        'src/map/map.c',
        'src/map/spawn.c',
        'src/asset/manager.c',
        'src/asset/mesh_loader.c',
        'src/asset/material_loader.c',
        'src/collision/aabb.c',
        'src/collision/triangle.c',
        'src/collision/box_collider.c',
        'src/collision/triangle_mesh_collider.c',
        'src/collision/collision_world.c',
        'src/collision/collider.c',
        'src/physics/character_controller.c',
)

volume_include = include_directories(
        'src',
        'libs',
)

volume_library = static_library(
        'volumeengine',
        volume_sources,
        dependencies : volume_dependencies,
        include_directories : volume_include,
        install : false,
)

volume_dependency = declare_dependency(
        link_with : volume_library,
        dependencies : volume_dependencies,
        include_directories : volume_include,
)

subdir('examples')
subdir('tests')
