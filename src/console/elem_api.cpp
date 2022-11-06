#include <new>

#include <elem/elem.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

#define API_TYPE_PATH    "Path"
#define API_TYPE_CURVE   "Curve"
#define API_TYPE_DASH    "Dash"
#define API_TYPE_MARKERS "Markers"
#define API_TYPE_PLOT    "Plot"

extern "C" {
static int f_object_gc(lua_State *L);

static int f_path_new(lua_State *L);
static int f_path_move_to(lua_State *L);
static int f_path_line_to(lua_State *L);
static int f_path_close(lua_State *L);

static int f_curve_new(lua_State *L);
static int f_curve_move_to(lua_State *L);
static int f_curve_line_to(lua_State *L);
static int f_curve_close(lua_State *L);
static int f_curve_curve3(lua_State *L);
static int f_curve_curve4(lua_State *L);
static int f_curve_arc_to(lua_State *L);

static int f_dash_new(lua_State *L);
static int f_dash_add_dash(lua_State *L);

static int f_plot_new(lua_State *L);
static int f_plot_gc(lua_State *L);
static int f_plot_add(lua_State *L);
static int f_plot_addline(lua_State *L);
static int f_plot_show(lua_State *L);

extern int luaopen_graph(lua_State *L);
}

int f_object_gc(lua_State *L) {
    /* We should check that the userdata is actually an instance of a
    ** derived Object type. In reality this method is normally only called
    ** by the Lua's GC and is therefore safe but in theory the function is
    ** accessible by inspecting the metatable and could therefore be
    ** erroneously called with any argument.
    ** For the moment we don't check assuming the method is not messed up. */
    elem::Object *self = (elem::Object *) lua_touserdata(L, 1);
    self->~Object();
    return 0;
}

static elem::Object *check_object_userdata(lua_State *L, int arg) {
    void *obj = nullptr;
    obj = luaL_testudata(L, arg, API_TYPE_PATH);
    if (obj) goto return_obj;
    obj = luaL_testudata(L, arg, API_TYPE_CURVE);
    if (obj) goto return_obj;
    obj = luaL_testudata(L, arg, API_TYPE_DASH);
    if (obj) goto return_obj;
    luaL_error(L, "expected argument of type Object");
return_obj:
    return (elem::Object *) obj;
}

static elem::Path *check_path_userdata(lua_State *L, int arg) {
    void *path = luaL_testudata(L, arg, API_TYPE_PATH);
    path = (path ? path : luaL_testudata(L, arg, API_TYPE_DASH));
    if (!path) {
        luaL_error(L, "expected argument of type Path");
    }
    return (elem::Path *) path;
}

int f_path_new(lua_State *L) {
    int nargs = lua_gettop(L);
    elem::Path *path = (elem::Path *) lua_newuserdata(L, sizeof(elem::Path));
    new(path) elem::Path;
    if (nargs >= 2) {
        double x = luaL_checknumber(L, 1);
        double y = luaL_checknumber(L, 2);
        path->MoveTo(x, y);
    }
    luaL_setmetatable(L, API_TYPE_PATH);
    return 1;
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
    void *udata = luaL_checkudata(L, arg, API_TYPE_CURVE);
    return (elem::CurvePath *) udata;
}

