graph_sources = ['graph-iter.lua', 'graph.lua']

graph_bc_sources = []
foreach lua_source : graph_sources
    graph_bc_sources += custom_target(lua_source + '.c',
        input: lua_source,
        output: '@BASENAME@.c',
        command: [luajit, '-bg', '@INPUT@', '@OUTPUT@']
    )
endforeach

bytecode_libs += static_library('graph-bc', graph_bc_sources)
