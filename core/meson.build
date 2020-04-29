# FIXME: the place of matrix_bc_loader should not be there.
core_sources = ['LuaInterpreter.cpp', 'welcome.cpp', 'matrix_bc_loader.c']

libcore_interp = static_library('coreinterp',
    core_sources + matrix_bc_headers,
    include_directories: [gsl_shell_include, bc_headers_dir],
    cpp_args: gsl_shell_defines,
    dependencies: [liblang_dep, luajit_dep],
    install: false,
)

executable('gsl-shell',
    ['gsl-shell.cpp'],
    cpp_args: gsl_shell_defines,
    include_directories: gsl_shell_include,
    link_with: libcore_interp,
    dependencies: [liblang_dep, luajit_dep, threads_dep],
    install: true,
)
