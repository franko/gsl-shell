
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
  fprintf(stderr, "LUA INTERRUPT HANDLER: locking...\n");
  m_lua_int.lock();
  m_waiting_lua = true;
  mutex().unlock();
  fprintf(stderr, "LUA INTERRUPT HANDLER: sending signal...\n");
  m_lua_int.signal();
  fprintf(stderr, "LUA INTERRUPT HANDLER: waiting END signal...\n");
  m_lua_int.wait();
  m_waiting_lua = false;
  m_lua_int.unlock();
  mutex().lock();
  fprintf(stderr, "LUA INTERRUPT HANDLER: Done.\n");
  return 1;
}
