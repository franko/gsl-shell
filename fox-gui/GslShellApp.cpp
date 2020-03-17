
#include <unistd.h>

#include "GslShellApp.h"
#include "GslShellWindow.h"
#include "elem_plot.h"
#include "elem_plot_fox.h"
#include "elem_plot_lua.h"
#include "fatal.h"
#include "icons.h"

FXDEFMAP(GslShellApp) GslShellApp_map[]=
{
    FXMAPFUNC(SEL_IO_READ, GslShellApp::ID_LUA_REQUEST, GslShellApp::on_lua_request),
    FXMAPFUNC(SEL_IO_READ, GslShellApp::ID_LUA_QUIT, GslShellApp::on_lua_quit),
    FXMAPFUNC(SEL_COMMAND, GslShellApp::ID_CONSOLE_CLOSE, GslShellApp::on_console_close),
    FXMAPFUNC(SEL_COMMAND, GslShellApp::ID_LUA_RESTART, GslShellApp::on_restart_lua_request),
    FXMAPFUNC(SEL_COMMAND, GslShellApp::ID_LUA_INTERRUPT, GslShellApp::on_lua_interrupt),
};

FXIMPLEMENT(GslShellApp,FXApp,GslShellApp_map,ARRAYNUMBER(GslShellApp_map))

GslShellApp* global_app;

GslShellApp::GslShellApp():
FXApp("GSL Shell", "GSL Shell"), m_engine(this), m_redirect(2048, 2048)
{
    m_signal_request = new FXGUISignal(this, this, ID_LUA_REQUEST);

    FXGUISignal* quit = new FXGUISignal(this, this, ID_LUA_QUIT);
    m_engine.set_closing_signal(quit);

    m_redirect.start();

    global_app = this;
    m_engine.start();

    gsl_shell_icon = new FXGIFIcon(this, gsl_shell_icon_data);
    gsl_shell_mini = new FXGIFIcon(this, gsl_shell_mini_data);
    plot_icon = new FXGIFIcon(this, plot_icon_data);

    GslShellWindow *gsw = new GslShellWindow(&m_engine, &m_redirect, this, "GSL Shell", gsl_shell_icon, gsl_shell_mini, 700, 600);
    m_console = gsw->console();

    elem::InitializeFonts();
    elem::LuaOpenLibrary(m_engine.L);
    elem::SetFoxWindowSystem(m_console->elem_start_signal());
}

GslShellApp::~GslShellApp()
{
    m_redirect.stop();

    delete m_signal_request;
    delete gsl_shell_icon;
    delete gsl_shell_mini;
    delete plot_icon;
}

void GslShellApp::create()
{
    FXApp::create();
    gsl_shell_icon->create();
    gsl_shell_mini->create();
    plot_icon->create();
}

void GslShellApp::window_create_request(FXMainWindow* win)
{
    m_request.cmd = create_window_rq;
    m_request.win = win;
    m_signal_request->signal();
}

// this is called when Lua ask to close a window
void GslShellApp::window_close_request(FXMainWindow* win)
{
    m_request.cmd = close_window_rq;
    m_request.win = win;
    m_signal_request->signal();
}

void GslShellApp::reset_console_request()
{
    m_request.cmd = clear_console_rq;
    m_signal_request->signal();
}


long GslShellApp::on_lua_request(FXObject*, FXSelector, void*)
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

long GslShellApp::on_lua_quit(FXObject*, FXSelector, void*)
{
    exit(0);
    return 1;
}

long GslShellApp::on_console_close(FXObject* sender, FXSelector, void*)
{
    m_engine.set_request(gsl_shell_thread::exit_request);
    return 1;
}

void GslShellApp::wait_action()
{
    FXMutex& app_mutex = mutex();
    do
    {
        m_request.wait(app_mutex);
    }
    while (m_request.cmd != no_rq);
}

long GslShellApp::on_restart_lua_request(FXObject*, FXSelector, void*)
{
    m_engine.set_request(gsl_shell_thread::restart_request);
    return 0;
}

long GslShellApp::on_lua_interrupt(FXObject*, FXSelector, void*)
{
    m_engine.interrupt_request();
    return 0;
}
