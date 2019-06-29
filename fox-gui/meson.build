foxgui_sources = ['io_thread.cpp', 'window_surface.cpp', 'Notebook.cpp', 'LuaConsole.cpp', 'redirect.cpp', 'gsl_shell_interp.cpp', 'gsl_shell_thread.cpp', 'fox_gsl_shell.cpp', 'GslShellWindow.cpp', 'window_part.cpp', 'fx_plot_canvas.cpp', 'fx_plot_window.cpp', 'lua_plot_window.cpp', 'GslShellApp.cpp', 'gsl-shell-fox.cpp']

fox_dep = dependency('fox')
gsl_dep = dependency('gsl')

executable('gsl-shell-fox',
    foxgui_sources,
    link_with: [libaggplot, libluagsl, libgdt],
    cpp_args: gsl_shell_defines,
    include_directories: [cpp_utils_include, lua_gsl_include, agg_plot_include, gsl_shell_include],
    dependencies: [gsl_dep, liblang_dep, agg_dep, freetype_dep, fox_dep, threads_dep],
    install: true,
)
