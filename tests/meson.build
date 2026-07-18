keyvalues_test = executable(
    'keyvalues-test',
    [
        'common.c',
        'keyvalues_test.c',
    ],
    dependencies : volume_dependency,
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
    ],
    dependencies : volume_dependency,
    install : false,
)

asset_manager_test = executable(
    'asset-manager-test',
    [
        'common.c',
        'asset_manager_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

map_spawn_test = executable(
    'map-spawn-test',
    [
        'common.c',
        'map_spawn_test.c',
    ],
    dependencies : volume_dependency,
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
    ],
    dependencies : volume_dependency,
    c_args : [
        '-DTEST_MATERIAL_FILE="@0@"'.format(
            material_fixture
        ),
    ],
    install : false,
)

aabb_test = executable(
    'aabb-test',
    [
        'common.c',
        'aabb_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

collision_world_test = executable(
    'collision-world-test',
    [
        'common.c',
        'collision_world_test.c',
    ],
    dependencies : volume_dependency,
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

test(
    'aabb',
    aabb_test,
)

test(
    'collision-world',
    collision_world_test,
)