# Readline not supported with meson build.

executable('gsl-shell', 'gsl-shell-jit.c',
    dependencies: [libgsl_dep, threads_dep, luajit_dep],
    include_directories: gsl_shell_include,
    cpp_args: gsl_shell_defines,
    link_with: [libluagsl, libgdt],
    link_args: gsl_shell_link_args,
    export_dynamic: true,
    win_subsystem: 'console',
    install: true,
    install_dir: gsl_shell_bindir,
)

