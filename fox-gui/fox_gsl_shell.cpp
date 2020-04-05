
#include "fox_gsl_shell.h"
#include "GslShellApp.h"

void fox_gsl_shell::init()
{
    gsl_shell_thread::Initialize();
}

void fox_gsl_shell::close()
{
    gsl_shell_thread::Close();
}

void
fox_gsl_shell::quit_callback()
{
    if (m_close)
        m_close->signal();
}
void
fox_gsl_shell::restart_callback()
{
    m_app->lock();
    m_app->reset_console_request();
    m_app->wait_action();
    m_app->unlock();
}
