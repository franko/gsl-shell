
#include "gsl_shell_app.h"
#include "fatal.h"

FXDEFMAP(gsl_shell_app) gsl_shell_app_map[]={
  FXMAPFUNC(SEL_IO_READ, gsl_shell_app::ID_LUA_INTERRUPT, gsl_shell_app::on_lua_interrupt),
};

FXIMPLEMENT(gsl_shell_app,FXApp,gsl_shell_app_map,ARRAYNUMBER(gsl_shell_app_map))

gsl_shell_app::gsl_shell_app() : FXApp("GSL Shell", "GSL Shell")
{
  m_signal = new FXGUISignal(this, this, ID_LUA_INTERRUPT);
}


long gsl_shell_app::on_lua_interrupt(FXObject*, FXSelector, void*)
{
  m_lua_int.lock();
  mutex().unlock(); // FXApp unlock
  m_lua_int.signal();
  mutex().lock();
  m_lua_int.unlock();
  return 1;
}