int f_curve_new(lua_State *L) {
    void *buf = lua_newuserdata(L, sizeof(elem::CurvePath));
    new(buf) elem::CurvePath;
    luaL_setmetatable(L, API_TYPE_CURVE);
    return 1;
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

int f_dash_new(lua_State *L) {
    void *buf = lua_newuserdata(L, sizeof(elem::DashPath));
    new(buf) elem::DashPath;
    luaL_setmetatable(L, API_TYPE_DASH);
    return 1;
}

int f_dash_add_dash(lua_State *L) {
    elem::DashPath *dash = (elem::DashPath *) luaL_checkudata(L, 1, API_TYPE_DASH);
    double d1 = luaL_checknumber(L, 2);
    double d2 = luaL_checknumber(L, 3);
    dash->AddDash(d1, d2);
    return 0;
}

int f_plot_new(lua_State *L) {
    elem::Plot *plot = (elem::Plot *) lua_newuserdata(L, sizeof(elem::Plot));
    new(plot) elem::Plot;
    luaL_setmetatable(L, API_TYPE_PLOT);
    if (lua_gettop(L) >= 2) {
        const char *title = luaL_checkstring(L, 1);
        plot->SetTitle(title);
    }
    return 1;
}

int f_plot_gc(lua_State *L) {
    elem::Plot *self = (elem::Plot *) lua_touserdata(L, 1);
    self->~Plot();
    return 0;
}

int f_plot_newindex(lua_State *L) {
    static const char *properties[] = {
        "title", "xtitle", "ytitle", "xlab_angle", "ylab_angle",
        "xlab_format", "ylab_format", nullptr
    };
    elem::Plot *plot = (elem::Plot *) luaL_checkudata(L, 1, API_TYPE_PLOT);
    int property_index = luaL_checkoption(L, 2, nullptr, properties);
    switch (property_index) {
        const char *sarg;
        double narg;
    case 0:
        sarg = luaL_checkstring(L, 3);
        plot->SetTitle(sarg);
        break;
    case 1:
        sarg = luaL_checkstring(L, 3);
        plot->SetXAxisTitle(sarg);
        break;
    case 2:
        sarg = luaL_checkstring(L, 3);
        plot->SetYAxisTitle(sarg);
        break;
    case 3:
        narg = luaL_checknumber(L, 3);
        plot->SetAxisLabelsAngle(elem::xAxis, narg);
        break;
    case 4:
        narg = luaL_checknumber(L, 3);
        plot->SetAxisLabelsAngle(elem::yAxis, narg);
        break;
    case 5:
        sarg = luaL_checkstring(L, 3);
        plot->EnableLabelFormat(elem::xAxis, sarg);
        break;
    case 6:
        sarg = luaL_checkstring(L, 3);
        plot->EnableLabelFormat(elem::yAxis, sarg);
        break;
    default:
        /* */ ;
    }
    return 0;
}

int f_plot_add(lua_State *L) {
    elem::Plot *plot = (elem::Plot *) luaL_checkudata(L, 1, API_TYPE_PLOT);
    elem::Object *element = check_object_userdata(L, 2);
    uint32_t stroke_color = luaL_checknumber(L, 3);
    double stroke_width = luaL_checknumber(L, 4);
    uint32_t fill_color = luaL_checknumber(L, 5);
    plot->Add(*element, stroke_color, stroke_width, fill_color);
    return 0;
}

int f_plot_addline(lua_State *L) {
    elem::Plot *plot = (elem::Plot *) luaL_checkudata(L, 1, API_TYPE_PLOT);
    elem::Object *element = check_object_userdata(L, 2);
    uint32_t stroke_color = luaL_checknumber(L, 3);
    double stroke_width = 1.5;
    if (lua_gettop(L) >= 4) {
        stroke_width = luaL_checknumber(L, 4);
    }
    plot->AddStroke(*element, stroke_color, stroke_width);
    return 0;
}

int f_plot_show(lua_State *L) {
    elem::Plot *plot = (elem::Plot *) luaL_checkudata(L, 1, API_TYPE_PLOT);
    int w, h;
    if (lua_gettop(L) >= 3) {
        w = luaL_checkint(L, 2);
        h = luaL_checkint(L, 3);
    } else {
        w = 640;
        h = 480;
    }
    plot->Show(w, h, elem::WindowResize);
    return 0;
}

static const luaL_Reg path_lib[] = {
    { "__gc",               f_object_gc         },
    { "move_to",            f_path_move_to      },
    { "line_to",            f_path_line_to      },
    { "close",              f_path_close        },
    { nullptr, nullptr }
};

static const luaL_Reg curve_lib[] = {
    { "__gc",               f_object_gc         },
    { "move_to",            f_curve_move_to     },
    { "line_to",            f_curve_line_to     },
    { "close",              f_curve_close       },
    { "curve3",             f_curve_curve3      },
    { "curve4",             f_curve_curve4      },
    { "arc_to",             f_curve_arc_to      },
    { nullptr, nullptr }
};

static const luaL_Reg dash_lib[] = {
    { "__gc",               f_object_gc         },
    { "move_to",            f_path_move_to      },
    { "line_to",            f_path_line_to      },
    { "close",              f_path_close        },
    { "add_dash",           f_dash_add_dash     },
    { nullptr, nullptr }
};

static const luaL_Reg plot_lib[] = {
    { "__gc",               f_plot_gc           },
    { "__newindex",         f_plot_newindex     },
    { "add",                f_plot_add          },
    { "addline",            f_plot_addline      },
    { "show",               f_plot_show         },
    { nullptr, nullptr }
};

static const luaL_Reg graph_lib[] = {
    {"path",                f_path_new          },
    {"curve",               f_curve_new         },
    {"dashed_path",         f_dash_new          },
    {"plot",                f_plot_new          },
    { nullptr, nullptr }
};

static void register_api_type(lua_State *L, const luaL_Reg *lib, const char *tname) {
    luaL_newmetatable(L, tname);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int luaopen_graph(lua_State *L) {
    register_api_type(L, path_lib,  API_TYPE_PATH);
    register_api_type(L, curve_lib, API_TYPE_CURVE);
    register_api_type(L, dash_lib,  API_TYPE_DASH);
    register_api_type(L, plot_lib,  API_TYPE_PLOT);

    lua_createtable(L, 0, sizeof(graph_lib) / sizeof(luaL_Reg) - 1);
    luaL_setfuncs(L, graph_lib, 0);
    lua_setglobal(L, "graph");
    return 0;
}
