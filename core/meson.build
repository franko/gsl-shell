core_sources = ['LuaInterpreter.cpp', 'welcome.cpp']

libcore_interp = static_library('coreinterp',
    core_sources,
    include_directories: gsl_shell_include,
    cpp_args: gsl_shell_defines,
    dependencies: [liblang_dep, luajit_dep],
    install: false,
)

# We set below the export_dynamic flag so that LuaJIT can
# see the bytecode symbols and load them on 'require'.
# We use also link_whole so that the linker does not discard
# the bytecode library.
executable('gsl-shell',
    ['gsl-shell.cpp'],
    cpp_args: gsl_shell_defines,
    include_directories: gsl_shell_include,
    link_with: libcore_interp,
    link_whole: matrix_bc_lib,
    export_dynamic: true,
    dependencies: [liblang_dep, luajit_dep, libelem_dep, libelemlua_dep, threads_dep],
    install: true,
)
