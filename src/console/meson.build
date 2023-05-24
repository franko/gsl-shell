# Readline not supported with meson build.

gsl_shell_console_deps = [libgsl_dep, threads_dep, luajit_dep]
gsl_shell_console_libs = [libluagsl, libgdt]
if build_graphics
    gsl_shell_console_deps += [libagg_dep, freetype_dep]
    gsl_shell_console_libs += libaggplot
endif

executable('gsl-shell', 'gsl-shell-jit.c',
    dependencies: gsl_shell_console_deps,
    include_directories: gsl_shell_include,
    c_args: gsl_shell_defines,
    link_with: gsl_shell_console_libs,
    link_args: gsl_shell_link_args,
    export_dynamic: true,
    win_subsystem: 'console',
    install: true,
    install_dir: gsl_shell_bindir,
)

