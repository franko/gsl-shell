#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"
#include "fox_gsl_shell.h"
#include "fx_console.h"
#include "redirect.h"

class gsl_shell_app : public FXApp
{
    FXDECLARE(gsl_shell_app)
public:
    enum lua_request_e {
        no_rq = 0,
        create_window_rq,
        close_window_rq,
        clear_console_rq,
    };

    struct lua_request {
        lua_request_e cmd;
        FXMainWindow* win;
    };

    gsl_shell_app();
    ~gsl_shell_app();

    virtual void create();

    void lock()
    {
        mutex().lock();
    }
    void unlock()
    {
        mutex().unlock();
    }

    void send_request(lua_request request);

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

    static float get_dpi_scale();

private:
    void wait_action();

    fox_gsl_shell m_engine;
    FXMessageChannel m_engine_channel;
    FXMessageChannel m_request_channel;
    FXCondition m_request_treated;
    bool m_request_done;
    fx_console* m_console;
    io_redirect m_redirect;
};

extern gsl_shell_app* global_app;

#endif
