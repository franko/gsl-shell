# Readline not supported with meson build.

executable('gsl-shell', 'gsl-shell-jit.c',
    dependencies: [libgsl_dep, libagg_dep, threads_dep, freetype_dep, luajit_dep],
    include_directories: gsl_shell_include,
    cpp_args: gsl_shell_defines,
    link_with: [libluagsl, libaggplot, libgdt],
    export_dynamic: true,
    win_subsystem: 'console'
)

