#include <new>

#include <elem/elem.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

#define API_TYPE_PATH   "Path"
#define API_TYPE_CURVE  "Curve"

extern "C" {
static int f_path_new(lua_State *L);
static int f_path_gc(lua_State *L);
static int f_path_move_to(lua_State *L);
static int f_path_line_to(lua_State *L);
static int f_path_close(lua_State *L);

static int f_curve_new(lua_State *L);
static int f_curve_gc(lua_State *L);
static int f_curve_move_to(lua_State *L);
static int f_curve_line_to(lua_State *L);
static int f_curve_close(lua_State *L);
static int f_curve_curve3(lua_State *L);
static int f_curve_curve4(lua_State *L);
static int f_curve_arc_to(lua_State *L);

extern int luaopen_graph(lua_State *L);
}

static elem::Path *check_path_userdata(lua_State *L, int arg) {
    void *path = luaL_testudata(L, arg, API_TYPE_PATH);
    // path = path ? path : luaL_testudata(L, arg, API_TYPE_CURVE);
    if (!path) {
        lua_pushstring(L, "expected argument of type Path");
        lua_error(L);
    }
    return (elem::Path *) path;
}

int f_path_new(lua_State *L) {
    void *path_buf = lua_newuserdata(L, sizeof(elem::Path));
    new(path_buf) elem::Path;
    luaL_setmetatable(L, API_TYPE_PATH);
    return 1;
}

int f_path_gc(lua_State *L) {
    auto self = check_path_userdata(L, 1);
    self->~Path();
    return 0;
}

int f_path_move_to(lua_State *L) {
    auto self = check_path_userdata(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->MoveTo(x, y);
    return 0;
}

int f_path_line_to(lua_State *L) {
    auto self = check_path_userdata(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->LineTo(x, y);
    return 0;
}

int f_path_close(lua_State *L) {
    auto self = check_path_userdata(L, 1);
    self->ClosePolygon();
    return 0;
}

static elem::CurvePath *check_curve_userdata(lua_State *L, int arg) {
    void *udata = luaL_testudata(L, arg, API_TYPE_CURVE);
    if (!udata) {
        lua_pushstring(L, "expected argument of type Curve");
        lua_error(L);
    }
    return (elem::CurvePath *) udata;
}

int f_curve_new(lua_State *L) {
    void *buf = lua_newuserdata(L, sizeof(elem::CurvePath));
    new(buf) elem::CurvePath;
    luaL_setmetatable(L, API_TYPE_CURVE);
    return 1;
}

int f_curve_gc(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    self->~CurvePath();
    return 0;
}

int f_curve_move_to(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->MoveTo(x, y);
    return 0;
}

int f_curve_line_to(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    self->LineTo(x, y);
    return 0;
}

int f_curve_close(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    self->ClosePolygon();
    return 0;
}

int f_curve_curve3(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    double x_ctrl = luaL_checknumber(L, 2);
    double y_ctrl = luaL_checknumber(L, 3);
    double x = luaL_checknumber(L, 3);
    double y = luaL_checknumber(L, 4);
    self->Curve3(x_ctrl, y_ctrl, x, y);
    return 0;
}

int f_curve_curve4(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    double x_ctrl1 = luaL_checknumber(L, 2);
    double y_ctrl1 = luaL_checknumber(L, 3);
    double x_ctrl2 = luaL_checknumber(L, 4);
    double y_ctrl2 = luaL_checknumber(L, 5);
    double x = luaL_checknumber(L, 6);
    double y = luaL_checknumber(L, 7);
    self->Curve4(x_ctrl1, y_ctrl1, x_ctrl2, y_ctrl2, x, y);
    return 0;
}


int f_curve_arc_to(lua_State *L) {
    auto self = check_curve_userdata(L, 1);
    double rx = luaL_checknumber(L, 2);
    double ry = luaL_checknumber(L, 3);
    double angle = luaL_checknumber(L, 4);
    bool large_arc = lua_toboolean(L, 5);
    bool sweep = lua_toboolean(L, 6);
    double x = luaL_checknumber(L, 7);
    double y = luaL_checknumber(L, 8);
    self->ArcTo(rx, ry, angle, large_arc, sweep, x, y);
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

    lua_createtable(L, 0, sizeof(graph_lib) / sizeof(luaL_Reg) - 1);
    fprintf(stderr, "DEBUG: %d\n", sizeof(graph_lib) / sizeof(luaL_Reg) - 1);
    luaL_setfuncs(L, graph_lib, 0);
    lua_setglobal(L, "graph");
    return 0;
}
