
#include "window-cpp.h"
#include "window.h"
#include "window_surface.h"
#include "window_registry.h"
#include "lua-cpp-utils.h"
#include "lua-plot-cpp.h"

__BEGIN_DECLS

static int window_show            (lua_State *L);
static int window_free            (lua_State *L);
static int window_layout           (lua_State *L);

static const struct luaL_Reg window_functions[] = {
    {"window",        window_new},
    {NULL, NULL}
};

static const struct luaL_Reg window_methods[] = {
    {"show",           window_show          },
    {"attach",         window_attach        },
    {"layout",         window_layout        },
    {"update",         window_slot_update   },
    {"close",          window_close         },
    {"__gc",           window_free          },
    {NULL, NULL}
};

__END_DECLS

void native_display_window::update_region(const agg::rect_i& r)
{
    m_window->surface_update_region(r);
}

int window::start (lua_State *L)
{
    this->lock();

    if (status != canvas_window::running)
    {
        typedef canvas_window::thread_info thread_info;
        std::auto_ptr<thread_info> inf(new thread_info(L, this));

        this->window_id = window_index_add(L, -1);
        inf->window_id = this->window_id;

        if (! this->start_new_thread(inf))
        {
            window_index_remove(L, this->window_id);
            this->unlock();
            return error_return(L, "error during thread initialization");
        }
    }
    else
    {
        this->unlock();
        return error_return(L, "window is already active");
    }

    return 0;
}

void window::surface_update_region(const agg::rect_i& r)
{
    int ww = r.x2 - r.x1, hh= r.y2 - r.y1;
    if (ww <= 0 || hh <= 0) return;

    const window_surface::image& src_img = m_surface.get_image();

    agg::rendering_buffer dest;
    rendering_buffer_get_view(dest, rbuf_window(), r, window_surface::image_pixel_width);

    rendering_buffer_ro src;
    rendering_buffer_get_const_view(src, src_img, r, window_surface::image_pixel_width);

    dest.copy_from(src);

    update_region(r);
}

int
window_new (lua_State *L)
{
    const char* split = lua_tostring(L, 1);

    window* win = new(L, GS_WINDOW) window(global_state, split);

    if (win->start(L) < 0) return lua_error(L);
    return 1;
}

int
window_show (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    if (win->start(L) < 0) return lua_error(L);
    return 0;
}

int
window_free (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);

    /* it is important here to lock to ensure that there isn't any thread starting
       a new graphical window right now */
    win->lock();

    /* This should never happens. Running windows are never garbage collected
       and before closing lua_State all the graphical windows are closed and
       their threads have naturally finished. */
    assert(win->status != window::running);

    win->unlock();
    win->~window();
    return 0;
}

static int
window_attach_try(lua_State *L)
{
    window::acquire w(L, 1);

    sg_plot* p = object_cast<sg_plot>(L, 2, GS_PLOT);
    if (!p) return type_error_return(L, 2, "plot");

    const char* slot_str = lua_tostring(L, 3);

    if (!slot_str) return type_error_return(L, 3, "string");

    window* win = w.get_window();
    window_surface& surface = win->surface();

    int index = surface.attach(p, slot_str);
    if (index < 0) return error_return(L, "invalid slot specification");

    if (surface.is_ready())
        surface.slot_update(index);

    window_refs_add (L, index + 1, 1, 2);
    return 0;
}

int
window_attach(lua_State* L)
{
    int nret = window_attach_try(L);
    if (nret < 0) lua_error(L);
    return nret;
}

static int
window_layout_try(lua_State* L)
{
    window::acquire w(L, 1);
    const char *spec = lua_tostring(L, 2);

    if (!spec) return type_error_return(L, 2, "string");

    window* win = w.get_window();
    window_surface& surface = win->surface();

    const int window_lua_index = 1;
    for (unsigned k = 0; k < surface.plot_number(); k++)
    {
        if (surface.plot(k))
            window_refs_remove(L, k + 1, window_lua_index);
    }

    surface.split(spec);
    surface.draw_all();
    return 0;
}

int
window_layout(lua_State* L)
{
    int nret = window_layout_try(L);
    if (nret < 0) lua_error(L);
    return nret;
}

int
window_slot_operation(lua_State *L, void (window_surface::*method_ptr)(unsigned))
{
    window::acquire wm(L, 1);

    if (!wm.is_defined()) return type_error_return(L, 1, "window");
    if (!wm.is_running()) return 0;

    if (!lua_isnumber(L, 2)) return type_error_return(L, 2, "integer");
    int slot_id = lua_tointeger(L, 2);

    if (slot_id <= 0) return error_return(L, "invalid slot index");

    window* win = wm.get_window();
    window_surface& surface = win->surface();

    if (surface.is_ready())
    {
        (surface.*method_ptr)(slot_id - 1);
    }

    return 0;
}

int
window_slot_refresh(lua_State* L)
{
    int nret = window_slot_operation(L, &window_surface::slot_refresh);
    if (nret < 0) lua_error(L);
    return nret;
}

int
window_slot_update(lua_State* L)
{
    int nret = window_slot_operation(L, &window_surface::slot_update);
    if (nret < 0) lua_error(L);
    return nret;
}

int
window_save_slot_image (lua_State *L)
{
    int nret = window_slot_operation(L, &window_surface::save_slot_image);
    if (nret < 0) lua_error(L);
    return nret;
}

int
window_restore_slot_image (lua_State *L)
{
    int nret = window_slot_operation(L, &window_surface::restore_slot_image);
    if (nret < 0) lua_error(L);
    return nret;
}

static int
window_close_try (lua_State *L)
{
    window::acquire w(L, 1);
    if (!w.is_defined()) return error_return(L, "window is not running");
    if (w.is_running())
        w.get_window()->close_request();
    return 0;
}

int
window_close(lua_State* L)
{
    int nret = window_close_try(L);
    if (nret < 0) return lua_error(L);
    return nret;
}

int
window_close_wait (lua_State *L)
{
    window *win = object_check<window>(L, 1, GS_WINDOW);
    win->shutdown_close();
    return 0;
}

void
window::on_draw()
{
    int ww = width(), hh = height();
    agg::rect_i r(0, 0, ww, hh);
    surface_update_region(r);
}

void
window::on_resize(int sx, int sy)
{
    this->canvas_window::on_resize(sx, sy);

    int ww = width(), hh = height();

    if (!m_surface.canvas_size_match(ww, hh))
    {
        m_surface.resize(ww, hh);
        m_surface.draw_image_buffer();
    }
}

void
window_register (lua_State *L)
{
    luaL_newmetatable (L, GS_METATABLE(GS_WINDOW));
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_register (L, NULL, window_methods);
    lua_pop (L, 1);

    /* gsl module registration */
    luaL_register (L, NULL, window_functions);
}
