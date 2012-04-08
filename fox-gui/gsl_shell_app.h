#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "agg_array.h"

class gsl_shell_app : public FXApp {
  FXDECLARE(gsl_shell_app)
public:
  gsl_shell_app();
  ~gsl_shell_app() { delete m_signal; }

  void schedule_window(FXMainWindow* win) { m_windows_queue.add(win); }
  void lua_signal() { m_signal->signal(); }

  void spawn_scheduled_window()
  {
    while (m_windows_queue.size() > 0)
      {
	FXMainWindow* w = m_windows_queue.last();
	w->create();
	w->show(PLACEMENT_SCREEN);
	m_windows_queue.remove_last();
      }
  }

  long on_lua_interrupt(FXObject*,FXSelector,void*);

  enum {
    ID_LUA_INTERRUPT = FXApp::ID_LAST,
    ID_LAST
  };

private:
  agg::pod_bvector<FXMainWindow*> m_windows_queue;
  FXGUISignal* m_signal;
};

#endif
