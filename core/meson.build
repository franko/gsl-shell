core_sources = ['LuaInterpreter.cpp', 'welcome.cpp']

libcore_interp = static_library('coreinterp',
    core_sources,
    include_directories: gsl_shell_include,
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
