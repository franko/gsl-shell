luagsl_sources = ['lua-gsl.c', 'fatal.c']

libluagsl = static_library('luagsl',
    luagsl_sources,
    dependencies: [luajit_dep, threads_dep],
    c_args : gsl_shell_defines,
    include_directories: [lua_gsl_include, gsl_shell_include]
)
