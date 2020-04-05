
#include "fox_gsl_shell.h"
#include "GslShellApp.h"

void
fox_gsl_shell::QuitCallback()
{
    if (m_close)
        m_close->signal();
}
void
fox_gsl_shell::RestartCallback()
{
    m_app->lock();
    m_app->reset_console_request();
    m_app->wait_action();
    m_app->unlock();
}
