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

luajit_dep = dependency('luajit', fallback: ['luajit', 'lua_dep'],
    default_options: ['default_library=static', 'app=false', 'shortfnsyn=true']
)
libagg_dep = dependency('libagg', fallback: ['libagg', 'libagg_dep'])

libgsl_proj = subproject('gsl', default_options: ['default_library=static'])
libgsl_dep = libgsl_proj.get_variable('libgsl_dep').as_link_whole()

subdir('lua-gsl')
subdir('agg-plot')
subdir('gdt')
subdir('fox-gui')

