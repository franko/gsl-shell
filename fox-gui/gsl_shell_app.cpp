
#include <unistd.h>

#include "gsl_shell_app.h"
#include "gsl_shell_window.h"
#include "fx_plot_window.h"
#include "lua_plot_window.h"
#include "fatal.h"
#include "icons.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]=
{
    FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_REQUEST, gsl_shell_app::on_lua_request),
    FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_QUIT, gsl_shell_app::on_lua_quit),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_CONSOLE_CLOSE, gsl_shell_app::on_console_close),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_LUA_RESTART, gsl_shell_app::on_restart_lua_request),
    FXMAPFUNC(SEL_CLOSE, 0, gsl_shell_app::on_window_close),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

gsl_shell_app* global_app;

gsl_shell_app::gsl_shell_app() : FXApp("GSL Shell", "GSL Shell"),
    m_engine(this)
{
    m_signal_request = new FXGUISignal(this, this, ID_LUA_REQUEST);

    FXGUISignal* quit = new FXGUISignal(this, this, ID_LUA_QUIT);
    m_engine.set_closing_signal(quit);

    global_app = this;
    m_engine.start();

    gsl_shell_icon = new FXGIFIcon(this, gsl_shell_icon_data);
    gsl_shell_mini = new FXGIFIcon(this, gsl_shell_mini_data);
    plot_icon = new FXGIFIcon(this, plot_icon_data);

    gsl_shell_window *gsw = new gsl_shell_window(&m_engine, this, "GSL Shell Console", gsl_shell_icon, gsl_shell_mini, 600, 500);
    m_console = gsw->console();
}

gsl_shell_app::~gsl_shell_app()
{
    delete m_signal_request;
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

void gsl_shell_app::window_create_request(FXMainWindow* win)
{
    m_request.cmd = create_window_rq;
    m_request.win = win;
    m_signal_request->signal();
}

// this is called when Lua ask to close a window
void gsl_shell_app::window_close_request(FXMainWindow* win)
{
    m_request.cmd = close_window_rq;
    m_request.win = win;
    m_signal_request->signal();
}

void gsl_shell_app::reset_console_request()
{
    m_request.cmd = clear_console_rq;
    m_signal_request->signal();
}


long gsl_shell_app::on_lua_request(FXObject*, FXSelector, void*)
{
    switch (m_request.cmd)
    {
    case create_window_rq:
    {
        FXMainWindow* win = m_request.win;
        win->create();
        win->show(PLACEMENT_SCREEN);
        break;
    }
    case close_window_rq:
    {
        FXMainWindow* win = m_request.win;
        win->close(FALSE);
        break;
    }
    case clear_console_rq:
    {
        m_console->init();
        break;
    }
    default:
        m_request.cmd = no_rq;
        return 1;
    }
    m_request.cmd = no_rq;
    m_request.signal_done();
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
        m_request.wait(app_mutex);
    }
    while (m_request.cmd != no_rq);
}

long gsl_shell_app::on_restart_lua_request(FXObject*, FXSelector, void*)
{
    m_engine.set_request(gsl_shell_thread::restart_request);
    return 0;
}
