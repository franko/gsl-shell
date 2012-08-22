
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
#include "plot.h"

__BEGIN_DECLS

static const struct luaL_Reg fox_window_functions[] =
{
    {"window",         fox_window_new},
    {NULL, NULL}
};

static const struct luaL_Reg fox_window_methods[] =
{
    {"attach",         fox_window_attach        },
    {"close",          fox_window_close        },
    {"refresh",        fox_window_slot_refresh        },
    {"update",         fox_window_slot_update },
    {NULL, NULL}
};

enum window_status_e { not_ready, running, closed };

struct lua_fox_window
{
    fx_plot_window* window;
    gsl_shell_app* app;
    enum window_status_e status;
};

__END_DECLS

typedef plot<manage_owner> sg_plot;

static lua_fox_window*
check_fox_window_lock(lua_State* L, int index)
{
    lua_fox_window *lwin = object_check<lua_fox_window>(L, 1, GS_FOX_WINDOW);
    lwin->app->lock();
    if (lwin->status != running)
    {
        lwin->app->unlock();
        return 0;
    }
    return lwin;
}

int
fox_window_new (lua_State *L)
{
    gsl_shell_app* app = global_app;
    app->lock();

    lua_fox_window* bwin = new(L, GS_FOX_WINDOW) lua_fox_window();
    fx_plot_window* win = new fx_plot_window(app, "GSL Shell FX plot", app->plot_icon, NULL, 480, 480);

    bwin->window = win;
    bwin->app    = app;
    bwin->status = not_ready;

    win->set_lua_window(bwin);

    win->setTarget(app);

    app->window_create_request(win);
    win->lua_id = window_index_add (L, -1);
    app->wait_action();

    bwin->status = running;

    app->unlock();
    return 1;
}

int
fox_window_attach (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return luaL_error(L, "window is not running");

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;
    sg_plot* p = object_check<sg_plot>(L, 2, GS_PLOT);
    win->canvas()->attach(p);
    app->unlock();
    int slot_id = 1;
    window_refs_add (L, slot_id, 1, 2);
    return 0;
}

int
fox_window_close (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return 0;

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;

    int window_id = win->lua_id;

    app->window_close_request(win);
    app->wait_action();
    app->unlock();

    window_index_remove (L, window_id);

    return 0;
}

int
fox_window_slot_refresh (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return 0;

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;
    fx_plot_canvas* canvas = win->canvas();

    if (canvas->is_ready())
    {
        agg::trans_affine& m = canvas->plot_matrix();
        bool redraw = canvas->get_plot()->need_redraw();
        if (redraw)
            canvas->plot_render(m);
        canvas->plot_draw_queue(m, redraw);
    }

    app->unlock();
    return 0;
}

int
fox_window_slot_update (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return 0;

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;
    fx_plot_canvas* canvas = win->canvas();

    if (canvas->is_ready())
    {
        agg::trans_affine& m = canvas->plot_matrix();
        canvas->plot_render(m);
        canvas->plot_draw_queue(m, true);
    }

    app->unlock();
    return 0;
}

int
fox_window_save_slot_image (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return 0;

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;
    fx_plot_canvas* canvas = win->canvas();
    canvas->save_image();
    app->unlock();
    return 0;
}

int
fox_window_restore_slot_image (lua_State *L)
{
    lua_fox_window *lwin = check_fox_window_lock(L, 1);
    if (!lwin) return 0;

    fx_plot_window* win = lwin->window;
    gsl_shell_app* app = lwin->app;
    fx_plot_canvas* canvas = win->canvas();
    if (!canvas->restore_image())
    {
        agg::trans_affine& m = canvas->plot_matrix();
        canvas->plot_render(m);
        canvas->save_image();
    }
    app->unlock();
    return 0;
}

void lua_window_set_closed(void* _win)
{
    lua_fox_window *win = (lua_fox_window*) _win;
    win->status = closed;
}

void
fox_window_register (lua_State *L)
{
    luaL_newmetatable (L, GS_METATABLE(GS_FOX_WINDOW));
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_register (L, NULL, fox_window_methods);
    lua_pop (L, 1);

    luaL_register (L, NULL, fox_window_functions);
}
