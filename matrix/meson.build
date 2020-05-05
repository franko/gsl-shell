matrix_sources = ['cblas.lua', 'matrix.lua', 'matrix-display.lua', 'lapacke.lua', 'lapack.lua', 'matrix-permutation.lua']

matrix_bc_sources = []
foreach lua_source : matrix_sources
    matrix_bc_sources += custom_target(lua_source + '.c',
        input: lua_source,
        output: '@BASENAME@.c',
        command: [luajit, '-b', '@INPUT@', '@OUTPUT@']
    )
endforeach

bytecode_libs += static_library('matrix-bc', matrix_bc_sources)
