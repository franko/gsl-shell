project('gsl-shell', ['cpp', 'c'], version : '2.3.1', default_options : ['cpp_std=c++03', 'c_std=c99'])

cpp_utils_include = include_directories('cpp-utils')
lua_gsl_include = include_directories('lua-gsl')
gsl_shell_include = include_directories('.')

threads_dep = dependency('threads')

gsl_shell_defines = []

subdir('agg-plot')
subdir('gdt')
