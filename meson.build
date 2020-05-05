project('gsl-shell', ['cpp', 'c'], version : '2.3.1', default_options : ['cpp_std=c++14', 'c_std=c99'])

lang_toolkit_proj = subproject('luajit-lang-toolkit', default_options: ['preload=true'])
liblang_dep = lang_toolkit_proj.get_variable('liblang_dep')

gsl_shell_include = include_directories('include')

threads_dep = dependency('threads')
luajit_dep = dependency('luajit')
libelem_dep = dependency('libelem')
libelemfox_dep = dependency('libelemfox')
libelemlua_dep = dependency('libelemlua')

gsl_shell_defines = []

if host_machine.system() == 'windows'
    gsl_shell_defines += '-DWIN32'
endif

luajit = find_program('luajit')
bytecode_libs = []

# matrix need to be done before core because the bytecode generated
# files are needed in core for the interpreter.
subdir('matrix')
subdir('graph')
subdir('core')
subdir('fox-gui')
subdir('tests')
