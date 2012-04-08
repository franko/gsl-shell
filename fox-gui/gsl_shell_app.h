#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "pthreadpp.h"
#include "agg_array.h"

class gsl_shell_app : public FXApp {
  FXDECLARE(gsl_shell_app)
public:
  gsl_shell_app();
  ~gsl_shell_app() { delete m_signal; }

  void interrupt()
  {
    m_lua_int.lock();
    m_signal->signal();
    m_lua_int.wait();
    mutex().lock();
    m_lua_int.unlock();
  }

  void resume()
  {
    mutex().unlock();
  }

  long on_lua_interrupt(FXObject*,FXSelector,void*);

  enum {
    ID_LUA_INTERRUPT = FXApp::ID_LAST,
    ID_LAST
  };

private:
  FXGUISignal* m_signal;
  pthread::cond m_lua_int;
};

#endif
