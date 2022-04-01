project('gsl-shell', ['cpp', 'c'], version : '2.3.1', default_options : ['cpp_std=c++11', 'c_std=gnu11'])

gsl_shell_defines = [
    # '-DDISABLE_GAMMA_CORR',
    # '-DDISABLE_SUBPIXEL_AA',
    # '-DUSE_READLINE',
]

if host_machine.system() == 'darwin'
    gsl_shell_defines += '-DDARWIN_MACOSX'
endif

gsl_shell_link_args = []
cc = meson.get_compiler('c')
if cc.get_id() == 'gcc' and get_option('buildtype') == 'release'
    gsl_shell_link_args += ['-static-libgcc', '-static-libstdc++']
endif

gsl_shell_include = include_directories('.', 'agg-plot', 'lua-gsl')
cpp_utils_include = include_directories('cpp-utils')

threads_dep  = dependency('threads')
freetype_dep = dependency('freetype2')
fox_dep      = dependency('fox')
libagg_dep = dependency('libagg', fallback: ['libagg', 'libagg_dep'])

luajit_proj = subproject('luajit', default_options: ['default_library=static', 'app=false', 'portable=true', 'shortfnsyn=true'])
luajit_dep = luajit_proj.get_variable('lua_dep')

libgsl_proj = subproject('gsl', default_options: ['default_library=static'])
libgsl_dep = libgsl_proj.get_variable('libgsl_dep').as_link_whole()

subdir('lua-gsl')
subdir('agg-plot')
subdir('gdt')
subdir('fox-gui')

executable('gsl-shell', 'gsl-shell-jit.c',
    dependencies: [libgsl_dep, libagg_dep, threads_dep, freetype_dep, luajit_dep],
    include_directories: gsl_shell_include,
    cpp_args: gsl_shell_defines,
    link_with: [libluagsl, libaggplot, libgdt],
    export_dynamic: true,
    win_subsystem: 'console'
)

