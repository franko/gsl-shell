project('gsl-shell', ['cpp', 'c'], version : '2.3.5', default_options : ['cpp_std=c++11', 'c_std=gnu11'])

gsl_shell_defines = [
    # '-DDISABLE_GAMMA_CORR',
    # '-DDISABLE_SUBPIXEL_AA',
    # '-DUSE_READLINE',
]

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('copyright', 'Copyright (C) 2009-2022 Francesco Abbate')

if host_machine.system() == 'darwin'
    gsl_shell_defines += '-DDARWIN_MACOSX'
endif

gsl_shell_link_args = []
cc = meson.get_compiler('c')
if cc.get_id() == 'gcc' and get_option('buildtype') == 'release'
    gsl_shell_link_args += ['-static-libgcc', '-static-libstdc++']
endif

gsl_shell_include = include_directories('include', 'src/gdt', 'src/agg-plot', 'src/lua-gsl')
cpp_utils_include = include_directories('src/cpp-utils')

threads_dep  = dependency('threads')
freetype_dep = dependency('freetype2')
fox_dep      = dependency('fox')
libagg_dep = dependency('libagg', fallback: ['libagg', 'libagg_dep'])

luajit_proj = subproject('luajit', default_options: ['default_library=static', 'app=false', 'portable=true', 'shortfnsyn=true'])
luajit_dep = luajit_proj.get_variable('lua_dep')

libgsl_options = ['default_library=static', 'blas=' + get_option('blas')]
foreach module_name : ['siman', 'wavelet', 'sparse', 'ode', 'monte', 'integ', 'min', 'fit']
    libgsl_options += module_name + '=false'
endforeach

libgsl_proj = subproject('gsl', default_options: libgsl_options)
libgsl_dep = libgsl_proj.get_variable('libgsl_dep').as_link_whole()

gsl_shell_bindir = 'bin'
gsl_shell_datadir = 'share/gsl-shell'
install_subdir('data', strip_directory : true, install_dir : gsl_shell_datadir)

subdir('include')
subdir('src')

