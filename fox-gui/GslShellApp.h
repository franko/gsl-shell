#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"
#include "fox_gsl_shell.h"
#include "LuaConsole.h"
#include "redirect.h"

class GslShellApp : public FXApp
{
    FXDECLARE(GslShellApp)

    enum lua_request_e {
        no_rq = 0,
        create_window_rq,
        close_window_rq,
        clear_console_rq,
    };

    struct lua_request {
        lua_request_e cmd;
        FXMainWindow* win;

        lua_request(): cmd(no_rq) { }

        void signal_done() { m_term_cond.signal(); }
        void wait(FXMutex& m) { m_term_cond.wait(m); }

    private:
        FXCondition m_term_cond;
    };

public:
    GslShellApp();
    ~GslShellApp();

    virtual void create();

    void lock()
    {
        mutex().lock();
    }
    void unlock()
    {
        mutex().unlock();
    }

    void window_create_request(FXMainWindow* win);
    void window_close_request(FXMainWindow* win);
    void reset_console_request();

    void wait_action();

    long on_lua_request(FXObject*,FXSelector,void*);
    long on_window_close(FXObject*,FXSelector,void*);
    long on_console_close(FXObject*,FXSelector,void*);
    long on_lua_quit(FXObject*,FXSelector,void*);
    long on_restart_lua_request(FXObject*,FXSelector,void*);
    long on_lua_interrupt(FXObject*,FXSelector,void*);

    FXIcon* gsl_shell_icon;
    FXIcon* gsl_shell_mini;
    FXIcon* plot_icon;

    enum
    {
        ID_LUA_REQUEST = FXApp::ID_LAST,
        ID_CONSOLE_CLOSE,
        ID_LUA_RESTART,
        ID_LUA_INTERRUPT,
        ID_LUA_QUIT,
        ID_LAST
    };

private:
    fox_gsl_shell m_engine;
    FXGUISignal* m_signal_request;
    lua_request m_request;
    LuaConsole* m_console;
    io_redirect m_redirect;
};

extern GslShellApp* global_app;

#endif
