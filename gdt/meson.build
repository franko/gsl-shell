gdt_sources = ['char_buffer.c', 'gdt_index.c', 'gdt_table.c']

libgdt = static_library('gdt',
    gdt_sources,
    dependencies: [],
    include_directories: gsl_shell_include,
    c_args: gsl_shell_defines,
)

