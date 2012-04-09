
#include "gsl_shell_app.h"
#include "fatal.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]={
  FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_INTERRUPT, gsl_shell_app::on_lua_interrupt),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

gsl_shell_app::gsl_shell_app() : FXApp("GSL Shell", "GSL Shell"),
  m_waiting_lua(false)
{
  m_event_loop = new FXGUISignal(this, this, ID_LUA_INTERRUPT);
}

gsl_shell_app::~gsl_shell_app()
{
  delete m_event_loop;
}

long gsl_shell_app::on_lua_interrupt(FXObject*, FXSelector, void*)
{
  FXMutex& app_mutex = mutex();
  m_waiting_lua = true;
  m_lua_int.signal();
  m_lua_int.wait(app_mutex);
  m_waiting_lua = false;
  return 1;
}

bool gsl_shell_app::interrupt()
{
  FXMutex& app_mutex = mutex();
  app_mutex.lock();

  if (!m_waiting_lua)
    {
      m_event_loop->signal();
      m_lua_int.wait(app_mutex);
      return true;
    }

  return false;
}

void gsl_shell_app::resume(bool signal_end)
{
  FXMutex& app_mutex = mutex();
  if (signal_end)
    m_lua_int.signal();
  app_mutex.unlock();
}
