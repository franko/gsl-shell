#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"

class gsl_shell_app : public FXApp {
  FXDECLARE(gsl_shell_app)
public:
  gsl_shell_app();
  ~gsl_shell_app();

  bool interrupt();
  void resume(bool signal_end);

  long on_lua_interrupt(FXObject*,FXSelector,void*);

  enum {
    ID_LUA_INTERRUPT = FXApp::ID_LAST,
    ID_LAST
  };

private:
  FXGUISignal* m_event_loop;
  FXCondition m_lua_int;
  bool m_waiting_lua;
};

#endif
