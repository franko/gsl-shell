#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"
#include "gsl_shell_thread.h"

class gsl_shell_app : public FXApp {
  FXDECLARE(gsl_shell_app)
public:
  gsl_shell_app();
  ~gsl_shell_app();

  bool interrupt();
  void resume(bool signal_end);

  void window_create_request(FXMainWindow* win);

  long on_lua_interrupt(FXObject*,FXSelector,void*);
  long on_lua_request(FXObject*,FXSelector,void*);
  long on_window_close(FXObject*,FXSelector,void*);

  enum {
    ID_LUA_INTERRUPT = FXApp::ID_LAST,
    ID_LUA_REQUEST,
    ID_LAST
  };

private:
  gsl_shell_thread m_engine;
  FXGUISignal* m_event_loop;
  FXGUISignal* m_lua_request;
  FXMainWindow* m_lua_request_win;
  FXCondition m_lua_int;
  bool m_waiting_lua;
};

#endif
