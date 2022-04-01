fox_gui_defines = []
if host_machine.system() == 'windows'
    # The FOX Library needs WIN32 to be defined on windows.
    fox_gui_defines += '-DWIN32'
endif

foxgui_sources = [
    'io_thread.cpp', 'window_surface.cpp', 'fx_console.cpp', 'redirect.cpp',
    'gsl_shell_interp.cpp', 'gsl_shell_thread.cpp', 'fox_gsl_shell.cpp',
    'gsl_shell_window.cpp', 'window_part.cpp', 'fx_plot_canvas.cpp', 'fx_plot_window.cpp',
    'lua_plot_window.cpp', 'gsl_shell_app.cpp', 'gsl-shell-fox.cpp',
]

executable('gsl-shell-gui',
    foxgui_sources,
    dependencies: [libagg_dep, threads_dep, freetype_dep, luajit_dep, fox_dep],
    include_directories: [gsl_shell_include, cpp_utils_include],
    cpp_args: gsl_shell_defines + fox_gui_defines,
    link_with: [libluagsl, libaggplot, libgdt],
)

