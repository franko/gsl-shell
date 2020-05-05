foxgui_sources = ['fatal.cpp', 'io_thread.cpp', 'Notebook.cpp', 'LuaConsole.cpp', 'redirect.cpp', 'InterpreterThread.cpp', 'FoxInterpreterThread.cpp', 'GslShellWindow.cpp', 'GslShellApp.cpp', 'gsl-shell-fox.cpp']

fox_dep = dependency('fox')

executable('gsl-shell-fox',
    foxgui_sources,
    cpp_args: gsl_shell_defines,
    include_directories: gsl_shell_include,
    link_with: libcore_interp,
    link_whole: bytecode_libs,
    export_dynamic: true,
    dependencies: [liblang_dep, fox_dep, libelemfox_dep, libelemlua_dep, threads_dep],
    install: true,
)
