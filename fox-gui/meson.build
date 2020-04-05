foxgui_sources = ['fatal.cpp', 'io_thread.cpp', 'Notebook.cpp', 'LuaConsole.cpp', 'redirect.cpp', 'LuaInterpreter.cpp', 'gsl_shell_thread.cpp', 'fox_gsl_shell.cpp', 'GslShellWindow.cpp', 'GslShellApp.cpp', 'gsl-shell-fox.cpp']

fox_dep = dependency('fox')

executable('gsl-shell-fox',
    foxgui_sources,
    cpp_args: gsl_shell_defines,
    include_directories: gsl_shell_include,
    dependencies: [liblang_dep, fox_dep, libelemfox_dep, libelemlua_dep, threads_dep],
    install: true,
)
