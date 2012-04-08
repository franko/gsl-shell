#ifndef FOXGUI_GSL_SHELL_APP_H
#define FOXGUI_GSL_SHELL_APP_H

#include <fx.h>

#include "pthreadpp.h"
#include "agg_array.h"

class gsl_shell_app : public FXApp {
  FXDECLARE(gsl_shell_app)
public:
  gsl_shell_app();
  ~gsl_shell_app();

  void interrupt()
  {
    fprintf(stderr, "INTERRUPTING: locking...\n");
    m_lua_int.lock();
    if (!m_waiting_lua)
      {
	fprintf(stderr, "INTERRUPTING: sending EVENT signal...\n");
	m_event_loop->signal();
	fprintf(stderr, "INTERRUPTING: waiting FOX main loop...\n");
	m_lua_int.wait();
      }
    m_lua_int.unlock();
    mutex().lock();
    fprintf(stderr, "INTERRUPTING: FOX main lock taken!\n");
  }

  void resume()
  {
    mutex().unlock();
    fprintf(stderr, "RESUME: locking...\n");
    m_lua_int.lock();
    fprintf(stderr, "RESUME: sending signal...\n");
    m_lua_int.signal();
    m_lua_int.unlock();
    fprintf(stderr, "RESUME: done.\n");
  }

  long on_lua_interrupt(FXObject*,FXSelector,void*);

  enum {
    ID_LUA_INTERRUPT = FXApp::ID_LAST,
    ID_LAST
  };

private:
  FXGUISignal* m_event_loop;
  pthread::mutex m_lua_handler;
  pthread::cond m_lua_int;
  //  pthread::cond m_lua_end;
  bool m_waiting_lua;
};

#endif
