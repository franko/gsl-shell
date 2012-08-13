#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"
#include "fox_gsl_shell.h"
#include "fx_console.h"

class gsl_shell_app : public FXApp
{
    FXDECLARE(gsl_shell_app)

    enum lua_request_e { no_rq = 0, create_window_rq, close_window_rq };

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
    gsl_shell_app();
    ~gsl_shell_app();

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
    void wait_action();
    void reset_console();

    long on_lua_request(FXObject*,FXSelector,void*);
    long on_window_close(FXObject*,FXSelector,void*);
    long on_console_close(FXObject*,FXSelector,void*);
    long on_lua_quit(FXObject*,FXSelector,void*);
    long on_restart_lua_request(FXObject*,FXSelector,void*);

    enum
    {
        ID_LUA_REQUEST = FXApp::ID_LAST,
        ID_CONSOLE_CLOSE,
        ID_LUA_RESTART,
        ID_LUA_QUIT,
        ID_LAST
    };

private:
    fox_gsl_shell m_engine;
    FXGUISignal* m_signal_request;
    lua_request m_request;
    fx_console* m_console;
};

extern gsl_shell_app* global_app;

#endif
