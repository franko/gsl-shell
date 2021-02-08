
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua_plot_window.h"
#include "gsl_shell_app.h"
#include "window_registry.h"
#include "fx_plot_window.h"
#include "lua-cpp-utils.h"
#include "lua-graph.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "plot.h"
#include "canvas_svg.h"

__BEGIN_DECLS

static int fox_window_layout(lua_State* L);
static int fox_window_export_svg (lua_State *L);

static const struct luaL_Reg fox_window_functions[] =
{
    {"window",         fox_window_new},
    {NULL, NULL}
};

static const struct luaL_Reg fox_window_methods[] =
{
    {"show",           fox_window_show },
    {"layout",         fox_window_layout },
    {"attach",         fox_window_attach        },
    {"close",          fox_window_close        },
    {"refresh",        fox_window_slot_refresh        },
    {"update",         fox_window_slot_update },
    {"save_svg",       fox_window_export_svg },
    {NULL, NULL}
};

__END_DECLS

typedef plot sg_plot;

class window_mutex {
public:
    window_mutex(lua_State* L, int index)
    {
        m_handle = (lua_fox_window*) gs_is_userdata(L, index, GS_WINDOW);
        if (m_handle)
            m_handle->app->lock();
    }

    ~window_mutex()
    {
        if (m_handle)
            m_handle->app->unlock();
    }

    bool is_defined() { return (m_handle != NULL); }
    bool is_running() { return (m_handle->status == running); }
    int window_status() { return m_handle->status; }

    gsl_shell_app*  app()    { return m_handle->app; }
    fx_plot_window* window() { return m_handle->window; }

private:
    lua_fox_window* m_handle;
};

static void
app_create_window(lua_State* L, gsl_shell_app* app, fx_plot_window* win)
{
    app->window_create_request(win);
    win->lua_id = window_index_add (L, -1);
    app->wait_action();

    win->set_lua_status(running);
}

int
fox_window_new (lua_State *L)
{
    gsl_shell_app* app = global_app;

    const char* split_str = lua_tostring(L, 1);
    int defer_show = (lua_gettop(L) >= 2 ? lua_toboolean(L, 2) : 0);

    app->lock();

    const float scale = gsl_shell_app::get_dpi_scale();
    lua_fox_window* bwin = new(L, GS_WINDOW) lua_fox_window();
    fx_plot_window* win = new fx_plot_window(app, "GSL Shell FX plot", app->plot_icon, NULL, 480 * scale, 480 * scale);

    if (split_str)
    {
        if (!win->surface().split(split_str))
        {
            delete win;
            app->unlock();
            return luaL_error(L, "invalid split specification");
        }
    }

    bwin->window = win;
    bwin->app    = app;
    bwin->status = not_ready;

    win->set_lua_window(bwin);

    win->setTarget(app);

    if (!defer_show)
        app_create_window(L, app, win);

    app->unlock();
    return 1;
}

static int
fox_window_show_try(lua_State* L)
{
    window_mutex wm(L, 1);
    if (!wm.is_defined()) return type_error_return(L, 2, "window");
    if (wm.window_status() != not_ready) return error_return(L, "window is already running or closed");
    fx_plot_window* win = wm.window();
    gsl_shell_app* app = win->get_app();
    app_create_window(L, app, win);
    return 0;
}

int
fox_window_show(lua_State* L)
{
    int nret = fox_window_show_try(L);
    if (nret < 0) return lua_error(L);
    return nret;
}

static int
fox_window_layout_try(lua_State* L)
{
    window_mutex wm(L, 1);
    const char *spec = lua_tostring(L, 2);

    if (!spec) return type_error_return(L, 2, "string");

    fx_plot_window* win = wm.window();
    window_surface& surface = win->surface();

    const int window_lua_index = 1;
    for (unsigned k = 0; k < surface.plot_number(); k++)
    {
        if (surface.plot(k))
            window_refs_remove(L, k + 1, window_lua_index);
    }

    bool split_success = surface.split(spec);
    surface.draw_all();

    if (!split_success)
        return error_return(L, "invalid split specification");

    return 0;
}

int
fox_window_layout(lua_State* L)
{
    int nret = fox_window_layout_try(L);
    if (nret < 0) lua_error(L);
    return nret;
}

static int
fox_window_attach_try(lua_State *L)
{
    window_mutex wm(L, 1);

    if (!wm.is_defined()) return type_error_return(L, 1, "window");

    sg_plot* p = object_cast<sg_plot>(L, 2, GS_PLOT);
    if (!p) return type_error_return(L, 2, "plot");

    const char* slot_str = lua_tostring(L, 3);

    if (!slot_str) return type_error_return(L, 3, "string");

    fx_plot_window* win = wm.window();
    window_surface& surface = win->surface();

    int index = surface.attach(p, slot_str);
    if (index < 0) return error_return(L, "invalid slot specification");

    if (surface.is_ready())
        surface.slot_update(index);

    window_refs_add (L, index + 1, 1, 2);
    return 0;
}

