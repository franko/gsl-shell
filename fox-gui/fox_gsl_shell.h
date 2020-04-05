#ifndef FOX_GSL_SHELL_H
#define FOX_GSL_SHELL_H

#include <memory>

#include "fx.h"

#include "InterpreterThread.h"

class GslShellApp;

class fox_gsl_shell : public InterpreterThread
{
public:
    fox_gsl_shell(GslShellApp* app);
    ~fox_gsl_shell() { delete m_close; }

    void RestartCallback() override;
    void QuitCallback() override;

    void set_closing_signal(FXGUISignal* s) { m_close = s; }

private:
    GslShellApp* m_app;
    FXGUISignal* m_close;
};

#endif
