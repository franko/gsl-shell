#ifdef _WIN32
  #include <windows.h>
#endif

#include <unistd.h>

#include "gsl_shell_app.h"
#include "gsl_shell_window.h"
#include "fx_plot_window.h"
#include "lua_plot_window.h"
#include "fatal.h"
#include "icons.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]=
{
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_LUA_REQUEST, gsl_shell_app::on_lua_request),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_LUA_QUIT, gsl_shell_app::on_lua_quit),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_CONSOLE_CLOSE, gsl_shell_app::on_console_close),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_LUA_RESTART, gsl_shell_app::on_restart_lua_request),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_LUA_INTERRUPT, gsl_shell_app::on_lua_interrupt),
    FXMAPFUNC(SEL_CLOSE, 0, gsl_shell_app::on_window_close),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

gsl_shell_app* global_app;

gsl_shell_app::gsl_shell_app():
    FXApp("GSL Shell", "GSL Shell"),
    m_engine(this), m_close_channel(this), m_request_channel(this),
    m_request_done(true),
    m_redirect(2048, 2048)
{
    m_engine.set_closing_signal(this, FXSEL(SEL_COMMAND, gsl_shell_app::ID_LUA_QUIT), &m_close_channel);

    m_redirect.start();

    global_app = this;
    m_engine.start();

    gsl_shell_icon = new FXGIFIcon(this, gsl_shell_icon_data);
    gsl_shell_mini = new FXGIFIcon(this, gsl_shell_mini_data);
    plot_icon = new FXGIFIcon(this, plot_icon_data);

    const float scale = get_dpi_scale();
    gsl_shell_window *gsw = new gsl_shell_window(&m_engine, &m_redirect, this, "GSL Shell", gsl_shell_icon, gsl_shell_mini, 700 * scale, 600 * scale);
    m_console = gsw->console();
}

gsl_shell_app::~gsl_shell_app()
{
    m_redirect.stop();

    delete gsl_shell_icon;
    delete gsl_shell_mini;
    delete plot_icon;
}

void gsl_shell_app::create()
{
    FXApp::create();
    gsl_shell_icon->create();
    gsl_shell_mini->create();
    plot_icon->create();
}

void gsl_shell_app::send_request(gsl_shell_app::lua_request request)
{
    m_request_done = false;
    m_request_channel.message(this, FXSEL(SEL_COMMAND, gsl_shell_app::ID_LUA_REQUEST),
        (void *) &request, sizeof(lua_request));
    wait_action();
}


long gsl_shell_app::on_lua_request(FXObject*, FXSelector, void* data)
{
    lua_request *request = (lua_request *) data;
    switch (request->cmd)
    {
    case create_window_rq:
    {
        FXMainWindow* win = request->win;
        win->create();
        win->show(PLACEMENT_SCREEN);
        break;
    }
    case close_window_rq:
    {
        FXMainWindow* win = request->win;
        win->close(FALSE);
        break;
    }
    case clear_console_rq:
    {
        m_console->init();
        break;
    }
    default:
        m_request_done = true;
        return 1;
    }
    m_request_done = true;
    m_request_treated.signal();
    return 1;
}

long gsl_shell_app::on_lua_quit(FXObject*, FXSelector, void*)
{
    exit(0);
    return 1;
}

long gsl_shell_app::on_console_close(FXObject* sender, FXSelector, void*)
{
    m_engine.set_request(gsl_shell_thread::exit_request);
    return 1;
}

// this is called when the user press the button to close the window.
long gsl_shell_app::on_window_close(FXObject* sender, FXSelector, void*)
{
    fx_plot_window* win = (fx_plot_window*) sender;
    m_engine.window_close_notify(win->lua_id);
    return 0;
}

void gsl_shell_app::wait_action()
{
    FXMutex& app_mutex = mutex();
    do
    {
        m_request_treated.wait(app_mutex);
    }
    while (!m_request_done);
}

long gsl_shell_app::on_restart_lua_request(FXObject*, FXSelector, void*)
{
    m_engine.set_request(gsl_shell_thread::restart_request);
    return 0;
}

long gsl_shell_app::on_lua_interrupt(FXObject*, FXSelector, void*)
{
    m_engine.interrupt_request();
    return 0;
}

float gsl_shell_app::get_dpi_scale()
{
#ifdef _WIN32
    return GetSystemMetrics(SM_CXCURSOR) / 32.0;
#else
    return 1.0;
#endif
}

