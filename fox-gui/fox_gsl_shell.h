#ifndef FOX_GSL_SHELL_H
#define FOX_GSL_SHELL_H

#include <fx.h>

#include "gsl_shell_thread.h"

class GslShellApp;

class fox_gsl_shell : public gsl_shell_thread
{
public:
    fox_gsl_shell(GslShellApp* app): m_app(app), m_close(0) { }

    ~fox_gsl_shell() { delete m_close; }

    virtual void init();
    virtual void close();

    virtual void restart_callback();
    virtual void quit_callback();

    void set_closing_signal(FXGUISignal* s) { m_close = s; }

private:
    GslShellApp* m_app;
    FXGUISignal* m_close;
};

#endif
