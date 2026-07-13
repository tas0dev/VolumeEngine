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

test(
    'keyvalues',
    keyvalues_test,
)