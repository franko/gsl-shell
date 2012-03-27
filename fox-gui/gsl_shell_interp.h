#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

#include "defs.h"
#include "pthreadpp.h"

extern "C" {
#include "lua.h"
}

class gsl_shell {
  enum { ERROR_MSG_MAX_LENGTH = 128 };
public:
  enum eval_result_e { eval_success, eval_error, incomplete_input };

  gsl_shell(): m_lua_state(0) { }
  ~gsl_shell();

  void init();
  int exec(const char* line);
  const char* error_msg() const { return m_error_msg; }

  void lock();
  void unlock();

private:
  int report(lua_State* L, int status);

  lua_State* m_lua_state;
  pthread::mutex m_interp;
  char m_error_msg[ERROR_MSG_MAX_LENGTH];
};

#endif
