matrix_sources = ['cblas.lua', 'matrix-core.lua', 'matrix-display.lua']

luajit = find_program('luajit')

matrix_bc_headers = []
foreach lua_source : matrix_sources
    matrix_bc_headers += custom_target(lua_source + '.h',
        input: lua_source,
        output: '@BASENAME@.h',
        command: [luajit, '-b', '@INPUT@', '@OUTPUT@']
    )
endforeach
