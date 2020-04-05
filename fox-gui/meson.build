foxgui_sources = ['io_thread.cpp', 'Notebook.cpp', 'LuaConsole.cpp', 'redirect.cpp', 'gsl_shell_interp.cpp', 'gsl_shell_thread.cpp', 'fox_gsl_shell.cpp', 'GslShellWindow.cpp', 'GslShellApp.cpp', 'gsl-shell-fox.cpp']

fox_dep = dependency('fox')

executable('gsl-shell-fox',
    foxgui_sources,
    link_with: libluagsl,
    cpp_args: gsl_shell_defines,
    include_directories: [cpp_utils_include, lua_gsl_include, gsl_shell_include],
    dependencies: [liblang_dep, fox_dep, libelemfox_dep, libelemlua_dep, threads_dep],
    install: true,
)
