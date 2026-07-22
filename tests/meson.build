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

prop_static_collision_test = executable(
    'prop-static-collision-test',
    [
        'common.c',
        'prop_static_collision_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

entity_io_test = executable(
    'entity-io-test',
    [
        'common.c',
        'entity_io_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

func_door_test = executable(
    'func-door-test',
    [
        'common.c',
        'func_door_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

player_test = executable(
    'player-test',
    [
        'common.c',
        'player_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

trigger_test = executable(
    'trigger-test',
    [
        'common.c',
        'trigger_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

func_button_test = executable(
    'func-button-test',
    [
        'common.c',
        'func_button_test.c',
    ],
    dependencies : volume_dependency,
    install : false,
)

info_player_start_test = executable(
    'info-player-start-test',
    [
        'common.c',
        'info_player_start_test.c',
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

test(
    'prop-static-collision-test',
    prop_static_collision_test
)

test(
    'entity-io',
    entity_io_test
)

test(
    'func-door',
    func_door_test
)

test(
    'player',
    player_test
)

test(
    'trigger',
    trigger_test
)

test(
    'func-button',
    func_button_test
)

test(
    'info-player-start',
    info_player_start_test
)
