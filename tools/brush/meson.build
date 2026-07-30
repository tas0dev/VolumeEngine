brush_perl = find_program('perl')

meson.add_postconf_script(
    brush_perl,
    meson.project_source_root() / 'scripts/stage-assets.pl',
    meson.project_source_root() / 'assets',
    meson.current_build_dir() / 'assets',
)

executable(
    'brush',
    'main.c',
    dependencies : volume_dependency,
    install : false,
)
