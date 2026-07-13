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
        '../src/asset/material_loader.c',
        '../src/map/keyvalues.c',
        '../src/renderer/material.c',
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
        '../src/asset/mesh_loader.c',
        '../src/asset/material_loader.c',
        '../src/core/log.c',
        '../src/entity/entity.c',
        '../src/entity/properties.c',
        '../src/entity/world.c',
        '../src/map/keyvalues.c',
        '../src/map/map.c',
        '../src/map/spawn.c',
        '../src/math/mat4.c',
        '../src/math/vec3.c',
        '../src/renderer/material.c',
        '../src/renderer/mesh.c',
        '../src/scene/transform.c',
    ],
    include_directories : include_directories('../src'),
    dependencies : [
        math,
        assimp,
        epoxy,
        opengl,
    ],
    install : false,
)

material_fixture = (
    meson.current_source_dir() /
    'fixtures/basic.volmat'
)

material_loader_test = executable(
    'material-loader-test',
    [
        'common.c',
        'material_loader_test.c',
        '../src/asset/material_loader.c',
        '../src/map/keyvalues.c',
        '../src/renderer/material.c',
    ],
    c_args : [
        '-DTEST_MATERIAL_FILE="@0@"'.format(
            material_fixture
        ),
    ],
    include_directories : include_directories('../src'),
    install : false,
)

test(
    'material-loader',
    material_loader_test,
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