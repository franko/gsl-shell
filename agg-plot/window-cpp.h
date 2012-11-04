
#include "lua-defs.h"

#include "lua-gsl.h"
#include "lua-utils.h"
#include "gs-types.h"
#include "window.h"
#include "canvas-window-cpp.h"
#include "window_surface.h"

class window;

class native_display_window : public display_window {
public:
    native_display_window(): m_window(0) {}

    void attach(window* win) { m_window = win; }

    virtual void update_region(const agg::rect_i& r);
private:
    window* m_window;
};

class window : public canvas_window {
public:
    int window_id;

    class acquire {
    public:
        acquire(lua_State*L, int index)
        {
            m_window = (window*) gs_is_userdata(L, index, GS_WINDOW);
            if (m_window)
                m_window->lock();
        }

        ~acquire()
        {
            if (m_window)
                m_window->unlock();
        }

        window* get_window() { return m_window; }

        bool is_defined() const { return (m_window != 0); }
        bool is_running() const { return m_window->status == canvas_window::running; }

    private:
        window* m_window;
    };

public:
    window(gsl_shell_state* gs, const char* split, agg::rgba8 bgcol= colors::white):
        canvas_window(gs, bgcol), m_surface(&m_window_proxy, split)
    {
        m_window_proxy.attach(this);
    }

    void surface_update_region(const agg::rect_i& r);

    window_surface& surface() { return m_surface; }

    int start(lua_State *L);

    virtual void on_draw();
    virtual void on_resize(int sx, int sy);

private:
    native_display_window m_window_proxy;
    window_surface m_surface;
};
