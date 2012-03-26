#ifndef FOXGUI_LUA_ENGINE_H
#define FOXGUI_LUA_ENGINE_H

extern "C" {
#include "lua.h"
}

#include "pthreadpp.h"
#include "gsl_shell_interp.h"
#include "redirect.h"

class lua_engine {
public:
  enum engine_status_e { starting, ready, busy, terminated };
  enum { eot_character = 0x04 };

  lua_engine();
  ~lua_engine();

  void input(const char* line);
  void start();
  void run();
  void set_state(engine_status_e s) { m_status = s; }
  bool is_ready();
  void start_gsl_shell() { m_gsl_shell.init(); }
  int read(char* buffer, unsigned buffer_size);

  int eval_status() const { return m_eval_status; }
  const char* error_msg() const { return m_gsl_shell.error_msg(); }

private:
  engine_status_e m_status;
  stdout_redirect m_redirect;
  pthread::cond m_eval;
  gsl_shell m_gsl_shell;
  const char* m_line_pending;
  int m_eval_status;
};

#endif
