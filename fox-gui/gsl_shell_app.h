#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"
#include "fox_gsl_shell.h"

class gsl_shell_app : public FXApp
{
    FXDECLARE(gsl_shell_app)
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
    void wait_window_mapping();

    long on_lua_request(FXObject*,FXSelector,void*);
    long on_window_close(FXObject*,FXSelector,void*);
    long on_console_close(FXObject*,FXSelector,void*);
    long on_lua_quit(FXObject*,FXSelector,void*);

    enum
    {
        ID_LUA_REQUEST = FXApp::ID_LAST,
        ID_CONSOLE_CLOSE,
        ID_LUA_QUIT,
        ID_LAST
    };

private:
    fox_gsl_shell m_engine;
    FXGUISignal* m_lua_request;
    agg::pod_bvector<FXMainWindow*> m_win_queue;
    FXCondition m_window_mapping;
};

extern gsl_shell_app* global_app;

#endif
