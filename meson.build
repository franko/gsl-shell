project('gsl-shell', ['cpp', 'c'], version : '2.3.1', default_options : ['cpp_std=c++11', 'c_std=c99'])

lang_toolkit_proj = subproject('luajit-lang-toolkit')

lang_headers_dir = lang_toolkit_proj.get_variable('lang_headers_dir')
liblang = lang_toolkit_proj.get_variable('liblang')
liblang_dep = declare_dependency(include_directories: lang_headers_dir, link_with: liblang)

cpp_utils_include = include_directories('cpp-utils')
lua_gsl_include = include_directories('lua-gsl')
agg_plot_include = include_directories('agg-plot')
gsl_shell_include = include_directories('.')

threads_dep = dependency('threads')

gsl_shell_defines = []

if host_machine.system() == 'windows'
    gsl_shell_defines += '-DWIN32'
endif

subdir('agg-plot')
subdir('lua-gsl')
subdir('gdt')
subdir('fox-gui')
