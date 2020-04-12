core_sources = ['LuaInterpreter.cpp']

libcore_interp = static_library('coreinterp',
    core_sources,
    include_directories: gsl_shell_include,
    cpp_args: gsl_shell_defines,
    dependencies: [liblang_dep, luajit_dep],
    install: false,
)
