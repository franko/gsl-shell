
#include <unistd.h>

#include "gsl_shell_app.h"
#include "gsl_shell_window.h"
#include "fx_plot_window.h"
#include "lua_plot_window.h"
#include "fatal.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]=
{
    FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_REQUEST, gsl_shell_app::on_lua_request),
    FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_QUIT, gsl_shell_app::on_lua_quit),
    FXMAPFUNC(SEL_COMMAND, gsl_shell_app::ID_CONSOLE_CLOSE, gsl_shell_app::on_console_close),
    FXMAPFUNC(SEL_CLOSE, 0, gsl_shell_app::on_window_close),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

gsl_shell_app* global_app;

gsl_shell_app::gsl_shell_app() : FXApp("GSL Shell", "GSL Shell"),
    m_engine(this)
{
    m_lua_request = new FXGUISignal(this, this, ID_LUA_REQUEST);

    FXGUISignal* quit = new FXGUISignal(this, this, ID_LUA_QUIT);
    m_engine.set_closing_signal(quit);

    global_app = this;
    m_engine.start();

    new gsl_shell_window(&m_engine, this, "GSL Shell Console", NULL, NULL, 600, 500);
}

gsl_shell_app::~gsl_shell_app()
{
    delete m_lua_request;
}

long gsl_shell_app::on_lua_request(FXObject*, FXSelector, void*)
{
    for (unsigned k = 0; k < m_win_queue.size(); k++)
    {
        FXMainWindow* win = m_win_queue[k];
        win->create();
        win->show(PLACEMENT_SCREEN);
    }
    m_win_queue.clear();
    m_window_mapping.signal();
    return 1;
}

long gsl_shell_app::on_lua_quit(FXObject*, FXSelector, void*)
{
    m_engine.close();
    exit(0);
    return 1;
}

void gsl_shell_app::window_create_request(FXMainWindow* win)
{
    m_win_queue.add(win);
    m_lua_request->signal();
}

long gsl_shell_app::on_console_close(FXObject* sender, FXSelector, void*)
{
    m_engine.stop();
    return 1;
}

long gsl_shell_app::on_window_close(FXObject* sender, FXSelector, void*)
{
    fx_plot_window* win = (fx_plot_window*) sender;
    m_engine.window_close_notify(win->lua_id);
    return 0;
}

void gsl_shell_app::wait_window_mapping()
{
    FXMutex& app_mutex = mutex();
    m_window_mapping.wait(app_mutex);
}
