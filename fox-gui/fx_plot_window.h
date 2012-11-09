#ifndef FOXGUI_FX_PLOT_WINDOW_H
#define FOXGUI_FX_PLOT_WINDOW_H

#include <fx.h>

#include "gsl_shell_app.h"
#include "fx_plot_canvas.h"

#include "window_surface.h"

class fx_plot_window;

__BEGIN_DECLS

enum window_status_e { not_ready, running, closed };

struct lua_fox_window
{
    fx_plot_window* window;
    gsl_shell_app* app;
    enum window_status_e status;
};

__END_DECLS

class fox_display_window : public display_window {
public:
    fox_display_window(): m_fox_canvas(0) {}

    void attach(fx_plot_canvas* can) { m_fox_canvas = can; }

    virtual void update_region(const agg::rect_i& r) { m_fox_canvas->update_region(r); }
private:
    fx_plot_canvas* m_fox_canvas;
};

class fx_plot_window : public FXMainWindow
{
    FXDECLARE(fx_plot_window)
public:
    fx_plot_window(FXApp* a, const FXString& name, FXIcon *ic=NULL, FXIcon *mi=NULL, FXint w=0, FXint h=0);

    ~fx_plot_window();

    fx_plot_canvas* canvas()
    {
        return m_canvas;
    }
    gsl_shell_app* get_app()
    {
        return (gsl_shell_app*) getApp();
    }

    window_surface& surface() { return m_surface; }

    void set_lua_window(lua_fox_window* w) { m_lua_window = w; }

    void set_lua_status(window_status_e s) { m_lua_window->status = s; }

    int lua_id; // the following is used by Lua to keep trace of the window

protected:
    fx_plot_window(): m_surface(&m_display_window) {}

private:
    FXMenuBar* m_menu_bar;
    FXMenuPane* m_plot_menu;
    fx_plot_canvas* m_canvas;
    lua_fox_window* m_lua_window;
    fox_display_window m_display_window;
    window_surface m_surface;
};

#endif
