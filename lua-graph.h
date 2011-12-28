#ifndef LUA_GRAPH_H
#define LUA_GRAPH_H

#ifndef MLUA_GRAPHLIBNAME
#define MLUA_GRAPHLIBNAME "graph"
#endif

extern void graph_close_windows (lua_State *L);
extern void register_graph (lua_State *L);

#endif