int
fox_window_attach(lua_State* L)
{
    int nret = fox_window_attach_try(L);
    if (nret < 0) lua_error(L);
    return nret;
}

static int
fox_window_close_try (lua_State *L)
{
    window_mutex wm(L, 1);

    if (!wm.is_defined()) return type_error_return(L, 1, "window");
    if (!wm.is_running()) return 0;

    fx_plot_window* win = wm.window();
    gsl_shell_app* app = wm.app();

    int window_id = win->lua_id;

    app->window_close_request(win);
    app->wait_action();

    window_index_remove (L, window_id);
    return 0;
}

int
fox_window_close(lua_State* L)
{
    int nret = fox_window_close_try(L);
    if (nret < 0) lua_error(L);
    return nret;
}

int
fx_canvas_slot_operation(lua_State *L, void (window_surface::*method_ptr)(unsigned))
{
    window_mutex wm(L, 1);

    if (!wm.is_defined()) return type_error_return(L, 1, "window");
    if (!wm.is_running()) return 0;

    if (!lua_isnumber(L, 2)) return type_error_return(L, 2, "integer");
    int slot_id = lua_tointeger(L, 2);

    if (slot_id <= 0) return error_return(L, "invalid slot index");

    fx_plot_window* win = wm.window();
    window_surface& surface = win->surface();

    if (surface.is_ready())
    {
        (surface.*method_ptr)(slot_id - 1);
    }

    return 0;
}

int
fox_window_slot_refresh(lua_State* L)
{
    int nret = fx_canvas_slot_operation(L, &window_surface::slot_refresh);
    if (nret < 0) lua_error(L);
    return nret;
}

int
fox_window_slot_update(lua_State* L)
{
    int nret = fx_canvas_slot_operation(L, &window_surface::slot_update);
    if (nret < 0) lua_error(L);
    return nret;
}

int
fox_window_save_slot_image (lua_State *L)
{
    int nret = fx_canvas_slot_operation(L, &window_surface::save_slot_image);
    if (nret < 0) lua_error(L);
    return nret;
}

int
fox_window_restore_slot_image (lua_State *L)
{
    int nret = fx_canvas_slot_operation(L, &window_surface::restore_slot_image);
    if (nret < 0) lua_error(L);
    return nret;
}

static int
fox_window_export_svg_try(lua_State *L)
{
    window_mutex wm(L, 1);
    const char *filename = lua_tostring(L, 2);
    const double w = luaL_optnumber(L, 3, 600.0);
    const double h = luaL_optnumber(L, 4, 600.0);

    if (!wm.is_defined()) return type_error_return(L, 1, "window");

    if (!wm.is_running()) return error_return(L, "window is not running");

    if (!filename) return type_error_return(L, 2, "string");

    unsigned fnlen = strlen(filename);
    if (fnlen <= 4 || strcmp(filename + (fnlen - 4), ".svg") != 0)
    {
        const char* basename = (fnlen > 0 ? filename : "unnamed");
        lua_pushfstring(L, "%s.svg", basename);
        filename = lua_tostring(L, -1);
    }

    FILE* f = fopen(filename, "w");
    if (!f)
    {
        lua_pushfstring(L, "cannot open filename: %s", filename);
        return (-1);
    }

    fx_plot_window* win = wm.window();
    window_surface& surface = win->surface();

    canvas_svg canvas(f, h);
    canvas.write_header(w, h);

    unsigned n = surface.plot_number();
    for (unsigned k = 0; k < n; k++)
    {
        char plot_name[64];
        sg_plot* p = surface.plot(k);
        if (p)
        {
            agg::rect_i area = surface.get_plot_area(k, int(w), int(h));
            sprintf(plot_name, "plot%u", k + 1);
            canvas.write_group_header(plot_name);
            p->draw(canvas, area, NULL);
            canvas.write_group_end(plot_name);
        }
    }

    canvas.write_end();
    fclose(f);

    return 0;
}

int
fox_window_export_svg(lua_State *L)
{
    int nret = fox_window_export_svg_try(L);
    if (nret < 0) return lua_error(L);
    return nret;
}

#if 0
void lua_window_set_closed(void* _win)
{
    lua_fox_window *win = (lua_fox_window*) _win;
    win->status = closed;
}
#endif

void
fox_window_register (lua_State *L)
{
    luaL_newmetatable (L, GS_METATABLE(GS_WINDOW));
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_register (L, NULL, fox_window_methods);
    lua_pop (L, 1);

    luaL_register (L, NULL, fox_window_functions);
}
