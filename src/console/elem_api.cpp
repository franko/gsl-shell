#include <new>

#include <elem/elem.h>
#include <lauxlib.h>
#include <lua.h>

#define API_TYPE_PATH   "Path"
#define API_TYPE_CURVE  "Curve"

// FIXME
#define GRAPH_NREC 2

extern "C" {
static int f_path_new(lua_State *L);
static int f_path_gc(lua_State *L);
static int f_path_move_to(lua_State *L);
static int f_path_line_to(lua_State *L);
static int f_path_close(lua_State *L);

int luaopen_graph(lua_State *L);
}

static int check_path_userdata(lua_State *L, int arg, const char *tname) {
    void *path = luaL_testudata(L, arg, tname);
    if (!path) {
    }
}

int f_path_new(lua_State *L) {
    void *path_buf = lua_newuserdata(L, sizeof(elem::Path));
    new(path_buf) elem::Path;
    luaL_setmetatable(L, API_TYPE_PATH);
    return 1;
}

int f_path_gc(lua_State *L) {
    elem::Path *self = (elem::Path *) luaL_checkudata(L, 1, API_TYPE_PATH);
    self->~Path();
    return 0;
}

int f_path_move_to(lua_State *L) {
    elem::Path *self = (elem::Path *) luaL_checkudata(L, 1, API_TYPE_PATH);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->MoveTo(x, y);
    return 0;
}

int f_path_line_to(lua_State *L) {
    elem::Path *self = (elem::Path *) luaL_checkudata(L, 1, API_TYPE_PATH);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->LineTo(x, y);
    return 0;
}

int f_path_close(lua_State *L) {
    elem::Path *self = (elem::Path *) luaL_checkudata(L, 1, API_TYPE_PATH);
    self->ClosePolygon();
    return 0;
}

static const luaL_Reg graph_lib[] = {
    {"path",                f_path_new           },
    {"curve",               f_curve_new          },
    { nullptr, nullptr }
};

static const luaL_Reg path_lib[] = {
    { "__gc",               f_path_gc            },
    { "move_to",            f_path_move_to       },
    { "line_to",            f_path_line_to       },
    { "close",              f_path_close         },
    { nullptr, nullptr }
};

static const luaL_Reg curve_lib[] = {
    { "__gc",               f_curve_gc            },
    { "move_to",            f_curve_move_to       },
    { "line_to",            f_curve_line_to       },
    { "close",              f_curve_close         },
    { "curve3",             f_curve_curve3        },
    { "curve4",             f_curve_curve4        },
    { "arc_to",             f_curve_arc_to        },
    { nullptr, nullptr }
};

int luaopen_graph(lua_State *L) {
    luaL_newmetatable(L, API_TYPE_PATH);
    luaL_setfuncs(L, path_lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    luaL_newmetatable(L, API_TYPE_CURVE);
    luaL_setfuncs(L, curve_lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    lua_createtable(L, 0, GRAPH_NREC);
    luaL_setfuncs(L, graph_lib, 0);
    return 1;
}
