keyvalues_test = executable(
    'keyvalues-test',
    [
        'common.c',
        'keyvalues_test.c',
        '../src/map/keyvalues.c',
    ],
    include_directories : include_directories('../src'),
    c_args : [
        '-DTEST_KEYVALUES_FILE="@0@"'.format(
            meson.current_source_dir() / 'fixtures/basic.volmap'
        ),
    ],
    install : false,
)

map_test = executable(
    'map-test',
    [
        'common.c',
        'map_test.c',
        '../src/map/map.c',
        '../src/map/keyvalues.c',
    ],
    include_directories : include_directories('../src'),
    install : false,
)

asset_manager_test = executable(
    'asset-manager-test',
    [
        'common.c',
        'asset_manager_test.c',
        '../src/asset/manager.c',
        '../src/asset/mesh_loader.c',
        '../src/renderer/mesh.c',
        '../src/core/log.c',
    ],
    dependencies : [
        assimp,
        epoxy,
        opengl,
    ],
    include_directories : include_directories('../src'),
    install : false,
)

map_spawn_test = executable(
    'map-spawn-test',
    [
        'common.c',
        'map_spawn_test.c',
        '../src/asset/manager.c',
        '../src/entity/entity.c',
        '../src/entity/properties.c',
        '../src/entity/world.c',
        '../src/map/keyvalues.c',
        '../src/map/map.c',
        '../src/map/spawn.c',
        '../src/math/mat4.c',
        '../src/math/vec3.c',
        '../src/scene/transform.c',
        '../src/asset/mesh_loader.c',
        '../src/renderer/mesh.c',
        '../src/core/log.c',
    ],
    dependencies : [
        math,
        assimp,
        epoxy,
        opengl,
    ],
    include_directories : include_directories('../src'),
    dependencies : math,
    install : false,
)

test(
    'map-spawn',
    map_spawn_test,
)
test(
    'asset-manager',
    asset_manager_test,
)
test(
    'map',
    map_test,
)
test(
    'keyvalues',
    keyvalues_test,
)