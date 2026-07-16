perl = find_program('perl')

sandbox_asset_source = meson.project_source_root() / 'assets'
sandbox_asset_destination = meson.current_build_dir() / 'assets'

meson.add_postconf_script(
    perl,
    meson.project_source_root() / 'scripts/stage-assets.pl',
    sandbox_asset_source,
    sandbox_asset_destination,
)

executable(
    'volume example',
    'main.c',
    dependencies : volume_dependency,
    install : false,
)