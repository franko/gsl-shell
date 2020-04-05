#include "fox_gsl_shell.h"

#include "GslShellApp.h"
#include "LuaInterpreter.h"

fox_gsl_shell::fox_gsl_shell(GslShellApp* app):
    InterpreterThread(std::make_unique<LuaInterpreter>(LuaLanguage::kLanguageExtension)),
    m_app(app), m_close(nullptr) { }

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
