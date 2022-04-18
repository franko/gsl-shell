if host_machine.system() == 'darwin'
    add_languages('objc')
endif

lite_includes = []
lite_cargs = []
# On macos we need to use the SDL renderer to support retina displays
if get_option('lite-renderer') or host_machine.system() == 'darwin'
    lite_cargs += '-DLITE_USE_SDL_RENDERER'
endif

lite_link_args = []
if cc.get_id() == 'gcc' and get_option('buildtype') == 'release'
    lite_link_args += ['-static-libgcc', '-static-libstdc++']
endif

if host_machine.system() == 'darwin'
    lite_link_args += ['-framework', 'CoreServices', '-framework', 'Foundation']
endif

lite_deps = [luajit_dep, sdl_dep, reproc_dep, pcre2_dep, libm, libdl, threads_dep]

if host_machine.system() == 'windows'
    # Note that we need to explicitly add the windows socket DLL because
    # the pkg-config file from reproc does not include it.
    lite_deps += meson.get_compiler('cpp').find_library('ws2_32', required: true)
endif

subdir('lib/font_renderer')
subdir('lib/dmon')

lite_sources = [
    'api/compat.c',
    'api/api.c',
    'api/cp_replace.c',
    'api/renderer.c',
    'api/renderer_font.c',
    'api/regex.c',
    'api/system.c',
    'api/process.c',
    'dirmonitor.c',
    'lbitlib.c',
    'renderer.c',
    'renwindow.c',
    'fontdesc.c',
    'rencache.c',
    'main.c',
]

lite_rc = []
if host_machine.system() == 'windows'
    windows = import('windows')
    lite_rc += windows.compile_resources('../resources/icons/icon.rc')
elif host_machine.system() == 'darwin'
    lite_sources += 'bundle_open.m'
endif

lite_includes += include_directories('.')

executable('gsl-shell-lite',
    lite_sources + lite_rc,
    include_directories: [lite_includes, resources_include],
    dependencies: lite_deps,
    c_args: lite_cargs,
    objc_args: lite_cargs,
    link_with: libfontrenderer,
    link_args: lite_link_args,
    install: true,
    gui_app: true,
)
