luagsl_sources = [
    'lua-properties.c',
    'gs-types.c',
    'lua-utils.c',
    'lua-gsl.c',
    'str.c',
    'fatal.c'
]

libluagsl = static_library('luagsl',
    luagsl_sources,
    dependencies: [gsl_dep, luajit_dep],
    include_directories: gsl_shell_include,
    c_args: gsl_shell_defines,
)

